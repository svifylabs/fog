// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/Error.h>
#include <Fog/Core/Math.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/Thread.h>
#include <Fog/Cpu/CpuInfo.h>
#include <Fog/Graphics/AffineMatrix.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Font.h>
#include <Fog/Graphics/Glyph.h>
#include <Fog/Graphics/GlyphSet.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/Painter.h>
#include <Fog/Graphics/Raster.h>
#include <Fog/Graphics/Rgba.h>

// [AntiGrain]
#include "agg_alpha_mask_u8.h"
#include "agg_basics.h"
#include "agg_path_storage.h"
#include "agg_conv_contour.h"
#include "agg_conv_curve.h"
#include "agg_conv_dash.h"
#include "agg_conv_stroke.h"
#include "agg_conv_smooth_poly1.h"
#include "agg_conv_transform.h"
#include "agg_pixfmt_rgb.h"
#include "agg_pixfmt_rgba.h"
#include "agg_pixfmt_amask_adaptor.h"
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_rasterizer_outline.h"
#include "agg_renderer_base.h"
#include "agg_renderer_scanline.h"
#include "agg_renderer_primitives.h"
#include "agg_rounded_rect.h"
#include "agg_scanline_p.h"
#include "agg_scanline_u.h"
#include "agg_scanline_bin.h"
#include "agg_trans_affine.h"
#include "agg_trans_viewport.h"
#include "agg_vcgen_dash.h"

// [BlitJit]

#include <BlitJit/BlitJit.h>

namespace Fog {

// ============================================================================
// [Fog::PainterLocal]
// ============================================================================

typedef BlitJit::FillSpanFn FillSpan;
typedef BlitJit::FillSpanWithMaskFn FillSpanM;

typedef BlitJit::BlitSpanFn BlitSpan;
typedef BlitJit::BlitSpanWithMaskFn BlitSpanM;

struct FillFuncs
{
  FillSpan  fillSpan;
  FillSpanM fillSpanM_A8;
  FillSpanM fillSpanM_RGB32;
};

struct FOG_HIDDEN PainterLocal
{
  PainterLocal() : _init(false)
  {
    BlitJit::Api::init();
  }

  ~PainterLocal()
  {
  }

  void getFillFuncs(FillFuncs* dst)
  {
    if (!_init) createFillFuncs();
    memcpy(dst, &_fillFuncs, sizeof(FillFuncs));
  }

  void createFillFuncs()
  {
    _fillFuncs.fillSpan = BlitJit::Api::genFillSpan(
      &BlitJit::Api::pixelFormats[BlitJit::PixelFormat::PRGB32],
      &BlitJit::Api::pixelFormats[BlitJit::PixelFormat::PRGB32],
      &BlitJit::Api::operators[BlitJit::Operator::CompositeOver]);

    _fillFuncs.fillSpanM_A8 = BlitJit::Api::genFillSpanWithMask(
      &BlitJit::Api::pixelFormats[BlitJit::PixelFormat::PRGB32],
      &BlitJit::Api::pixelFormats[BlitJit::PixelFormat::PRGB32],
      &BlitJit::Api::pixelFormats[BlitJit::PixelFormat::A8],
      &BlitJit::Api::operators[BlitJit::Operator::CompositeOver]);

    _fillFuncs.fillSpanM_RGB32 = NULL;

    _init = true;
  }

  bool _init;
  FillFuncs _fillFuncs;
};

static Static<PainterLocal> painter_local;

// ============================================================================
// [Fog::PainterDevice]
// ============================================================================

PainterDevice::PainterDevice() {}
PainterDevice::~PainterDevice() {}

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
  virtual ImageFormat format() const { return ImageFormat(); }

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
  //virtual void setSource(const Pattern& pattern) {}

  virtual Rgba sourceRgba() { return Rgba(0); }
  //virtual Pattern sourcePattern() {}

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
};

// ============================================================================
// [Fog::RasterPainterDevice - AntiGrain Wrappers]
//
// There are templates and classes that helps to wrap fog containers and paths
// to AntiGrain without converting them into antigrain storage containers. This
// way should improve performance to render larger images and complex paths, 
// but it also allows us to not depend to antigrain directly, but only 
// internally.
// ============================================================================

// Wraps Fog::Path to antigrain like vertex storage.
struct FOG_HIDDEN AggPath
{
  FOG_INLINE AggPath(const Path& path)
  {
    d = path._d;
    rewind(0);
  }

  FOG_INLINE ~AggPath() 
  {
  }

  FOG_INLINE void rewind(unsigned index)
  {
    vCur = d->data + index;
    vEnd = d->data + d->length;
  }

  FOG_INLINE unsigned vertex(double* x, double* y)
  {
    if (vCur == vEnd) return Path::CmdStop;

    *x = vCur->x;
    *y = vCur->y;

    uint command = vCur->cmd.cmd();
    vCur++;
    return command;
  }

private:
  const Path::Data* d;
  const Path::Vertex* vCur;
  const Path::Vertex* vEnd;
};

template<typename VertexStorage>
static err_t concatToPath(Path& dst, VertexStorage& src, unsigned path_id = 0)
{
  sysuint_t i, len = dst.length();
  sysuint_t step = 1024;

  Path::Vertex* v;
  err_t err;

  src.rewind(path_id);

  for (;;)
  {
    if ( (err = dst.reserve(len + step)) ) return err;
    v = dst._d->data + len;

    // Concat vertexes.
    for (i = step; i; i--, v++)
    {
      if ((v->cmd._cmd = src.vertex(&v->x, &v->y)) == Path::CmdStop) 
        goto done;
    }

    // If we are here it's needed to alloc more memory (really big path).
    len += step;
    dst._d->length = len;

    // Double step until we reach 1MB.
    if (step < 1024*1024) step <<= 1;
  }

done:
  dst._d->length = (sysuint_t)(v - dst._d->data);
  return Error::Ok;
}

// ============================================================================
// [Fog::RasterPainterDevice]
// ============================================================================

struct FOG_HIDDEN RasterPainterDevice : public PainterDevice
{
  // [Construction / Destruction]

  RasterPainterDevice(uint8_t* pixels, int width, int height, sysint_t stride, const ImageFormat& format);
  virtual ~RasterPainterDevice();

  // [Meta]

  virtual int width() const;
  virtual int height() const;
  virtual ImageFormat format() const;

  virtual void setMetaVariables(
    const Point& metaOrigin,
    const Region& metaRegion,
    bool useMetaRegion,
    bool reset);

  virtual void setMetaOrigin(const Point& pt);
  virtual void setUserOrigin(const Point& pt);

  virtual void translateMetaOrigin(const Point& pt);
  virtual void translateUserOrigin(const Point& pt);

  virtual void setUserRegion(const Rect& r);
  virtual void setUserRegion(const Region& r);

