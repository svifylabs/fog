// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_PAINTER_H
#define _FOG_GRAPHICS_PAINTER_H

// [Dependencies]
#include <Fog/Graphics/PainterEngine.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Fog::Painter]
// ============================================================================

//! @brief Painter.
//!
//! Painter is high level class that can be used to draw into the images or raw
//! memory buffers. Fog painter is state based and each rendering call depends
//! to current painter state that can be changed by methods like @c setLineWidth(),
//! @c setLineCap(), @c setOperator(), etc...
//!
//! @section Beginning and finalizing rendering.
//!
//! To start painting to an image or raw memory buffer use @c Painter constructor
//! or call @c begin() method with your raw buffer or @c Image instance. After 
//! you called @c begin() method that succeeded (check if return value is 
//! @c Fog::Error::Ok) you can start painting. If you no longer need painter
//! instance delete it (by @c delete operator or static destructor) or call
//! @c end() method.
//!
//! It's very important to delete painter or call @c end() method, because 
//! rendering can be asynchronous.
//!
//! Hint: to turn off multithreaded rendering use @c HintNoMultithreading in
//! @c begin() method.
//!
//! @section Meta and user region.
//!
//! Painter supports two indenpendent region and origin information that can
//! be used to affect painter origin and clipping. First region and origin 
//! informations are stored as meta region and meta origin. These variables
//! shouldn't be changed during rendering and are used usually by windowing
//! system to set correct origin and window clipping. If you are using Fog/UI
//! library, never change these variables in @c Widget::onPaint() event.
//!
//! Second region and origin information are stored as user region and user
//! origin. These variables are designed to be changeable by user during 
//! rendering, so use them if it's useable for you.
//!
//! Origin and region clipping is pixel based and it's not affected by affine
//! transformations (so resetting transformations will not reset user origin).
//!
//! @section Affine transformations.
//!
//! TODO
struct FOG_API Painter 
{
  // [Hints]

  //! @brief Hints that can be set during painter initialization.
  //!
  //! Currently this is mainly useable to turn multithreading off.
  enum Hints
  {
    //! @brief Do not use multithreading.
    HintNoMultithreading = (1 << 31)
  };

  // [Construction / Destruction]

  //! @brief Create null painter instance.
  Painter();

  //! @brief Create painter instance for a given image @a image.
  Painter(Image& image, int hints = 0);

  //! @brief Destroy painter instance.
  //!
  //! If painter is multithreaded, destructor would wait to finish the rendering
  //! (it will call @c end() method).
  ~Painter();

  // [Begin / End]

  err_t begin(uint8_t* pixels, int width, int height, sysint_t stride, int format, int hints = 0);
  err_t begin(Image& image, int hints = 0);
  void end();

  // [Meta]

  //! @brief Get painter width in pixels (width returned is width passed to @c begin() method).
  FOG_INLINE int getWidth() const { return _engine->getWidth(); }
  //! @brief Get painter heigh in pixels (height returned is height passed to @c begin() method).
  FOG_INLINE int getHeight() const { return _engine->getHeight(); }
  //! @brief Get painter format  (format returned is format passed to @c begin() method).
  FOG_INLINE int getFormat() const { return _engine->getFormat(); }

  //! @brief Set painter meta variables (meta origin, meta region, user origin and user region).
  //!
  //! This function were designed as an optimization for windowing systems to
  //! set all important origin and clipping information per one function call.
  FOG_INLINE void setMetaVariables(
    const Point& metaOrigin,
    const Region& metaRegion, 
    bool useMetaRegion,
    bool reset)
  { _engine->setMetaVariables(metaOrigin, metaRegion, useMetaRegion, reset); }

  //! @brief Change meta origin to @a p.
  FOG_INLINE void setMetaOrigin(const Point& p) { _engine->setMetaOrigin(p); }
  //! @brief Change user origin to @a p.
  FOG_INLINE void setUserOrigin(const Point& p) { _engine->setUserOrigin(p); }

  //! @brief Get meta origin.
  FOG_INLINE Point getMetaOrigin() const { return _engine->getMetaOrigin(); }
  //! @brief Get user origin.
  FOG_INLINE Point getUserOrigin() const { return _engine->getUserOrigin(); }

  //! @brief Translate meta origin by @a p.
  FOG_INLINE void translateMetaOrigin(const Point& p) { _engine->translateMetaOrigin(p); }
  //! @brief Translate user origin by @a p.
  FOG_INLINE void translateUserOrigin(const Point& p) { _engine->translateUserOrigin(p); }

  //! @brief Set user region to @a r.
  FOG_INLINE void setUserRegion(const Region& r) { _engine->setUserRegion(r); }
  //! @brief Set user region to @a r.
  //! @overload
  FOG_INLINE void setUserRegion(const Rect& r) { _engine->setUserRegion(r); }

