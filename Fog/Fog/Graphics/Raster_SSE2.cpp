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
#include <Fog/Graphics/Raster.h>
#include <Fog/Graphics/Raster_SSE2.h>

namespace Fog {
namespace Raster {

// ============================================================================
// [Fog::Raster - Ops - GradientSpan]
// ============================================================================

static void Ops_GradientSpan_ARGB32_SSE2(uint8_t* dst, uint32_t c0, uint32_t c1, sysint_t w, sysint_t x1, sysint_t x2)
{
  // This function is using fixed point 9.23 calculation to interpolate between
  // packed 8 bit integers. 9 bits are needed to extend 8 bit value with sign.
  uint8_t* dstCur = dst;

  // Sanity checks.
  FOG_ASSERT(w >= 0 && x1 <= x2);

  sysint_t xw = (x2 - x1);
  if (xw == 0) return;

  // Width is decreased by 1 to fit our gradient schema that first and last
  // points in interpolation are always equal to c0 and c1 recpectively.
  if (w) w--;

  // Loop counter.
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

    // Align.
    while (((sysuint_t)dstCur & 15) != 0)
    {
      xmm2 = xmm0;                               // xmm2 = [xAxxxRxxxGxxxBxx]
      xmm0 = _mm_add_epi32(xmm0, xmm1);          // xmm0 += xmm1

      xmm2 = _mm_packus_epi16(xmm2, xmm2);       // xmm2 = [AxRxGxBxAxRxGxBx]
      xmm2 = _mm_srai_epi16(xmm2, 8);            // xmm2 = [fAfRfGfBfAfRfGfB]
      xmm2 = _mm_packs_epi16(xmm2, xmm2);        // xmm2 = [ARGBARGBARGBARGB]

      ((int *)dstCur)[0] = _mm_cvtsi128_si32(xmm2);
      dstCur += 4;
      if (--i == 0) return;
    }

    // Loop: 4 pixels at time.
    while ((i -= 4) >= 0)
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
      xmm2 = _mm_srai_epi16(xmm2, 8);            // xmm2 = [xAxRxGxBxAxRxGxB]
      xmm4 = _mm_srai_epi16(xmm4, 8);            // xmm4 = [xAxRxGxBxAxRxGxB]
      xmm2 = _mm_packs_epi16(xmm2, xmm4);        // xmm2 = [ARGBARGBARGBARGB]

      _mm_store_si128((__m128i *)dstCur, xmm2);

      dstCur += 16;
      i += 4;
    }

    i += 4;

    // Tail.
    while (i)
    {
      xmm2 = xmm0;                               // xmm2 = [xAxxxRxxxGxxxBxx]
      xmm0 = _mm_add_epi32(xmm0, xmm1);          // xmm0 += xmm1

      xmm2 = _mm_packus_epi16(xmm2, xmm2);       // xmm2 = [AxRxGxBxAxRxGxBx]
      xmm2 = _mm_srai_epi16(xmm2, 8);            // xmm2 = [fAfRfGfBfAfRfGfB]
      xmm2 = _mm_packs_epi16(xmm2, xmm2);        // xmm2 = [ARGBARGBARGBARGB]

      ((int *)dstCur)[0] = _mm_cvtsi128_si32(xmm2);
      dstCur += 4;
      i--;
    }

    if (x1 == x2) return;
  }

  // Fill c1 after gradient end.
  i = x2 - x1;
  do { set4(dstCur, c1); dstCur += 4; } while (--i);
}

} // Raster namespace
} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE void fog_raster_init_sse2(void)
{
  using namespace Fog;

  Raster::Ops* o = Raster::ops;
  o->gradientSpanARGB32 = Raster::Ops_GradientSpan_ARGB32_SSE2;
}
