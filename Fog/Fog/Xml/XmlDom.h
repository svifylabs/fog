// [Fog/Xml Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_XML_XMLDOM_H
#define _FOG_XML_XMLDOM_H

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
struct XmlDomReader;
struct XmlElement;
struct XmlElementIdHash;
struct XmlPI;
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
  // [Construction / Destruction]

protected:
  XmlAttribute(XmlElement* element, const String32& name);
  virtual ~XmlAttribute();

  // [Methods]

  FOG_INLINE XmlElement* element() const { return _element; }
  FOG_INLINE const String32& name() const { return _name._string; }

  virtual String32 value() const;
  virtual err_t setValue(const String32& value);

protected:
  //! @brief Link to element that owns this attribute.
  XmlElement* _element;
  //! @brief Attribute name (managed string).
  XmlString _name;
  //! @brief Attribute value (or empty if value is provided by overriden class).
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

  //! @brief Element type id.
  enum Type
  {
    TypeMask = 0x0F,
    TypeElement = 0x01,
    TypeText = 0x03,
    TypeCDATA = 0x04,
    TypePI = 0x07,
    TypeComment = 0x08,
    TypeDocument = 0x09,

    TypeSvgMask = 0x10,
    TypeSvgElement = TypeSvgMask | TypeElement,
    TypeSvgDocument = TypeSvgMask | TypeDocument
  };

  // [Element Flags]

  enum Flags
  {
    //! @brief Whether element can be manipulated (DOM).
    AllowedDomManipulation = 0x01,
    //! @brief Whether element tag name can be changed.
    AllowedTag = 0x02,
    //! @brief Whether element supports attributes.
    AllowedAttributes = 0x04,
    //! @brief Whether element supports adding or removing of attributes.
    //!
    //! This flag is usually used by extensions (for example SVG).
    AllowedAttributesAddRemove = 0x04
  };

  // [Type and Flags]

  //! @brief Return element type, see @c Type.
  FOG_INLINE uint32_t type() const { return _type; }

  //! @brief Return true if element is @a TypeElement type.
  FOG_INLINE bool isElement() const { return (_type & TypeMask) == TypeElement; }
  //! @brief Return true if element is @a TypeText type.
  FOG_INLINE bool isText() const { return (_type & TypeMask) == TypeText; }
  //! @brief Return true if element is @a TypeCDATA type.
  FOG_INLINE bool isCDATA() const { return (_type & TypeMask) == TypeCDATA; }
  //! @brief Return true if element is @a TypePI type.
  FOG_INLINE bool isPI() const { return (_type & TypeMask) == TypePI; }
  //! @brief Return true if element is @a TypeComment type.
  FOG_INLINE bool isComment() const { return (_type & TypeMask) == TypeComment; }
  //! @brief Return true if element is @a TypeDocument type.
  FOG_INLINE bool isDocument() const { return (_type & TypeMask) == TypeDocument; }

  FOG_INLINE bool isSvg() const { return (_type & TypeSvgMask) != 0; }
  FOG_INLINE bool isSvgElement() const { return _type == TypeSvgElement; }
  FOG_INLINE bool isSvgDocument() const { return _type == TypeSvgDocument; }

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

  bool hasAttribute(const String32& name) const;
  err_t getAttribute(const String32& name, String32& value) const;
  err_t setAttribute(const String32& name, const String32& value);
  err_t removeAttribute(const String32& name);
  err_t removeAllAttributes();

  virtual XmlAttribute* _createAttribute(const String32& name) const;

  static void copyAttributes(XmlElement* dst, XmlElement* src);

  // [ID]

  FOG_INLINE String32 id() const { return _id; }
  err_t setId(const String32& id);

  // [Element and Text]

  FOG_INLINE const String32& tagName() const { return _tagName._string; }
  virtual err_t setTagName(const String32& name);

  virtual String32 textContent() const;
  virtual err_t setTextContent(const String32& text);

protected:
  uint8_t _type;
  uint8_t _reserved0;
  uint8_t _reserved1;
  mutable uint8_t _dirty;
  uint32_t _flags;

  XmlDocument* _document;
  XmlElement* _parent;
  XmlElement* _firstChild;
  XmlElement* _lastChild;
  XmlElement* _nextSibling;
  XmlElement* _prevSibling;

  Vector<XmlAttribute*> _attributes;
  mutable Vector<XmlElement*> _children;

  //! @brief Element tag name.
  XmlString _tagName;

  //! @brief Element id.
  String32 _id;
  //! @brief Element id chain in @c XmlHashTable.
  XmlElement* _idNext;

