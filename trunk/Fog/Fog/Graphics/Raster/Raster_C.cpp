// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled Headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/Math.h>
#include <Fog/Core/Misc.h>
#include <Fog/Graphics/ColorLut.h>
#include <Fog/Graphics/ColorMatrix.h>
#include <Fog/Graphics/DitherMatrix.h>
#include <Fog/Graphics/Error.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/ImageFilter.h>
#include <Fog/Graphics/Matrix.h>
#include <Fog/Graphics/Path.h>
#include <Fog/Graphics/Path_p.h>
#include <Fog/Graphics/Pattern.h>
#include <Fog/Graphics/Raster.h>
#include <Fog/Graphics/Raster/Raster_C.h>
#include <Fog/Graphics/RgbaAnalyzer.h>

// [Raster_C]
#include <Fog/Graphics/Raster/Raster_C_base.cpp.h>
#include <Fog/Graphics/Raster/Raster_C_adaptor.cpp.h>
#include <Fog/Graphics/Raster/Raster_C_convert.cpp.h>
#include <Fog/Graphics/Raster/Raster_C_composite.cpp.h>
#include <Fog/Graphics/Raster/Raster_C_gradient.cpp.h>
#include <Fog/Graphics/Raster/Raster_C_pattern.cpp.h>
#include <Fog/Graphics/Raster/Raster_C_filters.cpp.h>
#include <Fog/Graphics/Raster/Raster_C_vector.cpp.h>

// ============================================================================
// [Library Initializers]
// ============================================================================

static void fog_raster_set_nops(Fog::Raster::FunctionMap::RasterFuncs* ops)
{
  using namespace Fog;
  using namespace Fog::Raster;

  ops->pixel = raster_pixel_nop;
  ops->pixel_a8 = raster_pixel_a8_nop;
  ops->span_solid = raster_span_solid_nop;
  ops->span_solid_a8 = raster_span_solid_a8_nop;
  ops->span_solid_a8_const = raster_span_solid_a8_const_nop;

  ops->span_composite[Image::FormatARGB32] = (SpanCompositeFn)raster_span_solid_nop;
  ops->span_composite[Image::FormatPRGB32] = (SpanCompositeFn)raster_span_solid_nop;
  ops->span_composite[Image::FormatRGB32] = (SpanCompositeFn)raster_span_solid_nop;
  ops->span_composite[Image::FormatRGB24] = (SpanCompositeFn)raster_span_solid_nop;
  ops->span_composite[Image::FormatA8] = (SpanCompositeFn)raster_span_solid_nop;
  ops->span_composite[Image::FormatI8] = (SpanCompositeFn)raster_span_solid_nop;

  ops->span_composite_a8[Image::FormatARGB32] = (SpanCompositeMskFn)raster_span_solid_a8_nop;
  ops->span_composite_a8[Image::FormatPRGB32] = (SpanCompositeMskFn)raster_span_solid_a8_nop;
  ops->span_composite_a8[Image::FormatRGB32] = (SpanCompositeMskFn)raster_span_solid_a8_nop;
  ops->span_composite_a8[Image::FormatRGB24] = (SpanCompositeMskFn)raster_span_solid_a8_nop;
  ops->span_composite_a8[Image::FormatA8] = (SpanCompositeMskFn)raster_span_solid_a8_nop;
  ops->span_composite_a8[Image::FormatI8] = (SpanCompositeMskFn)raster_span_solid_a8_nop;

  ops->span_composite_a8_const[Image::FormatARGB32] = (SpanCompositeMskConstFn)raster_span_solid_a8_const_nop;
  ops->span_composite_a8_const[Image::FormatPRGB32] = (SpanCompositeMskConstFn)raster_span_solid_a8_const_nop;
  ops->span_composite_a8_const[Image::FormatRGB32] = (SpanCompositeMskConstFn)raster_span_solid_a8_const_nop;
  ops->span_composite_a8_const[Image::FormatRGB24] = (SpanCompositeMskConstFn)raster_span_solid_a8_const_nop;
  ops->span_composite_a8_const[Image::FormatA8] = (SpanCompositeMskConstFn)raster_span_solid_a8_const_nop;
  ops->span_composite_a8_const[Image::FormatI8] = (SpanCompositeMskConstFn)raster_span_solid_a8_const_nop;
}

