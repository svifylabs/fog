// [Fog/Xml Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled Headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/Assert.h>
#include <Fog/Core/Hash.h>
#include <Fog/Core/ManagedString.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/String.h>
#include <Fog/Core/Strings.h>
#include <Fog/Core/StringUtil.h>
#include <Fog/Core/Vector.h>
#include <Fog/Xml/Error.h>
#include <Fog/Xml/XmlDom.h>
#include <Fog/Xml/XmlEntity.h>
#include <Fog/Xml/XmlReader.h>
#include <Fog/Xml/XmlWriter.h>

namespace Fog {

// ============================================================================
// [Fog::XmlAttributesManager]
// ============================================================================

XmlAttributesManager::XmlAttributesManager()
{
  _capacity = FOG_ARRAY_SIZE(_bucketsBuffer);
  _length = 0;
  _buckets = _bucketsBuffer;

  Memory::zero(_buckets, FOG_ARRAY_SIZE(_bucketsBuffer) * sizeof(XmlAttribute*));
}

XmlAttributesManager::~XmlAttributesManager()
{
  if (_buckets && _buckets != _bucketsBuffer) Memory::free(_buckets);
}

void XmlAttributesManager::add(XmlAttribute* a)
{
  uint32_t hashCode = a->_name.getHashCode();
  uint32_t hashMod = hashCode % _capacity;

  a->_hashNext = _buckets[hashMod];
  _buckets[hashMod] = a;

  sysuint_t expand;

  if (_buckets == _bucketsBuffer)
    expand = FOG_ARRAY_SIZE(_bucketsBuffer);
  else
    expand = (_capacity >> 1) + (_capacity >> 2) + (_capacity >> 3);

  if (++_length >= expand) _rehash(_capacity << 1);
  _list.append(a);
}

void XmlAttributesManager::remove(XmlAttribute* a)
{
  uint32_t hashCode = a->_name.getHashCode();
  uint32_t hashMod = hashCode % _capacity;

  XmlAttribute* node = _buckets[hashMod];
  XmlAttribute* prev = NULL;

  while (node)
  {
    if (node == a)
    {
      if (prev)
        prev->_hashNext = node->_hashNext;
      else
        _buckets[hashMod] = node->_hashNext;

      node->_hashNext = NULL;
      _list.removeAt(_list.indexOf(node));

      sysuint_t shrink;

      if (_buckets == _bucketsBuffer)
        shrink = 0;
      else
        shrink = (_capacity >> 2);

      if (--_length <= shrink) _rehash(_capacity >> 1);
      return;
    }

    prev = node;
    node = node->_hashNext;
  }
}

void XmlAttributesManager::removeAll()
{
  _capacity = FOG_ARRAY_SIZE(_bucketsBuffer);
  _length = 0;
  if (_buckets != _bucketsBuffer) Memory::free(_buckets);
  _buckets = _bucketsBuffer;

  Vector<XmlAttribute*>::ConstIterator it(_list);
  for (it.toStart(); it.isValid(); it.toNext()) it.value()->_hashNext = NULL;
  _list.free();
}

XmlAttribute* XmlAttributesManager::get(const String& name) const
{
  uint32_t hashCode = name.getHashCode();
  uint32_t hashMod = hashCode % _capacity;

  XmlAttribute* a = _buckets[hashMod];
  while (a)
  {
    if (a->getName() == name) return a;
    a = a->_hashNext;
  }
  return NULL;
}

void XmlAttributesManager::_rehash(sysuint_t capacity)
{
  if (capacity <= FOG_ARRAY_SIZE(_bucketsBuffer))
  {
    capacity = FOG_ARRAY_SIZE(_bucketsBuffer);
    if (_buckets == _bucketsBuffer) return;
  }
  XmlAttribute** oldBuckets = _buckets;
  XmlAttribute** newBuckets = (capacity == FOG_ARRAY_SIZE(_bucketsBuffer))
    ? (XmlAttribute**)_bucketsBuffer
    : (XmlAttribute**)Memory::calloc(sizeof(XmlAttribute*) * capacity);
  if (!newBuckets) return;

  sysuint_t i, len = _capacity;
  for (i = 0; i < len; i++)
  {
    XmlAttribute* node = oldBuckets[i];
    while (node)
    {
      uint32_t hashMod = node->_name.getHashCode() % capacity;
      XmlAttribute* next = node->_hashNext;

      node->_hashNext = newBuckets[hashMod];
      newBuckets[hashMod] = node;

      node = next;
    }
  }

  _capacity = capacity;

  AtomicBase::ptr_setXchg(&_buckets, newBuckets);
  if (oldBuckets != _bucketsBuffer) Memory::free(oldBuckets);
}

// ============================================================================
// [Fog::XmlAttribute]
// ============================================================================

XmlAttribute::XmlAttribute(XmlElement* element, const ManagedString& name, int offset) :
  _element(element),
  _name(name),
  _hashNext(NULL),
  _offset(offset)
{
}

XmlAttribute::~XmlAttribute()
{
}

String XmlAttribute::getValue() const
{
  return _value;
}

err_t XmlAttribute::setValue(const String& value)
{
  return _value.set(value);
}

void XmlAttribute::destroy()
{
  if (_offset == -1)
  {
    delete this;
  }
  else
  {
    _hashNext = NULL;
    _value.free();
  }
}

// ============================================================================
// [Fog::XmlIdAttribute]
// ============================================================================

struct FOG_HIDDEN XmlIdAttribute : public XmlAttribute
{
  // [Construction / Destruction]

