// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// For some IDEs to enable code-assist.
#include <Fog/Build/Build.h>

#if defined(FOG_IDE)
#include <Fog/Graphics/RasterUtil/RasterUtil_Defs_SSE2.h>
#endif // FOG_IDE

#include <Fog/Graphics/RasterUtil/RasterUtil_Dib_C.h>

namespace Fog {
namespace RasterUtil {

// ============================================================================
// [Fog::RasterUtil::SSE2 - Dib]
// ============================================================================

struct FOG_HIDDEN DibSSE2
{
  // --------------------------------------------------------------------------
  // [DibSSE2 - MemCpy]
  // --------------------------------------------------------------------------

  static void FOG_FASTCALL memcpy8(
    uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
  {
    sysuint_t i;

    if (w < 16)
    {
      for (i = w; i; i--, dst++, src++) dst[0] = src[0];
    }
    else
    {
      sysuint_t align = (16 - (sysuint_t(dst) & 15)) & 15;
      w -= (sysint_t)align;

      for (i = align; i; i--)
      {
        copy1(dst, src);
        dst++;
        src++;
      }

      if (((sysuint_t)src & 15) == 0)
      {
        for (i = w >> 6; i; i--)
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
        }
      }
      else
      {
        for (i = w >> 6; i; i--)
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
        }
      }

      w &= 63;

      switch (w >> 2)
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

      switch (w & 3)
      {
        case 3: copy1(dst, src); dst += 1; src += 1;
        case 2: copy1(dst, src); dst += 1; src += 1;
        case 1: copy1(dst, src); dst += 1; src += 1;
      }
    }
  }

