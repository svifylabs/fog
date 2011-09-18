// [Fog-G2d]
//
// [License]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_G2D_PAINTING_RASTERCONSTANTS_P_H
#define _FOG_G2D_PAINTING_RASTERCONSTANTS_P_H

// [Dependencies]
#include <Fog/Core/Global/Global.h>

namespace Fog {

//! @addtogroup Fog_G2d_Painting
//! @{

// ============================================================================
// [Debugging]
// ============================================================================

// #define FOG_DEBUG_RASTER_CMD
// #define FOG_DEBUG_RASTER_MASK
// #define FOG_DEBUG_RASTER_SYNC
// #define FOG_DEBUG_RASTERIZER

// ============================================================================
// [Fog::RASTER_CORE]
// ============================================================================

enum RASTER_CORE
{
  // --------------------------------------------------------------------------
  // [Multithreading]
  // --------------------------------------------------------------------------

  // Minimum size to set multithreading on.
  RASTER_MIN_SIZE_THRESHOLD = 128 * 128,

  // Maximum number of threads which may be used by the raster engine.
  RASTER_MAX_THREADS_USED = 64,
  // Maximum number of threads which may be suggested for rendering by the
  // raster painter engine.
  RASTER_MAX_THREADS_SUGGESTED = 16,

  // Maximum commands to accumulate in the buffer.
#if defined(FOG_DEBUG_RASTER_SYNC)
  // For debugging it's better to have only minimal buffer size for the commands
  // and calculations.
  RASTER_MAX_COMMANDS = 32,
#else
  RASTER_MAX_COMMANDS = 1024,
#endif // FOG_DEBUG_RASTER_SYNC

  // Maximum calculations to accumulate in the buffer, at this time it must be
  // the same as the maximum count of commands. The reason is that each command
  // can use exactly one calculation so if you need serialize RASTER_MAX_COMMANDS
  // you need also RASTER_MAX_COMMANDS possible calculations.
  RASTER_MAX_CALCULATIONS = RASTER_MAX_COMMANDS,
};

// ============================================================================
// [Fog::RASTER_CALC_OPCODE]
// ============================================================================

enum RASTER_CALC_OPCODE
{
  RASTER_CALC_OPCODE_NULL = 0,

  RASTER_CALC_OPCODE_COUNT
};

// ============================================================================
// [Fog::RASTER_CALC_STATUS]
// ============================================================================

//! @internal
//!
//! @brief The status of raster calculation action (@c RasterCalc class).
enum RASTER_CALC_STATUS
{
  //! @brief The calculation is not finished yet, it might be processed at
  //! the moment.
  RASTER_CALC_STATUS_WAIT = 0,
  //! @brief The calculation is done and ready to use.
  RASTER_CALC_STATUS_DONE = 1,
  //! @brief The calculation evaluation produced no result, depending command
  //! should be skipped.
  RASTER_CALC_STATUS_SKIP = 2
};

// ============================================================================
// [Fog::RASTER_CMD_OPCODE]
// ============================================================================

enum RASTER_CMD_OPCODE
{
  RASTER_CMD_OPCODE_NULL = 0,
  RASTER_CMD_OPCODE_FILL_RECT,
  RASTER_CMD_OPCODE_BLIT_IMAGE,

  RASTER_CMD_OPCODE_COUNT
};

// ============================================================================
// [Fog::RASTER_INTEGRAL_TRANSFORM]
// ============================================================================

//! @internal
enum RASTER_INTEGRAL_TRANSFORM
{
  //! @brief Transform is not integral.
  RASTER_INTEGRAL_TRANSFORM_NONE = 0,

  //! @brief Transform is simple (identity or translation).
  RASTER_INTEGRAL_TRANSFORM_SIMPLE = 1,
  //! @brief Transform is scaling (translation + scaling).
  RASTER_INTEGRAL_TRANSFORM_SCALING = 2,
  //! @brief Transform is swap (translation + swap).
  RASTER_INTEGRAL_TRANSFORM_SWAP = 3
};

// ============================================================================
// [Fog::RASTER_STATE]
// ============================================================================

//! @internal
//!
//! @brief Painter state flags used by @c RasterPaintEngine::save() and
//! @c RasterPaintEngine::restore().
enum RASTER_STATE
{
  //! @brief Source.
  RASTER_STATE_SOURCE = 0x00000001,
  //! @brief Stroke parameters.
  RASTER_STATE_STROKE = 0x00000002,

