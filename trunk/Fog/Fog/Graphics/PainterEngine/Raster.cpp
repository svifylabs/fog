// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Atomic.h>
#include <Fog/Core/AutoLock.h>
#include <Fog/Core/AutoUnlock.h>
#include <Fog/Core/Constants.h>
#include <Fog/Core/CpuInfo.h>
#include <Fog/Core/Math.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/OS.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/Thread.h>
#include <Fog/Core/ThreadCondition.h>
#include <Fog/Core/ThreadPool.h>
#include <Fog/Graphics/Argb.h>
#include <Fog/Graphics/ColorLut.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Font.h>
#include <Fog/Graphics/Glyph.h>
#include <Fog/Graphics/GlyphSet.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/ImageFilter.h>
#include <Fog/Graphics/Matrix.h>
#include <Fog/Graphics/Painter.h>
#include <Fog/Graphics/Rasterizer.h>
#include <Fog/Graphics/RasterUtil.h>
#include <Fog/Graphics/Scanline.h>
#include <Fog/Graphics/Stroker.h>

#include <Fog/Graphics/PainterEngine/Raster.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_C.h>

namespace Fog {

// ============================================================================
// [Configuration]
// ============================================================================

// #define FOG_DEBUG_RASTER_SYNCHRONIZATION
// #define FOG_DEBUG_RASTER_COMMANDS

// ============================================================================
// [MemoryAllocator]
// ============================================================================

struct FOG_HIDDEN MemoryAllocator
{
  // --------------------------------------------------------------------------
  // [Constants]
  // --------------------------------------------------------------------------

  enum { BLOCK_SIZE = 8000 };

  // --------------------------------------------------------------------------
  // [Block]
  // --------------------------------------------------------------------------

  struct Block
  {
    Block* next;

    sysuint_t size;
    sysuint_t pos;
    Atomic<sysuint_t> used;

    uint8_t memory[BLOCK_SIZE];
  };

  // --------------------------------------------------------------------------
  // [Header]
  // --------------------------------------------------------------------------

  struct Header
  {
    Block* block;
    sysuint_t size;
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  MemoryAllocator();
  ~MemoryAllocator();

  // --------------------------------------------------------------------------
  // [Alloc / Free]
  // --------------------------------------------------------------------------

  void* alloc(sysuint_t size);
  FOG_INLINE void free(void* ptr);

  void reset();

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  Block* blocks;

private:
  FOG_DISABLE_COPY(MemoryAllocator)
};

MemoryAllocator::MemoryAllocator() :
  blocks(NULL)
{
}

MemoryAllocator::~MemoryAllocator()
{
  reset();
}

void* MemoryAllocator::alloc(sysuint_t size)
{
  size += sizeof(Header);

  FOG_ASSERT(size <= BLOCK_SIZE);

  if (FOG_UNLIKELY(!blocks || (blocks->size - blocks->pos) < size))
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

    cur->size = BLOCK_SIZE;
    cur->pos = 0;
    cur->used.init(0);

    cur->next = blocks;
    blocks = cur;
  }

allocFromBlocks:
  Header* header = reinterpret_cast<Header*>(blocks->memory + blocks->pos);

  header->block = blocks;
  header->size = size;

  blocks->pos += size;
  blocks->used.add(size);

  return reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(header) + sizeof(Header));
}

FOG_INLINE void MemoryAllocator::free(void* ptr)
{
  Header* header = reinterpret_cast<Header*>(reinterpret_cast<uint8_t*>(ptr) - sizeof(Header));
  header->block->used.sub(header->size);
}

void MemoryAllocator::reset()
{
  Block* cur = blocks;
  blocks = NULL;

  while (cur)
  {
    // It's failure if Block::used is not zero when reseting.
    FOG_ASSERT(cur->used.get() == 0);

    Block* next = cur->next;
    Memory::free(cur);
    cur = next;
  }
}

// ============================================================================
// [Fog::RasterPainterEngine]
// ============================================================================

struct FOG_HIDDEN RasterPainterEngine : public PainterEngine
{
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  RasterPainterEngine(const ImageBuffer& buffer, int hints);
  virtual ~RasterPainterEngine();

  // --------------------------------------------------------------------------
  // [Width / Height / Format]
  // --------------------------------------------------------------------------

  virtual int getWidth() const;
  virtual int getHeight() const;
  virtual int getFormat() const;

  // --------------------------------------------------------------------------
  // [Meta]
  // --------------------------------------------------------------------------

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

  virtual void setOperator(int op);
  virtual int getOperator() const;

  // --------------------------------------------------------------------------
  // [Source]
  // --------------------------------------------------------------------------

  virtual void setSource(Argb argb);
  virtual void setSource(const Pattern& pattern);
  virtual void setSource(const ColorFilter& colorFilter);

  virtual int getSourceType() const;
  virtual Argb getSourceAsArgb() const;
  virtual Pattern getSourceAsPattern() const;

  // --------------------------------------------------------------------------
  // [Hints]
  // --------------------------------------------------------------------------

  virtual void setHint(int hint, int value);
  virtual int getHint(int hint) const;

  // --------------------------------------------------------------------------
  // [Fill Parameters]
  // --------------------------------------------------------------------------

  virtual void setFillMode(int mode);
  virtual int getFillMode() const;

  // --------------------------------------------------------------------------
  // [Stroke Parameters]
  // --------------------------------------------------------------------------

  virtual void setStrokeParams(const StrokeParams& strokeParams);
  virtual void getStrokeParams(StrokeParams& strokeParams) const;

  virtual void setLineWidth(double lineWidth);
  virtual double getLineWidth() const;

  virtual void setLineCap(int lineCap);
  virtual int getLineCap() const;

  virtual void setLineJoin(int lineJoin);
  virtual int getLineJoin() const;

  virtual void setDashes(const double* dashes, sysuint_t count);
  virtual void setDashes(const List<double>& dashes);
  virtual List<double> getDashes() const;

  virtual void setDashOffset(double offset);
  virtual double getDashOffset() const;

  virtual void setMiterLimit(double miterLimit);
  virtual double getMiterLimit() const;

  // --------------------------------------------------------------------------
  // [Transformations]
  // --------------------------------------------------------------------------

  virtual void setMatrix(const Matrix& m);
  virtual void resetMatrix();
  virtual Matrix getMatrix() const;

  virtual void rotate(double angle, int order);
  virtual void scale(double sx, double sy);
  virtual void skew(double sx, double sy, int order);
  virtual void translate(double x, double y, int order);
  virtual void transform(const Matrix& m, int order);

  virtual void worldToScreen(PointD* pt) const;
  virtual void screenToWorld(PointD* pt) const;

  virtual void alignPoint(PointD* pt) const;

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

  virtual void drawPoint(const PointD& p);
  virtual void drawLine(const PointD& start, const PointD& end);
  virtual void drawLine(const PointD* pts, sysuint_t count);
  virtual void drawPolygon(const PointD* pts, sysuint_t count);
  virtual void drawRect(const RectD& r);
  virtual void drawRects(const RectD* r, sysuint_t count);
  virtual void drawRound(const RectD& r, const PointD& radius);
  virtual void drawEllipse(const PointD& cp, const PointD& r);
  virtual void drawArc(const PointD& cp, const PointD& r, double start, double sweep);
  virtual void drawPath(const Path& path);

  virtual void fillPolygon(const PointD* pts, sysuint_t count);
  virtual void fillRect(const RectD& r);
  virtual void fillRects(const RectD* r, sysuint_t count);
  virtual void fillRound(const RectD& r, const PointD& radius);
  virtual void fillEllipse(const PointD& cp, const PointD& r);
  virtual void fillArc(const PointD& cp, const PointD& r, double start, double sweep);
  virtual void fillPath(const Path& path);

  // --------------------------------------------------------------------------
  // [Glyph / Text Drawing]
  // --------------------------------------------------------------------------

  virtual void drawGlyph(const Point& pt, const Glyph& glyph, const Rect* clip);
  virtual void drawGlyphSet(const Point& pt, const GlyphSet& glyphSet, const Rect* clip);

  virtual void drawText(const Point& p, const String& text, const Font& font, const Rect* clip);
  virtual void drawText(const Rect& r, const String& text, const Font& font, uint32_t align, const Rect* clip);

  // --------------------------------------------------------------------------
  // [Image drawing]
  // --------------------------------------------------------------------------

  virtual void blitImage(const Point& p, const Image& image, const Rect* irect);
  virtual void blitImage(const PointD& p, const Image& image, const Rect* irect);

  // --------------------------------------------------------------------------
  // [Multithreading]
  // --------------------------------------------------------------------------

  virtual void setEngine(int engine, int cores = 0);
  virtual int getEngine() const;

  virtual void flush();
  void flushWithQuit();

  // --------------------------------------------------------------------------
  // [Configuration]
  // --------------------------------------------------------------------------

  // Minimum size to set multithreading on.
  enum { MIN_SIZE_THRESHOLD = 256*256 };

  // Maximum number of threads to use for rendering.
  enum { MAX_WORKERS = 16 };
  // Maximum commands and calculations to accumulate in buffer.
#if defined(FOG_DEBUG_RASTER_SYNCHRONIZATION)
  enum { MAX_COMMANDS = 32 };     // For debugging it's better to have only minimal
  enum { MAX_CALCULATIONS = 32 }; // buffer size for the commands and calculations.
#else
  enum { MAX_COMMANDS = 1024 };
  enum { MAX_CALCULATIONS = 1024 };
#endif // FOG_DEBUG_RASTER_SYNCHRONIZATION

  // --------------------------------------------------------------------------
  // [Constants]
  // --------------------------------------------------------------------------

  enum DIRTY_CAPS
  {
    DIRTY_CAPS_TRANSFORM = 0x00000001,
    DIRTY_CAPS_PATTERN   = 0x00000002
  };

  enum TRANSFORM_TYPE
  {
    TRANSFORM_TRANSLATE_EXACT = 0,
    TRANSFORM_TRANSLATE_SUBPX = 1,
    TRANSFORM_AFFINE = 2
  };

  // --------------------------------------------------------------------------
  // [Forward Declarations]
  // --------------------------------------------------------------------------

  struct Action;
  struct Calculation;
  struct CapsState;
  struct ClipState;
  struct Command;
  struct Context;
  struct Layer;
  struct SavedState;
  struct WorkerManager;
  struct WorkerTask;

  // --------------------------------------------------------------------------
  // [Layer]
  // --------------------------------------------------------------------------

  struct FOG_HIDDEN Layer
  {
    uint8_t* pixels;

    int width;
    int height;
    int format;

    sysint_t stride;
    sysint_t bpp;
  };

  // --------------------------------------------------------------------------
  // [ClipState]
  // --------------------------------------------------------------------------

  struct FOG_HIDDEN ClipState
  {
    // ------------------------------------------------------------------------
    // [Construction / Destruction]
    // ------------------------------------------------------------------------

    ClipState();
    ClipState(const ClipState& other);
    ~ClipState();

    // ------------------------------------------------------------------------
    // [Ref]
    // ------------------------------------------------------------------------

    FOG_INLINE ClipState* ref() const;

    // ------------------------------------------------------------------------
    // [Operator Overload]
    // ------------------------------------------------------------------------

    ClipState& operator=(const ClipState& other);

    // ------------------------------------------------------------------------
    // [Members]
    // ------------------------------------------------------------------------

    mutable Atomic<sysuint_t> refCount;

    Point metaOrigin;
    Point userOrigin;
    Point workOrigin;

    Region metaRegion;
    Region userRegion;
    Region workRegion;

    Box clipBox;

    bool metaRegionUsed;
    bool userRegionUsed;
    bool workRegionUsed;
    bool clipSimple;
  };

  // --------------------------------------------------------------------------
  // [CapsState]
  // --------------------------------------------------------------------------

  struct FOG_HIDDEN CapsState
  {
    // ------------------------------------------------------------------------
    // [Construction / Destruction]
    // ------------------------------------------------------------------------

    CapsState();
    CapsState(const CapsState& other);
    ~CapsState();

    // ------------------------------------------------------------------------
    // [Ref]
    // ------------------------------------------------------------------------

    FOG_INLINE CapsState* ref() const;

    // ------------------------------------------------------------------------
    // [Operator Overload]
    // ------------------------------------------------------------------------

    CapsState& operator=(const CapsState& other);

    // ------------------------------------------------------------------------
    // [Members]
    // ------------------------------------------------------------------------

    //! @brief Reference count.
    mutable Atomic<sysuint_t> refCount;

    // //! @brief Dirty flags, see @c RasterPainterEngine::DIRTY_CAPS.
    // int dirty;

    union {
      struct {
        //! @brief Compositing operator, see @c COMPOSITE_OP.
        uint8_t op;

        //! @brief Type of source, see @c PAINTER_SOURCE_TYPE.
        uint8_t sourceType;

        //! @brief Fill mode, see @c FILL_MODE.
        uint8_t fillMode;

        //! @brief Anti-aliasing type/quality, see @c ANTI_ALIASING_TYPE.
        uint8_t aaQuality;
        //! @brief Image interpolation type/quality, see @c INTERPOLATION_TYPE.
        uint8_t imageInterpolation;
        //! @brief Gradient interpolation type/quality, see @c INTERPOLATION_TYPE.
        uint8_t gradientInterpolation;

        //! @brief Whether line is simple (one pixel width and default caps).
        uint8_t lineIsSimple;
        //! @brief Whether complex transformation is used (complex transform means
        //! that 2x2 matrix is not identity, translation is not considered).
        uint8_t transformType;
      };

      //! @brief All packed data in one array (for fast copy).
      uint8_t data[8];
    };

    union {
      //! @brief Solid source data (if @c sourceType is @c PAINTER_SOURCE_SOLID).
      RasterUtil::Solid solid;
      //! @brief Pattern source data (if @c sourceType is @c PAINTER_SOURCE_PATTERN).
      Static<Pattern> pattern;
      //! @brief Color filter data (if @c sourceType is @c PAINTER_SOURCE_COLOR_FILTER).
      Static<ColorFilter> colorFilter;
    };

    //! @brief Pointer to compositing functions, see @c op.
    RasterUtil::FunctionMap::CompositeFuncs* rops;

