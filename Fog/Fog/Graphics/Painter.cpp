// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/AutoLock.h>
#include <Fog/Core/AutoUnlock.h>
#include <Fog/Core/CpuInfo.h>
#include <Fog/Core/Error.h>
#include <Fog/Core/Math.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/OS.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/Thread.h>
#include <Fog/Core/ThreadCondition.h>
#include <Fog/Core/ThreadPool.h>
#include <Fog/Graphics/AffineMatrix.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Font.h>
#include <Fog/Graphics/Glyph.h>
#include <Fog/Graphics/GlyphSet.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/Painter.h>
#include <Fog/Graphics/Raster.h>
#include <Fog/Graphics/Raster/Raster_ByteOp.h>
#include <Fog/Graphics/Raster/Raster_PixelOp.h>
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
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_rounded_rect.h"
#include "agg_scanline_p.h"
#include "agg_scanline_u.h"
#include "agg_scanline_bin.h"
#include "agg_trans_affine.h"
#include "agg_trans_viewport.h"
#include "agg_vcgen_dash.h"

#include "agg_rasterizer_scanline_aa_custom.h"

// [BlitJit]
#include <BlitJit/BlitJit.h>

namespace Fog {

// ============================================================================
// [Configuration]
// ============================================================================

#define RASTER_MAX_THREADS 4
#define RASTER_MAX_COMMANDS 4096

// #define RASTER_DEBUG

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct RasterPainterDevice;
struct RasterPainterCommand;
struct RasterPainterThreadData;
struct RasterPainterThreadTask;

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

  // [Properties]

  virtual err_t setProperty(const String32& name, const Value& value) { return Error::InvalidProperty; }
  virtual Value getProperty(const String32& name) const { return Value(); }
};

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
    if (refCount.deref()) delete this;
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
    if (refCount.deref()) delete this;
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
typedef agg::rasterizer_scanline_aa_custom<>           AggRasterizer;
typedef agg::scanline_p8                               AggScanlineP8;
typedef agg::scanline_u8                               AggScanlineU8;

// Context is accessed always from only one thread.
struct FOG_HIDDEN RasterPainterContext
{
  // [Construction / Destruction]

  RasterPainterContext();
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

  AggScanlineP8 slP8;
  AggScanlineU8 slU8;

  uint8_t* buffer;
  sysint_t bufferSize;
  uint8_t bufferStatic[1024*8];

private:
  FOG_DISABLE_COPY(RasterPainterContext)
};