  XmlIdAttribute(XmlElement* element, const ManagedString& name);
  virtual ~XmlIdAttribute();

  // [Methods]

  virtual err_t setValue(const String& value);
};

XmlIdAttribute::XmlIdAttribute(XmlElement* element, const ManagedString& name)
  : XmlAttribute(element, name)
{
}

XmlIdAttribute::~XmlIdAttribute()
{
  XmlElement* element = _element;
  XmlDocument* document = element->getDocument();

  if (document && !element->_id.isEmpty()) document->_elementIdsHash.remove(element);
  element->_id.free();
}

err_t XmlIdAttribute::setValue(const String& value)
{
  if (_value == value) return Error::Ok;

  XmlElement* element = _element;
  XmlDocument* document = element->getDocument();

  if (document && !element->_id.isEmpty()) document->_elementIdsHash.remove(element);

  // When assigning, we will generate hash code and we are omitting to call
  // this function from XmlIdManager.
  element->_id = value;
  element->_id.getHashCode();
  _value = value;

  if (document && !element->_id.isEmpty()) document->_elementIdsHash.add(element);
  return Error::Ok;
}

// ============================================================================
// [Fog::XmlElement]
// ============================================================================

XmlElement::XmlElement(const ManagedString& tagName) :
  _type(TypeElement),
  _dirty(0),
  _reserved0(0),
  _reserved1(0),
  _flags(AllowedDomManipulation | AllowedTag | AllowedAttributes),
  _document(NULL),
  _parent(NULL),
  _firstChild(NULL),
  _lastChild(NULL),
  _nextSibling(NULL),
  _prevSibling(NULL),
  _attributesManager(NULL),
  _tagName(tagName),
  _hashNextId(NULL)
{
  if (_tagName.isEmpty()) _tagName = fog_strings->getString(STR_XML_unnamed);
}

XmlElement::~XmlElement()
{
  if (_firstChild) deleteAll();
  if (_parent) unlink();
}

void XmlElement::_manage(XmlDocument* doc)
{
  FOG_ASSERT(_document == NULL);

  _document = doc;
  if (!_id.isEmpty()) _document->_elementIdsHash.add(this);

  if (_document->_documentRoot == NULL && isElement())
  {
    _document->_documentRoot = this;
  }

  XmlElement* e = _firstChild;
  while (e)
  {
    e->_manage(doc);
    e = e->_nextSibling;
  }
}

void XmlElement::_unmanage()
{
  FOG_ASSERT(_document != NULL);

  if (_document->_documentRoot == this) _document->_documentRoot = NULL;

  if (!_id.isEmpty()) _document->_elementIdsHash.remove(this);

  XmlElement* e = _firstChild;
  while (e)
  {
    e->_unmanage();
    e = e->_nextSibling;
  }

  _document = NULL;
}

XmlElement* XmlElement::clone() const
{
  XmlElement* e = new(std::nothrow) XmlElement(_tagName);
  if (e) copyAttributes(e, const_cast<XmlElement*>(this));
  return e;
}

void XmlElement::serialize(XmlWriter* writer)
{
}

void XmlElement::normalize()
{
  // Normalizing means to remove empty text nodes and join siblings.
  XmlElement* e = _firstChild;
  XmlText* text = NULL;

  while (e)
  {
    XmlElement* next = e->_nextSibling;

    if (e->getType() == TypeText)
    {
      if (((XmlText *)e)->_data.isEmpty())
      {
        // Remove empty text node.
        deleteChild(e);
      }
      else if (text != NULL)
      {
        // text is not NULL, this means that we have previous text node, so
        // we will join there this text node.
        text->appendData(((XmlText*)e)->_data);
        deleteChild(e);
      }
      else
      {
        text = (XmlText*)e;
      }
    }
    else
    {
      e->normalize();
      text = NULL;
    }

    e = next;
  }
}

err_t XmlElement::prependChild(XmlElement* ch)
{
  if (ch->contains(this, true)) return Error::XmlDomCyclic;

  err_t err = Error::Ok;
  if (_document == ch->_document)
    err = ch->_unlinkUnmanaged();
  else if (ch->_parent)
    err = ch->unlink();
  if (err) return err;

  ch->_parent = this;
  ch->_nextSibling = _firstChild;

  if (!hasChildNodes())
  {
    _firstChild = ch;
    _lastChild = ch;
  }
  else
  {
    _firstChild->_prevSibling = ch;
    _firstChild = ch;
  }

  _dirty = true;

  if ( _document)
  {
    if (_document != ch->_document) ch->_manage(_document);
  }
  return Error::Ok;
}

err_t XmlElement::appendChild(XmlElement* ch)
{
  if (ch->contains(this, true)) return Error::XmlDomCyclic;

  err_t err = Error::Ok;
  if (_document == ch->_document)
    err = ch->_unlinkUnmanaged();
  else if (ch->_parent)
    err = ch->unlink();

  if (err) return err;

  ch->_parent = this;
  ch->_prevSibling = _lastChild;

  if (!hasChildNodes())
  {
    _firstChild = ch;
    _lastChild = ch;
  }
  else
  {
    _lastChild->_nextSibling = ch;
    _lastChild = ch;
  }

  _dirty = true;

  if ( _document)
  {
    if (_document != ch->_document) ch->_manage(_document);
  }
  return Error::Ok;
}

err_t XmlElement::removeChild(XmlElement* ch)
{
  if (ch->_parent == this)
    return ch->unlink();
  else
    return Error::XmlDomInvalidChild;
}

err_t XmlElement::replaceChild(XmlElement* newch, XmlElement* oldch)
{
  if (oldch == NULL) return appendChild(newch);

  if (oldch->_parent != this) return Error::XmlDomInvalidChild;
  if ((oldch->_flags & AllowedDomManipulation) == 0) return Error::XmlDomManipulationNotAllowed;
  if (newch->contains(this, true)) return Error::XmlDomCyclic;

  err_t err = Error::Ok;
  if (_document == newch->_document)
    err = newch->_unlinkUnmanaged();
  else if (newch->_parent)
    err = newch->unlink();
  if (err) return err;

  newch->_parent = this;
  newch->_prevSibling = oldch->_prevSibling;
  newch->_nextSibling = oldch->_nextSibling;

  oldch->_unmanage();
  oldch->_parent = NULL;
  oldch->_prevSibling = NULL;
  oldch->_nextSibling = NULL;

  if (newch->_prevSibling == NULL)
    _firstChild = newch;
  if (newch->_nextSibling == NULL)
    _lastChild = newch;

  if ( _document)
  {
    if (_document != newch->_document) newch->_manage(_document);
  }
  return Error::Ok;
}

err_t XmlElement::deleteChild(XmlElement* ch)
{
  if (ch->_parent == this)
  {
    err_t err = ch->unlink();
    if (err) return err;

    delete ch;
    return Error::Ok;
  }
  else
    return Error::XmlDomInvalidChild;
}

err_t XmlElement::deleteAll()
{
  XmlElement* e = _firstChild;
  if (e == NULL) return Error::Ok;

  do {
    XmlElement* next = e->_nextSibling;

    if (e->_document) e->_unmanage();
    e->_parent = NULL;
    e->_prevSibling = NULL;
    e->_nextSibling = NULL;
    delete e;

    e = next;
  } while (e);

  _firstChild = NULL;
  _lastChild = NULL;
  _dirty = 0;
  _children.free();

  return Error::Ok;
}

err_t XmlElement::unlink()
{
  if (_parent == NULL) return Error::Ok;
  if ((_flags & AllowedDomManipulation) == 0) return Error::XmlDomManipulationNotAllowed;

  if (_document) _unmanage();
  return _unlinkUnmanaged();
}

err_t XmlElement::_unlinkUnmanaged()
{
  if (_parent == NULL) return Error::Ok;
  if ((_flags & AllowedDomManipulation) == 0) return Error::XmlDomManipulationNotAllowed;

  XmlElement* next = _nextSibling;
  XmlElement* prev = _prevSibling;

  if (prev)
    prev->_nextSibling = next;
  else
    _parent->_firstChild = next;

  if (next)
    next->_prevSibling = prev;
  else
    _parent->_lastChild = prev;

  _parent->_dirty = true;

  _parent = NULL;
  _prevSibling = NULL;
  _nextSibling = NULL;

  return Error::Ok;
}

bool XmlElement::contains(XmlElement* e, bool deep)
{
  if (!deep) return e->_parent == this;

  XmlElement* cur = this;
  do {
    if (cur == e) return true;
    cur = cur->_parent;
  } while (cur);
  return false;
}

Vector<XmlElement*> XmlElement::childNodes() const
{
  if (_dirty)
  {
    _children.clear();
    XmlElement* e = _firstChild;
    while (e)
    {
      _children.append(e);
      e = e->_nextSibling;
    }
    _dirty = false;
  }

  return _children;
}

Vector<XmlElement*> XmlElement::childNodesByTagName(const String& tagName) const
{
  Vector<XmlElement*> elms;

  ManagedString tagNameM;
  if (tagNameM.setIfManaged(tagName) != Error::Ok) return elms;

  for (XmlElement* e = firstChild(); e; e = e->nextSibling())
  {
    if (e->_tagName == tagNameM) elms.append(e);
  }

  return elms;
}

XmlElement* XmlElement::_nextChildByTagName(XmlElement* refElement, const String& tagName)
{
  XmlElement* e = refElement;
  if (e == NULL) return e;

  while ((e = e->nextSibling()))
  {
    if (e->_tagName == tagName) break;
  }
  return e;
}

XmlElement* XmlElement::_previousChildByTagName(XmlElement* refElement, const String& tagName)
{
  XmlElement* e = refElement;
  if (e == NULL) return e;

  while ((e = e->previousSibling()))
  {
    if (e->_tagName == tagName) break;
  }
  return e;
}

Vector<XmlAttribute*> XmlElement::attributes() const
{
  if (_attributesManager) 
    return _attributesManager->_list;
  else
    return Vector<XmlAttribute*>();
}

bool XmlElement::hasAttribute(const String& name) const
{
  if (!_attributesManager) return false;
  return _attributesManager->get(name) != NULL;
}

err_t XmlElement::setAttribute(const String& name, const String& value)
{
  if ((_flags & AllowedAttributes) == 0) return Error::XmlDomAttributesNotAllowed;
  if (name.isEmpty()) return Error::XmlDomInvalidAttribute;

  XmlAttribute* a = NULL;

  if (_attributesManager == NULL)
  {
    _attributesManager = new(std::nothrow) XmlAttributesManager();
    if (_attributesManager == NULL) return Error::OutOfMemory;
  }
  else
  {
    a = _attributesManager->get(name);
  }

  // Attribute not found, create new one.
  if (a == NULL)
  {
    a = _createAttribute(ManagedString(name));
    if (!a) return Error::OutOfMemory;

    a->_element = this;
    _attributesManager->add(a);
  }

  return a->setValue(value);
}

String XmlElement::getAttribute(const String& name) const
{
  if (!_attributesManager) return String();
 
  XmlAttribute* a = _attributesManager->get(name);
  return a ? a->getValue() : String();
}

err_t XmlElement::removeAttribute(const String& name)
{
  if ((_flags & AllowedAttributes) == 0) return Error::XmlDomAttributesNotAllowed;
  if (name.isEmpty()) return Error::XmlDomInvalidAttribute;

  if (!_attributesManager) return Error::XmlDomAttributeNotFound;

  XmlAttribute* a = _attributesManager->get(name);
  if (!a) return Error::XmlDomAttributeNotFound;

  _attributesManager->remove(a);
  a->destroy();
  return Error::Ok;
}

err_t XmlElement::removeAllAttributes()
{
  if ((_flags & AllowedAttributes) == 0) return Error::XmlDomAttributesNotAllowed;
  if (!_attributesManager) return Error::Ok;

  Vector<XmlAttribute*> list = _attributesManager->_list;

  delete _attributesManager;
  _attributesManager = NULL;

  Vector<XmlAttribute*>::ConstIterator it(list);
  for (it.toStart(); it.isValid(); it.toNext())
  {
    XmlAttribute* a = (XmlAttribute*)it.value();
    a->destroy();
  }

  return Error::Ok;
}

XmlAttribute* XmlElement::_createAttribute(const ManagedString& name) const
{
  if (name == fog_strings->getString(STR_XML_id))
    return new(std::nothrow) XmlIdAttribute(const_cast<XmlElement*>(this), name);
  else
    return new(std::nothrow) XmlAttribute(const_cast<XmlElement*>(this), name);
}

void XmlElement::copyAttributes(XmlElement* dst, XmlElement* src)
{
  if (src->_attributesManager)
  {
    Vector<XmlAttribute*>::ConstIterator it(src->_attributesManager->_list);
    for (it.toStart(); it.isValid(); it.toNext())
    {
      dst->setAttribute(it.value()->getName(), it.value()->getValue());
    }
  }
}

err_t XmlElement::setId(const String& id)
{
  return setAttribute(fog_strings->getString(STR_XML_id), id);
}

err_t XmlElement::setTagName(const String& name)
{
  if ((_flags & AllowedTag) == 0) return Error::XmlDomTagChangeNotAllowed;
  if (name.isEmpty()) return Error::XmlDomInvalidTagName;

  return _tagName.set(name);
}

String XmlElement::getTextContent() const
{
  // If we use the standard behavior that is in browsers, we should traverse
  // between all nodes and check for text content. I think this is right
  // behavior for us.

  // First try fast-path.
  if (_firstChild == _lastChild && _firstChild->getType() == TypeText)
  {
    return ((XmlText*)_firstChild)->_data;
  }
  else
  {
    String s;
    for (XmlElement* e = firstChild(); e; e = e->nextSibling()) s.append(e->getTextContent());
    return s;
  }
}

err_t XmlElement::setTextContent(const String& text)
{
  // First try fast-path.
  if (_firstChild == _lastChild && _firstChild->getType() == TypeText)
  {
    ((XmlText*)_firstChild)->_data = text;
    return Error::Ok;
  }
  else
  {
    deleteAll();
    XmlText* e = new(std::nothrow) XmlText(text);
    if (!e) return Error::OutOfMemory;
    return appendChild(e);
  }
}

// ============================================================================
// [Fog::XmlText]
// ============================================================================

XmlText::XmlText(const String& data) :
  XmlElement(fog_strings->getString(STR_XML__text)),
  _data(data)
{
  _type = TypeText;
  _flags &= ~(AllowedTag | AllowedAttributes);
}

XmlText::~XmlText()
{
}

XmlElement* XmlText::clone() const
{
  return new(std::nothrow) XmlText(_data);
}

String XmlText::getTextContent() const
{
  FOG_ASSERT(_type == TypeText);
  return _data;
}

err_t XmlText::setTextContent(const String& text)
{
  FOG_ASSERT(_type == TypeText);
  return _data.set(text);
}

err_t XmlText::setData(const String& data)
{
  FOG_ASSERT(_type == TypeText);
  return _data.set(data);
}

err_t XmlText::appendData(const String& data)
{
  FOG_ASSERT(_type == TypeText);
  return _data.append(data);
}

err_t XmlText::deleteData()
{
  FOG_ASSERT(_type == TypeText);
  _data.clear();
  return Error::Ok;
}

err_t XmlText::insertData(sysuint_t start, const String& data)
{
  FOG_ASSERT(_type == TypeText);
  return _data.insert(start, data);
}

err_t XmlText::replaceData(sysuint_t start, sysuint_t len, const String& data)
{
  FOG_ASSERT(_type == TypeText);
  return _data.replace(Range(start, len), data);
}

// ============================================================================
// [Fog::XmlNoTextElement]
// ============================================================================

XmlNoTextElement::XmlNoTextElement(const ManagedString& tagName) :
  XmlElement(tagName)
{
}

String XmlNoTextElement::getTextContent() const
{
  return String();
}

err_t XmlNoTextElement::setTextContent(const String& text)
{
  return Error::XmlDomNotATextNode;
}

// ============================================================================
// [Fog::XmlComment]
// ============================================================================

XmlComment::XmlComment(const String& data) :
  XmlNoTextElement(fog_strings->getString(STR_XML__comment)),
  _data(data)
{
  _type = TypeComment;
  _flags &= ~(AllowedTag | AllowedAttributes);
}

XmlComment::~XmlComment()
{
}

XmlElement* XmlComment::clone() const
{
  return new(std::nothrow) XmlComment(_data);
}

const String& XmlComment::getData() const
{
  FOG_ASSERT(_type == TypeComment);
  return _data;
}

err_t XmlComment::setData(const String& data)
{
  FOG_ASSERT(_type == TypeComment);
  return _data.set(data);
}

// ============================================================================
// [Fog::XmlCDATA]
// ============================================================================

XmlCDATA::XmlCDATA(const String& data) :
  XmlNoTextElement(fog_strings->getString(STR_XML__cdata)),
  _data(data)
{
  _type = TypeCDATA;
  _flags &= ~(AllowedTag | AllowedAttributes);
}

XmlCDATA::~XmlCDATA()
{
}

XmlElement* XmlCDATA::clone() const
{
  return new(std::nothrow) XmlCDATA(_data);
}

const String& XmlCDATA::getData() const
{
  FOG_ASSERT(_type == TypeCDATA);
  return _data;
}

err_t XmlCDATA::setData(const String& data)
{
  FOG_ASSERT(_type == TypeCDATA);
  return _data.set(data);
}

// ============================================================================
// [Fog::XmlPI]
// ============================================================================

XmlPI::XmlPI(const String& data) :
  XmlNoTextElement(fog_strings->getString(STR_XML__pi)),
  _data(data)
{
  _type = TypePI;
  _flags &= ~(AllowedTag | AllowedAttributes);
}

XmlPI::~XmlPI()
{
}

XmlElement* XmlPI::clone() const
{
  return new(std::nothrow) XmlPI(_data);
}

const String& XmlPI::getData() const
{
  FOG_ASSERT(_type == TypePI);
  return _data;
}

err_t XmlPI::setData(const String& data)
{
  FOG_ASSERT(_type == TypePI);
  return _data.set(data);
}

// ============================================================================
// [Fog::XmlIdManager]
// ============================================================================

XmlIdManager::XmlIdManager() :
  _capacity(16),
  _length(0),
  _expandCapacity(64),
  _expandLength(16),
  _shrinkCapacity(0),
  _shrinkLength(0),
  _buckets(_bucketsBuffer)
{
  Memory::zero(_bucketsBuffer, sizeof(_bucketsBuffer));
}

XmlIdManager::~XmlIdManager()
{
  if (_buckets != _bucketsBuffer) Memory::free(_buckets);
}

void XmlIdManager::add(XmlElement* e)
{
  uint32_t hashCode = e->_id._d->hashCode;
  uint32_t hashMod = hashCode % _capacity;

  XmlElement* node = _buckets[hashMod];
  XmlElement* prev = NULL;

  while (node)
  {
    prev = node;
    node = node->_hashNextId;
  }

  if (prev)
    prev->_hashNextId = e;
  else
    _buckets[hashMod] = e;
  if (++_length >= _expandLength) _rehash(_expandCapacity);
}

void XmlIdManager::remove(XmlElement* e)
{
  uint32_t hashCode = e->_id._d->hashCode;
  uint32_t hashMod = hashCode % _capacity;

  XmlElement* node = _buckets[hashMod];
  XmlElement* prev = NULL;

  while (node)
  {
    if (node == e)
    {
      if (prev)
        prev->_hashNextId = node->_hashNextId;
      else
        _buckets[hashMod] = node->_hashNextId;

      node->_hashNextId = NULL;
      if (--_length <= _shrinkLength) _rehash(_shrinkCapacity);
      return;
    }

    prev = node;
    node = node->_hashNextId;
  }
}

XmlElement* XmlIdManager::get(const String& id) const
{
  uint32_t hashCode = id.getHashCode();
  uint32_t hashMod = hashCode % _capacity;

  XmlElement* node = _buckets[hashMod];
  while (node)
  {
    if (node->_id == id) return node;
    node = node->_hashNextId;
  }
  return NULL;
}

XmlElement* XmlIdManager::get(const Char* idStr, sysuint_t idLen) const
{
  uint32_t hashCode = HashUtil::hashString(idStr, idLen);
  uint32_t hashMod = hashCode % _capacity;

  XmlElement* node = _buckets[hashMod];
  while (node)
  {
    if (node->_id._d->hashCode == hashCode && StringUtil::eq(node->_id.cData(), idStr, idLen)) return node;
    node = node->_hashNextId;
  }
  return NULL;
}

void XmlIdManager::_rehash(sysuint_t capacity)
{
  XmlElement** oldBuckets = _buckets;
  XmlElement** newBuckets = (XmlElement**)Memory::calloc(sizeof(XmlElement*) * capacity);
  if (!newBuckets) return;

  sysuint_t i, len = _capacity;
  for (i = 0; i < len; i++)
  {
    XmlElement* node = oldBuckets[i];
    while (node)
    {
      uint32_t hashMod = node->_id._d->hashCode % capacity;
      XmlElement* next = node->_hashNextId;

      XmlElement* newCur = newBuckets[hashMod];
      XmlElement* newPrev = NULL;
      while (newCur) { newPrev = newCur; newCur = newCur->_hashNextId; }

      if (newPrev)
        newPrev->_hashNextId = node;
      else
        newBuckets[hashMod] = node;
      node->_hashNextId = NULL;

      node = next;
    }
  }

  _capacity = capacity;

  _expandCapacity = Hash_Abstract::_calcExpandCapacity(capacity);
  _expandLength = (sysuint_t)((sysint_t)_capacity * 0.92);

  _shrinkCapacity = Hash_Abstract::_calcShrinkCapacity(capacity);
  _shrinkLength = (sysuint_t)((sysint_t)_shrinkCapacity * 0.70);

  AtomicBase::ptr_setXchg(&_buckets, newBuckets);
  if (oldBuckets != _bucketsBuffer) Memory::free(oldBuckets);
}

// ============================================================================
// [Fog::XmlDocument]
// ============================================================================

XmlDocument::XmlDocument() :
  XmlElement(fog_strings->getString(STR_XML__document)),
  _documentRoot(NULL)
{
  _type = TypeDocument;
  _flags &= ~(AllowedDomManipulation | AllowedTag);

  // We will link to self.
  _document = this;
}

XmlDocument::~XmlDocument()
{
  // Here is important to release all managed resources. Normally this is done
  // in XmlElement destructor, but if we go there, the managed hash table will
  // not exist at this time and segfault will occur. So destroy everything here.
  removeAllAttributes();
  deleteAll();

  // And clear _document pointer
  _document = NULL;
}

XmlElement* XmlDocument::clone() const
{
  XmlDocument* doc = new(std::nothrow) XmlDocument();
  if (!doc) return NULL;

  for (XmlElement* ch = firstChild(); ch; ch = ch->nextSibling())
  {
    XmlElement* e = ch->clone();
    if (e && doc->appendChild(e) != Error::Ok) delete e;
  }

  return doc;
}

XmlElement* XmlDocument::createElement(const ManagedString& tagName)
{
  return createElementStatic(tagName);
}

XmlElement* XmlDocument::createElementStatic(const ManagedString& tagName)
{
  return new (std::nothrow) XmlElement(tagName);
}

XmlDomReader* XmlDocument::createDomReader()
{
  return new(std::nothrow) XmlDomReader(this);
}

err_t XmlDocument::setDocumentRoot(XmlElement* e)
{
  if (_firstChild) return Error::XmlDomDocumentHasAlreadyRoot;
  return appendChild(e);
}

void XmlDocument::clear()
{
  deleteAll();
}

XmlElement* XmlDocument::getElementById(const String& id) const
{
  if (id.isEmpty()) return NULL;
  return _elementIdsHash.get(id);
}

XmlElement* XmlDocument::getElementById(const Utf16& id) const
{
  const Char* idStr = id.getData();
  sysuint_t idLen = id.getLength();

  if (idLen == DetectLength) idLen = StringUtil::len(idStr);
  if (idLen == 0) return NULL;

  return _elementIdsHash.get(idStr, idLen);
}

err_t XmlDocument::readFile(const String& fileName)
{
  clear();

  XmlDomReader* reader = createDomReader();
  if (!reader) return Error::OutOfMemory;

  err_t err = reader->parseFile(fileName);
  delete reader;
  return err;
}

err_t XmlDocument::readStream(Stream& stream)
{
  clear();

  XmlDomReader* reader = createDomReader();
  if (!reader) return Error::OutOfMemory;

  err_t err = reader->parseStream(stream);
  delete reader;
  return err;
}

err_t XmlDocument::readMemory(const void* mem, sysuint_t size)
{
  clear();

  XmlDomReader* reader = createDomReader();
  if (!reader) return Error::OutOfMemory;

  err_t err = reader->parseMemory(mem, size);
  delete reader;
  return err;
}

err_t XmlDocument::readString(const String& str)
{
  clear();

  XmlDomReader* reader = createDomReader();
  if (!reader) return Error::OutOfMemory;

  err_t err = reader->parseString(str);
  delete reader;
  return err;
}

} // Fog namespace
