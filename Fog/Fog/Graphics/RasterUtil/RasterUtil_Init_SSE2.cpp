// [Fog/Graphics Library - Private API]
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
#include <Fog/Graphics/ByteUtil_p.h>
#include <Fog/Graphics/ColorLut.h>
#include <Fog/Graphics/ColorMatrix.h>
#include <Fog/Graphics/Constants_p.h>
#include <Fog/Graphics/DitherMatrix_p.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/ImageFilter.h>
#include <Fog/Graphics/Matrix.h>
#include <Fog/Graphics/Path.h>
#include <Fog/Graphics/Pattern.h>
#include <Fog/Graphics/RasterUtil_p.h>

// [Raster_SSE2]
#include <Fog/Graphics/RasterUtil/RasterUtil_C_p.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Defs_SSE2_p.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Composite_SSE2_p.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Dib_C_p.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Dib_SSE2_p.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Interpolate_SSE2_p.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Pattern_SSE2_p.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Scale_SSE2_p.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Filters_SSE2_p.h>

// ============================================================================
// [Library Initializers]
// ============================================================================

static void fog_raster_init_sse2_constants(void)
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

  // RGB16 masks
  Mask_0000F8000000F800_0000F8000000F800 = pix_create_mask_8x2W(0x0000, 0xF800, 0x0000, 0xF800, 0x0000, 0xF800, 0x0000, 0xF800);
  Mask_0000E0000000E000_0000E0000000E000 = pix_create_mask_8x2W(0x0000, 0xE000, 0x0000, 0xE000, 0x0000, 0xE000, 0x0000, 0xE000);
  Mask_00007C0000007C00_00007C0000007C00 = pix_create_mask_8x2W(0x0000, 0x7C00, 0x0000, 0x7C00, 0x0000, 0x7C00, 0x0000, 0x7C00);
  Mask_0000700000007000_0000700000007000 = pix_create_mask_8x2W(0x0000, 0x7000, 0x0000, 0x7000, 0x0000, 0x7000, 0x0000, 0x7000);
  Mask_0000380000003800_0000380000003800 = pix_create_mask_8x2W(0x0000, 0x3800, 0x0000, 0x3800, 0x0000, 0x3800, 0x0000, 0x3800);
  Mask_00001F0000001F00_00001F0000001F00 = pix_create_mask_8x2W(0x0000, 0x1F00, 0x0000, 0x1F00, 0x0000, 0x1F00, 0x0000, 0x1F00);
  Mask_00001C0000001C00_00001C0000001C00 = pix_create_mask_8x2W(0x0000, 0x1C00, 0x0000, 0x1C00, 0x0000, 0x1C00, 0x0000, 0x1C00);
  Mask_000007E0000007E0_000007E0000007E0 = pix_create_mask_8x2W(0x0000, 0x07E0, 0x0000, 0x07E0, 0x0000, 0x07E0, 0x0000, 0x07E0);
  Mask_000003E0000003E0_000003E0000003E0 = pix_create_mask_8x2W(0x0000, 0x03E0, 0x0000, 0x03E0, 0x0000, 0x03E0, 0x0000, 0x03E0);
  Mask_000000F8000000F8_000000F8000000F8 = pix_create_mask_8x2W(0x0000, 0x00F8, 0x0000, 0x00F8, 0x0000, 0x00F8, 0x0000, 0x00F8);
  Mask_0000007C0000007C_0000007C0000007C = pix_create_mask_8x2W(0x0000, 0x007C, 0x0000, 0x007C, 0x0000, 0x007C, 0x0000, 0x007C);
  Mask_0000001F0000001F_0000001F0000001F = pix_create_mask_8x2W(0x0000, 0x001F, 0x0000, 0x001F, 0x0000, 0x001F, 0x0000, 0x001F);
  Mask_0000000700000007_0000000700000007 = pix_create_mask_8x2W(0x0000, 0x0007, 0x0000, 0x0007, 0x0000, 0x0007, 0x0000, 0x0007);
  Mask_0000000300000003_0000000300000003 = pix_create_mask_8x2W(0x0000, 0x0003, 0x0000, 0x0003, 0x0000, 0x0003, 0x0000, 0x0003);

  // D masks
  Mask_0001000000000000_0000000000000000 = pix_create_mask_4x1D(0x00010000, 0x00000000, 0x00000000, 0x00000000);
  Mask_0001000000010000_0000000000000000 = pix_create_mask_4x1D(0x00010000, 0x00010000, 0x00000000, 0x00000000);

  Mask_3F80000000000000_0000000000000000 = pix_create_mask_4x1D(0x3F800000, 0x00000000, 0x00000000, 0x00000000);

  // Float / Double masks
  sse2_t t;

  t.uq[0] = ~(FOG_UINT64_C(0x8000000080000000));
  t.uq[1] = ~(FOG_UINT64_C(0x8000000080000000));
  Mask_7FFFFFFF7FFFFFFF_7FFFFFFF7FFFFFFF = _mm_loadu_ps((float*)&t);

  t.uq[0] = ~(FOG_UINT64_C(0x8000000000000000));
  t.uq[1] = ~(FOG_UINT64_C(0x8000000000000000));
  Mask_7FFFFFFFFFFFFFFF_7FFFFFFFFFFFFFFF = _mm_loadu_pd((double*)&t);

  MaskD_0_5_0_5 = _mm_set1_pd(0.5);
  MaskD_1_0_1_0 = _mm_set1_pd(1.0);
  MaskD_65536_0_65536_0 = _mm_set1_pd(65536.0);
  MaskD_32768_0_32768_0 = _mm_set1_pd(32768.0);
}

FOG_INIT_DECLARE void fog_raster_init_sse2(void)
{
  using namespace Fog;
  using namespace Fog::RasterUtil;

  // [Constants]

  fog_raster_init_sse2_constants();

  FunctionMap* m = functionMap;

  // [Dib - MemCpy]

  m->dib.memcpy8 = DibSSE2::memcpy8;
  m->dib.memcpy16 = DibSSE2::memcpy16;
  m->dib.memcpy24 = DibSSE2::memcpy24;
  m->dib.memcpy32 = DibSSE2::memcpy32;

  // [Dib - Convert]

  m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_PRGB32_NATIVE    ] = DibSSE2::memcpy32;
  m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_ARGB32_NATIVE    ] = DibSSE2::prgb32_from_argb32;
  m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_XRGB32_NATIVE    ] = DibSSE2::frgb32_from_xrgb32;
  m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_A8               ] = DibSSE2::azzz32_from_a8;
  m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_I8               ] = DibSSE2::prgb32_from_i8;
//m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_PRGB32_SWAPPED   ] = DibSSE2::bswap32;
  m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_ARGB32_SWAPPED   ] = DibSSE2::prgb32_from_argb32_swapped;
//m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_XRGB32_SWAPPED   ] = DibSSE2::frgb32_from_xrgb32_swapped;
  m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_RGB24_NATIVE     ] = DibSSE2::frgb32_from_rgb24_native;
  m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_RGB24_SWAPPED    ] = DibSSE2::frgb32_from_rgb24_swapped;
//m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_RGB16_565_NATIVE ] = DibSSE2::frgb32_from_rgb16_565_native;
//m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_RGB16_565_SWAPPED] = DibSSE2::frgb32_from_rgb16_565_swapped;
//m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_RGB16_555_NATIVE ] = DibSSE2::frgb32_from_rgb16_555_native;
//m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_RGB16_555_SWAPPED] = DibSSE2::frgb32_from_rgb16_555_swapped;
//m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_GREY8            ] = DibSSE2::frgb32_from_grey8;

  m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_PRGB32_NATIVE    ] = DibSSE2::argb32_from_prgb32;
  m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_ARGB32_NATIVE    ] = DibSSE2::memcpy32;
  m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_XRGB32_NATIVE    ] = DibSSE2::frgb32_from_xrgb32;
  m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_A8               ] = DibSSE2::azzz32_from_a8;
  m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_I8               ] = DibSSE2::argb32_from_i8;
//m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_PRGB32_SWAPPED   ] = DibSSE2::argb32_from_prgb32_swapped;
//m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_ARGB32_SWAPPED   ] = DibSSE2::bswap32;
//m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_XRGB32_SWAPPED   ] = DibSSE2::frgb32_from_xrgb32_swapped;
  m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_RGB24_NATIVE     ] = DibSSE2::frgb32_from_rgb24_native;
  m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_RGB24_SWAPPED    ] = DibSSE2::frgb32_from_rgb24_swapped;
//m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_RGB16_565_NATIVE ] = DibSSE2::frgb32_from_rgb16_565_native;
//m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_RGB16_565_SWAPPED] = DibSSE2::frgb32_from_rgb16_565_swapped;
//m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_RGB16_555_NATIVE ] = DibSSE2::frgb32_from_rgb16_555_native;
//m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_RGB16_555_SWAPPED] = DibSSE2::frgb32_from_rgb16_555_swapped;
//m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_GREY8            ] = DibSSE2::frgb32_from_grey8;

  m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_PRGB32_NATIVE    ] = DibSSE2::frgb32_from_xrgb32;
  m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_ARGB32_NATIVE    ] = DibSSE2::frgb32_from_argb32;
  m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_XRGB32_NATIVE    ] = DibSSE2::frgb32_from_xrgb32;
//m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_A8               ] = DibSSE2::fzzz32_from_null;
  m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_I8               ] = DibSSE2::frgb32_from_i8;
//m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_PRGB32_SWAPPED   ] = DibSSE2::frgb32_from_xrgb32_swapped;
  m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_ARGB32_SWAPPED   ] = DibSSE2::frgb32_from_argb32_swapped;
//m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_XRGB32_SWAPPED   ] = DibSSE2::bswap32;
  m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_RGB24_NATIVE     ] = DibSSE2::frgb32_from_rgb24_native;
  m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_RGB24_SWAPPED    ] = DibSSE2::frgb32_from_rgb24_swapped;
