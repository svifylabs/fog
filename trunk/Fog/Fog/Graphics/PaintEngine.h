// [Fog/Graphics Library - Public API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_PAINTERENGINE_H
#define _FOG_GRAPHICS_PAINTERENGINE_H

// [Dependencies]
#include <Fog/Core/List.h>
#include <Fog/Core/String.h>
#include <Fog/Graphics/Argb.h>
#include <Fog/Graphics/Matrix.h>
#include <Fog/Graphics/Font.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/Path.h>
#include <Fog/Graphics/Pattern.h>
#include <Fog/Graphics/Region.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct ColorFilter;
struct StrokeParams;
struct ImageFilter;

// ============================================================================
// [Fog::PaintEngine]
// ============================================================================

//! @brief Painter engine is @c Painter backend.
//!
//! See @c Painter all virtual members documentation.
struct FOG_API PaintEngine
{
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  PaintEngine();
  virtual ~PaintEngine() = 0;

  // --------------------------------------------------------------------------
  // [Width / Height / Format]
  // --------------------------------------------------------------------------

  virtual int getWidth() const = 0;
  virtual int getHeight() const = 0;
  virtual int getFormat() const = 0;

  // --------------------------------------------------------------------------
  // [Engine / Flush]
  // --------------------------------------------------------------------------

  virtual uint32_t getEngine() const = 0;
  virtual void setEngine(uint32_t engine, uint32_t cores = 0) = 0;

  virtual void flush(uint32_t flags) = 0;

  // --------------------------------------------------------------------------
  // [Hints]
  // --------------------------------------------------------------------------

  virtual int getHint(uint32_t hint) const = 0;
  virtual void setHint(uint32_t hint, int value) = 0;

  // --------------------------------------------------------------------------
  // [Meta]
  // --------------------------------------------------------------------------

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

  // --------------------------------------------------------------------------
  // [Operator]
  // --------------------------------------------------------------------------

  virtual uint32_t getOperator() const = 0;
  virtual void setOperator(uint32_t op) = 0;

  // --------------------------------------------------------------------------
  // [Source]
  // --------------------------------------------------------------------------

  virtual uint32_t getSourceType() const = 0;

  virtual err_t getSourceArgb(Argb& argb) const = 0;
  virtual err_t getSourcePattern(Pattern& pattern) const = 0;

  virtual void setSource(Argb argb) = 0;
  virtual void setSource(const Pattern& pattern) = 0;
  virtual void setSource(const ColorFilter& colorFilter) = 0;

  // --------------------------------------------------------------------------
  // [Fill Parameters]
  // --------------------------------------------------------------------------

  virtual uint32_t getFillMode() const = 0;
  virtual void setFillMode(uint32_t mode) = 0;

  // --------------------------------------------------------------------------
  // [Stroke Parameters]
  // --------------------------------------------------------------------------

  virtual void getStrokeParams(StrokeParams& strokeParams) const = 0;
  virtual void setStrokeParams(const StrokeParams& strokeParams) = 0;

  virtual double getLineWidth() const = 0;
  virtual void setLineWidth(double lineWidth) = 0;

  virtual uint32_t getStartCap() const = 0;
  virtual void setStartCap(uint32_t startCap) = 0;

  virtual uint32_t getEndCap() const = 0;
  virtual void setEndCap(uint32_t endCap) = 0;

  virtual void setLineCaps(uint32_t lineCap) = 0;

  virtual uint32_t getLineJoin() const = 0;
  virtual void setLineJoin(uint32_t lineJoin) = 0;

  virtual double getMiterLimit() const = 0;
  virtual void setMiterLimit(double miterLimit) = 0;

  virtual List<double> getDashes() const = 0;
  virtual void setDashes(const double* dashes, sysuint_t count) = 0;
  virtual void setDashes(const List<double>& dashes) = 0;

  virtual double getDashOffset() const = 0;
  virtual void setDashOffset(double offset) = 0;

  // --------------------------------------------------------------------------
  // [Transformations]
  // --------------------------------------------------------------------------

  virtual Matrix getMatrix() const = 0;
  virtual void setMatrix(const Matrix& m) = 0;
  virtual void resetMatrix() = 0;

  virtual void rotate(double angle, int order) = 0;
  virtual void scale(double sx, double sy, int order) = 0;
  virtual void skew(double sx, double sy, int order) = 0;
  virtual void translate(double x, double y, int order) = 0;
  virtual void transform(const Matrix& m, int order) = 0;

  virtual void worldToScreen(PointD* pt) const = 0;
  virtual void screenToWorld(PointD* pt) const = 0;

  virtual void alignPoint(PointD* pt) const = 0;

  // --------------------------------------------------------------------------
  // [State]
  // --------------------------------------------------------------------------

  virtual void save() = 0;
  virtual void restore() = 0;

  // --------------------------------------------------------------------------
  // [Clear]
  // --------------------------------------------------------------------------

  virtual void clear() = 0;

  // --------------------------------------------------------------------------
  // [Raster Drawing]
  // --------------------------------------------------------------------------

  virtual void drawPoint(const Point& p) = 0;
  virtual void drawLine(const Point& start, const Point& end) = 0;
  virtual void drawRect(const Rect& r) = 0;
  virtual void drawRound(const Rect& r, const Point& radius) = 0;
  virtual void fillRect(const Rect& r) = 0;
  virtual void fillRects(const Rect* r, sysuint_t count) = 0;
  virtual void fillRound(const Rect& r, const Point& radius) = 0;
  virtual void fillRegion(const Region& region) = 0;

  // --------------------------------------------------------------------------
  // [Vector Drawing]
  // --------------------------------------------------------------------------

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

  // --------------------------------------------------------------------------
  // [Glyph / Text Drawing]
  // --------------------------------------------------------------------------

  virtual void drawGlyph(const Point& pt, const Glyph& glyph, const Rect* clip) = 0;
  virtual void drawGlyphSet(const Point& pt, const GlyphSet& glyphSet, const Rect* clip) = 0;

  virtual void drawText(const Point& p, const String& text, const Font& font, const Rect* clip) = 0;
  virtual void drawText(const Rect& r, const String& text, const Font& font, uint32_t align, const Rect* clip) = 0;

  // --------------------------------------------------------------------------
  // [Image Drawing]
  // --------------------------------------------------------------------------

  virtual void blitImage(const Point& p, const Image& image, const Rect* irect) = 0;
  virtual void blitImage(const PointD& p, const Image& image, const Rect* irect) = 0;

private:
  FOG_DISABLE_COPY(PaintEngine)
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_PAINTERENGINE_H
