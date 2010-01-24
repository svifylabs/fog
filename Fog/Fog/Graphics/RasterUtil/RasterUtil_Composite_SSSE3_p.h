// [Fog/Graphics Library - Private API]
//
// [Licence]
// MIT, See COPYING file in package

// For some IDEs to enable code-assist.
#include <Fog/Build/Build.h>

#if defined(FOG_IDE)
#include <Fog/Graphics/RasterUtil/RasterUtil_Defs_SSSE3_p.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Dib_SSSE3_p.h>

#include "RasterUtil_Defs_SSE2_p.h"
#endif // FOG_IDE

namespace Fog {
namespace RasterUtil {

// ============================================================================
// [Fog::RasterUtil::SSSE3 - Composite - SrcOver]
// ============================================================================

SSE2_DECLARE_CONST_PI8_VAR(SSSE3_unpack_argb_1x1W_from_1x1B,
  0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x80,
  0x80, 0x03, 0x80, 0x02,
  0x80, 0x01, 0x80, 0x00);

SSE2_DECLARE_CONST_PI8_VAR(SSSE3_unpack_argb_2x1W_from_4x1B_lo,
  0x80, 0x07, 0x80, 0x06,
  0x80, 0x05, 0x80, 0x04,
  0x80, 0x03, 0x80, 0x02,
  0x80, 0x01, 0x80, 0x00);

SSE2_DECLARE_CONST_PI8_VAR(SSSE3_unpack_argb_2x1W_from_4x1B_hi,
  0x80, 0x0F, 0x80, 0x0E,
  0x80, 0x0D, 0x80, 0x0C,
  0x80, 0x0B, 0x80, 0x0A,
  0x80, 0x09, 0x80, 0x08);

SSE2_DECLARE_CONST_PI8_VAR(SSSE3_unpack_0rgb_1x1W_from_1x1B,
  0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x02,
  0x80, 0x01, 0x80, 0x00);

SSE2_DECLARE_CONST_PI8_VAR(SSSE3_unpack_aaaa_1x1W_from_1x1B,
  0x80, 0x80, 0x80, 0x80,
  0x80, 0x80, 0x80, 0x80,
  0x80, 0x03, 0x80, 0x03,
  0x80, 0x03, 0x80, 0x03);

SSE2_DECLARE_CONST_PI8_VAR(SSSE3_unpack_aaaa_2x1W_from_4x1B_lo,
  0x80, 0x07, 0x80, 0x07,
  0x80, 0x07, 0x80, 0x07,
  0x80, 0x03, 0x80, 0x03,
  0x80, 0x03, 0x80, 0x03);

SSE2_DECLARE_CONST_PI8_VAR(SSSE3_unpack_aaaa_2x1W_from_4x1B_hi,
  0x80, 0x0F, 0x80, 0x0F,
  0x80, 0x0F, 0x80, 0x0F,
  0x80, 0x0B, 0x80, 0x0B,
  0x80, 0x0B, 0x80, 0x0B);

template<typename T>
static FOG_INLINE void ssse3_load16u(__m128i& dst, const T* src)
{
  dst = _mm_lddqu_si128(reinterpret_cast<const __m128i*>(src));
}

static FOG_INLINE void ssse3_unpack_argb_1x1W_from_1x1B(__m128i& dst, const __m128i& src)
{
  dst = _mm_shuffle_epi8(src, SSE2_GET_CONST_PI(SSSE3_unpack_argb_1x1W_from_1x1B));
}

static FOG_INLINE void ssse3_unpack_argb_2x1W_from_4x1B_lo(__m128i& dst, const __m128i& src)
{
  dst = _mm_shuffle_epi8(src, SSE2_GET_CONST_PI(SSSE3_unpack_argb_2x1W_from_4x1B_lo));
}

static FOG_INLINE void ssse3_unpack_argb_2x1W_from_4x1B_hi(__m128i& dst, const __m128i& src)
{
  dst = _mm_shuffle_epi8(src, SSE2_GET_CONST_PI(SSSE3_unpack_argb_2x1W_from_4x1B_hi));
}

static FOG_INLINE void ssse3_unpack_0rgb_1x1W_from_1x1B(__m128i& dst, const __m128i& src)
{
  dst = _mm_shuffle_epi8(src, SSE2_GET_CONST_PI(SSSE3_unpack_0rgb_1x1W_from_1x1B));
}

static FOG_INLINE void ssse3_unpack_aaaa_1x1W_from_1x1B(__m128i& dst, const __m128i& src)
{
  dst = _mm_shuffle_epi8(src, SSE2_GET_CONST_PI(SSSE3_unpack_aaaa_1x1W_from_1x1B));
}

static FOG_INLINE void ssse3_unpack_aaaa_2x1W_from_4x1B_lo(__m128i& dst, const __m128i& src)
{
  dst = _mm_shuffle_epi8(src, SSE2_GET_CONST_PI(SSSE3_unpack_aaaa_2x1W_from_4x1B_lo));
}

static FOG_INLINE void ssse3_unpack_aaaa_2x1W_from_4x1B_hi(__m128i& dst, const __m128i& src)
{
  dst = _mm_shuffle_epi8(src, SSE2_GET_CONST_PI(SSSE3_unpack_aaaa_2x1W_from_4x1B_hi));
}

struct FOG_HIDDEN CompositeSrcOverSSSE3
{
  // -------------------------------------------------------------------------
  // [CompositeSrcOverSSE2 - PRGB32]
  // -------------------------------------------------------------------------