  static void FOG_FASTCALL memcpy16(
    uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
  {
    memcpy8(dst, src, ByteUtil::mul2(w), closure);
  }

  static void FOG_FASTCALL memcpy24(
    uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
  {
    memcpy8(dst, src, ByteUtil::mul3(w), closure);
  }

  static void FOG_FASTCALL memcpy32(
    uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
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

  // --------------------------------------------------------------------------
  // [DibSSE2 - Convert]
  // --------------------------------------------------------------------------

  static void FOG_FASTCALL prgb32_from_argb32(
    uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
  {
    BLIT_SSE2_32x4_INIT(dst, w);

    BLIT_SSE2_32x4_SMALL_BEGIN(blt)
      __m128i src0mm;

      pix_load4(src0mm, src);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_premultiply_1x1W(src0mm, src0mm);
      pix_pack_1x1W(src0mm, src0mm);
      pix_store4(dst, src0mm);

      dst += 4;
      src += 4;
    BLIT_SSE2_32x4_SMALL_END(blt)

    BLIT_SSE2_32x4_LARGE_BEGIN(blt)
      __m128i src0mm;
      __m128i src1mm;

      pix_load16u(src0mm, src);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
      pix_pack_2x2W(src0mm, src0mm, src1mm);
      pix_store16a(dst, src0mm);

      dst += 16;
      src += 16;
    BLIT_SSE2_32x4_LARGE_END(blt)
  }

  static void FOG_FASTCALL frgb32_from_xrgb32(
    uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
  {
    BLIT_SSE2_32x4_INIT(dst, w);

    BLIT_SSE2_32x4_SMALL_BEGIN(blt)
      __m128i src0mm;

      pix_load4(src0mm, src);
      src0mm = _mm_or_si128(src0mm, Mask_FF000000FF000000_FF000000FF000000);
      pix_store4(dst, src0mm);

      src += 4;
      dst += 4;
    BLIT_SSE2_32x4_SMALL_END(blt)

    BLIT_SSE2_32x4_LARGE_BEGIN(blt)
      __m128i src0mm;

      pix_load16u(src0mm, src);
      src0mm = _mm_or_si128(src0mm, Mask_FF000000FF000000_FF000000FF000000);
      pix_store16a(dst, src0mm);

      src += 16;
      dst += 16;
    BLIT_SSE2_32x4_LARGE_END(blt)
  }

  static void FOG_FASTCALL azzz32_from_a8(
    uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
  {
    BLIT_SSE2_32x4_INIT(dst, w);

    BLIT_SSE2_32x4_SMALL_BEGIN(blt)
      ((uint32_t*)dst)[0] = src[0];

      dst += 4;
      src += 1;
    BLIT_SSE2_32x4_SMALL_END(blt)

    BLIT_SSE2_32x4_LARGE_BEGIN(blt)
      __m128i pix0xmm;

      pix_load4(pix0xmm, src);
      pix_unpack_1x1D(pix0xmm, pix0xmm);
      pix0xmm = _mm_srli_epi32(pix0xmm, 24);

      pix_store16a(dst, pix0xmm);

      dst += 16;
      src += 4;
    BLIT_SSE2_32x4_LARGE_END(blt)
  }

  static void FOG_FASTCALL prgb32_from_i8(
    uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
  {
    const Argb* srcPal = closure->srcPalette + Palette::INDEX_PRGB32;

    BLIT_SSE2_32x4_INIT(dst, w);

    BLIT_SSE2_32x4_SMALL_BEGIN(blt)
      __m128i src0mm;

      pix_load_1xI8(src0mm, src, srcPal);
      pix_store4(dst, src0mm);

      dst += 4;
      src += 1;
    BLIT_SSE2_32x4_SMALL_END(blt)

    BLIT_SSE2_32x4_LARGE_BEGIN(blt)
      __m128i src0mm;

      pix_load_4xI8(src0mm, src, srcPal);
      pix_store16a(dst, src0mm);

      dst += 16;
      src += 4;
    BLIT_SSE2_32x4_LARGE_END(blt)
  }

  static void FOG_FASTCALL prgb32_from_argb32_swapped(
    uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
  {
    BLIT_SSE2_32x4_INIT(dst, w);

    BLIT_SSE2_32x4_SMALL_BEGIN(blt)
      __m128i src0mm;

      pix_load4(src0mm, src);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_swap_1x1W(src0mm, src0mm);
      pix_premultiply_1x1W(src0mm, src0mm);
      pix_pack_1x1W(src0mm, src0mm);
      pix_store4(dst, src0mm);

      dst += 4;
      src += 4;
    BLIT_SSE2_32x4_SMALL_END(blt)

    BLIT_SSE2_32x4_LARGE_BEGIN(blt)
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
    BLIT_SSE2_32x4_LARGE_END(blt)
  }
  
  static void FOG_FASTCALL argb32_from_prgb32(
    uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
  {
    BLIT_SSE2_32x4_INIT(dst, w);

    BLIT_SSE2_32x4_SMALL_BEGIN(blt)
      __m128i src0mm;

      pix_load4(src0mm, src);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_demultiply_1x1W_srcbuf(src0mm, src0mm, src);
      pix_pack_1x1W(src0mm, src0mm);
      pix_store4(dst, src0mm);

      src += 4;
      dst += 4;
    BLIT_SSE2_32x4_SMALL_END(blt)

    BLIT_SSE2_32x4_LARGE_BEGIN(blt)
      __m128i src0mm, src1mm;

      pix_load16u(src0mm, src);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_demultiply_2x2W_srcbuf(src0mm, src0mm, src1mm, src1mm, src);
      pix_pack_2x2W(src0mm, src0mm, src1mm);
      pix_store16a(dst, src0mm);

      src += 16;
      dst += 16;
    BLIT_SSE2_32x4_LARGE_END(blt)
  }

  static void FOG_FASTCALL argb32_from_i8(
    uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
  {
    const Argb* srcPal = closure->srcPalette + Palette::INDEX_ARGB32;

    BLIT_SSE2_32x4_INIT(dst, w);

    BLIT_SSE2_32x4_SMALL_BEGIN(blt)
      __m128i src0mm;

      pix_load_1xI8(src0mm, src, srcPal);
      pix_store4(dst, src0mm);

      dst += 4;
      src += 1;
    BLIT_SSE2_32x4_SMALL_END(blt)

    BLIT_SSE2_32x4_LARGE_BEGIN(blt)
      __m128i src0mm;

      pix_load_4xI8(src0mm, src, srcPal);
      pix_store16a(dst, src0mm);

      dst += 16;
      src += 4;
    BLIT_SSE2_32x4_LARGE_END(blt)
  }










  static void FOG_FASTCALL frgb32_from_argb32(
    uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
  {
    BLIT_SSE2_32x4_INIT(dst, w);

    BLIT_SSE2_32x4_SMALL_BEGIN(blt)
      __m128i src0mm;

      pix_load4(src0mm, src);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_premultiply_1x1W(src0mm, src0mm);
      pix_pack_1x1W(src0mm, src0mm);
      src0mm = _mm_or_si128(src0mm, Mask_FF000000FF000000_FF000000FF000000);
      pix_store4(dst, src0mm);

      dst += 4;
      src += 4;
    BLIT_SSE2_32x4_SMALL_END(blt)

    BLIT_SSE2_32x4_LARGE_BEGIN(blt)
      __m128i src0mm;
      __m128i src1mm;

      pix_load16u(src0mm, src);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
      pix_pack_2x2W(src0mm, src0mm, src1mm);
      src0mm = _mm_or_si128(src0mm, Mask_FF000000FF000000_FF000000FF000000);
      pix_store16a(dst, src0mm);

      dst += 16;
      src += 16;
    BLIT_SSE2_32x4_LARGE_END(blt)
  }

  static void FOG_FASTCALL frgb32_from_i8(
    uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
  {
    const Argb* srcPal = closure->srcPalette;

    for (sysint_t i = w; i; i--)
    {
      ((uint32_t*)dst)[0] = srcPal[src[0]] | 0xFF000000;

      dst += 4;
      src += 1;
    }
  }

  static void FOG_FASTCALL frgb32_from_argb32_swapped(
    uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
  {
    BLIT_SSE2_32x4_INIT(dst, w);

    BLIT_SSE2_32x4_SMALL_BEGIN(blt)
      __m128i src0mm;

      pix_load4(src0mm, src);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_swap_1x1W(src0mm, src0mm);
      pix_premultiply_1x1W(src0mm, src0mm);
      pix_pack_1x1W(src0mm, src0mm);
      src0mm = _mm_or_si128(src0mm, Mask_FF000000FF000000_FF000000FF000000);
      pix_store4(dst, src0mm);

      dst += 4;
      src += 4;
    BLIT_SSE2_32x4_SMALL_END(blt)

    BLIT_SSE2_32x4_LARGE_BEGIN(blt)
      __m128i src0mm;
      __m128i src1mm;

      pix_load16u(src0mm, src);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_swap_2x2W(src0mm, src0mm, src1mm, src1mm);
      pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
      pix_pack_2x2W(src0mm, src0mm, src1mm);
      src0mm = _mm_or_si128(src0mm, Mask_FF000000FF000000_FF000000FF000000);
      pix_store16a(dst, src0mm);

      dst += 16;
      src += 16;
    BLIT_SSE2_32x4_LARGE_END(blt)
  }

  static void FOG_FASTCALL frgb32_from_rgb24_native(
    uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
  {
    sysint_t i = w;

    while ((sysuint_t(src) & 3))
    {
      ((uint32_t*)dst)[0] = RGB24_NATIVE_IO::fetch(src) | 0xFF000000;
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
      ((uint32_t*)dst)[0] = RGB24_NATIVE_IO::fetch(src) | 0xFF000000;
      dst += 4;
      src += 3;
      i--;
    }
  }

  static void FOG_FASTCALL frgb32_from_rgb24_swapped(
    uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
  {
    sysint_t i = w;

    while ((sysuint_t(src) & 3))
    {
      ((uint32_t*)dst)[0] = RGB24_SWAPPED_IO::fetch(src) | 0xFF000000;
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
      ((uint32_t*)dst)[0] = RGB24_SWAPPED_IO::fetch(src) | 0xFF000000;
      dst += 4;
      src += 3;
      i--;
    }
  }

  static void FOG_FASTCALL prgb32_swapped_from_argb32(
    uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
  {
    BLIT_SSE2_32x4_INIT(dst, w);

    BLIT_SSE2_32x4_SMALL_BEGIN(blt)
      __m128i src0mm;

      pix_load4(src0mm, src);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_premultiply_1x1W(src0mm, src0mm);
      pix_swap_1x1W(src0mm, src0mm);
      pix_pack_1x1W(src0mm, src0mm);
      pix_store4(dst, src0mm);

      dst += 4;
      src += 4;
    BLIT_SSE2_32x4_SMALL_END(blt)

    BLIT_SSE2_32x4_LARGE_BEGIN(blt)
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
    BLIT_SSE2_32x4_LARGE_END(blt)
  }

  static void FOG_FASTCALL a8_from_axxx32(
    uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
  {
    BLIT_SSE2_8x16_INIT(dst, w);

    BLIT_SSE2_8x16_SMALL_BEGIN(blt)
      dst[0] = src[ARGB32_ABYTE];

      dst += 1;
      src += 4;
    BLIT_SSE2_8x16_SMALL_END(blt)

    BLIT_SSE2_8x16_LARGE_BEGIN(blt)
      __m128i src0mm;
      __m128i src1mm;

      pix_load_and_unpack_axxx32_64B(src0mm, src1mm, src);
      pix_pack_2x2W(src0mm, src0mm, src1mm);
      pix_store16a(dst, src0mm);

      dst += 16;
      src += 64;
    BLIT_SSE2_8x16_LARGE_END(blt)
  }

  static void FOG_FASTCALL a8_from_i8(
    uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
  {
    const Argb* srcPal = closure->srcPalette + Palette::INDEX_ARGB32;

    for (sysint_t i = w; i; i--)
    {
      dst[0] = srcPal[src[0]].a;

      dst += 1;
      src += 1;
    }
  }
};

} // RasterUtil namespace
} // Fog namespace
