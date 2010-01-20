// [Fog/Graphics Library - Private API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif

#include <Fog/Build/Build.h>

// Don't build MMX under 64-bit mode. MSVC complains about MMX in 64-bit mode
// and it's always overriden by SSE2 implementation that is guaranted for every
// 64-bit processor.
#if defined(FOG_ARCH_X86)

// [Dependencies]
#include <Fog/Core/Intrin_MMX.h>
#include <Fog/Core/Math.h>
#include <Fog/Core/Misc.h>
#include <Fog/Graphics/Argb.h>
#include <Fog/Graphics/ArgbAnalyzer_p.h>
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

// [Raster_MMX]
#include <Fog/Graphics/RasterUtil/RasterUtil_C_p.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Defs_MMX_p.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Composite_MMX_p.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Dib_MMX_p.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Interpolate_MMX_p.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Pattern_MMX_p.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Scale_MMX_p.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Filters_MMX_p.h>

// ============================================================================
// [Library Initializers]
// ============================================================================

static void fog_raster_init_mmx_constants(void)
{
  using namespace Fog;
  using namespace Fog::RasterUtil;

  // RGB16 masks
  Mask_0000F8000000F800 = mmx_create_mask_4x2W(0x0000, 0xF800, 0x0000, 0xF800);
  Mask_0000E0000000E000 = mmx_create_mask_4x2W(0x0000, 0xE000, 0x0000, 0xE000);
  Mask_00007C0000007C00 = mmx_create_mask_4x2W(0x0000, 0x7C00, 0x0000, 0x7C00);
  Mask_00001F0000001F00 = mmx_create_mask_4x2W(0x0000, 0x1F00, 0x0000, 0x1F00);
  Mask_000007E0000007E0 = mmx_create_mask_4x2W(0x0000, 0x07E0, 0x0000, 0x07E0);
  Mask_000003E0000003E0 = mmx_create_mask_4x2W(0x0000, 0x03E0, 0x0000, 0x03E0);
  Mask_000000F8000000F8 = mmx_create_mask_4x2W(0x0000, 0x00F8, 0x0000, 0x00F8);
  Mask_0000007C0000007C = mmx_create_mask_4x2W(0x0000, 0x007C, 0x0000, 0x007C);
  Mask_0000001F0000001F = mmx_create_mask_4x2W(0x0000, 0x001F, 0x0000, 0x001F);
  Mask_0000000700000007 = mmx_create_mask_4x2W(0x0000, 0x0007, 0x0000, 0x0007);
  Mask_0000000300000003 = mmx_create_mask_4x2W(0x0000, 0x0003, 0x0000, 0x0003);

  _mm_empty();
}

