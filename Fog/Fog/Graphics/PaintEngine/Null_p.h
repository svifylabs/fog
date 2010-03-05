// [Fog-Graphics Library - Private API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_PAINTERENGINE_NULL_P_H
#define _FOG_GRAPHICS_PAINTERENGINE_NULL_P_H

// [Dependencies]
#include <Fog/Graphics/PaintEngine.h>
#include <Fog/Graphics/Stroker.h>

//! @addtogroup Fog_Graphics_Private
//! @{

namespace Fog {

// ============================================================================
// [Fog::NullPaintEngine]
// ============================================================================

struct FOG_HIDDEN NullPaintEngine : public PaintEngine
{
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  NullPaintEngine() {}
  virtual ~NullPaintEngine() {}

  // --------------------------------------------------------------------------
  // [Width / Height / Format]
  // --------------------------------------------------------------------------

  virtual int getWidth() const { return 0; }
  virtual int getHeight() const { return 0; }
  virtual int getFormat() const { return PIXEL_FORMAT_NULL; }

  // --------------------------------------------------------------------------
  // [Engine / Flush]
  // --------------------------------------------------------------------------

  virtual uint32_t getEngine() const { return PAINTER_ENGINE_NULL; }
  virtual void setEngine(uint32_t engine, uint32_t cores = 0) {}

  virtual void flush(uint32_t flags) {}

  // --------------------------------------------------------------------------
  // [Hints]
  // --------------------------------------------------------------------------

  virtual int getHint(uint32_t hint) const { return -1; }
  virtual void setHint(uint32_t hint, int value) {}

  // --------------------------------------------------------------------------
  // [Meta]
  // --------------------------------------------------------------------------

  virtual void setMetaVariables(const Point& metaOrigin, const Region& metaRegion, bool useMetaRegion, bool reset) {}

  virtual void setMetaOrigin(const Point& pt) {}
  virtual void setUserOrigin(const Point& pt) {}

  virtual Point getMetaOrigin() const { return Point(0, 0); }
  virtual Point getUserOrigin() const { return Point(0, 0); }

  virtual void translateMetaOrigin(const Point& pt) {}
  virtual void translateUserOrigin(const Point& pt) {}

  virtual void setUserRegion(const Rect& r) {}
  virtual void setUserRegion(const Region& r) {}

  virtual Region getMetaRegion() const { return Region(); }
  virtual Region getUserRegion() const { return Region(); }

  virtual bool isMetaRegionUsed() const { return false; }
  virtual bool isUserRegionUsed() const { return false; }

  virtual void resetMetaVars() {}
  virtual void resetUserVars() {}

  // --------------------------------------------------------------------------
  // [Operator]
  // --------------------------------------------------------------------------

  virtual uint32_t getOperator() const { return OPERATOR_SRC_OVER; }
  virtual void setOperator(uint32_t op) {}

  // --------------------------------------------------------------------------
  // [Source]
  // --------------------------------------------------------------------------

  virtual uint32_t getSourceType() const { return PAINTER_SOURCE_ARGB; }
  virtual err_t getSourceArgb(Argb& argb) const { argb.set(0x00000000); return ERR_RT_INVALID_CONTEXT; }
  virtual err_t getSourcePattern(Pattern& pattern) const { pattern.reset(); return ERR_RT_INVALID_CONTEXT; }

  virtual void setSource(Argb argb) {}
  virtual void setSource(const Pattern& pattern) {}
  virtual void setSource(const ColorFilter& colorFilter) {}

  // --------------------------------------------------------------------------
  // [Fill Parameters]
  // --------------------------------------------------------------------------

  virtual uint32_t getFillMode() const { return FILL_DEFAULT; }
  virtual void setFillMode(uint32_t mode) {}

  // --------------------------------------------------------------------------
  // [Stroke Parameters]
  // --------------------------------------------------------------------------

  virtual void getStrokeParams(StrokeParams& strokeParams) const { strokeParams.reset(); }
  virtual void setStrokeParams(const StrokeParams& strokeParams) {}

  virtual double getLineWidth() const { return 0.0; }
  virtual void setLineWidth(double lineWidth) {}

  virtual uint32_t getStartCap() const { return 0; }
  virtual void setStartCap(uint32_t startCap) {}

  virtual uint32_t getEndCap() const { return 0; }
  virtual void setEndCap(uint32_t endCap) {}