RasterPainterContext::RasterPainterContext()
{
  // AntiGrain bug workaround
  slP8.reset(0, 1000);

  owner = NULL;

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
// [Fog::RasterPainterCommandAllocator]
// ============================================================================

struct FOG_HIDDEN RasterPainterCommandAllocator
{
  // [Construction / Destruction]

  RasterPainterCommandAllocator();
  ~RasterPainterCommandAllocator();

  // [Allocator]

  void* alloc(sysuint_t size);
  void freeAll();

  // [Memory blocks]

  struct Block
  {
    enum { BlockSize = 16000 };

    Block* next;

    sysuint_t size;
    sysuint_t pos;
    Atomic<sysuint_t> used;

    uint8_t memory[BlockSize];
  };

  Block* blocks;
};

RasterPainterCommandAllocator::RasterPainterCommandAllocator() : 
  blocks(NULL)
{
}

RasterPainterCommandAllocator::~RasterPainterCommandAllocator()
{
  freeAll();
}

void* RasterPainterCommandAllocator::alloc(sysuint_t size)
{
  FOG_ASSERT(size <= Block::BlockSize);

  if (!blocks || (blocks->size - blocks->pos) < size)
  {
    // Traverse to previous blocks and try to find complete free one
    Block* cur = blocks;
    Block* prev = NULL;

    while (cur)
    {
      if (cur->used.get() == 0)
      {
        // Make it first.
        if (prev)
        {
          prev->next = cur->next;
          cur->next = blocks;
          blocks = cur;
        }

        cur->pos = 0;
        goto allocFromBlocks;
      }

      prev = cur;
      cur = cur->next;
    }

    // If we are here, it's needed to allocate new chunk of memory.
    cur = (Block*)Memory::alloc(sizeof(Block));
    if (!cur) return NULL;

    cur->size = Block::BlockSize;
    cur->pos = 0;
    cur->used.init(0);

    cur->next = blocks;
    blocks = cur;
  }

allocFromBlocks:
  void* mem = (void*)(blocks->memory + blocks->pos);
  blocks->pos += size;
  blocks->used.add(size);
  return mem;
}

void RasterPainterCommandAllocator::freeAll()
{
  Block* cur = blocks;
  blocks = NULL;

  while (cur)
  {
    Block* next = cur->next;

    // It's failure if Block::used is not zero when freeing everything.
    FOG_ASSERT(cur->used.get() == 0);
    Memory::free(cur);

    cur = next;
  }
}

// ============================================================================
// [Fog::RasterPainterCommand]
// ============================================================================

struct FOG_HIDDEN RasterPainterCommand
{
  // [Context]

  Atomic<sysuint_t> refCount;
  RasterPainterClipState* clipState;
  RasterPainterCapsState* capsState;

  Raster::FunctionMap::Raster* raster;
  Raster::PatternContext* pctx;

  // [Related to RasterPainterCommandAllocator]

  RasterPainterCommandAllocator::Block* this_block;
  sysuint_t this_size;

  // [Id]

  enum Id
  {
    PathId
  };

  int id;

  // [Data]

  struct PathData
  {
    AggRasterizer ras;
  };

  union
  {
    Static<PathData> path;
  };
};

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
  void flushWithQuit();

  // [Properties]

  virtual err_t setProperty(const String32& name, const Value& value);
  virtual Value getProperty(const String32& name) const;

  // [Multithreading]

  void setMultithreaded(bool mt);

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

  RasterPainterCommand* _createCommand();
  void _postCommand(RasterPainterCommand* cmd);

  // [Rasterizers]

  static bool _rasterizePath(RasterPainterContext* ctx, AggRasterizer& ras, const Path& path, bool stroke);

  // [Renderers]
  //
  // Renderers can be called from various threads (workers).

  void _renderPath(const AggRasterizer& ras);
  void _renderBoxes(const Box* box, sysuint_t count);
  void _renderImage(const Rect& dst, const Image& image, const Rect& src);
  void _renderGlyphSet(const Point& pt, const GlyphSet& glyphSet, const Rect* clip);

  void _renderPathMT(RasterPainterContext* ctx, int offset, int delta, const AggRasterizer& ras);
  //void _renderBoxesMT(RasterPainterContext* ctx, const Box* box, sysuint_t count);
  //void _renderImageMT(RasterPainterContext* ctx, const Rect& dst, const Image& image, const Rect& src);
  //void _renderGlyphSetMT(RasterPainterContext* ctx, const Point& pt, const GlyphSet& glyphSet, const Rect* clip);

  // [Constants]

  enum {
    MT_MinWidth = 128,
    MT_MinHeight = 128
  };

  // [Members]

  uint8_t* _metaRaster;
  sysint_t _stride;

  int _metaWidth;
  int _metaHeight;

  int _format;
  sysint_t _bpp;

  bool _compositingEnabled;
  bool _premultiplied;

  // Temporary path
  Path workPath;

  // Antigrain context is declared outside of painter because we want that it
  // will be accessible through multiple threads. Each thread have it's own
  // context.
  RasterPainterContext ctx;
  // If we are running in single-core environment it's better to embed one
  // antigrain rasterizer in device itself (and only this rasterizer will be 
  // used).
  AggRasterizer ras;

  // Multithreading
  RasterPainterThreadData* _threadData;

  // [Statics]

  static ThreadPool* threadPool;
};

ThreadPool* RasterPainterDevice::threadPool;

// ============================================================================
// [Fog::RasterPainterThreadData / Fog::RasterPainterThreadTask]
// ============================================================================