    //! @brief Stroke parameters.
    StrokeParams strokeParams;

    //! @brief Transformation matrix.
    Matrix transform;
    //! @brief Transformation approximation scale used by path flattening.
    double approximationScale;

    //! @brief Saved transform matrix translation (tx and ty values).
    PointD transformTranslateSaved;

    //! @brief Transformation translate point in pixels (can be used if 
    //! transform type is @c TRANSFORM_TRANSLATE_EXACT).
    Point transformTranslateInt;
  };

  // --------------------------------------------------------------------------
  // [SavedState]
  // --------------------------------------------------------------------------

  // State structure used by save() and restore() methods.
  struct FOG_HIDDEN SavedState
  {
    ClipState* clipState;
    CapsState* capsState;
    RasterUtil::PatternContext* pctx;
  };

  // --------------------------------------------------------------------------
  // [Context]
  // --------------------------------------------------------------------------

  // Context is accessed always from only one thread.
  struct FOG_HIDDEN Context
  {
    // ------------------------------------------------------------------------
    // [Construction / Destruction]
    // ------------------------------------------------------------------------

    Context();
    ~Context();

    // ------------------------------------------------------------------------
    // [Buffer Manager]
    // ------------------------------------------------------------------------

    uint8_t* getBuffer(sysint_t size);

    // ------------------------------------------------------------------------
    // [Members]
    // ------------------------------------------------------------------------

    // Owner of this context.
    RasterPainterEngine* engine;

    // Layer.
    Layer* layer;
    // Clip state.
    ClipState* clipState;
    // Capabilities state.
    CapsState* capsState;

    // Raster closure.
    RasterUtil::Closure closure;

    // Pattern context.
    RasterUtil::PatternContext* pctx;

    // Antigrain scanline rasterizer containers.
    Scanline32 scanline;

    // Multithreading id, offset and delta.
    int id;     // If multithreading is disabled, id is -1.
    int offset; // If multithreading is disabled, offset is 0.
    int delta;  // If multithreading is disabled, delta is 1.

    // Static embedded buffer for fast alloc/free, see getBuffer().
    uint8_t* buffer;
    sysint_t bufferSize;
    uint8_t bufferStatic[1024*8];

  private:
    FOG_DISABLE_COPY(Context)
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

    FOG_INLINE void _free();

    // [Members]

    RasterPainterEngine* engine;

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

    Layer* layer;
    ClipState* clipState;
    CapsState* capsState;

    RasterUtil::PatternContext* pctx;

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

    Rasterizer* ras;
    //! @brief Whether it's texture blit command instead of draw/fill path 
    //! command.
    //!
    //! Texture blit commands are serialized through @c _serializeImageAffine(),
    //! difference is that it's always used PAINTER_SOURCE_PATTERN type to do
    //! pattern based blit.
    int textureBlit;
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

    RasterPainterEngine* engine;

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

    Thread* threads[MAX_WORKERS];
    Static<WorkerTask> tasks[MAX_WORKERS];

    // Commands and calculations allocator.
    MemoryAllocator allocator;

    // Commands manager.
    volatile sysint_t commandsPosition;
    Command* volatile commandsData[MAX_COMMANDS];

    // Calculations manager.
    volatile sysint_t calculationsPosition;
    Calculation* volatile calculationsData[MAX_CALCULATIONS];
  };

  // --------------------------------------------------------------------------
  // [Helpers]
  // --------------------------------------------------------------------------

  void _updateWorkRegion();
  void _updateTransform(bool translationOnly);

  void _setClipDefaults();
  void _setCapsDefaults();

  RasterUtil::PatternContext* _getPatternContext();
  void _resetPatternContext();

  FOG_INLINE void _updateLineWidth()
  {
    ctx.capsState->lineIsSimple = (ctx.capsState->strokeParams.getLineWidth() == 1.0 &&
                                   ctx.capsState->strokeParams.getDashes().getLength() == 0);
  }

  bool _detachCaps();
  bool _detachClip();

  FOG_INLINE void _derefClipState(ClipState* clipState);
  FOG_INLINE void _derefCapsState(CapsState* capsState);

  void _deleteStates();

  // --------------------------------------------------------------------------
  // [Serializers]
  // --------------------------------------------------------------------------

  // Serializers are always called from painter thread.
  void _serializeBoxes(const Box* box, sysuint_t count);
  void _serializeImage(const Rect& dst, const Image& image, const Rect& src);
  void _serializeImageAffine(const PointD& pt, const Image& image, const Rect* irect);
  void _serializeGlyphSet(const Point& pt, const GlyphSet& glyphSet, const Rect* clip);
  void _serializePath(const Path& path, bool stroke);

  template<typename T> FOG_INLINE T* _createCommand(sysuint_t size = sizeof(T));
  template<typename T> FOG_INLINE T* _createCommand(sysuint_t size, RasterUtil::PatternContext* pctx);
  template<typename T> FOG_INLINE T* _createCalculation(sysuint_t size = sizeof(T));

  void _postCommand(Command* cmd, Calculation* clc = NULL);

  // --------------------------------------------------------------------------
  // [Rasterization]
  // --------------------------------------------------------------------------

  static bool _rasterizePath(Context* ctx, Rasterizer* ras, const Path& path, bool stroke);

  // --------------------------------------------------------------------------
  // [Renderering]
  // --------------------------------------------------------------------------

  void _renderBoxes(Context* ctx, const Box* box, sysuint_t count);
  void _renderImage(Context* ctx, const Rect& dst, const Image& image, const Rect& src);
  void _renderGlyphSet(Context* ctx, const Point& pt, const GlyphSet& glyphSet, const Box& boundingBox);
  void _renderPath(Context* ctx, Rasterizer* ras, bool textureBlit);

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  Layer main;

  MemoryAllocator allocator;

  // Temporary path.
  Path tmpPath;

  // Temporary glyph set.
  GlyphSet tmpGlyphSet;

  // Context that is used by single-threaded painter.
  Context ctx;

  // Context states LIFO buffer (for save() and restore() methods)
  List<SavedState> states;

  // If we are running in single-core environment it's better to use one
  // rasterizer for everythging.
  Rasterizer* ras;

  // Multithreading
  WorkerManager* workerManager;

  // --------------------------------------------------------------------------
  // [Statics]
  // --------------------------------------------------------------------------

  static ThreadPool* threadPool;
};

// ============================================================================
// [Fog::RasterPainterEngine::ClipState]
// ============================================================================

RasterPainterEngine::ClipState::ClipState()
{
  refCount.init(1);
}

RasterPainterEngine::ClipState::ClipState(const ClipState& other) :
  metaOrigin(other.metaOrigin),
  userOrigin(other.userOrigin),
  workOrigin(other.workOrigin),

  metaRegion(other.metaRegion),
  userRegion(other.userRegion),
  workRegion(other.workRegion),

  clipBox(other.clipBox),

  metaRegionUsed(other.metaRegionUsed),
  userRegionUsed(other.userRegionUsed),
  workRegionUsed(other.workRegionUsed),
  clipSimple(other.clipSimple)
{
  refCount.init(1);
}

RasterPainterEngine::ClipState::~ClipState()
{
}

FOG_INLINE RasterPainterEngine::ClipState* RasterPainterEngine::ClipState::ref() const
{
  refCount.inc();
  return const_cast<ClipState*>(this);
}

RasterPainterEngine::ClipState& RasterPainterEngine::ClipState::operator=(const ClipState& other)
{
  metaOrigin = other.metaOrigin;
  userOrigin = other.userOrigin;
  workOrigin = other.workOrigin;

  metaRegion = other.metaRegion;
  userRegion = other.userRegion;
  workRegion = other.workRegion;

  clipBox = other.clipBox;

  metaRegionUsed = other.metaRegionUsed;
  userRegionUsed = other.userRegionUsed;
  workRegionUsed = other.workRegionUsed;
  clipSimple = other.clipSimple;

  return *this;
}

// ============================================================================
// [Fog::RasterPainterEngine::CapsState]
// ============================================================================

RasterPainterEngine::CapsState::CapsState()
{
  refCount.init(1);
  sourceType = PAINTER_SOURCE_ARGB;
}

RasterPainterEngine::CapsState::CapsState(const CapsState& other) :
  rops(other.rops),
  strokeParams(other.strokeParams),
  transform(other.transform),
  approximationScale(other.approximationScale),
  transformTranslateSaved(other.transformTranslateSaved),
  transformTranslateInt(other.transformTranslateInt)
{
  refCount.init(1);
  Memory::copy8B(data, other.data);

  switch (sourceType)
  {
    case PAINTER_SOURCE_ARGB:
      solid.argb = other.solid.argb;
      solid.prgb = other.solid.prgb;
      break;
    case PAINTER_SOURCE_PATTERN:
      pattern.initCustom1(other.pattern.instance());
      break;
    case PAINTER_SOURCE_COLOR_FILTER:
      colorFilter.initCustom1(other.colorFilter.instance());
      break;
    default:
      FOG_ASSERT_NOT_REACHED();
  }
}

RasterPainterEngine::CapsState::~CapsState()
{
  switch (sourceType)
  {
    case PAINTER_SOURCE_ARGB:
      break;
    case PAINTER_SOURCE_PATTERN:
      pattern.destroy();
      break;
    case PAINTER_SOURCE_COLOR_FILTER:
      colorFilter.destroy();
      break;
    default:
      FOG_ASSERT_NOT_REACHED();
  }
}

FOG_INLINE RasterPainterEngine::CapsState* RasterPainterEngine::CapsState::ref() const
{
  refCount.inc();
  return const_cast<CapsState*>(this);
}

RasterPainterEngine::CapsState& RasterPainterEngine::CapsState::operator=(const CapsState& other)
{
  switch (sourceType)
  {
    case PAINTER_SOURCE_ARGB:
      break;
    case PAINTER_SOURCE_PATTERN:
      pattern.destroy();
      break;
    case PAINTER_SOURCE_COLOR_FILTER:
      colorFilter.destroy();
      break;
    default:
      FOG_ASSERT_NOT_REACHED();
  }

  Memory::copy8B(data, other.data);

  rops = other.rops;
  strokeParams = other.strokeParams;
  transform = other.transform;
  approximationScale = other.approximationScale;
  transformTranslateSaved = other.transformTranslateSaved;
  transformTranslateInt = other.transformTranslateInt;

  switch (other.sourceType)
  {
    case PAINTER_SOURCE_ARGB:
      solid.argb = other.solid.argb;
      solid.prgb = other.solid.prgb;
      break;
    case PAINTER_SOURCE_PATTERN:
      pattern.initCustom1(other.pattern.instance());
      break;
    case PAINTER_SOURCE_COLOR_FILTER:
      colorFilter.initCustom1(other.colorFilter.instance());
      break;
    default:
      FOG_ASSERT_NOT_REACHED();
  }

  return *this;
}

// ============================================================================
// [Fog::RasterPainterEngine::Context]
// ============================================================================

RasterPainterEngine::Context::Context()
{
  // Scanline must be reset before using it.
  scanline.reset();

  engine = NULL;

  clipState = NULL;
  capsState = NULL;

  pctx = NULL;

  closure.dstPalette = NULL;
  closure.srcPalette = NULL;

  id = -1;
  offset = 0;
  delta = 1;

  buffer = bufferStatic;
  bufferSize = FOG_ARRAY_SIZE(bufferStatic);
}

RasterPainterEngine::Context::~Context()
{
  if (buffer != bufferStatic) Memory::free(buffer);
}

uint8_t* RasterPainterEngine::Context::getBuffer(sysint_t size)
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
// [Fog::RasterPainterEngine::Action]
// ============================================================================

FOG_INLINE void RasterPainterEngine::Action::_free()
{
  engine->allocator.free(this);
}

// ============================================================================
// [Fog::RasterPainterEngine::Command]
// ============================================================================

FOG_INLINE void RasterPainterEngine::Command::_releaseObjects()
{
  engine->_derefClipState(clipState);
  engine->_derefCapsState(capsState);

  if (pctx && pctx->refCount.deref())
  {
    pctx->destroy(pctx);
    engine->allocator.free(pctx);
  }
}

// ============================================================================
// [Fog::RasterPainterEngine::Command_Boxes]
// ============================================================================

void RasterPainterEngine::Command_Boxes::run(Context* ctx)
{
  ctx->engine->_renderBoxes(ctx, boxes, count);
}

void RasterPainterEngine::Command_Boxes::release()
{
  _releaseObjects();
  _free();
}

// ============================================================================
// [Fog::RasterPainterEngine::Command_Image]
// ============================================================================

void RasterPainterEngine::Command_Image::run(Context* ctx)
{
  ctx->engine->_renderImage(ctx, dst, image.instance(), src);
}

void RasterPainterEngine::Command_Image::release()
{
  image.destroy();

  _releaseObjects();
  _free();
}

// ============================================================================
// [Fog::RasterPainterEngine::Command_GlyphSet]
// ============================================================================

void RasterPainterEngine::Command_GlyphSet::run(Context* ctx)
{
  ctx->engine->_renderGlyphSet(ctx, pt, glyphSet.instance(), boundingBox);
}

void RasterPainterEngine::Command_GlyphSet::release()
{
  glyphSet.destroy();

  _releaseObjects();
  _free();
}

// ============================================================================
// [Fog::RasterPainterEngine::Command_Path]
// ============================================================================

void RasterPainterEngine::Command_Path::run(Context* ctx)
{
  ctx->engine->_renderPath(ctx, ras, textureBlit);
}

void RasterPainterEngine::Command_Path::release()
{
  Rasterizer::releaseRasterizer(ras);

  _releaseObjects();
  _free();
}

// ============================================================================
// [Fog::RasterPainterEngine::Calculation_Path]
// ============================================================================

void RasterPainterEngine::Calculation_Path::run(Context* ctx)
{
  Command_Path* cmd = reinterpret_cast<Command_Path*>(relatedTo);
  cmd->ras = Rasterizer::getRasterizer();

  bool ok = _rasterizePath(ctx, cmd->ras, path.instance(), stroke);
  cmd->status.set(ok ? Command::Ready : Command::Skip);
}

void RasterPainterEngine::Calculation_Path::release()
{
  path.destroy();
  _free();
}

