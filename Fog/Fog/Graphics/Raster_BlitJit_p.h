// [Fog/Graphics library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_RASTER_BLITJIT_P_H
#define _FOG_GRAPHICS_RASTER_BLITJIT_P_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/Rgba.h>

// [BlitJit]
#include <BlitJit/BlitJit.h>

namespace Fog {
namespace Raster {

// ============================================================================
// [Fog::Raster - BlitJit]
// ============================================================================

struct JitContext
{
  typedef BlitJit::FillSpanFn FillSpan;
  typedef BlitJit::FillRectFn FillRect;
  typedef BlitJit::BlitSpanFn BlitSpan;

  typedef BlitJit::FillSpanWithMaskFn FillSpanM;
  typedef BlitJit::BlitSpanWithMaskFn BlitSpanM;

  FillSpan  fillSpan    ;
  FillRect  fillRect    ;
  FillSpanM fillSpanM_A8;
  BlitSpan  blitSpan    [Image::FormatCount];
  BlitSpanM blitSpanM_A8[Image::FormatCount];
};

FOG_API JitContext* getJitContext(uint32_t pfId, uint32_t oId);
FOG_API void releaseJitContext(JitContext* ctx);

} // Raster namespace
} // Fog namespace

// [Guard]
#endif // _FOG_GRAPHICS_RASTER_BLITJIT_P_H