//m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_RGB16_565_NATIVE ] = DibSSE2::frgb32_from_rgb16_565_native;
//m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_RGB16_565_SWAPPED] = DibSSE2::frgb32_from_rgb16_565_swapped;
//m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_RGB16_555_NATIVE ] = DibSSE2::frgb32_from_rgb16_555_native;
//m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_RGB16_555_SWAPPED] = DibSSE2::frgb32_from_rgb16_555_swapped;
//m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_GREY8            ] = DibSSE2::frgb32_from_grey8;

  m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_PRGB32_NATIVE    ] = DibSSE2::a8_from_axxx32;
  m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_ARGB32_NATIVE    ] = DibSSE2::a8_from_axxx32;
//m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_XRGB32_NATIVE    ] = DibSSE2::f8_from_null;
  m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_A8               ] = DibSSE2::memcpy8;
  m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_I8               ] = DibSSE2::a8_from_i8;
//m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_PRGB32_SWAPPED   ] = DibSSE2::a8_from_axxx32_swapped;
//m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_ARGB32_SWAPPED   ] = DibSSE2::a8_from_axxx32_swapped;
//m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_XRGB32_SWAPPED   ] = DibSSE2::f8_from_null;
//m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_RGB24_NATIVE     ] = DibSSE2::f8_from_null;
//m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_RGB24_SWAPPED    ] = DibSSE2::f8_from_null;
//m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_RGB16_565_NATIVE ] = DibSSE2::f8_from_null;
//m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_RGB16_565_SWAPPED] = DibSSE2::f8_from_null;
//m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_RGB16_555_NATIVE ] = DibSSE2::f8_from_null;
//m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_RGB16_555_SWAPPED] = DibSSE2::f8_from_null;
//m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_GREY8            ] = DibSSE2::f8_from_null;

//m->dib.convert[DIB_FORMAT_PRGB32_SWAPPED   ][PIXEL_FORMAT_PRGB32] = DibSSE2::bswap32;
  m->dib.convert[DIB_FORMAT_PRGB32_SWAPPED   ][PIXEL_FORMAT_ARGB32] = DibSSE2::prgb32_swapped_from_argb32;
//m->dib.convert[DIB_FORMAT_PRGB32_SWAPPED   ][PIXEL_FORMAT_XRGB32] = DibSSE2::frgb32_swapped_from_xrgb32;
//m->dib.convert[DIB_FORMAT_PRGB32_SWAPPED   ][PIXEL_FORMAT_A8    ] = DibSSE2::azzz32_swapped_from_a8;
//m->dib.convert[DIB_FORMAT_PRGB32_SWAPPED   ][PIXEL_FORMAT_I8    ] = DibSSE2::prgb32_swapped_from_i8;

//m->dib.convert[DIB_FORMAT_ARGB32_SWAPPED   ][PIXEL_FORMAT_PRGB32] = DibSSE2::argb32_swapped_from_prgb32;
//m->dib.convert[DIB_FORMAT_ARGB32_SWAPPED   ][PIXEL_FORMAT_ARGB32] = DibSSE2::bswap32;
//m->dib.convert[DIB_FORMAT_ARGB32_SWAPPED   ][PIXEL_FORMAT_XRGB32] = DibSSE2::frgb32_swapped_from_xrgb32;
//m->dib.convert[DIB_FORMAT_ARGB32_SWAPPED   ][PIXEL_FORMAT_A8    ] = DibSSE2::azzz32_swapped_from_a8;
//m->dib.convert[DIB_FORMAT_ARGB32_SWAPPED   ][PIXEL_FORMAT_I8    ] = DibSSE2::argb32_swapped_from_i8;

//m->dib.convert[DIB_FORMAT_XRGB32_SWAPPED   ][PIXEL_FORMAT_PRGB32] = DibSSE2::frgb32_swapped_from_xrgb32;
//m->dib.convert[DIB_FORMAT_XRGB32_SWAPPED   ][PIXEL_FORMAT_ARGB32] = DibSSE2::frgb32_swapped_from_argb32;
//m->dib.convert[DIB_FORMAT_XRGB32_SWAPPED   ][PIXEL_FORMAT_XRGB32] = DibSSE2::frgb32_swapped_from_xrgb32;
//m->dib.convert[DIB_FORMAT_XRGB32_SWAPPED   ][PIXEL_FORMAT_A8    ] = DibSSE2::fzzz32_swapped_from_null;
//m->dib.convert[DIB_FORMAT_XRGB32_SWAPPED   ][PIXEL_FORMAT_I8    ] = DibSSE2::frgb32_swapped_from_i8;

//m->dib.convert[DIB_FORMAT_RGB24_NATIVE     ][PIXEL_FORMAT_PRGB32] = DibSSE2::rgb24_native_from_xrgb32;
//m->dib.convert[DIB_FORMAT_RGB24_NATIVE     ][PIXEL_FORMAT_ARGB32] = DibSSE2::rgb24_native_from_argb32;
//m->dib.convert[DIB_FORMAT_RGB24_NATIVE     ][PIXEL_FORMAT_XRGB32] = DibSSE2::rgb24_native_from_xrgb32;
//m->dib.convert[DIB_FORMAT_RGB24_NATIVE     ][PIXEL_FORMAT_A8    ] = DibSSE2::zzz24_from_null;
//m->dib.convert[DIB_FORMAT_RGB24_NATIVE     ][PIXEL_FORMAT_I8    ] = DibSSE2::rgb24_native_from_i8;

//m->dib.convert[DIB_FORMAT_RGB24_SWAPPED    ][PIXEL_FORMAT_PRGB32] = DibSSE2::rgb24_swapped_from_xrgb32;
//m->dib.convert[DIB_FORMAT_RGB24_SWAPPED    ][PIXEL_FORMAT_ARGB32] = DibSSE2::rgb24_swapped_from_argb32;
//m->dib.convert[DIB_FORMAT_RGB24_SWAPPED    ][PIXEL_FORMAT_XRGB32] = DibSSE2::rgb24_swapped_from_xrgb32;
//m->dib.convert[DIB_FORMAT_RGB24_SWAPPED    ][PIXEL_FORMAT_A8    ] = DibSSE2::zzz24_from_null;
//m->dib.convert[DIB_FORMAT_RGB24_SWAPPED    ][PIXEL_FORMAT_I8    ] = DibSSE2::rgb24_swapped_from_i8;

  m->dib.convert[DIB_FORMAT_RGB16_565_NATIVE ][PIXEL_FORMAT_PRGB32] = DibSSE2::rgb16_565_native_from_xrgb32;
//m->dib.convert[DIB_FORMAT_RGB16_565_NATIVE ][PIXEL_FORMAT_ARGB32] = DibSSE2::rgb16_565_native_from_argb32;
  m->dib.convert[DIB_FORMAT_RGB16_565_NATIVE ][PIXEL_FORMAT_XRGB32] = DibSSE2::rgb16_565_native_from_xrgb32;
//m->dib.convert[DIB_FORMAT_RGB16_565_NATIVE ][PIXEL_FORMAT_A8    ] = DibSSE2::zzz16_from_null;
//m->dib.convert[DIB_FORMAT_RGB16_565_NATIVE ][PIXEL_FORMAT_I8    ] = DibSSE2::rgb16_565_native_from_i8;

  m->dib.convert[DIB_FORMAT_RGB16_565_SWAPPED][PIXEL_FORMAT_PRGB32] = DibSSE2::rgb16_565_swapped_from_xrgb32;
//m->dib.convert[DIB_FORMAT_RGB16_565_SWAPPED][PIXEL_FORMAT_ARGB32] = DibSSE2::rgb16_565_swapped_from_argb32;
  m->dib.convert[DIB_FORMAT_RGB16_565_SWAPPED][PIXEL_FORMAT_XRGB32] = DibSSE2::rgb16_565_swapped_from_xrgb32;
//m->dib.convert[DIB_FORMAT_RGB16_565_SWAPPED][PIXEL_FORMAT_A8    ] = DibSSE2::zzz16_from_null;
//m->dib.convert[DIB_FORMAT_RGB16_565_SWAPPED][PIXEL_FORMAT_I8    ] = DibSSE2::rgb16_565_swapped_from_i8;

  m->dib.convert[DIB_FORMAT_RGB16_555_NATIVE ][PIXEL_FORMAT_PRGB32] = DibSSE2::rgb16_555_native_from_xrgb32;
//m->dib.convert[DIB_FORMAT_RGB16_555_NATIVE ][PIXEL_FORMAT_ARGB32] = DibSSE2::rgb16_555_native_from_argb32;
  m->dib.convert[DIB_FORMAT_RGB16_555_NATIVE ][PIXEL_FORMAT_XRGB32] = DibSSE2::rgb16_555_native_from_xrgb32;
//m->dib.convert[DIB_FORMAT_RGB16_555_NATIVE ][PIXEL_FORMAT_A8    ] = DibSSE2::zzz16_from_null;
//m->dib.convert[DIB_FORMAT_RGB16_555_NATIVE ][PIXEL_FORMAT_I8    ] = DibSSE2::rgb16_555_native_from_i8;

  m->dib.convert[DIB_FORMAT_RGB16_555_SWAPPED][PIXEL_FORMAT_PRGB32] = DibSSE2::rgb16_555_swapped_from_xrgb32;
//m->dib.convert[DIB_FORMAT_RGB16_555_SWAPPED][PIXEL_FORMAT_ARGB32] = DibSSE2::rgb16_555_swapped_from_argb32;
  m->dib.convert[DIB_FORMAT_RGB16_555_SWAPPED][PIXEL_FORMAT_XRGB32] = DibSSE2::rgb16_555_swapped_from_xrgb32;
//m->dib.convert[DIB_FORMAT_RGB16_555_SWAPPED][PIXEL_FORMAT_A8    ] = DibSSE2::zzz16_from_null;
//m->dib.convert[DIB_FORMAT_RGB16_555_SWAPPED][PIXEL_FORMAT_I8    ] = DibSSE2::rgb16_555_swapped_from_i8;

