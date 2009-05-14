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
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/Raster.h>
#include <Fog/Graphics/Raster/Raster_ByteOp.h>
#include <Fog/Graphics/Raster/Raster_PixelOp.h>
#include <Fog/Graphics/Raster/Raster_C.h>

namespace Fog {
namespace Raster {

// ============================================================================
// [Fog::Raster - Defines]
// ============================================================================

#define READ_MASK_A8(ptr) ((const uint8_t *)ptr)[0]
#define READ_MASK_C8(ptr) ((const uint32_t*)ptr)[0]

// ============================================================================
// [Fog::Raster - Constants]
// ============================================================================

static __m128i Mask0080008000800080;
static __m128i Mask00FF00FF00FF00FF;
static __m128i Mask0101010101010101;
static __m128i MaskFFFFFFFFFFFFFFFF;
static __m128i Mask00FF000000000000;
static __m128i MaskFF000000FF000000;

// ============================================================================
// [Fog::Raster - SSE2]
// ============================================================================

static FOG_INLINE __m128i pix_create_mask_8x2W(uint16_t m0, uint16_t m1, uint16_t m2, uint16_t m3)
{
  sse2_t t;

  t.uw[3] = m0;
  t.uw[2] = m1;
  t.uw[1] = m2;
  t.uw[0] = m3;
  t.uw[7] = m0;
  t.uw[6] = m1;
  t.uw[5] = m2;
  t.uw[4] = m3;

  return _mm_loadu_si128((__m128i*)&t);
}

static FOG_INLINE void pix_unpack_1x1W(
  __m128i& dst0, __m128i data)
{
  __m128i xmmz = _mm_setzero_si128();
  dst0 = _mm_unpacklo_epi8(data, xmmz);
}

static FOG_INLINE void pix_unpack_1x2W(
  __m128i& dst0, __m128i data)
{
  __m128i xmmz = _mm_setzero_si128();
  dst0 = _mm_unpacklo_epi8(data, xmmz);
}

static FOG_INLINE void pix_unpack_1x1W(
  __m128i& dst0, uint32_t data)
{
  pix_unpack_1x1W(dst0, _mm_cvtsi32_si128(data));
}

static FOG_INLINE void pix_unpack_2x2W(
  __m128i& dst0,
  __m128i& dst1, __m128i data)
{
  __m128i xmmz = _mm_setzero_si128();
  dst1 = _mm_unpackhi_epi8(data, xmmz);
  dst0 = _mm_unpacklo_epi8(data, xmmz);
}

static FOG_INLINE void pix_pack_1x1W(
  __m128i& dst0, const __m128i& src0)
{
  dst0 = _mm_packus_epi16(src0, src0);
}

static FOG_INLINE void pix_pack_2x2W(
  __m128i& dst0, const __m128i& src0, const __m128i& src1)
{
  dst0 = _mm_packus_epi16(src0, src1);
}

static FOG_INLINE uint32_t pix_pack_alpha(
  __m128i x)
{
  __m128i xmmz = _mm_setzero_si128();
  __m128i tmp = _mm_srli_epi32(x, 24);
  tmp = _mm_packus_epi16(tmp, xmmz);
  tmp = _mm_packus_epi16(tmp, xmmz);
  return _mm_cvtsi128_si32(tmp);
}

static FOG_INLINE void pix_negate_1x1W(
  __m128i& dst0, const __m128i& src0)
{
  dst0 = _mm_xor_si128(src0, Mask00FF00FF00FF00FF);
}

static FOG_INLINE void pix_negate_2x2W(
  __m128i& dst0, const __m128i& src0,
  __m128i& dst1, const __m128i& src1)
{
  dst0 = _mm_xor_si128(src0, Mask00FF00FF00FF00FF);
  dst1 = _mm_xor_si128(src1, Mask00FF00FF00FF00FF);
}

/*
static FOG_INLINE __m128i pix_expandPixel_32_1x128(uint32_t data)
{
  return _mm_shuffle_epi32(pix_unpack_32_1x128(data), _MM_SHUFFLE(1, 0, 1, 0));
}*/

static FOG_INLINE void pix_expand_alpha_1x1W(
  __m128i& dst0, const __m128i& src0)
{
  dst0 = _mm_shufflelo_epi16(src0, _MM_SHUFFLE(3, 3, 3, 3));
}

static FOG_INLINE void pix_expand_alpha_1x2W(
  __m128i& dst0, const __m128i& src0)
{
  dst0 = _mm_shufflelo_epi16(src0, _MM_SHUFFLE(3, 3, 3, 3));
  dst0 = _mm_shufflehi_epi16(dst0, _MM_SHUFFLE(3, 3, 3, 3));
}

static FOG_INLINE void pix_expand_alpha_2x2W(
  __m128i& dst0, const __m128i& src0,
  __m128i& dst1, const __m128i& src1)
{
  dst0 = _mm_shufflelo_epi16(src0, _MM_SHUFFLE(3, 3, 3, 3));
  dst1 = _mm_shufflelo_epi16(src1, _MM_SHUFFLE(3, 3, 3, 3));
  dst0 = _mm_shufflehi_epi16(dst0, _MM_SHUFFLE(3, 3, 3, 3));
  dst1 = _mm_shufflehi_epi16(dst1, _MM_SHUFFLE(3, 3, 3, 3));
}

static FOG_INLINE void pix_expand_alpha_rev_1x1W(
  __m128i& dst0, const __m128i& src0)
{
  dst0 = _mm_shufflelo_epi16(src0, _MM_SHUFFLE(0, 0, 0, 0));
}

static FOG_INLINE void pix_expand_alpha_rev_1x2W(
  __m128i& dst0, const __m128i& src0)
{
  dst0 = _mm_shufflelo_epi16(src0, _MM_SHUFFLE(0, 0, 0, 0));
  dst0 = _mm_shufflehi_epi16(dst0, _MM_SHUFFLE(0, 0, 0, 0));
}

static FOG_INLINE void pix_multiply_1x1W(
  __m128i& dst0, const __m128i& data0, const __m128i& alpha0)
{
  __m128i t0;

  t0 = _mm_mullo_epi16(data0, alpha0);
  t0 = _mm_adds_epu16(t0, Mask0080008000800080);
  dst0 = _mm_mulhi_epu16(t0, Mask0101010101010101);
}

static FOG_INLINE void pix_multiply_2x2W(
  __m128i& dst0, const __m128i& data0, const __m128i& alpha0,
  __m128i& dst1, const __m128i& data1, const __m128i& alpha1)
{
  __m128i t0, t1;

  t0 = _mm_mullo_epi16(data0, alpha0);
  t1 = _mm_mullo_epi16(data1, alpha1);
  t0 = _mm_adds_epu16(t0, Mask0080008000800080);
  t1 = _mm_adds_epu16(t1, Mask0080008000800080);
  dst0 = _mm_mulhi_epu16(t0, Mask0101010101010101);
  dst1 = _mm_mulhi_epu16(t1, Mask0101010101010101);
}

static FOG_INLINE void pix_fill_alpha_1x1W(
  __m128i& dst0)
{
  dst0 = _mm_or_si128(dst0, Mask00FF000000000000);
}

static FOG_INLINE void pix_fill_alpha_2x2W(
  __m128i& dst0,
  __m128i& dst1)
{
  dst0 = _mm_or_si128(dst0, Mask00FF000000000000);
  dst1 = _mm_or_si128(dst1, Mask00FF000000000000);
}

static FOG_INLINE void pix_premultiply_1x1W(
  __m128i& dst0, const __m128i& src0)
{
  __m128i alpha0;

  pix_expand_alpha_1x1W(alpha0, src0);
  pix_fill_alpha_1x1W(alpha0);
  pix_multiply_1x1W(dst0, src0, alpha0);
}

static FOG_INLINE void pix_premultiply_2x2W(
  __m128i& dst0, const __m128i& src0,
  __m128i& dst1, const __m128i& src1)
{
  __m128i alpha0;
  __m128i alpha1;

  pix_expand_alpha_2x2W(
    alpha0, src0,
    alpha1, src1);
  pix_fill_alpha_2x2W(
    alpha0,
    alpha1);
  pix_multiply_2x2W(
    dst0, src0, alpha0,
    dst1, src1, alpha1);
}

static FOG_INLINE void pix_over_1x1W(
  __m128i& dst0, const __m128i& src0, const __m128i& alpha0)
{
  __m128i ialpha0;

  pix_negate_1x1W(ialpha0, alpha0);
  pix_multiply_1x1W(dst0, dst0, ialpha0);
  dst0 = _mm_adds_epu8(dst0, src0);
}

static FOG_INLINE void pix_over_2x2W(
  __m128i& dst0, const __m128i& src0, const __m128i& alpha0,
  __m128i& dst1, const __m128i& src1, const __m128i& alpha1)
{
  __m128i ialpha0;
  __m128i ialpha1;

  pix_negate_2x2W(
    ialpha0, alpha0,
    ialpha1, alpha1);
  pix_multiply_2x2W(
    dst0, dst0, ialpha0, 
    dst1, dst1, ialpha1);
  dst0 = _mm_adds_epu8(dst0, src0);
  dst1 = _mm_adds_epu8(dst1, src1);
}

// ============================================================================
// [Fog::Raster - Gradient - gradient]
// ============================================================================

static void FOG_FASTCALL gradient_gradient_argb32_sse2(uint8_t* dst, uint32_t c0, uint32_t c1, sysint_t w, sysint_t x1, sysint_t x2)
{
  uint8_t* dstCur = dst;

  // Sanity checks.
  FOG_ASSERT(w >= 0 && x1 <= x2);

  sysint_t xw = (x2 - x1);
  if (xw == 0) return;

  // Width is decreased by 1 to fit our gradient schema that first and last
  // points in interpolation are always equal to c0 and c1 recpectively.
  if (w) w--;

  // 0op counter.
  sysint_t i;

  // Fill c0 before gradient start.
  if (x1 < 0)
  {
    i = fog_min((sysint_t)0, x2) - x1;

    xw -= i;
    x1 = 0;

    do { set4(dstCur, c0); dstCur += 4; } while (--i);
    if (xw == 0) return;
  }

  // Fill c0 to c1 using linear interpolation.
  if (x1 < w)
  {
    __m128i xmmz;
    __m128i xmm0, xmm1;
    __m128i xmm2, xmm3;
    __m128i xmm4, xmm5;

    FOG_DECLARE_ALIGNED_VARIABLE(sse2_t, tmpARGB, 16);

    xmmz = _mm_setzero_si128();
    xmm0 = _mm_cvtsi32_si128((int)c0);           // xmm0 = [            ARGB] c0
    xmm1 = _mm_cvtsi32_si128((int)c1);           // xmm1 = [            ARGB] c1

    xmm0 = _mm_unpacklo_epi8(xmm0, xmmz);        // xmm0 = [        0A0R0G0B] c0
    xmm1 = _mm_unpacklo_epi8(xmm1, xmmz);        // xmm1 = [        0A0R0G0B] c1

    xmm0 = _mm_unpacklo_epi16(xmmz, xmm0);       // xmm0 = [0A000R000G000B00] c0
    xmm1 = _mm_unpacklo_epi16(xmmz, xmm1);       // xmm1 = [0A000R000G000B00] c1

    xmm1 = _mm_sub_epi32(xmm1, xmm0);            // xmm1 = difference

    // Divide.
    tmpARGB.m128i = xmm1;                        // copy xmm1 to temporary buffer

    tmpARGB.sd[0] /= (int32_t)w;
    tmpARGB.sd[1] /= (int32_t)w;
    tmpARGB.sd[2] /= (int32_t)w;
    tmpARGB.sd[3] /= (int32_t)w;

    xmm1 = tmpARGB.m128i;                        // xmm1 = increment

    // Offset interpolation to x1.
    tmpARGB.sd[0] *= (int32_t)x1;
    tmpARGB.sd[1] *= (int32_t)x1;
    tmpARGB.sd[2] *= (int32_t)x1;
    tmpARGB.sd[3] *= (int32_t)x1;

    xmm0 = _mm_add_epi32(xmm0, tmpARGB.m128i);   // xmm0 = c0 + offset

    i = fog_min(w + 1, x2) - x1;
    x1 += i;

    // Align.
    while (((sysuint_t)dstCur & 15) != 0)
    {
      xmm2 = xmm0;                               // xmm2 = [xAxxxRxxxGxxxBxx]
      xmm0 = _mm_add_epi32(xmm0, xmm1);          // xmm0 += xmm1

      xmm2 = _mm_packus_epi16(xmm2, xmm2);       // xmm2 = [AxRxGxBxAxRxGxBx]
      xmm2 = _mm_srli_epi16(xmm2, 8);            // xmm2 = [0A0R0G0B0A0R0G0B]
      xmm2 = _mm_packus_epi16(xmm2, xmm2);        // xmm2 = [ARGBARGBARGBARGB]

      ((int *)dstCur)[0] = _mm_cvtsi128_si32(xmm2);
      dstCur += 4;
      if (--i == 0) goto interpolation_end;
    }

    // 0op: 4 pixels at time.
    while (i >= 4)
    {
      xmm2 = xmm0;                               // xmm2 = [xAxxxRxxxGxxxBxx]
      xmm0 = _mm_add_epi32(xmm0, xmm1);          // xmm0 += xmm1

      xmm3 = xmm0;                               // xmm3 = [xAxxxRxxxGxxxBxx]
      xmm0 = _mm_add_epi32(xmm0, xmm1);          // xmm0 += xmm1

      xmm4 = xmm0;                               // xmm4 = [xAxxxRxxxGxxxBxx]
      xmm0 = _mm_add_epi32(xmm0, xmm1);          // xmm0 += xmm1

      xmm5 = xmm0;                               // xmm5 = [xAxxxRxxxGxxxBxx]
      xmm0 = _mm_add_epi32(xmm0, xmm1);          // xmm0 += xmm1

      xmm2 = _mm_packus_epi16(xmm2, xmm3);       // xmm2 = [AxRxGxBxAxRxGxBx]
      xmm4 = _mm_packus_epi16(xmm4, xmm5);       // xmm4 = [AxRxGxBxAxRxGxBx]
      xmm2 = _mm_srli_epi16(xmm2, 8);            // xmm2 = [0A0R0G0B0A0R0G0B]
      xmm4 = _mm_srli_epi16(xmm4, 8);            // xmm4 = [0A0R0G0B0A0R0G0B]

      xmm2 = _mm_packus_epi16(xmm2, xmm4);       // xmm2 = [ARGBARGBARGBARGB]

      _mm_store_si128((__m128i *)dstCur, xmm2);

      dstCur += 16;
      i -= 4;
    }

    // Tail.
    while (i)
    {
      xmm2 = xmm0;                               // xmm2 = [xAxxxRxxxGxxxBxx]
      xmm0 = _mm_add_epi32(xmm0, xmm1);          // xmm0 += xmm1

      xmm2 = _mm_packus_epi16(xmm2, xmm2);       // xmm2 = [AxRxGxBxAxRxGxBx]
      xmm2 = _mm_srli_epi16(xmm2, 8);            // xmm2 = [0A0R0G0B0A0R0G0B]
      xmm2 = _mm_packus_epi16(xmm2, xmm2);       // xmm2 = [ARGBARGBARGBARGB]

      ((int *)dstCur)[0] = _mm_cvtsi128_si32(xmm2);
      dstCur += 4;
      i--;
    }
interpolation_end:
    if (x1 == x2) return;
  }

  // Fill c1 after gradient end.
  i = x2 - x1;
  do { set4(dstCur, c1); dstCur += 4; } while (--i);
}

static void FOG_FASTCALL gradient_gradient_prgb32_sse2(uint8_t* dst, uint32_t c0, uint32_t c1, sysint_t w, sysint_t x1, sysint_t x2)
{
  uint8_t* dstCur = dst;

  // Sanity checks.
  FOG_ASSERT(w >= 0 && x1 <= x2);

  sysint_t xw = (x2 - x1);
  if (xw == 0) return;

  // Width is decreased by 1 to fit our gradient schema that first and last
  // points in interpolation are always equal to c0 and c1 recpectively.
  if (w) w--;

  // 0op counter.
  sysint_t i;

  // Fill c0 before gradient start.
  if (x1 < 0)
  {
    uint32_t c0p = premultiply(c0);
    i = fog_min((sysint_t)0, x2) - x1;

    xw -= i;
    x1 = 0;

    do { set4(dstCur, c0p); dstCur += 4; } while (--i);
    if (xw == 0) return;
  }

  // Fill c0 to c1 using linear interpolation.
  if (x1 < w)
  {
    __m128i xmmz;
    __m128i xmm0, xmm1;
    __m128i xmm2, xmm3;
    __m128i xmm4, xmm5;

    FOG_DECLARE_ALIGNED_VARIABLE(sse2_t, tmpARGB, 16);

    xmmz = _mm_setzero_si128();
    xmm0 = _mm_cvtsi32_si128((int)c0);           // xmm0 = [            ARGB] c0
    xmm1 = _mm_cvtsi32_si128((int)c1);           // xmm1 = [            ARGB] c1

    xmm0 = _mm_unpacklo_epi8(xmm0, xmmz);        // xmm0 = [        0A0R0G0B] c0
    xmm1 = _mm_unpacklo_epi8(xmm1, xmmz);        // xmm1 = [        0A0R0G0B] c1

    xmm0 = _mm_unpacklo_epi16(xmmz, xmm0);       // xmm0 = [0A000R000G000B00] c0
    xmm1 = _mm_unpacklo_epi16(xmmz, xmm1);       // xmm1 = [0A000R000G000B00] c1

    xmm1 = _mm_sub_epi32(xmm1, xmm0);            // xmm1 = difference

    // Divide.
    tmpARGB.m128i = xmm1;                        // copy xmm1 to temporary buffer

    tmpARGB.sd[0] /= (int32_t)w;
    tmpARGB.sd[1] /= (int32_t)w;
    tmpARGB.sd[2] /= (int32_t)w;
    tmpARGB.sd[3] /= (int32_t)w;

    xmm1 = tmpARGB.m128i;                        // xmm1 = increment

    // Offset interpolation to x1.
    tmpARGB.sd[0] *= (int32_t)x1;
    tmpARGB.sd[1] *= (int32_t)x1;
    tmpARGB.sd[2] *= (int32_t)x1;
    tmpARGB.sd[3] *= (int32_t)x1;

    xmm0 = _mm_add_epi32(xmm0, tmpARGB.m128i);   // xmm0 = c0 + offset

    i = fog_min(w + 1, x2) - x1;
    x1 += i;

    // Align.
    while (((sysuint_t)dstCur & 15) != 0)
    {
      xmm2 = xmm0;                               // xmm2 = [xAxxxRxxxGxxxBxx]
      xmm0 = _mm_add_epi32(xmm0, xmm1);          // xmm0 += xmm1

      xmm2 = _mm_packus_epi16(xmm2, xmm2);       // xmm2 = [AxRxGxBxAxRxGxBx]
      xmm2 = _mm_srli_epi16(xmm2, 8);            // xmm2 = [0A0R0G0B0A0R0G0B]
      pix_premultiply_1x1W(xmm2, xmm2);
      xmm2 = _mm_packus_epi16(xmm2, xmm2);       // xmm2 = [ARGBARGBARGBARGB]

      ((int *)dstCur)[0] = _mm_cvtsi128_si32(xmm2);
      dstCur += 4;
      if (--i == 0) goto interpolation_end;
    }

    // 0op: 4 pixels at time.
    while (i >= 4)
    {
      xmm2 = xmm0;                               // xmm2 = [xAxxxRxxxGxxxBxx]
      xmm0 = _mm_add_epi32(xmm0, xmm1);          // xmm0 += xmm1

      xmm3 = xmm0;                               // xmm3 = [xAxxxRxxxGxxxBxx]
      xmm0 = _mm_add_epi32(xmm0, xmm1);          // xmm0 += xmm1

      xmm4 = xmm0;                               // xmm4 = [xAxxxRxxxGxxxBxx]
      xmm0 = _mm_add_epi32(xmm0, xmm1);          // xmm0 += xmm1

      xmm5 = xmm0;                               // xmm5 = [xAxxxRxxxGxxxBxx]
      xmm0 = _mm_add_epi32(xmm0, xmm1);          // xmm0 += xmm1

      xmm2 = _mm_packus_epi16(xmm2, xmm3);       // xmm2 = [AxRxGxBxAxRxGxBx]
      xmm4 = _mm_packus_epi16(xmm4, xmm5);       // xmm4 = [AxRxGxBxAxRxGxBx]
      xmm2 = _mm_srli_epi16(xmm2, 8);            // xmm2 = [0A0R0G0B0A0R0G0B]
      xmm4 = _mm_srli_epi16(xmm4, 8);            // xmm4 = [0A0R0G0B0A0R0G0B]
      pix_premultiply_2x2W(xmm2, xmm2, xmm4, xmm4);
      xmm2 = _mm_packus_epi16(xmm2, xmm4);       // xmm2 = [ARGBARGBARGBARGB]

      _mm_store_si128((__m128i *)dstCur, xmm2);

      dstCur += 16;
      i -= 4;
    }

    // Tail.
    while (i)
    {
      xmm2 = xmm0;                               // xmm2 = [xAxxxRxxxGxxxBxx]
      xmm0 = _mm_add_epi32(xmm0, xmm1);          // xmm0 += xmm1

      xmm2 = _mm_packus_epi16(xmm2, xmm2);       // xmm2 = [AxRxGxBxAxRxGxBx]
      xmm2 = _mm_srli_epi16(xmm2, 8);            // xmm2 = [0A0R0G0B0A0R0G0B]
      pix_premultiply_1x1W(xmm2, xmm2);
      xmm2 = _mm_packus_epi16(xmm2, xmm2);       // xmm2 = [ARGBARGBARGBARGB]

      ((int *)dstCur)[0] = _mm_cvtsi128_si32(xmm2);
      dstCur += 4;
      i--;
    }

interpolation_end:
    if (x1 == x2) return;
  }

  // Fill c1 after gradient end.
  {
    uint32_t c1p = premultiply(c1);
    i = x2 - x1;
    do { set4(dstCur, c1p); dstCur += 4; } while (--i);
  }
}

// ============================================================================
// [Fog::Raster - Raster - Rgb32]
// ============================================================================

static void FOG_FASTCALL raster_rgb32_pixel_sse2(
  uint8_t* dst, uint32_t src)
{
  uint32_t a = src >> 24;

  if (a != 0xFF)
  {
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_unpack_1x1W(src0mm, src);
    pix_unpack_1x1W(dst0mm, ((uint32_t*)dst)[0]);
    pix_expand_alpha_1x1W(a0mm, src0mm);
    pix_over_1x1W(dst0mm, src0mm, a0mm);
    pix_pack_1x1W(dst0mm, dst0mm);

    ((uint32_t*)dst)[0] = _mm_cvtsi128_si32(dst0mm);
  }
  else
  {
    ((uint32_t*)dst)[0] = src;
  }
}

static void FOG_FASTCALL raster_rgb32_pixel_a8_sse2(
  uint8_t* dst, uint32_t src, uint32_t msk)
{
  uint32_t a = src >> 24;

  if (a != 0xFF || msk != 0xFF)
  {
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_unpack_1x1W(src0mm, src);
    pix_unpack_1x1W(a0mm, msk);
    pix_expand_alpha_rev_1x1W(a0mm, a0mm);
    pix_unpack_1x1W(dst0mm, ((uint32_t*)dst)[0]);
    pix_multiply_1x1W(src0mm, src0mm, a0mm);
    pix_expand_alpha_1x1W(a0mm, src0mm);
    pix_over_1x1W(dst0mm, src0mm, a0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    ((uint32_t*)dst)[0] = _mm_cvtsi128_si32(dst0mm);
  }
  else
    ((uint32_t*)dst)[0] = src;
}

static void FOG_FASTCALL raster_rgb32_span_solid_sse2(
  uint8_t* dst, uint32_t src, sysint_t w)
{
  sysint_t i = w;
  uint32_t a = src >> 24;

  __m128i src0mm = _mm_cvtsi32_si128(src);
  src0mm = _mm_shuffle_epi32(src0mm, _MM_SHUFFLE(0, 0, 0, 0));

  if (a != 0xFF)
  {
    __m128i dst0mm;
    __m128i dst1mm;
    __m128i a0mm;

    pix_unpack_1x2W(src0mm, src0mm);
    pix_expand_alpha_1x2W(a0mm, src0mm);

    while ((sysuint_t)dst & 15)
    {
      pix_unpack_1x1W(dst0mm, ((uint32_t*)dst)[0]);
      pix_over_1x1W(dst0mm, src0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);

      ((uint32_t*)dst)[0] = _mm_cvtsi128_si32(dst0mm);

      dst += 4;
      if (--i == 0) return;
    }

    while (i >= 4)
    {
      dst0mm = _mm_load_si128((__m128i *)dst);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_over_2x2W(dst0mm, src0mm, a0mm, dst1mm, src0mm, a0mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);

      _mm_store_si128((__m128i*)dst, dst0mm);

      dst += 16;
      i -= 4;
    }

    while (i)
    {
      pix_unpack_1x1W(dst0mm, ((uint32_t*)dst)[0]);
      pix_over_1x1W(dst0mm, src0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);

      ((uint32_t*)dst)[0] = _mm_cvtsi128_si32(dst0mm);

      dst += 4;
      i--;
    }
  }
  else
  {
    while ((sysuint_t)dst & 15)
    {
      ((uint32_t*)dst)[0] = src;

      dst += 4;
      if (--i == 0) return;
    }

    while (i >= 16)
    {
      _mm_store_si128((__m128i*)(dst), src0mm);
      _mm_store_si128((__m128i*)(dst + 16), src0mm);
      _mm_store_si128((__m128i*)(dst + 32), src0mm);
      _mm_store_si128((__m128i*)(dst + 48), src0mm);

      dst += 64;
      i -= 16;
    }

    while (i >= 4)
    {
      _mm_store_si128((__m128i*)dst, src0mm);

      dst += 16;
      i -= 4;
    }

    while (i)
    {
      ((uint32_t*)dst)[0] = src;

      dst += 4;
      i--;
    }
  }
}

static void FOG_FASTCALL raster_rgb32_span_solid_a8_sse2(
  uint8_t* dst, uint32_t src, const uint8_t* msk, sysint_t w)
{
  sysint_t i = w;
  uint32_t a = src >> 24;
  uint32_t m;

  __m128i src0orig = _mm_cvtsi32_si128(src);
  __m128i src0unpacked;

  __m128i src0mm;
  __m128i src1mm;

  __m128i dst0mm;
  __m128i dst1mm;

  __m128i a0mm;
  __m128i a1mm;

  src0orig = _mm_shuffle_epi32(src0orig, _MM_SHUFFLE(0, 0, 0, 0));
  pix_unpack_1x2W(src0unpacked, src0orig);

  if (a != 0xFF)
  {
    while ((sysuint_t)dst & 15)
    {
      if ((m = READ_MASK_A8(msk)))
      {
        pix_unpack_1x1W(dst0mm, ((uint32_t*)dst)[0]);
        pix_expand_alpha_rev_1x1W(a0mm, _mm_cvtsi32_si128(m));
        a0mm = _mm_shuffle_epi32(a0mm, _MM_SHUFFLE(1, 0, 1, 0));
        pix_multiply_1x1W(src0mm, src0unpacked, a0mm);
        pix_expand_alpha_1x1W(a0mm, src0mm);
        pix_over_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        ((uint32_t*)dst)[0] = _mm_cvtsi128_si32(dst0mm);
      }

      dst += 4;
      msk += 1;
      if (--i == 0) return;
    }

    while (i >= 4)
    {
      if ((m = ((uint32_t*)msk)[0]))
      {
        dst0mm = _mm_load_si128((__m128i *)(dst));
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);

        pix_unpack_1x1W(a0mm, _mm_cvtsi32_si128(m));
        a0mm = _mm_unpacklo_epi16(a0mm, a0mm);

        a1mm = _mm_shuffle_epi32(a0mm, _MM_SHUFFLE(3, 3, 2, 2));
        a0mm = _mm_shuffle_epi32(a0mm, _MM_SHUFFLE(1, 1, 0, 0));

        pix_multiply_2x2W(src0mm, src0unpacked, a0mm, src1mm, src0unpacked, a1mm);
        pix_expand_alpha_2x2W(a0mm, src0mm, a1mm, src1mm);
        pix_over_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);

        _mm_store_si128((__m128i*)(dst), dst0mm);
      }

      dst += 16;
      msk += 4;
      i -= 4;
    }

    while (i)
    {
      if ((m = READ_MASK_A8(msk)))
      {
        pix_unpack_1x1W(dst0mm, ((uint32_t*)dst)[0]);
        pix_expand_alpha_rev_1x1W(a0mm, _mm_cvtsi32_si128(m));
        a0mm = _mm_shuffle_epi32(a0mm, _MM_SHUFFLE(1, 0, 1, 0));
        pix_multiply_1x1W(src0mm, src0unpacked, a0mm);
        pix_expand_alpha_1x1W(a0mm, src0mm);
        pix_over_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        ((uint32_t*)dst)[0] = _mm_cvtsi128_si32(dst0mm);
      }

      dst += 4;
      msk += 1;
      i--;
    }
  }
  else
  {
    while ((sysuint_t)dst & 15)
    {
      if ((m = READ_MASK_A8(msk)) == 0xFF)
      {
        ((uint32_t*)dst)[0] = src;
      }
      else if (m)
      {
        pix_unpack_1x1W(dst0mm, ((uint32_t*)dst)[0]);
        pix_expand_alpha_rev_1x1W(a0mm, _mm_cvtsi32_si128(m));
        a0mm = _mm_shuffle_epi32(a0mm, _MM_SHUFFLE(1, 0, 1, 0));
        pix_multiply_1x1W(src0mm, src0unpacked, a0mm);
        pix_over_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        ((uint32_t*)dst)[0] = _mm_cvtsi128_si32(dst0mm);
      }

      dst += 4;
      msk += 1;
      if (--i == 0) return;
    }

    while (i >= 4)
    {
      if ((m = ((uint32_t*)msk)[0]))
      {
        if (m == 0xFFFFFFFF)
        {
          _mm_store_si128((__m128i*)dst, src0orig);
        }
        else
        {
          dst0mm = _mm_load_si128((__m128i *)(dst));
          pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);

          pix_unpack_1x1W(a0mm, _mm_cvtsi32_si128(m));
          a0mm = _mm_unpacklo_epi16(a0mm, a0mm);

          a1mm = _mm_shuffle_epi32(a0mm, _MM_SHUFFLE(3, 3, 2, 2));
          a0mm = _mm_shuffle_epi32(a0mm, _MM_SHUFFLE(1, 1, 0, 0));

          pix_multiply_2x2W(src0mm, src0unpacked, a0mm, src1mm, src0unpacked, a1mm);
          pix_over_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
          pix_pack_2x2W(dst0mm, dst0mm, dst1mm);

          _mm_store_si128((__m128i*)(dst), dst0mm);
        }
      }

      dst += 16;
      msk += 4;
      i -= 4;
    }

