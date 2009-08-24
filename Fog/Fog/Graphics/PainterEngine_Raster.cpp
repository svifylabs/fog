// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled Headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/AutoLock.h>
#include <Fog/Core/AutoUnlock.h>
#include <Fog/Core/Atomic.h>
#include <Fog/Core/CpuInfo.h>
#include <Fog/Core/Error.h>
#include <Fog/Core/Math.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/OS.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/Thread.h>
#include <Fog/Core/ThreadCondition.h>
#include <Fog/Core/ThreadPool.h>
#include <Fog/Graphics/Matrix.h>
#include <Fog/Graphics/ColorLut.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Error.h>
#include <Fog/Graphics/Font.h>
#include <Fog/Graphics/Glyph.h>
#include <Fog/Graphics/GlyphSet.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/Painter.h>
#include <Fog/Graphics/PainterEngine_Raster.h>
#include <Fog/Graphics/Raster.h>
#include <Fog/Graphics/Raster/Raster_C.h>
#include <Fog/Graphics/Rgba.h>

// [AntiGrain]
#include "agg_alpha_mask_u8.h"
#include "agg_basics.h"
#include "agg_scanline_p.h"
#include "agg_scanline_u.h"
#include "agg_scanline_bin.h"
#include "agg_trans_viewport.h"

#include "agg_rasterizer_scanline_aa_custom.h"

namespace Fog {

// ============================================================================
// [Configuration]
// ============================================================================

// #define FOG_DEBUG_RASTER_SYNCHRONIZATION
// #define FOG_DEBUG_RASTER_COMMANDS

// ============================================================================
// [AntiGrain]
// ============================================================================

// There are templates and classes that helps to wrap fog containers and paths
// to AntiGrain without converting them into antigrain storage containers. This
// way should improve performance to render larger images and complex paths,
// but it also allows us to not depend to antigrain directly, but only
// internally.

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

// Rasterizer and scanline storage
typedef agg::rasterizer_scanline_aa_custom<> AggRasterizer;
typedef agg::scanline_p8 AggScanlineP8;
typedef agg::scanline_u8 AggScanlineU8;

// ============================================================================
// [Fog::PainterEngine_Raster]
// ============================================================================

struct FOG_HIDDEN PainterEngine_Raster : public PainterEngine
{
  // --------------------------------------------------------------------------
  // [Configuration]
  // --------------------------------------------------------------------------

  // Minimum size to set multithreading on.
  enum { MinSizeThreshold = 256*256 };

  // Maximum number of threads to use for rendering.
  enum { MaxWorkers = 16 };
  // Maximum commands and calculations to accumulate in buffer.
#if defined(FOG_DEBUG_RASTER_SYNCHRONIZATION)
  enum { MaxCommands = 32 };     // For debugging it's better to have only minimal
  enum { MaxCalculations = 32 }; // buffer size for the commands and calculations.
#else
  enum { MaxCommands = 1024 };
  enum { MaxCalculations = 1024 };
#endif // FOG_DEBUG_RASTER_SYNCHRONIZATION

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  PainterEngine_Raster(uint8_t* pixels, int width, int height, sysint_t stride, int format, int hints);
  virtual ~PainterEngine_Raster();

  // --------------------------------------------------------------------------
  // [Meta]
  // --------------------------------------------------------------------------

  virtual int getWidth() const;
  virtual int getHeight() const;
  virtual int getFormat() const;

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

  virtual Point getMetaOrigin() const;
  virtual Point getUserOrigin() const;

  virtual Region getMetaRegion() const;
  virtual Region getUserRegion() const;

  virtual bool isMetaRegionUsed() const;
  virtual bool isUserRegionUsed() const;

  // --------------------------------------------------------------------------
  // [Operator]
  // --------------------------------------------------------------------------

  virtual void setOperator(uint32_t op);
  virtual uint32_t getOperator() const;

  // --------------------------------------------------------------------------
  // [Source]
  // --------------------------------------------------------------------------

  virtual void setSource(const Rgba& rgba);
  virtual void setSource(const Pattern& pattern);

  virtual Rgba sourceRgba();
  virtual Pattern sourcePattern();

  // --------------------------------------------------------------------------
  // [Parameters]
  // --------------------------------------------------------------------------

  virtual void setLineParams(const LineParams& params);
  virtual void getLineParams(LineParams& params) const;

  virtual void setLineWidth(double lineWidth);
  virtual double getLineWidth() const;

  virtual void setLineCap(uint32_t lineCap);
  virtual uint32_t getLineCap() const;

  virtual void setLineJoin(uint32_t lineJoin);
  virtual uint32_t getLineJoin() const;

  virtual void setDashes(const double* dashes, sysuint_t count);
  virtual void setDashes(const Vector<double>& dashes);
  virtual Vector<double> getDashes() const;

  virtual void setDashOffset(double offset);
  virtual double getDashOffset() const;

  virtual void setMiterLimit(double miterLimit);
  virtual double getMiterLimit() const;

  virtual void setFillMode(uint32_t mode);
  virtual uint32_t getFillMode();

  // --------------------------------------------------------------------------
  // [Transformations]
  // --------------------------------------------------------------------------

  virtual void setMatrix(const Matrix& m);
  virtual void resetMatrix();
  virtual Matrix getMatrix() const;

  virtual void rotate(double angle);
  virtual void scale(double sx, double sy);
  virtual void skew(double sx, double sy);
  virtual void translate(double x, double y);
  virtual void affine(const Matrix& m);
  virtual void parallelogram(double x1, double y1, double x2, double y2, const double* para);
  virtual void viewport(
    double worldX1,  double worldY1,  double worldX2,  double worldY2,
    double screenX1, double screenY1, double screenX2, double screenY2,
    uint32_t viewportOption);

  virtual void worldToScreen(PointF* pt) const;
  virtual void screenToWorld(PointF* pt) const;

  virtual void worldToScreen(double* scalar) const;
  virtual void screenToWorld(double* scalar) const;

  virtual void alignPoint(PointF* pt) const;

  // --------------------------------------------------------------------------
  // [State]
  // --------------------------------------------------------------------------

  virtual void save();
  virtual void restore();

  // --------------------------------------------------------------------------
  // [Raster Drawing]
  // --------------------------------------------------------------------------

  virtual void clear();
  virtual void drawPoint(const Point& p);
  virtual void drawLine(const Point& start, const Point& end);
  virtual void drawRect(const Rect& r);
  virtual void drawRound(const Rect& r, const Point& radius);
  virtual void fillRect(const Rect& r);
  virtual void fillRects(const Rect* r, sysuint_t count);
  virtual void fillRound(const Rect& r, const Point& radius);
  virtual void fillRegion(const Region& region);

  // --------------------------------------------------------------------------
  // [Vector Drawing]
  // --------------------------------------------------------------------------

  virtual void drawPoint(const PointF& p);
  virtual void drawLine(const PointF& start, const PointF& end);
  virtual void drawLine(const PointF* pts, sysuint_t count);
  virtual void drawPolygon(const PointF* pts, sysuint_t count);
  virtual void drawRect(const RectF& r);
  virtual void drawRects(const RectF* r, sysuint_t count);
  virtual void drawRound(const RectF& r, const PointF& radius);
  virtual void drawEllipse(const PointF& cp, const PointF& r);
  virtual void drawArc(const PointF& cp, const PointF& r, double start, double sweep);
  virtual void drawPath(const Path& path);

  virtual void fillPolygon(const PointF* pts, sysuint_t count);
  virtual void fillRect(const RectF& r);
  virtual void fillRects(const RectF* r, sysuint_t count);
  virtual void fillRound(const RectF& r, const PointF& radius);
  virtual void fillEllipse(const PointF& cp, const PointF& r);
  virtual void fillArc(const PointF& cp, const PointF& r, double start, double sweep);
  virtual void fillPath(const Path& path);

  // --------------------------------------------------------------------------
  // [Glyph / Text Drawing]
  // --------------------------------------------------------------------------

  virtual void drawGlyph(const Point& pt, const Glyph& glyph, const Rect* clip);
  virtual void drawGlyphSet(const Point& pt, const GlyphSet& glyphSet, const Rect* clip);

  virtual void drawText(const Point& p, const String32& text, const Font& font, const Rect* clip);
  virtual void drawText(const Rect& r, const String32& text, const Font& font, uint32_t align, const Rect* clip);

  // --------------------------------------------------------------------------
  // [Image drawing]
  // --------------------------------------------------------------------------

  virtual void drawImage(const Point& p, const Image& image, const Rect* irect);

  // --------------------------------------------------------------------------
  // [Flush]
  // --------------------------------------------------------------------------

  virtual void flush();
  void flushWithQuit();

  // --------------------------------------------------------------------------
  // [Properties]
  // --------------------------------------------------------------------------

  virtual err_t setProperty(const String32& name, const Value& value);
  virtual Value getProperty(const String32& name) const;

  // --------------------------------------------------------------------------
  // [Forward Declarations]
  // --------------------------------------------------------------------------

  struct Action;
  struct ActionAllocator;
  struct CapsState;
  struct ClipState;
  struct Context;
  struct Calculation;
  struct Command;
  struct SavedState;
  struct WorkerManager;
  struct WorkerTask;

  // --------------------------------------------------------------------------
  // [ClipState]
  // --------------------------------------------------------------------------

  struct FOG_HIDDEN ClipState
  {
    // [Construction / Destruction]

    ClipState();
    ClipState(const ClipState& other);
    ~ClipState();

    // [Ref / Deref]

    FOG_INLINE ClipState* ref() const;
    FOG_INLINE void deref();

    // [Operator Overload]

    ClipState& operator=(const ClipState& other);

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

  // --------------------------------------------------------------------------
  // [CapsState]
  // --------------------------------------------------------------------------

  struct FOG_HIDDEN CapsState
  {
    // [Construction / Destruction]

    CapsState();
    CapsState(const CapsState& other);
    ~CapsState();

    // [Ref / Deref]

    FOG_INLINE CapsState* ref() const;
    FOG_INLINE void deref();

    // [Operator Overload]

    CapsState& operator=(const CapsState& other);

    // [Members]

    mutable Atomic<sysuint_t> refCount;

    uint32_t op;

    Raster::Solid solidSource;
    Pattern patternSource;
    bool isSolidSource;

    double lineWidth;
    bool lineIsSimple;
    uint32_t lineCap;
    uint32_t lineJoin;
    double miterLimit;
    Vector<double> dashes;
    double dashOffset;

    uint32_t fillMode;

    Matrix transformations;
    double transformationsApproxScale;
    bool transformationsUsed;
  };

  // --------------------------------------------------------------------------
  // [SavedState]
  // --------------------------------------------------------------------------

  // State structure used by save() and restore() methods.
  struct FOG_HIDDEN SavedState
  {
    ClipState* clipState;
    CapsState* capsState;
    Raster::FunctionMap::RasterFuncs* rops;
    Raster::PatternContext* pctx;
  };

  // --------------------------------------------------------------------------
  // [Context]
  // --------------------------------------------------------------------------

  // Context is accessed always from only one thread.
  struct FOG_HIDDEN Context
  {
    // [Construction / Destruction]

    Context();
    ~Context();

    // [Buffer Manager]

    uint8_t* getBuffer(sysint_t size);
    void releaseBuffer(uint8_t* buffer);

    // [Members]

    // Owner of this context.
    PainterEngine_Raster* engine;

    // Clip state.
    ClipState* clipState;
    // Capabilities state.
    CapsState* capsState;

    // Raster functions.
    Raster::FunctionMap::RasterFuncs* rops;
    // Raster closure.
    Raster::Closure closure;

    // Pattern context.
    Raster::PatternContext* pctx;

    // Antigrain scanline rasterizer containers.
    AggScanlineP8 slP8;

    // Multithreading id, offset and delta. If id is equal to -1, multithreading
    // is disabled.
    int id;
    int offset; // If multithreading is disabled, offset is 0.
    int delta;  // If multithreading is disabled, delta is 1.

    // Static embedded buffer for fast alloc/free, see getBuffer(), releaseBuffer().
    uint8_t* buffer;
    sysint_t bufferSize;
    uint8_t bufferStatic[1024*8];

  private:
    FOG_DISABLE_COPY(Context)
  };

  // --------------------------------------------------------------------------
  // [ActionAllocator]
  // --------------------------------------------------------------------------

  struct FOG_HIDDEN ActionAllocator
  {
    // [Construction / Destruction]

    ActionAllocator();
    ~ActionAllocator();

    // [Block]

    struct Block
    {
      // [Block Size]

      enum { BlockSize = 32500 };

      // [Members]

      Block* next;

      sysuint_t size;
      sysuint_t pos;
      Atomic<sysuint_t> used;

      uint8_t memory[BlockSize];
    };

    // [Alloc / Free]

    void* alloc(sysuint_t size);
    void freeAll();

    // [Members]

    Block* blocks;

  private:
    FOG_DISABLE_COPY(ActionAllocator)
  };

