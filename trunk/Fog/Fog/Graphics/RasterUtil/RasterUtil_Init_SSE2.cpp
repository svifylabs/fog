// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/Intrin_SSE2.h>
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

// [Raster_SSE2]
#include <Fog/Graphics/RasterUtil/RasterUtil_C.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Defs_SSE2.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Convert_SSE2.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Composite_SSE2.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Gradient_SSE2.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Pattern_SSE2.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Scale_SSE2.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Filters_SSE2.h>

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE void fog_raster_init_sse2(void)
{
  using namespace Fog;
  using namespace Fog::RasterUtil;

  // W masks
  Mask_0080008000800080_0080008000800080 = pix_create_mask_8x2W(0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080);
  Mask_000000FF00FF00FF_000000FF00FF00FF = pix_create_mask_8x2W(0x0000, 0x00FF, 0x00FF, 0x00FF, 0x0000, 0x00FF, 0x00FF, 0x00FF);
  Mask_000000FF00FF00FF_00FF00FF00FF00FF = pix_create_mask_8x2W(0x0000, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF);
  Mask_00FF00FF00FF00FF_000000FF00FF00FF = pix_create_mask_8x2W(0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x0000, 0x00FF, 0x00FF, 0x00FF);
  Mask_00FF00FF00FF00FF_00FF00FF00FF00FF = pix_create_mask_8x2W(0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF);
  Mask_FF000000000000FF_FF000000000000FF = pix_create_mask_8x2W(0xFF00, 0x0000, 0x0000, 0x00FF, 0xFF00, 0x0000, 0x0000, 0x00FF);
  Mask_0101010101010101_0101010101010101 = pix_create_mask_8x2W(0x0101, 0x0101, 0x0101, 0x0101, 0x0101, 0x0101, 0x0101, 0x0101);
  Mask_FFFFFFFFFFFFFFFF_FFFFFFFFFFFFFFFF = pix_create_mask_8x2W(0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF);
  Mask_00FF000000000000_00FF000000000000 = pix_create_mask_8x2W(0x00FF, 0x0000, 0x0000, 0x0000, 0x00FF, 0x0000, 0x0000, 0x0000);
  Mask_0000000000000000_00FF000000000000 = pix_create_mask_8x2W(0x0000, 0x0000, 0x0000, 0x0000, 0x00FF, 0x0000, 0x0000, 0x0000);
  Mask_00FF000000000000_0000000000000000 = pix_create_mask_8x2W(0x00FF, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000);
  Mask_FF000000FF000000_FF000000FF000000 = pix_create_mask_8x2W(0xFF00, 0x0000, 0xFF00, 0x0000, 0xFF00, 0x0000, 0xFF00, 0x0000);
  Mask_000000FF000000FF_000000FF000000FF = pix_create_mask_8x2W(0x0000, 0x00FF, 0x0000, 0x00FF, 0x0000, 0x00FF, 0x0000, 0x00FF);
  Mask_00000000000000FF_00000000000000FF = pix_create_mask_8x2W(0x0000, 0x0000, 0x0000, 0x00FF, 0x0000, 0x0000, 0x0000, 0x00FF);
  Mask_00FFFFFF00FFFFFF_00FFFFFF00FFFFFF = pix_create_mask_8x2W(0x00FF, 0xFFFF, 0x00FF, 0xFFFF, 0x00FF, 0xFFFF, 0x00FF, 0xFFFF);
  Mask_0000000100000001_0000000100000001 = pix_create_mask_8x2W(0x0000, 0x0001, 0x0000, 0x0001, 0x0000, 0x0001, 0x0000, 0x0001);

  Mask_00FF00FF00FF00FF_0000000000000000 = pix_create_mask_8x2W(0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x0000, 0x0000, 0x0000, 0x0000);
  Mask_0000000000000000_00FF00FF00FF00FF = pix_create_mask_8x2W(0x0000, 0x0000, 0x0000, 0x0000, 0x00FF, 0x00FF, 0x00FF, 0x00FF);
  Mask_0001000200020002_0001000200020002 = pix_create_mask_8x2W(0x0001, 0x0002, 0x0002, 0x0002, 0x0001, 0x0002, 0x0002, 0x0002);

  Mask_0000000000000000_FFFFFFFFFFFFFFFF = pix_create_mask_8x2W(0x0000, 0x0000, 0x0000, 0x0000, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF);
  Mask_0000000000000000_0101010101010101 = pix_create_mask_8x2W(0x0000, 0x0000, 0x0000, 0x0000, 0x0101, 0x0101, 0x0101, 0x0101);


  // D masks
  Mask_0001000000000000_0000000000000000 = pix_create_mask_4x1D(0x00010000, 0x00000000, 0x00000000, 0x00000000);
  Mask_3F80000000000000_0000000000000000 = pix_create_mask_4x1D(0x3F800000, 0x00000000, 0x00000000, 0x00000000);

  // Float / Double masks
  sse2_t t;

  t.uq[0] = ~(FOG_UINT64_C(0x8000000080000000));
  t.uq[1] = ~(FOG_UINT64_C(0x8000000080000000));
  Mask_7FFFFFFF7FFFFFFF_7FFFFFFF7FFFFFFF = _mm_loadu_ps((float*)&t);

  t.uq[0] = ~(FOG_UINT64_C(0x8000000000000000));
  t.uq[1] = ~(FOG_UINT64_C(0x8000000000000000));
  Mask_7FFFFFFFFFFFFFFF_7FFFFFFFFFFFFFFF = _mm_loadu_pd((double*)&t);

  FunctionMap* m = functionMap;

  // [Convert]

  m->convert.memcpy32 = ConvertSSE2::memcpy32;
  m->convert.memcpy24 = ConvertSSE2::memcpy24;
  m->convert.memcpy16 = ConvertSSE2::memcpy16;
  m->convert.memcpy8 = ConvertSSE2::memcpy8;

  m->convert.axxx32_from_xxxx32 = ConvertSSE2::axxx32_from_xxxx32;
  m->convert.argb32_from_prgb32 = ConvertSSE2::argb32_from_prgb32;
  m->convert.argb32_from_rgb32 = ConvertSSE2::axxx32_from_xxxx32;
  m->convert.argb32_from_i8 = ConvertSSE2::argb32_from_i8;
  m->convert.prgb32_from_argb32 = ConvertSSE2::prgb32_from_argb32;
  m->convert.prgb32_from_argb32_bs = ConvertSSE2::prgb32_from_argb32_bs;
  m->convert.prgb32_from_i8 = ConvertSSE2::prgb32_from_i8;
  m->convert.prgb32_bs_from_argb32 = ConvertSSE2::prgb32_bs_from_argb32;
  m->convert.rgb32_from_argb32 = ConvertSSE2::rgb32_from_argb32;
  m->convert.rgb32_from_argb32_bs = ConvertSSE2::rgb32_from_argb32_bs;
  m->convert.rgb32_from_rgb24 = ConvertSSE2::rgb32_from_rgb24;
  m->convert.rgb32_from_bgr24 = ConvertSSE2::rgb32_from_bgr24;

  // [Gradient - Gradient]

  m->gradient.gradient_argb32 = GradientSSE2::gradient_argb32;
  m->gradient.gradient_prgb32 = GradientSSE2::gradient_prgb32;

  // [Pattern - Texture]

  m->pattern.texture_fetch_exact_repeat[PIXEL_FORMAT_PRGB32] = PatternSSE2::texture_fetch_exact_repeat_32;
  m->pattern.texture_fetch_exact_repeat[PIXEL_FORMAT_ARGB32] = PatternSSE2::texture_fetch_exact_repeat_32;
  m->pattern.texture_fetch_exact_repeat[PIXEL_FORMAT_XRGB32] = PatternSSE2::texture_fetch_exact_repeat_32;

  m->pattern.texture_fetch_exact_reflect[PIXEL_FORMAT_PRGB32] = PatternSSE2::texture_fetch_exact_reflect_32;
  m->pattern.texture_fetch_exact_reflect[PIXEL_FORMAT_ARGB32] = PatternSSE2::texture_fetch_exact_reflect_32;
  m->pattern.texture_fetch_exact_reflect[PIXEL_FORMAT_XRGB32] = PatternSSE2::texture_fetch_exact_reflect_32;

  m->pattern.texture_fetch_transform_bilinear_repeat[PIXEL_FORMAT_PRGB32] = PatternSSE2::texture_fetch_transform_bilinear_repeat_32;
  m->pattern.texture_fetch_transform_bilinear_repeat[PIXEL_FORMAT_ARGB32] = PatternSSE2::texture_fetch_transform_bilinear_repeat_32;
  m->pattern.texture_fetch_transform_bilinear_repeat[PIXEL_FORMAT_XRGB32] = PatternSSE2::texture_fetch_transform_bilinear_repeat_32;

  // [Pattern - Linear Gradient]

  m->pattern.linear_gradient_fetch_subxy_pad = PatternSSE2::linear_gradient_fetch_subxy_pad;
  m->pattern.linear_gradient_fetch_subxy_repeat = PatternSSE2::linear_gradient_fetch_subxy_repeat;

  // [Pattern - Radial Gradient]

  m->pattern.radial_gradient_fetch_pad = PatternSSE2::radial_gradient_fetch_pad;

  // [Filters - ColorMatrix]

  m->filter.color_matrix[PIXEL_FORMAT_PRGB32] = FilterSSE2::color_matrix_prgb32;
  m->filter.color_matrix[PIXEL_FORMAT_ARGB32] = FilterSSE2::color_matrix_argb32;
  m->filter.color_matrix[PIXEL_FORMAT_XRGB32] = FilterSSE2::color_matrix_xrgb32;
  m->filter.color_matrix[PIXEL_FORMAT_RGB24] = FilterSSE2::color_matrix_rgb24;
  m->filter.color_matrix[PIXEL_FORMAT_A8] = FilterSSE2::color_matrix_a8;

  // [Filters - BoxBlur]

  m->filter.box_blur_h[PIXEL_FORMAT_ARGB32] = FilterSSE2::box_blur_h_argb32;
  m->filter.box_blur_v[PIXEL_FORMAT_ARGB32] = FilterSSE2::box_blur_v_argb32;

  m->filter.box_blur_h[PIXEL_FORMAT_XRGB32] = FilterSSE2::box_blur_h_xrgb32;
  m->filter.box_blur_v[PIXEL_FORMAT_XRGB32] = FilterSSE2::box_blur_v_xrgb32;

  // [Filters - LinearBlur]

  m->filter.linear_blur_h[PIXEL_FORMAT_ARGB32] = FilterSSE2::linear_blur_h_argb32;
  m->filter.linear_blur_v[PIXEL_FORMAT_ARGB32] = FilterSSE2::linear_blur_v_argb32;

  // [Filters - SymmetricConvolveFloat]

  m->filter.symmetric_convolve_float_h[PIXEL_FORMAT_ARGB32] = FilterSSE2::symmetric_convolve_float_h_argb32;
  m->filter.symmetric_convolve_float_v[PIXEL_FORMAT_ARGB32] = FilterSSE2::symmetric_convolve_float_v_argb32;

  // [Composite - Src]

  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].pixel = CompositeSrcSSE2::prgb32_pixel;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].pixel_a8 = CompositeSrcSSE2::prgb32_pixel_a8;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].cspan = CompositeSrcSSE2::prgb32_cspan;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeSrcSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeSrcSSE2::prgb32_cspan_a8_const;

  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = ConvertSSE2::prgb32_from_argb32;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = ConvertSSE2::memcpy32;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = ConvertSSE2::axxx32_from_xxxx32;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_RGB24] = ConvertSSE2::rgb32_from_rgb24;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_A8] = ConvertSSE2::prgb32_from_a8;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_I8] = ConvertSSE2::prgb32_from_i8;

  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeSrcSSE2::prgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeSrcSSE2::prgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeSrcSSE2::prgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeSrcSSE2::prgb32_vspan_rgb24_a8;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_A8] = CompositeSrcSSE2::prgb32_vspan_a8_a8;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_I8] = CompositeSrcSSE2::prgb32_vspan_i8_a8;

  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeSrcSSE2::prgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeSrcSSE2::prgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeSrcSSE2::prgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeSrcSSE2::prgb32_vspan_rgb24_a8_const;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_A8] = CompositeSrcSSE2::prgb32_vspan_a8_a8_const;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_I8] = CompositeSrcSSE2::prgb32_vspan_i8_a8_const;

  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].pixel = CompositeSrcSSE2::xrgb32_pixel;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].pixel_a8 = CompositeSrcSSE2::xrgb32_pixel_a8;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].cspan = CompositeSrcSSE2::xrgb32_cspan;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeSrcSSE2::xrgb32_cspan_a8;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeSrcSSE2::xrgb32_cspan_a8_const;

  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = ConvertSSE2::rgb32_from_argb32;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = ConvertSSE2::axxx32_from_xxxx32;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = ConvertSSE2::memcpy32;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_RGB24] = ConvertSSE2::rgb32_from_rgb24;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_A8] = (VSpanFn)CompositeClearSSE2::xrgb32_cspan;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_I8] = ConvertSSE2::rgb32_from_i8;

  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeSrcSSE2::xrgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeSrcSSE2::xrgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeSrcSSE2::xrgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeSrcSSE2::xrgb32_vspan_rgb24_a8;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_A8] = (VSpanMskFn)CompositeClearSSE2::xrgb32_cspan_a8;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_I8] = CompositeSrcSSE2::xrgb32_vspan_i8_a8;

  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeSrcSSE2::xrgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeSrcSSE2::xrgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeSrcSSE2::xrgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeSrcSSE2::xrgb32_vspan_rgb24_a8_const;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_A8] = (VSpanMskConstFn)CompositeClearSSE2::xrgb32_cspan_a8_const;
  m->raster[COMPOSITE_SRC][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_I8] = CompositeSrcSSE2::xrgb32_vspan_i8_a8_const;

  // [Composite - Dst]

  // PIXEL_FORMAT_PRGB32 - NOP (already set by Raster_C).
  // PIXEL_FORMAT_XRGB32 - NOP (already set by Raster_C).
  // PIXEL_FORMAT_RGB24  - NOP (already set by Raster_C).
  // PIXEL_FORMAT_A8     - NOP (already set by Raster_C).

  // [Composite - SrcOver]

  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_PRGB32].pixel = CompositeSrcOverSSE2::prgb32_pixel;
  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_PRGB32].pixel_a8 = CompositeSrcOverSSE2::prgb32_pixel_a8;
  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_PRGB32].cspan = CompositeSrcOverSSE2::prgb32_cspan;
  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeSrcOverSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeSrcOverSSE2::prgb32_cspan_a8_const;

  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeSrcOverSSE2::prgb32_vspan_prgb32;
  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeSrcOverSSE2::prgb32_vspan_argb32;
  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = ConvertSSE2::axxx32_from_xxxx32;
  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_RGB24] = ConvertSSE2::rgb32_from_rgb24;

  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeSrcOverSSE2::prgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeSrcOverSSE2::prgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeSrcOverSSE2::prgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeSrcOverSSE2::prgb32_vspan_rgb24_a8;

  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeSrcOverSSE2::prgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeSrcOverSSE2::prgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeSrcOverSSE2::prgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeSrcOverSSE2::prgb32_vspan_rgb24_a8_const;

  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_XRGB32].pixel = CompositeSrcOverSSE2::xrgb32_pixel;
  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_XRGB32].pixel_a8 = CompositeSrcOverSSE2::xrgb32_pixel_a8;
  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_XRGB32].cspan = CompositeSrcOverSSE2::xrgb32_cspan;
  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeSrcOverSSE2::xrgb32_cspan_a8;
  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeSrcOverSSE2::xrgb32_cspan_a8_const;

  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeSrcOverSSE2::xrgb32_vspan_argb32;
  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeSrcOverSSE2::xrgb32_vspan_prgb32;
  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = ConvertSSE2::memcpy32;
  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_RGB24] = ConvertSSE2::rgb32_from_rgb24;

  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeSrcOverSSE2::xrgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeSrcOverSSE2::xrgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeSrcSSE2::xrgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeSrcSSE2::xrgb32_vspan_rgb24_a8;

  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeSrcOverSSE2::xrgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeSrcOverSSE2::xrgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeSrcSSE2::xrgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_SRC_OVER][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeSrcSSE2::xrgb32_vspan_rgb24_a8_const;

  // [Composite - DstOver]

  m->raster[COMPOSITE_DST_OVER][PIXEL_FORMAT_PRGB32].pixel = CompositeDstOverSSE2::prgb32_pixel;
  m->raster[COMPOSITE_DST_OVER][PIXEL_FORMAT_PRGB32].pixel_a8 = CompositeDstOverSSE2::prgb32_pixel_a8;
  m->raster[COMPOSITE_DST_OVER][PIXEL_FORMAT_PRGB32].cspan = CompositeDstOverSSE2::prgb32_cspan;
  m->raster[COMPOSITE_DST_OVER][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeDstOverSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_DST_OVER][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeDstOverSSE2::prgb32_cspan_a8_const;

  m->raster[COMPOSITE_DST_OVER][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeDstOverSSE2::prgb32_vspan_prgb32;
  m->raster[COMPOSITE_DST_OVER][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeDstOverSSE2::prgb32_vspan_argb32;
  m->raster[COMPOSITE_DST_OVER][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeDstOverSSE2::prgb32_vspan_xrgb32;
  m->raster[COMPOSITE_DST_OVER][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_RGB24] = CompositeDstOverSSE2::prgb32_vspan_rgb24;

  m->raster[COMPOSITE_DST_OVER][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeDstOverSSE2::prgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_DST_OVER][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeDstOverSSE2::prgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_DST_OVER][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeDstOverSSE2::prgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_DST_OVER][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeDstOverSSE2::prgb32_vspan_rgb24_a8;

  m->raster[COMPOSITE_DST_OVER][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeDstOverSSE2::prgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_DST_OVER][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeDstOverSSE2::prgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_DST_OVER][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeDstOverSSE2::prgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_DST_OVER][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeDstOverSSE2::prgb32_vspan_rgb24_a8_const;

  // PIXEL_FORMAT_XRGB32 - NOP (already set by Raster_C).
  // PIXEL_FORMAT_RGB24  - NOP (already set by Raster_C).

  // [Composite - SrcIn]

  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_PRGB32].pixel = CompositeSrcInSSE2::prgb32_pixel;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_PRGB32].pixel_a8 = CompositeSrcInSSE2::prgb32_pixel_a8;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_PRGB32].cspan = CompositeSrcInSSE2::prgb32_cspan;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeSrcInSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeSrcInSSE2::prgb32_cspan_a8_const;

  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeSrcInSSE2::prgb32_vspan_prgb32;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeSrcInSSE2::prgb32_vspan_argb32;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeSrcInSSE2::prgb32_vspan_xrgb32;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_RGB24] = CompositeSrcInSSE2::prgb32_vspan_rgb24;

  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeSrcInSSE2::prgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeSrcInSSE2::prgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeSrcInSSE2::prgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeSrcInSSE2::prgb32_vspan_rgb24_a8;

  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeSrcInSSE2::prgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeSrcInSSE2::prgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeSrcInSSE2::prgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeSrcInSSE2::prgb32_vspan_rgb24_a8_const;

  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_XRGB32].pixel = CompositeSrcSSE2::xrgb32_pixel;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_XRGB32].pixel_a8 = CompositeSrcSSE2::xrgb32_pixel_a8;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_XRGB32].cspan = CompositeSrcSSE2::xrgb32_cspan;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeSrcSSE2::xrgb32_cspan_a8;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeSrcSSE2::xrgb32_cspan_a8_const;

  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = ConvertSSE2::rgb32_from_argb32;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = ConvertSSE2::axxx32_from_xxxx32;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = ConvertSSE2::memcpy32;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_RGB24] = ConvertSSE2::rgb32_from_rgb24;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_A8] = (VSpanFn)CompositeClearSSE2::xrgb32_cspan;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_I8] = ConvertSSE2::rgb32_from_i8;

  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeSrcSSE2::xrgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeSrcSSE2::xrgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeSrcSSE2::xrgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeSrcSSE2::xrgb32_vspan_rgb24_a8;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_A8] = (VSpanMskFn)CompositeClearSSE2::xrgb32_cspan_a8;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_I8] = CompositeSrcSSE2::xrgb32_vspan_i8_a8;

  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeSrcSSE2::xrgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeSrcSSE2::xrgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeSrcSSE2::xrgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeSrcSSE2::xrgb32_vspan_rgb24_a8_const;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_A8] = (VSpanMskConstFn)CompositeClearSSE2::xrgb32_cspan_a8_const;
  m->raster[COMPOSITE_SRC_IN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_I8] = CompositeSrcSSE2::xrgb32_vspan_i8_a8_const;

  // [Composite - DstIn]

  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_PRGB32].pixel = CompositeDstInSSE2::prgb32_pixel;
  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_PRGB32].pixel_a8 = CompositeDstInSSE2::prgb32_pixel_a8;
  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_PRGB32].cspan = CompositeDstInSSE2::prgb32_cspan;
  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeDstInSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeDstInSSE2::prgb32_cspan_a8_const;

  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32;
  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32;
