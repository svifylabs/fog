// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct RasterPainterDevice;

// ============================================================================
// [Fog::RasterPainterClipState]
// ============================================================================

struct FOG_HIDDEN RasterPainterClipState
{
  // [Construction / Destruction]

  RasterPainterClipState();
  RasterPainterClipState(const RasterPainterClipState& other);
  ~RasterPainterClipState();

  RasterPainterClipState& operator=(const RasterPainterClipState& other);

  // [Ref / Deref]

  FOG_INLINE RasterPainterClipState* ref() const
  {
    refCount.inc();
    return const_cast<RasterPainterClipState*>(this);
  }

  FOG_INLINE void deref()
  {
    if (refCount.deref() == 0) delete this;
  }

  // [Members]

  mutable Atomic<sysuint_t> refCount;

  uint8_t* workRaster;

  Point metaOrigin;
  Point userOrigin;
  Point workOrigin;

  Region metaRegion;
  Region userRegion;
  Region workRegion;

  bool metaRegionUsed;
  bool userRegionUsed;
  bool workRegionUsed;

  bool clipSimple;
  Box clipBox;
};

RasterPainterClipState::RasterPainterClipState()
{
  refCount.init(1);
}

RasterPainterClipState::RasterPainterClipState(const RasterPainterClipState& other) :
  workRaster(other.workRaster),
  metaOrigin(other.metaOrigin),
  userOrigin(other.userOrigin),
  workOrigin(other.workOrigin),
  metaRegion(other.metaRegion),
  userRegion(other.userRegion),
  workRegion(other.workRegion),
  clipSimple(other.clipSimple),
  clipBox(other.clipBox)
{
  refCount.init(1);
}

RasterPainterClipState::~RasterPainterClipState()
{
}

RasterPainterClipState& RasterPainterClipState::operator=(const RasterPainterClipState& other)
{
  workRaster = other.workRaster;
  metaOrigin = other.metaOrigin;
  userOrigin = other.userOrigin;
  workOrigin = other.workOrigin;
  metaRegion = other.metaRegion;
  userRegion = other.userRegion;
  workRegion = other.workRegion;
  clipSimple = other.clipSimple;
  clipBox = other.clipBox;

  return *this;
}

// ============================================================================
// [Fog::RasterPainterCapsState]
// ============================================================================

struct FOG_HIDDEN RasterPainterCapsState
{
  // [Construction / Destruction]

  RasterPainterCapsState();
  RasterPainterCapsState(const RasterPainterCapsState& other);
  ~RasterPainterCapsState();

  RasterPainterCapsState& operator=(const RasterPainterCapsState& other);

  // [Ref / Deref]

  FOG_INLINE RasterPainterCapsState* ref() const
  {
    refCount.inc();
    return const_cast<RasterPainterCapsState*>(this);
  }

  FOG_INLINE void deref()
  {
    if (refCount.deref() == 0) delete this;
  }

  // [Members]

  mutable Atomic<sysuint_t> refCount;

  uint32_t op;

  uint32_t solidSource;
  uint32_t solidSourcePremultiplied;
  Pattern patternSource;
  bool isSolidSource;

  double lineWidth;
  bool lineIsSimple;
  uint32_t lineCap;
  uint32_t lineJoin;
  Vector<double> lineDash;
  double lineDashOffset;
  double miterLimit;

  uint32_t fillMode;

  AffineMatrix transformations;
  bool transformationsUsed;
};

RasterPainterCapsState::RasterPainterCapsState()
{
  refCount.init(1);
}

RasterPainterCapsState::RasterPainterCapsState(const RasterPainterCapsState& other) :
  op(other.op),
  solidSource(other.solidSource),
  solidSourcePremultiplied(other.solidSourcePremultiplied),
  patternSource(other.patternSource),
  isSolidSource(other.isSolidSource),
  lineWidth(other.lineWidth),
  lineIsSimple(other.lineIsSimple),
  lineCap(other.lineCap),
  lineJoin(other.lineJoin),
  lineDash(other.lineDash),
  lineDashOffset(other.lineDashOffset),
  miterLimit(other.miterLimit),
  fillMode(other.fillMode),
  transformations(other.transformations),
  transformationsUsed(other.transformationsUsed)
{
  refCount.init(1);
}

RasterPainterCapsState::~RasterPainterCapsState()
{
}

RasterPainterCapsState& RasterPainterCapsState::operator=(const RasterPainterCapsState& other)
{
  op = other.op;
  solidSource = other.solidSource;
  solidSourcePremultiplied = other.solidSourcePremultiplied;
  patternSource = other.patternSource;
  isSolidSource = other.isSolidSource;
  lineWidth = other.lineWidth;
  lineIsSimple = other.lineIsSimple;
  lineCap = other.lineCap;
  lineJoin = other.lineJoin;
  lineDash = other.lineDash;
  lineDashOffset = other.lineDashOffset;
  miterLimit = other.miterLimit;
  fillMode = other.fillMode;
  transformations = other.transformations;
  transformationsUsed = other.transformationsUsed;

  return *this;
}

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
// [Fog::RasterPainterContext]
// ============================================================================

// Agg pipeline typedefs
typedef agg::conv_curve<AggPath>                       AggConvCurve;
typedef agg::conv_stroke<AggConvCurve>                 AggConvCurveStroke;
typedef agg::conv_dash<AggConvCurve, agg::vcgen_dash>  AggConvCurveDash;
typedef agg::conv_stroke<AggConvCurveDash>             AggConvCurveDashStroke;

typedef agg::conv_transform<AggConvCurve>              AggConvCurveTransform;
typedef agg::conv_transform<AggConvCurveStroke>        AggConvCurveStrokeTransform;
typedef agg::conv_transform<AggConvCurveDashStroke>    AggConvCurveDashStrokeTransform;

// Rasterizer and scanline storage
typedef agg::rasterizer_scanline_aa<>         AggRasterizer;
typedef agg::scanline_p8                      AggScanlineP8;
typedef agg::scanline_u8                      AggScanlineU8;

// Context is accessed always from only one thread.
struct FOG_HIDDEN RasterPainterContext
{
  // [Construction / Destruction]

  RasterPainterContext(RasterPainterDevice* _owner);
  ~RasterPainterContext();

  // [Buffer Manager]

  uint8_t* getBuffer(sysint_t size);
  void releaseBuffer(uint8_t* buffer);

  // [Members]

  // Owner of this context.
  RasterPainterDevice* owner;

  // Clip state.
  RasterPainterClipState* clipState;
  // Capabilities state.
  RasterPainterCapsState* capsState;

  Raster::FunctionMap::Raster* raster;
  Raster::PatternContext* pctx;