private:
  friend struct XmlAttribute;
  friend struct XmlIdAttribute;
  friend struct XmlDocument;
  friend struct XmlElementIdHash;

  FOG_DISABLE_COPY(XmlElement)
};

// ============================================================================
// [Fog::XmlText]
// ============================================================================

struct FOG_API XmlText : public XmlElement
{
  // [Construction / Destruction]

  typedef XmlElement base;

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

  typedef XmlElement base;

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

  typedef XmlNoTextElement base;

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

  typedef XmlNoTextElement base;

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
// [Fog::XmlPI]
// ============================================================================

struct FOG_API XmlPI : public XmlNoTextElement
{
  // [Construction / Destruction]

  typedef XmlNoTextElement base;

  XmlPI(const String32& data = String32());
  virtual ~XmlPI();

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

  FOG_DISABLE_COPY(XmlPI)
};

// ============================================================================
// [Fog::XmlElementIdHash]
// ============================================================================

//! @brief Hash table used in @c XmlDocument. Do not use directly.
//! @internal
struct FOG_API XmlElementIdHash
{
  // [Construction / Destruction]

  XmlElementIdHash();
  ~XmlElementIdHash();

  void add(XmlElement* e);
  void remove(XmlElement* e);
  XmlElement* get(const String32& id) const;

private:
  void _rehash(sysuint_t capacity);

  // [Members]

  //! @brief Count of buckets.
  sysuint_t _capacity;
  //! @brief Count of nodes.
  sysuint_t _length;

  //! @brief Count of buckets we will expand to if length exceeds _expandLength.
  sysuint_t _expandCapacity;
  //! @brief Count of nodes to grow.
  sysuint_t _expandLength;

  //! @brief Count of buckeds we will shrink to if length gets _shinkLength.
  sysuint_t _shrinkCapacity;
  //! @brief Count of nodes to shrink.
  sysuint_t _shrinkLength;

  //! @brief Buckets.
  XmlElement** _buckets;

  XmlElement* _bucketsBuffer[16];
};

// ============================================================================
// [Fog::XmlDocument]
// ============================================================================

struct FOG_API XmlDocument : public XmlElement
{
  // [Construction / Destruction]

  typedef XmlElement base;

  XmlDocument();
  virtual ~XmlDocument();

  // [Clone]

  virtual XmlElement* clone() const;

  // [Root Node]

  //! @brief Set document root element to @a e.
  virtual err_t setDocumentRoot(XmlElement* e);

  //! @brief Return document root element.
  FOG_INLINE XmlElement* documentRoot() const { return _documentRoot; }

  // [Clear]

  virtual void clear();

  // [Document Extensions]

  virtual XmlElement* createElement(const String32& tagName);
  static XmlElement* createElementStatic(const String32& tagName);
  virtual XmlDomReader* createDomReader();

  // [Dom]

  XmlElement* getElementById(const String32& id) const;

  // [Read]

  virtual err_t readFile(const String32& fileName);
  virtual err_t readStream(Stream& stream);
  virtual err_t readMemory(const void* mem, sysuint_t size);
  virtual err_t readString(const String32& str);

  // [DOCTYPE]

  FOG_INLINE const String32& getDOCTYPE() const { return _doctype; }
  FOG_INLINE err_t setDOCTYPE(const String32& doctype) { return _doctype.set(doctype); }

protected:
  // [Managed Strings]

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
  String32 _getManagedString(const String32& resource);

  // [Members]

  //! @brief Document root.
  XmlElement* _documentRoot;

  //! @brief Hash table that contains all managed strings and reference counts.
  Hash<String32, sysuint_t> _managedStrings;

  //! @brief Hash table that contains all managed IDs.
  XmlElementIdHash _elementIdsHash;

  //! @brief DOCTYPE string.
  String32 _doctype;

private:
  friend struct XmlAttribute;
  friend struct XmlIdAttribute;
  friend struct XmlElement;

  FOG_DISABLE_COPY(XmlDocument)
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_XML_XMLDOM_H
