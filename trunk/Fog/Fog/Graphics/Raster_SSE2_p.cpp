// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Cpu/Intrin_SSE2.h>
#include <Fog/Graphics/Raster_p.h>
#include <Fog/Graphics/Raster_SSE2_p.h>

namespace Fog {
namespace Raster {

// ============================================================================
// [SSE2]
// ============================================================================

static __m128i Mask0080008000800080;
static __m128i Mask00FF00FF00FF00FF;
static __m128i Mask0101010101010101;
static __m128i MaskFFFFFFFFFFFFFFFF;
static __m128i Mask00FF000000000000;

static FOG_INLINE __m128i createMask8x2(uint16_t m0, uint16_t m1, uint16_t m2, uint16_t m3)
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

static FOG_INLINE __m128i unpack_32_1x128(uint32_t data)
{
  __m128i mmz = _mm_setzero_si128();
  return _mm_unpacklo_epi8(_mm_cvtsi32_si128(data), mmz);
}

static FOG_INLINE void unpack_128_2x128(__m128i& dst0, __m128i& dst1, __m128i data)
{
  __m128i mmz = _mm_setzero_si128();
  dst0 = _mm_unpacklo_epi8 (data, mmz);
  dst1 = _mm_unpackhi_epi8 (data, mmz);
}

static FOG_INLINE __m128i pack_2x128_128(__m128i src0, __m128i src1)
{
  return _mm_packus_epi16(src0, src1);
}

static FOG_INLINE uint32_t packAlpha(__m128i x)
{
  __m128i mmz = _mm_setzero_si128();
  __m128i tmp = _mm_srli_epi32(x, 24);
  tmp = _mm_packus_epi16(tmp, mmz);
  tmp = _mm_packus_epi16(tmp, mmz);
  return _mm_cvtsi128_si32(tmp);
}

static FOG_INLINE __m128i expandPixel_32_1x128(uint32_t data)
{
  return _mm_shuffle_epi32(unpack_32_1x128(data), _MM_SHUFFLE(1, 0, 1, 0));
}

template<int X, int Y, int Z, int W>
static FOG_INLINE __m128i shuffle_1x128(__m128i pix)
{
  pix = _mm_shufflelo_epi16(pix, _MM_SHUFFLE(X, Y, Z, W));
  return _mm_shufflehi_epi16(pix, _MM_SHUFFLE(X, Y, Z, W));
}

static FOG_INLINE void expandAlpha_1x128(
  __m128i& dst0, __m128i& src0)
{
  dst0 = shuffle_1x128<3, 3, 3, 3>(src0);
}

static FOG_INLINE void expandAlpha_2x128(
  __m128i& dst0, __m128i src0,
  __m128i& dst1, __m128i src1)
{
  dst0 = shuffle_1x128<3, 3, 3, 3>(src0);
  dst1 = shuffle_1x128<3, 3, 3, 3>(src1);
}

static FOG_INLINE void expandAlphaRev_1x128(
  __m128i& dst0, __m128i src0)
{
  dst0 = shuffle_1x128<0, 0, 0, 0>(src0);
}

static FOG_INLINE void expandAlphaRev_2x128(
  __m128i& dst0, __m128i src0,
  __m128i& dst1, __m128i src1)
{
  dst0 = shuffle_1x128<0, 0, 0, 0>(src0);
  dst1 = shuffle_1x128<0, 0, 0, 0>(src1);
}

static FOG_INLINE void pixMultiply_1x128(
  __m128i& dst0, __m128i& data0, __m128i& alpha0)
{
  __m128i t0;

  t0 = _mm_mullo_epi16(data0, alpha0);
  t0 = _mm_adds_epu16(t0, Mask0080008000800080);
  dst0 = _mm_mulhi_epu16(t0, Mask0101010101010101);
}

static FOG_INLINE void pixMultiply_2x128(
  __m128i& dst0, __m128i& data0, __m128i& alpha0,
  __m128i& dst1, __m128i& data1, __m128i& alpha1)
{
  __m128i t0, t1;

  t0 = _mm_mullo_epi16(data0, alpha0);
  t1 = _mm_mullo_epi16(data1, alpha1);
  t0 = _mm_adds_epu16(t0, Mask0080008000800080);
  t1 = _mm_adds_epu16(t1, Mask0080008000800080);
  dst0 = _mm_mulhi_epu16(t0, Mask0101010101010101);
  dst1 = _mm_mulhi_epu16(t1, Mask0101010101010101);
}

static FOG_INLINE void pixPremultiply_1x128(
  __m128i& dst0, __m128i& src0)
{
  __m128i alpha0;

  expandAlpha_1x128(alpha0, src0);
  _mm_or_si128(alpha0, Mask00FF000000000000);
  pixMultiply_1x128(dst0, src0, alpha0);
}

static FOG_INLINE void pixPremultiply_2x128(
  __m128i& dst0, __m128i& src0,
  __m128i& dst1, __m128i& src1)
{
  __m128i alpha0;
  __m128i alpha1;

  expandAlpha_2x128(
    alpha0, src0,
    alpha1, src1);
  _mm_or_si128(alpha0, Mask00FF000000000000);
  _mm_or_si128(alpha1, Mask00FF000000000000);
  pixMultiply_2x128(
    dst0, src0, alpha0,
    dst1, src1, alpha1);
}

// ============================================================================
// [Fog::Raster - Gradient - gradient]
// ============================================================================

static void FOG_FASTCALL gradient_gradient_argb32_SSE2(uint8_t* dst, uint32_t c0, uint32_t c1, sysint_t w, sysint_t x1, sysint_t x2)
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

static void FOG_FASTCALL gradient_gradient_prgb32_SSE2(uint8_t* dst, uint32_t c0, uint32_t c1, sysint_t w, sysint_t x1, sysint_t x2)
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
      pixPremultiply_1x128(xmm2, xmm2);
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
      pixPremultiply_2x128(xmm2, xmm2, xmm4, xmm4);
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
      pixPremultiply_1x128(xmm2, xmm2);
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

} // Raster namespace
} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE void fog_raster_init_sse2(void)
{
  Fog::Raster::Mask0080008000800080 = Fog::Raster::createMask8x2(0x0080, 0x0080, 0x0080, 0x0080);
  Fog::Raster::Mask00FF00FF00FF00FF = Fog::Raster::createMask8x2(0x00FF, 0x00FF, 0x00FF, 0x00FF);
  Fog::Raster::Mask0101010101010101 = Fog::Raster::createMask8x2(0x0101, 0x0101, 0x0101, 0x0101);
  Fog::Raster::MaskFFFFFFFFFFFFFFFF = Fog::Raster::createMask8x2(0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF);
  Fog::Raster::Mask00FF000000000000 = Fog::Raster::createMask8x2(0x00FF, 0x0000, 0x0000, 0x0000);

  Fog::Raster::FunctionMap* m = Fog::Raster::functionMap;

  m->gradient.gradient_argb32 = Fog::Raster::gradient_gradient_argb32_SSE2;
  m->gradient.gradient_prgb32 = Fog::Raster::gradient_gradient_prgb32_SSE2;
}