  AggRasterizer ras;
  AggScanlineP8 slP8;
  AggScanlineU8 slU8;

  uint8_t* buffer;
  sysint_t bufferSize;

  uint8_t bufferStatic[1024*16];

private:
  FOG_DISABLE_COPY(RasterPainterContext)
};

RasterPainterContext::RasterPainterContext(RasterPainterDevice* _owner)
{
  owner = _owner;

  clipState = NULL;
  capsState = NULL;

  raster = NULL;
  pctx = NULL;

  buffer = bufferStatic;
  bufferSize = FOG_ARRAY_SIZE(bufferStatic);
}

RasterPainterContext::~RasterPainterContext()
{
  if (buffer != bufferStatic) Memory::free(buffer);
}

uint8_t* RasterPainterContext::getBuffer(sysint_t size)
{
  if (size > bufferSize)
  {
    if (buffer != bufferStatic) Memory::free(buffer);

    // First dynamic memory will be twice as bufferStatic.
    sysint_t capacity = FOG_ARRAY_SIZE(bufferStatic) << 1;
    while (size < capacity) capacity <<= 1;

    buffer = (uint8_t*)Memory::alloc(size);
    if (buffer == NULL)
    {
      // Error, set to safe defaults and return NULL, painter can't continue
      // if this happenned.
      buffer = bufferStatic;
      bufferSize = FOG_ARRAY_SIZE(bufferStatic);
      return NULL;
    }

    bufferSize = capacity;
  }

  return buffer;
}

// ============================================================================
// [Fog::RasterPainterDevice]
// ============================================================================

struct FOG_HIDDEN RasterPainterDevice : public PainterDevice
{
  // [Construction / Destruction]

  RasterPainterDevice(uint8_t* pixels, int width, int height, sysint_t stride, int format);
  virtual ~RasterPainterDevice();

  // [Meta]

  virtual int width() const;
  virtual int height() const;
  virtual int format() const;

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
  virtual void setSource(const Pattern& pattern);

  virtual Rgba sourceRgba();
  virtual Pattern sourcePattern();

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

  // [Flush]

  virtual void flush();

  // [Helpers]

  void _updateWorkRegion();

  void _setClipDefaults();
  void _setCapsDefaults();

  Raster::PatternContext* _getPatternContext();
  void _releasePatternContext(Raster::PatternContext* pctx);
  void _resetPatternContext();

  FOG_INLINE void _updateLineWidth()
  {
    ctx.capsState->lineIsSimple = 
      (ctx.capsState->lineWidth == 1.0 && ctx.capsState->lineDash.length() == 0);
  }

  bool _detachCaps();
  bool _detachClip();

  // [Serializers]
  //
  // Serializers are always called from painter thread.

  void _serializeGlyphSet(const Point& pt, const GlyphSet& glyphSet, const Rect* clip);
  void _serializeBoxes(const Box* box, sysuint_t count);
  void _serializePath(const Path& path, bool stroke);
  void _serializeImage(const Rect& dst, const Image& image, const Rect& src);

  // [Renderers]
  //
  // Renderers can be called from various threads (workers).

  void _renderGlyphSet(RasterPainterContext* ctx, const Point& pt, const GlyphSet& glyphSet, const Rect* clip);
  void _renderBoxes(RasterPainterContext* ctx, const Box* box, sysuint_t count);
  void _renderPath(RasterPainterContext* ctx, const Path& path, bool stroke);
  void _renderImage(RasterPainterContext* ctx, const Rect& dst, const Image& image, const Rect& src);

  // [Members]

  uint8_t* _metaRaster;
  sysint_t _stride;

  int _metaWidth;
  int _metaHeight;

  int _format;
  sysint_t _bpp;

  bool _compositingEnabled;
  bool _premultiplied;
  bool _multithreaded;

  // Temporary path
  Path workPath;

  // Antigrain context is declared outside of painter because we want that it
  // will be accessible through multiple threads. Each thread have it's own
  // context.
  RasterPainterContext ctx;
};

// ============================================================================
// [Fog::RasterPainterDevice - Construction / Destruction]
// ============================================================================

RasterPainterDevice::RasterPainterDevice(uint8_t* pixels, int width, int height, sysint_t stride, int format) :
  _metaRaster(pixels),
  _stride(stride),
  _metaWidth(width),
  _metaHeight(height),
  _format(format),
  _bpp(Image::formatToDepth(format) >> 3),
  _premultiplied(format == Image::FormatPRGB32),
  _multithreaded(false),
  _compositingEnabled(format == Image::FormatARGB32 || format == Image::FormatPRGB32),
  ctx(this)
{
  ctx.clipState = new(std::nothrow) RasterPainterClipState();
  ctx.capsState = new(std::nothrow) RasterPainterCapsState();

  // Setup clip state
  _setClipDefaults();

  // Setup caps state
  _setCapsDefaults();

  // Setup multithreading if possible
}

