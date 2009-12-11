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
#include <Fog/Graphics/RasterUtil/RasterUtil_Convert_C.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Composite_C.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Gradient_C.h>
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

  ops->pixel = CompositeNopC::pixel;
  ops->pixel_a8 = CompositeNopC::pixel_a8;
  ops->cspan = CompositeNopC::cspan;
  ops->cspan_a8 = CompositeNopC::cspan_a8;
  ops->cspan_a8_const = CompositeNopC::cspan_a8_const;

  ops->vspan[PIXEL_FORMAT_ARGB32] = (VSpanFn)CompositeNopC::cspan;
  ops->vspan[PIXEL_FORMAT_PRGB32] = (VSpanFn)CompositeNopC::cspan;
  ops->vspan[PIXEL_FORMAT_XRGB32] = (VSpanFn)CompositeNopC::cspan;
  ops->vspan[PIXEL_FORMAT_RGB24] = (VSpanFn)CompositeNopC::cspan;
  ops->vspan[PIXEL_FORMAT_A8] = (VSpanFn)CompositeNopC::cspan;
  ops->vspan[PIXEL_FORMAT_I8] = (VSpanFn)CompositeNopC::cspan;

  ops->vspan_a8[PIXEL_FORMAT_ARGB32] = (VSpanMskFn)CompositeNopC::cspan_a8;
  ops->vspan_a8[PIXEL_FORMAT_PRGB32] = (VSpanMskFn)CompositeNopC::cspan_a8;
  ops->vspan_a8[PIXEL_FORMAT_XRGB32] = (VSpanMskFn)CompositeNopC::cspan_a8;
  ops->vspan_a8[PIXEL_FORMAT_RGB24] = (VSpanMskFn)CompositeNopC::cspan_a8;
  ops->vspan_a8[PIXEL_FORMAT_A8] = (VSpanMskFn)CompositeNopC::cspan_a8;
  ops->vspan_a8[PIXEL_FORMAT_I8] = (VSpanMskFn)CompositeNopC::cspan_a8;

  ops->vspan_a8_const[PIXEL_FORMAT_ARGB32] = (VSpanMskConstFn)CompositeNopC::cspan_a8_const;
  ops->vspan_a8_const[PIXEL_FORMAT_PRGB32] = (VSpanMskConstFn)CompositeNopC::cspan_a8_const;
  ops->vspan_a8_const[PIXEL_FORMAT_XRGB32] = (VSpanMskConstFn)CompositeNopC::cspan_a8_const;
  ops->vspan_a8_const[PIXEL_FORMAT_RGB24] = (VSpanMskConstFn)CompositeNopC::cspan_a8_const;
  ops->vspan_a8_const[PIXEL_FORMAT_A8] = (VSpanMskConstFn)CompositeNopC::cspan_a8_const;
  ops->vspan_a8_const[PIXEL_FORMAT_I8] = (VSpanMskConstFn)CompositeNopC::cspan_a8_const;
}