//m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = NOP;
//m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_RGB24] = NOP;

  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32_a8;
  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32_a8;
//m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = NOP;
//m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = NOP;

  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32_a8_const;
  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32_a8_const;
//m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = NOP;
//m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = NOP;

  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_XRGB32].pixel = CompositeDstInSSE2::prgb32_pixel;
  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_XRGB32].pixel_a8 = CompositeDstInSSE2::prgb32_pixel_a8;
  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_XRGB32].cspan = CompositeDstInSSE2::prgb32_cspan;
  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeDstInSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeDstInSSE2::prgb32_cspan_a8_const;

  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32;
  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32;
//m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = NOP;
//m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_RGB24] = NOP;

  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32_a8;
  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32_a8;
//m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = NOP;
//m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = NOP;

  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32_a8_const;
  m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32_a8_const;
//m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = NOP;
//m->raster[COMPOSITE_DST_IN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = NOP;

  // [Composite - SrcOut]

  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_PRGB32].pixel = CompositeSrcOutSSE2::prgb32_pixel;
  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_PRGB32].pixel_a8 = CompositeSrcOutSSE2::prgb32_pixel_a8;
  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_PRGB32].cspan = CompositeSrcOutSSE2::prgb32_cspan;
  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeSrcOutSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeSrcOutSSE2::prgb32_cspan_a8_const;

  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeSrcOutSSE2::prgb32_vspan_prgb32;
  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeSrcOutSSE2::prgb32_vspan_argb32;
  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeSrcOutSSE2::prgb32_vspan_xrgb32;
  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_RGB24] = CompositeSrcOutSSE2::prgb32_vspan_rgb24;

  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeSrcOutSSE2::prgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeSrcOutSSE2::prgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeSrcOutSSE2::prgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeSrcOutSSE2::prgb32_vspan_rgb24_a8;

  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeSrcOutSSE2::prgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeSrcOutSSE2::prgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeSrcOutSSE2::prgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeSrcOutSSE2::prgb32_vspan_rgb24_a8_const;

  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_XRGB32].pixel = CompositeClearSSE2::prgb32_pixel;
  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_XRGB32].pixel_a8 = CompositeClearSSE2::prgb32_pixel_a8;
  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_XRGB32].cspan = CompositeClearSSE2::prgb32_cspan;
  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeClearSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeClearSSE2::prgb32_cspan_a8_const;

  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = (VSpanFn)CompositeClearSSE2::prgb32_cspan;
  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = (VSpanFn)CompositeClearSSE2::prgb32_cspan;
  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = (VSpanFn)CompositeClearSSE2::prgb32_cspan;
  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_RGB24] = (VSpanFn)CompositeClearSSE2::prgb32_cspan;

  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = (VSpanMskFn)CompositeClearSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = (VSpanMskFn)CompositeClearSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = (VSpanMskFn)CompositeClearSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = (VSpanMskFn)CompositeClearSSE2::prgb32_cspan_a8;

  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = (VSpanMskConstFn)CompositeClearSSE2::prgb32_cspan_a8_const;
  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = (VSpanMskConstFn)CompositeClearSSE2::prgb32_cspan_a8_const;
  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = (VSpanMskConstFn)CompositeClearSSE2::prgb32_cspan_a8_const;
  m->raster[COMPOSITE_SRC_OUT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = (VSpanMskConstFn)CompositeClearSSE2::prgb32_cspan_a8_const;

  // [Composite - DstOut]

  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_PRGB32].pixel = CompositeDstOutSSE2::prgb32_pixel;
  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_PRGB32].pixel_a8 = CompositeDstOutSSE2::prgb32_pixel_a8;
  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_PRGB32].cspan = CompositeDstOutSSE2::prgb32_cspan;
  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeDstOutSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeDstOutSSE2::prgb32_cspan_a8_const;

  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32;
  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32;
  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = (VSpanFn)CompositeClearSSE2::prgb32_cspan;
  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_RGB24] = (VSpanFn)CompositeClearSSE2::prgb32_cspan;

  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32_a8;
  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32_a8;
  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = (VSpanMskFn)CompositeClearSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = (VSpanMskFn)CompositeClearSSE2::prgb32_cspan_a8;

  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32_a8_const;
  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32_a8_const;
  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = (VSpanMskConstFn)CompositeClearSSE2::prgb32_cspan_a8_const;
  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = (VSpanMskConstFn)CompositeClearSSE2::prgb32_cspan_a8_const;

  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_XRGB32].pixel = CompositeDstOutSSE2::prgb32_pixel;
  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_XRGB32].pixel_a8 = CompositeDstOutSSE2::prgb32_pixel_a8;
  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_XRGB32].cspan = CompositeDstOutSSE2::prgb32_cspan;
  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeDstOutSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeDstOutSSE2::prgb32_cspan_a8_const;

  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32;
  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32;
  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = (VSpanFn)CompositeClearSSE2::prgb32_cspan;
  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_RGB24] = (VSpanFn)CompositeClearSSE2::prgb32_cspan;

  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32_a8;
  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32_a8;
  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = (VSpanMskFn)CompositeClearSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = (VSpanMskFn)CompositeClearSSE2::prgb32_cspan_a8;

  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32_a8_const;
  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32_a8_const;
  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = (VSpanMskConstFn)CompositeClearSSE2::prgb32_cspan_a8_const;
  m->raster[COMPOSITE_DST_OUT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = (VSpanMskConstFn)CompositeClearSSE2::prgb32_cspan_a8_const;

  // [Composite - SrcAtop]

  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_PRGB32].pixel = CompositeSrcAtopSSE2::prgb32_pixel;
  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_PRGB32].pixel_a8 = CompositeSrcAtopSSE2::prgb32_pixel_a8;
  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_PRGB32].cspan = CompositeSrcAtopSSE2::prgb32_cspan;
  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeSrcAtopSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeSrcAtopSSE2::prgb32_cspan_a8_const;

  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeSrcAtopSSE2::prgb32_vspan_prgb32;
  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeSrcAtopSSE2::prgb32_vspan_argb32;
  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeSrcInSSE2::prgb32_vspan_xrgb32;
  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_RGB24] = CompositeSrcInSSE2::prgb32_vspan_rgb24;

  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeSrcAtopSSE2::prgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeSrcAtopSSE2::prgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeSrcInSSE2::prgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeSrcInSSE2::prgb32_vspan_rgb24_a8;

  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeSrcAtopSSE2::prgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeSrcAtopSSE2::prgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeSrcInSSE2::prgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeSrcInSSE2::prgb32_vspan_rgb24_a8_const;

  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_XRGB32].pixel = CompositeSrcOverSSE2::xrgb32_pixel;
  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_XRGB32].pixel_a8 = CompositeSrcOverSSE2::xrgb32_pixel_a8;
  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_XRGB32].cspan = CompositeSrcOverSSE2::xrgb32_cspan;
  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeSrcOverSSE2::xrgb32_cspan_a8;
  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeSrcOverSSE2::xrgb32_cspan_a8_const;

  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeSrcOverSSE2::xrgb32_vspan_prgb32;
  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeSrcOverSSE2::xrgb32_vspan_argb32;
  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = ConvertSSE2::memcpy32;
  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_RGB24] = ConvertSSE2::rgb32_from_rgb24;

  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeSrcOverSSE2::xrgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeSrcOverSSE2::xrgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeSrcSSE2::xrgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeSrcSSE2::xrgb32_vspan_rgb24_a8;

  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeSrcOverSSE2::xrgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeSrcOverSSE2::xrgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeSrcSSE2::xrgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_SRC_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeSrcSSE2::xrgb32_vspan_rgb24_a8_const;

  // PIXEL_FORMAT_A8     - NOP (already set by Raster_C).

  // [Composite - DstAtop]

  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_PRGB32].pixel = CompositeDstAtopSSE2::prgb32_pixel;
  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_PRGB32].pixel_a8 = CompositeDstAtopSSE2::prgb32_pixel_a8;
  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_PRGB32].cspan = CompositeDstAtopSSE2::prgb32_cspan;
  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeDstAtopSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeDstAtopSSE2::prgb32_cspan_a8_const;

  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeDstAtopSSE2::prgb32_vspan_prgb32;
  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeDstAtopSSE2::prgb32_vspan_argb32;
  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeDstOverSSE2::prgb32_vspan_xrgb32;
  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_RGB24] = CompositeDstOverSSE2::prgb32_vspan_rgb24;

  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeDstAtopSSE2::prgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeDstAtopSSE2::prgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeDstOverSSE2::prgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeDstOverSSE2::prgb32_vspan_rgb24_a8;

  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeDstAtopSSE2::prgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeDstAtopSSE2::prgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeDstOverSSE2::prgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeDstOverSSE2::prgb32_vspan_rgb24_a8_const;

  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_XRGB32].pixel = CompositeDstInSSE2::prgb32_pixel;
  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_XRGB32].pixel_a8 = CompositeDstInSSE2::prgb32_pixel_a8;
  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_XRGB32].cspan = CompositeDstInSSE2::prgb32_cspan;
  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeDstInSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeDstInSSE2::prgb32_cspan_a8_const;

  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32;
  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32;