  virtual void resetMetaVars();
  virtual void resetUserVars();

  virtual Point metaOrigin() const;
  virtual Point userOrigin() const;

  virtual Region metaRegion() const;
  virtual Region userRegion() const;

  virtual bool usedMetaRegion() const;
  virtual bool usedUserRegion() const;

  // [Operator]

  virtual void setOp(uint32_t op);
  virtual uint32_t op() const;

  // [Source]

  virtual void setSource(const Rgba& rgba);
  //virtual void setSource(const Pattern& pattern);

  virtual Rgba sourceRgba();
  //virtual Pattern sourcePattern();

  // [Parameters]

  virtual void setLineWidth(double lineWidth);
  virtual double lineWidth() const;

  virtual void setLineCap(uint32_t lineCap);
  virtual uint32_t lineCap() const;

  virtual void setLineJoin(uint32_t lineJoin);
  virtual uint32_t lineJoin() const;

  virtual void setLineDash(const double* dashes, sysuint_t count);
  virtual void setLineDash(const Vector<double>& dashes);
  virtual Vector<double> lineDash() const;

  virtual void setLineDashOffset(double offset);
  virtual double lineDashOffset() const;

  virtual void setMiterLimit(double miterLimit);
  virtual double miterLimit() const;

  virtual void setFillMode(uint32_t mode);
  virtual uint32_t fillMode();

  // [Transformations]

  virtual void setMatrix(const AffineMatrix& m);
  virtual void resetMatrix();
  virtual AffineMatrix matrix() const;

  virtual void rotate(double angle);
  virtual void scale(double sx, double sy);
  virtual void skew(double sx, double sy);
  virtual void translate(double x, double y);
  virtual void affine(const AffineMatrix& m);
  virtual void parallelogram(double x1, double y1, double x2, double y2, const double* para);
  virtual void viewport(
    double worldX1,  double worldY1,  double worldX2,  double worldY2,
    double screenX1, double screenY1, double screenX2, double screenY2,
    uint32_t viewportOption);

  // [Raster Drawing]

  virtual void clear();
  virtual void drawPixel(const Point& p);
  virtual void drawLine(const Point& start, const Point& end);
  virtual void drawRect(const Rect& r);
  virtual void drawRound(const Rect& r, const Point& radius);
  virtual void fillRect(const Rect& r);
  virtual void fillRects(const Rect* r, sysuint_t count);
  virtual void fillRound(const Rect& r, const Point& radius);
  virtual void fillRegion(const Region& region);

  // [Vector Drawing]

  virtual void drawPoint(const PointF& p);
  virtual void drawLine(const PointF& start, const PointF& end);
  virtual void drawLine(const PointF* pts, sysuint_t count);
  virtual void drawPolygon(const PointF* pts, sysuint_t count);
  virtual void drawRect(const RectF& r);
  virtual void drawRects(const RectF* r, sysuint_t count);
  virtual void drawRound(const RectF& r, const PointF& radius);
  virtual void drawRound(const RectF& r, 
    const PointF& tlr, const PointF& trr,
    const PointF& blr, const PointF& brr);
  virtual void drawEllipse(const PointF& cp, const PointF& r);
  virtual void drawArc(const PointF& cp, const PointF& r, double start, double sweep);
  virtual void drawPath(const Path& path);

  virtual void fillPolygon(const PointF* pts, sysuint_t count);
  virtual void fillRect(const RectF& r);
  virtual void fillRects(const RectF* r, sysuint_t count);
  virtual void fillRound(const RectF& r, const PointF& radius);
  virtual void fillRound(const RectF& r,
    const PointF& tlr, const PointF& trr,
    const PointF& blr, const PointF& brr);
  virtual void fillEllipse(const PointF& cp, const PointF& r);
  virtual void fillArc(const PointF& cp, const PointF& r, double start, double sweep);
  virtual void fillPath(const Path& path);

  // [Glyph / Text Drawing]

  virtual void drawGlyph(const Point& pt, const Glyph& glyph, const Rect* clip);
  virtual void drawGlyphSet(const Point& pt, const GlyphSet& glyphSet, const Rect* clip);

  virtual void drawText(const Point& p, const String32& text, const Font& font, const Rect* clip);
  virtual void drawText(const Rect& r, const String32& text, const Font& font, uint32_t align, const Rect* clip);

  // [Image drawing]

  virtual void drawImage(const Point& p, const Image& image, const Rect* irect);

  // [Helpers]

  void _updateWorkRegion();
  void _setDeviceDefaults();

  FOG_INLINE void _updateLineWidth()
  { _lineIsSimple = (_lineWidth == 1.0 && _lineDash.length() == 0); }

  // [Renderers]
  //
  // Renderers are designed as virtual methods, because they are reimplemented
  // by MTRasterPainterDevice class to be able to use multiple threads.

  virtual void _renderGlyphSet(const Point& pt, const GlyphSet& glyphSet, const Rect* clip);
  virtual void _renderBoxes(const Box* box, sysuint_t count);
  virtual void _renderPath(const Path& path, bool stroke);

  // [Members]

  uint8_t* _metaRaster;
  uint8_t* _workRaster;
  sysint_t _stride;

  int _metaWidth;
  int _metaHeight;

  ImageFormat _format;
  sysint_t _bpp;

  Point _metaOrigin;
  Point _userOrigin;
  Point _workOrigin;

  Region _metaRegion;
  Region _userRegion;
  Region _workRegion;

  bool _metaRegionUsed;
  bool _userRegionUsed;
  bool _workRegionUsed;

  bool _clipSimple;
  Box _clipBox;

  uint32_t _op;

  Rgba _source;

  double _lineWidth;
  bool _lineIsSimple;
  uint32_t _lineCap;
  uint32_t _lineJoin;
  Vector<double> _lineDash;
  double _lineDashOffset;
  double _miterLimit;

  uint32_t _fillMode;

  AffineMatrix _transformations;
  bool _transformationsUsed;

  // Temporary path
  Path _workPath;

  // [AntiGrain Members]
  typedef agg::conv_curve<AggPath>              ConvCurve;
  typedef agg::conv_stroke<ConvCurve>           ConvStroke;

  typedef agg::conv_transform<ConvCurve>        ConvCurveTransform;
  typedef agg::conv_transform<ConvStroke>       ConvStrokeTransform;

  typedef agg::rasterizer_scanline_aa<>         Rasterizer;
  typedef agg::scanline_p8                      ScanlineP8;
  typedef agg::scanline_u8                      ScanlineU8;

  Rasterizer _ras;
  ScanlineP8 _slP8;
  ScanlineU8 _slU8;

  FillFuncs _fillFuncs;
};

// ============================================================================
// [Fog::RasterPainterDevice - Construction / Destruction]
// ============================================================================

