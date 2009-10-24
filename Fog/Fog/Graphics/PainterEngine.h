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
#include <Fog/Graphics/Matrix.h>
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
// [Fog::LineParams]
// ============================================================================

struct FOG_API LineParams
{
  // [Construction / Destruction]

  LineParams();
  LineParams(const LineParams& other);
  ~LineParams();

  // [Operator Overload]

  LineParams& operator=(const LineParams& other);

  // [Members]

  double lineWidth;
  uint32_t lineCap;
  uint32_t lineJoin;
  double miterLimit;
  Vector<double> dashes;
  double dashOffset;
};

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

  virtual int getWidth() const = 0;
  virtual int getHeight() const = 0;
  virtual int getFormat() const = 0;

  virtual void setMetaVariables(
    const Point& metaOrigin,
    const Region& metaRegion,
    bool useMetaRegion,
    bool reset) = 0;

  virtual void setMetaOrigin(const Point& pt) = 0;
  virtual void setUserOrigin(const Point& pt) = 0;

  virtual Point getMetaOrigin() const = 0;
  virtual Point getUserOrigin() const = 0;

  virtual void translateMetaOrigin(const Point& pt) = 0;
  virtual void translateUserOrigin(const Point& pt) = 0;

  virtual void setUserRegion(const Rect& r) = 0;
  virtual void setUserRegion(const Region& r) = 0;

  virtual Region getMetaRegion() const = 0;
  virtual Region getUserRegion() const = 0;

  virtual bool isMetaRegionUsed() const = 0;
  virtual bool isUserRegionUsed() const = 0;

  virtual void resetMetaVars() = 0;
  virtual void resetUserVars() = 0;

  // [Operator]

  virtual void setOperator(uint32_t op) = 0;
  virtual uint32_t getOperator() const = 0;

  // [Source]

  virtual void setSource(const Rgba& rgba) = 0;
  virtual void setSource(const Pattern& pattern) = 0;

  virtual Rgba sourceRgba() = 0;
  virtual Pattern sourcePattern() = 0;

  // [Parameters]

  virtual void setLineParams(const LineParams& params) = 0;
  virtual void getLineParams(LineParams& params) const = 0;

  virtual void setLineWidth(double lineWidth) = 0;
  virtual double getLineWidth() const = 0;

  virtual void setLineCap(uint32_t lineCap) = 0;
  virtual uint32_t getLineCap() const = 0;

  virtual void setLineJoin(uint32_t lineJoin) = 0;
  virtual uint32_t getLineJoin() const = 0;

  virtual void setDashes(const double* dashes, sysuint_t count) = 0;
  virtual void setDashes(const Vector<double>& dashes) = 0;
  virtual Vector<double> getDashes() const = 0;

  virtual void setDashOffset(double offset) = 0;
  virtual double getDashOffset() const = 0;

  virtual void setMiterLimit(double miterLimit) = 0;
  virtual double getMiterLimit() const = 0;

  virtual void setFillMode(uint32_t mode) = 0;
  virtual uint32_t getFillMode() = 0;

  // [Transformations]

  virtual void setMatrix(const Matrix& m) = 0;
  virtual void resetMatrix() = 0;
  virtual Matrix getMatrix() const = 0;

  virtual void rotate(double angle) = 0;
  virtual void scale(double sx, double sy) = 0;
  virtual void skew(double sx, double sy) = 0;
  virtual void translate(double x, double y) = 0;
  virtual void affine(const Matrix& m) = 0;
  virtual void parallelogram(double x1, double y1, double x2, double y2, const double* para) = 0;
  virtual void viewport(
    double worldX1,  double worldY1,  double worldX2,  double worldY2,
    double screenX1, double screenY1, double screenX2, double screenY2,
    uint32_t viewportOption) = 0;

  virtual void worldToScreen(PointD* pt) const = 0;
  virtual void screenToWorld(PointD* pt) const = 0;

  virtual void worldToScreen(double* scalar) const = 0;
  virtual void screenToWorld(double* scalar) const = 0;

  virtual void alignPoint(PointD* pt) const = 0;

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

  virtual void drawPoint(const PointD& p) = 0;
  virtual void drawLine(const PointD& start, const PointD& end) = 0;
  virtual void drawLine(const PointD* pts, sysuint_t count) = 0;
  virtual void drawPolygon(const PointD* pts, sysuint_t count) = 0;
  virtual void drawRect(const RectD& r) = 0;
  virtual void drawRects(const RectD* r, sysuint_t count) = 0;
  virtual void drawRound(const RectD& r, const PointD& radius) = 0;
  virtual void drawEllipse(const PointD& cp, const PointD& r) = 0;
  virtual void drawArc(const PointD& cp, const PointD& r, double start, double sweep) = 0;
  virtual void drawPath(const Path& path) = 0;

  virtual void fillPolygon(const PointD* pts, sysuint_t count) = 0;
  virtual void fillRect(const RectD& r) = 0;
  virtual void fillRects(const RectD* r, sysuint_t count) = 0;
  virtual void fillRound(const RectD& r, const PointD& radius) = 0;
  virtual void fillEllipse(const PointD& cp, const PointD& r) = 0;
  virtual void fillArc(const PointD& cp, const PointD& r, double start, double sweep) = 0;
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