  // --------------------------------------------------------------------------
  // [Action]
  // --------------------------------------------------------------------------

  struct FOG_HIDDEN Action
  {
    FOG_INLINE Action() {}
    FOG_INLINE ~Action() {}

    // [Abstract]

    virtual void run(Context* ctx) = 0;
    virtual void release() = 0;

    // [Memory Management]

    // Must be called from implemented @c release() method.
    FOG_INLINE void _releaseMemory();

    // [Members]

    ActionAllocator::Block* this_block;
    sysuint_t this_size;

  private:
    FOG_DISABLE_COPY(Action)
  };

  // --------------------------------------------------------------------------
  // [Command]
  // --------------------------------------------------------------------------

  struct FOG_HIDDEN Command : public Action
  {
    // [Construction / Destruction]

    FOG_INLINE Command() {};
    FOG_INLINE ~Command() {};

    // [Members]

    // This method destroys only general Command data.
    FOG_INLINE void _releaseObjects();

    // [Context]

    Atomic<sysuint_t> refCount;
    PainterEngine_Raster* engine;
    ClipState* clipState;
    CapsState* capsState;

    Raster::FunctionMap::RasterFuncs* rops;
    Raster::PatternContext* pctx;

    // [Status]

    enum Status
    {
      Wait = 0,
      Ready = 1,
      Skip = 2
    };

    Atomic<int> status;
    Calculation* calculation;
  };

  // --------------------------------------------------------------------------
  // [Command_Boxes]
  // --------------------------------------------------------------------------

  struct FOG_HIDDEN Command_Boxes : public Command
  {
    // [Construction / Destruction]

    FOG_INLINE Command_Boxes() {};
    FOG_INLINE ~Command_Boxes() {};

    // [Implementation]

    virtual void run(Context* ctx);
    virtual void release();

    // [Members]

    sysuint_t count;
    Box boxes[1];
  };

  // --------------------------------------------------------------------------
  // [Command_Image]
  // --------------------------------------------------------------------------

  struct FOG_HIDDEN Command_Image : public Command
  {
    // [Construction / Destruction]

    FOG_INLINE Command_Image() {};
    FOG_INLINE ~Command_Image() {};

    // [Implementation]

    virtual void run(Context* ctx);
    virtual void release();

    // [Members]

    Static<Image> image;

    Rect dst;
    Rect src;
  };

  // --------------------------------------------------------------------------
  // [Command_GlyphSet]
  // --------------------------------------------------------------------------

  struct FOG_HIDDEN Command_GlyphSet : public Command
  {
    // [Construction / Destruction]

    FOG_INLINE Command_GlyphSet() {};
    FOG_INLINE ~Command_GlyphSet() {};

    // [Implementation]

    virtual void run(Context* ctx);
    virtual void release();

    // [Members]

    Static<GlyphSet> glyphSet;

    Point pt;
    Box boundingBox;
  };

  // --------------------------------------------------------------------------
  // [Command_Path]
  // --------------------------------------------------------------------------

  struct FOG_HIDDEN Command_Path : public Command
  {
    // [Construction / Destruction]

    FOG_INLINE Command_Path() {};
    FOG_INLINE ~Command_Path() {};

    // [Implementation]

    virtual void run(Context* ctx);
    virtual void release();

    // [Members]

    Static<AggRasterizer> ras;
  };

  // --------------------------------------------------------------------------
  // [Calculation]
  // --------------------------------------------------------------------------

  struct FOG_HIDDEN Calculation : public Action
  {
    FOG_INLINE Calculation() {};
    FOG_INLINE ~Calculation() {};

    Command* relatedTo;
  };

  // --------------------------------------------------------------------------
  // [Calculation]
  // --------------------------------------------------------------------------

  struct FOG_HIDDEN Calculation_Path : public Calculation
  {
    FOG_INLINE Calculation_Path() {};
    FOG_INLINE ~Calculation_Path() {};

    // [Implementation]

    virtual void run(Context* ctx);
    virtual void release();

    // [Members]

    PainterEngine_Raster* engine;

    Static<Path> path;
    bool stroke;
  };

  // --------------------------------------------------------------------------
  // [WorkerTask]
  // --------------------------------------------------------------------------

  // This is task created per painter thread that contains all variables needed
  // to process painter commands in parallel. The goal is that condition variable
  // is shared across all painter threads so one signal will wake them all.
  struct FOG_HIDDEN WorkerTask : public Task
  {
    WorkerTask(Lock* condLock);
    virtual ~WorkerTask();

    virtual void run();
    virtual void destroy();

    // State
    enum State
    {
      Running,
      Waiting,
      Done
    };

    Atomic<int> state;

    // True if worker should quit from main loop.
    Atomic<int> shouldQuit;

    // Commands and calculations.
    volatile sysint_t currentCalculation;
    volatile sysint_t currentCommand;

    // Worker context.
    Context ctx;

    // Worker condition.
    ThreadCondition cond;
  };

  // --------------------------------------------------------------------------
  // [WorkerManager]
  // --------------------------------------------------------------------------

  // Structure shared across all workers (threads).
  struct FOG_HIDDEN WorkerManager
  {
    // [Construction / Destruction]

    WorkerManager();
    ~WorkerManager();

    // [Methods]

    // To call the lock must be locked!
    WorkerTask* wakeUpScheduled(WorkerTask* calledFrom);
    WorkerTask* wakeUpSleeping(WorkerTask* calledFrom);
    bool isCompleted();

    // [Members]

    ThreadPool* threadPool;             // Thread pool.
    sysuint_t numWorkers;               // Count of workers used in engine.

    Atomic<sysuint_t> finishedWorkers;  // Count of workers finished (used to quit).
    Atomic<sysuint_t> waitingWorkers;   // Count of workers waiting (for calculation).

    Lock lock;                          // Lock for synchronization primitives.
    ThreadCondition allFinishedCondition;
    ThreadEvent* releaseEvent;

    Thread* threads[MaxWorkers];
    Static<WorkerTask> tasks[MaxWorkers];

    // Commands and calculations allocator.
    ActionAllocator allocator;

    // Commands manager.
    volatile sysint_t commandsPosition;
    Command* volatile commandsData[MaxCommands];

    // Calculations manager.
    volatile sysint_t calculationsPosition;
    Calculation* volatile calculationsData[MaxCalculations];
  };

  // --------------------------------------------------------------------------
  // [Multithreading]
  // --------------------------------------------------------------------------

  void setMultithreaded(bool mt);

  // --------------------------------------------------------------------------
  // [Helpers]
  // --------------------------------------------------------------------------

  void _updateWorkRegion();
  void _updateTransform();

  void _setClipDefaults();
  void _setCapsDefaults();

  Raster::PatternContext* _getPatternContext();
  void _resetPatternContext();

  FOG_INLINE void _updateLineWidth()
  {
    ctx.capsState->lineIsSimple =
      (ctx.capsState->lineWidth == 1.0 && ctx.capsState->dashes.getLength() == 0);
  }

  bool _detachCaps();
  bool _detachClip();

  void _deleteStates();

  // --------------------------------------------------------------------------
  // [Serializers]
  // --------------------------------------------------------------------------

  // Serializers are always called from painter thread.

  void _serializeBoxes(const Box* box, sysuint_t count);
  void _serializeImage(const Rect& dst, const Image& image, const Rect& src);
  void _serializeGlyphSet(const Point& pt, const GlyphSet& glyphSet, const Rect* clip);
  void _serializePath(const Path& path, bool stroke);

  template<typename T>
  FOG_INLINE T* _createCommand(sysuint_t size = sizeof(T));

  template<typename T>
  FOG_INLINE T* _createCalculation(sysuint_t size = sizeof(T));

  void _postCommand(Command* cmd, Calculation* clc = NULL);

  // --------------------------------------------------------------------------
  // [Rasterizers]
  // --------------------------------------------------------------------------

  static bool _rasterizePath(Context* ctx, AggRasterizer& ras, const Path& path, bool stroke);

  // --------------------------------------------------------------------------
  // [Renderers]
  // --------------------------------------------------------------------------

  void _renderBoxes(Context* ctx, const Box* box, sysuint_t count);
  void _renderImage(Context* ctx, const Rect& dst, const Image& image, const Rect& src);
  void _renderGlyphSet(Context* ctx, const Point& pt, const GlyphSet& glyphSet, const Box& boundingBox);
  void _renderPath(Context* ctx, const AggRasterizer& ras);

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  uint8_t* _metaRaster;
  sysint_t _stride;

  int _metaWidth;
  int _metaHeight;

  int _format;
  sysint_t _bpp;

  // Temporary path
  Path tmpPath;

  // Antigrain context is declared outside of painter because we want that it
  // will be accessible through multiple threads. Each thread have it's own
  // context.
  Context ctx;

  // Context states LIFO buffer (for save() and restore() methods)
  Vector<SavedState> states;

  // If we are running in single-core environment it's better to embed one
  // antigrain rasterizer in device itself (and only this rasterizer will be
  // used).
  AggRasterizer ras;

  // Multithreading
  WorkerManager* workerManager;

  // --------------------------------------------------------------------------
  // [Statics]
  // --------------------------------------------------------------------------