// ============================================================================
// [Fog::RasterPainterEngine::WorkerTask]
// ============================================================================

RasterPainterEngine::WorkerTask::WorkerTask(Lock* lock) :
  cond(lock)
  //event(false, false)
{
  state.init(Running);
  shouldQuit.init(false);

  currentCalculation = 0;
  currentCommand = 0;
}

RasterPainterEngine::WorkerTask::~WorkerTask()
{
}

void RasterPainterEngine::WorkerTask::run()
{
  RasterPainterEngine* engine = ctx.engine;
  WorkerManager* mgr = engine->workerManager;

  int id = ctx.id;
  int delta = ctx.delta;

#if defined(FOG_DEBUG_RASTER_SYNCHRONIZATION)
  fog_debug("Fog::Painter[Worker #%d]::run() - ThreadID=%d", ctx.id, Thread::getCurrent()->getId());
#endif // FOG_DEBUG_RASTER_SYNCHRONIZATION

  for (;;)
  {
    // Do calculations and commands.
    int cont = 0;
    bool wait = false;

    do {
      wait = false;

      // Do calculations (highest priority than commands).
      if (currentCalculation < mgr->calculationsPosition)
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
          ctx.layer = cmd->layer;
          ctx.clipState = cmd->clipState;
          ctx.capsState = cmd->capsState;
          ctx.pctx = cmd->pctx;

          clc->run(&ctx);
          clc->release();

          AutoLock locked(mgr->lock);
          mgr->wakeUpScheduled(this);
        }

        currentCalculation++;
        cont = 0;
      }

      // Do command.
      if (currentCommand < mgr->commandsPosition)
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
            int workerOffset = (id - cmd->clipState->workOrigin.y) % delta;
            if (workerOffset < 0) workerOffset += delta;

            ctx.offset = workerOffset;

            ctx.layer = cmd->layer;
            ctx.clipState = cmd->clipState;
            ctx.capsState = cmd->capsState;
            ctx.pctx = cmd->pctx;

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
    } while (cont <= 2);

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

      if (shouldQuit.get() == (int)true && currentCommand == mgr->commandsPosition)
      {
#if defined(FOG_DEBUG_RASTER_SYNCHRONIZATION)
        fog_debug("Fog::Painter[Worker #%d]::run() - quitting...", ctx.id);
#endif // FOG_DEBUG_RASTER_SYNCHRONIZATION

        // Set Running state so wakeUpSleeping() can't return us.
        state.set(Running);
        mgr->wakeUpSleeping(this);
        state.set(Done);
        return;
      }

      cond.wait();
      state.set(Running);
      mgr->waitingWorkers.dec();

      if (shouldQuit.get() == (int)true)
        mgr->wakeUpSleeping(this);
      else
        mgr->wakeUpScheduled(this);
    }
  }
}

void RasterPainterEngine::WorkerTask::destroy()
{
  RasterPainterEngine* engine = ctx.engine;
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
// [Fog::RasterPainterEngine::WorkerManager]
// ============================================================================

RasterPainterEngine::WorkerManager::WorkerManager() :
  allFinishedCondition(&lock),
  releaseEvent(NULL)
{
}

RasterPainterEngine::WorkerManager::~WorkerManager()
{
}

RasterPainterEngine::WorkerTask* RasterPainterEngine::WorkerManager::wakeUpScheduled(WorkerTask* calledFrom)
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

RasterPainterEngine::WorkerTask* RasterPainterEngine::WorkerManager::wakeUpSleeping(WorkerTask* calledFrom)
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

bool RasterPainterEngine::WorkerManager::isCompleted()
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
// [Fog::RasterPainterEngine - Construction / Destruction]
// ============================================================================

ThreadPool* RasterPainterEngine::threadPool;

RasterPainterEngine::RasterPainterEngine(const ImageBuffer& buffer, int hints) :
  workerManager(NULL)
{
  main.pixels = buffer.data;
  main.width = buffer.width;
  main.height = buffer.height;
  main.format = buffer.format;
  main.stride = buffer.stride;
  main.bpp = Image::formatToBytesPerPixel(buffer.format);

  ctx.engine = this;
  ctx.layer = &main;
  ctx.clipState = new( allocator.alloc(sizeof(ClipState)) ) ClipState();
  ctx.capsState = new( allocator.alloc(sizeof(CapsState)) ) CapsState();

  ras = Rasterizer::getRasterizer();
  // ras.gamma(ColorLut::linearLut);

  // Setup clip state.
  _setClipDefaults();

  // Setup caps state.
  _setCapsDefaults();

  // Setup multithreading if possible. If the painting buffer if too small,
  // we will not use multithreading, because it has no sense.
  if (cpuInfo->numberOfProcessors > 1 && (hints & PAINTER_HINT_NO_MT) == 0)
  {
    sysuint_t total = (sysuint_t)buffer.width * (sysuint_t)buffer.height;

    if (total >= MIN_SIZE_THRESHOLD)
    {
#if defined(FOG_DEBUG_RASTER_SYNCHRONIZATION)
      fog_debug("Fog::Painter::new() - Image %dx%d (total %d), selected for multithreading", width, height, width * height);
#endif // FOG_DEBUG_RASTER_SYNCHRONIZATION
      setEngine(PAINTER_ENGINE_RASTER_MT);
    }
  }
}

RasterPainterEngine::~RasterPainterEngine()
{
  // First set engine to singlethreaded (this means flush and releasing all
  // threads), then we can destroy engine.
  if (workerManager) setEngine(PAINTER_ENGINE_RASTER_ST);

  _deleteStates();

  // This is our context, it's imposible that other thread is using it!
  if (ctx.pctx)
  {
    if (ctx.pctx->initialized) ctx.pctx->destroy(ctx.pctx);
    allocator.free(ctx.pctx);
  }

  _derefClipState(ctx.clipState);
  _derefCapsState(ctx.capsState);

  Rasterizer::releaseRasterizer(ras);
}

// ============================================================================
// [Fog::RasterPainterEngine - Meta]
// ============================================================================

int RasterPainterEngine::getWidth() const
{
  return main.width;
}

int RasterPainterEngine::getHeight() const
{
  return main.height;
}

int RasterPainterEngine::getFormat() const
{
  return main.format;
}

void RasterPainterEngine::setMetaVariables(
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

    // We must set workOrigin here, because _setCapsDefaults() will set default
    // transformation matrix to identity + workOrigin.
    ctx.clipState->workOrigin.set(metaOrigin);
    if (_detachCaps()) _setCapsDefaults();
  }

  _updateWorkRegion();
}

void RasterPainterEngine::setMetaOrigin(const Point& pt)
{
  if (ctx.clipState->metaOrigin == pt) return;
  if (!_detachClip()) return;

  ctx.clipState->metaOrigin = pt;
  _updateWorkRegion();
}

void RasterPainterEngine::setUserOrigin(const Point& pt)
{
  if (ctx.clipState->userOrigin == pt) return;
  if (!_detachClip()) return;

  ctx.clipState->userOrigin = pt;
  _updateWorkRegion();
}

void RasterPainterEngine::translateMetaOrigin(const Point& pt)
{
  if (pt.x == 0 && pt.y == 0) return;
  if (!_detachClip()) return;

  ctx.clipState->metaOrigin += pt;
  _updateWorkRegion();
}

void RasterPainterEngine::translateUserOrigin(const Point& pt)
{
  if (pt.x == 0 && pt.y == 0) return;
  if (!_detachClip()) return;

  ctx.clipState->userOrigin += pt;
  _updateWorkRegion();
}

void RasterPainterEngine::setUserRegion(const Rect& r)
{
  if (!_detachClip()) return;

  ctx.clipState->userRegion = r;
  ctx.clipState->userRegionUsed = true;
  _updateWorkRegion();
}

void RasterPainterEngine::setUserRegion(const Region& r)
{
  if (!_detachClip()) return;

  ctx.clipState->userRegion = r;
  ctx.clipState->userRegionUsed = true;
  _updateWorkRegion();
}

void RasterPainterEngine::resetMetaVars()
{
  if (!_detachClip()) return;

  ctx.clipState->metaOrigin.set(0, 0);
  ctx.clipState->metaRegion.clear();
  ctx.clipState->metaRegionUsed = false;
  _updateWorkRegion();
}

void RasterPainterEngine::resetUserVars()
{
  if (!_detachClip()) return;

  ctx.clipState->userOrigin.set(0, 0);
  ctx.clipState->userRegion.clear();
  ctx.clipState->userRegionUsed = false;
  _updateWorkRegion();
}

Point RasterPainterEngine::getMetaOrigin() const
{
  return ctx.clipState->metaOrigin;
}

Point RasterPainterEngine::getUserOrigin() const
{
  return ctx.clipState->userOrigin;
}

Region RasterPainterEngine::getMetaRegion() const
{
  return ctx.clipState->metaRegion;
}

Region RasterPainterEngine::getUserRegion() const
{
  return ctx.clipState->userRegion;
}

bool RasterPainterEngine::isMetaRegionUsed() const
{
  return ctx.clipState->metaRegionUsed;
}

bool RasterPainterEngine::isUserRegionUsed() const
{
  return ctx.clipState->userRegionUsed;
}

// ============================================================================
// [Fog::RasterPainterEngine - Operator]
// ============================================================================

void RasterPainterEngine::setOperator(int op)
{
  if ((ctx.capsState->op == op) | ((uint)op >= COMPOSITE_COUNT)) return;
  if (!_detachCaps()) return;

  ctx.capsState->op = op;
  ctx.capsState->rops = RasterUtil::getRasterOps(ctx.layer->format, op);
}

int RasterPainterEngine::getOperator() const
{
  return ctx.capsState->op;
}

// ============================================================================
// [Fog::RasterPainterEngine - Source]
// ============================================================================

void RasterPainterEngine::setSource(Argb argb)
{
  if (!_detachCaps()) return;

  // Destroy old source object if needed.
  switch (ctx.capsState->sourceType)
  {
    case PAINTER_SOURCE_ARGB:
      break;
    case PAINTER_SOURCE_PATTERN:
      ctx.capsState->pattern.destroy();
      _resetPatternContext();
      break;
    case PAINTER_SOURCE_COLOR_FILTER:
      ctx.capsState->colorFilter.destroy();
      break;
    default:
      FOG_ASSERT_NOT_REACHED();
  }

  ctx.capsState->sourceType = PAINTER_SOURCE_ARGB;
  ctx.capsState->solid.argb = argb.value;
  ctx.capsState->solid.prgb = ArgbUtil::premultiply(argb.value);
}

void RasterPainterEngine::setSource(const Pattern& pattern)
{
  if (pattern.isSolid())
  {
    setSource(pattern.getColor());
    return;
  }

  if (!_detachCaps()) return;

  // Destroy old source object if needed.
  switch (ctx.capsState->sourceType)
  {
    case PAINTER_SOURCE_ARGB:
      break;
    case PAINTER_SOURCE_PATTERN:
      ctx.capsState->pattern.instance() = pattern;
      goto done;
    case PAINTER_SOURCE_COLOR_FILTER:
      ctx.capsState->colorFilter.destroy();
      break;
    default:
      FOG_ASSERT_NOT_REACHED();
  }

  ctx.capsState->sourceType = PAINTER_SOURCE_PATTERN;
  ctx.capsState->pattern.initCustom1(pattern);

done:
  _resetPatternContext();
}

void RasterPainterEngine::setSource(const ColorFilter& colorFilter)
{
  if (!_detachCaps()) return;

  // Destroy old source object if needed.
  switch (ctx.capsState->sourceType)
  {
    case PAINTER_SOURCE_ARGB:
      break;
    case PAINTER_SOURCE_PATTERN:
      ctx.capsState->pattern.destroy();
      _resetPatternContext();
      break;
    case PAINTER_SOURCE_COLOR_FILTER:
      ctx.capsState->colorFilter.instance() = colorFilter;
      goto done;
    default:
      FOG_ASSERT_NOT_REACHED();
  }

  ctx.capsState->sourceType = PAINTER_SOURCE_COLOR_FILTER;
  ctx.capsState->colorFilter.initCustom1(colorFilter);
done:
  ;
}

int RasterPainterEngine::getSourceType() const
{
  return ctx.capsState->sourceType;
}

Argb RasterPainterEngine::getSourceAsArgb() const
{
  switch (ctx.capsState->sourceType)
  {
    case PAINTER_SOURCE_ARGB:
      return Argb(ctx.capsState->solid.argb);
    default:
      return Argb(0x00000000);
  }
}

Pattern RasterPainterEngine::getSourceAsPattern() const
{
  Pattern pattern;

  switch (ctx.capsState->sourceType)
  {
    case PAINTER_SOURCE_ARGB:
      pattern.setColor(ctx.capsState->solid.argb);
      break;
    case PAINTER_SOURCE_PATTERN:
      pattern = ctx.capsState->pattern.instance();
      break;
  }

  return pattern;
}

// ============================================================================
// [Fog::RasterPainterEngine - Hints]
// ============================================================================

void RasterPainterEngine::setHint(int hint, int value)
{
  switch (hint)
  {
    case PAINTER_HINT_ANTIALIASING_QUALITY:
      if ((ctx.capsState->aaQuality == value) | ((uint)value >= ANTI_ALIASING_INVALID)) return;
      if (!_detachCaps()) return;

      ctx.capsState->aaQuality = value;
      break;

    case PAINTER_HINT_IMAGE_INTERPOLATION:
      if ((ctx.capsState->imageInterpolation == value) | ((uint)value >= INTERPOLATION_INVALID)) return;
      if (!_detachCaps()) return;

      ctx.capsState->imageInterpolation = value;
      break;

    case PAINTER_HINT_GRADIENT_INTERPOLATION:
      if ((ctx.capsState->gradientInterpolation == value) | ((uint)value >= INTERPOLATION_INVALID)) return;
      if (!_detachCaps()) return;

      ctx.capsState->gradientInterpolation = value;
      break;

    default:
      break;
  }
}

