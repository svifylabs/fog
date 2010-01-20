// [Fog/Graphics Library - Private API]
//
// [Licence]
// MIT, See COPYING file in package

// For some IDEs to enable code-assist.
#include <Fog/Build/Build.h>

#if defined(FOG_IDE)
#include <Fog/Graphics/RasterUtil/RasterUtil_Defs_MMX_p.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Dib_C_p.h>
#endif // FOG_IDE

namespace Fog {
namespace RasterUtil {

// ============================================================================
// [Fog::RasterUtil::MMX - Dib]
// ============================================================================

struct FOG_HIDDEN DibMMX
{
  // XXXXXXXX RRRRRRRR GGGGGGGG BBBBBBBB ->
  //                   RRRRRGGG GGGBBBBB
  static void FOG_FASTCALL rgb16_565_native_from_xrgb32(
    uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
  {
    sysint_t i;
    FOG_ASSERT(w);

    // Align destination to 8 bytes.
    while ((sysuint_t)dst & 7)
    {
      uint32_t pix0 = READ_32(src);
      ((uint16_t*)dst)[0] = (uint16_t)(
        ((pix0 >> 8) & 0xF800) |
        ((pix0 >> 5) & 0x07E0) |
        ((pix0 >> 3) & 0x001F));
      if (--w == 0) return;

      dst += 2;
      src += 4;
    }

    for (i = w >> 3; i; i--, dst += 16, src += 32)
    {
      __m64 pix0mmR, pix1mmR;
      __m64 pix0mmG, pix1mmG;
      __m64 pix0mmB, pix1mmB;

      mmx_load8(pix0mmB, src);
      mmx_load8(pix0mmB, src + 8);

      pix0mmR = _mm_srli_pi32(pix0mmB, 8);
      pix1mmR = _mm_srli_pi32(pix1mmB, 8);

      pix0mmG = _mm_srli_pi32(pix0mmB, 5);
      pix1mmG = _mm_srli_pi32(pix1mmB, 5);

      pix0mmB = _mm_srli_pi32(pix0mmB, 3);
      pix1mmB = _mm_srli_pi32(pix1mmB, 3);

      pix0mmR = _mm_and_si128(pix0mmR, Mask_F800F800F800F800);
      pix1mmR = _mm_and_si128(pix1mmR, Mask_F800F800F800F800);

      pix0mmG = _mm_and_si128(pix0mmG, Mask_07E007E007E007E0);
      pix1mmG = _mm_and_si128(pix1mmG, Mask_07E007E007E007E0);

      pix0mmB = _mm_and_si128(pix0mmB, Mask_001F001F001F001F);
      pix1mmB = _mm_and_si128(pix1mmB, Mask_001F001F001F001F);

      pix0mmB = _mm_or_si128(pix0mmB, pix0mmR);
      pix1mmB = _mm_or_si128(pix1mmB, pix1mmR);

      mmx_load8(pix0mmR, src + 16);
      mmx_load8(pix0mmR, src + 24);

      pix0mmB = _mm_or_si128(pix0mmB, pix0mmG);
      pix1mmB = _mm_or_si128(pix1mmB, pix1mmG);

      pix0mmB = _mm_packs_epi32(pix0mmB, pix0mmB);
      pix1mmB = _mm_packs_epi32(pix1mmB, pix1mmB);

      pix0mmB = _mm_packs_pu16(pix0mmB, pix1mmB);
      mmx_store8(dst, pix0mmB);

      pix0mmG = _mm_srli_pi32(pix0mmR, 5);
      pix1mmG = _mm_srli_pi32(pix1mmR, 5);

      pix0mmB = _mm_srli_pi32(pix0mmR, 3);
      pix1mmB = _mm_srli_pi32(pix1mmR, 3);

      pix0mmR = _mm_srli_pi32(pix0mmR, 8);
      pix1mmR = _mm_srli_pi32(pix1mmR, 8);

      pix0mmR = _mm_and_si128(pix0mmR, Mask_F800F800F800F800);
      pix1mmR = _mm_and_si128(pix1mmR, Mask_F800F800F800F800);

      pix0mmG = _mm_and_si128(pix0mmG, Mask_07E007E007E007E0);
      pix1mmG = _mm_and_si128(pix1mmG, Mask_07E007E007E007E0);

      pix0mmB = _mm_and_si128(pix0mmB, Mask_001F001F001F001F);
      pix1mmB = _mm_and_si128(pix1mmB, Mask_001F001F001F001F);

      pix0mmB = _mm_or_si128(pix0mmB, pix0mmR);
      pix1mmB = _mm_or_si128(pix1mmB, pix1mmR);

      pix0mmB = _mm_or_si128(pix0mmB, pix0mmG);
      pix1mmB = _mm_or_si128(pix1mmB, pix1mmG);

      pix0mmB = _mm_packs_epi32(pix0mmB, pix0mmB);
      pix1mmB = _mm_packs_epi32(pix1mmB, pix1mmB);

      pix0mmB = _mm_packs_pu16(pix0mmB, pix1mmB);
      mmx_store8(dst + 8, pix0mmB);
    }

    for (i = w & 7; i; i--, dst += 2, src += 4)
    {
      uint32_t pix0 = READ_32(src);
      ((uint16_t*)dst)[0] = (uint16_t)(
        ((pix0 >> 8) & 0xF800) |
        ((pix0 >> 5) & 0x07E0) |
        ((pix0 >> 3) & 0x001F));
    }

    _mm_empty();
  }
};

} // RasterUtil namespace
} // Fog namespace
