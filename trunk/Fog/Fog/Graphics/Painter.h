// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_PAINTER_H
#define _FOG_GRAPHICS_PAINTER_H

// [Dependencies]
#include <Fog/Core/Atomic.h>
#include <Fog/Core/Vector.h>
#include <Fog/Graphics/AffineMatrix.h>
#include <Fog/Graphics/Font.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/Path.h>
#include <Fog/Graphics/Region.h>
#include <Fog/Graphics/Rgba.h>

namespace Fog {

// ============================================================================
// [Fog::PainterDevice]
// ============================================================================

//! @brief Painter device.
struct FOG_API PainterDevice
{
  PainterDevice();
  virtual ~PainterDevice();

  // [Meta]

  virtual int width() const = 0;
  virtual int height() const = 0;
  virtual int format() const = 0;

  virtual void setMetaVariables(
    const Point& metaOrigin, 
    const Region& metaRegion,
    bool useMetaRegion,
    bool reset) = 0;

  virtual void setMetaOrigin(const Point& pt) = 0;
  virtual void setUserOrigin(const Point& pt) = 0;

  virtual void translateMetaOrigin(const Point& pt) = 0;
  virtual void translateUserOrigin(const Point& pt) = 0;

  virtual void setUserRegion(const Rect& r) = 0;
  virtual void setUserRegion(const Region& r) = 0;

  virtual void resetMetaVars() = 0;
  virtual void resetUserVars() = 0;

  virtual Point metaOrigin() const = 0;
  virtual Point userOrigin() const = 0;

  virtual Region metaRegion() const = 0;
  virtual Region userRegion() const = 0;

  virtual bool usedMetaRegion() const = 0;
  virtual bool usedUserRegion() const = 0;

  // [Operator]

  virtual void setOp(uint32_t op) = 0;
  virtual uint32_t op() const = 0;

  // [Source]

  virtual void setSource(const Rgba& rgba) = 0;
  //virtual void setSource(const Pattern& pattern) = 0;

  virtual Rgba sourceRgba() = 0;
  //virtual Pattern sourcePattern() = 0;

  // [Parameters]

  virtual void setLineWidth(double lineWidth) = 0;
  virtual double lineWidth() const = 0;

  virtual void setLineCap(uint32_t lineCap) = 0;
  virtual uint32_t lineCap() const = 0;

  virtual void setLineJoin(uint32_t lineJoin) = 0;
  virtual uint32_t lineJoin() const = 0;

  virtual void setLineDash(const double* dashes, sysuint_t count) = 0;
  virtual void setLineDash(const Vector<double>& dashes) = 0;
  virtual Vector<double> lineDash() const = 0;

  virtual void setLineDashOffset(double offset) = 0;
  virtual double lineDashOffset() const = 0;

  virtual void setMiterLimit(double miterLimit) = 0;
  virtual double miterLimit() const = 0;

  virtual void setFillMode(uint32_t mode) = 0;
  virtual uint32_t fillMode() = 0;

  // [Transformations]

  virtual void setMatrix(const AffineMatrix& m) = 0;
  virtual void resetMatrix() = 0;
  virtual AffineMatrix matrix() const = 0;

  virtual void rotate(double angle) = 0;
  virtual void scale(double sx, double sy) = 0;
  virtual void skew(double sx, double sy) = 0;
  virtual void translate(double x, double y) = 0;
  virtual void affine(const AffineMatrix& m) = 0;
  virtual void parallelogram(double x1, double y1, double x2, double y2, const double* para) = 0;
  virtual void viewport(
    double worldX1,  double worldY1,  double worldX2,  double worldY2,
    double screenX1, double screenY1, double screenX2, double screenY2,
    uint32_t viewportOption) = 0;

  // [Raster Drawing]

  virtual void clear() = 0;
  virtual void drawPixel(const Point& p) = 0;
  virtual void drawLine(const Point& start, const Point& end) = 0;
  virtual void drawRect(const Rect& r) = 0;
  virtual void drawRound(const Rect& r, const Point& radius) = 0;
  virtual void fillRect(const Rect& r) = 0;
  virtual void fillRects(const Rect* r, sysuint_t count) = 0;
  virtual void fillRound(const Rect& r, const Point& radius) = 0;
  virtual void fillRegion(const Region& region) = 0;

