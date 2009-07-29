// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/Intrin_SSE2.h>
#include <Fog/Core/Math.h>
#include <Fog/Core/Misc.h>
#include <Fog/Graphics/ColorLut.h>
#include <Fog/Graphics/ColorMatrix.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/ImageFilter.h>
#include <Fog/Graphics/Raster.h>
#include <Fog/Graphics/Raster/Raster_C.h>

// [Raster_SSE2]
#include <Fog/Graphics/Raster/Raster_SSE2_base.cpp.h>
#include <Fog/Graphics/Raster/Raster_SSE2_convert.cpp.h>
#include <Fog/Graphics/Raster/Raster_SSE2_composite.cpp.h>
#include <Fog/Graphics/Raster/Raster_SSE2_gradient.cpp.h>
#include <Fog/Graphics/Raster/Raster_SSE2_pattern.cpp.h>
#include <Fog/Graphics/Raster/Raster_SSE2_filters.cpp.h>

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE void fog_raster_init_sse2(void)
{
  using namespace Fog;
  using namespace Fog::Raster;

  // W masks
  Mask0080008000800080 = pix_create_mask_8x2W(0x0080, 0x0080, 0x0080, 0x0080);
  Mask000000FF00FF00FF = pix_create_mask_8x2W(0x0000, 0x00FF, 0x00FF, 0x00FF);
  Mask00FF00FF00FF00FF = pix_create_mask_8x2W(0x00FF, 0x00FF, 0x00FF, 0x00FF);
  MaskFF000000000000FF = pix_create_mask_8x2W(0xFF00, 0x0000, 0x0000, 0x00FF);
  Mask0101010101010101 = pix_create_mask_8x2W(0x0101, 0x0101, 0x0101, 0x0101);
  MaskFFFFFFFFFFFFFFFF = pix_create_mask_8x2W(0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF);
  Mask00FF000000000000 = pix_create_mask_8x2W(0x00FF, 0x0000, 0x0000, 0x0000);
  MaskFF000000FF000000 = pix_create_mask_8x2W(0xFF00, 0x0000, 0xFF00, 0x0000);
  Mask00FFFFFF00FFFFFF = pix_create_mask_8x2W(0x00FF, 0xFFFF, 0x00FF, 0xFFFF);
  Mask0000000100000001 = pix_create_mask_8x2W(0x0000, 0x0001, 0x0000, 0x0001);

  // D masks
  Mask00010000000000000000000000000000 = pix_create_mask_4x1D(0x00010000, 0x00000000, 0x00000000, 0x00000000);

  // Float / Double masks
  sse2_t t;

  t.uq[0] = ~(FOG_UINT64_C(0x8000000080000000));
  t.uq[1] = ~(FOG_UINT64_C(0x8000000080000000));
  Mask7FFFFFFF7FFFFFFF = _mm_loadu_ps((float*)&t);

  t.uq[0] = ~(FOG_UINT64_C(0x8000000000000000));
  t.uq[1] = ~(FOG_UINT64_C(0x8000000000000000));
  Mask7FFFFFFFFFFFFFFF = _mm_loadu_pd((double*)&t);

  FunctionMap* m = functionMap;

  // [Convert]

  m->convert.memcpy32 = convert_memcpy32_sse2;

  m->convert.prgb32_from_argb32 = convert_prgb32_from_argb32_sse2;
  m->convert.prgb32_from_argb32_bs = convert_prgb32_from_argb32_bs_sse2;
  m->convert.prgb32_bs_from_argb32 = convert_prgb32_bs_from_argb32_sse2;
  m->convert.argb32_from_rgb32 = convert_argb32_from_rgb32_sse2;
  m->convert.rgb32_from_rgb24 = convert_rgb32_from_rgb24_sse2;
  m->convert.rgb32_from_bgr24 = convert_rgb32_from_bgr24_sse2;

  // [Gradient - Gradient]

  m->gradient.gradient_argb32 = gradient_gradient_argb32_sse2;
  m->gradient.gradient_prgb32 = gradient_gradient_prgb32_sse2;

  // [Pattern - Texture]

  m->pattern.texture_fetch_repeat = pattern_texture_fetch_repeat_sse2;
  m->pattern.texture_fetch_reflect = pattern_texture_fetch_reflect_sse2;

  // [Pattern - Radial Gradient]

  m->pattern.radial_gradient_fetch_pad = pattern_radial_gradient_fetch_pad_sse2;

  // [Raster - CompositeSrc]

  m->raster[Image::FormatARGB32][CompositeSrc].pixel = raster_argb32_pixel_src_sse2;
  m->raster[Image::FormatARGB32][CompositeSrc].pixel_a8 = raster_argb32_pixel_a8_src_sse2;
  m->raster[Image::FormatARGB32][CompositeSrc].span_solid = raster_argb32_span_solid_src_sse2;
  m->raster[Image::FormatARGB32][CompositeSrc].span_solid_a8 = raster_argb32_span_solid_a8_src_sse2;
  m->raster[Image::FormatARGB32][CompositeSrc].span_solid_a8_const = raster_argb32_span_solid_a8_const_src_sse2;

  m->raster[Image::FormatARGB32][CompositeSrc].span_composite[Image::FormatPRGB32] = raster_argb32_span_composite_prgb32_src_sse2;
  m->raster[Image::FormatARGB32][CompositeSrc].span_composite[Image::FormatARGB32] = raster_argb32_span_composite_argb32_src_sse2;
  m->raster[Image::FormatARGB32][CompositeSrc].span_composite[Image::FormatRGB32] = raster_argb32_span_composite_rgb32_src_sse2;
  m->raster[Image::FormatARGB32][CompositeSrc].span_composite[Image::FormatRGB24] = raster_argb32_span_composite_rgb24_src_sse2;

  m->raster[Image::FormatARGB32][CompositeSrc].span_composite_a8[Image::FormatARGB32] = raster_argb32_span_composite_argb32_a8_src_sse2;
  m->raster[Image::FormatARGB32][CompositeSrc].span_composite_a8[Image::FormatARGB32] = raster_argb32_span_composite_prgb32_a8_src_sse2;
  m->raster[Image::FormatARGB32][CompositeSrc].span_composite_a8[Image::FormatRGB32] = raster_argb32_span_composite_rgb32_a8_src_sse2;
  m->raster[Image::FormatARGB32][CompositeSrc].span_composite_a8[Image::FormatRGB24] = raster_argb32_span_composite_rgb24_a8_src_sse2;

  m->raster[Image::FormatARGB32][CompositeSrc].span_composite_a8_const[Image::FormatARGB32] = raster_argb32_span_composite_argb32_a8_const_src_sse2;
  m->raster[Image::FormatARGB32][CompositeSrc].span_composite_a8_const[Image::FormatARGB32] = raster_argb32_span_composite_prgb32_a8_const_src_sse2;
  m->raster[Image::FormatARGB32][CompositeSrc].span_composite_a8_const[Image::FormatRGB32] = raster_argb32_span_composite_rgb32_a8_const_src_sse2;
  m->raster[Image::FormatARGB32][CompositeSrc].span_composite_a8_const[Image::FormatRGB24] = raster_argb32_span_composite_rgb24_a8_const_src_sse2;

  m->raster[Image::FormatPRGB32][CompositeSrc].pixel = raster_prgb32_pixel_src_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrc].pixel_a8 = raster_prgb32_pixel_a8_src_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrc].span_solid = raster_prgb32_span_solid_src_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrc].span_solid_a8 = raster_prgb32_span_solid_a8_src_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrc].span_solid_a8_const = raster_prgb32_span_solid_a8_const_src_sse2;

  m->raster[Image::FormatPRGB32][CompositeSrc].span_composite[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_src_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrc].span_composite[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_src_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrc].span_composite[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_src_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrc].span_composite[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_src_sse2;

  m->raster[Image::FormatPRGB32][CompositeSrc].span_composite_a8[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_src_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrc].span_composite_a8[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_src_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrc].span_composite_a8[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_src_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrc].span_composite_a8[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_a8_src_sse2;

  m->raster[Image::FormatPRGB32][CompositeSrc].span_composite_a8_const[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_const_src_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrc].span_composite_a8_const[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_const_src_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrc].span_composite_a8_const[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_const_src_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrc].span_composite_a8_const[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_a8_const_src_sse2;

  m->raster[Image::FormatRGB32][CompositeSrc].pixel = raster_rgb32_pixel_src_sse2;
  m->raster[Image::FormatRGB32][CompositeSrc].pixel_a8 = raster_rgb32_pixel_a8_src_sse2;
  m->raster[Image::FormatRGB32][CompositeSrc].span_solid = raster_rgb32_span_solid_src_sse2;
  m->raster[Image::FormatRGB32][CompositeSrc].span_solid_a8 = raster_rgb32_span_solid_a8_src_sse2;
  m->raster[Image::FormatRGB32][CompositeSrc].span_solid_a8_const = raster_rgb32_span_solid_a8_const_src_sse2;

  m->raster[Image::FormatRGB32][CompositeSrc].span_composite[Image::FormatARGB32] = raster_rgb32_span_composite_argb32_src_sse2;
  //m->raster[Image::FormatRGB32][CompositeSrc].span_composite[Image::FormatPRGB32] = raster_rgb32_span_composite_prgb32_src_sse2;
  m->raster[Image::FormatRGB32][CompositeSrc].span_composite[Image::FormatRGB32] = raster_rgb32_span_composite_rgb32_src_sse2;
  m->raster[Image::FormatRGB32][CompositeSrc].span_composite[Image::FormatRGB24] = raster_rgb32_span_composite_rgb24_src_sse2;

  m->raster[Image::FormatRGB32][CompositeSrc].span_composite_a8[Image::FormatARGB32] = raster_rgb32_span_composite_argb32_a8_src_sse2;
  //m->raster[Image::FormatRGB32][CompositeSrc].span_composite_a8[Image::FormatPRGB32] = raster_rgb32_span_composite_prgb32_a8_src_sse2;
  m->raster[Image::FormatRGB32][CompositeSrc].span_composite_a8[Image::FormatRGB32] = raster_rgb32_span_composite_rgb32_a8_src_sse2;
  m->raster[Image::FormatRGB32][CompositeSrc].span_composite_a8[Image::FormatRGB24] = raster_rgb32_span_composite_rgb24_a8_src_sse2;

  m->raster[Image::FormatRGB32][CompositeSrc].span_composite_a8_const[Image::FormatARGB32] = raster_rgb32_span_composite_argb32_a8_const_src_sse2;
  //m->raster[Image::FormatRGB32][CompositeSrc].span_composite_a8_const[Image::FormatPRGB32] = raster_rgb32_span_composite_prgb32_a8_const_src_sse2;
  m->raster[Image::FormatRGB32][CompositeSrc].span_composite_a8_const[Image::FormatRGB32] = raster_rgb32_span_composite_rgb32_a8_const_src_sse2;
  m->raster[Image::FormatRGB32][CompositeSrc].span_composite_a8_const[Image::FormatRGB24] = raster_rgb32_span_composite_rgb24_a8_const_src_sse2;

  // TODO

  // [Raster - CompositeDest (NOP)]

  // FormatARGB32 - Set to NOPs by Raster_C.cpp
  // FormatPRGB32 - Set to NOPs by Raster_C.cpp
  // FormatRGB32  - Set to NOPs by Raster_C.cpp
  // FormatRGB24  - Set to NOPs by Raster_C.cpp
  // FormatA8     - Set to NOPs by Raster_C.cpp
  // FormatI8     - Set to NOPs by Raster_C.cpp

  // [Raster - CompositeSrcOver]

  m->raster[Image::FormatARGB32][CompositeSrcOver].pixel = raster_argb32_pixel_srcover_sse2;
  m->raster[Image::FormatARGB32][CompositeSrcOver].pixel_a8 = raster_argb32_pixel_a8_srcover_sse2;
  m->raster[Image::FormatARGB32][CompositeSrcOver].span_solid = raster_argb32_span_solid_srcover_sse2;
  m->raster[Image::FormatARGB32][CompositeSrcOver].span_solid_a8 = raster_argb32_span_solid_a8_srcover_sse2;
  m->raster[Image::FormatARGB32][CompositeSrcOver].span_solid_a8_const = raster_argb32_span_solid_a8_const_srcover_sse2;

  // TODO
  m->raster[Image::FormatARGB32][CompositeSrcOver].span_composite[Image::FormatRGB24] = convert_rgb32_from_rgb24_sse2;

  m->raster[Image::FormatPRGB32][CompositeSrcOver].pixel = raster_prgb32_pixel_srcover_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcOver].pixel_a8 = raster_prgb32_pixel_a8_srcover_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcOver].span_solid = raster_prgb32_span_solid_srcover_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcOver].span_solid_a8 = raster_prgb32_span_solid_a8_srcover_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcOver].span_solid_a8_const = raster_prgb32_span_solid_a8_const_srcover_sse2;

  m->raster[Image::FormatPRGB32][CompositeSrcOver].span_composite[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_srcover_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcOver].span_composite[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_srcover_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcOver].span_composite[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_srcover_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcOver].span_composite[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_srcover_sse2;

  m->raster[Image::FormatPRGB32][CompositeSrcOver].span_composite_a8[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_srcover_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcOver].span_composite_a8[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_srcover_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcOver].span_composite_a8[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_srcover_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcOver].span_composite_a8[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_a8_srcover_sse2;

  m->raster[Image::FormatPRGB32][CompositeSrcOver].span_composite_a8_const[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_const_srcover_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcOver].span_composite_a8_const[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_const_srcover_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcOver].span_composite_a8_const[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_const_srcover_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcOver].span_composite_a8_const[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_a8_const_srcover_sse2;

  m->raster[Image::FormatRGB32][CompositeSrcOver].pixel = raster_rgb32_pixel_srcover_sse2;
  m->raster[Image::FormatRGB32][CompositeSrcOver].pixel_a8 = raster_rgb32_pixel_a8_srcover_sse2;
  m->raster[Image::FormatRGB32][CompositeSrcOver].span_solid = raster_rgb32_span_solid_srcover_sse2;
  m->raster[Image::FormatRGB32][CompositeSrcOver].span_solid_a8 = raster_rgb32_span_solid_a8_srcover_sse2;
  m->raster[Image::FormatRGB32][CompositeSrcOver].span_solid_a8_const = raster_rgb32_span_solid_a8_const_srcover_sse2;

  m->raster[Image::FormatRGB32][CompositeSrcOver].span_composite[Image::FormatARGB32] = raster_rgb32_span_composite_argb32_srcover_sse2;
  m->raster[Image::FormatRGB32][CompositeSrcOver].span_composite[Image::FormatPRGB32] = raster_rgb32_span_composite_prgb32_srcover_sse2;
  m->raster[Image::FormatRGB32][CompositeSrcOver].span_composite[Image::FormatRGB32] = raster_rgb32_span_composite_rgb32_srcover_sse2;
  m->raster[Image::FormatRGB32][CompositeSrcOver].span_composite[Image::FormatRGB24] = raster_rgb32_span_composite_rgb24_srcover_sse2;

  m->raster[Image::FormatRGB32][CompositeSrcOver].span_composite_a8[Image::FormatARGB32] = raster_rgb32_span_composite_argb32_a8_srcover_sse2;
  m->raster[Image::FormatRGB32][CompositeSrcOver].span_composite_a8[Image::FormatPRGB32] = raster_rgb32_span_composite_prgb32_a8_srcover_sse2;
  m->raster[Image::FormatRGB32][CompositeSrcOver].span_composite_a8[Image::FormatRGB32] = raster_rgb32_span_composite_rgb32_a8_srcover_sse2;
  m->raster[Image::FormatRGB32][CompositeSrcOver].span_composite_a8[Image::FormatRGB24] = raster_rgb32_span_composite_rgb24_a8_srcover_sse2;

  m->raster[Image::FormatRGB32][CompositeSrcOver].span_composite_a8_const[Image::FormatARGB32] = raster_rgb32_span_composite_argb32_a8_const_srcover_sse2;
  m->raster[Image::FormatRGB32][CompositeSrcOver].span_composite_a8_const[Image::FormatPRGB32] = raster_rgb32_span_composite_prgb32_a8_const_srcover_sse2;
  m->raster[Image::FormatRGB32][CompositeSrcOver].span_composite_a8_const[Image::FormatRGB32] = raster_rgb32_span_composite_rgb32_a8_const_srcover_sse2;
  m->raster[Image::FormatRGB32][CompositeSrcOver].span_composite_a8_const[Image::FormatRGB24] = raster_rgb32_span_composite_rgb24_a8_const_srcover_sse2;

  // TODO

  // [Raster - CompositeDestOver]

  m->raster[Image::FormatPRGB32][CompositeDestOver].pixel = raster_prgb32_pixel_dstover_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestOver].pixel_a8 = raster_prgb32_pixel_a8_dstover_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestOver].span_solid = raster_prgb32_span_solid_dstover_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestOver].span_solid_a8 = raster_prgb32_span_solid_a8_dstover_sse2;

  m->raster[Image::FormatPRGB32][CompositeDestOver].span_composite[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_dstover_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestOver].span_composite[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_dstover_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestOver].span_composite[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_dstover_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestOver].span_composite[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_dstover_sse2;

  m->raster[Image::FormatPRGB32][CompositeDestOver].span_composite_a8[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_dstover_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestOver].span_composite_a8[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_dstover_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestOver].span_composite_a8[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_dstover_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestOver].span_composite_a8[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_a8_dstover_sse2;

  // FormatRGB32  - Set to NOPs by Raster_C.cpp
  // FormatRGB24  - Set to NOPs by Raster_C.cpp

  // TODO

  // [Raster - CompositeSrcIn]

  m->raster[Image::FormatPRGB32][CompositeSrcIn].pixel = raster_prgb32_pixel_srcin_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcIn].pixel_a8 = raster_prgb32_pixel_a8_srcin_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcIn].span_solid = raster_prgb32_span_solid_srcin_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcIn].span_solid_a8 = raster_prgb32_span_solid_a8_srcin_sse2;

  m->raster[Image::FormatPRGB32][CompositeSrcIn].span_composite[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_srcin_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcIn].span_composite[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_srcin_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcIn].span_composite[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_srcin_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcIn].span_composite[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_srcin_sse2;

  m->raster[Image::FormatPRGB32][CompositeSrcIn].span_composite_a8[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_srcin_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcIn].span_composite_a8[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_srcin_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcIn].span_composite_a8[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_srcin_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcIn].span_composite_a8[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_a8_srcin_sse2;

  // TODO

  // [Raster - CompositeDestIn]

  m->raster[Image::FormatPRGB32][CompositeDestIn].pixel = raster_prgb32_pixel_dstin_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestIn].pixel_a8 = raster_prgb32_pixel_a8_dstin_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestIn].span_solid = raster_prgb32_span_solid_dstin_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestIn].span_solid_a8 = raster_prgb32_span_solid_a8_dstin_sse2;

  m->raster[Image::FormatPRGB32][CompositeDestIn].span_composite[Image::FormatARGB32] = raster_prgb32_span_composite_axxx32_dstin_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestIn].span_composite[Image::FormatPRGB32] = raster_prgb32_span_composite_axxx32_dstin_sse2;

  m->raster[Image::FormatPRGB32][CompositeDestIn].span_composite_a8[Image::FormatARGB32] = raster_prgb32_span_composite_axxx32_a8_dstin_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestIn].span_composite_a8[Image::FormatPRGB32] = raster_prgb32_span_composite_axxx32_a8_dstin_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestIn].span_composite_a8[Image::FormatRGB32] = raster_prgb32_span_composite_xxxx_a8_dstin_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestIn].span_composite_a8[Image::FormatRGB24] = raster_prgb32_span_composite_xxxx_a8_dstin_sse2;

  // TODO

  // [Raster - CompositeSrcOut]

  m->raster[Image::FormatPRGB32][CompositeSrcOut].pixel = raster_prgb32_pixel_srcout_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcOut].pixel_a8 = raster_prgb32_pixel_a8_srcout_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcOut].span_solid = raster_prgb32_span_solid_srcout_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcOut].span_solid_a8 = raster_prgb32_span_solid_a8_srcout_sse2;

  m->raster[Image::FormatPRGB32][CompositeSrcOut].span_composite[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_srcout_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcOut].span_composite[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_srcout_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcOut].span_composite[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_srcout_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcOut].span_composite[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_srcout_sse2;

  m->raster[Image::FormatPRGB32][CompositeSrcOut].span_composite_a8[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_srcout_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcOut].span_composite_a8[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_srcout_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcOut].span_composite_a8[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_srcout_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcOut].span_composite_a8[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_a8_srcout_sse2;

  // TODO

  // [Raster - CompositeDestOut]

  m->raster[Image::FormatPRGB32][CompositeDestOut].pixel = raster_prgb32_pixel_dstout_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestOut].pixel_a8 = raster_prgb32_pixel_a8_dstout_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestOut].span_solid = raster_prgb32_span_solid_dstout_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestOut].span_solid_a8 = raster_prgb32_span_solid_a8_dstout_sse2;

  m->raster[Image::FormatPRGB32][CompositeDestOut].span_composite[Image::FormatARGB32] = raster_prgb32_span_composite_axxx32_dstout_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestOut].span_composite[Image::FormatPRGB32] = raster_prgb32_span_composite_axxx32_dstout_sse2;

  m->raster[Image::FormatPRGB32][CompositeDestOut].span_composite_a8[Image::FormatARGB32] = raster_prgb32_span_composite_axxx32_a8_dstout_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestOut].span_composite_a8[Image::FormatPRGB32] = raster_prgb32_span_composite_axxx32_a8_dstout_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestOut].span_composite_a8[Image::FormatRGB32] = raster_prgb32_span_composite_xxxx_a8_dstout_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestOut].span_composite_a8[Image::FormatRGB24] = raster_prgb32_span_composite_xxxx_a8_dstout_sse2;

  // TODO

  // [Raster - CompositeSrcAtop]

  m->raster[Image::FormatPRGB32][CompositeSrcAtop].pixel = raster_prgb32_pixel_srcatop_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcAtop].pixel_a8 = raster_prgb32_pixel_a8_srcatop_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcAtop].span_solid = raster_prgb32_span_solid_srcatop_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcAtop].span_solid_a8 = raster_prgb32_span_solid_a8_srcatop_sse2;

  m->raster[Image::FormatPRGB32][CompositeSrcAtop].span_composite[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_srcatop_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcAtop].span_composite[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_srcatop_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcAtop].span_composite[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_srcatop_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcAtop].span_composite[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_srcatop_sse2;

  m->raster[Image::FormatPRGB32][CompositeSrcAtop].span_composite_a8[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_srcatop_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcAtop].span_composite_a8[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_srcatop_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcAtop].span_composite_a8[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_srcatop_sse2;
  m->raster[Image::FormatPRGB32][CompositeSrcAtop].span_composite_a8[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_a8_srcatop_sse2;

  // TODO

  // [Raster - CompositeDestAtop]

  m->raster[Image::FormatPRGB32][CompositeDestAtop].pixel = raster_prgb32_pixel_dstatop_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestAtop].pixel_a8 = raster_prgb32_pixel_a8_dstatop_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestAtop].span_solid = raster_prgb32_span_solid_dstatop_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestAtop].span_solid_a8 = raster_prgb32_span_solid_a8_dstatop_sse2;

  m->raster[Image::FormatPRGB32][CompositeDestAtop].span_composite[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_dstatop_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestAtop].span_composite[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_dstatop_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestAtop].span_composite[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_dstatop_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestAtop].span_composite[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_dstatop_sse2;

  m->raster[Image::FormatPRGB32][CompositeDestAtop].span_composite_a8[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_dstatop_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestAtop].span_composite_a8[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_dstatop_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestAtop].span_composite_a8[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_dstatop_sse2;
  m->raster[Image::FormatPRGB32][CompositeDestAtop].span_composite_a8[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_a8_dstatop_sse2;

  // TODO

  // [Raster - CompositeXor]

  m->raster[Image::FormatPRGB32][CompositeXor].pixel = raster_prgb32_pixel_xor_sse2;
  m->raster[Image::FormatPRGB32][CompositeXor].pixel_a8 = raster_prgb32_pixel_a8_xor_sse2;
  m->raster[Image::FormatPRGB32][CompositeXor].span_solid = raster_prgb32_span_solid_xor_sse2;
  m->raster[Image::FormatPRGB32][CompositeXor].span_solid_a8 = raster_prgb32_span_solid_a8_xor_sse2;

  m->raster[Image::FormatPRGB32][CompositeXor].span_composite[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_xor_sse2;
  m->raster[Image::FormatPRGB32][CompositeXor].span_composite[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_xor_sse2;
  m->raster[Image::FormatPRGB32][CompositeXor].span_composite[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_xor_sse2;
  m->raster[Image::FormatPRGB32][CompositeXor].span_composite[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_xor_sse2;

  m->raster[Image::FormatPRGB32][CompositeXor].span_composite_a8[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_xor_sse2;
  m->raster[Image::FormatPRGB32][CompositeXor].span_composite_a8[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_xor_sse2;
  m->raster[Image::FormatPRGB32][CompositeXor].span_composite_a8[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_xor_sse2;
  m->raster[Image::FormatPRGB32][CompositeXor].span_composite_a8[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_a8_xor_sse2;

  // TODO

  // [Raster - CompositeClear]

  m->raster[Image::FormatARGB32][CompositeClear].pixel = raster_argb32_pixel_clear_sse2;
  m->raster[Image::FormatARGB32][CompositeClear].pixel_a8 = raster_argb32_pixel_a8_clear_sse2;
  m->raster[Image::FormatARGB32][CompositeClear].span_solid = raster_argb32_span_solid_clear_sse2;
  m->raster[Image::FormatARGB32][CompositeClear].span_solid_a8 = raster_argb32_span_solid_a8_clear_sse2;
  m->raster[Image::FormatARGB32][CompositeClear].span_solid_a8_const = raster_argb32_span_solid_a8_const_clear_sse2;

  m->raster[Image::FormatARGB32][CompositeClear].span_composite[Image::FormatARGB32] = (SpanCompositeFn)raster_argb32_span_solid_clear_sse2;
  m->raster[Image::FormatARGB32][CompositeClear].span_composite[Image::FormatPRGB32] = (SpanCompositeFn)raster_argb32_span_solid_clear_sse2;
  m->raster[Image::FormatARGB32][CompositeClear].span_composite[Image::FormatRGB32] = (SpanCompositeFn)raster_argb32_span_solid_clear_sse2;
  m->raster[Image::FormatARGB32][CompositeClear].span_composite[Image::FormatRGB24] = (SpanCompositeFn)raster_argb32_span_solid_clear_sse2;

  m->raster[Image::FormatARGB32][CompositeClear].span_composite_a8[Image::FormatARGB32] = (SpanCompositeMskFn)raster_argb32_span_solid_a8_clear_sse2;
  m->raster[Image::FormatARGB32][CompositeClear].span_composite_a8[Image::FormatPRGB32] = (SpanCompositeMskFn)raster_argb32_span_solid_a8_clear_sse2;
  m->raster[Image::FormatARGB32][CompositeClear].span_composite_a8[Image::FormatRGB32] = (SpanCompositeMskFn)raster_argb32_span_solid_a8_clear_sse2;
  m->raster[Image::FormatARGB32][CompositeClear].span_composite_a8[Image::FormatRGB24] = (SpanCompositeMskFn)raster_argb32_span_solid_a8_clear_sse2;

  m->raster[Image::FormatARGB32][CompositeClear].span_composite_a8_const[Image::FormatARGB32] = (SpanCompositeMskConstFn)raster_argb32_span_solid_a8_const_clear_sse2;
  m->raster[Image::FormatARGB32][CompositeClear].span_composite_a8_const[Image::FormatPRGB32] = (SpanCompositeMskConstFn)raster_argb32_span_solid_a8_const_clear_sse2;
  m->raster[Image::FormatARGB32][CompositeClear].span_composite_a8_const[Image::FormatRGB32] = (SpanCompositeMskConstFn)raster_argb32_span_solid_a8_const_clear_sse2;
  m->raster[Image::FormatARGB32][CompositeClear].span_composite_a8_const[Image::FormatRGB24] = (SpanCompositeMskConstFn)raster_argb32_span_solid_a8_const_clear_sse2;

  m->raster[Image::FormatPRGB32][CompositeClear].pixel = raster_prgb32_pixel_clear_sse2;
  m->raster[Image::FormatPRGB32][CompositeClear].pixel_a8 = raster_prgb32_pixel_a8_clear_sse2;
  m->raster[Image::FormatPRGB32][CompositeClear].span_solid = raster_prgb32_span_solid_clear_sse2;
  m->raster[Image::FormatPRGB32][CompositeClear].span_solid_a8 = raster_prgb32_span_solid_a8_clear_sse2;
  m->raster[Image::FormatPRGB32][CompositeClear].span_solid_a8_const = raster_prgb32_span_solid_a8_const_clear_sse2;

  m->raster[Image::FormatPRGB32][CompositeClear].span_composite[Image::FormatARGB32] = (SpanCompositeFn)raster_prgb32_span_solid_clear_sse2;
  m->raster[Image::FormatPRGB32][CompositeClear].span_composite[Image::FormatPRGB32] = (SpanCompositeFn)raster_prgb32_span_solid_clear_sse2;
  m->raster[Image::FormatPRGB32][CompositeClear].span_composite[Image::FormatRGB32] = (SpanCompositeFn)raster_prgb32_span_solid_clear_sse2;
  m->raster[Image::FormatPRGB32][CompositeClear].span_composite[Image::FormatRGB24] = (SpanCompositeFn)raster_prgb32_span_solid_clear_sse2;

  m->raster[Image::FormatPRGB32][CompositeClear].span_composite_a8[Image::FormatARGB32] = (SpanCompositeMskFn)raster_prgb32_span_solid_a8_clear_sse2;
  m->raster[Image::FormatPRGB32][CompositeClear].span_composite_a8[Image::FormatPRGB32] = (SpanCompositeMskFn)raster_prgb32_span_solid_a8_clear_sse2;
  m->raster[Image::FormatPRGB32][CompositeClear].span_composite_a8[Image::FormatRGB32] = (SpanCompositeMskFn)raster_prgb32_span_solid_a8_clear_sse2;
  m->raster[Image::FormatPRGB32][CompositeClear].span_composite_a8[Image::FormatRGB24] = (SpanCompositeMskFn)raster_prgb32_span_solid_a8_clear_sse2;

  m->raster[Image::FormatPRGB32][CompositeClear].span_composite_a8_const[Image::FormatARGB32] = (SpanCompositeMskConstFn)raster_prgb32_span_solid_a8_const_clear_sse2;
  m->raster[Image::FormatPRGB32][CompositeClear].span_composite_a8_const[Image::FormatPRGB32] = (SpanCompositeMskConstFn)raster_prgb32_span_solid_a8_const_clear_sse2;
  m->raster[Image::FormatPRGB32][CompositeClear].span_composite_a8_const[Image::FormatRGB32] = (SpanCompositeMskConstFn)raster_prgb32_span_solid_a8_const_clear_sse2;
  m->raster[Image::FormatPRGB32][CompositeClear].span_composite_a8_const[Image::FormatRGB24] = (SpanCompositeMskConstFn)raster_prgb32_span_solid_a8_const_clear_sse2;

  m->raster[Image::FormatRGB32][CompositeClear].pixel = raster_rgb32_pixel_clear_sse2;
  m->raster[Image::FormatRGB32][CompositeClear].pixel_a8 = raster_rgb32_pixel_a8_clear_sse2;
  m->raster[Image::FormatRGB32][CompositeClear].span_solid = raster_rgb32_span_solid_clear_sse2;
  m->raster[Image::FormatRGB32][CompositeClear].span_solid_a8 = raster_rgb32_span_solid_a8_clear_sse2;
  m->raster[Image::FormatRGB32][CompositeClear].span_solid_a8_const = raster_rgb32_span_solid_a8_const_clear_sse2;

  m->raster[Image::FormatRGB32][CompositeClear].span_composite[Image::FormatARGB32] = (SpanCompositeFn)raster_rgb32_span_solid_clear_sse2;
  m->raster[Image::FormatRGB32][CompositeClear].span_composite[Image::FormatPRGB32] = (SpanCompositeFn)raster_rgb32_span_solid_clear_sse2;
  m->raster[Image::FormatRGB32][CompositeClear].span_composite[Image::FormatRGB32] = (SpanCompositeFn)raster_rgb32_span_solid_clear_sse2;
  m->raster[Image::FormatRGB32][CompositeClear].span_composite[Image::FormatRGB24] = (SpanCompositeFn)raster_rgb32_span_solid_clear_sse2;

  m->raster[Image::FormatRGB32][CompositeClear].span_composite_a8[Image::FormatARGB32] = (SpanCompositeMskFn)raster_rgb32_span_solid_a8_clear_sse2;
  m->raster[Image::FormatRGB32][CompositeClear].span_composite_a8[Image::FormatPRGB32] = (SpanCompositeMskFn)raster_rgb32_span_solid_a8_clear_sse2;
  m->raster[Image::FormatRGB32][CompositeClear].span_composite_a8[Image::FormatRGB32] = (SpanCompositeMskFn)raster_rgb32_span_solid_a8_clear_sse2;
  m->raster[Image::FormatRGB32][CompositeClear].span_composite_a8[Image::FormatRGB24] = (SpanCompositeMskFn)raster_rgb32_span_solid_a8_clear_sse2;

  m->raster[Image::FormatRGB32][CompositeClear].span_composite_a8_const[Image::FormatARGB32] = (SpanCompositeMskConstFn)raster_rgb32_span_solid_a8_const_clear_sse2;
  m->raster[Image::FormatRGB32][CompositeClear].span_composite_a8_const[Image::FormatPRGB32] = (SpanCompositeMskConstFn)raster_rgb32_span_solid_a8_const_clear_sse2;
  m->raster[Image::FormatRGB32][CompositeClear].span_composite_a8_const[Image::FormatRGB32] = (SpanCompositeMskConstFn)raster_rgb32_span_solid_a8_const_clear_sse2;
  m->raster[Image::FormatRGB32][CompositeClear].span_composite_a8_const[Image::FormatRGB24] = (SpanCompositeMskConstFn)raster_rgb32_span_solid_a8_const_clear_sse2;

  // TODO

  // [Raster - CompositeAdd]

  m->raster[Image::FormatPRGB32][CompositeAdd].pixel = raster_prgb32_pixel_add_sse2;
  m->raster[Image::FormatPRGB32][CompositeAdd].pixel_a8 = raster_prgb32_pixel_a8_add_sse2;
  m->raster[Image::FormatPRGB32][CompositeAdd].span_solid = raster_prgb32_span_solid_add_sse2;
  m->raster[Image::FormatPRGB32][CompositeAdd].span_solid_a8 = raster_prgb32_span_solid_a8_add_sse2;
  m->raster[Image::FormatPRGB32][CompositeAdd].span_solid_a8_const = raster_prgb32_span_solid_a8_const_add_sse2;

  m->raster[Image::FormatPRGB32][CompositeAdd].span_composite[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_add_sse2;
  m->raster[Image::FormatPRGB32][CompositeAdd].span_composite[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_add_sse2;
  m->raster[Image::FormatPRGB32][CompositeAdd].span_composite[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_add_sse2;
  m->raster[Image::FormatPRGB32][CompositeAdd].span_composite[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_add_sse2;

  m->raster[Image::FormatPRGB32][CompositeAdd].span_composite_a8[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_add_sse2;
  m->raster[Image::FormatPRGB32][CompositeAdd].span_composite_a8[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_add_sse2;
  m->raster[Image::FormatPRGB32][CompositeAdd].span_composite_a8[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_add_sse2;
  m->raster[Image::FormatPRGB32][CompositeAdd].span_composite_a8[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_a8_add_sse2;

  m->raster[Image::FormatPRGB32][CompositeAdd].span_composite_a8_const[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_const_add_sse2;
  m->raster[Image::FormatPRGB32][CompositeAdd].span_composite_a8_const[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_const_add_sse2;
  m->raster[Image::FormatPRGB32][CompositeAdd].span_composite_a8_const[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_const_add_sse2;
  m->raster[Image::FormatPRGB32][CompositeAdd].span_composite_a8_const[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_a8_const_add_sse2;

  m->raster[Image::FormatRGB32][CompositeAdd].pixel = raster_rgb32_pixel_add_sse2;
  m->raster[Image::FormatRGB32][CompositeAdd].pixel_a8 = raster_rgb32_pixel_a8_add_sse2;
  m->raster[Image::FormatRGB32][CompositeAdd].span_solid = raster_rgb32_span_solid_add_sse2;
  m->raster[Image::FormatRGB32][CompositeAdd].span_solid_a8 = raster_rgb32_span_solid_a8_add_sse2;
  m->raster[Image::FormatRGB32][CompositeAdd].span_solid_a8_const = raster_rgb32_span_solid_a8_const_add_sse2;

  m->raster[Image::FormatRGB32][CompositeAdd].span_composite[Image::FormatARGB32] = raster_rgb32_span_composite_argb32_add_sse2;
  m->raster[Image::FormatRGB32][CompositeAdd].span_composite[Image::FormatPRGB32] = raster_rgb32_span_composite_prgb32_add_sse2;
  m->raster[Image::FormatRGB32][CompositeAdd].span_composite[Image::FormatRGB32] = raster_rgb32_span_composite_rgb32_add_sse2;
  m->raster[Image::FormatRGB32][CompositeAdd].span_composite[Image::FormatRGB24] = raster_rgb32_span_composite_rgb24_add_sse2;

  m->raster[Image::FormatRGB32][CompositeAdd].span_composite_a8[Image::FormatARGB32] = raster_rgb32_span_composite_argb32_a8_add_sse2;
  m->raster[Image::FormatRGB32][CompositeAdd].span_composite_a8[Image::FormatPRGB32] = raster_rgb32_span_composite_prgb32_a8_add_sse2;
  m->raster[Image::FormatRGB32][CompositeAdd].span_composite_a8[Image::FormatRGB32] = raster_rgb32_span_composite_rgb32_a8_add_sse2;
  m->raster[Image::FormatRGB32][CompositeAdd].span_composite_a8[Image::FormatRGB24] = raster_rgb32_span_composite_rgb24_a8_add_sse2;

  m->raster[Image::FormatRGB32][CompositeAdd].span_composite_a8_const[Image::FormatARGB32] = raster_rgb32_span_composite_argb32_a8_const_add_sse2;
  m->raster[Image::FormatRGB32][CompositeAdd].span_composite_a8_const[Image::FormatPRGB32] = raster_rgb32_span_composite_prgb32_a8_const_add_sse2;
  m->raster[Image::FormatRGB32][CompositeAdd].span_composite_a8_const[Image::FormatRGB32] = raster_rgb32_span_composite_rgb32_a8_const_add_sse2;
  m->raster[Image::FormatRGB32][CompositeAdd].span_composite_a8_const[Image::FormatRGB24] = raster_rgb32_span_composite_rgb24_a8_const_add_sse2;

  // TODO

  // [Raster - CompositeSubtract]

  // TODO

  // [Raster - CompositeMultiply]

  // TODO

  // [Raster - CompositeScreen]

  // TODO

  // [Raster - CompositeDarken]

  // TODO

  // [Raster - CompositeLighten]

  // TODO

  // [Raster - CompositeDifference]

  // TODO

  // [Raster - CompositeExclusion]

  // TODO

  // [Raster - CompositeInvert]

  // TODO

  // [Raster - CompositeInvertRgb]

  // TODO

  // [Filters - ColorMatrix]

  m->filters.colorMatrix[Image::FormatARGB32] = colorMatrix_argb32_sse2;
  m->filters.colorMatrix[Image::FormatRGB32] = colorMatrix_rgb32_sse2;
  m->filters.colorMatrix[Image::FormatRGB24] = colorMatrix_rgb24_sse2;
  m->filters.colorMatrix[Image::FormatA8] = colorMatrix_a8_sse2;

  // [Filters - FloatConvolve / IntegerConvolve]

  m->filters.floatScanlineConvolveH[Image::FormatARGB32] = floatScanlineConvolveH_argb32_sse2;
  m->filters.floatScanlineConvolveV[Image::FormatARGB32] = floatScanlineConvolveV_argb32_sse2;

  // [Filters - BoxBlur]

  m->filters.boxBlurConvolveH[Image::FormatARGB32] = boxBlurConvolveH_argb32_sse2;
  m->filters.boxBlurConvolveV[Image::FormatARGB32] = boxBlurConvolveV_argb32_sse2;

  // [Filters - StackBlur]

  m->filters.stackBlurConvolveH[Image::FormatARGB32] = stackBlurConvolveH_argb32_sse2;
  m->filters.stackBlurConvolveV[Image::FormatARGB32] = stackBlurConvolveV_argb32_sse2;
}
