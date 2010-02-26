// [Fog/Graphics Library - Private API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_PAINTERENGINE_RASTER_P_H
#define _FOG_GRAPHICS_PAINTERENGINE_RASTER_P_H

// [Dependencies]
#include <Fog/Graphics/ImageFilter.h>
#include <Fog/Graphics/PaintEngine.h>
#include <Fog/Graphics/RasterEngine_p.h>
#include <Fog/Graphics/Rasterizer_p.h>
#include <Fog/Graphics/Scanline_p.h>
#include <Fog/Graphics/Stroker.h>

//! @addtogroup Fog_Graphics_Private
//! @{

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct RasterPaintCalc;
struct RasterPaintCapsState;
struct RasterPaintClipState;
struct RasterPaintCmd;
struct RasterPaintContext;
struct RasterPaintLayer;
struct RasterPaintStoredState;
struct RasterPaintWorkerManager;
struct RasterPaintTask;

struct RasterPaintEngine;

// ============================================================================
// [Constants]
// ============================================================================

//! @brief Type of current transform in raster paint engine.
enum TRANSFORM_TYPE
{
  //! @brief Transformation matrix is identity or translation only.
  //!
  //! If translation part of matrix is used the values are aligned to pixel
  //! grid (this means that integral numbers can be used for this part). This
  //! is most efficient transform type in simple cases (fill rect, blit image,
  //! etc...).
  TRANSFORM_TRANSLATE_EXACT = 0,

  //! @brief Transformation matrix is identity except translation part which
  //! can be represented only by real numbers. Painter has complex fast paths
  //! for this type, but it's used mainly by image blitting and texture
  //! fetching.
  TRANSFORM_TRANSLATE_SUBPX = 1,

  //! @brief Transformation matrix is affine (not identity or simple translation).
  TRANSFORM_AFFINE = 2
};

//! @brief Status of the raster paint engine command.
//!
//! Status of command is only used if it has associated calculation.
enum COMMAND_STATUS
{
  //! @brief Wait until calculation is done.
  COMMAND_STATUS_WAIT = 0,
  //! @brief Ready to process the command.
  COMMAND_STATUS_READY = 1,
  //! @brief Skip this command - don't call @c RasterPaintAction::run().
  COMMAND_STATUS_SKIP = 2
};

// ============================================================================
// [Fog::MemoryAllocator]
// ============================================================================

//! @brief Memory allocator used by raster paint engine.
//!
//! This allocator allocates larger blocks (see @c BLOCK_SIZE) dividing them
//! into small pieces demanded throught @c MemoryAllocator::alloc() method.
//! Each allocation contains information about memory block used by allocator
//! and when the memory is not needed (and @c MemoryAllocator::free() is called)
//! it's atomically removed from memory block.
//!
//! In short: Each memory block has information about used memory, increased by
//! @c MemoryAllocator::alloc() and decreased by @c MemoryAllocator::free().
//! When the number is decreased to zero then the block is free and will be 
//! reused. The goal of this algorithm is to provide fast memory alloc/free,
//! but do not eat too much memory (reuse it).
//!
//! @note The @c MemoryAllocator::alloc() is reentrant and can be called only
//! by main thread, but @c MemoryAllocator::free() is thread-safe and it can
//! be called (and it is) from worker threads.
struct FOG_HIDDEN MemoryAllocator
{
  // --------------------------------------------------------------------------
  // [Constants]
  // --------------------------------------------------------------------------

  //! @brief Size of one block (decreased by some value to always fit in 
  //! one/two pages). 96 bytes reserved for @c ::malloc() data and our
  //! @c Block data.
  enum { BLOCK_SIZE = 8096 };

  // --------------------------------------------------------------------------
  // [Block]
  // --------------------------------------------------------------------------

  //! @brief One larger memory block.
  struct Block
  {
    //! @brief Link to next block.
    Block* next;

    //! @brief Size of the block.
    sysuint_t size;
    //! @brief Allocator position, incremented by each @c MemoryAllocator::alloc().
    sysuint_t pos;
    //! @brief Count of bytes used by the block (atomic).
    Atomic<sysuint_t> used;

    //! @brief The memory.
    uint8_t memory[BLOCK_SIZE];
  };

  // --------------------------------------------------------------------------
  // [Header]
  // --------------------------------------------------------------------------

  //! @brief Block header.
  struct Header
  {
    //! @brief Link to block structure.
    Block* block;
    //! @brief Size of block (at this time same as BLOCK_SIZE, for future).
    sysuint_t size;
  };

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Memory allocator constructor.
  MemoryAllocator();
  //! @brief Memory allocator destructor, will check if all blocks are freed.
  ~MemoryAllocator();

  // --------------------------------------------------------------------------
  // [Alloc / Free]
  // --------------------------------------------------------------------------