// This is task created per painter thread that contains all variables needed
// to process painter commands in parallel. The goal is that condition variable
// is shared across all painter threads so one signal will wake them all.
struct FOG_HIDDEN RasterPainterThreadTask : public Task
{
  RasterPainterThreadTask();

  virtual void run();
  virtual void destroy();

  RasterPainterDevice* d;
  RasterPainterThreadData* data;

  volatile int shouldQuit;

  // Painter commands management.
  volatile sysuint_t currentCommand;

  // Thread offset and delta.
  int offset;
  int delta;

  RasterPainterContext ctx;
};

// Structure shared across all threads. 
struct FOG_HIDDEN RasterPainterThreadData
{
  RasterPainterThreadData() : 
    commandsReady(&commandsLock),
    commandsComplete(&commandsLock),
    releaseEvent(NULL)
  {
  }

  ~RasterPainterThreadData()
  {
  }

  ThreadPool* threadPool;

  sysuint_t numThreads;
  Atomic<sysuint_t> startedThreads;   // Count of threads started (total)
  Atomic<sysuint_t> finishedThreads;  // Count of threads finished (used to quit)
  Atomic<sysuint_t> workingThreads;   // Count of currently working threads
  Atomic<sysuint_t> completedThreads; // Count of threads that completed all tasks

  Lock commandsLock;
  ThreadCondition commandsReady;
  ThreadCondition commandsComplete;
  ThreadEvent* releaseEvent;

  Thread* threads[RASTER_MAX_THREADS];
  Static<RasterPainterThreadTask> tasks[RASTER_MAX_THREADS];

  // Commands
  Atomic<sysuint_t> commandsPosition;
  RasterPainterCommandAllocator commandAllocator;
  RasterPainterCommand* volatile commands[RASTER_MAX_COMMANDS];
};

RasterPainterThreadTask::RasterPainterThreadTask()
{
  d = NULL;
  data = NULL;
  shouldQuit = 0;
  currentCommand = 0;
}

void RasterPainterThreadTask::run()
{
#if defined(RASTER_DEBUG)
  fog_debug("#%d - run() [ThreadID=%d]", offset, Thread::current()->id());
#endif // RASTER_DEBUG

  for (;;)
  {
    // Process commands
    while (currentCommand < data->commandsPosition.get())
    {
      RasterPainterCommand* cmd = data->commands[currentCommand];

#if defined(RASTER_DEBUG)
      fog_debug("#%d - command %d (%p)", offset, (int)currentCommand, (cmd));
#endif // RASTER_DEBUG

      switch (cmd->id)
      {
        case RasterPainterCommand::PathId:
        {
          // Render
          ctx.clipState = cmd->clipState;
          ctx.capsState = cmd->capsState;
          ctx.raster = cmd->raster;
          ctx.pctx = cmd->pctx;
          d->_renderPathMT(&ctx, offset, delta, cmd->path->ras);

          // Destroy
          if (cmd->refCount.deref()) 
          {
            cmd->clipState->deref();
            cmd->capsState->deref();
            if (cmd->pctx && cmd->pctx->refCount.deref()) cmd->pctx->destroy(cmd->pctx);
            cmd->path.destroy();
            cmd->this_block->used.sub(cmd->this_size);
          }
          break;
        }

        default:
          FOG_ASSERT_NOT_REACHED();
      }

      currentCommand++;
    }

    {
      AutoLock locked(data->commandsLock);

      if (currentCommand < data->commandsPosition.get())
        continue;

#if defined(RASTER_DEBUG)
      fog_debug("#%d - done", offset);
#endif // RASTER_DEBUG

      if (data->completedThreads.addXchg(1)+1 == data->numThreads)
      {
#if defined(RASTER_DEBUG)
        fog_debug("#%d - complete, signaling commandComplete()", offset);
#endif // RASTER_DEBUG
        data->commandsComplete.signal();
      }
/*
      if (currentCommand < data->commandsPosition.get())
      {
        data->workingThreads.inc();
        continue;
      }
*/
      if (shouldQuit)
      {
#if defined(RASTER_DEBUG)
        fog_debug("#%d - shouldQuit is true, quitting", offset);
#endif // RASTER_DEBUG
        return;
      }

#if defined(RASTER_DEBUG)
      fog_debug("#%d - waiting...", offset);
#endif // RASTER_DEBUG

      data->workingThreads.dec();
      data->commandsReady.wait();
      data->workingThreads.inc();
    }
  }
}