RasterPainterDevice::RasterPainterDevice(uint8_t* pixels, int width, int height, sysint_t stride, const ImageFormat& format)
{
  _metaRaster = pixels;
  _workRaster = pixels;
  _stride = stride;

  _metaWidth = width;
  _metaHeight = height;

  _format = format;
  _bpp = format.depth() >> 3;

  _metaOrigin.set(0, 0);
  _userOrigin.set(0, 0);
  _workOrigin.set(0, 0);

  _metaRegionUsed = false;
  _userRegionUsed = false;
  _workRegionUsed = false;

  _clipSimple = true;
  _clipBox.set(0, 0, width, height);

  _op = OpCombine;
  _source = Rgba(255, 255, 255, 255);

  painter_local->getFillFuncs(&_fillFuncs);

  _setDeviceDefaults();
}

RasterPainterDevice::~RasterPainterDevice()
{
}

// ============================================================================
// [Fog::RasterPainterDevice - Meta]
// ============================================================================

int RasterPainterDevice::width() const
{
  return _metaWidth;
}

int RasterPainterDevice::height() const
{
  return _metaHeight;
}

ImageFormat RasterPainterDevice::format() const
{
  return _format;
}

void RasterPainterDevice::setMetaVariables(
  const Point& metaOrigin,
  const Region& metaRegion,
  bool useMetaRegion,
  bool reset)
{
  _metaOrigin = metaOrigin;

  _metaRegion = metaRegion;
  _metaRegionUsed = useMetaRegion;

  if (reset)
  {
    _userOrigin.set(0, 0);
    _userRegion.clear();
    _userRegionUsed = false;
  }

  _updateWorkRegion();
}

void RasterPainterDevice::setMetaOrigin(const Point& pt)
{
  if (_metaOrigin == pt) return;

  _metaOrigin = pt;
  _updateWorkRegion();
}

void RasterPainterDevice::setUserOrigin(const Point& pt)
{
  if (_userOrigin == pt) return;

  _userOrigin = pt;
  _updateWorkRegion();
}

void RasterPainterDevice::translateMetaOrigin(const Point& pt)
{
  if (pt.x() == 0 && pt.y() == 0) return;

  _metaOrigin += pt;
  _updateWorkRegion();
}

void RasterPainterDevice::translateUserOrigin(const Point& pt)
{
  if (pt.x() == 0 && pt.y() == 0) return;

  _userOrigin += pt;
  _updateWorkRegion();
}

void RasterPainterDevice::setUserRegion(const Rect& r)
{
  _userRegion = r;
  _userRegionUsed = true;

  _updateWorkRegion();
}

void RasterPainterDevice::setUserRegion(const Region& r)
{
  _userRegion = r;
  _userRegionUsed = true;

  _updateWorkRegion();
}

void RasterPainterDevice::resetMetaVars()
{
  _metaOrigin.set(0, 0);
  _metaRegion.clear();
  _metaRegionUsed = false;

  _updateWorkRegion();
}

void RasterPainterDevice::resetUserVars()
{
  _userOrigin.set(0, 0);
  _userRegion.clear();
  _userRegionUsed = false;

  _updateWorkRegion();
}

Point RasterPainterDevice::metaOrigin() const
{
  return _metaOrigin;
}

Point RasterPainterDevice::userOrigin() const
{
  return _userOrigin;
}

Region RasterPainterDevice::metaRegion() const
{
  return _metaRegion;
}

Region RasterPainterDevice::userRegion() const
{
  return _userRegion;
}

bool RasterPainterDevice::usedMetaRegion() const
{
  return _metaRegionUsed;
}

bool RasterPainterDevice::usedUserRegion() const
{
  return _userRegionUsed;
}

// ============================================================================
// [Fog::RasterPainterDevice - Operator]
// ============================================================================

void RasterPainterDevice::setOp(uint32_t op)
{
}

uint32_t RasterPainterDevice::op() const
{
  return _op;
}

// ============================================================================
// [Fog::RasterPainterDevice - Source]
// ============================================================================

void RasterPainterDevice::setSource(const Rgba& rgba)
{
  _source = rgba;
}

Rgba RasterPainterDevice::sourceRgba()
{
  return _source;
}

// ============================================================================
// [Fog::RasterPainterDevice - Parameters]
// ============================================================================

void RasterPainterDevice::setLineWidth(double lineWidth)
{
  _lineWidth = lineWidth;
  _updateLineWidth();
}

double RasterPainterDevice::lineWidth() const
{
  return _lineWidth;
}

void RasterPainterDevice::setLineCap(uint32_t lineCap)
{
  _lineCap = lineCap;
}

uint32_t RasterPainterDevice::lineCap() const
{
  return _lineCap;
}

void RasterPainterDevice::setLineJoin(uint32_t lineJoin)
{
  _lineJoin = lineJoin;
}

uint32_t RasterPainterDevice::lineJoin() const
{
  return _lineJoin;
}

void RasterPainterDevice::setLineDash(const double* dashes, sysuint_t count)
{
  _lineDash.clear();
  for (sysuint_t i = 0; i < count; i++) _lineDash.append(*dashes);
  _updateLineWidth();
}

void RasterPainterDevice::setLineDash(const Vector<double>& dashes)
{
  _lineDash = dashes;
  _updateLineWidth();
}

Vector<double> RasterPainterDevice::lineDash() const
{
  return _lineDash;
}

void RasterPainterDevice::setLineDashOffset(double offset)
{
  _lineDashOffset = offset;
  _updateLineWidth();
}

double RasterPainterDevice::lineDashOffset() const
{
  return _lineDashOffset;
}

void RasterPainterDevice::setMiterLimit(double miterLimit)
{
  _miterLimit = miterLimit;
}

double RasterPainterDevice::miterLimit() const
{
  return _miterLimit;
}

void RasterPainterDevice::setFillMode(uint32_t mode)
{
  _fillMode = mode;
}

uint32_t RasterPainterDevice::fillMode()
{
  return _fillMode;
}

// ============================================================================
// [Fog::RasterPainterDevice - Transformations]
// ============================================================================

void RasterPainterDevice::setMatrix(const AffineMatrix& m)
{
  _transformations = m;
  _transformationsUsed = !_transformations.isIdentity();
}

void RasterPainterDevice::resetMatrix()
{
  _transformations = AffineMatrix();
  _transformationsUsed = false;
}

AffineMatrix RasterPainterDevice::matrix() const
{
  return _transformations;
}

void RasterPainterDevice::rotate(double angle)
{
  _transformations *= AffineMatrix::fromRotation(angle);
  _transformationsUsed = !_transformations.isIdentity();
}

void RasterPainterDevice::scale(double sx, double sy)
{
  _transformations *= AffineMatrix::fromScale(sx, sy);
  _transformationsUsed = !_transformations.isIdentity();
}

void RasterPainterDevice::skew(double sx, double sy)
{
  _transformations *= AffineMatrix::fromSkew(sx, sy);
  _transformationsUsed = !_transformations.isIdentity();
}

