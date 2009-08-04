// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// For some IDEs to enable code-assist.
#include <Fog/Build/Build.h>
#if defined(FOG_IDE)
#include <Fog/Graphics/Raster/Raster_SSE2_base.cpp.h>
#endif // FOG_IDE

namespace Fog {
namespace Raster {

#define ADAPTOR_STEP 256

// ============================================================================
// [Fog::Raster - Adaptor - Argb32]
// ============================================================================

static void FOG_FASTCALL adaptor_argb32_pixel(
  uint8_t* dst, const Solid* src, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);

  f[Image::FormatARGB32].span_solid(dst, src, 1, closure);
}

static void FOG_FASTCALL adaptor_argb32_pixel_a8(
  uint8_t* dst, const Solid* src, uint32_t msk0, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);

  f[Image::FormatARGB32].span_solid_a8_const(dst, src, msk0, 1, closure);
}

/*
static void FOG_FASTCALL adaptor_argb32_span_solid(
  uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure)
{
  Must be implemented!
}
*/

/*
static void FOG_FASTCALL adaptor_argb32_span_solid_a8(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  Must be implemented!
}
*/

/*
static void FOG_FASTCALL adaptor_argb32_span_solid_a8_const(
  uint8_t* dst, const Solid* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  Must be implemented!
}
*/

static void FOG_FASTCALL adaptor_argb32_span_composite_argb32(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.prgb32_from_argb32(srcBuf, src, cw, closure);
    f[Image::FormatARGB32].span_composite[Image::FormatPRGB32](dst, srcBuf, cw, closure);

    dst += mul4(cw);
    src += mul4(cw);
    w -= cw;
  } while (w);
}

/*
static void FOG_FASTCALL adaptor_argb32_span_composite_prgb32(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  Must be implemented!
}
*/

static void FOG_FASTCALL adaptor_argb32_span_composite_rgb32(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.axxx32_from_xxxx32(srcBuf, src, cw, closure);
    f[Image::FormatARGB32].span_composite[Image::FormatPRGB32](dst, srcBuf, cw, closure);

    dst += mul4(cw);
    src += mul4(cw);
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_argb32_span_composite_rgb24(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(srcBuf, src, cw, closure);
    f[Image::FormatARGB32].span_composite[Image::FormatPRGB32](dst, srcBuf, cw, closure);

    dst += mul4(cw);
    src += mul3(cw);
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_argb32_span_composite_a8(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.axxx32_from_a8(srcBuf, src, cw, closure);
    f[Image::FormatARGB32].span_composite[Image::FormatPRGB32](dst, srcBuf, cw, closure);

    dst += mul4(cw);
    src += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_argb32_span_composite_i8(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.prgb32_from_i8(srcBuf, src, cw, closure);
    f[Image::FormatARGB32].span_composite[Image::FormatPRGB32](dst, srcBuf, cw, closure);

    dst += mul4(cw);
    src += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_argb32_span_composite_argb32_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.prgb32_from_argb32(srcBuf, src, cw, closure);
    f[Image::FormatARGB32].span_composite_a8[Image::FormatPRGB32](dst, srcBuf, msk, cw, closure);

    dst += mul4(cw);
    src += mul4(cw);
    msk += cw;
    w -= cw;
  } while (w);
}

/*
static void FOG_FASTCALL adaptor_argb32_span_composite_prgb32_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  Must be implemented!
}
*/

static void FOG_FASTCALL adaptor_argb32_span_composite_rgb32_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.axxx32_from_a8(srcBuf, src, cw, closure);
    f[Image::FormatARGB32].span_composite_a8[Image::FormatPRGB32](dst, srcBuf, msk, cw, closure);

    dst += mul4(cw);
    src += mul4(cw);
    msk += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_argb32_span_composite_rgb24_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(srcBuf, src, cw, closure);
    f[Image::FormatARGB32].span_composite_a8[Image::FormatPRGB32](dst, srcBuf, msk, cw, closure);

    dst += mul4(cw);
    src += mul3(cw);
    msk += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_argb32_span_composite_a8_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.axxx32_from_a8(srcBuf, src, cw, closure);
    f[Image::FormatARGB32].span_composite_a8[Image::FormatPRGB32](dst, srcBuf, msk, cw, closure);

    dst += mul4(cw);
    src += cw;
    msk += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_argb32_span_composite_i8_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.prgb32_from_i8(srcBuf, src, cw, closure);
    f[Image::FormatARGB32].span_composite_a8[Image::FormatPRGB32](dst, srcBuf, msk, cw, closure);

    dst += mul4(cw);
    src += cw;
    msk += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_argb32_span_composite_argb32_a8_const(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.prgb32_from_argb32(srcBuf, src, cw, closure);
    f[Image::FormatARGB32].span_composite_a8_const[Image::FormatPRGB32](dst, srcBuf, msk0, cw, closure);

    dst += mul4(cw);
    src += mul4(cw);
    w -= cw;
  } while (w);
}

/*
static void FOG_FASTCALL adaptor_argb32_span_composite_prgb32_a8_const(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  Must be implemented
}
*/

static void FOG_FASTCALL adaptor_argb32_span_composite_rgb32_a8_const(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.axxx32_from_xxxx32(srcBuf, src, cw, closure);
    f[Image::FormatARGB32].span_composite_a8_const[Image::FormatPRGB32](dst, srcBuf, msk0, cw, closure);

    dst += mul4(cw);
    src += mul4(cw);
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_argb32_span_composite_rgb24_a8_const(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(srcBuf, src, cw, closure);
    f[Image::FormatARGB32].span_composite_a8_const[Image::FormatPRGB32](dst, srcBuf, msk0, cw, closure);

    dst += mul4(cw);
    src += mul3(cw);
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_argb32_span_composite_a8_a8_const(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.axxx32_from_a8(srcBuf, src, cw, closure);
    f[Image::FormatARGB32].span_composite_a8_const[Image::FormatPRGB32](dst, srcBuf, msk0, cw, closure);

    dst += mul4(cw);
    src += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_argb32_span_composite_i8_a8_const(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.prgb32_from_i8(srcBuf, src, cw, closure);
    f[Image::FormatARGB32].span_composite_a8_const[Image::FormatPRGB32](dst, srcBuf, msk0, cw, closure);

    dst += mul4(cw);
    src += cw;
    w -= cw;
  } while (w);
}

// ============================================================================
// [Fog::Raster - Adaptor - Prgb32]
// ============================================================================

static void FOG_FASTCALL adaptor_prgb32_pixel(
  uint8_t* dst, const Solid* src, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);

  f[Image::FormatPRGB32].span_solid(dst, src, 1, closure);
}

static void FOG_FASTCALL adaptor_prgb32_pixel_a8(
  uint8_t* dst, const Solid* src, uint32_t msk0, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);

  f[Image::FormatPRGB32].span_solid_a8_const(dst, src, msk0, 1, closure);
}

/*
static void FOG_FASTCALL adaptor_prgb32_span_solid(
  uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure)
{
  Must be implemented!
}
*/

/*
static void FOG_FASTCALL adaptor_prgb32_span_solid_a8(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  Must be implemented!
}
*/

/*
static void FOG_FASTCALL adaptor_prgb32_span_solid_a8_const(
  uint8_t* dst, const Solid* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  Must be implemented!
}
*/

static void FOG_FASTCALL adaptor_prgb32_span_composite_argb32(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.prgb32_from_argb32(srcBuf, src, cw, closure);
    f[Image::FormatPRGB32].span_composite[Image::FormatPRGB32](dst, srcBuf, cw, closure);

    dst += mul4(cw);
    src += mul4(cw);
    w -= cw;
  } while (w);
}

