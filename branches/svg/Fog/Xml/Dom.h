// [Fog/Xml Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_XML_DOM_H
#define _FOG_XML_DOM_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Core/Hash.h>
#include <Fog/Core/String.h>
#include <Fog/Core/Vector.h>

//! @addtogroup Fog_Xml
//! @{

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct Stream;
struct XmlAttribute;
struct XmlCDATA;
struct XmlComment;
struct XmlDocument;
struct XmlElement;
struct XmlProcessingInstruction;
struct XmlReader;
struct XmlText;
struct XmlWriter;

// ============================================================================
// [Fog::XmlString]
// ============================================================================

//! @brief Xml managed string.
//!
//! Managed string is special class that contains string instance and pointer
//! to reference count. The manager is in this case XmlDocument instance defined
//! in XmlElement who owns the managed string.
//!
//! There are two states of managed string:
//! - managed - Managed string instance is connected with xml document.
//! - unmanaged - Managed string instance is not connected with xml document.
//!
//! The reason why to introduce this class is memory management of xml strings
//! for element tags and attribute names. There are only few unique element
//! names or attributes that can be shared between element and attribute
//! instances. If we can guarantee this, we can compare string equality only
//! by pointers to its data (or reference count pointer @c _refCount).
struct FOG_API XmlString
{
  // [Construction / Destruction]

  FOG_INLINE XmlString() :
    _refCount(NULL) {}
  FOG_INLINE XmlString(const String32& string) :
    _string(string), _refCount(NULL) {}
  FOG_INLINE ~XmlString() {}

  String32 _string;
  sysuint_t* _refCount;

private:
  friend struct XmlAttribute;
  friend struct XmlDocument;
  friend struct XmlElement;

  FOG_DISABLE_COPY(XmlString)
};

// ============================================================================
// [Fog::XmlAttribute]
// ============================================================================

struct FOG_API XmlAttribute
{
protected:
  XmlAttribute(XmlElement* element, const String32& name, const String32& value);
  ~XmlAttribute();

public:
  FOG_INLINE XmlElement* element() const { return _element; }

  FOG_INLINE const String32& name() const { return _name._string; }
  FOG_INLINE const String32& value() const { return _value; }

protected:
  XmlElement* _element;

  XmlString _name;
  String32 _value;

private:
  friend struct XmlElement;

  FOG_DISABLE_COPY(XmlAttribute)
};

// ============================================================================
// [Fog::XmlElement]
// ============================================================================

struct FOG_API XmlElement
{
  // [Construction / Destruction]

  XmlElement(const String32& tagName);
  virtual ~XmlElement();

  // [Element Type]

  //! @brief Element Type ID.
  enum Type
  {
    TypeElement = 1,
    TypeText = 3,
    TypeCDATA = 4,
    TypeProcessingInstruction = 7,
    TypeComment = 8,
    TypeDocument = 9
  };

  // [Type and Flags]

  //! @brief Return element type, see @c Type.
  FOG_INLINE uint32_t type() const { return _type; }

  //! @brief Return true if element is @a TypeElement type.
  FOG_INLINE bool isElement() const { return _type == TypeElement; }
  //! @brief Return true if element is @a TypeText type.
  FOG_INLINE bool isText() const { return _type == TypeText; }
  //! @brief Return true if element is @a TypeCDATA type.
  FOG_INLINE bool isCDATA() const { return _type == TypeCDATA; }
  //! @brief Return true if element is @a TypeComment type.
  FOG_INLINE bool isComment() const { return _type == TypeComment; }
  //! @brief Return true if element is @a TypeProcessingInstruction type.
  FOG_INLINE bool isProcessingInstruction() const { return _type == TypeProcessingInstruction; }
  //! @brief Return true if element is @a TypeDocument type.
  FOG_INLINE bool isDocument() const { return _type == TypeDocument; }

  // [Manage / Unmanage]

protected:
  //! @brief Start managing this element.
  virtual void _manage(XmlDocument* doc);
  //! @brief Stop managing this element.
  virtual void _unmanage();

  // [Clone]

public:
  //! @brief Clone this element with all children.
  virtual XmlElement* clone() const;

  // [Serialize]

  virtual void serialize(XmlWriter* writer);

  // [Normalize]

  //! @brief Normalize this element.
  //!
  //! Normalizing means to join text child elements and remove empty ones.
  virtual void normalize();

  // [Dom]

  //! @brief Prepend @a ch element to this element.
  err_t prependChild(XmlElement* ch);

  //! @brief Append @a ch element to this element.
  err_t appendChild(XmlElement* ch);

