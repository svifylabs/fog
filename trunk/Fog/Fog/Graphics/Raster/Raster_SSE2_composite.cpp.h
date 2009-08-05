// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// For some IDEs to enable code-assist.
#include <Fog/Build/Build.h>
#if defined(FOG_IDE)
#include <Fog/Graphics/Raster/Raster_SSE2_base.cpp.h>
#include <Fog/Graphics/Raster/Raster_SSE2_convert.cpp.h>
#endif // FOG_IDE

namespace Fog {
namespace Raster {

// ============================================================================
// [Fog::Raster - Raster - Src - Argb32]
// ============================================================================

static void FOG_FASTCALL raster_argb32_pixel_src_sse2(
  uint8_t* dst, const Solid* src, const Closure* closure)
{
  uint32_t src0 = src->rgba;
  ((uint32_t*)dst)[0] = src0;
}

static void FOG_FASTCALL raster_argb32_pixel_a8_src_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, const Closure* closure)
{
  if (msk0 == 0xFF)
  {
    __m128i src0mm;

    pix_load4(src0mm, &src->rgbp);
    pix_store4(dst, src0mm);
  }
  else
  {
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_load4(dst0mm, dst);
    pix_load4(src0mm, &src->rgba);

    pix_unpack_1x1W(src0mm, src0mm);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_expand_mask_1x1W(a0mm, msk0);
    pix_lerp_1x1W(dst0mm, src0mm, a0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);
  }
}

static void FOG_FASTCALL raster_argb32_span_solid_src_sse2(
  uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure)
{
  __m128i src0mm;

  pix_load4(src0mm, &src->rgba);
  pix_expand_pixel_1x4B(src0mm, src0mm);

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    pix_store4(dst, src0mm);
    dst += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  while (w >= 4)
  {
    pix_store16a(dst, src0mm);
    pix_store16a(dst + 16, src0mm);
    pix_store16a(dst + 32, src0mm);
    pix_store16a(dst + 48, src0mm);

    dst += 64;
    w -= 4;
  }

  switch (w & 3)
  {
    case 3: pix_store16a(dst, src0mm); dst += 16;
    case 2: pix_store16a(dst, src0mm); dst += 16;
    case 1: pix_store16a(dst, src0mm); dst += 16;
  }

  if ((_i = _j)) { w = 0; goto blt; }
}

static void FOG_FASTCALL raster_argb32_span_solid_a8_src_sse2(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  __m128i src0orig;
  __m128i src0unpacked;

  pix_load4(src0orig, &src->rgba);
  pix_expand_pixel_1x4B(src0orig, src0orig);
  pix_unpack_1x2W(src0unpacked, src0orig);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      if (msk0 == 0xFF)
      {
        pix_store4(dst, src0orig);
      }
      else
      {
        pix_load4(dst0mm, dst);
        pix_unpack_1x1W(dst0mm, dst0mm);
        pix_expand_mask_1x1W(a0mm, msk0);
        pix_lerp_1x1W(dst0mm, src0unpacked, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      if (msk0 == 0xFFFFFFFF)
      {
        pix_store16a(dst, src0orig);
      }
      else
      {
        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_lerp_2x2W(dst0mm, src0unpacked, a0mm, dst1mm, src0unpacked, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_argb32_span_solid_a8_const_src_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (msk0 == 0xFF)
  {
    raster_argb32_span_solid_src_sse2(dst, src, w, closure);
    return;
  }

  __m128i src0orig;
  __m128i src0unpacked;

  pix_load4(src0orig, &src->rgba);
  pix_expand_pixel_1x4B(src0orig, src0orig);
  pix_unpack_1x2W(src0unpacked, src0orig);

  __m128i m0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;

    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_lerp_1x1W(dst0mm, src0unpacked, m0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_lerp_2x2W(dst0mm, src0unpacked, m0mm, dst1mm, src0unpacked, m0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

#define raster_argb32_span_composite_argb32_src_sse2 convert_memcpy32_sse2
#define raster_argb32_span_composite_prgb32_src_sse2 convert_argb32_from_prgb32_sse2
#define raster_argb32_span_composite_rgb32_src_sse2 convert_axxx32_from_xxxx32_sse2
#define raster_argb32_span_composite_rgb24_src_sse2 convert_rgb32_from_rgb24_sse2

static void FOG_FASTCALL raster_argb32_span_composite_a8_src_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    ((uint32_t*)dst)[0] = (uint32_t)src[0] << 24;

    dst += 4;
    src += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm;
    pix_load4(src0mm, src);
    pix_unpack_1x1D(src0mm, src0mm);
    src0mm = _mm_slli_epi32(src0mm, 24);
    pix_store16a(dst, src0mm);

    dst += 16;
    src += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_argb32_span_composite_argb32_a8_src_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      pix_load4(src0mm, src);

      if (msk0 == 0xFF)
      {
        pix_store4(dst, src0mm);
      }
      else
      {
        pix_load4(dst0mm, dst);
        pix_unpack_1x1W(src0mm, src0mm);
        pix_unpack_1x1W(dst0mm, dst0mm);
        pix_expand_mask_1x1W(a0mm, msk0);
        pix_lerp_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      pix_load16u(src0mm, src);

      if (msk0 == 0xFFFFFFFF)
      {
        pix_store16a(dst, src0mm);
      }
      else
      {
        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_lerp_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_argb32_span_composite_prgb32_a8_src_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      pix_load4(src0mm, src);

      if (msk0 == 0xFF)
      {
        pix_store4(dst, src0mm);
      }
      else
      {
        pix_load4(dst0mm, dst);
        pix_unpack_1x1W(src0mm, src0mm);
        pix_unpack_1x1W(dst0mm, dst0mm);
        pix_demultiply_1x1W_srcbuf(src0mm, src0mm, src);
        pix_expand_mask_1x1W(a0mm, msk0);
        pix_lerp_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);

    if (msk0 != 0x00000000)
    {
      pix_load16u(src0mm, src);

      if (msk0 == 0xFFFFFFFF)
      {
        pix_store16a(dst, src0mm);
      }
      else
      {
        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_demultiply_2x2W_srcbuf(src0mm, src0mm, src1mm, src1mm, src);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_lerp_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_argb32_span_composite_rgb32_a8_src_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);

    if (msk0 != 0x00)
    {
      pix_load4(src0mm, src);
      src0mm = _mm_or_si128(src0mm, Mask_FF000000FF000000_FF000000FF000000);

      if (msk0 == 0xFF)
      {
        pix_store4(dst, src0mm);
      }
      else
      {
        pix_load4(dst0mm, dst);
        pix_unpack_1x1W(src0mm, src0mm);
        pix_unpack_1x1W(dst0mm, dst0mm);
        pix_expand_mask_1x1W(a0mm, msk0);
        pix_lerp_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);

    if (msk0 != 0x00000000)
    {
      pix_load16u(src0mm, src);
      src0mm = _mm_or_si128(src0mm, Mask_FF000000FF000000_FF000000FF000000);

      if (msk0 == 0xFFFFFFFF)
      {
        pix_store16a(dst, src0mm);
      }
      else
      {
        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_lerp_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_argb32_span_composite_rgb24_a8_src_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);

    if (msk0 != 0x00)
    {
      uint32_t src0 = PixFmt_RGB24::fetch(src);

      if (msk0 == 0xFF)
      {
        ((uint32_t*)dst)[0] = src0;
      }
      else
      {
        pix_load4(dst0mm, dst);
        pix_unpack_1x1W(src0mm, src0);
        pix_unpack_1x1W(dst0mm, dst0mm);
        pix_expand_mask_1x1W(a0mm, msk0);
        pix_lerp_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    src += 3;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);

    if (msk0 != 0x00000000)
    {
      pix_fetch_rgb24_2x2W(src0mm, src1mm, src);

      if (msk0 == 0xFFFFFFFF)
      {
        pix_pack_2x2W(src0mm, src0mm, src1mm);
        pix_store16a(dst, src0mm);
      }
      else
      {
        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_lerp_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 12;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_argb32_span_composite_a8_a8_src_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      uint32_t src0 = src[0] << 24;

      if (msk0 == 0xFF)
      {
        ((uint32_t*)dst)[0] = src0;
      }
      else
      {
        pix_load4(dst0mm, dst);
        pix_unpack_1x1W(src0mm, src0);
        pix_unpack_1x1W(dst0mm, dst0mm);
        pix_expand_mask_1x1W(a0mm, msk0);
        pix_lerp_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    src += 1;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      pix_load4(src0mm, src);
      pix_unpack_1x1D(src0mm, src0mm);
      src0mm = _mm_slli_epi32(src0mm, 24);

      if (msk0 == 0xFFFFFFFF)
      {
        pix_store16a(dst, src0mm);
      }
      else
      {
        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_lerp_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 4;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_argb32_span_composite_i8_a8_src_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const Rgba* srcPal = closure->srcPalette + Palette::IndexARGB32;

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      pix_load_1xI8(src0mm, src, srcPal);

      if (msk0 == 0xFF)
      {
        pix_store4(dst, src0mm);
      }
      else
      {
        pix_load4(dst0mm, dst);
        pix_unpack_1x1W(src0mm, src0mm);
        pix_unpack_1x1W(dst0mm, dst0mm);
        pix_expand_mask_1x1W(a0mm, msk0);
        pix_lerp_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    src += 1;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      pix_load_4xI8(src0mm, src, srcPal);

      if (msk0 == 0xFFFFFFFF)
      {
        pix_store16a(dst, src0mm);
      }
      else
      {
        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_lerp_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 4;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_argb32_span_composite_argb32_a8_const_src_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_argb32_span_composite_argb32_src_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    pix_load4(src0mm, src);
    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_lerp_ialpha_1x1W(dst0mm, src0mm, m0mm, im0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16u(src0mm, src);
    pix_load16a(dst0mm, dst);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_lerp_ialpha_2x2W(dst0mm, src0mm, m0mm, im0mm, dst1mm, src1mm, m0mm, im0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_argb32_span_composite_prgb32_a8_const_src_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_argb32_span_composite_prgb32_src_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    pix_load4(src0mm, src);
    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_demultiply_1x1W_srcbuf(src0mm, src0mm, src);
    pix_lerp_ialpha_1x1W(dst0mm, src0mm, m0mm, im0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16u(src0mm, src);
    pix_load16a(dst0mm, dst);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_demultiply_2x2W_srcbuf(src0mm, src0mm, src1mm, src1mm, src);
    pix_lerp_ialpha_2x2W(dst0mm, src0mm, m0mm, im0mm, dst1mm, src1mm, m0mm, im0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_argb32_span_composite_rgb32_a8_const_src_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_argb32_span_composite_rgb32_src_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    pix_load4(src0mm, src);
    src0mm = _mm_or_si128(src0mm, Mask_FF000000FF000000_FF000000FF000000);
    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_lerp_ialpha_1x1W(dst0mm, src0mm, m0mm, im0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16u(src0mm, src);
    pix_load16a(dst0mm, dst);
    src0mm = _mm_or_si128(src0mm, Mask_FF000000FF000000_FF000000FF000000);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_lerp_ialpha_2x2W(dst0mm, src0mm, m0mm, im0mm, dst1mm, src1mm, m0mm, im0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_argb32_span_composite_rgb24_a8_const_src_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_argb32_span_composite_rgb24_src_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(src0mm, PixFmt_RGB24::fetch(src));
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_lerp_ialpha_1x1W(dst0mm, src0mm, m0mm, im0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 3;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_fetch_rgb24_2x2W(src0mm, src1mm, src);
    pix_load16a(dst0mm, dst);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_lerp_ialpha_2x2W(dst0mm, src0mm, m0mm, im0mm, dst1mm, src1mm, m0mm, im0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 12;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_argb32_span_composite_a8_a8_const_src_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_argb32_span_composite_a8_src_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(src0mm, src[0]);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_lerp_ialpha_1x1W(dst0mm, src0mm, m0mm, im0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load4(src0mm, src);
    pix_load16a(dst0mm, dst);
    pix_unpack_1x1D(src0mm, src0mm);
    src0mm = _mm_slli_epi32(src0mm, 24);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_lerp_ialpha_2x2W(dst0mm, src0mm, m0mm, im0mm, dst1mm, src1mm, m0mm, im0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_argb32_span_composite_i8_a8_const_src_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_argb32_span_composite_argb32_src_sse2(dst, src, w, closure);
    return;
  }

  const Rgba* srcPal = closure->srcPalette + Palette::IndexARGB32;

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    pix_load_1xI8(src0mm, src, srcPal);
    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_lerp_ialpha_1x1W(dst0mm, src0mm, m0mm, im0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load_4xI8(src0mm, src, srcPal);
    pix_load16a(dst0mm, dst);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_lerp_ialpha_2x2W(dst0mm, src0mm, m0mm, im0mm, dst1mm, src1mm, m0mm, im0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

// ============================================================================
// [Fog::Raster - Raster - Src - Prgb32]
// ============================================================================

static void FOG_FASTCALL raster_prgb32_pixel_src_sse2(
  uint8_t* dst, const Solid* src, const Closure* closure)
{
  uint32_t src0 = src->rgbp;
  ((uint32_t*)dst)[0] = src0;
}

static void FOG_FASTCALL raster_prgb32_pixel_a8_src_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, const Closure* closure)
{
  if (msk0 == 0xFF)
  {
    __m128i src0mm;

    pix_load4(src0mm, &src->rgbp);
    pix_store4(dst, src0mm);
  }
  else
  {
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_load4(dst0mm, dst);
    pix_load4(src0mm, &src->rgbp);

    pix_unpack_1x1W(src0mm, src0mm);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_expand_mask_1x1W(a0mm, msk0);
    pix_lerp_1x1W(dst0mm, src0mm, a0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);
  }
}

static void FOG_FASTCALL raster_prgb32_span_solid_src_sse2(
  uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure)
{
  __m128i src0mm;

  pix_load4(src0mm, &src->rgbp);
  pix_expand_pixel_1x4B(src0mm, src0mm);

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    pix_store4(dst, src0mm);
    dst += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  while (w >= 4)
  {
    pix_store16a(dst, src0mm);
    pix_store16a(dst + 16, src0mm);
    pix_store16a(dst + 32, src0mm);
    pix_store16a(dst + 48, src0mm);

    dst += 64;
    w -= 4;
  }

  switch (w & 3)
  {
    case 3: pix_store16a(dst, src0mm); dst += 16;
    case 2: pix_store16a(dst, src0mm); dst += 16;
    case 1: pix_store16a(dst, src0mm); dst += 16;
  }

  if ((_i = _j)) { w = 0; goto blt; }
}

static void FOG_FASTCALL raster_prgb32_span_solid_a8_src_sse2(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  __m128i src0orig;
  __m128i src0unpacked;

  pix_load4(src0orig, &src->rgbp);
  pix_expand_pixel_1x4B(src0orig, src0orig);
  pix_unpack_1x2W(src0unpacked, src0orig);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);
    if (msk0 == 0xFF)
    {
      pix_store4(dst, src0orig);
    }
    else if (msk0)
    {
      pix_load4(dst0mm, dst);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_expand_mask_1x1W(a0mm, msk0);
      pix_lerp_1x1W(dst0mm, src0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      if (msk0 == 0xFFFFFFFF)
      {
        pix_store16a(dst, src0orig);
      }
      else
      {
        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_lerp_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_solid_a8_const_src_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (msk0 == 0xFF)
  {
    raster_prgb32_span_solid_src_sse2(dst, src, w, closure);
    return;
  }

  __m128i src0orig;
  __m128i src0unpacked;

  pix_load4(src0orig, &src->rgbp);
  pix_expand_pixel_1x4B(src0orig, src0orig);
  pix_unpack_1x2W(src0unpacked, src0orig);

  __m128i m0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;

    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_lerp_1x1W(dst0mm, src0mm, m0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;

    pix_load16a(dst0mm, dst);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_lerp_2x2W(dst0mm, src0mm, m0mm, dst1mm, src1mm, m0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

#define raster_prgb32_span_composite_argb32_src_sse2 convert_prgb32_from_argb32_sse2
#define raster_prgb32_span_composite_prgb32_src_sse2 convert_memcpy32_sse2
#define raster_prgb32_span_composite_rgb32_src_sse2 convert_axxx32_from_xxxx32_sse2
#define raster_prgb32_span_composite_rgb24_src_sse2 convert_rgb32_from_rgb24_sse2
#define raster_prgb32_span_composite_a8_src_sse2 raster_argb32_span_composite_a8_src_sse2
#define raster_prgb32_span_composite_i8_src_sse2 convert_prgb32_from_i8_sse2

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_a8_src_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);

    if (msk0 != 0x00)
    {
      pix_load4(src0mm, src);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_premultiply_1x1W(src0mm, src0mm);

      if (msk0 != 0xFF)
      {
        pix_load4(dst0mm, dst);
        pix_unpack_1x1W(dst0mm, dst0mm);
        pix_expand_mask_1x1W(a0mm, msk0);
        pix_lerp_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
      else
      {
        pix_pack_1x1W(src0mm, src0mm);
        pix_store4(dst, src0mm);
      }
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);

    if (msk0 != 0x00000000)
    {
      pix_load16u(src0mm, src);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);

      if (msk0 != 0xFFFFFFFF)
      {
        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_lerp_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
      else
      {
        pix_pack_2x2W(src0mm, src0mm, src1mm);
        pix_store16a(dst, src0mm);
      }
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_a8_src_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      pix_load4(src0mm, src);

      if (msk0 != 0xFF)
      {
        pix_load4(dst0mm, dst);
        pix_unpack_1x1W(src0mm, src0mm);
        pix_unpack_1x1W(dst0mm, dst0mm);
        pix_expand_mask_1x1W(a0mm, msk0);
        pix_lerp_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
      else
      {
        pix_store4(dst, src0mm);
      }
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      pix_load16u(src0mm, src);

      if (msk0 != 0xFFFFFFFF)
      {
        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_lerp_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
      else
      {
        pix_store16a(dst, src0mm);
      }
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb32_a8_src_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);

    if (msk0 != 0x00)
    {
      pix_load4(src0mm, src);
      src0mm = _mm_or_si128(src0mm, Mask_FF000000FF000000_FF000000FF000000);

      if (msk0 != 0xFF)
      {
        pix_load4(dst0mm, dst);
        pix_unpack_1x1W(src0mm, src0mm);
        pix_unpack_1x1W(dst0mm, dst0mm);
        pix_expand_mask_1x1W(a0mm, msk0);
        pix_lerp_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
      else
      {
        pix_store4(dst, src0mm);
      }
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      pix_load16u(src0mm, src);
      src0mm = _mm_or_si128(src0mm, Mask_FF000000FF000000_FF000000FF000000);

      if (msk0 != 0xFFFFFFFF)
      {
        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_lerp_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
      else
      {
        pix_store16a(dst, src0mm);
      }
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb24_a8_src_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      uint32_t src0 = PixFmt_RGB24::fetch(src);

      if (msk0 != 0xFF)
      {
        pix_load4(dst0mm, dst);
        pix_unpack_1x1W(src0mm, src0);
        pix_unpack_1x1W(dst0mm, dst0mm);
        pix_expand_mask_1x1W(a0mm, msk0);
        pix_lerp_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
      else
      {
        ((uint32_t*)dst)[0] = src0;
      }
    }

    dst += 4;
    src += 3;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);

    if (msk0 != 0x00000000)
    {
      pix_fetch_rgb24_2x2W(src0mm, src1mm, src);

      if (msk0 != 0xFFFFFFFF)
      {
        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_lerp_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
      else
      {
        pix_pack_2x2W(src0mm, src0mm, src1mm);
        pix_store16a(dst, src0mm);
      }
    }

    dst += 16;
    src += 12;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

#define raster_prgb32_span_composite_a8_a8_src_sse2 raster_argb32_span_composite_a8_a8_src_sse2

static void FOG_FASTCALL raster_prgb32_span_composite_i8_a8_src_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const Rgba* srcPal = closure->srcPalette + Palette::IndexPRGB32;

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      pix_load_1xI8(src0mm, src, srcPal);

      if (msk0 == 0xFF)
      {
        pix_store4(dst, src0mm);
      }
      else
      {
        pix_load4(dst0mm, dst);
        pix_unpack_1x1W(src0mm, src0mm);
        pix_unpack_1x1W(dst0mm, dst0mm);
        pix_expand_mask_1x1W(a0mm, msk0);
        pix_lerp_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    src += 1;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      pix_load_4xI8(src0mm, src, srcPal);

      if (msk0 == 0xFFFFFFFF)
      {
        pix_store16a(dst, src0mm);
      }
      else
      {
        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_lerp_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 4;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_a8_const_src_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_prgb32_span_composite_argb32_src_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    pix_load4(src0mm, src);
    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_premultiply_1x1W(src0mm, src0mm);
    pix_lerp_ialpha_1x1W(dst0mm, src0mm, m0mm, im0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16u(src0mm, src);
    pix_load16a(dst0mm, dst);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
    pix_lerp_ialpha_2x2W(dst0mm, src0mm, m0mm, im0mm, dst1mm, src1mm, m0mm, im0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_a8_const_src_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_prgb32_span_composite_prgb32_src_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    pix_load4(src0mm, src);
    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_lerp_ialpha_1x1W(dst0mm, src0mm, m0mm, im0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16u(src0mm, src);
    pix_load16a(dst0mm, dst);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_lerp_ialpha_2x2W(dst0mm, src0mm, m0mm, im0mm, dst1mm, src1mm, m0mm, im0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb32_a8_const_src_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_prgb32_span_composite_rgb32_src_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    pix_load4(src0mm, src);
    src0mm = _mm_or_si128(src0mm, Mask_FF000000FF000000_FF000000FF000000);
    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_lerp_ialpha_1x1W(dst0mm, src0mm, m0mm, im0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16u(src0mm, src);
    pix_load16a(dst0mm, dst);
    src0mm = _mm_or_si128(src0mm, Mask_FF000000FF000000_FF000000FF000000);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_lerp_ialpha_2x2W(dst0mm, src0mm, m0mm, im0mm, dst1mm, src1mm, m0mm, im0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb24_a8_const_src_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_prgb32_span_composite_rgb24_src_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(src0mm, PixFmt_RGB24::fetch(src));
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_lerp_ialpha_1x1W(dst0mm, src0mm, m0mm, im0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 3;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_fetch_rgb24_2x2W(src0mm, src1mm, src);
    pix_load16a(dst0mm, dst);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_lerp_ialpha_2x2W(dst0mm, src0mm, m0mm, im0mm, dst1mm, src1mm, m0mm, im0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 12;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

#define raster_prgb32_span_composite_a8_a8_const_src_sse2 raster_argb32_span_composite_a8_a8_const_src_sse2

static void FOG_FASTCALL raster_prgb32_span_composite_i8_a8_const_src_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_prgb32_span_composite_argb32_src_sse2(dst, src, w, closure);
    return;
  }

  const Rgba* srcPal = closure->srcPalette + Palette::IndexPRGB32;

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    pix_load_1xI8(src0mm, src, srcPal);
    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_lerp_ialpha_1x1W(dst0mm, src0mm, m0mm, im0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load_4xI8(src0mm, src, srcPal);
    pix_load16a(dst0mm, dst);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_lerp_ialpha_2x2W(dst0mm, src0mm, m0mm, im0mm, dst1mm, src1mm, m0mm, im0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

// ============================================================================
// [Fog::Raster - Raster - Src - Rgb32]
// ============================================================================

static void FOG_FASTCALL raster_rgb32_pixel_src_sse2(
  uint8_t* dst, const Solid* src, const Closure* closure)
{
  uint32_t src0 = src->rgbp | 0xFF000000;
  ((uint32_t*)dst)[0] = src0;
}

static void FOG_FASTCALL raster_rgb32_pixel_a8_src_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, const Closure* closure)
{
  uint32_t src0 = src->rgbp | 0xFF000000;

  if (msk0 == 0xFF)
  {
    ((uint32_t*)dst)[0] = src0;
  }
  else
  {
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_load4(src0mm, &src->rgba);
    pix_load4(dst0mm, dst);
    dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
    pix_expand_mask_1x1W(a0mm, msk0);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_lerp_1x1W(dst0mm, src0mm, a0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);
  }
}

static void FOG_FASTCALL raster_rgb32_span_solid_src_sse2(
  uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure)
{
  __m128i src0mm;

  pix_load4(src0mm, &src->rgbp);
  src0mm = _mm_or_si128(src0mm, Mask_FF000000FF000000_FF000000FF000000);
  pix_expand_pixel_1x4B(src0mm, src0mm);

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    pix_store4(dst, src0mm);
    dst += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  while (w >= 4)
  {
    pix_store16a(dst, src0mm);
    pix_store16a(dst + 16, src0mm);
    pix_store16a(dst + 32, src0mm);
    pix_store16a(dst + 48, src0mm);

    dst += 64;
    w -= 4;
  }
  switch (w & 3)
  {
    case 3: pix_store16a(dst, src0mm); dst += 16;
    case 2: pix_store16a(dst, src0mm); dst += 16;
    case 1: pix_store16a(dst, src0mm); dst += 16;
  }

  if ((_i = _j)) { w = 0; goto blt; }
}

static void FOG_FASTCALL raster_rgb32_span_solid_a8_src_sse2(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  __m128i src0orig;
  __m128i src0unpacked;

  pix_load4(src0orig, &src->rgbp);
  src0orig = _mm_or_si128(src0orig, Mask_FF000000FF000000_FF000000FF000000);
  pix_expand_pixel_1x4B(src0orig, src0orig);
  pix_unpack_1x2W(src0unpacked, src0orig);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      if (msk0 == 0xFF)
      {
        pix_store4(dst, src0orig);
      }
      else
      {
        pix_load4(dst0mm, dst);
        dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
        pix_unpack_1x1W(dst0mm, dst0mm);
        pix_expand_mask_1x1W(a0mm, msk0);
        pix_expand_pixel_1x2W(a0mm, a0mm);
        pix_lerp_1x1W(dst0mm, src0unpacked, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      if (msk0 == 0xFFFFFFFF)
      {
        pix_store16a(dst, src0orig);
      }
      else
      {
        pix_load16a(dst0mm, dst);
        dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_lerp_2x2W(dst0mm, src0unpacked, a0mm, dst1mm, src0unpacked, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_rgb32_span_solid_a8_const_src_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (msk0 == 0xFF)
  {
    raster_rgb32_span_solid_src_sse2(dst, src, w, closure);
    return;
  }

  __m128i src0mm;
  __m128i m0mm;

  pix_load4(src0mm, &src->rgbp);
  src0mm = _mm_or_si128(src0mm, Mask_FF000000FF000000_FF000000FF000000);
  pix_expand_pixel_1x4B(src0mm, src0mm);
  pix_unpack_1x2W(src0mm, src0mm);

  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_multiply_1x1W(src0mm, src0mm, m0mm);
  pix_pack_1x1W(src0mm, src0mm);
  pix_negate_1x1W(m0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;

    pix_load4(dst0mm, dst);
    dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_multiply_1x1W(dst0mm, dst0mm, m0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_adds_1x1B(dst0mm, dst0mm, src0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);
    dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_multiply_2x2W(dst0mm, dst0mm, m0mm, dst1mm, dst1mm, m0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_adds_1x4B(dst0mm, dst0mm, src0mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

#define raster_rgb32_span_composite_argb32_src_sse2 convert_rgb32_from_argb32_sse2
#define raster_rgb32_span_composite_prgb32_src_sse2 convert_axxx32_from_xxxx32_sse2
#define raster_rgb32_span_composite_rgb32_src_sse2 convert_memcpy32_sse2
#define raster_rgb32_span_composite_rgb24_src_sse2 convert_rgb32_from_rgb24_sse2
#define raster_rgb32_span_composite_a8_src_sse2 ((SpanCompositeFn)raster_rgb32_span_solid_clear_sse2)
#define raster_rgb32_span_composite_i8_src_sse2 convert_rgb32_from_i8_sse2

static void FOG_FASTCALL raster_rgb32_span_composite_argb32_a8_src_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;
    __m128i msk0mm;

    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      pix_load4(src0mm, src);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_premultiply_1x1W(src0mm, src0mm);

      if (msk0 == 0xFF)
      {
        pix_fill_alpha_1x1W(src0mm);
        pix_pack_1x1W(src0mm, src0mm);
        pix_store4(dst, src0mm);
      }
      else
      {
        pix_load4(dst0mm, dst);
        pix_unpack_1x1W(dst0mm, dst0mm);
        pix_expand_mask_1x1W(msk0mm, msk0);
        pix_lerp_1x1W(dst0mm, src0mm, msk0mm);
        pix_fill_alpha_1x1W(dst0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;
    __m128i msk0mm, msk1mm;

    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      pix_load16u(src0mm, src);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);

      if (msk0 == 0xFFFFFFFF)
      {
        pix_pack_2x2W(src0mm, src0mm, src1mm);
        src0mm = _mm_or_si128(src0mm, Mask_FF000000FF000000_FF000000FF000000);
        pix_store16a(dst, src0mm);
      }
      else
      {
        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(msk0mm, msk1mm, msk0);
        pix_lerp_2x2W(dst0mm, src0mm, msk0mm, dst1mm, src1mm, msk1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_rgb32_span_composite_i8_a8_src_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const Rgba* srcPal = closure->srcPalette;

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;
    __m128i msk0mm;

    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      pix_load_1xI8(src0mm, src, srcPal);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_premultiply_1x1W(src0mm, src0mm);

      if (msk0 == 0xFF)
      {
        pix_fill_alpha_1x1W(src0mm);
        pix_pack_1x1W(src0mm, src0mm);
        pix_store4(dst, src0mm);
      }
      else
      {
        pix_load4(dst0mm, dst);
        pix_unpack_1x1W(dst0mm, dst0mm);
        pix_expand_mask_1x1W(msk0mm, msk0);
        pix_lerp_1x1W(dst0mm, src0mm, msk0mm);
        pix_fill_alpha_1x1W(dst0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    src += 1;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;
    __m128i msk0mm, msk1mm;

    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      pix_load_4xI8(src0mm, src, srcPal);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);

      if (msk0 == 0xFFFFFFFF)
      {
        pix_pack_2x2W(src0mm, src0mm, src1mm);
        src0mm = _mm_or_si128(src0mm, Mask_FF000000FF000000_FF000000FF000000);
        pix_store16a(dst, src0mm);
      }
      else
      {
        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(msk0mm, msk1mm, msk0);
        pix_lerp_2x2W(dst0mm, src0mm, msk0mm, dst1mm, src1mm, msk1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 4;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

#define raster_rgb32_span_composite_prgb32_a8_src_sse2 raster_rgb32_span_composite_rgb32_a8_srcover_sse2
#define raster_rgb32_span_composite_rgb32_a8_src_sse2 raster_rgb32_span_composite_rgb32_a8_srcover_sse2
#define raster_rgb32_span_composite_rgb24_a8_src_sse2 raster_rgb32_span_composite_rgb24_a8_srcover_sse2
#define raster_rgb32_span_composite_a8_a8_src_sse2 ((SpanCompositeMskFn)raster_rgb32_span_solid_a8_clear_sse2)

static void FOG_FASTCALL raster_rgb32_span_composite_argb32_a8_const_src_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (msk0 == 0xFF)
  {
    raster_rgb32_span_composite_argb32_src_sse2(dst, src, w, closure);
    return;
  }

  __m128i msk0mm, imsk0mm;
  pix_expand_mask_1x1W(msk0mm, msk0);
  pix_expand_pixel_1x2W(msk0mm, msk0mm);
  pix_negate_1x2W(imsk0mm, msk0mm);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;

    pix_load4(src0mm, src);
    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_premultiply_1x1W(src0mm, src0mm);
    pix_lerp_ialpha_1x1W(dst0mm, src0mm, msk0mm, imsk0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;

    pix_load16u(src0mm, src);
    pix_load16a(dst0mm, dst);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
    pix_lerp_ialpha_2x2W(dst0mm, src0mm, msk0mm, imsk0mm, dst1mm, src1mm, msk0mm, imsk0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_rgb32_span_composite_i8_a8_const_src_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (msk0 == 0xFF)
  {
    raster_rgb32_span_composite_i8_src_sse2(dst, src, w, closure);
    return;
  }

  const Rgba* srcPal = closure->srcPalette + Palette::IndexPRGB32;

  __m128i msk0mm, imsk0mm;
  pix_expand_mask_1x1W(msk0mm, msk0);
  pix_expand_pixel_1x2W(msk0mm, msk0mm);
  pix_negate_1x2W(imsk0mm, msk0mm);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;

    pix_load_1xI8(src0mm, src, srcPal);
    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_lerp_ialpha_1x1W(dst0mm, src0mm, msk0mm, imsk0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;

    pix_load_4xI8(src0mm, src, srcPal);
    pix_load16a(dst0mm, dst);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_lerp_ialpha_2x2W(dst0mm, src0mm, msk0mm, imsk0mm, dst1mm, src1mm, msk0mm, imsk0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

#define raster_rgb32_span_composite_prgb32_a8_const_src_sse2 raster_rgb32_span_composite_rgb32_a8_const_srcover_sse2
#define raster_rgb32_span_composite_rgb32_a8_const_src_sse2 raster_rgb32_span_composite_rgb32_a8_const_srcover_sse2
#define raster_rgb32_span_composite_rgb24_a8_const_src_sse2 raster_rgb32_span_composite_rgb24_a8_const_srcover_sse2
#define raster_rgb32_span_composite_a8_a8_const_src_sse2 ((SpanCompositeMskConstFn)raster_rgb32_span_solid_a8_const_clear_sse2)

// ============================================================================
// [Fog::Raster - Raster - Src - A8]
// ============================================================================

static void FOG_FASTCALL raster_a8_pixel_src_sse2(
  uint8_t* dst, const Solid* src, const Closure* closure)
{
  uint32_t src0 = src->rgba >> 24;
  dst[0] = (uint8_t)src0;
}

static void FOG_FASTCALL raster_a8_pixel_a8_src_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, const Closure* closure)
{
  uint32_t src0 = src->rgba >> 24;

  if (msk0 == 0xFF)
  {
    dst[0] = (uint8_t)src0;
  }
  else
  {
    dst[0] = (uint8_t)singlelerp(dst[0], src0, msk0);
  }
}

static void FOG_FASTCALL raster_a8_span_solid_src_sse2(
  uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure)
{
  uint32_t src0 = src->rgba;
  __m128i src0mm;
  pix_expand_a8_1x4D(src0mm, src0 >> 24);

  BLIT_SSE2_8x16_INIT(dst, w)

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    dst[0] = (uint8_t)src0;
    dst += 1;
  BLIT_SSE2_8x16_SMALL_END(blt)

  while (w >= 4)
  {
    pix_store16a(dst, src0mm);
    pix_store16a(dst + 16, src0mm);
    pix_store16a(dst + 32, src0mm);
    pix_store16a(dst + 48, src0mm);

    dst += 64;
    w -= 4;
  }

  switch (w & 3)
  {
    case 3: pix_store16a(dst, src0mm); dst += 16;
    case 2: pix_store16a(dst, src0mm); dst += 16;
    case 1: pix_store16a(dst, src0mm); dst += 16;
  }

  if ((_i = _j)) { w = 0; goto blt; }
}

static void FOG_FASTCALL raster_a8_span_solid_a8_src_sse2(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  uint32_t src0 = src->rgba >> 24;

  __m128i src0orig;
  __m128i src0unpacked;

  pix_load4(src0orig, &src->rgba);
  pix_unpack_1x1W(src0unpacked, src0orig);
  pix_expand_alpha_1x1W(src0unpacked, src0unpacked);
  pix_expand_pixel_1x2W(src0unpacked, src0unpacked);
  pix_expand_pixel_1x4B(src0orig, src0orig);

  BLIT_SSE2_8x16_INIT(dst, w);

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      if (msk0 == 0xFF)
      {
        dst[0] = (uint8_t)src0;
      }
      else
      {
        dst[0] = (uint8_t)singlelerp(dst[0], src0, msk0);
      }
    }

    dst += 1;
    msk += 1;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    uint32_t msk0IsZero;
    uint32_t msk0IsFull;

    __m128i msk0mm;

    pix_load16u(msk0mm, msk);
    pix_analyze_mask_16B(msk0IsZero, msk0IsFull, msk0mm);

    if (msk0IsZero != 0xFFFF)
    {
      if (msk0IsFull == 0xFFFF)
      {
        pix_store16a(dst, src0orig);
      }
      else
      {
        __m128i dst0mm, dst1mm;
        __m128i a0mm, a1mm;

        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_unpack_2x2W(a0mm, a1mm, msk0mm);
        pix_lerp_2x2W(dst0mm, src0unpacked, a0mm, dst1mm, src0unpacked, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    msk += 16;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

static void FOG_FASTCALL raster_a8_span_solid_a8_const_src_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (msk0 == 0xFF)
  {
    raster_a8_span_solid_src_sse2(dst, src, w, closure);
    return;
  }

  uint32_t src0 = src->rgba >> 24;

  __m128i src0mm;
  __m128i m0mm;

  pix_load4(src0mm, &src->rgba);
  pix_unpack_1x1W(src0mm, src0mm);
  pix_expand_alpha_1x1W(src0mm, src0mm);
  pix_expand_pixel_1x2W(src0mm, src0mm);

  m0mm = _mm_cvtsi32_si128(singleexpand(msk0));
  pix_unpack_1x1W(m0mm, m0mm);
  pix_expand_pixel_1x2W(m0mm, m0mm);

  pix_multiply_1x1W(src0mm, src0mm, m0mm);
  pix_negate_1x1W(m0mm, m0mm);

  src0 = singlemul(src0, msk0);
  msk0 = singleneg(msk0);

  BLIT_SSE2_8x16_INIT(dst, w);

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    dst[0] = (uint8_t)(singlemul(dst[0], msk0) + src0);
    dst += 1;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_multiply_2x2W(dst0mm, dst0mm, m0mm, dst1mm, dst1mm, m0mm);
    pix_adds_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

static void FOG_FASTCALL raster_a8_span_composite_axxx32_src_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_8x16_INIT(dst, w);

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    dst[0] = src[RGB32_AByte];

    dst += 1;
    src += 4;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;

    pix_load_and_unpack_axxx32_64B(src0mm, src1mm, src);
    pix_pack_2x2W(src0mm, src0mm, src1mm);
    pix_store16a(dst, src0mm);

    dst += 16;
    src += 64;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

static void FOG_FASTCALL raster_a8_span_composite_0xff_src_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  FOG_UNUSED(src);

  __m128i src0mm = _mm_setzero_si128();
  src0mm = _mm_cmpeq_epi8(src0mm, src0mm);

  BLIT_SSE2_8x16_INIT(dst, w);

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    dst[0] = 0xFF;

    dst += 1;
    src += 4;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    pix_store16a(dst, src0mm);

    dst += 16;
    src += 64;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

#define raster_a8_span_composite_argb32_src_sse2 raster_a8_span_composite_axxx32_src_sse2
#define raster_a8_span_composite_prgb32_src_sse2 raster_a8_span_composite_axxx32_src_sse2
#define raster_a8_span_composite_rgb32_src_sse2 raster_a8_span_composite_0xff_src_sse2
#define raster_a8_span_composite_rgb24_src_sse2 raster_a8_span_composite_0xff_src_sse2
#define raster_a8_span_composite_a8_src_sse2 convert_memcpy8_sse2

static void FOG_FASTCALL raster_a8_span_composite_axxx32_a8_src_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_8x16_INIT(dst, w);

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      uint32_t src0 = src[RGB32_AByte];

      if (msk0 == 0xFF)
      {
        dst[0] = (uint8_t)src0;
      }
      else
      {
        dst[0] = (uint8_t)singlelerp(dst[0], src0, msk0);
      }
    }

    dst += 1;
    src += 4;
    msk += 1;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    uint32_t msk0IsZero;
    uint32_t msk0IsFull;

    __m128i src0mm, src1mm;
    __m128i msk0mm;
    __m128i a0mm, a1mm;

    pix_load16u(msk0mm, msk);
    pix_analyze_mask_16B(msk0IsZero, msk0IsFull, msk0mm);

    if (msk0IsZero != 0xFFFF)
    {
      pix_load_and_unpack_axxx32_64B(src0mm, src1mm, src);

      if (msk0IsFull == 0xFFFF)
      {
        pix_pack_2x2W(src0mm, src0mm, src1mm);
        pix_store16a(dst, src0mm);
      }
      else
      {
        __m128i dst0mm, dst1mm;

        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_unpack_2x2W(a0mm, a1mm, msk0mm);
        pix_lerp_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 64;
    msk += 16;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

static void FOG_FASTCALL raster_a8_span_composite_0xff_a8_src_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  FOG_UNUSED(src);

  __m128i src0ff;
  __m128i src0mm;

  src0ff = _mm_setzero_si128();
  src0ff = _mm_cmpeq_epi8(src0ff, src0ff);
  src0mm = _mm_srli_epi16(src0ff, 8);

  BLIT_SSE2_8x16_INIT(dst, w);

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      if (msk0 == 0xFF)
      {
        dst[0] = 0xFF;
      }
      else
      {
        dst[0] = (uint8_t)singlelerp(dst[0], 0xFF, msk0);
      }
    }

    dst += 1;
    msk += 1;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    uint32_t msk0IsZero;
    uint32_t msk0IsFull;

    __m128i msk0mm;
    __m128i a0mm, a1mm;

    pix_load16u(msk0mm, msk);
    pix_analyze_mask_16B(msk0IsZero, msk0IsFull, msk0mm);

    if (msk0IsZero != 0xFFFF)
    {
      if (msk0IsFull == 0xFFFF)
      {
        pix_store16a(dst, src0ff);
      }
      else
      {
        __m128i dst0mm, dst1mm;

        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_unpack_2x2W(a0mm, a1mm, msk0mm);
        pix_lerp_2x2W(dst0mm, src0mm, a0mm, dst1mm, src0mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    msk += 16;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

#define raster_a8_span_composite_argb32_a8_src_sse2 raster_a8_span_composite_axxx32_a8_src_sse2
#define raster_a8_span_composite_prgb32_a8_src_sse2 raster_a8_span_composite_axxx32_a8_src_sse2
#define raster_a8_span_composite_rgb32_a8_src_sse2 raster_a8_span_composite_0xff_a8_src_sse2
#define raster_a8_span_composite_rgb24_a8_src_sse2 raster_a8_span_composite_0xff_a8_src_sse2

static void FOG_FASTCALL raster_a8_span_composite_a8_a8_src_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_8x16_INIT(dst, w);

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      uint32_t src0 = src[0];

      if (msk0 != 0xFF)
      {
        dst[0] = (uint8_t)src0;
      }
      else
      {
        dst[0] = (uint8_t)singlelerp(dst[0], src0, msk0);
      }
    }

    dst += 1;
    src += 1;
    msk += 1;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    uint32_t msk0IsZero;
    uint32_t msk0IsFull;

    __m128i src0mm, src1mm;
    __m128i msk0mm;
    __m128i a0mm, a1mm;

    pix_load16u(msk0mm, msk);
    pix_analyze_mask_16B(msk0IsZero, msk0IsFull, msk0mm);

    if (msk0IsZero != 0xFFFF)
    {
      pix_load16u(src0mm, src);

      if (msk0IsFull == 0xFFFF)
      {
        pix_store16a(dst, src0mm);
      }
      else
      {
        __m128i dst0mm, dst1mm;

        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_unpack_2x2W(a0mm, a1mm, msk0mm);
        pix_lerp_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
    msk += 16;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

static void FOG_FASTCALL raster_a8_span_composite_i8_a8_src_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const Rgba* srcPal = closure->srcPalette;

  for (sysint_t i = w; i; i--)
  {
    uint32_t msk0 = READ_8(msk);

    if (msk0 != 0x00)
    {
      dst[0] = singlelerp(dst[0], src[0], msk0);
    }

    dst += 1;
    src += 1;
    msk += 1;
  }
}

static void FOG_FASTCALL raster_a8_span_composite_axxx32_a8_const_src_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_a8_span_composite_axxx32_src_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_8x16_INIT(dst, w);

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    uint32_t src0 = src[RGB32_AByte];
    dst[0] = (uint8_t)singlelerp(dst[0], src0, msk0);

    dst += 1;
    src += 4;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);
    pix_load_and_unpack_axxx32_64B(src0mm, src1mm, src);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_multiply_2x2W(src0mm, src0mm, m0mm, src1mm, src1mm, m0mm);
    pix_multiply_2x2W(dst0mm, dst0mm, im0mm, dst1mm, dst1mm, im0mm);
    pix_adds_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 64;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

static void FOG_FASTCALL raster_a8_span_composite_0xff_a8_const_src_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_a8_span_composite_0xff_src_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);
  pix_pack_1x1W(m0mm, m0mm);

  uint32_t imsk0 = singleneg(msk0);

  BLIT_SSE2_8x16_INIT(dst, w);

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    dst[0] = (uint8_t)(singlemul(dst[0], imsk0) + msk0);

    dst += 1;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_multiply_2x2W(dst0mm, dst0mm, im0mm, dst1mm, dst1mm, im0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_adds_1x1B(dst0mm, dst0mm, m0mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

#define raster_a8_span_composite_argb32_a8_const_src_sse2 raster_a8_span_composite_axxx32_a8_const_src_sse2
#define raster_a8_span_composite_prgb32_a8_const_src_sse2 raster_a8_span_composite_axxx32_a8_const_src_sse2
#define raster_a8_span_composite_rgb32_a8_const_src_sse2 raster_a8_span_composite_0xff_a8_const_src_sse2
#define raster_a8_span_composite_rgb24_a8_const_src_sse2 raster_a8_span_composite_0xff_a8_const_src_sse2

static void FOG_FASTCALL raster_a8_span_composite_a8_a8_const_src_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_a8_span_composite_a8_src_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_8x16_INIT(dst, w);

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    uint32_t src0 = src[0];
    dst[0] = (uint8_t)singlelerp(dst[0], src0, msk0);

    dst += 1;
    src += 1;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_multiply_2x2W(dst0mm, dst0mm, im0mm, dst1mm, dst1mm, im0mm);
    pix_multiply_2x2W(src0mm, src0mm, m0mm, src1mm, src1mm, m0mm);
    pix_adds_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

static void FOG_FASTCALL raster_a8_span_composite_i8_a8_const_src_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  const Rgba* srcPal = closure->srcPalette;

  if (msk0 == 0xFF)
  {
    functionMap->raster[Image::FormatA8][CompositeSrc].span_composite[Image::FormatI8](dst, src, w, closure);
    return;
  }

  for (sysint_t i = w; i; i--)
  {
    dst[0] = singlelerp(dst[0], src[0], msk0);

    dst += 1;
    src += 1;
  }
}

// ============================================================================
// [Fog::Raster - Raster - SrcOver - Argb32]
// ============================================================================

static void FOG_FASTCALL raster_argb32_pixel_srcover_sse2(
  uint8_t* dst, const Solid* src, const Closure* closure)
{
  uint32_t src0 = src->rgbp;

  if (isAlpha0xFF(src0))
  {
    ((uint32_t*)dst)[0] = src0;
  }
  else
  {
    __m128i src0mm;
    __m128i dst0mm;

    pix_load4(src0mm, &src->rgbp);
    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_premultiply_1x1W(dst0mm, dst0mm);
    pix_over_1x1W(dst0mm, src0mm);
    pix_demultiply_1x1W(dst0mm, dst0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);
  }
}

static void FOG_FASTCALL raster_argb32_pixel_a8_srcover_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, const Closure* closure)
{
  uint32_t src0 = src->rgba;

  if (isAlpha0xFF(src0) & (msk0 == 0xFF))
  {
    ((uint32_t*)dst)[0] = src0;
  }
  else
  {
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_load4(src0mm, &src->rgbp);
    pix_load4(dst0mm, dst);
    pix_expand_mask_1x1W(a0mm, msk0);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_multiply_1x1W(src0mm, src0mm, a0mm);
    pix_premultiply_1x1W(dst0mm, dst0mm);
    pix_over_1x1W(dst0mm, src0mm);
    pix_demultiply_1x1W(dst0mm, dst0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);
  }
}

static void FOG_FASTCALL raster_argb32_span_solid_srcover_sse2(
  uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure)
{
  uint32_t src0 = src->rgba;
  __m128i src0mm;

  pix_load4(src0mm, &src->rgbp);
  pix_expand_pixel_1x4B(src0mm, src0mm);

  BLIT_SSE2_32x4_INIT(dst, w)

  if (isAlpha0xFF(src0))
  {
    BLIT_SSE2_32x4_SMALL_BEGIN(blt_opaque)
      ((uint32_t*)dst)[0] = src0;
      dst += 4;
    BLIT_SSE2_32x4_SMALL_END(blt_opaque)

    while (w >= 4)
    {
      pix_store16a(dst, src0mm);
      pix_store16a(dst + 16, src0mm);
      pix_store16a(dst + 32, src0mm);
      pix_store16a(dst + 48, src0mm);

      dst += 64;
      w -= 4;
    }
    switch (w & 3)
    {
      case 3: pix_store16a(dst, src0mm); dst += 16;
      case 2: pix_store16a(dst, src0mm); dst += 16;
      case 1: pix_store16a(dst, src0mm); dst += 16;
    }

    if ((_i = _j)) { w = 0; goto blt_opaque; }
    return;
  }
  else
  {
    __m128i ia0mm;
    pix_unpack_1x2W(src0mm, src0mm);
    pix_expand_alpha_1x2W(ia0mm, src0mm);
    pix_negate_1x1W(ia0mm, ia0mm);

    BLIT_SSE2_32x4_SMALL_BEGIN(blt_trans)
      __m128i dst0mm;

      pix_load4(dst0mm, dst);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_premultiply_1x1W(dst0mm, dst0mm);
      pix_over_ialpha_1x1W(dst0mm, src0mm, ia0mm);
      pix_demultiply_1x1W(dst0mm, dst0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);

      dst += 4;
    BLIT_SSE2_32x4_SMALL_END(blt_trans)

    BLIT_SSE2_32x4_LARGE_BEGIN(blt_trans)
      __m128i dst0mm;
      __m128i dst1mm;

      pix_load16a(dst0mm, dst);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_premultiply_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_over_ialpha_2x2W(dst0mm, src0mm, ia0mm, dst1mm, src0mm, ia0mm);
      pix_demultiply_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);

      dst += 16;
    BLIT_SSE2_32x4_LARGE_END(blt_trans)
  }
}

static void FOG_FASTCALL raster_argb32_span_solid_a8_srcover_sse2(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  uint32_t src0 = src->rgba;

  BLIT_SSE2_32x4_INIT(dst, w);

  __m128i src0orig;
  __m128i src0unpacked;

  pix_load4(src0orig, &src->rgbp);
  pix_expand_pixel_1x4B(src0orig, src0orig);
  pix_unpack_1x2W(src0unpacked, src0orig);

  if (isAlpha0xFF(src0))
  {
    BLIT_SSE2_32x4_SMALL_BEGIN(blt_opaque)
      __m128i dst0mm;
      __m128i a0mm;

      uint32_t msk0 = READ_8(msk);
      if (msk0 != 0x00)
      {
        if (msk0 == 0xFF)
        {
          pix_store4(dst, src0orig);
        }
        else
        {
          pix_load4(dst0mm, dst);
          pix_unpack_1x1W(dst0mm, dst0mm);
          pix_expand_mask_1x1W(a0mm, msk0);
          pix_lerp_1x1W(dst0mm, src0unpacked, a0mm);
          pix_pack_1x1W(dst0mm, dst0mm);
          pix_store4(dst, dst0mm);
        }
      }

      dst += 4;
      msk += 1;
    BLIT_SSE2_32x4_SMALL_END(blt_opaque)

    BLIT_SSE2_32x4_LARGE_BEGIN(blt_opaque)
      __m128i dst0mm, dst1mm;
      __m128i a0mm, a1mm;

      uint32_t msk0 = READ_32(msk);
      if (msk0 != 0x00000000)
      {
        if (msk0 == 0xFFFFFFFF)
        {
          pix_store16a(dst, src0orig);
        }
        else
        {
          pix_load16a(dst0mm, dst);
          pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
          pix_expand_mask_2x2W(a0mm, a1mm, msk0);
          pix_lerp_2x2W(dst0mm, src0unpacked, a0mm, dst1mm, src0unpacked, a1mm);
          pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
          pix_store16a(dst, dst0mm);
        }
      }

      dst += 16;
      msk += 4;
    BLIT_SSE2_32x4_LARGE_END(blt_opaque)
  }
  else
  {
    BLIT_SSE2_32x4_SMALL_BEGIN(blt_trans)
      __m128i dst0mm;
      __m128i src0mm;
      __m128i a0mm;

      uint32_t msk0 = READ_8(msk);
      if (msk0 != 0x00)
      {
        pix_load4(dst0mm, dst);
        pix_unpack_1x1W(dst0mm, dst0mm);
        pix_premultiply_1x1W(dst0mm, dst0mm);
        pix_expand_mask_1x1W(a0mm, msk0);
        pix_multiply_1x1W(src0mm, src0unpacked, a0mm);
        pix_expand_alpha_1x1W(a0mm, src0mm);
        pix_over_1x1W(dst0mm, src0mm, a0mm);
        pix_demultiply_1x1W(dst0mm, dst0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }

      dst += 4;
      msk += 1;
    BLIT_SSE2_32x4_SMALL_END(blt_trans)

    BLIT_SSE2_32x4_LARGE_BEGIN(blt_trans)
      __m128i dst0mm, dst1mm;
      __m128i src0mm, src1mm;
      __m128i a0mm, a1mm;

      uint32_t msk0 = READ_32(msk);
      if (msk0 != 0x00000000)
      {
        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_premultiply_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_multiply_2x2W(src0mm, src0unpacked, a0mm, src1mm, src0unpacked, a1mm);
        pix_expand_alpha_2x2W(a0mm, src0mm, a1mm, src1mm);
        pix_over_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_demultiply_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }

      dst += 16;
      msk += 4;
    BLIT_SSE2_32x4_LARGE_END(blt_trans)
  }
}

static void FOG_FASTCALL raster_argb32_span_solid_a8_const_srcover_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (msk0 == 0xFF)
  {
    raster_argb32_span_solid_srcover_sse2(dst, src, w, closure);
    return;
  }

  __m128i src0mm;
  __m128i m0mm;

  pix_load4(src0mm, &src->rgbp);
  pix_unpack_1x1W(src0mm, src0mm);
  pix_expand_pixel_1x2W(src0mm, src0mm);

  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_multiply_1x1W(src0mm, src0mm, m0mm);
  pix_negate_1x1W(m0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;

    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_premultiply_1x1W(dst0mm, dst0mm);
    pix_multiply_1x1W(dst0mm, dst0mm, m0mm);
    pix_adds_1x1W(dst0mm, dst0mm, src0mm);
    pix_demultiply_1x1W(dst0mm, dst0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_premultiply_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
    pix_multiply_2x2W(dst0mm, dst0mm, m0mm, dst1mm, dst1mm, m0mm);
    pix_adds_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src0mm);
    pix_demultiply_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

// ============================================================================
// [Fog::Raster - Raster - SrcOver - Prgb32]
// ============================================================================

static void FOG_FASTCALL raster_prgb32_pixel_srcover_sse2(
  uint8_t* dst, const Solid* src, const Closure* closure)
{
  uint32_t src0 = src->rgbp;

  if (isAlpha0xFF(src0))
  {
    ((uint32_t*)dst)[0] = src0;
  }
  else
  {
    __m128i src0mm;
    __m128i dst0mm;

    pix_unpack_1x1W(src0mm, src0);
    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_over_1x1W(dst0mm, src0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);
  }
}

static void FOG_FASTCALL raster_prgb32_pixel_a8_srcover_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, const Closure* closure)
{
  uint32_t src0 = src->rgbp;

  if (isAlpha0xFF(src0) & (msk0 == 0xFF))
  {
    ((uint32_t*)dst)[0] = src0;
  }
  else
  {
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_unpack_1x1W(src0mm, src0);
    pix_unpack_1x1W(a0mm, msk0);
    pix_expand_alpha_rev_1x1W(a0mm, a0mm);
    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_multiply_1x1W(src0mm, src0mm, a0mm);
    pix_over_1x1W(dst0mm, src0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);
  }
}

static void FOG_FASTCALL raster_prgb32_span_solid_srcover_sse2(
  uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure)
{
  __m128i src0mm;

  pix_load4(src0mm, &src->rgbp);
  pix_expand_pixel_1x4B(src0mm, src0mm);

  BLIT_SSE2_32x4_INIT(dst, w)

  if (isAlpha0xFF(src->rgbp))
  {
    BLIT_SSE2_32x4_SMALL_BEGIN(blt_opaque)
      pix_store4(dst, src0mm);
      dst += 4;
    BLIT_SSE2_32x4_SMALL_END(blt_opaque)

    while (w >= 4)
    {
      pix_store16a(dst, src0mm);
      pix_store16a(dst + 16, src0mm);
      pix_store16a(dst + 32, src0mm);
      pix_store16a(dst + 48, src0mm);

      dst += 64;
      w -= 4;
    }
    switch (w & 3)
    {
      case 3: pix_store16a(dst, src0mm); dst += 16;
      case 2: pix_store16a(dst, src0mm); dst += 16;
      case 1: pix_store16a(dst, src0mm); dst += 16;
    }

    if ((_i = _j)) { w = 0; goto blt_opaque; }
    return;
  }
  else
  {
    __m128i ia0mm;
    pix_unpack_1x2W(src0mm, src0mm);
    pix_expand_alpha_1x2W(ia0mm, src0mm);
    pix_negate_1x1W(ia0mm, ia0mm);

    BLIT_SSE2_32x4_SMALL_BEGIN(blt_trans)
      __m128i dst0mm;

      pix_load4(dst0mm, dst);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_over_ialpha_1x1W(dst0mm, src0mm, ia0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);

      dst += 4;
    BLIT_SSE2_32x4_SMALL_END(blt_trans)

    BLIT_SSE2_32x4_LARGE_BEGIN(blt_trans)
      __m128i dst0mm;
      __m128i dst1mm;

      pix_load16a(dst0mm, dst);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_over_ialpha_2x2W(dst0mm, src0mm, ia0mm, dst1mm, src0mm, ia0mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);

      dst += 16;
    BLIT_SSE2_32x4_LARGE_END(blt_trans)
  }
}

static void FOG_FASTCALL raster_prgb32_span_solid_a8_srcover_sse2(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  __m128i src0orig;
  __m128i src0unpacked;

  pix_load4(src0orig, &src->rgbp);
  pix_expand_pixel_1x4B(src0orig, src0orig);
  pix_unpack_1x2W(src0unpacked, src0orig);

  BLIT_SSE2_32x4_INIT(dst, w);

  if (isAlpha0xFF(src->rgbp))
  {
    BLIT_SSE2_32x4_SMALL_BEGIN(blt_opaque)
      __m128i dst0mm;
      __m128i src0mm;
      __m128i a0mm;

      uint32_t msk0 = READ_8(msk);
      if (msk0 != 0x00)
      {
        if (msk0 == 0xFF)
        {
          pix_store4(dst, src0orig);
        }
        else
        {
          pix_load4(dst0mm, dst);
          pix_unpack_1x1W(dst0mm, dst0mm);
          pix_expand_alpha_rev_1x1W(a0mm, _mm_cvtsi32_si128(msk0));
          pix_expand_pixel_1x2W(a0mm, a0mm);
          pix_multiply_1x1W(src0mm, src0unpacked, a0mm);
          pix_over_1x1W(dst0mm, src0mm, a0mm);
          pix_pack_1x1W(dst0mm, dst0mm);
          pix_store4(dst, dst0mm);
        }
      }

      dst += 4;
      msk += 1;
    BLIT_SSE2_32x4_SMALL_END(blt_opaque)

    BLIT_SSE2_32x4_LARGE_BEGIN(blt_opaque)
      __m128i dst0mm, dst1mm;
      __m128i src0mm, src1mm;
      __m128i a0mm, a1mm;

      uint32_t msk0 = READ_32(msk);
      if (msk0 != 0x00000000)
      {
        if (msk0 == 0xFFFFFFFF)
        {
          pix_store16a(dst, src0orig);
        }
        else
        {
          pix_load16a(dst0mm, dst);
          pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
          pix_expand_mask_2x2W(a0mm, a1mm, msk0);
          pix_multiply_2x2W(src0mm, src0unpacked, a0mm, src1mm, src0unpacked, a1mm);
          pix_over_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
          pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
          pix_store16a(dst, dst0mm);
        }
      }

      dst += 16;
      msk += 4;
    BLIT_SSE2_32x4_LARGE_END(blt_opaque)
  }
  else
  {
    BLIT_SSE2_32x4_SMALL_BEGIN(blt_trans)
      __m128i dst0mm;
      __m128i src0mm;
      __m128i a0mm;

      uint32_t msk0 = READ_8(msk);
      if (msk0 != 0x00)
      {
        pix_load4(dst0mm, dst);
        pix_unpack_1x1W(dst0mm, dst0mm);
        pix_expand_alpha_rev_1x1W(a0mm, _mm_cvtsi32_si128(msk0));
        pix_expand_pixel_1x2W(a0mm, a0mm);
        pix_multiply_1x1W(src0mm, src0unpacked, a0mm);
        pix_expand_alpha_1x1W(a0mm, src0mm);
        pix_over_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }

      dst += 4;
      msk += 1;
    BLIT_SSE2_32x4_SMALL_END(blt_trans)

    BLIT_SSE2_32x4_LARGE_BEGIN(blt_trans)
      __m128i dst0mm, dst1mm;
      __m128i src0mm, src1mm;
      __m128i a0mm, a1mm;

      uint32_t msk0 = READ_32(msk);
      if (msk0 != 0x00000000)
      {
        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_multiply_2x2W(src0mm, src0unpacked, a0mm, src1mm, src0unpacked, a1mm);
        pix_expand_alpha_2x2W(a0mm, src0mm, a1mm, src1mm);
        pix_over_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }

      dst += 16;
      msk += 4;
    BLIT_SSE2_32x4_LARGE_END(blt_trans)
  }
}

static void FOG_FASTCALL raster_prgb32_span_solid_a8_const_srcover_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (msk0 == 0xFF)
  {
    raster_prgb32_span_solid_srcover_sse2(dst, src, w, closure);
    return;
  }

  __m128i src0mm;
  __m128i m0mm;

  pix_load4(src0mm, &src->rgbp);
  pix_unpack_1x2W(src0mm, src0mm);
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_multiply_1x1W(src0mm, src0mm, m0mm);
  pix_expand_alpha_1x1W(m0mm, src0mm);
  pix_negate_1x1W(m0mm, m0mm);

  pix_pack_1x1W(src0mm, src0mm);
  pix_expand_pixel_1x4B(src0mm, src0mm);
  pix_expand_pixel_1x2W(m0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;

    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_multiply_1x1W(dst0mm, dst0mm, m0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_adds_1x1B(dst0mm, dst0mm, src0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_multiply_2x2W(dst0mm, dst0mm, m0mm, dst1mm, dst1mm, m0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_adds_1x4B(dst0mm, dst0mm, src0mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}
static void FOG_FASTCALL raster_prgb32_span_composite_argb32_srcover_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;
    __m128i a0mm;

    uint32_t src0 = READ_32(src);
    if (!isAlpha0x00(src0))
    {
      pix_load4(dst0mm, dst);
      pix_unpack_1x1W(src0mm, src0);
      pix_unpack_1x1W(dst0mm, dst0mm);

      pix_expand_alpha_1x1W(a0mm, src0mm);
      pix_fill_alpha_1x1W(src0mm);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);

      pix_over_1x1W(dst0mm, src0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);

      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    pix_load16u(src0mm, src);
    uint32_t src0a = pix_pack_alpha_to_uint32(src0mm);
    if (src0a != 0x00000000)
    {
      if (src0a != 0xFFFFFFFF)
      {
        pix_load16a(dst0mm, dst);

        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_alpha_2x2W(a0mm, src0mm, a1mm, src1mm);
        pix_fill_alpha_2x2W(src0mm, src1mm);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);

        pix_over_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);

        pix_store16a(dst, dst0mm);
      }
      else
      {
        pix_store16a(dst, src0mm);
      }
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_a8_srcover_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t src0 = READ_32(src);
    uint32_t msk0 = READ_8(msk);

    if ((!isAlpha0x00(src0)) & (msk0 != 0x00))
    {
      msk0 = singlemul(src0 >> 24, msk0);

      pix_load4(dst0mm, dst);
      pix_unpack_1x1W(src0mm, src0);
      pix_unpack_1x1W(dst0mm, dst0mm);

      pix_expand_mask_1x1W(a0mm, msk0);
      pix_fill_alpha_1x1W(src0mm);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_over_1x1W(dst0mm, src0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);

      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16u(src0mm, src);
    uint32_t src0a = pix_pack_alpha_to_uint32(src0mm);
    uint32_t msk0 = ((uint32_t*)msk)[0];

    if ((src0a != 0x00000000) & (msk0 != 0x00000000))
    {
      if ((src0a == 0xFFFFFFFF) & (msk0 == 0xFFFFFFFF))
      {
        pix_store16a(dst, src0mm);
      }
      else
      {
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_expand_mask_2x2W(dst0mm, dst1mm, msk0);
        pix_expand_alpha_2x2W(a0mm, src0mm, a1mm, src1mm);
        pix_multiply_2x2W(a0mm, a0mm, dst0mm, a1mm, a1mm, dst1mm);

        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_fill_alpha_2x2W(src0mm, src1mm);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
        pix_over_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);

        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_srcover_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t src0 = READ_32(src);

    if (src0)
    {
      pix_load4(dst0mm, dst);
      pix_unpack_1x1W(src0mm, src0);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_expand_alpha_1x1W(a0mm, src0mm);
      pix_over_1x1W(dst0mm, src0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);

      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16u(src0mm, src);
    uint32_t src0a = pix_pack_alpha_to_uint32(src0mm);

    if (src0a != 0x00000000)
    {
      if (src0a != 0xFFFFFFFF)
      {
        pix_load16a(dst0mm, dst);

        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_alpha_2x2W(a0mm, src0mm, a1mm, src1mm);
        pix_over_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);

        pix_store16a(dst, dst0mm);
      }
      else
      {
        pix_store16a(dst, src0mm);
      }
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

#define raster_prgb32_span_composite_rgb32_srcover_sse2 convert_axxx32_from_xxxx32_sse2
#define raster_prgb32_span_composite_rgb24_srcover_sse2 convert_rgb32_from_rgb24_sse2

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_a8_srcover_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t src0 = READ_32(src);
    uint32_t msk0 = READ_8(msk);

    if ((src0 != 0x00000000) & (msk0 != 0x00))
    {
      pix_load4(dst0mm, dst);
      pix_unpack_1x1W(src0mm, src0);
      pix_unpack_1x1W(dst0mm, dst0mm);

      pix_expand_mask_1x1W(a0mm, msk0);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_expand_alpha_1x1W(a0mm, src0mm);
      pix_over_1x1W(dst0mm, src0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);

      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16u(src0mm, src);
    uint32_t src0a = pix_pack_alpha_to_uint32(src0mm);
    uint32_t msk0 = ((uint32_t*)msk)[0];

    if ((src0a != 0x00000000) & (msk0 != 0x00000000))
    {
      if ((src0a == 0xFFFFFFFF) & (msk0 == 0xFFFFFFFF))
      {
        pix_store16a(dst, src0mm);
      }
      else
      {
        pix_load16a(dst0mm, dst);

        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
        pix_expand_alpha_2x2W(a0mm, src0mm, a1mm, src1mm);
        pix_over_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);

        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb32_a8_srcover_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);

    if (msk0 != 0x00)
    {
      pix_load4(src0mm, src);

      if (msk0 != 0xFF)
      {
        pix_load4(dst0mm, dst);
        pix_unpack_1x1W(dst0mm, dst0mm);
        pix_unpack_1x1W(src0mm, src0mm);

        pix_expand_mask_1x1W(a0mm, msk0);
        pix_fill_alpha_1x1W(src0mm);
        pix_multiply_1x1W(src0mm, src0mm, a0mm);
        pix_over_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(src0mm, dst0mm);
      }

      pix_store4(dst, src0mm);
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = ((uint32_t*)msk)[0];

    if (msk0 != 0x00000000)
    {
      pix_load16u(src0mm, src);
      if (msk0 != 0xFFFFFFFF)
      {
        pix_load16a(dst0mm, dst);

        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_fill_alpha_2x2W(src0mm, src1mm);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
        pix_over_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(src0mm, dst0mm, dst1mm);
      }
      pix_store16a(dst, src0mm);
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb24_a8_srcover_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);

    if (msk0 != 0x00)
    {
      uint32_t src0 = PixFmt_RGB24::fetch(src);

      if (msk0 != 0xFF)
      {
        pix_load4(dst0mm, dst);
        pix_unpack_1x1W(dst0mm, dst0mm);
        pix_unpack_1x1W(src0mm, src0);

        pix_expand_mask_1x1W(a0mm, msk0);
        pix_fill_alpha_1x1W(src0mm);
        pix_multiply_1x1W(src0mm, src0mm, a0mm);
        pix_over_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
      else
      {
        ((uint32_t*)dst)[0] = src0;
      }
    }

    dst += 4;
    src += 3;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = ((uint32_t*)msk)[0];

    if (msk0 != 0x00000000)
    {
      pix_fetch_rgb24_2x2W(src0mm, src1mm, src);
      if (msk0 != 0xFFFFFFFF)
      {
        pix_load16a(dst0mm, dst);

        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_fill_alpha_2x2W(src0mm, src1mm);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
        pix_over_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
      else
      {
        pix_pack_2x2W(dst0mm, src0mm, src1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 12;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_a8_const_srcover_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_prgb32_span_composite_argb32_srcover_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t src0 = READ_32(src);

    if (!isAlpha0x00(src0))
    {
      if (isAlpha0xFF(src0))
      {
        pix_load4(dst0mm, dst);
        pix_unpack_1x1W(src0mm, src0);
        pix_unpack_1x1W(dst0mm, dst0mm);

        pix_multiply_1x1W(src0mm, src0mm, m0mm);
        pix_over_ialpha_1x1W(dst0mm, src0mm, im0mm);
        pix_pack_1x1W(dst0mm, dst0mm);

        pix_store4(dst, dst0mm);
      }
      else
      {
        uint32_t msk0 = singlemul(src0 >> 24, msk0);

        pix_load4(dst0mm, dst);
        pix_unpack_1x1W(src0mm, src0);
        pix_unpack_1x1W(dst0mm, dst0mm);

        pix_expand_mask_1x1W(a0mm, msk0);
        pix_fill_alpha_1x1W(src0mm);
        pix_multiply_1x1W(src0mm, src0mm, a0mm);
        pix_over_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);

        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16u(src0mm, src);
    uint32_t src0a = pix_pack_alpha_to_uint32(src0mm);

    if (src0a != 0x00000000)
    {
      if (src0a == 0xFFFFFFFF)
      {
        pix_load16a(dst0mm, dst);

        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_multiply_2x2W(src0mm, src0mm, m0mm, src1mm, src1mm, m0mm);
        pix_over_ialpha_2x2W(dst0mm, src0mm, im0mm, dst1mm, src1mm, im0mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);

        pix_store16a(dst, dst0mm);
      }
      else
      {
        pix_load16a(dst0mm, dst);

        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_expand_alpha_2x2W(a0mm, src0mm, a1mm, src1mm);
        pix_multiply_2x2W(a0mm, a0mm, m0mm, a1mm, a1mm, m0mm);

        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_fill_alpha_2x2W(src0mm, src1mm);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
        pix_over_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);

        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_a8_const_srcover_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_prgb32_span_composite_argb32_srcover_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t src0 = READ_32(src);

    if (!isAlpha0x00(src0))
    {
      if (isAlpha0xFF(src0))
      {
        pix_load4(dst0mm, dst);
        pix_unpack_1x1W(src0mm, src0);
        pix_unpack_1x1W(dst0mm, dst0mm);

        pix_multiply_1x1W(src0mm, src0mm, m0mm);
        pix_over_ialpha_1x1W(dst0mm, src0mm, im0mm);
        pix_pack_1x1W(dst0mm, dst0mm);

        pix_store4(dst, dst0mm);
      }
      else
      {
        pix_load4(dst0mm, dst);
        pix_unpack_1x1W(src0mm, src0);
        pix_unpack_1x1W(dst0mm, dst0mm);
        pix_multiply_1x1W(src0mm, src0mm, m0mm);
        pix_expand_alpha_1x1W(a0mm, src0mm);
        pix_over_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);

        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16u(src0mm, src);
    uint32_t src0a = pix_pack_alpha_to_uint32(src0mm);

    if (src0a != 0x00000000)
    {
      if (src0a == 0xFFFFFFFF)
      {
        pix_load16a(dst0mm, dst);

        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_multiply_2x2W(src0mm, src0mm, m0mm, src1mm, src1mm, m0mm);
        pix_over_ialpha_2x2W(dst0mm, src0mm, im0mm, dst1mm, src1mm, im0mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);

        pix_store16a(dst, dst0mm);
      }
      else
      {
        pix_load16a(dst0mm, dst);

        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);

        pix_multiply_2x2W(src0mm, src0mm, m0mm, src1mm, src1mm, m0mm);
        pix_expand_alpha_2x2W(a0mm, src0mm, a1mm, src1mm);
        pix_over_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);

        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb32_a8_const_srcover_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_prgb32_span_composite_rgb32_srcover_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm, im0mm;
  __m128i amask = Mask_FF000000FF000000_FF000000FF000000;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    uint32_t src0 = READ_32(src) | 0xFF000000;

    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(src0mm, src0);
    pix_unpack_1x1W(dst0mm, dst0mm);

    pix_multiply_1x1W(src0mm, src0mm, m0mm);
    pix_over_ialpha_1x1W(dst0mm, src0mm, im0mm);
    pix_pack_1x1W(dst0mm, dst0mm);

    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16u(src0mm, src);
    pix_load16a(dst0mm, dst);
    src0mm = _mm_or_si128(src0mm, amask);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_multiply_2x2W(src0mm, src0mm, m0mm, src1mm, src1mm, m0mm);
    pix_over_ialpha_2x2W(dst0mm, src0mm, im0mm, dst1mm, src1mm, im0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);

    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb24_a8_const_srcover_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    convert_rgb32_from_rgb24_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    uint32_t src0 = PixFmt_RGB24::fetch(src) | 0xFF000000;

    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(src0mm, src0);
    pix_unpack_1x1W(dst0mm, dst0mm);

    pix_multiply_1x1W(src0mm, src0mm, m0mm);
    pix_over_ialpha_1x1W(dst0mm, src0mm, im0mm);
    pix_pack_1x1W(dst0mm, dst0mm);

    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_fetch_rgb24_2x2W(src0mm, src1mm, src);
    pix_multiply_2x2W(src0mm, src0mm, m0mm, src1mm, src1mm, m0mm);
    pix_over_ialpha_2x2W(dst0mm, src0mm, im0mm, dst1mm, src1mm, im0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);

    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

// ============================================================================
// [Fog::Raster - Raster - SrcOver - Rgb32]
// ============================================================================

static void FOG_FASTCALL raster_rgb32_pixel_srcover_sse2(
  uint8_t* dst, const Solid* src, const Closure* closure)
{
  uint32_t src0 = src->rgbp;

  if (isAlpha0xFF(src0))
  {
    ((uint32_t*)dst)[0] = src0;
  }
  else
  {
    __m128i src0mm;
    __m128i dst0mm;

    pix_load4(src0mm, &src->rgbp);
    pix_load4(dst0mm, dst);
    dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_over_1x1W(dst0mm, src0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);
  }
}

static void FOG_FASTCALL raster_rgb32_pixel_a8_srcover_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, const Closure* closure)
{
  uint32_t src0 = src->rgbp;

  if (isAlpha0xFF(src0) & (msk0 == 0xFF))
  {
    ((uint32_t*)dst)[0] = src0;
  }
  else
  {
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_load4(src0mm, &src->rgbp);
    pix_load4(dst0mm, dst);
    pix_expand_mask_1x1W(a0mm, msk0);
    dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_multiply_1x1W(src0mm, src0mm, a0mm);
    pix_over_1x1W(dst0mm, src0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);
  }
}

static void FOG_FASTCALL raster_rgb32_span_solid_srcover_sse2(
  uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure)
{
  uint32_t src0 = src->rgbp;
  __m128i src0mm;

  pix_load4(src0mm, &src->rgbp);
  pix_expand_pixel_1x4B(src0mm, src0mm);

  BLIT_SSE2_32x4_INIT(dst, w)

  if (isAlpha0xFF(src0))
  {
    BLIT_SSE2_32x4_SMALL_BEGIN(blt_opaque)
      ((uint32_t*)dst)[0] = src0;
      dst += 4;
    BLIT_SSE2_32x4_SMALL_END(blt_opaque)

    while (w >= 4)
    {
      pix_store16a(dst, src0mm);
      pix_store16a(dst + 16, src0mm);
      pix_store16a(dst + 32, src0mm);
      pix_store16a(dst + 48, src0mm);

      dst += 64;
      w -= 4;
    }
    switch (w & 3)
    {
      case 3: pix_store16a(dst, src0mm); dst += 16;
      case 2: pix_store16a(dst, src0mm); dst += 16;
      case 1: pix_store16a(dst, src0mm); dst += 16;
    }

    if ((_i = _j)) { w = 0; goto blt_opaque; }
    return;
  }
  else
  {
    __m128i ia0mm;
    pix_unpack_1x2W(src0mm, src0mm);
    pix_expand_alpha_1x2W(ia0mm, src0mm);
    pix_negate_1x1W(ia0mm, ia0mm);

    BLIT_SSE2_32x4_SMALL_BEGIN(blt_trans)
      __m128i dst0mm;

      pix_load4(dst0mm, dst);
      dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_over_ialpha_1x1W(dst0mm, src0mm, ia0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);

      dst += 4;
    BLIT_SSE2_32x4_SMALL_END(blt_trans)

    BLIT_SSE2_32x4_LARGE_BEGIN(blt_trans)
      __m128i dst0mm;
      __m128i dst1mm;

      pix_load16a(dst0mm, dst);
      dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_over_ialpha_2x2W(dst0mm, src0mm, ia0mm, dst1mm, src0mm, ia0mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);

      dst += 16;
    BLIT_SSE2_32x4_LARGE_END(blt_trans)
  }
}

static void FOG_FASTCALL raster_rgb32_span_solid_a8_srcover_sse2(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  uint32_t src0 = src->rgbp;
  __m128i src0orig;
  __m128i src0unpacked;

  pix_load4(src0orig, &src->rgbp);
  pix_expand_pixel_1x4B(src0orig, src0orig);
  pix_unpack_1x2W(src0unpacked, src0orig);

  BLIT_SSE2_32x4_INIT(dst, w);

  if (isAlpha0xFF(src0))
  {
    BLIT_SSE2_32x4_SMALL_BEGIN(blt_opaque)
      __m128i dst0mm;
      __m128i src0mm;
      __m128i a0mm;

      uint32_t msk0 = READ_8(msk);
      if (msk0 != 0x00)
      {
        if (msk0 == 0xFF)
        {
          ((uint32_t*)dst)[0] = src0;
        }
        else
        {
          pix_load4(dst0mm, dst);
          dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
          pix_unpack_1x1W(dst0mm, dst0mm);
          pix_expand_mask_1x1W(a0mm, msk0);
          pix_expand_pixel_1x2W(a0mm, a0mm);
          pix_multiply_1x1W(src0mm, src0unpacked, a0mm);
          pix_over_1x1W(dst0mm, src0mm, a0mm);
          pix_pack_1x1W(dst0mm, dst0mm);
          pix_store4(dst, dst0mm);
        }
      }

      dst += 4;
      msk += 1;
    BLIT_SSE2_32x4_SMALL_END(blt_opaque)

    BLIT_SSE2_32x4_LARGE_BEGIN(blt_opaque)
      __m128i dst0mm, dst1mm;
      __m128i src0mm, src1mm;
      __m128i a0mm, a1mm;

      uint32_t msk0 = READ_32(msk);
      if (msk0 != 0x00000000)
      {
        if (msk0 == 0xFFFFFFFF)
        {
          pix_store16a(dst, src0orig);
        }
        else
        {
          pix_load16a(dst0mm, dst);
          dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
          pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);

          pix_unpack_1x1W(a0mm, _mm_cvtsi32_si128(msk0));
          a0mm = _mm_unpacklo_epi16(a0mm, a0mm);

          a1mm = _mm_shuffle_epi32(a0mm, _MM_SHUFFLE(3, 3, 2, 2));
          a0mm = _mm_shuffle_epi32(a0mm, _MM_SHUFFLE(1, 1, 0, 0));

          pix_multiply_2x2W(src0mm, src0unpacked, a0mm, src1mm, src0unpacked, a1mm);
          pix_over_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
          pix_pack_2x2W(dst0mm, dst0mm, dst1mm);

          pix_store16a(dst, dst0mm);
        }
      }

      dst += 16;
      msk += 4;
    BLIT_SSE2_32x4_LARGE_END(blt_opaque)
  }
  else
  {
    BLIT_SSE2_32x4_SMALL_BEGIN(blt_trans)
      __m128i dst0mm;
      __m128i src0mm;
      __m128i a0mm;

      uint32_t msk0 = READ_8(msk);
      if (msk0 != 0x00)
      {
        pix_load4(dst0mm, dst);
        dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
        pix_unpack_1x1W(dst0mm, dst0mm);
        pix_expand_mask_1x1W(a0mm, msk0);
        pix_expand_pixel_1x2W(a0mm, a0mm);
        pix_multiply_1x1W(src0mm, src0unpacked, a0mm);
        pix_expand_alpha_1x1W(a0mm, src0mm);
        pix_over_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }

      dst += 4;
      msk += 1;
    BLIT_SSE2_32x4_SMALL_END(blt_trans)

    BLIT_SSE2_32x4_LARGE_BEGIN(blt_trans)
      __m128i dst0mm, dst1mm;
      __m128i src0mm, src1mm;
      __m128i a0mm, a1mm;

      uint32_t msk0 = READ_32(msk);
      if (msk0 != 0x00000000)
      {
        pix_load16a(dst0mm, dst);
        dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);

        pix_unpack_1x1W(a0mm, _mm_cvtsi32_si128(msk0));
        a0mm = _mm_unpacklo_epi16(a0mm, a0mm);

        a1mm = _mm_shuffle_epi32(a0mm, _MM_SHUFFLE(3, 3, 2, 2));
        a0mm = _mm_shuffle_epi32(a0mm, _MM_SHUFFLE(1, 1, 0, 0));

        pix_multiply_2x2W(src0mm, src0unpacked, a0mm, src1mm, src0unpacked, a1mm);
        pix_expand_alpha_2x2W(a0mm, src0mm, a1mm, src1mm);
        pix_over_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);

        pix_store16a(dst, dst0mm);
      }

      dst += 16;
      msk += 4;
    BLIT_SSE2_32x4_LARGE_END(blt_trans)
  }
}

static void FOG_FASTCALL raster_rgb32_span_solid_a8_const_srcover_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  uint32_t src0 = src->rgbp;

  __m128i src0mm;
  pix_load4(src0mm, &src->rgbp);

  BLIT_SSE2_32x4_INIT(dst, w);

  if (isAlpha0xFF(src0) & (msk0 == 0xFF))
  {
    pix_expand_pixel_1x4B(src0mm, src0mm);

    BLIT_SSE2_32x4_SMALL_BEGIN(blt_opaque)
      pix_store4(dst, src0mm);
      dst += 4;
    BLIT_SSE2_32x4_SMALL_END(blt_opaque)

    BLIT_SSE2_32x4_LARGE_BEGIN(blt_opaque)
      pix_store16a(dst, src0mm);
      dst += 16;
    BLIT_SSE2_32x4_LARGE_END(blt_opaque)
  }
  else
  {
    __m128i m0mm;

    pix_unpack_1x2W(src0mm, src0mm);
    pix_expand_mask_1x1W(m0mm, msk0);
    pix_multiply_1x1W(src0mm, src0mm, m0mm);
    pix_expand_alpha_1x1W(m0mm, src0mm);
    pix_negate_1x1W(m0mm, m0mm);

    pix_pack_1x1W(src0mm, src0mm);
    pix_expand_pixel_1x4B(src0mm, src0mm);
    pix_expand_pixel_1x2W(m0mm, m0mm);
    src0mm = _mm_or_si128(src0mm, Mask_FF000000FF000000_FF000000FF000000);

    BLIT_SSE2_32x4_SMALL_BEGIN(blt_trans)
      __m128i dst0mm;

      pix_load4(dst0mm, dst);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, m0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_adds_1x1B(dst0mm, dst0mm, src0mm);
      pix_store4(dst, dst0mm);

      dst += 4;
    BLIT_SSE2_32x4_SMALL_END(blt_trans)

    BLIT_SSE2_32x4_LARGE_BEGIN(blt_trans)
      __m128i dst0mm, dst1mm;

      pix_load16a(dst0mm, dst);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_multiply_2x2W(dst0mm, dst0mm, m0mm, dst1mm, dst1mm, m0mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_adds_1x4B(dst0mm, dst0mm, src0mm);
      pix_store16a(dst, dst0mm);

      dst += 16;
    BLIT_SSE2_32x4_LARGE_END(blt_trans)
  }
}

static void FOG_FASTCALL raster_rgb32_span_composite_argb32_srcover_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;
    __m128i a0mm;

    uint32_t src0 = READ_32(src);
    if (!isAlpha0x00(src0))
    {
      if (isAlpha0xFF(src0))
      {
        ((uint32_t*)dst)[0] = src0;
      }
      else
      {
        pix_load4(dst0mm, dst);
        dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
        pix_unpack_1x1W(src0mm, src0);
        pix_unpack_1x1W(dst0mm, dst0mm);

        pix_expand_alpha_1x1W(a0mm, src0mm);
        pix_fill_alpha_1x1W(src0mm);
        pix_multiply_1x1W(src0mm, src0mm, a0mm);

        pix_over_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);

        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    pix_load16u(src0mm, src);
    uint32_t src0a = pix_pack_alpha_to_uint32(src0mm);
    if (src0a != 0x00000000)
    {
      if (src0a == 0xFFFFFFFF)
      {
        pix_store16a(dst, src0mm);
      }
      else
      {
        pix_load16a(dst0mm, dst);
        dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);

        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_alpha_2x2W(a0mm, src0mm, a1mm, src1mm);
        pix_fill_alpha_2x2W(src0mm, src1mm);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);

        pix_over_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);

        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_rgb32_span_composite_prgb32_srcover_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t src0 = READ_32(src);

    if (src0)
    {
      pix_load4(dst0mm, dst);
      dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
      pix_unpack_1x1W(src0mm, src0);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_expand_alpha_1x1W(a0mm, src0mm);
      pix_over_1x1W(dst0mm, src0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);

      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16u(src0mm, src);
    uint32_t src0a = pix_pack_alpha_to_uint32(src0mm);

    if (src0a != 0x00000000)
    {
      if (src0a == 0xFFFFFFFF)
      {
        pix_store16a(dst, src0mm);
      }
      else
      {
        pix_load16a(dst0mm, dst);
        dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);

        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_alpha_2x2W(a0mm, src0mm, a1mm, src1mm);
        pix_over_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);

        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

#define raster_rgb32_span_composite_rgb32_srcover_sse2 convert_memcpy32_sse2
#define raster_rgb32_span_composite_rgb24_srcover_sse2 convert_rgb32_from_rgb24_sse2

static void FOG_FASTCALL raster_rgb32_span_composite_argb32_a8_srcover_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t src0 = READ_32(src);
    uint32_t msk0 = READ_8(msk);

    if ((!isAlpha0x00(src0)) & (msk0 != 0x00))
    {
      msk0 = singlemul(src0 >> 24, msk0);

      pix_load4(dst0mm, dst);
      dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
      pix_unpack_1x1W(src0mm, src0);
      pix_unpack_1x1W(dst0mm, dst0mm);

      pix_expand_mask_1x1W(a0mm, msk0);
      pix_fill_alpha_1x1W(src0mm);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_over_1x1W(dst0mm, src0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);

      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16u(src0mm, src);
    uint32_t src0a = pix_pack_alpha_to_uint32(src0mm);
    uint32_t msk0 = ((uint32_t*)msk)[0];

    if ((src0a != 0x00000000) & (msk0 != 0x00000000))
    {
      if ((src0a == 0xFFFFFFFF) & (msk0 == 0xFFFFFFFF))
      {
        pix_store16a(dst, src0mm);
      }
      else
      {
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_expand_mask_2x2W(dst0mm, dst1mm, msk0);
        pix_expand_alpha_2x2W(a0mm, src0mm, a1mm, src1mm);
        pix_multiply_2x2W(a0mm, a0mm, dst0mm, a1mm, a1mm, dst1mm);

        pix_load16a(dst0mm, dst);
        dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_fill_alpha_2x2W(src0mm, src1mm);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
        pix_over_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);

        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_rgb32_span_composite_prgb32_a8_srcover_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t src0 = READ_32(src);
    uint32_t msk0 = READ_8(msk);

    if ((src0 != 0x00000000) & (msk0 != 0x00))
    {
      pix_load4(dst0mm, dst);
      dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
      pix_unpack_1x1W(src0mm, src0);
      pix_unpack_1x1W(dst0mm, dst0mm);

      pix_expand_mask_1x1W(a0mm, msk0);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_expand_alpha_1x1W(a0mm, src0mm);
      pix_over_1x1W(dst0mm, src0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);

      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16u(src0mm, src);
    uint32_t src0a = pix_pack_alpha_to_uint32(src0mm);
    uint32_t msk0 = ((uint32_t*)msk)[0];

    if ((src0a != 0x00000000) & (msk0 != 0x00000000))
    {
      if ((src0a == 0xFFFFFFFF) & (msk0 == 0xFFFFFFFF))
      {
        pix_store16a(dst, src0mm);
      }
      else
      {
        pix_load16a(dst0mm, dst);
        dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);

        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
        pix_expand_alpha_2x2W(a0mm, src0mm, a1mm, src1mm);
        pix_over_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);

        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_rgb32_span_composite_rgb32_a8_srcover_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);

    if (msk0 != 0x00)
    {
      pix_load4(src0mm, src);

      if (msk0 == 0xFF)
      {
        pix_store4(dst, src0mm);
      }
      else
      {
        pix_load4(dst0mm, dst);
        dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
        pix_unpack_1x1W(dst0mm, dst0mm);
        pix_unpack_1x1W(src0mm, src0mm);

        pix_expand_mask_1x1W(a0mm, msk0);
        pix_fill_alpha_1x1W(src0mm);
        pix_multiply_1x1W(src0mm, src0mm, a0mm);
        pix_over_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);

    if (msk0 != 0x00000000)
    {
      pix_load16u(src0mm, src);

      if (msk0 == 0xFFFFFFFF)
      {
        pix_store16a(dst, src0mm);
      }
      else
      {
        pix_load16a(dst0mm, dst);
        dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
        src0mm = _mm_or_si128(src0mm, Mask_FF000000FF000000_FF000000FF000000);

        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
        pix_over_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_rgb32_span_composite_rgb24_a8_srcover_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);

    if (msk0 != 0x00)
    {
      uint32_t src0 = PixFmt_RGB24::fetch(src);

      if (msk0 == 0xFF)
      {
        ((uint32_t*)dst)[0] = src0;
      }
      else
      {
        pix_load4(dst0mm, dst);
        dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
        pix_unpack_1x1W(dst0mm, dst0mm);
        pix_unpack_1x1W(src0mm, src0);

        pix_expand_mask_1x1W(a0mm, msk0);
        pix_fill_alpha_1x1W(src0mm);
        pix_multiply_1x1W(src0mm, src0mm, a0mm);
        pix_over_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    src += 3;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = ((uint32_t*)msk)[0];

    if (msk0 != 0x00000000)
    {
      pix_fetch_rgb24_2x2W(src0mm, src1mm, src);
      if (msk0 == 0xFFFFFFFF)
      {
        pix_pack_2x2W(dst0mm, src0mm, src1mm);
        pix_store16a(dst, dst0mm);
      }
      else
      {
        pix_load16a(dst0mm, dst);
        dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);

        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_fill_alpha_2x2W(src0mm, src1mm);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
        pix_over_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 12;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_rgb32_span_composite_argb32_a8_const_srcover_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_rgb32_span_composite_argb32_srcover_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t src0 = READ_32(src);

    if (!isAlpha0x00(src0))
    {
      if (isAlpha0xFF(src0))
      {
        pix_load4(dst0mm, dst);
        dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);

        pix_unpack_1x1W(src0mm, src0);
        pix_unpack_1x1W(dst0mm, dst0mm);

        pix_multiply_1x1W(src0mm, src0mm, m0mm);
        pix_over_ialpha_1x1W(dst0mm, src0mm, im0mm);
        pix_pack_1x1W(dst0mm, dst0mm);

        pix_store4(dst, dst0mm);
      }
      else
      {
        uint32_t m0 = singlemul(src0 >> 24, msk0);

        pix_load4(dst0mm, dst);
        dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);

        pix_unpack_1x1W(src0mm, src0);
        pix_unpack_1x1W(dst0mm, dst0mm);

        pix_expand_mask_1x1W(a0mm, m0);
        pix_fill_alpha_1x1W(src0mm);
        pix_multiply_1x1W(src0mm, src0mm, a0mm);
        pix_over_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);

        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16u(src0mm, src);
    uint32_t src0a = pix_pack_alpha_to_uint32(src0mm);

    if (src0a != 0x00000000)
    {
      if (src0a == 0xFFFFFFFF)
      {
        pix_load16a(dst0mm, dst);
        dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);

        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_multiply_2x2W(src0mm, src0mm, m0mm, src1mm, src1mm, m0mm);
        pix_over_ialpha_2x2W(dst0mm, src0mm, im0mm, dst1mm, src1mm, im0mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);

        pix_store16a(dst, dst0mm);
      }
      else
      {
        pix_load16a(dst0mm, dst);
        dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);

        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_expand_alpha_2x2W(a0mm, src0mm, a1mm, src1mm);
        pix_multiply_2x2W(a0mm, a0mm, m0mm, a1mm, a1mm, m0mm);

        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_fill_alpha_2x2W(src0mm, src1mm);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
        pix_over_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);

        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_rgb32_span_composite_prgb32_a8_const_srcover_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_rgb32_span_composite_argb32_srcover_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t src0 = READ_32(src);

    if (!(isAlpha0x00(src0)))
    {
      pix_load4(dst0mm, dst);
      dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);

      pix_unpack_1x1W(src0mm, src0);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_multiply_1x1W(src0mm, src0mm, m0mm);

      if (isAlpha0xFF(src0))
      {
        pix_over_ialpha_1x1W(dst0mm, src0mm, im0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
      else
      {
        pix_expand_alpha_1x1W(a0mm, src0mm);
        pix_over_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16u(src0mm, src);
    uint32_t src0a = pix_pack_alpha_to_uint32(src0mm);

    if (src0a != 0x00000000)
    {
      pix_load16a(dst0mm, dst);
      dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);

      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_multiply_2x2W(src0mm, src0mm, m0mm, src1mm, src1mm, m0mm);

      if (src0a == 0xFFFFFFFF)
      {
        pix_over_ialpha_2x2W(dst0mm, src0mm, im0mm, dst1mm, src1mm, im0mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
      else
      {
        pix_expand_alpha_2x2W(a0mm, src0mm, a1mm, src1mm);
        pix_over_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_rgb32_span_composite_rgb32_a8_const_srcover_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    convert_memcpy32_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm, im0mm;
  __m128i amask = Mask_FF000000FF000000_FF000000FF000000;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    uint32_t src0 = READ_32(src) | 0xFF000000;

    pix_load4(dst0mm, dst);
    dst0mm = _mm_or_si128(dst0mm, amask);
    pix_unpack_1x1W(src0mm, src0);
    pix_unpack_1x1W(dst0mm, dst0mm);

    pix_multiply_1x1W(src0mm, src0mm, m0mm);
    pix_over_ialpha_1x1W(dst0mm, src0mm, im0mm);
    pix_pack_1x1W(dst0mm, dst0mm);

    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16u(src0mm, src);
    pix_load16a(dst0mm, dst);
    dst0mm = _mm_or_si128(dst0mm, amask);
    src0mm = _mm_or_si128(src0mm, amask);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_multiply_2x2W(src0mm, src0mm, m0mm, src1mm, src1mm, m0mm);
    pix_over_ialpha_2x2W(dst0mm, src0mm, im0mm, dst1mm, src1mm, im0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);

    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_rgb32_span_composite_rgb24_a8_const_srcover_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    convert_rgb32_from_rgb24_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    uint32_t src0 = PixFmt_RGB24::fetch(src) | 0xFF000000;

    pix_load4(dst0mm, dst);
    dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
    pix_unpack_1x1W(src0mm, src0);
    pix_unpack_1x1W(dst0mm, dst0mm);

    pix_multiply_1x1W(src0mm, src0mm, m0mm);
    pix_over_ialpha_1x1W(dst0mm, src0mm, im0mm);
    pix_pack_1x1W(dst0mm, dst0mm);

    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);
    dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_fetch_rgb24_2x2W(src0mm, src1mm, src);
    pix_multiply_2x2W(src0mm, src0mm, m0mm, src1mm, src1mm, m0mm);
    pix_over_ialpha_2x2W(dst0mm, src0mm, im0mm, dst1mm, src1mm, im0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);

    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

// ============================================================================
// [Fog::Raster - Raster - SrcOver - A8]
// ============================================================================

static void FOG_FASTCALL raster_a8_pixel_srcover_sse2(
  uint8_t* dst, const Solid* src, const Closure* closure)
{
  uint32_t src0 = src->rgba >> 24;

  if (src0 == 0xFF)
    dst[0] = (uint8_t)src0;
  else
    dst[0] = (uint8_t)(src0 + singlemul(dst[0], singleneg(src0)));
}

static void FOG_FASTCALL raster_a8_pixel_a8_srcover_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, const Closure* closure)
{
  uint32_t src0 = src->rgba >> 24;

  if ((src0 == 0xFF) & (msk0 == 0xFF))
  {
    dst[0] = (uint8_t)src0;
  }
  else
  {
    src0 = singlemul(src0, msk0);
    dst[0] = (uint8_t)(src0 + singlemul(dst[0], singleneg(src0)));
  }
}

static void FOG_FASTCALL raster_a8_span_solid_srcover_sse2(
  uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure)
{
  uint32_t src0 = src->rgba >> 24;

  if (src0 == 0xFF)
  {
    __m128i src0mm;
    pix_expand_a8_1x4D(src0mm, src0);

    BLIT_SSE2_8x16_INIT(dst, w)

    BLIT_SSE2_8x16_SMALL_BEGIN(blt_opaque)
      dst[0] = (uint8_t)src0;
      dst += 1;
    BLIT_SSE2_8x16_SMALL_END(blt_opaque)

    while (w >= 4)
    {
      pix_store16a(dst, src0mm);
      pix_store16a(dst + 16, src0mm);
      pix_store16a(dst + 32, src0mm);
      pix_store16a(dst + 48, src0mm);

      dst += 64;
      w -= 4;
    }

    switch (w & 3)
    {
      case 3: pix_store16a(dst, src0mm); dst += 16;
      case 2: pix_store16a(dst, src0mm); dst += 16;
      case 1: pix_store16a(dst, src0mm); dst += 16;
    }

    if ((_i = _j)) { w = 0; goto blt_opaque; }
  }
  else
  {
    uint32_t src0neg = singleneg(src0);
    __m128i src0mm;
    __m128i isrc0mm;

    pix_expand_a8_1x2W(src0mm, src0);
    pix_negate_1x1W(isrc0mm, src0mm);
    pix_pack_1x1W(src0mm, src0mm);

    BLIT_SSE2_8x16_INIT(dst, w)

    BLIT_SSE2_8x16_SMALL_BEGIN(blt_trans)
      dst[0] = (uint8_t)(src0 + singlemul(dst[0], src0neg));
      dst += 1;
    BLIT_SSE2_8x16_SMALL_END(blt_trans)

    BLIT_SSE2_8x16_LARGE_BEGIN(blt_trans)
      __m128i dst0mm, dst1mm;

      pix_load16a(dst0mm, dst);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_multiply_2x2W(dst0mm, dst0mm, isrc0mm, dst1mm, dst1mm, isrc0mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_adds_1x4B(dst0mm, dst0mm, src0mm);
      pix_store16a(dst, dst0mm);
      dst += 16;
    BLIT_SSE2_8x16_LARGE_END(blt_trans)
  }
}

static void FOG_FASTCALL raster_a8_span_solid_a8_srcover_sse2(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  uint32_t src0 = src->rgba >> 24;
  uint32_t src0neg = singleneg(src0);

  __m128i src0orig;
  __m128i src0mm;
  __m128i isrc0mm;

  pix_expand_a8_1x2W(src0mm, src0);
  pix_negate_1x1W(isrc0mm, src0mm);
  pix_pack_1x1W(src0orig, src0mm);

  BLIT_SSE2_8x16_INIT(dst, w)

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      if (msk0 == 0xFF)
      {
        dst[0] = (uint8_t)(src0 + singlemul(dst[0], src0neg));
      }
      else
      {
        msk0 = singlemul(src0, msk0);
        dst[0] = (uint8_t)(msk0 + singlemul(dst[0], singleneg(msk0)));
      }
    }

    dst += 1;
    msk += 1;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    uint32_t msk0IsZero;
    uint32_t msk0IsFull;

    __m128i dst0mm, dst1mm;
    __m128i msk0mm, msk1mm;

    pix_load16u(msk0mm, msk);
    pix_analyze_mask_16B(msk0IsZero, msk0IsFull, msk0mm);

    if (msk0IsZero != 0xFFFF)
    {
      if (msk0IsFull == 0xFFFF)
      {
        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_multiply_2x2W(dst0mm, dst0mm, isrc0mm, dst1mm, dst1mm, isrc0mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_adds_1x4B(dst0mm, dst0mm, src0orig);
        pix_store16a(dst, dst0mm);
      }
      else
      {
        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_unpack_2x2W(msk0mm, msk1mm, msk0mm);
        pix_multiply_2x2W(msk0mm, msk0mm, src0mm, msk1mm, msk1mm, src0mm);
        pix_over_2x2W(dst0mm, msk0mm, msk0mm, dst1mm, msk1mm, msk1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    msk += 16;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

static void FOG_FASTCALL raster_a8_span_solid_a8_const_srcover_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (msk0 == 0xFF)
  {
    raster_a8_span_solid_srcover_sse2(dst, src, w, closure);
    return;
  }

  uint32_t src0 = singlemul(src->rgba >> 24, msk0);
  uint32_t src0neg = singleneg(src0);

  __m128i src0mm;
  __m128i isrc0mm;

  pix_expand_a8_1x2W(src0mm, src0);
  pix_negate_1x1W(isrc0mm, src0mm);

  BLIT_SSE2_8x16_INIT(dst, w)

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    dst[0] = (uint8_t)(src0 + singlemul(dst[0], src0neg));

    dst += 1;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_over_ialpha_2x2W(dst0mm, src0mm, isrc0mm, dst1mm, src0mm, isrc0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

static void FOG_FASTCALL raster_a8_span_composite_axxx32_srcover_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_8x16_INIT(dst, w);

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    uint32_t src0 = src[RGB32_AByte];
    if (src0 != 0x00)
    {
      if (src0 == 0xFF)
      {
        dst[0] = (uint8_t)src0;
      }
      else
      {
        dst[0] = (uint8_t)(src0 + singlemul(dst[0], singleneg(src0)));
      }
    }

    dst += 1;
    src += 4;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    uint32_t src0IsZero;
    uint32_t src0IsFull;

    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load_and_unpack_axxx32_64B(src0mm, src1mm, src);
    pix_pack_2x2W(src0mm, src0mm, src1mm);

    pix_analyze_mask_16B(src0IsZero, src0IsFull, src0mm);
    if (src0IsZero != 0xFFFF)
    {
      if (src0IsFull == 0xFFFF)
      {
        pix_store16a(dst, src0mm);
      }
      else
      {
        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_over_2x2W(dst0mm, src0mm, src0mm, dst1mm, src1mm, src1mm);
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 64;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

#define raster_a8_span_composite_argb32_srcover_sse2 raster_a8_span_composite_axxx32_srcover_sse2
#define raster_a8_span_composite_prgb32_srcover_sse2 raster_a8_span_composite_axxx32_srcover_sse2
#define raster_a8_span_composite_rgb32_srcover_sse2 raster_a8_span_composite_0xff_src_sse2
#define raster_a8_span_composite_rgb24_srcover_sse2 raster_a8_span_composite_0xff_src_sse2

static void FOG_FASTCALL raster_a8_span_composite_a8_srcover_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_8x16_INIT(dst, w);

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    uint32_t src0 = src[0];
    if (src0 != 0x00)
    {
      if (src0 == 0xFF)
      {
        dst[0] = (uint8_t)src0;
      }
      else
      {
        dst[0] = (uint8_t)(src0 + singlemul(dst[0], singleneg(src0)));
      }
    }

    dst += 1;
    src += 1;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    uint32_t src0IsZero;
    uint32_t src0IsFull;

    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16u(src0mm, src);
    pix_analyze_mask_16B(src0IsZero, src0IsFull, src0mm);

    if (src0IsZero != 0xFFFF)
    {
      if (src0IsFull == 0xFFFF)
      {
        pix_store16a(dst, src0mm);
      }
      else
      {
        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_over_2x2W(dst0mm, src0mm, src0mm, dst1mm, src1mm, src1mm);
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

static void FOG_FASTCALL raster_a8_span_composite_axxx32_a8_srcover_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_8x16_INIT(dst, w);

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      uint32_t src0 = singlemul(src[RGB32_AByte], msk0);
      dst[0] = (uint8_t)(src0 + singlemul(dst[0], singleneg(src0)));
    }

    dst += 1;
    src += 4;
    msk += 1;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    uint32_t msk0IsZero;
    uint32_t msk0IsFull;

    __m128i src0mm, src1mm;
    __m128i msk0mm;

    pix_load16u(msk0mm, msk);
    pix_analyze_mask_16B(msk0IsZero, msk0IsFull, msk0mm);

    if (msk0IsZero != 0xFFFF)
    {
      uint32_t src0IsZero;
      uint32_t src0IsFull;

      pix_load_and_unpack_axxx32_64B(src0mm, src1mm, src);
      pix_pack_2x2W(src0mm, src0mm, src1mm);
      pix_analyze_mask_16B(src0IsZero, src0IsFull, src0mm);

      if (src0IsZero != 0xFFFF)
      {
        if ((src0IsFull == 0xFFFF) & (msk0IsFull == 0xFFFF))
        {
          pix_store16a(dst, src0mm);
        }
        else
        {
          __m128i dst0mm, dst1mm;
          __m128i a0mm, a1mm;

          pix_load16a(dst0mm, dst);
          pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
          pix_unpack_2x2W(src0mm, src1mm, src0mm);
          pix_unpack_2x2W(a0mm, a1mm, msk0mm);
          pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
          pix_over_2x2W(dst0mm, src0mm, src0mm, dst1mm, src1mm, src1mm);
          pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
          pix_store16a(dst, dst0mm);
        }
      }
    }

    dst += 16;
    src += 64;
    msk += 16;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

#define raster_a8_span_composite_argb32_a8_srcover_sse2 raster_a8_span_composite_axxx32_a8_srcover_sse2
#define raster_a8_span_composite_prgb32_a8_srcover_sse2 raster_a8_span_composite_axxx32_a8_srcover_sse2
#define raster_a8_span_composite_rgb32_a8_srcover_sse2 raster_a8_span_composite_0xff_a8_src_sse2
#define raster_a8_span_composite_rgb24_a8_srcover_sse2 raster_a8_span_composite_0xff_a8_src_sse2

static void FOG_FASTCALL raster_a8_span_composite_a8_a8_srcover_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_8x16_INIT(dst, w);

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      uint32_t src0 = singlemul(src[RGB32_AByte], msk0);
      dst[0] = (uint8_t)(src0 + singlemul(dst[0], singleneg(src0)));
    }

    dst += 1;
    src += 1;
    msk += 1;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    uint32_t msk0IsZero;
    uint32_t msk0IsFull;

    __m128i src0mm, src1mm;
    __m128i msk0mm;

    pix_load16u(msk0mm, msk);
    pix_analyze_mask_16B(msk0IsZero, msk0IsFull, msk0mm);

    if (msk0IsZero != 0xFFFF)
    {
      uint32_t src0IsZero;
      uint32_t src0IsFull;

      pix_load16u(src0mm, src);
      pix_analyze_mask_16B(src0IsZero, src0IsFull, src0mm);

      if (src0IsZero != 0xFFFF)
      {
        if ((src0IsFull == 0xFFFF) & (msk0IsFull == 0xFFFF))
        {
          pix_store16a(dst, src0mm);
        }
        else
        {
          __m128i dst0mm, dst1mm;
          __m128i a0mm, a1mm;

          pix_load16a(dst0mm, dst);
          pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
          pix_unpack_2x2W(src0mm, src1mm, src0mm);
          pix_unpack_2x2W(a0mm, a1mm, msk0mm);
          pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
          pix_over_2x2W(dst0mm, src0mm, src0mm, dst1mm, src1mm, src1mm);
          pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
          pix_store16a(dst, dst0mm);
        }
      }
    }

    dst += 16;
    src += 16;
    msk += 16;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

static void FOG_FASTCALL raster_a8_span_composite_axxx32_a8_const_srcover_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_a8_span_composite_axxx32_srcover_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x1W(im0mm, m0mm);

  BLIT_SSE2_8x16_INIT(dst, w);

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    uint32_t src0 = singlemul(src[RGB32_AByte], msk0);
    dst[0] = (uint8_t)(src0 + singlemul(dst[0], singleneg(src0)));

    dst += 1;
    src += 4;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;
    __m128i srctmp;

    uint32_t src0IsZero;
    uint32_t src0IsFull;

    pix_load_and_unpack_axxx32_64B(src0mm, src1mm, src);
    pix_pack_2x2W(srctmp, src0mm, src1mm);
    pix_analyze_mask_16B(src0IsZero, src0IsFull, srctmp);

    if (src0IsZero != 0xFFFF)
    {
      pix_load16a(dst0mm, dst);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);

      if (src0IsFull == 0xFFFF)
      {
        pix_multiply_2x2W(src0mm, src0mm, m0mm, src1mm, src1mm, m0mm);
        pix_multiply_2x2W(dst0mm, dst0mm, im0mm, dst1mm, dst1mm, im0mm);
        pix_adds_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      }
      else
      {
        pix_multiply_2x2W(src0mm, src0mm, m0mm, src1mm, src1mm, m0mm);
        pix_over_2x2W(dst0mm, src0mm, src0mm, dst1mm, src1mm, src1mm);
      }

      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 64;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

#define raster_a8_span_composite_argb32_a8_const_srcover_sse2 raster_a8_span_composite_axxx32_a8_const_srcover_sse2
#define raster_a8_span_composite_prgb32_a8_const_srcover_sse2 raster_a8_span_composite_axxx32_a8_const_srcover_sse2
#define raster_a8_span_composite_rgb32_a8_const_srcover_sse2 raster_a8_span_composite_0xff_a8_const_src_sse2
#define raster_a8_span_composite_rgb24_a8_const_srcover_sse2 raster_a8_span_composite_0xff_a8_const_src_sse2

static void FOG_FASTCALL raster_a8_span_composite_a8_a8_const_srcover_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_a8_span_composite_a8_srcover_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x1W(im0mm, m0mm);

  BLIT_SSE2_8x16_INIT(dst, w);

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    uint32_t src0 = singlemul(src[0], msk0);
    dst[0] = (uint8_t)(src0 + singlemul(dst[0], singleneg(src0)));

    dst += 1;
    src += 1;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;

    uint32_t src0IsZero;
    uint32_t src0IsFull;

    pix_load16u(src0mm, src);
    pix_analyze_mask_16B(src0IsZero, src0IsFull, src0mm);

    if (src0IsZero != 0xFFFF)
    {
      pix_load16a(dst0mm, dst);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);

      if (src0IsFull == 0xFFFF)
      {
        pix_multiply_2x2W(src0mm, src0mm, m0mm, src1mm, src1mm, m0mm);
        pix_multiply_2x2W(dst0mm, dst0mm, im0mm, dst1mm, dst1mm, im0mm);
        pix_adds_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      }
      else
      {
        pix_multiply_2x2W(src0mm, src0mm, m0mm, src1mm, src1mm, m0mm);
        pix_over_2x2W(dst0mm, src0mm, src0mm, dst1mm, src1mm, src1mm);
      }

      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_8x16_LARGE_END(blt)
}










































// ============================================================================
// [Fog::Raster - Raster - DstOver - Argb32]
// ============================================================================

static void FOG_FASTCALL raster_argb32_pixel_dstover_sse2(
  uint8_t* dst, const Solid* src, const Closure* closure)
{
  uint32_t src0 = src->rgbp;
  if (src0 == 0x00000000) return;

  uint32_t dst0 = READ_32(dst);
  if (!isAlpha0xFF(dst0))
  {
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_unpack_1x1W(src0mm, src0);
    pix_unpack_1x1W(dst0mm, dst0);
    pix_expand_alpha_1x1W(a0mm, dst0mm);
    pix_fill_alpha_1x1W(dst0mm);
    pix_multiply_1x1W(dst0mm, dst0mm, a0mm);
    pix_overrev_1x1W(dst0mm, src0mm, a0mm);
    pix_demultiply_1x1W(dst0mm, dst0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);
  }
}

static void FOG_FASTCALL raster_argb32_pixel_a8_dstover_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, const Closure* closure)
{
  uint32_t src0 = src->rgbp;
  if (src0 == 0x00000000) return;

  uint32_t dst0 = READ_32(dst);
  if (!isAlpha0xFF(dst0) && msk0 != 0x00)
  {
    __m128i src0mm;
    __m128i dst0mm;
    __m128i msk0mm;
    __m128i a0mm;

    pix_unpack_1x1W(src0mm, src0);
    pix_unpack_1x1W(dst0mm, dst0);
    pix_expand_mask_1x1W(msk0mm, msk0);
    pix_expand_alpha_1x1W(a0mm, dst0mm);
    pix_fill_alpha_1x1W(dst0mm);
    pix_multiply_1x1W(dst0mm, dst0mm, a0mm);
    pix_negate_1x1W(a0mm, a0mm);
    pix_multiply_1x1W(a0mm, a0mm, msk0mm);
    pix_multiply_1x1W(a0mm, a0mm, src0mm);
    pix_adds_1x1W(dst0mm, dst0mm, a0mm);
    pix_demultiply_1x1W(dst0mm, dst0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);
  }
}

static void FOG_FASTCALL raster_argb32_span_solid_dstover_sse2(
  uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure)
{
  uint32_t src0 = src->rgbp;
  if (src0 == 0x00000000) return;

  __m128i src0mm;
  pix_load4(src0mm, &src->rgbp);
  pix_unpack_1x1W(src0mm, src0mm);
  pix_expand_pixel_1x2W(src0mm, src0mm);

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i a0mm;

    if (!isAlpha0xFF(READ_32(dst)))
    {
      pix_load4(dst0mm, dst);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_expand_alpha_1x1W(a0mm, dst0mm);
      pix_fill_alpha_1x1W(dst0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, a0mm);
      pix_overrev_1x1W(dst0mm, src0mm, a0mm);
      pix_demultiply_1x1W(dst0mm, dst0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0xFFFFFFFF)
    {
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
      pix_fill_alpha_2x2W(dst0mm, dst1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, a0mm, dst1mm, dst1mm, a1mm);
      pix_overrev_2x2W(dst0mm, src0mm, a0mm, dst1mm, src0mm, a1mm);
      pix_demultiply_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_argb32_span_solid_a8_dstover_sse2(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  uint32_t src0 = src->rgbp;
  if (src0 == 0x00000000) return;

  __m128i src0mm;
  pix_load4(src0mm, &src->rgbp);
  pix_unpack_1x1W(src0mm, src0mm);
  pix_expand_pixel_1x2W(src0mm, src0mm);

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i msk0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00 && !isAlpha0xFF(READ_32(dst)))
    {
      pix_load4(dst0mm, dst);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_expand_alpha_1x1W(a0mm, dst0mm);
      pix_fill_alpha_1x1W(dst0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, a0mm);
      pix_negate_1x1W(a0mm, a0mm);
      pix_multiply_1x1W(a0mm, a0mm, msk0mm);
      pix_multiply_1x1W(a0mm, a0mm, src0mm);
      pix_adds_1x1W(dst0mm, dst0mm, a0mm);
      pix_demultiply_1x1W(dst0mm, dst0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i msk0mm, msk1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      pix_load16a(dst0mm, dst);
      uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
      if (dst0a != 0xFFFFFFFF)
      {
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
        pix_fill_alpha_2x2W(dst0mm, dst1mm);
        pix_multiply_2x2W(dst0mm, dst0mm, a0mm, dst1mm, dst1mm, a1mm);
        pix_negate_2x2W(a0mm, a0mm, a1mm, a1mm);
        pix_multiply_2x2W(a0mm, a0mm, msk0mm, a1mm, a1mm, msk1mm);
        pix_multiply_2x2W(a0mm, a0mm, src0mm, a1mm, a1mm, src0mm);
        pix_adds_2x2W(dst0mm, dst0mm, a0mm, dst1mm, dst1mm, a1mm);
        pix_demultiply_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_argb32_span_solid_a8_const_dstover_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  uint32_t src0 = src->rgbp;
  if (src0 == 0x00000000) return;

  __m128i src0mm;
  pix_load4(src0mm, &src->rgbp);
  pix_unpack_1x1W(src0mm, src0mm);

  {
    __m128i tmp0mm;
    pix_expand_mask_1x1W(tmp0mm, msk0);
    pix_multiply_1x1W(src0mm, src0mm, tmp0mm);
  }
  pix_expand_pixel_1x2W(src0mm, src0mm);

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i a0mm;

    if (!isAlpha0xFF(READ_32(dst)))
    {
      pix_load4(dst0mm, dst);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_expand_alpha_1x1W(a0mm, dst0mm);
      pix_fill_alpha_1x1W(dst0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, a0mm);
      pix_overrev_1x1W(dst0mm, src0mm, a0mm);
      pix_demultiply_1x1W(dst0mm, dst0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0xFFFFFFFF)
    {
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
      pix_fill_alpha_2x2W(dst0mm, dst1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, a0mm, dst1mm, dst1mm, a1mm);
      pix_overrev_2x2W(dst0mm, src0mm, a0mm, dst1mm, src0mm, a1mm);
      pix_demultiply_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_argb32_span_composite_argb32_dstover_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;
    __m128i a0mm;

    if (!isAlpha0xFF(READ_32(dst)))
    {
      pix_load4(dst0mm, dst);
      pix_load4(src0mm, src);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_premultiply_1x1W(src0mm, src0mm);
      pix_expand_alpha_1x1W(a0mm, dst0mm);
      pix_fill_alpha_1x1W(dst0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, a0mm);
      pix_negate_1x1W(a0mm, a0mm);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_adds_1x1W(dst0mm, dst0mm, src0mm);
      pix_demultiply_1x1W(dst0mm, dst0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0xFFFFFFFF)
    {
      pix_load16u(src0mm, src);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
      pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
      pix_fill_alpha_2x2W(dst0mm, dst1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, a0mm, dst1mm, dst1mm, a1mm);
      pix_negate_2x2W(a0mm, a0mm, a1mm, a1mm);
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
      pix_adds_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_demultiply_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_argb32_span_composite_prgb32_dstover_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;
    __m128i a0mm;

    if (!isAlpha0xFF(READ_32(dst)))
    {
      pix_load4(dst0mm, dst);
      pix_load4(src0mm, src);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_expand_alpha_1x1W(a0mm, dst0mm);
      pix_fill_alpha_1x1W(dst0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, a0mm);
      pix_negate_1x1W(a0mm, a0mm);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_adds_1x1W(dst0mm, dst0mm, src0mm);
      pix_demultiply_1x1W(dst0mm, dst0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0xFFFFFFFF)
    {
      pix_load16u(src0mm, src);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
      pix_fill_alpha_2x2W(dst0mm, dst1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, a0mm, dst1mm, dst1mm, a1mm);
      pix_negate_2x2W(a0mm, a0mm, a1mm, a1mm);
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
      pix_adds_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_demultiply_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_argb32_span_composite_rgb32_dstover_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;
    __m128i a0mm;

    if (!isAlpha0xFF(READ_32(dst)))
    {
      pix_load4(dst0mm, dst);
      pix_load4(src0mm, src);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_expand_alpha_1x1W(a0mm, dst0mm);
      pix_lerp_1x1W(dst0mm, src0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_fill_alpha_1x1B(dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0xFFFFFFFF)
    {
      pix_load16u(src0mm, src);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
      pix_lerp_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_fill_alpha_1x4B(dst0mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_argb32_span_composite_argb32_a8_dstover_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00 && !isAlpha0xFF(READ_32(dst)))
    {
      pix_load4(dst0mm, dst);
      pix_load4(src0mm, src);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_premultiply_1x1W(src0mm, src0mm);
      pix_expand_mask_1x1W(a0mm, msk0);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_expand_alpha_1x1W(a0mm, dst0mm);
      pix_fill_alpha_1x1W(dst0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, a0mm);
      pix_negate_1x1W(a0mm, a0mm);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_adds_1x1W(dst0mm, dst0mm, src0mm);
      pix_demultiply_1x1W(dst0mm, dst0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      pix_load16a(dst0mm, dst);
      uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
      if (dst0a != 0xFFFFFFFF)
      {
        pix_load16u(src0mm, src);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
        pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
        pix_fill_alpha_2x2W(dst0mm, dst1mm);
        pix_multiply_2x2W(dst0mm, dst0mm, a0mm, dst1mm, dst1mm, a1mm);
        pix_negate_2x2W(a0mm, a0mm, a1mm, a1mm);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
        pix_adds_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
        pix_demultiply_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_argb32_span_composite_prgb32_a8_dstover_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00 && !isAlpha0xFF(READ_32(dst)))
    {
      pix_load4(dst0mm, dst);
      pix_load4(src0mm, src);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_expand_mask_1x1W(a0mm, msk0);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_expand_alpha_1x1W(a0mm, dst0mm);
      pix_fill_alpha_1x1W(dst0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, a0mm);
      pix_negate_1x1W(a0mm, a0mm);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_adds_1x1W(dst0mm, dst0mm, src0mm);
      pix_demultiply_1x1W(dst0mm, dst0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      pix_load16a(dst0mm, dst);
      uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
      if (dst0a != 0xFFFFFFFF)
      {
        pix_load16u(src0mm, src);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
        pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
        pix_fill_alpha_2x2W(dst0mm, dst1mm);
        pix_multiply_2x2W(dst0mm, dst0mm, a0mm, dst1mm, dst1mm, a1mm);
        pix_negate_2x2W(a0mm, a0mm, a1mm, a1mm);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
        pix_adds_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
        pix_demultiply_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_argb32_span_composite_rgb32_a8_dstover_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00 && !isAlpha0xFF(READ_32(dst)))
    {
      pix_load4(dst0mm, dst);
      pix_load4(src0mm, src);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_expand_alpha_1x1W(a0mm, dst0mm);
      pix_fill_alpha_1x1W(src0mm);
      pix_fill_alpha_1x1W(dst0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, a0mm);
      pix_negate_1x1W(a0mm, a0mm);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_expand_mask_1x1W(a0mm, msk0);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_adds_1x1W(dst0mm, dst0mm, src0mm);
      pix_demultiply_1x1W(dst0mm, dst0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      pix_load16a(dst0mm, dst);
      uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
      if (dst0a != 0xFFFFFFFF)
      {
        pix_load16u(src0mm, src);
        pix_fill_alpha_1x4B(src0mm);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
        pix_fill_alpha_2x2W(dst0mm, dst1mm);
        pix_multiply_2x2W(dst0mm, dst0mm, a0mm, dst1mm, dst1mm, a1mm);
        pix_negate_2x2W(a0mm, a0mm, a1mm, a1mm);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
        pix_adds_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
        pix_demultiply_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_argb32_span_composite_argb32_a8_const_dstover_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (msk0 == 0xFF)
  {
    raster_argb32_span_composite_argb32_dstover_sse2(dst, src, w, closure);
    return;
  }

  __m128i msk0mm;
  pix_expand_mask_1x1W(msk0mm, msk0);
  pix_expand_pixel_1x2W(msk0mm, msk0mm);

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;
    __m128i a0mm;

    if (!isAlpha0xFF(READ_32(dst)))
    {
      pix_load4(dst0mm, dst);
      pix_load4(src0mm, src);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_premultiply_1x1W(src0mm, src0mm);
      pix_multiply_1x1W(src0mm, src0mm, msk0mm);
      pix_expand_alpha_1x1W(a0mm, dst0mm);
      pix_fill_alpha_1x1W(dst0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, a0mm);
      pix_negate_1x1W(a0mm, a0mm);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_adds_1x1W(dst0mm, dst0mm, src0mm);
      pix_demultiply_1x1W(dst0mm, dst0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0xFFFFFFFF)
    {
      pix_load16u(src0mm, src);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
      pix_multiply_2x2W(src0mm, src0mm, msk0mm, src1mm, src1mm, msk0mm);
      pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
      pix_fill_alpha_2x2W(dst0mm, dst1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, a0mm, dst1mm, dst1mm, a1mm);
      pix_negate_2x2W(a0mm, a0mm, a1mm, a1mm);
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
      pix_adds_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_demultiply_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_argb32_span_composite_prgb32_a8_const_dstover_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (msk0 == 0xFF)
  {
    raster_argb32_span_composite_prgb32_dstover_sse2(dst, src, w, closure);
    return;
  }

  __m128i msk0mm;
  pix_expand_mask_1x1W(msk0mm, msk0);
  pix_expand_pixel_1x2W(msk0mm, msk0mm);

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;
    __m128i a0mm;

    if (!isAlpha0xFF(READ_32(dst)))
    {
      pix_load4(dst0mm, dst);
      pix_load4(src0mm, src);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_multiply_1x1W(src0mm, src0mm, msk0mm);
      pix_expand_alpha_1x1W(a0mm, dst0mm);
      pix_fill_alpha_1x1W(dst0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, a0mm);
      pix_negate_1x1W(a0mm, a0mm);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_adds_1x1W(dst0mm, dst0mm, src0mm);
      pix_demultiply_1x1W(dst0mm, dst0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0xFFFFFFFF)
    {
      pix_load16u(src0mm, src);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_multiply_2x2W(src0mm, src0mm, msk0mm, src1mm, src1mm, msk0mm);
      pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
      pix_fill_alpha_2x2W(dst0mm, dst1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, a0mm, dst1mm, dst1mm, a1mm);
      pix_negate_2x2W(a0mm, a0mm, a1mm, a1mm);
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
      pix_adds_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_demultiply_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_argb32_span_composite_rgb32_a8_const_dstover_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (msk0 == 0xFF)
  {
    raster_argb32_span_composite_rgb32_dstover_sse2(dst, src, w, closure);
    return;
  }

  __m128i msk0mm;
  pix_expand_mask_1x1W(msk0mm, msk0);
  pix_expand_pixel_1x2W(msk0mm, msk0mm);

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;
    __m128i a0mm;

    if (!isAlpha0xFF(READ_32(dst)))
    {
      pix_load4(dst0mm, dst);
      pix_load4(src0mm, src);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_expand_alpha_1x1W(a0mm, dst0mm);
      pix_fill_alpha_1x1W(src0mm);
      pix_fill_alpha_1x1W(dst0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, a0mm);
      pix_negate_1x1W(a0mm, a0mm);
      pix_multiply_1x1W(a0mm, a0mm, msk0mm);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_adds_1x1W(dst0mm, dst0mm, src0mm);
      pix_demultiply_1x1W(dst0mm, dst0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0xFFFFFFFF)
    {
      pix_load16u(src0mm, src);
      pix_fill_alpha_1x4B(src0mm);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
      pix_fill_alpha_2x2W(dst0mm, dst1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, a0mm, dst1mm, dst1mm, a1mm);
      pix_negate_2x2W(a0mm, a0mm, a1mm, a1mm);
      pix_multiply_2x2W(a0mm, a0mm, msk0mm, a1mm, a1mm, msk0mm);
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
      pix_adds_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_demultiply_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

// ============================================================================
// [Fog::Raster - Raster - DstOver - Prgb32]
// ============================================================================

static void FOG_FASTCALL raster_prgb32_pixel_dstover_sse2(
  uint8_t* dst, const Solid* src, const Closure* closure)
{
  uint32_t src0 = src->rgbp;
  if (src0 == 0x00000000) return;

  uint32_t dst0 = READ_32(dst);
  if (!isAlpha0xFF(dst0))
  {
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_unpack_1x1W(src0mm, src0);
    pix_unpack_1x1W(dst0mm, dst0);
    pix_expand_alpha_1x1W(a0mm, dst0mm);
    pix_negate_1x1W(a0mm, a0mm);
    pix_multiply_1x1W(src0mm, src0mm, a0mm);
    pix_adds_1x1W(dst0mm, dst0mm, src0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);
  }
}

static void FOG_FASTCALL raster_prgb32_pixel_a8_dstover_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, const Closure* closure)
{
  uint32_t src0 = src->rgbp;
  if (src0 == 0x00000000) return;

  uint32_t dst0 = READ_32(dst);
  if (!isAlpha0xFF(dst0))
  {
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_unpack_1x1W(src0mm, src0);
    pix_unpack_1x1W(dst0mm, dst0);
    pix_expand_mask_1x1W(a0mm, msk0);
    pix_multiply_1x1W(src0mm, src0mm, a0mm);
    pix_expand_alpha_1x1W(a0mm, dst0mm);
    pix_negate_1x1W(a0mm, a0mm);
    pix_multiply_1x1W(src0mm, src0mm, a0mm);
    pix_adds_1x1W(dst0mm, dst0mm, src0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);
  }
}

static void FOG_FASTCALL raster_prgb32_span_solid_dstover_sse2(
  uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure)
{
  __m128i src0mm;
  pix_load4(src0mm, src);
  pix_unpack_1x1W(src0mm, src0mm);
  pix_expand_pixel_1x2W(src0mm, src0mm);

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    if (!isAlpha0xFF(READ_32(dst)))
    {
      __m128i dst0mm;
      __m128i a0mm;

      pix_load4(dst0mm, dst);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_expand_alpha_1x1W(a0mm, dst0mm);
      pix_negate_1x1W(a0mm, a0mm);
      pix_multiply_1x1W(a0mm, a0mm, src0mm);
      pix_adds_1x1W(dst0mm, dst0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0xFFFFFFFF)
    {
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
      pix_negate_2x2W(a0mm, a0mm, a1mm, a1mm);
      pix_multiply_2x2W(a0mm, a0mm, src0mm, a1mm, a1mm, src0mm);
      pix_adds_2x2W(dst0mm, dst0mm, a0mm, dst1mm, dst1mm, a1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_solid_a8_dstover_sse2(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  uint32_t src0 = src->rgbp;
  if (src0 == 0x00000000) return;

  __m128i src0mm;
  pix_load4(src0mm, &src->rgbp);
  pix_unpack_1x1W(src0mm, src0mm);
  pix_expand_pixel_1x2W(src0mm, src0mm);

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00 && !isAlpha0xFF(READ_32(dst)))
    {
      __m128i dst0mm;
      __m128i msk0mm;
      __m128i a0mm;

      pix_load4(dst0mm, dst);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_expand_mask_1x1W(msk0mm, msk0);
      pix_expand_alpha_1x1W(a0mm, dst0mm);
      pix_negate_1x1W(a0mm, a0mm);
      pix_multiply_1x1W(a0mm, a0mm, msk0mm);
      pix_multiply_1x1W(a0mm, a0mm, src0mm);
      pix_adds_1x1W(dst0mm, dst0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i msk0mm, msk1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      pix_load16a(dst0mm, dst);
      uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
      if (dst0a != 0xFFFFFFFF)
      {
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(msk0mm, msk1mm, msk0);
        pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
        pix_negate_2x2W(a0mm, a0mm, a1mm, a1mm);
        pix_multiply_2x2W(a0mm, a0mm, msk0mm, a1mm, a1mm, msk1mm);
        pix_multiply_2x2W(a0mm, a0mm, src0mm, a1mm, a1mm, src0mm);
        pix_adds_2x2W(dst0mm, dst0mm, a0mm, dst1mm, dst1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_solid_a8_const_dstover_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  uint32_t src0 = src->rgbp;
  if (src0 == 0x00000000) return;

  __m128i src0mm;
  pix_load4(src0mm, &src->rgbp);
  pix_unpack_1x1W(src0mm, src0mm);

  {
    __m128i tmp0mm;
    pix_expand_mask_1x1W(tmp0mm, msk0);
    pix_multiply_1x1W(src0mm, src0mm, tmp0mm);
  }
  pix_expand_pixel_1x2W(src0mm, src0mm);

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    if (!isAlpha0xFF(READ_32(dst)))
    {
      __m128i dst0mm;
      __m128i a0mm;

      pix_load4(dst0mm, dst);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_expand_alpha_1x1W(a0mm, dst0mm);
      pix_negate_1x1W(a0mm, a0mm);
      pix_multiply_1x1W(a0mm, a0mm, src0mm);
      pix_adds_1x1W(dst0mm, dst0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0xFFFFFFFF)
    {
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
      pix_negate_2x2W(a0mm, a0mm, a1mm, a1mm);
      pix_multiply_2x2W(a0mm, a0mm, src0mm, a1mm, a1mm, src0mm);
      pix_adds_2x2W(dst0mm, dst0mm, a0mm, dst1mm, dst1mm, a1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_dstover_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    if (!isAlpha0xFF(READ_32(dst)))
    {
      __m128i src0mm;
      __m128i dst0mm;
      __m128i a0mm;

      pix_load4(dst0mm, dst);
      pix_load4(src0mm, src);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_premultiply_1x1W(src0mm, src0mm);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_expand_alpha_1x1W(a0mm, dst0mm);
      pix_negate_1x1W(a0mm, a0mm);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_adds_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0xFFFFFFFF)
    {
      pix_load16u(src0mm, src);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
      pix_negate_2x2W(a0mm, a0mm, a1mm, a1mm);
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
      pix_adds_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_dstover_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    if (!isAlpha0xFF(READ_32(dst)))
    {
      __m128i src0mm;
      __m128i dst0mm;
      __m128i a0mm;

      pix_load4(dst0mm, dst);
      pix_load4(src0mm, src);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_expand_alpha_1x1W(a0mm, dst0mm);
      pix_negate_1x1W(a0mm, a0mm);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_adds_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0xFFFFFFFF)
    {
      pix_load16u(src0mm, src);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
      pix_negate_2x2W(a0mm, a0mm, a1mm, a1mm);
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
      pix_adds_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb32_dstover_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    if (!isAlpha0xFF(READ_32(dst)))
    {
      __m128i src0mm;
      __m128i dst0mm;
      __m128i a0mm;

      pix_load4(dst0mm, dst);
      pix_load4(src0mm, src);
      pix_fill_alpha_1x1B(src0mm);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_expand_alpha_1x1W(a0mm, dst0mm);
      pix_negate_1x1W(a0mm, a0mm);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_adds_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0xFFFFFFFF)
    {
      pix_load16u(src0mm, src);
      pix_fill_alpha_1x4B(src0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
      pix_negate_2x2W(a0mm, a0mm, a1mm, a1mm);
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
      pix_adds_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_a8_dstover_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00 && !isAlpha0xFF(READ_32(dst)))
    {
      __m128i src0mm;
      __m128i dst0mm;
      __m128i a0mm;

      pix_load4(dst0mm, dst);
      pix_load4(src0mm, src);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_premultiply_1x1W(src0mm, src0mm);
      pix_expand_mask_1x1W(a0mm, msk0);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_expand_alpha_1x1W(a0mm, dst0mm);
      pix_negate_1x1W(a0mm, a0mm);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_adds_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      pix_load16a(dst0mm, dst);
      uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
      if (dst0a != 0xFFFFFFFF)
      {
        pix_load16u(src0mm, src);
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
        pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
        pix_negate_2x2W(a0mm, a0mm, a1mm, a1mm);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
        pix_adds_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_a8_dstover_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00 && !isAlpha0xFF(READ_32(dst)))
    {
      __m128i src0mm;
      __m128i dst0mm;
      __m128i a0mm;

      pix_load4(dst0mm, dst);
      pix_load4(src0mm, src);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_expand_mask_1x1W(a0mm, msk0);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_expand_alpha_1x1W(a0mm, dst0mm);
      pix_negate_1x1W(a0mm, a0mm);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_adds_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      pix_load16a(dst0mm, dst);
      uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
      if (dst0a != 0xFFFFFFFF)
      {
        pix_load16u(src0mm, src);
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
        pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
        pix_negate_2x2W(a0mm, a0mm, a1mm, a1mm);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
        pix_adds_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb32_a8_dstover_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00 && !isAlpha0xFF(READ_32(dst)))
    {
      __m128i src0mm;
      __m128i dst0mm;
      __m128i a0mm;

      pix_load4(dst0mm, dst);
      pix_load4(src0mm, src);
      pix_fill_alpha_1x1B(src0mm);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_expand_mask_1x1W(a0mm, msk0);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_expand_alpha_1x1W(a0mm, dst0mm);
      pix_negate_1x1W(a0mm, a0mm);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_adds_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      pix_load16a(dst0mm, dst);
      uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
      if (dst0a != 0xFFFFFFFF)
      {
        pix_load16u(src0mm, src);
        pix_fill_alpha_1x4B(src0mm);
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
        pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
        pix_negate_2x2W(a0mm, a0mm, a1mm, a1mm);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
        pix_adds_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_a8_const_dstover_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (msk0 == 0xFF)
  {
    raster_prgb32_span_composite_argb32_dstover_sse2(dst, src, w, closure);
    return;
  }

  __m128i msk0mm;
  pix_expand_mask_1x1W(msk0mm, msk0);
  pix_expand_pixel_1x2W(msk0mm, msk0mm);

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    if (!isAlpha0xFF(READ_32(dst)))
    {
      __m128i src0mm;
      __m128i dst0mm;
      __m128i a0mm;

      pix_load4(dst0mm, dst);
      pix_load4(src0mm, src);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_premultiply_1x1W(src0mm, src0mm);
      pix_multiply_1x1W(src0mm, src0mm, msk0mm);
      pix_expand_alpha_1x1W(a0mm, dst0mm);
      pix_negate_1x1W(a0mm, a0mm);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_adds_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0xFFFFFFFF)
    {
      pix_load16u(src0mm, src);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
      pix_multiply_2x2W(src0mm, src0mm, msk0mm, src1mm, src1mm, msk0mm);
      pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
      pix_negate_2x2W(a0mm, a0mm, a1mm, a1mm);
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
      pix_adds_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_a8_const_dstover_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (msk0 == 0xFF)
  {
    raster_prgb32_span_composite_prgb32_dstover_sse2(dst, src, w, closure);
    return;
  }

  __m128i msk0mm;
  pix_expand_mask_1x1W(msk0mm, msk0);
  pix_expand_pixel_1x2W(msk0mm, msk0mm);

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    if (!isAlpha0xFF(READ_32(dst)))
    {
      __m128i src0mm;
      __m128i dst0mm;
      __m128i a0mm;

      pix_load4(dst0mm, dst);
      pix_load4(src0mm, src);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_multiply_1x1W(src0mm, src0mm, msk0mm);
      pix_expand_alpha_1x1W(a0mm, dst0mm);
      pix_negate_1x1W(a0mm, a0mm);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_adds_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0xFFFFFFFF)
    {
      pix_load16u(src0mm, src);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_multiply_2x2W(src0mm, src0mm, msk0mm, src1mm, src1mm, msk0mm);
      pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
      pix_negate_2x2W(a0mm, a0mm, a1mm, a1mm);
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
      pix_adds_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb32_a8_const_dstover_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (msk0 == 0xFF)
  {
    raster_prgb32_span_composite_rgb32_dstover_sse2(dst, src, w, closure);
    return;
  }

  __m128i msk0mm;
  pix_expand_mask_1x1W(msk0mm, msk0);
  pix_expand_pixel_1x2W(msk0mm, msk0mm);

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    if (!isAlpha0xFF(READ_32(dst)))
    {
      __m128i src0mm;
      __m128i dst0mm;
      __m128i a0mm;

      pix_load4(dst0mm, dst);
      pix_load4(src0mm, src);
      pix_fill_alpha_1x1B(src0mm);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_multiply_1x1W(src0mm, src0mm, msk0mm);
      pix_expand_alpha_1x1W(a0mm, dst0mm);
      pix_negate_1x1W(a0mm, a0mm);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_adds_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0xFFFFFFFF)
    {
      pix_load16u(src0mm, src);
      pix_fill_alpha_1x4B(src0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_multiply_2x2W(src0mm, src0mm, msk0mm, src1mm, src1mm, msk0mm);
      pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
      pix_negate_2x2W(a0mm, a0mm, a1mm, a1mm);
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
      pix_adds_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}































#if 0

// ============================================================================
// [Fog::Raster - Raster - SrcIn - Argb32]
// ============================================================================

static void FOG_FASTCALL raster_argb32_pixel_srcin_sse2(
  uint8_t* dst, const Solid* src, const Closure* closure)
{
  uint32_t dst0 = READ_32(dst);
  if (!isAlpha0x00(dst0))
  {
    uint32_t src0 = src->rgba;
    ((uint32_t*)dst)[0] = (src0 & 0x00FFFFFF) | (singlemul(src0 >> 24, dst0 >> 24) << 24);
  }
}

static void FOG_FASTCALL raster_argb32_pixel_a8_srcin_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, const Closure* closure)
{
  uint32_t dst0 = READ_32(dst);
  if (!isAlpha0x00(dst0))
  {
    __m128i pix0mm;
    __m128i pix1mm;
    __m128i a0mm;

    pix_load4(pix0mm, &src->rgbp);
    pix1mm = _mm_cvtsi32_si128(dst0);
    pix_combine_1x1B_1x1B(pix0mm, pix1mm);
    pix_unpack_1x2W(pix0mm, pix0mm);
    pix_expand_alpha_1x1W(a0mm, pix0mm);
    pix_fill_alpha_1x2W_lo(pix0mm);
    pix_expand_pixel_1x2W(a0mm, a0mm);
    pix_multiply_1x2W(pix0mm, pix0mm, a0mm);
    pix_expand_mask_1x1W(a0mm, msk0);
    pix_expand_pixel_1x2W(a0mm, a0mm);
    pix_negate_1x2W_lo(a0mm, a0mm);
    pix_multiply_1x2W(pix0mm, pix0mm, a0mm);
    pix_split_1x1W_1x1W(pix0mm, pix1mm);
    pix_adds_1x1W(pix0mm, pix0mm, pix1mm);
    pix_demultiply_1x1W(pix0mm, pix0mm);
    pix_pack_1x1W(pix0mm, pix0mm);
    pix_store4(dst, pix0mm);
  }
}

static void FOG_FASTCALL raster_argb32_span_solid_srcin_sse2(
  uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  uint32_t src0 = src->rgba;
  uint32_t src0a = src0 >> 24;

  src0 &= 0x00FFFFFF;

  __m128i src0mm;
  __m128i a0mm;

  src0mm = _mm_cvtsi32_si128(src0);
  pix_expand_pixel_1x4B(src0mm, src0mm);
  pix_zero_alpha_1x4B(src0mm);

  pix_expand_mask_1x1W(a0mm, src0a);
  pix_expand_pixel_1x2W(a0mm, a0mm);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    uint32_t dst0 = READ_32(dst);
    if (!isAlpha0x00(dst0))
    {
      ((uint32_t*)dst)[0] = src0 | (singlemul(src0a, dst0 >> 24) << 24);
    }

    dst += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm;

    pix_load16a(dst0mm, dst);
    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0x00000000)
    {
      dst0mm = _mm_srli_epi32(dst0mm, 24);
      pix_multiply_1x2W(dst0mm, dst0mm, a0mm);
      dst0mm = _mm_slli_epi32(dst0mm, 24);
      dst0mm = _mm_or_si128(dst0mm, src0mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

// znacka
static void FOG_FASTCALL raster_argb32_span_solid_a8_srcin_sse2(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  __m128i src0hi;
  __m128i src0mm;

  pix_load4(src0mm, &src->rgbp);

  src0hi = src0mm;
  src0hi = _mm_shuffle_epi32(src0hi, _MM_SHUFFLE(3, 3, 0, 3));
  pix_unpack_1x1W(src0mm, src0mm);
  pix_expand_pixel_1x1W(src0mm, src0mm);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    uint32_t msk0 = READ_8(msk);
    if (msk != 0x00)
    {
      uint32_t dst0 = READ_32(dst);
      if (!isAlpha0x00(dst0))
      {
        __m128i pix0mm;
        __m128i pix1mm;
        __m128i a0mm;

        pix_load4(pix0mm, dst);
        pix0mm = _mm_or_si128(pix0mm, src0hi);
        pix_unpack_1x2W(pix0mm, pix0mm);
        pix_expand_alpha_1x1W(a0mm, pix0mm);
        pix_fill_alpha_1x2W_lo(pix0mm);
        pix_expand_pixel_1x2W(a0mm, a0mm);
        pix_multiply_1x2W(pix0mm, pix0mm, a0mm);
        pix_expand_mask_1x1W(a0mm, msk0);
        pix_expand_pixel_1x2W(a0mm, a0mm);
        pix_negate_1x2W_lo(a0mm, a0mm);
        pix_multiply_1x2W(pix0mm, pix0mm, a0mm);
        pix_split_1x1W_1x1W(pix0mm, pix1mm);
        pix_adds_1x1W(pix0mm, pix0mm, pix1mm);
        pix_demultiply_1x1W(pix0mm, pix0mm);
        pix_pack_1x1W(pix0mm, pix0mm);
        pix_store4(dst, pix0mm);
      }
    }

    dst += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      __m128i dst0mm, dst1mm;
      __m128i a0mm, a1mm;
      __m128i ia0mm, ia1mm;

      pix_load16a(dst0mm, dst);
      uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
      if (dst0a != 0x00000000)
      {
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_alpha_2x2W(ia0mm, dst0mm, ia1mm, dst1mm);
        pix_fill_alpha_2x2W(dst0mm, dst1mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_multiply_2x2W(a0mm, a0mm, ia0mm, a1mm, a1mm, ia1mm);
        pix_subs_2x2W(ia0mm, ia0mm, a0mm, ia1mm, ia1mm, a1mm);
        pix_multiply_2x2W(a0mm, a0mm, src0mm, a1mm, a1mm, src0mm);
        pix_multiply_2x2W(dst0mm, dst0mm, ia0mm, dst1mm, dst1mm, ia1mm);
        pix_adds_2x2W(dst0mm, dst0mm, a0mm, dst1mm, dst1mm, a1mm);
        pix_demultiply_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_argb32_span_solid_a8_const_srcin_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL raster_argb32_span_composite_argb32_srcin_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL raster_argb32_span_composite_prgb32_srcin_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL raster_argb32_span_composite_rgb32_srcin_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL raster_argb32_span_composite_argb32_a8_srcin_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL raster_argb32_span_composite_prgb32_a8_srcin_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL raster_argb32_span_composite_rgb32_a8_srcin_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL raster_argb32_span_composite_argb32_a8_const_srcin_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL raster_argb32_span_composite_prgb32_a8_const_srcin_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL raster_argb32_span_composite_rgb32_a8_const_srcin_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
}

// ============================================================================
// [Fog::Raster - Raster - SrcIn - Prgb32]
// ============================================================================

static void FOG_FASTCALL raster_prgb32_pixel_srcin_sse2(
  uint8_t* dst, const Solid* src, const Closure* closure)
{
}

static void FOG_FASTCALL raster_prgb32_pixel_a8_srcin_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, const Closure* closure)
{
}

static void FOG_FASTCALL raster_prgb32_span_solid_srcin_sse2(
  uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL raster_prgb32_span_solid_a8_srcin_sse2(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL raster_prgb32_span_solid_a8_const_srcin_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_srcin_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_srcin_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb32_srcin_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_a8_srcin_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_a8_srcin_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb32_a8_srcin_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_a8_const_srcin_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_a8_const_srcin_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb32_a8_const_srcin_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
}

#endif











































































// ============================================================================
// [Fog::Raster - Raster - Clear - Argb32]
// ============================================================================

// forward declarations for macros
static void FOG_FASTCALL raster_prgb32_span_solid_clear_sse2(
  uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure);

#define raster_argb32_pixel_clear_sse2 raster_prgb32_pixel_clear_sse2

static void FOG_FASTCALL raster_argb32_pixel_a8_clear_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, const Closure* closure)
{
  FOG_UNUSED(src);

  if (msk0 != 0x00)
  {
    if (msk0 == 0xFF)
      ((uint32_t*)dst)[0] = 0x00000000;
    else
      ((uint32_t*)dst)[0] = alphamul(READ_32(dst), alphaneg(msk0));
  }
}

#define raster_argb32_span_solid_clear_sse2 raster_prgb32_span_solid_clear_sse2

static void FOG_FASTCALL raster_argb32_span_solid_a8_clear_sse2(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  FOG_UNUSED(src);

  BLIT_SSE2_32x4_INIT(dst, w)

  __m128i mmzero = _mm_setzero_si128();
  __m128i colormask = Mask_00FFFFFF00FFFFFF_00FFFFFF00FFFFFF;
  __m128i submask = Mask_0000000100000001_0000000100000001;

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      if (msk0 == 0xFF)
        pix_store4(dst, mmzero);
      else
        ((uint32_t*)dst)[0] = alphamul(READ_32(dst), alphaneg(msk0));
    }

    dst += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      if (msk0 == 0xFFFFFFFF)
      {
        pix_store16a(dst, mmzero);
      }
      else
      {
        __m128i dst0mm;
        __m128i bck0mm;
        __m128i m0mm;

        pix_load16a(dst0mm, dst);
        pix_unpack_1x1D(m0mm, ~msk0);
        bck0mm = dst0mm;
        dst0mm = _mm_srli_epi32(dst0mm, 24);
        pix_multiply_1x1W(dst0mm, dst0mm, m0mm);

        // If alpha in some pixel is 0, this will clear all colors in it.
        m0mm = _mm_sub_epi32(dst0mm, submask);
        m0mm = _mm_srli_epi32(m0mm, 8);
        m0mm = _mm_andnot_si128(m0mm, bck0mm);
        m0mm = _mm_and_si128(m0mm, colormask);

        dst0mm = _mm_slli_epi32(dst0mm, 24);
        dst0mm = _mm_or_si128(dst0mm, m0mm);

        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_argb32_span_solid_a8_const_clear_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_argb32_span_solid_clear_sse2(dst, src, w, closure);
    return;
  }
  if (FOG_UNLIKELY(msk0 == 0x00)) return;

  BLIT_SSE2_32x4_INIT(dst, w)

  msk0 = alphaneg(msk0);

  __m128i m0mm = _mm_cvtsi32_si128(msk0);
  __m128i colormask = Mask_00FFFFFF00FFFFFF_00FFFFFF00FFFFFF;
  pix_expand_pixel_1x4B(m0mm, m0mm);

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    ((uint32_t*)dst)[0] = alphamul(READ_32(dst), msk0);

    dst += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm;
    __m128i bck0mm;

    pix_load16a(dst0mm, dst);
    bck0mm = dst0mm;
    dst0mm = _mm_srli_epi32(dst0mm, 24);
    pix_multiply_1x1W(dst0mm, dst0mm, m0mm);
    dst0mm = _mm_slli_epi32(dst0mm, 24);
    bck0mm = _mm_and_si128(bck0mm, colormask);
    dst0mm = _mm_or_si128(dst0mm, bck0mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

// ============================================================================
// [Fog::Raster - Raster - Clear - Prgb32]
// ============================================================================

static void FOG_FASTCALL raster_prgb32_pixel_clear_sse2(
  uint8_t* dst, const Solid* src, const Closure* closure)
{
  FOG_UNUSED(src);

  ((uint32_t*)dst)[0] = 0x00000000;
}

static void FOG_FASTCALL raster_prgb32_pixel_a8_clear_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, const Closure* closure)
{
  FOG_UNUSED(src);

  if (msk0 != 0x00)
  {
    __m128i dst0mm;
    __m128i a0mm;

    pix_load4(dst0mm, dst);
    pix_expand_mask_1x1W(a0mm, msk0);
    pix_negate_1x1W(a0mm, a0mm);
    pix_multiply_1x1W(dst0mm, dst0mm, a0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);
  }
}

static void FOG_FASTCALL raster_prgb32_span_solid_clear_sse2(
  uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure)
{
  FOG_UNUSED(src);

  BLIT_SSE2_32x4_INIT(dst, w)

  __m128i mmzero = _mm_setzero_si128();

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    pix_store4(dst, mmzero);
    dst += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    pix_store16a(dst, mmzero);
    dst += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_solid_a8_clear_sse2(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  FOG_UNUSED(src);

  BLIT_SSE2_32x4_INIT(dst, w)

  __m128i mmzero = _mm_setzero_si128();

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      if (msk0 == 0xFF)
      {
        pix_store4(dst, mmzero);
      }
      else
      {
        __m128i dst0mm;
        __m128i a0mm;

        pix_load4(dst0mm, dst);
        pix_expand_mask_1x1W(a0mm, msk0);
        pix_negate_1x1W(a0mm, a0mm);
        pix_multiply_1x1W(dst0mm, dst0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      if (msk0 == 0xFFFFFFFF)
      {
        pix_store16a(dst, mmzero);
      }
      else
      {
        __m128i dst0mm, dst1mm;
        __m128i a0mm, a1mm;

        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_negate_2x2W(a0mm, a0mm, a1mm, a1mm);
        pix_multiply_2x2W(dst0mm, dst0mm, a0mm, dst1mm, dst1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_solid_a8_const_clear_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_prgb32_span_solid_clear_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x1W(m0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;

    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_multiply_1x1W(dst0mm, dst0mm, m0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_multiply_2x2W(dst0mm, dst0mm, m0mm, dst1mm, dst1mm, m0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

// ============================================================================
// [Fog::Raster - Raster - Clear - Rgb32]
// ============================================================================

#define raster_rgb32_pixel_clear_sse2 raster_prgb32_pixel_clear_sse2

static void FOG_FASTCALL raster_rgb32_pixel_a8_clear_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, const Closure* closure)
{
  FOG_UNUSED(src);

  if (msk0 != 0x00)
  {
    __m128i dst0mm;
    __m128i a0mm;

    pix_load4(dst0mm, dst);
    pix_expand_mask_1x1W(a0mm, msk0);
    pix_negate_1x1W(a0mm, a0mm);
    pix_multiply_1x1W(dst0mm, dst0mm, a0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
    pix_store4(dst, dst0mm);
  }
}

#define raster_rgb32_span_solid_clear_sse2 raster_prgb32_span_solid_clear_sse2

static void FOG_FASTCALL raster_rgb32_span_solid_a8_clear_sse2(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  FOG_UNUSED(src);

  BLIT_SSE2_32x4_INIT(dst, w)

  __m128i mmzero = _mm_setzero_si128();

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      if (msk0 == 0xFF)
      {
        pix_store4(dst, mmzero);
      }
      else
      {
        __m128i dst0mm;
        __m128i a0mm;

        pix_load4(dst0mm, dst);
        pix_expand_mask_1x1W(a0mm, msk0);
        pix_negate_1x1W(a0mm, a0mm);
        pix_multiply_1x1W(dst0mm, dst0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      if (msk0 == 0xFFFFFFFF)
      {
        pix_store16a(dst, mmzero);
      }
      else
      {
        __m128i dst0mm, dst1mm;
        __m128i a0mm, a1mm;

        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_negate_2x2W(a0mm, a0mm, a1mm, a1mm);
        pix_multiply_2x2W(dst0mm, dst0mm, a0mm, dst1mm, dst1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_rgb32_span_solid_a8_const_clear_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_prgb32_span_solid_clear_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x1W(m0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;

    pix_load4(dst0mm, dst);
    pix_multiply_1x1W(dst0mm, dst0mm, m0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
    pix_store4(dst, dst0mm);

    dst += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_multiply_2x2W(dst0mm, dst0mm, m0mm, dst1mm, dst1mm, m0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    dst0mm = _mm_or_si128(dst0mm, Mask_FF000000FF000000_FF000000FF000000);
    pix_store16a(dst, dst0mm);

    dst += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

// ============================================================================
// [Fog::Raster - Raster - Clear - A8]
// ============================================================================

static void FOG_FASTCALL raster_a8_pixel_clear_sse2(
  uint8_t* dst, const Solid* src, const Closure* closure)
{
  FOG_UNUSED(src);

  dst[0] = 0x00;
}

static void FOG_FASTCALL raster_a8_pixel_a8_clear_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, const Closure* closure)
{
  FOG_UNUSED(src);

  if (msk0 != 0x00)
  {
    dst[0] = singlemul(dst[0], singleneg(msk0));
  }
}

static void FOG_FASTCALL raster_a8_span_solid_clear_sse2(
  uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure)
{
  FOG_UNUSED(src);

  BLIT_SSE2_8x16_INIT(dst, w)

  __m128i mmzero = _mm_setzero_si128();

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    dst[0] = 0x00;
    dst += 1;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    pix_store16a(dst, mmzero);
    dst += 16;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

static void FOG_FASTCALL raster_a8_span_solid_a8_clear_sse2(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  FOG_UNUSED(src);

  BLIT_SSE2_8x16_INIT(dst, w)

  __m128i mmzero = _mm_setzero_si128();

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      dst[0] = singlemul(dst[0], singleneg(msk0));
    }

    dst += 1;
    msk += 1;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    uint32_t msk0IsZero;
    uint32_t msk0IsFull;

    __m128i dst0mm, dst1mm;
    __m128i msk0mm, msk1mm;
    pix_load16u(msk0mm, msk);
    pix_analyze_mask_16B(msk0IsZero, msk0IsFull, msk0mm);

    if (msk0IsZero != 0xFFFF)
    {
      if (msk0IsFull == 0xFFFF)
      {
        pix_store16a(dst, mmzero);
      }
      else
      {
        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_unpack_2x2W(msk0mm, msk1mm, msk0mm);
        pix_negate_2x2W(msk0mm, msk0mm, msk1mm, msk1mm);
        pix_multiply_2x2W(dst0mm, dst0mm, msk0mm, dst1mm, dst1mm, msk1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    msk += 16;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

static void FOG_FASTCALL raster_a8_span_solid_a8_const_clear_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_a8_span_solid_clear_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x1W(m0mm, m0mm);

  msk0 = singleneg(msk0);

  BLIT_SSE2_8x16_INIT(dst, w)

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    dst[0] = singlemul(dst[0], msk0);

    dst += 1;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_multiply_2x2W(dst0mm, dst0mm, m0mm, dst1mm, dst1mm, m0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
  BLIT_SSE2_8x16_LARGE_END(blt)
}















































































// ============================================================================
// [Fog::Raster - Raster - Add - Prgb32]
// ============================================================================

static void FOG_FASTCALL raster_prgb32_pixel_add_sse2(
  uint8_t* dst, const Solid* src, const Closure* closure)
{
  if (src->rgbp == 0x00000000) return;

  __m128i dst0mm;
  __m128i src0mm;

  pix_load4(src0mm, &src->rgbp);
  pix_load4(dst0mm, dst);
  pix_adds_1x1B(dst0mm, dst0mm, src0mm);
  pix_store4(dst, dst0mm);
}

static void FOG_FASTCALL raster_prgb32_pixel_a8_add_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, const Closure* closure)
{
  if (src->rgbp == 0x00000000) return;

  if (msk0 != 0x00)
  {
    __m128i dst0mm;
    __m128i src0mm;
    __m128i a0mm;

    pix_load4(src0mm, &src->rgbp);
    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_expand_mask_1x1W(a0mm, msk0);
    pix_multiply_1x1W(src0mm, src0mm, a0mm);
    pix_pack_1x1W(src0mm, src0mm);
    pix_adds_1x1B(dst0mm, dst0mm, src0mm);
    pix_store4(dst, dst0mm);
  }
}

static void FOG_FASTCALL raster_prgb32_span_solid_add_sse2(
  uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure)
{
  if (src->rgbp == 0x00000000) return;

  __m128i src0mm;

  pix_load4(src0mm, &src->rgbp);
  pix_expand_pixel_1x4B(src0mm, src0mm);

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;

    pix_load4(dst0mm, dst);
    pix_adds_1x1B(dst0mm, dst0mm, src0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm;

    pix_load16a(dst0mm, dst);
    pix_adds_1x4B(dst0mm, dst0mm, src0mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_solid_a8_add_sse2(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  if (src->rgbp == 0x00000000) return;

  __m128i src0orig;
  __m128i src0mm;

  pix_load4(src0orig, &src->rgbp);
  pix_expand_pixel_1x4B(src0orig, src0orig);
  pix_unpack_1x2W(src0mm, src0orig);

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      pix_load4(dst0mm, dst);
      if (msk0 == 0xFF)
      {
        pix_adds_1x1B(dst0mm, dst0mm, src0orig);
        pix_store4(dst, dst0mm);
      }
      else
      {
        pix_expand_mask_1x1W(a0mm, msk0);
        pix_multiply_1x1W(a0mm, a0mm, src0mm);
        pix_pack_1x1W(a0mm, a0mm);
        pix_adds_1x1B(dst0mm, dst0mm, a0mm);
        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      pix_load16a(dst0mm, dst);
      if (msk0 != 0xFFFFFFFF)
      {
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_multiply_2x2W(a0mm, a0mm, src0mm, a1mm, a1mm, src0mm);
        pix_pack_2x2W(a0mm, a0mm, a1mm);
        pix_adds_1x4B(dst0mm, dst0mm, a0mm);
        pix_store16a(dst, dst0mm);
      }
      else
      {
        pix_adds_1x4B(dst0mm, dst0mm, src0orig);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_solid_a8_const_add_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (src->rgbp == 0x00000000) return;
  if (msk0 == 0x00) return;

  __m128i src0mm;
  __m128i a0mm;

  pix_load4(src0mm, &src->rgbp);
  pix_expand_mask_1x1W(a0mm, msk0);
  pix_multiply_1x1W(src0mm, src0mm, a0mm);
  pix_pack_1x1W(src0mm, src0mm);
  pix_expand_pixel_1x4B(src0mm, src0mm);

  uint32_t src0 = _mm_cvtsi128_si32(src0mm);
  if (src0 == 0x00000000) return;

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;

    pix_load4(dst0mm, dst);
    pix_adds_1x1B(dst0mm, dst0mm, src0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm;

    pix_load16a(dst0mm, dst);
    pix_adds_1x4B(dst0mm, dst0mm, src0mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_add_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;

    pix_load4(dst0mm, dst);
    pix_load4(src0mm, src);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_premultiply_1x1W(src0mm, src0mm);
    pix_pack_1x1W(src0mm, src0mm);
    pix_adds_1x1B(dst0mm, dst0mm, src0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm, src1mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
    pix_pack_2x2W(src0mm, src0mm, src1mm);
    pix_adds_1x4B(dst0mm, dst0mm, src0mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_add_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    pix_load4(dst0mm, dst);
    pix_load4(src0mm, src);
    pix_adds_1x1B(dst0mm, dst0mm, src0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);
    pix_adds_1x4B(dst0mm, dst0mm, src0mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb32_add_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  __m128i amask = Mask_FF000000FF000000_FF000000FF000000;

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;

    pix_load4(dst0mm, dst);
    pix_load4(src0mm, src);
    src0mm = _mm_or_si128(src0mm, amask);
    pix_adds_1x1B(dst0mm, dst0mm, src0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);
    src0mm = _mm_or_si128(src0mm, amask);
    pix_adds_1x4B(dst0mm, dst0mm, src0mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb24_add_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;

    pix_load4(dst0mm, dst);
    src0mm = _mm_cvtsi32_si128(PixFmt_RGB24::fetch(src) | 0xFF000000);
    pix_adds_1x1B(dst0mm, dst0mm, src0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 3;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm, src1mm;

    pix_load16a(dst0mm, dst);
    pix_fetch_rgb24_2x2W(src0mm, src1mm, src);
    pix_pack_2x2W(src0mm, src0mm, src1mm);
    pix_adds_1x4B(dst0mm, dst0mm, src0mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 12;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_a8_add_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;

    pix_load_1xA8(src0mm, src);
    pix_load4(dst0mm, dst);
    pix_adds_1x1B(dst0mm, dst0mm, src0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;

    pix_load_4xA8(src0mm, src);
    pix_load16a(dst0mm, dst);
    pix_adds_1x4B(dst0mm, dst0mm, src0mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_i8_add_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const Rgba* srcPal = closure->srcPalette + Palette::IndexPRGB32;

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;

    pix_load_1xI8(src0mm, src, srcPal);
    pix_load4(dst0mm, dst);
    pix_adds_1x1B(dst0mm, dst0mm, src0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;

    pix_load_4xI8(src0mm, src, srcPal);
    pix_load16a(dst0mm, dst);
    pix_adds_1x4B(dst0mm, dst0mm, src0mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_a8_add_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      pix_load4(dst0mm, dst);
      pix_load4(src0mm, src);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_premultiply_1x1W(src0mm, src0mm);
      pix_expand_mask_1x1W(a0mm, msk0);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_pack_1x1W(src0mm, src0mm);
      pix_adds_1x1B(dst0mm, dst0mm, src0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      pix_load16a(dst0mm, dst);
      pix_load16u(src0mm, src);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
      pix_expand_mask_2x2W(a0mm, a1mm, msk0);
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
      pix_pack_2x2W(src0mm, src0mm, src1mm);
      pix_adds_1x4B(dst0mm, dst0mm, src0mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_a8_add_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      pix_load4(dst0mm, dst);
      pix_load4(src0mm, src);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_expand_mask_1x1W(a0mm, msk0);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_pack_1x1W(src0mm, src0mm);
      pix_adds_1x1B(dst0mm, dst0mm, src0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      pix_load16a(dst0mm, dst);
      pix_load16u(src0mm, src);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_expand_mask_2x2W(a0mm, a1mm, msk0);
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
      pix_pack_2x2W(src0mm, src0mm, src1mm);
      pix_adds_1x4B(dst0mm, dst0mm, src0mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb32_a8_add_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  __m128i amask = Mask_FF000000FF000000_FF000000FF000000;

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      pix_load4(dst0mm, dst);
      pix_load4(src0mm, src);
      src0mm = _mm_or_si128(src0mm, amask);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_expand_mask_1x1W(a0mm, msk0);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_pack_1x1W(src0mm, src0mm);
      pix_adds_1x1B(dst0mm, dst0mm, src0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      pix_load16a(dst0mm, dst);
      pix_load16u(src0mm, src);
      src0mm = _mm_or_si128(src0mm, amask);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_expand_mask_2x2W(a0mm, a1mm, msk0);
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
      pix_pack_2x2W(src0mm, src0mm, src1mm);
      pix_adds_1x4B(dst0mm, dst0mm, src0mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb24_a8_add_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      pix_load4(dst0mm, dst);
      src0mm = _mm_cvtsi32_si128(PixFmt_RGB24::fetch(src) | 0xFF000000);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_expand_mask_1x1W(a0mm, msk0);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_pack_1x1W(src0mm, src0mm);
      pix_adds_1x1B(dst0mm, dst0mm, src0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 3;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      pix_load16a(dst0mm, dst);
      pix_fetch_rgb24_2x2W(src0mm, src1mm, src);
      pix_expand_mask_2x2W(a0mm, a1mm, msk0);
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
      pix_pack_2x2W(src0mm, src0mm, src1mm);
      pix_adds_1x4B(dst0mm, dst0mm, src0mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 12;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_i8_a8_add_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const Rgba* srcPal = closure->srcPalette + Palette::IndexPRGB32;

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;
    __m128i a0mm;

    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      pix_load4(dst0mm, dst);
      pix_load_1xI8(src0mm, src, srcPal);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_expand_mask_1x1W(a0mm, msk0);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_pack_1x1W(src0mm, src0mm);
      pix_adds_1x1B(dst0mm, dst0mm, src0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 1;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      pix_load16a(dst0mm, dst);
      pix_load_4xI8(src0mm, src, srcPal);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_expand_mask_2x2W(a0mm, a1mm, msk0);
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
      pix_pack_2x2W(src0mm, src0mm, src1mm);
      pix_adds_1x4B(dst0mm, dst0mm, src0mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 4;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_a8_a8_add_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;
    __m128i msk0mm;

    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      pix_load4(dst0mm, dst);
      pix_load_1xA8(src0mm, src);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_expand_mask_1x1W(msk0mm, msk0);
      pix_multiply_1x1W(src0mm, src0mm, msk0mm);
      pix_pack_1x1W(src0mm, src0mm);
      pix_adds_1x1B(dst0mm, dst0mm, src0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 1;
    msk += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;
    __m128i msk0mm;

    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0x00000000)
    {
      pix_load16a(dst0mm, dst);
      pix_load4(src0mm, src);

      pix_unpack_1x1W(src0mm, src0mm);
      pix_unpack_1x1W(msk0mm, msk0);
      pix_multiply_1x1W(src0mm, src0mm, msk0mm);
      src0mm = _mm_unpacklo_epi16(src0mm, src0mm);
      src0mm = _mm_slli_epi32(src0mm, 24);
      pix_adds_1x4B(dst0mm, dst0mm, src0mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 4;
    msk += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_a8_const_add_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (msk0 == 0xFF)
  {
    raster_prgb32_span_composite_argb32_add_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;

    pix_load4(dst0mm, dst);
    pix_load4(src0mm, src);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_premultiply_1x1W(src0mm, src0mm);
    pix_multiply_1x1W(src0mm, src0mm, m0mm);
    pix_pack_1x1W(src0mm, src0mm);
    pix_adds_1x1B(dst0mm, dst0mm, src0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm, src1mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
    pix_multiply_2x2W(src0mm, src0mm, m0mm, src1mm, src1mm, m0mm);
    pix_pack_2x2W(src0mm, src0mm, src1mm);
    pix_adds_1x4B(dst0mm, dst0mm, src0mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_a8_const_add_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (msk0 == 0xFF)
  {
    raster_prgb32_span_composite_prgb32_add_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;

    pix_load4(dst0mm, dst);
    pix_load4(src0mm, src);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_multiply_1x1W(src0mm, src0mm, m0mm);
    pix_pack_1x1W(src0mm, src0mm);
    pix_adds_1x1B(dst0mm, dst0mm, src0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm, src1mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_multiply_2x2W(src0mm, src0mm, m0mm, src1mm, src1mm, m0mm);
    pix_pack_2x2W(src0mm, src0mm, src1mm);
    pix_adds_1x4B(dst0mm, dst0mm, src0mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb32_a8_const_add_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (msk0 == 0xFF)
  {
    raster_prgb32_span_composite_rgb32_add_sse2(dst, src, w, closure);
    return;
  }

  __m128i amask = Mask_FF000000FF000000_FF000000FF000000;
  __m128i m0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;

    pix_load4(dst0mm, dst);
    pix_load4(src0mm, src);
    src0mm = _mm_or_si128(src0mm, amask);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_multiply_1x1W(src0mm, src0mm, m0mm);
    pix_pack_1x1W(src0mm, src0mm);
    pix_adds_1x1B(dst0mm, dst0mm, src0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm, src1mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);
    src0mm = _mm_or_si128(src0mm, amask);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_multiply_2x2W(src0mm, src0mm, m0mm, src1mm, src1mm, m0mm);
    pix_pack_2x2W(src0mm, src0mm, src1mm);
    pix_adds_1x4B(dst0mm, dst0mm, src0mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb24_a8_const_add_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (msk0 == 0xFF)
  {
    raster_prgb32_span_composite_rgb24_add_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;

    pix_load4(dst0mm, dst);
    src0mm = _mm_cvtsi32_si128(PixFmt_RGB24::fetch(src) | 0xFF000000);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_multiply_1x1W(src0mm, src0mm, m0mm);
    pix_pack_1x1W(src0mm, src0mm);
    pix_adds_1x1B(dst0mm, dst0mm, src0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 3;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm, src1mm;

    pix_load16a(dst0mm, dst);
    pix_fetch_rgb24_2x2W(src0mm, src1mm, src);
    pix_multiply_2x2W(src0mm, src0mm, m0mm, src1mm, src1mm, m0mm);
    pix_pack_2x2W(src0mm, src0mm, src1mm);
    pix_adds_1x4B(dst0mm, dst0mm, src0mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 12;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_a8_a8_const_add_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (msk0 == 0xFF)
  {
    raster_prgb32_span_composite_a8_add_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;

    pix_load4(dst0mm, dst);
    pix_load_1xA8(src0mm, src);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_multiply_1x1W(src0mm, src0mm, m0mm);
    pix_pack_1x1W(src0mm, src0mm);
    pix_adds_1x1B(dst0mm, dst0mm, src0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;

    pix_load16a(dst0mm, dst);
    pix_load_4xA8(src0mm, src);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_multiply_1x1W(src0mm, src0mm, m0mm);
    src0mm = _mm_unpacklo_epi16(src0mm, src0mm);
    src0mm = _mm_slli_epi32(src0mm, 24);
    pix_adds_1x4B(dst0mm, dst0mm, src0mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_i8_a8_const_add_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  const Rgba* srcPal = closure->srcPalette + Palette::IndexPRGB32;

  if (msk0 == 0xFF)
  {
    raster_prgb32_span_composite_i8_add_sse2(dst, src, w, closure);
    return;
  }

  __m128i m0mm;
  pix_expand_mask_1x1W(m0mm, msk0);
  pix_expand_pixel_1x2W(m0mm, m0mm);

  BLIT_SSE2_32x4_INIT(dst, w)

  BLIT_SSE2_32x4_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;

    pix_load4(dst0mm, dst);
    pix_load_1xI8(src0mm, src, srcPal);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_multiply_1x1W(src0mm, src0mm, m0mm);
    pix_pack_1x1W(src0mm, src0mm);
    pix_adds_1x1B(dst0mm, dst0mm, src0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 1;
  BLIT_SSE2_32x4_SMALL_END(blt)

  BLIT_SSE2_32x4_LARGE_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm, src1mm;

    pix_load16a(dst0mm, dst);
    pix_load_4xI8(src0mm, src, srcPal);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_multiply_2x2W(src0mm, src0mm, m0mm, src1mm, src1mm, m0mm);
    pix_pack_2x2W(src0mm, src0mm, src1mm);
    pix_adds_1x4B(dst0mm, dst0mm, src0mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 4;
  BLIT_SSE2_32x4_LARGE_END(blt)
}

// ============================================================================
// [Fog::Raster - Raster - Add - Rgb32]
// ============================================================================

#define raster_rgb32_pixel_add_sse2 raster_prgb32_pixel_add_sse2
#define raster_rgb32_pixel_a8_add_sse2 raster_prgb32_pixel_a8_add_sse2
#define raster_rgb32_span_solid_add_sse2 raster_prgb32_span_solid_add_sse2
#define raster_rgb32_span_solid_a8_add_sse2 raster_prgb32_span_solid_a8_add_sse2
#define raster_rgb32_span_solid_a8_const_add_sse2 raster_prgb32_span_solid_a8_const_add_sse2

#define raster_rgb32_span_composite_argb32_add_sse2 raster_prgb32_span_composite_argb32_add_sse2
#define raster_rgb32_span_composite_prgb32_add_sse2 raster_prgb32_span_composite_prgb32_add_sse2
#define raster_rgb32_span_composite_rgb32_add_sse2 raster_prgb32_span_composite_rgb32_add_sse2
#define raster_rgb32_span_composite_rgb24_add_sse2 raster_prgb32_span_composite_rgb24_add_sse2
#define raster_rgb32_span_composite_i8_add_sse2 raster_prgb32_span_composite_i8_add_sse2

#define raster_rgb32_span_composite_argb32_a8_add_sse2 raster_prgb32_span_composite_argb32_a8_add_sse2
#define raster_rgb32_span_composite_prgb32_a8_add_sse2 raster_prgb32_span_composite_prgb32_a8_add_sse2
#define raster_rgb32_span_composite_rgb32_a8_add_sse2 raster_prgb32_span_composite_rgb32_a8_add_sse2
#define raster_rgb32_span_composite_rgb24_a8_add_sse2 raster_prgb32_span_composite_rgb24_a8_add_sse2
#define raster_rgb32_span_composite_i8_a8_add_sse2 raster_prgb32_span_composite_i8_a8_add_sse2

#define raster_rgb32_span_composite_argb32_a8_const_add_sse2 raster_prgb32_span_composite_argb32_a8_const_add_sse2
#define raster_rgb32_span_composite_prgb32_a8_const_add_sse2 raster_prgb32_span_composite_prgb32_a8_const_add_sse2
#define raster_rgb32_span_composite_rgb32_a8_const_add_sse2 raster_prgb32_span_composite_rgb32_a8_const_add_sse2
#define raster_rgb32_span_composite_rgb24_a8_const_add_sse2 raster_prgb32_span_composite_rgb24_a8_const_add_sse2
#define raster_rgb32_span_composite_i8_a8_const_add_sse2 raster_prgb32_span_composite_i8_a8_const_add_sse2

// ============================================================================
// [Fog::Raster - Raster - A8 - Add]
// ============================================================================

static void FOG_FASTCALL raster_a8_pixel_add_sse2(
  uint8_t* dst, const Solid* src, const Closure* closure)
{
  uint32_t src0 = src->rgba >> 24;

  if (src0 != 0x00)
  {
    dst[0] = (uint8_t)singleaddsat(dst[0], src0);
  }
}

static void FOG_FASTCALL raster_a8_pixel_a8_add_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, const Closure* closure)
{
  uint32_t src0 = singlemul(src->rgba >> 24, msk0);

  if (src0 != 0x00)
  {
    dst[0] = (uint8_t)singleaddsat(dst[0], src0);
  }
}

static void FOG_FASTCALL raster_a8_span_solid_add_sse2(
  uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure)
{
  uint32_t src0 = src->rgba >> 24;

  if (src0 == 0xFF)
  {
    raster_a8_span_solid_src_sse2(dst, src, w, closure);
    return;
  }
  if (src0 == 0x00) return;

  __m128i src0orig;
  pix_expand_a8_1x4D(src0orig, src0);

  BLIT_SSE2_8x16_INIT(dst, w)

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    dst[0] = (uint8_t)singleaddsat(dst[0], src0);
    dst += 1;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    __m128i dst0mm;

    pix_load16a(dst0mm, dst);
    pix_adds_1x4B(dst0mm, dst0mm, src0orig);
    pix_store16a(dst, dst0mm);

    dst += 16;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

static void FOG_FASTCALL raster_a8_span_solid_a8_add_sse2(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  uint32_t src0 = src->rgba >> 24;

  if (src0 == 0xFF)
  {
    raster_a8_span_solid_src_sse2(dst, src, w, closure);
    return;
  }
  if (src0 == 0x00) return;

  __m128i src0orig;
  __m128i src0mm;

  pix_expand_a8_1x4D(src0orig, src0);
  pix_unpack_1x2W(src0mm, src0orig);

  BLIT_SSE2_8x16_INIT(dst, w)

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      dst[0] = singleaddsat(dst[0], singlemul(src0, msk0));
    }

    dst += 1;
    msk += 1;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    uint32_t msk0IsZero;
    uint32_t msk0IsFull;

    __m128i dst0mm, dst1mm;
    __m128i msk0mm, msk1mm;

    pix_load16u(msk0mm, msk);
    pix_analyze_mask_16B(msk0IsZero, msk0IsFull, msk0mm);

    if (msk0IsZero != 0xFFFF)
    {
      if (msk0IsFull == 0xFFFF)
      {
        pix_load16a(dst0mm, dst);
        pix_adds_1x4B(dst0mm, dst0mm, src0orig);
        pix_store16a(dst, dst0mm);
      }
      else
      {
        pix_load16a(dst0mm, dst);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_unpack_2x2W(msk0mm, msk1mm, msk0mm);
        pix_multiply_2x2W(msk0mm, msk0mm, src0mm, msk1mm, msk1mm, src0mm);
        pix_adds_2x2W(dst0mm, dst0mm, msk0mm, dst1mm, dst1mm, msk1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    msk += 16;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

static void FOG_FASTCALL raster_a8_span_solid_a8_const_add_sse2(
  uint8_t* dst, const Solid* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (msk0 == 0xFF)
  {
    raster_a8_span_solid_add_sse2(dst, src, w, closure);
    return;
  }

  uint32_t src0 = singlemul(src->rgba >> 24, msk0);
  if (src0 == 0x00) return;

  __m128i src0orig;
  pix_expand_a8_1x4D(src0orig, src0);

  BLIT_SSE2_8x16_INIT(dst, w)

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    dst[0] = singleaddsat(dst[0], src0);

    dst += 1;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    __m128i dst0mm;

    pix_load16a(dst0mm, dst);
    pix_adds_1x4B(dst0mm, dst0mm, src0orig);
    pix_store16a(dst, dst0mm);

    dst += 16;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

static void FOG_FASTCALL raster_a8_span_composite_axxx32_add_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_8x16_INIT(dst, w);

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    uint32_t src0 = src[RGB32_AByte];
    if (src0 != 0x00)
    {
      dst[0] = (uint8_t)(singleaddsat(dst[0], src0));
    }

    dst += 1;
    src += 4;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    uint32_t src0IsZero;

    __m128i src0mm, src1mm;
    __m128i dst0mm;

    pix_load_and_unpack_axxx32_64B(src0mm, src1mm, src);
    pix_pack_2x2W(src0mm, src0mm, src1mm);

    pix_analyze_mask_16B_zero(src0IsZero, src0mm);
    if (src0IsZero != 0xFFFF)
    {
      pix_load16a(dst0mm, dst);
      pix_adds_1x4B(dst0mm, dst0mm, src0mm);
      pix_store16a(dst, src0mm);
    }

    dst += 16;
    src += 64;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

#define raster_a8_span_composite_argb32_add_sse2 raster_a8_span_composite_axxx32_add_sse2
#define raster_a8_span_composite_prgb32_add_sse2 raster_a8_span_composite_axxx32_add_sse2
#define raster_a8_span_composite_rgb32_add_sse2 raster_a8_span_composite_0xff_src_sse2
#define raster_a8_span_composite_rgb24_add_sse2 raster_a8_span_composite_0xff_src_sse2

static void FOG_FASTCALL raster_a8_span_composite_a8_add_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_8x16_INIT(dst, w);

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    uint32_t src0 = src[0];
    if (src0 != 0x00)
    {
      dst[0] = (uint8_t)(singleaddsat(dst[0], src0));
    }

    dst += 1;
    src += 1;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    uint32_t src0IsZero;

    __m128i src0mm;
    __m128i dst0mm;

    pix_load16a(src0mm, src);
    pix_analyze_mask_16B_zero(src0IsZero, src0mm);
    if (src0IsZero != 0xFFFF)
    {
      pix_adds_1x4B(dst0mm, dst0mm, src0mm);
      pix_store16a(dst, src0mm);
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

static void FOG_FASTCALL raster_a8_span_composite_i8_add_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const Rgba* srcPal = closure->srcPalette;

  for (sysint_t i = w; i; i--)
  {
    dst[0] = singleaddsat(dst[0], srcPal[src[0]].a);

    dst += 1;
    src += 1;
  }
}

static void FOG_FASTCALL raster_a8_span_composite_axxx32_a8_add_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_8x16_INIT(dst, w);

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      uint32_t src0 = singlemul(src[RGB32_AByte], msk0);
      dst[0] = (uint8_t)singleaddsat(dst[0], src0);
    }

    dst += 1;
    src += 4;
    msk += 1;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    uint32_t msk0IsZero;
    uint32_t src0IsZero;

    __m128i src0mm, src1mm;
    __m128i msk0mm, msk1mm;

    pix_load16u(msk0mm, msk);
    pix_analyze_mask_16B_zero(msk0IsZero, msk0mm);

    pix_load_and_unpack_axxx32_64B(src0mm, src1mm, src);
    pix_pack_2x2W(src0mm, src0mm, src1mm);
    pix_analyze_mask_16B_zero(src0IsZero, src0mm);

    if ((msk0IsZero | src0IsZero) != 0xFFFF)
    {
      __m128i dst0mm, dst1mm;

      pix_load16a(dst0mm, dst);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_unpack_2x2W(msk0mm, msk1mm, msk0mm);
      pix_multiply_2x2W(src0mm, src0mm, msk0mm, src1mm, src1mm, msk1mm);
      pix_adds_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 64;
    msk += 16;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

#define raster_a8_span_composite_argb32_a8_add_sse2 raster_a8_span_composite_axxx32_a8_add_sse2
#define raster_a8_span_composite_prgb32_a8_add_sse2 raster_a8_span_composite_axxx32_a8_add_sse2
#define raster_a8_span_composite_rgb32_a8_add_sse2 raster_a8_span_composite_0xff_a8_src_sse2
#define raster_a8_span_composite_rgb24_a8_add_sse2 raster_a8_span_composite_0xff_a8_src_sse2

static void FOG_FASTCALL raster_a8_span_composite_a8_a8_add_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  BLIT_SSE2_8x16_INIT(dst, w);

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      uint32_t src0 = singlemul(src[0], msk0);
      dst[0] = (uint8_t)singleaddsat(dst[0], src0);
    }

    dst += 1;
    src += 1;
    msk += 1;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    uint32_t msk0IsZero;
    uint32_t src0IsZero;

    __m128i src0mm, src1mm;
    __m128i msk0mm, msk1mm;

    pix_load16u(msk0mm, msk);
    pix_load16u(src0mm, src);

    pix_analyze_mask_16B_zero(msk0IsZero, msk0mm);
    pix_analyze_mask_16B_zero(src0IsZero, src0mm);

    if ((msk0IsZero | src0IsZero) != 0xFFFF)
    {
      __m128i dst0mm, dst1mm;

      pix_load16a(dst0mm, dst);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_unpack_2x2W(msk0mm, msk1mm, msk0mm);
      pix_multiply_2x2W(src0mm, src0mm, msk0mm, src1mm, src1mm, msk1mm);
      pix_adds_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
    msk += 16;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

static void FOG_FASTCALL raster_a8_span_composite_i8_a8_add_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure)
{
  const Rgba* srcPal = closure->srcPalette;

  for (sysint_t i = w; i; i--)
  {
    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0x00)
    {
      uint32_t src0 = singlemul(srcPal[src[0]].a, msk0);
      dst[0] = (uint8_t)singleaddsat(dst[0], src0);
    }

    dst += 1;
    src += 1;
    msk += 1;
  }
}

static void FOG_FASTCALL raster_a8_span_composite_axxx32_a8_const_add_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_a8_span_composite_axxx32_add_sse2(dst, src, w, closure);
    return;
  }

  __m128i msk0mm;
  pix_expand_a8_1x2W(msk0mm, msk0);

  BLIT_SSE2_8x16_INIT(dst, w);

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    uint32_t src0 = src[RGB32_AByte];
    if (src0 != 0x00)
    {
      dst[0] = (uint8_t)singleaddsat(dst[0], singlemul(src0, msk0));
    }

    dst += 1;
    src += 4;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    uint32_t src0IsZero;

    __m128i src0mm, src1mm;
    __m128i msk0mm;

    pix_load_and_unpack_axxx32_64B(src0mm, src1mm, src);
    pix_pack_2x2W(src0mm, src0mm, src1mm);
    pix_analyze_mask_16B_zero(src0IsZero, src0mm);

    if (src0IsZero != 0xFFFF)
    {
      __m128i dst0mm, dst1mm;

      pix_load16a(dst0mm, dst);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_multiply_2x2W(src0mm, src0mm, msk0mm, src1mm, src1mm, msk0mm);
      pix_adds_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 64;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

#define raster_a8_span_composite_argb32_a8_const_add_sse2 raster_a8_span_composite_axxx32_a8_const_add_sse2
#define raster_a8_span_composite_prgb32_a8_const_add_sse2 raster_a8_span_composite_axxx32_a8_const_add_sse2
#define raster_a8_span_composite_rgb32_a8_const_add_sse2 raster_a8_span_composite_0xff_a8_const_src_sse2
#define raster_a8_span_composite_rgb24_a8_const_add_sse2 raster_a8_span_composite_0xff_a8_const_src_sse2

static void FOG_FASTCALL raster_a8_span_composite_a8_a8_const_add_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_a8_span_composite_a8_add_sse2(dst, src, w, closure);
    return;
  }

  __m128i msk0mm;
  pix_expand_a8_1x2W(msk0mm, msk0);

  BLIT_SSE2_8x16_INIT(dst, w);

  BLIT_SSE2_8x16_SMALL_BEGIN(blt)
    uint32_t src0 = src[0];
    if (src0 != 0x00)
    {
      dst[0] = (uint8_t)singleaddsat(dst[0], singlemul(src0, msk0));
    }

    dst += 1;
    src += 1;
  BLIT_SSE2_8x16_SMALL_END(blt)

  BLIT_SSE2_8x16_LARGE_BEGIN(blt)
    uint32_t src0IsZero;

    __m128i src0mm, src1mm;
    __m128i msk0mm;

    pix_load16u(src0mm, src);
    pix_analyze_mask_16B_zero(src0IsZero, src0mm);

    if (src0IsZero != 0xFFFF)
    {
      __m128i dst0mm, dst1mm;

      pix_load16a(dst0mm, dst);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_multiply_2x2W(src0mm, src0mm, msk0mm, src1mm, src1mm, msk0mm);
      pix_adds_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_8x16_LARGE_END(blt)
}

static void FOG_FASTCALL raster_a8_span_composite_i8_a8_const_add_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk0, sysint_t w, const Closure* closure)
{
  if (FOG_UNLIKELY(msk0 == 0xFF))
  {
    raster_a8_span_composite_i8_add_sse2(dst, src, w, closure);
    return;
  }

  const Rgba* srcPal = closure->srcPalette;

  for (sysint_t i = w; i; i--)
  {
    uint32_t src0 = srcPal[src[0]].a;
    if (src0 != 0x00)
    {
      dst[0] = (uint8_t)singleaddsat(dst[0], singlemul(src0, msk0));
    }

    dst += 1;
    src += 1;
  }
}

} // Raster namespace
} // Fog namespace