  static ThreadPool* threadPool;
};

// ============================================================================
// [Fog::PainterEngine_Raster::ClipState]
// ============================================================================

PainterEngine_Raster::ClipState::ClipState()
{
  refCount.init(1);
}

PainterEngine_Raster::ClipState::ClipState(const ClipState& other) :
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

PainterEngine_Raster::ClipState::~ClipState()
{
}

FOG_INLINE PainterEngine_Raster::ClipState* PainterEngine_Raster::ClipState::ref() const
{
  refCount.inc();
  return const_cast<ClipState*>(this);
}

FOG_INLINE void PainterEngine_Raster::ClipState::deref()
{
  if (refCount.deref()) delete this;
}

PainterEngine_Raster::ClipState& PainterEngine_Raster::ClipState::operator=(const ClipState& other)
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
// [Fog::PainterEngine_Raster::CapsState]
// ============================================================================

PainterEngine_Raster::CapsState::CapsState()
{
  refCount.init(1);
}

PainterEngine_Raster::CapsState::CapsState(const CapsState& other) :
  op(other.op),
  solidSource(other.solidSource),
  patternSource(other.patternSource),
  isSolidSource(other.isSolidSource),
  lineWidth(other.lineWidth),
  lineIsSimple(other.lineIsSimple),
  lineCap(other.lineCap),
  lineJoin(other.lineJoin),
  dashes(other.dashes),
  dashOffset(other.dashOffset),
  miterLimit(other.miterLimit),
  fillMode(other.fillMode),
  transformations(other.transformations),
  transformationsApproxScale(1.0),
  transformationsUsed(other.transformationsUsed)
{
  refCount.init(1);
}

PainterEngine_Raster::CapsState::~CapsState()
{
}

FOG_INLINE PainterEngine_Raster::CapsState* PainterEngine_Raster::CapsState::ref() const
{
  refCount.inc();
  return const_cast<CapsState*>(this);
}

FOG_INLINE void PainterEngine_Raster::CapsState::deref()
{
  if (refCount.deref()) delete this;
}

PainterEngine_Raster::CapsState& PainterEngine_Raster::CapsState::operator=(const CapsState& other)
{
  op = other.op;
  solidSource = other.solidSource;
  patternSource = other.patternSource;
  isSolidSource = other.isSolidSource;
  lineWidth = other.lineWidth;
  lineIsSimple = other.lineIsSimple;
  lineCap = other.lineCap;
  lineJoin = other.lineJoin;
  dashes = other.dashes;
  dashOffset = other.dashOffset;
  miterLimit = other.miterLimit;
  fillMode = other.fillMode;
  transformations = other.transformations;
  transformationsUsed = other.transformationsUsed;

  return *this;
}

// ============================================================================
// [Fog::PainterEngine_Raster::Context]
// ============================================================================

PainterEngine_Raster::Context::Context()
{
  // Scanline must be reset before using it.
  slP8.reset(0, 1000);

  engine = NULL;

  clipState = NULL;
  capsState = NULL;

  rops = NULL;
  pctx = NULL;

  closure.closure = NULL;
  closure.dstPalette = NULL;
  closure.srcPalette = NULL;

  id = -1;
  offset = 0;
  delta = 1;

  buffer = bufferStatic;
  bufferSize = FOG_ARRAY_SIZE(bufferStatic);
}

PainterEngine_Raster::Context::~Context()
{
  if (buffer != bufferStatic) Memory::free(buffer);
}

uint8_t* PainterEngine_Raster::Context::getBuffer(sysint_t size)
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
// [Fog::PainterEngine_Raster::ActionAllocator]
// ============================================================================

PainterEngine_Raster::ActionAllocator::ActionAllocator() :
  blocks(NULL)
{
}

PainterEngine_Raster::ActionAllocator::~ActionAllocator()
{
  freeAll();
}

void* PainterEngine_Raster::ActionAllocator::alloc(sysuint_t size)
{
  FOG_ASSERT(size <= Block::BlockSize);

  if (!blocks || (blocks->size - blocks->pos) < size)
  {
    // Traverse to previous blocks and try to find complete free one.
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
  Action* action = (Action*)(blocks->memory + blocks->pos);

  blocks->pos += size;
  blocks->used.add(size);

  action->this_block = blocks;
  action->this_size = size;

  return (void*)action;
}

void PainterEngine_Raster::ActionAllocator::freeAll()
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
// [Fog::PainterEngine_Raster::Action]
// ============================================================================

FOG_INLINE void PainterEngine_Raster::Action::_releaseMemory()
{
  this_block->used.sub(this_size);
}

// ============================================================================
// [Fog::PainterEngine_Raster::Command]
// ============================================================================

FOG_INLINE void PainterEngine_Raster::Command::_releaseObjects()
{
  clipState->deref();
  capsState->deref();
  if (pctx && pctx->refCount.deref())
  {
    pctx->destroy(pctx);
    Memory::free(pctx);
  }
}

// ============================================================================
// [Fog::PainterEngine_Raster::Command_Boxes]
// ============================================================================

void PainterEngine_Raster::Command_Boxes::run(Context* ctx)
{
  ctx->engine->_renderBoxes(ctx, boxes, count);
}

void PainterEngine_Raster::Command_Boxes::release()
{
  _releaseObjects();
  _releaseMemory();
}

// ============================================================================
// [Fog::PainterEngine_Raster::Command_Image]
// ============================================================================

void PainterEngine_Raster::Command_Image::run(Context* ctx)
{
  ctx->engine->_renderImage(ctx, dst, image.instance(), src);
}

void PainterEngine_Raster::Command_Image::release()
{
  image.destroy();

  _releaseObjects();
  _releaseMemory();
}

// ============================================================================
// [Fog::PainterEngine_Raster::Command_GlyphSet]
// ============================================================================

void PainterEngine_Raster::Command_GlyphSet::run(Context* ctx)
{
  ctx->engine->_renderGlyphSet(ctx, pt, glyphSet.instance(), boundingBox);
}

void PainterEngine_Raster::Command_GlyphSet::release()
{
  glyphSet.destroy();

  _releaseObjects();
  _releaseMemory();
}

// ============================================================================
// [Fog::PainterEngine_Raster::Command_Path]
// ============================================================================

void PainterEngine_Raster::Command_Path::run(Context* ctx)
{
  ctx->engine->_renderPath(ctx, ras.instance());
}

void PainterEngine_Raster::Command_Path::release()
{
  ras.destroy();

  _releaseObjects();
  _releaseMemory();
}

// ============================================================================
// [Fog::PainterEngine_Raster::Calculation_Path]
// ============================================================================

void PainterEngine_Raster::Calculation_Path::run(Context* ctx)
{
  Command_Path* cmd = reinterpret_cast<Command_Path*>(relatedTo);

  bool ok = _rasterizePath(ctx, cmd->ras.instance(), path.instance(), stroke);
  cmd->status.set(ok ? Command::Ready : Command::Skip);
}

void PainterEngine_Raster::Calculation_Path::release()
{
  path.destroy();
  _releaseMemory();
}

// ============================================================================
// [Fog::PainterEngine_Raster::WorkerTask]
// ============================================================================

PainterEngine_Raster::WorkerTask::WorkerTask(Lock* lock) :
  cond(lock)
  //event(false, false)
{
  state.init(Running);
  shouldQuit.init(false);

  currentCalculation = 0;
  currentCommand = 0;
}

PainterEngine_Raster::WorkerTask::~WorkerTask()
{
}

void PainterEngine_Raster::WorkerTask::run()
{
  PainterEngine_Raster* engine = ctx.engine;
  WorkerManager* mgr = engine->workerManager;

  int id = ctx.id;
  int delta = ctx.delta;

#if defined(FOG_DEBUG_RASTER_SYNCHRONIZATION)
  fog_debug("Fog::Painter[Worker #%d]::run() - ThreadID=%d", ctx.id, Thread::current()->getId());
#endif // FOG_DEBUG_RASTER_SYNCHRONIZATION

  for (;;)
  {
    // Do calculations and commands.
    int cont = 0;
    bool wait = false;

    do {
      wait = false;

      // Do calculations (highest priority than commands).
      while (currentCalculation < mgr->calculationsPosition)
      {
        // Here is high thread concurrency and we are doing this without locking,
        // atomic operations helps us to get Calculation* or NULL. If we get NULL,
        // it's being calculated or done.
        Calculation** pclc = (Calculation**)&mgr->calculationsData[currentCalculation];
        Calculation* clc = *pclc;

        if (clc && AtomicOperation<Calculation*>::cmpXchg(pclc, clc, NULL))
        {
#if defined(FOG_DEBUG_RASTER_COMMANDS)
          fog_debug("Fog::Painter[Worker #%d]::run() - calculation %d (%p)", ctx.id, (int)currentCalculation, clc);
#endif // FOG_DEBUG_RASTER_COMMANDS

          // If we are here, we won a battle with other threads
          // and Calculation* is ours.
          Command* cmd = clc->relatedTo;
          ctx.clipState = cmd->clipState;
          ctx.capsState = cmd->capsState;
          ctx.rops = cmd->rops;
          ctx.pctx = cmd->pctx;
          ctx.closure.closure = ctx.rops->closure;

          clc->run(&ctx);
          clc->release();

          AutoLock locked(mgr->lock);
          mgr->wakeUpScheduled(this);
        }

        currentCalculation++;
        cont = 0;
      }

      // Do command.
      while (currentCommand < mgr->commandsPosition)
      {
        Command* cmd = mgr->commandsData[currentCommand];

#if defined(FOG_DEBUG_RASTER_COMMANDS)
        static const char* statusMsg[] = { "Wait", "Ready", "Skip" };
        fog_debug("Fog::Painter[Worker #%d]::run() - command %d (%p) status=%s", ctx.id, (int)currentCommand, cmd, statusMsg[cmd->status.get()]);
#endif // FOG_DEBUG_RASTER_COMMANDS

        switch (cmd->status.get())
        {
          case Command::Ready:
          {
            // If meta origin or user origin is set, it's needed to calculate new
            // (correct) offset variable or different threads can paint into the
            // same area.
            int workerOffset = (id - cmd->clipState->workOrigin.getY()) % delta;
            if (workerOffset < 0) workerOffset += delta;

            ctx.offset = workerOffset;

            ctx.clipState = cmd->clipState;
            ctx.capsState = cmd->capsState;
            ctx.rops = cmd->rops;
            ctx.pctx = cmd->pctx;
            ctx.closure.closure = ctx.rops->closure;

            cmd->run(&ctx);
            if (cmd->refCount.deref()) cmd->release();

            currentCommand++;
            cont = 0;
            break;
          }

          case Command::Skip:
          {
            currentCommand++;
            if (cmd->refCount.deref()) cmd->release();
            cont = 0;
            break;
          }

          case Command::Wait:
          {
            wait = true;
            goto skipCommands;
          }
        }
      }
skipCommands:
      cont++;
      // We try two times to get work before we wait or quit.
    } while (cont < 2);

    {
      AutoLock locked(mgr->lock);

      if (currentCalculation < mgr->calculationsPosition || (currentCommand < mgr->commandsPosition && !wait))
      {
        continue;
      }

#if defined(FOG_DEBUG_RASTER_SYNCHRONIZATION)
      fog_debug("Fog::Painter[Worker #%d]::run() - going to wait (currently waiting=%d)",
        ctx.id,
        (int)mgr->waitingWorkers.get());
#endif // FOG_DEBUG_RASTER_SYNCHRONIZATION

      state.set(Waiting);

      if (mgr->waitingWorkers.addXchg(1) + 1 == mgr->numWorkers)
      {
        // If count of waiting workers will be now count of workers (so this is
        // the last running one), we need to check if everything was completed
        // and if we can fire allFinishedCondition signal.
        if (mgr->isCompleted())
        {
#if defined(FOG_DEBUG_RASTER_SYNCHRONIZATION)
          fog_debug("Fog::Painter[Worker #%d]::run() - everything done, signaling allFinished...", ctx.id);
#endif // FOG_DEBUG_RASTER_SYNCHRONIZATION
          mgr->allFinishedCondition.signal();
        }
        else
        {
          if (mgr->wakeUpScheduled(this) == this)
          {
            mgr->waitingWorkers.dec();
            continue;
          }
        }
      }

      if (shouldQuit.get() == true && currentCommand == mgr->commandsPosition)
      {
#if defined(FOG_DEBUG_RASTER_SYNCHRONIZATION)
        fog_debug("Fog::Painter[Worker #%d]::run() - quitting...", ctx.id);
#endif // FOG_DEBUG_RASTER_SYNCHRONIZATION

        // Set Tunning state so wakeUpSleeping() can't return us.
        state.set(Running);
        mgr->wakeUpSleeping(this);
        state.set(Done);
        return;
      }

      cond.wait();
      state.set(Running);
      mgr->waitingWorkers.dec();

      if (shouldQuit.get() == true)
        mgr->wakeUpSleeping(this);
      else
        mgr->wakeUpScheduled(this);
    }
  }
}

void PainterEngine_Raster::WorkerTask::destroy()
{
  PainterEngine_Raster* engine = ctx.engine;
  WorkerManager* mgr = engine->workerManager;

#if defined(FOG_DEBUG_RASTER_SYNCHRONIZATION)
  fog_debug("Fog::Painter[Worker #%d]::WorkerTask::destroy()", ctx.id);
#endif // FOG_DEBUG_RASTER_SYNCHRONIZATION

  if (mgr->finishedWorkers.addXchg(1) + 1 == mgr->numWorkers)
  {
#if defined(FOG_DEBUG_RASTER_SYNCHRONIZATION)
    fog_debug("Fog::Painter[Worker #%d]::WorkerTask::destroy() - I'm last, signaling release event...", ctx.id);
#endif // FOG_DEBUG_RASTER_SYNCHRONIZATION

    mgr->releaseEvent->signal();
  }
}

// ============================================================================
// [Fog::PainterEngine_Raster::WorkerManager]
// ============================================================================

PainterEngine_Raster::WorkerManager::WorkerManager() :
  allFinishedCondition(&lock),
  releaseEvent(NULL)
{
}

PainterEngine_Raster::WorkerManager::~WorkerManager()
{
}

PainterEngine_Raster::WorkerTask* PainterEngine_Raster::WorkerManager::wakeUpScheduled(WorkerTask* calledFrom)
{
  for (sysuint_t i = 0; i < numWorkers; i++)
  {
    WorkerTask* task = tasks[i].instancep();

    if (task->state.get() == WorkerTask::Waiting && task->currentCommand < commandsPosition)
    {
      if (calledFrom != task) task->cond.signal();
      return task;
    }
  }

  return NULL;
}

PainterEngine_Raster::WorkerTask* PainterEngine_Raster::WorkerManager::wakeUpSleeping(WorkerTask* calledFrom)
{
  for (sysuint_t i = 0; i < numWorkers; i++)
  {
    WorkerTask* task = tasks[i].instancep();

    if (task->state.get() == WorkerTask::Waiting)
    {
      AutoLock locked(lock);
      if (calledFrom != task) task->cond.signal();
      return task;
    }
  }

  return NULL;
}

bool PainterEngine_Raster::WorkerManager::isCompleted()
{
  sysuint_t done = 0;

  for (sysuint_t i = 0; i < numWorkers; i++)
  {
    WorkerTask* task = tasks[i].instancep();
    if (task->currentCommand == commandsPosition) done++;
  }

  return done == numWorkers;
}


// ============================================================================
// [Fog::PainterEngine_Raster - Construction / Destruction]
// ============================================================================

ThreadPool* PainterEngine_Raster::threadPool;

PainterEngine_Raster::PainterEngine_Raster(uint8_t* pixels, int width, int height, sysint_t stride, int format, int hints) :
  _metaRaster(pixels),
  _stride(stride),
  _metaWidth(width),
  _metaHeight(height),
  _format(format),
  _bpp(Image::formatToBytesPerPixel(format)),
  workerManager(NULL)
{
  ctx.engine = this;
  ctx.clipState = new(std::nothrow) ClipState();
  ctx.capsState = new(std::nothrow) CapsState();

  ras.gamma(ColorLut::linearLut);

  // Setup clip state.
  _setClipDefaults();

  // Setup caps state.
  _setCapsDefaults();

  // Setup multithreading if possible. If the painting buffer if too small, we
  // will not use multithreading, because it has no sense.
  if (cpuInfo->numberOfProcessors > 1 && (hints & Painter::HintNoMultithreading) == 0)
  {
    sysuint_t total = (sysuint_t)width * (sysuint_t)height;

    if (total >= MinSizeThreshold)
    {
#if defined(FOG_DEBUG_RASTER_SYNCHRONIZATION)
      fog_debug("Fog::Painter::new() - Image %dx%d (total %d), selected for multithreading", width, height, width * height);
#endif // FOG_DEBUG_RASTER_SYNCHRONIZATION
      setMultithreaded(true);
    }
  }
}

PainterEngine_Raster::~PainterEngine_Raster()
{
  if (workerManager) setMultithreaded(false);

  _deleteStates();
  ctx.clipState->deref();
  ctx.capsState->deref();
}

// ============================================================================
// [Fog::PainterEngine_Raster - Meta]
// ============================================================================

int PainterEngine_Raster::getWidth() const
{
  return _metaWidth;
}

int PainterEngine_Raster::getHeight() const
{
  return _metaHeight;
}

int PainterEngine_Raster::getFormat() const
{
  return _format;
}

void PainterEngine_Raster::setMetaVariables(
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
    _deleteStates();

    ctx.clipState->userOrigin.set(0, 0);
    ctx.clipState->userRegion.clear();
    ctx.clipState->userRegionUsed = false;

    if (_detachCaps())
      _setCapsDefaults();
  }

  _updateWorkRegion();
}

void PainterEngine_Raster::setMetaOrigin(const Point& pt)
{
  if (ctx.clipState->metaOrigin == pt) return;
  if (!_detachClip()) return;

  ctx.clipState->metaOrigin = pt;
  _updateWorkRegion();
}

void PainterEngine_Raster::setUserOrigin(const Point& pt)
{
  if (ctx.clipState->userOrigin == pt) return;
  if (!_detachClip()) return;

  ctx.clipState->userOrigin = pt;
  _updateWorkRegion();
}

void PainterEngine_Raster::translateMetaOrigin(const Point& pt)
{
  if (pt.getX() == 0 && pt.getY() == 0) return;
  if (!_detachClip()) return;

  ctx.clipState->metaOrigin += pt;
  _updateWorkRegion();
}

void PainterEngine_Raster::translateUserOrigin(const Point& pt)
{
  if (pt.getX() == 0 && pt.getY() == 0) return;
  if (!_detachClip()) return;

  ctx.clipState->userOrigin += pt;
  _updateWorkRegion();
}

void PainterEngine_Raster::setUserRegion(const Rect& r)
{
  if (!_detachClip()) return;

  ctx.clipState->userRegion = r;
  ctx.clipState->userRegionUsed = true;
  _updateWorkRegion();
}

void PainterEngine_Raster::setUserRegion(const Region& r)
{
  if (!_detachClip()) return;

  ctx.clipState->userRegion = r;
  ctx.clipState->userRegionUsed = true;
  _updateWorkRegion();
}

void PainterEngine_Raster::resetMetaVars()
{
  if (!_detachClip()) return;

  ctx.clipState->metaOrigin.set(0, 0);
  ctx.clipState->metaRegion.clear();
  ctx.clipState->metaRegionUsed = false;
  _updateWorkRegion();
}

void PainterEngine_Raster::resetUserVars()
{
  if (!_detachClip()) return;

  ctx.clipState->userOrigin.set(0, 0);
  ctx.clipState->userRegion.clear();
  ctx.clipState->userRegionUsed = false;
  _updateWorkRegion();
}

Point PainterEngine_Raster::getMetaOrigin() const
{
  return ctx.clipState->metaOrigin;
}

Point PainterEngine_Raster::getUserOrigin() const
{
  return ctx.clipState->userOrigin;
}

Region PainterEngine_Raster::getMetaRegion() const
{
  return ctx.clipState->metaRegion;
}

Region PainterEngine_Raster::getUserRegion() const
{
  return ctx.clipState->userRegion;
}

bool PainterEngine_Raster::isMetaRegionUsed() const
{
  return ctx.clipState->metaRegionUsed;
}

bool PainterEngine_Raster::isUserRegionUsed() const
{
  return ctx.clipState->userRegionUsed;
}

// ============================================================================
// [Fog::PainterEngine_Raster - Operator]
// ============================================================================

void PainterEngine_Raster::setOperator(uint32_t op)
{
  if (op >= CompositeCount) return;
  if (!_detachCaps()) return;

  ctx.capsState->op = op;
  ctx.rops = Raster::getRasterOps(_format, (int)op);
  ctx.closure.closure = ctx.rops->closure;
}

uint32_t PainterEngine_Raster::getOperator() const
{
  return ctx.capsState->op;
}

// ============================================================================
// [Fog::PainterEngine_Raster - Source]
// ============================================================================

void PainterEngine_Raster::setSource(const Rgba& rgba)
{
  if (!_detachCaps()) return;

  ctx.capsState->solidSource.rgba = rgba.value;
  ctx.capsState->solidSource.rgbp = Raster::premultiply(rgba.value);
  ctx.capsState->isSolidSource = true;

  // Free pattern resource if not needed.
  if (!ctx.capsState->patternSource.isNull())
  {
    ctx.capsState->patternSource.free();
    _resetPatternContext();
  }
}

void PainterEngine_Raster::setSource(const Pattern& pattern)
{
  if (pattern.isSolid())
  {
    setSource(pattern.getColor());
    return;
  }

  if (!_detachCaps()) return;

  ctx.capsState->solidSource.rgba = 0xFFFFFFFF;
  ctx.capsState->solidSource.rgbp = 0xFFFFFFFF;
  ctx.capsState->isSolidSource = false;
  ctx.capsState->patternSource = pattern;
  _resetPatternContext();
}

Rgba PainterEngine_Raster::sourceRgba()
{
  return Rgba(ctx.capsState->solidSource.rgba);
}

Pattern PainterEngine_Raster::sourcePattern()
{
  Pattern pattern;
  if (ctx.capsState->isSolidSource)
    pattern.setColor(ctx.capsState->solidSource.rgba);
  else
    pattern = ctx.capsState->patternSource;
  return pattern;
}

// ============================================================================
// [Fog::PainterEngine_Raster - Parameters]
// ============================================================================

void PainterEngine_Raster::setLineParams(const LineParams& params)
{
  if (!_detachCaps()) return;

  ctx.capsState->lineWidth  = params.lineWidth;
  ctx.capsState->lineCap    = params.lineCap;
  ctx.capsState->lineJoin   = params.lineJoin;
  ctx.capsState->miterLimit = params.miterLimit;
  ctx.capsState->dashes     = params.dashes;
  ctx.capsState->dashOffset = params.dashOffset;

  if (ctx.capsState->lineCap >= LineCapInvalid) ctx.capsState->lineCap = LineCapSquare;
  if (ctx.capsState->lineJoin >= LineJoinInvalid) ctx.capsState->lineJoin = LineJoinMiter;

  _updateLineWidth();
}

void PainterEngine_Raster::getLineParams(LineParams& params) const
{
  params.lineWidth  = ctx.capsState->lineWidth;
  params.lineCap    = ctx.capsState->lineCap;
  params.lineJoin   = ctx.capsState->lineJoin;
  params.miterLimit = ctx.capsState->miterLimit;
  params.dashes     = ctx.capsState->dashes;
  params.dashOffset = ctx.capsState->dashOffset;
}

void PainterEngine_Raster::setLineWidth(double lineWidth)
{
  if (ctx.capsState->lineWidth == lineWidth) return;
  if (!_detachCaps()) return;

  ctx.capsState->lineWidth = lineWidth;
  _updateLineWidth();
}

double PainterEngine_Raster::getLineWidth() const
{
  return ctx.capsState->lineWidth;
}

void PainterEngine_Raster::setLineCap(uint32_t lineCap)
{
  if (ctx.capsState->lineCap == lineCap) return;
  if (!_detachCaps()) return;

  ctx.capsState->lineCap = lineCap;
}

uint32_t PainterEngine_Raster::getLineCap() const
{
  return ctx.capsState->lineCap;
}

void PainterEngine_Raster::setLineJoin(uint32_t lineJoin)
{
  if (ctx.capsState->lineJoin == lineJoin) return;
  if (!_detachCaps()) return;

  ctx.capsState->lineJoin = lineJoin;
}

uint32_t PainterEngine_Raster::getLineJoin() const
{
  return ctx.capsState->lineJoin;
}

void PainterEngine_Raster::setDashes(const double* dashes, sysuint_t count)
{
  if (!_detachCaps()) return;

  ctx.capsState->dashes.clear();
  for (sysuint_t i = count; i; i--, dashes++) ctx.capsState->dashes.append(*dashes);
  _updateLineWidth();
}

void PainterEngine_Raster::setDashes(const Vector<double>& dashes)
{
  if (!_detachCaps()) return;

  ctx.capsState->dashes = dashes;
  _updateLineWidth();
}

Vector<double> PainterEngine_Raster::getDashes() const
{
  return ctx.capsState->dashes;
}

void PainterEngine_Raster::setDashOffset(double offset)
{
  if (ctx.capsState->dashOffset == offset) return;
  if (!_detachCaps()) return;

  ctx.capsState->dashOffset = offset;
  _updateLineWidth();
}

double PainterEngine_Raster::getDashOffset() const
{
  return ctx.capsState->dashOffset;
}

void PainterEngine_Raster::setMiterLimit(double miterLimit)
{
  if (ctx.capsState->miterLimit == miterLimit) return;
  if (!_detachCaps()) return;

  ctx.capsState->miterLimit = miterLimit;
}

double PainterEngine_Raster::getMiterLimit() const
{
  return ctx.capsState->miterLimit;
}

void PainterEngine_Raster::setFillMode(uint32_t mode)
{
  if (ctx.capsState->fillMode == mode) return;
  if (!_detachCaps()) return;

  ctx.capsState->fillMode = mode;
}

uint32_t PainterEngine_Raster::getFillMode()
{
  return ctx.capsState->fillMode;
}

// ============================================================================
// [Fog::PainterEngine_Raster - Transformations]
// ============================================================================

void PainterEngine_Raster::setMatrix(const Matrix& m)
{
  if (!_detachCaps()) return;

  ctx.capsState->transformations = m;
  _updateTransform();
}

void PainterEngine_Raster::resetMatrix()
{
  if (!_detachCaps()) return;

  ctx.capsState->transformations = Matrix();
  ctx.capsState->transformationsApproxScale = 1.0;
  ctx.capsState->transformationsUsed = false;
}

Matrix PainterEngine_Raster::getMatrix() const
{
  return ctx.capsState->transformations;
}

void PainterEngine_Raster::rotate(double angle)
{
  if (!_detachCaps()) return;

  ctx.capsState->transformations *= Matrix::fromRotation(angle);
  _updateTransform();
}

void PainterEngine_Raster::scale(double sx, double sy)
{
  if (!_detachCaps()) return;

  ctx.capsState->transformations *= Matrix::fromScale(sx, sy);
  _updateTransform();
}

void PainterEngine_Raster::skew(double sx, double sy)
{
  if (!_detachCaps()) return;

  ctx.capsState->transformations *= Matrix::fromSkew(sx, sy);
  _updateTransform();
}

void PainterEngine_Raster::translate(double x, double y)
{
  if (!_detachCaps()) return;

  ctx.capsState->transformations *= Matrix::fromTranslation(x, y);
  _updateTransform();
}

void PainterEngine_Raster::affine(const Matrix& m)
{
  if (!_detachCaps()) return;

  ctx.capsState->transformations *= m;
  _updateTransform();
}

void PainterEngine_Raster::parallelogram(
  double x1, double y1, double x2, double y2, const double* para)
{
  if (!_detachCaps()) return;

  ctx.capsState->transformations *= Matrix(x1, y1, x2, y2, para);
  _updateTransform();
}

void PainterEngine_Raster::viewport(
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
  ctx.capsState->transformations *= *((const Matrix *)&aff);
  _updateTransform();
}

void PainterEngine_Raster::worldToScreen(PointF* pt) const
{
  FOG_ASSERT(pt != NULL);

  if (ctx.capsState->transformationsUsed)
    ctx.capsState->transformations.transform(&pt->x, &pt->y);
}

void PainterEngine_Raster::screenToWorld(PointF* pt) const
{
  FOG_ASSERT(pt != NULL);

  if (ctx.capsState->transformationsUsed)
    ctx.capsState->transformations.transformInv(&pt->x, &pt->y);
}

void PainterEngine_Raster::worldToScreen(double* scalar) const
{
  FOG_ASSERT(scalar != NULL);

  if (ctx.capsState->transformationsUsed)
  {
    PointF p1(0, 0);
    PointF p2(*scalar, *scalar);

    PainterEngine_Raster::worldToScreen(&p1);
    PainterEngine_Raster::worldToScreen(&p2);

    double dx = p2.getX() - p1.getX();
    double dy = p2.getY() - p1.getY();

    *scalar = sqrt(dx * dx + dy * dy) * 0.7071068;
  }
}

void PainterEngine_Raster::screenToWorld(double* scalar) const
{
  FOG_ASSERT(scalar != NULL);

  if (ctx.capsState->transformationsUsed)
  {
    PointF p1(0, 0);
    PointF p2(*scalar, *scalar);

    PainterEngine_Raster::screenToWorld(&p1);
    PainterEngine_Raster::screenToWorld(&p2);

    double dx = p2.getX() - p1.getX();
    double dy = p2.getY() - p1.getY();

    *scalar = sqrt(dx * dx + dy * dy) * 0.7071068;
  }
}

void PainterEngine_Raster::alignPoint(PointF* pt) const
{
  FOG_ASSERT(pt != NULL);

  PainterEngine_Raster::worldToScreen(pt);
  pt->setX(floor(pt->getX()) + 0.5);
  pt->setY(floor(pt->getY()) + 0.5);
  PainterEngine_Raster::screenToWorld(pt);
}

// ============================================================================
// [Fog::PainterEngine_Raster - State]
// ============================================================================

void PainterEngine_Raster::save()
{
  SavedState s;

  s.clipState = ctx.clipState->ref();
  s.capsState = ctx.capsState->ref();
  s.rops = ctx.rops;
  s.pctx = NULL;

  if (ctx.pctx && ctx.pctx->initialized)
  {
    s.pctx = ctx.pctx;
    s.pctx->refCount.inc();
  }

  states.append(s);
}

void PainterEngine_Raster::restore()
{
  if (states.isEmpty()) return;

  SavedState s = states.takeLast();

  ctx.clipState->deref();
  ctx.capsState->deref();

  if (ctx.pctx && ctx.pctx->refCount.deref())
  {
    if (ctx.pctx->initialized) ctx.pctx->destroy(ctx.pctx);
    Memory::free(ctx.pctx);
  }

  ctx.clipState = s.clipState;
  ctx.capsState = s.capsState;
  ctx.rops = s.rops;
  ctx.pctx = s.pctx;
  ctx.closure.closure = ctx.rops->closure;
}

// ============================================================================
// [Fog::PainterEngine_Raster - Raster drawing]
// ============================================================================

void PainterEngine_Raster::clear()
{
  if (ctx.clipState->clipSimple)
    _serializeBoxes(&ctx.clipState->clipBox, 1);
  else
    _serializeBoxes(ctx.clipState->workRegion.cData(), ctx.clipState->workRegion.getLength());
}

void PainterEngine_Raster::drawPoint(const Point& p)
{
  PainterEngine_Raster::drawPoint(
    PointF((double)p.getX() + 0.5, (double)p.getY() + 0.5));
}

void PainterEngine_Raster::drawLine(const Point& start, const Point& end)
{
  PainterEngine_Raster::drawLine(
    PointF((double)start.getX() + 0.5, (double)start.getY() + 0.5),
    PointF((double)end.getX() + 0.5, (double)end.getY() + 0.5));
}

void PainterEngine_Raster::drawRect(const Rect& r)
{
  if (!r.isValid()) return;

  if (ctx.capsState->transformationsUsed || !ctx.capsState->lineIsSimple)
  {
    PainterEngine_Raster::drawRect(
      RectF(
        (double)r.getX() + 0.5,
        (double)r.getY() + 0.5,
        (double)r.getWidth(),
        (double)r.getHeight()));
    return;
  }

  Box box[4];
  sysuint_t count = 4;

  if (r.getWidth() <= 2 || r.getHeight() <= 2)
  {
    box[0].set(r.getX1(), r.getY1(), r.getX2(), r.getY2());
    count = 1;
  }
  else
  {
    box[0].set(r.getX1()  , r.getY1()  , r.getX2()  , r.getY1()+1);
    box[1].set(r.getX1()  , r.getY1()+1, r.getX1()+1, r.getY2()-1);
    box[2].set(r.getX2()-1, r.getY1()+1, r.getX2()  , r.getY2()-1);
    box[3].set(r.getX1()  , r.getY2()-1, r.getX2()  , r.getY2()  );
  }

  if (ctx.clipState->clipSimple)
  {
    if (!ctx.clipState->clipBox.subsumes(r))
    {
      for (sysuint_t i = 0; i < count; i++) Box::intersect(box[i], box[i], ctx.clipState->clipBox);
    }
    PainterEngine_Raster::_serializeBoxes(box, count);
  }
  else
  {
    TemporaryRegion<4> regionBox;
    TemporaryRegion<16> regionISect;
    regionBox.set(box, count);

    Region::intersect(regionISect, regionBox, ctx.clipState->workRegion);
    if (!regionISect.getLength()) return;

    PainterEngine_Raster::_serializeBoxes(regionISect.cData(), regionISect.getLength());
  }
}

void PainterEngine_Raster::drawRound(const Rect& r, const Point& radius)
{
  PainterEngine_Raster::drawRound(
    RectF((double)r.getX() + 0.5, (double)r.getY() + 0.5, r.getWidth(), r.getHeight()),
    PointF((double)radius.getX(), (double)radius.getY()));
}

void PainterEngine_Raster::fillRect(const Rect& r)
{
  if (!r.isValid()) return;

  if (ctx.capsState->transformationsUsed)
  {
    PainterEngine_Raster::fillRect(
      RectF(
        (double)r.getX(),
        (double)r.getY(),
        (double)r.getWidth(),
        (double)r.getHeight()));
    return;
  }

  Box box(r.getX1(), r.getY1(), r.getX2(), r.getY2());

  if (ctx.clipState->clipSimple)
  {
    Box::intersect(box, box, ctx.clipState->clipBox);
    if (!box.isValid()) return;

    PainterEngine_Raster::_serializeBoxes(&box, 1);
  }
  else
  {
    TemporaryRegion<1> regionBox;
    TemporaryRegion<16> regionISect;
    regionBox.set(&box, 1);

    Region::intersect(regionISect, regionBox, ctx.clipState->workRegion);
    if (!regionISect.getLength()) return;

    PainterEngine_Raster::_serializeBoxes(regionISect.cData(), regionISect.getLength());
  }
}

void PainterEngine_Raster::fillRects(const Rect* r, sysuint_t count)
{
  if (!count) return;

  if (ctx.capsState->transformationsUsed)
  {
    tmpPath.clear();
    for (sysuint_t i = 0; i < count; i++)
    {
      if (r[i].isValid()) tmpPath.addRect(
        RectF(
          (double)r[i].getX(),
          (double)r[i].getY(),
          (double)r[i].getWidth(),
          (double)r[i].getHeight()));
    }
    fillPath(tmpPath);
    return;
  }

  Region region;
  region.set(r, count);
  if (region.isEmpty()) return;

  if (ctx.clipState->clipSimple)
  {
    if (ctx.clipState->clipBox.subsumes(region.extents()))
    {
      _serializeBoxes(region.cData(), region.getLength());
      return;
    }
    region.intersect(ctx.clipState->clipBox);
    if (region.isEmpty()) return;

    _serializeBoxes(region.cData(), region.getLength());
  }
  else
  {
    Region regionISect;
    Region::intersect(regionISect, ctx.clipState->workRegion, region);
    if (regionISect.isEmpty()) return;

    _serializeBoxes(regionISect.cData(), regionISect.getLength());
  }
}

void PainterEngine_Raster::fillRound(const Rect& r, const Point& radius)
{
  PainterEngine_Raster::fillRound(
    RectF((double)r.getX(), (double)r.getY(), r.getWidth(), r.getHeight()),
    PointF(radius.getX(), radius.getY()));
}

void PainterEngine_Raster::fillRegion(const Region& region)
{
  if (ctx.clipState->clipSimple && ctx.clipState->clipBox.subsumes(region.extents()))
  {
    _serializeBoxes(region.cData(), region.getLength());
  }
  else
  {
    TemporaryRegion<16> dst;
    Region::intersect(dst, ctx.clipState->workRegion, region);
    if (dst.isEmpty()) return;

    _serializeBoxes(dst.cData(), dst.getLength());
  }
}

// ============================================================================
// [Fog::PainterEngine_Raster - Vector drawing]
// ============================================================================

void PainterEngine_Raster::drawPoint(const PointF& p)
{
  tmpPath.clear();
  tmpPath.moveTo(p);
  tmpPath.lineTo(p.getX(), p.getY() + 0.0001);
  _serializePath(tmpPath, true);
}

void PainterEngine_Raster::drawLine(const PointF& start, const PointF& end)
{
  tmpPath.clear();
  tmpPath.moveTo(start);
  tmpPath.lineTo(end);
  _serializePath(tmpPath, true);
}

void PainterEngine_Raster::drawLine(const PointF* pts, sysuint_t count)
{
  if (!count) return;

  tmpPath.clear();
  tmpPath.moveTo(pts[0]);
  if (count > 1)
    tmpPath.lineTo(pts + 1, count - 1);
  else
    tmpPath.lineTo(pts[0].getX(), pts[0].getY() + 0.0001);
  _serializePath(tmpPath, true);
}

void PainterEngine_Raster::drawPolygon(const PointF* pts, sysuint_t count)
{
  if (!count) return;

  tmpPath.clear();
  tmpPath.moveTo(pts[0]);
  if (count > 1)
    tmpPath.lineTo(pts + 1, count - 1);
  else
    tmpPath.lineTo(pts[0].getX(), pts[0].getY() + 0.0001);
  tmpPath.closePolygon();
  _serializePath(tmpPath, true);
}

void PainterEngine_Raster::drawRect(const RectF& r)
{
  if (!r.isValid()) return;

  tmpPath.clear();
  tmpPath.addRect(r);
  _serializePath(tmpPath, true);
}

void PainterEngine_Raster::drawRects(const RectF* r, sysuint_t count)
{
  if (!count) return;

  tmpPath.clear();
  tmpPath.addRects(r, count);
  _serializePath(tmpPath, true);
}

void PainterEngine_Raster::drawRound(const RectF& r, const PointF& radius)
{
  tmpPath.clear();
  tmpPath.addRound(r, radius);
  _serializePath(tmpPath, true);
}

void PainterEngine_Raster::drawEllipse(const PointF& cp, const PointF& r)
{
  tmpPath.clear();
  tmpPath.addEllipse(cp, r);
  _serializePath(tmpPath, true);
}

void PainterEngine_Raster::drawArc(const PointF& cp, const PointF& r, double start, double sweep)
{
  tmpPath.clear();
  tmpPath.addArc(cp, r, start, sweep);
  _serializePath(tmpPath, true);
}

void PainterEngine_Raster::drawPath(const Path& path)
{
  _serializePath(path, true);
}

void PainterEngine_Raster::fillPolygon(const PointF* pts, sysuint_t count)
{
  if (!count) return;

  tmpPath.clear();
  tmpPath.moveTo(pts[0]);
  if (count > 1)
    tmpPath.lineTo(pts + 1, count - 1);
  else
    tmpPath.lineTo(pts[0].getX(), pts[0].getY() + 0.0001);
  tmpPath.closePolygon();
  _serializePath(tmpPath, false);
}

void PainterEngine_Raster::fillRect(const RectF& r)
{
  if (!r.isValid()) return;

  tmpPath.clear();
  tmpPath.addRect(r);
  _serializePath(tmpPath, false);
}

void PainterEngine_Raster::fillRects(const RectF* r, sysuint_t count)
{
  if (!count) return;

  tmpPath.clear();
  tmpPath.addRects(r, count);
  _serializePath(tmpPath, false);
}

void PainterEngine_Raster::fillRound(const RectF& r, const PointF& radius)
{
  tmpPath.clear();
  tmpPath.addRound(r, radius);
  _serializePath(tmpPath, false);
}

void PainterEngine_Raster::fillEllipse(const PointF& cp, const PointF& r)
{
  tmpPath.clear();
  tmpPath.addEllipse(cp, r);
  _serializePath(tmpPath, false);
}

void PainterEngine_Raster::fillArc(const PointF& cp, const PointF& r, double start, double sweep)
{
  tmpPath.clear();
  tmpPath.addArc(cp, r, start, sweep);
  _serializePath(tmpPath, false);
}

void PainterEngine_Raster::fillPath(const Path& path)
{
  _serializePath(path, false);
}

// ============================================================================
// [Fog::PainterEngine_Raster - Glyph / Text Drawing]
// ============================================================================

void PainterEngine_Raster::drawGlyph(const Point& pt, const Glyph& glyph, const Rect* clip)
{
  TemporaryGlyphSet<1> glyphSet;
  err_t err;

  if ( (err = glyphSet.begin(1)) ) return;
  glyphSet._add(glyph._d->ref());
  if ( (err = glyphSet.end()) ) return;

  _serializeGlyphSet(pt, glyphSet, clip);
}

void PainterEngine_Raster::drawGlyphSet(const Point& pt, const GlyphSet& glyphSet, const Rect* clip)
{
  _serializeGlyphSet(pt, glyphSet, clip);
}

void PainterEngine_Raster::drawText(const Point& pt, const String32& text, const Font& font, const Rect* clip)
{
  TemporaryGlyphSet<128> glyphSet;
  if (font.getGlyphs(text.cData(), text.getLength(), glyphSet)) return;

  _serializeGlyphSet(pt, glyphSet, clip);
}

void PainterEngine_Raster::drawText(const Rect& r, const String32& text, const Font& font, uint32_t align, const Rect* clip)
{
  TemporaryGlyphSet<128> glyphSet;
  if (font.getGlyphs(text.cData(), text.getLength(), glyphSet)) return;

  int wsize = glyphSet.getAdvance();
  int hsize = font.height();

  int x = r.getX();
  int y = r.getY();
  int w = r.getWidth();
  int h = r.getHeight();

  switch (align & TextAlignHMask)
  {
    case TextAlignLeft:
      break;
    case TextAlignRight:
      x = x + w - wsize;
      break;
    case TextAlignHCenter:
      x = x + (w - wsize) / 2;
      break;
  }

  switch (align & TextAlignVMask)
  {
    case TextAlignTop:
      break;
    case TextAlignBottom:
      y = y + h - hsize;
      break;
    case TextAlignVCenter:
      y = y + (h - hsize) / 2;
      break;
  }

  _serializeGlyphSet(Point(x, y), glyphSet, clip);
}

// ============================================================================
// [Fog::PainterEngine_Raster - Image drawing]
// ============================================================================

void PainterEngine_Raster::drawImage(const Point& p, const Image& image, const Rect* irect)
{
  int srcx = 0;
  int srcy = 0;
  int dstx = p.getX();
  int dsty = p.getY();
  int dstw;
  int dsth;

  if (irect == NULL)
  {
    dstw = image.getWidth();
    if (dstw == 0) return;
    dsth = image.getHeight();
    if (dsth == 0) return;
  }
  else
  {
    if (!irect->isValid()) return;

    srcx = irect->getX();
    if (srcx < 0) return;
    srcy = irect->getY();
    if (srcy < 0) return;

    dstw = Math::min(image.getWidth(), irect->getWidth());
    if (dstw == 0) return;
    dsth = Math::min(image.getHeight(), irect->getHeight());
    if (dsth == 0) return;
  }

  int d;

  if ((uint)(d = dstx - ctx.clipState->clipBox.getX1()) >= (uint)ctx.clipState->clipBox.getWidth())
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

  if ((uint)(d = dsty - ctx.clipState->clipBox.getY1()) >= (uint)ctx.clipState->clipBox.getHeight())
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

  if ((d = ctx.clipState->clipBox.getX2() - dstx) < dstw) dstw = d;
  if ((d = ctx.clipState->clipBox.getY2() - dsty) < dsth) dsth = d;

  Rect dst(dstx, dsty, dstw, dsth);
  Rect src(srcx, srcy, dstw, dsth);
  _serializeImage(dst, image, src);
}

// ============================================================================
// [Fog::PainterEngine_Raster - Flush]
// ============================================================================

void PainterEngine_Raster::flush()
{
  if (workerManager == NULL || workerManager->commandsPosition == 0) return;

  {
    AutoLock locked(workerManager->lock);
    if (!workerManager->isCompleted())
    {
      workerManager->wakeUpScheduled(NULL);
      workerManager->allFinishedCondition.wait();
    }
  }

  // Reset command position and local command/calculation counters
#if defined(FOG_DEBUG_RASTER_SYNCHRONIZATION)
  fog_debug("Fog::Painter::flush() - done");
#endif // FOG_DEBUG_RASTER_SYNCHRONIZATION

  workerManager->commandsPosition = 0;
  workerManager->calculationsPosition = 0;

  for (sysuint_t i = 0; i < workerManager->numWorkers; i++)
  {
    workerManager->tasks[i]->currentCommand = 0;
    workerManager->tasks[i]->currentCalculation = 0;
  }
}

void PainterEngine_Raster::flushWithQuit()
{
  FOG_ASSERT(workerManager);

  //AutoLock locked(workerManager->lock);

#if defined(FOG_DEBUG_RASTER_SYNCHRONIZATION)
  fog_debug("Fog::Painter::flushWithQuit() - quitting...");
#endif // FOG_DEBUG_RASTER_SYNCHRONIZATION

  AutoLock locked(workerManager->lock);
  workerManager->wakeUpSleeping(NULL);
}

// ============================================================================
// [Fog::PainterEngine_Raster - Properties]
// ============================================================================

err_t PainterEngine_Raster::setProperty(const String32& name, const Value& value)
{
  err_t err = Error::InvalidPropertyName;
  int p_int;

  if (name == Ascii8("multithreaded"))
  {
    if ((err = value.getInt32(&p_int)) == Error::Ok)
      if (value.isInteger()) setMultithreaded(p_int != 0);
  }

  return err;
}

Value PainterEngine_Raster::getProperty(const String32& name) const
{
  Value result;

  if (name == Ascii8("multithreaded"))
    result.setInt32(workerManager != NULL);

  return result;
}

// ============================================================================
// [Fog::PainterEngine_Raster - Multithreading - Start / Stop]
// ============================================================================

void PainterEngine_Raster::setMultithreaded(bool mt)
{
  int i;

  if ((workerManager != NULL) == mt) return;

  // If worker pool is not created we can't start multithreaded rendering.
  if (mt && !threadPool) return;

  // Start multithreading...
  if (mt)
  {
    int max = Math::min<int>(cpuInfo->numberOfProcessors, MaxWorkers);

#if defined(FOG_DEBUG_RASTER_SYNCHRONIZATION)
    fog_debug("Fog::Painter::setMultithreaded() - starting (%d threads)", max);
#endif // FOG_DEBUG_RASTER_SYNCHRONIZATION

    workerManager = new(std::nothrow) WorkerManager;
    if (workerManager == NULL) return;

    // This is for testing multithreaded rendering on single cores.
    if (max < 2)
    {
#if defined(FOG_DEBUG_RASTER_SYNCHRONIZATION)
      fog_debug("Fog::Painter::setMultithreaded() - cpu detection says 1, switching to 2");
#endif // FOG_DEBUG_RASTER_SYNCHRONIZATION
      max = 2;
    }

    for (i = 0; i < max; i++)
    {
      if ((workerManager->threads[i] = threadPool->getThread(i)) == NULL) break;
    }

    // Failed to get workers. This can happen if there are many threads that
    // uses multithreaded painter, we must destroy all resources and return.
    if (i <= 1)
    {
#if defined(FOG_DEBUG_RASTER_SYNCHRONIZATION)
      fog_debug("Fog::Painter::setMultithreaded() - failed to get %d threads from pool, releasing...", max);
#endif // FOG_DEBUG_RASTER_SYNCHRONIZATION
      if (workerManager->threads[0])
      {
        threadPool->releaseThread(workerManager->threads[0]);
        workerManager->threads[0] = NULL;
      }

      delete workerManager;
      workerManager = NULL;

      // Bailout
      return;
    }

    int count = i;

    workerManager->threadPool = threadPool;
    workerManager->numWorkers = count;

    workerManager->finishedWorkers.init(0);
    workerManager->waitingWorkers.init(0);

    workerManager->commandsPosition = 0;
    workerManager->calculationsPosition = 0;

    // Set threads affinity.
    // if (count >= (int)cpuInfo->numberOfProcessors)
    // {
    //   for (i = 0; i < count; i++) workerManager->threads[i]->setAffinity(1 << i);
    // }

    // Create worker tasks.
    for (i = 0; i < count; i++)
    {
      workerManager->tasks[i].initCustom1(&workerManager->lock);
      WorkerTask* task = workerManager->tasks[i].instancep();
      task->ctx.engine = this;
      task->ctx.id = i;
      task->ctx.offset = i;
      task->ctx.delta = count;
    }

    // Post worker tasks.
    for (i = 0; i < count; i++)
    {
      WorkerTask* task = workerManager->tasks[i].instancep();
      workerManager->threads[i]->getEventLoop()->postTask(task);
    }

#if defined(FOG_DEBUG_RASTER_SYNCHRONIZATION)
    fog_debug("Fog::Painter::setMultithreaded() - done");
#endif // FOG_DEBUG_RASTER_SYNCHRONIZATION
  }
  // Stop multithreading
  else
  {
#if defined(FOG_DEBUG_RASTER_SYNCHRONIZATION)
    fog_debug("Fog::Painter::setMultithreaded() - stopping...");
#endif // FOG_DEBUG_RASTER_SYNCHRONIZATION

    int count = workerManager->numWorkers;

    ThreadEvent releaseEvent(false, false);
    workerManager->releaseEvent = &releaseEvent;

    // Release threads.
    for (i = 0; i < count; i++)
    {
      workerManager->tasks[i]->shouldQuit.set(true);
    }

    // Flush everything and wait for completion.
    flushWithQuit();

    releaseEvent.wait();

    // Reset threads affinity.
    // if (count >= (int)cpuInfo->numberOfProcessors)
    // {
    //   for (i = 0; i < count; i++) workerManager->threads[i]->resetAffinity();
    // }

    for (i = 0; i < count; i++)
    {
      threadPool->releaseThread(workerManager->threads[i], i);
      workerManager->tasks[i].destroy();
    }

    delete workerManager;
    workerManager = NULL;

#if defined(FOG_DEBUG_RASTER_SYNCHRONIZATION)
    fog_debug("Fog::Painter::setMultithreaded() - done");
#endif // FOG_DEBUG_RASTER_SYNCHRONIZATION
  }
}

// ============================================================================
// [Fog::PainterEngine_Raster - Helpers]
// ============================================================================

void PainterEngine_Raster::_updateWorkRegion()
{
  FOG_ASSERT(ctx.clipState->refCount.get() == 1);

  ctx.clipState->workOrigin = ctx.clipState->metaOrigin + ctx.clipState->userOrigin;

  int negx = -ctx.clipState->workOrigin.getX();
  int negy = -ctx.clipState->workOrigin.getY();

  // This is maximal clip box that can be used by painter.
  ctx.clipState->clipBox.set(negx, negy, negx + _metaWidth, negy + _metaHeight);
  ctx.clipState->clipSimple = true;

  // Do region calculations only if they are really used.
  if (ctx.clipState->metaRegionUsed || ctx.clipState->userRegionUsed)
  {
    TemporaryRegion<1> boundsReg(Box(negx, negy, negx + _metaWidth, negy + _metaHeight));
    if (ctx.clipState->metaRegionUsed)
    {
      if (negx == 0 && negy == 0)
      {
        Region::intersect(ctx.clipState->workRegion, ctx.clipState->metaRegion, boundsReg);
      }
      else
      {
        TemporaryRegion<64> tmp;
        Region::translate(tmp, ctx.clipState->metaRegion, Point(negx, negy));
        Region::intersect(ctx.clipState->workRegion, tmp, boundsReg);
      }
    }
    else
    {
      ctx.clipState->workRegion.set(boundsReg);
    }

    if (ctx.clipState->userRegionUsed)
    {
      if (ctx.clipState->metaOrigin.getX() || ctx.clipState->metaOrigin.getY())
      {
        TemporaryRegion<64> tmp;
        Region::translate(tmp, ctx.clipState->userRegion, ctx.clipState->metaOrigin.negated());
        Region::intersect(ctx.clipState->workRegion, ctx.clipState->workRegion, tmp);
      }
      else
      {
        Region::intersect(ctx.clipState->workRegion, ctx.clipState->workRegion, ctx.clipState->userRegion);
      }
    }

    // Switch to box clip implementation if resulting region is simple.
    if (ctx.clipState->workRegion.getLength() == 1)
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
    (sysint_t)ctx.clipState->workOrigin.getX() * _bpp +
    (sysint_t)ctx.clipState->workOrigin.getY() * _stride;
}

void PainterEngine_Raster::_updateTransform()
{
  bool used = !ctx.capsState->transformations.isIdentity();

  ctx.capsState->transformationsUsed = used;
  ctx.capsState->transformationsApproxScale = 1.0;

  if (used)
  {
    double scalar = 1.0;
    PainterEngine_Raster::worldToScreen(&scalar);
    ctx.capsState->transformationsApproxScale = scalar;
  }
}

void PainterEngine_Raster::_setClipDefaults()
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

void PainterEngine_Raster::_setCapsDefaults()
{
  FOG_ASSERT(ctx.capsState->refCount.get() == 1);

  ctx.capsState->op = CompositeSrcOver;
  ctx.capsState->solidSource.rgba = 0xFFFFFFFF;
  ctx.capsState->solidSource.rgbp = 0xFFFFFFFF;
  ctx.capsState->patternSource.free();
  ctx.capsState->isSolidSource = true;

  ctx.capsState->lineWidth = 1.0;
  ctx.capsState->lineIsSimple = true;

  ctx.capsState->lineCap = LineCapButt;
  ctx.capsState->lineJoin = LineJoinMiter;

  ctx.capsState->dashes.free();
  ctx.capsState->dashOffset = 0.0;

  ctx.capsState->miterLimit = 4.0;

  ctx.capsState->fillMode = FillNonZero;

  ctx.capsState->transformations = Matrix();
  ctx.capsState->transformationsApproxScale = 1.0;
  ctx.capsState->transformationsUsed = false;

  ctx.rops = Raster::getRasterOps(_format, ctx.capsState->op);
  ctx.closure.closure = ctx.rops->closure;
}

Raster::PatternContext* PainterEngine_Raster::_getPatternContext()
{
  // Sanity, calling _getPatternContext() for solid source is invalid.
  FOG_ASSERT(ctx.capsState->isSolidSource == false);
  if (ctx.capsState->isSolidSource) return NULL;

  Raster::PatternContext* pctx = ctx.pctx;
  err_t err = Error::Ok;

  if (!pctx)
  {
    pctx = ctx.pctx = (Raster::PatternContext*)Memory::alloc(sizeof(Raster::PatternContext));
    if (!pctx) return NULL;
    pctx->refCount.init(1);
    pctx->initialized = false;
  }

  if (!pctx->initialized)
  {
    switch (ctx.capsState->patternSource.getType())
    {
      case Pattern::TypeTexture:
        err = Raster::functionMap->pattern.texture_init(pctx, ctx.capsState->patternSource);
        break;
      case Pattern::TypeLinearGradient:
        err = Raster::functionMap->pattern.linear_gradient_init(pctx, ctx.capsState->patternSource);
        break;
      case Pattern::TypeRadialGradient:
        err = Raster::functionMap->pattern.radial_gradient_init(pctx, ctx.capsState->patternSource);
        break;
      case Pattern::TypeConicalGradient:
        err = Raster::functionMap->pattern.conical_gradient_init(pctx, ctx.capsState->patternSource);
        break;
      default:
        FOG_ASSERT_NOT_REACHED();
    }
  }

  if (err)
  {
    fog_debug("Fog::PainterEngine_Raster::_getPatternContext() - Failed to get context (error %d)\n", err);
    return NULL;
  }

  // Be sure that pattern context is always marked as initialized.
  FOG_ASSERT(pctx->initialized);

  return pctx;
}

void PainterEngine_Raster::_resetPatternContext()
{
  if (ctx.pctx)
  {
    if (ctx.pctx->refCount.deref())
    {
      if (ctx.pctx->initialized) ctx.pctx->destroy(ctx.pctx);
      ctx.pctx->refCount.inc();
    }
    else
    {
      ctx.pctx = NULL;
    }
  }
}

bool PainterEngine_Raster::_detachClip()
{
  if (ctx.clipState->refCount.get() == 1) return true;

  ClipState* newd = new(std::nothrow) ClipState(*ctx.clipState);
  if (newd == NULL) return false;

  AtomicBase::ptr_setXchg(&ctx.clipState, newd)->deref();
  return true;
}

bool PainterEngine_Raster::_detachCaps()
{
  if (ctx.capsState->refCount.get() == 1) return true;

  CapsState* newd = new(std::nothrow) CapsState(*ctx.capsState);
  if (newd == NULL) return false;

  AtomicBase::ptr_setXchg(&ctx.capsState, newd)->deref();
  return true;
}

void PainterEngine_Raster::_deleteStates()
{
  if (states.isEmpty()) return;

  Vector<SavedState>::ConstIterator it(states);
  for (it.toStart(); it.isValid(); it.toNext())
  {
    SavedState& s = const_cast<SavedState&>(it.value());
    s.clipState->deref();
    s.capsState->deref();

    if (s.pctx && s.pctx->refCount.deref())
    {
      if (s.pctx->initialized) s.pctx->destroy(s.pctx);
      Memory::free(s.pctx);
    }
  }
  states.clear();
}

// ============================================================================
// [Fog::PainterEngine_Raster - Serializers]
// ============================================================================

static FOG_INLINE int alignToDelta(int y, int offset, int delta)
{
  FOG_ASSERT(offset >= 0);
  FOG_ASSERT(delta >= 1);
  FOG_ASSERT(offset < delta);

  int mody = y % delta;
  if (mody < 0) mody += delta;

  int newy = y - (mody) + offset;
  if (newy < y) newy += delta;

  return newy;
}

void PainterEngine_Raster::_serializeBoxes(const Box* box, sysuint_t count)
{
  // Pattern context must be always set up before _render() methods are called.
  if (!ctx.capsState->isSolidSource && !_getPatternContext()) return;

  // Singlethreaded.
  if (workerManager == NULL)
  {
    _renderBoxes(&ctx, box, count);
  }
  // Multithreaded.
  else
  {
    while (count > 0)
    {
      sysuint_t n = Math::min<sysuint_t>(count, 128);
      Command_Boxes* cmd = _createCommand<Command_Boxes>(sizeof(Command_Boxes) - sizeof(Box) + n * sizeof(Box));
      if (!cmd) return;

      cmd->count = n;
      for (sysuint_t j = 0; j < n; j++) cmd->boxes[j] = box[j];
      _postCommand(cmd);

      count -= n;
      box += n;
    }
  }
}

void PainterEngine_Raster::_serializeImage(const Rect& dst, const Image& image, const Rect& src)
{
  // Singlethreaded.
  if (workerManager == NULL)
  {
    _renderImage(&ctx, dst, image, src);
  }
  // Multithreaded.
  else
  {
    Command_Image* cmd = _createCommand<Command_Image>();
    if (!cmd) return;

    cmd->dst = dst;
    cmd->src = src;
    cmd->image.init(image);
    _postCommand(cmd);
  }
}

void PainterEngine_Raster::_serializeGlyphSet(const Point& pt, const GlyphSet& glyphSet, const Rect* clip)
{
  Box boundingBox = ctx.clipState->clipBox;

  if (clip)
  {
    Box::intersect(boundingBox, boundingBox, Box(*clip));
    if (!boundingBox.isValid()) return;
  }

  // Pattern context must be always set up before _render() methods are called.
  if (!ctx.capsState->isSolidSource && !_getPatternContext()) return;

  // Singlethreaded.
  if (workerManager == NULL)
  {
    _renderGlyphSet(&ctx, pt, glyphSet, boundingBox);
  }
  // Multithreaded.
  else
  {
    Command_GlyphSet* cmd = _createCommand<Command_GlyphSet>();
    if (!cmd) return;

    cmd->pt = pt;
    cmd->boundingBox = boundingBox;
    cmd->glyphSet.init(glyphSet);
    _postCommand(cmd);
  }
}

void PainterEngine_Raster::_serializePath(const Path& path, bool stroke)
{
  // Pattern context must be always set up before _render() methods are called.
  if (!ctx.capsState->isSolidSource && !_getPatternContext()) return;

  // Singlethreaded.
  if (workerManager == NULL)
  {
    if (_rasterizePath(&ctx, ras, path, stroke)) _renderPath(&ctx, ras);
  }
  // Multithreaded.
  else
  {
    Command_Path* cmd = _createCommand<Command_Path>();
    if (!cmd) return;

    Calculation_Path* clc = _createCalculation<Calculation_Path>();
    if (!clc) { cmd->release(); return; }

    clc->relatedTo = cmd;
    clc->path.init(path);
    clc->stroke = stroke;
    cmd->status.init(Command::Wait);
    cmd->calculation = clc;
    cmd->ras.init();
    cmd->ras.instance().gamma(ColorLut::linearLut);
    _postCommand(cmd, clc);
  }
}

template <typename T>
FOG_INLINE T* PainterEngine_Raster::_createCommand(sysuint_t size)
{
  T* command = reinterpret_cast<T*>(workerManager->allocator.alloc(size));
  if (!command) return NULL;

  new(command) T;

  command->refCount.init(workerManager->numWorkers);
  command->engine = this;
  command->clipState = ctx.clipState->ref();
  command->capsState = ctx.capsState->ref();
  command->rops = ctx.rops;
  command->pctx = NULL;
  command->status.init(Command::Ready);
  command->calculation = NULL;

  if (!ctx.capsState->isSolidSource)
  {
    FOG_ASSERT(ctx.pctx && ctx.pctx->initialized);
    ctx.pctx->refCount.inc();
    command->pctx = ctx.pctx;
  }

  return command;
}

template<typename T>
T* PainterEngine_Raster::_createCalculation(sysuint_t size)
{
  T* calculation = reinterpret_cast<T*>(workerManager->allocator.alloc(size));
  if (!calculation) return NULL;

  new(calculation) T;
  calculation->engine = this;
  return calculation;
}

void PainterEngine_Raster::_postCommand(Command* cmd, Calculation* clc)
{
#if defined(FOG_DEBUG_RASTER_COMMANDS)
  fog_debug("Fog::Painter::_postCommand() - posting...");
#endif // FOG_DEBUG_RASTER_COMMANDS

  // Flush everything if commands get maximum.
  if (workerManager->commandsPosition == MaxCommands)
  {
#if defined(FOG_DEBUG_RASTER_COMMANDS)
    fog_debug("Fog::Painter::_postCommand() - command buffer is full");
#endif // FOG_DEBUG_RASTER_COMMANDS
    flush();
  }

  if (clc)
  {
    sysuint_t pos = workerManager->calculationsPosition;
    workerManager->calculationsData[pos] = clc;
    workerManager->calculationsPosition++;
  }

  {
    sysuint_t pos = workerManager->commandsPosition;
    workerManager->commandsData[pos] = cmd;
    workerManager->commandsPosition++;

    if ((pos & 15) == 0 && workerManager->waitingWorkers.get() > 0)
    {
#if defined(FOG_DEBUG_RASTER_SYNCHRONIZATION)
      fog_debug("Fog::Painter::_postCommand() - waking up...");
#endif // FOG_DEBUG_RASTER_SYNCHRONIZATION
      AutoLock locked(workerManager->lock);
      workerManager->wakeUpScheduled(NULL);
    }
  }
}

// ============================================================================
// [Fog::PainterEngine_Raster - Renderers - AntiGrain]
// ============================================================================

static bool FOG_FASTCALL AggRasterizePath(
  PainterEngine_Raster::Context* ctx, AggRasterizer& ras,
  const Path& path, bool stroke)
{
  PainterEngine_Raster::ClipState* clipState = ctx->clipState;
  PainterEngine_Raster::CapsState* capsState = ctx->capsState;

  Path dst(path);

  if (stroke)
  {
    dst.flatten(NULL, capsState->transformationsApproxScale);

    if (capsState->dashes.getLength() > 1)
      dst.dash(capsState->dashes, capsState->dashOffset, capsState->transformationsApproxScale);

    dst.stroke(StrokeParams(capsState->lineWidth, capsState->miterLimit, capsState->lineCap, capsState->lineJoin), capsState->transformationsApproxScale);
  }
  else
  {
    dst.flatten(NULL, capsState->transformationsApproxScale);
  }

  if (capsState->transformationsUsed)
  {
    dst.applyMatrix(capsState->transformations);
  }

  ras.reset();
  ras.filling_rule(stroke ? agg::fill_even_odd : static_cast<agg::filling_rule_e>(capsState->fillMode));
  ras.clip_box(
    (double)clipState->clipBox.getX1(),
    (double)clipState->clipBox.getY1(),
    (double)clipState->clipBox.getX2(),
    (double)clipState->clipBox.getY2());

  ras.add_path(dst.cData(), dst.getLength());

  ras.sort();
  return ras.has_cells();
}

template<int BytesPerPixel, class Rasterizer, class Scanline>
static void FOG_INLINE AggRenderPath(PainterEngine_Raster::Context* ctx, const Rasterizer& ras, Scanline& sl)
{
  PainterEngine_Raster::ClipState* clipState = ctx->clipState;
  PainterEngine_Raster::CapsState* capsState = ctx->capsState;

  sl.reset(ras.min_x(), ras.max_x());

  int y = ras.min_y();
  int y_end = ras.max_y();
  int delta = ctx->delta;

  if (ctx->id != -1)
  {
    y = alignToDelta(y, ctx->offset, delta);
    if (y > y_end) return;
  }

  sysint_t stride = ctx->engine->_stride;
  uint8_t* pBase = clipState->workRaster + y * stride;
  uint8_t* pCur;

  stride *= delta;

  // Solid source type.
  if (capsState->isSolidSource)
  {
    const Raster::Solid* source = &capsState->solidSource;

    Raster::SpanSolidFn span_solid = ctx->rops->span_solid;
    Raster::SpanSolidMskFn span_solid_a8 = ctx->rops->span_solid_a8;
    Raster::SpanSolidMskConstFn span_solid_a8_const = ctx->rops->span_solid_a8_const;
    Raster::Closure* closure = &ctx->closure;

    for (; y <= y_end; y += delta, pBase += stride)
    {
      unsigned numSpans = ras.sweep_scanline(sl, y);
      if (numSpans == 0) continue;
      typename Scanline::const_iterator span = sl.begin();

      for (;;)
      {
        int x = span->x;
        int len = span->len;

        pCur = pBase + Raster::mul<int, BytesPerPixel>(x);

        if (len > 0)
        {
          span_solid_a8(pCur, source, span->covers, (unsigned)len, closure);
        }
        else
        {
          len = -len;
          FOG_ASSERT(len > 0);

          uint32_t cover = (uint32_t)*(span->covers);
          if (cover == 0xFF)
          {
            span_solid(pCur, source, len, closure);
          }
          else
          {
            span_solid_a8_const(pCur, source, cover, len, closure);
          }
        }

        if (--numSpans == 0) break;
        ++span;
      }
    }
  }
  // Pattern source type.
  else
  {
    Raster::PatternContext* pctx = ctx->pctx;
    if (!pctx) return;

    uint8_t* pbuf = ctx->getBuffer(Raster::mul4(clipState->clipBox.getWidth()));
    if (!pbuf) return;

    Raster::SpanCompositeFn span_composite = ctx->rops->span_composite[pctx->format];
    Raster::SpanCompositeMskFn span_composite_a8 = ctx->rops->span_composite_a8[pctx->format];
    Raster::SpanCompositeMskConstFn span_composite_a8_const = ctx->rops->span_composite_a8_const[pctx->format];
    Raster::Closure* closure = &ctx->closure;

    for (; y <= y_end; y += delta, pBase += stride)
    {
      unsigned numSpans = ras.sweep_scanline(sl, y);
      if (numSpans == 0) continue;
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
            span->covers, len, closure);
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
              len, closure);
          }
          else
          {
            span_composite_a8_const(pCur,
              pctx->fetch(pctx, pbuf, x, y, len),
              cover, len, closure);
          }
        }

