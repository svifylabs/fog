// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/Cpu.h>
#include <Fog/Core/Memory.h>
#include <Fog/Graphics/Painter.h>
#include <Fog/Graphics/Rgba.h>

// [Antigrain]
#include "agg_alpha_mask_u8.h"
#include "agg_basics.h"
#include "agg_path_storage.h"
#include "agg_conv_contour.h"
#include "agg_conv_curve.h"
#include "agg_conv_dash.h"
#include "agg_conv_stroke.h"
#include "agg_conv_smooth_poly1.h"
#include "agg_pixfmt_rgb.h"
#include "agg_pixfmt_rgba.h"
#include "agg_pixfmt_amask_adaptor.h"
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_rasterizer_outline.h"
#include "agg_renderer_base.h"
#include "agg_renderer_scanline.h"
#include "agg_renderer_primitives.h"
#include "agg_rounded_rect.h"
#include "agg_scanline_p.h"
#include "agg_scanline_u.h"
#include "agg_scanline_bin.h"
#include "agg_vcgen_dash.h"

namespace Fog {

// ============================================================================
// [Fog::PainterDevice]
// ============================================================================

PainterDevice::PainterDevice() {}
PainterDevice::~PainterDevice() {}

// ============================================================================
// [Fog::NullPainterDevice]
// ============================================================================

//! @brief Painter device.
struct FOG_HIDDEN NullPainterDevice : public PainterDevice
{
  NullPainterDevice() {}
  virtual ~NullPainterDevice() {}

  // [Meta]

  virtual uint width() const { return 0; }
  virtual uint height() const { return 0; }
  virtual ImageFormat format() const { return ImageFormat(); }

  virtual void setMetaVariables(const Point& pt, const Region& r, bool begin) {}

  virtual void setMetaOrigin(const Point& pt) {}
  virtual void setUserOrigin(const Point& pt) {}
  virtual void translateMetaOrigin(const Point& pt) {}
  virtual void translateUserOrigin(const Point& pt) {}
  virtual void setUserRegion(const Rect& r) {}
  virtual void setUserRegion(const Region& r) {}

  virtual Point metaOrigin() const { return Point(0, 0); }
  virtual Point userOrigin() const { return Point(0, 0); }
  virtual Region metaRegion() const { return Region(); }
  virtual Region userRegion() const { return Region(); }

  // [Operator]

  virtual void setOp(uint32_t op) {}
  virtual uint32_t op() const { return 0; }

  // [Source]

  virtual void setSource(const Rgba& rgba) {}
  //virtual void setSource(const Pattern& pattern) {}

  virtual Rgba sourceRgba() { return Rgba(0); }
  //virtual Pattern sourcePattern() {}

  // [Parameters]

  virtual void setLineWidth(double lineWidth) {}
  virtual double lineWidth() const { return 0.0; }

  virtual void setLineCap(uint32_t lineCap) {}
  virtual uint32_t lineCap() const { return 0; }

  virtual void setLineJoin(uint32_t lineJoin) {}
  virtual uint32_t lineJoin() const { return 0; }

  virtual void setLineDash(const double* dashes, uint32_t count) {}
  virtual void setLineDash(const Vector<double>& dashes) {}
  virtual Vector<double> lineDash() const { return Vector<double>(); }

  virtual void setLineDashOffset(double offset) {}
  virtual double lineDashOffset() const { return 0.0; }

  virtual void setMiterLimit(double miterLimit) {}
  virtual double miterLimit() const { return 0.0; }

  // [Raster drawing]

  virtual void clear() {}
  virtual void drawPixel(const Point& p) {}
  virtual void drawLine(const Point& start, const Point& end) {}
  virtual void drawRect(const Rect& r) {}
  virtual void drawRound(const Rect& r, const Point& radius) {}
  virtual void fillRect(const Rect& r) {}
  virtual void fillRects(const Rect* r, sysuint_t count) {}
  virtual void fillRound(const Rect& r, const Point& radius) {}
  virtual void fillRegion(const Region& region) {}
  //virtual void fillRegion(const Rect* r, sysuint_t count, const Rect* extents) {}