FOG_INIT_DECLARE void fog_raster_init_c(void)
{
  using namespace Fog;
  using namespace Fog::Raster;

  FunctionMap* m = functionMap;

  // [Convert - ByteSwap]

  m->convert.bswap16 = convert_bswap16;
  m->convert.bswap24 = convert_bswap24;
  m->convert.bswap32 = convert_bswap32;

  // [Convert - MemCpy]

  m->convert.memcpy8 = convert_memcpy8;
  m->convert.memcpy16 = convert_memcpy16;
  m->convert.memcpy24 = convert_memcpy24;
  m->convert.memcpy32 = convert_memcpy32;

  // [Convert - Axxx32 Dest]

  m->convert.axxx32_from_xxxx32 = convert_axxx32_from_xxxx32;
  m->convert.axxx32_from_a8 = convert_axxx32_from_a8;
  m->convert.axxx32_bs_from_a8 = convert_axxx32_bs_from_a8;

  // [Convert - Argb32 Dest]

  m->convert.argb32_from_prgb32 = convert_argb32_from_prgb32;
  m->convert.argb32_from_prgb32_bs = convert_argb32_from_prgb32_bs;
  m->convert.argb32_from_rgb32 = convert_axxx32_from_xxxx32;
  m->convert.argb32_from_rgb32_bs = convert_argb32_from_rgb32_bs;
  m->convert.argb32_from_i8 = convert_argb32_from_i8;

  m->convert.argb32_bs_from_rgb32 = convert_argb32_bs_from_rgb32;
  m->convert.argb32_bs_from_prgb32 = convert_argb32_bs_from_prgb32;
  m->convert.argb32_bs_from_i8 = convert_argb32_bs_from_i8;

  // [Convert - Prgb32 Dest]

  m->convert.prgb32_from_argb32 = convert_prgb32_from_argb32;
  m->convert.prgb32_from_argb32_bs = convert_prgb32_from_argb32_bs;
  m->convert.prgb32_from_i8 = convert_prgb32_from_i8;

  m->convert.prgb32_bs_from_argb32 = convert_prgb32_bs_from_argb32;
  m->convert.prgb32_bs_from_i8 = convert_prgb32_bs_from_i8;

  // [Convert - Rgb32 Dest]

  m->convert.rgb32_from_argb32 = convert_rgb32_from_argb32;
  m->convert.rgb32_from_argb32_bs = convert_rgb32_from_argb32_bs;
  m->convert.rgb32_from_rgb24 = convert_rgb32_from_rgb24;
  m->convert.rgb32_from_bgr24 = convert_rgb32_from_bgr24;
  m->convert.rgb32_from_rgb16_5550 = convert_rgb32_from_rgb16_5550;
  m->convert.rgb32_from_rgb16_5550_bs = convert_rgb32_from_rgb16_5550_bs;
  m->convert.rgb32_from_rgb16_5650 = convert_rgb32_from_rgb16_5650;
  m->convert.rgb32_from_rgb16_5650_bs = convert_rgb32_from_rgb16_5650_bs;
  m->convert.rgb32_from_i8 = convert_rgb32_from_i8;

  m->convert.rgb32_bs_from_rgb24 = convert_rgb32_bs_from_rgb24;

  // [Convert - Rgb24/Bgr24 Dest]

  m->convert.rgb24_from_rgb32 = convert_rgb24_from_rgb32;
  m->convert.rgb24_from_rgb32_bs = convert_rgb24_from_rgb32_bs;
  m->convert.rgb24_from_rgb16_5550 = convert_rgb24_from_rgb16_5550;
  m->convert.rgb24_from_rgb16_5550_bs = convert_rgb24_from_rgb16_5550_bs;
  m->convert.rgb24_from_rgb16_5650 = convert_rgb24_from_rgb16_5650;
  m->convert.rgb24_from_rgb16_5650_bs = convert_rgb24_from_rgb16_5650_bs;
  m->convert.rgb24_from_i8 = convert_rgb24_from_i8;

  m->convert.bgr24_from_rgb32 = convert_bgr24_from_rgb32;
  m->convert.bgr24_from_i8 = convert_bgr24_from_i8;

  // [Convert - Rgb16 Dest]

  m->convert.rgb16_5550_from_rgb32 = convert_rgb16_5550_from_rgb32;
  m->convert.rgb16_5550_from_rgb24 = convert_rgb16_5550_from_rgb24;
  m->convert.rgb16_5550_from_i8 = convert_rgb16_5550_from_i8;

  m->convert.rgb16_5650_from_rgb32 = convert_rgb16_5650_from_rgb32;
  m->convert.rgb16_5650_from_rgb24 = convert_rgb16_5650_from_rgb24;
  m->convert.rgb16_5650_from_i8 = convert_rgb16_5650_from_i8;

  m->convert.rgb16_5550_bs_from_rgb32 = convert_rgb16_5550_bs_from_rgb32;
  m->convert.rgb16_5550_bs_from_rgb24 = convert_rgb16_5550_bs_from_rgb24;
  m->convert.rgb16_5550_bs_from_i8 = convert_rgb16_5550_bs_from_i8;

  m->convert.rgb16_5650_bs_from_rgb32 = convert_rgb16_5650_bs_from_rgb32;
  m->convert.rgb16_5650_bs_from_rgb24 = convert_rgb16_5650_bs_from_rgb24;
  m->convert.rgb16_5650_bs_from_i8 = convert_rgb16_5650_bs_from_i8;

  // [Convert - A8 Dest]

  m->convert.a8_from_axxx32 = convert_a8_from_axxx32;
  m->convert.a8_from_i8 = convert_a8_from_i8;

  // [Convert - Greyscale]

  m->convert.greyscale8_from_rgb32 = convert_greyscale8_from_rgb32;
  m->convert.greyscale8_from_rgb24 = convert_greyscale8_from_rgb24;
  m->convert.greyscale8_from_bgr24 = convert_greyscale8_from_bgr24;
  m->convert.greyscale8_from_i8 = convert_greyscale8_from_i8;

  m->convert.rgb32_from_greyscale8 = convert_rgb32_from_greyscale8;
  m->convert.rgb24_from_greyscale8 = convert_rgb24_from_greyscale8;

  // [Convert - Dithering]

  m->convert.i8rgb232_from_rgb32_dither = convert_i8rgb232_from_rgb32_dither;
  m->convert.i8rgb222_from_rgb32_dither = convert_i8rgb222_from_rgb32_dither;
  m->convert.i8rgb111_from_rgb32_dither = convert_i8rgb111_from_rgb32_dither;

  m->convert.i8rgb232_from_rgb24_dither = convert_i8rgb232_from_rgb24_dither;
  m->convert.i8rgb222_from_rgb24_dither = convert_i8rgb222_from_rgb24_dither;
  m->convert.i8rgb111_from_rgb24_dither = convert_i8rgb111_from_rgb24_dither;

  m->convert.rgb16_5550_from_rgb32_dither = convert_rgb16_5550_from_rgb32_dither;
  m->convert.rgb16_5550_from_rgb24_dither = convert_rgb16_5550_from_rgb24_dither;

  m->convert.rgb16_5650_from_rgb32_dither = convert_rgb16_5650_from_rgb32_dither;
  m->convert.rgb16_5650_from_rgb24_dither = convert_rgb16_5650_from_rgb24_dither;

  m->convert.rgb16_5550_bs_from_rgb32_dither = convert_rgb16_5550_bs_from_rgb32_dither;
  m->convert.rgb16_5550_bs_from_rgb24_dither = convert_rgb16_5550_bs_from_rgb24_dither;

  m->convert.rgb16_5650_bs_from_rgb32_dither = convert_rgb16_5650_bs_from_rgb32_dither;
  m->convert.rgb16_5650_bs_from_rgb24_dither = convert_rgb16_5650_bs_from_rgb24_dither;

  // [Gradient - Gradient]

  m->gradient.gradient_argb32 = gradient_gradient_argb32;
  m->gradient.gradient_prgb32 = gradient_gradient_prgb32;
  m->gradient.gradient_rgb32 = gradient_gradient_rgb32;
  m->gradient.gradient_rgb24 = gradient_gradient_rgb24;
  m->gradient.gradient_a8 = gradient_gradient_a8;

  // [Pattern - Texture]

  m->pattern.texture_init = pattern_texture_init;
  m->pattern.texture_fetch_repeat = pattern_texture_fetch_repeat;
  m->pattern.texture_fetch_reflect = pattern_texture_fetch_reflect;

  // [Pattern - Linear Gradient]

  m->pattern.linear_gradient_init = pattern_linear_gradient_init;
  m->pattern.linear_gradient_fetch_pad = pattern_linear_gradient_fetch_pad;
  m->pattern.linear_gradient_fetch_repeat = pattern_linear_gradient_fetch_repeat;

  // [Pattern - Radial Gradient]

  m->pattern.radial_gradient_init = pattern_radial_gradient_init;
  m->pattern.radial_gradient_fetch_pad = pattern_radial_gradient_fetch_pad;
  m->pattern.radial_gradient_fetch_repeat = pattern_radial_gradient_fetch_repeat;

  // [Pattern - Conical Gradient]

  m->pattern.conical_gradient_init = pattern_conical_gradient_init;
  m->pattern.conical_gradient_fetch = pattern_conical_gradient_fetch;

  // [Raster - CompositeSrc]

  // TODO

  // [Raster - CompositeDest (NOP)]

  fog_raster_set_nops(&m->raster[CompositeDest][Image::FormatARGB32]);
  fog_raster_set_nops(&m->raster[CompositeDest][Image::FormatPRGB32]);
  fog_raster_set_nops(&m->raster[CompositeDest][Image::FormatRGB32]);
  fog_raster_set_nops(&m->raster[CompositeDest][Image::FormatRGB24]);
  fog_raster_set_nops(&m->raster[CompositeDest][Image::FormatA8]);
  fog_raster_set_nops(&m->raster[CompositeDest][Image::FormatI8]);

  // [Raster - CompositeSrcOver]

  // TODO

  // [Raster - CompositeDestOver]

  fog_raster_set_nops(&m->raster[CompositeDest][Image::FormatRGB32]);
  fog_raster_set_nops(&m->raster[CompositeDest][Image::FormatRGB24]);

  // TODO

  // [Raster - CompositeSrcIn]

  // TODO

  // [Raster - CompositeDestIn]

  // TODO

  // [Raster - CompositeSrcOut]

  // TODO

  // [Raster - CompositeDestOut]

  // TODO

  // [Raster - CompositeSrcAtop]

  // TODO

  // [Raster - CompositeDestAtop]

  fog_raster_set_nops(&m->raster[CompositeDestAtop][Image::FormatA8]);

  // TODO

  // [Raster - CompositeXor]

  // TODO

  // [Raster - CompositeClear]

  // TODO

  // [Raster - CompositeAdd]

  m->raster[CompositeAdd][Image::FormatRGB32].span_composite[Image::FormatA8] = (SpanCompositeFn)raster_span_solid_nop;
  m->raster[CompositeAdd][Image::FormatRGB32].span_composite_a8[Image::FormatA8] = (SpanCompositeMskFn)raster_span_solid_a8_nop;
  m->raster[CompositeAdd][Image::FormatRGB32].span_composite_a8_const[Image::FormatA8] = (SpanCompositeMskConstFn)raster_span_solid_a8_const_nop;

  // TODO

  // [Adaptor - Argb32]

  m->adaptor[Image::FormatARGB32].pixel = adaptor_argb32_pixel;
  m->adaptor[Image::FormatARGB32].pixel_a8 = adaptor_argb32_pixel_a8;
//m->adaptor[Image::FormatARGB32].span_solid = NULL;
//m->adaptor[Image::FormatARGB32].span_solid_a8 = NULL;
//m->adaptor[Image::FormatARGB32].span_solid_a8_const = NULL;

  m->adaptor[Image::FormatARGB32].span_composite[Image::FormatARGB32] = adaptor_argb32_span_composite_argb32;
//m->adaptor[Image::FormatARGB32].span_composite[Image::FormatPRGB32] = NULL;
  m->adaptor[Image::FormatARGB32].span_composite[Image::FormatRGB32] = adaptor_argb32_span_composite_rgb32;
  m->adaptor[Image::FormatARGB32].span_composite[Image::FormatRGB24] = adaptor_argb32_span_composite_rgb24;
  m->adaptor[Image::FormatARGB32].span_composite[Image::FormatA8] = adaptor_argb32_span_composite_a8;
  m->adaptor[Image::FormatARGB32].span_composite[Image::FormatI8] = adaptor_argb32_span_composite_i8;

  m->adaptor[Image::FormatARGB32].span_composite_a8[Image::FormatARGB32] = adaptor_argb32_span_composite_argb32_a8;
//m->adaptor[Image::FormatARGB32].span_composite_a8[Image::FormatPRGB32] = NULL;
  m->adaptor[Image::FormatARGB32].span_composite_a8[Image::FormatRGB32] = adaptor_argb32_span_composite_rgb32_a8;
  m->adaptor[Image::FormatARGB32].span_composite_a8[Image::FormatRGB24] = adaptor_argb32_span_composite_rgb24_a8;
  m->adaptor[Image::FormatARGB32].span_composite_a8[Image::FormatA8] = adaptor_argb32_span_composite_a8_a8;
  m->adaptor[Image::FormatARGB32].span_composite_a8[Image::FormatI8] = adaptor_argb32_span_composite_i8_a8;

  m->adaptor[Image::FormatARGB32].span_composite_a8_const[Image::FormatARGB32] = adaptor_argb32_span_composite_argb32_a8_const;
//m->adaptor[Image::FormatARGB32].span_composite_a8_const[Image::FormatPRGB32] = NULL;
  m->adaptor[Image::FormatARGB32].span_composite_a8_const[Image::FormatRGB32] = adaptor_argb32_span_composite_rgb32_a8_const;
  m->adaptor[Image::FormatARGB32].span_composite_a8_const[Image::FormatRGB24] = adaptor_argb32_span_composite_rgb24_a8_const;
  m->adaptor[Image::FormatARGB32].span_composite_a8_const[Image::FormatA8] = adaptor_argb32_span_composite_a8_a8_const;
  m->adaptor[Image::FormatARGB32].span_composite_a8_const[Image::FormatI8] = adaptor_argb32_span_composite_i8_a8_const;

  // [Adaptor - Prgb32]

  m->adaptor[Image::FormatPRGB32].pixel = adaptor_prgb32_pixel;
  m->adaptor[Image::FormatPRGB32].pixel_a8 = adaptor_prgb32_pixel_a8;
//m->adaptor[Image::FormatPRGB32].span_solid = NULL;
//m->adaptor[Image::FormatPRGB32].span_solid_a8 = NULL;
//m->adaptor[Image::FormatPRGB32].span_solid_a8_const = NULL;

  m->adaptor[Image::FormatPRGB32].span_composite[Image::FormatARGB32] = adaptor_prgb32_span_composite_argb32;
//m->adaptor[Image::FormatPRGB32].span_composite[Image::FormatPRGB32] = NULL;
  m->adaptor[Image::FormatPRGB32].span_composite[Image::FormatRGB32] = adaptor_prgb32_span_composite_rgb32;
  m->adaptor[Image::FormatPRGB32].span_composite[Image::FormatRGB24] = adaptor_prgb32_span_composite_rgb24;
  m->adaptor[Image::FormatPRGB32].span_composite[Image::FormatA8] = adaptor_prgb32_span_composite_a8;
  m->adaptor[Image::FormatPRGB32].span_composite[Image::FormatI8] = adaptor_prgb32_span_composite_i8;

  m->adaptor[Image::FormatPRGB32].span_composite_a8[Image::FormatARGB32] = adaptor_prgb32_span_composite_argb32_a8;
//m->adaptor[Image::FormatPRGB32].span_composite_a8[Image::FormatPRGB32] = NULL;
  m->adaptor[Image::FormatPRGB32].span_composite_a8[Image::FormatRGB32] = adaptor_prgb32_span_composite_rgb32_a8;
  m->adaptor[Image::FormatPRGB32].span_composite_a8[Image::FormatRGB24] = adaptor_prgb32_span_composite_rgb24_a8;
  m->adaptor[Image::FormatPRGB32].span_composite_a8[Image::FormatA8] = adaptor_prgb32_span_composite_a8_a8;
  m->adaptor[Image::FormatPRGB32].span_composite_a8[Image::FormatI8] = adaptor_prgb32_span_composite_i8_a8;

  m->adaptor[Image::FormatPRGB32].span_composite_a8_const[Image::FormatARGB32] = adaptor_prgb32_span_composite_argb32_a8_const;
//m->adaptor[Image::FormatPRGB32].span_composite_a8_const[Image::FormatPRGB32] = NULL;
  m->adaptor[Image::FormatPRGB32].span_composite_a8_const[Image::FormatRGB32] = adaptor_prgb32_span_composite_rgb32_a8_const;
  m->adaptor[Image::FormatPRGB32].span_composite_a8_const[Image::FormatRGB24] = adaptor_prgb32_span_composite_rgb24_a8_const;
  m->adaptor[Image::FormatPRGB32].span_composite_a8_const[Image::FormatA8] = adaptor_prgb32_span_composite_a8_a8_const;
  m->adaptor[Image::FormatPRGB32].span_composite_a8_const[Image::FormatI8] = adaptor_prgb32_span_composite_i8_a8_const;

  // [Adaptor - Rgb32]

  m->adaptor[Image::FormatRGB32].pixel = adaptor_rgb32_pixel;
  m->adaptor[Image::FormatRGB32].pixel_a8 = adaptor_rgb32_pixel_a8;
//m->adaptor[Image::FormatRGB32].span_solid = NULL;
//m->adaptor[Image::FormatRGB32].span_solid_a8 = NULL;
//m->adaptor[Image::FormatRGB32].span_solid_a8_const = NULL;

  m->adaptor[Image::FormatRGB32].span_composite[Image::FormatARGB32] = adaptor_rgb32_span_composite_argb32;
//m->adaptor[Image::FormatRGB32].span_composite[Image::FormatPRGB32] = NULL;
  m->adaptor[Image::FormatRGB32].span_composite[Image::FormatRGB32] = adaptor_rgb32_span_composite_rgb32;
  m->adaptor[Image::FormatRGB32].span_composite[Image::FormatRGB24] = adaptor_rgb32_span_composite_rgb24;
  m->adaptor[Image::FormatRGB32].span_composite[Image::FormatA8] = adaptor_rgb32_span_composite_a8;
  m->adaptor[Image::FormatRGB32].span_composite[Image::FormatI8] = adaptor_rgb32_span_composite_i8;

  m->adaptor[Image::FormatRGB32].span_composite_a8[Image::FormatARGB32] = adaptor_rgb32_span_composite_argb32_a8;
//m->adaptor[Image::FormatRGB32].span_composite_a8[Image::FormatPRGB32] = NULL;
  m->adaptor[Image::FormatRGB32].span_composite_a8[Image::FormatRGB32] = adaptor_rgb32_span_composite_rgb32_a8;
  m->adaptor[Image::FormatRGB32].span_composite_a8[Image::FormatRGB24] = adaptor_rgb32_span_composite_rgb24_a8;
  m->adaptor[Image::FormatRGB32].span_composite_a8[Image::FormatA8] = adaptor_rgb32_span_composite_a8_a8;
  m->adaptor[Image::FormatRGB32].span_composite_a8[Image::FormatI8] = adaptor_rgb32_span_composite_i8_a8;

  m->adaptor[Image::FormatRGB32].span_composite_a8_const[Image::FormatARGB32] = adaptor_rgb32_span_composite_argb32_a8_const;
//m->adaptor[Image::FormatRGB32].span_composite_a8_const[Image::FormatPRGB32] = NULL;
  m->adaptor[Image::FormatRGB32].span_composite_a8_const[Image::FormatRGB32] = adaptor_rgb32_span_composite_rgb32_a8_const;
  m->adaptor[Image::FormatRGB32].span_composite_a8_const[Image::FormatRGB24] = adaptor_rgb32_span_composite_rgb24_a8_const;
  m->adaptor[Image::FormatRGB32].span_composite_a8_const[Image::FormatA8] = adaptor_rgb32_span_composite_a8_a8_const;
  m->adaptor[Image::FormatRGB32].span_composite_a8_const[Image::FormatI8] = adaptor_rgb32_span_composite_i8_a8_const;

  // [Adaptor - Rgb24]

  m->adaptor[Image::FormatRGB24].pixel = adaptor_rgb24_pixel;
  m->adaptor[Image::FormatRGB24].pixel_a8 = adaptor_rgb24_pixel_a8;
  m->adaptor[Image::FormatRGB24].span_solid = adaptor_rgb24_span_solid;
  m->adaptor[Image::FormatRGB24].span_solid_a8 = adaptor_rgb24_span_solid_a8;
  m->adaptor[Image::FormatRGB24].span_solid_a8_const = adaptor_rgb24_span_solid_a8_const;

  m->adaptor[Image::FormatRGB24].span_composite[Image::FormatARGB32] = adaptor_rgb24_span_composite_argb32;
  m->adaptor[Image::FormatRGB24].span_composite[Image::FormatPRGB32] = adaptor_rgb24_span_composite_prgb32;
  m->adaptor[Image::FormatRGB24].span_composite[Image::FormatRGB32] = adaptor_rgb24_span_composite_rgb32;
  m->adaptor[Image::FormatRGB24].span_composite[Image::FormatRGB24] = adaptor_rgb24_span_composite_rgb24;
  m->adaptor[Image::FormatRGB24].span_composite[Image::FormatA8] = adaptor_rgb24_span_composite_a8;
  m->adaptor[Image::FormatRGB24].span_composite[Image::FormatI8] = adaptor_rgb24_span_composite_i8;

  m->adaptor[Image::FormatRGB24].span_composite_a8[Image::FormatARGB32] = adaptor_rgb24_span_composite_argb32_a8;
  m->adaptor[Image::FormatRGB24].span_composite_a8[Image::FormatPRGB32] = adaptor_rgb24_span_composite_prgb32_a8;
  m->adaptor[Image::FormatRGB24].span_composite_a8[Image::FormatRGB32] = adaptor_rgb24_span_composite_rgb32_a8;
  m->adaptor[Image::FormatRGB24].span_composite_a8[Image::FormatRGB24] = adaptor_rgb24_span_composite_rgb24_a8;
  m->adaptor[Image::FormatRGB24].span_composite_a8[Image::FormatA8] = adaptor_rgb24_span_composite_a8_a8;
  m->adaptor[Image::FormatRGB24].span_composite_a8[Image::FormatI8] = adaptor_rgb24_span_composite_i8_a8;

  m->adaptor[Image::FormatRGB24].span_composite_a8_const[Image::FormatARGB32] = adaptor_rgb24_span_composite_argb32_a8_const;
  m->adaptor[Image::FormatRGB24].span_composite_a8_const[Image::FormatPRGB32] = adaptor_rgb24_span_composite_prgb32_a8_const;
  m->adaptor[Image::FormatRGB24].span_composite_a8_const[Image::FormatRGB32] = adaptor_rgb24_span_composite_rgb32_a8_const;
  m->adaptor[Image::FormatRGB24].span_composite_a8_const[Image::FormatRGB24] = adaptor_rgb24_span_composite_rgb24_a8_const;
  m->adaptor[Image::FormatRGB24].span_composite_a8_const[Image::FormatA8] = adaptor_rgb24_span_composite_a8_a8_const;
  m->adaptor[Image::FormatRGB24].span_composite_a8_const[Image::FormatI8] = adaptor_rgb24_span_composite_i8_a8_const;

  // [Filters - ColorLut]

  m->filter.colorLut[Image::FormatARGB32] = colorLut_argb32;
  m->filter.colorLut[Image::FormatPRGB32] = colorLut_prgb32;
  m->filter.colorLut[Image::FormatRGB32] = colorLut_rgb32;
  m->filter.colorLut[Image::FormatRGB24] = colorLut_rgb24;
  m->filter.colorLut[Image::FormatA8] = colorLut_a8;

  // [Filters - ColorMatrix]

  m->filter.colorMatrix[Image::FormatARGB32] = colorMatrix_argb32;
  m->filter.colorMatrix[Image::FormatPRGB32] = colorMatrix_prgb32;
  m->filter.colorMatrix[Image::FormatRGB32] = colorMatrix_rgb32;
  m->filter.colorMatrix[Image::FormatRGB24] = colorMatrix_rgb24;
  m->filter.colorMatrix[Image::FormatA8] = colorMatrix_a8;

  // [Filters - CopyArea]

  m->filter.copyArea[Image::FormatARGB32] = copyArea_32;
  m->filter.copyArea[Image::FormatPRGB32] = copyArea_32;
  m->filter.copyArea[Image::FormatRGB32] = copyArea_32;
  m->filter.copyArea[Image::FormatRGB24] = copyArea_24;
  m->filter.copyArea[Image::FormatA8] = copyArea_8;
  m->filter.copyArea[Image::FormatI8] = copyArea_8;

  // [Filters - FloatScanlineConvolve]

  m->filter.floatScanlineConvolveH[Image::FormatARGB32] = floatScanlineConvolveH_argb32;
  m->filter.floatScanlineConvolveV[Image::FormatARGB32] = floatScanlineConvolveV_argb32;
  m->filter.floatScanlineConvolveH[Image::FormatRGB32] = floatScanlineConvolveH_argb32;
  m->filter.floatScanlineConvolveV[Image::FormatRGB32] = floatScanlineConvolveV_argb32;
  m->filter.floatScanlineConvolveH[Image::FormatRGB24] = floatScanlineConvolveH_rgb24;
  m->filter.floatScanlineConvolveV[Image::FormatRGB24] = floatScanlineConvolveV_rgb24;
  m->filter.floatScanlineConvolveH[Image::FormatA8] = floatScanlineConvolveH_a8;
  m->filter.floatScanlineConvolveV[Image::FormatA8] = floatScanlineConvolveV_a8;

  // [Filters - BoxBlurConvolve]

  m->filter.boxBlurConvolveH[Image::FormatARGB32] = boxBlurConvolveH_argb32;
  m->filter.boxBlurConvolveV[Image::FormatARGB32] = boxBlurConvolveV_argb32;
  m->filter.boxBlurConvolveH[Image::FormatRGB32] = boxBlurConvolveH_argb32;
  m->filter.boxBlurConvolveV[Image::FormatRGB32] = boxBlurConvolveV_argb32;
  m->filter.boxBlurConvolveH[Image::FormatRGB24] = boxBlurConvolveH_rgb24;
  m->filter.boxBlurConvolveV[Image::FormatRGB24] = boxBlurConvolveV_rgb24;
  m->filter.boxBlurConvolveH[Image::FormatA8] = boxBlurConvolveH_a8;
  m->filter.boxBlurConvolveV[Image::FormatA8] = boxBlurConvolveV_a8;

  // [Filters - StackBlurConvolve]

  m->filter.stackBlur8Mul = stackBlur8Mul;
  m->filter.stackBlur8Shr = stackBlur8Shr;

  m->filter.stackBlurConvolveH[Image::FormatARGB32] = stackBlurConvolveH_argb32;
  m->filter.stackBlurConvolveV[Image::FormatARGB32] = stackBlurConvolveV_argb32;
  m->filter.stackBlurConvolveH[Image::FormatRGB32] = stackBlurConvolveH_argb32;
  m->filter.stackBlurConvolveV[Image::FormatRGB32] = stackBlurConvolveV_argb32;
  m->filter.stackBlurConvolveH[Image::FormatRGB24] = stackBlurConvolveH_rgb24;
  m->filter.stackBlurConvolveV[Image::FormatRGB24] = stackBlurConvolveV_rgb24;
  m->filter.stackBlurConvolveH[Image::FormatA8] = stackBlurConvolveH_a8;
  m->filter.stackBlurConvolveV[Image::FormatA8] = stackBlurConvolveV_a8;

  // [Vector]

  m->vector.pathVertexTransform = pathVertexTransform;
  m->vector.approximateCurve3 = approximateCurve3;
  m->vector.approximateCurve4 = approximateCurve4;
}