void RasterPainterDevice::translate(double x, double y)
{
  _transformations *= AffineMatrix::fromTranslation(x, y);
  _transformationsUsed = !_transformations.isIdentity();
}

void RasterPainterDevice::affine(const AffineMatrix& m)
{
  _transformations *= m;
  _transformationsUsed = !_transformations.isIdentity();
}

void RasterPainterDevice::parallelogram(
  double x1, double y1, double x2, double y2, const double* para)
{
  _transformations *= AffineMatrix(x1, y1, x2, y2, para);
  _transformationsUsed = !_transformations.isIdentity();
}

void RasterPainterDevice::viewport(
  double worldX1,  double worldY1,  double worldX2,  double worldY2,
  double screenX1, double screenY1, double screenX2, double screenY2,
  uint32_t viewportOption)
{
  agg::trans_viewport vp;

  switch (viewportOption)
  {
    case ViewAnisotropic: vp.preserve_aspect_ratio(0.0, 0.0, agg::aspect_ratio_stretch); break;
    case ViewXMinYMin:    vp.preserve_aspect_ratio(0.0, 0.0, agg::aspect_ratio_meet);    break;
    case ViewXMidYMin:    vp.preserve_aspect_ratio(0.5, 0.0, agg::aspect_ratio_meet);    break;
    case ViewXMaxYMin:    vp.preserve_aspect_ratio(1.0, 0.0, agg::aspect_ratio_meet);    break;
    case ViewXMinYMid:    vp.preserve_aspect_ratio(0.0, 0.5, agg::aspect_ratio_meet);    break;
    case ViewXMidYMid:    vp.preserve_aspect_ratio(0.5, 0.5, agg::aspect_ratio_meet);    break;
    case ViewXMaxYMid:    vp.preserve_aspect_ratio(1.0, 0.5, agg::aspect_ratio_meet);    break;
    case ViewXMinYMax:    vp.preserve_aspect_ratio(0.0, 1.0, agg::aspect_ratio_meet);    break;
    case ViewXMidYMax:    vp.preserve_aspect_ratio(0.5, 1.0, agg::aspect_ratio_meet);    break;
    case ViewXMaxYMax:    vp.preserve_aspect_ratio(1.0, 1.0, agg::aspect_ratio_meet);    break;
  }

  vp.world_viewport(worldX1,   worldY1,  worldX2,  worldY2);
  vp.device_viewport(screenX1, screenY1, screenX2, screenY2);

  agg::trans_affine aff = vp.to_affine();
  _transformations *= *((const AffineMatrix *)&aff);
  _transformationsUsed = !_transformations.isIdentity();
}

// ============================================================================
// [Fog::RasterPainterDevice - Raster drawing]
// ============================================================================

void RasterPainterDevice::clear()
{
  if (_clipSimple)
    _renderBoxes(&_clipBox, 1);
  else
    _renderBoxes(_workRegion.cData(), _workRegion.count());
}

void RasterPainterDevice::drawPixel(const Point& p)
{
  RasterPainterDevice::drawPoint(
    PointF((double)p.x() + 0.5, (double)p.y() + 0.5));
}

void RasterPainterDevice::drawLine(const Point& start, const Point& end)
{
  RasterPainterDevice::drawLine(
    PointF((double)start.x() + 0.5, (double)start.y() + 0.5),
    PointF((double)end.x() + 0.5, (double)end.y() + 0.5));
}

void RasterPainterDevice::drawRect(const Rect& r)
{
  if (!r.isValid()) return;

  if (_transformationsUsed || !_lineIsSimple)
  {
    RasterPainterDevice::drawRect(
      RectF(
        (double)r.x1() + 0.5,
        (double)r.y1() + 0.5,
        (double)r.width(),
        (double)r.height()));
    return;
  }

  Box box[4];
  sysuint_t count = 4;

  if (r.width() <= 2 || r.height() <= 2)
  {
    box[0].set(r.x1(), r.y1(), r.x2(), r.y2());
    count = 1;
  }
  else
  {
    box[0].set(r.x1()  , r.y1()  , r.x2()  , r.y1()+1);
    box[1].set(r.x1()  , r.y1()+1, r.x1()+1, r.y2()-1);
    box[2].set(r.x2()-1, r.y1()+1, r.x2()  , r.y2()-1);
    box[3].set(r.x1()  , r.y2()-1, r.x2()  , r.y2()  );
  }

  if (_clipSimple)
  {
    if (!_clipBox.subsumes(r))
    {
      for (sysuint_t i = 0; i < count; i++) Box::intersect(box[0], box[0], _clipBox);
    }
    RasterPainterDevice::_renderBoxes(box, count);
  }
  else
  {
    TemporaryRegion<4> regionBox;
    TemporaryRegion<16> regionISect;
    regionBox.set(box, count);

    Region::intersect(regionISect, regionBox, _workRegion);
    if (!regionISect.count()) return;

    RasterPainterDevice::_renderBoxes(regionISect.cData(), regionISect.count());
  }
}

void RasterPainterDevice::drawRound(const Rect& r, const Point& radius)
{
  RasterPainterDevice::drawRound(
    RectF((double)r.x1() + 0.5, (double)r.y1() + 0.5, r.width(), r.height()),
    PointF((double)radius.x(), (double)radius.y()));
}

void RasterPainterDevice::fillRect(const Rect& r)
{
  if (!r.isValid()) return;

  if (_transformationsUsed)
  {
    RasterPainterDevice::fillRect(
      RectF(
        (double)r.x1() + 0.5,
        (double)r.y1() + 0.5,
        (double)r.width(),
        (double)r.height()));
    return;
  }

  Box box(r.x1(), r.y1(), r.x2(), r.y2());

  if (_clipSimple)
  {
    _clipBox.intersect(box, box, _clipBox);
    if (!box.isValid()) return;

    RasterPainterDevice::_renderBoxes(&box, 1);
  }
  else
  {
    TemporaryRegion<1> regionBox;
    TemporaryRegion<16> regionISect;
    regionBox.set(&box, 1);

    Region::intersect(regionISect, regionBox, _workRegion);
    if (!regionISect.count()) return;

    RasterPainterDevice::_renderBoxes(regionISect.cData(), regionISect.count());
  }
}

void RasterPainterDevice::fillRects(const Rect* r, sysuint_t count)
{
  if (!count) return;

  if (_transformationsUsed)
  {
    _workPath.clear();
    for (sysuint_t i = 0; i < count; i++)
    {
      if (r[i].isValid()) _workPath.addRect(
        RectF(
          (double)r[i].x1() + 0.5,
          (double)r[i].y1() + 0.5,
          (double)r[i].width(),
          (double)r[i].height()));
    }
    fillPath(_workPath);
    return;
  }

  Region region;
  region.set(r, count);
  if (!region.count()) return;

  if (_clipSimple)
  {
    if (_clipBox.subsumes(region.extents()))
    {
      _renderBoxes(region.cData(), region.count());
      return;
    }
    region.intersect(_clipBox);
    if (!region.count()) return;

    _renderBoxes(region.cData(), region.count());
  }
  else
  {
    Region regionISect;
    Region::intersect(regionISect, _workRegion, region);
    if (!regionISect.count()) return;

    _renderBoxes(regionISect.cData(), regionISect.count());
  }
}