int RasterPainterEngine::getHint(int hint) const
{
  switch (hint)
  {
    case PAINTER_HINT_ANTIALIASING_QUALITY:
      return ctx.capsState->aaQuality;

    case PAINTER_HINT_IMAGE_INTERPOLATION:
      return ctx.capsState->imageInterpolation;

    case PAINTER_HINT_GRADIENT_INTERPOLATION:
      return ctx.capsState->gradientInterpolation;

    default:
      return -1;
  }
}

// ============================================================================
// [Fog::RasterPainterEngine - Fill Parameters]
// ============================================================================

void RasterPainterEngine::setFillMode(int mode)
{
  if ((ctx.capsState->fillMode == mode) | ((uint)mode >= FILL_INVALID)) return;
  if (!_detachCaps()) return;

  ctx.capsState->fillMode = mode;
}

int RasterPainterEngine::getFillMode() const
{
  return ctx.capsState->fillMode;
}

// ============================================================================
// [Fog::RasterPainterEngine - Stroke Parameters]
// ============================================================================

void RasterPainterEngine::setStrokeParams(const StrokeParams& strokeParams)
{
  if (!_detachCaps()) return;

  ctx.capsState->strokeParams = strokeParams;

  if (ctx.capsState->strokeParams.getLineCap() >= LINE_CAP_INVALID) ctx.capsState->strokeParams.setLineCap(LINE_CAP_DEFAULT);
  if (ctx.capsState->strokeParams.getLineJoin() >= LINE_JOIN_INVALID) ctx.capsState->strokeParams.setLineJoin(LINE_JOIN_DEFAULT);

  _updateLineWidth();
}

void RasterPainterEngine::getStrokeParams(StrokeParams& strokeParams) const
{
  strokeParams = ctx.capsState->strokeParams;
}

void RasterPainterEngine::setLineWidth(double lineWidth)
{
  if (ctx.capsState->strokeParams.getLineWidth() == lineWidth) return;
  if (!_detachCaps()) return;

  ctx.capsState->strokeParams.setLineWidth(lineWidth);
  _updateLineWidth();
}

double RasterPainterEngine::getLineWidth() const
{
  return ctx.capsState->strokeParams.getLineWidth();
}

void RasterPainterEngine::setLineCap(int lineCap)
{
  if ((ctx.capsState->strokeParams.getLineCap() == lineCap) | ((uint)lineCap >= LINE_CAP_INVALID)) return;
  if (!_detachCaps()) return;

  ctx.capsState->strokeParams.setLineCap(lineCap);
}

int RasterPainterEngine::getLineCap() const
{
  return ctx.capsState->strokeParams.getLineCap();
}

void RasterPainterEngine::setLineJoin(int lineJoin)
{
  if ((ctx.capsState->strokeParams.getLineJoin() == lineJoin) | ((uint)lineJoin >= LINE_JOIN_INVALID)) return;
  if (!_detachCaps()) return;

  ctx.capsState->strokeParams.setLineJoin(lineJoin);
}

int RasterPainterEngine::getLineJoin() const
{
  return ctx.capsState->strokeParams.getLineJoin();
}

void RasterPainterEngine::setDashes(const double* dashes, sysuint_t count)
{
  if (!_detachCaps()) return;

  ctx.capsState->strokeParams.setDashes(dashes, count);
  _updateLineWidth();
}

void RasterPainterEngine::setDashes(const List<double>& dashes)
{
  if (!_detachCaps()) return;

  ctx.capsState->strokeParams.setDashes(dashes);
  _updateLineWidth();
}

List<double> RasterPainterEngine::getDashes() const
{
  return ctx.capsState->strokeParams.getDashes();
}

void RasterPainterEngine::setDashOffset(double offset)
{
  if (ctx.capsState->strokeParams.getDashOffset() == offset) return;
  if (!_detachCaps()) return;

  ctx.capsState->strokeParams.setDashOffset(offset);
  _updateLineWidth();
}

double RasterPainterEngine::getDashOffset() const
{
  return ctx.capsState->strokeParams.getDashOffset();
}

void RasterPainterEngine::setMiterLimit(double miterLimit)
{
  if (ctx.capsState->strokeParams.getMiterLimit() == miterLimit) return;
  if (!_detachCaps()) return;

  ctx.capsState->strokeParams.setMiterLimit(miterLimit);
}

double RasterPainterEngine::getMiterLimit() const
{
  return ctx.capsState->strokeParams.getMiterLimit();
}

// ============================================================================
// [Fog::RasterPainterEngine - Transformations]
// ============================================================================

#define RASTER_BEFORE_MATRIX_OP(__clipState__, __capsState__) \
  __capsState__->transform.tx = __capsState__->transformTranslateSaved.x; \
  __capsState__->transform.ty = __capsState__->transformTranslateSaved.y;

#define RASTER_AFTER_MATRIX_OP(__clipState__, __capsState__) \
  __capsState__->transformTranslateSaved.x = __capsState__->transform.tx; \
  __capsState__->transformTranslateSaved.y = __capsState__->transform.ty; \
  __capsState__->transform.tx += (double)__clipState__->workOrigin.x; \
  __capsState__->transform.ty += (double)__clipState__->workOrigin.y; \

void RasterPainterEngine::setMatrix(const Matrix& m)
{
  if (!_detachCaps()) return;

  ClipState* clipState = ctx.clipState;
  CapsState* capsState = ctx.capsState;

  capsState->transform = m;
  RASTER_AFTER_MATRIX_OP(clipState, capsState)

  _updateTransform(false);
}

void RasterPainterEngine::resetMatrix()
{
  if (!_detachCaps()) return;

  ClipState* clipState = ctx.clipState;
  CapsState* capsState = ctx.capsState;

  capsState->transform.reset();
  capsState->approximationScale = 1.0;
  capsState->transformType = TRANSFORM_TRANSLATE_EXACT;
  RASTER_AFTER_MATRIX_OP(clipState, capsState)

  // Free pattern resource if not needed.
  if (capsState->sourceType == PAINTER_SOURCE_PATTERN) _resetPatternContext();
}

Matrix RasterPainterEngine::getMatrix() const
{
  CapsState* capsState = ctx.capsState;

  return Matrix(
    capsState->transform.sx,
    capsState->transform.shy,
    capsState->transform.shx,
    capsState->transform.sy,
    capsState->transformTranslateSaved.x,
    capsState->transformTranslateSaved.y);
}

void RasterPainterEngine::rotate(double angle, int order)
{
  if (!_detachCaps()) return;

  ClipState* clipState = ctx.clipState;
  CapsState* capsState = ctx.capsState;

  RASTER_BEFORE_MATRIX_OP(clipState, capsState)
  capsState->transform.rotate(angle, order);
  RASTER_AFTER_MATRIX_OP(clipState, capsState)

  _updateTransform(true);
}

void RasterPainterEngine::scale(double sx, double sy)
{
  if (!_detachCaps()) return;

  ClipState* clipState = ctx.clipState;
  CapsState* capsState = ctx.capsState;

  RASTER_BEFORE_MATRIX_OP(clipState, capsState)
  capsState->transform.scale(sx, sy);
  RASTER_AFTER_MATRIX_OP(clipState, capsState)

  _updateTransform(true);
}

void RasterPainterEngine::skew(double sx, double sy, int order)
{
  if (!_detachCaps()) return;

  ClipState* clipState = ctx.clipState;
  CapsState* capsState = ctx.capsState;

  RASTER_BEFORE_MATRIX_OP(clipState, capsState)
  capsState->transform.skew(sx, sy, order);
  RASTER_AFTER_MATRIX_OP(clipState, capsState)

  _updateTransform(true);
}

void RasterPainterEngine::translate(double x, double y, int order)
{
  if (!_detachCaps()) return;

  ClipState* clipState = ctx.clipState;
  CapsState* capsState = ctx.capsState;

  RASTER_BEFORE_MATRIX_OP(clipState, capsState)
  capsState->transform.translate(x, y, order);
  RASTER_AFTER_MATRIX_OP(clipState, capsState)

  _updateTransform(true);
}

void RasterPainterEngine::transform(const Matrix& m, int order)
{
  if (!_detachCaps()) return;

  ClipState* clipState = ctx.clipState;
  CapsState* capsState = ctx.capsState;

  RASTER_BEFORE_MATRIX_OP(clipState, capsState)
  capsState->transform.multiply(m, order);
  RASTER_AFTER_MATRIX_OP(clipState, capsState)

  _updateTransform(false);
}

// Cleanup.
#undef RASTER_BEFORE_MATRIX_OP
#undef RASTER_AFTER_MATRIX_OP

void RasterPainterEngine::worldToScreen(PointD* pt) const
{
  FOG_ASSERT(pt != NULL);

  CapsState* capsState = ctx.capsState;

  if (capsState->transformType >= TRANSFORM_AFFINE)
  {
    capsState->transform.transformVector(&pt->x, &pt->y);
  }

  pt->x += capsState->transformTranslateSaved.x;
  pt->y += capsState->transformTranslateSaved.y;
}

void RasterPainterEngine::screenToWorld(PointD* pt) const
{
  FOG_ASSERT(pt != NULL);

  CapsState* capsState = ctx.capsState;

  if (capsState->transformType >= TRANSFORM_AFFINE)
  {
    capsState->transform.inverseTransformVector(&pt->x, &pt->y);
  }

  pt->x -= capsState->transformTranslateSaved.x;
  pt->y -= capsState->transformTranslateSaved.y;
}

void RasterPainterEngine::alignPoint(PointD* pt) const
{
  FOG_ASSERT(pt != NULL);

  RasterPainterEngine::worldToScreen(pt);
  pt->setX(floor(pt->x) + 0.5);
  pt->setY(floor(pt->y) + 0.5);
  RasterPainterEngine::screenToWorld(pt);
}

// ============================================================================
// [Fog::RasterPainterEngine - State]
// ============================================================================

void RasterPainterEngine::save()
{
  SavedState s;

  s.clipState = ctx.clipState->ref();
  s.capsState = ctx.capsState->ref();
  s.pctx = NULL;

  if (ctx.pctx && ctx.pctx->initialized)
  {
    s.pctx = ctx.pctx;
    s.pctx->refCount.inc();
  }

  states.append(s);
}

void RasterPainterEngine::restore()
{
  if (states.isEmpty()) return;

  SavedState s = states.takeLast();

  _derefClipState(ctx.clipState);
  _derefCapsState(ctx.capsState);

  if (ctx.pctx && ctx.pctx->refCount.deref())
  {
    if (ctx.pctx->initialized) ctx.pctx->destroy(ctx.pctx);
    allocator.free(ctx.pctx);
  }

  ctx.clipState = s.clipState;
  ctx.capsState = s.capsState;
  ctx.pctx = s.pctx;
}

// ============================================================================
// [Fog::RasterPainterEngine - Raster drawing]
// ============================================================================

void RasterPainterEngine::clear()
{
  const Region& r = ctx.clipState->workRegion;
  _serializeBoxes(r.getData(), r.getLength());
}

void RasterPainterEngine::drawPoint(const Point& p)
{
  RasterPainterEngine::drawPoint(
    PointD((double)p.x + 0.5, (double)p.y + 0.5));
}

void RasterPainterEngine::drawLine(const Point& start, const Point& end)
{
  RasterPainterEngine::drawLine(
    PointD((double)start.x + 0.5, (double)start.y + 0.5),
    PointD((double)end.x   + 0.5, (double)end.y   + 0.5));
}

void RasterPainterEngine::drawRect(const Rect& r)
{
  if (!r.isValid()) return;

  ClipState* clipState = ctx.clipState;
  CapsState* capsState = ctx.capsState;

  if (capsState->transformType != TRANSFORM_TRANSLATE_EXACT || !capsState->lineIsSimple)
  {
    RasterPainterEngine::drawRect(
      RectD((double)r.x + 0.5, (double)r.y + 0.5,
            (double)r.w      , (double)r.h)     );
  }
  else
  {
    TemporaryRegion<4> box;
    TemporaryRegion<16> boxISect;

    {
      Region::Data* box_d = box._d;

      int tx = capsState->transformTranslateInt.x;
      int ty = capsState->transformTranslateInt.y;

      box_d->extents.set(r.getX1() + tx, r.getY1() + ty, r.getX2() + tx, r.getY2() + ty);

      if (r.w <= 2 || r.h <= 2)
      {
        box_d->rects[0] = box_d->extents;
        box_d->length = 1;
      }
      else
      {
        box_d->rects[0].set(r.getX1()     + tx, r.getY1()     + ty, r.getX2()     + tx, r.getY1() + 1 + ty);
        box_d->rects[1].set(r.getX1()     + tx, r.getY1() + 1 + ty, r.getX1() + 1 + tx, r.getY2() - 1 + ty);
        box_d->rects[2].set(r.getX2() - 1 + tx, r.getY1() + 1 + ty, r.getX2()     + tx, r.getY2() - 1 + ty);
        box_d->rects[3].set(r.getX1()     + tx, r.getY2() - 1 + ty, r.getX2()     + tx, r.getY2()     + ty);
        box_d->length = 4;
      }
    }

    Region::intersect(boxISect, box, clipState->workRegion);
    _serializeBoxes(boxISect.getData(), boxISect.getLength());
  }
}

void RasterPainterEngine::drawRound(const Rect& r, const Point& radius)
{
  RasterPainterEngine::drawRound(
    RectD((double)r.x + 0.5, (double)r.y + 0.5, r.getWidth(), r.getHeight()),
    PointD((double)radius.x, (double)radius.y));
}

void RasterPainterEngine::fillRect(const Rect& r)
{
  if (!r.isValid()) return;

  ClipState* clipState = ctx.clipState;
  CapsState* capsState = ctx.capsState;

  if (capsState->transformType != TRANSFORM_TRANSLATE_EXACT)
  {
    RasterPainterEngine::fillRect(
      RectD((double)r.x, (double)r.y,
            (double)r.w, (double)r.h));
  }
  else
  {
    int tx = capsState->transformTranslateInt.x;
    int ty = capsState->transformTranslateInt.y;

    Box box(r.getX1() + tx, r.getY1() + ty, r.getX2() + tx, r.getY2() + ty);

    if (clipState->clipSimple)
    {
      Box::intersect(box, box, clipState->clipBox);
      if (!box.isValid()) return;

      _serializeBoxes(&box, 1);
    }
    else
    {
      TemporaryRegion<1> box_(box);
      TemporaryRegion<16> boxISect(box);

      Region::intersect(boxISect, box_, clipState->workRegion);
      if (boxISect.isEmpty()) return;

      _serializeBoxes(boxISect.getData(), boxISect.getLength());
    }
  }
}