  //! @brief The painter transformation matrix.
  RASTER_STATE_TRANSFORM = 0x00000004,
  //! @brief The painter clipping (clip region or mask).
  RASTER_STATE_CLIPPING = 0x00000008,

  //! @brief All states, used internally.
  RASTER_STATE_ALL =
    RASTER_STATE_SOURCE    |
    RASTER_STATE_STROKE    |
    RASTER_STATE_TRANSFORM |
    RASTER_STATE_CLIPPING
};

// ============================================================================
// [Fog::RASTER_CLIP]
// ============================================================================

//! @internal
//!
//! @brief Raster clip mode.
//!
//! Possible combinations are:
//! - @c RASTER_CLIP_NULL - This can happen, painting is disabled (there is
//!   no area where to paint. Together with this flag the painter state
//!   should also have also set related RASTER_ENGINE_NO_PAINT_... flags.
//! - @c RASTER_CLIP_BOX - Simple clip region (rectangle).
//! - @c RASTER_CLIP_REGION - Complex clip region (set of rectangles).
//! - @c RASTER_CLIP_MASK - Mask clip.
enum RASTER_CLIP
{
  //! @brief Null clip (no paint).
  RASTER_CLIP_NULL = 0,

  //! @brief The clip is a rectangle.
  //!
  //! This flag is never set together with @c RASTER_CLIP_REGION, only one flag
  //! can be set at the time.
  RASTER_CLIP_BOX = 1,

  //! @brief The clip is a region.
  //!
  //! This flag is never set together with @c RASTER_CLIP_BOX, only one flag
  //! can be set at the time.
  RASTER_CLIP_REGION = 2,

  //! @brief The clip is a mask.
  //!
  //! This is addition to @c RASTER_CLIP_BOX and @c RASTER_CLIP_REGION flags.
  RASTER_CLIP_MASK = 3,

  //! @brief Count of raster clip modes.
  RASTER_CLIP_COUNT = 4
};

// ============================================================================
// [Fog::RASTER_CONTEXT_FLAGS]
// ============================================================================

//! @internal
//!
//! @brief Raster context flags (used by master context).
//!
//! NO_PAINT - flags that will be set if some internal state disables
//! painting, see flags and their meanings for details.
//!
//! DIRTY - Some members are dirty and need to be updated.
//!
//! PENDING - flags used to send specific commands to workers. All flags
//! are grouped together and when needed the command is sent. Commands
//! are generally not generated when it's not needed. For example if you save
//! clip state using save() and then restore() it and there weren't made
//! changes, then save and restore commands are not send to workers, because
//! they are not needed.
enum RASTER_MASTER_FLAGS
{
  // --------------------------------------------------------------------------
  // [No-Paint Flags]
  //
  // If any of the no-paint flags is set then painting command will be terminated
  // before the parameters are evaluated. It's one of fastest checking available
  // if raster painter engine.
  // --------------------------------------------------------------------------

  //! @brief Mask including all no-paint states and fatal-error.
  RASTER_NO_PAINT_ALL_FLAGS = 0x8000FFFF,

  //! @brief The meta clip-region contains no painting area.
  RASTER_NO_PAINT_META_REGION = 0x00000001,

  //! @brief The meta transform is invalid.
  RASTER_NO_PAINT_META_TRANSFORM = 0x00000002,

  //! @brief The user clip-region contains no painting area.
  RASTER_NO_PAINT_USER_REGION = 0x00000004,

  //! @brief The user transform is invalid.
  RASTER_NO_PAINT_USER_TRANSFORM = 0x00000008,

  //! @brief The user clip-mask contains no painting area.
  RASTER_NO_PAINT_USER_MASK = 0x00000010,

  //! @brief The compositing operator produces no painting onto the current layer.
  RASTER_NO_PAINT_COMPOSITING_OPERATOR = 0x00000020,

  //! @brief The source color/pattern is invalid.
  RASTER_NO_PAINT_SOURCE = 0x00000040,
  //! @brief The global opacity is zero or very close to zero (zero when
  //! converted to integer).
  RASTER_NO_PAINT_OPACITY = 0x00000080,

  //! @brief Some of the stroke parameters contains degenerated value (for
  //! example line-width set to zero), so, using stroke is no-paint.
  RASTER_NO_PAINT_STROKE = 0x00001000,