/*
static void FOG_FASTCALL adaptor_prgb32_span_composite_prgb32(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  Must be implemented!
}
*/

static void FOG_FASTCALL adaptor_prgb32_span_composite_rgb32(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w,(sysint_t) ADAPTOR_STEP);
    functionMap->convert.axxx32_from_xxxx32(srcBuf, src, cw, closure);
    f[Image::FormatPRGB32].span_composite[Image::FormatPRGB32](dst, srcBuf, cw, closure);

    dst += mul4(cw);
    src += mul4(cw);
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_prgb32_span_composite_rgb24(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(srcBuf, src, cw, closure);
    f[Image::FormatPRGB32].span_composite[Image::FormatPRGB32](dst, srcBuf, cw, closure);

    dst += mul4(cw);
    src += mul3(cw);
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_prgb32_span_composite_a8(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.axxx32_from_a8(srcBuf, src, cw, closure);
    f[Image::FormatPRGB32].span_composite[Image::FormatPRGB32](dst, srcBuf, cw, closure);

    dst += mul4(cw);
    src += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_prgb32_span_composite_i8(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.prgb32_from_i8(srcBuf, src, cw, closure);
    f[Image::FormatPRGB32].span_composite[Image::FormatPRGB32](dst, srcBuf, cw, closure);

    dst += mul4(cw);
    src += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_prgb32_span_composite_argb32_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.prgb32_from_argb32(srcBuf, src, cw, closure);
    f[Image::FormatPRGB32].span_composite_a8[Image::FormatPRGB32](dst, srcBuf, msk, cw, closure);

    dst += mul4(cw);
    src += mul4(cw);
    msk += cw;
    w -= cw;
  } while (w);
}

/*
static void FOG_FASTCALL adaptor_prgb32_span_composite_prgb32_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  Must be implemented!
}
*/

static void FOG_FASTCALL adaptor_prgb32_span_composite_rgb32_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.axxx32_from_xxxx32(srcBuf, src, cw, closure);
    f[Image::FormatPRGB32].span_composite_a8[Image::FormatPRGB32](dst, srcBuf, msk, cw, closure);

    dst += mul4(cw);
    src += mul4(cw);
    msk += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_prgb32_span_composite_rgb24_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(srcBuf, src, cw, closure);
    f[Image::FormatPRGB32].span_composite_a8[Image::FormatPRGB32](dst, srcBuf, msk, cw, closure);

    dst += mul4(cw);
    src += mul3(cw);
    msk += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_prgb32_span_composite_a8_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.axxx32_from_a8(srcBuf, src, cw, closure);
    f[Image::FormatPRGB32].span_composite_a8[Image::FormatPRGB32](dst, srcBuf, msk, cw, closure);

    dst += mul4(cw);
    src += cw;
    msk += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_prgb32_span_composite_i8_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.prgb32_from_i8(srcBuf, src, cw, closure);
    f[Image::FormatPRGB32].span_composite_a8[Image::FormatPRGB32](dst, srcBuf, msk, cw, closure);

    dst += mul4(cw);
    src += cw;
    msk += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_prgb32_span_composite_argb32_a8_const(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.prgb32_from_argb32(srcBuf, src, cw, closure);
    f[Image::FormatPRGB32].span_composite_a8_const[Image::FormatPRGB32](dst, srcBuf, msk0, cw, closure);

    dst += mul4(cw);
    src += mul4(cw);
    w -= cw;
  } while (w);
}