    while (i)
    {
      if ((m = READ_MASK_A8(msk)) == 0xFF)
      {
        ((uint32_t*)dst)[0] = src;
      }
      else if (m)
      {
        pix_unpack_1x1W(dst0mm, ((uint32_t*)dst)[0]);
        pix_expand_alpha_rev_1x1W(a0mm, _mm_cvtsi32_si128(m));
        a0mm = _mm_shuffle_epi32(a0mm, _MM_SHUFFLE(1, 0, 1, 0));
        pix_multiply_1x1W(src0mm, src0unpacked, a0mm);
        pix_over_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        ((uint32_t*)dst)[0] = _mm_cvtsi128_si32(dst0mm);
      }

      dst += 4;
      msk += 1;
      i--;
    }
  }
}

static void FOG_FASTCALL raster_rgb32_span_composite_rgb32_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  sysint_t i = w;

  __m128i amask = MaskFF000000FF000000;

  while ((sysuint_t)dst & 15)
  {
    ((uint32_t*)dst)[0] = ((const uint32_t*)src)[0] | 0xFF000000;

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

    src0mm = _mm_loadu_si128((__m128i *)(src));
    src1mm = _mm_loadu_si128((__m128i *)(src + 16));
    src2mm = _mm_loadu_si128((__m128i *)(src + 32));
    src3mm = _mm_loadu_si128((__m128i *)(src + 48));

    src0mm = _mm_or_si128(src0mm, amask);
    src1mm = _mm_or_si128(src1mm, amask);
    src2mm = _mm_or_si128(src2mm, amask);
    src3mm = _mm_or_si128(src3mm, amask);

    _mm_store_si128((__m128i*)(dst), src0mm);
    _mm_store_si128((__m128i*)(dst + 16), src1mm);
    _mm_store_si128((__m128i*)(dst + 32), src2mm);
    _mm_store_si128((__m128i*)(dst + 48), src3mm);

    dst += 64;
    src += 64;
    i -= 16;
  }

  while (i >= 4)
  {
    __m128i src0mm;

    src0mm = _mm_loadu_si128((__m128i *)src);
    src0mm = _mm_or_si128(src0mm, amask);
    _mm_store_si128((__m128i*)dst, src0mm);

    dst += 16;
    src += 16;
    i -= 4;
  }

  while (i)
  {
    ((uint32_t*)dst)[0] = ((const uint32_t*)src)[0] | 0xFF000000;

    dst += 4;
    src += 4;
    i--;
  }
}