void RasterPainterThreadTask::destroy()
{
#if defined(RASTER_DEBUG)
  fog_debug("#%d - destroy()", offset);
#endif // RASTER_DEBUG

  if (data->finishedThreads.addXchg(1) == data->numThreads-1)
  {
#if defined(RASTER_DEBUG)
    fog_debug("#%d - I'm last, signaling releaseEvent()", offset);
#endif // RASTER_DEBUG

    data->releaseEvent->signal();
  }
}

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
  _compositingEnabled(format == Image::FormatARGB32 || format == Image::FormatPRGB32),
  _threadData(NULL)
{
  ctx.owner = this;
  ctx.clipState = new(std::nothrow) RasterPainterClipState();
  ctx.capsState = new(std::nothrow) RasterPainterCapsState();

  // Setup clip state
  _setClipDefaults();

  // Setup caps state
  _setCapsDefaults();

  // Setup multithreading if possible. If the painting buffer if too small, we
  // will not use multithreading, because it has no sense.
  if (cpuInfo->numberOfProcessors > 1 && width >= MT_MinWidth && height >= MT_MinHeight)
  {
    setMultithreaded(true);
  }
}

RasterPainterDevice::~RasterPainterDevice()
{
  if (_threadData) setMultithreaded(false);

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
  if (!_threadData) return;

  AutoLock locked(_threadData->commandsLock);

#if defined(RASTER_DEBUG)
  fog_debug("== flush, working threads: %d, complete threads: %d, command position: %d",
    (int)_threadData->workingThreads.get(),
    (int)_threadData->completedThreads.get(),
    (int)_threadData->commandsPosition.get());
#endif // RASTER_DEBUG

  while (_threadData->completedThreads.get() != _threadData->numThreads)
    _threadData->commandsComplete.wait();

  // Reset command position and local command counters for each thread.
#if defined(RASTER_DEBUG)
  fog_debug("== flush, reseting command position and thread current commands");
#endif // RASTER_DEBUG

  _threadData->commandsPosition.setXchg(0);
  for (sysuint_t i = 0; i < _threadData->numThreads; i++)
  {
    _threadData->tasks[i]->currentCommand = 0;
  }
}

void RasterPainterDevice::flushWithQuit()
{
  FOG_ASSERT(_threadData);

  AutoLock locked(_threadData->commandsLock);

#if defined(RASTER_DEBUG)
  fog_debug("== quitting");
#endif // RASTER_DEBUG

  if (_threadData->workingThreads.get() != _threadData->numThreads)
    _threadData->commandsReady.broadcast();
}

// ============================================================================
// [Fog::RasterPainterDevice - Properties]
// ============================================================================

err_t RasterPainterDevice::setProperty(const String32& name, const Value& value)
{
  err_t err = Error::InvalidProperty;
  int p_int;

  if (name == StubAscii8("multithreaded"))
  {
    if ((err = value.toInt32(&p_int)) == Error::Ok)
      if (value.isInteger()) setMultithreaded(p_int != 0);
  }

  return err;
}

Value RasterPainterDevice::getProperty(const String32& name) const
{
  Value result;

  if (name == StubAscii8("multithreaded"))
    result.setInt32(_threadData != NULL);

  return result;
}

// ============================================================================
// [Fog::RasterPainterDevice - Multithreading - Start / Stop]
// ============================================================================