  // [Vector drawing]

  virtual void drawPoint(const PointF& p) {}
  virtual void drawLine(const PointF& start, const PointF& end) {}
  virtual void drawLine(const PointF* pts, sysuint_t count) {}
  virtual void drawPolygon(const PointF* pts, sysuint_t count) {}
  virtual void drawRect(const RectF& r) {}
  virtual void drawRects(const RectF* r, sysuint_t count) {}
  virtual void drawRound(const RectF& r, const PointF& radius) {}
  virtual void drawRound(const RectF& r, 
    const PointF& tlr, const PointF& trr,
    const PointF& blr, const PointF& brr) {}
  virtual void drawPath(const Path& path) {}

  virtual void fillPolygon(const PointF* pts, sysuint_t count) {}
  virtual void fillRect(const RectF& r) {}
  virtual void fillRects(const RectF* r, sysuint_t count) {}
  virtual void fillRound(const RectF& r, const PointF& radius) {}
  virtual void fillRound(const RectF& r,
    const PointF& tlr, const PointF& trr,
    const PointF& blr, const PointF& brr) {}
  virtual void fillPath(const Path* path) {}

  // [Glyph drawing]

  virtual void drawGlyph(const Point& p, const Image& glyph, const Rect* clip) {}
  virtual void drawGlyphs(const Point* pts, const Image* glyphs, sysuint_t count, const Rect* clip) {}

  // [Text drawing]

  virtual void drawText(const Point& p, const String32& text, const Font& font, const Rect* clip) {}
  virtual void drawText(const Rect& r, const String32& text, const Font& font, uint32_t align, const Rect* clip) {}

  // [Image drawing]

  virtual void drawImage(const Point& p, const Image& image, const Rect* irect) {}
};

// ============================================================================
// [Fog::RasterPainterDevice]
// ============================================================================
#if 0
struct FOG_HIDDEN RasterPainterDevice : public PainterDevice
{
  RasterPainterDevice() {}
  virtual ~RasterPainterDevice() {}

  // [Meta]

  virtual uint width() const;
  virtual uint height() const;
  virtual ImageFormat format() const;

  virtual void setMetaVariables(const Point& pt, const Region& r, bool begin);

  virtual void setMetaOrigin(const Point& pt);
  virtual void setUserOrigin(const Point& pt);
  virtual void translateMetaOrigin(const Point& pt);
  virtual void translateUserOrigin(const Point& pt);
  virtual void setUserRegion(const Rect& r);
  virtual void setUserRegion(const Region& r);

  virtual Point metaOrigin() const;
  virtual Point userOrigin() const;
  virtual Region metaRegion() const;
  virtual Region userRegion() const;

  // [Operator]

  virtual void setOp(uint32_t op);
  virtual uint32_t op() const;

  // [Source]

  virtual void setSource(const Rgba& rgba);
  //virtual void setSource(const Pattern& pattern);

  virtual Rgba sourceRgba();
  //virtual Pattern sourcePattern();

  // [Parameters]

  virtual void setLineWidth(double lineWidth);
  virtual double lineWidth() const;

  virtual void setLineCap(uint32_t lineCap);
  virtual uint32_t lineCap() const;

  virtual void setLineJoin(uint32_t lineJoin);
  virtual uint32_t lineJoin() const;

  virtual void setLineDash(const double* dashes, uint32_t count);
  virtual void setLineDash(const Vector<double>& dashes);
  virtual Vector<double> lineDash() const;

  virtual void setLineDashOffset(double offset);
  virtual double lineDashOffset() const;

  virtual void setMiterLimit(double miterLimit);
  virtual double miterLimit() const;

  // [Raster drawing]

