// [Fog/Svg Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_SVG_SVGCONTEXT_H
#define _FOG_SVG_SVGCONTEXT_H

// [Dependencies]
#include <Fog/Graphics/Painter.h>
#include <Fog/Svg/SvgDom.h>

namespace Fog {

//! @addtogroup Fog_Svg
//! @{

// ============================================================================
// [Forward Declarations]
// ============================================================================

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
      enabled(true),
      opacity(1.0)
    {
    }

    FOG_INLINE Style(const Style& other) :
      enabled(other.enabled),
      opacity(other.opacity),
      pattern(other.pattern)
    {
    }

    FOG_INLINE ~Style()
    {
    }

    // [Operator Overload]

    FOG_INLINE Style& operator=(const Style& other)
    {
      enabled = other.enabled;
      opacity = other.opacity;
      pattern = other.pattern;
      return *this;
    }

    // [Members]

    uint32_t enabled;
    float opacity;
    Pattern pattern;
  };

  // [Stroke Style]

  FOG_INLINE const Style& strokeStyle()
  {
    return _strokeStyle;
  }

  FOG_INLINE void setStrokeNone()
  {
    _strokeStyle.enabled = false;
  }

  FOG_INLINE void setStrokeColor(Rgba color)
  {
    _strokeStyle.pattern.setColor(color);
  }

  FOG_INLINE void setStrokeStyle(const Style& style)
  {
    _strokeStyle = style;
  }

  // [Fill Style]

  FOG_INLINE const Style& fillStyle()
  {
    return _fillStyle;
  }

  FOG_INLINE void setFillNone()
  {
    _fillStyle.enabled = false;
  }

  FOG_INLINE void setFillColor(Rgba color)
  {
    _fillStyle.pattern.setColor(color);
  }

  FOG_INLINE void setFillStyle(const Style& style)
  {
    _fillStyle = style;
  }

  // [Painter]

  FOG_INLINE Painter* painter() const
  {
    return _painter;
  }

  void drawEllipse(const PointF& cp, const PointF& r);
  void drawLine(const PointF& p1, const PointF& p2);
  void drawRect(const RectF& rect);
  void drawRound(const RectF& rect, const PointF& r);
  void drawPath(const Path& path);

protected:
  Painter* _painter;

  Style _strokeStyle;
  Style _fillStyle;

private:
  FOG_DISABLE_COPY(SvgContext)
};

//! @}

} // Fog namespace

// [Guard]
#endif // _FOG_SVG_SVGCONTEXT_H