//m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = NOP;
//m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_RGB24] = NOP;

  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32_a8;
  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32_a8;
//m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = NOP;
//m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = NOP;

  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32_a8_const;
  m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32_a8_const;
//m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = NOP;
//m->raster[COMPOSITE_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = NOP;

  // [Composite - Xor]

  // TODO

  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_PRGB32].pixel = CompositeXorSSE2::prgb32_pixel;
  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_PRGB32].pixel_a8 = CompositeXorSSE2::prgb32_pixel_a8;
  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_PRGB32].cspan = CompositeXorSSE2::prgb32_cspan;
  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeXorSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeXorSSE2::prgb32_cspan_a8_const;

  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeXorSSE2::prgb32_vspan_prgb32;
  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeXorSSE2::prgb32_vspan_argb32;
  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeSrcOutSSE2::prgb32_vspan_xrgb32;
  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_RGB24] = CompositeSrcOutSSE2::prgb32_vspan_rgb24;

  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeXorSSE2::prgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeXorSSE2::prgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeSrcOutSSE2::prgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeSrcOutSSE2::prgb32_vspan_rgb24_a8;

  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeXorSSE2::prgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeXorSSE2::prgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeSrcOutSSE2::prgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeSrcOutSSE2::prgb32_vspan_rgb24_a8_const;

  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_XRGB32].pixel = CompositeDstOutSSE2::prgb32_pixel;
  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_XRGB32].pixel_a8 = CompositeDstOutSSE2::prgb32_pixel_a8;
  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_XRGB32].cspan = CompositeDstOutSSE2::prgb32_cspan;
  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeDstOutSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeDstOutSSE2::prgb32_cspan_a8_const;

  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32;
  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32;
  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = (VSpanFn)CompositeClearSSE2::xrgb32_cspan;
  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_RGB24] = (VSpanFn)CompositeClearSSE2::xrgb32_cspan;

  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32_a8;
  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32_a8;
  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = (VSpanMskFn)CompositeClearSSE2::xrgb32_cspan_a8;
  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = (VSpanMskFn)CompositeClearSSE2::xrgb32_cspan_a8;

  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32_a8_const;
  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32_a8_const;
  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = (VSpanMskConstFn)CompositeClearSSE2::xrgb32_cspan_a8_const;
  m->raster[COMPOSITE_XOR][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = (VSpanMskConstFn)CompositeClearSSE2::xrgb32_cspan_a8_const;

  // [Composite - Clear]

  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_PRGB32].pixel = CompositeClearSSE2::prgb32_pixel;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_PRGB32].pixel_a8 = CompositeClearSSE2::prgb32_pixel_a8;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_PRGB32].cspan = CompositeClearSSE2::prgb32_cspan;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeClearSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeClearSSE2::prgb32_cspan_a8_const;

  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = (VSpanFn)CompositeClearSSE2::prgb32_cspan;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = (VSpanFn)CompositeClearSSE2::prgb32_cspan;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = (VSpanFn)CompositeClearSSE2::prgb32_cspan;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_RGB24] = (VSpanFn)CompositeClearSSE2::prgb32_cspan;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_A8] = (VSpanFn)CompositeClearSSE2::prgb32_cspan;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_I8] = (VSpanFn)CompositeClearSSE2::prgb32_cspan;

  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = (VSpanMskFn)CompositeClearSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = (VSpanMskFn)CompositeClearSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = (VSpanMskFn)CompositeClearSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = (VSpanMskFn)CompositeClearSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_A8] = (VSpanMskFn)CompositeClearSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_I8] = (VSpanMskFn)CompositeClearSSE2::prgb32_cspan_a8;

  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = (VSpanMskConstFn)CompositeClearSSE2::prgb32_cspan_a8_const;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = (VSpanMskConstFn)CompositeClearSSE2::prgb32_cspan_a8_const;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = (VSpanMskConstFn)CompositeClearSSE2::prgb32_cspan_a8_const;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = (VSpanMskConstFn)CompositeClearSSE2::prgb32_cspan_a8_const;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_A8] = (VSpanMskConstFn)CompositeClearSSE2::prgb32_cspan_a8_const;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_I8] = (VSpanMskConstFn)CompositeClearSSE2::prgb32_cspan_a8_const;

  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_XRGB32].pixel = CompositeClearSSE2::xrgb32_pixel;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_XRGB32].pixel_a8 = CompositeClearSSE2::xrgb32_pixel_a8;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_XRGB32].cspan = CompositeClearSSE2::xrgb32_cspan;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeClearSSE2::xrgb32_cspan_a8;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeClearSSE2::xrgb32_cspan_a8_const;

  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = (VSpanFn)CompositeClearSSE2::xrgb32_cspan;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = (VSpanFn)CompositeClearSSE2::xrgb32_cspan;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = (VSpanFn)CompositeClearSSE2::xrgb32_cspan;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_RGB24] = (VSpanFn)CompositeClearSSE2::xrgb32_cspan;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_A8] = (VSpanFn)CompositeClearSSE2::xrgb32_cspan;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_I8] = (VSpanFn)CompositeClearSSE2::xrgb32_cspan;

  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = (VSpanMskFn)CompositeClearSSE2::xrgb32_cspan_a8;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = (VSpanMskFn)CompositeClearSSE2::xrgb32_cspan_a8;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = (VSpanMskFn)CompositeClearSSE2::xrgb32_cspan_a8;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = (VSpanMskFn)CompositeClearSSE2::xrgb32_cspan_a8;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_A8] = (VSpanMskFn)CompositeClearSSE2::xrgb32_cspan_a8;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_I8] = (VSpanMskFn)CompositeClearSSE2::xrgb32_cspan_a8;

  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = (VSpanMskConstFn)CompositeClearSSE2::xrgb32_cspan_a8_const;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = (VSpanMskConstFn)CompositeClearSSE2::xrgb32_cspan_a8_const;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = (VSpanMskConstFn)CompositeClearSSE2::xrgb32_cspan_a8_const;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = (VSpanMskConstFn)CompositeClearSSE2::xrgb32_cspan_a8_const;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_A8] = (VSpanMskConstFn)CompositeClearSSE2::xrgb32_cspan_a8_const;
  m->raster[COMPOSITE_CLEAR][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_I8] = (VSpanMskConstFn)CompositeClearSSE2::xrgb32_cspan_a8_const;

  // [Composite - Add]

  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_PRGB32].pixel = CompositeAddSSE2::prgb32_pixel;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_PRGB32].pixel_a8 = CompositeAddSSE2::prgb32_pixel_a8;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_PRGB32].cspan = CompositeAddSSE2::prgb32_cspan;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeAddSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeAddSSE2::prgb32_cspan_a8_const;

  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeAddSSE2::prgb32_vspan_prgb32;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeAddSSE2::prgb32_vspan_argb32;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeAddSSE2::prgb32_vspan_xrgb32;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_RGB24] = CompositeAddSSE2::prgb32_vspan_rgb24;

  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeAddSSE2::prgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeAddSSE2::prgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeAddSSE2::prgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeAddSSE2::prgb32_vspan_rgb24_a8;

  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeAddSSE2::prgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeAddSSE2::prgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeAddSSE2::prgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeAddSSE2::prgb32_vspan_rgb24_a8_const;

  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_XRGB32].pixel = CompositeAddSSE2::xrgb32_pixel;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_XRGB32].pixel_a8 = CompositeAddSSE2::xrgb32_pixel_a8;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_XRGB32].cspan = CompositeAddSSE2::xrgb32_cspan;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeAddSSE2::xrgb32_cspan_a8;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeAddSSE2::xrgb32_cspan_a8_const;

  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeAddSSE2::xrgb32_vspan_prgb32;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeAddSSE2::xrgb32_vspan_argb32;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeAddSSE2::xrgb32_vspan_xrgb32;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_RGB24] = CompositeAddSSE2::xrgb32_vspan_rgb24;

  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeAddSSE2::xrgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeAddSSE2::xrgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeAddSSE2::xrgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeAddSSE2::xrgb32_vspan_rgb24_a8;

  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeAddSSE2::xrgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeAddSSE2::xrgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeAddSSE2::xrgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_ADD][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeAddSSE2::xrgb32_vspan_rgb24_a8_const;

  // [Composite - Subtract]

  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_PRGB32].pixel = CompositeSubtractSSE2::prgb32_pixel;
  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_PRGB32].pixel_a8 = CompositeSubtractSSE2::prgb32_pixel_a8;
  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_PRGB32].cspan = CompositeSubtractSSE2::prgb32_cspan;
  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeSubtractSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeSubtractSSE2::prgb32_cspan_a8_const;

  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeSubtractSSE2::prgb32_vspan_prgb32;
  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeSubtractSSE2::prgb32_vspan_argb32;
  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeSubtractSSE2::prgb32_vspan_xrgb32;
  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_RGB24] = CompositeSubtractSSE2::prgb32_vspan_rgb24;

  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeSubtractSSE2::prgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeSubtractSSE2::prgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeSubtractSSE2::prgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeSubtractSSE2::prgb32_vspan_rgb24_a8;

  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeSubtractSSE2::prgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeSubtractSSE2::prgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeSubtractSSE2::prgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeSubtractSSE2::prgb32_vspan_rgb24_a8_const;

  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_XRGB32].pixel = CompositeSubtractSSE2::xrgb32_pixel;
  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_XRGB32].pixel_a8 = CompositeSubtractSSE2::xrgb32_pixel_a8;
  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_XRGB32].cspan = CompositeSubtractSSE2::xrgb32_cspan;
  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeSubtractSSE2::xrgb32_cspan_a8;
  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeSubtractSSE2::xrgb32_cspan_a8_const;

  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeSubtractSSE2::xrgb32_vspan_prgb32;
  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeSubtractSSE2::xrgb32_vspan_argb32;
  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeSubtractSSE2::xrgb32_vspan_xrgb32;
  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_RGB24] = CompositeSubtractSSE2::xrgb32_vspan_rgb24;

  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeSubtractSSE2::xrgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeSubtractSSE2::xrgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeSubtractSSE2::xrgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeSubtractSSE2::xrgb32_vspan_rgb24_a8;

  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeSubtractSSE2::xrgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeSubtractSSE2::xrgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeSubtractSSE2::xrgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_SUBTRACT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeSubtractSSE2::xrgb32_vspan_rgb24_a8_const;

  // [Composite - Screen]

  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_PRGB32].pixel = CompositeScreenSSE2::prgb32_pixel;
  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_PRGB32].pixel_a8 = CompositeScreenSSE2::prgb32_pixel_a8;
  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_PRGB32].cspan = CompositeScreenSSE2::prgb32_cspan;
  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeScreenSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeScreenSSE2::prgb32_cspan_a8_const;

  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeScreenSSE2::prgb32_vspan_prgb32;
  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeScreenSSE2::prgb32_vspan_argb32;
  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeScreenSSE2::prgb32_vspan_xrgb32;
  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_RGB24] = CompositeScreenSSE2::prgb32_vspan_rgb24;

  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeScreenSSE2::prgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeScreenSSE2::prgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeScreenSSE2::prgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeScreenSSE2::prgb32_vspan_rgb24_a8;

  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeScreenSSE2::prgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeScreenSSE2::prgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeScreenSSE2::prgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeScreenSSE2::prgb32_vspan_rgb24_a8_const;

  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_XRGB32].pixel = CompositeScreenSSE2::xrgb32_pixel;
  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_XRGB32].pixel_a8 = CompositeScreenSSE2::xrgb32_pixel_a8;
  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_XRGB32].cspan = CompositeScreenSSE2::xrgb32_cspan;
  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeScreenSSE2::xrgb32_cspan_a8;
  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeScreenSSE2::xrgb32_cspan_a8_const;

  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeScreenSSE2::xrgb32_vspan_prgb32;
  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeScreenSSE2::xrgb32_vspan_argb32;
  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeScreenSSE2::xrgb32_vspan_xrgb32;
  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_RGB24] = CompositeScreenSSE2::xrgb32_vspan_rgb24;

  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeScreenSSE2::xrgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeScreenSSE2::xrgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeScreenSSE2::xrgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeScreenSSE2::xrgb32_vspan_rgb24_a8;

  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeScreenSSE2::xrgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeScreenSSE2::xrgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeScreenSSE2::xrgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_SCREEN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeScreenSSE2::xrgb32_vspan_rgb24_a8_const;

  // [Composite - Darken]

  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_PRGB32].pixel = CompositeDarkenSSE2::prgb32_pixel;
  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_PRGB32].pixel_a8 = CompositeDarkenSSE2::prgb32_pixel_a8;
  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_PRGB32].cspan = CompositeDarkenSSE2::prgb32_cspan;
  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeDarkenSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeDarkenSSE2::prgb32_cspan_a8_const;

  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeDarkenSSE2::prgb32_vspan_prgb32;
  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeDarkenSSE2::prgb32_vspan_argb32;
  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeDarkenSSE2::prgb32_vspan_xrgb32;
  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_RGB24] = CompositeDarkenSSE2::prgb32_vspan_rgb24;

  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeDarkenSSE2::prgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeDarkenSSE2::prgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeDarkenSSE2::prgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeDarkenSSE2::prgb32_vspan_rgb24_a8;

  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeDarkenSSE2::prgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeDarkenSSE2::prgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeDarkenSSE2::prgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeDarkenSSE2::prgb32_vspan_rgb24_a8_const;

  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_XRGB32].pixel = CompositeDarkenSSE2::xrgb32_pixel;
  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_XRGB32].pixel_a8 = CompositeDarkenSSE2::xrgb32_pixel_a8;
  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_XRGB32].cspan = CompositeDarkenSSE2::xrgb32_cspan;
  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeDarkenSSE2::xrgb32_cspan_a8;
  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeDarkenSSE2::xrgb32_cspan_a8_const;

  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeDarkenSSE2::xrgb32_vspan_prgb32;
  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeDarkenSSE2::xrgb32_vspan_argb32;
  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeDarkenSSE2::xrgb32_vspan_xrgb32;
  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_RGB24] = CompositeDarkenSSE2::xrgb32_vspan_rgb24;

  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeDarkenSSE2::xrgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeDarkenSSE2::xrgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeDarkenSSE2::xrgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeDarkenSSE2::xrgb32_vspan_rgb24_a8;

  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeDarkenSSE2::xrgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeDarkenSSE2::xrgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeDarkenSSE2::xrgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_DARKEN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeDarkenSSE2::xrgb32_vspan_rgb24_a8_const;

  // [Composite - Lighten]

  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_PRGB32].pixel = CompositeLightenSSE2::prgb32_pixel;
  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_PRGB32].pixel_a8 = CompositeLightenSSE2::prgb32_pixel_a8;
  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_PRGB32].cspan = CompositeLightenSSE2::prgb32_cspan;
  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeLightenSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeLightenSSE2::prgb32_cspan_a8_const;

  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeLightenSSE2::prgb32_vspan_prgb32;
  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeLightenSSE2::prgb32_vspan_argb32;
  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeLightenSSE2::prgb32_vspan_xrgb32;
  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_RGB24] = CompositeLightenSSE2::prgb32_vspan_rgb24;

  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeLightenSSE2::prgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeLightenSSE2::prgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeLightenSSE2::prgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeLightenSSE2::prgb32_vspan_rgb24_a8;

  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeLightenSSE2::prgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeLightenSSE2::prgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeLightenSSE2::prgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeLightenSSE2::prgb32_vspan_rgb24_a8_const;

  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_XRGB32].pixel = CompositeLightenSSE2::xrgb32_pixel;
  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_XRGB32].pixel_a8 = CompositeLightenSSE2::xrgb32_pixel_a8;
  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_XRGB32].cspan = CompositeLightenSSE2::xrgb32_cspan;
  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeLightenSSE2::xrgb32_cspan_a8;
  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeLightenSSE2::xrgb32_cspan_a8_const;

  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeLightenSSE2::xrgb32_vspan_prgb32;
  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeLightenSSE2::xrgb32_vspan_argb32;
  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeLightenSSE2::xrgb32_vspan_xrgb32;
  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_RGB24] = CompositeLightenSSE2::xrgb32_vspan_rgb24;

  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeLightenSSE2::xrgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeLightenSSE2::xrgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeLightenSSE2::xrgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeLightenSSE2::xrgb32_vspan_rgb24_a8;

  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeLightenSSE2::xrgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeLightenSSE2::xrgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeLightenSSE2::xrgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_LIGHTEN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeLightenSSE2::xrgb32_vspan_rgb24_a8_const;

  // [Composite - Difference]

  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_PRGB32].pixel = CompositeDifferenceSSE2::prgb32_pixel;
  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_PRGB32].pixel_a8 = CompositeDifferenceSSE2::prgb32_pixel_a8;
  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_PRGB32].cspan = CompositeDifferenceSSE2::prgb32_cspan;
  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeDifferenceSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeDifferenceSSE2::prgb32_cspan_a8_const;

  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeDifferenceSSE2::prgb32_vspan_prgb32;
  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeDifferenceSSE2::prgb32_vspan_argb32;
  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeDifferenceSSE2::prgb32_vspan_xrgb32;
  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_RGB24] = CompositeDifferenceSSE2::prgb32_vspan_rgb24;

  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeDifferenceSSE2::prgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeDifferenceSSE2::prgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeDifferenceSSE2::prgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeDifferenceSSE2::prgb32_vspan_rgb24_a8;

  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeDifferenceSSE2::prgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeDifferenceSSE2::prgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeDifferenceSSE2::prgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeDifferenceSSE2::prgb32_vspan_rgb24_a8_const;

  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_XRGB32].pixel = CompositeDifferenceSSE2::xrgb32_pixel;
  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_XRGB32].pixel_a8 = CompositeDifferenceSSE2::xrgb32_pixel_a8;
  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_XRGB32].cspan = CompositeDifferenceSSE2::xrgb32_cspan;
  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeDifferenceSSE2::xrgb32_cspan_a8;
  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeDifferenceSSE2::xrgb32_cspan_a8_const;

  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeDifferenceSSE2::xrgb32_vspan_prgb32;
  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeDifferenceSSE2::xrgb32_vspan_argb32;
  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeDifferenceSSE2::xrgb32_vspan_xrgb32;
  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_RGB24] = CompositeDifferenceSSE2::xrgb32_vspan_rgb24;

  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeDifferenceSSE2::xrgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeDifferenceSSE2::xrgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeDifferenceSSE2::xrgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeDifferenceSSE2::xrgb32_vspan_rgb24_a8;

  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeDifferenceSSE2::xrgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeDifferenceSSE2::xrgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeDifferenceSSE2::xrgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_DIFFERENCE][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeDifferenceSSE2::xrgb32_vspan_rgb24_a8_const;

  // [Composite - Exclusion]

  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_PRGB32].pixel = CompositeExclusionSSE2::prgb32_pixel;
  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_PRGB32].pixel_a8 = CompositeExclusionSSE2::prgb32_pixel_a8;
  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_PRGB32].cspan = CompositeExclusionSSE2::prgb32_cspan;
  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeExclusionSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeExclusionSSE2::prgb32_cspan_a8_const;

  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeExclusionSSE2::prgb32_vspan_prgb32;
  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeExclusionSSE2::prgb32_vspan_argb32;
  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeExclusionSSE2::prgb32_vspan_xrgb32;
  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_RGB24] = CompositeExclusionSSE2::prgb32_vspan_rgb24;

  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeExclusionSSE2::prgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeExclusionSSE2::prgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeExclusionSSE2::prgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeExclusionSSE2::prgb32_vspan_rgb24_a8;

  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeExclusionSSE2::prgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeExclusionSSE2::prgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeExclusionSSE2::prgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeExclusionSSE2::prgb32_vspan_rgb24_a8_const;

  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_XRGB32].pixel = CompositeExclusionSSE2::xrgb32_pixel;
  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_XRGB32].pixel_a8 = CompositeExclusionSSE2::xrgb32_pixel_a8;
  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_XRGB32].cspan = CompositeExclusionSSE2::xrgb32_cspan;
  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeExclusionSSE2::xrgb32_cspan_a8;
  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeExclusionSSE2::xrgb32_cspan_a8_const;

  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeExclusionSSE2::xrgb32_vspan_prgb32;
  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeExclusionSSE2::xrgb32_vspan_argb32;
  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeExclusionSSE2::xrgb32_vspan_xrgb32;
  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_RGB24] = CompositeExclusionSSE2::xrgb32_vspan_rgb24;

  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeExclusionSSE2::xrgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeExclusionSSE2::xrgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeExclusionSSE2::xrgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeExclusionSSE2::xrgb32_vspan_rgb24_a8;

  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeExclusionSSE2::xrgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeExclusionSSE2::xrgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeExclusionSSE2::xrgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_EXCLUSION][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeExclusionSSE2::xrgb32_vspan_rgb24_a8_const;

  // [Composite - Invert]

  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_PRGB32].pixel = CompositeInvertSSE2::prgb32_pixel;
  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_PRGB32].pixel_a8 = CompositeInvertSSE2::prgb32_pixel_a8;
  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_PRGB32].cspan = CompositeInvertSSE2::prgb32_cspan;
  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeInvertSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeInvertSSE2::prgb32_cspan_a8_const;

  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeInvertSSE2::prgb32_vspan_prgb32;
  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeInvertSSE2::prgb32_vspan_argb32;
  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeInvertSSE2::prgb32_vspan_xrgb32;
  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_RGB24] = CompositeInvertSSE2::prgb32_vspan_rgb24;

  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeInvertSSE2::prgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeInvertSSE2::prgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeInvertSSE2::prgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeInvertSSE2::prgb32_vspan_rgb24_a8;

  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeInvertSSE2::prgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeInvertSSE2::prgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeInvertSSE2::prgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeInvertSSE2::prgb32_vspan_rgb24_a8_const;

  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_XRGB32].pixel = CompositeInvertSSE2::xrgb32_pixel;
  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_XRGB32].pixel_a8 = CompositeInvertSSE2::xrgb32_pixel_a8;
  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_XRGB32].cspan = CompositeInvertSSE2::xrgb32_cspan;
  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeInvertSSE2::xrgb32_cspan_a8;
  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeInvertSSE2::xrgb32_cspan_a8_const;

  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeInvertSSE2::xrgb32_vspan_prgb32;
  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeInvertSSE2::xrgb32_vspan_argb32;
  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeInvertSSE2::xrgb32_vspan_xrgb32;
  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_RGB24] = CompositeInvertSSE2::xrgb32_vspan_rgb24;

  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeInvertSSE2::xrgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeInvertSSE2::xrgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeInvertSSE2::xrgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeInvertSSE2::xrgb32_vspan_rgb24_a8;

  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeInvertSSE2::xrgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeInvertSSE2::xrgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeInvertSSE2::xrgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_INVERT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeInvertSSE2::xrgb32_vspan_rgb24_a8_const;

  // [Composite - InvertRgb]

  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_PRGB32].pixel = CompositeInvertRgbSSE2::prgb32_pixel;
  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_PRGB32].pixel_a8 = CompositeInvertRgbSSE2::prgb32_pixel_a8;
  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_PRGB32].cspan = CompositeInvertRgbSSE2::prgb32_cspan;
  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeInvertRgbSSE2::prgb32_cspan_a8;
  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeInvertRgbSSE2::prgb32_cspan_a8_const;

  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeInvertRgbSSE2::prgb32_vspan_prgb32;
  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeInvertRgbSSE2::prgb32_vspan_argb32;
  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeInvertRgbSSE2::prgb32_vspan_xrgb32;
  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_RGB24] = CompositeInvertRgbSSE2::prgb32_vspan_rgb24;

  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeInvertRgbSSE2::prgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeInvertRgbSSE2::prgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeInvertRgbSSE2::prgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeInvertRgbSSE2::prgb32_vspan_rgb24_a8;

  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeInvertRgbSSE2::prgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeInvertRgbSSE2::prgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeInvertRgbSSE2::prgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeInvertRgbSSE2::prgb32_vspan_rgb24_a8_const;

  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_XRGB32].pixel = CompositeInvertRgbSSE2::xrgb32_pixel;
  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_XRGB32].pixel_a8 = CompositeInvertRgbSSE2::xrgb32_pixel_a8;
  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_XRGB32].cspan = CompositeInvertRgbSSE2::xrgb32_cspan;
  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeInvertRgbSSE2::xrgb32_cspan_a8;
  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeInvertRgbSSE2::xrgb32_cspan_a8_const;

  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeInvertRgbSSE2::xrgb32_vspan_prgb32;
  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeInvertRgbSSE2::xrgb32_vspan_argb32;
  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeInvertRgbSSE2::xrgb32_vspan_xrgb32;
  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_RGB24] = CompositeInvertRgbSSE2::xrgb32_vspan_rgb24;

  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeInvertRgbSSE2::xrgb32_vspan_prgb32_a8;
  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeInvertRgbSSE2::xrgb32_vspan_argb32_a8;
  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeInvertRgbSSE2::xrgb32_vspan_xrgb32_a8;
  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_RGB24] = CompositeInvertRgbSSE2::xrgb32_vspan_rgb24_a8;

  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeInvertRgbSSE2::xrgb32_vspan_prgb32_a8_const;
  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeInvertRgbSSE2::xrgb32_vspan_argb32_a8_const;
  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeInvertRgbSSE2::xrgb32_vspan_xrgb32_a8_const;
  m->raster[COMPOSITE_INVERT_RGB][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_RGB24] = CompositeInvertRgbSSE2::xrgb32_vspan_rgb24_a8_const;
}
