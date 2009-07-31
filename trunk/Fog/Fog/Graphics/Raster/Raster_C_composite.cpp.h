// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// For some IDEs to enable code-assist.
#include <Fog/Build/Build.h>
#if defined(FOG_IDE)
#include <Fog/Graphics/Raster/Raster_C_base.cpp.h>
#include <Fog/Graphics/Raster/Raster_C_convert.cpp.h>
#endif // FOG_IDE

namespace Fog {
namespace Raster {

// ============================================================================
// [Fog::Raster - Raster - NOP]
// ============================================================================

static void FOG_FASTCALL raster_pixel_nop(
  uint8_t* dst, const Solid* src, const Closure* closure)
{
  FOG_UNUSED(dst);
  FOG_UNUSED(src);
  FOG_UNUSED(closure);
}

static void FOG_FASTCALL raster_pixel_a8_nop(
  uint8_t* dst, const Solid* src, uint32_t msk, const Closure* closure)
{
  FOG_UNUSED(dst);
  FOG_UNUSED(src);
  FOG_UNUSED(msk);
  FOG_UNUSED(closure);
}

static void FOG_FASTCALL raster_span_solid_nop(
  uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure)
{
  FOG_UNUSED(dst);
  FOG_UNUSED(src);
  FOG_UNUSED(w);
  FOG_UNUSED(closure);
}

static void FOG_FASTCALL raster_span_solid_a8_nop(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  FOG_UNUSED(dst);
  FOG_UNUSED(src);
  FOG_UNUSED(msk);
  FOG_UNUSED(w);
  FOG_UNUSED(closure);
}

static void FOG_FASTCALL raster_span_solid_a8_const_nop(
  uint8_t* dst, const Solid* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  FOG_UNUSED(dst);
  FOG_UNUSED(src);
  FOG_UNUSED(msk0);
  FOG_UNUSED(w);
  FOG_UNUSED(closure);
}

} // Raster namespace
} // Fog namespace