void RasterPainterDevice::setMultithreaded(bool mt)
{
  int i;

  if ((_threadData != NULL) == mt) return;

  // If worker pool is not created we can't start multithreaded rendering.
  if (mt && !threadPool) return;

  // Start multithreading...
  if (mt)
  {
    int max = fog_min<int>(cpuInfo->numberOfProcessors, RASTER_MAX_THREADS);

    _threadData = new(std::nothrow) RasterPainterThreadData;
    if (_threadData == NULL) return;

    // This is for testing multithreaded rendering on single cores.
    if (max < 2) max = 2;

    for (i = 0; i < max; i++)
    {
      if ((_threadData->threads[i] = threadPool->getThread()) == NULL) break;
    }

    // Failed to get workers. This can happen if there are many threads that
    // uses multithreaded painter, we must destroy all resources and return.
    if (i <= 1)
    {
      if (_threadData->threads[0])
      {
        threadPool->releaseThread(_threadData->threads[0]);
        _threadData->threads[0] = NULL;
      }

      delete _threadData;
      _threadData = NULL;

      // Bailout
      return;
    }

    int count = i;
    _threadData->numThreads = count;
    _threadData->startedThreads.init(0);
    _threadData->finishedThreads.init(0);
    _threadData->workingThreads.init(count);
    _threadData->completedThreads.init(0);
    _threadData->threadPool = threadPool;
    _threadData->commandsPosition.init(0);

    for (i = 0; i < count; i++)
    {
      _threadData->tasks[i].init();
      RasterPainterThreadTask* task = _threadData->tasks[i].instancep();
      task->d = this;
      task->data = _threadData;
      task->offset = i;
      task->delta = count;
      task->ctx.owner = this;
    }

    for (i = 0; i < count; i++)
    {
      RasterPainterThreadTask* task = _threadData->tasks[i].instancep();
      _threadData->threads[i]->eventLoop()->postTask(task);
    }

    // Wait for threads to initialize
    {
      AutoLock locked(_threadData->commandsLock);
      while (_threadData->completedThreads.get() != _threadData->numThreads)
        _threadData->commandsComplete.wait();
    }

#if defined(RASTER_DEBUG)
    fog_debug("== multithreaded set to true");
#endif // RASTER_DEBUG
  }
  // Stop multithreading
  else
  {
    int count = _threadData->numThreads;

    ThreadEvent releaseEvent(false, false);
    _threadData->releaseEvent = &releaseEvent;

    // Release threads
    for (i = 0; i < count; i++)
    {
      _threadData->tasks[i]->shouldQuit = 1;
    }

    // Flush everything and wait for completion.
    flushWithQuit();

    releaseEvent.wait();

    for (i = 0; i < count; i++)
    {
      threadPool->releaseThread(_threadData->threads[i]);
      _threadData->tasks[i].destroy();
    }

    delete _threadData;
    _threadData = NULL;

#if defined(RASTER_DEBUG)
    fog_debug("== multithreaded set to false");
#endif // RASTER_DEBUG
  }
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

void RasterPainterDevice::_serializePath(const Path& path, bool stroke)
{
  // Pattern context must be always set up before _render() methods are called.
  if (!ctx.capsState->isSolidSource && !_getPatternContext()) return;

  if (_threadData)
  {
    // MultiThreaded - Serialize command.
    RasterPainterCommand* cmd = _createCommand();
    cmd->id = RasterPainterCommand::PathId;
    cmd->path.init();
    if (_rasterizePath(&ctx, cmd->path->ras, path, stroke))
    {
      _postCommand(cmd);
    }
    else
    {
      // Destroy
      // FIXME: Move this to separate function or to command directly?
      cmd->clipState->deref();
      cmd->capsState->deref();
      if (cmd->pctx && cmd->pctx->refCount.deref()) cmd->pctx->destroy(cmd->pctx);
      cmd->path.destroy();
      cmd->this_block->used.sub(cmd->this_size);
    }
  }
  else
  {
    // SingleThreaded - Render now.
    if (_rasterizePath(&ctx, ras, path, stroke))
    {
      _renderPath(ras);
    }
  }
}

void RasterPainterDevice::_serializeBoxes(const Box* box, sysuint_t count)
{
  // Pattern context must be always set up before _render() methods are called.
  if (!ctx.capsState->isSolidSource && !_getPatternContext()) return;

  if (_threadData)
  {
  }
  else
  {
    _renderBoxes(box, count);
  }
}

void RasterPainterDevice::_serializeImage(const Rect& dst, const Image& image, const Rect& src)
{
  if (_threadData)
  {
  }
  else
  {
    _renderImage(dst, image, src);
  }
}

void RasterPainterDevice::_serializeGlyphSet(const Point& pt, const GlyphSet& glyphSet, const Rect* clip)
{
  // Pattern context must be always set up before _render() methods are called.
  if (!ctx.capsState->isSolidSource && !_getPatternContext()) return;

  if (_threadData)
  {
  }
  else
  {
    _renderGlyphSet(pt, glyphSet, clip);
  }
}

RasterPainterCommand* RasterPainterDevice::_createCommand()
{
  RasterPainterCommand* command = 
    reinterpret_cast<RasterPainterCommand*>(
      _threadData->commandAllocator.alloc(sizeof(RasterPainterCommand)));

  if (!command) return NULL;

  command->this_block = _threadData->commandAllocator.blocks;
  command->this_size = sizeof(RasterPainterCommand);
  command->refCount.init(_threadData->numThreads);
  command->clipState = ctx.clipState->ref();
  command->capsState = ctx.capsState->ref();
  command->raster = ctx.raster;
  command->pctx = NULL;

  if (ctx.pctx && ctx.pctx->initialized)
  {
    ctx.pctx->refCount.inc();
    command->pctx = ctx.pctx;
  }

  return command;
}

void RasterPainterDevice::_postCommand(RasterPainterCommand* cmd)
{
  // Flush everything if commands get to maximum
  if (_threadData->commandsPosition.get() == RASTER_MAX_COMMANDS)
  {
    flush();
    FOG_ASSERT(_threadData->commandsPosition.get() == 0);

    _threadData->commands[0] = cmd;
    _threadData->commandsPosition.setXchg(1);
  }
  else
  {
    _threadData->commands[_threadData->commandsPosition.get()] = cmd;
    _threadData->commandsPosition.inc();
  }

  AutoLock locked(_threadData->commandsLock);
  if (_threadData->completedThreads.get() > 0)
  {
    _threadData->completedThreads.setXchg(0);
    _threadData->commandsReady.broadcast();
  }
}

// ============================================================================
// [Fog::RasterPainterDevice - Renderers - AntiGrain]
// ============================================================================

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

static bool FOG_FASTCALL AggRasterizePath(
  RasterPainterContext* ctx, AggRasterizer& ras,
  const Path& path, bool stroke)
{
  RasterPainterClipState* clipState = ctx->clipState;
  RasterPainterCapsState* capsState = ctx->capsState;

  ras.reset();
  ras.filling_rule(static_cast<agg::filling_rule_e>(capsState->fillMode));
  ras.clip_box(
    (double)clipState->clipBox.x1(),
    (double)clipState->clipBox.y1(),
    (double)clipState->clipBox.x2(),
    (double)clipState->clipBox.y2());

  AggPath pAgg(path);
  AggConvCurve pCurve(pAgg);

  // This can be a bit messy, but it's here to increase performance. We will
  // not calculate using transformations if they are not used. Also we add
  // stroke and line dash pipeline only if it's needed. This is goal of 
  // AntiGrain to be able to setup only pipelines what are really needed.
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
        ras.add_path(strokeTransform);
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
        ras.add_path(pCurveDashStrokeTransform);
      }
    }
    else
    {
      AggConvCurveTransform pCurveTransform(
        pCurve, *((agg::trans_affine *)&capsState->transformations));

      ras.add_path(pCurveTransform);
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
        ras.add_path(pCurveStroke);
      }
      else
      {
        AggConvCurveDash pCurveDash(pCurve);
        AggSetupDash(pCurveDash, capsState);
        AggConvCurveDashStroke pCurveDashStroke(pCurveDash);
        AggSetupStroke(pCurveDashStroke, capsState);
        ras.add_path(pCurveDashStroke);
      }
    }
    else
    {
      ras.add_path(pCurve);
    }
  }
  ras.sort();
  return ras.has_cells();
}

