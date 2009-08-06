// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_PAINTERENGINE_H
#define _FOG_GRAPHICS_PAINTERENGINE_H

// [Dependencies]
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

//! @brief Painter engine is @c Painter backend.
//!
//! See @c Painter all virtual members documentation.
struct FOG_API PainterEngine
{
  PainterEngine();
  virtual ~PainterEngine() = 0;

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
  virtual void drawPoint(const Point& p) = 0;
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
  virtual void drawEllipse(const PointF& cp, const PointF& r) = 0;
  virtual void drawArc(const PointF& cp, const PointF& r, double start, double sweep) = 0;
  virtual void drawPath(const Path& path) = 0;

  virtual void fillPolygon(const PointF* pts, sysuint_t count) = 0;
  virtual void fillRect(const RectF& r) = 0;
  virtual void fillRects(const RectF* r, sysuint_t count) = 0;
  virtual void fillRound(const RectF& r, const PointF& radius) = 0;
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

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_PAINTERENGINE_H