  //! @brief Alloc @a size bytes of memory, like @c ::malloc().
  //!
  //! This method is reentrant, never call it from other than engine thread.
  void* alloc(sysuint_t size);

  //! @brief Free @a ptr allocated by @c alloc();
  FOG_INLINE void free(void* ptr);

  //! @brief Reset, will check if all blocks are freed.
  void reset();

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Link to first memory block (should be the most free one).
  Block* blocks;

private:
  FOG_DISABLE_COPY(MemoryAllocator)
};

FOG_INLINE void MemoryAllocator::free(void* ptr)
{
  Header* header = reinterpret_cast<Header*>(reinterpret_cast<uint8_t*>(ptr) - sizeof(Header));
  header->block->used.sub(header->size);
}

// ============================================================================
// [Fog::RasterPaintLayer]
// ============================================================================

//! @brief Raster paint engine layer.
struct FOG_HIDDEN RasterPaintLayer
{
  //! @brief Data to first raster scanline.
  uint8_t* pixels;

  //! @brief Layer width.
  int width;
  //! @brief Layer height.
  int height;
  //! @brief Layer format.
  int format;

  //! @brief Layer stride (bytes per width including padding).
  sysint_t stride;
  //! @brief Layer bytes per pixel.
  sysint_t bpp;
};

// ============================================================================
// [Fog::RasterPaintClipState]
// ============================================================================

//! @brief Raster paint engine clipping state.
struct FOG_HIDDEN RasterPaintClipState
{
  // ------------------------------------------------------------------------
  // [Construction / Destruction]
  // ------------------------------------------------------------------------

  RasterPaintClipState();
  RasterPaintClipState(const RasterPaintClipState& other);
  ~RasterPaintClipState();

  // ------------------------------------------------------------------------
  // [Ref]
  // ------------------------------------------------------------------------

  FOG_INLINE RasterPaintClipState* ref() const;

  // ------------------------------------------------------------------------
  // [Operator Overload]
  // ------------------------------------------------------------------------

  RasterPaintClipState& operator=(const RasterPaintClipState& other);

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

  uint8_t metaRegionUsed;
  uint8_t userRegionUsed;
  uint8_t workRegionUsed;
  uint8_t clipSimple;
};

// ============================================================================
// [Fog::RasterPaintCapsState]
// ============================================================================

//! @brief Raster paint engine capabilities.
struct FOG_HIDDEN RasterPaintCapsState
{
  // ------------------------------------------------------------------------
  // [Construction / Destruction]
  // ------------------------------------------------------------------------

  RasterPaintCapsState();
  RasterPaintCapsState(const RasterPaintCapsState& other);
  ~RasterPaintCapsState();

  // ------------------------------------------------------------------------
  // [Ref]
  // ------------------------------------------------------------------------

  FOG_INLINE RasterPaintCapsState* ref() const;

  // ------------------------------------------------------------------------
  // [Operator Overload]
  // ------------------------------------------------------------------------

  RasterPaintCapsState& operator=(const RasterPaintCapsState& other);

  // ------------------------------------------------------------------------
  // [Members]
  // ------------------------------------------------------------------------

  //! @brief Reference count.
  mutable Atomic<sysuint_t> refCount;

  union
  {
    struct
    {
      //! @brief Compositing operator, see @c OPERATOR_TYPE.
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

  union
  {
    //! @brief Solid source data (if @c sourceType is @c PAINTER_SOURCE_SOLID).
    RasterEngine::Solid solid;
    //! @brief Pattern source data (if @c sourceType is @c PAINTER_SOURCE_PATTERN).
    Static<Pattern> pattern;
    //! @brief Color filter data (if @c sourceType is @c PAINTER_SOURCE_COLOR_FILTER).
    Static<ColorFilter> colorFilter;
  };

  //! @brief Pointer to compositing functions, see @c op.
  RasterEngine::FunctionMap::CompositeFuncs* rops;

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

// ============================================================================
// [Fog::RasterPaintStoredState]
// ============================================================================

//! @brief State structure used by save() and restore() methods.
struct FOG_HIDDEN RasterPaintStoredState
{
  RasterPaintClipState* clipState;
  RasterPaintCapsState* capsState;
  RasterEngine::PatternContext* pctx;
};

// ============================================================================
// [Fog::RasterPaintContext]
// ============================================================================

// RasterPaintContext is accessed always from only one thread.
struct FOG_HIDDEN RasterPaintContext
{
  // ------------------------------------------------------------------------
  // [Construction / Destruction]
  // ------------------------------------------------------------------------

  RasterPaintContext();
  ~RasterPaintContext();

  // ------------------------------------------------------------------------
  // [Buffer Manager]
  // ------------------------------------------------------------------------

  uint8_t* getBuffer(sysint_t size);