  // OPERATOR_SRC_OVER implementation for newest architectures with SSSE3
  // support. Idea is very simple. These architectures are really powerful,
  // so I all jumps were eliminated and inner loop was unrolled a lot.
  static void FOG_FASTCALL prgb32_vspan_prgb32(
    uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
  {
    FOG_ASSERT(w);

    while ((sysuint_t)dst & 15)
    {
      __m128i src0xmm;
      __m128i dst0xmm;
      __m128i msk0xmm;

      sse2_load4(src0xmm, src);
      sse2_load4(dst0xmm, dst);

      sse2_unpack_1x1W(msk0xmm, src0xmm);
      sse2_unpack_1x1W(dst0xmm, dst0xmm);

      sse2_negate_1x1W(msk0xmm, msk0xmm);
      sse2_muldiv255_1x1W(dst0xmm, dst0xmm, msk0xmm);

      sse2_pack_1x1W(dst0xmm, dst0xmm);
      sse2_adds_1x1B(src0xmm, src0xmm, dst0xmm);

      sse2_store4(dst, src0xmm);
      if (--w == 0) return;

      src += 4;
      dst += 4;
    }

    sysint_t i = w;

#if defined(FOG_ARCH_X86_64)
goto start;

skip:
    src += 64;
    dst += 64;
start:
    i -= 16;
    while (i >= 0)
    {
      __m128i src0xmm, inv0xmm;
      __m128i src2xmm, inv2xmm;
      __m128i src4xmm, src6xmm;

      __m128i dst0xmm, dst1xmm;
      __m128i dst2xmm, dst3xmm;
      __m128i dst4xmm, dst6xmm;

      ssse3_load16u(src0xmm, src     );
      ssse3_load16u(src2xmm, src + 16);
      ssse3_load16u(src4xmm, src + 32);
      ssse3_load16u(src6xmm, src + 48);

      inv0xmm = _mm_packs_epi16(src0xmm, src2xmm);
      inv2xmm = _mm_packs_epi16(src4xmm, src6xmm);

      inv0xmm = _mm_packs_epi16(inv0xmm, inv2xmm);
      inv0xmm = _mm_cmpeq_epi8(inv0xmm, _mm_setzero_si128());

      int k = _mm_movemask_epi8(inv0xmm);
      if (k == 0xFFFF) goto skip;

      sse2_load16a(dst0xmm, dst     );
      sse2_load16a(dst2xmm, dst + 16);
      ssse3_unpack_argb_2x1W_from_4x1B_lo(dst1xmm, dst0xmm);
      sse2_load16a(dst4xmm, dst + 32);
      ssse3_unpack_argb_2x1W_from_4x1B_hi(dst3xmm, dst0xmm);
      sse2_load16a(dst6xmm, dst + 48);

      ssse3_unpack_aaaa_2x1W_from_4x1B_lo(inv0xmm, src0xmm);
      ssse3_unpack_aaaa_2x1W_from_4x1B_hi(inv2xmm, src0xmm);

      sse2_muldiv255_2x2W(dst1xmm, dst1xmm, inv0xmm, dst3xmm, dst3xmm, inv2xmm);
      ssse3_unpack_aaaa_2x1W_from_4x1B_lo(inv0xmm, src2xmm);
      sse2_pack_2x2W(dst1xmm, dst1xmm, dst3xmm);
      ssse3_unpack_aaaa_2x1W_from_4x1B_hi(inv2xmm, src2xmm);
      sse2_subs_1x4B(dst0xmm, dst0xmm, dst1xmm);

      ssse3_unpack_argb_2x1W_from_4x1B_lo(dst1xmm, dst2xmm);
      ssse3_unpack_argb_2x1W_from_4x1B_hi(dst3xmm, dst2xmm);

      sse2_muldiv255_2x2W(dst1xmm, dst1xmm, inv0xmm, dst3xmm, dst3xmm, inv2xmm);
      ssse3_unpack_aaaa_2x1W_from_4x1B_lo(inv0xmm, src4xmm);
      sse2_adds_1x4B(src0xmm, src0xmm, dst0xmm);
      ssse3_unpack_aaaa_2x1W_from_4x1B_hi(inv2xmm, src4xmm);
      sse2_pack_2x2W(dst1xmm, dst1xmm, dst3xmm);
      ssse3_unpack_argb_2x1W_from_4x1B_hi(dst3xmm, dst4xmm);
      sse2_subs_1x4B(dst2xmm, dst2xmm, dst1xmm);
      ssse3_unpack_argb_2x1W_from_4x1B_lo(dst1xmm, dst4xmm);

      sse2_muldiv255_2x2W(dst1xmm, dst1xmm, inv0xmm, dst3xmm, dst3xmm, inv2xmm);
      ssse3_unpack_aaaa_2x1W_from_4x1B_lo(inv0xmm, src6xmm);
      sse2_pack_2x2W(dst1xmm, dst1xmm, dst3xmm);
      ssse3_unpack_argb_2x1W_from_4x1B_hi(dst3xmm, dst6xmm);
      sse2_subs_1x4B(dst4xmm, dst4xmm, dst1xmm);
      ssse3_unpack_argb_2x1W_from_4x1B_lo(dst1xmm, dst6xmm);
      sse2_adds_1x4B(src4xmm, src4xmm, dst4xmm);
      ssse3_unpack_aaaa_2x1W_from_4x1B_hi(inv2xmm, src6xmm);

      sse2_muldiv255_2x2W(dst1xmm, dst1xmm, inv0xmm, dst3xmm, dst3xmm, inv2xmm);
      sse2_adds_1x4B(src2xmm, src2xmm, dst2xmm);
      sse2_pack_2x2W(dst1xmm, dst1xmm, dst3xmm);

      sse2_store16a(dst     , src0xmm);
      sse2_subs_1x4B(dst6xmm, dst6xmm, dst1xmm);
      sse2_store16a(dst + 16, src2xmm);
      sse2_adds_1x4B(src6xmm, src6xmm, dst6xmm);
      sse2_store16a(dst + 32, src4xmm);
      sse2_store16a(dst + 48, src6xmm);

      src += 64;
      dst += 64;

      i -= 16;
    }
    i += 16;
#endif // FOG_ARCH_X86_64

/*
#if defined(FOG_ARCH_X86_64)
goto start;

skip:
    src += 64;
    dst += 64;
start:
    i -= 16;
    while (i >= 0)
    {
      __m128i src0xmm, inv0xmm;
      __m128i src2xmm, inv2xmm;
      __m128i src4xmm, src6xmm;

      __m128i dst0xmm, dst1xmm;
      __m128i dst2xmm, dst3xmm;
      __m128i dst4xmm, dst6xmm;

      ssse3_load16u(src0xmm, src     );
      ssse3_load16u(src2xmm, src + 16);
      ssse3_load16u(src4xmm, src + 32);
      ssse3_load16u(src6xmm, src + 48);

      inv0xmm = _mm_packs_epi16(src0xmm, src2xmm);
      inv2xmm = _mm_packs_epi16(src4xmm, src6xmm);

      src0xmm = _mm_xor_si128(src0xmm, SSE2_GET_CONST_PI(FF000000FF000000_FF000000FF000000));
      inv0xmm = _mm_packs_epi16(inv0xmm, inv2xmm);

      src2xmm = _mm_xor_si128(src2xmm, SSE2_GET_CONST_PI(FF000000FF000000_FF000000FF000000));
      inv0xmm = _mm_cmpeq_epi8(inv0xmm, _mm_setzero_si128());

      int k = _mm_movemask_epi8(inv0xmm);
      if (k == 0xFFFF) goto skip;

      sse2_load16a(dst0xmm, dst     );
      sse2_load16a(dst2xmm, dst + 16);
      sse2_load16a(dst4xmm, dst + 32);
      sse2_load16a(dst6xmm, dst + 48);

      ssse3_unpack_argb_2x1W_from_4x1B_hi(dst1xmm, dst0xmm);
      ssse3_unpack_argb_2x1W_from_4x1B_hi(dst3xmm, dst2xmm);

      ssse3_unpack_aaaa_2x1W_from_4x1B_hi(inv0xmm, src0xmm);
      ssse3_unpack_aaaa_2x1W_from_4x1B_hi(inv2xmm, src2xmm);

      sse2_muldiv255_2x2W(dst1xmm, dst1xmm, inv0xmm, dst3xmm, dst3xmm, inv2xmm);

      ssse3_unpack_aaaa_2x1W_from_4x1B_lo(inv0xmm, src0xmm);
      ssse3_unpack_aaaa_2x1W_from_4x1B_lo(inv2xmm, src2xmm);

      src0xmm = _mm_xor_si128(src0xmm, SSE2_GET_CONST_PI(FF000000FF000000_FF000000FF000000));
      src2xmm = _mm_xor_si128(src2xmm, SSE2_GET_CONST_PI(FF000000FF000000_FF000000FF000000));

      ssse3_unpack_argb_2x1W_from_4x1B_lo(dst0xmm, dst0xmm);
      ssse3_unpack_argb_2x1W_from_4x1B_lo(dst2xmm, dst2xmm);

      sse2_muldiv255_2x2W(dst0xmm, dst0xmm, inv0xmm, dst2xmm, dst2xmm, inv2xmm);

      sse2_pack_2x2W(dst0xmm, dst0xmm, dst1xmm);
      sse2_pack_2x2W(dst2xmm, dst2xmm, dst3xmm);

      sse2_adds_1x4B(src0xmm, src0xmm, dst0xmm);
      sse2_adds_1x4B(src2xmm, src2xmm, dst2xmm);

      ssse3_unpack_argb_2x1W_from_4x1B_hi(dst1xmm, dst4xmm);
      ssse3_unpack_argb_2x1W_from_4x1B_hi(dst3xmm, dst6xmm);

      src4xmm = _mm_xor_si128(src4xmm, SSE2_GET_CONST_PI(FF000000FF000000_FF000000FF000000));
      src6xmm = _mm_xor_si128(src6xmm, SSE2_GET_CONST_PI(FF000000FF000000_FF000000FF000000));

      ssse3_unpack_aaaa_2x1W_from_4x1B_hi(inv0xmm, src4xmm);
      ssse3_unpack_aaaa_2x1W_from_4x1B_hi(inv2xmm, src6xmm);

      sse2_muldiv255_2x2W(dst1xmm, dst1xmm, inv0xmm, dst3xmm, dst3xmm, inv2xmm);

      ssse3_unpack_aaaa_2x1W_from_4x1B_lo(inv0xmm, src4xmm);
      ssse3_unpack_aaaa_2x1W_from_4x1B_lo(inv2xmm, src6xmm);

      src4xmm = _mm_xor_si128(src4xmm, SSE2_GET_CONST_PI(FF000000FF000000_FF000000FF000000));
      src6xmm = _mm_xor_si128(src6xmm, SSE2_GET_CONST_PI(FF000000FF000000_FF000000FF000000));

      ssse3_unpack_argb_2x1W_from_4x1B_lo(dst4xmm, dst4xmm);
      ssse3_unpack_argb_2x1W_from_4x1B_lo(dst6xmm, dst6xmm);

      sse2_muldiv255_2x2W(dst4xmm, dst4xmm, inv0xmm, dst6xmm, dst6xmm, inv2xmm);

      sse2_pack_2x2W(dst4xmm, dst4xmm, dst1xmm);
      sse2_pack_2x2W(dst6xmm, dst6xmm, dst3xmm);

      sse2_adds_1x4B(src4xmm, src4xmm, dst4xmm);
      sse2_adds_1x4B(src6xmm, src6xmm, dst6xmm);

      sse2_store16a(dst     , src0xmm);
      sse2_store16a(dst + 16, src2xmm);
      sse2_store16a(dst + 32, src4xmm);
      sse2_store16a(dst + 48, src6xmm);

      src += 64;
      dst += 64;

      i -= 16;
    }
    i += 16;
#endif // FOG_ARCH_X86_64
*/
    i -= 4;
    while (i >= 0)
    {
      __m128i src0xmm;
      __m128i inv0xmm, inv1xmm;
      __m128i dst0xmm, dst1xmm;

      ssse3_load16u(src0xmm, src);
      sse2_load16a(dst0xmm, dst);

      src0xmm = _mm_xor_si128(src0xmm, SSE2_GET_CONST_PI(FF000000FF000000_FF000000FF000000));

      ssse3_unpack_argb_2x1W_from_4x1B_hi(dst1xmm, dst0xmm);
      ssse3_unpack_argb_2x1W_from_4x1B_lo(dst0xmm, dst0xmm);

      ssse3_unpack_aaaa_2x1W_from_4x1B_hi(inv1xmm, src0xmm);
      ssse3_unpack_aaaa_2x1W_from_4x1B_lo(inv0xmm, src0xmm);

      sse2_muldiv255_2x2W(dst0xmm, dst0xmm, inv0xmm, dst1xmm, dst1xmm, inv1xmm);

      src0xmm = _mm_xor_si128(src0xmm, SSE2_GET_CONST_PI(FF000000FF000000_FF000000FF000000));

      sse2_pack_2x2W(dst0xmm, dst0xmm, dst1xmm);
      sse2_adds_1x4B(src0xmm, src0xmm, dst0xmm);

      sse2_store16a(dst, src0xmm);

      src += 16;
      dst += 16;

      i -= 4;
    }
    i += 4;

    while (i)
    {
      __m128i src0xmm;
      __m128i dst0xmm;
      __m128i msk0xmm;

      sse2_load4(src0xmm, src);
      sse2_load4(dst0xmm, dst);

      sse2_unpack_1x1W(msk0xmm, src0xmm);
      sse2_unpack_1x1W(dst0xmm, dst0xmm);

      sse2_negate_1x1W(msk0xmm, msk0xmm);
      sse2_muldiv255_1x1W(dst0xmm, dst0xmm, msk0xmm);

      sse2_pack_1x1W(dst0xmm, dst0xmm);
      sse2_adds_1x1B(src0xmm, src0xmm, dst0xmm);

      sse2_store4(dst, src0xmm);

      src += 4;
      dst += 4;

      i--;
    }
  }
};

} // RasterUtil namespace
} // Fog namespace
