// [Fog/Xml Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/Assert.h>
#include <Fog/Core/Hash.h>
#include <Fog/Core/String.h>
#include <Fog/Core/StringCache.h>
#include <Fog/Core/Strings.h>
#include <Fog/Core/Vector.h>
#include <Fog/Xml/Error.h>
#include <Fog/Xml/XmlDom.h>
#include <Fog/Xml/XmlEntity.h>
#include <Fog/Xml/XmlReader.h>
#include <Fog/Xml/XmlWriter.h>

namespace Fog {

// ============================================================================
// [Fog::XmlAttribute]
// ============================================================================

XmlAttribute::XmlAttribute(XmlElement* element, const String32& name) :
  _element(element)
{
  // Manage attribute name
  _name._string = name;
  if (_element->_document) _element->_document->_manageString(_name);
}

XmlAttribute::~XmlAttribute()
{
  if (_element->_document) _element->_document->_unmanageString(_name);
}

String32 XmlAttribute::value() const
{
  return _value;
}

err_t XmlAttribute::setValue(const String32& value)
{
  return _value.set(value);
}

// ============================================================================
// [Fog::XmlIdAttribute]
// ============================================================================

struct FOG_HIDDEN XmlIdAttribute : public XmlAttribute
{
  // [Construction / Destruction]

  XmlIdAttribute(XmlElement* element, const String32& name);
  virtual ~XmlIdAttribute();

  // [Methods]

  virtual err_t setValue(const String32& value);
};

XmlIdAttribute::XmlIdAttribute(XmlElement* element, const String32& name)
  : XmlAttribute(element, name)
{
}

XmlIdAttribute::~XmlIdAttribute()
{
  XmlElement* element = _element;
  XmlDocument* document = element->document();

  if (document && !element->_id.isEmpty()) document->_elementIdsHash.remove(element);
  element->_id.free();
}

err_t XmlIdAttribute::setValue(const String32& value)
{
  if (_value == value) return Error::Ok;

  XmlElement* element = _element;
  XmlDocument* document = element->document();

  if (document && !element->_id.isEmpty()) document->_elementIdsHash.remove(element);

  // When assigning, we will generate hash code and we are omitting to call
  // this function from XmlElementIdHash.
  element->_id = value;
  element->_id.toHashCode();
  _value = value;

  if (document && !element->_id.isEmpty()) document->_elementIdsHash.add(element);
  return Error::Ok;
}

// ============================================================================
// [Fog::XmlElementIdHash]
// ============================================================================

XmlElementIdHash::XmlElementIdHash() :
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

XmlElementIdHash::~XmlElementIdHash()
{
  if (_buckets != _bucketsBuffer) Memory::free(_buckets);
}

void XmlElementIdHash::add(XmlElement* e)
{
  uint32_t hashCode = e->_id._d->hashCode;
  uint32_t hashMod = hashCode % _capacity;

  XmlElement* node = _buckets[hashMod];
  XmlElement* prev = NULL;

  while (node)
  {
    prev = node;
    node = node->_idNext;
  }

  if (prev)
    prev->_idNext = e;
  else
    _buckets[hashMod] = e;
  if (++_length >= _expandLength) _rehash(_expandCapacity);
}

void XmlElementIdHash::remove(XmlElement* e)
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
        prev->_idNext = node->_idNext;
      else
        _buckets[hashMod] = node->_idNext;

      node->_idNext = NULL;
      if (--_length <= _shrinkLength) _rehash(_shrinkCapacity);
      return;
    }

    prev = node;
    node = node->_idNext;
  }
}

XmlElement* XmlElementIdHash::get(const String32& id) const
{
  uint32_t hashCode = id.toHashCode();
  uint32_t hashMod = hashCode % _capacity;

  XmlElement* node = _buckets[hashMod];
  while (node)
  {
    if (node->_id == id) return node;
    node = node->_idNext;
  }
  return NULL;
}

void XmlElementIdHash::_rehash(sysuint_t capacity)
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
      XmlElement* next = node->_idNext;

      XmlElement* newCur = newBuckets[hashMod];
      XmlElement* newPrev = NULL;
      while (newCur) { newPrev = newCur; newCur = newCur->_idNext; }

      if (newPrev)
        newPrev->_idNext = node;
      else
        newBuckets[hashMod] = node;
      node->_idNext = NULL;

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
// [Fog::XmlElement]
// ============================================================================

