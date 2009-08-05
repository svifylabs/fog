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
#include <Fog/Graphics/Raster/Raster_SSE2_adaptor.cpp.h>
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
  Mask_00FFFFFF00FFFFFF_00FFFFFF00FFFFFF = pix_create_mask_8x2W(0x00FF, 0xFFFF, 0x00FF, 0xFFFF, 0x00FF, 0xFFFF, 0x00FF, 0xFFFF);
  Mask_0000000100000001_0000000100000001 = pix_create_mask_8x2W(0x0000, 0x0001, 0x0000, 0x0001, 0x0000, 0x0001, 0x0000, 0x0001);

  Mask_00FF00FF00FF00FF_0000000000000000 = pix_create_mask_8x2W(0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x0000, 0x0000, 0x0000, 0x0000);
  Mask_0000000000000000_00FF00FF00FF00FF = pix_create_mask_8x2W(0x0000, 0x0000, 0x0000, 0x0000, 0x00FF, 0x00FF, 0x00FF, 0x00FF);

  // D masks
  Mask_0001000000000000_0000000000000000 = pix_create_mask_4x1D(0x00010000, 0x00000000, 0x00000000, 0x00000000);

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

  m->convert.memcpy32 = convert_memcpy32_sse2;
  m->convert.memcpy24 = convert_memcpy24_sse2;
  m->convert.memcpy16 = convert_memcpy16_sse2;
  m->convert.memcpy8 = convert_memcpy8_sse2;

  m->convert.axxx32_from_xxxx32 = convert_axxx32_from_xxxx32_sse2;
  m->convert.argb32_from_prgb32 = convert_argb32_from_prgb32_sse2;
  m->convert.argb32_from_rgb32 = convert_axxx32_from_xxxx32_sse2;
  m->convert.argb32_from_i8 = convert_argb32_from_i8_sse2;
  m->convert.prgb32_from_argb32 = convert_prgb32_from_argb32_sse2;
  m->convert.prgb32_from_argb32_bs = convert_prgb32_from_argb32_bs_sse2;
  m->convert.prgb32_from_i8 = convert_prgb32_from_i8_sse2;
  m->convert.prgb32_bs_from_argb32 = convert_prgb32_bs_from_argb32_sse2;
  m->convert.rgb32_from_argb32 = convert_rgb32_from_argb32_sse2;
  m->convert.rgb32_from_argb32_bs = convert_rgb32_from_argb32_bs_sse2;
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

  m->raster[CompositeSrc][Image::FormatARGB32].pixel = raster_argb32_pixel_src_sse2;
  m->raster[CompositeSrc][Image::FormatARGB32].pixel_a8 = raster_argb32_pixel_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatARGB32].span_solid = raster_argb32_span_solid_src_sse2;
  m->raster[CompositeSrc][Image::FormatARGB32].span_solid_a8 = raster_argb32_span_solid_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatARGB32].span_solid_a8_const = raster_argb32_span_solid_a8_const_src_sse2;

  m->raster[CompositeSrc][Image::FormatARGB32].span_composite[Image::FormatARGB32] = raster_argb32_span_composite_argb32_src_sse2;
  m->raster[CompositeSrc][Image::FormatARGB32].span_composite[Image::FormatPRGB32] = raster_argb32_span_composite_prgb32_src_sse2;
  m->raster[CompositeSrc][Image::FormatARGB32].span_composite[Image::FormatRGB32] = raster_argb32_span_composite_rgb32_src_sse2;
  m->raster[CompositeSrc][Image::FormatARGB32].span_composite[Image::FormatRGB24] = raster_argb32_span_composite_rgb24_src_sse2;
  m->raster[CompositeSrc][Image::FormatARGB32].span_composite[Image::FormatA8] = raster_argb32_span_composite_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatARGB32].span_composite[Image::FormatI8] = m->convert.argb32_from_i8;

  m->raster[CompositeSrc][Image::FormatARGB32].span_composite_a8[Image::FormatARGB32] = raster_argb32_span_composite_argb32_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatARGB32].span_composite_a8[Image::FormatPRGB32] = raster_argb32_span_composite_prgb32_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatARGB32].span_composite_a8[Image::FormatRGB32] = raster_argb32_span_composite_rgb32_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatARGB32].span_composite_a8[Image::FormatRGB24] = raster_argb32_span_composite_rgb24_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatARGB32].span_composite_a8[Image::FormatA8] = raster_argb32_span_composite_a8_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatARGB32].span_composite_a8[Image::FormatI8] = raster_argb32_span_composite_i8_a8_src_sse2;

  m->raster[CompositeSrc][Image::FormatARGB32].span_composite_a8_const[Image::FormatARGB32] = raster_argb32_span_composite_argb32_a8_const_src_sse2;
  m->raster[CompositeSrc][Image::FormatARGB32].span_composite_a8_const[Image::FormatPRGB32] = raster_argb32_span_composite_prgb32_a8_const_src_sse2;
  m->raster[CompositeSrc][Image::FormatARGB32].span_composite_a8_const[Image::FormatRGB32] = raster_argb32_span_composite_rgb32_a8_const_src_sse2;
  m->raster[CompositeSrc][Image::FormatARGB32].span_composite_a8_const[Image::FormatRGB24] = raster_argb32_span_composite_rgb24_a8_const_src_sse2;
  m->raster[CompositeSrc][Image::FormatARGB32].span_composite_a8_const[Image::FormatA8] = raster_argb32_span_composite_a8_a8_const_src_sse2;
  m->raster[CompositeSrc][Image::FormatARGB32].span_composite_a8_const[Image::FormatI8] = raster_argb32_span_composite_i8_a8_const_src_sse2;

  m->raster[CompositeSrc][Image::FormatPRGB32].pixel = raster_prgb32_pixel_src_sse2;
  m->raster[CompositeSrc][Image::FormatPRGB32].pixel_a8 = raster_prgb32_pixel_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatPRGB32].span_solid = raster_prgb32_span_solid_src_sse2;
  m->raster[CompositeSrc][Image::FormatPRGB32].span_solid_a8 = raster_prgb32_span_solid_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatPRGB32].span_solid_a8_const = raster_prgb32_span_solid_a8_const_src_sse2;

  m->raster[CompositeSrc][Image::FormatPRGB32].span_composite[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_src_sse2;
  m->raster[CompositeSrc][Image::FormatPRGB32].span_composite[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_src_sse2;
  m->raster[CompositeSrc][Image::FormatPRGB32].span_composite[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_src_sse2;
  m->raster[CompositeSrc][Image::FormatPRGB32].span_composite[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_src_sse2;
  m->raster[CompositeSrc][Image::FormatPRGB32].span_composite[Image::FormatA8] = raster_prgb32_span_composite_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatPRGB32].span_composite[Image::FormatI8] = raster_prgb32_span_composite_i8_src_sse2;

  m->raster[CompositeSrc][Image::FormatPRGB32].span_composite_a8[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatPRGB32].span_composite_a8[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatPRGB32].span_composite_a8[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatPRGB32].span_composite_a8[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatPRGB32].span_composite_a8[Image::FormatA8] = raster_prgb32_span_composite_a8_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatPRGB32].span_composite_a8[Image::FormatI8] = raster_prgb32_span_composite_i8_a8_src_sse2;

  m->raster[CompositeSrc][Image::FormatPRGB32].span_composite_a8_const[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_const_src_sse2;
  m->raster[CompositeSrc][Image::FormatPRGB32].span_composite_a8_const[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_const_src_sse2;
  m->raster[CompositeSrc][Image::FormatPRGB32].span_composite_a8_const[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_const_src_sse2;
  m->raster[CompositeSrc][Image::FormatPRGB32].span_composite_a8_const[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_a8_const_src_sse2;
  m->raster[CompositeSrc][Image::FormatPRGB32].span_composite_a8_const[Image::FormatA8] = raster_prgb32_span_composite_a8_a8_const_src_sse2;
  m->raster[CompositeSrc][Image::FormatPRGB32].span_composite_a8_const[Image::FormatI8] = raster_prgb32_span_composite_i8_a8_const_src_sse2;

  m->raster[CompositeSrc][Image::FormatRGB32].pixel = raster_rgb32_pixel_src_sse2;
  m->raster[CompositeSrc][Image::FormatRGB32].pixel_a8 = raster_rgb32_pixel_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatRGB32].span_solid = raster_rgb32_span_solid_src_sse2;
  m->raster[CompositeSrc][Image::FormatRGB32].span_solid_a8 = raster_rgb32_span_solid_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatRGB32].span_solid_a8_const = raster_rgb32_span_solid_a8_const_src_sse2;

  m->raster[CompositeSrc][Image::FormatRGB32].span_composite[Image::FormatARGB32] = raster_rgb32_span_composite_argb32_src_sse2;
  m->raster[CompositeSrc][Image::FormatRGB32].span_composite[Image::FormatPRGB32] = raster_rgb32_span_composite_prgb32_src_sse2;
  m->raster[CompositeSrc][Image::FormatRGB32].span_composite[Image::FormatRGB32] = raster_rgb32_span_composite_rgb32_src_sse2;
  m->raster[CompositeSrc][Image::FormatRGB32].span_composite[Image::FormatRGB24] = raster_rgb32_span_composite_rgb24_src_sse2;
  m->raster[CompositeSrc][Image::FormatRGB32].span_composite[Image::FormatA8] = raster_rgb32_span_composite_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatRGB32].span_composite[Image::FormatI8] = raster_rgb32_span_composite_i8_src_sse2;

  m->raster[CompositeSrc][Image::FormatRGB32].span_composite_a8[Image::FormatARGB32] = raster_rgb32_span_composite_argb32_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatRGB32].span_composite_a8[Image::FormatPRGB32] = raster_rgb32_span_composite_prgb32_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatRGB32].span_composite_a8[Image::FormatRGB32] = raster_rgb32_span_composite_rgb32_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatRGB32].span_composite_a8[Image::FormatRGB24] = raster_rgb32_span_composite_rgb24_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatRGB32].span_composite_a8[Image::FormatA8] = raster_rgb32_span_composite_a8_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatRGB32].span_composite_a8[Image::FormatI8] = raster_rgb32_span_composite_i8_a8_src_sse2;

  m->raster[CompositeSrc][Image::FormatRGB32].span_composite_a8_const[Image::FormatARGB32] = raster_rgb32_span_composite_argb32_a8_const_src_sse2;
  m->raster[CompositeSrc][Image::FormatRGB32].span_composite_a8_const[Image::FormatPRGB32] = raster_rgb32_span_composite_prgb32_a8_const_src_sse2;
  m->raster[CompositeSrc][Image::FormatRGB32].span_composite_a8_const[Image::FormatRGB32] = raster_rgb32_span_composite_rgb32_a8_const_src_sse2;
  m->raster[CompositeSrc][Image::FormatRGB32].span_composite_a8_const[Image::FormatRGB24] = raster_rgb32_span_composite_rgb24_a8_const_src_sse2;
  m->raster[CompositeSrc][Image::FormatRGB32].span_composite_a8_const[Image::FormatA8] = raster_rgb32_span_composite_a8_a8_const_src_sse2;
  m->raster[CompositeSrc][Image::FormatRGB32].span_composite_a8_const[Image::FormatI8] = raster_rgb32_span_composite_i8_a8_const_src_sse2;

  m->raster[CompositeSrc][Image::FormatA8].pixel = raster_a8_pixel_src_sse2;
  m->raster[CompositeSrc][Image::FormatA8].pixel_a8 = raster_a8_pixel_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatA8].span_solid = raster_a8_span_solid_src_sse2;
  m->raster[CompositeSrc][Image::FormatA8].span_solid_a8 = raster_a8_span_solid_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatA8].span_solid_a8_const = raster_a8_span_solid_a8_const_src_sse2;

  m->raster[CompositeSrc][Image::FormatA8].span_composite[Image::FormatARGB32] = raster_a8_span_composite_argb32_src_sse2;
  m->raster[CompositeSrc][Image::FormatA8].span_composite[Image::FormatPRGB32] = raster_a8_span_composite_prgb32_src_sse2;
  m->raster[CompositeSrc][Image::FormatA8].span_composite[Image::FormatRGB32] = raster_a8_span_composite_rgb32_src_sse2;
  m->raster[CompositeSrc][Image::FormatA8].span_composite[Image::FormatRGB24] = raster_a8_span_composite_rgb24_src_sse2;
  m->raster[CompositeSrc][Image::FormatA8].span_composite[Image::FormatA8] = raster_a8_span_composite_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatA8].span_composite[Image::FormatI8] = m->convert.a8_from_i8;

  m->raster[CompositeSrc][Image::FormatA8].span_composite_a8[Image::FormatARGB32] = raster_a8_span_composite_argb32_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatA8].span_composite_a8[Image::FormatPRGB32] = raster_a8_span_composite_prgb32_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatA8].span_composite_a8[Image::FormatRGB32] = raster_a8_span_composite_rgb32_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatA8].span_composite_a8[Image::FormatRGB24] = raster_a8_span_composite_rgb24_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatA8].span_composite_a8[Image::FormatA8] = raster_a8_span_composite_a8_a8_src_sse2;
  m->raster[CompositeSrc][Image::FormatA8].span_composite_a8[Image::FormatI8] = raster_a8_span_composite_i8_a8_src_sse2;

  m->raster[CompositeSrc][Image::FormatA8].span_composite_a8_const[Image::FormatARGB32] = raster_a8_span_composite_argb32_a8_const_src_sse2;
  m->raster[CompositeSrc][Image::FormatA8].span_composite_a8_const[Image::FormatPRGB32] = raster_a8_span_composite_prgb32_a8_const_src_sse2;
  m->raster[CompositeSrc][Image::FormatA8].span_composite_a8_const[Image::FormatRGB32] = raster_a8_span_composite_rgb32_a8_const_src_sse2;
  m->raster[CompositeSrc][Image::FormatA8].span_composite_a8_const[Image::FormatRGB24] = raster_a8_span_composite_rgb24_a8_const_src_sse2;
  m->raster[CompositeSrc][Image::FormatA8].span_composite_a8_const[Image::FormatA8] = raster_a8_span_composite_a8_a8_const_src_sse2;
  m->raster[CompositeSrc][Image::FormatA8].span_composite_a8_const[Image::FormatI8] = raster_a8_span_composite_i8_a8_const_src_sse2;

  // [Raster - CompositeDest (NOP)]

  // FormatARGB32 - Set to NOPs by Raster_C.cpp
  // FormatPRGB32 - Set to NOPs by Raster_C.cpp
  // FormatRGB32  - Set to NOPs by Raster_C.cpp
  // FormatRGB24  - Set to NOPs by Raster_C.cpp
  // FormatA8     - Set to NOPs by Raster_C.cpp
  // FormatI8     - Set to NOPs by Raster_C.cpp

  // [Raster - CompositeSrcOver]

  m->raster[CompositeSrcOver][Image::FormatARGB32].pixel = raster_argb32_pixel_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatARGB32].pixel_a8 = raster_argb32_pixel_a8_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatARGB32].span_solid = raster_argb32_span_solid_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatARGB32].span_solid_a8 = raster_argb32_span_solid_a8_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatARGB32].span_solid_a8_const = raster_argb32_span_solid_a8_const_srcover_sse2;

  // TODO
  m->raster[CompositeSrcOver][Image::FormatARGB32].span_composite[Image::FormatRGB24] = convert_rgb32_from_rgb24_sse2;

  m->raster[CompositeSrcOver][Image::FormatPRGB32].pixel = raster_prgb32_pixel_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatPRGB32].pixel_a8 = raster_prgb32_pixel_a8_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatPRGB32].span_solid = raster_prgb32_span_solid_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatPRGB32].span_solid_a8 = raster_prgb32_span_solid_a8_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatPRGB32].span_solid_a8_const = raster_prgb32_span_solid_a8_const_srcover_sse2;

  m->raster[CompositeSrcOver][Image::FormatPRGB32].span_composite[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatPRGB32].span_composite[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatPRGB32].span_composite[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatPRGB32].span_composite[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_srcover_sse2;
//  m->raster[CompositeSrcOver][Image::FormatPRGB32].span_composite[Image::FormatA8] = raster_prgb32_span_composite_a8_srcover_sse2;
//  m->raster[CompositeSrcOver][Image::FormatPRGB32].span_composite[Image::FormatI8] = raster_prgb32_span_composite_i8_srcover_sse2;

  m->raster[CompositeSrcOver][Image::FormatPRGB32].span_composite_a8[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatPRGB32].span_composite_a8[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatPRGB32].span_composite_a8[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatPRGB32].span_composite_a8[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_a8_srcover_sse2;
//  m->raster[CompositeSrcOver][Image::FormatPRGB32].span_composite_a8[Image::FormatA8] = raster_prgb32_span_composite_a8_a8_srcover_sse2;
//  m->raster[CompositeSrcOver][Image::FormatPRGB32].span_composite_a8[Image::FormatI8] = raster_prgb32_span_composite_i8_a8_srcover_sse2;

  m->raster[CompositeSrcOver][Image::FormatPRGB32].span_composite_a8_const[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_const_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatPRGB32].span_composite_a8_const[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_const_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatPRGB32].span_composite_a8_const[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_const_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatPRGB32].span_composite_a8_const[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_a8_const_srcover_sse2;
//  m->raster[CompositeSrcOver][Image::FormatPRGB32].span_composite_a8_const[Image::FormatA8] = raster_prgb32_span_composite_a8_a8_const_srcover_sse2;
//  m->raster[CompositeSrcOver][Image::FormatPRGB32].span_composite_a8_const[Image::FormatI8] = raster_prgb32_span_composite_i8_a8_const_srcover_sse2;

  m->raster[CompositeSrcOver][Image::FormatRGB32].pixel = raster_rgb32_pixel_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatRGB32].pixel_a8 = raster_rgb32_pixel_a8_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatRGB32].span_solid = raster_rgb32_span_solid_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatRGB32].span_solid_a8 = raster_rgb32_span_solid_a8_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatRGB32].span_solid_a8_const = raster_rgb32_span_solid_a8_const_srcover_sse2;

  m->raster[CompositeSrcOver][Image::FormatRGB32].span_composite[Image::FormatARGB32] = raster_rgb32_span_composite_argb32_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatRGB32].span_composite[Image::FormatPRGB32] = raster_rgb32_span_composite_prgb32_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatRGB32].span_composite[Image::FormatRGB32] = raster_rgb32_span_composite_rgb32_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatRGB32].span_composite[Image::FormatRGB24] = raster_rgb32_span_composite_rgb24_srcover_sse2;
