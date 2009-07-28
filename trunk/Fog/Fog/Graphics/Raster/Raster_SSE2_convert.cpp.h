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

// ============================================================================
// [Fog::Raster - Convert - Argb32 Dest]
// ============================================================================

static void FOG_FASTCALL convert_argb32_from_prgb32_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w);

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;

    pix_load4(src0mm, src);
    pix_unpack_and_demultiply_1x1W(src0mm, src0mm);
    pix_pack_1x1W(src0mm, src0mm);
    pix_store4(dst, src0mm);

    src += 4;
    dst += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;

    pix_load16u(src0mm, src);
    pix_unpack_and_demultiply_2x2W(src0mm, src1mm, src0mm);
    pix_pack_2x2W(src0mm, src0mm, src1mm);
    pix_store16a(dst, src0mm);

    src += 16;
    dst += 16;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL convert_argb32_from_rgb32_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  sysint_t i = w;

  __m128i amask = MaskFF000000FF000000;

  while ((sysuint_t)dst & 15)
  {
    ((uint32_t*)dst)[0] = READ_32(src) | 0xFF000000;

    dst += 4;
    src += 4;
    if (--i == 0) return;
  }

  while (i >= 16)
  {
    __m128i src0mm;
    __m128i src1mm;
    __m128i src2mm;
    __m128i src3mm;

    pix_load16u(src0mm, src + 0);
    pix_load16u(src1mm, src + 16);
    pix_load16u(src2mm, src + 32);
    pix_load16u(src3mm, src + 48);

    src0mm = _mm_or_si128(src0mm, amask);
    src1mm = _mm_or_si128(src1mm, amask);
    src2mm = _mm_or_si128(src2mm, amask);
    src3mm = _mm_or_si128(src3mm, amask);

    pix_store16a(dst + 0, src0mm);
    pix_store16a(dst + 16, src1mm);
    pix_store16a(dst + 32, src2mm);
    pix_store16a(dst + 48, src3mm);

    dst += 64;
    src += 64;
    i -= 16;
  }

  while (i >= 4)
  {
    __m128i src0mm;

    pix_load16u(src0mm, src);
    src0mm = _mm_or_si128(src0mm, amask);
    pix_store16a(dst, src0mm);

    dst += 16;
    src += 16;
    i -= 4;
  }

  while (i)
  {
    ((uint32_t*)dst)[0] = READ_32(src) | 0xFF000000;

    dst += 4;
    src += 4;
    i--;
  }
}

// ============================================================================
// [Fog::Raster - Convert - Prgb32 Dest]
// ============================================================================

static void FOG_FASTCALL convert_prgb32_from_argb32_sse2(uint8_t* dst, const uint8_t* src, sysint_t w)
{
  sysint_t i = w;

  while ((sysuint_t(dst) & 15))
  {
    __m128i src0mm;

    pix_load4(src0mm, src);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_premultiply_1x1W(src0mm, src0mm);
    pix_pack_1x1W(src0mm, src0mm);
    pix_store4(dst, src0mm);

    dst += 4;
    src += 4;
    if (--i == 0) return;
  }

  while (i >= 4)
  {
    __m128i src0mm;
    __m128i src1mm;

    pix_load16u(src0mm, src);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
    pix_pack_2x2W(src0mm, src0mm, src1mm);
    pix_store16a(dst, src0mm);

    dst += 16;
    src += 16;
    i -= 4;
  }

  while (i)
  {
    __m128i src0mm;

    pix_load4(src0mm, src);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_premultiply_1x1W(src0mm, src0mm);
    pix_pack_1x1W(src0mm, src0mm);
    pix_store4(dst, src0mm);

    dst += 4;
    src += 4;
    i--;
  }
}

static void FOG_FASTCALL convert_prgb32_from_argb32_bs_sse2(uint8_t* dst, const uint8_t* src, sysint_t w)
{
  sysint_t i = w;

  while ((sysuint_t(dst) & 15))
  {
    __m128i src0mm;

    pix_load4(src0mm, src);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_swap_1x1W(src0mm, src0mm);
    pix_premultiply_1x1W(src0mm, src0mm);
    pix_pack_1x1W(src0mm, src0mm);
    pix_store4(dst, src0mm);

    dst += 4;
    src += 4;
    if (--i == 0) return;
  }

  while (i >= 4)
  {
    __m128i src0mm;
    __m128i src1mm;

    pix_load16u(src0mm, src);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_swap_2x2W(src0mm, src0mm, src1mm, src1mm);
    pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
    pix_pack_2x2W(src0mm, src0mm, src1mm);
    pix_store16a(dst, src0mm);

    dst += 16;
    src += 16;
    i -= 4;
  }

  while (i)
  {
    __m128i src0mm;

    pix_load4(src0mm, src);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_swap_1x1W(src0mm, src0mm);
    pix_premultiply_1x1W(src0mm, src0mm);
    pix_pack_1x1W(src0mm, src0mm);
    pix_store4(dst, src0mm);

    dst += 4;
    src += 4;
    i--;
  }
}