  // [Vector Drawing]

  virtual void drawPoint(const PointF& p) = 0;
  virtual void drawLine(const PointF& start, const PointF& end) = 0;
  virtual void drawLine(const PointF* pts, sysuint_t count) = 0;
  virtual void drawPolygon(const PointF* pts, sysuint_t count) = 0;
  virtual void drawRect(const RectF& r) = 0;
  virtual void drawRects(const RectF* r, sysuint_t count) = 0;
  virtual void drawRound(const RectF& r, const PointF& radius) = 0;
  virtual void drawRound(const RectF& r, 
    const PointF& tlr, const PointF& trr,
    const PointF& blr, const PointF& brr) = 0;
  virtual void drawEllipse(const PointF& cp, const PointF& r) = 0;
  virtual void drawArc(const PointF& cp, const PointF& r, double start, double sweep) = 0;
  virtual void drawPath(const Path& path) = 0;

  virtual void fillPolygon(const PointF* pts, sysuint_t count) = 0;
  virtual void fillRect(const RectF& r) = 0;
  virtual void fillRects(const RectF* r, sysuint_t count) = 0;
  virtual void fillRound(const RectF& r, const PointF& radius) = 0;
  virtual void fillRound(const RectF& r,
    const PointF& tlr, const PointF& trr,
    const PointF& blr, const PointF& brr) = 0;
  virtual void fillEllipse(const PointF& cp, const PointF& r) = 0;
  virtual void fillArc(const PointF& cp, const PointF& r, double start, double sweep) = 0;
  virtual void fillPath(const Path& path) = 0;

  // [Glyph / Text Drawing]

  virtual void drawGlyph(const Point& pt, const Glyph& glyph, const Rect* clip) = 0;
  virtual void drawGlyphSet(const Point& pt, const GlyphSet& glyphSet, const Rect* clip) = 0;

  virtual void drawText(const Point& p, const String32& text, const Font& font, const Rect* clip) = 0;
  virtual void drawText(const Rect& r, const String32& text, const Font& font, uint32_t align, const Rect* clip) = 0;

  // [Image Drawing]

  virtual void drawImage(const Point& p, const Image& image, const Rect* irect) = 0;

  // [Flush]

  virtual void flush() = 0;

private:
  FOG_DISABLE_COPY(PainterDevice)
};

// ============================================================================
// [Fog::Painter]
// ============================================================================

//! @brief Painter.
struct FOG_API Painter 
{
  // [Construction / Destruction]

  Painter();
  Painter(uint8_t* pixels, int width, int height, sysint_t stride, int format);
  Painter(Image& image);
  ~Painter();

  // [Begin / End]

  err_t begin(uint8_t* pixels, int width, int height, sysint_t stride, int format);
  err_t begin(Image& image);
  void end();

  // [Meta]

  FOG_INLINE int width() const { return _d->width(); }
  FOG_INLINE int height() const { return _d->height(); }
  FOG_INLINE int format() const { return _d->format(); }

  FOG_INLINE void setMetaVariables(
    const Point& metaOrigin,
    const Region& metaRegion, 
    bool useMetaRegion,
    bool reset)
  { _d->setMetaVariables(metaOrigin, metaRegion, useMetaRegion, reset); }

  FOG_INLINE void setMetaOrigin(const Point& p) { _d->setMetaOrigin(p); }
  FOG_INLINE void setUserOrigin(const Point& p) { _d->setUserOrigin(p); }

  FOG_INLINE void translateMetaOrigin(const Point& p) { _d->translateMetaOrigin(p); }
  FOG_INLINE void translateUserOrigin(const Point& p) { _d->translateUserOrigin(p); }

  FOG_INLINE void setUserRegion(const Rect& r) { _d->setUserRegion(r); }
  FOG_INLINE void setUserRegion(const Region& r) { _d->setUserRegion(r); }

  FOG_INLINE void resetMetaVars() { _d->resetMetaVars(); }
  FOG_INLINE void resetUserVars() { _d->resetUserVars(); }

  FOG_INLINE Point metaOrigin() const { return _d->metaOrigin(); }
  FOG_INLINE Point userOrigin() const { return _d->userOrigin(); }

