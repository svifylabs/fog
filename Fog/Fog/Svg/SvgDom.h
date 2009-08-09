// [Fog/Svg Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_SVG_SVGDOM_H
#define _FOG_SVG_SVGDOM_H

// [Dependencies]
#include <Fog/Core/Hash.h>
#include <Fog/Core/TypeInfo.h>
#include <Fog/Graphics/Path.h>
#include <Fog/Xml/XmlDom.h>

namespace Fog {

//! @addtogroup Fog_Svg
//! @{

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct SvgContext;

// ============================================================================
// [Fog::SvgUnit]
// ============================================================================

enum SvgUnit
{
  SvgUnitNone = 0,

  SvgUnitCm,
  SvgUnitPercent,
  SvgUnitPixel,
};

// ============================================================================
// [Fog::SvgCoord]
// ============================================================================

struct FOG_HIDDEN SvgCoord
{
  FOG_INLINE SvgCoord() {}
  FOG_INLINE SvgCoord(double value, uint32_t unit) : value(value), unit(unit) {}

  double value;
  uint32_t unit;
};

// ============================================================================
// [Fog::SvgStyleItem]
// ============================================================================

struct FOG_API SvgStyleItem
{
  SvgStyleItem();
  SvgStyleItem(const SvgStyleItem& other);
  ~SvgStyleItem();

  // [Name / Value]

  FOG_INLINE const String32& name() const { return _name; }
  err_t setName(const String32& name);

  FOG_INLINE const String32& value() const { return _value; }
  err_t setValue(const String32& value);

  // [Style Type]

  enum StyleType
  {
    // When adding value here, it's important to add value to Fog/Core/Strings.h
    // and Fog/Core/Strings.cpp. There are static strings for style names.
    StyleNone = 0,

    StyleClipPath,
    StyleClipRule,
    StyleEnableBackground,
    StyleFill,
    StyleFillOpacity,
    StyleFillRule,
    StyleFilter,
    StyleFontFamily,
    StyleFontSize,
    StyleLetterSpacing,
    StyleMask,
    StyleOpacity,
    StyleStopColor,
    StyleStopOpacity,
    StyleStroke,
    StyleStrokeDashArray,
    StyleStrokeLineCap,
    StyleStrokeLineJoin,
    StyleStrokeMiterLimit,
    StyleStrokeOpacity,
    StyleStrokeWidth,

    StyleCount
  };

  FOG_INLINE uint32_t styleType() const { return _styleType; }

  // [Value Type]

  enum ValueType
  {
    //! @brief Value is nothing special (not recognized style).
    ValueNone = 0,

    //! @brief Value is enumeration. To get it use @c value() or @c valueAsUInt32().
    ValueEnum,
    //! @brief Value is color. To get it use @c value() or @c valueAsUInt32().
    ValueColor,
    //! @brief Value is color or url (for gradients).
    ValueColorOrUrl,
    //! @brief Value is opacity. To get it use @c value() or @c valueAsDouble().
    ValueOpacity,
    //! @brief Value is string. To get it use @c value().
    ValueString,
    //! @brief Value is string. To get it use @c value() or @c valueAsDouble()
    //! and @c unitType().
    ValueUnit,
  };

  FOG_INLINE uint32_t valueType() const { return _valueType; }

  // [Unit Type]

  FOG_INLINE uint32_t unitType() const { return _unitType; }

  // [Is Valid]

  FOG_INLINE uint32_t isValid() const { return _isValid; }

  // [Value]

  FOG_INLINE SvgCoord valueAsCoord() const { return SvgCoord(_valueAsDouble, _unitType); }
  FOG_INLINE uint32_t valueAsUInt32() const { return _valueAsUInt32; }
  FOG_INLINE uint64_t valueAsUInt64() const { return _valueAsUInt64; }
  FOG_INLINE double valueAsDouble() const { return _valueAsDouble; }

  // [Operator Overload]

  SvgStyleItem& operator=(const SvgStyleItem& other);

protected:
  String32 _name;
  String32 _value;

  union {
    struct {
      uint16_t _styleType;
      uint16_t _valueType;
      uint16_t _unitType;
      uint16_t _isValid;
    };
    uint64_t _data0;
  };

  union {
    uint32_t _valueAsUInt32;
    uint64_t _valueAsUInt64;
    double _valueAsDouble;