  virtual void setLineCaps(uint32_t lineCaps) {}

  virtual uint32_t getLineJoin() const { return 0; }
  virtual void setLineJoin(uint32_t lineJoin) {}

  virtual double getMiterLimit() const { return 0.0; }
  virtual void setMiterLimit(double miterLimit) {}

  virtual List<double> getDashes() const { return List<double>(); }
  virtual void setDashes(const double* dashes, sysuint_t count) {}
  virtual void setDashes(const List<double>& dashes) {}

  virtual double getDashOffset() const { return 0.0; }
  virtual void setDashOffset(double offset) {}

  // --------------------------------------------------------------------------
  // [Transformations]
  // --------------------------------------------------------------------------

  virtual Matrix getMatrix() const { return Matrix(); }
  virtual void setMatrix(const Matrix& m) {}
  virtual void resetMatrix() {}

  virtual void rotate(double angle, int order) {}
  virtual void scale(double sx, double sy, int order) {}
  virtual void skew(double sx, double sy, int order) {}
  virtual void translate(double x, double y, int order) {}
  virtual void transform(const Matrix& m, int order) {}

  virtual void worldToScreen(PointD* pt) const {}
  virtual void screenToWorld(PointD* pt) const {}

  virtual void alignPoint(PointD* pt) const {}

  // --------------------------------------------------------------------------
  // [State]
  // --------------------------------------------------------------------------

  virtual void save() {}
  virtual void restore() {}

  // --------------------------------------------------------------------------
  // [Clear]
  // --------------------------------------------------------------------------

  virtual void clear() {}

  // --------------------------------------------------------------------------
  // [Raster Drawing]
  // --------------------------------------------------------------------------

  virtual void drawPoint(const Point& p) {}
  virtual void drawLine(const Point& start, const Point& end) {}
  virtual void drawRect(const Rect& r) {}
  virtual void drawRound(const Rect& r, const Point& radius) {}
  virtual void fillRect(const Rect& r) {}
  virtual void fillRects(const Rect* r, sysuint_t count) {}
  virtual void fillRound(const Rect& r, const Point& radius) {}
  virtual void fillRegion(const Region& region) {}

  // --------------------------------------------------------------------------
  // [Vector Drawing]
  // --------------------------------------------------------------------------

  virtual void drawPoint(const PointD& p) {}
  virtual void drawLine(const PointD& start, const PointD& end) {}
  virtual void drawLine(const PointD* pts, sysuint_t count) {}
  virtual void drawPolygon(const PointD* pts, sysuint_t count) {}
  virtual void drawRect(const RectD& r) {}
  virtual void drawRects(const RectD* r, sysuint_t count) {}
  virtual void drawRound(const RectD& r, const PointD& radius) {}
  virtual void drawEllipse(const PointD& cp, const PointD& r) {}
  virtual void drawArc(const PointD& cp, const PointD& r, double start, double sweep) {}
  virtual void drawPath(const Path& path) {}

  virtual void fillPolygon(const PointD* pts, sysuint_t count) {}
  virtual void fillRect(const RectD& r) {}
  virtual void fillRects(const RectD* r, sysuint_t count) {}
  virtual void fillRound(const RectD& r, const PointD& radius) {}
  virtual void fillEllipse(const PointD& cp, const PointD& r) {}
  virtual void fillArc(const PointD& cp, const PointD& r, double start, double sweep) {}
  virtual void fillPath(const Path& path) {}

  // --------------------------------------------------------------------------
  // [Glyph / Text Drawing]
  // --------------------------------------------------------------------------

  virtual void drawGlyph(const Point& pt, const Glyph& glyph, const Rect* clip) {}
  virtual void drawGlyphSet(const Point& pt, const GlyphSet& glyphSet, const Rect* clip) {}

  virtual void drawText(const Point& p, const String& text, const Font& font, const Rect* clip) {}
  virtual void drawText(const Rect& r, const String& text, const Font& font, uint32_t align, const Rect* clip) {}

  // --------------------------------------------------------------------------
  // [Image drawing]
  // --------------------------------------------------------------------------

  virtual void blitImage(const Point& p, const Image& image, const Rect* irect) {}
  virtual void blitImage(const PointD& p, const Image& image, const Rect* irect) {}
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_PAINTERENGINE_NULL_P_H
