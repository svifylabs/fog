// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/Math.h>
#include <Fog/Core/Misc.h>
#include <Fog/Graphics/Argb.h>
#include <Fog/Graphics/ArgbUtil.h>
#include <Fog/Graphics/ColorLut.h>
#include <Fog/Graphics/ColorMatrix.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/DitherMatrix.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/ImageFilter.h>
#include <Fog/Graphics/Matrix.h>
#include <Fog/Graphics/Path.h>
#include <Fog/Graphics/Pattern.h>
#include <Fog/Graphics/RasterUtil.h>
#include <Fog/Graphics/ArgbAnalyzer.h>

// [Raster_C]
#include <Fog/Graphics/RasterUtil/RasterUtil_C.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Defs_C.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Composite_C.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Dib_C.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Interpolate_C.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Pattern_C.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Scale_C.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Filters_C.h>

// ============================================================================
// [Library Initializers]
// ============================================================================

static void fog_raster_set_nops(Fog::RasterUtil::FunctionMap::RasterFuncs* ops)
{
  using namespace Fog;
  using namespace Fog::RasterUtil;

  ops->cspan = CompositeNopC::cspan;
  ops->cspan_a8 = CompositeNopC::cspan_a8;
  ops->cspan_a8_const = CompositeNopC::cspan_a8_const;

  ops->vspan[PIXEL_FORMAT_ARGB32] = (VSpanFn)CompositeNopC::cspan;
  ops->vspan[PIXEL_FORMAT_PRGB32] = (VSpanFn)CompositeNopC::cspan;
  ops->vspan[PIXEL_FORMAT_XRGB32] = (VSpanFn)CompositeNopC::cspan;
  ops->vspan[PIXEL_FORMAT_A8] = (VSpanFn)CompositeNopC::cspan;
  ops->vspan[PIXEL_FORMAT_I8] = (VSpanFn)CompositeNopC::cspan;

  ops->vspan_a8[PIXEL_FORMAT_ARGB32] = (VSpanMskFn)CompositeNopC::cspan_a8;
  ops->vspan_a8[PIXEL_FORMAT_PRGB32] = (VSpanMskFn)CompositeNopC::cspan_a8;
  ops->vspan_a8[PIXEL_FORMAT_XRGB32] = (VSpanMskFn)CompositeNopC::cspan_a8;
  ops->vspan_a8[PIXEL_FORMAT_A8] = (VSpanMskFn)CompositeNopC::cspan_a8;
  ops->vspan_a8[PIXEL_FORMAT_I8] = (VSpanMskFn)CompositeNopC::cspan_a8;

  ops->vspan_a8_const[PIXEL_FORMAT_ARGB32] = (VSpanMskConstFn)CompositeNopC::cspan_a8_const;
  ops->vspan_a8_const[PIXEL_FORMAT_PRGB32] = (VSpanMskConstFn)CompositeNopC::cspan_a8_const;
  ops->vspan_a8_const[PIXEL_FORMAT_XRGB32] = (VSpanMskConstFn)CompositeNopC::cspan_a8_const;
  ops->vspan_a8_const[PIXEL_FORMAT_A8] = (VSpanMskConstFn)CompositeNopC::cspan_a8_const;
  ops->vspan_a8_const[PIXEL_FORMAT_I8] = (VSpanMskConstFn)CompositeNopC::cspan_a8_const;
}

