// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_PAINTER_H
#define _FOG_GRAPHICS_PAINTER_H

// [Dependencies]
#include <Fog/Core/Atomic.h>
#include <Fog/Core/String.h>
#include <Fog/Core/Value.h>
#include <Fog/Core/Vector.h>
#include <Fog/Graphics/AffineMatrix.h>
#include <Fog/Graphics/Font.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/Path.h>
#include <Fog/Graphics/Pattern.h>
#include <Fog/Graphics/Region.h>
#include <Fog/Graphics/Rgba.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Fog::PainterEngine]
// ============================================================================

//! @brief Painter engine.
struct FOG_API PainterEngine
{
  PainterEngine();
  virtual ~PainterEngine();

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
  virtual void setSource(const Pattern& pattern) = 0;

  virtual Rgba sourceRgba() = 0;
  virtual Pattern sourcePattern() = 0;

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

  virtual void worldToScreen(PointF* pt) const = 0;
  virtual void screenToWorld(PointF* pt) const = 0;

  virtual void worldToScreen(double* scalar) const = 0;
  virtual void screenToWorld(double* scalar) const = 0;

  virtual void alignPoint(PointF* pt) const = 0;

  // [State]

  virtual void save() = 0;
  virtual void restore() = 0;

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

  // [Properties]

  virtual err_t setProperty(const String32& name, const Value& value) = 0;
  virtual Value getProperty(const String32& name) const = 0;

private:
  FOG_DISABLE_COPY(PainterEngine)
};

// ============================================================================
// [Fog::Painter]
// ============================================================================

//! @brief Painter.
struct FOG_API Painter 
{
  // [Hints]

  enum Hints
  {
    HintNoMultithreading = (1 << 31)
  };

  // [Construction / Destruction]

  Painter();
  Painter(Image& image, int hints = 0);
  ~Painter();

  // [Begin / End]

  err_t begin(uint8_t* pixels, int width, int height, sysint_t stride, int format, int hints = 0);
  err_t begin(Image& image, int hints = 0);
  void end();

  // [Meta]

  FOG_INLINE int width() const { return _engine->width(); }
  FOG_INLINE int height() const { return _engine->height(); }
  FOG_INLINE int format() const { return _engine->format(); }

  FOG_INLINE void setMetaVariables(
    const Point& metaOrigin,
    const Region& metaRegion, 
    bool useMetaRegion,
    bool reset)
  { _engine->setMetaVariables(metaOrigin, metaRegion, useMetaRegion, reset); }

  FOG_INLINE void setMetaOrigin(const Point& p) { _engine->setMetaOrigin(p); }
  FOG_INLINE void setUserOrigin(const Point& p) { _engine->setUserOrigin(p); }

  FOG_INLINE void translateMetaOrigin(const Point& p) { _engine->translateMetaOrigin(p); }
  FOG_INLINE void translateUserOrigin(const Point& p) { _engine->translateUserOrigin(p); }

  FOG_INLINE void setUserRegion(const Rect& r) { _engine->setUserRegion(r); }
  FOG_INLINE void setUserRegion(const Region& r) { _engine->setUserRegion(r); }

  FOG_INLINE void resetMetaVars() { _engine->resetMetaVars(); }
  FOG_INLINE void resetUserVars() { _engine->resetUserVars(); }

  FOG_INLINE Point metaOrigin() const { return _engine->metaOrigin(); }
  FOG_INLINE Point userOrigin() const { return _engine->userOrigin(); }

  FOG_INLINE Region metaRegion() const { return _engine->metaRegion(); }
  FOG_INLINE Region userRegion() const { return _engine->userRegion(); }

  FOG_INLINE bool usedMetaRegion() const { return _engine->usedMetaRegion(); }
  FOG_INLINE bool usedUserRegion() const { return _engine->usedUserRegion(); }

  // [Operator]

  FOG_INLINE void setOp(uint32_t op) { _engine->setOp(op); }
  FOG_INLINE uint32_t op() const { return _engine->op(); }

  // [Source]

  FOG_INLINE void setSource(const Rgba& rgba) { _engine->setSource(rgba); }
  FOG_INLINE void setSource(const Pattern& pattern) { _engine->setSource(pattern); }