RasterPainterDevice::~RasterPainterDevice()
{
  flush();

  ctx.clipState->deref();
  ctx.capsState->deref();
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

int RasterPainterDevice::format() const
{
  return _format;
}

void RasterPainterDevice::setMetaVariables(
  const Point& metaOrigin,
  const Region& metaRegion,
  bool useMetaRegion,
  bool reset)
{
  if (!_detachClip()) return;

  ctx.clipState->metaOrigin = metaOrigin;
  ctx.clipState->metaRegion = metaRegion;
  ctx.clipState->metaRegionUsed = useMetaRegion;

  if (reset)
  {
    ctx.clipState->userOrigin.set(0, 0);
    ctx.clipState->userRegion.clear();
    ctx.clipState->userRegionUsed = false;

    _setCapsDefaults();
  }

  _updateWorkRegion();
}

void RasterPainterDevice::setMetaOrigin(const Point& pt)
{
  if (ctx.clipState->metaOrigin == pt) return;
  if (!_detachClip()) return;

  ctx.clipState->metaOrigin = pt;
  _updateWorkRegion();
}

void RasterPainterDevice::setUserOrigin(const Point& pt)
{
  if (ctx.clipState->userOrigin == pt) return;
  if (!_detachClip()) return;

  ctx.clipState->userOrigin = pt;
  _updateWorkRegion();
}

void RasterPainterDevice::translateMetaOrigin(const Point& pt)
{
  if (pt.x() == 0 && pt.y() == 0) return;
  if (!_detachClip()) return;

  ctx.clipState->metaOrigin += pt;
  _updateWorkRegion();
}

void RasterPainterDevice::translateUserOrigin(const Point& pt)
{
  if (pt.x() == 0 && pt.y() == 0) return;
  if (!_detachClip()) return;

  ctx.clipState->userOrigin += pt;
  _updateWorkRegion();
}

void RasterPainterDevice::setUserRegion(const Rect& r)
{
  if (!_detachClip()) return;

  ctx.clipState->userRegion = r;
  ctx.clipState->userRegionUsed = true;
  _updateWorkRegion();
}

void RasterPainterDevice::setUserRegion(const Region& r)
{
  if (!_detachClip()) return;

  ctx.clipState->userRegion = r;
  ctx.clipState->userRegionUsed = true;
  _updateWorkRegion();
}

void RasterPainterDevice::resetMetaVars()
{
  if (!_detachClip()) return;

  ctx.clipState->metaOrigin.set(0, 0);
  ctx.clipState->metaRegion.clear();
  ctx.clipState->metaRegionUsed = false;
  _updateWorkRegion();
}

void RasterPainterDevice::resetUserVars()
{
  if (!_detachClip()) return;

  ctx.clipState->userOrigin.set(0, 0);
  ctx.clipState->userRegion.clear();
  ctx.clipState->userRegionUsed = false;
  _updateWorkRegion();
}

Point RasterPainterDevice::metaOrigin() const
{
  return ctx.clipState->metaOrigin;
}

Point RasterPainterDevice::userOrigin() const
{
  return ctx.clipState->userOrigin;
}

Region RasterPainterDevice::metaRegion() const
{
  return ctx.clipState->metaRegion;
}

Region RasterPainterDevice::userRegion() const
{
  return ctx.clipState->userRegion;
}

bool RasterPainterDevice::usedMetaRegion() const
{
  return ctx.clipState->metaRegionUsed;
}

bool RasterPainterDevice::usedUserRegion() const
{
  return ctx.clipState->userRegionUsed;
}

// ============================================================================
// [Fog::RasterPainterDevice - Operator]
// ============================================================================

void RasterPainterDevice::setOp(uint32_t op)
{
  if (!_compositingEnabled) return;
  if (op >= CompositeCount) return;
  if (!_detachCaps()) return;

  ctx.capsState->op = op;
  ctx.raster = &Raster::functionMap->raster_argb32[_premultiplied][op];
}

uint32_t RasterPainterDevice::op() const
{
  return ctx.capsState->op;
}

// ============================================================================
// [Fog::RasterPainterDevice - Source]
// ============================================================================

void RasterPainterDevice::setSource(const Rgba& rgba)
{
  if (!_detachCaps()) return;

  ctx.capsState->solidSource = rgba;
  ctx.capsState->solidSourcePremultiplied = Raster::premultiply(rgba);

  ctx.capsState->isSolidSource = true;

  // Free pattern resource if not needed.
  if (!ctx.capsState->patternSource.isNull())
  {
    ctx.capsState->patternSource.free();
    _resetPatternContext();
  }
}

void RasterPainterDevice::setSource(const Pattern& pattern)
{
  if (pattern.isSolid())
  {
    setSource(pattern.color());
    return;
  }

  if (!_detachCaps()) return;

  ctx.capsState->solidSource = 0xFFFFFFFF;
  ctx.capsState->solidSourcePremultiplied = 0xFFFFFFFF;
  ctx.capsState->isSolidSource = false;
  ctx.capsState->patternSource = pattern;
  _resetPatternContext();
}

Rgba RasterPainterDevice::sourceRgba()
{
  return ctx.capsState->solidSource;
}

Pattern RasterPainterDevice::sourcePattern()
{
  Pattern pattern;
  if (ctx.capsState->isSolidSource)
    pattern.setColor(ctx.capsState->solidSource);
  else
    pattern = ctx.capsState->patternSource;
  return pattern;
}

// ============================================================================
// [Fog::RasterPainterDevice - Parameters]
// ============================================================================

void RasterPainterDevice::setLineWidth(double lineWidth)
{
  if (ctx.capsState->lineWidth == lineWidth) return;
  if (!_detachCaps()) return;

  ctx.capsState->lineWidth = lineWidth;
  _updateLineWidth();
}

double RasterPainterDevice::lineWidth() const
{
  return ctx.capsState->lineWidth;
}

void RasterPainterDevice::setLineCap(uint32_t lineCap)
{
  if (ctx.capsState->lineCap == lineCap) return;
  if (!_detachCaps()) return;

  ctx.capsState->lineCap = lineCap;
}

uint32_t RasterPainterDevice::lineCap() const
{
  return ctx.capsState->lineCap;
}

void RasterPainterDevice::setLineJoin(uint32_t lineJoin)
{
  if (ctx.capsState->lineJoin == lineJoin) return;
  if (!_detachCaps()) return;

  ctx.capsState->lineJoin = lineJoin;
}

uint32_t RasterPainterDevice::lineJoin() const
{
  return ctx.capsState->lineJoin;
}

void RasterPainterDevice::setLineDash(const double* dashes, sysuint_t count)
{
  if (!_detachCaps()) return;

  ctx.capsState->lineDash.clear();
  for (sysuint_t i = 0; i < count; i++) ctx.capsState->lineDash.append(*dashes);
  _updateLineWidth();
}

void RasterPainterDevice::setLineDash(const Vector<double>& dashes)
{
  if (!_detachCaps()) return;

  ctx.capsState->lineDash = dashes;
  _updateLineWidth();
}

Vector<double> RasterPainterDevice::lineDash() const
{
  return ctx.capsState->lineDash;
}

void RasterPainterDevice::setLineDashOffset(double offset)
{
  if (ctx.capsState->lineDashOffset == offset) return;
  if (!_detachCaps()) return;

  ctx.capsState->lineDashOffset = offset;
  _updateLineWidth();
}

double RasterPainterDevice::lineDashOffset() const
{
  return ctx.capsState->lineDashOffset;
}

void RasterPainterDevice::setMiterLimit(double miterLimit)
{
  if (ctx.capsState->miterLimit == miterLimit) return;
  if (!_detachCaps()) return;

  ctx.capsState->miterLimit = miterLimit;
}

double RasterPainterDevice::miterLimit() const
{
  return ctx.capsState->miterLimit;
}

void RasterPainterDevice::setFillMode(uint32_t mode)
{
  if (ctx.capsState->fillMode == mode) return;
  if (!_detachCaps()) return;

  ctx.capsState->fillMode = mode;
}

uint32_t RasterPainterDevice::fillMode()
{
  return ctx.capsState->fillMode;
}

// ============================================================================
// [Fog::RasterPainterDevice - Transformations]
// ============================================================================

void RasterPainterDevice::setMatrix(const AffineMatrix& m)
{
  if (!_detachCaps()) return;

  ctx.capsState->transformations = m;
  ctx.capsState->transformationsUsed = !ctx.capsState->transformations.isIdentity();
}

void RasterPainterDevice::resetMatrix()
{
  if (!_detachCaps()) return;

  ctx.capsState->transformations = AffineMatrix();
  ctx.capsState->transformationsUsed = false;
}

AffineMatrix RasterPainterDevice::matrix() const
{
  return ctx.capsState->transformations;
}

void RasterPainterDevice::rotate(double angle)
{
  if (!_detachCaps()) return;

  ctx.capsState->transformations *= AffineMatrix::fromRotation(angle);
  ctx.capsState->transformationsUsed = !ctx.capsState->transformations.isIdentity();
}

void RasterPainterDevice::scale(double sx, double sy)
{
  if (!_detachCaps()) return;

  ctx.capsState->transformations *= AffineMatrix::fromScale(sx, sy);
  ctx.capsState->transformationsUsed = !ctx.capsState->transformations.isIdentity();
}

void RasterPainterDevice::skew(double sx, double sy)
{
  if (!_detachCaps()) return;

  ctx.capsState->transformations *= AffineMatrix::fromSkew(sx, sy);
  ctx.capsState->transformationsUsed = !ctx.capsState->transformations.isIdentity();
}

void RasterPainterDevice::translate(double x, double y)
{
  if (!_detachCaps()) return;

  ctx.capsState->transformations *= AffineMatrix::fromTranslation(x, y);
  ctx.capsState->transformationsUsed = !ctx.capsState->transformations.isIdentity();
}

void RasterPainterDevice::affine(const AffineMatrix& m)
{
  if (!_detachCaps()) return;

  ctx.capsState->transformations *= m;
  ctx.capsState->transformationsUsed = !ctx.capsState->transformations.isIdentity();
}

void RasterPainterDevice::parallelogram(
  double x1, double y1, double x2, double y2, const double* para)
{
  if (!_detachCaps()) return;

  ctx.capsState->transformations *= AffineMatrix(x1, y1, x2, y2, para);
  ctx.capsState->transformationsUsed = !ctx.capsState->transformations.isIdentity();
}

void RasterPainterDevice::viewport(
  double worldX1,  double worldY1,  double worldX2,  double worldY2,
  double screenX1, double screenY1, double screenX2, double screenY2,
  uint32_t viewportOption)
{
  if (!_detachCaps()) return;

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
  ctx.capsState->transformations *= *((const AffineMatrix *)&aff);
  ctx.capsState->transformationsUsed = !ctx.capsState->transformations.isIdentity();
}

// ============================================================================
// [Fog::RasterPainterDevice - Raster drawing]
// ============================================================================

void RasterPainterDevice::clear()
{
  if (ctx.clipState->clipSimple)
    _serializeBoxes(&ctx.clipState->clipBox, 1);
  else
    _serializeBoxes(ctx.clipState->workRegion.cData(), ctx.clipState->workRegion.count());
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

  if (ctx.capsState->transformationsUsed || !ctx.capsState->lineIsSimple)
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

  if (ctx.clipState->clipSimple)
  {
    if (!ctx.clipState->clipBox.subsumes(r))
    {
      for (sysuint_t i = 0; i < count; i++) Box::intersect(box[0], box[0], ctx.clipState->clipBox);
    }
    RasterPainterDevice::_serializeBoxes(box, count);
  }
  else
  {
    TemporaryRegion<4> regionBox;
    TemporaryRegion<16> regionISect;
    regionBox.set(box, count);

    Region::intersect(regionISect, regionBox, ctx.clipState->workRegion);
    if (!regionISect.count()) return;

    RasterPainterDevice::_serializeBoxes(regionISect.cData(), regionISect.count());
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

  if (ctx.capsState->transformationsUsed)
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

  if (ctx.clipState->clipSimple)
  {
    Box::intersect(box, box, ctx.clipState->clipBox);
    if (!box.isValid()) return;

    RasterPainterDevice::_serializeBoxes(&box, 1);
  }
  else
  {
    TemporaryRegion<1> regionBox;
    TemporaryRegion<16> regionISect;
    regionBox.set(&box, 1);

    Region::intersect(regionISect, regionBox, ctx.clipState->workRegion);
    if (!regionISect.count()) return;

    RasterPainterDevice::_serializeBoxes(regionISect.cData(), regionISect.count());
  }
}

void RasterPainterDevice::fillRects(const Rect* r, sysuint_t count)
{
  if (!count) return;

  if (ctx.capsState->transformationsUsed)
  {
    workPath.clear();
    for (sysuint_t i = 0; i < count; i++)
    {
      if (r[i].isValid()) workPath.addRect(
        RectF(
          (double)r[i].x1() + 0.5,
          (double)r[i].y1() + 0.5,
          (double)r[i].width(),
          (double)r[i].height()));
    }
    fillPath(workPath);
    return;
  }

  Region region;
  region.set(r, count);
  if (!region.count()) return;

  if (ctx.clipState->clipSimple)
  {
    if (ctx.clipState->clipBox.subsumes(region.extents()))
    {
      _serializeBoxes(region.cData(), region.count());
      return;
    }
    region.intersect(ctx.clipState->clipBox);
    if (!region.count()) return;

    _serializeBoxes(region.cData(), region.count());
  }
  else
  {
    Region regionISect;
    Region::intersect(regionISect, ctx.clipState->workRegion, region);
    if (!regionISect.count()) return;

    _serializeBoxes(regionISect.cData(), regionISect.count());
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
  if (ctx.clipState->clipSimple && ctx.clipState->clipBox.subsumes(region.extents()))
  {
    _serializeBoxes(region.cData(), region.count());
  }
  else
  {
    TemporaryRegion<16> dst;
    Region::intersect(dst, ctx.clipState->workRegion, region);
    if (!dst.count()) return;

    _serializeBoxes(dst.cData(), dst.count());
  }
}

// ============================================================================
// [Fog::RasterPainterDevice - Vector drawing]
// ============================================================================

void RasterPainterDevice::drawPoint(const PointF& p)
{
  workPath.clear();
  workPath.moveTo(p);
  workPath.lineTo(p.x(), p.y() + 0.0001);
  drawPath(workPath);
}

void RasterPainterDevice::drawLine(const PointF& start, const PointF& end)
{
  workPath.clear();
  workPath.moveTo(start);
  workPath.lineTo(end);
  drawPath(workPath);
}

void RasterPainterDevice::drawLine(const PointF* pts, sysuint_t count)
{
  if (!count) return;

  workPath.clear();
  workPath.moveTo(pts[0]);
  if (count > 1)
    workPath.addLineTo(pts + 1, count - 1);
  else
    workPath.lineTo(pts[0].x(), pts[0].y() + 0.0001);
  drawPath(workPath);
}

void RasterPainterDevice::drawPolygon(const PointF* pts, sysuint_t count)
{
  if (!count) return;

  workPath.clear();
  workPath.moveTo(pts[0]);
  if (count > 1)
    workPath.addLineTo(pts + 1, count - 1);
  else
    workPath.lineTo(pts[0].x(), pts[0].y() + 0.0001);
  workPath.closePolygon();
  drawPath(workPath);
}

void RasterPainterDevice::drawRect(const RectF& r)
{
  if (!r.isValid()) return;

  workPath.clear();
  workPath.addRect(r);
  drawPath(workPath);
}

void RasterPainterDevice::drawRects(const RectF* r, sysuint_t count)
{
  if (!count) return;

  workPath.clear();
  for (sysuint_t i = 0; i < count; i++)
  {
    if (r[i].isValid()) workPath.addRect(r[i]);
  }
  drawPath(workPath);
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

  workPath.clear();
  concatToPath(workPath, rc, 0);
  drawPath(workPath);
}

void RasterPainterDevice::drawEllipse(const PointF& cp, const PointF& r)
{
  RasterPainterDevice::drawArc(cp, r, 0.0, 2.0 * M_PI);
}

void RasterPainterDevice::drawArc(const PointF& cp, const PointF& r, double start, double sweep)
{
  agg::bezier_arc arc(cp.x(), cp.y(), r.x(), r.y(), start, sweep);

  workPath.clear();
  concatToPath(workPath, arc, 0);
  drawPath(workPath);
}

void RasterPainterDevice::drawPath(const Path& path)
{
  _serializePath(path, true);
}

void RasterPainterDevice::fillPolygon(const PointF* pts, sysuint_t count)
{
  if (!count) return;

  workPath.clear();
  workPath.moveTo(pts[0]);
  if (count > 1)
    workPath.addLineTo(pts + 1, count - 1);
  else
    workPath.lineTo(pts[0].x(), pts[0].y() + 0.0001);
  workPath.closePolygon();
  fillPath(workPath);
}

void RasterPainterDevice::fillRect(const RectF& r)
{
  if (!r.isValid()) return;

  workPath.clear();
  workPath.addRect(r);
  fillPath(workPath);
}

void RasterPainterDevice::fillRects(const RectF* r, sysuint_t count)
{
  if (!count) return;

  workPath.clear();
  for (sysuint_t i = 0; i < count; i++)
  {
    if (r[i].isValid()) workPath.addRect(r[i]);
  }
  fillPath(workPath);
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

  workPath.clear();
  concatToPath(workPath, rc, 0);
  fillPath(workPath);
}

void RasterPainterDevice::fillEllipse(const PointF& cp, const PointF& r)
{
  RasterPainterDevice::fillArc(cp, r, 0.0, 2.0 * M_PI);
}

void RasterPainterDevice::fillArc(const PointF& cp, const PointF& r, double start, double sweep)
{
  agg::bezier_arc arc(cp.x(), cp.y(), r.x(), r.y(), start, sweep);

  workPath.clear();
  concatToPath(workPath, arc, 0);
  fillPath(workPath);
}

void RasterPainterDevice::fillPath(const Path& path)
{
  _serializePath(path, false);
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

  _serializeGlyphSet(pt, glyphSet, clip);
}

void RasterPainterDevice::drawGlyphSet(const Point& pt, const GlyphSet& glyphSet, const Rect* clip)
{
  _serializeGlyphSet(pt, glyphSet, clip);
}

void RasterPainterDevice::drawText(const Point& pt, const String32& text, const Font& font, const Rect* clip)
{
  TemporaryGlyphSet<128> glyphSet;
  if (font.getGlyphs(text.cData(), text.length(), glyphSet)) return;

  _serializeGlyphSet(pt, glyphSet, clip);
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
  int srcx = 0;
  int srcy = 0;
  int dstx = p.x();
  int dsty = p.y();
  int dstw;
  int dsth;

  if (irect == NULL)
  {
    dstw = image.width();
    if (dstw == 0) return;
    dsth = image.height();
    if (dsth == 0) return;
  }
  else
  {
    if (!irect->isValid()) return;

    srcx = irect->x1();
    if (srcx < 0) return;
    srcy = irect->y1();
    if (srcy < 0) return;

    dstw = fog_min(image.width(), irect->width());
    if (dstw == 0) return;
    dsth = fog_min(image.height(), irect->height());
    if (dsth == 0) return;
  }

  int d;

  if ((uint)(d = dstx - ctx.clipState->clipBox.x1()) >= (uint)ctx.clipState->clipBox.width())
  {
    if (d < 0)
    {
      if ((dstw += d) <= 0) return;
      dstx = 0;
      srcx = -d;
    }
    else
    {
      return;
    }
  }

  if ((uint)(d = dsty - ctx.clipState->clipBox.y1()) >= (uint)ctx.clipState->clipBox.height())
  {
    if (d < 0)
    {
      if ((dsth += d) <= 0) return;
      dsty = 0;
      srcy = -d;
    }
    else
    {
      return;
    }
  }

  if ((d = ctx.clipState->clipBox.x2() - dstx) < dstw) dstw = d;
  if ((d = ctx.clipState->clipBox.y2() - dsty) < dsth) dsth = d;

  Rect dst(dstx, dsty, dstw, dsth);
  Rect src(srcx, srcy, dstw, dsth);
  _serializeImage(dst, image, src);
}

// ============================================================================
// [Fog::RasterPainterDevice - Flush]
// ============================================================================

void RasterPainterDevice::flush()
{
}

// ============================================================================
// [Fog::RasterPainterDevice - Helpers]
// ============================================================================

void RasterPainterDevice::_updateWorkRegion()
{
  FOG_ASSERT(ctx.clipState->refCount.get() == 1);

  ctx.clipState->workOrigin = ctx.clipState->metaOrigin + ctx.clipState->userOrigin;

  int negx = -ctx.clipState->workOrigin.x();
  int negy = -ctx.clipState->workOrigin.y();

  // This is maximal clip box that can be used by painter.
  ctx.clipState->clipBox.set(negx, negy, negx + _metaWidth, negy + _metaHeight);
  ctx.clipState->clipSimple = true;

  // Do region calculations only if they are really used.
  if (ctx.clipState->metaRegionUsed || ctx.clipState->userRegionUsed)
  {
    if (ctx.clipState->metaRegionUsed)
    {
      Region::translate(ctx.clipState->workRegion, ctx.clipState->metaRegion, Point(negx, negy));
    }
    else
    {
      ctx.clipState->workRegion.set(Box(negx, negy, negx + _metaWidth, negy + _metaHeight));
    }

    if (ctx.clipState->userRegionUsed)
    {
      if (ctx.clipState->metaOrigin.x() || ctx.clipState->metaOrigin.y())
      {
        TemporaryRegion<64> userTmp;
        Region::translate(userTmp, ctx.clipState->userRegion, ctx.clipState->metaOrigin.negated());
        Region::subtract(ctx.clipState->workRegion, ctx.clipState->workRegion, userTmp);
      }
      else
      {
        // Fast path.
        Region::subtract(
          ctx.clipState->workRegion,
          ctx.clipState->workRegion,
          ctx.clipState->userRegion);
      }
    }

    // Switch to box clip implementation if resulting region is simple.
    if (ctx.clipState->workRegion.count() == 1)
    {
      ctx.clipState->clipBox.set(ctx.clipState->workRegion.extents());
      ctx.clipState->workRegion.clear();
      ctx.clipState->workRegionUsed = false;
    }
    else
    {
      ctx.clipState->clipSimple = false;
      ctx.clipState->clipBox.set(ctx.clipState->workRegion.extents());
      ctx.clipState->workRegionUsed = true;
    }
  }
  else
  {
    // There is no clip box, we will use painter bounds that was in _clipBox
    // already
    ctx.clipState->workRegion.clear();
    ctx.clipState->workRegionUsed = false;
  }

  // This is very good trick. Make raster -relative to _workOrigin, so no
  // calculations are needed to draw pixels relative to _workOrigin.
  ctx.clipState->workRaster = _metaRaster +
    (sysint_t)ctx.clipState->workOrigin.x() * _bpp +
    (sysint_t)ctx.clipState->workOrigin.y() * _stride;
}

void RasterPainterDevice::_setClipDefaults()
{
  FOG_ASSERT(ctx.clipState->refCount.get() == 1);

  ctx.clipState->workRaster = _metaRaster;
  ctx.clipState->metaOrigin.set(0, 0);
  ctx.clipState->userOrigin.set(0, 0);
  ctx.clipState->workOrigin.set(0, 0);
  ctx.clipState->metaRegionUsed = false;
  ctx.clipState->userRegionUsed = false;
  ctx.clipState->workRegionUsed = false;
  ctx.clipState->clipSimple = true;
  ctx.clipState->clipBox.set(0, 0, _metaWidth, _metaHeight);
}

void RasterPainterDevice::_setCapsDefaults()
{
  FOG_ASSERT(ctx.capsState->refCount.get() == 1);

  ctx.capsState->op = CompositeOver;
  ctx.capsState->solidSource = 0xFFFFFFFF;
  ctx.capsState->solidSourcePremultiplied = 0xFFFFFFFF;
  ctx.capsState->patternSource.free();
  ctx.capsState->isSolidSource = true;

  ctx.capsState->lineWidth = 1.0;
  ctx.capsState->lineIsSimple = true;

  ctx.capsState->lineCap = LineCapRound;
  ctx.capsState->lineJoin = LineJoinRound;

  ctx.capsState->lineDash.free();
  ctx.capsState->lineDashOffset = 0.0;

  ctx.capsState->miterLimit = 1.0;

  ctx.capsState->fillMode = FillNonZero;

  ctx.capsState->transformations = AffineMatrix();
  ctx.capsState->transformationsUsed = false;

  ctx.raster = Raster::getRasterOps(_format, ctx.capsState->op);
}

Raster::PatternContext* RasterPainterDevice::_getPatternContext()
{
  // Sanity, calling _getPatternContext() for solid source is invalid.
  if (ctx.capsState->isSolidSource) return NULL;

  Raster::PatternContext* pctx = ctx.pctx;
  err_t err = Error::Ok;

  if (!pctx)
  {
    pctx = ctx.pctx = (Raster::PatternContext*)Memory::alloc(sizeof(Raster::PatternContext));
    if (!pctx) return NULL;
    pctx->initialized = false;
  }

  if (!pctx->initialized)
  {
    switch (ctx.capsState->patternSource.type())
    {
      case Pattern::Texture:
        err = Raster::functionMap->pattern.texture_init(pctx, ctx.capsState->patternSource);
        break;
      case Pattern::LinearGradient:
        err = Raster::functionMap->pattern.linear_gradient_init(pctx, ctx.capsState->patternSource);
        break;
      default:
        FOG_ASSERT_NOT_REACHED();
    }
  }

  if (err)
  {
    fog_debug("Fog::RasterPainterDevice::_getPatternContext() - Failed to get context (error %d)\n", err);
    return NULL;
  }

  // Be sure that pattern context is always marked as initialized.
  FOG_ASSERT(pctx->initialized);

  return pctx;
}

void RasterPainterDevice::_releasePatternContext(Raster::PatternContext* pctx)
{
  FOG_ASSERT(pctx != NULL);

  if (pctx->destroy) pctx->destroy(pctx);
  Memory::free(pctx);

  if (ctx.pctx == pctx) ctx.pctx = NULL;
}

void RasterPainterDevice::_resetPatternContext()
{
  if (ctx.pctx && ctx.pctx->initialized)
    ctx.pctx->destroy(ctx.pctx);
}

bool RasterPainterDevice::_detachClip()
{
  if (ctx.clipState->refCount.get() == 1) return true;

  RasterPainterClipState* newd = new(std::nothrow) RasterPainterClipState(*ctx.clipState);
  if (newd == NULL) return false;

  AtomicBase::ptr_setXchg(&ctx.clipState, newd)->deref();
  return true;
}

bool RasterPainterDevice::_detachCaps()
{
  if (ctx.capsState->refCount.get() == 1) return true;

  RasterPainterCapsState* newd = new(std::nothrow) RasterPainterCapsState(*ctx.capsState);
  if (newd == NULL) return false;

  AtomicBase::ptr_setXchg(&ctx.capsState, newd)->deref();
  return true;
}

// ============================================================================
// [Fog::RasterPainterDevice - Serializers]
// ============================================================================

void RasterPainterDevice::_serializeGlyphSet(const Point& pt, const GlyphSet& glyphSet, const Rect* clip)
{
  // Pattern context must be always set up before _render() methods are called.
  if (!ctx.capsState->isSolidSource && !_getPatternContext()) return;

  if (!_multithreaded)
  {
    _renderGlyphSet(&ctx, pt, glyphSet, clip);
  }
  else
  {
  }
}

void RasterPainterDevice::_serializeBoxes(const Box* box, sysuint_t count)
{
  // Pattern context must be always set up before _render() methods are called.
  if (!ctx.capsState->isSolidSource && !_getPatternContext()) return;

  if (!_multithreaded)
  {
    _renderBoxes(&ctx, box, count);
  }
  else
  {
  }
}

void RasterPainterDevice::_serializePath(const Path& path, bool stroke)
{
  // Pattern context must be always set up before _render() methods are called.
  if (!ctx.capsState->isSolidSource && !_getPatternContext()) return;

  if (!_multithreaded)
  {
    _renderPath(&ctx, path, stroke);
  }
  else
  {
  }
}

void RasterPainterDevice::_serializeImage(const Rect& dst, const Image& image, const Rect& src)
{
  if (!_multithreaded)
  {
    _renderImage(&ctx, dst, image, src);
  }
  else
  {
  }
}

// ============================================================================
// [Fog::RasterPainterDevice - Renderers]
// ============================================================================

void RasterPainterDevice::_renderGlyphSet(
  RasterPainterContext* ctx,
  const Point& pt, const GlyphSet& glyphSet, const Rect* clip)
{
  // TODO: Hardcoded to A8 glyph format
  // TODO: Clipping

  Box clipBox = ctx->clipState->clipBox;
  if (clip) Box::intersect(clipBox, clipBox, Box(*clip));

  if (!clipBox.isValid()) return;
  if (!glyphSet.length()) return;

  const Glyph* glyphs = glyphSet.glyphs();
  sysuint_t count = glyphSet.length();

  int px = pt.x();
  int py = pt.y();

  uint8_t* pBuf = ctx->clipState->workRaster;
  sysint_t stride = _stride;

  Raster::SpanSolidMskFn span_solid_a8 = ctx->raster->span_solid_a8;

  // Used only if source is solid
  uint32_t rgba = ctx->capsState->solidSourcePremultiplied;

  // Used only if source is pattern
  Raster::PatternContext* pctx;
  uint8_t* pbuf;
  Raster::SpanCompositeMskFn span_composite_a8;

  if (!ctx->capsState->isSolidSource)
  {
    pctx = ctx->pctx;
    if (!pctx) return;

    pbuf = ctx->getBuffer(Raster::mul4(ctx->clipState->clipBox.width()));
    if (!pbuf) return;

    span_composite_a8 = ctx->raster->span_composite_a8[pctx->format];
  }

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

    uint8_t* pCur = pBuf;
    pCur += (sysint_t)y1 * stride;
    pCur += (sysint_t)x1 * 4;

    // TODO: Hardcoded
    sysint_t glyphStride = glyphd->image.stride();
    const uint8_t* pGlyph = glyphd->image.cData();

    pGlyph += (sysint_t)(y1 - py1) * glyphStride;
    pGlyph += (sysint_t)(x1 - px1);

    if (ctx->capsState->isSolidSource)
    {
      do {
        span_solid_a8(pCur, rgba, pGlyph, (sysuint_t)w);
        pCur += stride;
        pGlyph += glyphStride;
      } while (--h);
    }
    else
    {
      do {
        span_composite_a8(pCur, 
          pctx->fetch(pctx, pbuf, x1, y1, w),
          pGlyph, (sysuint_t)w);

        pCur += stride;
        pGlyph += glyphStride;
        y1++;
      } while (--h);
    }
  }
}

void RasterPainterDevice::_renderBoxes(
  RasterPainterContext* ctx,
  const Box* box, sysuint_t count)
{
  if (!count) return;

  uint8_t* pBuf = ctx->clipState->workRaster;
  sysint_t stride = _stride;
  sysint_t bpp = _bpp;

  if (ctx->capsState->isSolidSource)
  {
    uint32_t rgba = ctx->capsState->solidSourcePremultiplied;
    Raster::SpanSolidFn span_solid = ctx->raster->span_solid;

    for (sysuint_t i = 0; i < count; i++)
    {
      sysint_t x = box[i].x1();
      sysint_t y = box[i].y1();

      sysint_t w = box[i].width();
      if (w <= 0) continue;
      sysint_t h = box[i].height();
      if (h <= 0) continue;

      uint8_t* pCur = pBuf + y * stride + x * bpp;
      do {
        span_solid(pCur, rgba, (sysuint_t)w);
        pCur += stride;
      } while (--h);
    }
  }
  else
  {
    Raster::PatternContext* pctx = ctx->pctx;
    if (!pctx) return;

    uint8_t* pbuf = ctx->getBuffer(Raster::mul4(ctx->clipState->clipBox.width()));
    if (!pbuf) return;

    Raster::SpanCompositeFn span_composite = ctx->raster->span_composite[pctx->format];

    for (sysuint_t i = 0; i < count; i++)
    {
      sysint_t x = box[i].x1();
      sysint_t y = box[i].y1();

      sysint_t w = box[i].width();
      if (w <= 0) continue;
      sysint_t h = box[i].height();
      if (h <= 0) continue;

      uint8_t* pCur = pBuf + y * stride + x * bpp;
      do {
        span_composite(pCur, 
          pctx->fetch(pctx, pbuf, x, y, w),
          (sysuint_t)w);
        pCur += stride;
        y++;
      } while (--h);
    }
  }
}


template <typename PathT>
static void FOG_INLINE AggSetupStroke(PathT& path, RasterPainterCapsState* capsState)
{
  path.width(capsState->lineWidth);
  path.line_join(static_cast<agg::line_join_e>(capsState->lineJoin));
  path.line_cap(static_cast<agg::line_cap_e>(capsState->lineCap));
  path.miter_limit(capsState->miterLimit);
}

template <typename PathT>
static void FOG_INLINE AggSetupDash(PathT& path, RasterPainterCapsState* capsState)
{
  Vector<double>::ConstIterator it(capsState->lineDash);
  
  for (;;)
  {
    double d1 = it.value(); it.toNext();
    if (!it.isValid()) break;
    double d2 = it.value(); it.toNext();
    path.add_dash(d1, d2);
    if (!it.isValid()) break;
  }

  path.dash_start(capsState->lineDashOffset);
}

static void FOG_FASTCALL AggRasterizePath(
  RasterPainterContext* ctx,
  const Path& path, bool stroke)
{
  RasterPainterClipState* clipState = ctx->clipState;
  RasterPainterCapsState* capsState = ctx->capsState;

  ctx->ras.reset();
  ctx->ras.filling_rule(static_cast<agg::filling_rule_e>(capsState->fillMode));
  ctx->ras.clip_box(
    (double)clipState->clipBox.x1(),
    (double)clipState->clipBox.y1(),
    (double)clipState->clipBox.x2(),
    (double)clipState->clipBox.y2());

  AggPath pAgg(path);
  AggConvCurve pCurve(pAgg);

  // This can be a bit messy, but it's here to increase performance. We will
  // not calculate using transformations if they are not used. Also we add
  // stroke and line dash pipeline only if it's needed. This is goal of 
  // AntiGrain to be able to setup only pipelines what are really need.
  if (capsState->transformationsUsed)
  {
    if (stroke)
    {
      if (capsState->lineDash.length() <= 1)
      {
        AggConvCurveStroke pCurveStroke(pCurve);
        AggSetupStroke(pCurveStroke, capsState);
        AggConvCurveStrokeTransform strokeTransform(
          pCurveStroke,
          *((const agg::trans_affine *)&capsState->transformations));
        ctx->ras.add_path(strokeTransform);
      }
      else
      {
        AggConvCurveDash pCurveDash(pCurve);
        AggSetupDash(pCurveDash, capsState);
        AggConvCurveDashStroke pCurveDashStroke(pCurveDash);
        AggSetupStroke(pCurveDashStroke, capsState);
        AggConvCurveDashStrokeTransform pCurveDashStrokeTransform(
          pCurveDashStroke,
          *((const agg::trans_affine *)&capsState->transformations));
        ctx->ras.add_path(pCurveDashStrokeTransform);
      }
    }
    else
    {
      AggConvCurveTransform pCurveTransform(
        pCurve, *((agg::trans_affine *)&capsState->transformations));

      ctx->ras.add_path(pCurveTransform);
    }
  }
  else
  {
    if (stroke)
    {
      if (capsState->lineDash.length() <= 1)
      {
        AggConvCurveStroke pCurveStroke(pCurve);
        AggSetupStroke(pCurveStroke, capsState);
        ctx->ras.add_path(pCurveStroke);
      }
      else
      {
        AggConvCurveDash pCurveDash(pCurve);
        AggSetupDash(pCurveDash, capsState);
        AggConvCurveDashStroke pCurveDashStroke(pCurveDash);
        AggSetupStroke(pCurveDashStroke, capsState);
        ctx->ras.add_path(pCurveDashStroke);
      }
    }
    else
    {
      ctx->ras.add_path(pCurve);
    }
  }
}

template<int BytesPerPixel, class Rasterizer, class Scanline>
static void FOG_FASTCALL AggRenderScanlines(
  RasterPainterDevice* d,
  RasterPainterContext* ctx, Rasterizer& ras, Scanline& sl)
{
  if (!ras.rewind_scanlines()) return;

  RasterPainterClipState* clipState = ctx->clipState;
  RasterPainterCapsState* capsState = ctx->capsState;

  uint8_t* pBase = clipState->workRaster;
  sysint_t stride = d->_stride;

  sl.reset(ras.min_x(), ras.max_x());

  Raster::SpanSolidFn span_solid = ctx->raster->span_solid;
  Raster::SpanSolidMskFn span_solid_a8 = ctx->raster->span_solid_a8;

  // solid source
  if (capsState->isSolidSource)
  {
    uint32_t solidColor = capsState->solidSourcePremultiplied;

    while (ras.sweep_scanline(sl))
    {
      unsigned num_spans = sl.num_spans();
      typename Scanline::const_iterator span = sl.begin();

      sysint_t y = sl.y();
      uint8_t* pRas = pBase + y * stride;
      uint8_t* pCur;

      for (;;)
      {
        int x = span->x;
        int len = span->len;

        pCur = pRas + Raster::mul<int, BytesPerPixel>(x);

        if (len > 0)
        {
          span_solid_a8(pCur, solidColor, span->covers, (unsigned)len);
        }
        else
        {
          len = -len;
          FOG_ASSERT(len > 0);

          uint32_t cover = (uint32_t)*(span->covers);
          if (cover == 0xFF)
          {
            span_solid(pCur, solidColor, len);
          }
          else
          {
            span_solid(pCur, Raster::bytemul(solidColor, cover), len);
          }
        }

        if (--num_spans == 0) break;
        ++span;
      }
    }
  }
  else
  {
    Raster::PatternContext* pctx = ctx->pctx;
    if (!pctx) return;

    uint8_t* pbuf = ctx->getBuffer(Raster::mul4(clipState->clipBox.width()));
    if (!pbuf) return;

    Raster::SpanCompositeFn span_composite = ctx->raster->span_composite[pctx->format];
    Raster::SpanCompositeMskFn span_composite_a8 = ctx->raster->span_composite_a8[pctx->format];

    while (ras.sweep_scanline(sl))
    {
      unsigned num_spans = sl.num_spans();
      typename Scanline::const_iterator span = sl.begin();

      sysint_t y = sl.y();
      uint8_t* pRas = pBase + y * stride;
      uint8_t* pCur;

      for (;;)
      {
        int x = span->x;
        int len = span->len;

        pCur = pRas + Raster::mul<int, BytesPerPixel>(x);

        if (len > 0)
        {
          span_composite_a8(pCur, 
            pctx->fetch(pctx, pbuf, x, y, len),
            span->covers, len);
        }
        else
        {
          len = -len;
          FOG_ASSERT(len > 0);

          uint32_t cover = (uint32_t)*(span->covers);
          if (cover == 0xFF)
          {
            span_composite(pCur,
              pctx->fetch(pctx, pbuf, x, y, len),
              len);
          }
          else
          {
            // TODO
          }
        }

        if (--num_spans == 0) break;
        ++span;
      }
    }
  }
}

void RasterPainterDevice::_renderPath(
  RasterPainterContext* ctx,
  const Path& path, bool stroke)
{
  AggRasterizePath(ctx, path, stroke);

  switch (_bpp)
  {
    case 4:
      AggRenderScanlines<4, AggRasterizer, AggScanlineP8>(this, ctx, ctx->ras, ctx->slP8);
      break;
    case 3:
      AggRenderScanlines<3, AggRasterizer, AggScanlineP8>(this, ctx, ctx->ras, ctx->slP8);
      break;
    default:
      FOG_ASSERT_NOT_REACHED();
  }
}

void RasterPainterDevice::_renderImage(
  RasterPainterContext* ctx,
  const Rect& dst, const Image& image, const Rect& src)
{
  Image::Data* image_d = image._d;
  sysint_t dstStride = _stride;
  sysint_t srcStride = image_d->stride;

  Raster::SpanCompositeFn span_composite = ctx->raster->span_composite[image.format()];

  sysint_t x = dst.x1();
  sysint_t y = dst.y1();

  sysint_t w = dst.width();
  sysint_t h = dst.height();

  uint8_t* dstCur = ctx->clipState->workRaster + y * dstStride + x * _bpp;
  const uint8_t* srcCur = image_d->first + src.y1() * srcStride + src.x1() * image_d->bytesPerPixel;

  do {
    span_composite(dstCur, srcCur, (sysuint_t)w);
    dstCur += dstStride;
    srcCur += srcStride;
  } while (--h);
}

} // Fog namespace