static void FOG_FASTCALL raster_rgb32_span_composite_argb32_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  sysint_t i = w;

  __m128i src0mm, src1mm;
  __m128i dst0mm, dst1mm;
  __m128i a0mm, a1mm;

  while ((sysuint_t)dst & 15)
  {
    uint32_t src0 = ((const uint32_t*)src)[0];

    if (src0 >> 24)
    {
      pix_unpack_1x1W(src0mm, src0);
      pix_unpack_1x1W(dst0mm, ((uint32_t*)dst)[0]);

      pix_expand_alpha_1x1W(a0mm, src0mm);
      pix_fill_alpha_1x1W(src0mm);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);

      pix_over_1x1W(dst0mm, src0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);

      ((uint32_t*)dst)[0] = _mm_cvtsi128_si32(dst0mm);
    }

    dst += 4;
    src += 4;
    if (--i == 0) return;
  }

  while (i >= 4)
  {
    src0mm = _mm_loadu_si128((__m128i *)src);

    // This is efficient optimization. We check four pixel alpha values and 
    // determine if they are:
    // - all zeros (NOP)
    // - full opaque (SRC)
    // - variant (SRC OVER)

    // NOTE: This first setzero is useless, it's here because MSVC fails
    // to run this code in debug mode (uninitialized value exception).
    dst0mm = _mm_setzero_si128();                // dst0 = [0000000000000000]
    dst1mm = _mm_setzero_si128();                // dst1 = [0000000000000000]
    dst0mm = _mm_cmpeq_epi8(dst0mm, dst0mm);     // dst0 = [FFFFFFFFFFFFFFFF]

    dst1mm = _mm_cmpeq_epi8(dst1mm, src0mm);     // dst1
    dst0mm = _mm_cmpeq_epi8(dst0mm, src0mm);     // dst0

    uint32_t t = (uint32_t)_mm_movemask_epi8(dst1mm) & 0x8888;
    uint32_t k = (uint32_t)_mm_movemask_epi8(dst0mm) & 0x8888;

    // if t == 0xFFFF then all alpha values are zeros.
    if (t != 0x0000)
    {
      // if k != 0x8888 then alpha values are variant.
      if (k != 0x8888)
      {
        dst0mm = _mm_load_si128((__m128i *)dst);

        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_alpha_2x2W(a0mm, src0mm, a1mm, src1mm);
        pix_fill_alpha_2x2W(src0mm, src1mm);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);

        pix_over_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);

        _mm_store_si128((__m128i*)dst, dst0mm);
      }
      else
      {
        _mm_store_si128((__m128i*)dst, src0mm);
      }
    }

    dst += 16;
    src += 16;
    i -= 4;
  }

  while (i)
  {
    uint32_t src0 = ((const uint32_t*)src)[0];

    if (src0)
    {
      pix_unpack_1x1W(src0mm, src0);
      pix_unpack_1x1W(dst0mm, ((uint32_t*)dst)[0]);

      pix_expand_alpha_1x1W(a0mm, src0mm);
      pix_fill_alpha_1x1W(src0mm);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);

      pix_over_1x1W(dst0mm, src0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);

      ((uint32_t*)dst)[0] = _mm_cvtsi128_si32(dst0mm);
    }

    dst += 4;
    src += 4;
    i--;
  }
}