  FOG_INLINE Region metaRegion() const { return _d->metaRegion(); }
  FOG_INLINE Region userRegion() const { return _d->userRegion(); }

  FOG_INLINE bool usedMetaRegion() const { return _d->usedMetaRegion(); }
  FOG_INLINE bool usedUserRegion() const { return _d->usedUserRegion(); }

  // [Operator]

  FOG_INLINE void setOp(uint32_t op) { _d->setOp(op); }
  FOG_INLINE uint32_t op() const { return _d->op(); }

  // [Source]

  FOG_INLINE void setSource(const Rgba& rgba) { _d->setSource(rgba); }
  //FOG_INLINE void setSource(const Pattern& pattern) { _d->setSource(pattern); }

  FOG_INLINE Rgba sourceRgba() const { return _d->sourceRgba(); }
  //FOG_INLINE Pattern sourcePattern() const { return _d->sourcePattern(); }

  // [Parameters]

  FOG_INLINE void setLineWidth(double lineWidth) { _d->setLineWidth(lineWidth); }
  FOG_INLINE double lineWidth() const { return _d->lineWidth(); }

  FOG_INLINE void setLineCap(uint32_t lineCap) { _d->setLineCap(lineCap);}
  FOG_INLINE uint32_t lineCap() const { return _d->lineCap(); }

  FOG_INLINE void setLineJoin(uint32_t lineJoin) { _d->setLineJoin(lineJoin); }
  FOG_INLINE uint32_t lineJoin() const { return _d->lineJoin(); }

  FOG_INLINE void setLineDash(const double* dashes, sysuint_t count) { _d->setLineDash(dashes, count); }
  FOG_INLINE void setLineDash(const Vector<double>& dashes) { _d->setLineDash(dashes); }
  FOG_INLINE Vector<double> lineDash() const { return _d->lineDash(); }

  FOG_INLINE void setLineDashOffset(double offset) { _d->setLineDashOffset(offset); }
  FOG_INLINE double lineDashOffset() const { return _d->lineDashOffset(); }

  FOG_INLINE void setMiterLimit(double miterLimit) { _d->setMiterLimit(miterLimit); }
  FOG_INLINE double miterLimit() const { return _d->miterLimit(); }

  FOG_INLINE void setFillMode(uint32_t mode) { _d->setFillMode(mode); }
  FOG_INLINE uint32_t fillMode() { return _d->fillMode(); }

  // [Transformations]

  FOG_INLINE void setMatrix(const AffineMatrix& m) { _d->setMatrix(m); }
  FOG_INLINE void resetMatrix() { _d->resetMatrix(); }
  FOG_INLINE AffineMatrix matrix() const { return _d->matrix(); }

  FOG_INLINE void rotate(double angle) { _d->rotate(angle); }
  FOG_INLINE void scale(double sx, double sy) { _d->scale(sx, sy); }
  FOG_INLINE void skew(double sx, double sy) { _d->skew(sx, sy); }
  FOG_INLINE void translate(double x, double y) { _d->translate(x, y); }
  FOG_INLINE void affine(const AffineMatrix& m) { _d->affine(m); }

  FOG_INLINE void parallelogram(double x1, double y1, double x2, double y2, const double* para)
  { _d->parallelogram(x1, y1, x2, y2, para); }

  FOG_INLINE void viewport(
    double worldX1,  double worldY1,  double worldX2,  double worldY2,
    double screenX1, double screenY1, double screenX2, double screenY2,
    uint32_t viewportOption = ViewXMidYMid)
  {
    _d->viewport(
      worldX1, worldY1, worldX2, worldY2,
      screenX1, screenY1, screenX2, screenY2,
      viewportOption);
  }

  // [Raster Drawing]

