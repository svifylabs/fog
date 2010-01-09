// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Static.h>
#include <Fog/Graphics/Painter.h>
#include <Fog/Graphics/Stroker.h>

#include <Fog/Graphics/PaintEngine/Null.h>

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

  virtual int getOperator() const { return COMPOSITE_SRC_OVER; }
  virtual void setOperator(int op) {}

  // --------------------------------------------------------------------------
  // [Source]
  // --------------------------------------------------------------------------

  virtual int getSourceType() const { return PAINTER_SOURCE_ARGB; }
  virtual err_t getSourceArgb(Argb& argb) const { argb.set(0x00000000); return ERR_RT_INVALID_CONTEXT; }
  virtual err_t getSourcePattern(Pattern& pattern) const { pattern.reset(); return ERR_RT_INVALID_CONTEXT; }

  virtual void setSource(Argb argb) {}
  virtual void setSource(const Pattern& pattern) {}
  virtual void setSource(const ColorFilter& colorFilter) {}

  // --------------------------------------------------------------------------
  // [Hints]
  // --------------------------------------------------------------------------

  virtual int getHint(int hint) const { return -1; }
  virtual void setHint(int hint, int value) {}

  // --------------------------------------------------------------------------
  // [Fill Parameters]
  // --------------------------------------------------------------------------

  virtual int getFillMode() const { return FILL_DEFAULT; }
  virtual void setFillMode(int mode) {}

  // --------------------------------------------------------------------------
  // [Stroke Parameters]
  // --------------------------------------------------------------------------

  virtual void getStrokeParams(StrokeParams& strokeParams) const { strokeParams.reset(); }
  virtual void setStrokeParams(const StrokeParams& strokeParams) {}

  virtual double getLineWidth() const { return 0.0; }
  virtual void setLineWidth(double lineWidth) {}

  virtual int getStartCap() const { return 0; }
  virtual void setStartCap(int startCap) {}

  virtual int getEndCap() const { return 0; }
  virtual void setEndCap(int endCap) {}

  virtual void setLineCaps(int lineCaps) {}

  virtual int getLineJoin() const { return 0; }
  virtual void setLineJoin(int lineJoin) {}

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
  // [Raster Drawing]
  // --------------------------------------------------------------------------

  virtual void clear() {}
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

  // --------------------------------------------------------------------------
  // [Multithreading]
  // --------------------------------------------------------------------------

  virtual int getEngine() const { return PAINTER_ENGINE_NULL; }
  virtual void setEngine(int engine, int cores = 0) {}

  virtual void flush() {}
};

static Static<NullPaintEngine> _nullPaintEngine;

PaintEngine* _getNullPaintEngine()
{
  return _nullPaintEngine.instancep();
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_painter_null_init(void)
{
  using namespace Fog;

  _nullPaintEngine.init();
  Painter::sharedNull = _nullPaintEngine.instancep();

  return ERR_OK;
}

FOG_INIT_DECLARE void fog_painter_null_shutdown(void)
{
  using namespace Fog;

  _nullPaintEngine.destroy();
  Painter::sharedNull = NULL;
}