FOG_INIT_DECLARE void fog_raster_init_c(void)
{
  using namespace Fog;
  using namespace Fog::RasterUtil;

  FunctionMap* m = functionMap;

  // [Convert - ByteSwap]

  m->convert.bswap16 = ConvertC::bswap16;
  m->convert.bswap24 = ConvertC::bswap24;
  m->convert.bswap32 = ConvertC::bswap32;

  // [Convert - MemCpy]

  m->convert.memcpy8 = ConvertC::memcpy8;
  m->convert.memcpy16 = ConvertC::memcpy16;
  m->convert.memcpy24 = ConvertC::memcpy24;
  m->convert.memcpy32 = ConvertC::memcpy32;

  // [Convert - Axxx32 Dest]

  m->convert.axxx32_from_xxxx32 = ConvertC::axxx32_from_xxxx32;
  m->convert.axxx32_from_a8 = ConvertC::axxx32_from_a8;
  m->convert.axxx32_bs_from_a8 = ConvertC::axxx32_bs_from_a8;

  // [Convert - Argb32 Dest]

  m->convert.argb32_from_prgb32 = ConvertC::argb32_from_prgb32;
  m->convert.argb32_from_prgb32_bs = ConvertC::argb32_from_prgb32_bs;
  m->convert.argb32_from_rgb32 = ConvertC::axxx32_from_xxxx32;
  m->convert.argb32_from_rgb32_bs = ConvertC::argb32_from_rgb32_bs;
  m->convert.argb32_from_i8 = ConvertC::argb32_from_i8;

  m->convert.argb32_bs_from_rgb32 = ConvertC::argb32_bs_from_rgb32;
  m->convert.argb32_bs_from_prgb32 = ConvertC::argb32_bs_from_prgb32;
  m->convert.argb32_bs_from_i8 = ConvertC::argb32_bs_from_i8;

  // [Convert - Prgb32 Dest]

  m->convert.prgb32_from_argb32 = ConvertC::prgb32_from_argb32;
  m->convert.prgb32_from_argb32_bs = ConvertC::prgb32_from_argb32_bs;
  m->convert.prgb32_from_i8 = ConvertC::prgb32_from_i8;

  m->convert.prgb32_bs_from_argb32 = ConvertC::prgb32_bs_from_argb32;
  m->convert.prgb32_bs_from_i8 = ConvertC::prgb32_bs_from_i8;

  // [Convert - Rgb32 Dest]

  m->convert.rgb32_from_argb32 = ConvertC::rgb32_from_argb32;
  m->convert.rgb32_from_argb32_bs = ConvertC::rgb32_from_argb32_bs;
  m->convert.rgb32_from_rgb24 = ConvertC::rgb32_from_rgb24;
  m->convert.rgb32_from_bgr24 = ConvertC::rgb32_from_bgr24;
  m->convert.rgb32_from_rgb16_5550 = ConvertC::rgb32_from_rgb16_5550;
  m->convert.rgb32_from_rgb16_5550_bs = ConvertC::rgb32_from_rgb16_5550_bs;
  m->convert.rgb32_from_rgb16_5650 = ConvertC::rgb32_from_rgb16_5650;
  m->convert.rgb32_from_rgb16_5650_bs = ConvertC::rgb32_from_rgb16_5650_bs;
  m->convert.rgb32_from_i8 = ConvertC::rgb32_from_i8;

  m->convert.rgb32_bs_from_rgb24 = ConvertC::rgb32_bs_from_rgb24;

  // [Convert - Rgb24/Bgr24 Dest]

  m->convert.rgb24_from_rgb32 = ConvertC::rgb24_from_rgb32;
  m->convert.rgb24_from_rgb32_bs = ConvertC::rgb24_from_rgb32_bs;
  m->convert.rgb24_from_rgb16_5550 = ConvertC::rgb24_from_rgb16_5550;
  m->convert.rgb24_from_rgb16_5550_bs = ConvertC::rgb24_from_rgb16_5550_bs;
  m->convert.rgb24_from_rgb16_5650 = ConvertC::rgb24_from_rgb16_5650;
  m->convert.rgb24_from_rgb16_5650_bs = ConvertC::rgb24_from_rgb16_5650_bs;
  m->convert.rgb24_from_i8 = ConvertC::rgb24_from_i8;

  m->convert.bgr24_from_rgb32 = ConvertC::bgr24_from_rgb32;
  m->convert.bgr24_from_i8 = ConvertC::bgr24_from_i8;

  // [Convert - Rgb16 Dest]

  m->convert.rgb16_5550_from_rgb32 = ConvertC::rgb16_5550_from_rgb32;
  m->convert.rgb16_5550_from_rgb24 = ConvertC::rgb16_5550_from_rgb24;
  m->convert.rgb16_5550_from_i8 = ConvertC::rgb16_5550_from_i8;

  m->convert.rgb16_5650_from_rgb32 = ConvertC::rgb16_5650_from_rgb32;
  m->convert.rgb16_5650_from_rgb24 = ConvertC::rgb16_5650_from_rgb24;
  m->convert.rgb16_5650_from_i8 = ConvertC::rgb16_5650_from_i8;

  m->convert.rgb16_5550_bs_from_rgb32 = ConvertC::rgb16_5550_bs_from_rgb32;
  m->convert.rgb16_5550_bs_from_rgb24 = ConvertC::rgb16_5550_bs_from_rgb24;
  m->convert.rgb16_5550_bs_from_i8 = ConvertC::rgb16_5550_bs_from_i8;

  m->convert.rgb16_5650_bs_from_rgb32 = ConvertC::rgb16_5650_bs_from_rgb32;
  m->convert.rgb16_5650_bs_from_rgb24 = ConvertC::rgb16_5650_bs_from_rgb24;
  m->convert.rgb16_5650_bs_from_i8 = ConvertC::rgb16_5650_bs_from_i8;

  // [Convert - A8 Dest]

  m->convert.a8_from_axxx32 = ConvertC::a8_from_axxx32;
  m->convert.a8_from_i8 = ConvertC::a8_from_i8;

  // [Convert - Greyscale]

  m->convert.greyscale8_from_rgb32 = ConvertC::greyscale8_from_rgb32;
  m->convert.greyscale8_from_rgb24 = ConvertC::greyscale8_from_rgb24;
  m->convert.greyscale8_from_bgr24 = ConvertC::greyscale8_from_bgr24;
  m->convert.greyscale8_from_i8 = ConvertC::greyscale8_from_i8;

  m->convert.rgb32_from_greyscale8 = ConvertC::rgb32_from_greyscale8;
  m->convert.rgb24_from_greyscale8 = ConvertC::rgb24_from_greyscale8;

  // [Convert - Dithering]

  m->convert.i8rgb232_from_rgb32_dither = ConvertC::i8rgb232_from_rgb32_dither;
  m->convert.i8rgb222_from_rgb32_dither = ConvertC::i8rgb222_from_rgb32_dither;
  m->convert.i8rgb111_from_rgb32_dither = ConvertC::i8rgb111_from_rgb32_dither;

  m->convert.i8rgb232_from_rgb24_dither = ConvertC::i8rgb232_from_rgb24_dither;
  m->convert.i8rgb222_from_rgb24_dither = ConvertC::i8rgb222_from_rgb24_dither;
  m->convert.i8rgb111_from_rgb24_dither = ConvertC::i8rgb111_from_rgb24_dither;

  m->convert.rgb16_5550_from_rgb32_dither = ConvertC::rgb16_5550_from_rgb32_dither;
  m->convert.rgb16_5550_from_rgb24_dither = ConvertC::rgb16_5550_from_rgb24_dither;

  m->convert.rgb16_5650_from_rgb32_dither = ConvertC::rgb16_5650_from_rgb32_dither;
  m->convert.rgb16_5650_from_rgb24_dither = ConvertC::rgb16_5650_from_rgb24_dither;

  m->convert.rgb16_5550_bs_from_rgb32_dither = ConvertC::rgb16_5550_bs_from_rgb32_dither;
  m->convert.rgb16_5550_bs_from_rgb24_dither = ConvertC::rgb16_5550_bs_from_rgb24_dither;

  m->convert.rgb16_5650_bs_from_rgb32_dither = ConvertC::rgb16_5650_bs_from_rgb32_dither;
  m->convert.rgb16_5650_bs_from_rgb24_dither = ConvertC::rgb16_5650_bs_from_rgb24_dither;

  // [Gradient - Gradient]

  m->gradient.gradient_argb32 = GradientC::gradient_argb32;
  m->gradient.gradient_prgb32 = GradientC::gradient_prgb32;
  m->gradient.gradient_rgb32 = GradientC::gradient_rgb32;
  m->gradient.gradient_rgb24 = GradientC::gradient_rgb24;
  m->gradient.gradient_a8 = GradientC::gradient_a8;

  // [Pattern - Solid]

  m->pattern.solid_init = PatternC::solid_init;
  m->pattern.solid_fetch = PatternC::solid_fetch;

  // [Pattern - Texture]

  m->pattern.texture_init = PatternC::texture_init;
  m->pattern.texture_fetch_exact_repeat[PIXEL_FORMAT_PRGB32] = PatternC::texture_fetch_exact_repeat_32;
  m->pattern.texture_fetch_exact_repeat[PIXEL_FORMAT_ARGB32] = PatternC::texture_fetch_exact_repeat_32;
  m->pattern.texture_fetch_exact_repeat[PIXEL_FORMAT_XRGB32] = PatternC::texture_fetch_exact_repeat_32;
  m->pattern.texture_fetch_exact_repeat[PIXEL_FORMAT_RGB24] = PatternC::texture_fetch_exact_repeat_24;

  m->pattern.texture_fetch_exact_reflect[PIXEL_FORMAT_PRGB32] = PatternC::texture_fetch_exact_reflect_32;
  m->pattern.texture_fetch_exact_reflect[PIXEL_FORMAT_ARGB32] = PatternC::texture_fetch_exact_reflect_32;
  m->pattern.texture_fetch_exact_reflect[PIXEL_FORMAT_XRGB32] = PatternC::texture_fetch_exact_reflect_32;
  m->pattern.texture_fetch_exact_reflect[PIXEL_FORMAT_RGB24] = PatternC::texture_fetch_exact_reflect_24;

  m->pattern.texture_fetch_subx0_repeat[PIXEL_FORMAT_PRGB32] = PatternC::texture_fetch_subx0_repeat_32;
  m->pattern.texture_fetch_subx0_repeat[PIXEL_FORMAT_ARGB32] = PatternC::texture_fetch_subx0_repeat_32;
  m->pattern.texture_fetch_subx0_repeat[PIXEL_FORMAT_XRGB32] = PatternC::texture_fetch_subx0_repeat_32;
  m->pattern.texture_fetch_subx0_repeat[PIXEL_FORMAT_RGB24] = PatternC::texture_fetch_subx0_repeat_24;

  m->pattern.texture_fetch_subx0_reflect[PIXEL_FORMAT_PRGB32] = PatternC::texture_fetch_subx0_reflect_32;
  m->pattern.texture_fetch_subx0_reflect[PIXEL_FORMAT_ARGB32] = PatternC::texture_fetch_subx0_reflect_32;
  m->pattern.texture_fetch_subx0_reflect[PIXEL_FORMAT_XRGB32] = PatternC::texture_fetch_subx0_reflect_32;
  m->pattern.texture_fetch_subx0_reflect[PIXEL_FORMAT_RGB24] = PatternC::texture_fetch_subx0_reflect_24;

  m->pattern.texture_fetch_sub0y_repeat[PIXEL_FORMAT_PRGB32] = PatternC::texture_fetch_sub0y_repeat_32;
  m->pattern.texture_fetch_sub0y_repeat[PIXEL_FORMAT_ARGB32] = PatternC::texture_fetch_sub0y_repeat_32;
  m->pattern.texture_fetch_sub0y_repeat[PIXEL_FORMAT_XRGB32] = PatternC::texture_fetch_sub0y_repeat_32;
  m->pattern.texture_fetch_sub0y_repeat[PIXEL_FORMAT_RGB24] = PatternC::texture_fetch_sub0y_repeat_24;

  m->pattern.texture_fetch_sub0y_reflect[PIXEL_FORMAT_PRGB32] = PatternC::texture_fetch_sub0y_reflect_32;
  m->pattern.texture_fetch_sub0y_reflect[PIXEL_FORMAT_ARGB32] = PatternC::texture_fetch_sub0y_reflect_32;
  m->pattern.texture_fetch_sub0y_reflect[PIXEL_FORMAT_XRGB32] = PatternC::texture_fetch_sub0y_reflect_32;
  m->pattern.texture_fetch_sub0y_reflect[PIXEL_FORMAT_RGB24] = PatternC::texture_fetch_sub0y_reflect_24;

  m->pattern.texture_fetch_subxy_repeat[PIXEL_FORMAT_PRGB32] = PatternC::texture_fetch_subxy_repeat_32;
  m->pattern.texture_fetch_subxy_repeat[PIXEL_FORMAT_ARGB32] = PatternC::texture_fetch_subxy_repeat_32;
  m->pattern.texture_fetch_subxy_repeat[PIXEL_FORMAT_XRGB32] = PatternC::texture_fetch_subxy_repeat_32;
  m->pattern.texture_fetch_subxy_repeat[PIXEL_FORMAT_RGB24] = PatternC::texture_fetch_subxy_repeat_24;

  m->pattern.texture_fetch_subxy_reflect[PIXEL_FORMAT_PRGB32] = PatternC::texture_fetch_subxy_reflect_32;
  m->pattern.texture_fetch_subxy_reflect[PIXEL_FORMAT_ARGB32] = PatternC::texture_fetch_subxy_reflect_32;
  m->pattern.texture_fetch_subxy_reflect[PIXEL_FORMAT_XRGB32] = PatternC::texture_fetch_subxy_reflect_32;
  m->pattern.texture_fetch_subxy_reflect[PIXEL_FORMAT_RGB24] = PatternC::texture_fetch_subxy_reflect_24;

  m->pattern.texture_fetch_transform_nearest_repeat[PIXEL_FORMAT_PRGB32] = PatternC::texture_fetch_transform_nearest_repeat_32;
  m->pattern.texture_fetch_transform_nearest_repeat[PIXEL_FORMAT_ARGB32] = PatternC::texture_fetch_transform_nearest_repeat_32;
  m->pattern.texture_fetch_transform_nearest_repeat[PIXEL_FORMAT_XRGB32] = PatternC::texture_fetch_transform_nearest_repeat_32;
  m->pattern.texture_fetch_transform_nearest_repeat[PIXEL_FORMAT_RGB24] = PatternC::texture_fetch_transform_nearest_repeat_24;
  m->pattern.texture_fetch_transform_nearest_repeat[PIXEL_FORMAT_A8] = PatternC::texture_fetch_transform_nearest_repeat_a8;

  m->pattern.texture_fetch_transform_bilinear_repeat[PIXEL_FORMAT_PRGB32] = PatternC::texture_fetch_transform_bilinear_repeat_32;
  m->pattern.texture_fetch_transform_bilinear_repeat[PIXEL_FORMAT_ARGB32] = PatternC::texture_fetch_transform_bilinear_repeat_32;
  m->pattern.texture_fetch_transform_bilinear_repeat[PIXEL_FORMAT_XRGB32] = PatternC::texture_fetch_transform_bilinear_repeat_32;
  m->pattern.texture_fetch_transform_bilinear_repeat[PIXEL_FORMAT_RGB24] = PatternC::texture_fetch_transform_bilinear_repeat_24;
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
  m->filter.color_lut[PIXEL_FORMAT_RGB24] = FilterC::color_lut_rgb24;
  m->filter.color_lut[PIXEL_FORMAT_A8] = FilterC::color_lut_a8;

  // [Filter - ColorMatrix]

  m->filter.color_matrix[PIXEL_FORMAT_PRGB32] = FilterC::color_matrix_prgb32;
  m->filter.color_matrix[PIXEL_FORMAT_ARGB32] = FilterC::color_matrix_argb32;
  m->filter.color_matrix[PIXEL_FORMAT_XRGB32] = FilterC::color_matrix_xrgb32;
  m->filter.color_matrix[PIXEL_FORMAT_RGB24] = FilterC::color_matrix_rgb24;
  m->filter.color_matrix[PIXEL_FORMAT_A8] = FilterC::color_matrix_a8;

  // [Filters - CopyArea]

  m->filter.copy_area[PIXEL_FORMAT_PRGB32] = FilterC::copy_area_32;
  m->filter.copy_area[PIXEL_FORMAT_ARGB32] = FilterC::copy_area_32;
  m->filter.copy_area[PIXEL_FORMAT_XRGB32] = FilterC::copy_area_32;
  m->filter.copy_area[PIXEL_FORMAT_RGB24] = FilterC::copy_area_24;
  m->filter.copy_area[PIXEL_FORMAT_A8] = FilterC::copy_area_8;
  m->filter.copy_area[PIXEL_FORMAT_I8] = FilterC::copy_area_8;

  // [Filters - BoxBlur]

  m->filter.box_blur_h[PIXEL_FORMAT_ARGB32] = FilterC::box_blur_h_argb32;
  m->filter.box_blur_v[PIXEL_FORMAT_ARGB32] = FilterC::box_blur_v_argb32;
  m->filter.box_blur_h[PIXEL_FORMAT_XRGB32] = FilterC::box_blur_h_xrgb32;
  m->filter.box_blur_v[PIXEL_FORMAT_XRGB32] = FilterC::box_blur_v_xrgb32;
  m->filter.box_blur_h[PIXEL_FORMAT_RGB24] = FilterC::box_blur_h_rgb24;
  m->filter.box_blur_v[PIXEL_FORMAT_RGB24] = FilterC::box_blur_v_rgb24;
  m->filter.box_blur_h[PIXEL_FORMAT_A8] = FilterC::box_blur_h_a8;
  m->filter.box_blur_v[PIXEL_FORMAT_A8] = FilterC::box_blur_v_a8;

  // [Filters - LinearBlur]

  m->filter.linear_blur_h[PIXEL_FORMAT_ARGB32] = FilterC::linear_blur_h_argb32;
  m->filter.linear_blur_v[PIXEL_FORMAT_ARGB32] = FilterC::linear_blur_v_argb32;
  m->filter.linear_blur_h[PIXEL_FORMAT_XRGB32] = FilterC::linear_blur_h_argb32;
  m->filter.linear_blur_v[PIXEL_FORMAT_XRGB32] = FilterC::linear_blur_v_argb32;
  m->filter.linear_blur_h[PIXEL_FORMAT_RGB24] = FilterC::linear_blur_h_rgb24;
  m->filter.linear_blur_v[PIXEL_FORMAT_RGB24] = FilterC::linear_blur_v_rgb24;
  m->filter.linear_blur_h[PIXEL_FORMAT_A8] = FilterC::linear_blur_h_a8;
  m->filter.linear_blur_v[PIXEL_FORMAT_A8] = FilterC::linear_blur_v_a8;

  // [Filters - SymmetricConvolveFloat]

  m->filter.symmetric_convolve_float_h[PIXEL_FORMAT_ARGB32] = FilterC::symmetric_convolve_float_h_argb32;
  m->filter.symmetric_convolve_float_v[PIXEL_FORMAT_ARGB32] = FilterC::symmetric_convolve_float_v_argb32;
  m->filter.symmetric_convolve_float_h[PIXEL_FORMAT_XRGB32] = FilterC::symmetric_convolve_float_h_argb32;
  m->filter.symmetric_convolve_float_v[PIXEL_FORMAT_XRGB32] = FilterC::symmetric_convolve_float_v_argb32;
  m->filter.symmetric_convolve_float_h[PIXEL_FORMAT_RGB24] = FilterC::symmetric_convolve_float_h_rgb24;
  m->filter.symmetric_convolve_float_v[PIXEL_FORMAT_RGB24] = FilterC::symmetric_convolve_float_v_rgb24;
  m->filter.symmetric_convolve_float_h[PIXEL_FORMAT_A8] = FilterC::symmetric_convolve_float_h_a8;
  m->filter.symmetric_convolve_float_v[PIXEL_FORMAT_A8] = FilterC::symmetric_convolve_float_v_a8;

  // [Composite - Src]

  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = ConvertC::memcpy32;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = ConvertC::prgb32_from_argb32;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = ConvertC::axxx32_from_xxxx32;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_RGB24] = ConvertC::rgb32_from_rgb24;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_I8] = ConvertC::prgb32_from_i8;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_A8] = ConvertC::axxx32_from_a8;

  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_ARGB32].vspan[PIXEL_FORMAT_PRGB32] = ConvertC::argb32_from_prgb32;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_ARGB32].vspan[PIXEL_FORMAT_ARGB32] = ConvertC::memcpy32;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_ARGB32].vspan[PIXEL_FORMAT_XRGB32] = ConvertC::axxx32_from_xxxx32;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_ARGB32].vspan[PIXEL_FORMAT_RGB24] = ConvertC::rgb32_from_rgb24;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_ARGB32].vspan[PIXEL_FORMAT_I8] = ConvertC::argb32_from_i8;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_ARGB32].vspan[PIXEL_FORMAT_A8] = ConvertC::axxx32_from_a8;

  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = ConvertC::axxx32_from_xxxx32;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = ConvertC::rgb32_from_argb32;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = ConvertC::memcpy32;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_RGB24] = ConvertC::rgb32_from_rgb24;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_I8] = ConvertC::rgb32_from_i8;

  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_RGB24].vspan[PIXEL_FORMAT_XRGB32] = ConvertC::rgb24_from_rgb32;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_RGB24].vspan[PIXEL_FORMAT_PRGB32] = ConvertC::rgb24_from_rgb32;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_RGB24].vspan[PIXEL_FORMAT_I8] = ConvertC::rgb24_from_i8;

  // TODO

  // [Composite - Dst]

  fog_raster_set_nops(&m->raster[COMPOSITE_DST][PIXEL_FORMAT_ARGB32]);
  fog_raster_set_nops(&m->raster[COMPOSITE_DST][PIXEL_FORMAT_PRGB32]);
  fog_raster_set_nops(&m->raster[COMPOSITE_DST][PIXEL_FORMAT_XRGB32]);
  fog_raster_set_nops(&m->raster[COMPOSITE_DST][PIXEL_FORMAT_RGB24]);
  fog_raster_set_nops(&m->raster[COMPOSITE_DST][PIXEL_FORMAT_A8]);
  fog_raster_set_nops(&m->raster[COMPOSITE_DST][PIXEL_FORMAT_I8]);

  // [Composite - SrcOver]

  // TODO

  // [Composite - DstOver]

  fog_raster_set_nops(&m->raster[COMPOSITE_DST_OVER][PIXEL_FORMAT_XRGB32]);
  fog_raster_set_nops(&m->raster[COMPOSITE_DST_OVER][PIXEL_FORMAT_RGB24]);

  // TODO

  // [Composite - SrcIn]

  // TODO

  // [Composite - DstIn]

  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = (VSpanFn)CompositeNopC::cspan;
  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_RGB24] = (VSpanFn)CompositeNopC::cspan;

  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = (VSpanMskFn)CompositeNopC::cspan_a8;
  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = (VSpanMskFn)CompositeNopC::cspan_a8;

  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = (VSpanMskConstFn)CompositeNopC::cspan_a8_const;
  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = (VSpanMskConstFn)CompositeNopC::cspan_a8_const;

  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = (VSpanFn)CompositeNopC::cspan;
  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_RGB24] = (VSpanFn)CompositeNopC::cspan;

  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = (VSpanMskFn)CompositeNopC::cspan_a8;
  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = (VSpanMskFn)CompositeNopC::cspan_a8;

  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = (VSpanMskConstFn)CompositeNopC::cspan_a8_const;
  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = (VSpanMskConstFn)CompositeNopC::cspan_a8_const;

  // TODO

  // [Composite - SrcOut]

  // TODO

  // [Composite - DstOut]

  // TODO

  // [Composite - SrcAtop]

  // TODO

  // [Composite - DstAtop]

  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = (VSpanFn)CompositeNopC::cspan;
  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_RGB24] = (VSpanFn)CompositeNopC::cspan;

  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = (VSpanMskFn)CompositeNopC::cspan_a8;
  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = (VSpanMskFn)CompositeNopC::cspan_a8;

  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = (VSpanMskConstFn)CompositeNopC::cspan_a8_const;
  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = (VSpanMskConstFn)CompositeNopC::cspan_a8_const;

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