void RasterPainterDevice::fillRound(const Rect& r, const Point& radius)
{
  RasterPainterDevice::fillRound(
    RectF((double)r.x1() + 0.5, (double)r.y1() + 0.5, r.width(), r.height()),
    PointF(radius.x(), radius.y()));
}

void RasterPainterDevice::fillRegion(const Region& region)
{
  if (_clipSimple && _clipBox.subsumes(region.extents()))
  {
    _renderBoxes(region.cData(), region.count());
  }
  else
  {
    TemporaryRegion<16> dst;
    Region::intersect(dst, _workRegion, region);
    if (!dst.count()) return;

    _renderBoxes(dst.cData(), dst.count());
  }
}

// ============================================================================
// [Fog::RasterPainterDevice - Vector drawing]
// ============================================================================

void RasterPainterDevice::drawPoint(const PointF& p)
{
  _workPath.clear();
  _workPath.moveTo(p);
  _workPath.lineTo(p.x(), p.y() + 0.0001);
  drawPath(_workPath);
}

void RasterPainterDevice::drawLine(const PointF& start, const PointF& end)
{
  _workPath.clear();
  _workPath.moveTo(start);
  _workPath.lineTo(end);
  drawPath(_workPath);
}

void RasterPainterDevice::drawLine(const PointF* pts, sysuint_t count)
{
  if (!count) return;

  _workPath.clear();
  _workPath.moveTo(pts[0]);
  if (count > 1)
    _workPath.addLineTo(pts + 1, count - 1);
  else
    _workPath.lineTo(pts[0].x(), pts[0].y() + 0.0001);
  drawPath(_workPath);
}

void RasterPainterDevice::drawPolygon(const PointF* pts, sysuint_t count)
{
  if (!count) return;

  _workPath.clear();
  _workPath.moveTo(pts[0]);
  if (count > 1)
    _workPath.addLineTo(pts + 1, count - 1);
  else
    _workPath.lineTo(pts[0].x(), pts[0].y() + 0.0001);
  _workPath.closePolygon();
  drawPath(_workPath);
}

void RasterPainterDevice::drawRect(const RectF& r)
{
  if (!r.isValid()) return;

  _workPath.clear();
  _workPath.addRect(r);
  drawPath(_workPath);
}

void RasterPainterDevice::drawRects(const RectF* r, sysuint_t count)
{
  if (!count) return;

  _workPath.clear();
  for (sysuint_t i = 0; i < count; i++)
  {
    if (r[i].isValid()) _workPath.addRect(r[i]);
  }
  drawPath(_workPath);
}

void RasterPainterDevice::drawRound(const RectF& r, const PointF& radius)
{
  RasterPainterDevice::drawRound(r, radius, radius, radius, radius);
}

void RasterPainterDevice::drawRound(const RectF& r,
  const PointF& tlr, const PointF& trr,
  const PointF& blr, const PointF& brr)
{
  if (!r.isValid()) return;
  agg::rounded_rect rc;

  rc.rect(
    r.x1(), r.y1(),
    r.x2(), r.y2());
  rc.radius(
    tlr.x(), tlr.y(), trr.x(), trr.y(),
    blr.x(), blr.y(), brr.x(), brr.y());
  rc.normalize_radius();
  // TODO:
  // rc.approximation_scale(worldToScreen(1.0) * g_approxScale);

  _workPath.clear();
  concatToPath(_workPath, rc, 0);
  drawPath(_workPath);
}

void RasterPainterDevice::drawEllipse(const PointF& cp, const PointF& r)
{
  RasterPainterDevice::drawArc(cp, r, 0.0, 2.0 * M_PI);
}

void RasterPainterDevice::drawArc(const PointF& cp, const PointF& r, double start, double sweep)
{
  agg::bezier_arc arc(cp.x(), cp.y(), r.x(), r.y(), start, sweep);

  _workPath.clear();
  concatToPath(_workPath, arc, 0);
  drawPath(_workPath);
}

void RasterPainterDevice::drawPath(const Path& path)
{
  _renderPath(path, true);
}

void RasterPainterDevice::fillPolygon(const PointF* pts, sysuint_t count)
{
  if (!count) return;

  _workPath.clear();
  _workPath.moveTo(pts[0]);
  if (count > 1)
    _workPath.addLineTo(pts + 1, count - 1);
  else
    _workPath.lineTo(pts[0].x(), pts[0].y() + 0.0001);
  _workPath.closePolygon();
  fillPath(_workPath);
}

void RasterPainterDevice::fillRect(const RectF& r)
{
  if (!r.isValid()) return;

  _workPath.clear();
  _workPath.addRect(r);
  fillPath(_workPath);
}

void RasterPainterDevice::fillRects(const RectF* r, sysuint_t count)
{
  if (!count) return;

  _workPath.clear();
  for (sysuint_t i = 0; i < count; i++)
  {
    if (r[i].isValid()) _workPath.addRect(r[i]);
  }
  fillPath(_workPath);
}

void RasterPainterDevice::fillRound(const RectF& r, const PointF& radius)
{
  RasterPainterDevice::fillRound(r, radius, radius, radius, radius);
}

void RasterPainterDevice::fillRound(const RectF& r,
  const PointF& tlr, const PointF& trr,
  const PointF& blr, const PointF& brr)
{
  if (!r.isValid()) return;
  agg::rounded_rect rc;

  rc.rect(
    r.x1(), r.y1(),
    r.x2(), r.y2());
  rc.radius(
    tlr.x(), tlr.y(), trr.x(), trr.y(),
    blr.x(), blr.y(), brr.x(), brr.y());
  rc.normalize_radius();
  // TODO:
  // rc.approximation_scale(worldToScreen(1.0) * g_approxScale);

  _workPath.clear();
  concatToPath(_workPath, rc, 0);
  fillPath(_workPath);
}

void RasterPainterDevice::fillEllipse(const PointF& cp, const PointF& r)
{
  RasterPainterDevice::fillArc(cp, r, 0.0, 2.0 * M_PI);
}

void RasterPainterDevice::fillArc(const PointF& cp, const PointF& r, double start, double sweep)
{
  agg::bezier_arc arc(cp.x(), cp.y(), r.x(), r.y(), start, sweep);

  _workPath.clear();
  concatToPath(_workPath, arc, 0);
  fillPath(_workPath);
}