  FOG_INLINE Rgba sourceRgba() const { return _engine->sourceRgba(); }
  FOG_INLINE Pattern sourcePattern() const { return _engine->sourcePattern(); }

  // [Parameters]

  FOG_INLINE void setLineWidth(double lineWidth) { _engine->setLineWidth(lineWidth); }
  FOG_INLINE double lineWidth() const { return _engine->lineWidth(); }

  FOG_INLINE void setLineCap(uint32_t lineCap) { _engine->setLineCap(lineCap);}
  FOG_INLINE uint32_t lineCap() const { return _engine->lineCap(); }

  FOG_INLINE void setLineJoin(uint32_t lineJoin) { _engine->setLineJoin(lineJoin); }
  FOG_INLINE uint32_t lineJoin() const { return _engine->lineJoin(); }

  FOG_INLINE void setLineDash(const double* dashes, sysuint_t count) { _engine->setLineDash(dashes, count); }
  FOG_INLINE void setLineDash(const Vector<double>& dashes) { _engine->setLineDash(dashes); }
  FOG_INLINE Vector<double> lineDash() const { return _engine->lineDash(); }

  FOG_INLINE void setLineDashOffset(double offset) { _engine->setLineDashOffset(offset); }
  FOG_INLINE double lineDashOffset() const { return _engine->lineDashOffset(); }

  FOG_INLINE void setMiterLimit(double miterLimit) { _engine->setMiterLimit(miterLimit); }
  FOG_INLINE double miterLimit() const { return _engine->miterLimit(); }

  FOG_INLINE void setFillMode(uint32_t mode) { _engine->setFillMode(mode); }
  FOG_INLINE uint32_t fillMode() { return _engine->fillMode(); }

  // [Transformations]

  FOG_INLINE void setMatrix(const AffineMatrix& m) { _engine->setMatrix(m); }
  FOG_INLINE void resetMatrix() { _engine->resetMatrix(); }
  FOG_INLINE AffineMatrix matrix() const { return _engine->matrix(); }

  FOG_INLINE void rotate(double angle) { _engine->rotate(angle); }
  FOG_INLINE void scale(double sx, double sy) { _engine->scale(sx, sy); }
  FOG_INLINE void skew(double sx, double sy) { _engine->skew(sx, sy); }
  FOG_INLINE void translate(double x, double y) { _engine->translate(x, y); }
  FOG_INLINE void affine(const AffineMatrix& m) { _engine->affine(m); }

  FOG_INLINE void parallelogram(double x1, double y1, double x2, double y2, const double* para)
  { _engine->parallelogram(x1, y1, x2, y2, para); }

  FOG_INLINE void viewport(
    double worldX1,  double worldY1,  double worldX2,  double worldY2,
    double screenX1, double screenY1, double screenX2, double screenY2,
    uint32_t viewportOption = ViewXMidYMid)
  {
    _engine->viewport(
      worldX1, worldY1, worldX2, worldY2,
      screenX1, screenY1, screenX2, screenY2,
      viewportOption);
  }

  FOG_INLINE void worldToScreen(PointF* pt) const { _engine->worldToScreen(pt); }
  FOG_INLINE void screenToWorld(PointF* pt) const { _engine->screenToWorld(pt); }

  FOG_INLINE void worldToScreen(double* scalar) const { _engine->worldToScreen(scalar); }
  FOG_INLINE void screenToWorld(double* scalar) const { _engine->screenToWorld(scalar); }

  FOG_INLINE void alignPoint(PointF* pt) const { _engine->alignPoint(pt); }

  // [State]

  FOG_INLINE void save() { _engine->save(); }
  FOG_INLINE void restore() { _engine->restore(); }

  // [Raster Drawing]

  FOG_INLINE void clear() { _engine->clear(); }
  FOG_INLINE void drawPixel(const Point& p) { _engine->drawPixel(p); }
  FOG_INLINE void drawLine(const Point& start, const Point& end) { _engine->drawLine(start, end); }
  FOG_INLINE void drawRect(const Rect& r) { _engine->drawRect(r); }
  FOG_INLINE void drawRound(const Rect& r, const Point& radius) { _engine->drawRound(r, radius); }
  FOG_INLINE void fillRect(const Rect& r) { _engine->fillRect(r); }
  FOG_INLINE void fillRects(const Rect* r, sysuint_t count) { _engine->fillRects(r, count); }
  FOG_INLINE void fillRound(const Rect& r, const Point& radius) { _engine->fillRound(r, radius); }
  FOG_INLINE void fillRegion(const Region& region) { _engine->fillRegion(region); }