FOG_INIT_DECLARE void fog_raster_init_c(void)
{
  using namespace Fog;
  using namespace Fog::RasterUtil;

  FunctionMap* m = functionMap;

  // [Dib - ByteSwap]

  m->dib.bswap16 = DibC::bswap16;
  m->dib.bswap24 = DibC::bswap24;
  m->dib.bswap32 = DibC::bswap32;

  // [Dib - MemCpy]

  m->dib.memcpy8 = DibC::memcpy8;
  m->dib.memcpy16 = DibC::memcpy16;
  m->dib.memcpy24 = DibC::memcpy24;
  m->dib.memcpy32 = DibC::memcpy32;

  // [Dib - Convert]

  m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_PRGB32_NATIVE    ] = DibC::memcpy32;
  m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_ARGB32_NATIVE    ] = DibC::prgb32_from_argb32;
  m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_XRGB32_NATIVE    ] = DibC::frgb32_from_xrgb32;
  m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_A8               ] = DibC::azzz32_from_a8;
  m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_I8               ] = DibC::prgb32_from_i8;
  m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_PRGB32_SWAPPED   ] = DibC::bswap32;
  m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_ARGB32_SWAPPED   ] = DibC::prgb32_from_argb32_swapped;
  m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_XRGB32_SWAPPED   ] = DibC::frgb32_from_xrgb32_swapped;
  m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_RGB24_NATIVE     ] = DibC::frgb32_from_rgb24_native;
  m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_RGB24_SWAPPED    ] = DibC::frgb32_from_rgb24_swapped;
  m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_RGB16_565_NATIVE ] = DibC::frgb32_from_rgb16_565_native;
  m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_RGB16_565_SWAPPED] = DibC::frgb32_from_rgb16_565_swapped;
  m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_RGB16_555_NATIVE ] = DibC::frgb32_from_rgb16_555_native;
  m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_RGB16_555_SWAPPED] = DibC::frgb32_from_rgb16_555_swapped;
  m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_GREY8            ] = DibC::frgb32_from_grey8;

  m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_PRGB32_NATIVE    ] = DibC::argb32_from_prgb32;
  m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_ARGB32_NATIVE    ] = DibC::memcpy32;
  m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_XRGB32_NATIVE    ] = DibC::frgb32_from_xrgb32;
  m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_A8               ] = DibC::azzz32_from_a8;
  m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_I8               ] = DibC::argb32_from_i8;
  m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_PRGB32_SWAPPED   ] = DibC::argb32_from_prgb32_swapped;
  m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_ARGB32_SWAPPED   ] = DibC::bswap32;
  m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_XRGB32_SWAPPED   ] = DibC::frgb32_from_xrgb32_swapped;
  m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_RGB24_NATIVE     ] = DibC::frgb32_from_rgb24_native;
  m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_RGB24_SWAPPED    ] = DibC::frgb32_from_rgb24_swapped;
  m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_RGB16_565_NATIVE ] = DibC::frgb32_from_rgb16_565_native;
  m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_RGB16_565_SWAPPED] = DibC::frgb32_from_rgb16_565_swapped;
  m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_RGB16_555_NATIVE ] = DibC::frgb32_from_rgb16_555_native;
  m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_RGB16_555_SWAPPED] = DibC::frgb32_from_rgb16_555_swapped;
  m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_GREY8            ] = DibC::frgb32_from_grey8;

  m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_PRGB32_NATIVE    ] = DibC::frgb32_from_xrgb32;
  m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_ARGB32_NATIVE    ] = DibC::frgb32_from_argb32;
  m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_XRGB32_NATIVE    ] = DibC::frgb32_from_xrgb32;
  m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_A8               ] = DibC::fzzz32_from_null;
  m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_I8               ] = DibC::frgb32_from_i8;
  m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_PRGB32_SWAPPED   ] = DibC::frgb32_from_xrgb32_swapped;
  m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_ARGB32_SWAPPED   ] = DibC::frgb32_from_argb32_swapped;
  m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_XRGB32_SWAPPED   ] = DibC::bswap32;
  m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_RGB24_NATIVE     ] = DibC::frgb32_from_rgb24_native;
  m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_RGB24_SWAPPED    ] = DibC::frgb32_from_rgb24_swapped;
  m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_RGB16_565_NATIVE ] = DibC::frgb32_from_rgb16_565_native;
  m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_RGB16_565_SWAPPED] = DibC::frgb32_from_rgb16_565_swapped;
  m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_RGB16_555_NATIVE ] = DibC::frgb32_from_rgb16_555_native;
  m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_RGB16_555_SWAPPED] = DibC::frgb32_from_rgb16_555_swapped;
  m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_GREY8            ] = DibC::frgb32_from_grey8;

  m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_PRGB32_NATIVE    ] = DibC::a8_from_axxx32;
  m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_ARGB32_NATIVE    ] = DibC::a8_from_axxx32;
  m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_XRGB32_NATIVE    ] = DibC::f8_from_null;
  m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_A8               ] = DibC::memcpy8;
  m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_I8               ] = DibC::a8_from_i8;
  m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_PRGB32_SWAPPED   ] = DibC::a8_from_axxx32_swapped;
  m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_ARGB32_SWAPPED   ] = DibC::a8_from_axxx32_swapped;
  m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_XRGB32_SWAPPED   ] = DibC::f8_from_null;
  m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_RGB24_NATIVE     ] = DibC::f8_from_null;
  m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_RGB24_SWAPPED    ] = DibC::f8_from_null;
  m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_RGB16_565_NATIVE ] = DibC::f8_from_null;
  m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_RGB16_565_SWAPPED] = DibC::f8_from_null;
  m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_RGB16_555_NATIVE ] = DibC::f8_from_null;
  m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_RGB16_555_SWAPPED] = DibC::f8_from_null;
  m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_GREY8            ] = DibC::f8_from_null;

  m->dib.convert[DIB_FORMAT_PRGB32_SWAPPED   ][PIXEL_FORMAT_PRGB32] = DibC::bswap32;
  m->dib.convert[DIB_FORMAT_PRGB32_SWAPPED   ][PIXEL_FORMAT_ARGB32] = DibC::prgb32_swapped_from_argb32;
  m->dib.convert[DIB_FORMAT_PRGB32_SWAPPED   ][PIXEL_FORMAT_XRGB32] = DibC::frgb32_swapped_from_xrgb32;
  m->dib.convert[DIB_FORMAT_PRGB32_SWAPPED   ][PIXEL_FORMAT_A8    ] = DibC::azzz32_swapped_from_a8;
  m->dib.convert[DIB_FORMAT_PRGB32_SWAPPED   ][PIXEL_FORMAT_I8    ] = DibC::prgb32_swapped_from_i8;

  m->dib.convert[DIB_FORMAT_ARGB32_SWAPPED   ][PIXEL_FORMAT_PRGB32] = DibC::argb32_swapped_from_prgb32;
  m->dib.convert[DIB_FORMAT_ARGB32_SWAPPED   ][PIXEL_FORMAT_ARGB32] = DibC::bswap32;
  m->dib.convert[DIB_FORMAT_ARGB32_SWAPPED   ][PIXEL_FORMAT_XRGB32] = DibC::frgb32_swapped_from_xrgb32;
  m->dib.convert[DIB_FORMAT_ARGB32_SWAPPED   ][PIXEL_FORMAT_A8    ] = DibC::azzz32_swapped_from_a8;
  m->dib.convert[DIB_FORMAT_ARGB32_SWAPPED   ][PIXEL_FORMAT_I8    ] = DibC::argb32_swapped_from_i8;

  m->dib.convert[DIB_FORMAT_XRGB32_SWAPPED   ][PIXEL_FORMAT_PRGB32] = DibC::frgb32_swapped_from_xrgb32;
  m->dib.convert[DIB_FORMAT_XRGB32_SWAPPED   ][PIXEL_FORMAT_ARGB32] = DibC::frgb32_swapped_from_argb32;
  m->dib.convert[DIB_FORMAT_XRGB32_SWAPPED   ][PIXEL_FORMAT_XRGB32] = DibC::frgb32_swapped_from_xrgb32;
  m->dib.convert[DIB_FORMAT_XRGB32_SWAPPED   ][PIXEL_FORMAT_A8    ] = DibC::fzzz32_swapped_from_null;
  m->dib.convert[DIB_FORMAT_XRGB32_SWAPPED   ][PIXEL_FORMAT_I8    ] = DibC::frgb32_swapped_from_i8;

  m->dib.convert[DIB_FORMAT_RGB24_NATIVE     ][PIXEL_FORMAT_PRGB32] = DibC::rgb24_native_from_xrgb32;
  m->dib.convert[DIB_FORMAT_RGB24_NATIVE     ][PIXEL_FORMAT_ARGB32] = DibC::rgb24_native_from_argb32;
  m->dib.convert[DIB_FORMAT_RGB24_NATIVE     ][PIXEL_FORMAT_XRGB32] = DibC::rgb24_native_from_xrgb32;
  m->dib.convert[DIB_FORMAT_RGB24_NATIVE     ][PIXEL_FORMAT_A8    ] = DibC::zzz24_from_null;
  m->dib.convert[DIB_FORMAT_RGB24_NATIVE     ][PIXEL_FORMAT_I8    ] = DibC::rgb24_native_from_i8;

  m->dib.convert[DIB_FORMAT_RGB24_SWAPPED    ][PIXEL_FORMAT_PRGB32] = DibC::rgb24_swapped_from_xrgb32;
  m->dib.convert[DIB_FORMAT_RGB24_SWAPPED    ][PIXEL_FORMAT_ARGB32] = DibC::rgb24_swapped_from_argb32;
  m->dib.convert[DIB_FORMAT_RGB24_SWAPPED    ][PIXEL_FORMAT_XRGB32] = DibC::rgb24_swapped_from_xrgb32;
  m->dib.convert[DIB_FORMAT_RGB24_SWAPPED    ][PIXEL_FORMAT_A8    ] = DibC::zzz24_from_null;
  m->dib.convert[DIB_FORMAT_RGB24_SWAPPED    ][PIXEL_FORMAT_I8    ] = DibC::rgb24_swapped_from_i8;

  m->dib.convert[DIB_FORMAT_RGB16_565_NATIVE ][PIXEL_FORMAT_PRGB32] = DibC::rgb16_565_native_from_xrgb32;
  m->dib.convert[DIB_FORMAT_RGB16_565_NATIVE ][PIXEL_FORMAT_ARGB32] = DibC::rgb16_565_native_from_argb32;
  m->dib.convert[DIB_FORMAT_RGB16_565_NATIVE ][PIXEL_FORMAT_XRGB32] = DibC::rgb16_565_native_from_xrgb32;
  m->dib.convert[DIB_FORMAT_RGB16_565_NATIVE ][PIXEL_FORMAT_A8    ] = DibC::zzz16_from_null;
  m->dib.convert[DIB_FORMAT_RGB16_565_NATIVE ][PIXEL_FORMAT_I8    ] = DibC::rgb16_565_native_from_i8;

  m->dib.convert[DIB_FORMAT_RGB16_565_SWAPPED][PIXEL_FORMAT_PRGB32] = DibC::rgb16_565_swapped_from_xrgb32;
  m->dib.convert[DIB_FORMAT_RGB16_565_SWAPPED][PIXEL_FORMAT_ARGB32] = DibC::rgb16_565_swapped_from_argb32;
  m->dib.convert[DIB_FORMAT_RGB16_565_SWAPPED][PIXEL_FORMAT_XRGB32] = DibC::rgb16_565_swapped_from_xrgb32;
  m->dib.convert[DIB_FORMAT_RGB16_565_SWAPPED][PIXEL_FORMAT_A8    ] = DibC::zzz16_from_null;
  m->dib.convert[DIB_FORMAT_RGB16_565_SWAPPED][PIXEL_FORMAT_I8    ] = DibC::rgb16_565_swapped_from_i8;

  m->dib.convert[DIB_FORMAT_RGB16_555_NATIVE ][PIXEL_FORMAT_PRGB32] = DibC::rgb16_555_native_from_xrgb32;
  m->dib.convert[DIB_FORMAT_RGB16_555_NATIVE ][PIXEL_FORMAT_ARGB32] = DibC::rgb16_555_native_from_argb32;
  m->dib.convert[DIB_FORMAT_RGB16_555_NATIVE ][PIXEL_FORMAT_XRGB32] = DibC::rgb16_555_native_from_xrgb32;
  m->dib.convert[DIB_FORMAT_RGB16_555_NATIVE ][PIXEL_FORMAT_A8    ] = DibC::zzz16_from_null;
  m->dib.convert[DIB_FORMAT_RGB16_555_NATIVE ][PIXEL_FORMAT_I8    ] = DibC::rgb16_555_native_from_i8;

  m->dib.convert[DIB_FORMAT_RGB16_555_SWAPPED][PIXEL_FORMAT_PRGB32] = DibC::rgb16_555_swapped_from_xrgb32;
  m->dib.convert[DIB_FORMAT_RGB16_555_SWAPPED][PIXEL_FORMAT_ARGB32] = DibC::rgb16_555_swapped_from_argb32;
  m->dib.convert[DIB_FORMAT_RGB16_555_SWAPPED][PIXEL_FORMAT_XRGB32] = DibC::rgb16_555_swapped_from_xrgb32;
  m->dib.convert[DIB_FORMAT_RGB16_555_SWAPPED][PIXEL_FORMAT_A8    ] = DibC::zzz16_from_null;
  m->dib.convert[DIB_FORMAT_RGB16_555_SWAPPED][PIXEL_FORMAT_I8    ] = DibC::rgb16_555_swapped_from_i8;

  m->dib.convert[DIB_FORMAT_GREY8            ][PIXEL_FORMAT_PRGB32] = DibC::grey8_from_xrgb32;
  m->dib.convert[DIB_FORMAT_GREY8            ][PIXEL_FORMAT_ARGB32] = DibC::grey8_from_argb32;
  m->dib.convert[DIB_FORMAT_GREY8            ][PIXEL_FORMAT_XRGB32] = DibC::grey8_from_xrgb32;
  m->dib.convert[DIB_FORMAT_GREY8            ][PIXEL_FORMAT_A8    ] = DibC::z8_from_null;
  m->dib.convert[DIB_FORMAT_GREY8            ][PIXEL_FORMAT_I8    ] = DibC::grey8_from_i8;

  // [Dib - Dither]

  m->dib.i8rgb232_from_xrgb32_dither = DibC::i8rgb232_from_xrgb32_dither;
  m->dib.i8rgb222_from_xrgb32_dither = DibC::i8rgb222_from_xrgb32_dither;
  m->dib.i8rgb111_from_xrgb32_dither = DibC::i8rgb111_from_xrgb32_dither;

  m->dib.rgb16_555_native_from_xrgb32_dither = DibC::rgb16_555_native_from_xrgb32_dither;
  m->dib.rgb16_565_native_from_xrgb32_dither = DibC::rgb16_565_native_from_xrgb32_dither;

  m->dib.rgb16_555_swapped_from_xrgb32_dither = DibC::rgb16_555_swapped_from_xrgb32_dither;
  m->dib.rgb16_565_swapped_from_xrgb32_dither = DibC::rgb16_565_swapped_from_xrgb32_dither;

  // [Gradient - Interpolation]

  m->interpolate.gradient[PIXEL_FORMAT_PRGB32] = InterpolateC::gradient_prgb32;
  m->interpolate.gradient[PIXEL_FORMAT_ARGB32] = InterpolateC::gradient_argb32;
  m->interpolate.gradient[PIXEL_FORMAT_XRGB32] = InterpolateC::gradient_xrgb32;
  m->interpolate.gradient[PIXEL_FORMAT_A8] = InterpolateC::gradient_a8;

  // [Pattern - Solid]

  m->pattern.solid_init = PatternC::solid_init;
  m->pattern.solid_fetch = PatternC::solid_fetch;

  // [Pattern - Texture]

  m->pattern.texture_init = PatternC::texture_init;
  m->pattern.texture_fetch_exact_repeat[PIXEL_FORMAT_PRGB32] = PatternC::texture_fetch_exact_repeat_32;
  m->pattern.texture_fetch_exact_repeat[PIXEL_FORMAT_ARGB32] = PatternC::texture_fetch_exact_repeat_32;
  m->pattern.texture_fetch_exact_repeat[PIXEL_FORMAT_XRGB32] = PatternC::texture_fetch_exact_repeat_32;

  m->pattern.texture_fetch_exact_reflect[PIXEL_FORMAT_PRGB32] = PatternC::texture_fetch_exact_reflect_32;
  m->pattern.texture_fetch_exact_reflect[PIXEL_FORMAT_ARGB32] = PatternC::texture_fetch_exact_reflect_32;
  m->pattern.texture_fetch_exact_reflect[PIXEL_FORMAT_XRGB32] = PatternC::texture_fetch_exact_reflect_32;

  m->pattern.texture_fetch_subx0_repeat[PIXEL_FORMAT_PRGB32] = PatternC::texture_fetch_subx0_repeat_32;
  m->pattern.texture_fetch_subx0_repeat[PIXEL_FORMAT_ARGB32] = PatternC::texture_fetch_subx0_repeat_32;
  m->pattern.texture_fetch_subx0_repeat[PIXEL_FORMAT_XRGB32] = PatternC::texture_fetch_subx0_repeat_32;

  m->pattern.texture_fetch_subx0_reflect[PIXEL_FORMAT_PRGB32] = PatternC::texture_fetch_subx0_reflect_32;
  m->pattern.texture_fetch_subx0_reflect[PIXEL_FORMAT_ARGB32] = PatternC::texture_fetch_subx0_reflect_32;
  m->pattern.texture_fetch_subx0_reflect[PIXEL_FORMAT_XRGB32] = PatternC::texture_fetch_subx0_reflect_32;

  m->pattern.texture_fetch_sub0y_repeat[PIXEL_FORMAT_PRGB32] = PatternC::texture_fetch_sub0y_repeat_32;
  m->pattern.texture_fetch_sub0y_repeat[PIXEL_FORMAT_ARGB32] = PatternC::texture_fetch_sub0y_repeat_32;
  m->pattern.texture_fetch_sub0y_repeat[PIXEL_FORMAT_XRGB32] = PatternC::texture_fetch_sub0y_repeat_32;

  m->pattern.texture_fetch_sub0y_reflect[PIXEL_FORMAT_PRGB32] = PatternC::texture_fetch_sub0y_reflect_32;
  m->pattern.texture_fetch_sub0y_reflect[PIXEL_FORMAT_ARGB32] = PatternC::texture_fetch_sub0y_reflect_32;
  m->pattern.texture_fetch_sub0y_reflect[PIXEL_FORMAT_XRGB32] = PatternC::texture_fetch_sub0y_reflect_32;

  m->pattern.texture_fetch_subxy_repeat[PIXEL_FORMAT_PRGB32] = PatternC::texture_fetch_subxy_repeat_32;
  m->pattern.texture_fetch_subxy_repeat[PIXEL_FORMAT_ARGB32] = PatternC::texture_fetch_subxy_repeat_32;
  m->pattern.texture_fetch_subxy_repeat[PIXEL_FORMAT_XRGB32] = PatternC::texture_fetch_subxy_repeat_32;

  m->pattern.texture_fetch_subxy_reflect[PIXEL_FORMAT_PRGB32] = PatternC::texture_fetch_subxy_reflect_32;
  m->pattern.texture_fetch_subxy_reflect[PIXEL_FORMAT_ARGB32] = PatternC::texture_fetch_subxy_reflect_32;
  m->pattern.texture_fetch_subxy_reflect[PIXEL_FORMAT_XRGB32] = PatternC::texture_fetch_subxy_reflect_32;

  m->pattern.texture_fetch_transform_nearest_repeat[PIXEL_FORMAT_PRGB32] = PatternC::texture_fetch_transform_nearest_repeat_32;
  m->pattern.texture_fetch_transform_nearest_repeat[PIXEL_FORMAT_ARGB32] = PatternC::texture_fetch_transform_nearest_repeat_32;
  m->pattern.texture_fetch_transform_nearest_repeat[PIXEL_FORMAT_XRGB32] = PatternC::texture_fetch_transform_nearest_repeat_32;
  m->pattern.texture_fetch_transform_nearest_repeat[PIXEL_FORMAT_A8] = PatternC::texture_fetch_transform_nearest_repeat_a8;

  m->pattern.texture_fetch_transform_bilinear_repeat[PIXEL_FORMAT_PRGB32] = PatternC::texture_fetch_transform_bilinear_repeat_32;
  m->pattern.texture_fetch_transform_bilinear_repeat[PIXEL_FORMAT_ARGB32] = PatternC::texture_fetch_transform_bilinear_repeat_32;
  m->pattern.texture_fetch_transform_bilinear_repeat[PIXEL_FORMAT_XRGB32] = PatternC::texture_fetch_transform_bilinear_repeat_32;
  m->pattern.texture_fetch_transform_bilinear_repeat[PIXEL_FORMAT_A8] = PatternC::texture_fetch_transform_bilinear_repeat_a8;

  // [Pattern - Linear Gradient]

  m->pattern.linear_gradient_init = PatternC::linear_gradient_init;
  m->pattern.linear_gradient_fetch_exact_pad = PatternC::linear_gradient_fetch_exact_pad;
  m->pattern.linear_gradient_fetch_exact_repeat = PatternC::linear_gradient_fetch_exact_repeat;
  m->pattern.linear_gradient_fetch_subxy_pad = PatternC::linear_gradient_fetch_subxy_pad;
  m->pattern.linear_gradient_fetch_subxy_repeat = PatternC::linear_gradient_fetch_subxy_repeat;

  // [Pattern - Radial Gradient]

  m->pattern.radial_gradient_init = PatternC::radial_gradient_init;
  m->pattern.radial_gradient_fetch_pad = PatternC::radial_gradient_fetch_pad;
  m->pattern.radial_gradient_fetch_repeat = PatternC::radial_gradient_fetch_repeat;

  // [Pattern - Conical Gradient]

  m->pattern.conical_gradient_init = PatternC::conical_gradient_init;
  m->pattern.conical_gradient_fetch = PatternC::conical_gradient_fetch;

  // [Scale]

  m->pattern.scale_init = ScaleC::scale_init;
  //m->pattern.scale_fetch = ScaleC::scale_fetch;

  // [Filter - ColorLut]

  m->filter.color_lut[PIXEL_FORMAT_PRGB32] = FilterC::color_lut_prgb32;
  m->filter.color_lut[PIXEL_FORMAT_ARGB32] = FilterC::color_lut_argb32;
  m->filter.color_lut[PIXEL_FORMAT_XRGB32] = FilterC::color_lut_xrgb32;
  m->filter.color_lut[PIXEL_FORMAT_A8] = FilterC::color_lut_a8;

  // [Filter - ColorMatrix]

  m->filter.color_matrix[PIXEL_FORMAT_PRGB32] = FilterC::color_matrix_prgb32;
  m->filter.color_matrix[PIXEL_FORMAT_ARGB32] = FilterC::color_matrix_argb32;
  m->filter.color_matrix[PIXEL_FORMAT_XRGB32] = FilterC::color_matrix_xrgb32;
  m->filter.color_matrix[PIXEL_FORMAT_A8] = FilterC::color_matrix_a8;

  // [Filters - CopyArea]

  m->filter.copy_area[PIXEL_FORMAT_PRGB32] = FilterC::copy_area_32;
  m->filter.copy_area[PIXEL_FORMAT_ARGB32] = FilterC::copy_area_32;
  m->filter.copy_area[PIXEL_FORMAT_XRGB32] = FilterC::copy_area_32;
  m->filter.copy_area[PIXEL_FORMAT_A8] = FilterC::copy_area_8;
  m->filter.copy_area[PIXEL_FORMAT_I8] = FilterC::copy_area_8;

  // [Filters - BoxBlur]

  m->filter.box_blur_h[PIXEL_FORMAT_ARGB32] = FilterC::box_blur_h_argb32;
  m->filter.box_blur_v[PIXEL_FORMAT_ARGB32] = FilterC::box_blur_v_argb32;
  m->filter.box_blur_h[PIXEL_FORMAT_XRGB32] = FilterC::box_blur_h_xrgb32;
  m->filter.box_blur_v[PIXEL_FORMAT_XRGB32] = FilterC::box_blur_v_xrgb32;
  m->filter.box_blur_h[PIXEL_FORMAT_A8] = FilterC::box_blur_h_a8;
  m->filter.box_blur_v[PIXEL_FORMAT_A8] = FilterC::box_blur_v_a8;

  // [Filters - LinearBlur]

  m->filter.linear_blur_h[PIXEL_FORMAT_ARGB32] = FilterC::linear_blur_h_argb32;
  m->filter.linear_blur_v[PIXEL_FORMAT_ARGB32] = FilterC::linear_blur_v_argb32;
  m->filter.linear_blur_h[PIXEL_FORMAT_XRGB32] = FilterC::linear_blur_h_argb32;
  m->filter.linear_blur_v[PIXEL_FORMAT_XRGB32] = FilterC::linear_blur_v_argb32;
  m->filter.linear_blur_h[PIXEL_FORMAT_A8] = FilterC::linear_blur_h_a8;
  m->filter.linear_blur_v[PIXEL_FORMAT_A8] = FilterC::linear_blur_v_a8;

  // [Filters - SymmetricConvolveFloat]

  m->filter.symmetric_convolve_float_h[PIXEL_FORMAT_ARGB32] = FilterC::symmetric_convolve_float_h_argb32;
  m->filter.symmetric_convolve_float_v[PIXEL_FORMAT_ARGB32] = FilterC::symmetric_convolve_float_v_argb32;
  m->filter.symmetric_convolve_float_h[PIXEL_FORMAT_XRGB32] = FilterC::symmetric_convolve_float_h_argb32;
  m->filter.symmetric_convolve_float_v[PIXEL_FORMAT_XRGB32] = FilterC::symmetric_convolve_float_v_argb32;
  m->filter.symmetric_convolve_float_h[PIXEL_FORMAT_A8] = FilterC::symmetric_convolve_float_h_a8;
  m->filter.symmetric_convolve_float_v[PIXEL_FORMAT_A8] = FilterC::symmetric_convolve_float_v_a8;

  // [Composite - Src]

  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = DibC::memcpy32;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = DibC::prgb32_from_argb32;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = DibC::frgb32_from_xrgb32;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_I8] = DibC::prgb32_from_i8;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_A8] = DibC::azzz32_from_a8;

  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_ARGB32].vspan[PIXEL_FORMAT_PRGB32] = DibC::argb32_from_prgb32;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_ARGB32].vspan[PIXEL_FORMAT_ARGB32] = DibC::memcpy32;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_ARGB32].vspan[PIXEL_FORMAT_XRGB32] = DibC::frgb32_from_xrgb32;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_ARGB32].vspan[PIXEL_FORMAT_I8] = DibC::argb32_from_i8;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_ARGB32].vspan[PIXEL_FORMAT_A8] = DibC::azzz32_from_a8;

  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = DibC::frgb32_from_xrgb32;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = DibC::frgb32_from_argb32;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = DibC::memcpy32;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_I8] = DibC::frgb32_from_i8;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_A8] = DibC::fzzz32_from_null;

  // TODO

  // [Composite - Dst]

  fog_raster_set_nops(&m->raster[COMPOSITE_DST][PIXEL_FORMAT_ARGB32]);
  fog_raster_set_nops(&m->raster[COMPOSITE_DST][PIXEL_FORMAT_PRGB32]);
  fog_raster_set_nops(&m->raster[COMPOSITE_DST][PIXEL_FORMAT_XRGB32]);
  fog_raster_set_nops(&m->raster[COMPOSITE_DST][PIXEL_FORMAT_A8]);
  fog_raster_set_nops(&m->raster[COMPOSITE_DST][PIXEL_FORMAT_I8]);

  // [Composite - SrcOver]

  // TODO

  // [Composite - DstOver]

  fog_raster_set_nops(&m->raster[COMPOSITE_DST_OVER][PIXEL_FORMAT_XRGB32]);

  // TODO

  // [Composite - SrcIn]

  // TODO

  // [Composite - DstIn]

  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = (VSpanFn)CompositeNopC::cspan;
  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = (VSpanMskFn)CompositeNopC::cspan_a8;
  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = (VSpanMskConstFn)CompositeNopC::cspan_a8_const;

  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = (VSpanFn)CompositeNopC::cspan;
  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = (VSpanMskFn)CompositeNopC::cspan_a8;
  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = (VSpanMskConstFn)CompositeNopC::cspan_a8_const;

  // TODO

  // [Composite - SrcOut]

  // TODO

  // [Composite - DstOut]

  // TODO

  // [Composite - SrcAtop]

  // TODO

  // [Composite - DstAtop]

  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = (VSpanFn)CompositeNopC::cspan;
  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = (VSpanMskFn)CompositeNopC::cspan_a8;
  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = (VSpanMskConstFn)CompositeNopC::cspan_a8_const;

  fog_raster_set_nops(&m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_A8]);

  // TODO

  // [Composite - Xor]

  // TODO

  // [Composite - Clear]

  // TODO

  // [Composite - Add]

  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_A8] = (VSpanFn)CompositeNopC::cspan;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_A8] = (VSpanMskFn)CompositeNopC::cspan_a8;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_A8] = (VSpanMskConstFn)CompositeNopC::cspan_a8_const;

  // TODO
}