        if (--numSpans == 0) break;
        ++span;
      }
    }
  }
}

bool PainterEngine_Raster::_rasterizePath(Context* ctx, AggRasterizer& ras, const Path& path, bool stroke)
{
  return AggRasterizePath(ctx, ras, path, stroke);
}

// ============================================================================
// [Fog::PainterEngine_Raster - Renderers]
// ============================================================================

void PainterEngine_Raster::_renderBoxes(Context* ctx, const Box* box, sysuint_t count)
{
  if (!count) return;

  uint8_t* pBuf = ctx->clipState->workRaster;
  sysint_t stride = _stride;
  sysint_t bpp = _bpp;

  Raster::Closure* closure = &ctx->closure;

  int offset = ctx->offset;
  int delta = ctx->delta;
  sysint_t strideWithDelta = stride * delta;

  // Solid source type.
  if (ctx->capsState->isSolidSource)
  {
    const Raster::Solid* source = &ctx->capsState->solidSource;
    Raster::SpanSolidFn span_solid = ctx->rops->span_solid;

    for (sysuint_t i = 0; i < count; i++)
    {
      int x1 = box[i].getX1();
      int y1 = box[i].getY1();
      int y2 = box[i].getY2();

      int w = box[i].getWidth();
      if (w <= 0) continue;

      if (delta != 1) y1 = alignToDelta(y1, offset, delta);
      if (y1 >= y2) continue;

      uint8_t* pCur = pBuf + (sysint_t)y1 * stride + (sysint_t)x1 * bpp;
      do {
        span_solid(pCur, source, (sysuint_t)w, closure);
        pCur += strideWithDelta;
        y1 += delta;
      } while (y1 < y2);
    }
  }
  // Pattern source type.
  else
  {
    Raster::PatternContext* pctx = ctx->pctx;
    if (!pctx) return;

    uint32_t op = ctx->capsState->op;
    Raster::SpanCompositeFn span_composite = ctx->rops->span_composite[pctx->format];

    // Fastpath: Do not copy pattern to extra buffer, if compositing operation
    // is copy. We need to check for pixel formats and operator.
    if (_format == pctx->format && (op == CompositeSrc || (op == CompositeSrcOver && _format == Image::FormatRGB32)))
    {
      for (sysuint_t i = 0; i < count; i++)
      {
        int x1 = box[i].getX1();
        int y1 = box[i].getY1();
        int y2 = box[i].getY2();

        int w = box[i].getWidth();
        if (w <= 0) continue;

        if (delta != 1) y1 = alignToDelta(y1, offset, delta);
        if (y1 >= y2) continue;

        uint8_t* pCur = pBuf + (sysint_t)y1 * stride + (sysint_t)x1 * bpp;
        do {
          uint8_t* f = pctx->fetch(pctx, pCur, x1, y1, w);
          if (f != pCur) span_composite(pCur, f, w, closure);

          pCur += strideWithDelta;
          y1 += delta;
        } while (y1 < y2);
      }
    }
    else
    {
      uint8_t* pbuf = ctx->getBuffer(Raster::mul4(ctx->clipState->clipBox.getWidth()));
      if (!pbuf) return;

      for (sysuint_t i = 0; i < count; i++)
      {
        int x1 = box[i].getX1();
        int y1 = box[i].getY1();
        int y2 = box[i].getY2();

        int w = box[i].getWidth();
        if (w <= 0) continue;

        if (delta != 1) y1 = alignToDelta(y1, offset, delta);
        if (y1 >= y2) continue;

        uint8_t* pCur = pBuf + (sysint_t)y1 * stride + (sysint_t)x1 * bpp;
        do {
          span_composite(pCur,
            pctx->fetch(pctx, pbuf, x1, y1, w),
            (sysuint_t)w, closure);
          pCur += strideWithDelta;
          y1 += delta;
        } while (y1 < y2);
      }
    }
  }
}