void RasterPainterDevice::fillPath(const Path& path)
{
  _renderPath(path, false);
}

// ============================================================================
// [Fog::RasterPainterDevice - Glyph / Text Drawing]
// ============================================================================

void RasterPainterDevice::drawGlyph(const Point& pt, const Glyph& glyph, const Rect* clip)
{
  TemporaryGlyphSet<1> glyphSet;
  err_t err;

  if ( (err = glyphSet.begin(1)) ) return;
  glyphSet._add(glyph._d->ref());
  if ( (err = glyphSet.end()) ) return;

  _renderGlyphSet(pt, glyphSet, clip);
}

void RasterPainterDevice::drawGlyphSet(const Point& pt, const GlyphSet& glyphSet, const Rect* clip)
{
  _renderGlyphSet(pt, glyphSet, clip);
}

void RasterPainterDevice::drawText(const Point& pt, const String32& text, const Font& font, const Rect* clip)
{
  TemporaryGlyphSet<128> glyphSet;
  if (font.getGlyphs(text.cData(), text.length(), glyphSet)) return;

  _renderGlyphSet(pt, glyphSet, clip);
}

void RasterPainterDevice::drawText(const Rect& r, const String32& text, const Font& font, uint32_t align, const Rect* clip)
{
  // TODO
}

// ============================================================================
// [Fog::RasterPainterDevice - Image drawing]
// ============================================================================

void RasterPainterDevice::drawImage(const Point& p, const Image& image, const Rect* irect)
{
  // TODO
}

// ============================================================================
// [Fog::RasterPainterDevice - Helpers]
// ============================================================================

void RasterPainterDevice::_updateWorkRegion()
{
  _workOrigin = _metaOrigin + _userOrigin;

  int negx = -_workOrigin.x();
  int negy = -_workOrigin.y();

  // This is maximal clip box that can be used by painter.
  _clipBox.set(negx, negy, negx + _metaWidth, negy + _metaHeight);
  _clipSimple = true;

  // Do region calculations only if they are really used.
  if (_metaRegionUsed || _userRegionUsed)
  {
    if (_metaRegionUsed)
    {
      Region::translate(_workRegion, _metaRegion, Point(negx, negy));
    }
    else
    {
      _workRegion.set(Box(negx, negy, negx + _metaWidth, negy + _metaHeight));
    }

    if (_userRegionUsed)
    {
      // Optimized!
      if (_metaOrigin.x() || _metaOrigin.y())
      {
        TemporaryRegion<64> _userTmp;
        Region::translate(_userTmp, _userRegion, _metaOrigin.negated());
        Region::subtract(_workRegion, _workRegion, _userTmp);
      }
      else
      {
        Region::subtract(_workRegion, _workRegion, _userRegion);
      }
    }

    // Switch to box clip implementation if resulting region is simple.
    if (_workRegion.count() == 1)
    {
      _clipBox.set(_workRegion.extents());

      _workRegion.clear();
      _workRegionUsed = false;
    }
    else
    {
      _clipSimple = false;
      _clipBox.set(_workRegion.extents());

      _workRegionUsed = true;
    }
  }
  else
  {
    // There is no clip box, we will use painter bounds that was in _clipBox
    // already
    _workRegion.clear();
    _workRegionUsed = false;
  }

  // This is very good trick. Make raster -relative to _workOrigin, so no
  // calculations are needed to draw pixels relative to _workOrigin.
  _workRaster = _metaRaster +
    (sysint_t)_workOrigin.x() * _bpp +
    (sysint_t)_workOrigin.y() * _stride;
}

void RasterPainterDevice::_setDeviceDefaults()
{
  _lineWidth = 1.0;
  _lineIsSimple = true;

  _lineCap = LineCapRound;
  _lineJoin = LineJoinRound;

  _lineDash.free();
  _lineDashOffset = 0.0;

  _miterLimit = 1.0;

  _fillMode = FillNonZero;

  _transformations = AffineMatrix();
  _transformationsUsed = false;
}

// ============================================================================
// [Fog::RasterPainterDevice - AntiGrain Renderers]
// ============================================================================

#if 0
template<int BitsPerPixel, class Rasterizer, class Scanline>
static void FOG_OPTIMIZEDCALL WdeAGG_render_scanlines(RasterPainterDevice* painter_d, Rasterizer& ras, Scanline& sl)
{
  if (!ras.rewind_scanlines()) return;

  int y = 0;
  uint8_t* destBase = painter_d->_pixels + y * painter_d->_stride;
  uint8_t* destCur;
  sysint_t stride = painter_d->_stride;

  sl.reset(ras.min_x(), ras.max_x());

  //int extx1 = painter_d->_realRegion.extents().x1();
  int exty1 = painter_d->_realRegion.extents().y1();
  //int extx2 = painter_d->_realRegion.extents().x2();
  int exty2 = painter_d->_realRegion.extents().y2();

  const PainterSoftwareDrawFuncs* funcs = D_DRAW(painter_d);

  // solid source
  if (painter_d->_sourceType == Painter::SolidSourceType)
  {
    uint32_t src = painter_d->_rgbaSource;

    while(ras.sweep_scanline(sl))
    {
      unsigned num_spans = sl.num_spans();
      typename Scanline::const_iterator span = sl.begin();

      // vertical clipping to extents
      if (sl.y() < exty1) continue;
      if (sl.y() >= exty2) break;

      if (sl.y() == y + 1)
        destBase += stride;
      else
        destBase += (sl.y()-y) * stride;
      y = sl.y();

      for (;;)
      {
        int x = span->x;
        int len = span->len;

        destCur = destBase + PainterSoftware_xaddr<BitsPerPixel>(x);

        if (len > 0)
        {
          funcs->solidSpanMaskA8(destCur, src, span->covers, 0, (unsigned)len, &cpuState);
        }
        else
        {
          len = -len;
          uint32_t cover = (uint32_t) *(span->covers);
          if (cover == 0xFF)
            funcs->solidSpan(destCur, src, (unsigned)len, &cpuState);
          else
            funcs->solidSpanConstMaskA8(destCur, src, cover, (unsigned)len, &cpuState);
        }
        cpuState.clearFPU();

        if (--num_spans == 0) break;
        ++span;
      }
    }
  }
  // pattern source
  else 
  {
    PainterSoftwareRenderContext* context = (PainterSoftwareRenderContext* )painter_d->_context;

    LocalBuffer<2048*sizeof(RGBA)> bufferStorage;
    uint8_t* buffer = (uint8_t*)bufferStorage.alloc( (uint)(ras.max_x() - ras.min_x() + 3 /* rounding errors */) * sizeof(RGBA) );
    
    while(ras.sweep_scanline(sl))
    {
      unsigned num_spans = sl.num_spans();
      typename Scanline::const_iterator span = sl.begin();

      // vertical clipping to extents
      if (sl.y() < exty1) continue;
      if (sl.y() >= exty2) break;

      if (sl.y() == y + 1)
        destBase += stride;
      else
        destBase += (sl.y()-y) * stride;
      y = sl.y();

      for (;;)
      {
        int x = span->x;
        int len = span->len;

        destCur = destBase + PainterSoftware_xaddr<BitsPerPixel>(x);

        if (len > 0)
        {
          context->indirectRender(buffer, context, x, y, (unsigned)len, &cpuState);
          funcs->argbPatternSpanMaskA8(destCur, (uint32_t*)buffer, span->covers, 0, (unsigned)len, &cpuState);
        }
        else
        {
          len = -len;
          context->indirectRender(buffer, context, x, y, (unsigned)len, &cpuState);

          uint32_t cover = (uint32_t) *(span->covers);
          if (cover == 0xFF)
            funcs->argbPatternSpan(destCur, (uint32_t*)buffer, (unsigned)len, &cpuState);
          else
            funcs->argbPatternSpanConstMaskA8(destCur, (uint32_t*)buffer, cover, (unsigned)len, &cpuState);
        }
        cpuState.clearFPU();

        if (--num_spans == 0) break;
        ++span;
      }
    }
  }
}