XmlElement::XmlElement(const String32& tagName) :
  _type(TypeElement),
  _dirty(0),
  _reserved0(0),
  _reserved1(0),
  _flags(AllowedDomManipulation |
         AllowedTag |
         AllowedAttributes |
         AllowedAttributesAddRemove),
  _document(NULL),
  _parent(NULL),
  _firstChild(NULL),
  _lastChild(NULL),
  _nextSibling(NULL),
  _prevSibling(NULL),
  _tagName(tagName),
  _idNext(NULL)
{
  if (_tagName._string.isEmpty())
    _tagName._string = fog_strings->get(STR_XML_unnamed);
}

XmlElement::~XmlElement()
{
  if (_firstChild) deleteAll();
  if (_parent) unlink();
}

void XmlElement::_manage(XmlDocument* doc)
{
  FOG_ASSERT(_document == NULL);
  FOG_ASSERT(_tagName._refCount == NULL);

  _document = doc;
  _document->_manageString(_tagName);
  if (!_id.isEmpty()) _document->_elementIdsHash.add(this);

  if (_document->_documentRoot == NULL && isElement())
  {
    _document->_documentRoot = this;
  }

  Vector<XmlAttribute*>::ConstIterator it(_attributes);
  for (it.toStart(); it.isValid(); it.toNext())
  {
    _document->_manageString(it.value()->_name);
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

  _document->_unmanageString(_tagName);
  if (!_id.isEmpty()) _document->_elementIdsHash.remove(this);

  Vector<XmlAttribute*>::ConstIterator it(_attributes);
  for (it.toStart(); it.isValid(); it.toNext())
  {
    _document->_unmanageString(it.value()->_name);
  }

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
  XmlElement* e = new(std::nothrow) XmlElement(_tagName._string);
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

    if (e->type() == TypeText)
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

Vector<XmlElement*> XmlElement::childNodesByTagName(const String32& tagName) const
{
  Vector<XmlElement*> elms;
  XmlElement* e = _firstChild;

  if (_document)
  {
    String32 tagNameM = _document->_getManagedString(tagName);
    if (!tagNameM.isEmpty())
    {
      while (e)
      {
        if (e->_tagName._string._d == tagNameM._d) elms.append(e);
        e = e->_nextSibling;
      }
    }
  }
  else
  {
    while (e)
    {
      if (e->_tagName._string == tagName) elms.append(e);
      e = e->_nextSibling;
    }
  }

  return elms;
}

XmlElement* XmlElement::_nextChildByTagName(XmlElement* refElement, const String32& tagName)
{
  XmlElement* e = refElement;
  if (e == NULL) return NULL;

  if (e->_document)
  {
    String32 tagNameM = e->_document->_getManagedString(tagName);
    if (tagNameM.isEmpty()) return NULL;

    do {
      if (e->_tagName._string._d == tagNameM._d) return e;
    } while ((e = e->_nextSibling));
  }
  else
  {
    do {
      if (e->_tagName._string == tagName) return e;
    } while ((e = e->_nextSibling));
  }

  return NULL;
}

XmlElement* XmlElement::_previousChildByTagName(XmlElement* refElement, const String32& tagName)
{
  XmlElement* e = refElement;
  if (e == NULL) return NULL;

  if (e->_document)
  {
    String32 tagNameM = e->_document->_getManagedString(tagName);
    if (tagNameM.isEmpty()) return NULL;

    do {
      if (e->_tagName._string._d == tagNameM._d) return e;
    } while ((e = e->_nextSibling));
  }
  else
  {
    do {
      if (e->_tagName._string == tagName) return e;
    } while ((e = e->_nextSibling));
  }

  return NULL;
}

bool XmlElement::hasAttribute(const String32& _name) const
{
  if ((_flags & AllowedAttributes) == 0) return false;
  if (_name.isEmpty()) return false;

  // If XmlElement is part of XmlDocument that manages some resources, we can
  // use some tricks to match XmlAttribute faster without comparing strings.
  Vector<XmlAttribute*>::ConstIterator it(_attributes);
  if (!it.isValid()) return false;

  if (_document && _attributes.length() > 4)
  {
    String32 name = _document->_getManagedString(name);
    if (name.isEmpty()) return false;

    do {
      // Managed strings are shared, so we need only to compare String::Data.
      if (it.value()->_name._string._d == name._d) return true;

      it.toNext();
    } while (it.isValid());
  }
  else
  {
    do {
      if (it.value()->_name._string == _name) return true;

      it.toNext();
    } while (it.isValid());
  }

  // Not found.
  return false;
}

err_t XmlElement::getAttribute(const String32& _name, String32& value) const
{
  if ((_flags & AllowedAttributes) == 0) return Error::XmlDomAttributesNotAllowed;
  if (_name.isEmpty()) return Error::XmlDomInvalidAttribute;

  // If XmlElement is part of XmlDocument that manages some resources, we can
  // use some tricks to match XmlAttribute faster without comparing strings.
  Vector<XmlAttribute*>::ConstIterator it(_attributes);
  if (!it.isValid()) return Error::XmlDomAttributeNotFound;

  if (_document && _attributes.length() > 4)
  {
    String32 name = _document->_getManagedString(name);
    if (name.isEmpty()) return Error::XmlDomAttributeNotFound;

    do {
      // Managed strings are shared, so we need only to compare String::Data.
      if (it.value()->_name._string._d == name._d)
      {
        return value.set(it.value()->value());
      }

      it.toNext();
    } while (it.isValid());
  }
  else
  {
    do {
      if (it.value()->_name._string == _name)
      {
        return value.set(it.value()->value());
      }

      it.toNext();
    } while (it.isValid());
  }

  // Not found.
  return Error::XmlDomAttributeNotFound;
}

err_t XmlElement::setAttribute(const String32& _name, const String32& value)
{
  if ((_flags & AllowedAttributes) == 0) return Error::XmlDomAttributesNotAllowed;
  if (_name.isEmpty()) return Error::XmlDomInvalidAttribute;

  XmlAttribute* a;
  if (!_attributes.isEmpty())
  {
    Vector<XmlAttribute*>::ConstIterator it(_attributes);

    // If XmlElement is part of XmlDocument that manages some resources, we can
    // use some tricks to match XmlAttribute faster without comparing strings.
    if (_document && _attributes.length() > 3)
    {
      String32 name = _document->_getManagedString(_name);
      if (!name.isEmpty())
      {
        do {
          // Managed strings are shared, so we need only to compare String::Data.
          if (it.value()->_name._string._d == name._d) { a = it.value(); goto done; }
          it.toNext();
        } while (it.isValid());
      }
    }
    else
    {
      do {
        if (it.value()->_name._string == _name) { a = it.value(); goto done; }
        it.toNext();
      } while (it.isValid());
    }
  }

notFound:
  // Attribute not found, create new one.
  if ((_flags & AllowedAttributesAddRemove) == 0) 
    return Error::XmlDomAttributesAddRemoveNotAllowed;

  a = _createAttribute(_name);
  if (!a) return Error::OutOfMemory;
  _attributes.append(a);

done:
  return a->setValue(value);
}

err_t XmlElement::removeAttribute(const String32& name)
{
  if ((_flags & (AllowedAttributes | AllowedAttributesAddRemove)) != 
                (AllowedAttributes | AllowedAttributesAddRemove))
  {
    if ((_flags & AllowedAttributes) == 0) return Error::XmlDomAttributesNotAllowed;
    if ((_flags & AllowedAttributesAddRemove) == 0) return Error::XmlDomAttributesAddRemoveNotAllowed;
  }

  if (name.isEmpty()) return Error::XmlDomInvalidAttribute;

  // If XmlElement is part of XmlDocument that manages some resources, we can
  // use some tricks to match XmlAttribute faster without comparing strings.
  Vector<XmlAttribute*>::ConstIterator it(_attributes);

  if (_document && _attributes.length() > 4)
  {
    String32 name = _document->_getManagedString(name);
    if (name.isEmpty()) return Error::XmlDomAttributeNotFound;

    do {
      // Managed strings are shared, so we need only to compare String::Data.
      if (it.value()->_name._string._d == name._d) goto found;
      it.toNext();
    } while (it.isValid());
  }
  else
  {
    do {
      if (it.value()->_name._string == name) goto found;
      it.toNext();
    } while (it.isValid());
  }

  // Attribute not found.
  return Error::XmlDomAttributeNotFound;

found:
  delete it.value();
  _attributes.removeAt(it.index());

  return Error::Ok;
}

err_t XmlElement::removeAllAttributes()
{
  if ((_flags & AllowedAttributes) == 0) return Error::XmlDomAttributesNotAllowed;

  Vector<XmlAttribute*>::ConstIterator it(_attributes);
  do {
    XmlAttribute* a = (XmlAttribute*)it.value();
    delete a;
    it.toNext();
  } while (it.isValid());
  _attributes.clear();

  return Error::Ok;
}

XmlAttribute* XmlElement::_createAttribute(const String32& name) const
{
  if (name == fog_strings->get(STR_XML_id))
    return new(std::nothrow) XmlIdAttribute(const_cast<XmlElement*>(this), name);
  else
    return new(std::nothrow) XmlAttribute(const_cast<XmlElement*>(this), name);
}

void XmlElement::copyAttributes(XmlElement* dst, XmlElement* src)
{
  Vector<XmlAttribute*>::ConstIterator it(src->_attributes);
  for (it.toStart(); it.isValid(); it.toNext())
  {
    dst->setAttribute(it.value()->name(), it.value()->value());
  }
}

err_t XmlElement::setId(const String32& id)
{
  return setAttribute(fog_strings->get(STR_XML_id), id);
}

err_t XmlElement::setTagName(const String32& name)
{
  if ((_flags & AllowedTag) == 0) return Error::XmlDomTagChangeNotAllowed;
  if (name.isEmpty()) return Error::XmlDomInvalidAttribute;

  if (_document)
  {
    err_t err = _document->_unmanageString(_tagName);
    if (err) return err;

    _tagName._string = name;
    return _document->_manageString(_tagName);
  }
  else
  {
    _tagName._string = name;
    return Error::Ok;
  }
}

String32 XmlElement::textContent() const
{
  // If we use the standard behavior that is in browsers, we should traverse
  // between all nodes and check for text content. I think this is right
  // behavior for us.

  // first try fast-path
  if (_firstChild == _lastChild && _firstChild->type() == TypeText)
  {
    return ((XmlText*)_firstChild)->_data;
  }
  else
  {
    String32 s;

    XmlElement* e = _firstChild;
    if (e)
    {
      do {
        s.append(e->textContent());
        e = e->_nextSibling;
      } while (e);
    }

    return s;
  }
}

err_t XmlElement::setTextContent(const String32& text)
{
  // First try fast-path.
  if (_firstChild == _lastChild && _firstChild->type() == TypeText)
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

XmlText::XmlText(const String32& data) :
  XmlElement(fog_strings->get(STR_XML__text)),
  _data(data)
{
  _type = TypeText;
  _flags &= ~(AllowedTag | AllowedAttributes | AllowedAttributesAddRemove);
}

XmlText::~XmlText()
{
}

XmlElement* XmlText::clone() const
{
  return new(std::nothrow) XmlText(_data);
}

String32 XmlText::textContent() const
{
  FOG_ASSERT(_type == TypeText);
  return _data;
}

err_t XmlText::setTextContent(const String32& text)
{
  FOG_ASSERT(_type == TypeText);
  return _data.set(text);
}

err_t XmlText::setData(const String32& data)
{
  FOG_ASSERT(_type == TypeText);
  return _data.set(data);
}

err_t XmlText::appendData(const String32& data)
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

err_t XmlText::insertData(sysuint_t start, const String32& data)
{
  FOG_ASSERT(_type == TypeText);
  return _data.insert(start, data);
}

err_t XmlText::replaceData(sysuint_t start, sysuint_t len, const String32& data)
{
  FOG_ASSERT(_type == TypeText);
  return _data.replace(Range(start, len), data);
}

// ============================================================================
// [Fog::XmlNoTextElement]
// ============================================================================

XmlNoTextElement::XmlNoTextElement(const String32& tagName) :
  XmlElement(tagName)
{
}

String32 XmlNoTextElement::textContent() const
{
  return String32();
}

err_t XmlNoTextElement::setTextContent(const String32& text)
{
  return Error::XmlDomNotATextNode;
}

// ============================================================================
// [Fog::XmlComment]
// ============================================================================

XmlComment::XmlComment(const String32& data) :
  XmlNoTextElement(fog_strings->get(STR_XML__comment)),
  _data(data)
{
  _type = TypeComment;
  _flags &= ~(AllowedTag | AllowedAttributes | AllowedAttributesAddRemove);
}

XmlComment::~XmlComment()
{
}

XmlElement* XmlComment::clone() const
{
  return new(std::nothrow) XmlComment(_data);
}

const String32& XmlComment::data() const
{
  FOG_ASSERT(_type == TypeComment);
  return _data;
}

err_t XmlComment::setData(const String32& data)
{
  FOG_ASSERT(_type == TypeComment);
  return _data.set(data);
}

// ============================================================================
// [Fog::XmlCDATA]
// ============================================================================

XmlCDATA::XmlCDATA(const String32& data) :
  XmlNoTextElement(fog_strings->get(STR_XML__cdata)),
  _data(data)
{
  _type = TypeCDATA;
  _flags &= ~(AllowedTag | AllowedAttributes | AllowedAttributesAddRemove);
}

XmlCDATA::~XmlCDATA()
{
}

XmlElement* XmlCDATA::clone() const
{
  return new(std::nothrow) XmlCDATA(_data);
}

const String32& XmlCDATA::data() const
{
  FOG_ASSERT(_type == TypeCDATA);
  return _data;
}

err_t XmlCDATA::setData(const String32& data)
{
  FOG_ASSERT(_type == TypeCDATA);
  return _data.set(data);
}

// ============================================================================
// [Fog::XmlPI]
// ============================================================================

XmlPI::XmlPI(const String32& data) :
  XmlNoTextElement(fog_strings->get(STR_XML__pi)),
  _data(data)
{
  _type = TypePI;
  _flags &= ~(AllowedTag | AllowedAttributes | AllowedAttributesAddRemove);
}

XmlPI::~XmlPI()
{
}

XmlElement* XmlPI::clone() const
{
  return new(std::nothrow) XmlPI(_data);
}

const String32& XmlPI::data() const
{
  FOG_ASSERT(_type == TypePI);
  return _data;
}

err_t XmlPI::setData(const String32& data)
{
  FOG_ASSERT(_type == TypePI);
  return _data.set(data);
}

// ============================================================================
// [Fog::XmlDocument]
// ============================================================================

XmlDocument::XmlDocument() :
  XmlElement(fog_strings->get(STR_XML__document)),
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

XmlElement* XmlDocument::createElement(const String32& tagName)
{
  return createElementStatic(tagName);
}

XmlElement* XmlDocument::createElementStatic(const String32& tagName)
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

XmlElement* XmlDocument::getElementById(const String32& id) const
{
  if (id.isEmpty()) return NULL;
  return _elementIdsHash.get(id);
}

err_t XmlDocument::readFile(const String32& fileName)
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

err_t XmlDocument::readString(const String32& str)
{
  clear();

  XmlDomReader* reader = createDomReader();
  if (!reader) return Error::OutOfMemory;

  err_t err = reader->parseString(str);
  delete reader;
  return err;
}

err_t XmlDocument::_manageString(XmlString& resource)
{
  sysuint_t* rc = (sysuint_t*)_managedStrings.get(resource._string);

  if (rc)
  {
    resource._refCount = rc;
    (*rc)++;
  }
  else
  {
    err_t err = _managedStrings.put(resource._string, 1, true);
    if (err) return err;

    Hash<String32, sysuint_t>::Node* node = _managedStrings._getNode(resource._string);
    node->key.squeeze();
    resource._refCount = &node->value;
    resource._string = node->key;
  }
  return Error::Ok;
}

err_t XmlDocument::_unmanageString(XmlString& resource)
{
  if (--(*resource._refCount) == 0)
  {
    _managedStrings.remove(resource._string);
  }

  resource._refCount = NULL;
  return Error::Ok;
}

String32 XmlDocument::_getManagedString(const String32& resource)
{
  Hash<String32, sysuint_t>::Node* node = _managedStrings._getNode(resource);

  if (node)
    return String32(node->key);
  else
    return String32();
}

} // Fog namespace
