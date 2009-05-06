// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Cpu/Intrin_MMX.h>
#include <Fog/Graphics/Raster_p.h>
#include <Fog/Graphics/Raster_MMX_p.h>

namespace Fog {
namespace Raster {

// ============================================================================
// [Fog::Raster - Gradient - gradient]
// ============================================================================

static void FOG_FASTCALL gradient_gradient_argb32_MMX(uint8_t* dst, uint32_t c0, uint32_t c1, sysint_t w, sysint_t x1, sysint_t x2)
{
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
    __m64 mmz;
    __m64 mm0, mm1;
    __m64 mm2, mm3;
    __m64 mm6, mm7;

    mmx_t tmpAR;
    mmx_t tmpGB;

    mmz = _mm_setzero_si64();
    mm0 = _mm_cvtsi32_si64((int32_t)c0);         // mm0 = [    ARGB] c0
    mm1 = _mm_cvtsi32_si64((int32_t)c1);         // mm1 = [    ARGB] c1

    mm0 = _mm_unpacklo_pi8(mm0, mmz);            // mm0 = [0A0R0G0B] c0
    mm1 = _mm_unpacklo_pi8(mm1, mmz);            // mm1 = [0A0R0G0B] c1

    mm2 = mm0;                                   // mm2 = [0A0R0G0B] c0
    mm3 = mm1;                                   // mm3 = [0A0R0G0B] c1

    mm0 = _mm_unpackhi_pi16(mm0, mmz);           // mm0 = [000A000R] c0
    mm1 = _mm_unpackhi_pi16(mm1, mmz);           // mm1 = [000A000R] c1

    mm2 = _mm_unpacklo_pi16(mm2, mmz);           // mm2 = [000G000B] c0
    mm3 = _mm_unpacklo_pi16(mm3, mmz);           // mm3 = [000G000B] c1

    mm0 = _mm_slli_pi32(mm0, 16);                // mm0 = [0A000R00] c0
    mm1 = _mm_slli_pi32(mm1, 16);                // mm1 = [0A000R00] c1

    mm2 = _mm_slli_pi32(mm2, 16);                // mm2 = [0G000B00] c0
    mm3 = _mm_slli_pi32(mm3, 16);                // mm3 = [0G000B00] c1

    mm1 = _mm_sub_pi32(mm1, mm0);                // mm1 = mm1 - mm0
    mm3 = _mm_sub_pi32(mm3, mm2);                // mm3 = mm3 - mm2

    // Divice.
    // 'mm0' and 'mm2' == c0 color and will be incremented by
    // 'mm1' and 'mm3' == c1 color / w
    tmpAR.m64 = mm1;                             // copy mm1 to temporary buffer (AR)
    tmpGB.m64 = mm3;                             // copy mm3 to temporary buffer (GB)

    tmpAR.sd[0] /= (int32_t)w;
    tmpAR.sd[1] /= (int32_t)w;
    tmpGB.sd[0] /= (int32_t)w;
    tmpGB.sd[1] /= (int32_t)w;

    mm1 = tmpAR.m64;                             // mm1 = AR increment
    mm3 = tmpGB.m64;                             // mm3 = GB increment

    // Offset interpolation to x1.
    tmpAR.sd[0] *= (int32_t)x1;
    tmpAR.sd[1] *= (int32_t)x1;
    tmpGB.sd[0] *= (int32_t)x1;
    tmpGB.sd[1] *= (int32_t)x1;

    mm0 = _mm_add_pi32(mm0, tmpAR.m64);          // mm0 = AR c0 + offset
    mm2 = _mm_add_pi32(mm2, tmpGB.m64);          // mm2 = GB c0 + offset

    i = w - x1;
    x1 += i;

    // Align.
    if ((sysuint_t)(dstCur) & 7)
    {
      mm6 = mm2;                                 // mm6 = mm2
      mm6 = _mm_packs_pu16(mm6, mm0);            // mm6 = [A0R0G0B0]
      mm0 = _mm_add_pi32(mm0, mm1);              // increment mm0 by AR step (mm1)
      mm6 = _mm_srli_pi16(mm6, 8);               // mm6 = [0A0R0G0B]
      mm2 = _mm_add_pi32(mm2, mm3);              // increment mm2 by GB step (mm3)
      mm6 = _mm_packs_pu16(mm6, mm6);            // mm6 = [    ARGB]

      ((int *)dstCur)[0] = _mm_cvtsi64_si32(mm6);
      dstCur += 4;
      if (--i == 0) goto end;
    }

    // Loop: Two pixels per time.
    while ((i -= 2) >= 0)
    {
      mm6 = mm2;                                 // mm6 = mm2
      mm6 = _mm_packs_pu16(mm6, mm0);            // mm6 = [A0R0G0B0]

      mm2 = _mm_add_pi32(mm2, mm3);              // increment mm2 by GB step (mm3)
      mm0 = _mm_add_pi32(mm0, mm1);              // increment mm0 by AR step (mm1)

      mm7 = mm2;                                 // mm7 = mm2
      mm7 = _mm_packs_pu16(mm7, mm0);            // mm7 = [A0R0G0B0]

      mm6 = _mm_srli_pi16(mm6, 8);               // mm6 = [0A0R0G0B]
      mm7 = _mm_srli_pi16(mm7, 8);               // mm7 = [0A0R0G0B]

      mm2 = _mm_add_pi32(mm2, mm3);              // increment mm2 by GB step (mm3)
      mm0 = _mm_add_pi32(mm0, mm1);              // increment mm0 by AR step (mm1)

      mm6 = _mm_packs_pu16(mm6, mm7);            // mm6 = [ARGBARGB]

      ((__m64 *)dstCur)[0] = mm6;
      dstCur += 8;
    }
    
    // Tail.
    if ((i += 2) != 0)
    {
      mm6 = mm2;                                 // mm6 = mm2
      mm6 = _mm_packs_pu16(mm6, mm0);            // mm6 = [A0R0G0B0]
      mm0 = _mm_add_pi32(mm0, mm1);              // increment mm0 by AR step (mm1)
      mm6 = _mm_srli_pi16(mm6, 8);               // mm6 = [0A0R0G0B]
      mm2 = _mm_add_pi32(mm2, mm3);              // increment mm2 by GB step (mm3)
      mm6 = _mm_packs_pu16(mm6, mm6);            // mm6 = [    ARGB]

      ((int *)dstCur)[0] = _mm_cvtsi64_si32(mm6);
      dstCur += 4;
    }

    if (x1 == x2) goto end;
  }

  // Fill c1 after gradient end.
  i = x2 - x1;
  do { set4(dstCur, c1); dstCur += 4; } while (--i);

end:
  _mm_empty();
}

} // Raster namespace
} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE void fog_raster_init_mmx(void)
{
  Fog::Raster::FunctionMap* m = Fog::Raster::functionMap;

  m->gradient.gradient_argb32 = Fog::Raster::gradient_gradient_argb32_MMX;
}