  RASTER_NO_PAINT_BASE_FLAGS   = RASTER_NO_PAINT_META_REGION          |
                                 RASTER_NO_PAINT_META_TRANSFORM       |
                                 RASTER_NO_PAINT_USER_REGION          |
                                 RASTER_NO_PAINT_USER_TRANSFORM       |
                                 RASTER_NO_PAINT_USER_MASK            |
                                 RASTER_NO_PAINT_COMPOSITING_OPERATOR |
                                 RASTER_NO_PAINT_OPACITY              ,

  //! @brief Painting is disabled, because something bad happened, for example
  //! memory allocation error for core objects. This error can be cleared only
  //! by @c Painter::switchTo(), @c endLayer() methods.
  //!
  //! This is fatal error that can disable all painting until the paint engine
  //! is destroyed or reused.
  RASTER_NO_PAINT_FATAL = 0x80000000,

  // --------------------------------------------------------------------------
  // [Pending Flags - TOP-TO-BOTTOM Order]
  //
  // The pending flags are evaluated from top-to-bottom order. The evaluation
  // of pending flags can result in no-paint flags so it's needed to check them
  // again after processed.
  // --------------------------------------------------------------------------

  //! @brief Mask that contains all pending states.
  RASTER_PENDING_ALL_FLAGS = 0x7FF00000,

  //! @brief Global transformation matrix was changed, it's needed to
  //! recalculate other members related to transformations before a final
  //! transformation matrix may be used.
  RASTER_PENDING_GLOBAL_TRANSFORM = 0x00100000,

  RASTER_PENDING_CLIP_RECT = 0x00200000,
  RASTER_PENDING_CLIP_REGION = 0x00400000,
  RASTER_PENDING_CLIP_STACK = 0x00800000

  // TODO: Finish

  //! @brief The mask has been marked for saving.
  //!
  //! This flag is used in situation that user called save(RASTER_STATE_CLIP)
  //RASTER_PENDING_MASK_SAVE = 0,
  //RASTER_PENDING_MASK_RESTORE = 0,

#if 0
  // TODO: Remove...

  //! @brief Clipping was changed so the first paint call (that is likely to
  //! paint something) must ensure to call methods that will send the changes
  //! to the workers (making command). If not running in multithreaded mode
  //! then this flag shouldn't be set.
  RASTER_CONTEXT_PENDING_CLIP_REGION = 0x00010000,

  //! @brief Clip state need to be saved (send the command to the workers).
  RASTER_CONTEXT_PENDING_CLIP_SAVE = 0x00020000,

  //! @brief Clip mask initialization is pending.
  RASTER_CONTEXT_PENDING_CLIP_MASK_CREATED = 0x00040000,

  //! @brief Clip state need to be restored (send the command to the workers).
  RASTER_CONTEXT_PENDING_CLIP_MASK_DELETED = 0x00080000,
#endif
};

// ============================================================================
// [Fog::RASTER_MODE]
// ============================================================================

enum RASTER_MODE
{
  RASTER_MODE_ST = 0,
  RASTER_MODE_MT = 1,

  RASTER_MODE_COUNT = 2
};

// ============================================================================
// [Fog::RASTER_MASK_VSPAN_ENUM]
// ============================================================================

enum RASTER_MASK_VSPAN_ENUM
{
  // --------------------------------------------------------------------------
  // [Granularity]
  // --------------------------------------------------------------------------

  // Granularity of pool allocator.
  //
  // Granularity means pool allocator step. For example if granularity is 32,
  // then for span which length is 8 bytes will be allocated span as 32 bytes.
  //
  // Because span structure is relatevely long (20 or 28 bytes?) then it makes
  // no sense to limit granularity too much, 32 or 64 are acceptable values.
  //
  // NOTE: This number must be exponent of two!
  RASTER_MASK_VSPAN_POOL_GRANULARITY_BASE = 32,
  RASTER_MASK_VSPAN_POOL_GRANULARITY_SHIFT = 5, // Log2(32) == 5, 2^5 == 32

  // --------------------------------------------------------------------------
  // [Max Pools]
  // --------------------------------------------------------------------------

  // Count of raster vspan pools (32 is enough).
  RASTER_MASK_VSPAN_POOL_COUNT = 32,

  // --------------------------------------------------------------------------
  // [Max Length]
  // --------------------------------------------------------------------------