void PainterEngine_Raster::_renderImage(Context* ctx, const Rect& dst, const Image& image, const Rect& src)
{
  Image::Data* image_d = image._d;
  sysint_t dstStride = _stride;
  sysint_t srcStride = image_d->stride;

  int x = dst.getX();
  int w = dst.getWidth();

  int y1 = dst.getY1();
  int y2 = dst.getY2();

  Raster::SpanCompositeFn span_composite = ctx->rops->span_composite[image.getFormat()];
  Raster::Closure* closure = &ctx->closure;

  uint8_t* dstCur;
  const uint8_t* srcCur;

  int delta = ctx->delta;
  if (delta != 1)
  {
    int offset = ctx->offset;
    y1 = alignToDelta(y1, offset, delta);
    if (y1 >= y2) return;

    dstCur = ctx->clipState->workRaster + (sysint_t)y1 * dstStride + (sysint_t)x * _bpp;
    srcCur = image_d->first + ((sysint_t)src.getY() + y1 - dst.getY()) * srcStride + (sysint_t)src.getX() * image_d->bytesPerPixel;

    dstStride *= delta;
    srcStride *= delta;
  }
  else
  {
    dstCur = ctx->clipState->workRaster + (sysint_t)y1 * dstStride + (sysint_t)x * _bpp;
    srcCur = image_d->first + (sysint_t)src.getY() * srcStride + (sysint_t)src.getX() * image_d->bytesPerPixel;
  }

  closure->srcPalette = image._d->palette.cData();
  do {
    span_composite(dstCur, srcCur, (sysuint_t)w, closure);
    dstCur += dstStride;
    srcCur += srcStride;
    y1 += delta;
  } while (y1 < y2);
  closure->srcPalette = NULL;
}