void RasterPainterEngine::fillRects(const Rect* r, sysuint_t count)
{
  if (!count) return;

  CapsState* capsState = ctx.capsState;

  if (capsState->transformType != TRANSFORM_TRANSLATE_EXACT)
  {
    tmpPath.clear();
    for (sysuint_t i = 0; i < count; i++)
    {
      if (r[i].isValid()) tmpPath.addRect(
        RectD((double)r[i].x, (double)r[i].y,
              (double)r[i].w, (double)r[i].h));
    }
    fillPath(tmpPath);
  }
  else
  {
    Region region;
    region.set(r, count);
    region.translateAndClip(region, region, capsState->transformTranslateInt, ctx.clipState->clipBox);
    if (region.isEmpty()) return;

    if (!ctx.clipState->clipSimple)
    {
      Region::intersect(region, region, ctx.clipState->workRegion);
      if (region.isEmpty()) return;
    }

    _serializeBoxes(region.getData(), region.getLength());
  }
}

void RasterPainterEngine::fillRound(const Rect& r, const Point& radius)
{
  RasterPainterEngine::fillRound(
    RectD((double)r.x, (double)r.y, (double)r.w, (double)r.h),
    PointD((double)radius.x, (double)radius.y));
}

void RasterPainterEngine::fillRegion(const Region& region)
{
  if (region.isEmpty()) return;

  CapsState* capsState = ctx.capsState;

  if (capsState->transformType != TRANSFORM_TRANSLATE_EXACT)
  {
    const Box* r = region.getData();
    sysuint_t count = region.getLength();

    tmpPath.clear();
    for (sysuint_t i = 0; i < count; i++)
    {
      tmpPath.addRect(
        RectD((double)r[i].getX(), (double)r[i].getY(),
              (double)r[i].getWidth(), (double)r[i].getHeight()));
    }
    fillPath(tmpPath);
  }
  else
  {
    Region regionISect;
    region.translateAndClip(regionISect, region, capsState->transformTranslateInt, ctx.clipState->clipBox);
    if (regionISect.isEmpty()) return;

    if (!ctx.clipState->clipSimple)
    {
      Region::intersect(regionISect, regionISect, ctx.clipState->workRegion);
      if (region.isEmpty()) return;
    }

    _serializeBoxes(region.getData(), region.getLength());
  }
}

// ============================================================================
// [Fog::RasterPainterEngine - Vector drawing]
// ============================================================================

void RasterPainterEngine::drawPoint(const PointD& p)
{
  tmpPath.clear();
  tmpPath.moveTo(p);
  tmpPath.lineTo(p.x, p.y + 0.0001);
  _serializePath(tmpPath, true);
}

void RasterPainterEngine::drawLine(const PointD& start, const PointD& end)
{
  tmpPath.clear();
  tmpPath.moveTo(start);
  tmpPath.lineTo(end);
  _serializePath(tmpPath, true);
}

void RasterPainterEngine::drawLine(const PointD* pts, sysuint_t count)
{
  if (!count) return;

  tmpPath.clear();
  tmpPath.moveTo(pts[0]);
  if (count > 1)
    tmpPath.lineTo(pts + 1, count - 1);
  else
    tmpPath.lineTo(pts[0].x, pts[0].y + 0.0001);
  _serializePath(tmpPath, true);
}

void RasterPainterEngine::drawPolygon(const PointD* pts, sysuint_t count)
{
  if (!count) return;

  tmpPath.clear();
  tmpPath.moveTo(pts[0]);
  if (count > 1)
    tmpPath.lineTo(pts + 1, count - 1);
  else
    tmpPath.lineTo(pts[0].x, pts[0].y + 0.0001);
  tmpPath.closePolygon();
  _serializePath(tmpPath, true);
}

void RasterPainterEngine::drawRect(const RectD& r)
{
  if (!r.isValid()) return;

  tmpPath.clear();
  tmpPath.addRect(r);
  _serializePath(tmpPath, true);
}

void RasterPainterEngine::drawRects(const RectD* r, sysuint_t count)
{
  if (!count) return;

  tmpPath.clear();
  tmpPath.addRects(r, count);
  _serializePath(tmpPath, true);
}

void RasterPainterEngine::drawRound(const RectD& r, const PointD& radius)
{
  tmpPath.clear();
  tmpPath.addRound(r, radius);
  _serializePath(tmpPath, true);
}

void RasterPainterEngine::drawEllipse(const PointD& cp, const PointD& r)
{
  tmpPath.clear();
  tmpPath.addEllipse(cp, r);
  _serializePath(tmpPath, true);
}

void RasterPainterEngine::drawArc(const PointD& cp, const PointD& r, double start, double sweep)
{
  tmpPath.clear();
  tmpPath.addArc(cp, r, start, sweep);
  _serializePath(tmpPath, true);
}

void RasterPainterEngine::drawPath(const Path& path)
{
  _serializePath(path, true);
}

void RasterPainterEngine::fillPolygon(const PointD* pts, sysuint_t count)
{
  if (!count) return;

  tmpPath.clear();
  tmpPath.moveTo(pts[0]);
  if (count > 1)
    tmpPath.lineTo(pts + 1, count - 1);
  else
    tmpPath.lineTo(pts[0].x, pts[0].y + 0.0001);
  tmpPath.closePolygon();
  _serializePath(tmpPath, false);
}

void RasterPainterEngine::fillRect(const RectD& r)
{
  if (!r.isValid()) return;

  CapsState* capsState = ctx.capsState;

  if (capsState->transformType < TRANSFORM_AFFINE)
  {
    ClipState* clipState = ctx.clipState;

    int64_t rx = Math::iround((r.x + capsState->transform.tx) * 256.0);
    if ((rx & 0xFF) != 0x80) goto usePath;

    int64_t ry = Math::iround((r.y + capsState->transform.ty) * 256.0);
    if ((ry & 0xFF) != 0x80) goto usePath;

    int64_t rw = Math::iround((r.w) * 256.0);
    if ((rw & 0xFF) != 0x80) goto usePath;

    int64_t rh = Math::iround((r.h) * 256.0);
    if ((rh & 0xFF) != 0x80) goto usePath;

    Box box;
    box.x1 = (int)(rx >> 8);
    box.y1 = (int)(ry >> 8);
    box.x2 = box.x1 + (int)(rw >> 8);
    box.y2 = box.y1 + (int)(rh >> 8);

    if (clipState->clipSimple)
    {
      Box::intersect(box, box, clipState->clipBox);
      if (!box.isValid()) return;

      _serializeBoxes(&box, 1);
    }
    else
    {
      TemporaryRegion<1> box_(box);
      TemporaryRegion<16> boxISect(box);

      Region::intersect(boxISect, box_, clipState->workRegion);
      if (boxISect.isEmpty()) return;

      _serializeBoxes(boxISect.getData(), boxISect.getLength());
    }
    return;
  }

usePath:
  tmpPath.clear();
  tmpPath.addRect(r);
  _serializePath(tmpPath, false);
}

void RasterPainterEngine::fillRects(const RectD* r, sysuint_t count)
{
  if (!count) return;

  // Single rect fills can be more optimized.
  if (count == 1) fillRect(r[0]);

  tmpPath.clear();
  tmpPath.addRects(r, count);
  _serializePath(tmpPath, false);
}

void RasterPainterEngine::fillRound(const RectD& r, const PointD& radius)
{
  tmpPath.clear();
  tmpPath.addRound(r, radius);
  _serializePath(tmpPath, false);
}

void RasterPainterEngine::fillEllipse(const PointD& cp, const PointD& r)
{
  tmpPath.clear();
  tmpPath.addEllipse(cp, r);
  _serializePath(tmpPath, false);
}

void RasterPainterEngine::fillArc(const PointD& cp, const PointD& r, double start, double sweep)
{
  tmpPath.clear();
  tmpPath.addArc(cp, r, start, sweep);
  _serializePath(tmpPath, false);
}

void RasterPainterEngine::fillPath(const Path& path)
{
  _serializePath(path, false);
}

// ============================================================================
// [Fog::RasterPainterEngine - Glyph / Text Drawing]
// ============================================================================

void RasterPainterEngine::drawGlyph(const Point& pt_, const Glyph& glyph, const Rect* clip_)
{
  tmpGlyphSet.clear();

  int tx = ctx.capsState->transformTranslateInt.x;
  int ty = ctx.capsState->transformTranslateInt.y;

  Point pt(pt_.x + tx, pt_.y + ty);
  Rect clip;

  if (clip_)
  {
    clip.set(clip_->x + tx, clip_->y + ty, clip_->w, clip_->h);
    clip_ = &clip;
  }

  err_t err;
  if ((err = tmpGlyphSet.begin(1))) return;
  tmpGlyphSet._add(glyph._d->ref());
  if ((err = tmpGlyphSet.end())) return;

  _serializeGlyphSet(pt, tmpGlyphSet, clip_);
}

void RasterPainterEngine::drawGlyphSet(const Point& pt_, const GlyphSet& glyphSet, const Rect* clip_)
{
  int tx = ctx.capsState->transformTranslateInt.x;
  int ty = ctx.capsState->transformTranslateInt.y;

  Point pt(pt_.x + tx, pt_.y + ty);
  Rect clip;

  if (clip_)
  {
    clip.set(clip_->x + tx, clip_->y + ty, clip_->w, clip_->h);
    clip_ = &clip;
  }

  _serializeGlyphSet(pt, glyphSet, clip_);
}

void RasterPainterEngine::drawText(const Point& pt_, const String& text, const Font& font, const Rect* clip_)
{
  int tx = ctx.capsState->transformTranslateInt.x;
  int ty = ctx.capsState->transformTranslateInt.y;

  Point pt(pt_.x + tx, pt_.y + ty);
  Rect clip;

  if (clip_)
  {
    clip.set(clip_->x + tx, clip_->y + ty, clip_->w, clip_->h);
    clip_ = &clip;
  }

  tmpGlyphSet.clear();
  if (font.getGlyphSet(text.getData(), text.getLength(), tmpGlyphSet)) return;

  _serializeGlyphSet(pt, tmpGlyphSet, clip_);
}

void RasterPainterEngine::drawText(const Rect& r, const String& text, const Font& font, uint32_t align, const Rect* clip_)
{
  int tx = ctx.capsState->transformTranslateInt.x;
  int ty = ctx.capsState->transformTranslateInt.y;

  Rect clip;

  if (clip_)
  {
    clip.set(clip_->x + tx, clip_->y + ty, clip_->w, clip_->h);
    clip_ = &clip;
  }

  tmpGlyphSet.clear();
  if (font.getGlyphSet(text.getData(), text.getLength(), tmpGlyphSet)) return;

  int wsize = tmpGlyphSet.getAdvance();
  int hsize = font.getHeight();

  int x = r.x + tx;
  int y = r.y + ty;
  int w = r.w;
  int h = r.h;

  switch (align & TEXT_ALIGN_HMASK)
  {
    case TEXT_ALIGN_LEFT:
      break;
    case TEXT_ALIGN_RIGHT:
      x = x + w - wsize;
      break;
    case TEXT_ALIGN_HCENTER:
      x = x + (w - wsize) / 2;
      break;
  }

  switch (align & TEXT_ALIGN_VMASK)
  {
    case TEXT_ALIGN_TOP:
      break;
    case TEXT_ALIGN_BOTTOM:
      y = y + h - hsize;
      break;
    case TEXT_ALIGN_VCENTER:
      y = y + (h - hsize) / 2;
      break;
  }

  _serializeGlyphSet(Point(x, y), tmpGlyphSet, clip_);
}

// ============================================================================
// [Fog::RasterPainterEngine - Image drawing]
// ============================================================================

void RasterPainterEngine::blitImage(const Point& p, const Image& image, const Rect* irect)
{
  CapsState* capsState = ctx.capsState;
  ClipState* clipState = ctx.clipState;

  if (capsState->transformType == TRANSFORM_TRANSLATE_EXACT)
  {
    int srcx = 0;
    int srcy = 0;
    int dstx = p.x + capsState->transformTranslateInt.x;
    int dsty = p.y + capsState->transformTranslateInt.y;
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

      srcx = irect->x;
      if (srcx < 0) return;
      srcy = irect->y;
      if (srcy < 0) return;

      dstw = Math::min(image.getWidth(), irect->getWidth());
      if (dstw == 0) return;
      dsth = Math::min(image.getHeight(), irect->getHeight());
      if (dsth == 0) return;
    }

    int d;

    if ((uint)(d = dstx - clipState->clipBox.getX1()) >= (uint)clipState->clipBox.getWidth())
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

    if ((uint)(d = dsty - clipState->clipBox.getY1()) >= (uint)clipState->clipBox.getHeight())
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

    if ((d = clipState->clipBox.getX2() - dstx) < dstw) dstw = d;
    if ((d = clipState->clipBox.getY2() - dsty) < dsth) dsth = d;

    Rect dst(dstx, dsty, dstw, dsth);
    Rect src(srcx, srcy, dstw, dsth);
    _serializeImage(dst, image, src);
  }
  else
  {
    PointD pd((double)p.x, (double)p.y);
    _serializeImageAffine(pd, image, irect);
  }
}

void RasterPainterEngine::blitImage(const PointD& p, const Image& image, const Rect* irect)
{
  _serializeImageAffine(p, image, irect);
}

// ============================================================================
// [Fog::RasterPainterEngine - Multithreading]
// ============================================================================