template<int BytesPerPixel, class Rasterizer, class Scanline>
static void FOG_INLINE AggRenderPath(
  RasterPainterContext* ctx, const Rasterizer& ras, Scanline& sl, int offset, int delta)
{
  RasterPainterClipState* clipState = ctx->clipState;
  RasterPainterCapsState* capsState = ctx->capsState;

  sl.reset(ras.min_x(), ras.max_x());

  Raster::SpanSolidFn span_solid = ctx->raster->span_solid;
  Raster::SpanSolidMskFn span_solid_a8 = ctx->raster->span_solid_a8;

  int y = ras.min_y() + offset;
  int y_end = ras.max_y();
  if (y > y_end) return;

  sysint_t stride = ctx->owner->_stride;
  uint8_t* pBase = clipState->workRaster + y * stride;
  uint8_t* pCur;

  stride *= delta;

  // solid source
  if (capsState->isSolidSource)
  {
    uint32_t solidColor = capsState->solidSourcePremultiplied;

    for (; y <= y_end; y += delta, pBase += stride)
    {
      if (!ras.sweep_scanline(sl, y)) continue;

      unsigned num_spans = sl.num_spans();
      typename Scanline::const_iterator span = sl.begin();

      for (;;)
      {
        int x = span->x;
        int len = span->len;

        pCur = pBase + Raster::mul<int, BytesPerPixel>(x);

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

    for (; y <= y_end; y += delta, pBase += stride)
    {
      if (!ras.sweep_scanline(sl, y)) continue;

      unsigned num_spans = sl.num_spans();
      typename Scanline::const_iterator span = sl.begin();

      for (;;)
      {
        int x = span->x;
        int len = span->len;

        pCur = pBase + Raster::mul<int, BytesPerPixel>(x);

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

bool RasterPainterDevice::_rasterizePath(RasterPainterContext* ctx, AggRasterizer& ras, const Path& path, bool stroke)
{
  return AggRasterizePath(ctx, ras, path, stroke);
}

// ============================================================================
// [Fog::RasterPainterDevice - Renderers - SingleThreaded]
// ============================================================================

void RasterPainterDevice::_renderPath(const AggRasterizer& ras)
{
  switch (_bpp)
  {
    case 4:
      AggRenderPath<4, AggRasterizer, AggScanlineP8>(&ctx, ras, ctx.slP8, 0, 1);
      break;
    case 3:
      AggRenderPath<3, AggRasterizer, AggScanlineP8>(&ctx, ras, ctx.slP8, 0, 1);
      break;
    default:
      FOG_ASSERT_NOT_REACHED();
  }
}

void RasterPainterDevice::_renderBoxes(const Box* box, sysuint_t count)
{
  if (!count) return;

  uint8_t* pBuf = ctx.clipState->workRaster;
  sysint_t stride = _stride;
  sysint_t bpp = _bpp;

  if (ctx.capsState->isSolidSource)
  {
    uint32_t rgba = ctx.capsState->solidSourcePremultiplied;
    Raster::SpanSolidFn span_solid = ctx.raster->span_solid;

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
    Raster::PatternContext* pctx = ctx.pctx;
    if (!pctx) return;

    uint8_t* pbuf = ctx.getBuffer(Raster::mul4(ctx.clipState->clipBox.width()));
    if (!pbuf) return;

    Raster::SpanCompositeFn span_composite = ctx.raster->span_composite[pctx->format];

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

void RasterPainterDevice::_renderImage(const Rect& dst, const Image& image, const Rect& src)
{
  Image::Data* image_d = image._d;
  sysint_t dstStride = _stride;
  sysint_t srcStride = image_d->stride;

  Raster::SpanCompositeFn span_composite = ctx.raster->span_composite[image.format()];

  sysint_t x = dst.x1();
  sysint_t y = dst.y1();

  sysint_t w = dst.width();
  sysint_t h = dst.height();

  uint8_t* dstCur = ctx.clipState->workRaster + y * dstStride + x * _bpp;
  const uint8_t* srcCur = image_d->first + src.y1() * srcStride + src.x1() * image_d->bytesPerPixel;

  do {
    span_composite(dstCur, srcCur, (sysuint_t)w);
    dstCur += dstStride;
    srcCur += srcStride;
  } while (--h);
}

void RasterPainterDevice::_renderGlyphSet(const Point& pt, const GlyphSet& glyphSet, const Rect* clip)
{
  // TODO: Hardcoded to A8 glyph format
  // TODO: Clipping

  Box clipBox = ctx.clipState->clipBox;
  if (clip) Box::intersect(clipBox, clipBox, Box(*clip));

  if (!clipBox.isValid()) return;
  if (!glyphSet.length()) return;

  const Glyph* glyphs = glyphSet.glyphs();
  sysuint_t count = glyphSet.length();

  int px = pt.x();
  int py = pt.y();

  uint8_t* pBuf = ctx.clipState->workRaster;
  sysint_t stride = _stride;

  Raster::SpanSolidMskFn span_solid_a8 = ctx.raster->span_solid_a8;

  // Used only if source is solid
  uint32_t rgba = ctx.capsState->solidSourcePremultiplied;

  // Used only if source is pattern
  Raster::PatternContext* pctx;
  uint8_t* pbuf;
  Raster::SpanCompositeMskFn span_composite_a8;

  if (!ctx.capsState->isSolidSource)
  {
    pctx = ctx.pctx;
    if (!pctx) return;

    pbuf = ctx.getBuffer(Raster::mul4(ctx.clipState->clipBox.width()));
    if (!pbuf) return;

    span_composite_a8 = ctx.raster->span_composite_a8[pctx->format];
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

    if (ctx.capsState->isSolidSource)
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

// ============================================================================
// [Fog::RasterPainterDevice - Renderers - MultiThreaded]
// ============================================================================

void RasterPainterDevice::_renderPathMT(
  RasterPainterContext* ctx, int offset, int delta,
  const AggRasterizer& ras)
{
  switch (_bpp)
  {
    case 4:
      AggRenderPath<4, AggRasterizer, AggScanlineP8>(ctx, ras, ctx->slP8, offset, delta);
      break;
    case 3:
      AggRenderPath<3, AggRasterizer, AggScanlineP8>(ctx, ras, ctx->slP8, offset, delta);
      break;
    default:
      FOG_ASSERT_NOT_REACHED();
  }
}

#if 0
void RasterPainterDevice::_renderBoxes(
  RasterPainterContext* ctx, int offset, int delta,
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

void RasterPainterDevice::_renderImage(
  RasterPainterContext* ctx, int offset, int delta,
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

void RasterPainterDevice::_renderGlyphSet(
  RasterPainterContext* ctx, int offset, int delta,
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
#endif

// ============================================================================
// [Fog::Painter]
// ============================================================================

PainterDevice* Painter::sharedNull;

Painter::Painter()
{
  _d = sharedNull;
}

Painter::Painter(uint8_t* pixels, int width, int height, sysint_t stride, int format)
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

err_t Painter::begin(uint8_t* pixels, int width, int height, sysint_t stride, int format)
{
  end();

  if (width <= 0 || height <= 0) return Error::InvalidArgument;

  PainterDevice* d = new(std::nothrow) RasterPainterDevice(
    pixels, width, height, stride, format);
  if (!d) return Error::OutOfMemory;

  _d = d;
  return Error::Ok;
}

err_t Painter::begin(Image& image)
{
  err_t err = image.detach();
  if (err) return err;

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
  static Fog::NullPainterDevice sharedNullDevice;
  Fog::Painter::sharedNull = &sharedNullDevice;

  // Setup multithreading.
  // 
  // If CPU contains more cores, we can improve rendering speed by using them.
  // By default we set number of threads (workers) to count of CPU cores + 1.
  Fog::RasterPainterDevice::threadPool = NULL;
  Fog::RasterPainterDevice::threadPool = new(std::nothrow) Fog::ThreadPool();

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_painter_shutdown(void)
{
  // Kill multithreading.
  if (Fog::RasterPainterDevice::threadPool)
  {
    delete Fog::RasterPainterDevice::threadPool;
    Fog::RasterPainterDevice::threadPool = NULL;
  }
}
