// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

namespace Fog {

// ============================================================================
// [Fog::NullPainterDevice]
// ============================================================================

//! @brief Painter device.
struct FOG_HIDDEN NullPainterDevice : public PainterDevice
{
  NullPainterDevice() {}
  virtual ~NullPainterDevice() {}

  // [Meta]

  virtual int width() const { return 0; }
  virtual int height() const { return 0; }
  virtual int format() const { return Image::FormatNull; }

  virtual void setMetaVariables(
    const Point& metaOrigin,
    const Region& metaRegion,
    bool useMetaRegion,
    bool reset) {}

  virtual void setMetaOrigin(const Point& pt) {}
  virtual void setUserOrigin(const Point& pt) {}

  virtual void translateMetaOrigin(const Point& pt) {}
  virtual void translateUserOrigin(const Point& pt) {}

  virtual void setUserRegion(const Rect& r) {}
  virtual void setUserRegion(const Region& r) {}

  virtual void resetMetaVars() {}
  virtual void resetUserVars() {}

  virtual Point metaOrigin() const { return Point(0, 0); }
  virtual Point userOrigin() const { return Point(0, 0); }

  virtual Region metaRegion() const { return Region(); }
  virtual Region userRegion() const { return Region(); }

  virtual bool usedMetaRegion() const { return false; }
  virtual bool usedUserRegion() const { return false; }

  // [Operator]

  virtual void setOp(uint32_t op) {}
  virtual uint32_t op() const { return 0; }

  // [Source]

  virtual void setSource(const Rgba& rgba) {}
  virtual void setSource(const Pattern& pattern) {}

  virtual Rgba sourceRgba() { return Rgba(0); }
  virtual Pattern sourcePattern() { return Pattern(); }

  // [Parameters]

  virtual void setLineWidth(double lineWidth) {}
  virtual double lineWidth() const { return 0.0; }

  virtual void setLineCap(uint32_t lineCap) {}
  virtual uint32_t lineCap() const { return 0; }

  virtual void setLineJoin(uint32_t lineJoin) {}
  virtual uint32_t lineJoin() const { return 0; }

  virtual void setLineDash(const double* dashes, sysuint_t count) {}
  virtual void setLineDash(const Vector<double>& dashes) {}
  virtual Vector<double> lineDash() const { return Vector<double>(); }

  virtual void setLineDashOffset(double offset) {}
  virtual double lineDashOffset() const { return 0.0; }

  virtual void setMiterLimit(double miterLimit) {}
  virtual double miterLimit() const { return 0.0; }

  virtual void setFillMode(uint32_t mode) {}
  virtual uint32_t fillMode() { return FillNonZero; }

  // [Transformations]

  virtual void setMatrix(const AffineMatrix& m) {}
  virtual void resetMatrix() {}
  virtual AffineMatrix matrix() const { return AffineMatrix(); }

  virtual void rotate(double angle) {}
  virtual void scale(double sx, double sy) {}
  virtual void skew(double sx, double sy) {}
  virtual void translate(double x, double y) {}
  virtual void affine(const AffineMatrix& m) {}
  virtual void parallelogram(double x1, double y1, double x2, double y2, const double* para) {}
  virtual void viewport(
    double worldX1,  double worldY1,  double worldX2,  double worldY2,
    double screenX1, double screenY1, double screenX2, double screenY2,
    uint32_t viewportOption) {}

  // [Raster Drawing]

  virtual void clear() {}
  virtual void drawPixel(const Point& p) {}
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
  virtual void drawRound(const RectF& r, 
    const PointF& tlr, const PointF& trr,
    const PointF& blr, const PointF& brr) {}
  virtual void drawEllipse(const PointF& cp, const PointF& r) {}
  virtual void drawArc(const PointF& cp, const PointF& r, double start, double sweep) {}
  virtual void drawPath(const Path& path) {}

  virtual void fillPolygon(const PointF* pts, sysuint_t count) {}
  virtual void fillRect(const RectF& r) {}
  virtual void fillRects(const RectF* r, sysuint_t count) {}
  virtual void fillRound(const RectF& r, const PointF& radius) {}
  virtual void fillRound(const RectF& r,
    const PointF& tlr, const PointF& trr,
    const PointF& blr, const PointF& brr) {}
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
};

} // Fog namespace