/*
static void FOG_FASTCALL adaptor_prgb32_span_composite_prgb32_a8_const(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  Must be implemented
}
*/

static void FOG_FASTCALL adaptor_prgb32_span_composite_rgb32_a8_const(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.axxx32_from_xxxx32(srcBuf, src, cw, closure);
    f[Image::FormatPRGB32].span_composite_a8_const[Image::FormatPRGB32](dst, srcBuf, msk0, cw, closure);

    dst += mul4(cw);
    src += mul4(cw);
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_prgb32_span_composite_rgb24_a8_const(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(srcBuf, src, cw, closure);
    f[Image::FormatPRGB32].span_composite_a8_const[Image::FormatPRGB32](dst, srcBuf, msk0, cw, closure);

    dst += mul4(cw);
    src += mul3(cw);
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_prgb32_span_composite_a8_a8_const(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.axxx32_from_a8(srcBuf, src, cw, closure);
    f[Image::FormatPRGB32].span_composite_a8_const[Image::FormatPRGB32](dst, srcBuf, msk0, cw, closure);

    dst += mul4(cw);
    src += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_prgb32_span_composite_i8_a8_const(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.prgb32_from_i8(srcBuf, src, cw, closure);
    f[Image::FormatPRGB32].span_composite_a8_const[Image::FormatPRGB32](dst, srcBuf, msk0, cw, closure);

    dst += mul4(cw);
    src += cw;
    w -= cw;
  } while (w);
}

// ============================================================================
// [Fog::Raster - Adaptor - Rgb32]
// ============================================================================

static void FOG_FASTCALL adaptor_rgb32_pixel(
  uint8_t* dst, const Solid* src, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);

  f[Image::FormatRGB32].span_solid(dst, src, 1, closure);
}

static void FOG_FASTCALL adaptor_rgb32_pixel_a8(
  uint8_t* dst, const Solid* src, uint32_t msk0, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);

  f[Image::FormatRGB32].span_solid_a8_const(dst, src, msk0, 1, closure);
}

/*
static void FOG_FASTCALL adaptor_rgb32_span_solid(
  uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure)
{
  Must be implemented!
}
*/

/*
static void FOG_FASTCALL adaptor_rgb32_span_solid_a8(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  Must be implemented!
}
*/

/*
static void FOG_FASTCALL adaptor_rgb32_span_solid_a8_const(
  uint8_t* dst, const Solid* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  Must be implemented!
}
*/

static void FOG_FASTCALL adaptor_rgb32_span_composite_argb32(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.prgb32_from_argb32(srcBuf, src, cw, closure);
    f[Image::FormatRGB32].span_composite[Image::FormatPRGB32](dst, srcBuf, cw, closure);

    dst += mul4(cw);
    src += mul4(cw);
    w -= cw;
  } while (w);
}

/*
static void FOG_FASTCALL adaptor_rgb32_span_composite_prgb32(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  Must be implemented!
}
*/