static void FOG_FASTCALL convert_prgb32_bs_from_argb32_sse2(uint8_t* dst, const uint8_t* src, sysint_t w)
{
  sysint_t i = w;

  while ((sysuint_t(dst) & 15))
  {
    __m128i src0mm;

    pix_load4(src0mm, src);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_premultiply_1x1W(src0mm, src0mm);
    pix_swap_1x1W(src0mm, src0mm);
    pix_pack_1x1W(src0mm, src0mm);
    pix_store4(dst, src0mm);

    dst += 4;
    src += 4;
    if (--i == 0) return;
  }

  while (i >= 4)
  {
    __m128i src0mm;
    __m128i src1mm;

    pix_load16u(src0mm, src);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
    pix_swap_2x2W(src0mm, src0mm, src1mm, src1mm);
    pix_pack_2x2W(src0mm, src0mm, src1mm);
    pix_store16a(dst, src0mm);

    dst += 16;
    src += 16;
    i -= 4;
  }

  while (i)
  {
    __m128i src0mm;

    pix_load4(src0mm, src);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_premultiply_1x1W(src0mm, src0mm);
    pix_swap_1x1W(src0mm, src0mm);
    pix_pack_1x1W(src0mm, src0mm);
    pix_store4(dst, src0mm);

    dst += 4;
    src += 4;
    i--;
  }
}

// ============================================================================
// [Fog::Raster - Convert - Rgb32 Dest]
// ============================================================================

static void FOG_FASTCALL convert_rgb32_from_rgb24_sse2(uint8_t* dst, const uint8_t* src, sysint_t w)
{
  sysint_t i = w;

  while ((sysuint_t(src) & 3))
  {
    ((uint32_t*)dst)[0] = PixFmt_RGB24::fetch(src) | 0xFF000000;
    dst += 4;
    src += 3;
    if (--i == 0) return;
  }

  while (i >= 4)
  {
    __m128i src0mm;
    __m128i src1mm;

    pix_fetch_rgb24_2x2W(src0mm, src1mm, src);
    pix_pack_2x2W(src0mm, src0mm, src1mm);
    pix_store16u(dst, src0mm);
    dst += 16;
    src += 12;
    i -= 4;
  }

  while (i)
  {
    ((uint32_t*)dst)[0] = PixFmt_RGB24::fetch(src) | 0xFF000000;
    dst += 4;
    src += 3;
    i--;
  }
}

static void FOG_FASTCALL convert_rgb32_from_bgr24_sse2(uint8_t* dst, const uint8_t* src, sysint_t w)
{
  sysint_t i = w;

  while ((sysuint_t(src) & 3))
  {
    ((uint32_t*)dst)[0] = PixFmt_BGR24::fetch(src) | 0xFF000000;
    dst += 4;
    src += 3;
    if (--i == 0) return;
  }

  while (i >= 4)
  {
    __m128i src0mm;
    __m128i src1mm;

    pix_fetch_bgr24_2x2W(src0mm, src1mm, src);
    pix_pack_2x2W(src0mm, src0mm, src1mm);
    pix_store16u(dst, src0mm);
    dst += 16;
    src += 12;
    i -= 4;
  }

  while (i)
  {
    ((uint32_t*)dst)[0] = PixFmt_BGR24::fetch(src) | 0xFF000000;
    dst += 4;
    src += 3;
    i--;
  }
}

// ============================================================================
// [Fog::Raster - Convert - MemCpy]
// ============================================================================

static void FOG_FASTCALL convert_memcpy32_sse2(uint8_t* dst, const uint8_t* src, sysint_t w)
{
  while (sysuint_t(dst) & 15)
  {
    copy4(dst, src);
    if (--w == 0) return;
    dst += 4;
    src += 4;
  }

  if (sysuint_t(src) & 15)
  {
    while (w >= 16)
    {
      __m128i src0mm;
      __m128i src1mm;
      __m128i src2mm;
      __m128i src3mm;

      _mm_prefetch((const char*)(src + 64), _MM_HINT_T0);

      pix_load16u(src0mm, src + 0);
      pix_load16u(src1mm, src + 16);
      pix_load16u(src2mm, src + 32);
      pix_load16u(src3mm, src + 48);
      pix_store16a(dst + 0, src0mm);
      pix_store16a(dst + 16, src1mm);
      pix_store16a(dst + 32, src2mm);
      pix_store16a(dst + 48, src3mm);

      dst += 64;
      src += 64;
      w -= 16;
    }
  }
  else
  {
    while (w >= 16)
    {
      __m128i src0mm;
      __m128i src1mm;
      __m128i src2mm;
      __m128i src3mm;

      _mm_prefetch((const char*)(src + 64), _MM_HINT_T0);

      pix_load16a(src0mm, src + 0);
      pix_load16a(src1mm, src + 16);
      pix_load16a(src2mm, src + 32);
      pix_load16a(src3mm, src + 48);
      pix_store16a(dst + 0, src0mm);
      pix_store16a(dst + 16, src1mm);
      pix_store16a(dst + 32, src2mm);
      pix_store16a(dst + 48, src3mm);

      dst += 64;
      src += 64;
      w -= 16;
    }
  }

  switch (w & 15)
  {
    case 15: copy4(dst, src); dst += 4; src += 4;
    case 14: copy4(dst, src); dst += 4; src += 4;
    case 13: copy4(dst, src); dst += 4; src += 4;
    case 12: copy4(dst, src); dst += 4; src += 4;
    case 11: copy4(dst, src); dst += 4; src += 4;
    case 10: copy4(dst, src); dst += 4; src += 4;
    case 9: copy4(dst, src); dst += 4; src += 4;
    case 8: copy4(dst, src); dst += 4; src += 4;
    case 7: copy4(dst, src); dst += 4; src += 4;
    case 6: copy4(dst, src); dst += 4; src += 4;
    case 5: copy4(dst, src); dst += 4; src += 4;
    case 4: copy4(dst, src); dst += 4; src += 4;
    case 3: copy4(dst, src); dst += 4; src += 4;
    case 2: copy4(dst, src); dst += 4; src += 4;
    case 1: copy4(dst, src); dst += 4; src += 4;
  }
}

} // Raster namespace
} // Fog namespace
