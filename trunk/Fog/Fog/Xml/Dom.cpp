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
#include <Fog/Core/Vector.h>
#include <Fog/Xml/Dom.h>
#include <Fog/Xml/Entity.h>
#include <Fog/Xml/Error.h>
#include <Fog/Xml/Reader.h>
#include <Fog/Xml/Writer.h>

namespace Fog {

struct FOG_HIDDEN XmlLocal
{
  String32 xmlDocumentTagName;
  String32 xmlTextTagName;
  String32 xmlCommentTagName;
  String32 xmlCDATATagName;
  String32 xmlProcessingInstructionTagName;

  String32 xmlUnnamedTagName;
};

static Static<XmlLocal> xml_local;

// ============================================================================
// [Fog::XmlAttribute]
// ============================================================================

XmlAttribute::XmlAttribute(XmlElement* element, const String32& name, const String32& value) :
  _element(element),
  _value(value)
{
  // Manage attribute name
  _name._string = name;
  if (_element->_document) _element->_document->_manageString(_name);
}

XmlAttribute::~XmlAttribute()
{
  if (_element->_document) _element->_document->_unmanageString(_name);
}

// ============================================================================
// [Fog::XmlElement]
// ============================================================================

XmlElement::XmlElement(const String32& tagName) :
  _type(TypeElement),
  _dirty(0),
  _movable(true),
  _attributesAllowed(true),
  _tagNameAllowed(true),
  _document(NULL),
  _parent(NULL),
  _firstChild(NULL),
  _lastChild(NULL),
  _nextSibling(NULL),
  _prevSibling(NULL),
  _tagName(tagName)
{
  if (_tagName._string.isEmpty()) _tagName._string = xml_local->xmlUnnamedTagName;
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

  _document->_unmanageString(_tagName);

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
  if (!e) return NULL;

  Vector<XmlAttribute*>::ConstIterator it(_attributes);
  for (it.toStart(); it.isValid(); it.toNext())
  {
    e->setAttribute(it.value()->name(), it.value()->value());
  }

  e->setTagName(_tagName._string);
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

  if ( _document && _document != ch->_document) ch->_manage(_document);
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

  if ( _document && _document != ch->_document) ch->_manage(_document);
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
  if (!oldch->_movable) return Error::XmlDomNotAllowed;
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

  if ( _document && _document != newch->_document) newch->_manage(_document);
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
  if (!_movable) return Error::XmlDomNotAllowed;

  if (_document) _unmanage();
  return _unlinkUnmanaged();
}

err_t XmlElement::_unlinkUnmanaged()
{
  if (_parent == NULL) return Error::Ok;
  if (!_movable) return Error::XmlDomNotAllowed;

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
    String32 tagNameM = _document->_getManaged(tagName);
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
    String32 tagNameM = e->_document->_getManaged(tagName);
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
    String32 tagNameM = e->_document->_getManaged(tagName);
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


bool XmlElement::hasAttribute(const String32& _name)
{
  if (_name.isEmpty()) return false;

  // If XmlElement is part of XmlDocument that manages some resources, we can
  // use some tricks to match XmlAttribute faster without comparing strings.
  Vector<XmlAttribute*>::ConstIterator it(_attributes);
  if (!it.isValid()) return false;

  if (_document && _attributes.length() > 4)
  {
    String32 name = _document->_getManaged(name);
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

err_t XmlElement::setAttribute(const String32& _name, const String32& value)
{
  if (_name.isEmpty()) return Error::XmlDomInvalidAttribute;
  if (!_attributesAllowed) return Error::XmlDomNotAllowed;

  if (!_attributes.isEmpty())
  {
    Vector<XmlAttribute*>::ConstIterator it(_attributes);

    // If XmlElement is part of XmlDocument that manages some resources, we can
    // use some tricks to match XmlAttribute faster without comparing strings.
    if (_document && _attributes.length() > 3)
    {
      String32 name = _document->_getManaged(_name);
      if (!name.isEmpty())
      {
        do {
          // Managed strings are shared, so we need only to compare String::Data.
          if (it.value()->_name._string._d == name._d)
          {
            it.value()->_value = value;
            return Error::Ok;
          }

          it.toNext();
        } while (it.isValid());
      }
    }
    else
    {
      do {
        if (it.value()->_name._string == _name)
        {
          it.value()->_value = value;
          return Error::Ok;
        }

        it.toNext();
      } while (it.isValid());
    }
  }

  // Attribute not found, create new one.
  XmlAttribute* a = new(std::nothrow) XmlAttribute(this, _name, value);
  if (!a) return Error::OutOfMemory;

  _attributes.append(a);
  return Error::Ok;
}

err_t XmlElement::removeAttribute(const String32& name)
{
  if (name.isEmpty()) return Error::XmlDomInvalidAttribute;
  if (!_attributesAllowed) return Error::XmlDomNotAllowed;

  // If XmlElement is part of XmlDocument that manages some resources, we can
  // use some tricks to match XmlAttribute faster without comparing strings.
  Vector<XmlAttribute*>::ConstIterator it(_attributes);

  if (_document && _attributes.length() > 4)
  {
    String32 name = _document->_getManaged(name);
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
  Vector<XmlAttribute*>::ConstIterator it(_attributes);
  do {
    XmlAttribute* a = (XmlAttribute*)it.value();
    delete a;
    it.toNext();
  } while (it.isValid());
  _attributes.clear();

  return Error::Ok;
}

err_t XmlElement::setTagName(const String32& name)
{
  if (name.isEmpty()) return Error::XmlDomInvalidAttribute;
  if (!_tagNameAllowed) return Error::XmlDomNotAllowed;

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
  XmlElement(xml_local->xmlTextTagName),
  _data(data)
{
  _type = TypeText;
  _attributesAllowed = false;
  _tagNameAllowed = false;
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
  return Error::XmlDomNotAllowed;
}

// ============================================================================
// [Fog::XmlComment]
// ============================================================================

XmlComment::XmlComment(const String32& data) :
  XmlNoTextElement(xml_local->xmlCommentTagName),
  _data(data)
{
  _type = TypeComment;
  _attributesAllowed = false;
  _tagNameAllowed = false;
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
  XmlNoTextElement(xml_local->xmlCDATATagName),
  _data(data)
{
  _type = TypeCDATA;
  _attributesAllowed = false;
  _tagNameAllowed = false;
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
// [Fog::XmlProcessingInstruction]
// ============================================================================

XmlProcessingInstruction::XmlProcessingInstruction(const String32& data) :
  XmlNoTextElement(xml_local->xmlProcessingInstructionTagName),
  _data(data)
{
  _type = TypeProcessingInstruction;
  _attributesAllowed = false;
  _tagNameAllowed = false;
}

XmlProcessingInstruction::~XmlProcessingInstruction()
{
}

XmlElement* XmlProcessingInstruction::clone() const
{
  return new(std::nothrow) XmlProcessingInstruction(_data);
}

const String32& XmlProcessingInstruction::data() const
{
  FOG_ASSERT(_type == TypeProcessingInstruction);
  return _data;
}

err_t XmlProcessingInstruction::setData(const String32& data)
{
  FOG_ASSERT(_type == TypeProcessingInstruction);
  return _data.set(data);
}

// ============================================================================
// [Fog::XmlDocument]
// ============================================================================

XmlDocument::XmlDocument() :
  XmlElement(xml_local->xmlDocumentTagName)
{
  _type = TypeDocument;
  _movable = false;
  _tagNameAllowed = false;

  // We will link to self.
  _document = this;
}

XmlDocument::~XmlDocument()
{
  // Here is important to release all managed resources. Normally this is done
  // in XmlElement destructor, but if we go there, the managed hash table will
  // not exist at this time and segfault will occur. So destroy everythin here.
  removeAllAttributes();
  deleteAll();

  // And clear _document pointer
  _document = NULL;
}

XmlElement* XmlDocument::clone() const
{
  XmlDocument* doc = new(std::nothrow) XmlDocument();
  if (!doc) return NULL;

  // TODO:

  return doc;
}

err_t XmlDocument::setDocumentRoot(XmlElement* e)
{
  if (_firstChild) return Error::XmlDomDocumentHasAlreadyRoot;
  return appendChild(e);
}

XmlElement* XmlDocument::documentRoot() const
{
  return _firstChild;
}

void XmlDocument::clear()
{
  deleteAll();
}

err_t XmlDocument::readFile(const String32& fileName)
{
  clear();
  XmlDomReader reader(this);
  return reader.parseFile(fileName);
}

err_t XmlDocument::readStream(Stream& stream)
{
  clear();
  XmlDomReader reader(this);
  return reader.parseStream(stream);
}

err_t XmlDocument::readMemory(const void* mem, sysuint_t size)
{
  clear();
  XmlDomReader reader(this);
  return reader.parseMemory(mem, size);
}

err_t XmlDocument::readString(const String32& str)
{
  clear();
  XmlDomReader reader(this);
  return reader.parseString(str);
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

String32 XmlDocument::_getManaged(const String32& resource)
{
  Hash<String32, sysuint_t>::Node* node = _managedStrings._getNode(resource);

  if (node)
    return String32(node->key);
  else
    return String32();
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_xmldom_init(void)
{
  Fog::xml_local.init();
  Fog::xml_local->xmlDocumentTagName.set(Fog::StubAscii8("#document"));
  Fog::xml_local->xmlTextTagName.set(Fog::StubAscii8("#text"));
  Fog::xml_local->xmlCommentTagName.set(Fog::StubAscii8("#comment"));
  Fog::xml_local->xmlCDATATagName.set(Fog::StubAscii8("#cdata"));
  Fog::xml_local->xmlProcessingInstructionTagName.set(Fog::StubAscii8("#pi"));
  Fog::xml_local->xmlUnnamedTagName.set(Fog::StubAscii8("unnamed"));

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_xmldom_shutdown(void)
{
  Fog::xml_local.destroy();
}