template<class VertexSource>
static void WdeAGG_renderPath(RasterPainterDevice* d, VertexSource& path)
{
  d->ras.reset();
  d->ras.add_path(path);
  d->render_scanlines_aa_p8(d, aggContext->ras, aggContext->sl_p8);
}

template<class PathT>
static void FOG_OPTIMIZEDCALL PainterSoftware_drawPoly_f_private(RasterPainterDevice* d, PathT& path)
{
  if (d->_lineDashes.count() <= 1)
  {
    // stroke
    agg::conv_stroke<PathT> s1(path);

    s1.width(d->_lineWidth);
    s1.line_join((agg::line_join_e)d->_lineJoin);
    s1.line_cap((agg::line_cap_e)d->_lineCap);
    s1.miter_limit(d->_miterLimit);

    WdeAGG_renderPath(d, s1);
  }
  else
  {
    // convert curves to lines and create agg::conv_dash 
    agg::conv_dash<PathT, agg::vcgen_dash > s1(path);

    // generate dashes
    List<double>::ConstIterator i(d->_lineDashes);
    i.toStart();

    for (;;) 
    {
      double d1 = i.value(); i.toNext();
      if (!i.valid()) break;
      double d2 = i.value(); i.toNext();
      s1.add_dash(d1, d2);
      if (!i.valid()) break;
    }
    s1.dash_start(d->_lineDashOffset);

    // stroke
    agg::conv_stroke< agg::conv_dash<PathT, agg::vcgen_dash> > s2(s1);

    s2.width(d->_lineWidth);
    s2.line_join((agg::line_join_e)d->_lineJoin);
    s2.line_cap((agg::line_cap_e)d->_lineCap);
    s2.miter_limit(d->_miterLimit);

    WdeAGG_renderPath(d, s2);
  }
}

template<class Path>
static void FOG_OPTIMIZEDCALL PainterSoftware_drawPath_f_private(RasterPainterDevice* d, Path& path)
{
  if (d->_lineDashes.count() <= 1)
  {
    // convert curves to lines 
    agg::conv_curve<Path> s0(path);

    // stroke
    agg::conv_stroke< agg::conv_curve<Path> > s1(s0);

    s1.width(d->_lineWidth);
    s1.line_join((agg::line_join_e)d->_lineJoin);
    s1.line_cap((agg::line_cap_e)d->_lineCap);
    s1.miter_limit(d->_miterLimit);

    WdeAGG_renderPath(d, s1);
  }
  else
  {
    // convert curves to lines and create agg::conv_dash 
    agg::conv_curve<Path> s0(path);
    agg::conv_dash<agg::conv_curve<Path>, agg::vcgen_dash > s1(s0);

    // generate dashes
    s1.dash_start(d->_lineDashOffset);

    List<double>::ConstIterator i(d->_lineDashes);
    i.toStart();

    for (;;) 
    {
      double d1 = i.value(); i.toNext();
      if (!i.valid()) break;
      double d2 = i.value(); i.toNext();
      s1.add_dash(d1, d2);
      if (!i.valid()) break;
    }

    // stroke
    agg::conv_stroke< agg::conv_dash<agg::conv_curve<Path>, agg::vcgen_dash> > s2(s1);

    s2.width(d->_lineWidth);
    s2.line_join((agg::line_join_e)d->_lineJoin);
    s2.line_cap((agg::line_cap_e)d->_lineCap);
    s2.miter_limit(d->_miterLimit);

    WdeAGG_renderPath(d, s2);
  }
}

template<class Path>
static void FOG_OPTIMIZEDCALL PainterSoftware_fillPoly_f_private(RasterPainterDevice* d, Path& path)
{
  WdeAGG_renderPath(d, path);
}

template<class Path>
static void FOG_OPTIMIZEDCALL PainterSoftware_fillPath_f_private(RasterPainterDevice* d, Path& path)
{
  agg::conv_curve<Path> c(path);
  WdeAGG_renderPath(d, c);
}
#endif

template<int BytesPerPixel, class Rasterizer, class Scanline>
static void FOG_OPTIMIZEDCALL AggRenderScanlines(RasterPainterDevice* d, Rasterizer& ras, Scanline& sl)
{
  if (!ras.rewind_scanlines()) return;

  uint8_t* pBase = d->_workRaster;
  uint8_t* pRas;
  uint8_t* pCur;
  sysint_t stride = d->_stride;

  sl.reset(ras.min_x(), ras.max_x());

  // TODO: Not needed ?
  // int extx1 = painter_d->_realRegion.extents().x1();
  // int exty1 = d->_clipBox.y1();
  // int extx2 = painter_d->_realRegion.extents().x2();
  // int exty2 = d->_clipBox.y2();

  FillSpan fillSpan = d->_fillFuncs.fillSpan;
  FillSpanM fillSpanM_A8 = d->_fillFuncs.fillSpanM_A8;

  // solid source
  if (1)
  {
    while (ras.sweep_scanline(sl))
    {
      unsigned num_spans = sl.num_spans();
      typename Scanline::const_iterator span = sl.begin();

      sysint_t y = sl.y();

      // TODO: Not needed ?
      // Vertical clipping to extents.
      // if (y < exty1) continue;
      // if (y >= exty2) break;

      pRas = pBase + y * stride;

      for (;;)
      {
        int x = span->x;
        int len = span->len;

        pCur = pRas + Raster::mul<int, BytesPerPixel>(x);

        if (len > 0)
        {
          fillSpanM_A8(pCur, &d->_source, span->covers, (unsigned)len);
        }
        else
        {
          len = -len;
          FOG_ASSERT(len > 0);

          uint32_t cover = (uint32_t)*(span->covers);
          if (cover == 0xFF)
          {
            fillSpan(pCur, &d->_source, len);
          }
          else
          {
            uint32_t t = Raster::bytemul(d->_source.i, cover);
            fillSpan(pCur, &t, len);
          }
        }

        if (--num_spans == 0) break;
        ++span;
      }
    }
  }
}

