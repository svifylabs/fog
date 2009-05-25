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
#include <Fog/Core/Lock.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Raster.h>
#include <Fog/Graphics/Raster/Raster_Jit.h>

namespace Fog {
namespace Raster {

struct BlitJitLocal
{
  // Private jit contexts data.
  JitContext contexts_data[Image::FormatCount * CompositeCount];

  // Table of pointers, because it can be read from multiple threads without
  // locking.
  JitContext* contexts[Image::FormatCount * CompositeCount];

  // Internal lock.
  Lock lock;

  BlitJitLocal()
  {
    BlitJit::Api::init();

    Memory::zero(contexts, sizeof(contexts));
    Memory::zero(contexts_data, sizeof(contexts_data));
  }

  ~BlitJitLocal()
  {

  }
};

Static<BlitJitLocal> blitjit_local;

static FOG_INLINE sysint_t jitIndex(uint32_t pfId, uint32_t oId)
{
  return pfId * CompositeCount + oId;
}

static const BlitJit::PixelFormat* fogPixelFormatToBlitJit(uint32_t pfId)
{
  switch (pfId)
  {
    case Image::FormatARGB32:
      return &BlitJit::Api::pixelFormats[BlitJit::PixelFormat::ARGB32];

    case Image::FormatPRGB32:
      return &BlitJit::Api::pixelFormats[BlitJit::PixelFormat::PRGB32];

    case Image::FormatRGB32:
      return &BlitJit::Api::pixelFormats[BlitJit::PixelFormat::XRGB32];

    case Image::FormatRGB24:
      return &BlitJit::Api::pixelFormats[BlitJit::PixelFormat::RGB24];

    case Image::FormatA8:
      return &BlitJit::Api::pixelFormats[BlitJit::PixelFormat::A8];

    default:
      FOG_ASSERT(0);
      return NULL;
  }
}

static const BlitJit::Operator* fogOperatorToBlitJit(uint32_t oId)
{
  switch (oId)
  {
    case CompositeSrc        : return &BlitJit::Api::operators[BlitJit::Operator::CompositeSrc];
    case CompositeDest       : return &BlitJit::Api::operators[BlitJit::Operator::CompositeDest];
    case CompositeSrcOver    : return &BlitJit::Api::operators[BlitJit::Operator::CompositeOver];
    case CompositeDestOver   : return &BlitJit::Api::operators[BlitJit::Operator::CompositeOverReverse];
    case CompositeSrcIn      : return &BlitJit::Api::operators[BlitJit::Operator::CompositeIn];
    case CompositeDestIn     : return &BlitJit::Api::operators[BlitJit::Operator::CompositeInReverse];
    case CompositeSrcOut     : return &BlitJit::Api::operators[BlitJit::Operator::CompositeOut];
    case CompositeDestOut    : return &BlitJit::Api::operators[BlitJit::Operator::CompositeOutReverse];
    case CompositeSrcAtop    : return &BlitJit::Api::operators[BlitJit::Operator::CompositeAtop];
    case CompositeDestAtop   : return &BlitJit::Api::operators[BlitJit::Operator::CompositeAtopReverse];
    case CompositeXor        : return &BlitJit::Api::operators[BlitJit::Operator::CompositeXor];
    case CompositeClear      : return &BlitJit::Api::operators[BlitJit::Operator::CompositeClear];

    case CompositeAdd        : return &BlitJit::Api::operators[BlitJit::Operator::CompositeAdd];
    case CompositeSubtract   : return &BlitJit::Api::operators[BlitJit::Operator::CompositeSubtract];
    case CompositeMultiply   : return &BlitJit::Api::operators[BlitJit::Operator::CompositeMultiply];
    case CompositeScreen     : return &BlitJit::Api::operators[BlitJit::Operator::CompositeScreen];
    case CompositeDarken     : return &BlitJit::Api::operators[BlitJit::Operator::CompositeDarken];
    case CompositeLighten    : return &BlitJit::Api::operators[BlitJit::Operator::CompositeLighten];
    case CompositeDifference : return &BlitJit::Api::operators[BlitJit::Operator::CompositeDifference];
    case CompositeExclusion  : return &BlitJit::Api::operators[BlitJit::Operator::CompositeExclusion];
    case CompositeInvert     : return &BlitJit::Api::operators[BlitJit::Operator::CompositeInvert];
    case CompositeInvertRgb  : return &BlitJit::Api::operators[BlitJit::Operator::CompositeInvertRgb];

    default:
      FOG_ASSERT(0);
      return NULL;
  }
}

FOG_API JitContext* getJitContext(uint32_t pfId, uint32_t oId)
{
  FOG_ASSERT(pfId < Image::FormatCount);
  FOG_ASSERT(oId < CompositeCount);

  sysint_t index = jitIndex(pfId, oId);
  if (blitjit_local->contexts[index]) return blitjit_local->contexts[index];

  AutoLock locked(blitjit_local->lock);
  // Other thread initialized the context?
  if (blitjit_local->contexts[index]) return blitjit_local->contexts[index];

  JitContext* ctx = &blitjit_local->contexts_data[index];

  const BlitJit::PixelFormat* pfDst =
    fogPixelFormatToBlitJit(pfId);

  const BlitJit::PixelFormat* pfSrc =
    fogPixelFormatToBlitJit(
      pfId == Image::FormatPRGB32
        ? Image::FormatPRGB32
        : Image::FormatARGB32);

  const BlitJit::PixelFormat* pfMaskA8 = fogPixelFormatToBlitJit(Image::FormatA8);
  const BlitJit::Operator* op = fogOperatorToBlitJit(oId);


  ctx->fillSpan = BlitJit::Api::genFillSpan(pfDst, pfSrc, op);
  ctx->fillRect = BlitJit::Api::genFillRect(pfDst, pfSrc, op);
  ctx->fillSpanM_A8 = BlitJit::Api::genFillSpanWithMask(pfDst, pfSrc, pfMaskA8, op);

  // TODO: Hacked
  //for (i = 0; i < ImageFormat::Count-1; i++)
  //  ctx->blitSpan[i] = BlitJit::Api::genBlitSpan(pfDst, fogPixelFormatToBlitJit(i), op);
  ctx->blitSpan[Image::FormatARGB32] =
    BlitJit::Api::genBlitSpan(pfDst, 
      fogPixelFormatToBlitJit(Image::FormatARGB32), op);

  ctx->blitSpan[Image::FormatPRGB32] =
    BlitJit::Api::genBlitSpan(pfDst,
      fogPixelFormatToBlitJit(Image::FormatPRGB32), op);

  ctx->blitSpan[Image::FormatRGB32] =
    BlitJit::Api::genBlitSpan(pfDst,
      fogPixelFormatToBlitJit(Image::FormatRGB32), op);

  blitjit_local->contexts[index] = ctx;
  return ctx;
}

FOG_API void releaseJitContext(JitContext* ctx)
{
  FOG_ASSERT(ctx != NULL);
}

} // Raster namespace
} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE void fog_raster_init_jit(void)
{
  Fog::Raster::blitjit_local.init();
}

FOG_INIT_DECLARE void fog_raster_shutdown_jit(void)
{
  Fog::Raster::blitjit_local.destroy();
}