void RasterPainterEngine::setEngine(int engine, int cores)
{
  int i;
  bool mt = (engine == PAINTER_ENGINE_RASTER_MT);

  // If worker pool is not created we can't start multithreaded rendering.
  if (!threadPool) return;

  if ((workerManager != NULL) == mt) return;

  // Start multithreading...
  if (mt)
  {
    int max = Math::min<int>(cores > 0 ? cores : cpuInfo->numberOfProcessors, MAX_WORKERS);

#if defined(FOG_DEBUG_RASTER_SYNCHRONIZATION)
    fog_debug("Fog::Painter::setEngine() - starting multithreading (%d threads)", max);
#endif // FOG_DEBUG_RASTER_SYNCHRONIZATION

    workerManager = new(std::nothrow) WorkerManager;
    if (workerManager == NULL) return;

    // This is for testing multithreaded rendering on single cores.
    if (max < 2)
    {
#if defined(FOG_DEBUG_RASTER_SYNCHRONIZATION)
      fog_debug("Fog::Painter::setEngine() - cpu detection says 1, switching to 2");
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
      fog_debug("Fog::Painter::setEngine() - failed to get %d threads from pool, releasing...", max);
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
    fog_debug("Fog::Painter::setEngine() - done");
#endif // FOG_DEBUG_RASTER_SYNCHRONIZATION
  }
  // Stop multithreading
  else
  {
#if defined(FOG_DEBUG_RASTER_SYNCHRONIZATION)
    fog_debug("Fog::Painter::setEngine() - stopping multithreading...");
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
    fog_debug("Fog::Painter::setEngine() - done");
#endif // FOG_DEBUG_RASTER_SYNCHRONIZATION
  }
}

int RasterPainterEngine::getEngine() const
{
  return (workerManager != NULL)
    ? PAINTER_ENGINE_RASTER_MT
    : PAINTER_ENGINE_RASTER_ST;
}

void RasterPainterEngine::flush()
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

void RasterPainterEngine::flushWithQuit()
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
// [Fog::RasterPainterEngine - Helpers]
// ============================================================================

void RasterPainterEngine::_updateWorkRegion()
{
  Layer* layer = ctx.layer;
  ClipState* clipState = ctx.clipState;

  // Caller must ensure this.
  FOG_ASSERT(clipState->refCount.get() == 1);

  // Work origin is point that is added to painter translation matrix and it
  // ensured that raster will be always from [0, 0] -> [W-1, H-1] inclusive.
  Point workOrigin(clipState->metaOrigin + clipState->userOrigin);
  bool workOriginChanged = clipState->workOrigin != workOrigin;

  // Default clip box is no clip.
  clipState->clipBox.set(0, 0, layer->width, layer->height);
  clipState->clipSimple = true;
  clipState->workOrigin = workOrigin;

  // Calculate final clip region.
  uint clipBits = ((uint)clipState->metaRegionUsed << 1) | ((uint)(clipState->userRegionUsed) << 0);

  switch (clipBits)
  {
    // User region only.
    case 0x00000001:
      Region::translateAndClip(clipState->workRegion, clipState->userRegion, clipState->metaOrigin, clipState->clipBox);
      break;

    // Meta region only.
    case 0x00000002:
      Region::translateAndClip(clipState->workRegion, clipState->metaRegion, Point(0, 0), clipState->clipBox);
      break;

    // Meta region & user region.
    case 0x00000003:
    {
      Region tmp;

      Region::translate(tmp, clipState->userRegion, clipState->metaOrigin);
      Region::intersectAndClip(clipState->workRegion, clipState->metaRegion, tmp, clipState->clipBox);
      break;
    }
  }

  if (clipBits)
  {
    clipState->clipBox = clipState->workRegion.extents();
    clipState->clipSimple = clipState->workRegion.getLength() <= 1;
  }

  clipState->workRegionUsed = (clipBits != 0) && !clipState->clipSimple;
  if (workOriginChanged) _updateTransform(true);
}

void RasterPainterEngine::_updateTransform(bool translationOnly)
{
  CapsState* capsState = ctx.capsState;

  // Update translation in pixels.
  capsState->transformTranslateInt.set(
    Math::iround(capsState->transform.tx),
    Math::iround(capsState->transform.ty));

  // If only matrix translation was changed, we can skip some expensive
  // calculations and checking.
  if (translationOnly)
  {
    if (capsState->transformType < TRANSFORM_AFFINE)
    {
      bool isExact = 
        ((Math::iround(capsState->transform.tx * 256.0) & 0xFF) == 0x00) &
        ((Math::iround(capsState->transform.ty * 256.0) & 0xFF) == 0x00) ;

      capsState->transformType = (isExact) 
        ? TRANSFORM_TRANSLATE_EXACT
        : TRANSFORM_TRANSLATE_SUBPX;
    }
  }
  else
  {
    // Just check for matrix characteristics.
    bool isIdentity = 
      (Math::feq(capsState->transform.sx , 1.0)) &
      (Math::feq(capsState->transform.sy , 1.0)) &
      (Math::feq(capsState->transform.shx, 0.0)) &
      (Math::feq(capsState->transform.shy, 0.0)) ;
    bool isExact = isIdentity && (
      ((Math::iround(capsState->transform.tx * 256.0) & 0xFF) == 0x00) &
      ((Math::iround(capsState->transform.ty * 256.0) & 0xFF) == 0x00) );

    // Use matrix characteristics to set correct transform type.
    uint transformType = isIdentity
      ? (isExact ? TRANSFORM_TRANSLATE_EXACT : TRANSFORM_TRANSLATE_SUBPX)
      : (TRANSFORM_AFFINE);

    capsState->transformType = (uint8_t)transformType;

    // sqrt(2.0)/2 ~~ 0.7071068
    if (transformType >= TRANSFORM_AFFINE)
    {
      double x = capsState->transform.sx + capsState->transform.shx;
      double y = capsState->transform.sy + capsState->transform.shy;
      capsState->approximationScale = sqrt(x * x + y * y) * 0.7071068;
    }
    else
    {
      capsState->approximationScale = 1.0;
    }
  }

  // Free pattern context, because transform was changed.
  if (capsState->sourceType == PAINTER_SOURCE_PATTERN) _resetPatternContext();
}

void RasterPainterEngine::_setClipDefaults()
{
  ClipState* clipState = ctx.clipState;

  // Called must ensure this.
  FOG_ASSERT(clipState->refCount.get() == 1);

  clipState->metaOrigin.set(0, 0);
  clipState->userOrigin.set(0, 0);
  clipState->workOrigin.set(0, 0);
  clipState->clipBox.set(0, 0, ctx.layer->width, ctx.layer->height);
  clipState->metaRegionUsed = false;
  clipState->userRegionUsed = false;
  clipState->workRegionUsed = false;
  clipState->clipSimple = true;
}

void RasterPainterEngine::_setCapsDefaults()
{
  CapsState* capsState = ctx.capsState;

  // Caller must ensure this.
  FOG_ASSERT(capsState->refCount.get() == 1);

  switch(capsState->sourceType)
  {
    case PAINTER_SOURCE_ARGB:
      break;
    case PAINTER_SOURCE_PATTERN:
      capsState->pattern.destroy();
      break;
    case PAINTER_SOURCE_COLOR_FILTER:
      capsState->colorFilter.destroy();
      break;
    default:
      FOG_ASSERT_NOT_REACHED();
  }

  capsState->op = COMPOSITE_SRC_OVER;
  capsState->sourceType = PAINTER_SOURCE_ARGB;
  capsState->fillMode = FILL_DEFAULT;
  capsState->aaQuality = ANTI_ALIASING_SMOOTH;
  capsState->imageInterpolation = INTERPOLATION_SMOOTH;
  capsState->gradientInterpolation = INTERPOLATION_SMOOTH;
  capsState->lineIsSimple = true;
  capsState->transformType = TRANSFORM_TRANSLATE_EXACT;

  capsState->solid.argb = 0xFF000000;
  capsState->solid.prgb = 0xFF000000;

  capsState->rops = RasterUtil::getRasterOps(ctx.layer->format, COMPOSITE_SRC_OVER);

  capsState->strokeParams.reset();
  capsState->transform.reset();
  capsState->approximationScale = 1.0;
  capsState->transformTranslateSaved.set(0.0, 0.0);
  capsState->transformTranslateInt.set(0, 0);
}

RasterUtil::PatternContext* RasterPainterEngine::_getPatternContext()
{
  CapsState* capsState = ctx.capsState;

  // Sanity, calling _getPatternContext() for solid source is not allowed.
  FOG_ASSERT(capsState->sourceType == PAINTER_SOURCE_PATTERN);

  RasterUtil::PatternContext* pctx = ctx.pctx;
  err_t err = ERR_OK;

  if (!pctx)
  {
    pctx = ctx.pctx = (RasterUtil::PatternContext*)allocator.alloc(sizeof(RasterUtil::PatternContext));
    if (!pctx) return NULL;

    pctx->refCount.init(0);
    pctx->initialized = false;
  }

  if (!pctx->initialized)
  {
    const Pattern& pattern = capsState->pattern.instance();
    const Matrix& matrix = capsState->transform;

    switch (pattern.getType())
    {
      case PATTERN_TEXTURE:
        err = RasterUtil::functionMap->pattern.texture_init(pctx, pattern, matrix, capsState->imageInterpolation);
        break;
      case PATTERN_LINEAR_GRADIENT:
        err = RasterUtil::functionMap->pattern.linear_gradient_init(pctx, pattern, matrix, capsState->gradientInterpolation);
        break;
      case PATTERN_RADIAL_GRADIENT:
        err = RasterUtil::functionMap->pattern.radial_gradient_init(pctx, pattern, matrix, capsState->gradientInterpolation);
        break;
      case PATTERN_CONICAL_GRADIENT:
        err = RasterUtil::functionMap->pattern.conical_gradient_init(pctx, pattern, matrix, capsState->gradientInterpolation);
        break;
      default:
        FOG_ASSERT_NOT_REACHED();
    }

    pctx->refCount.init(1);
  }

  if (err)
  {
    fog_debug("Fog::RasterPainterEngine::_getPatternContext() - Failed to get context (error %d)\n", err);
    return NULL;
  }

  // Be sure that pattern context is always marked as initialized.
  FOG_ASSERT(pctx->initialized);

  return pctx;
}

void RasterPainterEngine::_resetPatternContext()
{
  // Ignore non-initialize context.
  if (ctx.pctx && ctx.pctx->initialized)
  {
    // Each initialized context must have reference count larger or equal to one.
    // If we dereference it here it means that it's not used by another thread
    // so we can reuse allocated memory later.
    if (ctx.pctx->refCount.deref())
    {
      // Destroy context and reuse memory location later.
      if (ctx.pctx->initialized) ctx.pctx->destroy(ctx.pctx);
    }
    else
    {
      // Other thread will destroy it.
      ctx.pctx = NULL;
    }
  }
}

bool RasterPainterEngine::_detachClip()
{
  if (ctx.clipState->refCount.get() == 1) return true;

  ClipState* newd = reinterpret_cast<ClipState*>(allocator.alloc(sizeof(ClipState)));
  if (!newd) return false;

  _derefClipState(
    atomicPtrXchg(&ctx.clipState, new(newd) ClipState(*(ctx.clipState)))
  );
  return true;
}

bool RasterPainterEngine::_detachCaps()
{
  if (ctx.capsState->refCount.get() == 1) return true;

  CapsState* newd = reinterpret_cast<CapsState*>(allocator.alloc(sizeof(CapsState)));
  if (!newd) return false;

  _derefCapsState(
    atomicPtrXchg(&ctx.capsState, new(newd) CapsState(*(ctx.capsState)))
  );
  return true;
}

FOG_INLINE void RasterPainterEngine::_derefClipState(ClipState* clipState)
{
  if (clipState->refCount.deref())
  {
    clipState->~ClipState();
    allocator.free(clipState);
  }
}

FOG_INLINE void RasterPainterEngine::_derefCapsState(CapsState* capsState)
{
  if (capsState->refCount.deref())
  {
    capsState->~CapsState();
    allocator.free(capsState);
  }
}

void RasterPainterEngine::_deleteStates()
{
  if (states.isEmpty()) return;

  List<SavedState>::ConstIterator it(states);
  for (it.toStart(); it.isValid(); it.toNext())
  {
    SavedState& s = const_cast<SavedState&>(it.value());
    _derefClipState(s.clipState);
    _derefCapsState(s.capsState);

    if (s.pctx && (!s.pctx->initialized || s.pctx->refCount.deref()))
    {
      if (s.pctx->initialized) s.pctx->destroy(s.pctx);
      allocator.free(s.pctx);
    }
  }

  states.clear();
}

// ============================================================================
// [Fog::RasterPainterEngine - Serializers]
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

void RasterPainterEngine::_serializeBoxes(const Box* box, sysuint_t count)
{
  // Pattern context must be always set up before _render() methods are called.
  if (ctx.capsState->sourceType == PAINTER_SOURCE_PATTERN && !_getPatternContext()) return;

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

void RasterPainterEngine::_serializeImage(const Rect& dst, const Image& image, const Rect& src)
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

void RasterPainterEngine::_serializeImageAffine(const PointD& pt, const Image& image, const Rect* irect)
{
  FOG_ASSERT(!image.isEmpty());

  tmpPath.clear();
  tmpPath.addRect(RectD(pt.x, pt.y, (double)image.getWidth(), (double)image.getHeight()));

  // Singlethreaded.
  if (workerManager == NULL)
  {
    if (_rasterizePath(&ctx, ras, tmpPath, false))
    {
      // Save current pattern context (it will be replaced by context created
      // for the image).
      RasterUtil::PatternContext* oldpctx = ctx.pctx;

      // Create new transformation matrix (based on current matrix and point 
      // where the image should be drawn).
      const Matrix& tr = ctx.capsState->transform;
      PointD pt_(pt);
      tr.transformPoint(&pt_.x, &pt_.y);
      Matrix matrix(tr.sx, tr.shy, tr.shx, tr.sy, pt_.x, pt_.y);

      // Create new pattern context (based on the image).
      RasterUtil::PatternContext imagectx;
      imagectx.initialized = false;
      RasterUtil::functionMap->pattern.texture_init_blit(
        &imagectx, image, matrix, SPREAD_PAD, ctx.capsState->imageInterpolation);
      ctx.pctx = &imagectx;

      // Render path using specific pattern context.
      _renderPath(&ctx, ras, true);

      // Restore old pattern context.
      ctx.pctx = oldpctx;
    }
  }
  // Multithreaded.
  else
  {
    // This is a bit code duplication (see singlethreaded mode), but it quite
    // different. Here we are not interested about rectangle rasterization and
    // replacing pattern context in ctx, but we instead serialize path command
    // with new pattern context (current context will not be replaced at all).

    // Create new transformation matrix (based on current matrix and point 
    // where the image should be drawn).
    const Matrix& tr = ctx.capsState->transform;
    PointD pt_(pt);
    tr.transformPoint(&pt_.x, &pt_.y);
    Matrix matrix(tr.sx, tr.shy, tr.shx, tr.sy, pt_.x, pt_.y);

    // Create new pattern context (based on the image).
    RasterUtil::PatternContext* imagectx = 
      reinterpret_cast<RasterUtil::PatternContext*>(
        allocator.alloc(sizeof(RasterUtil::PatternContext)));
    if (!imagectx) return;

    imagectx->initialized = false;

    RasterUtil::functionMap->pattern.texture_init_blit(
      imagectx, image, matrix, SPREAD_PAD, ctx.capsState->imageInterpolation);
    ctx.pctx = imagectx;

    Command_Path* cmd = _createCommand<Command_Path>(sizeof(Command_Path), imagectx);
    if (!cmd)
    {
      imagectx->destroy(imagectx);
      allocator.free(imagectx);
      return;
    }

    Calculation_Path* clc = _createCalculation<Calculation_Path>();
    if (!clc)
    {
      // This is enough, because release() will dereference pattern context and 
      // free it.
      cmd->release();
      return;
    }

    clc->relatedTo = cmd;
    clc->path.init(tmpPath);
    clc->stroke = false;
    cmd->status.init(Command::Wait);
    cmd->calculation = clc;
    cmd->ras = NULL; // Will be initialized by calculation.
    cmd->textureBlit = true;
    _postCommand(cmd, clc);
  }
}

void RasterPainterEngine::_serializeGlyphSet(const Point& pt, const GlyphSet& glyphSet, const Rect* clip)
{
  Box boundingBox = ctx.clipState->clipBox;

  if (clip)
  {
    Box::intersect(boundingBox, boundingBox, Box(*clip));
    if (!boundingBox.isValid()) return;
  }

  // Pattern context must be always set up before _render() methods are called.
  if (ctx.capsState->sourceType == PAINTER_SOURCE_PATTERN && !_getPatternContext()) return;

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

void RasterPainterEngine::_serializePath(const Path& path, bool stroke)
{
  // Pattern context must be always set up before _render() methods are called.
  if (ctx.capsState->sourceType == PAINTER_SOURCE_PATTERN && !_getPatternContext()) return;

  // Singlethreaded.
  if (workerManager == NULL)
  {
    if (_rasterizePath(&ctx, ras, path, stroke)) _renderPath(&ctx, ras, false);
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
    cmd->ras = NULL; // Will be initialized by calculation.
    cmd->textureBlit = false;
    _postCommand(cmd, clc);
  }
}

template <typename T>
FOG_INLINE T* RasterPainterEngine::_createCommand(sysuint_t size)
{
  T* command = reinterpret_cast<T*>(workerManager->allocator.alloc(size));
  if (!command) return NULL;

  new(command) T;

  command->refCount.init(workerManager->numWorkers);
  command->engine = this;
  command->layer = ctx.layer;
  command->clipState = ctx.clipState->ref();
  command->capsState = ctx.capsState->ref();
  command->pctx = NULL;
  command->status.init(Command::Ready);
  command->calculation = NULL;

  // Initialize pattern context if source is pattern type.
  if (ctx.capsState->sourceType == PAINTER_SOURCE_PATTERN)
  {
    FOG_ASSERT(ctx.pctx && ctx.pctx->initialized);
    ctx.pctx->refCount.inc();
    command->pctx = ctx.pctx;
  }

  return command;
}

template <typename T>
FOG_INLINE T* RasterPainterEngine::_createCommand(sysuint_t size, RasterUtil::PatternContext* pctx)
{
  T* command = reinterpret_cast<T*>(workerManager->allocator.alloc(size));
  if (!command) return NULL;

  new(command) T;

  command->refCount.init(workerManager->numWorkers);
  command->engine = this;
  command->layer = ctx.layer;
  command->clipState = ctx.clipState->ref();
  command->capsState = ctx.capsState->ref();
  command->pctx = pctx;
  command->status.init(Command::Ready);
  command->calculation = NULL;

  return command;
}
template<typename T>
T* RasterPainterEngine::_createCalculation(sysuint_t size)
{
  T* calculation = reinterpret_cast<T*>(workerManager->allocator.alloc(size));
  if (!calculation) return NULL;

  new(calculation) T;
  calculation->engine = this;
  return calculation;
}

void RasterPainterEngine::_postCommand(Command* cmd, Calculation* clc)
{
#if defined(FOG_DEBUG_RASTER_COMMANDS)
  fog_debug("Fog::Painter::_postCommand() - posting...");
#endif // FOG_DEBUG_RASTER_COMMANDS

  // Flush everything if commands get maximum.
  if (workerManager->commandsPosition == MAX_COMMANDS)
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
// [Fog::RasterPainterEngine - Renderers - AntiGrain]
// ============================================================================

bool RasterPainterEngine::_rasterizePath(Context* ctx, Rasterizer* ras, const Path& path, bool stroke)
{
  RasterPainterEngine::ClipState* clipState = ctx->clipState;
  RasterPainterEngine::CapsState* capsState = ctx->capsState;

  // Use transformation matrix only if it makes sense.
  const Matrix* matrix = NULL;
  if (capsState->transformType >= TRANSFORM_AFFINE) matrix = &capsState->transform;

  Path dst;

  ras->reset();
  ras->setClipBox(clipState->clipBox);

  bool noTransform = (
    capsState->transformType == TRANSFORM_TRANSLATE_EXACT &&
    capsState->transformTranslateInt.x == 0 &&
    capsState->transformTranslateInt.y == 0);

  if (stroke)
  {
    // Stroke mode. Stroke will flatten the path.
    Stroker(capsState->strokeParams, capsState->approximationScale).stroke(dst, path);

    if (capsState->transformType >= TRANSFORM_AFFINE)
    {
      dst.applyMatrix(capsState->transform);
    }
    else if (!noTransform)
    {
      dst.translate(capsState->transform.tx, capsState->transform.ty);
    }

    // Stroke not respects fill mode set in caps state, instead we are using
    // FILL_NON_ZERO.
    ras->setFillRule(FILL_NON_ZERO);
  }
  else
  {
    // Fill mode.
    path.flattenTo(dst, noTransform ? NULL : &capsState->transform, capsState->approximationScale);

    // Fill respects fill mode set in caps state.
    ras->setFillRule(capsState->fillMode);
  }

  ras->addPath(dst);
  ras->finalize();

  return ras->hasCells();
}

// ============================================================================
// [Fog::RasterPainterEngine - Renderers]
// ============================================================================

void RasterPainterEngine::_renderBoxes(Context* ctx, const Box* box, sysuint_t count)
{
  if (!count) return;

  RasterPainterEngine::Layer* layer = ctx->layer;
  RasterPainterEngine::CapsState* capsState = ctx->capsState;

  uint8_t* pixels = layer->pixels;
  sysint_t stride = layer->stride;
  sysint_t bpp = layer->bpp;

  RasterUtil::Closure* closure = &ctx->closure;

  int offset = ctx->offset;
  int delta = ctx->delta;
  sysint_t strideWithDelta = stride * delta;

  switch (capsState->sourceType)
  {
    // Solid source type.
    case PAINTER_SOURCE_ARGB:
    {
      const RasterUtil::Solid* source = &capsState->solid;
      RasterUtil::CSpanFn cspan = capsState->rops->cspan;

      for (sysuint_t i = 0; i < count; i++)
      {
        int x1 = box[i].getX1();
        int y1 = box[i].getY1();
        int y2 = box[i].getY2();

        int w = box[i].getWidth();
        if (w <= 0) continue;

        if (delta != 1) y1 = alignToDelta(y1, offset, delta);
        if (y1 >= y2) continue;

        uint8_t* pCur = pixels + (sysint_t)y1 * stride + (sysint_t)x1 * bpp;
        do {
          cspan(pCur, source, (sysuint_t)w, closure);
          pCur += strideWithDelta;
          y1 += delta;
        } while (y1 < y2);
      }
      break;
    }

    // Pattern source type.
    case PAINTER_SOURCE_PATTERN:
    {
      RasterUtil::PatternContext* pctx = ctx->pctx;
      if (!pctx) return;

      int format = layer->format;
      int op = capsState->op;
      RasterUtil::VSpanFn vspan = capsState->rops->vspan[pctx->format];

      // Fastpath: Do not copy pattern to extra buffer, if compositing operation
      // is copy. We need to match pixel formats and make sure that operator is
      // COMPOSITE_SRC or COMPOSITE_SRC_OVER without alpha channel (opaque).
      if (format == pctx->format && (op == COMPOSITE_SRC || (op == COMPOSITE_SRC_OVER && format == PIXEL_FORMAT_XRGB32)))
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

          uint8_t* pCur = pixels + (sysint_t)y1 * stride + (sysint_t)x1 * bpp;
          do {
            uint8_t* f = pctx->fetch(pctx, pCur, x1, y1, w);
            if (f != pCur) vspan(pCur, f, w, closure);

            pCur += strideWithDelta;
            y1 += delta;
          } while (y1 < y2);
        }
      }
      else
      {
        uint8_t* pBuf = ctx->getBuffer(ByteUtil::mul4(ctx->clipState->clipBox.getWidth()));
        if (!pBuf) return;

        for (sysuint_t i = 0; i < count; i++)
        {
          int x1 = box[i].getX1();
          int y1 = box[i].getY1();
          int y2 = box[i].getY2();

          int w = box[i].getWidth();
          if (w <= 0) continue;

          if (delta != 1) y1 = alignToDelta(y1, offset, delta);
          if (y1 >= y2) continue;

          uint8_t* pCur = pixels + (sysint_t)y1 * stride + (sysint_t)x1 * bpp;
          do {
            vspan(pCur,
              pctx->fetch(pctx, pBuf, x1, y1, w),
              (sysuint_t)w, closure);
            pCur += strideWithDelta;
            y1 += delta;
          } while (y1 < y2);
        }
      }
      break;
    }

    // Color filter.
    case PAINTER_SOURCE_COLOR_FILTER:
    {
      const ImageFilterEngine* cfEngine = capsState->colorFilter->getEngine();
      const void* cfContext = cfEngine->getContext();
      ColorFilterFn cspan = cfEngine->getColorFilterFn(layer->format);

      // TODO:
      if (1)
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

          uint8_t* pCur = pixels + (sysint_t)y1 * stride + (sysint_t)x1 * bpp;
          do {
            cspan(pCur, pCur, (sysuint_t)w, cfContext);
            pCur += strideWithDelta;
            y1 += delta;
          } while (y1 < y2);
        }
      }
      else
      {
        // TODO:
        //uint8_t* buf = ctx->getBuffer(this->width * 4);
      }

      cfEngine->releaseContext(cfContext);
      break;
    }
  }
}