static void FOG_FASTCALL raster_rgb32_span_composite_prgb32_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  sysint_t i = w;

  __m128i src0mm, src1mm;
  __m128i dst0mm, dst1mm;
  __m128i a0mm, a1mm;

  while ((sysuint_t)dst & 15)
  {
    uint32_t src0 = ((const uint32_t*)src)[0];

    if (src0)
    {
      pix_unpack_1x1W(src0mm, src0);
      pix_unpack_1x1W(dst0mm, ((uint32_t*)dst)[0]);
      pix_expand_alpha_1x1W(a0mm, src0mm);
      pix_over_1x1W(dst0mm, src0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);

      ((uint32_t*)dst)[0] = _mm_cvtsi128_si32(dst0mm);
    }

    dst += 4;
    src += 4;
    if (--i == 0) return;
  }

  while (i >= 4)
  {
    src0mm = _mm_loadu_si128((__m128i *)src);

    // This is efficient optimization. We check four pixel alpha values and 
    // determine if they are:
    // - all zeros (NOP)
    // - full opaque (SRC)
    // - variant (SRC OVER)

    // NOTE: This first setzero is useless, it's here because MSVC fails
    // to run this code in debug mode (uninitialized value exception).
    dst0mm = _mm_setzero_si128();                // dst0 = [0000000000000000]
    dst1mm = _mm_setzero_si128();                // dst1 = [0000000000000000]
    dst0mm = _mm_cmpeq_epi8(dst0mm, dst0mm);     // dst0 = [FFFFFFFFFFFFFFFF]

    dst1mm = _mm_cmpeq_epi8(dst1mm, src0mm);     // dst1
    dst0mm = _mm_cmpeq_epi8(dst0mm, src0mm);     // dst0

    uint32_t t = (uint32_t)_mm_movemask_epi8(dst1mm);
    uint32_t k = (uint32_t)_mm_movemask_epi8(dst0mm) & 0x8888;

    // if t == 0xFFFF then all alpha values are zeros.
    if (t != 0xFFFF)
    {
      // if k != 0x8888 then alpha values are variant.
      if (k != 0x8888)
      {
        dst0mm = _mm_load_si128((__m128i *)dst);

        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_alpha_2x2W(a0mm, src0mm, a1mm, src1mm);
        pix_over_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);

        _mm_store_si128((__m128i*)dst, dst0mm);
      }
      else
      {
        _mm_store_si128((__m128i*)dst, src0mm);
      }
    }

    dst += 16;
    src += 16;
    i -= 4;
  }

  while (i)
  {
    uint32_t src0 = ((const uint32_t*)src)[0];

    if (src0)
    {
      pix_unpack_1x1W(src0mm, src0);
      pix_unpack_1x1W(dst0mm, ((uint32_t*)dst)[0]);
      pix_expand_alpha_1x1W(a0mm, src0mm);
      pix_over_1x1W(dst0mm, src0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);

      ((uint32_t*)dst)[0] = _mm_cvtsi128_si32(dst0mm);
    }

    dst += 4;
    src += 4;
    i--;
  }
}

} // Raster namespace
} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE void fog_raster_init_sse2(void)
{
  using namespace Fog;
  using namespace Fog::Raster;

  Mask0080008000800080 = pix_create_mask_8x2W(0x0080, 0x0080, 0x0080, 0x0080);
  Mask00FF00FF00FF00FF = pix_create_mask_8x2W(0x00FF, 0x00FF, 0x00FF, 0x00FF);
  Mask0101010101010101 = pix_create_mask_8x2W(0x0101, 0x0101, 0x0101, 0x0101);
  MaskFFFFFFFFFFFFFFFF = pix_create_mask_8x2W(0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF);
  Mask00FF000000000000 = pix_create_mask_8x2W(0x00FF, 0x0000, 0x0000, 0x0000);
  MaskFF000000FF000000 = pix_create_mask_8x2W(0xFF00, 0x0000, 0xFF00, 0x0000);

  FunctionMap* m = functionMap;

  // [Gradient]

  m->gradient.gradient_argb32 = gradient_gradient_argb32_sse2;
  m->gradient.gradient_prgb32 = gradient_gradient_prgb32_sse2;

  // [Raster - Rgb32]

  m->raster_rgb32.pixel = raster_rgb32_pixel_sse2;
  m->raster_rgb32.pixel_a8 = raster_rgb32_pixel_a8_sse2;
  m->raster_rgb32.span_solid = raster_rgb32_span_solid_sse2;
  m->raster_rgb32.span_solid_a8 = raster_rgb32_span_solid_a8_sse2;
  m->raster_rgb32.span_composite[Image::FormatPRGB32] = raster_rgb32_span_composite_rgb32_sse2;
  m->raster_rgb32.span_composite[Image::FormatPRGB32] = raster_rgb32_span_composite_argb32_sse2;
  m->raster_rgb32.span_composite[Image::FormatPRGB32] = raster_rgb32_span_composite_prgb32_sse2;
}