  virtual void clear();
  virtual void drawPixel(const Point& p);
  virtual void drawLine(const Point& start, const Point& end);
  virtual void drawRect(const Rect& r);
  virtual void drawRound(const Rect& r, const Point& radius);
  virtual void fillRect(const Rect& r);
  virtual void fillRects(const Rect* r, sysuint_t count);
  virtual void fillRound(const Rect& r, const Point& radius);
  virtual void fillRegion(const Region& region);
  //virtual void fillRegion(const Rect* r, sysuint_t count, const Rect* extents);

  // [Vector drawing]

  virtual void drawPoint(const PointF& p);
  virtual void drawLine(const PointF& start, const PointF& end);
  virtual void drawLine(const PointF* pts, sysuint_t count);
  virtual void drawPolygon(const PointF* pts, sysuint_t count);
  virtual void drawRect(const RectF& r);
  virtual void drawRects(const RectF* r, sysuint_t count);
  virtual void drawRound(const RectF& r, const PointF& radius);
  virtual void drawRound(const RectF& r, 
    const PointF& tlr, const PointF& trr,
    const PointF& blr, const PointF& brr);
  virtual void drawPath(const Path& path);

  virtual void fillPolygon(const PointF* pts, sysuint_t count);
  virtual void fillRect(const RectF& r);
  virtual void fillRects(const RectF* r, sysuint_t count);
  virtual void fillRound(const RectF& r, const PointF& radius);
  virtual void fillRound(const RectF& r,
    const PointF& tlr, const PointF& trr,
    const PointF& blr, const PointF& brr);
  virtual void fillPath(const Path* path);

  // [Glyph drawing]

  virtual void drawGlyph(const Point& p, const Image& glyph, const Rect* clip);
  virtual void drawGlyphs(const Point* pts, const Image* glyphs, sysuint_t count, const Rect* clip);

  // [Text drawing]

  virtual void drawText(const Point& p, const String32& text, const Font& font, const Rect* clip);
  virtual void drawText(const Rect& r, const String32& text, const Font& font, uint32_t align, const Rect* clip);

  // [Image drawing]

  virtual void drawImage(const Point& p, const Image& image, const Rect* irect);