void RasterPainterEngine::_renderImage(Context* ctx, const Rect& dst, const Image& image, const Rect& src)
{
  RasterPainterEngine::Layer* layer = ctx->layer;
  RasterPainterEngine::CapsState* capsState = ctx->capsState;

  Image::Data* image_d = image._d;
  sysint_t dstStride = layer->stride;
  sysint_t srcStride = image_d->stride;

  int x = dst.x;
  int w = dst.w;

  int y1 = dst.y;
  int y2 = dst.y + dst.h;

  uint8_t* dstCur = layer->pixels + (sysint_t)x * layer->bpp;
  const uint8_t* srcCur = image_d->first + (sysint_t)src.x * image_d->bytesPerPixel;

  int delta = ctx->delta;
  if (delta == 1)
  {
    dstCur += (sysint_t)y1 * dstStride;
    srcCur += (sysint_t)src.y * srcStride;
  }
  else
  {
    int offset = ctx->offset;
    y1 = alignToDelta(y1, offset, delta);
    if (y1 >= y2) return;

    dstCur += (sysint_t)y1 * dstStride;
    srcCur += ((sysint_t)src.y + y1 - dst.y) * srcStride;

    dstStride *= delta;
    srcStride *= delta;
  }

  RasterUtil::VSpanFn vspan = capsState->rops->vspan[image_d->format];
  RasterUtil::Closure closure;

  closure.dstPalette = NULL;
  closure.srcPalette = image._d->palette.getData();

  do {
    vspan(dstCur, srcCur, (sysuint_t)w, &closure);
    dstCur += dstStride;
    srcCur += srcStride;
    y1 += delta;
  } while (y1 < y2);
}