//  m->raster[CompositeSrcOver][Image::FormatRGB32].span_composite[Image::FormatA8] = raster_rgb32_span_composite_a8_srcover_sse2;
//  m->raster[CompositeSrcOver][Image::FormatRGB32].span_composite[Image::FormatI8] = raster_rgb32_span_composite_i8_srcover_sse2;

  m->raster[CompositeSrcOver][Image::FormatRGB32].span_composite_a8[Image::FormatARGB32] = raster_rgb32_span_composite_argb32_a8_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatRGB32].span_composite_a8[Image::FormatPRGB32] = raster_rgb32_span_composite_prgb32_a8_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatRGB32].span_composite_a8[Image::FormatRGB32] = raster_rgb32_span_composite_rgb32_a8_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatRGB32].span_composite_a8[Image::FormatRGB24] = raster_rgb32_span_composite_rgb24_a8_srcover_sse2;
//  m->raster[CompositeSrcOver][Image::FormatRGB32].span_composite_a8[Image::FormatA8] = raster_rgb32_span_composite_a8_a8_srcover_sse2;
//  m->raster[CompositeSrcOver][Image::FormatRGB32].span_composite_a8[Image::FormatI8] = raster_rgb32_span_composite_i8_a8_srcover_sse2;

  m->raster[CompositeSrcOver][Image::FormatRGB32].span_composite_a8_const[Image::FormatARGB32] = raster_rgb32_span_composite_argb32_a8_const_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatRGB32].span_composite_a8_const[Image::FormatPRGB32] = raster_rgb32_span_composite_prgb32_a8_const_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatRGB32].span_composite_a8_const[Image::FormatRGB32] = raster_rgb32_span_composite_rgb32_a8_const_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatRGB32].span_composite_a8_const[Image::FormatRGB24] = raster_rgb32_span_composite_rgb24_a8_const_srcover_sse2;
