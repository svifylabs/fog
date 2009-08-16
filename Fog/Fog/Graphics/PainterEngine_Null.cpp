// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled Headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/Static.h>
#include <Fog/Graphics/Painter.h>
#include <Fog/Graphics/PainterEngine_Null.h>

namespace Fog {

// ============================================================================
// [Fog::PainterEngine_Null]
// ============================================================================

struct FOG_HIDDEN PainterEngine_Null : public PainterEngine
{
  PainterEngine_Null() {}
  virtual ~PainterEngine_Null() {}

  // [Meta]

  virtual int getWidth() const { return 0; }
  virtual int getHeight() const { return 0; }
  virtual int getFormat() const { return Image::FormatNull; }

  virtual void setMetaVariables(
    const Point& metaOrigin,
    const Region& metaRegion,
    bool useMetaRegion,
    bool reset) {}

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

  // [Operator]

  virtual void setOperator(uint32_t op) {}
  virtual uint32_t getOperator() const { return 0; }

  // [Source]

  virtual void setSource(const Rgba& rgba) {}
  virtual void setSource(const Pattern& pattern) {}

  virtual Rgba sourceRgba() { return Rgba(0); }
  virtual Pattern sourcePattern() { return Pattern(); }

  // [Parameters]

  virtual void setLineParams(const LineParams& params) {}
  virtual void getLineParams(LineParams& params) const
  {
    params.lineWidth = 1.0;
    params.lineCap = LineCapSquare;
    params.lineJoin = LineJoinMiter;
    params.dashes.free(); 
    params.dashOffset = 0.0;
  }

  virtual void setLineWidth(double lineWidth) {}
  virtual double getLineWidth() const { return 0.0; }

  virtual void setLineCap(uint32_t lineCap) {}
  virtual uint32_t getLineCap() const { return 0; }

  virtual void setLineJoin(uint32_t lineJoin) {}
  virtual uint32_t getLineJoin() const { return 0; }

  virtual void setDashes(const double* dashes, sysuint_t count) {}
  virtual void setDashes(const Vector<double>& dashes) {}
  virtual Vector<double> getDashes() const { return Vector<double>(); }

  virtual void setDashOffset(double offset) {}
  virtual double getDashOffset() const { return 0.0; }

  virtual void setMiterLimit(double miterLimit) {}
  virtual double getMiterLimit() const { return 0.0; }

  virtual void setFillMode(uint32_t mode) {}
  virtual uint32_t getFillMode() { return FillNonZero; }

  // [Transformations]

  virtual void setMatrix(const Matrix& m) {}
  virtual void resetMatrix() {}
  virtual Matrix getMatrix() const { return Matrix(); }

  virtual void rotate(double angle) {}
  virtual void scale(double sx, double sy) {}
  virtual void skew(double sx, double sy) {}
  virtual void translate(double x, double y) {}
  virtual void affine(const Matrix& m) {}
  virtual void parallelogram(double x1, double y1, double x2, double y2, const double* para) {}
  virtual void viewport(
    double worldX1,  double worldY1,  double worldX2,  double worldY2,
    double screenX1, double screenY1, double screenX2, double screenY2,
    uint32_t viewportOption) {}

  virtual void worldToScreen(PointF* pt) const {}
  virtual void screenToWorld(PointF* pt) const {}

  virtual void worldToScreen(double* scalar) const {}
  virtual void screenToWorld(double* scalar) const {}

  virtual void alignPoint(PointF* pt) const {}

  // [State]

  virtual void save() {}
  virtual void restore() {}

  // [Raster Drawing]

  virtual void clear() {}
  virtual void drawPoint(const Point& p) {}
  virtual void drawLine(const Point& start, const Point& end) {}
  virtual void drawRect(const Rect& r) {}
  virtual void drawRound(const Rect& r, const Point& radius) {}
  virtual void fillRect(const Rect& r) {}
  virtual void fillRects(const Rect* r, sysuint_t count) {}
  virtual void fillRound(const Rect& r, const Point& radius) {}
  virtual void fillRegion(const Region& region) {}

  // [Vector Drawing]

  virtual void drawPoint(const PointF& p) {}
  virtual void drawLine(const PointF& start, const PointF& end) {}
  virtual void drawLine(const PointF* pts, sysuint_t count) {}
  virtual void drawPolygon(const PointF* pts, sysuint_t count) {}
  virtual void drawRect(const RectF& r) {}
  virtual void drawRects(const RectF* r, sysuint_t count) {}
  virtual void drawRound(const RectF& r, const PointF& radius) {}
  virtual void drawEllipse(const PointF& cp, const PointF& r) {}
  virtual void drawArc(const PointF& cp, const PointF& r, double start, double sweep) {}
  virtual void drawPath(const Path& path) {}

  virtual void fillPolygon(const PointF* pts, sysuint_t count) {}
  virtual void fillRect(const RectF& r) {}
  virtual void fillRects(const RectF* r, sysuint_t count) {}
  virtual void fillRound(const RectF& r, const PointF& radius) {}
  virtual void fillEllipse(const PointF& cp, const PointF& r) {}
  virtual void fillArc(const PointF& cp, const PointF& r, double start, double sweep) {}
  virtual void fillPath(const Path& path) {}

  // [Glyph / Text Drawing]

  virtual void drawGlyph(const Point& pt, const Glyph& glyph, const Rect* clip) {}
  virtual void drawGlyphSet(const Point& pt, const GlyphSet& glyphSet, const Rect* clip) {}

  virtual void drawText(const Point& p, const String32& text, const Font& font, const Rect* clip) {}
  virtual void drawText(const Rect& r, const String32& text, const Font& font, uint32_t align, const Rect* clip) {}

  // [Image drawing]

  virtual void drawImage(const Point& p, const Image& image, const Rect* irect) {}

  // [Flush]

  virtual void flush() {}

  // [Properties]

  virtual err_t setProperty(const String32& name, const Value& value) { return Error::InvalidPropertyName; }
  virtual Value getProperty(const String32& name) const { return Value(); }
};

static Static<PainterEngine_Null> _nullPainterEngine;

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_painter_null_init(void)
{
  using namespace Fog;

  _nullPainterEngine.init();
  Painter::sharedNull = _nullPainterEngine.instancep();

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_painter_null_shutdown(void)
{
  using namespace Fog;

  _nullPainterEngine.destroy();
  Painter::sharedNull = NULL;
}