void RasterPainterEngine::_renderGlyphSet(Context* ctx, const Point& pt, const GlyphSet& glyphSet, const Box& boundingBox)
{
  // TODO: Hardcoded to A8 glyph format.
  // TODO: Clipping.

  RasterPainterEngine::Layer* layer = ctx->layer;
  RasterPainterEngine::CapsState* capsState = ctx->capsState;
  RasterPainterEngine::ClipState* clipState = ctx->clipState;

  if (!glyphSet.getLength()) return;

  int offset = ctx->offset;
  int delta = ctx->delta;

  const Glyph* glyphs = glyphSet.glyphs();
  sysuint_t count = glyphSet.getLength();

  int px = pt.x;
  int py = pt.y;

  uint8_t* pixels = layer->pixels;
  sysint_t stride = layer->stride;
  sysint_t strideWithDelta = stride * delta;
  sysint_t bpp = layer->bpp;

  RasterUtil::Closure closure;
  closure.dstPalette = NULL;
  closure.srcPalette = NULL;

  switch (capsState->sourceType)
  {
    // Solid source type.
    case PAINTER_SOURCE_ARGB:
    {
      RasterUtil::CSpanMskFn cspan_a8 = capsState->rops->cspan_a8;
      const RasterUtil::Solid* source = &capsState->solid;

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

        uint8_t* pCur = pixels;
        pCur += (sysint_t)y1 * stride;
        pCur += (sysint_t)x1 * bpp;

        sysint_t glyphStride = bitmapd->stride;
        const uint8_t* pGlyph = bitmapd->first;

        pGlyph += (sysint_t)(y1 - py1) * glyphStride;
        pGlyph += (sysint_t)(x1 - px1);

        if (delta != 1) glyphStride *= delta;

        do {
          cspan_a8(pCur, source, pGlyph, (sysuint_t)w, &closure);
          pCur += strideWithDelta;
          pGlyph += glyphStride;
          y1 += delta;
        } while (y1 < y2);
      }
      break;
    }

    // Pattern source type.
    case PAINTER_SOURCE_PATTERN:
    {
      RasterUtil::PatternContext* pctx = ctx->pctx;
      if (!pctx) return;

      RasterUtil::VSpanMskFn vspan_a8 = capsState->rops->vspan_a8[pctx->format];

      uint8_t* pbuf = ctx->getBuffer(ByteUtil::mul4(clipState->clipBox.getWidth()));
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

        uint8_t* pCur = pixels;
        pCur += (sysint_t)y1 * stride;
        pCur += (sysint_t)x1 * bpp;

        sysint_t glyphStride = bitmapd->stride;
        const uint8_t* pGlyph = bitmapd->first;

        pGlyph += (sysint_t)(y1 - py1) * glyphStride;
        pGlyph += (sysint_t)(x1 - px1);

        if (delta != 1) glyphStride *= delta;

        do {
          vspan_a8(pCur,
            pctx->fetch(pctx, pbuf, x1, y1, w),
            pGlyph, (sysuint_t)w, &closure);

          pCur += strideWithDelta;
          pGlyph += glyphStride;
          y1 += delta;
        } while (y1 < y2);
      }
      break;
    }

    // Color filter.
    case PAINTER_SOURCE_COLOR_FILTER:
    {
      // TODO:
      break;
    }
  }
}

void RasterPainterEngine::_renderPath(Context* ctx, Rasterizer* ras, bool textureBlit)
{
  RasterPainterEngine::Layer* layer = ctx->layer;
  RasterPainterEngine::ClipState* clipState = ctx->clipState;
  RasterPainterEngine::CapsState* capsState = ctx->capsState;
  Scanline32* scanline = &ctx->scanline;

  if (scanline->init(ras->getCellsBounds().x1, ras->getCellsBounds().x2) != ERR_OK) return;

  int y = ras->getCellsBounds().y1;
  int y_end = ras->getCellsBounds().y2;
  int delta = ctx->delta;

  if (ctx->id != -1)
  {
    y = alignToDelta(y, ctx->offset, delta);
    if (y >= y_end) return;
  }

  sysint_t stride = layer->stride;
  sysint_t strideWithDelta = stride * delta;
  sysint_t bpp = layer->bpp;

  uint8_t* pBase;
  uint8_t* pCur;

  RasterUtil::Closure closure;
  closure.dstPalette = NULL;
  closure.srcPalette = NULL;

  uint sourceType = textureBlit ? PAINTER_SOURCE_PATTERN : capsState->sourceType;
  switch (sourceType)
  {
    // Solid source type.
    case PAINTER_SOURCE_ARGB:
    {
      RasterUtil::CSpanScanlineFn blitter = capsState->rops->cspan_a8_scanline;
      const RasterUtil::Solid* source = &capsState->solid;

      if (clipState->clipSimple)
      {
        pBase = layer->pixels + y * stride;
        for (; y < y_end; y += delta, pBase += strideWithDelta)
        {
          uint numSpans = ras->sweepScanline(scanline, y);
          if (numSpans == 0) continue;

          blitter(pBase, source, scanline->getSpansData(), numSpans, &closure);
        }
      }
      else
      {
        const Box* clipCur = clipState->workRegion.getData();
        const Box* clipTo;
        const Box* clipEnd = clipCur + clipState->workRegion.getLength();
        sysuint_t clipLen;

        // Advance clip pointer.
sourceArgbClipAdvance:
        while (clipCur->y2 <= y)
        {
          if (++clipCur == clipEnd) return;
        }
        // Advance to end of the current span list (same y1, y2).
        clipTo = clipCur + 1;
        if (clipTo != clipEnd && clipCur->y1 == clipTo->y1) clipTo++;
        clipLen = (sysuint_t)(clipTo - clipCur);

        // Skip some rows if needed.
        if (y < clipCur->y1)
        {
          y = clipCur->y1;
          if (ctx->id != -1)
          {
            y = alignToDelta(y, ctx->offset, delta);
            if (y >= clipCur->y2)
            {
              clipCur = clipTo;
              if (clipCur == clipEnd) return;
              goto sourceArgbClipAdvance;
            }
          }
        }

        pBase = layer->pixels + y * stride;
        for (; y < y_end; y += delta, pBase += strideWithDelta)
        {
          // Advance clip pointer if needed.
          if (y >= clipCur->y2)
          {
            clipCur = clipTo;
            if (clipCur == clipEnd) return;
            goto sourceArgbClipAdvance;
          }

          uint numSpans = ras->sweepScanline(scanline, y, clipCur, clipLen);
          if (numSpans == 0) continue;

          blitter(pBase, source, scanline->getSpansData(), numSpans, &closure);
        }
      }
      break;
    }

    // Pattern source type.
    case PAINTER_SOURCE_PATTERN:
    {
      RasterUtil::PatternContext* pctx = ctx->pctx;
      if (!pctx) return;

      uint8_t* pBuf = ctx->getBuffer(ByteUtil::mul4(clipState->clipBox.getWidth()));
      if (!pBuf) return;

      RasterUtil::VSpanFn vspan = capsState->rops->vspan[pctx->format];
      RasterUtil::VSpanMskFn vspan_a8 = capsState->rops->vspan_a8[pctx->format];
      RasterUtil::VSpanMskConstFn vspan_a8_const = capsState->rops->vspan_a8_const[pctx->format];

      if (clipState->clipSimple)
      {
        pBase = layer->pixels + y * stride;
        for (; y < y_end; y += delta, pBase += strideWithDelta)
        {
          uint numSpans = ras->sweepScanline(scanline, y);
          if (numSpans == 0) continue;

          const Scanline32::Span* span = scanline->getSpansData();

          for (;;)
          {
            int x = span->x;
            int len = span->len;

            pCur = pBase + x * bpp;

            if (len > 0)
            {
              vspan_a8(pCur,
                pctx->fetch(pctx, pBuf, x, y, len),
                span->covers, len, &closure);
            }
            else
            {
              len = -len;
              FOG_ASSERT(len > 0);

              uint32_t cover = (uint32_t)*(span->covers);
              if (cover == 0xFF)
              {
                vspan(pCur,
                  pctx->fetch(pctx, pBuf, x, y, len),
                  len, &closure);
              }
              else
              {
                vspan_a8_const(pCur,
                  pctx->fetch(pctx, pBuf, x, y, len),
                  cover, len, &closure);
              }
            }

            if (--numSpans == 0) break;
            ++span;
          }
        }
      }
      else
      {
        const Box* clipCur = clipState->workRegion.getData();
        const Box* clipTo;
        const Box* clipEnd = clipCur + clipState->workRegion.getLength();
        sysuint_t clipLen;

        // Advance clip pointer.
sourcePatternClipAdvance:
        while (clipCur->y2 <= y)
        {
          if (++clipCur == clipEnd) return;
        }
        // Advance to end of the current span list (same y1, y2).
        clipTo = clipCur + 1;
        if (clipTo != clipEnd && clipCur->y1 == clipTo->y1) clipTo++;
        clipLen = (sysuint_t)(clipTo - clipCur);

        // Skip some rows if needed.
        if (y < clipCur->y1)
        {
          y = clipCur->y1;
          if (ctx->id != -1)
          {
            y = alignToDelta(y, ctx->offset, delta);
            if (y >= clipCur->y2)
            {
              clipCur = clipTo;
              if (clipCur == clipEnd) return;
              goto sourcePatternClipAdvance;
            }
          }
        }

        pBase = layer->pixels + y * stride;
        for (; y < y_end; y += delta, pBase += strideWithDelta)
        {
          // Advance clip pointer if needed.
          if (y >= clipCur->y2)
          {
            clipCur = clipTo;
            if (clipCur == clipEnd) return;
            goto sourcePatternClipAdvance;
          }

          uint numSpans = ras->sweepScanline(scanline, y, clipCur, clipLen);
          if (numSpans == 0) continue;

          const Scanline32::Span* span = scanline->getSpansData();

          for (;;)
          {
            int x = span->x;
            int len = span->len;

            pCur = pBase + x * bpp;

            if (len > 0)
            {
              vspan_a8(pCur,
                pctx->fetch(pctx, pBuf, x, y, len),
                span->covers, len, &closure);
            }
            else
            {
              len = -len;
              FOG_ASSERT(len > 0);

              uint32_t cover = (uint32_t)*(span->covers);
              if (cover == 0xFF)
              {
                vspan(pCur,
                  pctx->fetch(pctx, pBuf, x, y, len),
                  len, &closure);
              }
              else
              {
                vspan_a8_const(pCur,
                  pctx->fetch(pctx, pBuf, x, y, len),
                  cover, len, &closure);
              }
            }

            if (--numSpans == 0) break;
            ++span;
          }
        }
      }
      break;
    }

    case PAINTER_SOURCE_COLOR_FILTER:
    {
      uint8_t* pBuf = ctx->getBuffer(ByteUtil::mul4(clipState->clipBox.getWidth()));
      if (!pBuf) return;

      int format = layer->format;

      const ImageFilterEngine* cfEngine = capsState->colorFilter->getEngine();
      const void* cfContext = cfEngine->getContext();
      ColorFilterFn cspan = cfEngine->getColorFilterFn(format);

      RasterUtil::VSpanMskFn vspan_a8 =
        RasterUtil::functionMap->composite[COMPOSITE_SRC][format].vspan_a8[format];
      RasterUtil::VSpanMskConstFn vspan_a8_const =
        RasterUtil::functionMap->composite[COMPOSITE_SRC][format].vspan_a8_const[format];

      if (clipState->clipSimple)
      {
        pBase = layer->pixels + y * stride;
        for (; y < y_end; y += delta, pBase += strideWithDelta)
        {
          uint numSpans = ras->sweepScanline(scanline, y);
          if (numSpans == 0) continue;

          const Scanline32::Span* span = scanline->getSpansData();

          for (;;)
          {
            int x = span->x;
            int len = span->len;

            pCur = pBase + x * bpp;

            if (len > 0)
            {
              cspan(pBuf, pCur, len, cfContext);
              vspan_a8(pCur, pBuf, span->covers, len, &closure);
            }
            else
            {
              len = -len;
              FOG_ASSERT(len > 0);

              uint32_t cover = (uint32_t)*(span->covers);
              if (cover == 0xFF)
              {
                cspan(pCur, pCur, len, cfContext);
              }
              else
              {
                cspan(pBuf, pCur, len, cfContext);
                vspan_a8_const(pCur, pBuf, cover, len, &closure);
              }
            }

            if (--numSpans == 0) break;
            ++span;
          }
        }
      }
      else
      {
        const Box* clipCur = clipState->workRegion.getData();
        const Box* clipTo;
        const Box* clipEnd = clipCur + clipState->workRegion.getLength();
        sysuint_t clipLen;

        // Advance clip pointer.
sourceColorFilterClipAdvance:
        while (clipCur->y2 <= y)
        {
          if (++clipCur == clipEnd) goto sourceColorFilterClipEnd;
        }
        // Advance to end of the current span list (same y1, y2).
        clipTo = clipCur + 1;
        if (clipTo != clipEnd && clipCur->y1 == clipTo->y1) clipTo++;
        clipLen = (sysuint_t)(clipTo - clipCur);

        // Skip some rows if needed.
        if (y < clipCur->y1)
        {
          y = clipCur->y1;
          if (ctx->id != -1)
          {
            y = alignToDelta(y, ctx->offset, delta);
            if (y >= clipCur->y2)
            {
              clipCur = clipTo;
              if (clipCur == clipEnd) goto sourceColorFilterClipEnd;
              goto sourceColorFilterClipAdvance;
            }
          }
        }

        pBase = layer->pixels + y * stride;
        for (; y < y_end; y += delta, pBase += strideWithDelta)
        {
          // Advance clip pointer if needed.
          if (y >= clipCur->y2)
          {
            clipCur = clipTo;
            if (clipCur == clipEnd) goto sourceColorFilterClipEnd;
            goto sourceColorFilterClipAdvance;
          }

          uint numSpans = ras->sweepScanline(scanline, y, clipCur, clipLen);
          if (numSpans == 0) continue;

          const Scanline32::Span* span = scanline->getSpansData();

          for (;;)
          {
            int x = span->x;
            int len = span->len;

            pCur = pBase + x * bpp;

            if (len > 0)
            {
              cspan(pBuf, pCur, len, cfContext);
              vspan_a8(pCur, pBuf, span->covers, len, &closure);
            }
            else
            {
              len = -len;
              FOG_ASSERT(len > 0);

              uint32_t cover = (uint32_t)*(span->covers);
              if (cover == 0xFF)
              {
                cspan(pCur, pCur, len, cfContext);
              }
              else
              {
                cspan(pBuf, pCur, len, cfContext);
                vspan_a8_const(pCur, pBuf, cover, len, &closure);
              }
            }

            if (--numSpans == 0) break;
            ++span;
          }
        }
      }
sourceColorFilterClipEnd:
      cfEngine->releaseContext(cfContext);
      break;
    }
  }
}

// ============================================================================
// [Public API]
// ============================================================================

PainterEngine* _getRasterPainterEngine(const ImageBuffer& buffer, int hints)
{
  return new(std::nothrow) RasterPainterEngine(buffer, hints);
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_painter_raster_init(void)
{
  using namespace Fog;

  // Init multithreading.
  //
  // If CPU contains more cores, we can improve rendering speed by using them.
  // By default we set number of threads (workers) to count of CPU cores + 1.
  RasterPainterEngine::threadPool = new(std::nothrow) ThreadPool();

  return ERR_OK;
}

FOG_INIT_DECLARE void fog_painter_raster_shutdown(void)
{
  using namespace Fog;

  // Kill multithreading.
  if (RasterPainterEngine::threadPool)
  {
    delete RasterPainterEngine::threadPool;
    RasterPainterEngine::threadPool = NULL;
  }
}
