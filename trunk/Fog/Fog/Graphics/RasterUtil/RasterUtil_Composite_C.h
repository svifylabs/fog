// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// For some IDEs to enable code-assist.
#include <Fog/Build/Build.h>

#if defined(FOG_IDE)
#include <Fog/Graphics/RasterUtil/RasterUtil_Defs_C.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Dib_C.h>
#endif // FOG_IDE

namespace Fog {
namespace RasterUtil {

// ============================================================================
// [Fog::RasterUtil::C - Composite - NOP]
// ============================================================================

struct FOG_HIDDEN CompositeNopC
{
  static void FOG_FASTCALL cspan(
    uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure)
  {
    FOG_UNUSED(dst);
    FOG_UNUSED(src);
    FOG_UNUSED(w);
    FOG_UNUSED(closure);
  }

  static void FOG_FASTCALL cspan_a8(
    uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure)
  {
    FOG_UNUSED(dst);
    FOG_UNUSED(src);
    FOG_UNUSED(msk);
    FOG_UNUSED(w);
    FOG_UNUSED(closure);
  }

  static void FOG_FASTCALL cspan_a8_const(
    uint8_t* dst, const Solid* src, uint32_t msk0, sysint_t w, const Closure* closure)
  {
    FOG_UNUSED(dst);
    FOG_UNUSED(src);
    FOG_UNUSED(msk0);
    FOG_UNUSED(w);
    FOG_UNUSED(closure);
  }

  static void FOG_FASTCALL cspan_a8_scanline(
    uint8_t* dst, const Solid* src, const Scanline32::Span* spans, sysuint_t numSpans, const Closure* closure)
  {
    FOG_UNUSED(dst);
    FOG_UNUSED(src);
    FOG_UNUSED(spans);
    FOG_UNUSED(numSpans);
    FOG_UNUSED(closure);
  }
};

} // RasterUtil namespace
} // Fog namespace
