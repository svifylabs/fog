// [Fog/Svg Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_SVG_SVGCONTEXT_H
#define _FOG_SVG_SVGCONTEXT_H

// [Dependencies]
#include <Fog/Core/Static.h>
#include <Fog/Graphics/Painter.h>
#include <Fog/Graphics/PainterEngine.h>
#include <Fog/Svg/SvgDom.h>

namespace Fog {

//! @addtogroup Fog_Svg
//! @{

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct SvgContext;
struct SvgContextBackup;
struct SvgDocument;
struct SvgElement;

// ============================================================================
// [Fog::SvgContext]
// ============================================================================

//! @brief SVG Rendering context.
struct FOG_API SvgContext
{
  // [Construction / Destruction]

  SvgContext(Painter* painter);
  ~SvgContext();

  // [Style]

  struct FOG_HIDDEN Style
  {
    // [Construction / Destruction]

    FOG_INLINE Style() :
      type(SvgPatternColor),
      opacity(1.0),
      color(0xFF000000)
    {
    }

    FOG_INLINE Style(const Style& other) :
      type(other.type),
      opacity(other.opacity),
      color(other.color),
      pattern(other.pattern)
    {
    }

    FOG_INLINE ~Style()
    {
    }

    // [Operator Overload]

    FOG_INLINE Style& operator=(const Style& other)
    {
      type    = other.type;
      opacity = other.opacity;
      color   = other.color;
      pattern = other.pattern;
      return *this;
    }

    // [Members]

    uint32_t type;
    float opacity;
    Rgba color;
    Pattern pattern;
  };

  // [Painter]

  FOG_INLINE Painter* getPainter() const
  {
    return _painter;
  }

  // [Matrix]

  FOG_INLINE Matrix getMatrix() const
  {
    return _painter->getMatrix();
  }

  // [Fill Parameters]

  FOG_INLINE const Style& getFillStyle() const
  {
    return _fillStyle;
  }

  FOG_INLINE void setFillStyle(const Style& style)
  {
    _fillStyle = style;
  }

  FOG_INLINE void setFillNone()
  {
    _fillStyle.type = SvgPatternNone;
  }

  FOG_INLINE void setFillColor(Rgba color)
  {
    _fillStyle.type = SvgPatternColor;
    _fillStyle.color = color;
  }

  FOG_INLINE void setFillPattern(const Pattern& pattern)
  {
    _fillStyle.type = SvgPatternUri;
    _fillStyle.pattern = pattern;
  }

  FOG_INLINE void setFillMode(uint32_t rule)
  {
    _fillMode = rule;
  }

  // [Line Parameters]

  FOG_INLINE const Style& getLineStyle()
  {
    return _lineStyle;
  }

  FOG_INLINE void setLineStyle(const Style& style)
  {
    _lineStyle = style;
  }

  FOG_INLINE void setLineNone()
  {
    _lineStyle.type = SvgPatternNone;
  }

  FOG_INLINE void setLineColor(Rgba color)
  {
    _lineStyle.type = SvgPatternColor;
    _lineStyle.color = color;
  }

  FOG_INLINE void setLinePattern(const Pattern& pattern)
  {
    _lineStyle.type = SvgPatternUri;
    _lineStyle.pattern = pattern;
  }

  FOG_INLINE void setLineParams(const LineParams& lineParams)
  {
    _lineParams = lineParams;
  }

  FOG_INLINE void setLineWidth(double lineWidth)
  {
    _lineParams.lineWidth = lineWidth;
  }

  FOG_INLINE void setLineCap(uint32_t lineCap)
  {
    _lineParams.lineCap = lineCap;
  }

  FOG_INLINE void setLineJoin(uint32_t lineJoin)
  {
    _lineParams.lineJoin = lineJoin;
  }

  FOG_INLINE void setMiterLimit(double miterLimit)
  {
    _lineParams.miterLimit = miterLimit;
  }

  // [DPI]

  FOG_INLINE double getDpi() const
  {
    return _dpi; 
  }

  void setDpi(double dpi);

  FOG_INLINE double translateCoord(double val, uint32_t unit) const
  {
    FOG_ASSERT(unit < SvgUnitCount);
    return val * _translateCoordData[unit];
  }

  // [Draw]

  FOG_INLINE void setupFillStyle()
  {
    if (_fillStyle.type == SvgPatternColor)
      _painter->setSource(_fillStyle.color);
    else
      _painter->setSource(_fillStyle.pattern);

    _painter->setFillMode(_fillMode);
  }

  FOG_INLINE void setupStrokeStyle()
  {
    if (_lineStyle.type == SvgPatternColor)
      _painter->setSource(_lineStyle.color);
    else
      _painter->setSource(_lineStyle.pattern);

    _painter->setLineParams(_lineParams);
  }

  void drawEllipse(const PointD& cp, const PointD& r);
  void drawLine(const PointD& p1, const PointD& p2);
  void drawRect(const RectD& rect);
  void drawRound(const RectD& rect, const PointD& r);
  void drawPath(const Path& path);

  // [Members]

  Painter* _painter;

  Style _fillStyle;
  Style _lineStyle;
  LineParams _lineParams;
  uint32_t _fillMode;

  double _dpi;
  double _translateCoordData[SvgUnitCount];

private:
  friend struct SvgContextBackup;

  FOG_DISABLE_COPY(SvgContext)
};

// ============================================================================
// [Fog::SvgContextBackup]
// ============================================================================

struct FOG_HIDDEN SvgContextBackup
{
  FOG_INLINE SvgContextBackup() : _context(NULL) {}
  FOG_INLINE SvgContextBackup(SvgContext* context) { init(context); }
  FOG_INLINE ~SvgContextBackup() { if (_context) destroy(); }

  FOG_INLINE void init(SvgContext* context)
  {
    _context = context;

    _fillStyle.init(_context->_fillStyle);
    _fillMode = _context->_fillMode;

    _lineStyle.init(_context->_lineStyle);
    _lineParams.init(_context->_lineParams);

    _matrixBackup = false;
  }

  FOG_INLINE void destroy()
  {
    _context->_fillStyle = _fillStyle.instance();
    _context->_fillMode = _fillMode;

    _context->_lineStyle = _lineStyle.instance();
    _context->_lineParams = _lineParams.instance();

    if (_matrixBackup)
    {
      _context->_painter->setMatrix(_matrix.instance());
    }

    _fillStyle.destroy();

    _lineStyle.destroy();
    _lineParams.destroy();
  }

  SvgContext* _context;

  Static<Matrix> _matrix;

  Static<SvgContext::Style> _fillStyle;
  uint32_t _fillMode;

  Static<SvgContext::Style> _lineStyle;
  Static<LineParams> _lineParams;

  bool _matrixBackup;

  friend struct SvgContext;

  FOG_DISABLE_COPY(SvgContextBackup)
};

//! @}

} // Fog namespace

// [Guard]
#endif // _FOG_SVG_SVGCONTEXT_H