//m->dib.convert[DIB_FORMAT_GREY8            ][PIXEL_FORMAT_PRGB32] = DibSSE2::grey8_from_xrgb32;
//m->dib.convert[DIB_FORMAT_GREY8            ][PIXEL_FORMAT_ARGB32] = DibSSE2::grey8_from_argb32;
//m->dib.convert[DIB_FORMAT_GREY8            ][PIXEL_FORMAT_XRGB32] = DibSSE2::grey8_from_xrgb32;
//m->dib.convert[DIB_FORMAT_GREY8            ][PIXEL_FORMAT_A8    ] = DibSSE2::z8_from_null;
//m->dib.convert[DIB_FORMAT_GREY8            ][PIXEL_FORMAT_I8    ] = DibSSE2::grey8_from_i8;

  // [Interpolate - Gradient]

  m->interpolate.gradient[PIXEL_FORMAT_PRGB32] = InterpolateSSE2::gradient_prgb32;
  m->interpolate.gradient[PIXEL_FORMAT_ARGB32] = InterpolateSSE2::gradient_argb32;
  m->interpolate.gradient[PIXEL_FORMAT_XRGB32] = InterpolateSSE2::gradient_argb32;

  // [Pattern - Texture]

  // TODO: None, Pad

  m->pattern.texture_fetch_exact[PIXEL_FORMAT_PRGB32][SPREAD_REPEAT] = PatternSSE2::texture_fetch_exact_repeat_32;
  m->pattern.texture_fetch_exact[PIXEL_FORMAT_ARGB32][SPREAD_REPEAT] = PatternSSE2::texture_fetch_exact_repeat_32;
  m->pattern.texture_fetch_exact[PIXEL_FORMAT_XRGB32][SPREAD_REPEAT] = PatternSSE2::texture_fetch_exact_repeat_32;

  m->pattern.texture_fetch_exact[PIXEL_FORMAT_PRGB32][SPREAD_REFLECT] = PatternSSE2::texture_fetch_exact_reflect_32;
  m->pattern.texture_fetch_exact[PIXEL_FORMAT_ARGB32][SPREAD_REFLECT] = PatternSSE2::texture_fetch_exact_reflect_32;
  m->pattern.texture_fetch_exact[PIXEL_FORMAT_XRGB32][SPREAD_REFLECT] = PatternSSE2::texture_fetch_exact_reflect_32;

  m->pattern.texture_fetch_transform_bilinear[PIXEL_FORMAT_PRGB32][SPREAD_NONE] = PatternSSE2::texture_fetch_transform_bilinear_none_32;
  m->pattern.texture_fetch_transform_bilinear[PIXEL_FORMAT_ARGB32][SPREAD_NONE] = PatternSSE2::texture_fetch_transform_bilinear_none_32;
  m->pattern.texture_fetch_transform_bilinear[PIXEL_FORMAT_XRGB32][SPREAD_NONE] = PatternSSE2::texture_fetch_transform_bilinear_none_32;

  m->pattern.texture_fetch_transform_bilinear[PIXEL_FORMAT_PRGB32][SPREAD_PAD] = PatternSSE2::texture_fetch_transform_bilinear_pad_32;
  m->pattern.texture_fetch_transform_bilinear[PIXEL_FORMAT_ARGB32][SPREAD_PAD] = PatternSSE2::texture_fetch_transform_bilinear_pad_32;
  m->pattern.texture_fetch_transform_bilinear[PIXEL_FORMAT_XRGB32][SPREAD_PAD] = PatternSSE2::texture_fetch_transform_bilinear_pad_32;

  m->pattern.texture_fetch_transform_bilinear[PIXEL_FORMAT_PRGB32][SPREAD_REPEAT] = PatternSSE2::texture_fetch_transform_bilinear_repeat_32;
  m->pattern.texture_fetch_transform_bilinear[PIXEL_FORMAT_ARGB32][SPREAD_REPEAT] = PatternSSE2::texture_fetch_transform_bilinear_repeat_32;
  m->pattern.texture_fetch_transform_bilinear[PIXEL_FORMAT_XRGB32][SPREAD_REPEAT] = PatternSSE2::texture_fetch_transform_bilinear_repeat_32;

  // TODO: Reflect

  // [Pattern - Linear Gradient]

  m->pattern.linear_gradient_fetch_subxy[SPREAD_NONE] = PatternSSE2::linear_gradient_fetch_subxy_pad;
  m->pattern.linear_gradient_fetch_subxy[SPREAD_PAD] = PatternSSE2::linear_gradient_fetch_subxy_pad;
  m->pattern.linear_gradient_fetch_subxy[SPREAD_REPEAT] = PatternSSE2::linear_gradient_fetch_subxy_repeat;
  m->pattern.linear_gradient_fetch_subxy[SPREAD_REFLECT] = PatternSSE2::linear_gradient_fetch_subxy_repeat;

  // [Pattern - Radial Gradient]

  m->pattern.radial_gradient_fetch[SPREAD_NONE] = PatternSSE2::radial_gradient_fetch_pad;
  m->pattern.radial_gradient_fetch[SPREAD_PAD] = PatternSSE2::radial_gradient_fetch_pad;

  // [Filters - ColorMatrix]

  m->filter.color_matrix[PIXEL_FORMAT_PRGB32] = FilterSSE2::color_matrix_prgb32;
  m->filter.color_matrix[PIXEL_FORMAT_ARGB32] = FilterSSE2::color_matrix_argb32;
  m->filter.color_matrix[PIXEL_FORMAT_XRGB32] = FilterSSE2::color_matrix_xrgb32;
  m->filter.color_matrix[PIXEL_FORMAT_A8] = FilterSSE2::color_matrix_a8;

  // [Filters - BoxBlur]

  m->filter.box_blur_h[PIXEL_FORMAT_ARGB32] = FilterSSE2::box_blur_h_argb32;
  m->filter.box_blur_v[PIXEL_FORMAT_ARGB32] = FilterSSE2::box_blur_v_argb32;

  m->filter.box_blur_h[PIXEL_FORMAT_XRGB32] = FilterSSE2::box_blur_h_xrgb32;
  m->filter.box_blur_v[PIXEL_FORMAT_XRGB32] = FilterSSE2::box_blur_v_xrgb32;

  // [Filters - LinearBlur]

  m->filter.linear_blur_h[PIXEL_FORMAT_ARGB32] = FilterSSE2::linear_blur_h_argb32;
  m->filter.linear_blur_h[PIXEL_FORMAT_XRGB32] = FilterSSE2::linear_blur_h_argb32;

  m->filter.linear_blur_v[PIXEL_FORMAT_ARGB32] = FilterSSE2::linear_blur_v_argb32;
  m->filter.linear_blur_v[PIXEL_FORMAT_XRGB32] = FilterSSE2::linear_blur_v_argb32;

  // [Filters - SymmetricConvolveFloat]

  m->filter.symmetric_convolve_float_h[PIXEL_FORMAT_ARGB32] = FilterSSE2::symmetric_convolve_float_h_argb32;
  m->filter.symmetric_convolve_float_h[PIXEL_FORMAT_XRGB32] = FilterSSE2::symmetric_convolve_float_h_argb32;
  m->filter.symmetric_convolve_float_v[PIXEL_FORMAT_ARGB32] = FilterSSE2::symmetric_convolve_float_v_argb32;
  m->filter.symmetric_convolve_float_v[PIXEL_FORMAT_XRGB32] = FilterSSE2::symmetric_convolve_float_v_argb32;

  // [Composite - Src]

  m->composite[OPERATOR_SRC][PIXEL_FORMAT_PRGB32].cspan = CompositeSrcSSE2::prgb32_cspan;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeSrcSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeSrcSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_PRGB32].cspan_a8_scanline = CompositeSrcSSE2::prgb32_cspan_a8_scanline;

  m->composite[OPERATOR_SRC][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = DibSSE2::memcpy32;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = DibSSE2::prgb32_from_argb32;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = DibSSE2::frgb32_from_xrgb32;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_A8] = DibSSE2::azzz32_from_a8;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_I8] = DibSSE2::prgb32_from_i8;

  m->composite[OPERATOR_SRC][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeSrcSSE2::prgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeSrcSSE2::prgb32_vspan_argb32_a8;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeSrcSSE2::prgb32_vspan_xrgb32_a8;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_A8] = CompositeSrcSSE2::prgb32_vspan_a8_a8;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_I8] = CompositeSrcSSE2::prgb32_vspan_i8_a8;

  m->composite[OPERATOR_SRC][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeSrcSSE2::prgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeSrcSSE2::prgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeSrcSSE2::prgb32_vspan_xrgb32_a8_const;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_A8] = CompositeSrcSSE2::prgb32_vspan_a8_a8_const;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_I8] = CompositeSrcSSE2::prgb32_vspan_i8_a8_const;

  m->composite[OPERATOR_SRC][PIXEL_FORMAT_XRGB32].cspan = CompositeSrcSSE2::xrgb32_cspan;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeSrcSSE2::xrgb32_cspan_a8;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeSrcSSE2::xrgb32_cspan_a8_const;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_XRGB32].cspan_a8_scanline = CompositeSrcSSE2::xrgb32_cspan_a8_scanline;

  m->composite[OPERATOR_SRC][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = DibSSE2::frgb32_from_xrgb32;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = DibSSE2::frgb32_from_argb32;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = DibSSE2::memcpy32;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_A8] = (VSpanFn)CompositeClearSSE2::xrgb32_cspan;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_I8] = DibSSE2::frgb32_from_i8;

  m->composite[OPERATOR_SRC][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeSrcSSE2::xrgb32_vspan_xrgb32_a8;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeSrcSSE2::xrgb32_vspan_argb32_a8;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeSrcSSE2::xrgb32_vspan_xrgb32_a8;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_A8] = (VSpanMskFn)CompositeClearSSE2::xrgb32_cspan_a8;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_I8] = CompositeSrcSSE2::xrgb32_vspan_i8_a8;

  m->composite[OPERATOR_SRC][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeSrcSSE2::xrgb32_vspan_xrgb32_a8_const;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeSrcSSE2::xrgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeSrcSSE2::xrgb32_vspan_xrgb32_a8_const;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_A8] = (VSpanMskConstFn)CompositeClearSSE2::xrgb32_cspan_a8_const;
  m->composite[OPERATOR_SRC][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_I8] = CompositeSrcSSE2::xrgb32_vspan_i8_a8_const;

  // [Composite - SrcOver]

  m->composite[OPERATOR_SRC_OVER][PIXEL_FORMAT_PRGB32].cspan = CompositeSrcOverSSE2::prgb32_cspan;
  m->composite[OPERATOR_SRC_OVER][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeSrcOverSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_SRC_OVER][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeSrcOverSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_SRC_OVER][PIXEL_FORMAT_PRGB32].cspan_a8_scanline = CompositeSrcOverSSE2::prgb32_cspan_a8_scanline;

  m->composite[OPERATOR_SRC_OVER][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeSrcOverSSE2::prgb32_vspan_prgb32;
  m->composite[OPERATOR_SRC_OVER][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeSrcOverSSE2::prgb32_vspan_argb32;
  m->composite[OPERATOR_SRC_OVER][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = DibSSE2::frgb32_from_xrgb32;

  m->composite[OPERATOR_SRC_OVER][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeSrcOverSSE2::prgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_SRC_OVER][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeSrcOverSSE2::prgb32_vspan_argb32_a8;
  m->composite[OPERATOR_SRC_OVER][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeSrcOverSSE2::prgb32_vspan_xrgb32_a8;

  m->composite[OPERATOR_SRC_OVER][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeSrcOverSSE2::prgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_SRC_OVER][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeSrcOverSSE2::prgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_SRC_OVER][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeSrcOverSSE2::prgb32_vspan_xrgb32_a8_const;

  m->composite[OPERATOR_SRC_OVER][PIXEL_FORMAT_XRGB32].cspan = CompositeSrcOverSSE2::xrgb32_cspan;
  m->composite[OPERATOR_SRC_OVER][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeSrcOverSSE2::xrgb32_cspan_a8;
  m->composite[OPERATOR_SRC_OVER][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeSrcOverSSE2::xrgb32_cspan_a8_const;
  m->composite[OPERATOR_SRC_OVER][PIXEL_FORMAT_XRGB32].cspan_a8_scanline = CompositeSrcOverSSE2::xrgb32_cspan_a8_scanline;

  m->composite[OPERATOR_SRC_OVER][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeSrcOverSSE2::xrgb32_vspan_argb32;
  m->composite[OPERATOR_SRC_OVER][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeSrcOverSSE2::xrgb32_vspan_prgb32;
  m->composite[OPERATOR_SRC_OVER][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = DibSSE2::memcpy32;

  m->composite[OPERATOR_SRC_OVER][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeSrcOverSSE2::xrgb32_vspan_argb32_a8;
  m->composite[OPERATOR_SRC_OVER][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeSrcOverSSE2::xrgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_SRC_OVER][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeSrcSSE2::xrgb32_vspan_xrgb32_a8;

  m->composite[OPERATOR_SRC_OVER][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeSrcOverSSE2::xrgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_SRC_OVER][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeSrcOverSSE2::xrgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_SRC_OVER][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeSrcSSE2::xrgb32_vspan_xrgb32_a8_const;

  // [Composite - DstOver]

  m->composite[OPERATOR_DST_OVER][PIXEL_FORMAT_PRGB32].cspan = CompositeDstOverSSE2::prgb32_cspan;
  m->composite[OPERATOR_DST_OVER][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeDstOverSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_DST_OVER][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeDstOverSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_DST_OVER][PIXEL_FORMAT_PRGB32].cspan_a8_scanline = CompositeDstOverSSE2::prgb32_cspan_a8_scanline;

  m->composite[OPERATOR_DST_OVER][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeDstOverSSE2::prgb32_vspan_prgb32;
  m->composite[OPERATOR_DST_OVER][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeDstOverSSE2::prgb32_vspan_argb32;
  m->composite[OPERATOR_DST_OVER][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeDstOverSSE2::prgb32_vspan_xrgb32;

  m->composite[OPERATOR_DST_OVER][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeDstOverSSE2::prgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_DST_OVER][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeDstOverSSE2::prgb32_vspan_argb32_a8;
  m->composite[OPERATOR_DST_OVER][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeDstOverSSE2::prgb32_vspan_xrgb32_a8;

  m->composite[OPERATOR_DST_OVER][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeDstOverSSE2::prgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_DST_OVER][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeDstOverSSE2::prgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_DST_OVER][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeDstOverSSE2::prgb32_vspan_xrgb32_a8_const;

  // PIXEL_FORMAT_XRGB32 - NOP (already set by RasterUtil_Init_C.cpp).

  // [Composite - SrcIn]

  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_PRGB32].cspan = CompositeSrcInSSE2::prgb32_cspan;
  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeSrcInSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeSrcInSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_PRGB32].cspan_a8_scanline = CompositeSrcInSSE2::prgb32_cspan_a8_scanline;

  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeSrcInSSE2::prgb32_vspan_prgb32;
  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeSrcInSSE2::prgb32_vspan_argb32;
  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeSrcInSSE2::prgb32_vspan_xrgb32;

  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeSrcInSSE2::prgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeSrcInSSE2::prgb32_vspan_argb32_a8;
  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeSrcInSSE2::prgb32_vspan_xrgb32_a8;

  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeSrcInSSE2::prgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeSrcInSSE2::prgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeSrcInSSE2::prgb32_vspan_xrgb32_a8_const;

  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_XRGB32].cspan = CompositeSrcSSE2::xrgb32_cspan;
  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeSrcSSE2::xrgb32_cspan_a8;
  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeSrcSSE2::xrgb32_cspan_a8_const;
  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_XRGB32].cspan_a8_scanline = CompositeSrcSSE2::xrgb32_cspan_a8_scanline;

  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = DibSSE2::frgb32_from_argb32;
  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = DibSSE2::frgb32_from_xrgb32;
  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = DibSSE2::memcpy32;
  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_A8] = (VSpanFn)CompositeClearSSE2::xrgb32_cspan;
  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_I8] = DibSSE2::frgb32_from_i8;

  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeSrcSSE2::xrgb32_vspan_argb32_a8;
  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeSrcSSE2::xrgb32_vspan_xrgb32_a8;
  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeSrcSSE2::xrgb32_vspan_xrgb32_a8;
  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_A8] = (VSpanMskFn)CompositeClearSSE2::xrgb32_cspan_a8;
  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_I8] = CompositeSrcSSE2::xrgb32_vspan_i8_a8;

  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeSrcSSE2::xrgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeSrcSSE2::xrgb32_vspan_xrgb32_a8_const;
  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeSrcSSE2::xrgb32_vspan_xrgb32_a8_const;
  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_A8] = (VSpanMskConstFn)CompositeClearSSE2::xrgb32_cspan_a8_const;
  m->composite[OPERATOR_SRC_IN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_I8] = CompositeSrcSSE2::xrgb32_vspan_i8_a8_const;

  // [Composite - DstIn]

  m->composite[OPERATOR_DST_IN][PIXEL_FORMAT_PRGB32].cspan = CompositeDstInSSE2::prgb32_cspan;
  m->composite[OPERATOR_DST_IN][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeDstInSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_DST_IN][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeDstInSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_DST_IN][PIXEL_FORMAT_PRGB32].cspan_a8_scanline = CompositeDstInSSE2::prgb32_cspan_a8_scanline;

  m->composite[OPERATOR_DST_IN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32;
  m->composite[OPERATOR_DST_IN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32;
//m->composite[OPERATOR_DST_IN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = NOP;

  m->composite[OPERATOR_DST_IN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32_a8;
  m->composite[OPERATOR_DST_IN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32_a8;
//m->composite[OPERATOR_DST_IN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = NOP;

  m->composite[OPERATOR_DST_IN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32_a8_const;
  m->composite[OPERATOR_DST_IN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32_a8_const;
//m->composite[OPERATOR_DST_IN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = NOP;

  m->composite[OPERATOR_DST_IN][PIXEL_FORMAT_XRGB32].cspan = CompositeDstInSSE2::prgb32_cspan;
  m->composite[OPERATOR_DST_IN][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeDstInSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_DST_IN][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeDstInSSE2::prgb32_cspan_a8_const;
//  m->composite[OPERATOR_DST_IN][PIXEL_FORMAT_XRGB32].cspan_a8_scanline = CompositeDstInSSE2::prgb32_cspan_a8_scanline;

  m->composite[OPERATOR_DST_IN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32;
  m->composite[OPERATOR_DST_IN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32;
//m->composite[OPERATOR_DST_IN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = NOP;

  m->composite[OPERATOR_DST_IN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32_a8;
  m->composite[OPERATOR_DST_IN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32_a8;
//m->composite[OPERATOR_DST_IN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = NOP;

  m->composite[OPERATOR_DST_IN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32_a8_const;
  m->composite[OPERATOR_DST_IN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32_a8_const;
//m->composite[OPERATOR_DST_IN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = NOP;

  // [Composite - SrcOut]

  m->composite[OPERATOR_SRC_OUT][PIXEL_FORMAT_PRGB32].cspan = CompositeSrcOutSSE2::prgb32_cspan;
  m->composite[OPERATOR_SRC_OUT][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeSrcOutSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_SRC_OUT][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeSrcOutSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_SRC_OUT][PIXEL_FORMAT_PRGB32].cspan_a8_scanline = CompositeSrcOutSSE2::prgb32_cspan_a8_scanline;

  m->composite[OPERATOR_SRC_OUT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeSrcOutSSE2::prgb32_vspan_prgb32;
  m->composite[OPERATOR_SRC_OUT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeSrcOutSSE2::prgb32_vspan_argb32;
  m->composite[OPERATOR_SRC_OUT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeSrcOutSSE2::prgb32_vspan_xrgb32;

  m->composite[OPERATOR_SRC_OUT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeSrcOutSSE2::prgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_SRC_OUT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeSrcOutSSE2::prgb32_vspan_argb32_a8;
  m->composite[OPERATOR_SRC_OUT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeSrcOutSSE2::prgb32_vspan_xrgb32_a8;

  m->composite[OPERATOR_SRC_OUT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeSrcOutSSE2::prgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_SRC_OUT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeSrcOutSSE2::prgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_SRC_OUT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeSrcOutSSE2::prgb32_vspan_xrgb32_a8_const;

  m->composite[OPERATOR_SRC_OUT][PIXEL_FORMAT_XRGB32].cspan = CompositeClearSSE2::prgb32_cspan;
  m->composite[OPERATOR_SRC_OUT][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeClearSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_SRC_OUT][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeClearSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_SRC_OUT][PIXEL_FORMAT_XRGB32].cspan_a8_scanline = CompositeClearSSE2::prgb32_cspan_a8_scanline;

  m->composite[OPERATOR_SRC_OUT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = (VSpanFn)CompositeClearSSE2::prgb32_cspan;
  m->composite[OPERATOR_SRC_OUT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = (VSpanFn)CompositeClearSSE2::prgb32_cspan;
  m->composite[OPERATOR_SRC_OUT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = (VSpanFn)CompositeClearSSE2::prgb32_cspan;

  m->composite[OPERATOR_SRC_OUT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = (VSpanMskFn)CompositeClearSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_SRC_OUT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = (VSpanMskFn)CompositeClearSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_SRC_OUT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = (VSpanMskFn)CompositeClearSSE2::prgb32_cspan_a8;

  m->composite[OPERATOR_SRC_OUT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = (VSpanMskConstFn)CompositeClearSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_SRC_OUT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = (VSpanMskConstFn)CompositeClearSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_SRC_OUT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = (VSpanMskConstFn)CompositeClearSSE2::prgb32_cspan_a8_const;

  // [Composite - DstOut]

  m->composite[OPERATOR_DST_OUT][PIXEL_FORMAT_PRGB32].cspan = CompositeDstOutSSE2::prgb32_cspan;
  m->composite[OPERATOR_DST_OUT][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeDstOutSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_DST_OUT][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeDstOutSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_DST_OUT][PIXEL_FORMAT_PRGB32].cspan_a8_scanline = CompositeDstOutSSE2::prgb32_cspan_a8_scanline;

  m->composite[OPERATOR_DST_OUT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32;
  m->composite[OPERATOR_DST_OUT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32;
  m->composite[OPERATOR_DST_OUT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = (VSpanFn)CompositeClearSSE2::prgb32_cspan;

  m->composite[OPERATOR_DST_OUT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32_a8;
  m->composite[OPERATOR_DST_OUT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32_a8;
  m->composite[OPERATOR_DST_OUT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = (VSpanMskFn)CompositeClearSSE2::prgb32_cspan_a8;

  m->composite[OPERATOR_DST_OUT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32_a8_const;
  m->composite[OPERATOR_DST_OUT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32_a8_const;
  m->composite[OPERATOR_DST_OUT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = (VSpanMskConstFn)CompositeClearSSE2::prgb32_cspan_a8_const;

  m->composite[OPERATOR_DST_OUT][PIXEL_FORMAT_XRGB32].cspan = CompositeDstOutSSE2::prgb32_cspan;
  m->composite[OPERATOR_DST_OUT][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeDstOutSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_DST_OUT][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeDstOutSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_DST_OUT][PIXEL_FORMAT_XRGB32].cspan_a8_scanline = CompositeDstOutSSE2::prgb32_cspan_a8_scanline;

  m->composite[OPERATOR_DST_OUT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32;
  m->composite[OPERATOR_DST_OUT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32;
  m->composite[OPERATOR_DST_OUT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = (VSpanFn)CompositeClearSSE2::prgb32_cspan;

  m->composite[OPERATOR_DST_OUT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32_a8;
  m->composite[OPERATOR_DST_OUT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32_a8;
  m->composite[OPERATOR_DST_OUT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = (VSpanMskFn)CompositeClearSSE2::prgb32_cspan_a8;

  m->composite[OPERATOR_DST_OUT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32_a8_const;
  m->composite[OPERATOR_DST_OUT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32_a8_const;
  m->composite[OPERATOR_DST_OUT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = (VSpanMskConstFn)CompositeClearSSE2::prgb32_cspan_a8_const;

  // [Composite - SrcAtop]

  m->composite[OPERATOR_SRC_ATOP][PIXEL_FORMAT_PRGB32].cspan = CompositeSrcAtopSSE2::prgb32_cspan;
  m->composite[OPERATOR_SRC_ATOP][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeSrcAtopSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_SRC_ATOP][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeSrcAtopSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_SRC_ATOP][PIXEL_FORMAT_PRGB32].cspan_a8_scanline = CompositeSrcAtopSSE2::prgb32_cspan_a8_scanline;

  m->composite[OPERATOR_SRC_ATOP][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeSrcAtopSSE2::prgb32_vspan_prgb32;
  m->composite[OPERATOR_SRC_ATOP][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeSrcAtopSSE2::prgb32_vspan_argb32;
  m->composite[OPERATOR_SRC_ATOP][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeSrcInSSE2::prgb32_vspan_xrgb32;

  m->composite[OPERATOR_SRC_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeSrcAtopSSE2::prgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_SRC_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeSrcAtopSSE2::prgb32_vspan_argb32_a8;
  m->composite[OPERATOR_SRC_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeSrcInSSE2::prgb32_vspan_xrgb32_a8;

  m->composite[OPERATOR_SRC_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeSrcAtopSSE2::prgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_SRC_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeSrcAtopSSE2::prgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_SRC_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeSrcInSSE2::prgb32_vspan_xrgb32_a8_const;

  m->composite[OPERATOR_SRC_ATOP][PIXEL_FORMAT_XRGB32].cspan = CompositeSrcOverSSE2::xrgb32_cspan;
  m->composite[OPERATOR_SRC_ATOP][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeSrcOverSSE2::xrgb32_cspan_a8;
  m->composite[OPERATOR_SRC_ATOP][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeSrcOverSSE2::xrgb32_cspan_a8_const;
  m->composite[OPERATOR_SRC_ATOP][PIXEL_FORMAT_XRGB32].cspan_a8_scanline = CompositeSrcOverSSE2::xrgb32_cspan_a8_scanline;

  m->composite[OPERATOR_SRC_ATOP][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeSrcOverSSE2::xrgb32_vspan_prgb32;
  m->composite[OPERATOR_SRC_ATOP][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeSrcOverSSE2::xrgb32_vspan_argb32;
  m->composite[OPERATOR_SRC_ATOP][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = DibSSE2::memcpy32;

  m->composite[OPERATOR_SRC_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeSrcOverSSE2::xrgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_SRC_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeSrcOverSSE2::xrgb32_vspan_argb32_a8;
  m->composite[OPERATOR_SRC_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeSrcSSE2::xrgb32_vspan_xrgb32_a8;

  m->composite[OPERATOR_SRC_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeSrcOverSSE2::xrgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_SRC_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeSrcOverSSE2::xrgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_SRC_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeSrcSSE2::xrgb32_vspan_xrgb32_a8_const;

  // PIXEL_FORMAT_A8     - NOP (already set by RasterUtil_Init_C.cpp).

  // [Composite - DstAtop]

  m->composite[OPERATOR_DST_ATOP][PIXEL_FORMAT_PRGB32].cspan = CompositeDstAtopSSE2::prgb32_cspan;
  m->composite[OPERATOR_DST_ATOP][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeDstAtopSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_DST_ATOP][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeDstAtopSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_DST_ATOP][PIXEL_FORMAT_PRGB32].cspan_a8_scanline = CompositeDstAtopSSE2::prgb32_cspan_a8_scanline;

  m->composite[OPERATOR_DST_ATOP][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeDstAtopSSE2::prgb32_vspan_prgb32;
  m->composite[OPERATOR_DST_ATOP][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeDstAtopSSE2::prgb32_vspan_argb32;
  m->composite[OPERATOR_DST_ATOP][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeDstOverSSE2::prgb32_vspan_xrgb32;

  m->composite[OPERATOR_DST_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeDstAtopSSE2::prgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_DST_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeDstAtopSSE2::prgb32_vspan_argb32_a8;
  m->composite[OPERATOR_DST_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeDstOverSSE2::prgb32_vspan_xrgb32_a8;

  m->composite[OPERATOR_DST_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeDstAtopSSE2::prgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_DST_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeDstAtopSSE2::prgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_DST_ATOP][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeDstOverSSE2::prgb32_vspan_xrgb32_a8_const;

  m->composite[OPERATOR_DST_ATOP][PIXEL_FORMAT_XRGB32].cspan = CompositeDstInSSE2::prgb32_cspan;
  m->composite[OPERATOR_DST_ATOP][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeDstInSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_DST_ATOP][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeDstInSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_DST_ATOP][PIXEL_FORMAT_PRGB32].cspan_a8_scanline = CompositeDstInSSE2::prgb32_cspan_a8_scanline;

  m->composite[OPERATOR_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32;
  m->composite[OPERATOR_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32;
//m->composite[OPERATOR_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = NOP;

  m->composite[OPERATOR_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32_a8;
  m->composite[OPERATOR_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32_a8;
//m->composite[OPERATOR_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = NOP;

  m->composite[OPERATOR_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32_a8_const;
  m->composite[OPERATOR_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeDstInSSE2::prgb32_vspan_prgb32_or_argb32_a8_const;
//m->composite[OPERATOR_DST_ATOP][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = NOP;

  // [Composite - Xor]

  m->composite[OPERATOR_XOR][PIXEL_FORMAT_PRGB32].cspan = CompositeXorSSE2::prgb32_cspan;
  m->composite[OPERATOR_XOR][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeXorSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_XOR][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeXorSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_XOR][PIXEL_FORMAT_PRGB32].cspan_a8_scanline = CompositeXorSSE2::prgb32_cspan_a8_scanline;

  m->composite[OPERATOR_XOR][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeXorSSE2::prgb32_vspan_prgb32;
  m->composite[OPERATOR_XOR][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeXorSSE2::prgb32_vspan_argb32;
  m->composite[OPERATOR_XOR][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeSrcOutSSE2::prgb32_vspan_xrgb32;

  m->composite[OPERATOR_XOR][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeXorSSE2::prgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_XOR][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeXorSSE2::prgb32_vspan_argb32_a8;
  m->composite[OPERATOR_XOR][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeSrcOutSSE2::prgb32_vspan_xrgb32_a8;

  m->composite[OPERATOR_XOR][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeXorSSE2::prgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_XOR][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeXorSSE2::prgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_XOR][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeSrcOutSSE2::prgb32_vspan_xrgb32_a8_const;

  m->composite[OPERATOR_XOR][PIXEL_FORMAT_XRGB32].cspan = CompositeDstOutSSE2::prgb32_cspan;
  m->composite[OPERATOR_XOR][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeDstOutSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_XOR][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeDstOutSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_XOR][PIXEL_FORMAT_XRGB32].cspan_a8_scanline = CompositeDstOutSSE2::prgb32_cspan_a8_scanline;

  m->composite[OPERATOR_XOR][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32;
  m->composite[OPERATOR_XOR][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32;
  m->composite[OPERATOR_XOR][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = (VSpanFn)CompositeClearSSE2::xrgb32_cspan;

  m->composite[OPERATOR_XOR][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32_a8;
  m->composite[OPERATOR_XOR][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32_a8;
  m->composite[OPERATOR_XOR][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = (VSpanMskFn)CompositeClearSSE2::xrgb32_cspan_a8;

  m->composite[OPERATOR_XOR][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32_a8_const;
  m->composite[OPERATOR_XOR][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeDstOutSSE2::prgb32_vspan_prgb32_or_argb32_a8_const;
  m->composite[OPERATOR_XOR][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = (VSpanMskConstFn)CompositeClearSSE2::xrgb32_cspan_a8_const;

  // [Composite - Clear]

  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_PRGB32].cspan = CompositeClearSSE2::prgb32_cspan;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeClearSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeClearSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_PRGB32].cspan_a8_scanline = CompositeClearSSE2::prgb32_cspan_a8_scanline;

  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = (VSpanFn)CompositeClearSSE2::prgb32_cspan;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = (VSpanFn)CompositeClearSSE2::prgb32_cspan;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = (VSpanFn)CompositeClearSSE2::prgb32_cspan;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_A8] = (VSpanFn)CompositeClearSSE2::prgb32_cspan;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_I8] = (VSpanFn)CompositeClearSSE2::prgb32_cspan;

  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = (VSpanMskFn)CompositeClearSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = (VSpanMskFn)CompositeClearSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = (VSpanMskFn)CompositeClearSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_A8] = (VSpanMskFn)CompositeClearSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_I8] = (VSpanMskFn)CompositeClearSSE2::prgb32_cspan_a8;

  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = (VSpanMskConstFn)CompositeClearSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = (VSpanMskConstFn)CompositeClearSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = (VSpanMskConstFn)CompositeClearSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_A8] = (VSpanMskConstFn)CompositeClearSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_I8] = (VSpanMskConstFn)CompositeClearSSE2::prgb32_cspan_a8_const;

  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_XRGB32].cspan = CompositeClearSSE2::xrgb32_cspan;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeClearSSE2::xrgb32_cspan_a8;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeClearSSE2::xrgb32_cspan_a8_const;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_XRGB32].cspan_a8_scanline = CompositeClearSSE2::xrgb32_cspan_a8_scanline;

  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = (VSpanFn)CompositeClearSSE2::xrgb32_cspan;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = (VSpanFn)CompositeClearSSE2::xrgb32_cspan;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = (VSpanFn)CompositeClearSSE2::xrgb32_cspan;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_A8] = (VSpanFn)CompositeClearSSE2::xrgb32_cspan;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_I8] = (VSpanFn)CompositeClearSSE2::xrgb32_cspan;

  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = (VSpanMskFn)CompositeClearSSE2::xrgb32_cspan_a8;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = (VSpanMskFn)CompositeClearSSE2::xrgb32_cspan_a8;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = (VSpanMskFn)CompositeClearSSE2::xrgb32_cspan_a8;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_A8] = (VSpanMskFn)CompositeClearSSE2::xrgb32_cspan_a8;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_I8] = (VSpanMskFn)CompositeClearSSE2::xrgb32_cspan_a8;

  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = (VSpanMskConstFn)CompositeClearSSE2::xrgb32_cspan_a8_const;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = (VSpanMskConstFn)CompositeClearSSE2::xrgb32_cspan_a8_const;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = (VSpanMskConstFn)CompositeClearSSE2::xrgb32_cspan_a8_const;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_A8] = (VSpanMskConstFn)CompositeClearSSE2::xrgb32_cspan_a8_const;
  m->composite[OPERATOR_CLEAR][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_I8] = (VSpanMskConstFn)CompositeClearSSE2::xrgb32_cspan_a8_const;

  // [Composite - Add]

  m->composite[OPERATOR_ADD][PIXEL_FORMAT_PRGB32].cspan = CompositeAddSSE2::prgb32_cspan;
  m->composite[OPERATOR_ADD][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeAddSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_ADD][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeAddSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_ADD][PIXEL_FORMAT_PRGB32].cspan_a8_scanline = CompositeAddSSE2::prgb32_cspan_a8_scanline;

  m->composite[OPERATOR_ADD][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeAddSSE2::prgb32_vspan_prgb32;
  m->composite[OPERATOR_ADD][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeAddSSE2::prgb32_vspan_argb32;
  m->composite[OPERATOR_ADD][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeAddSSE2::prgb32_vspan_xrgb32;

  m->composite[OPERATOR_ADD][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeAddSSE2::prgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_ADD][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeAddSSE2::prgb32_vspan_argb32_a8;
  m->composite[OPERATOR_ADD][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeAddSSE2::prgb32_vspan_xrgb32_a8;

  m->composite[OPERATOR_ADD][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeAddSSE2::prgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_ADD][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeAddSSE2::prgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_ADD][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeAddSSE2::prgb32_vspan_xrgb32_a8_const;

  m->composite[OPERATOR_ADD][PIXEL_FORMAT_XRGB32].cspan = CompositeAddSSE2::xrgb32_cspan;
  m->composite[OPERATOR_ADD][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeAddSSE2::xrgb32_cspan_a8;
  m->composite[OPERATOR_ADD][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeAddSSE2::xrgb32_cspan_a8_const;
  m->composite[OPERATOR_ADD][PIXEL_FORMAT_XRGB32].cspan_a8_scanline = CompositeAddSSE2::xrgb32_cspan_a8_scanline;

  m->composite[OPERATOR_ADD][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeAddSSE2::xrgb32_vspan_prgb32;
  m->composite[OPERATOR_ADD][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeAddSSE2::xrgb32_vspan_argb32;
  m->composite[OPERATOR_ADD][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeAddSSE2::xrgb32_vspan_xrgb32;

  m->composite[OPERATOR_ADD][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeAddSSE2::xrgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_ADD][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeAddSSE2::xrgb32_vspan_argb32_a8;
  m->composite[OPERATOR_ADD][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeAddSSE2::xrgb32_vspan_xrgb32_a8;

  m->composite[OPERATOR_ADD][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeAddSSE2::xrgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_ADD][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeAddSSE2::xrgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_ADD][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeAddSSE2::xrgb32_vspan_xrgb32_a8_const;

  // [Composite - Subtract]

  m->composite[OPERATOR_SUBTRACT][PIXEL_FORMAT_PRGB32].cspan = CompositeSubtractSSE2::prgb32_cspan;
  m->composite[OPERATOR_SUBTRACT][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeSubtractSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_SUBTRACT][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeSubtractSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_SUBTRACT][PIXEL_FORMAT_PRGB32].cspan_a8_scanline = CompositeSubtractSSE2::prgb32_cspan_a8_scanline;

  m->composite[OPERATOR_SUBTRACT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeSubtractSSE2::prgb32_vspan_prgb32;
  m->composite[OPERATOR_SUBTRACT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeSubtractSSE2::prgb32_vspan_argb32;
  m->composite[OPERATOR_SUBTRACT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeSubtractSSE2::prgb32_vspan_xrgb32;

  m->composite[OPERATOR_SUBTRACT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeSubtractSSE2::prgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_SUBTRACT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeSubtractSSE2::prgb32_vspan_argb32_a8;
  m->composite[OPERATOR_SUBTRACT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeSubtractSSE2::prgb32_vspan_xrgb32_a8;

  m->composite[OPERATOR_SUBTRACT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeSubtractSSE2::prgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_SUBTRACT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeSubtractSSE2::prgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_SUBTRACT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeSubtractSSE2::prgb32_vspan_xrgb32_a8_const;

  m->composite[OPERATOR_SUBTRACT][PIXEL_FORMAT_XRGB32].cspan = CompositeSubtractSSE2::xrgb32_cspan;
  m->composite[OPERATOR_SUBTRACT][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeSubtractSSE2::xrgb32_cspan_a8;
  m->composite[OPERATOR_SUBTRACT][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeSubtractSSE2::xrgb32_cspan_a8_const;
  m->composite[OPERATOR_SUBTRACT][PIXEL_FORMAT_XRGB32].cspan_a8_scanline = CompositeSubtractSSE2::xrgb32_cspan_a8_scanline;

  m->composite[OPERATOR_SUBTRACT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeSubtractSSE2::xrgb32_vspan_prgb32;
  m->composite[OPERATOR_SUBTRACT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeSubtractSSE2::xrgb32_vspan_argb32;
  m->composite[OPERATOR_SUBTRACT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeSubtractSSE2::xrgb32_vspan_xrgb32;

  m->composite[OPERATOR_SUBTRACT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeSubtractSSE2::xrgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_SUBTRACT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeSubtractSSE2::xrgb32_vspan_argb32_a8;
  m->composite[OPERATOR_SUBTRACT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeSubtractSSE2::xrgb32_vspan_xrgb32_a8;

  m->composite[OPERATOR_SUBTRACT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeSubtractSSE2::xrgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_SUBTRACT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeSubtractSSE2::xrgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_SUBTRACT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeSubtractSSE2::xrgb32_vspan_xrgb32_a8_const;

  // [Composite - Screen]

  m->composite[OPERATOR_SCREEN][PIXEL_FORMAT_PRGB32].cspan = CompositeScreenSSE2::prgb32_cspan;
  m->composite[OPERATOR_SCREEN][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeScreenSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_SCREEN][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeScreenSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_SCREEN][PIXEL_FORMAT_PRGB32].cspan_a8_scanline = CompositeScreenSSE2::prgb32_cspan_a8_scanline;

  m->composite[OPERATOR_SCREEN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeScreenSSE2::prgb32_vspan_prgb32;
  m->composite[OPERATOR_SCREEN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeScreenSSE2::prgb32_vspan_argb32;
  m->composite[OPERATOR_SCREEN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeScreenSSE2::prgb32_vspan_xrgb32;

  m->composite[OPERATOR_SCREEN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeScreenSSE2::prgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_SCREEN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeScreenSSE2::prgb32_vspan_argb32_a8;
  m->composite[OPERATOR_SCREEN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeScreenSSE2::prgb32_vspan_xrgb32_a8;

  m->composite[OPERATOR_SCREEN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeScreenSSE2::prgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_SCREEN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeScreenSSE2::prgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_SCREEN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeScreenSSE2::prgb32_vspan_xrgb32_a8_const;

  m->composite[OPERATOR_SCREEN][PIXEL_FORMAT_XRGB32].cspan = CompositeScreenSSE2::xrgb32_cspan;
  m->composite[OPERATOR_SCREEN][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeScreenSSE2::xrgb32_cspan_a8;
  m->composite[OPERATOR_SCREEN][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeScreenSSE2::xrgb32_cspan_a8_const;
  m->composite[OPERATOR_SCREEN][PIXEL_FORMAT_XRGB32].cspan_a8_scanline = CompositeScreenSSE2::xrgb32_cspan_a8_scanline;

  m->composite[OPERATOR_SCREEN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeScreenSSE2::xrgb32_vspan_prgb32;
  m->composite[OPERATOR_SCREEN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeScreenSSE2::xrgb32_vspan_argb32;
  m->composite[OPERATOR_SCREEN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeScreenSSE2::xrgb32_vspan_xrgb32;

  m->composite[OPERATOR_SCREEN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeScreenSSE2::xrgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_SCREEN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeScreenSSE2::xrgb32_vspan_argb32_a8;
  m->composite[OPERATOR_SCREEN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeScreenSSE2::xrgb32_vspan_xrgb32_a8;

  m->composite[OPERATOR_SCREEN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeScreenSSE2::xrgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_SCREEN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeScreenSSE2::xrgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_SCREEN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeScreenSSE2::xrgb32_vspan_xrgb32_a8_const;

  // [Composite - Darken]

  m->composite[OPERATOR_DARKEN][PIXEL_FORMAT_PRGB32].cspan = CompositeDarkenSSE2::prgb32_cspan;
  m->composite[OPERATOR_DARKEN][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeDarkenSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_DARKEN][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeDarkenSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_DARKEN][PIXEL_FORMAT_PRGB32].cspan_a8_scanline = CompositeDarkenSSE2::prgb32_cspan_a8_scanline;

  m->composite[OPERATOR_DARKEN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeDarkenSSE2::prgb32_vspan_prgb32;
  m->composite[OPERATOR_DARKEN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeDarkenSSE2::prgb32_vspan_argb32;
  m->composite[OPERATOR_DARKEN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeDarkenSSE2::prgb32_vspan_xrgb32;

  m->composite[OPERATOR_DARKEN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeDarkenSSE2::prgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_DARKEN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeDarkenSSE2::prgb32_vspan_argb32_a8;
  m->composite[OPERATOR_DARKEN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeDarkenSSE2::prgb32_vspan_xrgb32_a8;

  m->composite[OPERATOR_DARKEN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeDarkenSSE2::prgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_DARKEN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeDarkenSSE2::prgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_DARKEN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeDarkenSSE2::prgb32_vspan_xrgb32_a8_const;

  m->composite[OPERATOR_DARKEN][PIXEL_FORMAT_XRGB32].cspan = CompositeDarkenSSE2::xrgb32_cspan;
  m->composite[OPERATOR_DARKEN][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeDarkenSSE2::xrgb32_cspan_a8;
  m->composite[OPERATOR_DARKEN][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeDarkenSSE2::xrgb32_cspan_a8_const;
  m->composite[OPERATOR_DARKEN][PIXEL_FORMAT_XRGB32].cspan_a8_scanline = CompositeDarkenSSE2::xrgb32_cspan_a8_scanline;

  m->composite[OPERATOR_DARKEN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeDarkenSSE2::xrgb32_vspan_prgb32;
  m->composite[OPERATOR_DARKEN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeDarkenSSE2::xrgb32_vspan_argb32;
  m->composite[OPERATOR_DARKEN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeDarkenSSE2::xrgb32_vspan_xrgb32;

  m->composite[OPERATOR_DARKEN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeDarkenSSE2::xrgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_DARKEN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeDarkenSSE2::xrgb32_vspan_argb32_a8;
  m->composite[OPERATOR_DARKEN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeDarkenSSE2::xrgb32_vspan_xrgb32_a8;

  m->composite[OPERATOR_DARKEN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeDarkenSSE2::xrgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_DARKEN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeDarkenSSE2::xrgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_DARKEN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeDarkenSSE2::xrgb32_vspan_xrgb32_a8_const;

  // [Composite - Lighten]

  m->composite[OPERATOR_LIGHTEN][PIXEL_FORMAT_PRGB32].cspan = CompositeLightenSSE2::prgb32_cspan;
  m->composite[OPERATOR_LIGHTEN][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeLightenSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_LIGHTEN][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeLightenSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_LIGHTEN][PIXEL_FORMAT_PRGB32].cspan_a8_scanline = CompositeLightenSSE2::prgb32_cspan_a8_scanline;

  m->composite[OPERATOR_LIGHTEN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeLightenSSE2::prgb32_vspan_prgb32;
  m->composite[OPERATOR_LIGHTEN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeLightenSSE2::prgb32_vspan_argb32;
  m->composite[OPERATOR_LIGHTEN][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeLightenSSE2::prgb32_vspan_xrgb32;

  m->composite[OPERATOR_LIGHTEN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeLightenSSE2::prgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_LIGHTEN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeLightenSSE2::prgb32_vspan_argb32_a8;
  m->composite[OPERATOR_LIGHTEN][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeLightenSSE2::prgb32_vspan_xrgb32_a8;

  m->composite[OPERATOR_LIGHTEN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeLightenSSE2::prgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_LIGHTEN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeLightenSSE2::prgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_LIGHTEN][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeLightenSSE2::prgb32_vspan_xrgb32_a8_const;

  m->composite[OPERATOR_LIGHTEN][PIXEL_FORMAT_XRGB32].cspan = CompositeLightenSSE2::xrgb32_cspan;
  m->composite[OPERATOR_LIGHTEN][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeLightenSSE2::xrgb32_cspan_a8;
  m->composite[OPERATOR_LIGHTEN][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeLightenSSE2::xrgb32_cspan_a8_const;
  m->composite[OPERATOR_LIGHTEN][PIXEL_FORMAT_XRGB32].cspan_a8_scanline = CompositeLightenSSE2::xrgb32_cspan_a8_scanline;

  m->composite[OPERATOR_LIGHTEN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeLightenSSE2::xrgb32_vspan_prgb32;
  m->composite[OPERATOR_LIGHTEN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeLightenSSE2::xrgb32_vspan_argb32;
  m->composite[OPERATOR_LIGHTEN][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeLightenSSE2::xrgb32_vspan_xrgb32;

  m->composite[OPERATOR_LIGHTEN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeLightenSSE2::xrgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_LIGHTEN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeLightenSSE2::xrgb32_vspan_argb32_a8;
  m->composite[OPERATOR_LIGHTEN][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeLightenSSE2::xrgb32_vspan_xrgb32_a8;

  m->composite[OPERATOR_LIGHTEN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeLightenSSE2::xrgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_LIGHTEN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeLightenSSE2::xrgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_LIGHTEN][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeLightenSSE2::xrgb32_vspan_xrgb32_a8_const;

  // [Composite - Difference]

  m->composite[OPERATOR_DIFFERENCE][PIXEL_FORMAT_PRGB32].cspan = CompositeDifferenceSSE2::prgb32_cspan;
  m->composite[OPERATOR_DIFFERENCE][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeDifferenceSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_DIFFERENCE][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeDifferenceSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_DIFFERENCE][PIXEL_FORMAT_PRGB32].cspan_a8_scanline = CompositeDifferenceSSE2::prgb32_cspan_a8_scanline;

  m->composite[OPERATOR_DIFFERENCE][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeDifferenceSSE2::prgb32_vspan_prgb32;
  m->composite[OPERATOR_DIFFERENCE][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeDifferenceSSE2::prgb32_vspan_argb32;
  m->composite[OPERATOR_DIFFERENCE][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeDifferenceSSE2::prgb32_vspan_xrgb32;

  m->composite[OPERATOR_DIFFERENCE][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeDifferenceSSE2::prgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_DIFFERENCE][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeDifferenceSSE2::prgb32_vspan_argb32_a8;
  m->composite[OPERATOR_DIFFERENCE][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeDifferenceSSE2::prgb32_vspan_xrgb32_a8;

  m->composite[OPERATOR_DIFFERENCE][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeDifferenceSSE2::prgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_DIFFERENCE][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeDifferenceSSE2::prgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_DIFFERENCE][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeDifferenceSSE2::prgb32_vspan_xrgb32_a8_const;

  m->composite[OPERATOR_DIFFERENCE][PIXEL_FORMAT_XRGB32].cspan = CompositeDifferenceSSE2::xrgb32_cspan;
  m->composite[OPERATOR_DIFFERENCE][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeDifferenceSSE2::xrgb32_cspan_a8;
  m->composite[OPERATOR_DIFFERENCE][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeDifferenceSSE2::xrgb32_cspan_a8_const;
  m->composite[OPERATOR_DIFFERENCE][PIXEL_FORMAT_XRGB32].cspan_a8_scanline = CompositeDifferenceSSE2::xrgb32_cspan_a8_scanline;

  m->composite[OPERATOR_DIFFERENCE][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeDifferenceSSE2::xrgb32_vspan_prgb32;
  m->composite[OPERATOR_DIFFERENCE][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeDifferenceSSE2::xrgb32_vspan_argb32;
  m->composite[OPERATOR_DIFFERENCE][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeDifferenceSSE2::xrgb32_vspan_xrgb32;

  m->composite[OPERATOR_DIFFERENCE][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeDifferenceSSE2::xrgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_DIFFERENCE][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeDifferenceSSE2::xrgb32_vspan_argb32_a8;
  m->composite[OPERATOR_DIFFERENCE][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeDifferenceSSE2::xrgb32_vspan_xrgb32_a8;

  m->composite[OPERATOR_DIFFERENCE][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeDifferenceSSE2::xrgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_DIFFERENCE][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeDifferenceSSE2::xrgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_DIFFERENCE][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeDifferenceSSE2::xrgb32_vspan_xrgb32_a8_const;

  // [Composite - Exclusion]

  m->composite[OPERATOR_EXCLUSION][PIXEL_FORMAT_PRGB32].cspan = CompositeExclusionSSE2::prgb32_cspan;
  m->composite[OPERATOR_EXCLUSION][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeExclusionSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_EXCLUSION][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeExclusionSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_EXCLUSION][PIXEL_FORMAT_PRGB32].cspan_a8_scanline = CompositeExclusionSSE2::prgb32_cspan_a8_scanline;

  m->composite[OPERATOR_EXCLUSION][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeExclusionSSE2::prgb32_vspan_prgb32;
  m->composite[OPERATOR_EXCLUSION][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeExclusionSSE2::prgb32_vspan_argb32;
  m->composite[OPERATOR_EXCLUSION][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeExclusionSSE2::prgb32_vspan_xrgb32;

  m->composite[OPERATOR_EXCLUSION][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeExclusionSSE2::prgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_EXCLUSION][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeExclusionSSE2::prgb32_vspan_argb32_a8;
  m->composite[OPERATOR_EXCLUSION][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeExclusionSSE2::prgb32_vspan_xrgb32_a8;

  m->composite[OPERATOR_EXCLUSION][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeExclusionSSE2::prgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_EXCLUSION][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeExclusionSSE2::prgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_EXCLUSION][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeExclusionSSE2::prgb32_vspan_xrgb32_a8_const;

  m->composite[OPERATOR_EXCLUSION][PIXEL_FORMAT_XRGB32].cspan = CompositeExclusionSSE2::xrgb32_cspan;
  m->composite[OPERATOR_EXCLUSION][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeExclusionSSE2::xrgb32_cspan_a8;
  m->composite[OPERATOR_EXCLUSION][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeExclusionSSE2::xrgb32_cspan_a8_const;
  m->composite[OPERATOR_EXCLUSION][PIXEL_FORMAT_XRGB32].cspan_a8_scanline = CompositeExclusionSSE2::xrgb32_cspan_a8_scanline;

  m->composite[OPERATOR_EXCLUSION][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeExclusionSSE2::xrgb32_vspan_prgb32;
  m->composite[OPERATOR_EXCLUSION][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeExclusionSSE2::xrgb32_vspan_argb32;
  m->composite[OPERATOR_EXCLUSION][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeExclusionSSE2::xrgb32_vspan_xrgb32;

  m->composite[OPERATOR_EXCLUSION][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeExclusionSSE2::xrgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_EXCLUSION][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeExclusionSSE2::xrgb32_vspan_argb32_a8;
  m->composite[OPERATOR_EXCLUSION][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeExclusionSSE2::xrgb32_vspan_xrgb32_a8;

  m->composite[OPERATOR_EXCLUSION][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeExclusionSSE2::xrgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_EXCLUSION][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeExclusionSSE2::xrgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_EXCLUSION][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeExclusionSSE2::xrgb32_vspan_xrgb32_a8_const;

  // [Composite - Invert]

  m->composite[OPERATOR_INVERT][PIXEL_FORMAT_PRGB32].cspan = CompositeInvertSSE2::prgb32_cspan;
  m->composite[OPERATOR_INVERT][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeInvertSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_INVERT][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeInvertSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_INVERT][PIXEL_FORMAT_PRGB32].cspan_a8_scanline = CompositeInvertSSE2::prgb32_cspan_a8_scanline;

  m->composite[OPERATOR_INVERT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeInvertSSE2::prgb32_vspan_prgb32;
  m->composite[OPERATOR_INVERT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeInvertSSE2::prgb32_vspan_prgb32;
  m->composite[OPERATOR_INVERT][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeInvertSSE2::prgb32_vspan_xrgb32;

  m->composite[OPERATOR_INVERT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeInvertSSE2::prgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_INVERT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeInvertSSE2::prgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_INVERT][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeInvertSSE2::prgb32_vspan_xrgb32_a8;

  m->composite[OPERATOR_INVERT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeInvertSSE2::prgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_INVERT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeInvertSSE2::prgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_INVERT][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeInvertSSE2::prgb32_vspan_xrgb32_a8_const;

  m->composite[OPERATOR_INVERT][PIXEL_FORMAT_XRGB32].cspan = CompositeInvertSSE2::xrgb32_cspan;
  m->composite[OPERATOR_INVERT][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeInvertSSE2::xrgb32_cspan_a8;
  m->composite[OPERATOR_INVERT][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeInvertSSE2::xrgb32_cspan_a8_const;
  m->composite[OPERATOR_INVERT][PIXEL_FORMAT_XRGB32].cspan_a8_scanline = CompositeInvertSSE2::xrgb32_cspan_a8_scanline;

  m->composite[OPERATOR_INVERT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeInvertSSE2::xrgb32_vspan_prgb32;
  m->composite[OPERATOR_INVERT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeInvertSSE2::xrgb32_vspan_prgb32;
  m->composite[OPERATOR_INVERT][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeInvertSSE2::xrgb32_vspan_xrgb32;

  m->composite[OPERATOR_INVERT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeInvertSSE2::xrgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_INVERT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeInvertSSE2::xrgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_INVERT][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeInvertSSE2::xrgb32_vspan_xrgb32_a8;

  m->composite[OPERATOR_INVERT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeInvertSSE2::xrgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_INVERT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeInvertSSE2::xrgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_INVERT][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeInvertSSE2::xrgb32_vspan_xrgb32_a8_const;

  // [Composite - InvertRgb]

  m->composite[OPERATOR_INVERT_RGB][PIXEL_FORMAT_PRGB32].cspan = CompositeInvertRgbSSE2::prgb32_cspan;
  m->composite[OPERATOR_INVERT_RGB][PIXEL_FORMAT_PRGB32].cspan_a8 = CompositeInvertRgbSSE2::prgb32_cspan_a8;
  m->composite[OPERATOR_INVERT_RGB][PIXEL_FORMAT_PRGB32].cspan_a8_const = CompositeInvertRgbSSE2::prgb32_cspan_a8_const;
  m->composite[OPERATOR_INVERT_RGB][PIXEL_FORMAT_PRGB32].cspan_a8_scanline = CompositeInvertRgbSSE2::prgb32_cspan_a8_scanline;

  m->composite[OPERATOR_INVERT_RGB][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeInvertRgbSSE2::prgb32_vspan_prgb32;
  m->composite[OPERATOR_INVERT_RGB][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeInvertRgbSSE2::prgb32_vspan_argb32;
  m->composite[OPERATOR_INVERT_RGB][PIXEL_FORMAT_PRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeInvertRgbSSE2::prgb32_vspan_xrgb32;

  m->composite[OPERATOR_INVERT_RGB][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeInvertRgbSSE2::prgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_INVERT_RGB][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeInvertRgbSSE2::prgb32_vspan_argb32_a8;
  m->composite[OPERATOR_INVERT_RGB][PIXEL_FORMAT_PRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeInvertRgbSSE2::prgb32_vspan_xrgb32_a8;

  m->composite[OPERATOR_INVERT_RGB][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeInvertRgbSSE2::prgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_INVERT_RGB][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeInvertRgbSSE2::prgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_INVERT_RGB][PIXEL_FORMAT_PRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeInvertRgbSSE2::prgb32_vspan_xrgb32_a8_const;

  m->composite[OPERATOR_INVERT_RGB][PIXEL_FORMAT_XRGB32].cspan = CompositeInvertRgbSSE2::xrgb32_cspan;
  m->composite[OPERATOR_INVERT_RGB][PIXEL_FORMAT_XRGB32].cspan_a8 = CompositeInvertRgbSSE2::xrgb32_cspan_a8;
  m->composite[OPERATOR_INVERT_RGB][PIXEL_FORMAT_XRGB32].cspan_a8_const = CompositeInvertRgbSSE2::xrgb32_cspan_a8_const;
  m->composite[OPERATOR_INVERT_RGB][PIXEL_FORMAT_XRGB32].cspan_a8_scanline = CompositeInvertRgbSSE2::xrgb32_cspan_a8_scanline;

  m->composite[OPERATOR_INVERT_RGB][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_PRGB32] = CompositeInvertRgbSSE2::xrgb32_vspan_prgb32;
  m->composite[OPERATOR_INVERT_RGB][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_ARGB32] = CompositeInvertRgbSSE2::xrgb32_vspan_argb32;
  m->composite[OPERATOR_INVERT_RGB][PIXEL_FORMAT_XRGB32].vspan[PIXEL_FORMAT_XRGB32] = CompositeInvertRgbSSE2::xrgb32_vspan_xrgb32;

  m->composite[OPERATOR_INVERT_RGB][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_PRGB32] = CompositeInvertRgbSSE2::xrgb32_vspan_prgb32_a8;
  m->composite[OPERATOR_INVERT_RGB][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_ARGB32] = CompositeInvertRgbSSE2::xrgb32_vspan_argb32_a8;
  m->composite[OPERATOR_INVERT_RGB][PIXEL_FORMAT_XRGB32].vspan_a8[PIXEL_FORMAT_XRGB32] = CompositeInvertRgbSSE2::xrgb32_vspan_xrgb32_a8;

  m->composite[OPERATOR_INVERT_RGB][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_PRGB32] = CompositeInvertRgbSSE2::xrgb32_vspan_prgb32_a8_const;
  m->composite[OPERATOR_INVERT_RGB][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_ARGB32] = CompositeInvertRgbSSE2::xrgb32_vspan_argb32_a8_const;
  m->composite[OPERATOR_INVERT_RGB][PIXEL_FORMAT_XRGB32].vspan_a8_const[PIXEL_FORMAT_XRGB32] = CompositeInvertRgbSSE2::xrgb32_vspan_xrgb32_a8_const;
}