FOG_INIT_DECLARE void fog_raster_init_mmx(void)
{
  using namespace Fog;
  using namespace Fog::RasterUtil;

  // [Constants]

  fog_raster_init_mmx_constants();

  FunctionMap* m = functionMap;

  // [Dib - MemCpy]

//m->dib.memcpy8 = DibMMX::memcpy8;
//m->dib.memcpy16 = DibMMX::memcpy16;
//m->dib.memcpy24 = DibMMX::memcpy24;
//m->dib.memcpy32 = DibMMX::memcpy32;

  // [Dib - Convert]

//m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_PRGB32_NATIVE    ] = DibMMX::memcpy32;
//m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_ARGB32_NATIVE    ] = DibMMX::prgb32_from_argb32;
//m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_XRGB32_NATIVE    ] = DibMMX::frgb32_from_xrgb32;
//m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_A8               ] = DibMMX::azzz32_from_a8;
//m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_I8               ] = DibMMX::prgb32_from_i8;
//m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_PRGB32_SWAPPED   ] = DibMMX::bswap32;
//m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_ARGB32_SWAPPED   ] = DibMMX::prgb32_from_argb32_swapped;
//m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_XRGB32_SWAPPED   ] = DibMMX::frgb32_from_xrgb32_swapped;
//m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_RGB24_NATIVE     ] = DibMMX::frgb32_from_rgb24_native;
//m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_RGB24_SWAPPED    ] = DibMMX::frgb32_from_rgb24_swapped;
//m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_RGB16_565_NATIVE ] = DibMMX::frgb32_from_rgb16_565_native;
//m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_RGB16_565_SWAPPED] = DibMMX::frgb32_from_rgb16_565_swapped;
//m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_RGB16_555_NATIVE ] = DibMMX::frgb32_from_rgb16_555_native;
//m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_RGB16_555_SWAPPED] = DibMMX::frgb32_from_rgb16_555_swapped;
//m->dib.convert[PIXEL_FORMAT_PRGB32][DIB_FORMAT_GREY8            ] = DibMMX::frgb32_from_grey8;

//m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_PRGB32_NATIVE    ] = DibMMX::argb32_from_prgb32;
//m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_ARGB32_NATIVE    ] = DibMMX::memcpy32;
//m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_XRGB32_NATIVE    ] = DibMMX::frgb32_from_xrgb32;
//m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_A8               ] = DibMMX::azzz32_from_a8;
//m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_I8               ] = DibMMX::argb32_from_i8;
//m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_PRGB32_SWAPPED   ] = DibMMX::argb32_from_prgb32_swapped;
//m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_ARGB32_SWAPPED   ] = DibMMX::bswap32;
//m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_XRGB32_SWAPPED   ] = DibMMX::frgb32_from_xrgb32_swapped;
//m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_RGB24_NATIVE     ] = DibMMX::frgb32_from_rgb24_native;
//m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_RGB24_SWAPPED    ] = DibMMX::frgb32_from_rgb24_swapped;
//m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_RGB16_565_NATIVE ] = DibMMX::frgb32_from_rgb16_565_native;
//m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_RGB16_565_SWAPPED] = DibMMX::frgb32_from_rgb16_565_swapped;
//m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_RGB16_555_NATIVE ] = DibMMX::frgb32_from_rgb16_555_native;
//m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_RGB16_555_SWAPPED] = DibMMX::frgb32_from_rgb16_555_swapped;
//m->dib.convert[PIXEL_FORMAT_ARGB32][DIB_FORMAT_GREY8            ] = DibMMX::frgb32_from_grey8;

//m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_PRGB32_NATIVE    ] = DibMMX::frgb32_from_xrgb32;
//m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_ARGB32_NATIVE    ] = DibMMX::frgb32_from_argb32;
//m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_XRGB32_NATIVE    ] = DibMMX::frgb32_from_xrgb32;
//m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_A8               ] = DibMMX::fzzz32_from_null;
//m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_I8               ] = DibMMX::frgb32_from_i8;
//m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_PRGB32_SWAPPED   ] = DibMMX::frgb32_from_xrgb32_swapped;
//m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_ARGB32_SWAPPED   ] = DibMMX::frgb32_from_argb32_swapped;
//m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_XRGB32_SWAPPED   ] = DibMMX::bswap32;
//m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_RGB24_NATIVE     ] = DibMMX::frgb32_from_rgb24_native;
//m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_RGB24_SWAPPED    ] = DibMMX::frgb32_from_rgb24_swapped;
//m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_RGB16_565_NATIVE ] = DibMMX::frgb32_from_rgb16_565_native;
//m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_RGB16_565_SWAPPED] = DibMMX::frgb32_from_rgb16_565_swapped;
//m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_RGB16_555_NATIVE ] = DibMMX::frgb32_from_rgb16_555_native;
//m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_RGB16_555_SWAPPED] = DibMMX::frgb32_from_rgb16_555_swapped;
//m->dib.convert[PIXEL_FORMAT_XRGB32][DIB_FORMAT_GREY8            ] = DibMMX::frgb32_from_grey8;

//m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_PRGB32_NATIVE    ] = DibMMX::a8_from_axxx32;
//m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_ARGB32_NATIVE    ] = DibMMX::a8_from_axxx32;
//m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_XRGB32_NATIVE    ] = DibMMX::f8_from_null;
//m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_A8               ] = DibMMX::memcpy8;
//m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_I8               ] = DibMMX::a8_from_i8;
//m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_PRGB32_SWAPPED   ] = DibMMX::a8_from_axxx32_swapped;
//m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_ARGB32_SWAPPED   ] = DibMMX::a8_from_axxx32_swapped;
//m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_XRGB32_SWAPPED   ] = DibMMX::f8_from_null;
//m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_RGB24_NATIVE     ] = DibMMX::f8_from_null;
//m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_RGB24_SWAPPED    ] = DibMMX::f8_from_null;
//m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_RGB16_565_NATIVE ] = DibMMX::f8_from_null;
//m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_RGB16_565_SWAPPED] = DibMMX::f8_from_null;
//m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_RGB16_555_NATIVE ] = DibMMX::f8_from_null;
//m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_RGB16_555_SWAPPED] = DibMMX::f8_from_null;
//m->dib.convert[PIXEL_FORMAT_A8    ][DIB_FORMAT_GREY8            ] = DibMMX::f8_from_null;

//m->dib.convert[DIB_FORMAT_PRGB32_SWAPPED   ][PIXEL_FORMAT_PRGB32] = DibMMX::bswap32;
//m->dib.convert[DIB_FORMAT_PRGB32_SWAPPED   ][PIXEL_FORMAT_ARGB32] = DibMMX::prgb32_swapped_from_argb32;
//m->dib.convert[DIB_FORMAT_PRGB32_SWAPPED   ][PIXEL_FORMAT_XRGB32] = DibMMX::frgb32_swapped_from_xrgb32;
//m->dib.convert[DIB_FORMAT_PRGB32_SWAPPED   ][PIXEL_FORMAT_A8    ] = DibMMX::azzz32_swapped_from_a8;
//m->dib.convert[DIB_FORMAT_PRGB32_SWAPPED   ][PIXEL_FORMAT_I8    ] = DibMMX::prgb32_swapped_from_i8;

//m->dib.convert[DIB_FORMAT_ARGB32_SWAPPED   ][PIXEL_FORMAT_PRGB32] = DibMMX::argb32_swapped_from_prgb32;
//m->dib.convert[DIB_FORMAT_ARGB32_SWAPPED   ][PIXEL_FORMAT_ARGB32] = DibMMX::bswap32;
//m->dib.convert[DIB_FORMAT_ARGB32_SWAPPED   ][PIXEL_FORMAT_XRGB32] = DibMMX::frgb32_swapped_from_xrgb32;
//m->dib.convert[DIB_FORMAT_ARGB32_SWAPPED   ][PIXEL_FORMAT_A8    ] = DibMMX::azzz32_swapped_from_a8;
//m->dib.convert[DIB_FORMAT_ARGB32_SWAPPED   ][PIXEL_FORMAT_I8    ] = DibMMX::argb32_swapped_from_i8;

//m->dib.convert[DIB_FORMAT_XRGB32_SWAPPED   ][PIXEL_FORMAT_PRGB32] = DibMMX::frgb32_swapped_from_xrgb32;
//m->dib.convert[DIB_FORMAT_XRGB32_SWAPPED   ][PIXEL_FORMAT_ARGB32] = DibMMX::frgb32_swapped_from_argb32;
//m->dib.convert[DIB_FORMAT_XRGB32_SWAPPED   ][PIXEL_FORMAT_XRGB32] = DibMMX::frgb32_swapped_from_xrgb32;
//m->dib.convert[DIB_FORMAT_XRGB32_SWAPPED   ][PIXEL_FORMAT_A8    ] = DibMMX::fzzz32_swapped_from_null;
//m->dib.convert[DIB_FORMAT_XRGB32_SWAPPED   ][PIXEL_FORMAT_I8    ] = DibMMX::frgb32_swapped_from_i8;

//m->dib.convert[DIB_FORMAT_RGB24_NATIVE     ][PIXEL_FORMAT_PRGB32] = DibMMX::rgb24_native_from_xrgb32;
//m->dib.convert[DIB_FORMAT_RGB24_NATIVE     ][PIXEL_FORMAT_ARGB32] = DibMMX::rgb24_native_from_argb32;
//m->dib.convert[DIB_FORMAT_RGB24_NATIVE     ][PIXEL_FORMAT_XRGB32] = DibMMX::rgb24_native_from_xrgb32;
//m->dib.convert[DIB_FORMAT_RGB24_NATIVE     ][PIXEL_FORMAT_A8    ] = DibMMX::zzz24_from_null;
//m->dib.convert[DIB_FORMAT_RGB24_NATIVE     ][PIXEL_FORMAT_I8    ] = DibMMX::rgb24_native_from_i8;

//m->dib.convert[DIB_FORMAT_RGB24_SWAPPED    ][PIXEL_FORMAT_PRGB32] = DibMMX::rgb24_swapped_from_xrgb32;
//m->dib.convert[DIB_FORMAT_RGB24_SWAPPED    ][PIXEL_FORMAT_ARGB32] = DibMMX::rgb24_swapped_from_argb32;
//m->dib.convert[DIB_FORMAT_RGB24_SWAPPED    ][PIXEL_FORMAT_XRGB32] = DibMMX::rgb24_swapped_from_xrgb32;
//m->dib.convert[DIB_FORMAT_RGB24_SWAPPED    ][PIXEL_FORMAT_A8    ] = DibMMX::zzz24_from_null;
//m->dib.convert[DIB_FORMAT_RGB24_SWAPPED    ][PIXEL_FORMAT_I8    ] = DibMMX::rgb24_swapped_from_i8;

  m->dib.convert[DIB_FORMAT_RGB16_565_NATIVE ][PIXEL_FORMAT_PRGB32] = DibMMX::rgb16_565_native_from_xrgb32;
//m->dib.convert[DIB_FORMAT_RGB16_565_NATIVE ][PIXEL_FORMAT_ARGB32] = DibMMX::rgb16_565_native_from_argb32;
  m->dib.convert[DIB_FORMAT_RGB16_565_NATIVE ][PIXEL_FORMAT_XRGB32] = DibMMX::rgb16_565_native_from_xrgb32;
//m->dib.convert[DIB_FORMAT_RGB16_565_NATIVE ][PIXEL_FORMAT_A8    ] = DibMMX::zzz16_from_null;
//m->dib.convert[DIB_FORMAT_RGB16_565_NATIVE ][PIXEL_FORMAT_I8    ] = DibMMX::rgb16_565_native_from_i8;

//m->dib.convert[DIB_FORMAT_RGB16_565_SWAPPED][PIXEL_FORMAT_PRGB32] = DibMMX::rgb16_565_swapped_from_xrgb32;
//m->dib.convert[DIB_FORMAT_RGB16_565_SWAPPED][PIXEL_FORMAT_ARGB32] = DibMMX::rgb16_565_swapped_from_argb32;
//m->dib.convert[DIB_FORMAT_RGB16_565_SWAPPED][PIXEL_FORMAT_XRGB32] = DibMMX::rgb16_565_swapped_from_xrgb32;
//m->dib.convert[DIB_FORMAT_RGB16_565_SWAPPED][PIXEL_FORMAT_A8    ] = DibMMX::zzz16_from_null;
//m->dib.convert[DIB_FORMAT_RGB16_565_SWAPPED][PIXEL_FORMAT_I8    ] = DibMMX::rgb16_565_swapped_from_i8;

  m->dib.convert[DIB_FORMAT_RGB16_555_NATIVE ][PIXEL_FORMAT_PRGB32] = DibMMX::rgb16_555_native_from_xrgb32;
//m->dib.convert[DIB_FORMAT_RGB16_555_NATIVE ][PIXEL_FORMAT_ARGB32] = DibMMX::rgb16_555_native_from_argb32;
  m->dib.convert[DIB_FORMAT_RGB16_555_NATIVE ][PIXEL_FORMAT_XRGB32] = DibMMX::rgb16_555_native_from_xrgb32;
//m->dib.convert[DIB_FORMAT_RGB16_555_NATIVE ][PIXEL_FORMAT_A8    ] = DibMMX::zzz16_from_null;
//m->dib.convert[DIB_FORMAT_RGB16_555_NATIVE ][PIXEL_FORMAT_I8    ] = DibMMX::rgb16_555_native_from_i8;

//m->dib.convert[DIB_FORMAT_RGB16_555_SWAPPED][PIXEL_FORMAT_PRGB32] = DibMMX::rgb16_555_swapped_from_xrgb32;
//m->dib.convert[DIB_FORMAT_RGB16_555_SWAPPED][PIXEL_FORMAT_ARGB32] = DibMMX::rgb16_555_swapped_from_argb32;
//m->dib.convert[DIB_FORMAT_RGB16_555_SWAPPED][PIXEL_FORMAT_XRGB32] = DibMMX::rgb16_555_swapped_from_xrgb32;
//m->dib.convert[DIB_FORMAT_RGB16_555_SWAPPED][PIXEL_FORMAT_A8    ] = DibMMX::zzz16_from_null;
//m->dib.convert[DIB_FORMAT_RGB16_555_SWAPPED][PIXEL_FORMAT_I8    ] = DibMMX::rgb16_555_swapped_from_i8;

//m->dib.convert[DIB_FORMAT_GREY8            ][PIXEL_FORMAT_PRGB32] = DibMMX::grey8_from_xrgb32;
//m->dib.convert[DIB_FORMAT_GREY8            ][PIXEL_FORMAT_ARGB32] = DibMMX::grey8_from_argb32;
//m->dib.convert[DIB_FORMAT_GREY8            ][PIXEL_FORMAT_XRGB32] = DibMMX::grey8_from_xrgb32;
//m->dib.convert[DIB_FORMAT_GREY8            ][PIXEL_FORMAT_A8    ] = DibMMX::z8_from_null;
//m->dib.convert[DIB_FORMAT_GREY8            ][PIXEL_FORMAT_I8    ] = DibMMX::grey8_from_i8;

  // [Interpolate - Gradient]

  m->interpolate.gradient[PIXEL_FORMAT_ARGB32] = InterpolateMMX::gradient_argb32;
}

#endif // FOG_ARCH_X86