  // [Vector Drawing]

  FOG_INLINE void drawPoint(const PointF& p) { _engine->drawPoint(p); }
  FOG_INLINE void drawLine(const PointF& start, const PointF& end) { _engine->drawLine(start, end); }
  FOG_INLINE void drawLine(const PointF* pts, sysuint_t count) { _engine->drawLine(pts, count); }
  FOG_INLINE void drawPolygon(const PointF* pts, sysuint_t count) { _engine->drawPolygon(pts, count); }
  FOG_INLINE void drawRect(const RectF& r) { _engine->drawRect(r); }
  FOG_INLINE void drawRects(const RectF* r, sysuint_t count) { _engine->drawRects(r, count); }
  FOG_INLINE void drawRound(const RectF& r, const PointF& radius) { _engine->drawRound(r, radius); }
  FOG_INLINE void drawRound(const RectF& r, 
    const PointF& tlr, const PointF& trr,
    const PointF& blr, const PointF& brr) { _engine->drawRound(r, tlr, trr, blr, brr); }
  FOG_INLINE void drawEllipse(const PointF& cp, const PointF& r) { _engine->drawEllipse(cp, r); }
  FOG_INLINE void drawArc(const PointF& cp, const PointF& r, double start, double sweep) { _engine->drawArc(cp, r, start, sweep); }
  FOG_INLINE void drawPath(const Path& path) { _engine->drawPath(path); }

  FOG_INLINE void fillPolygon(const PointF* pts, sysuint_t count) { _engine->fillPolygon(pts, count); }
  FOG_INLINE void fillRect(const RectF& r) { _engine->fillRect(r); }
  FOG_INLINE void fillRects(const RectF* r, sysuint_t count) { _engine->fillRects(r, count); }
  FOG_INLINE void fillRound(const RectF& r, const PointF& radius) { _engine->fillRound(r, radius); }
  FOG_INLINE void fillRound(const RectF& r,
    const PointF& tlr, const PointF& trr,
    const PointF& blr, const PointF& brr) { _engine->fillRound(r, tlr, trr, blr, brr); }
  FOG_INLINE void fillEllipse(const PointF& cp, const PointF& r) { _engine->fillEllipse(cp, r); }
  FOG_INLINE void fillArc(const PointF& cp, const PointF& r, double start, double sweep) { _engine->fillArc(cp, r, start, sweep); }
  FOG_INLINE void fillPath(const Path& path) { _engine->fillPath(path); }

  // [Glyph / Text Drawing]

  FOG_INLINE void drawGlyph(
    const Point& pt, const Glyph& glyph, const Rect* clip = 0)
  { _engine->drawGlyph(pt, glyph, clip); }

  FOG_INLINE void drawGlyphSet(
    const Point& pt, const GlyphSet& glyphSet, const Rect* clip = 0)
  { _engine->drawGlyphSet(pt, glyphSet, clip); }

  // [Text drawing]

  FOG_INLINE void drawText(
    const Point& p, const String32& text, const Font& font, const Rect* clip = NULL)
  { _engine->drawText(p, text, font, clip); }

  FOG_INLINE void drawText(
    const Rect& r, const String32& text, const Font& font, uint32_t align, const Rect* clip = NULL)
  { _engine->drawText(r, text, font, align, clip); }

  // [Image Drawing]

  FOG_INLINE void drawImage(const Point& p, const Image& image, const Rect* irect = 0)
  { _engine->drawImage(p, image, irect); }

  // [Flush]

  FOG_INLINE void flush() { _engine->flush(); }

  // [Properties]

  FOG_INLINE err_t setProperty(const String32& name, const Value& value)
  { return _engine->setProperty(name, value); }

  FOG_INLINE Value getProperty(const String32& name) const
  { return _engine->getProperty(name); }

  // [Members]

  PainterEngine* _engine;

  static PainterEngine* sharedNull;

private:
  FOG_DISABLE_COPY(Painter);
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_PAINTER_H