    uint64_t _data1;
  };
};

} FOG_DECLARE_TYPEINFO(Fog::SvgCoord    , Fog::PrimitiveType) namespace Fog {
} FOG_DECLARE_TYPEINFO(Fog::SvgStyleItem, Fog::MoveableType ) namespace Fog {

// ============================================================================
// [Fog::SvgStyleAttribute]
// ============================================================================

//! @brief Attribute used for the svg styles.
struct FOG_API SvgStyleAttribute : public XmlAttribute
{
  // [Construction / Destruction]

  typedef XmlAttribute base;

  SvgStyleAttribute(XmlElement* element, const String32& name);
  virtual ~SvgStyleAttribute();

  // [Methods]

  virtual String32 value() const;
  virtual err_t setValue(const String32& value);

  // [Styles]

  FOG_INLINE const Vector<SvgStyleItem>& styles() const { return _styles; }

  String32 getStyle(const String32& name) const;
  err_t setStyle(const String32& name, const String32& value);

  // [Members]
protected:
  Vector<SvgStyleItem> _styles;

private:
  FOG_DISABLE_COPY(SvgStyleAttribute)
};

// ============================================================================
// [Fog::SvgEmbeddedAttribute]
// ============================================================================

//! @brief Embedded attributes are linked directly from |SvgElement|s.
struct FOG_API SvgEmbeddedAttribute : public XmlAttribute
{
  // [Construction / Destruction]

  typedef XmlAttribute base;

  SvgEmbeddedAttribute(XmlElement* element, const String32& name, uint32_t id);
  virtual ~SvgEmbeddedAttribute();

  // [Methods]

  FOG_INLINE uint32_t id() const { return _id; }

  // [Members]

protected:
  uint32_t _id;

private:
  FOG_DISABLE_COPY(SvgEmbeddedAttribute)
};

// ============================================================================
// [Fog::SvgCoordAttribute]
// ============================================================================

//! @brief Attribute used for the svg coordinates.
struct FOG_API SvgCoordAttribute : public SvgEmbeddedAttribute
{
  // [Construction / Destruction]

  typedef SvgEmbeddedAttribute base;

  SvgCoordAttribute(XmlElement* element, const String32& name, uint32_t id);
  virtual ~SvgCoordAttribute();

  // [Methods]

  virtual err_t setValue(const String32& value);

  // [Coords]

  FOG_INLINE const SvgCoord& coord() const { return _coord; }

  // [Members]
protected:
  SvgCoord _coord;

private:
  FOG_DISABLE_COPY(SvgCoordAttribute)
};

// ============================================================================
// [Fog::SvgPathAttribute]
// ============================================================================

struct FOG_API SvgPathAttribute : public SvgEmbeddedAttribute
{
  // [Construction / Destruction]

  typedef SvgEmbeddedAttribute base;

  SvgPathAttribute(XmlElement* element, const String32& name, uint32_t id);
  virtual ~SvgPathAttribute();

  // [Methods]

  virtual err_t setValue(const String32& value);

  // [Coords]

  FOG_INLINE const Path& path() const { return _path; }

  // [Members]
protected:
  Path _path;

private:
  FOG_DISABLE_COPY(SvgPathAttribute)
};

// ============================================================================
// [Fog::SvgPointsAttribute]
// ============================================================================

struct FOG_API SvgPointsAttribute : public SvgEmbeddedAttribute
{
  // [Construction / Destruction]

  typedef SvgEmbeddedAttribute base;

  SvgPointsAttribute(XmlElement* element, const String32& name, uint32_t id);
  virtual ~SvgPointsAttribute();

  // [Methods]

  virtual err_t setValue(const String32& value);

  // [Coords]

  FOG_INLINE const Path& path() const { return _path; }

  // [Members]
protected:
  Path _path;

private:
  FOG_DISABLE_COPY(SvgPointsAttribute)
};

// ============================================================================
// [Fog::SvgAttributeDef]
// ============================================================================

struct FOG_HIDDEN SvgAttributeDef
{
  const char name[16];
  uint16_t def;
  uint16_t id;

  enum Def
  {
    DefEnd = 0,

    DefCoord,
    DefPath,
    DefPoints
  };
};

// ============================================================================
// [Fog::SvgElement]
// ============================================================================

struct FOG_API SvgElement : public XmlElement
{
  // [Construction / Destruction]

  typedef XmlElement base;

  SvgElement(uint32_t svgType);
  virtual ~SvgElement();

  // [SVG Type]

  enum SvgType
  {
    SvgTypeNone,

    SvgTypeDef,
    SvgTypeCircle,
    SvgTypeEllipse,
    SvgTypeG,
    SvgTypeLine,
    SvgTypePath,
    SvgTypeRect,
    SvgTypePolygon,
    SvgTypePolyLine,
  };

  FOG_INLINE uint32_t svgType() const { return _svgType; }

  // [Clone]

  virtual SvgElement* clone() const;

  // [Attributes]

  virtual XmlAttribute* _createAttribute(const Fog::String32& name) const;

  // [Embedded]

  virtual const SvgAttributeDef* getEmbeddedAttributesDef() const;
  virtual SvgEmbeddedAttribute** getEmbeddedAttributesPtr() const;

  // [Style]

  String32 getStyle(const String32& name) const;
  err_t setStyle(const String32& name, const String32& value);

  // [SVG Rendering]

  virtual err_t render(SvgContext* context) const;
  virtual err_t renderShape(SvgContext* context) const;

  static err_t _walkAndRender(const XmlElement* root, SvgContext* context);

protected:
  uint32_t _svgType;
  SvgStyleAttribute* _styles;

private:
  friend struct SvgStyleAttribute;

  FOG_DISABLE_COPY(SvgElement)
};

// ============================================================================
// [Fog::SvgShapeElement]
// ============================================================================

struct FOG_API SvgShapeElement : public SvgElement
{
  // [Construction / Destruction]

  typedef SvgElement base;

  SvgShapeElement(uint32_t svgType);
  virtual ~SvgShapeElement();

  // [Embedded]

  virtual SvgEmbeddedAttribute** getEmbeddedAttributesPtr() const;

  // [SVG Rendering]

  virtual err_t renderShape(SvgContext* context) const;

protected:
  SvgEmbeddedAttribute* _data[6];

private:
  FOG_DISABLE_COPY(SvgShapeElement)
};

// ============================================================================
// [Fog::SvgDocument]
// ============================================================================

struct FOG_API SvgDocument : public XmlDocument
{
  // [Construction / Destruction]

  typedef XmlDocument base;

  SvgDocument();
  virtual ~SvgDocument();

  // [Clone]

  virtual XmlElement* clone() const;

  // [Document Extensions]

  virtual XmlElement* createElement(const String32& tagName);
  static XmlElement* createElementStatic(const String32& tagName);

  // [Rendering]

  virtual err_t render(SvgContext* context) const;

private:
  FOG_DISABLE_COPY(SvgDocument)
};

//! @}

} // Fog namespace

// [Guard]
#endif // _FOG_SVG_SVGDOM_H