  // ------------------------------------------------------------------------
  // [Members]
  // ------------------------------------------------------------------------

  // Owner of this context.
  RasterPaintEngine* engine;

  // Layer.
  RasterPaintLayer* layer;
  // Clip state.
  RasterPaintClipState* clipState;
  // Capabilities state.
  RasterPaintCapsState* capsState;

  // Raster closure.
  RasterEngine::Closure closure;

  // Pattern context.
  RasterEngine::PatternContext* pctx;

  // Scanline rasterizer container.
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
  FOG_DISABLE_COPY(RasterPaintContext)
};

// ============================================================================
// [Fog::RasterRenderImageAffineBound]
// ============================================================================

//! @brief Class that is used as a wrapper to do image affine transformation
//! using compositing operator that is bound (that means that multiplying weight
//! and image alpha is permited).
struct RasterRenderImageAffineBound
{
  // [Construction / Destruction]

  FOG_INLINE RasterRenderImageAffineBound();
  FOG_INLINE ~RasterRenderImageAffineBound();

  // [Init]

  bool init(const Image& image, const Matrix& matrix, const Box& clipBox, int interpolationType);
  FOG_INLINE bool isInitialized() const { return (bool)ictx.initialized; }

  // [Render]

  void render(RasterPaintContext* ctx);

  // [Members]

  RasterEngine::PatternContext ictx;
  PointD pts[4];
  int pty[4];

  int leftStart;
  int leftDirection;

  int rightStart;
  int rightDirection;

  int ytop;

  int xmin;
  int ymin;
  int xmax;
  int ymax;
};

// ============================================================================
// [Fog::RasterPaintAction]
// ============================================================================

//! @brief Raster paint action is base class for @c RasterPaintCmd and
//! @c RasterPaintCalc classes. It shares common stuff and dispatching.
struct FOG_HIDDEN RasterPaintAction
{
  //! @brief Action constructor.
  FOG_INLINE RasterPaintAction() {}

  //! @brief Action destructor (non-virtual).
  //!
  //! It's not virtual! Use @c release() to destroy resources associated
  //! with the action.
  FOG_INLINE ~RasterPaintAction() {}

  // [Abstract]

  //! @brief Run action (command or calculation).
  virtual void run(RasterPaintContext* ctx) = 0;
  //! @brief Release action (it means free / dereference all associated
  //! resources with this action).
  virtual void release() = 0;

  //! @brief Free all common resources (called only by @c release() method).
  //!
  //! This is convenience method that is often used by @c release() method
  //! implementation. It's inlined for efficiency.
  FOG_INLINE void _free();

  // [Members]

  //! @brief Owner of this action.
  RasterPaintEngine* engine;

private:
  FOG_DISABLE_COPY(RasterPaintAction)
};

// ============================================================================
// [Fog::RasterPaintCmd]
// ============================================================================

//! @brief Command - base class for all painter commands.
//!
//! Command is action that is executed by each painter's thread (worker). To 
//! make sure that threads are working with own data the @c run() method must
//! use worker delta and offset variables.
//!
//! Commands are executed in LILO (last-in, last-out) order for each thread,
//! but threads are not synchronized. It means that threads may process different
//! commands in parallel, but one-by-one.
//!
//! Command can depend to @c calculation. If calculation is not @c NULL and
//! command status is @c COMMAND_STATUS_WAIT then thread can't process
//! current command until command status is set to @c COMMAND_STATUS_READY
//! or COMMAND_STATUS_SKIP (that means skip this command). Instead of waiting
//! the worker can work on different calculation, see @c RasterPaintTask::run()
//! implementation. Notice that command status is set by @c RasterPaintCalc if
//! used.
//!
//! After command is processed the @c RasterPaintAction::release() is called.
//! To create own command the method must be implemented and you must ensure
//! to call @c RasterPaintCmd::_releaseObjects() there. When writing command
//! concentrate only to @c RasterPaintAction::release() method, because all
//! other stuff should be done by worker task.
struct FOG_HIDDEN RasterPaintCmd : public RasterPaintAction
{
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  FOG_INLINE RasterPaintCmd() {};
  FOG_INLINE ~RasterPaintCmd() {};

  //! @brief This method destroys only general @c RasterPaintCmd data.
  //!
  //! Within this method the @c RasterPaintAction::_free() is called, so
  //! never call it again
  FOG_INLINE void _releaseObjects();

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Reference count.
  //!
  //! Initial value is count of workers and each worker decreases it when
  //! finished. If value is decreased to zero worker must release the command
  //! using @c RasterPaintAction::release() method. This is done by worker task.
  Atomic<int> refCount;

  //! @brief Status of this command.
  Atomic<int> status;

  //! @brief Layer - contains info about raster.
  RasterPaintLayer* layer;
  //! @brief Clip state of the command.
  RasterPaintClipState* clipState;
  //! @brief Capabilities of the command.
  RasterPaintCapsState* capsState;