void RasterPainterDevice::_renderGlyphSet(const Point& pt, const GlyphSet& glyphSet, const Rect* clip)
{
  // TODO: Hardcoded to A8 glyph format
  // TODO: Clipping

  Box clipBox = _clipBox;
  if (clip) Box::intersect(clipBox, clipBox, Box(*clip));

  if (!clipBox.isValid()) return;
  if (!glyphSet.length()) return;

  const Glyph* glyphs = glyphSet.glyphs();
  sysuint_t count = glyphSet.length();

  int px = pt.x();
  int py = pt.y();

  uint8_t* pBuf = _workRaster;
  sysint_t stride = _stride;

  FillSpanM fillSpanM = _fillFuncs.fillSpanM_A8;

  for (sysuint_t i = 0; i < count; i++)
  {
    Glyph::Data* glyphd = glyphs[i]._d;

    int px1 = px + glyphd->offsetX;
    int py1 = py + glyphd->offsetY;
    int px2 = px1 + glyphd->image.width();
    int py2 = py1 + glyphd->image.height();

    px += glyphd->advance;

    int x1 = px1; if (x1 < clipBox.x1()) x1 = clipBox.x1();
    int y1 = py1; if (y1 < clipBox.y1()) y1 = clipBox.y1();
    int x2 = px2; if (x2 > clipBox.x2()) x2 = clipBox.x2();
    int y2 = py2; if (y2 > clipBox.y2()) y2 = clipBox.y2();

    int w = x2 - x1; if (w <= 0) continue;
    int h = y2 - y1; if (h <= 0) continue;

    // TODO: Hardcoded
    uint8_t* pCur = pBuf;
    pCur += (sysint_t)y1 * stride;
    pCur += (sysint_t)x1 * 4;

    // TODO: Hardcoded
    sysint_t glyphStride = glyphd->image.stride();
    const uint8_t* pGlyph = glyphd->image.cData();

    pGlyph += (sysint_t)(y1 - py1) * glyphStride;
    pGlyph += (sysint_t)(x1 - px1);

    do {
      fillSpanM(pCur, &_source, pGlyph, (sysuint_t)w);
      pCur += stride;
      pGlyph += glyphStride;
    } while (--h);
  }
}

void RasterPainterDevice::_renderBoxes(const Box* box, sysuint_t count)
{
  if (!count) return;

  uint8_t* pBuf = _workRaster;
  sysint_t stride = _stride;
  sysint_t bpp = _bpp;

  FillSpan fillSpan = _fillFuncs.fillSpan;

  for (sysuint_t i = 0; i < count; i++)
  {
    sysint_t x = box[i].x1();
    sysint_t y = box[i].y1();

    sysint_t w = box[i].width();
    if (w <= 0) continue;
    sysint_t h = box[i].height();
    if (h <= 0) continue;

    // TODO: Hardcoded
    uint8_t* pCur = pBuf + y * stride + x * bpp;

    do {
      fillSpan(pCur, &_source, (sysuint_t)w);
      pCur += stride;
    } while (--h);
  }
}

void RasterPainterDevice::_renderPath(const Path& path, bool stroke)
{
  AggPath aggPath(path);
  ConvCurve curvesPath(aggPath);

  _ras.reset();
  _ras.filling_rule(static_cast<agg::filling_rule_e>(_fillMode));
  _ras.clip_box(
    (double)_clipBox.x1(),
    (double)_clipBox.y1(),
    (double)_clipBox.x2()-0.000001,
    (double)_clipBox.y2()-0.000001);

  // This can be a bit messy, but it's here to increase performance. We will
  // not calculate using transformations if they are not used. Also we add
  // stroke and line dash pipeline only if it's needed. This is goal of 
  // AntiGrain to be able to setup only pipelines what are really need.
  if (_transformationsUsed)
  {
    if (stroke)
    {
      ConvStroke strokePath(curvesPath);
      ConvStrokeTransform strokeTransform(
        strokePath, *((agg::trans_affine *)&_transformations));

      strokePath.width(_lineWidth);
      strokePath.line_join(static_cast<agg::line_join_e>(_lineJoin));
      strokePath.line_cap(static_cast<agg::line_cap_e>(_lineCap));
      strokePath.miter_limit(_miterLimit);

      _ras.add_path(strokeTransform);
    }
    else
    {
      ConvCurveTransform curvesTransform(
        curvesPath, *((agg::trans_affine *)&_transformations));

      _ras.add_path(curvesTransform);
    }
  }
  else
  {
    if (stroke)
    {
      ConvStroke strokePath(curvesPath);

      strokePath.width(_lineWidth);
      strokePath.line_join(static_cast<agg::line_join_e>(_lineJoin));
      strokePath.line_cap(static_cast<agg::line_cap_e>(_lineCap));
      strokePath.miter_limit(_miterLimit);

      _ras.add_path(strokePath);
    }
    else
    {
      _ras.add_path(curvesPath);
    }
  }

  AggRenderScanlines<4, Rasterizer, ScanlineP8>(this, _ras, _slP8);
}

// ============================================================================
// [Fog::Painter]
// ============================================================================

PainterDevice* Painter::sharedNull;

Painter::Painter()
{
  _d = sharedNull;
}

Painter::Painter(uint8_t* pixels, int width, int height, sysint_t stride, const ImageFormat& format)
{
  _d = sharedNull;
  begin(pixels, width, height, stride, format);
}

Painter::Painter(Image& image)
{
  _d = sharedNull;
  begin(image);
}

Painter::~Painter()
{
  if (_d != sharedNull) delete _d;
}

err_t Painter::begin(uint8_t* pixels, int width, int height, sysint_t stride, const ImageFormat& format)
{
  if (_d != sharedNull) delete _d;

  _d = new(std::nothrow) RasterPainterDevice(
    pixels, width, height, stride, format);

  if (!_d)
  {
    _d = sharedNull;
    return Error::OutOfMemory;
  }
  else
  {
    return Error::Ok;
  }
}

err_t Painter::begin(Image& image)
{
  return begin(image.mData(), image.width(), image.height(), image.stride(), image.format());
}

void Painter::end()
{
  if (_d != sharedNull) delete _d;
  _d = sharedNull;
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_painter_init(void)
{
  Fog::painter_local.init();

  static Fog::NullPainterDevice sharedNullDevice;
  Fog::Painter::sharedNull = &sharedNullDevice;

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_painter_shutdown(void)
{
  Fog::painter_local.destroy();
}