  // Maximum size of span that can be generated by clipper, larger spans are
  // split into two or more ones. This simplifies and improves the clipper memory
  // management, because of pooling.
  RASTER_MASK_VSPAN_MAX_LENGTH_8 =
    RASTER_MASK_VSPAN_POOL_GRANULARITY_BASE * RASTER_MASK_VSPAN_POOL_COUNT,

  RASTER_MASK_VSPAN_MAX_LENGTH_16 =
    RASTER_MASK_VSPAN_POOL_GRANULARITY_BASE * RASTER_MASK_VSPAN_POOL_COUNT / 2,

  RASTER_MASK_VSPAN_MAX_LENGTH_32 =
    RASTER_MASK_VSPAN_POOL_GRANULARITY_BASE * RASTER_MASK_VSPAN_POOL_COUNT / 4
};

// ============================================================================
// [Fog::RASTER_PRECISION]
// ============================================================================

enum RASTER_PRECISION
{
  RASTER_PRECISION_NONE = 0x0,
  RASTER_PRECISION_F    = 0x1,
  RASTER_PRECISION_D    = 0x2,
  RASTER_PRECISION_BOTH = 0x3
};

// ============================================================================
// [Fog::RASTER_SOURCE]
// ============================================================================

//! @internal
//!
//! @brief Raster source type.
enum RASTER_SOURCE
{
  RASTER_SOURCE_NONE = 0,
  RASTER_SOURCE_ARGB32 = 1,
  RASTER_SOURCE_COLOR = 2,
  RASTER_SOURCE_TEXTURE = 3,
  RASTER_SOURCE_GRADIENT = 4
};

// ============================================================================
// [Fog::RASTER_SPAN]
// ============================================================================

//! @internal
//!
//! @brief Type of @c RasterSpan.
enum RASTER_SPAN
{
  // --------------------------------------------------------------------------
  // NOTE: When changing these constants, please make sure that the span methods
  // like isConst()/isVariant() are also changed. There are some optimizations
  // which are based on order of these constants.
  // --------------------------------------------------------------------------

  // --------------------------------------------------------------------------
  // [Const]
  // --------------------------------------------------------------------------

  //! @brief Span is a const-mask.
  RASTER_SPAN_C = 0,

  // --------------------------------------------------------------------------
  // [Variant]
  // --------------------------------------------------------------------------

  //! @brief Start of variant-alpha span types (may be used by asserts).
  //!
  //! @note This constant is only shadow to valid mask type, don't use this
  //! value in switch() {}.
  RASTER_SPAN_V_BEGIN = 1,

  //! @brief Span is a variable-alpha mask (8-bit, A8).
  //!
  //! @sa @c RASTER_SPAN_AX_EXTRA.
  RASTER_SPAN_A8_GLYPH = 1,

  //! @brief Span is a variable-alpha mask (target bit-depth, A8, A16 or A32).
  //!
  //! @sa @c RASTER_SPAN_AX_EXTRA.
  RASTER_SPAN_AX_GLYPH = 2,

  //! @brief Span is an extended variable-alpha mask (target bit-depth + 1 bit
  //! for exact scaling).
  RASTER_SPAN_AX_EXTRA = 3,

  //! @brief Span is a variable-argb mask (8-bit, PRGB32).
  RASTER_SPAN_ARGB32_GLYPH = 4,

  //! @brief Span is a variable-argb mask (target bit-depth, PRGB32 or PRGB64).
  RASTER_SPAN_ARGBXX_GLYPH = 5,

  //! @brief The count of span types.
  RASTER_SPAN_COUNT = 6
};

enum
{
  //! @brief Helper constant to determine how many constant pixels are
  //! profitable for pure CMask span instance. Minimum is 1, but recommended
  //! are 4 or more (it depends on the  available SIMD instructions per platform).
  RASTER_SPAN_C_THRESHOLD = 4
};

// ============================================================================
// [Fog::RASTER_WORKER_STATE]
// ============================================================================

//! @internal
//!
//! @brief Raster worker state.
enum RASTER_WORKER_STATE
{
  RASTER_WORKER_RUNNING,
  RASTER_WORKER_WAITING,
  RASTER_WORKER_WAITING_AS_MAIN,
  RASTER_WORKER_DONE
};

//! @}

} // Fog namespace

// [Guard]
#endif // _FOG_G2D_PAINTING_RASTERCONSTANTS_P_H
