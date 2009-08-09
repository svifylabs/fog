// [Fog/Svg Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Dependencies]
#include <Fog/Core/Char.h>
#include <Fog/Core/String.h>
#include <Fog/Core/StringCache.h>
#include <Fog/Core/Strings.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/Painter.h>
#include <Fog/Svg/SvgContext.h>
#include <Fog/Svg/SvgDom.h>
#include <Fog/Svg/SvgUtil.h>

namespace Fog {

// ============================================================================
// [Fog::SvgStyleItem]
// ============================================================================

static const uint8_t svgStyleTypeToValueType[] =
{
  /* StyleNone             -> */ SvgStyleItem::ValueNone,
  /* StyleClipPath         -> */ 0,
  /* StyleClipRule         -> */ 0,
  /* StyleEnableBackground -> */ 0,
  /* StyleFill             -> */ SvgStyleItem::ValueColorOrUrl,
  /* StyleFillOpacity      -> */ SvgStyleItem::ValueOpacity,
  /* StyleFillRule         -> */ SvgStyleItem::ValueEnum,
  /* StyleFilter           -> */ 0,
  /* StyleFontFamily       -> */ SvgStyleItem::ValueString,
  /* StyleFontSize         -> */ SvgStyleItem::ValueUnit,
  /* StyleLetterSpacing    -> */ SvgStyleItem::ValueUnit,
  /* StyleMask             -> */ 0,
  /* StyleOpacity          -> */ SvgStyleItem::ValueOpacity,
  /* StyleStopColor        -> */ SvgStyleItem::ValueColor,
  /* StyleStopOpacity      -> */ SvgStyleItem::ValueOpacity,
  /* StyleStroke           -> */ SvgStyleItem::ValueColorOrUrl,
  /* StyleStrokeDashArray  -> */ 0,
  /* StyleStrokeLineCap    -> */ SvgStyleItem::ValueEnum,
  /* StyleStrokeLineJoin   -> */ SvgStyleItem::ValueEnum,
  /* StyleStrokeMiterLimit -> */ SvgStyleItem::ValueUnit,
  /* StyleStrokeOpacity    -> */ SvgStyleItem::ValueOpacity,
  /* StyleStrokeWidth      -> */ SvgStyleItem::ValueUnit
};

SvgStyleItem::SvgStyleItem() :
  _data0(FOG_UINT64_C(0)),
  _data1(FOG_UINT64_C(0))
{
}

SvgStyleItem::SvgStyleItem(const SvgStyleItem& other) : 
  _name(other._name),
  _value(other._value),
  _data0(other._data0),
  _data1(other._data1)
{
}

SvgStyleItem::~SvgStyleItem()
{
}

err_t SvgStyleItem::setName(const String32& name)
{
  _data0 = FOG_UINT64_C(0);
  _data1 = FOG_UINT64_C(0);

  // Match the name in string array (we get also StyleType ID).
  for (uint32_t styleId = 1; styleId < StyleCount; styleId++)
  {
    const String32& s = fog_strings->get(STR_SVG_STYLE_NAMES + styleId);
    if (s == name)
    {
      _name = s;
      _value.free();
      _styleType = styleId;
      FOG_ASSERT(styleId < FOG_ARRAY_SIZE(svgStyleTypeToValueType));
      _valueType = svgStyleTypeToValueType[styleId];
      return Error::Ok;
    }
  }

  // Not supported style. We will create it, but it will be ignored by 
  // svg rendering engine.
  _name = name;
  _value.free();
  return Error::Ok;
}

err_t SvgStyleItem::setValue(const String32& value)
{
  err_t err = _value.set(value);
  if (err) return err;

  switch (_valueType)
  {
    case ValueNone:
    {
      // SVG TODO:
      break;
    }
    case ValueColor:
    {
      _valueAsUInt32 = SvgUtil::parseColor(value);
      break;
    }
    case ValueColorOrUrl:
    {
      // SVG TODO: Url patterns.
      _valueAsUInt32 = SvgUtil::parseColor(value);
      break;
    }
    case ValueOpacity:
    {
      // SVG TODO:
      break;
    }
    case ValueString:
    {
      // SVG TODO:
      break;
    }
    case ValueUnit:
    {
      SvgCoord coord = SvgUtil::parseCoord(value);
      _valueAsDouble = coord.value;
      _unitType = coord.unit;
      _isValid = coord.unit != SvgUnitNone;
      break;
    }
    default:
      FOG_ASSERT_NOT_REACHED();
  }

  return err;
}

SvgStyleItem& SvgStyleItem::operator=(const SvgStyleItem& other)
{
  _name = other._name;
  _value = other._value;
  _data0 = other._data0;
  _data1 = other._data1;

  return *this;
}

// ============================================================================
// [Fog::SvgStyleAttribute]
// ============================================================================

SvgStyleAttribute::SvgStyleAttribute(XmlElement* element, const String32& name) : 
  XmlAttribute(element, name)
{
  reinterpret_cast<SvgElement*>(element)->_styles = this;
}

SvgStyleAttribute::~SvgStyleAttribute()
{
  reinterpret_cast<SvgElement*>(_element)->_styles = NULL;
}

String32 SvgStyleAttribute::value() const
{
  return SvgUtil::joinStyles(_styles);
}

err_t SvgStyleAttribute::setValue(const String32& value)
{
  _styles = SvgUtil::parseStyles(value);
  return Error::Ok;
}

String32 SvgStyleAttribute::getStyle(const String32& name) const
{
  Vector<SvgStyleItem>::ConstIterator it(_styles);
  for (it.toStart(); it.isValid(); it.toNext())
  {
    if (it.value().name() == name)
    {
      return it.value().value();
    }
  }
  return String32();
}

err_t SvgStyleAttribute::setStyle(const String32& name, const String32& value)
{
  Vector<SvgStyleItem>::ConstIterator it(_styles);
  for (it.toStart(); it.isValid(); it.toNext())
  {
    if (it.value().name() == name) 
    {
      return _styles.mAt(it.index()).setValue(value);
    }
  }

  SvgStyleItem item;
  item.setName(name);
  item.setValue(value);
  _styles.append(item);
  return Error::Ok;
}

// ============================================================================
// [Fog::SvgEmbeddedAttribute]
// ============================================================================

SvgEmbeddedAttribute::SvgEmbeddedAttribute(XmlElement* element, const String32& name, uint32_t id) : 
  XmlAttribute(element, name),
  _id(id)
{
  SvgEmbeddedAttribute** attrs = reinterpret_cast<SvgElement*>(_element)->getEmbeddedAttributesPtr();

  attrs[_id] = this;
}

SvgEmbeddedAttribute::~SvgEmbeddedAttribute()
{
  SvgEmbeddedAttribute** attrs = reinterpret_cast<SvgElement*>(_element)->getEmbeddedAttributesPtr();

  // attrs is NULL when SvgElement is being destroyed.
  if (attrs) attrs[_id] = NULL;
}

// ============================================================================
// [Fog::SvgCoordAttribute]
// ============================================================================

SvgCoordAttribute::SvgCoordAttribute(XmlElement* element, const String32& name, uint32_t id) :
  SvgEmbeddedAttribute(element, name, id)
{
  _coord.value = 0.0;
  _coord.unit = SvgUnitNone;
}

SvgCoordAttribute::~SvgCoordAttribute()
{
}

err_t SvgCoordAttribute::setValue(const String32& value)
{
  err_t err = _value.set(value);
  if (err) return err;

  _coord = SvgUtil::parseCoord(value);
  return Error::Ok;
}

// ============================================================================
// [Fog::SvgPathAttribute]
// ============================================================================

SvgPathAttribute::SvgPathAttribute(XmlElement* element, const String32& name, uint32_t id) :
  SvgEmbeddedAttribute(element, name, id)
{
}

SvgPathAttribute::~SvgPathAttribute()
{
}

err_t SvgPathAttribute::setValue(const String32& value)
{
  err_t err = _value.set(value);
  if (err) return err;

  _path = SvgUtil::parsePath(value);
  return Error::Ok;
}

// ============================================================================
// [Fog::SvgPointsAttribute]
// ============================================================================

SvgPointsAttribute::SvgPointsAttribute(XmlElement* element, const String32& name, uint32_t id) :
  SvgEmbeddedAttribute(element, name, id)
{
}

SvgPointsAttribute::~SvgPointsAttribute()
{
}

err_t SvgPointsAttribute::setValue(const String32& value)
{
  err_t err = _value.set(value);
  if (err) return err;

  _path = SvgUtil::parsePoints(value);
  return Error::Ok;
}

// ============================================================================
// [Fog::SvgElement]
// ============================================================================

static const SvgAttributeDef svgCircleAttributes[] =
{
  { "cx"     , SvgAttributeDef::DefCoord , 0 },
  { "cy"     , SvgAttributeDef::DefCoord , 1 },
  { "r"      , SvgAttributeDef::DefCoord , 2 },
  { ""       , SvgAttributeDef::DefEnd   , 0xFF }
};

static const SvgAttributeDef svgEllipseAttributes[] =
{
  { "cx"     , SvgAttributeDef::DefCoord , 0 },
  { "cy"     , SvgAttributeDef::DefCoord , 1 },
  { "rx"     , SvgAttributeDef::DefCoord , 2 },
  { "ry"     , SvgAttributeDef::DefCoord , 3 },
  { ""       , SvgAttributeDef::DefEnd   , 0xFF }
};

static const SvgAttributeDef svgLineAttributes[] =
{
  { "x1"     , SvgAttributeDef::DefCoord , 0 },
  { "y1"     , SvgAttributeDef::DefCoord , 1 },
  { "x2"     , SvgAttributeDef::DefCoord , 2 },
  { "y2"     , SvgAttributeDef::DefCoord , 3 },
  { ""       , SvgAttributeDef::DefEnd   , 0xFF }
};

static const SvgAttributeDef svgGAttributes[] =
{
  { ""       , SvgAttributeDef::DefEnd   , 0xFF }
};

static const SvgAttributeDef svgPolygonAttributes[] =
{
  { "points" , SvgAttributeDef::DefPoints, 0 },
  { ""       , SvgAttributeDef::DefEnd   , 0xFF }
};

static const SvgAttributeDef svgPolyLineAttributes[] =
{
  { "points" , SvgAttributeDef::DefPoints, 0 },
  { ""       , SvgAttributeDef::DefEnd   , 0xFF }
};

static const SvgAttributeDef svgRectAttributes[] =
{
  { "x"      , SvgAttributeDef::DefCoord , 0 },
  { "y"      , SvgAttributeDef::DefCoord , 1 },
  { "width"  , SvgAttributeDef::DefCoord , 2 },
  { "height" , SvgAttributeDef::DefCoord , 3 },
  { "rx"     , SvgAttributeDef::DefCoord , 4 },
  { "ry"     , SvgAttributeDef::DefCoord , 5 },
  { ""       , SvgAttributeDef::DefEnd   , 0xFF }
};

static const SvgAttributeDef svgPathAttributes[] =
{
  { "d"      , SvgAttributeDef::DefPath  , 0 },
  { ""       , SvgAttributeDef::DefEnd   , 0xFF }
};

static String32 svgTypeToTagName(uint32_t svgType)
{
  switch (svgType)
  {
    case SvgElement::SvgTypeNone    : return fog_strings->get(STR_SVG_none); 
    case SvgElement::SvgTypeCircle  : return fog_strings->get(STR_SVG_circle); 
    case SvgElement::SvgTypeEllipse : return fog_strings->get(STR_SVG_ellipse); 
    case SvgElement::SvgTypeG       : return fog_strings->get(STR_SVG_g); 
    case SvgElement::SvgTypeLine    : return fog_strings->get(STR_SVG_line); 
    case SvgElement::SvgTypePolygon : return fog_strings->get(STR_SVG_polygon); 
    case SvgElement::SvgTypePolyLine: return fog_strings->get(STR_SVG_polyline); 
    case SvgElement::SvgTypeRect    : return fog_strings->get(STR_SVG_rect); 
    case SvgElement::SvgTypePath    : return fog_strings->get(STR_SVG_path); 

    default:
      FOG_ASSERT_NOT_REACHED();
      return String32();
  }
}

SvgElement::SvgElement(uint32_t svgType) :
  XmlElement(svgTypeToTagName(svgType)),
  _svgType(svgType),
  _styles(NULL)
{
  _type |= TypeSvgMask;
  _flags &= ~(AllowedTag);
}

SvgElement::~SvgElement()
{
  // Remove all attributes here, because SvgStyleAttribute casts XmlElement to
  // SvgElement and if we destroy it in XmlElement it's too late (SvgElement
  // data are no longer valid).
  removeAllAttributes();
}

SvgElement* SvgElement::clone() const
{
  SvgElement* e = reinterpret_cast<SvgElement*>(
    SvgDocument::createElementStatic(tagName()));
  if (e) copyAttributes(e, const_cast<SvgElement*>(this));
  return e;
}

XmlAttribute* SvgElement::_createAttribute(const Fog::String32& name) const
{
  if (name == fog_strings->get(STR_XML_style))
  {
    return new(std::nothrow) SvgStyleAttribute(const_cast<SvgElement*>(this), name);
  }

  const SvgAttributeDef* defs = getEmbeddedAttributesDef();
  sysuint_t i = 0;

  for (sysuint_t i = 0; defs[i].def != SvgAttributeDef::DefEnd; i++)
  {
    if (name == Ascii8(defs[i].name))
    {
      switch (defs[i].def)
      {
        case SvgAttributeDef::DefCoord:
          return new(std::nothrow) SvgCoordAttribute(const_cast<SvgElement*>(this), name, defs[i].id);
        case SvgAttributeDef::DefPath:
          return new(std::nothrow) SvgPathAttribute(const_cast<SvgElement*>(this), name, defs[i].id);
        case SvgAttributeDef::DefPoints:
          return new(std::nothrow) SvgPointsAttribute(const_cast<SvgElement*>(this), name, defs[i].id);
      }
    }
  }

  return base::_createAttribute(name);
}

const SvgAttributeDef* SvgElement::getEmbeddedAttributesDef() const
{
  switch (_svgType)
  {
    case SvgElement::SvgTypeNone    : return NULL;
    case SvgElement::SvgTypeCircle  : return svgCircleAttributes; 
    case SvgElement::SvgTypeEllipse : return svgEllipseAttributes;
    case SvgElement::SvgTypeG       : return svgGAttributes;
    case SvgElement::SvgTypeLine    : return svgLineAttributes;
    case SvgElement::SvgTypePolygon : return svgPolygonAttributes;
    case SvgElement::SvgTypePolyLine: return svgPolyLineAttributes;
    case SvgElement::SvgTypeRect    : return svgRectAttributes;
    case SvgElement::SvgTypePath    : return svgPathAttributes;

    default:
      FOG_ASSERT_NOT_REACHED();
      return NULL;
  }
}

SvgEmbeddedAttribute** SvgElement::getEmbeddedAttributesPtr() const
{
  return (SvgEmbeddedAttribute**)NULL;
}

String32 SvgElement::getStyle(const String32& name) const
{
  if (_styles)
    return _styles->getStyle(name);
  else
    return String32();
}

err_t SvgElement::setStyle(const String32& name, const String32& value)
{
  if (_styles == NULL)
  {
    err_t err = setAttribute(fog_strings->get(STR_XML_style), String32());
    if (err) return err;

    FOG_ASSERT(_styles != NULL);
  }

  return _styles->setStyle(name, value);
}

err_t SvgElement::render(SvgContext* context) const
{
  err_t err = Error::Ok;

  // Before render: Save painter state.
  context->painter()->save();

  SvgContext::Style savedStrokeStyle(context->strokeStyle());
  SvgContext::Style savedFillStyle(context->fillStyle());

  // Before render: Set-up styles defined in this element.
  if (_styles && !_styles->styles().isEmpty()) 
  {
    Vector<SvgStyleItem>::ConstIterator it(_styles->styles());
    for (it.toStart(); it.isValid(); it.toNext())
    {
      const SvgStyleItem& item = it.value();

      switch (item.styleType())
      {
        case SvgStyleItem::StyleFill:
        {
          Rgba color(item.valueAsUInt32());
          context->setFillColor(color);
          break;
        }
        case SvgStyleItem::StyleStroke:
        {
          Rgba color(item.valueAsUInt32());
          context->setStrokeColor(color);
          break;
        }
        case SvgStyleItem::StyleStrokeWidth:
        {
          SvgCoord coord = item.valueAsCoord();
          context->painter()->setLineWidth(coord.value);
          break;
        }
      }
    }
  }

  // Actual render: object or children (depends to Svg type)
  if (_svgType == SvgTypeG)
  {
    if (hasChildNodes()) err = _walkAndRender(this, context);
  }
  else
  {
    err = renderShape(context);
  }

  // After render: Restore painter state.
  context->setStrokeStyle(savedStrokeStyle);
  context->setFillStyle(savedFillStyle);

  context->painter()->restore();

  return err;
}

err_t SvgElement::renderShape(SvgContext* context) const
{
  return Error::Ok;
}

err_t SvgElement::_walkAndRender(const XmlElement* root, SvgContext* context)
{
  err_t err = Error::Ok;
  XmlElement* e;

  for (e = root->firstChild(); e; e = e->nextSibling())
  {
    if (e->isSvgElement())
    {
      err = reinterpret_cast<SvgElement*>(e)->render(context);
      if (err) break;
    }
    else
    {
      _walkAndRender(e, context);
    }
  }

  return err;
}
// ============================================================================
// [Fog::SvgShapeElement]
// ============================================================================

SvgShapeElement::SvgShapeElement(uint32_t svgType) :
  SvgElement(svgType)
{
  _svgType = svgType;
  Memory::zero(_data, sizeof(XmlAttribute*) * FOG_ARRAY_SIZE(_data));
}

SvgShapeElement::~SvgShapeElement()
{
}

SvgEmbeddedAttribute** SvgShapeElement::getEmbeddedAttributesPtr() const
{
  return (SvgEmbeddedAttribute**)_data;
}

err_t SvgShapeElement::renderShape(SvgContext* context) const
{
  switch (_svgType)
  {
    case SvgTypeNone:
    {
      break;
    }

    case SvgTypeCircle:
    {
      if (_data[0] && _data[1] && _data[2])
      {
        double cx = reinterpret_cast<const SvgCoordAttribute*>(_data[0])->coord().value;
        double cy = reinterpret_cast<const SvgCoordAttribute*>(_data[1])->coord().value;
        double r  = reinterpret_cast<const SvgCoordAttribute*>(_data[2])->coord().value;

        context->drawEllipse(PointF(cx, cy), PointF(r, r));
      }
      break;
    }

    case SvgTypeEllipse:
    {
      if (_data[0] && _data[1] && _data[2] && _data[3])
      {
        double cx = reinterpret_cast<const SvgCoordAttribute*>(_data[0])->coord().value;
        double cy = reinterpret_cast<const SvgCoordAttribute*>(_data[1])->coord().value;
        double rx = reinterpret_cast<const SvgCoordAttribute*>(_data[2])->coord().value;
        double ry = reinterpret_cast<const SvgCoordAttribute*>(_data[3])->coord().value;

        context->drawEllipse(PointF(cx, cy), PointF(rx, ry));
      }
      break;
    }

    case SvgTypeLine:
    {
      if (_data[0] && _data[1] && _data[2] && _data[3])
      {
        double x1 = reinterpret_cast<const SvgCoordAttribute*>(_data[0])->coord().value;
        double y1 = reinterpret_cast<const SvgCoordAttribute*>(_data[1])->coord().value;
        double x2 = reinterpret_cast<const SvgCoordAttribute*>(_data[2])->coord().value;
        double y2 = reinterpret_cast<const SvgCoordAttribute*>(_data[3])->coord().value;

        context->drawLine(PointF(x1, y1), PointF(x2, y2));
      }
      break;
    }

    case SvgTypePath:
    {
      if (_data[0])
      {
        const Path& path = reinterpret_cast<const SvgPathAttribute*>(_data[0])->path();
        context->drawPath(path);
      }
      break;
    }

    case SvgTypePolygon:
    {
      if (_data[0])
      {
        const Path& path = reinterpret_cast<const SvgPointsAttribute*>(_data[0])->path();
        context->drawPath(path);
      }
      break;
    }

    case SvgTypePolyLine:
    {
      if (_data[0])
      {
        const Path& path = reinterpret_cast<const SvgPointsAttribute*>(_data[0])->path();
        context->drawPath(path);
      }
      break;
    }

    case SvgTypeRect:
    {
      if (_data[2] && _data[3])
      {
        double w  = reinterpret_cast<const SvgCoordAttribute*>(_data[2])->coord().value;
        double h  = reinterpret_cast<const SvgCoordAttribute*>(_data[3])->coord().value;

        double x  = _data[0] ? reinterpret_cast<const SvgCoordAttribute*>(_data[0])->coord().value : 0.0;
        double y  = _data[1] ? reinterpret_cast<const SvgCoordAttribute*>(_data[1])->coord().value : 0.0;
        double rx = _data[4] ? reinterpret_cast<const SvgCoordAttribute*>(_data[4])->coord().value : 0.0;
        double ry = _data[5] ? reinterpret_cast<const SvgCoordAttribute*>(_data[5])->coord().value : 0.0;

        if (rx == 0.0 && ry == 0.0)
          context->drawRect(RectF(x, y, w, h));
        else
          context->drawRound(RectF(x, y, w, h), PointF(rx, ry));
      }
      break;
    }
  }

  return Error::Ok;
}

// ============================================================================
// [Fog::SvgDocument]
// ============================================================================

SvgDocument::SvgDocument()
{
  _type |= TypeSvgMask;
}

SvgDocument::~SvgDocument()
{
}

XmlElement* SvgDocument::clone() const
{
  SvgDocument* doc = new(std::nothrow) SvgDocument();
  if (!doc) return NULL;

  for (XmlElement* ch = firstChild(); ch; ch = ch->nextSibling())
  {
    XmlElement* e = ch->clone();
    if (e && doc->appendChild(e) != Error::Ok) delete e;
  }

  return doc;
}

XmlElement* SvgDocument::createElement(const String32& tagName)
{
  return createElementStatic(tagName);
}

XmlElement* SvgDocument::createElementStatic(const String32& tagName)
{
  if (tagName == Ascii8("circle"  )) return new(std::nothrow) SvgShapeElement(SvgElement::SvgTypeCircle);
  if (tagName == Ascii8("ellipse" )) return new(std::nothrow) SvgShapeElement(SvgElement::SvgTypeEllipse);
  if (tagName == Ascii8("g"       )) return new(std::nothrow) SvgShapeElement(SvgElement::SvgTypeG);
  if (tagName == Ascii8("line"    )) return new(std::nothrow) SvgShapeElement(SvgElement::SvgTypeLine);
  if (tagName == Ascii8("polygon" )) return new(std::nothrow) SvgShapeElement(SvgElement::SvgTypePolygon);
  if (tagName == Ascii8("polyline")) return new(std::nothrow) SvgShapeElement(SvgElement::SvgTypePolyLine);
  if (tagName == Ascii8("rect"    )) return new(std::nothrow) SvgShapeElement(SvgElement::SvgTypeRect);
  if (tagName == Ascii8("path"    )) return new(std::nothrow) SvgShapeElement(SvgElement::SvgTypePath);

  // If element is not SVG, use base class to determine correct type.
  return XmlDocument::createElementStatic(tagName);
}

err_t SvgDocument::render(SvgContext* context) const
{
  return SvgElement::_walkAndRender(this, context);
}

} // Fog namespace
