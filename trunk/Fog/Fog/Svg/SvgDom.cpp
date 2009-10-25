// [Fog/Svg Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Dependencies]
#include <Fog/Core/Char.h>
#include <Fog/Core/String.h>
#include <Fog/Core/Strings.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/Painter.h>
#include <Fog/Svg/SvgContext.h>
#include <Fog/Svg/SvgDom.h>
#include <Fog/Svg/SvgUtil.h>

namespace Fog {

// ============================================================================
// [Fog::SvgStyleAttribute]
// ============================================================================

SvgStyleAttribute::SvgStyleAttribute(XmlElement* element, const ManagedString& name) : 
  XmlAttribute(element, name)
{
  reinterpret_cast<SvgElement*>(element)->_styles = this;
}

SvgStyleAttribute::~SvgStyleAttribute()
{
  reinterpret_cast<SvgElement*>(_element)->_styles = NULL;
}

String SvgStyleAttribute::getValue() const
{
  return SvgUtil::joinStyles(_styles);
}

err_t SvgStyleAttribute::setValue(const String& value)
{
  _styles = SvgUtil::parseStyles(value);
  return Error::Ok;
}

String SvgStyleAttribute::getStyle(const String& name) const
{
  Vector<SvgStyleItem>::ConstIterator it(_styles);
  for (it.toStart(); it.isValid(); it.toNext())
  {
    if (it.value().getName() == name)
    {
      return it.value().getValue();
    }
  }
  return String();
}

err_t SvgStyleAttribute::setStyle(const String& name, const String& value)
{
  Vector<SvgStyleItem>::ConstIterator it(_styles);
  for (it.toStart(); it.isValid(); it.toNext())
  {
    if (it.value().getName() == name)
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
// [Fog::SvgTransformAttribute]
// ============================================================================

struct FOG_HIDDEN SvgTransformAttribute : public XmlAttribute
{
  // [Construction / Destruction]

  typedef XmlAttribute base;

  SvgTransformAttribute(XmlElement* element, const ManagedString& name, int offset = -1);
  virtual ~SvgTransformAttribute();

  // [Methods]

  virtual err_t setValue(const String& value);

  // [Data]

  FOG_INLINE bool isValid() const { return _isValid; }
  FOG_INLINE const Matrix& getMatrix() const { return _matrix; }

  // [Members]
protected:
  bool _isValid;
  Matrix _matrix;

private:
  FOG_DISABLE_COPY(SvgTransformAttribute)
};

SvgTransformAttribute::SvgTransformAttribute(XmlElement* element, const ManagedString& name, int offset) :
    XmlAttribute(element, name, offset)
{
  if (_name == fog_strings->getString(STR_SVG_transform))
  {
    reinterpret_cast<SvgElement*>(_element)->_transform = this;
  }
}

SvgTransformAttribute::~SvgTransformAttribute()
{
  if (_name == fog_strings->getString(STR_SVG_transform))
  {
    reinterpret_cast<SvgElement*>(_element)->_transform = NULL;
  }
}

err_t SvgTransformAttribute::setValue(const String& value)
{
  err_t err = _value.set(value);
  if (err) return err;

  if (SvgUtil::parseMatrix(value, &_matrix) == SvgStatusOk)
  {
    _isValid = true;
  }
  else
  {
    _isValid = false;
  }

  return Error::Ok;
}

// ============================================================================
// [Fog::SvgCoordAttribute]
// ============================================================================

struct FOG_HIDDEN SvgCoordAttribute : public XmlAttribute
{
  // [Construction / Destruction]

  typedef XmlAttribute base;

  SvgCoordAttribute(XmlElement* element, const ManagedString& name, int offset = -1);
  virtual ~SvgCoordAttribute();

  // [Methods]

  virtual err_t setValue(const String& value);

  // [Coords]

  FOG_INLINE const SvgCoord& getCoord() const { return _coord; }
  FOG_INLINE double getDouble() const { return _coord.value; }
  FOG_INLINE uint32_t getUnit() const { return _coord.unit; }

  // [Members]
protected:
  SvgCoord _coord;

private:
  FOG_DISABLE_COPY(SvgCoordAttribute)
};

SvgCoordAttribute::SvgCoordAttribute(XmlElement* element, const ManagedString& name, int offset) :
  XmlAttribute(element, name, offset)
{
  _coord.value = 0.0;
  _coord.unit = SvgUnitNotDefined;
}

SvgCoordAttribute::~SvgCoordAttribute()
{
}

err_t SvgCoordAttribute::setValue(const String& value)
{
  err_t err = _value.set(value);
  if (err) return err;

  _coord = SvgUtil::parseCoord(value);

  if (_element) reinterpret_cast<SvgElement*>(_element)->_boundingRectDirty = true;
  return Error::Ok;
}

// ============================================================================
// [Fog::SvgOffsetAttribute]
// ============================================================================

struct FOG_HIDDEN SvgOffsetAttribute : public XmlAttribute
{
  // [Construction / Destruction]

  typedef XmlAttribute base;

  SvgOffsetAttribute(XmlElement* element, const ManagedString& name, int offset = -1);
  virtual ~SvgOffsetAttribute();

  // [Methods]

  virtual err_t setValue(const String& value);

  // [Coords]

  FOG_INLINE double getOffset() const { return _offset; }

  // [Members]
protected:
  double _offset;

private:
  FOG_DISABLE_COPY(SvgOffsetAttribute)
};

SvgOffsetAttribute::SvgOffsetAttribute(XmlElement* element, const ManagedString& name, int offset) :
  XmlAttribute(element, name, offset)
{
  _offset = 0.0;
}

SvgOffsetAttribute::~SvgOffsetAttribute()
{
}

err_t SvgOffsetAttribute::setValue(const String& value)
{
  err_t err = _value.set(value);
  if (err) return err;

  sysuint_t end;
  if (value.atod(&_offset, NULL, &end) == Error::Ok)
  {
    if (end < value.getLength() && value.at(end) == Char('%')) _offset *= 0.01;

    if (_offset < 0.0) _offset = 0.0;
    if (_offset > 1.0) _offset = 1.0;
  }
  else
  {
    _offset = 0.0;
  }

  return Error::Ok;
}

// ============================================================================
// [Fog::SvgPathAttribute]
// ============================================================================

struct FOG_HIDDEN SvgPathAttribute : public XmlAttribute
{
  // [Construction / Destruction]

  typedef XmlAttribute base;

  SvgPathAttribute(XmlElement* element, const ManagedString& name, int offset = -1);
  virtual ~SvgPathAttribute();

  // [Methods]

  virtual err_t setValue(const String& value);

  // [Coords]

  FOG_INLINE const Path& getPath() const { return _path; }

  // [Members]
protected:
  Path _path;

private:
  FOG_DISABLE_COPY(SvgPathAttribute)
};

SvgPathAttribute::SvgPathAttribute(XmlElement* element, const ManagedString& name, int offset) :
  XmlAttribute(element, name, offset)
{
}

SvgPathAttribute::~SvgPathAttribute()
{
}

err_t SvgPathAttribute::setValue(const String& value)
{
  err_t err = _value.set(value);
  if (err) return err;

  _path = SvgUtil::parsePath(value);
  if (_element) reinterpret_cast<SvgElement*>(_element)->_boundingRectDirty = true;
  return Error::Ok;
}

// ============================================================================
// [Fog::SvgPointsAttribute]
// ============================================================================

struct FOG_HIDDEN SvgPointsAttribute : public XmlAttribute
{
  // [Construction / Destruction]

  typedef XmlAttribute base;

  SvgPointsAttribute(XmlElement* element, const ManagedString& name, bool closePath, int offset = -1);
  virtual ~SvgPointsAttribute();

  // [Methods]

  virtual err_t setValue(const String& value);

  // [Coords]

  FOG_INLINE const Path& getPath() const { return _path; }

  // [Members]
protected:
  Path _path;
  bool _closePath;

private:
  FOG_DISABLE_COPY(SvgPointsAttribute)
};

SvgPointsAttribute::SvgPointsAttribute(XmlElement* element, const ManagedString& name, bool closePath, int offset) :
  XmlAttribute(element, name, offset),
  _closePath(closePath)
{
}

SvgPointsAttribute::~SvgPointsAttribute()
{
}

err_t SvgPointsAttribute::setValue(const String& value)
{
  err_t err = _value.set(value);
  if (err) return err;

  _path = SvgUtil::parsePoints(value);
  if (!_path.isEmpty() && _closePath) _path.closePolygon();

  if (_element) reinterpret_cast<SvgElement*>(_element)->_boundingRectDirty = true;
  return Error::Ok;
}

// ============================================================================
// [Fog::SvgEnumAttribute]
// ============================================================================

struct SvgEnumList
{
  char name[20];
  int value;
};

static const SvgEnumList svgEnumList_gradientUnits[] =
{
  { "userSpaceOnUse", SvgUserSpaceOnUse },
  { "objectBoundingBox", SvgObjectBoundingBox },
  { "", -1 }
};

static const SvgEnumList svgEnumList_spreadMethod[] =
{
  { "pad", Pattern::SpreadPad },
  { "reflect", Pattern::SpreadReflect },
  { "repeat", Pattern::SpreadRepeat },
  { "", -1 }
};

struct FOG_HIDDEN SvgEnumAttribute : public XmlAttribute
{
  // [Construction / Destruction]

  typedef XmlAttribute base;

  SvgEnumAttribute(XmlElement* element, const ManagedString& name, const SvgEnumList* enumList, int offset = -1);
  virtual ~SvgEnumAttribute();

  // [Methods]

  virtual err_t setValue(const String& value);

  // [Coords]

  FOG_INLINE int getEnum() const { return _valueEnum; }

  // [Members]
protected:
  const SvgEnumList* _enumList;
  int _valueEnum;

private:
  FOG_DISABLE_COPY(SvgEnumAttribute)
};

SvgEnumAttribute::SvgEnumAttribute(XmlElement* element, const ManagedString& name, const SvgEnumList* enumList, int offset) :
    XmlAttribute(element, name, offset),
    _enumList(enumList),
    _valueEnum(-1)
{
}

SvgEnumAttribute::~SvgEnumAttribute()
{
}

err_t SvgEnumAttribute::setValue(const String& value)
{
  err_t err = _value.set(value);
  if (err) return err;

  const SvgEnumList* p = _enumList;
  for (;;)
  {
    if (p->name[0] == '\0') 
    {
      _valueEnum = p->value;
      break;
    }

    if (value == Ascii8(p->name))
    {
      _valueEnum = p->value;
      break;
    }

    p++;
  }

  return Error::Ok;
}

// ============================================================================
// [Fog::SvgElement]
// ============================================================================

SvgElement::SvgElement(const ManagedString& tagName, uint32_t svgType) :
  XmlElement(tagName),
  _svgType(svgType),
  _boundingRectDirty(true),
  _unused(0),
  _transform(NULL),
  _styles(NULL)
{
  _type |= TypeSvgMask;
  _flags &= ~(AllowedTag);
}

SvgElement::~SvgElement()
{
  // Remove all attributes here, because SvgStyleAttribute casts XmlElement
  // to SvgElement and if we destroy it in XmlElement it's too late.
  // (SvgElement data are no longer valid)
  removeAllAttributes();
}

SvgElement* SvgElement::clone() const
{
  SvgElement* e = reinterpret_cast<SvgElement*>(SvgDocument::createElementStatic(_tagName));
  if (e) copyAttributes(e, const_cast<SvgElement*>(this));
  return e;
}

XmlAttribute* SvgElement::_createAttribute(const ManagedString& name) const
{
  if (name == fog_strings->getString(STR_XML_style    )) return new(std::nothrow) SvgStyleAttribute    (const_cast<SvgElement*>(this), name);
  if (name == fog_strings->getString(STR_SVG_transform)) return new(std::nothrow) SvgTransformAttribute(const_cast<SvgElement*>(this), name);

  return base::_createAttribute(name);
}

String SvgElement::getStyle(const String& name) const
{
  if (_styles)
    return _styles->getStyle(name);
  else
    return String();
}

err_t SvgElement::setStyle(const String& name, const String& value)
{
  if (_styles == NULL)
  {
    err_t err = setAttribute(fog_strings->getString(STR_XML_style), String());
    if (err) return err;

    FOG_ASSERT(_styles != NULL);
  }

  return _styles->setStyle(name, value);
}

static Utf16 parseId(const String& url)
{
  const Char* idStr;
  const Char* idEnd;
  const Char* idMark;

  if (url.getLength() < 7) return Utf16((const Char*)NULL, 0);

  idStr = url.cData() + 5;
  idEnd = idStr + url.getLength() - 5;

  while (idStr->isSpace())
  {
    if (++idStr == idEnd) goto bail;
  }

  if (idStr == idEnd) goto bail;

  idMark = idStr;
  while (*idStr != Char(')'))
  {
    if (++idStr == idEnd) goto bail;
  }
  return Utf16(idMark, (sysuint_t)(idStr - idMark));

bail:
  return Utf16((const Char*)NULL, 0);
}

err_t SvgElement::onRender(SvgContext* context) const
{
  SvgContextBackup backup;
  err_t err = Error::Ok;

  // Before render: Apply transformations and setup styles defined in this element.
  if ((_transform != NULL && _transform->isValid()) || (_styles != NULL && !_styles->getStyles().isEmpty()))
  {
    backup.init(context);

    // Transformations.
    if (_transform)
    {
      backup._matrix.instance() = context->getPainter()->getMatrix();
      backup._matrixBackup = true;

      context->getPainter()->affine(_transform->getMatrix());
    }

    // Styles.
    if (_styles)
    {
      Vector<SvgStyleItem>::ConstIterator it(_styles->getStyles());
      for (it.toStart(); it.isValid(); it.toNext())
      {
        const SvgStyleItem& item = it.value();
        if (!item.isValid()) continue;

        switch (item.getStyleType())
        {
          case SvgStyleFill:
          {
            switch (item.getPatternType())
            {
              case SvgPatternNone:
                context->setFillNone();
                break;
              case SvgPatternColor:
                context->setFillColor(item.getColor());
                break;
              case SvgPatternUri:
              {
                const String& v = item.getValue();
                XmlElement* r = getDocument()->getElementById(parseId(v));
                if (r && r->isSvg())
                {
                  reinterpret_cast<SvgElement*>(r)->onApplyPattern(context, const_cast<SvgElement*>(this), SvgPaintFill);
                }
                break;
              }
            }
            break;
          }
          case SvgStyleFillRule:
          {
            context->setFillMode(item.getEnum());
            break;
          }
          case SvgStyleStroke:
          {
            switch (item.getPatternType())
            {
              case SvgPatternNone:
                context->setLineNone();
                break;
              case SvgPatternColor:
                context->setLineColor(item.getColor());
                break;
              case SvgPatternUri:
              {
                const String& v = item.getValue();
                XmlElement* r = getDocument()->getElementById(parseId(v));
                if (r && r->isSvg())
                {
                  reinterpret_cast<SvgElement*>(r)->onApplyPattern(context, const_cast<SvgElement*>(this), SvgPaintStroke);
                }
                break;
              }
            }
            break;
          }
          case SvgStyleStrokeLineCap:
          {
            context->setLineCap(item.getEnum());
            break;
          }
          case SvgStyleStrokeLineJoin:
          {
            context->setLineJoin(item.getEnum());
            break;
          }
          case SvgStyleStrokeMiterLimit:
          {
            SvgCoord coord = item.getCoord();
            context->setMiterLimit(coord.value);
            break;
          }
          case SvgStyleStrokeWidth:
          {
            SvgCoord coord = item.getCoord();
            context->setLineWidth(coord.value);
            break;
          }
        }
      }
    }
  }

  // Actual render: object or children (depends to Svg type).
  err = onRenderShape(context);

  // After render: SvgContextBackup destructor will restore SvgContext state
  // if modified.
  return err;
}

err_t SvgElement::onRenderShape(SvgContext* context) const
{
  return Error::Ok;
}

err_t SvgElement::onApplyPattern(SvgContext* context, SvgElement* obj, int paintType) const
{
  return Error::InvalidFunction;
}

err_t SvgElement::onCalcBoundingBox(RectD* box) const
{
  return Error::InvalidFunction;
}

err_t SvgElement::_walkAndRender(const XmlElement* root, SvgContext* context)
{
  err_t err = Error::Ok;
  XmlElement* e;

  for (e = root->firstChild(); e; e = e->nextSibling())
  {
    if (e->isSvgElement())
    {
      err = reinterpret_cast<SvgElement*>(e)->onRender(context);
      if (err) break;
    }
    else if (e->hasChildNodes())
    {
      _walkAndRender(e, context);
    }
  }

  return err;
}

const RectD& SvgElement::getBoundingRect() const
{
  if (_boundingRectDirty)
  {
    onCalcBoundingBox(&_boundingBox);
    _boundingRectDirty = false;
  }

  return _boundingBox;
}

// ============================================================================
// [Fog::SvgCircleElement]
// ============================================================================

struct FOG_HIDDEN SvgCircleElement : public SvgElement
{
  // [Construction / Destruction]

  typedef SvgElement base;

  SvgCircleElement();
  virtual ~SvgCircleElement();

  // [Attributes]

  virtual XmlAttribute* _createAttribute(const ManagedString& name) const;

  // [SVG Rendering]

  virtual err_t onRenderShape(SvgContext* context) const;
  virtual err_t onCalcBoundingBox(RectD* box) const;

  // [Embedded Attributes]

  SvgCoordAttribute a_cx;
  SvgCoordAttribute a_cy;
  SvgCoordAttribute a_r;

private:
  FOG_DISABLE_COPY(SvgCircleElement)
};

SvgCircleElement::SvgCircleElement() :
  SvgElement(fog_strings->getString(STR_SVG_circle), SvgTypeCircle),
  a_cx(NULL, fog_strings->getString(STR_SVG_cx), FOG_OFFSET_OF(SvgCircleElement, a_cx)),
  a_cy(NULL, fog_strings->getString(STR_SVG_cy), FOG_OFFSET_OF(SvgCircleElement, a_cy)),
  a_r (NULL, fog_strings->getString(STR_SVG_r ), FOG_OFFSET_OF(SvgCircleElement, a_r ))
{
}

SvgCircleElement::~SvgCircleElement()
{
  removeAllAttributes();
}

XmlAttribute* SvgCircleElement::_createAttribute(const ManagedString& name) const
{
  if (name == fog_strings->getString(STR_SVG_cx)) return const_cast<SvgCoordAttribute*>(&a_cx);
  if (name == fog_strings->getString(STR_SVG_cy)) return const_cast<SvgCoordAttribute*>(&a_cy);
  if (name == fog_strings->getString(STR_SVG_r )) return const_cast<SvgCoordAttribute*>(&a_r );

  return base::_createAttribute(name);
}

err_t SvgCircleElement::onRenderShape(SvgContext* context) const
{
  if (a_r.isAssigned())
  {
    double cx = a_cx.isAssigned() ? a_cx.getCoord().value : 0.0;
    double cy = a_cy.isAssigned() ? a_cy.getCoord().value : 0.0;
    double r = fabs(a_r.getCoord().value);

    context->drawEllipse(PointD(cx, cy), PointD(r, r));
    return Error::Ok;
  }
  else
  {
    return Error::Ok;
  }
}

err_t SvgCircleElement::onCalcBoundingBox(RectD* box) const
{
  if (a_r.isAssigned())
  {
    double cx = a_cx.isAssigned() ? a_cx.getCoord().value : 0.0;
    double cy = a_cy.isAssigned() ? a_cy.getCoord().value : 0.0;
    double r = fabs(a_r.getCoord().value);

    box->set(cx - r, cy - r, r * 2.0, r * 2.0);
    return Error::Ok;
  }
  else
  {
    box->clear();
    return Error::Ok;
  }
}

// ============================================================================
// [Fog::SvgDefsElement]
// ============================================================================

struct FOG_HIDDEN SvgDefsElement : public SvgElement
{
  // [Construction / Destruction]

  typedef SvgElement base;

  SvgDefsElement();
  virtual ~SvgDefsElement();

  // [SVG Rendering]

  virtual err_t onRender(SvgContext* context) const;

private:
  FOG_DISABLE_COPY(SvgDefsElement)
};

SvgDefsElement::SvgDefsElement() : 
  SvgElement(fog_strings->getString(STR_SVG_defs), SvgTypeDefs)
{
}

SvgDefsElement::~SvgDefsElement()
{
}

err_t SvgDefsElement::onRender(SvgContext* context) const
{
  // <defs> is used only to define shared resources or gradients.
  return Error::Ok;
}

// ============================================================================
// [Fog::SvgEllipseElement]
// ============================================================================

struct FOG_HIDDEN SvgEllipseElement : public SvgElement
{
  // [Construction / Destruction]

  typedef SvgElement base;

  SvgEllipseElement();
  virtual ~SvgEllipseElement();

  // [Attributes]

  virtual XmlAttribute* _createAttribute(const ManagedString& name) const;

  // [SVG Rendering]

  virtual err_t onRenderShape(SvgContext* context) const;
  virtual err_t onCalcBoundingBox(RectD* box) const;

  // [Embedded Attributes]

  SvgCoordAttribute a_cx;
  SvgCoordAttribute a_cy;
  SvgCoordAttribute a_rx;
  SvgCoordAttribute a_ry;

private:
  FOG_DISABLE_COPY(SvgEllipseElement)
};

SvgEllipseElement::SvgEllipseElement() :
  SvgElement(fog_strings->getString(STR_SVG_ellipse), SvgTypeEllipse),
  a_cx(NULL, fog_strings->getString(STR_SVG_cx), FOG_OFFSET_OF(SvgEllipseElement, a_cx)),
  a_cy(NULL, fog_strings->getString(STR_SVG_cy), FOG_OFFSET_OF(SvgEllipseElement, a_cy)),
  a_rx(NULL, fog_strings->getString(STR_SVG_rx), FOG_OFFSET_OF(SvgEllipseElement, a_rx)),
  a_ry(NULL, fog_strings->getString(STR_SVG_ry), FOG_OFFSET_OF(SvgEllipseElement, a_ry))
{
}

SvgEllipseElement::~SvgEllipseElement()
{
  removeAllAttributes();
}

XmlAttribute* SvgEllipseElement::_createAttribute(const ManagedString& name) const
{
  if (name == fog_strings->getString(STR_SVG_cx)) return const_cast<SvgCoordAttribute*>(&a_cx);
  if (name == fog_strings->getString(STR_SVG_cy)) return const_cast<SvgCoordAttribute*>(&a_cy);
  if (name == fog_strings->getString(STR_SVG_rx)) return const_cast<SvgCoordAttribute*>(&a_rx);
  if (name == fog_strings->getString(STR_SVG_ry)) return const_cast<SvgCoordAttribute*>(&a_ry);

  return base::_createAttribute(name);
}

err_t SvgEllipseElement::onRenderShape(SvgContext* context) const
{
  if (a_rx.isAssigned() && a_ry.isAssigned())
  {
    double cx = a_cx.isAssigned() ? a_cx.getCoord().value : 0.0;
    double cy = a_cy.isAssigned() ? a_cy.getCoord().value : 0.0;
    double rx = fabs(a_rx.getCoord().value);
    double ry = fabs(a_ry.getCoord().value);

    context->drawEllipse(PointD(cx, cy), PointD(rx, ry));
    return Error::Ok;
  }
  else
  {
    return Error::Ok;
  }
}

err_t SvgEllipseElement::onCalcBoundingBox(RectD* box) const
{
  if (a_rx.isAssigned() && a_ry.isAssigned())
  {
    double cx = a_cx.isAssigned() ? a_cx.getCoord().value : 0.0;
    double cy = a_cy.isAssigned() ? a_cy.getCoord().value : 0.0;

    double rx = a_rx.getDouble();
    double ry = a_ry.getDouble();

    box->set(cx - rx, cy - ry, rx * 2.0, ry * 2.0);
    return Error::Ok;
  }
  else
  {
    box->clear();
    return Error::Ok;
  }
  return Error::NotImplemented;
}

// ============================================================================
// [Fog::SvgGElement]
// ============================================================================

struct FOG_HIDDEN SvgGElement : public SvgElement
{
  // [Construction / Destruction]

  typedef SvgElement base;

  SvgGElement();
  virtual ~SvgGElement();

  // [SVG Rendering]

  virtual err_t onRenderShape(SvgContext* context) const;

private:
  FOG_DISABLE_COPY(SvgGElement)
};

SvgGElement::SvgGElement() :
  SvgElement(fog_strings->getString(STR_SVG_g), SvgTypeG)
{
}

SvgGElement::~SvgGElement()
{
}

err_t SvgGElement::onRenderShape(SvgContext* context) const
{
  if (hasChildNodes())
    return _walkAndRender(this, context);
  else
    return Error::Ok;
}

// ============================================================================
// [Fog::SvgLineElement]
// ============================================================================

struct FOG_HIDDEN SvgLineElement : public SvgElement
{
  // [Construction / Destruction]

  typedef SvgElement base;

  SvgLineElement();
  virtual ~SvgLineElement();

  // [Attributes]

  virtual XmlAttribute* _createAttribute(const ManagedString& name) const;

  // [SVG Rendering]

  virtual err_t onRenderShape(SvgContext* context) const;
  virtual err_t onCalcBoundingBox(RectD* box) const;

  // [Embedded Attributes]

  SvgCoordAttribute a_x1;
  SvgCoordAttribute a_y1;
  SvgCoordAttribute a_x2;
  SvgCoordAttribute a_y2;

private:
  FOG_DISABLE_COPY(SvgLineElement)
};

SvgLineElement::SvgLineElement() :
  SvgElement(fog_strings->getString(STR_SVG_line), SvgTypeLine),
  a_x1(NULL, fog_strings->getString(STR_SVG_x1), FOG_OFFSET_OF(SvgLineElement, a_x1)),
  a_y1(NULL, fog_strings->getString(STR_SVG_y1), FOG_OFFSET_OF(SvgLineElement, a_y1)),
  a_x2(NULL, fog_strings->getString(STR_SVG_x2), FOG_OFFSET_OF(SvgLineElement, a_x2)),
  a_y2(NULL, fog_strings->getString(STR_SVG_y2), FOG_OFFSET_OF(SvgLineElement, a_y2))
{
}

SvgLineElement::~SvgLineElement()
{
  removeAllAttributes();
}

XmlAttribute* SvgLineElement::_createAttribute(const ManagedString& name) const
{
  if (name == fog_strings->getString(STR_SVG_x1)) return const_cast<SvgCoordAttribute*>(&a_x1);
  if (name == fog_strings->getString(STR_SVG_y1)) return const_cast<SvgCoordAttribute*>(&a_y1);
  if (name == fog_strings->getString(STR_SVG_x2)) return const_cast<SvgCoordAttribute*>(&a_x2);
  if (name == fog_strings->getString(STR_SVG_y2)) return const_cast<SvgCoordAttribute*>(&a_y2);

  return base::_createAttribute(name);
}

err_t SvgLineElement::onRenderShape(SvgContext* context) const
{
  if (a_x1.isAssigned() && a_y1.isAssigned() && a_x2.isAssigned() && a_y2.isAssigned())
  {
    double x1 = a_x1.getCoord().value;
    double y1 = a_y1.getCoord().value;
    double x2 = a_x2.getCoord().value;
    double y2 = a_y2.getCoord().value;

    context->drawLine(PointD(x1, y1), PointD(x2, y2));
    return Error::Ok;
  }
  else
  {
    return Error::Ok;
  }
}

err_t SvgLineElement::onCalcBoundingBox(RectD* box) const
{
  if (a_x1.isAssigned() && a_y1.isAssigned() && a_x2.isAssigned() && a_y2.isAssigned())
  {
    double x1 = a_x1.getCoord().value;
    double y1 = a_y1.getCoord().value;
    double x2 = a_x2.getCoord().value;
    double y2 = a_y2.getCoord().value;

    double x = (x1 < x2) ? x1 : x2;
    double y = (y1 < y2) ? y1 : y2;
    double w = (x1 < x2) ? x2 - x1 : x1 - x2;
    double h = (y1 < y2) ? y2 - y1 : y1 - y2;

    box->set(x, y, w, h);
    return Error::Ok;
  }
  else
  {
    box->clear();
    return Error::Ok;
  }
}

// ============================================================================
// [Fog::SvgPathElement]
// ============================================================================

struct FOG_HIDDEN SvgPathElement : public SvgElement
{
  // [Construction / Destruction]

  typedef SvgElement base;

  SvgPathElement();
  virtual ~SvgPathElement();

  // [Attributes]

  virtual XmlAttribute* _createAttribute(const ManagedString& name) const;

  // [SVG Rendering]

  virtual err_t onRenderShape(SvgContext* context) const;
  virtual err_t onCalcBoundingBox(RectD* box) const;

  // [Embedded Attributes]

  SvgPathAttribute a_d;

private:
  FOG_DISABLE_COPY(SvgPathElement)
};

SvgPathElement::SvgPathElement() :
  SvgElement(fog_strings->getString(STR_SVG_path), SvgTypePath),
  a_d(NULL, fog_strings->getString(STR_SVG_d), FOG_OFFSET_OF(SvgPathElement, a_d))
{
}

SvgPathElement::~SvgPathElement()
{
  removeAllAttributes();
}

XmlAttribute* SvgPathElement::_createAttribute(const ManagedString& name) const
{
  if (name == fog_strings->getString(STR_SVG_d)) return const_cast<SvgPathAttribute*>(&a_d);

  return base::_createAttribute(name);
}

err_t SvgPathElement::onRenderShape(SvgContext* context) const
{
  if (a_d.isAssigned())
  {
    const Path& path = a_d.getPath();
    context->drawPath(path);
    return Error::Ok;
  }
  else
  {
    return Error::Ok;
  }
}

err_t SvgPathElement::onCalcBoundingBox(RectD* box) const
{
  if (a_d.isAssigned())
  {
    const Path& path = a_d.getPath();
    box->set(path.boundingRect());
    return Error::Ok;
  }
  else
  {
    box->clear();
    return Error::Ok;
  }
}

// ============================================================================
// [Fog::SvgPolygonElement]
// ============================================================================

struct FOG_HIDDEN SvgPolygonElement : public SvgElement
{
  // [Construction / Destruction]

  typedef SvgElement base;

  SvgPolygonElement();
  virtual ~SvgPolygonElement();

  // [Attributes]

  virtual XmlAttribute* _createAttribute(const ManagedString& name) const;

  // [SVG Rendering]

  virtual err_t onRenderShape(SvgContext* context) const;
  virtual err_t onCalcBoundingBox(RectD* box) const;

  // [Embedded Attributes]

  SvgPointsAttribute a_points;

private:
  FOG_DISABLE_COPY(SvgPolygonElement)
};

SvgPolygonElement::SvgPolygonElement() :
  SvgElement(fog_strings->getString(STR_SVG_polygon), SvgTypePolygon),
  a_points(NULL, fog_strings->getString(STR_SVG_points), true, FOG_OFFSET_OF(SvgPolygonElement, a_points))
{
}

SvgPolygonElement::~SvgPolygonElement()
{
  removeAllAttributes();
}

XmlAttribute* SvgPolygonElement::_createAttribute(const ManagedString& name) const
{
  if (name == fog_strings->getString(STR_SVG_points)) return const_cast<SvgPointsAttribute*>(&a_points);

  return base::_createAttribute(name);
}

err_t SvgPolygonElement::onRenderShape(SvgContext* context) const
{
  if (a_points.isAssigned())
  {
    const Path& path = a_points.getPath();
    context->drawPath(path);
    return Error::Ok;
  }
  else
  {
    return Error::Ok;
  }
}

err_t SvgPolygonElement::onCalcBoundingBox(RectD* box) const
{
  if (a_points.isAssigned())
  {
    const Path& path = a_points.getPath();
    box->set(path.boundingRect());
    return Error::Ok;
  }
  else
  {
    box->clear();
    return Error::Ok;
  }
}

// ============================================================================
// [Fog::SvgPolyLineElement]
// ============================================================================

struct FOG_HIDDEN SvgPolyLineElement : public SvgElement
{
  // [Construction / Destruction]

  typedef SvgElement base;

  SvgPolyLineElement();
  virtual ~SvgPolyLineElement();

  // [Attributes]

  virtual XmlAttribute* _createAttribute(const ManagedString& name) const;

  // [SVG Rendering]

  virtual err_t onRenderShape(SvgContext* context) const;
  virtual err_t onCalcBoundingBox(RectD* box) const;

  // [Embedded Attributes]

  SvgPointsAttribute a_points;

private:
  FOG_DISABLE_COPY(SvgPolyLineElement)
};

SvgPolyLineElement::SvgPolyLineElement() :
  SvgElement(fog_strings->getString(STR_SVG_polyline), SvgTypePolyLine),
  a_points(NULL, fog_strings->getString(STR_SVG_points), false, FOG_OFFSET_OF(SvgPolygonElement, a_points))
{
}

SvgPolyLineElement::~SvgPolyLineElement()
{
  removeAllAttributes();
}

XmlAttribute* SvgPolyLineElement::_createAttribute(const ManagedString& name) const
{
  if (name == fog_strings->getString(STR_SVG_points)) return const_cast<SvgPointsAttribute*>(&a_points);

  return base::_createAttribute(name);
}

err_t SvgPolyLineElement::onRenderShape(SvgContext* context) const
{
  if (a_points.isAssigned())
  {
    const Path& path = a_points.getPath();
    context->drawPath(path);
    return Error::Ok;
  }
  else
  {
    return Error::Ok;
  }
}

err_t SvgPolyLineElement::onCalcBoundingBox(RectD* box) const
{
  if (a_points.isAssigned())
  {
    const Path& path = a_points.getPath();
    box->set(path.boundingRect());
    return Error::Ok;
  }
  else
  {
    box->clear();
    return Error::Ok;
  }
}

// ============================================================================
// [Fog::SvgRectElement]
// ============================================================================

struct FOG_HIDDEN SvgRectElement : public SvgElement
{
  // [Construction / Destruction]

  typedef SvgElement base;

  SvgRectElement();
  virtual ~SvgRectElement();

  // [Attributes]

  virtual XmlAttribute* _createAttribute(const ManagedString& name) const;

  // [SVG Rendering]

  virtual err_t onRenderShape(SvgContext* context) const;
  virtual err_t onCalcBoundingBox(RectD* box) const;

  // [Embedded Attributes]

  SvgCoordAttribute a_x;
  SvgCoordAttribute a_y;
  SvgCoordAttribute a_width;
  SvgCoordAttribute a_height;
  SvgCoordAttribute a_rx;
  SvgCoordAttribute a_ry;

private:
  FOG_DISABLE_COPY(SvgRectElement)
};

SvgRectElement::SvgRectElement() :
  SvgElement(fog_strings->getString(STR_SVG_rect), SvgTypeRect),
  a_x     (NULL, fog_strings->getString(STR_SVG_x     ), FOG_OFFSET_OF(SvgRectElement, a_x     )),
  a_y     (NULL, fog_strings->getString(STR_SVG_y     ), FOG_OFFSET_OF(SvgRectElement, a_y     )),
  a_width (NULL, fog_strings->getString(STR_SVG_width ), FOG_OFFSET_OF(SvgRectElement, a_width )),
  a_height(NULL, fog_strings->getString(STR_SVG_height), FOG_OFFSET_OF(SvgRectElement, a_height)),
  a_rx    (NULL, fog_strings->getString(STR_SVG_rx    ), FOG_OFFSET_OF(SvgRectElement, a_rx    )),
  a_ry    (NULL, fog_strings->getString(STR_SVG_ry    ), FOG_OFFSET_OF(SvgRectElement, a_ry    ))
{
}

SvgRectElement::~SvgRectElement()
{
  removeAllAttributes();
}

XmlAttribute* SvgRectElement::_createAttribute(const ManagedString& name) const
{
  if (name == fog_strings->getString(STR_SVG_x     )) return const_cast<SvgCoordAttribute*>(&a_x     );
  if (name == fog_strings->getString(STR_SVG_y     )) return const_cast<SvgCoordAttribute*>(&a_y     );
  if (name == fog_strings->getString(STR_SVG_width )) return const_cast<SvgCoordAttribute*>(&a_width );
  if (name == fog_strings->getString(STR_SVG_height)) return const_cast<SvgCoordAttribute*>(&a_height);
  if (name == fog_strings->getString(STR_SVG_rx    )) return const_cast<SvgCoordAttribute*>(&a_rx    );
  if (name == fog_strings->getString(STR_SVG_ry    )) return const_cast<SvgCoordAttribute*>(&a_ry    );

  return base::_createAttribute(name);
}

err_t SvgRectElement::onRenderShape(SvgContext* context) const
{
  if (a_width.isAssigned() && a_height.isAssigned())
  {
    double x = a_x.isAssigned() ? a_x.getCoord().value : 0.0;
    double y = a_y.isAssigned() ? a_y.getCoord().value : 0.0;

    double w = a_width.getCoord().value;
    double h = a_height.getCoord().value;

    double rx = a_rx.isAssigned() ? a_rx.getCoord().value : 0.0;
    double ry = a_ry.isAssigned() ? a_ry.getCoord().value : 0.0;

    if (rx == 0.0 && ry == 0.0)
      context->drawRect(RectD(x, y, w, h));
    else
      context->drawRound(RectD(x, y, w, h), PointD(rx, ry));
  }
  return Error::Ok;
}

err_t SvgRectElement::onCalcBoundingBox(RectD* box) const
{
  if (a_width.isAssigned() && a_height.isAssigned())
  {
    double x = a_x.isAssigned() ? a_x.getCoord().value : 0.0;
    double y = a_y.isAssigned() ? a_y.getCoord().value : 0.0;

    double w = a_width.getCoord().value;
    double h = a_height.getCoord().value;

    box->set(x, y, w, h);
    return Error::Ok;
  }
  else
  {
    box->clear();
    return Error::Ok;
  }
}

// ============================================================================
// [Fog::SvgSolidColorElement]
// ============================================================================

struct FOG_HIDDEN SvgSolidColorElement : public SvgElement
{
  // [Construction / Destruction]

  typedef SvgElement base;

  SvgSolidColorElement();
  virtual ~SvgSolidColorElement();

  // [Attributes]

  virtual XmlAttribute* _createAttribute(const ManagedString& name) const;

  // [SVG Rendering]

  virtual err_t onRender(SvgContext* context) const;

  // [Embedded Attributes]

private:
  FOG_DISABLE_COPY(SvgSolidColorElement)
};

SvgSolidColorElement::SvgSolidColorElement() :
  SvgElement(fog_strings->getString(STR_SVG_solidColor), SvgTypeSolidColor)
{
}

SvgSolidColorElement::~SvgSolidColorElement()
{
  removeAllAttributes();
}

XmlAttribute* SvgSolidColorElement::_createAttribute(const ManagedString& name) const
{
  return base::_createAttribute(name);
}

err_t SvgSolidColorElement::onRender(SvgContext* context) const
{
  return Error::Ok;
}

// ============================================================================
// [Fog::SvgStopElement]
// ============================================================================

struct FOG_HIDDEN SvgStopElement : public SvgElement
{
  // [Construction / Destruction]

  typedef SvgElement base;

  SvgStopElement();
  virtual ~SvgStopElement();

  // [Attributes]

  virtual XmlAttribute* _createAttribute(const ManagedString& name) const;

  // [SVG Rendering]

  virtual err_t onRender(SvgContext* context) const;

  // [Embedded Attributes]

  SvgOffsetAttribute a_offset;

private:
  FOG_DISABLE_COPY(SvgStopElement)
};

SvgStopElement::SvgStopElement() :
  SvgElement(fog_strings->getString(STR_SVG_stop), SvgTypeStop),
  a_offset(NULL, fog_strings->getString(STR_SVG_offset), FOG_OFFSET_OF(SvgStopElement, a_offset))
{
}

SvgStopElement::~SvgStopElement()
{
  removeAllAttributes();
}

XmlAttribute* SvgStopElement::_createAttribute(const ManagedString& name) const
{
  if (name == fog_strings->getString(STR_SVG_offset)) return const_cast<SvgOffsetAttribute*>(&a_offset);

  return base::_createAttribute(name);
}

err_t SvgStopElement::onRender(SvgContext* context) const
{
  return Error::Ok;
}

// ============================================================================
// [Fog::SvgLinearGradientElement]
// ============================================================================

static void walkAndAddGradientStops(XmlElement* root, Pattern& pattern)
{
  bool stopsParsed = false;
  XmlElement* stop;
  int depth = 0;

start:
  for (stop = root->firstChild(); stop; stop = stop->nextSibling())
  {
    if (stop->isSvg() && reinterpret_cast<SvgElement*>(stop)->getSvgType() == SvgElement::SvgTypeStop)
    {
      SvgStopElement* _stop = reinterpret_cast<SvgStopElement*>(stop);

      double offset = _stop->a_offset.getOffset();
      Rgba color;

      String stopColorValue = _stop->getStyle(fog_strings->getString(STR_SVG_stop_color));
      String stopOpacityValue = _stop->getStyle(fog_strings->getString(STR_SVG_stop_opacity));

      if (SvgUtil::parseColor(stopColorValue, &color) == SvgStatusOk)
      {
        double opacity;
        if (!stopOpacityValue.isEmpty() && stopOpacityValue.atod(&opacity) == Error::Ok)
        {
          if (opacity < 0.0) opacity = 0.0;
          if (opacity > 1.0) opacity = 1.0;

          color.a = (uint8_t)(int)(opacity * 255.0);
        }
        pattern.addGradientStop(GradientStop(offset, color));
        stopsParsed = true;
      }
    }
  }

  if (!stopsParsed)
  {
    XmlElement* e;
    String link = root->getAttribute(fog_strings->getString(STR_SVG_xlink_href));

    if ((!link.isEmpty() && link.at(0) == Char('#')) && 
        (e = root->getDocument()->getElementById(Utf16(link.cData() + 1, link.getLength() - 1))))
    {
      root = e;
      if (++depth == 32) return;
      goto start;
    }
  }
}

struct FOG_HIDDEN SvgLinearGradientElement : public SvgElement
{
  // [Construction / Destruction]

  typedef SvgElement base;

  SvgLinearGradientElement();
  virtual ~SvgLinearGradientElement();

  // [Attributes]

  virtual XmlAttribute* _createAttribute(const ManagedString& name) const;

  // [SVG Rendering]

  virtual err_t onRender(SvgContext* context) const;
  virtual err_t onApplyPattern(SvgContext* context, SvgElement* obj, int paintType) const;

  // [Embedded Attributes]

  SvgEnumAttribute a_spreadMethod;
  SvgEnumAttribute a_gradientUnits;
  SvgTransformAttribute a_gradientTransform;
  SvgCoordAttribute a_x1;
  SvgCoordAttribute a_y1;
  SvgCoordAttribute a_x2;
  SvgCoordAttribute a_y2;

private:
  FOG_DISABLE_COPY(SvgLinearGradientElement)
};

SvgLinearGradientElement::SvgLinearGradientElement() :
  SvgElement(fog_strings->getString(STR_SVG_linearGradient), SvgTypeLinearGradient),
  a_spreadMethod(NULL, fog_strings->getString(STR_SVG_spreadMethod), svgEnumList_spreadMethod, FOG_OFFSET_OF(SvgLinearGradientElement, a_spreadMethod)),
  a_gradientUnits(NULL, fog_strings->getString(STR_SVG_gradientUnits), svgEnumList_gradientUnits, FOG_OFFSET_OF(SvgLinearGradientElement, a_gradientUnits)),
  a_gradientTransform(NULL, fog_strings->getString(STR_SVG_gradientTransform), FOG_OFFSET_OF(SvgLinearGradientElement, a_gradientTransform)),
  a_x1(NULL, fog_strings->getString(STR_SVG_x1), FOG_OFFSET_OF(SvgLinearGradientElement, a_x1)),
  a_y1(NULL, fog_strings->getString(STR_SVG_y1), FOG_OFFSET_OF(SvgLinearGradientElement, a_y1)),
  a_x2(NULL, fog_strings->getString(STR_SVG_x2), FOG_OFFSET_OF(SvgLinearGradientElement, a_x2)),
  a_y2(NULL, fog_strings->getString(STR_SVG_y2), FOG_OFFSET_OF(SvgLinearGradientElement, a_y2))
{
}

SvgLinearGradientElement::~SvgLinearGradientElement()
{
  removeAllAttributes();
}

XmlAttribute* SvgLinearGradientElement::_createAttribute(const ManagedString& name) const
{
  if (name == fog_strings->getString(STR_SVG_spreadMethod)) return const_cast<SvgEnumAttribute*>(&a_spreadMethod);
  if (name == fog_strings->getString(STR_SVG_gradientUnits)) return const_cast<SvgEnumAttribute*>(&a_gradientUnits);
  if (name == fog_strings->getString(STR_SVG_gradientTransform)) return const_cast<SvgTransformAttribute*>(&a_gradientTransform);
  if (name == fog_strings->getString(STR_SVG_x1)) return const_cast<SvgCoordAttribute*>(&a_x1);
  if (name == fog_strings->getString(STR_SVG_y1)) return const_cast<SvgCoordAttribute*>(&a_y1);
  if (name == fog_strings->getString(STR_SVG_x2)) return const_cast<SvgCoordAttribute*>(&a_x2);
  if (name == fog_strings->getString(STR_SVG_y2)) return const_cast<SvgCoordAttribute*>(&a_y2);

  return base::_createAttribute(name);
}

err_t SvgLinearGradientElement::onRender(SvgContext* context) const
{
  return Error::Ok;
}

err_t SvgLinearGradientElement::onApplyPattern(SvgContext* context, SvgElement* obj, int paintType) const
{
  // SVG TODO: Linear Gradient.

  Pattern pattern;

  // Set gradient type to LinearGradient.
  pattern.setType(Pattern::TypeLinearGradient);

  // Set start and end points - [x1, y1] and [x2, y2].
  if (a_x1.isAssigned() && a_y1.isAssigned() && a_x2.isAssigned() && a_y2.isAssigned())
  {
    double x1 = context->translateCoord(a_x1.getDouble(), a_x1.getUnit());
    double y1 = context->translateCoord(a_y1.getDouble(), a_y1.getUnit());
    double x2 = context->translateCoord(a_x2.getDouble(), a_x2.getUnit());
    double y2 = context->translateCoord(a_y2.getDouble(), a_y2.getUnit());

    if (!a_gradientUnits.isAssigned() || a_gradientUnits.getEnum() == SvgObjectBoundingBox)
    {
      RectD bbox = obj->getBoundingRect();
      x1 = bbox.getX() + bbox.getWidth () * x1;
      y1 = bbox.getY() + bbox.getHeight() * y1;
      x2 = bbox.getX() + bbox.getWidth () * x2;
      y2 = bbox.getY() + bbox.getHeight() * y2;
    }

    pattern.setPoints(PointD(x1, y1), PointD(x2, y2));
  }
  else
  {
    RectD bbox = obj->getBoundingRect();
    pattern.setPoints(PointD(bbox.getX1(), bbox.getY1()), PointD(bbox.getX2(), bbox.getY2()));
  }

  // Add color stops.
  walkAndAddGradientStops(const_cast<SvgLinearGradientElement*>(this), pattern);

  if (paintType == SvgPaintFill)
    context->setFillPattern(pattern);
  else
    context->setLinePattern(pattern);

  return Error::Ok;
}

// ============================================================================
// [Fog::SvgRadialGradientElement]
// ============================================================================

struct FOG_HIDDEN SvgRadialGradientElement : public SvgElement
{
  // [Construction / Destruction]

  typedef SvgElement base;

  SvgRadialGradientElement();
  virtual ~SvgRadialGradientElement();

  // [Attributes]

  virtual XmlAttribute* _createAttribute(const ManagedString& name) const;

  // [SVG Rendering]

  virtual err_t onRender(SvgContext* context) const;
  virtual err_t onApplyPattern(SvgContext* context, SvgElement* obj, int paintType) const;

  // [Embedded Attributes]

  SvgEnumAttribute a_spreadMethod;
  SvgEnumAttribute a_gradientUnits;
  SvgTransformAttribute a_gradientTransform;
  SvgCoordAttribute a_cx;
  SvgCoordAttribute a_cy;
  SvgCoordAttribute a_fx;
  SvgCoordAttribute a_fy;
  SvgCoordAttribute a_r;

private:
  FOG_DISABLE_COPY(SvgRadialGradientElement)
};

SvgRadialGradientElement::SvgRadialGradientElement() :
  SvgElement(fog_strings->getString(STR_SVG_radialGradient), SvgTypeRadialGradient),
  a_spreadMethod(NULL, fog_strings->getString(STR_SVG_spreadMethod), svgEnumList_spreadMethod, FOG_OFFSET_OF(SvgRadialGradientElement, a_spreadMethod)),
  a_gradientUnits(NULL, fog_strings->getString(STR_SVG_gradientUnits), svgEnumList_gradientUnits, FOG_OFFSET_OF(SvgRadialGradientElement, a_gradientUnits)),
  a_gradientTransform(NULL, fog_strings->getString(STR_SVG_gradientTransform), FOG_OFFSET_OF(SvgRadialGradientElement, a_gradientTransform)),
  a_cx(NULL, fog_strings->getString(STR_SVG_cx), FOG_OFFSET_OF(SvgRadialGradientElement, a_cx)),
  a_cy(NULL, fog_strings->getString(STR_SVG_cy), FOG_OFFSET_OF(SvgRadialGradientElement, a_cy)),
  a_fx(NULL, fog_strings->getString(STR_SVG_fx), FOG_OFFSET_OF(SvgRadialGradientElement, a_fx)),
  a_fy(NULL, fog_strings->getString(STR_SVG_fy), FOG_OFFSET_OF(SvgRadialGradientElement, a_fx)),
  a_r(NULL, fog_strings->getString(STR_SVG_r), FOG_OFFSET_OF(SvgRadialGradientElement, a_r))
{
}

SvgRadialGradientElement::~SvgRadialGradientElement()
{
  removeAllAttributes();
}

XmlAttribute* SvgRadialGradientElement::_createAttribute(const ManagedString& name) const
{
  if (name == fog_strings->getString(STR_SVG_spreadMethod)) return const_cast<SvgEnumAttribute*>(&a_spreadMethod);
  if (name == fog_strings->getString(STR_SVG_gradientUnits)) return const_cast<SvgEnumAttribute*>(&a_gradientUnits);
  if (name == fog_strings->getString(STR_SVG_gradientTransform)) return const_cast<SvgTransformAttribute*>(&a_gradientTransform);
  if (name == fog_strings->getString(STR_SVG_cx)) return const_cast<SvgCoordAttribute*>(&a_cx);
  if (name == fog_strings->getString(STR_SVG_cy)) return const_cast<SvgCoordAttribute*>(&a_cy);
  if (name == fog_strings->getString(STR_SVG_fx)) return const_cast<SvgCoordAttribute*>(&a_fx);
  if (name == fog_strings->getString(STR_SVG_fy)) return const_cast<SvgCoordAttribute*>(&a_fy);
  if (name == fog_strings->getString(STR_SVG_r)) return const_cast<SvgCoordAttribute*>(&a_r);

  return base::_createAttribute(name);
}

err_t SvgRadialGradientElement::onRender(SvgContext* context) const
{
  return Error::Ok;
}

err_t SvgRadialGradientElement::onApplyPattern(SvgContext* context, SvgElement* obj, int paintType) const
{
  // SVG TODO: Radial Gradient.

  Pattern pattern;

  // Set gradient type to LinearGradient.
  pattern.setType(Pattern::TypeRadialGradient);

  // Set center and focal point - [cx, cy] and [fx, fy].
  if (a_cx.isAssigned() && a_cy.isAssigned() && a_fx.isAssigned() && a_fy.isAssigned() && a_r.isAssigned())
  {
    double cx = context->translateCoord(a_cx.getDouble(), a_cx.getUnit());
    double cy = context->translateCoord(a_cy.getDouble(), a_cy.getUnit());
    double fx = context->translateCoord(a_fx.getDouble(), a_fx.getUnit());
    double fy = context->translateCoord(a_fy.getDouble(), a_fy.getUnit());
    double r = context->translateCoord(a_r.getDouble(), a_r.getUnit());

    if (!a_gradientUnits.isAssigned() || a_gradientUnits.getEnum() == SvgObjectBoundingBox)
    {
      RectD bbox = obj->getBoundingRect();
      cx = bbox.getX() + bbox.getWidth () * cx;
      cy = bbox.getY() + bbox.getHeight() * cy;
      fx = bbox.getX() + bbox.getWidth () * fx;
      fy = bbox.getY() + bbox.getHeight() * fy;
      r = Math::max(bbox.getWidth(), bbox.getHeight()) * r;
    }

    pattern.setPoints(PointD(cx, cy), PointD(fx, fy));
    pattern.setGradientRadius(r);
  }
  else
  {
    RectD bbox = obj->getBoundingRect();
    double x = bbox.getX1() + bbox.getWidth() / 2.0;
    double y = bbox.getY1() + bbox.getHeight() / 2.0; 

    pattern.setPoints(PointD(x, y), PointD(x, y));
    pattern.setGradientRadius(Math::min(bbox.getWidth(), bbox.getHeight()) / 2.0);
  }

  // Add color stops.
  walkAndAddGradientStops(const_cast<SvgRadialGradientElement*>(this), pattern);

  if (paintType == SvgPaintFill)
    context->setFillPattern(pattern);
  else
    context->setLinePattern(pattern);

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

XmlElement* SvgDocument::createElement(const ManagedString& tagName)
{
  return createElementStatic(tagName);
}

XmlElement* SvgDocument::createElementStatic(const ManagedString& tagName)
{
  if (tagName == fog_strings->getString(STR_SVG_circle        )) return new(std::nothrow) SvgCircleElement();
  if (tagName == fog_strings->getString(STR_SVG_defs          )) return new(std::nothrow) SvgDefsElement();
  if (tagName == fog_strings->getString(STR_SVG_ellipse       )) return new(std::nothrow) SvgEllipseElement();
  if (tagName == fog_strings->getString(STR_SVG_g             )) return new(std::nothrow) SvgGElement();
  if (tagName == fog_strings->getString(STR_SVG_line          )) return new(std::nothrow) SvgLineElement();
  if (tagName == fog_strings->getString(STR_SVG_linearGradient)) return new(std::nothrow) SvgLinearGradientElement();
  if (tagName == fog_strings->getString(STR_SVG_path          )) return new(std::nothrow) SvgPathElement();
  if (tagName == fog_strings->getString(STR_SVG_polygon       )) return new(std::nothrow) SvgPolygonElement();
  if (tagName == fog_strings->getString(STR_SVG_polyline      )) return new(std::nothrow) SvgPolyLineElement();
  if (tagName == fog_strings->getString(STR_SVG_radialGradient)) return new(std::nothrow) SvgRadialGradientElement();
  if (tagName == fog_strings->getString(STR_SVG_rect          )) return new(std::nothrow) SvgRectElement();
  if (tagName == fog_strings->getString(STR_SVG_solidColor    )) return new(std::nothrow) SvgSolidColorElement();
  if (tagName == fog_strings->getString(STR_SVG_stop          )) return new(std::nothrow) SvgStopElement();

  // If element is not SVG, use base class to create a default element
  // for the given tagName.
  return XmlDocument::createElementStatic(tagName);
}

err_t SvgDocument::onRender(SvgContext* context) const
{
  return SvgElement::_walkAndRender(this, context);
}

} // Fog namespace