  FOG_INLINE void clear() { _d->clear(); }
  FOG_INLINE void drawPixel(const Point& p) { _d->drawPixel(p); }
  FOG_INLINE void drawLine(const Point& start, const Point& end) { _d->drawLine(start, end); }
  FOG_INLINE void drawRect(const Rect& r) { _d->drawRect(r); }
  FOG_INLINE void drawRound(const Rect& r, const Point& radius) { _d->drawRound(r, radius); }
  FOG_INLINE void fillRect(const Rect& r) { _d->fillRect(r); }
  FOG_INLINE void fillRects(const Rect* r, sysuint_t count) { _d->fillRects(r, count); }
  FOG_INLINE void fillRound(const Rect& r, const Point& radius) { _d->fillRound(r, radius); }
  FOG_INLINE void fillRegion(const Region& region) { _d->fillRegion(region); }

  // [Vector Drawing]

  FOG_INLINE void drawPoint(const PointF& p) { _d->drawPoint(p); }
  FOG_INLINE void drawLine(const PointF& start, const PointF& end) { _d->drawLine(start, end); }
  FOG_INLINE void drawLine(const PointF* pts, sysuint_t count) { _d->drawLine(pts, count); }
  FOG_INLINE void drawPolygon(const PointF* pts, sysuint_t count) { _d->drawPolygon(pts, count); }
  FOG_INLINE void drawRect(const RectF& r) { _d->drawRect(r); }
  FOG_INLINE void drawRects(const RectF* r, sysuint_t count) { _d->drawRects(r, count); }
  FOG_INLINE void drawRound(const RectF& r, const PointF& radius) { _d->drawRound(r, radius); }
  FOG_INLINE void drawRound(const RectF& r, 
    const PointF& tlr, const PointF& trr,
    const PointF& blr, const PointF& brr) { _d->drawRound(r, tlr, trr, blr, brr); }
  FOG_INLINE void drawEllipse(const PointF& cp, const PointF& r) { _d->drawEllipse(cp, r); }
  FOG_INLINE void drawArc(const PointF& cp, const PointF& r, double start, double sweep) { _d->drawArc(cp, r, start, sweep); }
  FOG_INLINE void drawPath(const Path& path) { _d->drawPath(path); }

  FOG_INLINE void fillPolygon(const PointF* pts, sysuint_t count) { _d->fillPolygon(pts, count); }
  FOG_INLINE void fillRect(const RectF& r) { _d->fillRect(r); }
  FOG_INLINE void fillRects(const RectF* r, sysuint_t count) { _d->fillRects(r, count); }
  FOG_INLINE void fillRound(const RectF& r, const PointF& radius) { _d->fillRound(r, radius); }
  FOG_INLINE void fillRound(const RectF& r,
    const PointF& tlr, const PointF& trr,
    const PointF& blr, const PointF& brr) { _d->fillRound(r, tlr, trr, blr, brr); }
  FOG_INLINE void fillEllipse(const PointF& cp, const PointF& r) { _d->fillEllipse(cp, r); }
  FOG_INLINE void fillArc(const PointF& cp, const PointF& r, double start, double sweep) { _d->fillArc(cp, r, start, sweep); }
  FOG_INLINE void fillPath(const Path& path) { _d->fillPath(path); }

  // [Glyph / Text Drawing]

  FOG_INLINE void drawGlyph(
    const Point& pt, const Glyph& glyph, const Rect* clip = 0)
  { _d->drawGlyph(pt, glyph, clip); }

  FOG_INLINE void drawGlyphSet(
    const Point& pt, const GlyphSet& glyphSet, const Rect* clip = 0)
  { _d->drawGlyphSet(pt, glyphSet, clip); }

  // [Text drawing]

  FOG_INLINE void drawText(
    const Point& p, const String32& text, const Font& font, const Rect* clip = NULL)
  { _d->drawText(p, text, font, clip); }

  FOG_INLINE void drawText(
    const Rect& r, const String32& text, const Font& font, uint32_t align, const Rect* clip = NULL)
  { _d->drawText(r, text, font, align, clip); }

  // [Image Drawing]

  FOG_INLINE void drawImage(const Point& p, const Image& image, const Rect* irect = 0)
  { _d->drawImage(p, image, irect); }

  // [Flush]

  FOG_INLINE void flush() { _d->flush(); }

  // [Members]

  FOG_DECLARE_D(PainterDevice)

  static PainterDevice* sharedNull;

private:
  FOG_DISABLE_COPY(Painter);
};

} // Fog namespace

// [Guard]
#endif // _FOG_GRAPHICS_PAINTER_H