  // [Members]
};


uint RasterPainterDevice::width() const;
uint RasterPainterDevice::height() const;
ImageFormat RasterPainterDevice::format() const;

void RasterPainterDevice::setMetaVariables(const Point& pt, const Region& r, bool begin);

void RasterPainterDevice::setMetaOrigin(const Point& pt);
void RasterPainterDevice::setUserOrigin(const Point& pt);
void RasterPainterDevice::translateMetaOrigin(const Point& pt);
void RasterPainterDevice::translateUserOrigin(const Point& pt);
void RasterPainterDevice::setUserRegion(const Rect& r);
void RasterPainterDevice::setUserRegion(const Region& r);

Point RasterPainterDevice::metaOrigin() const;
Point RasterPainterDevice::userOrigin() const;
Region RasterPainterDevice::metaRegion() const;
Region RasterPainterDevice::userRegion() const;

// [Operator]

void RasterPainterDevice::setOp(uint32_t op);
uint32_t RasterPainterDevice::op() const;

// [Source]

void RasterPainterDevice::setSource(const Rgba& rgba);
//void RasterPainterDevice::setSource(const Pattern& pattern);

Rgba RasterPainterDevice::sourceRgba();
//Pattern RasterPainterDevice::sourcePattern();

// [Parameters]

void RasterPainterDevice::setLineWidth(double lineWidth);
double RasterPainterDevice::lineWidth() const;

void RasterPainterDevice::setLineCap(uint32_t lineCap);
uint32_t RasterPainterDevice::lineCap() const;

void RasterPainterDevice::setLineJoin(uint32_t lineJoin);
uint32_t RasterPainterDevice::lineJoin() const;

void RasterPainterDevice::setLineDash(const double* dashes, uint32_t count);
void RasterPainterDevice::setLineDash(const Vector<double>& dashes);
Vector<double> RasterPainterDevice::lineDash() const;

void RasterPainterDevice::setLineDashOffset(double offset);
double RasterPainterDevice::lineDashOffset() const;

void RasterPainterDevice::setMiterLimit(double miterLimit);
double RasterPainterDevice::miterLimit() const;

// [Raster drawing]

void RasterPainterDevice::clear();
void RasterPainterDevice::drawPixel(const Point& p);
void RasterPainterDevice::drawLine(const Point& start, const Point& end);
void RasterPainterDevice::drawRect(const Rect& r);
void RasterPainterDevice::drawRound(const Rect& r, const Point& radius);
void RasterPainterDevice::fillRect(const Rect& r);
void RasterPainterDevice::fillRects(const Rect* r, sysuint_t count);
void RasterPainterDevice::fillRound(const Rect& r, const Point& radius);
void RasterPainterDevice::fillRegion(const Region& region);
//void fillRegion(const Rect* r, sysuint_t count, const Rect* extents);

// [Vector drawing]

void RasterPainterDevice::drawPoint(const PointF& p);
void RasterPainterDevice::drawLine(const PointF& start, const PointF& end);
void RasterPainterDevice::drawLine(const PointF* pts, sysuint_t count);
void RasterPainterDevice::drawPolygon(const PointF* pts, sysuint_t count);
void RasterPainterDevice::drawRect(const RectF& r);
void RasterPainterDevice::drawRects(const RectF* r, sysuint_t count);
void RasterPainterDevice::drawRound(const RectF& r, const PointF& radius);
void RasterPainterDevice::drawRound(const RectF& r, 
  const PointF& tlr, const PointF& trr,
  const PointF& blr, const PointF& brr);
void RasterPainterDevice::drawPath(const Path& path);

void RasterPainterDevice::fillPolygon(const PointF* pts, sysuint_t count);
void RasterPainterDevice::fillRect(const RectF& r);
void RasterPainterDevice::fillRects(const RectF* r, sysuint_t count);
void RasterPainterDevice::fillRound(const RectF& r, const PointF& radius);
void RasterPainterDevice::fillRound(const RectF& r,
  const PointF& tlr, const PointF& trr,
  const PointF& blr, const PointF& brr);
void RasterPainterDevice::fillPath(const Path* path);

// [Glyph drawing]

void RasterPainterDevice::drawGlyph(const Point& p, const Image& glyph, const Rect* clip);
void RasterPainterDevice::drawGlyphs(const Point* pts, const Image* glyphs, sysuint_t count, const Rect* clip);

// [Text drawing]

void RasterPainterDevice::drawText(const Point& p, const String32& text, const Font& font, const Rect* clip);
void RasterPainterDevice::drawText(const Rect& r, const String32& text, const Font& font, uint32_t align, const Rect* clip);

// [Image drawing]

void RasterPainterDevice::drawImage(const Point& p, const Image& image, const Rect* irect);
#endif
// ============================================================================
// [Fog::Painter]
// ============================================================================

PainterDevice* Painter::sharedNull;

Painter::Painter()
{
  _d = sharedNull;
}

Painter::Painter(uint8_t* pixels, int width, int height, sysint_t stride, const ImageFormat& format)
{
  _d = sharedNull;
  begin(pixels, width, height, stride, format);
}

Painter::Painter(Image& image)
{
  _d = sharedNull;
  begin(image);
}

Painter::~Painter()
{
  if (_d != sharedNull) delete _d;
}

void Painter::begin(uint8_t* pixels, int width, int height, sysint_t stride, const ImageFormat& format)
{
}

void Painter::begin(Image& image)
{
}

void Painter::end()
{
  if (_d != sharedNull) delete _d;
  _d = sharedNull;
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_painter_init(void)
{
  static Fog::NullPainterDevice sharedNullDevice;
  Fog::Painter::sharedNull = &sharedNullDevice;

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_painter_shutdown(void)
{
}