  //! @brief Pattern context (must be @c NULL if single color is used).
  //!
  //! TODOC
  RasterEngine::PatternContext* pctx;

  //! @brief Calculation.
  RasterPaintCalc* calculation;
};

// ============================================================================
// [Fog::RasterPaintCmdBoxes]
// ============================================================================

struct FOG_HIDDEN RasterPaintCmdBoxes : public RasterPaintCmd
{
  // [Construction / Destruction]

  FOG_INLINE RasterPaintCmdBoxes() {};
  FOG_INLINE ~RasterPaintCmdBoxes() {};

  // [Implementation]

  virtual void run(RasterPaintContext* ctx);
  virtual void release();

  // [Members]

  sysuint_t count;
  Box boxes[1];
};

// ============================================================================
// [Fog::RasterPaintCmdImage]
// ============================================================================

struct FOG_HIDDEN RasterPaintCmdImage : public RasterPaintCmd
{
  // [Construction / Destruction]

  FOG_INLINE RasterPaintCmdImage() {};
  FOG_INLINE ~RasterPaintCmdImage() {};

  // [Implementation]

  virtual void run(RasterPaintContext* ctx);
  virtual void release();

  // [Members]

  Static<Image> image;

  Rect dst;
  Rect src;
};

// ============================================================================
// [Fog::RasterPaintCmdImageAffineBound]
// ============================================================================

struct FOG_HIDDEN RasterPaintCmdImageAffineBound : public RasterPaintCmd
{
  // [Construction / Destruction]

  FOG_INLINE RasterPaintCmdImageAffineBound() { renderer.init(); };
  FOG_INLINE ~RasterPaintCmdImageAffineBound() {};

  // [Implementation]

  virtual void run(RasterPaintContext* ctx);
  virtual void release();

  // [Members]

  Static<RasterRenderImageAffineBound> renderer;
};

// ============================================================================
// [Fog::RasterPaintCmdGlyphSet]
// ============================================================================

struct FOG_HIDDEN RasterPaintCmdGlyphSet : public RasterPaintCmd
{
  // [Construction / Destruction]

  FOG_INLINE RasterPaintCmdGlyphSet() {};
  FOG_INLINE ~RasterPaintCmdGlyphSet() {};

  // [Implementation]

  virtual void run(RasterPaintContext* ctx);
  virtual void release();

  // [Members]

  Static<GlyphSet> glyphSet;

  Point pt;
  Box boundingBox;
};

// ============================================================================
// [Fog::RasterPaintCmdPath]
// ============================================================================

struct FOG_HIDDEN RasterPaintCmdPath : public RasterPaintCmd
{
  // [Construction / Destruction]

  FOG_INLINE RasterPaintCmdPath() {};
  FOG_INLINE ~RasterPaintCmdPath() {};

  // [Implementation]

  virtual void run(RasterPaintContext* ctx);
  virtual void release();

  // [Members]

  //! @brief Rasterizer (where path is rasterized by @c RasterPaintCalc)
  Rasterizer* ras;

  //! @brief Whether it's texture blit command instead of draw/fill path
  //! command.
  //!
  //! Texture blit commands are serialized through @c _serializeImageAffine(),
  //! difference is that it's always used PAINTER_SOURCE_PATTERN type to do
  //! pattern based blit.
  int textureBlit;
};

// ============================================================================
// [Fog::RasterPaintCalc]
// ============================================================================

//! @brief Calculation
//!
//! Calculation is complex action (usually sequental) that can be run only by
//! one thread. To improve performance ofsuch calculations raster paint engine
//! can distribute them to different threads.
struct FOG_HIDDEN RasterPaintCalc : public RasterPaintAction
{
  FOG_INLINE RasterPaintCalc() {};
  FOG_INLINE ~RasterPaintCalc() {};

  //! @brief Related command to this calculation.
  RasterPaintCmd* relatedTo;
};

// ============================================================================
// [Fog::RasterPaintCalcPath]
// ============================================================================

struct FOG_HIDDEN RasterPaintCalcPath : public RasterPaintCalc
{
  FOG_INLINE RasterPaintCalcPath() {};
  FOG_INLINE ~RasterPaintCalcPath() {};

  // [Implementation]

  virtual void run(RasterPaintContext* ctx);
  virtual void release();

  // [Members]

  //! @brief Owner raster paint engine.
  RasterPaintEngine* engine;

  //! @brief Path to process.
  Static<Path> path;
  bool stroke;
};

// ============================================================================
// [API]
// ============================================================================

FOG_HIDDEN PaintEngine* _getRasterPaintEngine(const ImageBuffer& buffer, int hints);

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_PAINTERENGINE_RASTER_P_H