void PainterEngine_Raster::_renderGlyphSet(Context* ctx, const Point& pt, const GlyphSet& glyphSet, const Box& boundingBox)
{
  // TODO: Hardcoded to A8 glyph format
  // TODO: Clipping

  if (!glyphSet.getLength()) return;

  int offset = ctx->offset;
  int delta = ctx->delta;

  const Glyph* glyphs = glyphSet.glyphs();
  sysuint_t count = glyphSet.getLength();

  int px = pt.getX();
  int py = pt.getY();

  uint8_t* pBuf = ctx->clipState->workRaster;
  sysint_t stride = _stride;
  sysint_t strideWithDelta = stride * delta;
  sysint_t bpp = _bpp;

  // Solid source type.
  if (ctx->capsState->isSolidSource)
  {
    Raster::SpanSolidMskFn span_solid_a8 = ctx->rops->span_solid_a8;
    Raster::Closure* closure = &ctx->closure;

    const Raster::Solid* source = &ctx->capsState->solidSource;

    for (sysuint_t i = 0; i < count; i++)
    {
      Glyph::Data* glyphd = glyphs[i]._d;
      Image::Data* bitmapd = glyphd->bitmap._d;

      int px1 = px + glyphd->bitmapX;
      int py1 = py + glyphd->bitmapY;
      int px2 = px1 + bitmapd->width;
      int py2 = py1 + bitmapd->height;

      px += glyphd->advance;

      int x1 = px1; if (x1 < boundingBox.getX1()) x1 = boundingBox.getX1();
      int y1 = py1; if (y1 < boundingBox.getY1()) y1 = boundingBox.getY1();
      int x2 = px2; if (x2 > boundingBox.getX2()) x2 = boundingBox.getX2();
      int y2 = py2; if (y2 > boundingBox.getY2()) y2 = boundingBox.getY2();

      if (delta != 1) y1 = alignToDelta(y1, offset, delta);

      int w = x2 - x1; if (w <= 0) continue;
      int h = y2 - y1; if (h <= 0) continue;

      uint8_t* pCur = pBuf;
      pCur += (sysint_t)y1 * stride;
      pCur += (sysint_t)x1 * bpp;

      sysint_t glyphStride = bitmapd->stride;
      const uint8_t* pGlyph = bitmapd->first;

      pGlyph += (sysint_t)(y1 - py1) * glyphStride;
      pGlyph += (sysint_t)(x1 - px1);

      if (delta != 1) glyphStride *= delta;

      do {
        span_solid_a8(pCur, source, pGlyph, (sysuint_t)w, closure);
        pCur += strideWithDelta;
        pGlyph += glyphStride;
        y1 += delta;
      } while (y1 < y2);
    }
  }
  // Pattern source type.
  else
  {
    Raster::PatternContext* pctx = ctx->pctx;
    if (!pctx) return;

    Raster::SpanCompositeMskFn span_composite_a8 = ctx->rops->span_composite_a8[pctx->format];
    Raster::Closure* closure = &ctx->closure;

    uint8_t* pbuf = ctx->getBuffer(Raster::mul4(ctx->clipState->clipBox.getWidth()));
    if (!pbuf) return;

    for (sysuint_t i = 0; i < count; i++)
    {
      Glyph::Data* glyphd = glyphs[i]._d;
      Image::Data* bitmapd = glyphd->bitmap._d;

      int px1 = px + glyphd->bitmapX;
      int py1 = py + glyphd->bitmapY;
      int px2 = px1 + bitmapd->width;
      int py2 = py1 + bitmapd->height;

      px += glyphd->advance;

      int x1 = px1; if (x1 < boundingBox.getX1()) x1 = boundingBox.getX1();
      int y1 = py1; if (y1 < boundingBox.getY1()) y1 = boundingBox.getY1();
      int x2 = px2; if (x2 > boundingBox.getX2()) x2 = boundingBox.getX2();
      int y2 = py2; if (y2 > boundingBox.getY2()) y2 = boundingBox.getY2();

      if (delta != 1) y1 = alignToDelta(y1, offset, delta);

      int w = x2 - x1; if (w <= 0) continue;
      int h = y2 - y1; if (h <= 0) continue;

      uint8_t* pCur = pBuf;
      pCur += (sysint_t)y1 * stride;
      pCur += (sysint_t)x1 * bpp;

      sysint_t glyphStride = bitmapd->stride;
      const uint8_t* pGlyph = bitmapd->first;

      pGlyph += (sysint_t)(y1 - py1) * glyphStride;
      pGlyph += (sysint_t)(x1 - px1);

      if (delta != 1) glyphStride *= delta;

      do {
        span_composite_a8(pCur,
          pctx->fetch(pctx, pbuf, x1, y1, w),
          pGlyph, (sysuint_t)w, closure);

        pCur += strideWithDelta;
        pGlyph += glyphStride;
        y1 += delta;
      } while (y1 < y2);
    }
  }
}