  //! @brief Reset meta variables (meta origin and meta region).
  //!
  //! Resetting means set meta origin to [0, 0] and clear meta region.
  FOG_INLINE void resetMetaVars() { _engine->resetMetaVars(); }
  //! @brief Reset user variables (user origin and user region).
  //!
  //! Resetting means set user origin to [0, 0] and clear user region.
  FOG_INLINE void resetUserVars() { _engine->resetUserVars(); }

  //! @brief Get meta region.
  FOG_INLINE Region getMetaRegion() const { return _engine->getMetaRegion(); }
  //! @brief Get user origin.
  FOG_INLINE Region getUserRegion() const { return _engine->getUserRegion(); }

  //! @brief Tells if current meta region is used (calling @c resetMetaVars() will unuse it).
  FOG_INLINE bool isMetaRegionUsed() const { return _engine->isMetaRegionUsed(); }
  //! @brief Tells if current user region is used (calling @c resetUserVars() will unuse it).
  FOG_INLINE bool isUserRegionUsed() const { return _engine->isUserRegionUsed(); }

  // [Operator]

  //! @brief Set compositing operator.
  //!
  //! See @c CompositeOp enumeration for operators and their descriptions.
  FOG_INLINE void setOperator(uint32_t op) { _engine->setOperator(op); }
  //! @brief Get compositing operator.
  //!
  //! See @c CompositeOp enumeration for operators and their descriptions.
  FOG_INLINE uint32_t getOperator() const { return _engine->getOperator(); }

  // [Source]

  //! @brief Set source as solid @a rgba color.
  FOG_INLINE void setSource(const Rgba& rgba) { _engine->setSource(rgba); }
  //! @brief Set source as pattern @a pattern.
  FOG_INLINE void setSource(const Pattern& pattern) { _engine->setSource(pattern); }

  //! @brief Get source color as @c Rgba instance.
  //!
  //! If current source isn't solid color, the @c Rgba(0x00000000) color is
  //! returned.
  FOG_INLINE Rgba sourceRgba() const { return _engine->sourceRgba(); }
  //! @brief Get source pattern.
  //!
  //! If source color was set through @c setSource(const Rgba& rgba) method,
  //! pattern is auto-created.
  FOG_INLINE Pattern sourcePattern() const { return _engine->sourcePattern(); }

  // [Parameters]

  //! @brief Set line parameters.
  FOG_INLINE void setLineParams(const LineParams& params) { _engine->setLineParams(params); }
  //! @brief Get line parameters.
  FOG_INLINE void getLineParams(LineParams& params) const { return _engine->getLineParams(params); }

  //! @brief Set line width.
  //!
  //! @note Line width is scaled by affine transformations.
  FOG_INLINE void setLineWidth(double lineWidth) { _engine->setLineWidth(lineWidth); }
  //! @brief Get line width.
  FOG_INLINE double getLineWidth() const { return _engine->getLineWidth(); }

  //! @brief Set line cap.
  FOG_INLINE void setLineCap(uint32_t lineCap) { _engine->setLineCap(lineCap);}
  //! @brief Get line cap.
  FOG_INLINE uint32_t getLineCap() const { return _engine->getLineCap(); }

  //! @brief Set line join.
  FOG_INLINE void setLineJoin(uint32_t lineJoin) { _engine->setLineJoin(lineJoin); }
  //! @brief Get line join.
  FOG_INLINE uint32_t getLineJoin() const { return _engine->getLineJoin(); }

  //! @brief Set line dash.
  FOG_INLINE void setDashes(const Vector<double>& dashes) { _engine->setDashes(dashes); }
  //! @overload.
  FOG_INLINE void setDashes(const double* dashes, sysuint_t count) { _engine->setDashes(dashes, count); }
  //! @brief Set line dash.
  FOG_INLINE Vector<double> getDashes() const { return _engine->getDashes(); }

  //! @brief Set line dash offset.
  FOG_INLINE void setDashOffset(double offset) { _engine->setDashOffset(offset); }
  //! @brief Get line dash offset.
  FOG_INLINE double getDashOffset() const { return _engine->getDashOffset(); }

  //! @brief Set line miter limit.
  FOG_INLINE void setMiterLimit(double miterLimit) { _engine->setMiterLimit(miterLimit); }
  //! @brief Get line miter limit.
  FOG_INLINE double getMiterLimit() const { return _engine->getMiterLimit(); }

  //! @brief Set fill mode, see @c FillMode enumeration.
  FOG_INLINE void setFillMode(uint32_t mode) { _engine->setFillMode(mode); }
  //! @brief Get fill mode, see @c FillMode enumeration.
  FOG_INLINE uint32_t getFillMode() { return _engine->getFillMode(); }

  // [Transformations]