static void FOG_FASTCALL adaptor_rgb32_span_composite_rgb32(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.axxx32_from_xxxx32(srcBuf, src, cw, closure);
    f[Image::FormatRGB32].span_composite[Image::FormatPRGB32](dst, srcBuf, cw, closure);

    dst += mul4(cw);
    src += mul4(cw);
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb32_span_composite_rgb24(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(srcBuf, src, cw, closure);
    f[Image::FormatRGB32].span_composite[Image::FormatPRGB32](dst, srcBuf, cw, closure);

    dst += mul4(cw);
    src += mul3(cw);
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb32_span_composite_a8(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.axxx32_from_a8(srcBuf, src, cw, closure);
    f[Image::FormatRGB32].span_composite[Image::FormatPRGB32](dst, srcBuf, cw, closure);

    dst += mul4(cw);
    src += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb32_span_composite_i8(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.prgb32_from_i8(srcBuf, src, cw, closure);
    f[Image::FormatRGB32].span_composite[Image::FormatPRGB32](dst, srcBuf, cw, closure);

    dst += mul4(cw);
    src += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb32_span_composite_argb32_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.prgb32_from_argb32(srcBuf, src, cw, closure);
    f[Image::FormatRGB32].span_composite_a8[Image::FormatPRGB32](dst, srcBuf, msk, cw, closure);

    dst += mul4(cw);
    src += mul4(cw);
    msk += cw;
    w -= cw;
  } while (w);
}

/*
static void FOG_FASTCALL adaptor_rgb32_span_composite_prgb32_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  Must be implemented!
}
*/

static void FOG_FASTCALL adaptor_rgb32_span_composite_rgb32_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.axxx32_from_xxxx32(srcBuf, src, cw, closure);
    f[Image::FormatRGB32].span_composite_a8[Image::FormatPRGB32](dst, srcBuf, msk, cw, closure);

    dst += mul4(cw);
    src += mul4(cw);
    msk += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb32_span_composite_rgb24_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(srcBuf, src, cw, closure);
    f[Image::FormatRGB32].span_composite_a8[Image::FormatPRGB32](dst, srcBuf, msk, cw, closure);

    dst += mul4(cw);
    src += mul3(cw);
    msk += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb32_span_composite_a8_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.axxx32_from_a8(srcBuf, src, cw, closure);
    f[Image::FormatRGB32].span_composite_a8[Image::FormatPRGB32](dst, srcBuf, msk, cw, closure);

    dst += mul4(cw);
    src += cw;
    msk += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb32_span_composite_i8_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.prgb32_from_i8(srcBuf, src, cw, closure);
    f[Image::FormatRGB32].span_composite_a8[Image::FormatPRGB32](dst, srcBuf, msk, cw, closure);

    dst += mul4(cw);
    src += cw;
    msk += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb32_span_composite_argb32_a8_const(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.prgb32_from_argb32(srcBuf, src, cw, closure);
    f[Image::FormatRGB32].span_composite_a8_const[Image::FormatPRGB32](dst, srcBuf, msk0, cw, closure);

    dst += mul4(cw);
    src += mul4(cw);
    w -= cw;
  } while (w);
}

/*
static void FOG_FASTCALL adaptor_rgb32_span_composite_prgb32_a8_const(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  Must be implemented
}
*/

static void FOG_FASTCALL adaptor_rgb32_span_composite_rgb32_a8_const(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.axxx32_from_xxxx32(srcBuf, src, cw, closure);
    f[Image::FormatRGB32].span_composite_a8_const[Image::FormatPRGB32](dst, srcBuf, msk0, cw, closure);

    dst += mul4(cw);
    src += mul4(cw);
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb32_span_composite_rgb24_a8_const(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(srcBuf, src, cw, closure);
    f[Image::FormatRGB32].span_composite_a8_const[Image::FormatPRGB32](dst, srcBuf, msk0, cw, closure);

    dst += mul4(cw);
    src += mul3(cw);
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb32_span_composite_a8_a8_const(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.axxx32_from_a8(srcBuf, src, cw, closure);
    f[Image::FormatRGB32].span_composite_a8_const[Image::FormatPRGB32](dst, srcBuf, msk0, cw, closure);

    dst += mul4(cw);
    src += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb32_span_composite_i8_a8_const(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.prgb32_from_i8(srcBuf, src, cw, closure);
    f[Image::FormatRGB32].span_composite_a8_const[Image::FormatPRGB32](dst, srcBuf, msk0, cw, closure);

    dst += mul4(cw);
    src += cw;
    w -= cw;
  } while (w);
}

// ============================================================================
// [Fog::Raster - Adaptor - Rgb24]
// ============================================================================

static void FOG_FASTCALL adaptor_rgb24_pixel(
  uint8_t* dst, const Solid* src, const Closure* closure)
{
  uint8_t dstBuf[4];
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);

  ((uint32_t*)dstBuf)[0] = PixFmt_RGB24::fetch(dst);
  f[Image::FormatRGB32].span_solid(dstBuf, src, 1, closure);
  PixFmt_RGB24::store(dst, READ_32(dstBuf));
}

static void FOG_FASTCALL adaptor_rgb24_pixel_a8(
  uint8_t* dst, const Solid* src, uint32_t msk0, const Closure* closure)
{
  uint8_t dstBuf[4];
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);

  ((uint32_t*)dstBuf)[0] = PixFmt_RGB24::fetch(dst);
  f[Image::FormatRGB32].span_solid_a8_const(dst, src, msk0, 1, closure);
  PixFmt_RGB24::store(dst, READ_32(dstBuf));
}

static void FOG_FASTCALL adaptor_rgb24_span_solid(
  uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t dstBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(dstBuf, dst, cw, closure);
    f[Image::FormatRGB32].span_solid(dst, src, cw, closure);
    functionMap->convert.rgb24_from_rgb32(dst, dstBuf, cw, closure);

    dst += mul3(cw);
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb24_span_solid_a8(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t dstBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(dstBuf, dst, cw, closure);
    f[Image::FormatRGB32].span_solid_a8(dst, src, msk, cw, closure);
    functionMap->convert.rgb24_from_rgb32(dst, dstBuf, cw, closure);

    dst += mul3(cw);
    msk += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb24_span_solid_a8_const(
  uint8_t* dst, const Solid* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t dstBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(dstBuf, dst, cw, closure);
    f[Image::FormatRGB32].span_solid_a8_const(dst, src, msk0, cw, closure);
    functionMap->convert.rgb24_from_rgb32(dst, dstBuf, cw, closure);

    dst += mul3(cw);
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb24_span_composite_argb32(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t dstBuf[ADAPTOR_STEP * sizeof(uint32_t)];
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(dstBuf, dst, cw, closure);
    functionMap->convert.prgb32_from_argb32(srcBuf, src, cw, closure);
    f[Image::FormatRGB32].span_composite[Image::FormatPRGB32](dstBuf, srcBuf, cw, closure);
    functionMap->convert.rgb24_from_rgb32(dst, dstBuf, cw, closure);

    dst += mul3(cw);
    src += mul4(cw);
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb24_span_composite_prgb32(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t dstBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(dstBuf, dst, cw, closure);
    f[Image::FormatRGB32].span_composite[Image::FormatPRGB32](dstBuf, src, cw, closure);
    functionMap->convert.rgb24_from_rgb32(dst, dstBuf, cw, closure);

    dst += mul3(cw);
    src += mul4(cw);
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb24_span_composite_rgb32(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t dstBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(dstBuf, dst, cw, closure);
    f[Image::FormatRGB32].span_composite[Image::FormatRGB32](dstBuf, src, cw, closure);
    functionMap->convert.rgb24_from_rgb32(dst, dstBuf, cw, closure);

    dst += mul3(cw);
    src += mul4(cw);
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb24_span_composite_rgb24(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t dstBuf[ADAPTOR_STEP * sizeof(uint32_t)];
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(dstBuf, dst, cw, closure);
    functionMap->convert.rgb32_from_rgb24(srcBuf, src, cw, closure);
    f[Image::FormatRGB32].span_composite[Image::FormatRGB32](dstBuf, srcBuf, cw, closure);
    functionMap->convert.rgb24_from_rgb32(dst, dstBuf, cw, closure);

    dst += mul3(cw);
    src += mul3(cw);
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb24_span_composite_a8(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t dstBuf[ADAPTOR_STEP * sizeof(uint32_t)];
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(dstBuf, dst, cw, closure);
    functionMap->convert.axxx32_from_a8(srcBuf, src, cw, closure);
    f[Image::FormatRGB32].span_composite[Image::FormatPRGB32](dstBuf, srcBuf, cw, closure);
    functionMap->convert.rgb24_from_rgb32(dst, dstBuf, cw, closure);

    dst += mul3(cw);
    src += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb24_span_composite_i8(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t dstBuf[ADAPTOR_STEP * sizeof(uint32_t)];
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(dstBuf, dst, cw, closure);
    functionMap->convert.prgb32_from_i8(srcBuf, src, cw, closure);
    f[Image::FormatRGB32].span_composite[Image::FormatPRGB32](dstBuf, srcBuf, cw, closure);
    functionMap->convert.rgb24_from_rgb32(dst, dstBuf, cw, closure);

    dst += mul3(cw);
    src += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb24_span_composite_argb32_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t dstBuf[ADAPTOR_STEP * sizeof(uint32_t)];
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(dstBuf, dst, cw, closure);
    functionMap->convert.prgb32_from_argb32(srcBuf, src, cw, closure);
    f[Image::FormatRGB32].span_composite_a8[Image::FormatPRGB32](dstBuf, srcBuf, msk, cw, closure);
    functionMap->convert.rgb24_from_rgb32(dst, dstBuf, cw, closure);

    dst += mul3(cw);
    src += mul4(cw);
    msk += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb24_span_composite_prgb32_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t dstBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(dstBuf, dst, cw, closure);
    f[Image::FormatRGB32].span_composite_a8[Image::FormatPRGB32](dstBuf, src, msk, cw, closure);
    functionMap->convert.rgb24_from_rgb32(dst, dstBuf, cw, closure);

    dst += mul3(cw);
    src += mul4(cw);
    msk += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb24_span_composite_rgb32_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t dstBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(dstBuf, dst, cw, closure);
    f[Image::FormatRGB32].span_composite_a8[Image::FormatRGB32](dstBuf, src, msk, cw, closure);
    functionMap->convert.rgb24_from_rgb32(dst, dstBuf, cw, closure);

    dst += mul3(cw);
    src += mul4(cw);
    msk += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb24_span_composite_rgb24_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t dstBuf[ADAPTOR_STEP * sizeof(uint32_t)];
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(dstBuf, dst, cw, closure);
    functionMap->convert.rgb32_from_rgb24(srcBuf, src, cw, closure);
    f[Image::FormatRGB32].span_composite_a8[Image::FormatRGB32](dstBuf, srcBuf, msk, cw, closure);
    functionMap->convert.rgb24_from_rgb32(dst, dstBuf, cw, closure);

    dst += mul3(cw);
    src += mul3(cw);
    msk += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb24_span_composite_a8_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t dstBuf[ADAPTOR_STEP * sizeof(uint32_t)];
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(dstBuf, dst, cw, closure);
    functionMap->convert.axxx32_from_a8(srcBuf, src, cw, closure);
    f[Image::FormatRGB32].span_composite_a8[Image::FormatPRGB32](dstBuf, srcBuf, msk, cw, closure);
    functionMap->convert.rgb24_from_rgb32(dst, dstBuf, cw, closure);

    dst += mul3(cw);
    src += cw;
    msk += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb24_span_composite_i8_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t dstBuf[ADAPTOR_STEP * sizeof(uint32_t)];
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(dstBuf, dst, cw, closure);
    functionMap->convert.prgb32_from_i8(srcBuf, src, cw, closure);
    f[Image::FormatRGB32].span_composite_a8[Image::FormatPRGB32](dstBuf, srcBuf, msk, cw, closure);
    functionMap->convert.rgb24_from_rgb32(dst, dstBuf, cw, closure);

    dst += mul3(cw);
    src += cw;
    msk += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb24_span_composite_argb32_a8_const(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t dstBuf[ADAPTOR_STEP * sizeof(uint32_t)];
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(dstBuf, dst, cw, closure);
    functionMap->convert.prgb32_from_argb32(srcBuf, src, cw, closure);
    f[Image::FormatRGB32].span_composite_a8_const[Image::FormatPRGB32](dstBuf, srcBuf, msk0, cw, closure);
    functionMap->convert.rgb24_from_rgb32(dst, dstBuf, cw, closure);

    dst += mul3(cw);
    src += mul4(cw);
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb24_span_composite_prgb32_a8_const(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t dstBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(dstBuf, dst, cw, closure);
    f[Image::FormatRGB32].span_composite_a8_const[Image::FormatPRGB32](dstBuf, src, msk0, cw, closure);
    functionMap->convert.rgb24_from_rgb32(dst, dstBuf, cw, closure);

    dst += mul3(cw);
    src += mul4(cw);
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb24_span_composite_rgb32_a8_const(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t dstBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(dstBuf, dst, cw, closure);
    f[Image::FormatRGB32].span_composite_a8_const[Image::FormatRGB32](dstBuf, src, msk0, cw, closure);
    functionMap->convert.rgb24_from_rgb32(dst, dstBuf, cw, closure);

    dst += mul3(cw);
    src += mul4(cw);
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb24_span_composite_rgb24_a8_const(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t dstBuf[ADAPTOR_STEP * sizeof(uint32_t)];
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(dstBuf, dst, cw, closure);
    functionMap->convert.rgb32_from_rgb24(srcBuf, src, cw, closure);
    f[Image::FormatRGB32].span_composite_a8_const[Image::FormatRGB32](dstBuf, srcBuf, msk0, cw, closure);
    functionMap->convert.rgb24_from_rgb32(dst, dstBuf, cw, closure);

    dst += mul3(cw);
    src += mul3(cw);
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb24_span_composite_a8_a8_const(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t dstBuf[ADAPTOR_STEP * sizeof(uint32_t)];
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(dstBuf, dst, cw, closure);
    functionMap->convert.axxx32_from_a8(srcBuf, src, cw, closure);
    f[Image::FormatRGB32].span_composite_a8_const[Image::FormatPRGB32](dstBuf, srcBuf, msk0, cw, closure);
    functionMap->convert.rgb24_from_rgb32(dst, dstBuf, cw, closure);

    dst += mul4(cw);
    src += cw;
    w -= cw;
  } while (w);
}

static void FOG_FASTCALL adaptor_rgb24_span_composite_i8_a8_const(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  const FunctionMap::RasterFuncs* f = (const FunctionMap::RasterFuncs*)(closure->closure);
  uint8_t dstBuf[ADAPTOR_STEP * sizeof(uint32_t)];
  uint8_t srcBuf[ADAPTOR_STEP * sizeof(uint32_t)];

  do {
    sysint_t cw = Math::min(w, (sysint_t)ADAPTOR_STEP);
    functionMap->convert.rgb32_from_rgb24(dstBuf, dst, cw, closure);
    functionMap->convert.prgb32_from_i8(srcBuf, src, cw, closure);
    f[Image::FormatRGB32].span_composite_a8_const[Image::FormatPRGB32](dstBuf, srcBuf, msk0, cw, closure);
    functionMap->convert.rgb24_from_rgb32(dst, dstBuf, cw, closure);

    dst += mul3(cw);
    src += cw;
    w -= cw;
  } while (w);
}

// ============================================================================
// [Fog::Raster - Adaptor - A8]
// ============================================================================

#undef ADAPTOR_BUFFER_SIZE

} // Raster namespace
} // Fog namespace

// ============================================================================
// [Skeleton for Optimal Implementation]
// ============================================================================

/*
// ============================================================================
// [Fog::Raster - Raster - Operator - Argb32]
// ============================================================================

static void FOG_FASTCALL adaptor_argb32_pixel_Operator_Implementation(
  uint8_t* dst, const Solid* src, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_argb32_pixel_a8_Operator_Implementation(
  uint8_t* dst, const Solid* src, uint32_t msk0, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_argb32_span_solid_Operator_Implementation(
  uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_argb32_span_solid_a8_Operator_Implementation(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_argb32_span_solid_a8_const_Operator_Implementation(
  uint8_t* dst, const Solid* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_argb32_span_composite_argb32_Operator_Implementation(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_argb32_span_composite_prgb32_Operator_Implementation(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_argb32_span_composite_rgb32_Operator_Implementation(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_argb32_span_composite_argb32_a8_Operator_Implementation(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_argb32_span_composite_prgb32_a8_Operator_Implementation(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_argb32_span_composite_rgb32_a8_Operator_Implementation(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_argb32_span_composite_argb32_a8_const_Operator_Implementation(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_argb32_span_composite_prgb32_a8_const_Operator_Implementation(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_argb32_span_composite_rgb32_a8_const_Operator_Implementation(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
}

// ============================================================================
// [Fog::Raster - Raster - Operator - Prgb32]
// ============================================================================

static void FOG_FASTCALL adaptor_prgb32_pixel_Operator_Implementation(
  uint8_t* dst, const Solid* src, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_prgb32_pixel_a8_Operator_Implementation(
  uint8_t* dst, const Solid* src, uint32_t msk0, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_prgb32_span_solid_Operator_Implementation(
  uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_prgb32_span_solid_a8_Operator_Implementation(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_prgb32_span_solid_a8_const_Operator_Implementation(
  uint8_t* dst, const Solid* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_prgb32_span_composite_argb32_Operator_Implementation(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_prgb32_span_composite_prgb32_Operator_Implementation(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_prgb32_span_composite_rgb32_Operator_Implementation(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_prgb32_span_composite_argb32_a8_Operator_Implementation(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_prgb32_span_composite_prgb32_a8_Operator_Implementation(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_prgb32_span_composite_rgb32_a8_Operator_Implementation(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_prgb32_span_composite_argb32_a8_const_Operator_Implementation(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_prgb32_span_composite_prgb32_a8_const_Operator_Implementation(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_prgb32_span_composite_rgb32_a8_const_Operator_Implementation(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
}

// ============================================================================
// [Fog::Raster - Raster - Operator - Rgb32]
// ============================================================================

static void FOG_FASTCALL adaptor_rgb32_pixel_Operator_Implementation(
  uint8_t* dst, const Solid* src, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_rgb32_pixel_a8_Operator_Implementation(
  uint8_t* dst, const Solid* src, uint32_t msk0, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_rgb32_span_solid_Operator_Implementation(
  uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_rgb32_span_solid_a8_Operator_Implementation(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_rgb32_span_solid_a8_const_Operator_Implementation(
  uint8_t* dst, const Solid* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_rgb32_span_composite_argb32_Operator_Implementation(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_rgb32_span_composite_prgb32_Operator_Implementation(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_rgb32_span_composite_rgb32_Operator_Implementation(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_rgb32_span_composite_argb32_a8_Operator_Implementation(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_rgb32_span_composite_prgb32_a8_Operator_Implementation(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_rgb32_span_composite_rgb32_a8_Operator_Implementation(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_rgb32_span_composite_argb32_a8_const_Operator_Implementation(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_rgb32_span_composite_prgb32_a8_const_Operator_Implementation(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL adaptor_rgb32_span_composite_rgb32_a8_const_Operator_Implementation(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
}

// ============================================================================
// [Function Map]
// ============================================================================

// [Raster - Operator]

m->raster[Operator][Image::FormatARGB32].pixel = raster_argb32_pixel_Operator_Implementation;
m->raster[Operator][Image::FormatARGB32].pixel_a8 = raster_argb32_pixel_a8_Operator_Implementation;
m->raster[Operator][Image::FormatARGB32].span_solid = raster_argb32_span_solid_Operator_Implementation;
m->raster[Operator][Image::FormatARGB32].span_solid_a8 = raster_argb32_span_solid_a8_Operator_Implementation;
m->raster[Operator][Image::FormatARGB32].span_solid_a8_const = raster_argb32_span_solid_a8_const_Operator_Implementation;

m->raster[Operator][Image::FormatARGB32].span_composite[Image::FormatARGB32] = raster_argb32_span_composite_argb32_Operator_Implementation;
m->raster[Operator][Image::FormatARGB32].span_composite[Image::FormatPRGB32] = raster_argb32_span_composite_prgb32_Operator_Implementation;
m->raster[Operator][Image::FormatARGB32].span_composite[Image::FormatRGB32] = raster_argb32_span_composite_rgb32_Operator_Implementation;

m->raster[Operator][Image::FormatARGB32].span_composite_a8[Image::FormatARGB32] = raster_argb32_span_composite_argb32_a8_Operator_Implementation;
m->raster[Operator][Image::FormatARGB32].span_composite_a8[Image::FormatPRGB32] = raster_argb32_span_composite_prgb32_a8_Operator_Implementation;
m->raster[Operator][Image::FormatARGB32].span_composite_a8[Image::FormatRGB32] = raster_argb32_span_composite_rgb32_a8_Operator_Implementation;

m->raster[Operator][Image::FormatARGB32].span_composite_a8_const[Image::FormatARGB32] = raster_argb32_span_composite_argb32_a8_const_Operator_Implementation;
m->raster[Operator][Image::FormatARGB32].span_composite_a8_const[Image::FormatPRGB32] = raster_argb32_span_composite_prgb32_a8_const_Operator_Implementation;
m->raster[Operator][Image::FormatARGB32].span_composite_a8_const[Image::FormatRGB32] = raster_argb32_span_composite_rgb32_a8_const_Operator_Implementation;

m->raster[Operator][Image::FormatPRGB32].pixel = raster_prgb32_pixel_Operator_Implementation;
m->raster[Operator][Image::FormatPRGB32].pixel_a8 = raster_prgb32_pixel_a8_Operator_Implementation;
m->raster[Operator][Image::FormatPRGB32].span_solid = raster_prgb32_span_solid_Operator_Implementation;
m->raster[Operator][Image::FormatPRGB32].span_solid_a8 = raster_prgb32_span_solid_a8_Operator_Implementation;
m->raster[Operator][Image::FormatPRGB32].span_solid_a8_const = raster_prgb32_span_solid_a8_const_Operator_Implementation;

m->raster[Operator][Image::FormatPRGB32].span_composite[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_Operator_Implementation;
m->raster[Operator][Image::FormatPRGB32].span_composite[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_Operator_Implementation;
m->raster[Operator][Image::FormatPRGB32].span_composite[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_Operator_Implementation;

m->raster[Operator][Image::FormatPRGB32].span_composite_a8[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_Operator_Implementation;
m->raster[Operator][Image::FormatPRGB32].span_composite_a8[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_Operator_Implementation;
m->raster[Operator][Image::FormatPRGB32].span_composite_a8[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_Operator_Implementation;

m->raster[Operator][Image::FormatPRGB32].span_composite_a8_const[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_const_Operator_Implementation;
m->raster[Operator][Image::FormatPRGB32].span_composite_a8_const[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_const_Operator_Implementation;
m->raster[Operator][Image::FormatPRGB32].span_composite_a8_const[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_const_Operator_Implementation;

m->raster[Operator][Image::FormatRGB32].pixel = raster_rgb32_pixel_Operator_Implementation;
m->raster[Operator][Image::FormatRGB32].pixel_a8 = raster_rgb32_pixel_a8_Operator_Implementation;
m->raster[Operator][Image::FormatRGB32].span_solid = raster_rgb32_span_solid_Operator_Implementation;
m->raster[Operator][Image::FormatRGB32].span_solid_a8 = raster_rgb32_span_solid_a8_Operator_Implementation;
m->raster[Operator][Image::FormatRGB32].span_solid_a8_const = raster_rgb32_span_solid_a8_const_Operator_Implementation;

m->raster[Operator][Image::FormatRGB32].span_composite[Image::FormatARGB32] = raster_rgb32_span_composite_argb32_Operator_Implementation;
m->raster[Operator][Image::FormatRGB32].span_composite[Image::FormatPRGB32] = raster_rgb32_span_composite_prgb32_Operator_Implementation;
m->raster[Operator][Image::FormatRGB32].span_composite[Image::FormatRGB32] = raster_rgb32_span_composite_rgb32_Operator_Implementation;

m->raster[Operator][Image::FormatRGB32].span_composite_a8[Image::FormatARGB32] = raster_rgb32_span_composite_argb32_a8_Operator_Implementation;
m->raster[Operator][Image::FormatRGB32].span_composite_a8[Image::FormatPRGB32] = raster_rgb32_span_composite_prgb32_a8_Operator_Implementation;
m->raster[Operator][Image::FormatRGB32].span_composite_a8[Image::FormatRGB32] = raster_rgb32_span_composite_rgb32_a8_Operator_Implementation;

m->raster[Operator][Image::FormatRGB32].span_composite_a8_const[Image::FormatARGB32] = raster_rgb32_span_composite_argb32_a8_const_Operator_Implementation;
m->raster[Operator][Image::FormatRGB32].span_composite_a8_const[Image::FormatPRGB32] = raster_rgb32_span_composite_prgb32_a8_const_Operator_Implementation;
m->raster[Operator][Image::FormatRGB32].span_composite_a8_const[Image::FormatRGB32] = raster_rgb32_span_composite_rgb32_a8_const_Operator_Implementation;
*/