void PainterEngine_Raster::_renderPath(Context* ctx, const AggRasterizer& ras)
{
  switch (_bpp)
  {
    case 4:
      AggRenderPath<4, AggRasterizer, AggScanlineP8>(ctx, ras, ctx->slP8);
      break;
    case 3:
      AggRenderPath<3, AggRasterizer, AggScanlineP8>(ctx, ras, ctx->slP8);
      break;
    case 1:
      AggRenderPath<1, AggRasterizer, AggScanlineP8>(ctx, ras, ctx->slP8);
      break;
    default:
      FOG_ASSERT_NOT_REACHED();
  }
}

// ============================================================================
// [Public API]
// ============================================================================

PainterEngine* _getRasterPainterEngine(uint8_t* pixels, int width, int height, sysint_t stride, int format, int hints)
{
  return new(std::nothrow) PainterEngine_Raster(pixels, width, height, stride, format, hints);
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_painter_raster_init(void)
{
  using namespace Fog;

  // Setup multithreading.
  //
  // If CPU contains more cores, we can improve rendering speed by using them.
  // By default we set number of threads (workers) to count of CPU cores + 1.
  PainterEngine_Raster::threadPool = new(std::nothrow) ThreadPool();

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_painter_raster_shutdown(void)
{
  using namespace Fog;

  // Kill multithreading.
  if (PainterEngine_Raster::threadPool)
  {
    delete PainterEngine_Raster::threadPool;
    PainterEngine_Raster::threadPool = NULL;
  }
}