  //! @brief Set affine matrix to @a m, discarding current transformations.
  FOG_INLINE void setMatrix(const Matrix& m) { _engine->setMatrix(m); }
  //! @brief Discard current transformations.
  FOG_INLINE void resetMatrix() { _engine->resetMatrix(); }
  //! @brief Get current affine matrix used for transformations.
  FOG_INLINE Matrix getMatrix() const { return _engine->getMatrix(); }

  //! @brief Rotate (will modify affine matrix).
  FOG_INLINE void rotate(double angle) { _engine->rotate(angle); }
  //! @brief Scale (will modify affine matrix).
  FOG_INLINE void scale(double sx, double sy) { _engine->scale(sx, sy); }
  //! @brief Skew (will modify affine matrix).
  FOG_INLINE void skew(double sx, double sy) { _engine->skew(sx, sy); }
  //! @brief Translate (will modify affine matrix).
  FOG_INLINE void translate(double x, double y) { _engine->translate(x, y); }
  //! @brief Affine (will modify affine matrix).
  FOG_INLINE void affine(const Matrix& m) { _engine->affine(m); }

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

  //! @brief Convert world coordinate into screen one (using transformation matrix).
  FOG_INLINE void worldToScreen(PointD* pt) const { _engine->worldToScreen(pt); }
  //! @brief Convert screen coordinate into world one (using transformation matrix).
  FOG_INLINE void screenToWorld(PointD* pt) const { _engine->screenToWorld(pt); }

  //! @brief Convert world scalar into screen one (using transformation matrix).
  FOG_INLINE void worldToScreen(double* scalar) const { _engine->worldToScreen(scalar); }
  //! @brief Convert screen scalar into world one (using transformation matrix).
  FOG_INLINE void screenToWorld(double* scalar) const { _engine->screenToWorld(scalar); }

  //! @brief Align point to center (X.5, Y.5) after applied all transformations.
  FOG_INLINE void alignPoint(PointD* pt) const { _engine->alignPoint(pt); }

  // [State]

  //! @brief Save current painter state.
  FOG_INLINE void save() { _engine->save(); }
  //! @brief Restore current painter state.
  FOG_INLINE void restore() { _engine->restore(); }

  // [Raster Drawing]

  FOG_INLINE void clear() { _engine->clear(); }
  FOG_INLINE void drawPoint(const Point& p) { _engine->drawPoint(p); }
  FOG_INLINE void drawLine(const Point& start, const Point& end) { _engine->drawLine(start, end); }
  FOG_INLINE void drawRect(const Rect& r) { _engine->drawRect(r); }
  FOG_INLINE void drawRound(const Rect& r, const Point& radius) { _engine->drawRound(r, radius); }
  FOG_INLINE void fillRect(const Rect& r) { _engine->fillRect(r); }
  FOG_INLINE void fillRects(const Rect* r, sysuint_t count) { _engine->fillRects(r, count); }
  FOG_INLINE void fillRound(const Rect& r, const Point& radius) { _engine->fillRound(r, radius); }
  FOG_INLINE void fillRegion(const Region& region) { _engine->fillRegion(region); }

  // [Vector Drawing]

  FOG_INLINE void drawPoint(const PointD& p) { _engine->drawPoint(p); }
  FOG_INLINE void drawLine(const PointD& start, const PointD& end) { _engine->drawLine(start, end); }
  FOG_INLINE void drawLine(const PointD* pts, sysuint_t count) { _engine->drawLine(pts, count); }
  FOG_INLINE void drawPolygon(const PointD* pts, sysuint_t count) { _engine->drawPolygon(pts, count); }
  FOG_INLINE void drawRect(const RectD& r) { _engine->drawRect(r); }
  FOG_INLINE void drawRects(const RectD* r, sysuint_t count) { _engine->drawRects(r, count); }
  FOG_INLINE void drawRound(const RectD& r, const PointD& radius) { _engine->drawRound(r, radius); }
  FOG_INLINE void drawEllipse(const PointD& cp, const PointD& r) { _engine->drawEllipse(cp, r); }
  FOG_INLINE void drawArc(const PointD& cp, const PointD& r, double start, double sweep) { _engine->drawArc(cp, r, start, sweep); }
  FOG_INLINE void drawPath(const Path& path) { _engine->drawPath(path); }

  FOG_INLINE void fillPolygon(const PointD* pts, sysuint_t count) { _engine->fillPolygon(pts, count); }
  FOG_INLINE void fillRect(const RectD& r) { _engine->fillRect(r); }
  FOG_INLINE void fillRects(const RectD* r, sysuint_t count) { _engine->fillRects(r, count); }
  FOG_INLINE void fillRound(const RectD& r, const PointD& radius) { _engine->fillRound(r, radius); }
  FOG_INLINE void fillEllipse(const PointD& cp, const PointD& r) { _engine->fillEllipse(cp, r); }
  FOG_INLINE void fillArc(const PointD& cp, const PointD& r, double start, double sweep) { _engine->fillArc(cp, r, start, sweep); }
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