  //! @brief Remove @a ch element from this element.
  //!
  //! @note Element will not be deleted.
  err_t removeChild(XmlElement* ch);

  //! @brief Replace @a newch element with @a oldch one.
  //!
  //! @note @a oldch element will not be deleted.
  err_t replaceChild(XmlElement* newch, XmlElement* oldch);

  //! @brief Remove and delete child element @a ch.
  err_t deleteChild(XmlElement* ch);

  //! @brief Delete all children elements.
  err_t deleteAll();

  //! @brief Unlink this element from its parent.
  //!
  //! @note this operation is same as removing this node from parent by
  //! parent()->removeChild(this) call.
  err_t unlink();

  //! @brief Unlink, but not unmanage this element from document.
  //! @internal
  err_t _unlinkUnmanaged();

  //! @brief Returns true if @c e element is child of this element or all
  //! its descendents.
  bool contains(XmlElement* e, bool deep = false);

  //! @brief Return xml document.
  FOG_INLINE XmlDocument* document() const { return _document; }

  //! @brief Return parent node.
  FOG_INLINE XmlElement* parent() const { return _parent; }

  //! @brief Return array of child nodes.
  Vector<XmlElement*> childNodes() const;

  //! @brief Return first child node.
  FOG_INLINE XmlElement* firstChild() const { return _firstChild; }

  //! @brief Return last child node.
  FOG_INLINE XmlElement* lastChild() const { return _lastChild; }

  //! @brief Return next sibling node.
  FOG_INLINE XmlElement* nextSibling() const { return _nextSibling; }

  //! @brief Return previous sibling node.
  FOG_INLINE XmlElement* previousSibling() const { return _prevSibling; }

  //! @brief Return true if current node is first.
  FOG_INLINE bool isFirst() const { return _prevSibling == NULL; }

  //! @brief Return true if current node is last.
  FOG_INLINE bool isLast() const { return _nextSibling == NULL; }

  //! @brief Return true if current node has parent.
  FOG_INLINE bool hasParent() const { return _parent != NULL; }

  //! @brief Return true if current node has child nodes.
  FOG_INLINE bool hasChildNodes() const { return _firstChild != NULL; }
 
  Vector<XmlElement*> childNodesByTagName(const String32& tagName) const;

  FOG_INLINE XmlElement* firstChildByTagName(const String32& tagName) const
  { return _nextChildByTagName(_firstChild, tagName); }

  FOG_INLINE XmlElement* lastChildByTagName(const String32& tagName) const
  { return _previousChildByTagName(_lastChild, tagName); }

  FOG_INLINE XmlElement* nextChildByTagName(const String32& tagName) const
  { return _nextChildByTagName(_nextSibling, tagName); }

  FOG_INLINE XmlElement* previousChildByTagName(const String32& tagName) const
  { return _previousChildByTagName(_prevSibling, tagName); }

  static XmlElement* _nextChildByTagName(XmlElement* refElement, const String32& tagName);
  static XmlElement* _previousChildByTagName(XmlElement* refElement, const String32& tagName);

  // [Attributes]

  //! @brief Return true if current node contains attributes.
  FOG_INLINE bool hasAttributes() const { return _attributes.length() != 0; }

  //! @brief Return array of child nodes.
  FOG_INLINE Vector<XmlAttribute*> attributes() const { return _attributes; }

  bool hasAttribute(const String32& name);
  err_t setAttribute(const String32& name, const String32& value);
  err_t removeAttribute(const String32& name);
  err_t removeAllAttributes();

  // [Element and Text]

  FOG_INLINE const String32& tagName() const { return _tagName._string; }
  virtual err_t setTagName(const String32& name);

  virtual String32 textContent() const;
  virtual err_t setTextContent(const String32& text);

protected:
  uint32_t _type;
  mutable uint8_t _dirty;
  uint8_t _movable;
  uint8_t _attributesAllowed;
  uint8_t _tagNameAllowed;

  XmlDocument* _document;

  XmlElement* _parent;

  XmlElement* _firstChild;
  XmlElement* _lastChild;

  XmlElement* _nextSibling;
  XmlElement* _prevSibling;

  Vector<XmlAttribute*> _attributes;
  mutable Vector<XmlElement*> _children;

  XmlString _tagName;

private:
  friend struct XmlAttribute;
  friend struct XmlDocument;

  FOG_DISABLE_COPY(XmlElement)
};

// ============================================================================
// [Fog::XmlText]
// ============================================================================

struct FOG_API XmlText : public XmlElement
{
  // [Construction / Destruction]

  XmlText(const String32& data = String32());
  virtual ~XmlText();

  // [Clone]

  virtual XmlElement* clone() const;

  // [Text Specific]