//  m->raster[CompositeSrcOver][Image::FormatRGB32].span_composite_a8_const[Image::FormatA8] = raster_rgb32_span_composite_a8_a8_const_srcover_sse2;
//  m->raster[CompositeSrcOver][Image::FormatRGB32].span_composite_a8_const[Image::FormatI8] = raster_rgb32_span_composite_i8_a8_const_srcover_sse2;

  m->raster[CompositeSrcOver][Image::FormatA8].pixel = raster_a8_pixel_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatA8].pixel_a8 = raster_a8_pixel_a8_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatA8].span_solid = raster_a8_span_solid_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatA8].span_solid_a8 = raster_a8_span_solid_a8_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatA8].span_solid_a8_const = raster_a8_span_solid_a8_const_srcover_sse2;

  m->raster[CompositeSrcOver][Image::FormatA8].span_composite[Image::FormatARGB32] = raster_a8_span_composite_argb32_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatA8].span_composite[Image::FormatPRGB32] = raster_a8_span_composite_prgb32_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatA8].span_composite[Image::FormatRGB32] = raster_a8_span_composite_rgb32_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatA8].span_composite[Image::FormatRGB24] = raster_a8_span_composite_rgb24_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatA8].span_composite[Image::FormatA8] = raster_a8_span_composite_a8_srcover_sse2;

  m->raster[CompositeSrcOver][Image::FormatA8].span_composite_a8[Image::FormatARGB32] = raster_a8_span_composite_argb32_a8_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatA8].span_composite_a8[Image::FormatPRGB32] = raster_a8_span_composite_prgb32_a8_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatA8].span_composite_a8[Image::FormatRGB32] = raster_a8_span_composite_rgb32_a8_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatA8].span_composite_a8[Image::FormatRGB24] = raster_a8_span_composite_rgb24_a8_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatA8].span_composite_a8[Image::FormatA8] = raster_a8_span_composite_a8_a8_srcover_sse2;

  m->raster[CompositeSrcOver][Image::FormatA8].span_composite_a8_const[Image::FormatARGB32] = raster_a8_span_composite_argb32_a8_const_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatA8].span_composite_a8_const[Image::FormatPRGB32] = raster_a8_span_composite_prgb32_a8_const_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatA8].span_composite_a8_const[Image::FormatRGB32] = raster_a8_span_composite_rgb32_a8_const_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatA8].span_composite_a8_const[Image::FormatRGB24] = raster_a8_span_composite_rgb24_a8_const_srcover_sse2;
  m->raster[CompositeSrcOver][Image::FormatA8].span_composite_a8_const[Image::FormatA8] = raster_a8_span_composite_a8_a8_const_srcover_sse2;

  // [Raster - CompositeDestOver]

  m->raster[CompositeDestOver][Image::FormatARGB32].pixel = raster_argb32_pixel_dstover_sse2;
  m->raster[CompositeDestOver][Image::FormatARGB32].pixel_a8 = raster_argb32_pixel_a8_dstover_sse2;
  m->raster[CompositeDestOver][Image::FormatARGB32].span_solid = raster_argb32_span_solid_dstover_sse2;
  m->raster[CompositeDestOver][Image::FormatARGB32].span_solid_a8 = raster_argb32_span_solid_a8_dstover_sse2;
  m->raster[CompositeDestOver][Image::FormatARGB32].span_solid_a8_const = raster_argb32_span_solid_a8_const_dstover_sse2;

  m->raster[CompositeDestOver][Image::FormatARGB32].span_composite[Image::FormatARGB32] = raster_argb32_span_composite_argb32_dstover_sse2;
  m->raster[CompositeDestOver][Image::FormatARGB32].span_composite[Image::FormatPRGB32] = raster_argb32_span_composite_prgb32_dstover_sse2;
  m->raster[CompositeDestOver][Image::FormatARGB32].span_composite[Image::FormatRGB32] = raster_argb32_span_composite_rgb32_dstover_sse2;

  m->raster[CompositeDestOver][Image::FormatARGB32].span_composite_a8[Image::FormatARGB32] = raster_argb32_span_composite_argb32_a8_dstover_sse2;
  m->raster[CompositeDestOver][Image::FormatARGB32].span_composite_a8[Image::FormatPRGB32] = raster_argb32_span_composite_prgb32_a8_dstover_sse2;
  m->raster[CompositeDestOver][Image::FormatARGB32].span_composite_a8[Image::FormatRGB32] = raster_argb32_span_composite_rgb32_a8_dstover_sse2;

  m->raster[CompositeDestOver][Image::FormatARGB32].span_composite_a8_const[Image::FormatARGB32] = raster_argb32_span_composite_argb32_a8_const_dstover_sse2;
  m->raster[CompositeDestOver][Image::FormatARGB32].span_composite_a8_const[Image::FormatPRGB32] = raster_argb32_span_composite_prgb32_a8_const_dstover_sse2;
  m->raster[CompositeDestOver][Image::FormatARGB32].span_composite_a8_const[Image::FormatRGB32] = raster_argb32_span_composite_rgb32_a8_const_dstover_sse2;

  m->raster[CompositeDestOver][Image::FormatPRGB32].pixel = raster_prgb32_pixel_dstover_sse2;
  m->raster[CompositeDestOver][Image::FormatPRGB32].pixel_a8 = raster_prgb32_pixel_a8_dstover_sse2;
  m->raster[CompositeDestOver][Image::FormatPRGB32].span_solid = raster_prgb32_span_solid_dstover_sse2;
  m->raster[CompositeDestOver][Image::FormatPRGB32].span_solid_a8 = raster_prgb32_span_solid_a8_dstover_sse2;
  m->raster[CompositeDestOver][Image::FormatPRGB32].span_solid_a8_const = raster_prgb32_span_solid_a8_const_dstover_sse2;

  m->raster[CompositeDestOver][Image::FormatPRGB32].span_composite[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_dstover_sse2;
  m->raster[CompositeDestOver][Image::FormatPRGB32].span_composite[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_dstover_sse2;
  m->raster[CompositeDestOver][Image::FormatPRGB32].span_composite[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_dstover_sse2;

  m->raster[CompositeDestOver][Image::FormatPRGB32].span_composite_a8[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_dstover_sse2;
  m->raster[CompositeDestOver][Image::FormatPRGB32].span_composite_a8[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_dstover_sse2;
  m->raster[CompositeDestOver][Image::FormatPRGB32].span_composite_a8[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_dstover_sse2;

  m->raster[CompositeDestOver][Image::FormatPRGB32].span_composite_a8_const[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_const_dstover_sse2;
  m->raster[CompositeDestOver][Image::FormatPRGB32].span_composite_a8_const[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_const_dstover_sse2;
  m->raster[CompositeDestOver][Image::FormatPRGB32].span_composite_a8_const[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_const_dstover_sse2;

  // FormatRGB32  - Set to NOPs by Raster_C.cpp
  // FormatRGB24  - Set to NOPs by Raster_C.cpp

  // [Raster - CompositeClear]

  m->raster[CompositeClear][Image::FormatARGB32].pixel = raster_argb32_pixel_clear_sse2;
  m->raster[CompositeClear][Image::FormatARGB32].pixel_a8 = raster_argb32_pixel_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatARGB32].span_solid = raster_argb32_span_solid_clear_sse2;
  m->raster[CompositeClear][Image::FormatARGB32].span_solid_a8 = raster_argb32_span_solid_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatARGB32].span_solid_a8_const = raster_argb32_span_solid_a8_const_clear_sse2;

  m->raster[CompositeClear][Image::FormatARGB32].span_composite[Image::FormatARGB32] = (SpanCompositeFn)raster_argb32_span_solid_clear_sse2;
  m->raster[CompositeClear][Image::FormatARGB32].span_composite[Image::FormatPRGB32] = (SpanCompositeFn)raster_argb32_span_solid_clear_sse2;
  m->raster[CompositeClear][Image::FormatARGB32].span_composite[Image::FormatRGB32] = (SpanCompositeFn)raster_argb32_span_solid_clear_sse2;
  m->raster[CompositeClear][Image::FormatARGB32].span_composite[Image::FormatRGB24] = (SpanCompositeFn)raster_argb32_span_solid_clear_sse2;
  m->raster[CompositeClear][Image::FormatARGB32].span_composite[Image::FormatA8] = (SpanCompositeFn)raster_argb32_span_solid_clear_sse2;
  m->raster[CompositeClear][Image::FormatARGB32].span_composite[Image::FormatI8] = (SpanCompositeFn)raster_argb32_span_solid_clear_sse2;

  m->raster[CompositeClear][Image::FormatARGB32].span_composite_a8[Image::FormatARGB32] = (SpanCompositeMskFn)raster_argb32_span_solid_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatARGB32].span_composite_a8[Image::FormatPRGB32] = (SpanCompositeMskFn)raster_argb32_span_solid_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatARGB32].span_composite_a8[Image::FormatRGB32] = (SpanCompositeMskFn)raster_argb32_span_solid_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatARGB32].span_composite_a8[Image::FormatRGB24] = (SpanCompositeMskFn)raster_argb32_span_solid_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatARGB32].span_composite_a8[Image::FormatA8] = (SpanCompositeMskFn)raster_argb32_span_solid_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatARGB32].span_composite_a8[Image::FormatI8] = (SpanCompositeMskFn)raster_argb32_span_solid_a8_clear_sse2;

  m->raster[CompositeClear][Image::FormatARGB32].span_composite_a8_const[Image::FormatARGB32] = (SpanCompositeMskConstFn)raster_argb32_span_solid_a8_const_clear_sse2;
  m->raster[CompositeClear][Image::FormatARGB32].span_composite_a8_const[Image::FormatPRGB32] = (SpanCompositeMskConstFn)raster_argb32_span_solid_a8_const_clear_sse2;
  m->raster[CompositeClear][Image::FormatARGB32].span_composite_a8_const[Image::FormatRGB32] = (SpanCompositeMskConstFn)raster_argb32_span_solid_a8_const_clear_sse2;
  m->raster[CompositeClear][Image::FormatARGB32].span_composite_a8_const[Image::FormatRGB24] = (SpanCompositeMskConstFn)raster_argb32_span_solid_a8_const_clear_sse2;
  m->raster[CompositeClear][Image::FormatARGB32].span_composite_a8_const[Image::FormatA8] = (SpanCompositeMskConstFn)raster_argb32_span_solid_a8_const_clear_sse2;
  m->raster[CompositeClear][Image::FormatARGB32].span_composite_a8_const[Image::FormatI8] = (SpanCompositeMskConstFn)raster_argb32_span_solid_a8_const_clear_sse2;

  m->raster[CompositeClear][Image::FormatPRGB32].pixel = raster_prgb32_pixel_clear_sse2;
  m->raster[CompositeClear][Image::FormatPRGB32].pixel_a8 = raster_prgb32_pixel_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatPRGB32].span_solid = raster_prgb32_span_solid_clear_sse2;
  m->raster[CompositeClear][Image::FormatPRGB32].span_solid_a8 = raster_prgb32_span_solid_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatPRGB32].span_solid_a8_const = raster_prgb32_span_solid_a8_const_clear_sse2;

  m->raster[CompositeClear][Image::FormatPRGB32].span_composite[Image::FormatARGB32] = (SpanCompositeFn)raster_prgb32_span_solid_clear_sse2;
  m->raster[CompositeClear][Image::FormatPRGB32].span_composite[Image::FormatPRGB32] = (SpanCompositeFn)raster_prgb32_span_solid_clear_sse2;
  m->raster[CompositeClear][Image::FormatPRGB32].span_composite[Image::FormatRGB32] = (SpanCompositeFn)raster_prgb32_span_solid_clear_sse2;
  m->raster[CompositeClear][Image::FormatPRGB32].span_composite[Image::FormatRGB24] = (SpanCompositeFn)raster_prgb32_span_solid_clear_sse2;
  m->raster[CompositeClear][Image::FormatPRGB32].span_composite[Image::FormatA8] = (SpanCompositeFn)raster_prgb32_span_solid_clear_sse2;
  m->raster[CompositeClear][Image::FormatPRGB32].span_composite[Image::FormatI8] = (SpanCompositeFn)raster_prgb32_span_solid_clear_sse2;

  m->raster[CompositeClear][Image::FormatPRGB32].span_composite_a8[Image::FormatARGB32] = (SpanCompositeMskFn)raster_prgb32_span_solid_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatPRGB32].span_composite_a8[Image::FormatPRGB32] = (SpanCompositeMskFn)raster_prgb32_span_solid_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatPRGB32].span_composite_a8[Image::FormatRGB32] = (SpanCompositeMskFn)raster_prgb32_span_solid_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatPRGB32].span_composite_a8[Image::FormatRGB24] = (SpanCompositeMskFn)raster_prgb32_span_solid_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatPRGB32].span_composite_a8[Image::FormatA8] = (SpanCompositeMskFn)raster_prgb32_span_solid_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatPRGB32].span_composite_a8[Image::FormatI8] = (SpanCompositeMskFn)raster_prgb32_span_solid_a8_clear_sse2;

  m->raster[CompositeClear][Image::FormatPRGB32].span_composite_a8_const[Image::FormatARGB32] = (SpanCompositeMskConstFn)raster_prgb32_span_solid_a8_const_clear_sse2;
  m->raster[CompositeClear][Image::FormatPRGB32].span_composite_a8_const[Image::FormatPRGB32] = (SpanCompositeMskConstFn)raster_prgb32_span_solid_a8_const_clear_sse2;
  m->raster[CompositeClear][Image::FormatPRGB32].span_composite_a8_const[Image::FormatRGB32] = (SpanCompositeMskConstFn)raster_prgb32_span_solid_a8_const_clear_sse2;
  m->raster[CompositeClear][Image::FormatPRGB32].span_composite_a8_const[Image::FormatRGB24] = (SpanCompositeMskConstFn)raster_prgb32_span_solid_a8_const_clear_sse2;
  m->raster[CompositeClear][Image::FormatPRGB32].span_composite_a8_const[Image::FormatA8] = (SpanCompositeMskConstFn)raster_prgb32_span_solid_a8_const_clear_sse2;
  m->raster[CompositeClear][Image::FormatPRGB32].span_composite_a8_const[Image::FormatI8] = (SpanCompositeMskConstFn)raster_prgb32_span_solid_a8_const_clear_sse2;

  m->raster[CompositeClear][Image::FormatRGB32].pixel = raster_rgb32_pixel_clear_sse2;
  m->raster[CompositeClear][Image::FormatRGB32].pixel_a8 = raster_rgb32_pixel_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatRGB32].span_solid = raster_rgb32_span_solid_clear_sse2;
  m->raster[CompositeClear][Image::FormatRGB32].span_solid_a8 = raster_rgb32_span_solid_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatRGB32].span_solid_a8_const = raster_rgb32_span_solid_a8_const_clear_sse2;

  m->raster[CompositeClear][Image::FormatRGB32].span_composite[Image::FormatARGB32] = (SpanCompositeFn)raster_rgb32_span_solid_clear_sse2;
  m->raster[CompositeClear][Image::FormatRGB32].span_composite[Image::FormatPRGB32] = (SpanCompositeFn)raster_rgb32_span_solid_clear_sse2;
  m->raster[CompositeClear][Image::FormatRGB32].span_composite[Image::FormatRGB32] = (SpanCompositeFn)raster_rgb32_span_solid_clear_sse2;
  m->raster[CompositeClear][Image::FormatRGB32].span_composite[Image::FormatRGB24] = (SpanCompositeFn)raster_rgb32_span_solid_clear_sse2;
  m->raster[CompositeClear][Image::FormatRGB32].span_composite[Image::FormatA8] = (SpanCompositeFn)raster_rgb32_span_solid_clear_sse2;
  m->raster[CompositeClear][Image::FormatRGB32].span_composite[Image::FormatI8] = (SpanCompositeFn)raster_rgb32_span_solid_clear_sse2;

  m->raster[CompositeClear][Image::FormatRGB32].span_composite_a8[Image::FormatARGB32] = (SpanCompositeMskFn)raster_rgb32_span_solid_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatRGB32].span_composite_a8[Image::FormatPRGB32] = (SpanCompositeMskFn)raster_rgb32_span_solid_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatRGB32].span_composite_a8[Image::FormatRGB32] = (SpanCompositeMskFn)raster_rgb32_span_solid_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatRGB32].span_composite_a8[Image::FormatRGB24] = (SpanCompositeMskFn)raster_rgb32_span_solid_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatRGB32].span_composite_a8[Image::FormatA8] = (SpanCompositeMskFn)raster_rgb32_span_solid_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatRGB32].span_composite_a8[Image::FormatI8] = (SpanCompositeMskFn)raster_rgb32_span_solid_a8_clear_sse2;

  m->raster[CompositeClear][Image::FormatRGB32].span_composite_a8_const[Image::FormatARGB32] = (SpanCompositeMskConstFn)raster_rgb32_span_solid_a8_const_clear_sse2;
  m->raster[CompositeClear][Image::FormatRGB32].span_composite_a8_const[Image::FormatPRGB32] = (SpanCompositeMskConstFn)raster_rgb32_span_solid_a8_const_clear_sse2;
  m->raster[CompositeClear][Image::FormatRGB32].span_composite_a8_const[Image::FormatRGB32] = (SpanCompositeMskConstFn)raster_rgb32_span_solid_a8_const_clear_sse2;
  m->raster[CompositeClear][Image::FormatRGB32].span_composite_a8_const[Image::FormatRGB24] = (SpanCompositeMskConstFn)raster_rgb32_span_solid_a8_const_clear_sse2;
  m->raster[CompositeClear][Image::FormatRGB32].span_composite_a8_const[Image::FormatA8] = (SpanCompositeMskConstFn)raster_rgb32_span_solid_a8_const_clear_sse2;
  m->raster[CompositeClear][Image::FormatRGB32].span_composite_a8_const[Image::FormatI8] = (SpanCompositeMskConstFn)raster_rgb32_span_solid_a8_const_clear_sse2;

  m->raster[CompositeClear][Image::FormatA8].pixel = raster_a8_pixel_clear_sse2;
  m->raster[CompositeClear][Image::FormatA8].pixel_a8 = raster_a8_pixel_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatA8].span_solid = raster_a8_span_solid_clear_sse2;
  m->raster[CompositeClear][Image::FormatA8].span_solid_a8 = raster_a8_span_solid_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatA8].span_solid_a8_const = raster_a8_span_solid_a8_const_clear_sse2;

  m->raster[CompositeClear][Image::FormatA8].span_composite[Image::FormatARGB32] = (SpanCompositeFn)raster_a8_span_solid_clear_sse2;
  m->raster[CompositeClear][Image::FormatA8].span_composite[Image::FormatPRGB32] = (SpanCompositeFn)raster_a8_span_solid_clear_sse2;
  m->raster[CompositeClear][Image::FormatA8].span_composite[Image::FormatRGB32] = (SpanCompositeFn)raster_a8_span_solid_clear_sse2;
  m->raster[CompositeClear][Image::FormatA8].span_composite[Image::FormatRGB24] = (SpanCompositeFn)raster_a8_span_solid_clear_sse2;
  m->raster[CompositeClear][Image::FormatA8].span_composite[Image::FormatA8] = (SpanCompositeFn)raster_a8_span_solid_clear_sse2;
  m->raster[CompositeClear][Image::FormatA8].span_composite[Image::FormatI8] = (SpanCompositeFn)raster_a8_span_solid_clear_sse2;

  m->raster[CompositeClear][Image::FormatA8].span_composite_a8[Image::FormatARGB32] = (SpanCompositeMskFn)raster_a8_span_solid_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatA8].span_composite_a8[Image::FormatPRGB32] = (SpanCompositeMskFn)raster_a8_span_solid_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatA8].span_composite_a8[Image::FormatRGB32] = (SpanCompositeMskFn)raster_a8_span_solid_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatA8].span_composite_a8[Image::FormatRGB24] = (SpanCompositeMskFn)raster_a8_span_solid_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatA8].span_composite_a8[Image::FormatA8] = (SpanCompositeMskFn)raster_a8_span_solid_a8_clear_sse2;
  m->raster[CompositeClear][Image::FormatA8].span_composite_a8[Image::FormatI8] = (SpanCompositeMskFn)raster_a8_span_solid_a8_clear_sse2;

  m->raster[CompositeClear][Image::FormatA8].span_composite_a8_const[Image::FormatARGB32] = (SpanCompositeMskConstFn)raster_a8_span_solid_a8_const_clear_sse2;
  m->raster[CompositeClear][Image::FormatA8].span_composite_a8_const[Image::FormatPRGB32] = (SpanCompositeMskConstFn)raster_a8_span_solid_a8_const_clear_sse2;
  m->raster[CompositeClear][Image::FormatA8].span_composite_a8_const[Image::FormatRGB32] = (SpanCompositeMskConstFn)raster_a8_span_solid_a8_const_clear_sse2;
  m->raster[CompositeClear][Image::FormatA8].span_composite_a8_const[Image::FormatRGB24] = (SpanCompositeMskConstFn)raster_a8_span_solid_a8_const_clear_sse2;
  m->raster[CompositeClear][Image::FormatA8].span_composite_a8_const[Image::FormatA8] = (SpanCompositeMskConstFn)raster_a8_span_solid_a8_const_clear_sse2;
  m->raster[CompositeClear][Image::FormatA8].span_composite_a8_const[Image::FormatI8] = (SpanCompositeMskConstFn)raster_a8_span_solid_a8_const_clear_sse2;

  // [Raster - CompositeAdd]

  m->raster[CompositeAdd][Image::FormatPRGB32].pixel = raster_prgb32_pixel_add_sse2;
  m->raster[CompositeAdd][Image::FormatPRGB32].pixel_a8 = raster_prgb32_pixel_a8_add_sse2;
  m->raster[CompositeAdd][Image::FormatPRGB32].span_solid = raster_prgb32_span_solid_add_sse2;
  m->raster[CompositeAdd][Image::FormatPRGB32].span_solid_a8 = raster_prgb32_span_solid_a8_add_sse2;
  m->raster[CompositeAdd][Image::FormatPRGB32].span_solid_a8_const = raster_prgb32_span_solid_a8_const_add_sse2;

  m->raster[CompositeAdd][Image::FormatPRGB32].span_composite[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_add_sse2;
  m->raster[CompositeAdd][Image::FormatPRGB32].span_composite[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_add_sse2;
  m->raster[CompositeAdd][Image::FormatPRGB32].span_composite[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_add_sse2;
  m->raster[CompositeAdd][Image::FormatPRGB32].span_composite[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_add_sse2;
  m->raster[CompositeAdd][Image::FormatPRGB32].span_composite[Image::FormatA8] = raster_prgb32_span_composite_a8_add_sse2;
  m->raster[CompositeAdd][Image::FormatPRGB32].span_composite[Image::FormatI8] = raster_prgb32_span_composite_i8_add_sse2;

  m->raster[CompositeAdd][Image::FormatPRGB32].span_composite_a8[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_add_sse2;
  m->raster[CompositeAdd][Image::FormatPRGB32].span_composite_a8[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_add_sse2;
  m->raster[CompositeAdd][Image::FormatPRGB32].span_composite_a8[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_add_sse2;
  m->raster[CompositeAdd][Image::FormatPRGB32].span_composite_a8[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_a8_add_sse2;
  m->raster[CompositeAdd][Image::FormatPRGB32].span_composite_a8[Image::FormatA8] = raster_prgb32_span_composite_a8_a8_add_sse2;
  m->raster[CompositeAdd][Image::FormatPRGB32].span_composite_a8[Image::FormatI8] = raster_prgb32_span_composite_i8_a8_add_sse2;

  m->raster[CompositeAdd][Image::FormatPRGB32].span_composite_a8_const[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_const_add_sse2;
  m->raster[CompositeAdd][Image::FormatPRGB32].span_composite_a8_const[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_const_add_sse2;
  m->raster[CompositeAdd][Image::FormatPRGB32].span_composite_a8_const[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_const_add_sse2;
  m->raster[CompositeAdd][Image::FormatPRGB32].span_composite_a8_const[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_a8_const_add_sse2;
  m->raster[CompositeAdd][Image::FormatPRGB32].span_composite_a8_const[Image::FormatA8] = raster_prgb32_span_composite_a8_a8_const_add_sse2;
  m->raster[CompositeAdd][Image::FormatPRGB32].span_composite_a8_const[Image::FormatI8] = raster_prgb32_span_composite_i8_a8_const_add_sse2;

  m->raster[CompositeAdd][Image::FormatRGB32].pixel = raster_rgb32_pixel_add_sse2;
  m->raster[CompositeAdd][Image::FormatRGB32].pixel_a8 = raster_rgb32_pixel_a8_add_sse2;
  m->raster[CompositeAdd][Image::FormatRGB32].span_solid = raster_rgb32_span_solid_add_sse2;
  m->raster[CompositeAdd][Image::FormatRGB32].span_solid_a8 = raster_rgb32_span_solid_a8_add_sse2;
  m->raster[CompositeAdd][Image::FormatRGB32].span_solid_a8_const = raster_rgb32_span_solid_a8_const_add_sse2;

  m->raster[CompositeAdd][Image::FormatRGB32].span_composite[Image::FormatARGB32] = raster_rgb32_span_composite_argb32_add_sse2;
  m->raster[CompositeAdd][Image::FormatRGB32].span_composite[Image::FormatPRGB32] = raster_rgb32_span_composite_prgb32_add_sse2;
  m->raster[CompositeAdd][Image::FormatRGB32].span_composite[Image::FormatRGB32] = raster_rgb32_span_composite_rgb32_add_sse2;
  m->raster[CompositeAdd][Image::FormatRGB32].span_composite[Image::FormatRGB24] = raster_rgb32_span_composite_rgb24_add_sse2;
//m->raster[CompositeAdd][Image::FormatRGB32].span_composite[Image::FormatA8] = NOP;
  m->raster[CompositeAdd][Image::FormatRGB32].span_composite[Image::FormatI8] = raster_rgb32_span_composite_i8_add_sse2;

  m->raster[CompositeAdd][Image::FormatRGB32].span_composite_a8[Image::FormatARGB32] = raster_rgb32_span_composite_argb32_a8_add_sse2;
  m->raster[CompositeAdd][Image::FormatRGB32].span_composite_a8[Image::FormatPRGB32] = raster_rgb32_span_composite_prgb32_a8_add_sse2;
  m->raster[CompositeAdd][Image::FormatRGB32].span_composite_a8[Image::FormatRGB32] = raster_rgb32_span_composite_rgb32_a8_add_sse2;
  m->raster[CompositeAdd][Image::FormatRGB32].span_composite_a8[Image::FormatRGB24] = raster_rgb32_span_composite_rgb24_a8_add_sse2;
//m->raster[CompositeAdd][Image::FormatRGB32].span_composite_a8[Image::FormatA8] = NOP;
  m->raster[CompositeAdd][Image::FormatRGB32].span_composite_a8[Image::FormatI8] = raster_rgb32_span_composite_i8_a8_add_sse2;

  m->raster[CompositeAdd][Image::FormatRGB32].span_composite_a8_const[Image::FormatARGB32] = raster_rgb32_span_composite_argb32_a8_const_add_sse2;
  m->raster[CompositeAdd][Image::FormatRGB32].span_composite_a8_const[Image::FormatPRGB32] = raster_rgb32_span_composite_prgb32_a8_const_add_sse2;
  m->raster[CompositeAdd][Image::FormatRGB32].span_composite_a8_const[Image::FormatRGB32] = raster_rgb32_span_composite_rgb32_a8_const_add_sse2;
  m->raster[CompositeAdd][Image::FormatRGB32].span_composite_a8_const[Image::FormatRGB24] = raster_rgb32_span_composite_rgb24_a8_const_add_sse2;
//m->raster[CompositeAdd][Image::FormatRGB32].span_composite_a8_const[Image::FormatA8] = NOP;
  m->raster[CompositeAdd][Image::FormatRGB32].span_composite_a8_const[Image::FormatI8] = raster_rgb32_span_composite_i8_a8_const_add_sse2;

  m->raster[CompositeAdd][Image::FormatA8].pixel = raster_a8_pixel_add_sse2;
  m->raster[CompositeAdd][Image::FormatA8].pixel_a8 = raster_a8_pixel_a8_add_sse2;
  m->raster[CompositeAdd][Image::FormatA8].span_solid = raster_a8_span_solid_add_sse2;
  m->raster[CompositeAdd][Image::FormatA8].span_solid_a8 = raster_a8_span_solid_a8_add_sse2;
  m->raster[CompositeAdd][Image::FormatA8].span_solid_a8_const = raster_a8_span_solid_a8_const_add_sse2;

  m->raster[CompositeAdd][Image::FormatA8].span_composite[Image::FormatARGB32] = raster_a8_span_composite_argb32_add_sse2;
  m->raster[CompositeAdd][Image::FormatA8].span_composite[Image::FormatPRGB32] = raster_a8_span_composite_prgb32_add_sse2;
  m->raster[CompositeAdd][Image::FormatA8].span_composite[Image::FormatRGB32] = raster_a8_span_composite_rgb32_add_sse2;
  m->raster[CompositeAdd][Image::FormatA8].span_composite[Image::FormatRGB24] = raster_a8_span_composite_rgb24_add_sse2;
  m->raster[CompositeAdd][Image::FormatA8].span_composite[Image::FormatA8] = raster_a8_span_composite_a8_add_sse2;
  m->raster[CompositeAdd][Image::FormatA8].span_composite[Image::FormatI8] = raster_a8_span_composite_i8_add_sse2;

  m->raster[CompositeAdd][Image::FormatA8].span_composite_a8[Image::FormatARGB32] = raster_a8_span_composite_argb32_a8_add_sse2;
  m->raster[CompositeAdd][Image::FormatA8].span_composite_a8[Image::FormatPRGB32] = raster_a8_span_composite_prgb32_a8_add_sse2;
  m->raster[CompositeAdd][Image::FormatA8].span_composite_a8[Image::FormatRGB32] = raster_a8_span_composite_rgb32_a8_add_sse2;
  m->raster[CompositeAdd][Image::FormatA8].span_composite_a8[Image::FormatRGB24] = raster_a8_span_composite_rgb24_a8_add_sse2;
  m->raster[CompositeAdd][Image::FormatA8].span_composite_a8[Image::FormatA8] = raster_a8_span_composite_a8_a8_add_sse2;
  m->raster[CompositeAdd][Image::FormatA8].span_composite_a8[Image::FormatI8] = raster_a8_span_composite_i8_a8_add_sse2;

  m->raster[CompositeAdd][Image::FormatA8].span_composite_a8_const[Image::FormatARGB32] = raster_a8_span_composite_argb32_a8_const_add_sse2;
  m->raster[CompositeAdd][Image::FormatA8].span_composite_a8_const[Image::FormatPRGB32] = raster_a8_span_composite_prgb32_a8_const_add_sse2;
  m->raster[CompositeAdd][Image::FormatA8].span_composite_a8_const[Image::FormatRGB32] = raster_a8_span_composite_rgb32_a8_const_add_sse2;
  m->raster[CompositeAdd][Image::FormatA8].span_composite_a8_const[Image::FormatRGB24] = raster_a8_span_composite_rgb24_a8_const_add_sse2;
  m->raster[CompositeAdd][Image::FormatA8].span_composite_a8_const[Image::FormatA8] = raster_a8_span_composite_a8_a8_const_add_sse2;
  m->raster[CompositeAdd][Image::FormatA8].span_composite_a8_const[Image::FormatI8] = raster_a8_span_composite_i8_a8_const_add_sse2;

  // TODO

  // [Filters - ColorMatrix]

  m->filter.colorMatrix[Image::FormatARGB32] = colorMatrix_argb32_sse2;
  m->filter.colorMatrix[Image::FormatRGB32] = colorMatrix_rgb32_sse2;
  m->filter.colorMatrix[Image::FormatRGB24] = colorMatrix_rgb24_sse2;
  m->filter.colorMatrix[Image::FormatA8] = colorMatrix_a8_sse2;

  // [Filters - FloatConvolve / IntegerConvolve]

  m->filter.floatScanlineConvolveH[Image::FormatARGB32] = floatScanlineConvolveH_argb32_sse2;
  m->filter.floatScanlineConvolveV[Image::FormatARGB32] = floatScanlineConvolveV_argb32_sse2;

  // [Filters - BoxBlur]

  m->filter.boxBlurConvolveH[Image::FormatARGB32] = boxBlurConvolveH_argb32_sse2;
  m->filter.boxBlurConvolveV[Image::FormatARGB32] = boxBlurConvolveV_argb32_sse2;

  // [Filters - StackBlur]

  m->filter.stackBlurConvolveH[Image::FormatARGB32] = stackBlurConvolveH_argb32_sse2;
  m->filter.stackBlurConvolveV[Image::FormatARGB32] = stackBlurConvolveV_argb32_sse2;
}