  virtual String32 textContent() const;
  virtual err_t setTextContent(const String32& text);

  FOG_INLINE const String32& data() const { return _data; }
  err_t setData(const String32& data);
  err_t appendData(const String32& data);
  err_t deleteData();
  err_t insertData(sysuint_t start, const String32& data);
  err_t replaceData(sysuint_t start, sysuint_t len, const String32& data);

protected:
  String32 _data;

private:
  friend struct XmlElement;
  friend struct XmlDocument;

  FOG_DISABLE_COPY(XmlText)
};

// ============================================================================
// [Fog::XmlNoTextElement]
// ============================================================================

struct FOG_API XmlNoTextElement : public XmlElement
{
  // [Construction / Destruction]

  XmlNoTextElement(const String32& tagName);

  virtual String32 textContent() const;
  virtual err_t setTextContent(const String32& text);

private:
  FOG_DISABLE_COPY(XmlNoTextElement)
};

// ============================================================================
// [Fog::XmlComment]
// ============================================================================

struct FOG_API XmlComment : public XmlNoTextElement
{
  // [Construction / Destruction]

  XmlComment(const String32& data = String32());
  virtual ~XmlComment();

  // [Clone]

  virtual XmlElement* clone() const;

  // [Comment Specific]

  const String32& data() const;
  err_t setData(const String32& data);

protected:
  String32 _data;

private:
  friend struct XmlElement;
  friend struct XmlDocument;

  FOG_DISABLE_COPY(XmlComment)
};

// ============================================================================
// [Fog::XmlCDATA]
// ============================================================================

struct FOG_API XmlCDATA : public XmlNoTextElement
{
  // [Construction / Destruction]

  XmlCDATA(const String32& data = String32());
  virtual ~XmlCDATA();

  // [Clone]

  virtual XmlElement* clone() const;

  // [CDATA Specific]

  const String32& data() const;
  err_t setData(const String32& data);

protected:
  String32 _data;

private:
  friend struct XmlElement;
  friend struct XmlDocument;

  FOG_DISABLE_COPY(XmlCDATA)
};

// ============================================================================
// [Fog::XmlProcessingInstruction]
// ============================================================================

struct FOG_API XmlProcessingInstruction : public XmlNoTextElement
{
  // [Construction / Destruction]

  XmlProcessingInstruction(const String32& data = String32());
  virtual ~XmlProcessingInstruction();

  // [Clone]

  virtual XmlElement* clone() const;

  // [Processing Instruction Specific]

  const String32& data() const;
  err_t setData(const String32& data);

protected:
  String32 _data;

private:
  friend struct XmlElement;
  friend struct XmlDocument;

  FOG_DISABLE_COPY(XmlProcessingInstruction)
};

// ============================================================================
// [Fog::XmlDocument]
// ============================================================================

struct FOG_API XmlDocument : public XmlElement
{
  // [Construction / Destruction]

  XmlDocument();
  virtual ~XmlDocument();

  // [Clone]

  virtual XmlElement* clone() const;

  // [Root Node]

  //! @brief Set document root element to @a e.
  err_t setDocumentRoot(XmlElement* e);

  //! @brief Return document root element.
  XmlElement* documentRoot() const;

  void clear();

  err_t readFile(const String32& fileName);
  err_t readStream(Stream& stream);
  err_t readMemory(const void* mem, sysuint_t size);
  err_t readString(const String32& str);

  FOG_INLINE const String32& getDOCTYPE() const { return _doctype; }
  FOG_INLINE err_t setDOCTYPE(const String32& doctype) { return _doctype.set(doctype); }

protected:
  //! @brief Must be called to manage @a resource string when node is inserted
  //! into DOM where _document is not @c NULL.
  err_t _manageString(XmlString& resource);

  //! @brief Must be called to unmanage @a resource string when node is removed
  //! from DOM where _document is not @c NULL.
  //!
  //! For dom operation where dom element is removed and inserted into same
  //! document the @c _manageString() and @c _unmanageString() calls are not
  //! needed. But ensure that no one was called or bad things happen.
  err_t _unmanageString(XmlString& resource);

  //! @brief Get managed string from @a resource. The managed string means
  //! string that can be compared with all managed strings only by its data
  //! pointer. If returned string is empty the managed resource does not exists
  //! (this means that comparing will always fail).
  String32 _getManaged(const String32& resource);

  //! @brief Hash table that contains all managed strings and reference counts.
  Hash<String32, sysuint_t> _managedStrings;

  String32 _doctype;

private:
  friend struct XmlAttribute;
  friend struct XmlElement;

  FOG_DISABLE_COPY(XmlDocument)
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_XML_DOM_H
