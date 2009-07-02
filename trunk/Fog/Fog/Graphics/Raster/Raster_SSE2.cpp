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

#define READ_8(ptr)  (((const uint8_t *)ptr)[0])
#define READ_16(ptr) (((const uint16_t*)ptr)[0])
#define READ_32(ptr) (((const uint32_t*)ptr)[0])

// These macros were designed to simplify blit functions. The idea is very simple.
// There are usually three loops that you can see in blitters. One is align loop.
// The align loop wants to align data into 16 bytes. When aligning is done, the
// main loop is run and it usually uses 16 byte increments (so 4 ARGB pixels).
// The reason for third loop (tail loop) is that it's needed to process remaining
// pixels that can't fit into main loop.
//
// There is idea to merge align and tail loop into one loop. This will save some
// library binary size (about 1/4 size of blitting functions) and simplify the
// development. So to not repeat dirty stuff in each function there are macros
// that will do everything for you.
//
// The two loops are named SMALL and LARGE.
//
// BLIT_SSE2_INIT(dst, w) - This macro will declare '_i' variable that contains
// initial value for small loop and '_j' variable that contains count of pixels to
// process in tail loop. It also modifies _w variable to contain only how many cycles
// will be processed by large loop (4 pixels at time), not whole width in pixels.
//
// 1 pixel at time:
// - BLIT_SSE2_SMALL_BEGIN(dst) - Small loop begin.
// - BLIT_SSE2_SMALL_END(dst) - Small loop end.
//
// 4 pixels at time:
// - BLIT_SSE2_LARGE_BEGIN(dst) - Main loop begin.
// - BLIT_SSE2_LARGE_END(dst) - Main loop end.
//
// Because compilers can be quite missed from our machinery, it's needed
// to follow some rules to help them to optimize this code:
// - declare temporary variables (mainly sse2 registers) in local loop scope.
// - do not add anything between BLIT_SSE2_SMALL_END and BLIT_SSE2_LARGE_BEGIN.

#define BLIT_SSE2_INIT(_dst, _w) \
  sysuint_t _i = (sysuint_t)_w; \
  sysuint_t _j = 0; \
  \
  if (_i >= 4 && ((_i = ((-(sysuint_t)(_dst)) & 15)) & 3) == 0x0) \
  { \
    _i >>= 2; \
    _w -= (sysint_t)_i; \
    _j = _w & 3; \
    _w >>= 2; \
    if (_w == 0) _i += _j; \
  } \
  else \
  { \
    _i = _w; \
    _w = 0; \
  }

#define BLIT_SSE2_SMALL_BEGIN(group) \
  if (_i) \
  { \
group: \
    do {

#define BLIT_SSE2_SMALL_END(group) \
    } while (--_i); \
    if (!w) return; \
  } \

#define BLIT_SSE2_LARGE_BEGIN(group) \
  do {

#define BLIT_SSE2_LARGE_END(group) \
  } while (--w); \
  \
  if ((_i = _j)) goto group; \
  return;

// ============================================================================
// [Fog::Raster - Constants]
// ============================================================================

static __m128i Mask0080008000800080;
static __m128i Mask000000FF00FF00FF;
static __m128i Mask00FF00FF00FF00FF;
static __m128i MaskFF000000000000FF;
static __m128i Mask0101010101010101;
static __m128i MaskFFFFFFFFFFFFFFFF;
static __m128i Mask00FF000000000000;
static __m128i MaskFF000000FF000000;

static __m128  Mask7FFFFFFF7FFFFFFF;
static __m128d Mask7FFFFFFFFFFFFFFF;

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

static FOG_INLINE void pix_load4(__m128i& dst0, const uint8_t* srcp)
{
  dst0 = _mm_cvtsi32_si128(((int *)srcp)[0]);
}

static FOG_INLINE void pix_load8(__m128i& dst0, const uint8_t* srcp)
{
  dst0 = _mm_loadl_epi64((__m128i*)(srcp));
}

static FOG_INLINE void pix_load16a(__m128i& dst0, const uint8_t* srcp)
{
  dst0 = _mm_load_si128((__m128i *)(srcp));
}

static FOG_INLINE void pix_load16u(__m128i& dst0, const uint8_t* srcp)
{
  dst0 = _mm_loadu_si128((__m128i *)(srcp));
}

static FOG_INLINE void pix_store4(uint8_t* dstp, __m128i& src0)
{
  ((int *)dstp)[0] = _mm_cvtsi128_si32(src0);
}

static FOG_INLINE void pix_store8(uint8_t* dstp, __m128i& src0)
{
  _mm_storel_epi64((__m128i*)(dstp), src0);
}

static FOG_INLINE void pix_store16a(uint8_t* dstp, __m128i& src0)
{
  _mm_store_si128((__m128i *)(dstp), src0);
}

static FOG_INLINE void pix_store16u(uint8_t* dstp, __m128i& src0)
{
  _mm_storeu_si128((__m128i *)(dstp), src0);
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

static FOG_INLINE void pix_pack_alpha(__m128i& dst0, __m128i& src0)
{
  __m128i mmz = _mm_setzero_si128();
  dst0 = src0;

  dst0 = _mm_srli_epi32(dst0, 24);
  dst0 = _mm_packus_epi16(dst0, mmz);
  dst0 = _mm_packus_epi16(dst0, mmz);
}

static FOG_INLINE uint32_t pix_pack_alpha_to_uint32(__m128i& src)
{
  __m128i dst;
  pix_pack_alpha(dst, src);
  return _mm_cvtsi128_si32(dst);
}

static FOG_INLINE void pix_negate_1x1W(
  __m128i& dst0, const __m128i& src0)
{
  dst0 = _mm_xor_si128(src0, Mask00FF00FF00FF00FF);
}

static FOG_INLINE void pix_negate_1x2W(
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

static FOG_INLINE void pix_swap_1x1W(
  __m128i& dst0, const __m128i& src0)
{
  dst0 = _mm_shufflelo_epi16(src0, _MM_SHUFFLE(0, 1, 2, 3));
}

static FOG_INLINE void pix_swap_1x2W(
  __m128i& dst0, const __m128i& src0)
{
  dst0 = _mm_shufflelo_epi16(src0, _MM_SHUFFLE(0, 1, 2, 3));
  dst0 = _mm_shufflehi_epi16(dst0, _MM_SHUFFLE(0, 1, 2, 3));
}

static FOG_INLINE void pix_swap_2x2W(
  __m128i& dst0, const __m128i& src0,
  __m128i& dst1, const __m128i& src1)
{
  dst0 = _mm_shufflelo_epi16(src0, _MM_SHUFFLE(0, 1, 2, 3));
  dst1 = _mm_shufflelo_epi16(src1, _MM_SHUFFLE(0, 1, 2, 3));
  dst0 = _mm_shufflehi_epi16(dst0, _MM_SHUFFLE(0, 1, 2, 3));
  dst1 = _mm_shufflehi_epi16(dst1, _MM_SHUFFLE(0, 1, 2, 3));
}

static FOG_INLINE void pix_expand_pixel_1x1W(
  __m128i& dst0, const __m128i& src0)
{
  dst0 = _mm_shuffle_epi32(src0, _MM_SHUFFLE(1, 0, 1, 0));
}

static FOG_INLINE void pix_expand_pixel_1x2W(
  __m128i& dst0, const __m128i& src0)
{
  dst0 = _mm_shuffle_epi32(src0, _MM_SHUFFLE(1, 0, 1, 0));
}

static FOG_INLINE void pix_expand_pixel_1x4B(
  __m128i& dst0, const __m128i& src0)
{
  dst0 = _mm_shuffle_epi32(src0, _MM_SHUFFLE(0, 0, 0, 0));
}

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

static FOG_INLINE void pix_expand_alpha_rev_1x1W(
  __m128i& dst0, uint32_t src0)
{
  pix_expand_alpha_rev_1x1W(dst0, _mm_cvtsi32_si128(src0));
}

static FOG_INLINE void pix_expand_alpha_rev_1x2W(
  __m128i& dst0, const __m128i& src0)
{
  dst0 = _mm_shufflelo_epi16(src0, _MM_SHUFFLE(0, 0, 0, 0));
  dst0 = _mm_shufflehi_epi16(dst0, _MM_SHUFFLE(0, 0, 0, 0));
}

static FOG_INLINE void pix_expand_alpha_rev_2x2W(
  __m128i& dst0, const __m128i& src0,
  __m128i& dst1, const __m128i& src1)
{
  dst0 = _mm_shufflelo_epi16(src0, _MM_SHUFFLE(0, 0, 0, 0));
  dst1 = _mm_shufflelo_epi16(src1, _MM_SHUFFLE(0, 0, 0, 0));
  dst0 = _mm_shufflehi_epi16(dst0, _MM_SHUFFLE(0, 0, 0, 0));
  dst1 = _mm_shufflehi_epi16(dst1, _MM_SHUFFLE(0, 0, 0, 0));
}

static FOG_INLINE void pix_expand_mask_1x1W(
  __m128i& dst0, uint32_t msk)
{
  pix_expand_alpha_rev_1x1W(dst0, _mm_cvtsi32_si128(msk));
}

static FOG_INLINE void pix_expand_mask_2x2W(
  __m128i& dst0, __m128i& dst1, uint32_t msk)
{
  dst0 = _mm_cvtsi32_si128(msk);
  pix_unpack_1x1W(dst0, dst0);
  dst0 = _mm_shuffle_epi32(dst0, _MM_SHUFFLE(1, 0, 1, 0));

  dst1 = _mm_shufflelo_epi16(dst0, _MM_SHUFFLE(2, 2, 2, 2));
  dst0 = _mm_shufflelo_epi16(dst0, _MM_SHUFFLE(0, 0, 0, 0));
  dst1 = _mm_shufflehi_epi16(dst1, _MM_SHUFFLE(3, 3, 3, 3));
  dst0 = _mm_shufflehi_epi16(dst0, _MM_SHUFFLE(1, 1, 1, 1));
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

static FOG_INLINE void pix_zero_alpha_1x1W(
  __m128i& dst0)
{
  dst0 = _mm_and_si128(dst0, Mask000000FF00FF00FF);
}

static FOG_INLINE void pix_zero_alpha_2x2W(
  __m128i& dst0,
  __m128i& dst1)
{
  dst0 = _mm_and_si128(dst0, Mask000000FF00FF00FF);
  dst1 = _mm_and_si128(dst1, Mask000000FF00FF00FF);
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

// over

static FOG_INLINE void pix_over_ialpha_1x1W(
  __m128i& dst0, const __m128i& src0, const __m128i& ialpha0)
{
  pix_multiply_1x1W(dst0, dst0, ialpha0);
  dst0 = _mm_adds_epu8(dst0, src0);
}

static FOG_INLINE void pix_over_1x1W(
  __m128i& dst0, const __m128i& src0, const __m128i& alpha0)
{
  __m128i ialpha0;

  pix_negate_1x1W(ialpha0, alpha0);
  pix_multiply_1x1W(dst0, dst0, ialpha0);
  dst0 = _mm_adds_epu8(dst0, src0);
}

static FOG_INLINE void pix_over_ialpha_2x2W(
  __m128i& dst0, const __m128i& src0, const __m128i& ialpha0,
  __m128i& dst1, const __m128i& src1, const __m128i& ialpha1)
{
  pix_multiply_2x2W(
    dst0, dst0, ialpha0,
    dst1, dst1, ialpha1);
  dst0 = _mm_adds_epu8(dst0, src0);
  dst1 = _mm_adds_epu8(dst1, src1);
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

static FOG_INLINE void pix_over_1x1W(
  __m128i& dst0, __m128i& src0)
{
  __m128i src0ia;

  pix_expand_alpha_1x1W(src0ia, src0);
  pix_negate_1x1W(src0ia, src0ia);
  pix_over_ialpha_1x1W(dst0, src0, src0ia);
}

static FOG_INLINE void pix_over_2x2W(
  __m128i& dst0, __m128i& src0,
  __m128i& dst1, __m128i& src1)
{
  __m128i src0ia;
  __m128i src1ia;

  pix_expand_alpha_2x2W(src0ia, src0, src1ia, src1);
  pix_negate_2x2W(src0ia, src0ia, src1ia, src1ia);
  pix_over_ialpha_2x2W(dst0, src0, src0ia, dst1, src1, src1ia);
}

// overrev

static FOG_INLINE void pix_overrev_ialpha_1x1W(
  __m128i& dst0, const __m128i& src0, const __m128i& ialpha0)
{
  __m128i t0;
  pix_multiply_1x1W(t0, src0, ialpha0);
  dst0 = _mm_adds_epu8(dst0, t0);
}

static FOG_INLINE void pix_overrev_1x1W(
  __m128i& dst0, const __m128i& src0, const __m128i& alpha0)
{
  __m128i t0;
  __m128i ialpha0;

  pix_negate_1x1W(ialpha0, alpha0);
  pix_multiply_1x1W(t0, src0, ialpha0);
  dst0 = _mm_adds_epu8(dst0, t0);
}

static FOG_INLINE void pix_overrev_ialpha_2x2W(
  __m128i& dst0, const __m128i& src0, const __m128i& ialpha0,
  __m128i& dst1, const __m128i& src1, const __m128i& ialpha1)
{
  __m128i t0;
  __m128i t1;

  pix_multiply_2x2W(
    t0, src0, ialpha0,
    t1, src1, ialpha1);
  dst0 = _mm_adds_epu8(dst0, t0);
  dst1 = _mm_adds_epu8(dst1, t1);
}

static FOG_INLINE void pix_overrev_2x2W(
  __m128i& dst0, const __m128i& src0, const __m128i& alpha0,
  __m128i& dst1, const __m128i& src1, const __m128i& alpha1)
{
  __m128i t0;
  __m128i t1;
  __m128i ialpha0;
  __m128i ialpha1;

  pix_negate_2x2W(
    ialpha0, alpha0,
    ialpha1, alpha1);
  pix_multiply_2x2W(
    t0, src0, ialpha0,
    t1, src1, ialpha1);
  dst0 = _mm_adds_epu8(dst0, t0);
  dst1 = _mm_adds_epu8(dst1, t1);
}

static FOG_INLINE void pix_overrev_1x1W(
  __m128i& dst0, __m128i& src0)
{
  __m128i dst0ia;

  pix_expand_alpha_1x1W(dst0ia, dst0);
  pix_negate_1x1W(dst0ia, dst0ia);
  pix_overrev_ialpha_1x1W(dst0, src0, dst0ia);
}

static FOG_INLINE void pix_overrev_2x2W(
  __m128i& dst0, __m128i& src0,
  __m128i& dst1, __m128i& src1)
{
  __m128i dst0ia;
  __m128i dst1ia;

  pix_expand_alpha_2x2W(dst0ia, dst0, dst1ia, dst1);
  pix_negate_2x2W(dst0ia, dst0ia, dst1ia, dst1ia);
  pix_overrev_ialpha_2x2W(dst0, src0, dst0ia, dst1, src1, dst1ia);
}

// atop

static void FOG_INLINE pix_atop_ialpha_1x1W(
  __m128i& dst0, __m128i& src0, __m128i& src0ia)
{
  __m128i dst0a;

  pix_expand_alpha_1x1W(dst0a, dst0);

  pix_multiply_1x1W(dst0, dst0, src0ia);
  pix_multiply_1x1W(dst0a, dst0a, src0);

  dst0 = _mm_adds_epu8(dst0, dst0a);
}

static void FOG_INLINE pix_atop_ialpha_2x2W(
  __m128i& dst0, __m128i& src0, __m128i& src0ia,
  __m128i& dst1, __m128i& src1, __m128i& src1ia)
{
  __m128i dst0a;
  __m128i dst1a;

  pix_expand_alpha_2x2W(dst0a, dst0, dst1a, dst1);

  pix_multiply_2x2W(dst0, dst0, src0ia, dst1, dst1, src1ia);
  pix_multiply_2x2W(dst0a, dst0a, src0, dst1a, dst1a, src1);

  dst0 = _mm_adds_epu8(dst0, dst0a);
  dst1 = _mm_adds_epu8(dst1, dst1a);
}

static void FOG_INLINE pix_atop_1x1W(
  __m128i& dst0, __m128i& src0, __m128i& src0a)
{
  __m128i src0ia;

  pix_negate_1x1W(src0ia, src0a);
  pix_atop_ialpha_1x1W(dst0, src0, src0ia);
}

static void FOG_INLINE pix_atop_2x2W(
  __m128i& dst0, __m128i& src0, __m128i& src0a,
  __m128i& dst1, __m128i& src1, __m128i& src1a)
{
  __m128i src0ia;
  __m128i src1ia;

  pix_negate_1x1W(src0ia, src0a);
  pix_negate_1x1W(src1ia, src1a);
  pix_atop_ialpha_2x2W(dst0, src0, src0ia, dst1, src1, src1ia);
}

static void FOG_INLINE pix_atop_1x1W(
  __m128i& dst0, __m128i& src0)
{
  __m128i src0ia;
  pix_expand_alpha_1x1W(src0ia, src0);
  pix_negate_1x1W(src0ia, src0ia);
  pix_atop_ialpha_1x1W(dst0, src0, src0ia);
}

static void FOG_INLINE pix_atop_2x2W(
  __m128i& dst0, __m128i& src0,
  __m128i& dst1, __m128i& src1)
{
  __m128i src0ia;
  __m128i src1ia;

  pix_expand_alpha_2x2W(src0ia, src0, src1ia, src1);
  pix_negate_2x2W(src0ia, src0ia, src1ia, src1ia);
  pix_atop_ialpha_2x2W(dst0, src0, src0ia, dst1, src1, src1ia);
}

// atoprev

static void FOG_INLINE pix_atoprev_1x1W(
  __m128i& dst0, const __m128i& src0, const __m128i& srca0)
{
  __m128i dstia0;

  pix_expand_alpha_1x1W(dstia0, dst0);
  pix_negate_1x1W(dstia0, dstia0);

  pix_multiply_1x1W(dst0, dst0, srca0);
  pix_multiply_1x1W(dstia0, dstia0, src0);

  dst0 = _mm_adds_epu8(dst0, dstia0);
}

static void FOG_INLINE pix_atoprev_2x2W(
  __m128i& dst0, const __m128i& src0, const __m128i& srca0,
  __m128i& dst1, const __m128i& src1, const __m128i& srca1)
{
  __m128i dstia0;
  __m128i dstia1;

  pix_expand_alpha_2x2W(dstia0, dst0, dstia1, dst1);
  pix_negate_2x2W(dstia0, dstia0, dstia1, dstia1);

  pix_multiply_2x2W(dst0, dst0, srca0, dst1, dst1, srca1);
  pix_multiply_2x2W(dstia0, dstia0, src0, dstia1, dstia1, src1);

  dst0 = _mm_adds_epu8(dst0, dstia0);
  dst1 = _mm_adds_epu8(dst1, dstia1);
}

// xor

static void FOG_INLINE pix_xor_ialpha_1x1W(
  __m128i& dst0, const __m128i& src0, const __m128i& srcia0)
{
  __m128i dstia0;

  pix_expand_alpha_1x1W(dstia0, dst0);
  pix_negate_1x1W(dstia0, dstia0);

  pix_multiply_1x1W(dst0, dst0, srcia0);
  pix_multiply_1x1W(dstia0, dstia0, src0);

  dst0 = _mm_adds_epu8(dst0, dstia0);
}

static void FOG_INLINE pix_xor_1x1W(
  __m128i& dst0, const __m128i& src0, const __m128i& srca0)
{
  __m128i srcia0;
  pix_negate_1x1W(srcia0, srca0);
  pix_xor_ialpha_1x1W(dst0, src0, srcia0);
}

static void FOG_INLINE pix_xor_ialpha_2x2W(
  __m128i& dst0, const __m128i& src0, const __m128i& srcia0,
  __m128i& dst1, const __m128i& src1, const __m128i& srcia1)
{
  __m128i dstia0;
  __m128i dstia1;

  pix_expand_alpha_2x2W(dstia0, dst0, dstia1, dst1);
  pix_negate_2x2W(dstia0, dstia0, dstia1, dstia1);

  pix_multiply_2x2W(dst0, dst0, srcia0, dst1, dst1, srcia1);
  pix_multiply_2x2W(dstia0, dstia0, src0, dstia1, dstia1, src1);

  dst0 = _mm_adds_epu8(dst0, dstia0);
  dst1 = _mm_adds_epu8(dst1, dstia1);
}

static void FOG_INLINE pix_xor_2x2W(
  __m128i& dst0, const __m128i& src0, const __m128i& srca0,
  __m128i& dst1, const __m128i& src1, const __m128i& srca1)
{
  __m128i srcia0;
  __m128i srcia1;

  pix_negate_2x2W(srcia0, srca0, srcia1, srca1);
  pix_xor_ialpha_2x2W(dst0, src0, srcia0, dst1, src1, srcia1);
}

// fetch

static FOG_INLINE void pix_fetch_rgb24_1x1W(__m128i& dst0, const uint8_t* srcp)
{
  pix_unpack_1x1W(dst0, READ_32(srcp));
  pix_fill_alpha_1x1W(dst0);
}

static FOG_INLINE void pix_fetch_rgb24_2x2W(__m128i& dst0, __m128i& dst1, const uint8_t* srcp)
{
  __m128i xmmz = _mm_setzero_si128();

  pix_load8(dst0, srcp + 0);                                 // dst0 = [G2 B2 R1 G1 B1 R0 G0 B0]
  pix_load8(dst1, srcp + 4);                                 // dst1 = [R3 G3 B3 R2 G2 B2 R1 G1]

  dst0 = _mm_slli_epi64(dst0, 8);                            // dst0 = [B2 R1 G1 B1 R0 G0 B0   ]
  dst1 = _mm_srli_epi64(dst1, 8);                            // dst1 = [   R3 G3 B3 R2 G2 B2 R1]

  dst0 = _mm_or_si128(dst0, MaskFF000000000000FF);           // dst0 = [FF R1 G1 B1 R0 G0 B0 FF]
  dst1 = _mm_or_si128(dst1, MaskFF000000000000FF);           // dst1 = [FF R3 G3 B3 R2 G2 B2 FF]

  dst0 = _mm_unpacklo_epi8(dst0, xmmz);
  dst1 = _mm_unpacklo_epi8(dst1, xmmz);

  dst0 = _mm_shufflelo_epi16(dst0, _MM_SHUFFLE(0, 3, 2, 1)); // dst0 = [FF R1 G1 B1 FF R0 G0 B0]
  dst1 = _mm_shufflelo_epi16(dst1, _MM_SHUFFLE(0, 3, 2, 1)); // dst1 = [FF R3 G3 B3 FF R2 G2 B2]
}

static FOG_INLINE void pix_fetch_bgr24_2x2W(__m128i& dst0, __m128i& dst1, const uint8_t* srcp)
{
  __m128i xmmz = _mm_setzero_si128();

  pix_load8(dst0, srcp + 0);                                 // dst0 = [G2 R2 B1 G1 R1 B0 G0 R0]
  pix_load8(dst1, srcp + 4);                                 // dst1 = [B3 G3 R3 B2 G2 R2 B1 G1]

  dst0 = _mm_slli_epi64(dst0, 8);                            // dst0 = [R2 B1 G1 R1 B0 G0 R0   ]
  dst1 = _mm_srli_epi64(dst1, 8);                            // dst1 = [   B3 G3 R3 B2 G2 R2 B1]

  dst0 = _mm_or_si128(dst0, MaskFF000000000000FF);           // dst0 = [FF B1 G1 R1 B0 G0 R0 FF]
  dst1 = _mm_or_si128(dst1, MaskFF000000000000FF);           // dst1 = [FF B3 G3 R3 B2 G2 R2 FF]

  dst0 = _mm_unpacklo_epi8(dst0, xmmz);
  dst1 = _mm_unpacklo_epi8(dst1, xmmz);

  dst0 = _mm_shufflelo_epi16(dst0, _MM_SHUFFLE(0, 1, 2, 3)); // dst0 = [FF B1 G1 R1 FF R0 G0 B0]
  dst1 = _mm_shufflelo_epi16(dst1, _MM_SHUFFLE(0, 1, 2, 3)); // dst1 = [FF B3 G3 R3 FF R2 G2 B2]

  dst0 = _mm_shufflehi_epi16(dst0, _MM_SHUFFLE(0, 1, 2, 3)); // dst0 = [FF R1 G1 B1 FF R0 G0 B0]
  dst1 = _mm_shufflehi_epi16(dst1, _MM_SHUFFLE(0, 1, 2, 3)); // dst1 = [FF R3 G3 B3 FF R2 G2 B2]
}

// ============================================================================
// [Fog::Raster - Convert - Argb32 Dest]
// ============================================================================

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
    if (--i) return;
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
    if (--i) return;
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
    if (--i) return;
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
    if (--i) return;
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
    if (--i) return;
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
// [Fog::Raster - Pattern - Texture]
// ============================================================================

static uint8_t* FOG_FASTCALL pattern_texture_fetch_repeat_sse2(
  PatternContext* ctx,
  uint8_t* dst, int x, int y, int w)
{
  uint8_t* dstCur = dst;

  int tw = ctx->texture.w;
  int th = ctx->texture.h;

  x -= ctx->texture.dx;
  y -= ctx->texture.dy;

  if (x < 0) x = (x % tw) + tw;
  if (x >= tw) x %= tw;

  if (y < 0) y = (y % th) + th;
  if (y >= th) y %= th;

  const uint8_t* srcBase = ctx->texture.bits + y * ctx->texture.stride;
  const uint8_t* srcCur;

  int i;

  srcCur = srcBase + mul4(x);

  // Return image buffer if span fits to it (this is very efficient
  // optimization for short spans or large textures)
  i = fog_min(tw - x, w);
  if (w < tw - x)
    return const_cast<uint8_t*>(srcCur);

  // This is equal to C implementation in Raster_C.cpp
  for (;;)
  {
    w -= i;

    if (i >= 32)
    {
      // We can improve texture fetching by more pixels at a time.
      while ((sysuint_t)dstCur & 15)
      {
        ((uint32_t*)dstCur)[0] = ((const uint32_t*)srcCur)[0];
        dstCur += 4;
        srcCur += 4;
        if (--i == 0) goto end;
      }

      while (i >= 8)
      {
        __m128i src0mm;
        __m128i src1mm;
        pix_load16u(src0mm, srcCur +  0);
        pix_load16u(src1mm, srcCur + 16);
        pix_store16a(dstCur +  0, src0mm);
        pix_store16a(dstCur + 16, src1mm);

        dstCur += 32;
        srcCur += 32;
        i -= 8;
      }
      if (i == 0) goto end;
    }

    do {
      ((uint32_t*)dstCur)[0] = ((const uint32_t*)srcCur)[0];
      dstCur += 4;
      srcCur += 4;
    } while (--i);
end:
    if (!w) break;

    i = fog_min(w, tw);
    srcCur = srcBase;
  }

  return dst;
}

static uint8_t* FOG_FASTCALL pattern_texture_fetch_reflect_sse2(
  PatternContext* ctx,
  uint8_t* dst, int x, int y, int w)
{
  FOG_ASSERT(w);

  uint8_t* dstCur = dst;

  int tw = ctx->texture.w;
  int th = ctx->texture.h;

  int tw2 = tw << 1;
  int th2 = th << 1;

  x -= ctx->texture.dx;
  y -= ctx->texture.dy;

  if (x < 0) x = (x % tw2) + tw2;
  if (x >= tw2) x %= tw2;

  if (y < 0) y = (y % th2) + th2;
  if (y >= th2) y %= th2;

  // Modify Y if reflected (if it lies in second section).
  if (y >= th) y = th2 - y - 1;

  const uint8_t* srcBase = ctx->texture.bits + y * ctx->texture.stride;
  const uint8_t* srcCur;

  if (x >= 0 && x <= tw && w < tw - x)
    return const_cast<uint8_t*>(srcBase + mul4(x));

  do {
    // Reflect mode
    if (x >= tw)
    {
      int i = fog_min(tw2 - x, w);

      srcCur = srcBase + mul4(tw2 - x - 1);

      w -= i;
      x = 0;

      if (i >= 32)
      {
        // We can improve texture fetching by more pixels at a time.
        while ((sysuint_t)dstCur & 15)
        {
          ((uint32_t*)dstCur)[0] = ((const uint32_t*)srcCur)[0];
          dstCur += 4;
          srcCur -= 4;
          if (--i == 0) goto end;
        }

        while (i >= 8)
        {
          __m128i src0mm;
          __m128i src1mm;

          srcCur -= 32;

          pix_load16u(src0mm, srcCur + 20);
          pix_load16u(src1mm, srcCur + 4);
          src0mm = _mm_shuffle_epi32(src0mm, _MM_SHUFFLE(0, 1, 2, 3));
          src1mm = _mm_shuffle_epi32(src1mm, _MM_SHUFFLE(0, 1, 2, 3));
          pix_store16a(dstCur +  0, src0mm);
          pix_store16a(dstCur + 16, src1mm);

          dstCur += 32;
          i -= 8;
        }
        if (i == 0) goto end;
      }

      do {
        ((uint32_t*)dstCur)[0] = ((const uint32_t*)srcCur)[0];
        dstCur += 4;
        srcCur -= 4;
      } while (--i);
    }
    // Repeat mode
    else
    {
      int i = fog_min(tw - x, w);

      srcCur = srcBase + mul4(x);

      w -= i;
      x += i;

      if (i >= 32)
      {
        // We can improve texture fetching by more pixels at a time.
        while ((sysuint_t)dstCur & 15)
        {
          ((uint32_t*)dstCur)[0] = ((const uint32_t*)srcCur)[0];
          dstCur += 4;
          srcCur += 4;
          if (--i == 0) goto end;
        }

        while (i >= 8)
        {
          __m128i src0mm;
          __m128i src1mm;
          pix_load16u(src0mm, srcCur +  0);
          pix_load16u(src1mm, srcCur + 16);
          pix_store16a(dstCur +  0, src0mm);
          pix_store16a(dstCur + 16, src1mm);

          dstCur += 32;
          srcCur += 32;
          i -= 8;
        }
        if (i == 0) goto end;
      }

      do {
        ((uint32_t*)dstCur)[0] = ((const uint32_t*)srcCur)[0];
        dstCur += 4;
        srcCur += 4;
      } while (--i);
    }
end:
    ;
  } while (w);

  return dst;
}

// ============================================================================
// [Fog::Raster - Pattern - Gradient - Radial]
// ============================================================================

static uint8_t* FOG_FASTCALL pattern_radial_gradient_fetch_pad_sse2(
  PatternContext* ctx,
  uint8_t* dst, int x, int y, int w)
{
  FOG_ASSERT(w);

  uint8_t* dstCur = dst;

  const uint32_t* colors = (const uint32_t*)ctx->radialGradient.colors;
  sysint_t colorsLength = ctx->radialGradient.colorsLength;

  // Here we use some tricks to do computation faster. First trick is that we
  // can add incrementing values [3, 2, 1, 0] to dx. We need this, because we 
  // are computing 4 pixels at time and we need to set correct offset for each.
  __m128 dx = _mm_add_ps(_mm_set1_ps((float)x - (float)ctx->radialGradient.dx), _mm_set_ps(3.0f, 2.0f, 1.0f, 0.0f));
  __m128 dy = _mm_set1_ps((float)y - (float)ctx->radialGradient.dy);

  __m128 fx = _mm_set1_ps((float)ctx->radialGradient.fx);
  __m128 fy = _mm_set1_ps((float)ctx->radialGradient.fy);
  __m128 r2 = _mm_set1_ps((float)ctx->radialGradient.r2);

  __m128 dxdx = _mm_mul_ps(dx, dx);

  // cc = (dy*dy + dx*dx) * r2 - ((dx*fy - dy*fx) * (dx*fy - dy*fx))
  // cx = (dx*r2)              - ((dx*fy - dy*fx) * fy)
  // ci = r2                   - (fy*fy)
  __m128 cc = _mm_add_ps(_mm_mul_ps(dy, dy), dxdx);
  cc = _mm_mul_ps(cc, r2);
  __m128 cx = _mm_mul_ps(dx, r2);
  __m128 ci = _mm_sub_ps(r2, _mm_mul_ps(fy, fy));

  // pp = dx * fy - dy * fx
  __m128 pp = _mm_sub_ps(_mm_mul_ps(dx, fy), _mm_mul_ps(dy, fx));

  cx = _mm_sub_ps(cx, _mm_mul_ps(pp, fy));
  cc = _mm_sub_ps(cc, _mm_mul_ps(pp, pp));

  // dd = (dx*fx + dy*fy)
  // di = fx
  __m128 dd = _mm_add_ps(_mm_mul_ps(dx, fx), _mm_mul_ps(dy, fy));
  __m128 di = _mm_mul_ps(fx, _mm_set1_ps(4.0));

  // ci4 = ci * 4
  // ci16 = ci * 16
  __m128 ci4 = _mm_mul_ps(ci, _mm_set1_ps(4.0));
  __m128 ci16 = _mm_mul_ps(ci, _mm_set1_ps(16.0));

  __m128 eight = _mm_set1_ps(8.0);

  __m128 scale         = _mm_set1_ps((float)ctx->radialGradient.mul);
  __m128 zero          = _mm_set1_ps((float)0.0f);
  __m128 threshold     = _mm_set1_ps((float)(colorsLength-1));

  // Here we are using trict to get another performance gain. The square root
  // calculation is not cheap, so we want situation where FPU calculates square
  // root for next 4 pixels while we are processing current ones.
  __m128 idxf;

  idxf = _mm_and_ps(cc, Mask7FFFFFFF7FFFFFFF); // abs()
  idxf = _mm_sqrt_ps(idxf);

  for (;;)
  {
    __m128i idxi;
    uint index;

    idxf = _mm_add_ps(idxf, dd);
    idxf = _mm_mul_ps(idxf, scale);
    idxf = _mm_max_ps(idxf, zero);
    idxf = _mm_min_ps(idxf, threshold);

    idxi = _mm_cvtps_epi32(idxf);

    // Next 4 pixels

    // cc += (cx + cx + ci) + 
    //       (cx+ci + cx+ci + ci) + 
    //       (cx+ci+ci + cx+ci+ci + ci) + 
    //       (cx+ci+ci+ci + cx+ci+ci+ci + ci)
    // cc += cx*8 + ci*16;
    // cx += ci*4
    cc = _mm_add_ps(cc, _mm_mul_ps(cx, eight));
    cc = _mm_add_ps(cc, ci16);
    cx = _mm_add_ps(cx, ci4);

    dd = _mm_add_ps(dd, di);

    // This step is important, _mm_sqrt_ps() is last called and from now
    // this register will not be used.
    idxf = _mm_and_ps(cc, Mask7FFFFFFF7FFFFFFF); // abs()
    idxf = _mm_sqrt_ps(idxf);

    // Pixel #0
    index = (uint)_mm_cvtsi128_si32(idxi);
    idxi = _mm_shuffle_epi32(idxi, _MM_SHUFFLE(0, 3, 2, 1));

    ((uint32_t*)dstCur)[0] = colors[index];
    if (--w == 0) break;

    // Pixel #1
    index = (uint)_mm_cvtsi128_si32(idxi);
    idxi = _mm_shuffle_epi32(idxi, _MM_SHUFFLE(0, 3, 2, 1));

    ((uint32_t*)dstCur)[1] = colors[index];
    if (--w == 0) break;

    // Pixel #2
    index = (uint)_mm_cvtsi128_si32(idxi);
    idxi = _mm_shuffle_epi32(idxi, _MM_SHUFFLE(0, 3, 2, 1));

    ((uint32_t*)dstCur)[2] = colors[index];
    if (--w == 0) break;

    // Pixel #3
    index = (uint)_mm_cvtsi128_si32(idxi);

    ((uint32_t*)dstCur)[3] = colors[index];
    if (--w == 0) break;

    dstCur += 16;
  }

  return dst;
}

// ============================================================================
// [Fog::Raster - Raster - Prgb32 - SrcOver]
// ============================================================================

static void FOG_FASTCALL raster_prgb32_pixel_srcover_sse2(
  uint8_t* dst, uint32_t src)
{
  uint32_t a = src >> 24;

  if (a != 0xFF)
  {
    __m128i src0mm;
    __m128i dst0mm;

    pix_unpack_1x1W(src0mm, src);
    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_over_1x1W(dst0mm, src0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);
  }
  else
  {
    ((uint32_t*)dst)[0] = src;
  }
}

static void FOG_FASTCALL raster_prgb32_pixel_a8_srcover_sse2(
  uint8_t* dst, uint32_t src, uint32_t msk)
{
  uint32_t a = src >> 24;

  if ((a != 0xFF) | (msk != 0xFF))
  {
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_unpack_1x1W(src0mm, src);
    pix_unpack_1x1W(a0mm, msk);
    pix_expand_alpha_rev_1x1W(a0mm, a0mm);
    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_multiply_1x1W(src0mm, src0mm, a0mm);
    pix_over_1x1W(dst0mm, src0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);
  }
  else
  {
    ((uint32_t*)dst)[0] = src;
  }
}

static void FOG_FASTCALL raster_prgb32_span_solid_srcover_sse2(
  uint8_t* dst, uint32_t src, sysint_t w)
{
  __m128i src0mm = _mm_cvtsi32_si128(src);
  pix_expand_pixel_1x4B(src0mm, src0mm);

  BLIT_SSE2_INIT(dst, w)

  if ((src >> 24) == 0xFF)
  {
    BLIT_SSE2_SMALL_BEGIN(blt_opaque)
      ((uint32_t*)dst)[0] = src;
      dst += 4;
    BLIT_SSE2_SMALL_END(blt_opaque)

    while (w >= 4)
    {
      _mm_store_si128((__m128i*)(dst), src0mm);
      _mm_store_si128((__m128i*)(dst + 16), src0mm);
      _mm_store_si128((__m128i*)(dst + 32), src0mm);
      _mm_store_si128((__m128i*)(dst + 48), src0mm);

      dst += 64;
      w -= 4;
    }
    switch (w & 3)
    {
      case 3: _mm_store_si128((__m128i*)(dst), src0mm); dst += 16;
      case 2: _mm_store_si128((__m128i*)(dst), src0mm); dst += 16;
      case 1: _mm_store_si128((__m128i*)(dst), src0mm); dst += 16;
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

    BLIT_SSE2_SMALL_BEGIN(blt_trans)
      __m128i dst0mm;

      pix_load4(dst0mm, dst);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_over_ialpha_1x1W(dst0mm, src0mm, ia0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);

      dst += 4;
    BLIT_SSE2_SMALL_END(blt_trans)

    BLIT_SSE2_LARGE_BEGIN(blt_trans)
      __m128i dst0mm;
      __m128i dst1mm;

      pix_load16a(dst0mm, dst);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_over_ialpha_2x2W(dst0mm, src0mm, ia0mm, dst1mm, src0mm, ia0mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);

      dst += 16;
    BLIT_SSE2_LARGE_END(blt_trans)
  }
}

static void FOG_FASTCALL raster_prgb32_span_solid_a8_srcover_sse2(
  uint8_t* dst, uint32_t src, const uint8_t* msk, sysint_t w)
{
  __m128i src0orig = _mm_cvtsi32_si128(src);
  __m128i src0unpacked;
  pix_expand_pixel_1x4B(src0orig, src0orig);
  pix_unpack_1x2W(src0unpacked, src0orig);

  BLIT_SSE2_INIT(dst, w);

  if ((src >> 24) == 0xFF)
  {
    BLIT_SSE2_SMALL_BEGIN(blt_opaque)
      __m128i dst0mm;
      __m128i src0mm;
      __m128i a0mm;

      uint32_t msk0;
      if ((msk0 = READ_8(msk)) == 0xFF)
      {
        ((uint32_t*)dst)[0] = src;
      }
      else if (msk0)
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

      dst += 4;
      msk += 1;
    BLIT_SSE2_SMALL_END(blt_opaque)

    BLIT_SSE2_LARGE_BEGIN(blt_opaque)
      __m128i dst0mm, dst1mm;
      __m128i src0mm, src1mm;
      __m128i a0mm, a1mm;

      uint32_t msk0;
      if ((msk0 = ((uint32_t*)msk)[0]))
      {
        if (msk0 == 0xFFFFFFFF)
        {
          _mm_store_si128((__m128i*)dst, src0orig);
        }
        else
        {
          pix_load16a(dst0mm, dst);
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
    BLIT_SSE2_LARGE_END(blt_opaque)
  }
  else
  {
    BLIT_SSE2_SMALL_BEGIN(blt_trans)
      __m128i dst0mm;
      __m128i src0mm;
      __m128i a0mm;

      uint32_t msk0;
      if ((msk0 = READ_8(msk)))
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
    BLIT_SSE2_SMALL_END(blt_trans)

    BLIT_SSE2_LARGE_BEGIN(blt_trans)
      __m128i dst0mm, dst1mm;
      __m128i src0mm, src1mm;
      __m128i a0mm, a1mm;

      uint32_t msk0;
      if ((msk0 = ((uint32_t*)msk)[0]))
      {
        pix_load16a(dst0mm, dst);
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
    BLIT_SSE2_LARGE_END(blt_trans)
  }
}

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_srcover_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w);

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;
    __m128i a0mm;

    uint32_t src0 = READ_32(src);
    if (src0 >> 24)
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
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
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
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_srcover_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
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
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
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
  BLIT_SSE2_LARGE_END(blt)
}

#define raster_prgb32_span_composite_rgb32_srcover_sse2 convert_argb32_from_rgb32_sse2
#define raster_prgb32_span_composite_rgb24_srcover_sse2 convert_rgb32_from_rgb24_sse2

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_a8_srcover_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w);

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t src0 = READ_32(src);
    uint32_t src0a = src0 >> 24;
    uint32_t msk0 = READ_8(msk);

    if ((src0a != 0x00) & (msk0 != 0x00))
    {
      msk0 = alphamul(src0a, msk0);

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
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
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
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_a8_srcover_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
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
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
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
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb32_a8_srcover_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
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
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
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
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb24_a8_srcover_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
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
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
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
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_a8_const_srcover_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk, sysint_t w)
{
  if (FOG_UNLIKELY(msk == 0xFF))
  {
    raster_prgb32_span_composite_argb32_srcover_sse2(dst, src, w);
    return;
  }

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_INIT(dst, w);

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t src0 = READ_32(src);
    uint32_t src0a = src0 >> 24;

    if ((src0a != 0x00))
    {
      if ((src0a == 0xFF))
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
        uint32_t msk0 = alphamul(src0a, msk);

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
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
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
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_a8_const_srcover_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk, sysint_t w)
{
  if (FOG_UNLIKELY(msk == 0xFF))
  {
    raster_prgb32_span_composite_argb32_srcover_sse2(dst, src, w);
    return;
  }

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_INIT(dst, w);

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t src0 = READ_32(src);
    uint32_t src0a = src0 >> 24;

    if ((src0a != 0x00))
    {
      if ((src0a == 0xFF))
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
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
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
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb32_a8_const_srcover_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk, sysint_t w)
{
  if (FOG_UNLIKELY(msk == 0xFF))
  {
    raster_prgb32_span_composite_rgb32_srcover_sse2(dst, src, w);
    return;
  }

  __m128i m0mm, im0mm;
  __m128i amask = MaskFF000000FF000000;
  pix_expand_mask_1x1W(m0mm, msk);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_INIT(dst, w);

  BLIT_SSE2_SMALL_BEGIN(blt)
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
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
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
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb24_a8_const_srcover_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk, sysint_t w)
{
  if (FOG_UNLIKELY(msk == 0xFF))
  {
    convert_rgb32_from_rgb24_sse2(dst, src, w);
    return;
  }

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_INIT(dst, w);

  BLIT_SSE2_SMALL_BEGIN(blt)
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
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
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
  BLIT_SSE2_LARGE_END(blt)
}

// ============================================================================
// [Fog::Raster - Raster - Prgb32 - DstOver]
// ============================================================================

static void FOG_FASTCALL raster_prgb32_pixel_dstover_sse2(
  uint8_t* dst, uint32_t src)
{
  uint32_t dst0 = ((uint32_t*)dst)[0];
  uint32_t dst0a = dst0 >> 24;
  uint32_t src0a = src >> 24;

  if (dst0a != 0xFF && src0a != 0x00)
  {
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_unpack_1x1W(src0mm, src);
    pix_unpack_1x1W(dst0mm, dst0);
    pix_expand_alpha_1x1W(a0mm, dst0mm);
    pix_overrev_1x1W(dst0mm, src0mm, a0mm);
    pix_pack_1x1W(dst0mm, dst0mm);

    pix_store4(dst, dst0mm);
  }
}

static void FOG_FASTCALL raster_prgb32_pixel_a8_dstover_sse2(
  uint8_t* dst, uint32_t src, uint32_t msk)
{
  uint32_t dst0 = ((uint32_t*)dst)[0];
  uint32_t dst0a = dst0 >> 24;
  uint32_t src0a = src >> 24;

  if (dst0a != 0xFF && src0a != 0x00 && msk != 0x00)
  {
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_unpack_1x1W(src0mm, src);
    pix_expand_mask_1x1W(a0mm, msk);
    pix_unpack_1x1W(dst0mm, dst0);
    pix_multiply_1x1W(src0mm, src0mm, a0mm);
    pix_expand_alpha_1x1W(a0mm, dst0mm);
    pix_overrev_1x1W(dst0mm, src0mm, a0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);
  }
}

static void FOG_FASTCALL raster_prgb32_span_solid_dstover_sse2(
  uint8_t* dst, uint32_t src, sysint_t w)
{
  uint32_t a = src >> 24;

  __m128i src0orig = _mm_cvtsi32_si128(src);
  __m128i src0mm;

  pix_expand_pixel_1x4B(src0orig, src0orig);
  pix_unpack_1x2W(src0mm, src0orig);

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t dst0 = READ_32(dst);
    uint32_t dst0a = dst0 >> 24;

    if (dst0a != 0xFF)
    {
      if (dst0a == 0x00)
      {
        ((uint32_t*)dst)[0] = src;
      }
      else
      {
        pix_unpack_1x1W(dst0mm, dst0);
        pix_expand_alpha_1x1W(a0mm, dst0mm);
        pix_overrev_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);

    if (dst0a != 0xFFFFFFFF)
    {
      if (dst0a == 0x00000000)
      {
        _mm_store_si128((__m128i*)dst, src0orig);
      }
      else
      {
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
        pix_overrev_2x2W(dst0mm, src0mm, a0mm, dst1mm, src0mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_solid_a8_dstover_sse2(
  uint8_t* dst, uint32_t src, const uint8_t* msk, sysint_t w)
{
  uint32_t a = src >> 24;

  __m128i src0orig = _mm_cvtsi32_si128(src);
  __m128i src0unpacked;

  pix_expand_pixel_1x2W(src0orig, src0orig);
  pix_unpack_1x2W(src0unpacked, src0orig);

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t dst0 = ((uint32_t*)dst)[0];
    uint32_t dst0a = dst0 >> 24;
    uint32_t msk0;

    if (dst0a != 0xFF && (msk0 = READ_8(msk)))
    {
      pix_unpack_1x1W(dst0mm, dst0);
      pix_expand_mask_1x1W(a0mm, msk0);
      pix_expand_pixel_1x1W(a0mm, a0mm);
      pix_multiply_1x1W(src0mm, src0unpacked, a0mm);
      pix_expand_alpha_1x1W(a0mm, dst0mm);
      pix_overrev_1x1W(dst0mm, src0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    uint32_t msk0;

    if (dst0a != 0xFFFFFFFF && (msk0 = ((uint32_t*)msk)[0]))
    {
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);

      pix_unpack_1x1W(a0mm, _mm_cvtsi32_si128(msk0));
      a0mm = _mm_unpacklo_epi16(a0mm, a0mm);

      a1mm = _mm_shuffle_epi32(a0mm, _MM_SHUFFLE(3, 3, 2, 2));
      a0mm = _mm_shuffle_epi32(a0mm, _MM_SHUFFLE(1, 1, 0, 0));

      pix_multiply_2x2W(src0mm, src0unpacked, a0mm, src1mm, src0unpacked, a1mm);
      pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
      pix_overrev_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);

      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_dstover_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t dst0 = READ_32(dst);
    uint32_t dst0a = dst0 >> 24;

    if (dst0a != 0xFF)
    {
      pix_load4(src0mm, src);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_premultiply_1x1W(src0mm, src0mm);

      if (dst0a == 0x00)
      {
        pix_pack_1x1W(src0mm, src0mm);
        pix_store4(dst, src0mm);
      }
      else
      {
        pix_unpack_1x1W(dst0mm, dst0);
        pix_unpack_1x1W(src0mm, src0mm);
        pix_expand_alpha_1x1W(a0mm, dst0mm);
        pix_overrev_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);

    if (dst0a != 0xFFFFFFFF)
    {
      pix_load16u(src0mm, src);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);

      if (dst0a == 0x00000000)
      {
        pix_pack_2x2W(src0mm, src0mm, src1mm);
        pix_store16a(dst, src0mm);
      }
      else
      {
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
        pix_overrev_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_dstover_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t dst0 = READ_32(dst);
    uint32_t dst0a = dst0 >> 24;

    if (dst0a != 0xFF)
    {
      uint32_t src0 = READ_32(src);

      if (dst0a == 0x00)
      {
        ((uint32_t*)dst)[0] = src0;
      }
      else
      {
        pix_unpack_1x1W(dst0mm, dst0);
        pix_unpack_1x1W(src0mm, src0);
        pix_expand_alpha_1x1W(a0mm, dst0mm);
        pix_overrev_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);

    if (dst0a != 0xFFFFFFFF)
    {
      pix_load16u(src0mm, src);

      if (dst0a == 0x00000000)
      {
        pix_store16a(dst, src0mm);
      }
      else
      {
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
        pix_overrev_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb32_dstover_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  __m128i amask = MaskFF000000FF000000;

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t dst0 = READ_32(dst);
    uint32_t dst0a = dst0 >> 24;

    if (dst0a != 0xFF)
    {
      uint32_t src0 = READ_32(src) | 0xFF000000;
      if (dst0a == 0x00)
      {
        ((uint32_t*)dst)[0] = src0;
      }
      else
      {
        src0mm = _mm_cvtsi32_si128(src0);
        pix_unpack_1x1W(dst0mm, dst0);
        pix_unpack_1x1W(src0mm, src0mm);
        pix_expand_alpha_1x1W(a0mm, dst0mm);
        pix_overrev_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);

    if (dst0a != 0xFFFFFFFF)
    {
      pix_load16u(src0mm, src);
      src0mm = _mm_or_si128(src0mm, amask);

      if (dst0a == 0x00000000)
      {
        pix_store16a(dst, src0mm);
      }
      else
      {
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
        pix_overrev_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb24_dstover_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  __m128i amask = MaskFF000000FF000000;

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t dst0 = READ_32(dst);
    uint32_t dst0a = dst0 >> 24;

    if (dst0a != 0xFF)
    {
      uint32_t src0 = PixFmt_RGB24::fetch(src) | 0xFF000000;
      if (dst0a == 0x00)
      {
        ((uint32_t*)dst)[0] = src0;
      }
      else
      {
        src0mm = _mm_cvtsi32_si128(src0);
        pix_unpack_1x1W(dst0mm, dst0);
        pix_unpack_1x1W(src0mm, src0mm);
        pix_expand_alpha_1x1W(a0mm, dst0mm);
        pix_overrev_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    src += 3;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);

    if (dst0a != 0xFFFFFFFF)
    {
      pix_fetch_rgb24_2x2W(src0mm, src1mm, src);

      if (dst0a == 0x00000000)
      {
        pix_pack_2x2W(src0mm, src0mm, src1mm);
        pix_store16a(dst, src0mm);
      }
      else
      {
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
        pix_overrev_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 12;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_a8_dstover_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t dst0 = READ_32(dst);
    uint32_t dst0a = dst0 >> 24;
    uint32_t msk0 = READ_8(msk);

    if ((dst0a != 0xFF) & (msk0 != 0))
    {
      msk0 = alphamul(dst0a, msk0);
      pix_load4(src0mm, src);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_expand_mask_1x1W(a0mm, msk0);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);

      if (dst0a == 0x00)
      {
        pix_pack_1x1W(src0mm, src0mm);
        pix_store4(dst, src0mm);
      }
      else
      {
        pix_unpack_1x1W(dst0mm, dst0);
        pix_expand_alpha_1x1W(a0mm, dst0mm);
        pix_overrev_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    uint32_t msk0 = READ_32(msk);

    if ((dst0a != 0xFFFFFFFF) & (msk0 != 0x00000000))
    {
      pix_load16u(src0mm, src);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_expand_mask_2x2W(a0mm, a0mm, msk0);
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);

      if (dst0a == 0x00000000)
      {
        pix_pack_2x2W(src0mm, src0mm, src1mm);
        pix_store16a(dst, src0mm);
      }
      else
      {
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
        pix_overrev_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_a8_dstover_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t dst0 = READ_32(dst);
    uint32_t dst0a = dst0 >> 24;
    uint32_t msk0 = READ_8(msk);

    if ((dst0a != 0xFF) & (msk0 != 0x00))
    {
      uint32_t src0 = READ_32(src);

      if ((dst0a == 0x00) & (msk0 == 0xFF))
      {
        ((uint32_t*)dst)[0] = src0;
      }
      else
      {
        pix_unpack_1x1W(dst0mm, dst0);
        pix_unpack_1x1W(src0mm, src0);
        pix_expand_mask_1x1W(a0mm, msk0);
        pix_multiply_1x1W(src0mm, src0mm, a0mm);
        pix_expand_alpha_1x1W(a0mm, dst0mm);
        pix_overrev_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    uint32_t msk0 = READ_32(msk);

    if ((dst0a != 0xFFFFFFFF) & (msk0 != 0x00000000))
    {
      pix_load16u(src0mm, src);

      if ((dst0a == 0x00000000) & (msk0 == 0xFFFFFFFF))
      {
        pix_store16a(dst, src0mm);
      }
      else
      {
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
        pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
        pix_overrev_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb32_a8_dstover_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  __m128i amask = MaskFF000000FF000000;

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t dst0 = READ_32(dst);
    uint32_t dst0a = dst0 >> 24;
    uint32_t msk0 = READ_8(msk);

    if ((dst0a != 0xFF) & (msk0 != 0x00))
    {
      uint32_t src0 = READ_32(src) | 0xFF000000;
      if ((dst0a == 0x00) & (msk0 == 0xFF))
      {
        ((uint32_t*)dst)[0] = src0;
      }
      else
      {
        pix_unpack_1x1W(dst0mm, dst0);
        pix_unpack_1x1W(src0mm, src0);
        pix_expand_mask_1x1W(a0mm, msk0);
        pix_multiply_1x1W(src0mm, src0mm, a0mm);
        pix_expand_alpha_1x1W(a0mm, dst0mm);
        pix_overrev_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    uint32_t msk0 = READ_32(msk);

    if ((dst0a != 0xFFFFFFFF) & (msk0 != 0x00000000))
    {
      pix_load16u(src0mm, src);
      src0mm = _mm_or_si128(src0mm, amask);

      if ((dst0a == 0x00000000) & (msk0 == 0xFFFFFFFF))
      {
        pix_store16a(dst, src0mm);
      }
      else
      {
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
        pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
        pix_overrev_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb24_a8_dstover_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  __m128i amask = MaskFF000000FF000000;

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t dst0 = READ_32(dst);
    uint32_t dst0a = dst0 >> 24;
    uint32_t msk0 = READ_8(msk);

    if ((dst0a != 0xFF) & (msk0 != 0x00))
    {
      uint32_t src0 = PixFmt_RGB24::fetch(src) | 0xFF000000;
      if ((dst0a == 0x00) & (msk0 == 0xFF))
      {
        ((uint32_t*)dst)[0] = src0;
      }
      else
      {
        src0mm = _mm_cvtsi32_si128(src0);
        pix_unpack_1x1W(dst0mm, dst0);
        pix_unpack_1x1W(src0mm, src0mm);
        pix_expand_mask_1x1W(a0mm, msk0);
        pix_multiply_1x1W(src0mm, src0mm, a0mm);
        pix_expand_alpha_1x1W(a0mm, dst0mm);
        pix_overrev_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }
    }

    dst += 4;
    src += 3;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    uint32_t msk0 = READ_32(msk);

    if ((dst0a != 0xFFFFFFFF) & (msk0 != 0x00000000))
    {
      pix_fetch_rgb24_2x2W(src0mm, src1mm, src);

      if ((dst0a == 0x00000000) & (msk0 == 0xFFFFFFFF))
      {
        pix_pack_2x2W(src0mm, src0mm, src1mm);
        pix_store16a(dst, src0mm);
      }
      else
      {
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
        pix_expand_alpha_2x2W(a0mm, dst0mm, a1mm, dst1mm);
        pix_overrev_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 12;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

// ============================================================================
// [Fog::Raster - Raster - Prgb32 - SrcIn]
// ============================================================================

static void FOG_FASTCALL raster_prgb32_pixel_srcin_sse2(
  uint8_t* dst, uint32_t src)
{
  uint32_t dst0 = ((uint32_t*)dst)[0];
  uint32_t dst0a = dst0 >> 24;

  if (dst0a != 0x00)
  {
    __m128i src0mm;
    __m128i a0mm;

    pix_unpack_1x1W(src0mm, src);
    pix_expand_mask_1x1W(a0mm, dst0a);
    pix_multiply_1x1W(src0mm, src0mm, a0mm);
    pix_pack_1x1W(src0mm, src0mm);
    pix_store4(dst, src0mm);
  }
}

static void FOG_FASTCALL raster_prgb32_pixel_a8_srcin_sse2(
  uint8_t* dst, uint32_t src, uint32_t msk)
{
  uint32_t dst0 = ((uint32_t*)dst)[0];
  uint32_t dst0a = dst0 >> 24;

  if (dst0a != 0x00)
  {
    __m128i src0mm;
    __m128i dst0mm;
    __m128i m0mm;

    pix_unpack_1x1W(src0mm, src);
    pix_expand_mask_1x1W(dst0mm, dst0a);
    pix_expand_mask_1x1W(m0mm, msk);
    pix_multiply_1x1W(dst0mm, dst0mm, m0mm);
    pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);
  }
}

static void FOG_FASTCALL raster_prgb32_span_solid_srcin_sse2(
  uint8_t* dst, uint32_t src, sysint_t w)
{
  __m128i src0mm = _mm_cvtsi32_si128(src);
  pix_expand_pixel_1x4B(src0mm, src0mm);
  pix_unpack_1x2W(src0mm, src0mm);

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;
    uint32_t dst0 = READ_32(dst);

    if (dst0)
    {
      pix_unpack_1x1W(dst0mm, dst0);
      pix_expand_alpha_1x1W(dst0mm, dst0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0x00000000)
    {
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_expand_alpha_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src0mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_solid_a8_srcin_sse2(
  uint8_t* dst, uint32_t src, const uint8_t* msk, sysint_t w)
{
  __m128i src0mm = _mm_cvtsi32_si128(src);
  pix_expand_pixel_1x4B(src0mm, src0mm);
  pix_unpack_1x2W(src0mm, src0mm);

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t dst0 = READ_32(dst);

    if (dst0)
    {
      pix_unpack_1x1W(dst0mm, dst0);
      pix_expand_alpha_1x1W(dst0mm, dst0mm);
      pix_expand_mask_1x1W(a0mm, READ_8(msk));
      pix_multiply_1x1W(dst0mm, dst0mm, a0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0x00000000)
    {
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_expand_alpha_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_expand_mask_2x2W(a0mm, a1mm, READ_32(msk));
      pix_multiply_2x2W(dst0mm, dst0mm, a0mm, dst1mm, dst1mm, a1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src0mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_srcin_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    uint32_t dst0 = READ_32(dst);

    if (dst0)
    {
      pix_load4(src0mm, src);
      pix_unpack_1x1W(dst0mm, dst0);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_expand_alpha_1x1W(dst0mm, dst0mm);
      pix_premultiply_1x1W(src0mm, src0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0x00000000)
    {
      pix_load16u(src0mm, src);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_expand_alpha_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_srcin_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    uint32_t dst0 = READ_32(dst);

    if (dst0)
    {
      pix_load4(src0mm, src);
      pix_unpack_1x1W(dst0mm, dst0);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_expand_alpha_1x1W(dst0mm, dst0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0x00000000)
    {
      pix_load16u(src0mm, src);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_expand_alpha_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb32_srcin_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  __m128i amask = MaskFF000000FF000000;

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    uint32_t dst0 = READ_32(dst);

    if (dst0)
    {
      pix_load4(src0mm, src);
      src0mm = _mm_or_si128(src0mm, amask);
      pix_unpack_1x1W(dst0mm, dst0);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_expand_alpha_1x1W(dst0mm, dst0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0x00000000)
    {
      pix_load16u(src0mm, src);
      src0mm = _mm_or_si128(src0mm, amask);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_expand_alpha_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb24_srcin_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    uint32_t dst0 = READ_32(dst);

    if (dst0)
    {
      pix_unpack_1x1W(dst0mm, dst0);
      pix_unpack_1x1W(src0mm, PixFmt_RGB24::fetch(src) | 0xFF000000);
      pix_expand_alpha_1x1W(dst0mm, dst0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 3;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0x00000000)
    {
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_fetch_rgb24_2x2W(src0mm, src1mm, src);
      pix_expand_alpha_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 12;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_a8_srcin_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t dst0 = READ_32(dst);

    if (dst0)
    {
      pix_load4(src0mm, src);
      pix_unpack_1x1W(dst0mm, dst0);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_expand_alpha_1x1W(dst0mm, dst0mm);
      pix_premultiply_1x1W(src0mm, src0mm);
      pix_expand_mask_1x1W(a0mm, READ_8(msk));
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0x00000000)
    {
      pix_load16u(src0mm, src);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_expand_alpha_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
      pix_expand_mask_2x2W(a0mm, a1mm, READ_32(msk));
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_a8_srcin_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t dst0 = READ_32(dst);

    if (dst0)
    {
      pix_load4(src0mm, src);
      pix_unpack_1x1W(dst0mm, dst0);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_expand_alpha_1x1W(dst0mm, dst0mm);
      pix_expand_mask_1x1W(a0mm, READ_8(msk));
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0x00000000)
    {
      pix_load16u(src0mm, src);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_expand_alpha_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_expand_mask_2x2W(a0mm, a1mm, READ_32(msk));
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb32_a8_srcin_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  __m128i amask = MaskFF000000FF000000;

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t dst0 = READ_32(dst);

    if (dst0)
    {
      pix_load4(src0mm, src);
      src0mm = _mm_or_si128(src0mm, amask);
      pix_unpack_1x1W(dst0mm, dst0);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_expand_alpha_1x1W(dst0mm, dst0mm);
      pix_expand_mask_1x1W(a0mm, READ_8(msk));
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0x00000000)
    {
      pix_load16u(src0mm, src);
      src0mm = _mm_or_si128(src0mm, amask);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_expand_alpha_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_expand_mask_2x2W(a0mm, a1mm, READ_32(msk));
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb24_a8_srcin_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t dst0 = READ_32(dst);

    if (dst0)
    {
      pix_unpack_1x1W(dst0mm, dst0);
      pix_unpack_1x1W(src0mm, PixFmt_RGB24::fetch(src) | 0xFF000000);
      pix_expand_alpha_1x1W(dst0mm, dst0mm);
      pix_expand_mask_1x1W(a0mm, READ_8(msk));
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 3;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0x00000000)
    {
      pix_fetch_rgb24_2x2W(src0mm, src1mm, src);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_expand_alpha_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_expand_mask_2x2W(a0mm, a1mm, READ_32(msk));
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 12;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

// ============================================================================
// [Fog::Raster - Raster - Prgb32 - DstIn]
// ============================================================================

static void FOG_FASTCALL raster_prgb32_pixel_dstin_sse2(
  uint8_t* dst, uint32_t src)
{
  uint32_t dst0 = ((uint32_t*)dst)[0];

  if (dst0 != 0x00000000)
  {
    __m128i dst0mm;
    __m128i src0mm;

    pix_unpack_1x1W(src0mm, src);
    pix_unpack_1x1W(dst0mm, dst0);
    pix_expand_alpha_1x1W(src0mm, src0mm);
    pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);
  }
}

static void FOG_FASTCALL raster_prgb32_pixel_a8_dstin_sse2(
  uint8_t* dst, uint32_t src, uint32_t msk)
{
  uint32_t dst0 = ((uint32_t*)dst)[0];

  if (dst0 != 0x00000000)
  {
    __m128i dst0mm;
    __m128i src0mm;
    __m128i m0mm;

    pix_unpack_1x1W(src0mm, src);
    pix_unpack_1x1W(dst0mm, dst0);
    pix_expand_alpha_1x1W(src0mm, src0mm);
    pix_expand_mask_1x1W(m0mm, msk);
    pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
    pix_multiply_1x1W(dst0mm, dst0mm, m0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);
  }
}

static void FOG_FASTCALL raster_prgb32_span_solid_dstin_sse2(
  uint8_t* dst, uint32_t src, sysint_t w)
{
  __m128i src0mm = _mm_cvtsi32_si128(src);
  pix_expand_pixel_1x4B(src0mm, src0mm);
  pix_unpack_1x2W(src0mm, src0mm);
  pix_expand_alpha_1x2W(src0mm, src0mm);

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;

    uint32_t dst0 = ((uint32_t*)dst)[0];
    if (dst0 != 0x00000000)
    {
      pix_unpack_1x1W(dst0mm, dst0);
      pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0x00000000)
    {
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_multiply_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src0mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_solid_a8_dstin_sse2(
  uint8_t* dst, uint32_t src, const uint8_t* msk, sysint_t w)
{
  __m128i src0mm = _mm_cvtsi32_si128(src);
  pix_expand_pixel_1x4B(src0mm, src0mm);
  pix_unpack_1x2W(src0mm, src0mm);
  pix_expand_alpha_1x2W(src0mm, src0mm);

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t dst0 = ((uint32_t*)dst)[0];

    if (dst0 != 0x00000000)
    {
      pix_unpack_1x1W(dst0mm, dst0);
      pix_expand_mask_1x1W(a0mm, READ_8(msk));
      pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0x00000000)
    {
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_expand_mask_2x2W(a0mm, a1mm, READ_32(msk));
      pix_multiply_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src0mm);
      pix_multiply_2x2W(dst0mm, dst0mm, a0mm, dst1mm, dst1mm, a1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_axxx32_dstin_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    uint32_t dst0 = ((uint32_t*)dst)[0];

    if (dst0 != 0x00000000)
    {
      pix_load4(src0mm, src);
      pix_unpack_1x1W(dst0mm, dst0);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_expand_alpha_1x1W(src0mm, src0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0x00000000)
    {
      pix_load16u(src0mm, src);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_expand_alpha_2x2W(src0mm, src0mm, src1mm, src1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_axxx32_a8_dstin_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t dst0 = ((uint32_t*)dst)[0];

    if (dst0 != 0x00000000)
    {
      pix_load4(src0mm, src);
      pix_unpack_1x1W(dst0mm, dst0);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_unpack_1x1W(a0mm, READ_8(msk));
      pix_expand_alpha_1x1W(src0mm, src0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0x00000000)
    {
      pix_load16u(src0mm, src);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_expand_alpha_2x2W(src0mm, src0mm, src1mm, src1mm);
      pix_expand_mask_2x2W(a0mm, a1mm, READ_32(msk));
      pix_multiply_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, a0mm, dst1mm, dst1mm, a1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_xxxx_a8_dstin_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  FOG_UNUSED(src);

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t dst0 = ((uint32_t*)dst)[0];
    if (dst0 != 0x00000000)
    {
      pix_unpack_1x1W(dst0mm, dst0);
      pix_unpack_1x1W(a0mm, READ_8(msk));
      pix_multiply_1x1W(dst0mm, dst0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0x00000000)
    {
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_expand_mask_2x2W(a0mm, a1mm, READ_32(msk));
      pix_multiply_2x2W(dst0mm, dst0mm, a0mm, dst1mm, dst1mm, a1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

// ============================================================================
// [Fog::Raster - Raster - Prgb32 - SrcOut]
// ============================================================================

static void FOG_FASTCALL raster_prgb32_pixel_srcout_sse2(
  uint8_t* dst, uint32_t src)
{
  uint32_t dst0 = ((uint32_t*)dst)[0];
  uint32_t dst0ia = (~dst0) >> 24;

  if (dst0ia == 0xFF)
  {
    ((uint32_t*)dst)[0] = src;
  }
  else
  {
    __m128i src0mm;
    __m128i a0mm;

    pix_unpack_1x1W(src0mm, src);
    pix_expand_mask_1x1W(a0mm, dst0ia);
    pix_multiply_1x1W(src0mm, src0mm, a0mm);
    pix_pack_1x1W(src0mm, src0mm);
    pix_store4(dst, src0mm);
  }
}

static void FOG_FASTCALL raster_prgb32_pixel_a8_srcout_sse2(
  uint8_t* dst, uint32_t src, uint32_t msk)
{
  uint32_t dst0 = ((uint32_t*)dst)[0];
  uint32_t dst0ia = (~dst0) >> 24;

  if ((dst0ia == 0xFF) & (msk == 0xFF))
  {
    ((uint32_t*)dst)[0] = src;
  }
  else
  {
    __m128i src0mm;
    __m128i a0mm;

    pix_unpack_1x1W(src0mm, src);
    pix_expand_mask_1x1W(a0mm, alphamul(dst0ia, msk));
    pix_multiply_1x1W(src0mm, src0mm, a0mm);
    pix_pack_1x1W(src0mm, src0mm);
    pix_store4(dst, src0mm);
  }
}

static void FOG_FASTCALL raster_prgb32_span_solid_srcout_sse2(
  uint8_t* dst, uint32_t src, sysint_t w)
{
  __m128i src0orig = _mm_cvtsi32_si128(src);
  __m128i src0mm;
  pix_expand_pixel_1x4B(src0orig, src0orig);
  pix_unpack_1x2W(src0mm, src0orig);

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;

    uint32_t dst0 = READ_32(dst);

    if (dst0 == 0x00000000)
    {
      ((uint32_t*)dst)[0] = src;
    }
    else
    {
      pix_unpack_1x1W(dst0mm, dst0);
      pix_expand_alpha_1x1W(dst0mm, dst0mm);
      pix_negate_1x1W(dst0mm, dst0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a == 0x00000000)
    {
      pix_store16a(dst, src0orig);
    }
    else
    {
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_expand_alpha_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_negate_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src0mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_solid_a8_srcout_sse2(
  uint8_t* dst, uint32_t src, const uint8_t* msk, sysint_t w)
{
  __m128i src0orig = _mm_cvtsi32_si128(src);
  __m128i src0mm;
  pix_expand_pixel_1x4B(src0orig, src0orig);
  pix_unpack_1x2W(src0mm, src0orig);

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t dst0 = READ_32(dst);
    uint32_t msk0 = READ_8(msk);

    if ((dst0 == 0x00000000) & (msk0 == 0xFF))
    {
      ((uint32_t*)dst)[0] = src;
    }
    else
    {
      pix_unpack_1x1W(dst0mm, dst0);
      pix_expand_alpha_1x1W(dst0mm, dst0mm);
      pix_negate_1x1W(dst0mm, dst0mm);
      pix_expand_mask_1x1W(a0mm, msk0);
      pix_multiply_1x1W(a0mm, a0mm, src0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    uint32_t msk0 = READ_32(msk);

    if ((dst0a == 0x00000000) & (msk0 == 0xFFFFFFFF))
    {
      pix_store16a(dst, src0orig);
    }
    else
    {
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_expand_alpha_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_negate_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_expand_mask_2x2W(a0mm, a1mm, msk0);
      pix_multiply_2x2W(a0mm, a0mm, src0mm, a1mm, a1mm, src0mm);
      pix_multiply_2x2W(dst0mm, dst0mm, a0mm, dst1mm, dst1mm, a0mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_srcout_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    uint32_t dst0 = READ_32(dst);
    pix_load4(src0mm, src);

    if (dst0 == 0x00000000)
    {
      pix_store4(dst, src0mm);
    }
    else
    {
      pix_load4(src0mm, src);
      pix_unpack_1x1W(dst0mm, dst0);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_expand_alpha_1x1W(dst0mm, dst0mm);
      pix_negate_1x1W(dst0mm, dst0mm);
      pix_premultiply_1x1W(src0mm, src0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a == 0x00000000)
    {
      pix_store16a(dst, src0mm);
    }
    else
    {
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_expand_alpha_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_negate_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_srcout_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    uint32_t dst0 = READ_32(dst);
    pix_load4(src0mm, src);

    if (dst0 == 0x00000000)
    {
      pix_store4(dst, src0mm);
    }
    else
    {
      pix_load4(src0mm, src);
      pix_unpack_1x1W(dst0mm, dst0);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_expand_alpha_1x1W(dst0mm, dst0mm);
      pix_negate_1x1W(dst0mm, dst0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a == 0x00000000)
    {
      pix_store16a(dst, src0mm);
    }
    else
    {
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_expand_alpha_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_negate_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb32_srcout_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  __m128i amask = MaskFF000000FF000000;

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    uint32_t dst0 = READ_32(dst);

    pix_load4(src0mm, src);
    src0mm = _mm_or_si128(src0mm, amask);

    if (dst0 == 0x00000000)
    {
      pix_store4(dst, src0mm);
    }
    else
    {
      pix_unpack_1x1W(dst0mm, dst0);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_expand_alpha_1x1W(dst0mm, dst0mm);
      pix_negate_1x1W(dst0mm, dst0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);
    src0mm = _mm_or_si128(src0mm, amask);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);

    if (dst0a == 0x00000000)
    {
      pix_store16a(dst, src0mm);
    }
    else
    {
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_expand_alpha_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_negate_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb24_srcout_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    uint32_t dst0 = READ_32(dst);
    uint32_t src0 = PixFmt_RGB24::fetch(src) | 0xFF000000;

    if (dst0 == 0x00000000)
    {
      ((uint32_t*)dst)[0] = src0;
    }
    else
    {
      pix_unpack_1x1W(dst0mm, dst0);
      pix_unpack_1x1W(src0mm, src0);
      pix_expand_alpha_1x1W(dst0mm, dst0mm);
      pix_negate_1x1W(dst0mm, dst0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 3;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);
    pix_fetch_rgb24_2x2W(src0mm, src1mm, src);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);

    if (dst0a == 0x00000000)
    {
      pix_pack_2x2W(src0mm, src0mm, src1mm);
      pix_store16a(dst, src0mm);
    }
    else
    {
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_expand_alpha_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_negate_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 12;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_a8_srcout_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_load4(src0mm, src);

    uint32_t dst0 = READ_32(dst);
    uint32_t msk0 = READ_8(msk);

    pix_unpack_1x1W(src0mm, src0mm);
    pix_premultiply_1x1W(src0mm, src0mm);

    if ((dst0 == 0x00000000) & (msk0 == 0xFF))
    {
      pix_pack_1x1W(src0mm, src0mm);
      pix_store4(dst, src0mm);
    }
    else
    {
      pix_unpack_1x1W(dst0mm, dst0);
      pix_expand_alpha_1x1W(dst0mm, dst0mm);
      pix_negate_1x1W(dst0mm, dst0mm);
      pix_expand_mask_1x1W(a0mm, READ_8(msk));
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    uint32_t msk0 = READ_32(msk);

    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);

    if ((dst0a == 0x00000000) & (msk0 == 0xFFFFFFFF))
    {
      pix_pack_2x2W(src0mm, src0mm, src1mm);
      pix_store16a(dst, src0mm);
    }
    else
    {
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_expand_alpha_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_negate_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_expand_mask_2x2W(a0mm, a1mm, msk0);
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_a8_srcout_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t dst0 = READ_32(dst);
    uint32_t msk0 = READ_8(msk);
    pix_load4(src0mm, src);

    if ((dst0 == 0x00000000) & (msk0 == 0xFF))
    {
      pix_store4(dst, src0mm);
    }
    else
    {
      pix_unpack_1x1W(dst0mm, dst0);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_expand_alpha_1x1W(dst0mm, dst0mm);
      pix_negate_1x1W(dst0mm, dst0mm);
      pix_expand_mask_1x1W(a0mm, READ_8(msk));
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    uint32_t msk0 = READ_32(msk);

    if ((dst0a == 0x00000000) & (msk0 == 0xFFFFFFFF))
    {
      pix_store16a(dst, src0mm);
    }
    else
    {
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_expand_alpha_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_negate_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_expand_mask_2x2W(a0mm, a1mm, msk0);
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb32_a8_srcout_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  __m128i amask = MaskFF000000FF000000;

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t dst0 = READ_32(dst);
    uint32_t msk0 = READ_8(msk);
    pix_load4(src0mm, src);
    src0mm = _mm_or_si128(src0mm, amask);

    if ((dst0 == 0x00000000) & (msk0 == 0xFF))
    {
      pix_store4(dst, src0mm);
    }
    else
    {
      pix_unpack_1x1W(dst0mm, dst0);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_expand_alpha_1x1W(dst0mm, dst0mm);
      pix_negate_1x1W(dst0mm, dst0mm);
      pix_expand_mask_1x1W(a0mm, msk0);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);
    src0mm = _mm_or_si128(src0mm, amask);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    uint32_t msk0 = READ_32(msk);

    if ((dst0a == 0x00000000) & (msk0 == 0xFFFFFFFF))
    {
      pix_store16a(dst, src0mm);
    }
    else
    {
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_expand_alpha_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_negate_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_expand_mask_2x2W(a0mm, a1mm, msk0);
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb24_a8_srcout_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t dst0 = READ_32(dst);
    uint32_t msk0 = READ_8(msk);
    uint32_t src0 = PixFmt_RGB24::fetch(src) | 0xFF000000;

    if ((dst0 == 0x00000000) & (msk0 == 0xFF))
    {
      ((uint32_t*)dst)[0] = src0;
    }
    else
    {
      pix_unpack_1x1W(dst0mm, dst0);
      pix_unpack_1x1W(src0mm, src0);
      pix_expand_alpha_1x1W(dst0mm, dst0mm);
      pix_negate_1x1W(dst0mm, dst0mm);
      pix_expand_mask_1x1W(a0mm, msk0);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 3;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    pix_fetch_rgb24_2x2W(src0mm, src1mm, src);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    uint32_t msk0 = READ_32(msk);


    if ((dst0a == 0x00000000) & (msk0 == 0xFFFFFFFF))
    {
      pix_pack_2x2W(src0mm, src0mm, src1mm);
      pix_store16a(dst, src0mm);
    }
    else
    {
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_expand_alpha_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_negate_2x2W(dst0mm, dst0mm, dst1mm, dst1mm);
      pix_expand_mask_2x2W(a0mm, a1mm, msk0);
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 12;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

// ============================================================================
// [Fog::Raster - Raster - Prgb32 - DstOut]
// ============================================================================

static void FOG_FASTCALL raster_prgb32_pixel_dstout_sse2(
  uint8_t* dst, uint32_t src)
{
  uint32_t dst0 = ((uint32_t*)dst)[0];
  uint32_t srcia = (~src) >> 24;

  if ((dst0 != 0x00000000) & (srcia != 0xFF))
  {
    __m128i dst0mm;
    __m128i src0mm;

    pix_unpack_1x1W(dst0mm, dst0);
    pix_expand_mask_1x1W(src0mm, srcia);
    pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);
  }
}

static void FOG_FASTCALL raster_prgb32_pixel_a8_dstout_sse2(
  uint8_t* dst, uint32_t src, uint32_t msk)
{
  uint32_t dst0 = ((uint32_t*)dst)[0];
  uint32_t srcia = alphaneg(alphamul(src >> 24, msk));

  if ((dst0 != 0x00000000) & (srcia != 0xFF))
  {
    __m128i dst0mm;
    __m128i src0mm;

    pix_unpack_1x1W(dst0mm, dst0);
    pix_expand_mask_1x1W(src0mm, srcia);
    pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);
  }
}

static void FOG_FASTCALL raster_prgb32_span_solid_dstout_sse2(
  uint8_t* dst, uint32_t src, sysint_t w)
{
  if ((src >> 24) == 0) return; // nop

  __m128i src0mm = _mm_cvtsi32_si128(src);
  pix_expand_pixel_1x4B(src0mm, src0mm);
  pix_unpack_1x2W(src0mm, src0mm);
  pix_expand_alpha_1x2W(src0mm, src0mm);
  pix_negate_1x1W(src0mm, src0mm);

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;

    uint32_t dst0 = ((uint32_t*)dst)[0];
    if (dst0 != 0x00000000)
    {
      pix_unpack_1x1W(dst0mm, dst0);
      pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0x00000000)
    {
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_multiply_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src0mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_solid_a8_dstout_sse2(
  uint8_t* dst, uint32_t src, const uint8_t* msk, sysint_t w)
{
  __m128i src0mm = _mm_cvtsi32_si128(src);
  pix_expand_pixel_1x4B(src0mm, src0mm);
  pix_unpack_1x2W(src0mm, src0mm);
  pix_expand_alpha_1x2W(src0mm, src0mm);
  pix_negate_1x1W(src0mm, src0mm);

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t dst0 = ((uint32_t*)dst)[0];

    if (dst0 != 0x00000000)
    {
      pix_unpack_1x1W(dst0mm, dst0);
      pix_expand_mask_1x1W(a0mm, READ_8(msk));
      pix_multiply_1x1W(a0mm, a0mm, src0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  if ((src >> 24) == 0)
  {
    BLIT_SSE2_LARGE_BEGIN(blt)
      __m128i dst0mm, dst1mm;
      __m128i a0mm, a1mm;

      pix_load16a(dst0mm, dst);

      uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
      if (dst0a != 0x00000000)
      {
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, READ_32(msk));
        pix_multiply_2x2W(dst0mm, dst0mm, a0mm, dst1mm, dst1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }

      dst += 16;
      msk += 4;
    BLIT_SSE2_LARGE_END(blt)
  }
  else
  {
    BLIT_SSE2_LARGE_BEGIN(blt)
      __m128i dst0mm, dst1mm;
      __m128i a0mm, a1mm;

      pix_load16a(dst0mm, dst);

      uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
      if (dst0a != 0x00000000)
      {
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, READ_32(msk));
        pix_multiply_2x2W(a0mm, a0mm, src0mm, a1mm, a1mm, src0mm);
        pix_multiply_2x2W(dst0mm, dst0mm, a0mm, dst1mm, dst1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }

      dst += 16;
      msk += 4;
    BLIT_SSE2_LARGE_END(blt)
  }
}

static void FOG_FASTCALL raster_prgb32_span_composite_axxx32_dstout_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    uint32_t dst0 = ((uint32_t*)dst)[0];
    uint32_t srcia = (~((uint32_t*)src)[0]) >> 24;

    if ((dst0 != 0x00000000) & (srcia != 0xFF))
    {
      pix_unpack_1x1W(dst0mm, dst0);
      pix_expand_mask_1x1W(src0mm, srcia);
      pix_multiply_1x1W(dst0mm, dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);
    src0mm = _mm_xor_si128(src0mm, MaskFFFFFFFFFFFFFFFF);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    uint32_t src0a = pix_pack_alpha_to_uint32(src0mm);

    if ((dst0a != 0x00000000) & (src0a != 0xFFFFFFFF))
    {
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_expand_alpha_2x2W(src0mm, src0mm, src1mm, src1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, src0mm, dst1mm, dst1mm, src1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_axxx32_a8_dstout_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t dst0 = ((uint32_t*)dst)[0];

    if (dst0 != 0x00000000)
    {
      pix_load4(src0mm, src);
      pix_unpack_1x1W(dst0mm, dst0);
      pix_unpack_1x1W(src0mm, src0mm);
      pix_unpack_1x1W(a0mm, READ_8(msk));
      pix_expand_alpha_1x1W(src0mm, src0mm);
      pix_negate_1x1W(src0mm, src0mm);
      pix_multiply_1x1W(a0mm, a0mm, src0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0x00000000)
    {
      pix_load16u(src0mm, src);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_expand_alpha_2x2W(src0mm, src0mm, src1mm, src1mm);
      pix_negate_2x2W(src0mm, src0mm, src1mm, src1mm);
      pix_expand_mask_2x2W(a0mm, a1mm, READ_32(msk));
      pix_multiply_2x2W(a0mm, a0mm, src0mm, a1mm, a1mm, src1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, a0mm, dst1mm, dst1mm, a1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_xxxx_a8_dstout_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  FOG_UNUSED(src);

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t dst0 = ((uint32_t*)dst)[0];

    if (dst0 != 0x00000000)
    {
      pix_unpack_1x1W(dst0mm, dst0);
      pix_unpack_1x1W(a0mm, READ_8(msk));
      pix_negate_1x1W(a0mm, a0mm);
      pix_multiply_1x1W(dst0mm, dst0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    if (dst0a != 0x00000000)
    {
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_expand_mask_2x2W(a0mm, a1mm, READ_32(msk));
      pix_negate_2x2W(a0mm, a0mm, a1mm, a1mm);
      pix_multiply_2x2W(dst0mm, dst0mm, a0mm, dst1mm, dst1mm, a1mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

// ============================================================================
// [Fog::Raster - Raster - Prgb32 - SrcAtop]
// ============================================================================

static void FOG_FASTCALL raster_prgb32_pixel_srcatop_sse2(
  uint8_t* dst, uint32_t src)
{
  uint32_t dst0 = ((uint32_t*)dst)[0];

  if (dst0 != 0x00000000)
  {
    uint32_t src0a = src >> 24;

    if (src0a != 0xFF)
    {
      __m128i src0mm;
      __m128i dst0mm;

      pix_unpack_1x1W(dst0mm, dst0);
      pix_unpack_1x1W(src0mm, src);
      pix_atop_1x1W(dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }
    else
    {
      ((uint32_t*)dst)[0] = src;
    }
  }
}

static void FOG_FASTCALL raster_prgb32_pixel_a8_srcatop_sse2(
  uint8_t* dst, uint32_t src, uint32_t msk)
{
  uint32_t dst0 = ((uint32_t*)dst)[0];

  if (dst0 != 0x00000000)
  {
    uint32_t src0a = src >> 24;

    if ((src0a != 0xFF) | (msk != 0xFF))
    {
      __m128i src0mm;
      __m128i dst0mm;
      __m128i a0mm;

      pix_unpack_1x1W(dst0mm, dst0);
      pix_unpack_1x1W(src0mm, src);
      pix_expand_mask_1x1W(a0mm, msk);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_atop_1x1W(dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }
    else
    {
      ((uint32_t*)dst)[0] = src;
    }
  }
}

static void FOG_FASTCALL raster_prgb32_span_solid_srcatop_sse2(
  uint8_t* dst, uint32_t src, sysint_t w)
{
  uint32_t a = src >> 24;

  __m128i src0orig = _mm_cvtsi32_si128(src);
  __m128i src0mm;
  __m128i srcia0mm;

  pix_expand_pixel_1x4B(src0orig, src0orig);
  pix_unpack_1x2W(src0mm, src0orig);
  pix_expand_alpha_1x2W(srcia0mm, src0mm);
  pix_negate_1x1W(srcia0mm, srcia0mm);

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;

    uint32_t dst0 = ((uint32_t*)dst)[0];
    if (dst0 != 0x00000000)
    {
      pix_unpack_1x1W(dst0mm, dst0);
      pix_atop_ialpha_1x1W(dst0mm, src0mm, srcia0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
  BLIT_SSE2_SMALL_END(blt)

  if (a != 0xFF)
  {
    BLIT_SSE2_LARGE_BEGIN(blt)
      __m128i dst0mm, dst1mm;

      pix_load16a(dst0mm, dst);
      uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);

      if (dst0a != 0x00000000)
      {
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_atop_ialpha_2x2W(dst0mm, src0mm, srcia0mm, dst1mm, src0mm, srcia0mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }

      dst += 16;
    BLIT_SSE2_LARGE_END(blt)
  }
  else
  {
    BLIT_SSE2_LARGE_BEGIN(blt)
      __m128i dst0mm, dst1mm;

      pix_load16a(dst0mm, dst);
      uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);

      if (dst0a != 0x00000000)
      {
        if (dst0a == 0xFFFFFFFF)
        {
          pix_store16a(dst, src0orig);
        }
        else
        {
          pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
          pix_atop_ialpha_2x2W(dst0mm, src0mm, srcia0mm, dst1mm, src0mm, srcia0mm);
          pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
          pix_store16a(dst, dst0mm);
        }
      }

      dst += 16;
    BLIT_SSE2_LARGE_END(blt)
  }
}

static void FOG_FASTCALL raster_prgb32_span_solid_a8_srcatop_sse2(
  uint8_t* dst, uint32_t src, const uint8_t* msk, sysint_t w)
{
  uint32_t a = src >> 24;

  __m128i src0orig = _mm_cvtsi32_si128(src);
  __m128i src0mm;

  pix_expand_pixel_1x4B(src0orig, src0orig);
  pix_unpack_1x2W(src0mm, src0orig);

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t dst0 = ((uint32_t*)dst)[0];
    uint32_t msk0 = READ_8(msk);

    if ((dst0 != 0x00000000) & (msk0 != 0x00))
    {
      pix_unpack_1x1W(dst0mm, dst0);
      pix_expand_mask_1x1W(a0mm, msk0);
      pix_multiply_1x1W(a0mm, a0mm, src0mm);
      pix_atop_1x1W(dst0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  if (a != 0xFF)
  {
    BLIT_SSE2_LARGE_BEGIN(blt)
      __m128i dst0mm, dst1mm;
      __m128i a0mm, a1mm;

      pix_load16a(dst0mm, dst);
      uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
      uint32_t msk0 = READ_32(msk);

      if ((dst0a != 0x00000000) & (msk0 != 0x00000000))
      {
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_multiply_2x2W(a0mm, a0mm, src0mm, a1mm, a1mm, src0mm);
        pix_atop_2x2W(dst0mm, a0mm, dst1mm, a1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }

      dst += 16;
      msk += 4;
    BLIT_SSE2_LARGE_END(blt)
  }
  else
  {
    BLIT_SSE2_LARGE_BEGIN(blt)
      __m128i dst0mm, dst1mm;
      __m128i a0mm, a1mm;

      pix_load16a(dst0mm, dst);
      uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
      uint32_t msk0 = READ_32(msk);

      if ((dst0a != 0x00000000) & (msk0 != 0x00000000))
      {
        if ((dst0a == 0xFFFFFFFF) & (msk0 == 0xFFFFFFFF))
        {
          pix_store16a(dst, src0orig);
        }
        else
        {
          pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
          pix_expand_mask_2x2W(a0mm, a1mm, msk0);
          pix_multiply_2x2W(a0mm, a0mm, src0mm, a1mm, a1mm, src0mm);
          pix_atop_2x2W(dst0mm, a0mm, dst1mm, a1mm);
          pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
          pix_store16a(dst, dst0mm);
        }
      }

      dst += 16;
      msk += 4;
    BLIT_SSE2_LARGE_END(blt)
  }
}

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_srcatop_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    uint32_t dst0, src0;
    if ((dst0 = READ_32(dst)) && (src0 = READ_32(src)))
    {
      pix_unpack_1x1W(src0mm, src0);
      pix_unpack_1x1W(dst0mm, dst0);
      pix_premultiply_1x1W(src0mm, src0mm);
      pix_atop_1x1W(dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);

      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    uint32_t src0a;

    if (dst0a != 0x00000000)
    {
      pix_load16u(src0mm, src);

      if ((src0a = pix_pack_alpha_to_uint32(src0mm)) != 0x00000000)
      {
        if ((dst0a == 0xFFFFFFFF) & (src0a == 0xFFFFFFFF))
        {
          pix_store16a(dst, src0mm);
        }
        else
        {
          pix_unpack_2x2W(src0mm, src1mm, src0mm);
          pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
          pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
          pix_atop_2x2W(dst0mm, src0mm, dst1mm, src1mm);
          pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
          pix_store16a(dst, dst0mm);
        }
      }
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_srcatop_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    uint32_t dst0, src0;
    if ((dst0 = READ_32(dst)) && (src0 = READ_32(src)))
    {
      pix_unpack_1x1W(src0mm, src0);
      pix_unpack_1x1W(dst0mm, dst0);
      pix_atop_1x1W(dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);

      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    uint32_t src0a;

    if (dst0a != 0x00000000)
    {
      pix_load16u(src0mm, src);
      src0a = pix_pack_alpha_to_uint32(src0mm);

      if (src0a != 0x00000000)
      {
        if ((dst0a == 0xFFFFFFFF) & (src0a == 0xFFFFFFFF))
        {
          pix_store16a(dst, src0mm);
        }
        else
        {
          pix_unpack_2x2W(src0mm, src1mm, src0mm);
          pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
          pix_atop_2x2W(dst0mm, src0mm, dst1mm, src1mm);
          pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
          pix_store16a(dst, dst0mm);
        }
      }
    }

    dst += 16;
    src += 16;
  BLIT_SSE2_LARGE_END(blt)
}

#define raster_prgb32_span_composite_rgb32_srcatop_sse2 raster_prgb32_span_composite_rgb32_srcin_sse2
#define raster_prgb32_span_composite_rgb24_srcatop_sse2 raster_prgb32_span_composite_rgb24_srcin_sse2

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_a8_srcatop_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    uint32_t dst0, src0, msk0;
    if ((dst0 = READ_32(dst)) && (src0 = READ_32(src)) && (msk0 = READ_8(msk)))
    {
      __m128i m0mm;

      pix_unpack_1x1W(src0mm, src0);
      pix_unpack_1x1W(dst0mm, dst0);
      pix_premultiply_1x1W(src0mm, src0mm);
      pix_expand_mask_1x1W(m0mm, msk0);
      pix_multiply_1x1W(src0mm, src0mm, m0mm);
      pix_atop_1x1W(dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);

      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    uint32_t src0a;
    uint32_t msk0;

    if (dst0a != 0x00000000)
    {
      pix_load16u(src0mm, src);
      if ((src0a = pix_pack_alpha_to_uint32(src0mm)) != 0x00000000)
      {
        if ((msk0 = READ_32(msk)) != 0x00000000)
        {
          if ((dst0a == 0xFFFFFFFF) & (src0a == 0xFFFFFFFF) & (msk0 == 0xFFFFFFFF))
          {
            pix_store16a(dst, src0mm);
          }
          else
          {
            __m128i m0mm, m1mm;

            pix_unpack_2x2W(src0mm, src1mm, src0mm);
            pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
            pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
            pix_expand_mask_2x2W(m0mm, m1mm, msk0);
            pix_multiply_2x2W(src0mm, src0mm, m0mm, src1mm, src1mm, m1mm);
            pix_atop_2x2W(dst0mm, src0mm, dst1mm, src1mm);
            pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
            pix_store16a(dst, dst0mm);
          }
        }
      }
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_a8_srcatop_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    uint32_t dst0, src0, msk0;
    if ((dst0 = READ_32(dst)) && (src0 = READ_32(src)) && (msk0 = READ_8(msk)))
    {
      __m128i m0mm;

      pix_unpack_1x1W(src0mm, src0);
      pix_unpack_1x1W(dst0mm, dst0);
      pix_expand_mask_1x1W(m0mm, msk0);
      pix_multiply_1x1W(src0mm, src0mm, m0mm);
      pix_atop_1x1W(dst0mm, src0mm);
      pix_pack_1x1W(dst0mm, dst0mm);

      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    uint32_t src0a;
    uint32_t msk0;

    if (dst0a != 0x00000000)
    {
      pix_load16u(src0mm, src);

      if ((src0a = pix_pack_alpha_to_uint32(src0mm)) != 0x00000000)
      {
        if ((msk0 = READ_32(msk)) != 0x00000000)
        {
          if ((dst0a == 0xFFFFFFFF) & (src0a == 0xFFFFFFFF) & (msk0 == 0xFFFFFFFF))
          {
            pix_store16a(dst, src0mm);
          }
          else
          {
            __m128i m0mm, m1mm;
            pix_unpack_2x2W(src0mm, src1mm, src0mm);
            pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
            pix_expand_mask_2x2W(m0mm, m1mm, msk0);
            pix_multiply_2x2W(src0mm, src0mm, m0mm, src1mm, src1mm, m1mm);
            pix_atop_2x2W(dst0mm, src0mm, dst1mm, src1mm);
            pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
            pix_store16a(dst, dst0mm);
          }
        }
      }
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb32_a8_srcatop_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    uint32_t dst0, src0, msk0;
    if ((dst0 = READ_32(dst)) && (msk0 = READ_8(msk)))
    {
      __m128i m0mm;

      src0 = READ_32(src) | 0xFF000000;

      pix_unpack_1x1W(src0mm, src0);
      pix_unpack_1x1W(dst0mm, dst0);
      pix_expand_mask_1x1W(m0mm, msk0);
      pix_multiply_1x1W(src0mm, src0mm, m0mm);
      pix_negate_1x1W(m0mm, m0mm);
      pix_atop_ialpha_1x1W(dst0mm, src0mm, m0mm);
      pix_pack_1x1W(dst0mm, dst0mm);

      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    uint32_t msk0;

    if (dst0a != 0x00000000 && (msk0 = READ_32(msk)) != 0x00000000)
    {
      pix_load16u(src0mm, src);
      src0mm = _mm_or_si128(src0mm, MaskFF000000FF000000);

      if ((dst0a == 0xFFFFFFFF) & (msk0 == 0xFFFFFFFF))
      {
        pix_store16a(dst, src0mm);
      }
      else
      {
        __m128i m0mm, m1mm;
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(m0mm, m1mm, msk0);
        pix_multiply_2x2W(src0mm, src0mm, m0mm, src1mm, src1mm, m1mm);
        pix_negate_2x2W(m0mm, m0mm, m1mm, m1mm);
        pix_atop_ialpha_2x2W(dst0mm, src0mm, m0mm, dst1mm, src1mm, m1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb24_a8_srcatop_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    uint32_t dst0, src0, msk0;
    if ((dst0 = READ_32(dst)) && (msk0 = READ_8(msk)))
    {
      __m128i m0mm;

      src0 = PixFmt_RGB24::fetch(src) | 0xFF000000;

      pix_unpack_1x1W(src0mm, src0);
      pix_unpack_1x1W(dst0mm, dst0);
      pix_expand_mask_1x1W(m0mm, msk0);
      pix_multiply_1x1W(src0mm, src0mm, m0mm);
      pix_negate_1x1W(m0mm, m0mm);
      pix_atop_ialpha_1x1W(dst0mm, src0mm, m0mm);
      pix_pack_1x1W(dst0mm, dst0mm);

      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 3;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);

    uint32_t dst0a = pix_pack_alpha_to_uint32(dst0mm);
    uint32_t msk0;

    if (dst0a != 0x00000000 && (msk0 = READ_32(msk)) != 0x00000000)
    {
      pix_fetch_rgb24_2x2W(src0mm, src1mm, src);

      if ((dst0a == 0xFFFFFFFF) & (msk0 == 0xFFFFFFFF))
      {
        pix_pack_2x2W(src0mm, src0mm, src1mm);
        pix_store16a(dst, src0mm);
      }
      else
      {
        __m128i m0mm, m1mm;

        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_expand_mask_2x2W(m0mm, m1mm, msk0);
        pix_multiply_2x2W(src0mm, src0mm, m0mm, src1mm, src1mm, m1mm);
        pix_negate_2x2W(m0mm, m0mm, m1mm, m1mm);
        pix_atop_ialpha_2x2W(dst0mm, src0mm, m0mm, dst1mm, src1mm, m1mm);
        pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
        pix_store16a(dst, dst0mm);
      }
    }

    dst += 16;
    src += 12;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

// ============================================================================
// [Fog::Raster - Raster - Prgb32 - DstAtop]
// ============================================================================

static void FOG_FASTCALL raster_prgb32_pixel_dstatop_sse2(
  uint8_t* dst, uint32_t src)
{
  uint32_t dst0 = ((uint32_t*)dst)[0];

  __m128i src0mm;
  __m128i dst0mm;
  __m128i a0mm;

  pix_unpack_1x1W(src0mm, src);
  pix_unpack_1x1W(dst0mm, dst0);
  pix_expand_alpha_1x1W(a0mm, src0mm);
  pix_atoprev_1x1W(dst0mm, src0mm, a0mm);
  pix_pack_1x1W(dst0mm, dst0mm);

  pix_store4(dst, dst0mm);
}

static void FOG_FASTCALL raster_prgb32_pixel_a8_dstatop_sse2(
  uint8_t* dst, uint32_t src, uint32_t msk)
{
  uint32_t dst0 = ((uint32_t*)dst)[0];

  __m128i src0mm;
  __m128i dst0mm;
  __m128i a0mm;

  pix_unpack_1x1W(src0mm, src);
  pix_unpack_1x1W(dst0mm, dst0);
  pix_expand_mask_1x1W(a0mm, msk);
  pix_multiply_1x1W(src0mm, src0mm, a0mm);
  pix_expand_alpha_1x1W(a0mm, src0mm);
  pix_atoprev_1x1W(dst0mm, src0mm, a0mm);
  pix_pack_1x1W(dst0mm, dst0mm);

  pix_store4(dst, dst0mm);
}

static void FOG_FASTCALL raster_prgb32_span_solid_dstatop_sse2(
  uint8_t* dst, uint32_t src, sysint_t w)
{
  __m128i src0mm = _mm_cvtsi32_si128(src);
  __m128i a0mm;

  pix_expand_pixel_1x4B(src0mm, src0mm);
  pix_unpack_1x2W(src0mm, src0mm);
  pix_expand_alpha_1x2W(a0mm, src0mm);

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;

    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_atoprev_1x1W(dst0mm, src0mm, a0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_atoprev_2x2W(dst0mm, src0mm, a0mm, dst1mm, src0mm, a0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_solid_a8_dstatop_sse2(
  uint8_t* dst, uint32_t src, const uint8_t* msk, sysint_t w)
{
  __m128i src0mm = _mm_cvtsi32_si128(src);
  pix_expand_pixel_1x4B(src0mm, src0mm);
  pix_unpack_1x2W(src0mm, src0mm);

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i a0mm;
    __m128i m0mm;

    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_expand_mask_1x1W(m0mm, READ_8(msk));
    pix_multiply_1x1W(m0mm, m0mm, src0mm);
    pix_expand_alpha_1x1W(a0mm, m0mm);
    pix_atoprev_1x1W(dst0mm, m0mm, a0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;
    __m128i m0mm, m1mm;

    pix_load16a(dst0mm, dst);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_expand_mask_2x2W(m0mm, m1mm, READ_32(msk));
    pix_multiply_2x2W(m0mm, m0mm, src0mm, m1mm, m1mm, src0mm);
    pix_expand_alpha_2x2W(a0mm, m0mm, a1mm, m1mm);
    pix_atoprev_2x2W(dst0mm, m0mm, a0mm, dst1mm, m1mm, a1mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_dstatop_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_load4(dst0mm, dst);
    pix_load4(src0mm, src);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_premultiply_1x1W(src0mm, src0mm);
    pix_expand_alpha_1x1W(a0mm, src0mm);
    pix_atoprev_1x1W(dst0mm, src0mm, a0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
    pix_expand_alpha_2x2W(a0mm, src0mm, a1mm, src1mm);
    pix_atoprev_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_dstatop_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_load4(dst0mm, dst);
    pix_load4(src0mm, src);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_expand_alpha_1x1W(a0mm, src0mm);
    pix_atoprev_1x1W(dst0mm, src0mm, a0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_expand_alpha_2x2W(a0mm, src0mm, a1mm, src1mm);
    pix_atoprev_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
  BLIT_SSE2_LARGE_END(blt)
}

#define raster_prgb32_span_composite_rgb32_dstatop_sse2 raster_prgb32_span_composite_rgb32_dstover_sse2
#define raster_prgb32_span_composite_rgb24_dstatop_sse2 raster_prgb32_span_composite_rgb24_dstover_sse2

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_a8_dstatop_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_load4(dst0mm, dst);
    pix_load4(src0mm, src);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_expand_mask_1x1W(a0mm, READ_8(msk));
    pix_premultiply_1x1W(src0mm, src0mm);
    pix_multiply_1x1W(src0mm, src0mm, a0mm);
    pix_expand_alpha_1x1W(a0mm, src0mm);
    pix_atoprev_1x1W(dst0mm, src0mm, a0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_expand_mask_2x2W(a0mm, a1mm, READ_32(msk));
    pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
    pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a0mm);
    pix_expand_alpha_2x2W(a0mm, src0mm, a1mm, src1mm);
    pix_atoprev_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_a8_dstatop_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_load4(dst0mm, dst);
    pix_load4(src0mm, src);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_expand_mask_1x1W(a0mm, READ_8(msk));
    pix_multiply_1x1W(src0mm, src0mm, a0mm);
    pix_expand_alpha_1x1W(a0mm, src0mm);
    pix_atoprev_1x1W(dst0mm, src0mm, a0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_expand_mask_2x2W(a0mm, a1mm, READ_32(msk));
    pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a0mm);
    pix_expand_alpha_2x2W(a0mm, src0mm, a1mm, src1mm);
    pix_atoprev_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb32_a8_dstatop_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_load4(dst0mm, dst);
    pix_load4(src0mm, src);
    src0mm = _mm_or_si128(src0mm, MaskFF000000FF000000);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_expand_mask_1x1W(a0mm, READ_8(msk));
    pix_multiply_1x1W(src0mm, src0mm, a0mm);
    pix_atoprev_1x1W(dst0mm, src0mm, a0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);
    src0mm = _mm_or_si128(src0mm, MaskFF000000FF000000);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_expand_mask_2x2W(a0mm, a1mm, READ_32(msk));
    pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a0mm);
    pix_atoprev_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb24_a8_dstatop_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_unpack_1x1W(src0mm, PixFmt_RGB24::fetch(src) | 0xFF000000);
    pix_expand_mask_1x1W(a0mm, READ_8(msk));
    pix_multiply_1x1W(src0mm, src0mm, a0mm);
    pix_atoprev_1x1W(dst0mm, src0mm, a0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 3;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    pix_fetch_rgb24_2x2W(src0mm, src1mm, src);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_expand_mask_2x2W(a0mm, a1mm, READ_32(msk));
    pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a0mm);
    pix_atoprev_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 12;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

// ============================================================================
// [Fog::Raster - Raster - Prgb32 - Xor]
// ============================================================================

static void FOG_FASTCALL raster_prgb32_pixel_xor_sse2(
  uint8_t* dst, uint32_t src)
{
  if (src)
  {
    uint32_t dst0 = ((uint32_t*)dst)[0];

    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_unpack_1x1W(src0mm, src);
    pix_unpack_1x1W(dst0mm, dst0);
    pix_expand_alpha_1x1W(a0mm, src0mm);
    pix_xor_1x1W(dst0mm, src0mm, a0mm);
    pix_pack_1x1W(dst0mm, dst0mm);

    pix_store4(dst, dst0mm);
  }
}

static void FOG_FASTCALL raster_prgb32_pixel_a8_xor_sse2(
  uint8_t* dst, uint32_t src, uint32_t msk)
{
  if (src && msk)
  {
    uint32_t dst0 = ((uint32_t*)dst)[0];

    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_unpack_1x1W(src0mm, src);
    pix_unpack_1x1W(dst0mm, dst0);
    pix_expand_mask_1x1W(a0mm, msk);
    pix_multiply_1x1W(src0mm, src0mm, a0mm);
    pix_expand_alpha_1x1W(a0mm, src0mm);
    pix_xor_1x1W(dst0mm, src0mm, a0mm);
    pix_pack_1x1W(dst0mm, dst0mm);

    pix_store4(dst, dst0mm);
  }
}

static void FOG_FASTCALL raster_prgb32_span_solid_xor_sse2(
  uint8_t* dst, uint32_t src, sysint_t w)
{
  if ((src >> 24) == 0xFF)
  {
    raster_prgb32_span_solid_srcout_sse2(dst, src, w);
    return;
  }

  __m128i src0mm = _mm_cvtsi32_si128(src);
  __m128i srcia0mm;
  pix_expand_pixel_1x4B(src0mm, src0mm);
  pix_unpack_1x2W(src0mm, src0mm);
  pix_expand_alpha_1x2W(srcia0mm, src0mm);
  pix_negate_1x1W(srcia0mm, srcia0mm);

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;

    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_xor_ialpha_1x1W(dst0mm, src0mm, srcia0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_xor_ialpha_2x2W(dst0mm, src0mm, srcia0mm, dst1mm, src0mm, srcia0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_solid_a8_xor_sse2(
  uint8_t* dst, uint32_t src, const uint8_t* msk, sysint_t w)
{
  __m128i src0mm = _mm_cvtsi32_si128(src);
  pix_expand_pixel_1x4B(src0mm, src0mm);
  pix_unpack_1x2W(src0mm, src0mm);

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i a0mm;
    __m128i m0mm;

    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_expand_mask_1x1W(m0mm, READ_8(msk));
    pix_multiply_1x1W(m0mm, m0mm, src0mm);
    pix_expand_alpha_1x1W(a0mm, m0mm);
    pix_xor_1x1W(dst0mm, m0mm, a0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;
    __m128i m0mm, m1mm;

    pix_load16a(dst0mm, dst);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_expand_mask_2x2W(m0mm, m1mm, READ_32(msk));
    pix_multiply_2x2W(m0mm, m0mm, src0mm, m1mm, m1mm, src0mm);
    pix_expand_alpha_2x2W(a0mm, m0mm, a1mm, m1mm);
    pix_xor_2x2W(dst0mm, m0mm, a0mm, dst1mm, m1mm, a1mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_xor_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_load4(dst0mm, dst);
    pix_load4(src0mm, src);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_premultiply_1x1W(src0mm, src0mm);
    pix_expand_alpha_1x1W(a0mm, src0mm);
    pix_xor_1x1W(dst0mm, src0mm, a0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
    pix_expand_alpha_2x2W(a0mm, src0mm, a1mm, src1mm);
    pix_xor_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_xor_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_load4(dst0mm, dst);
    pix_load4(src0mm, src);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_expand_alpha_1x1W(a0mm, src0mm);
    pix_xor_1x1W(dst0mm, src0mm, a0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_expand_alpha_2x2W(a0mm, src0mm, a1mm, src1mm);
    pix_xor_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
  BLIT_SSE2_LARGE_END(blt)
}

#define raster_prgb32_span_composite_rgb32_xor_sse2 raster_prgb32_span_composite_rgb32_srcout_sse2
#define raster_prgb32_span_composite_rgb24_xor_sse2 raster_prgb32_span_composite_rgb24_srcout_sse2

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_a8_xor_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_load4(dst0mm, dst);
    pix_load4(src0mm, src);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_expand_mask_1x1W(a0mm, READ_8(msk));
    pix_premultiply_1x1W(src0mm, src0mm);
    pix_multiply_1x1W(src0mm, src0mm, a0mm);
    pix_expand_alpha_1x1W(a0mm, src0mm);
    pix_xor_1x1W(dst0mm, src0mm, a0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_expand_mask_2x2W(a0mm, a1mm, READ_32(msk));
    pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
    pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a0mm);
    pix_expand_alpha_2x2W(a0mm, src0mm, a1mm, src1mm);
    pix_xor_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_a8_xor_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_load4(dst0mm, dst);
    pix_load4(src0mm, src);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_expand_mask_1x1W(a0mm, READ_8(msk));
    pix_multiply_1x1W(src0mm, src0mm, a0mm);
    pix_expand_alpha_1x1W(a0mm, src0mm);
    pix_xor_1x1W(dst0mm, src0mm, a0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_expand_mask_2x2W(a0mm, a1mm, READ_32(msk));
    pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a0mm);
    pix_expand_alpha_2x2W(a0mm, src0mm, a1mm, src1mm);
    pix_xor_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb32_a8_xor_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_load4(dst0mm, dst);
    pix_load4(src0mm, src);
    src0mm = _mm_or_si128(src0mm, MaskFF000000FF000000);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_expand_mask_1x1W(a0mm, READ_8(msk));
    pix_multiply_1x1W(src0mm, src0mm, a0mm);
    pix_xor_1x1W(dst0mm, src0mm, a0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);
    src0mm = _mm_or_si128(src0mm, MaskFF000000FF000000);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_expand_mask_2x2W(a0mm, a1mm, READ_32(msk));
    pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a0mm);
    pix_xor_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb24_a8_xor_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_load4(dst0mm, dst);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_unpack_1x1W(src0mm, PixFmt_RGB24::fetch(src) | 0xFF000000);
    pix_expand_mask_1x1W(a0mm, READ_8(msk));
    pix_multiply_1x1W(src0mm, src0mm, a0mm);
    pix_xor_1x1W(dst0mm, src0mm, a0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 3;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    pix_fetch_rgb24_2x2W(src0mm, src1mm, src);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_expand_mask_2x2W(a0mm, a1mm, READ_32(msk));
    pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a0mm);
    pix_xor_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 12;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

// ============================================================================
// [Fog::Raster - Raster - Prgb32 - Add]
// ============================================================================

static void FOG_FASTCALL raster_prgb32_pixel_add_sse2(
  uint8_t* dst, uint32_t src)
{
  if (src)
  {
    __m128i dst0mm;
    __m128i src0mm;

    pix_load4(dst0mm, dst);
    src0mm = _mm_cvtsi32_si128(src);
    dst0mm = _mm_adds_epu8(dst0mm, src0mm);
    pix_store4(dst, dst0mm);
  }
}

static void FOG_FASTCALL raster_prgb32_pixel_a8_add_sse2(
  uint8_t* dst, uint32_t src, uint32_t msk)
{
  if (src)
  {
    __m128i dst0mm;
    __m128i src0mm;
    __m128i a0mm;

    pix_load4(dst0mm, dst);
    src0mm = _mm_cvtsi32_si128(src);

    if (msk)
    {
      pix_unpack_1x1W(src0mm, src0mm);
      pix_expand_mask_1x1W(a0mm, msk);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_pack_1x1W(src0mm, src0mm);
    }

    dst0mm = _mm_adds_epu8(dst0mm, src0mm);
    pix_store4(dst, dst0mm);
  }
}

static void FOG_FASTCALL raster_prgb32_span_solid_add_sse2(
  uint8_t* dst, uint32_t src, sysint_t w)
{
  __m128i src0mm = _mm_cvtsi32_si128(src);
  pix_expand_pixel_1x4B(src0mm, src0mm);

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;

    pix_load4(dst0mm, dst);
    dst0mm = _mm_adds_epu8(dst0mm, src0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i dst0mm;

    pix_load16a(dst0mm, dst);
    dst0mm = _mm_adds_epu8(dst0mm, src0mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_solid_a8_add_sse2(
  uint8_t* dst, uint32_t src, const uint8_t* msk, sysint_t w)
{
  __m128i src0orig = _mm_cvtsi32_si128(src);
  __m128i src0mm;
  pix_expand_pixel_1x4B(src0orig, src0orig);
  pix_unpack_1x2W(src0mm, src0orig);

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i a0mm;

    pix_load4(dst0mm, dst);
    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0xFF)
    {
      pix_expand_mask_1x1W(a0mm, msk0);
      pix_multiply_1x1W(a0mm, a0mm, src0mm);
      pix_pack_1x1W(a0mm, a0mm);
      dst0mm = _mm_adds_epu8(dst0mm, a0mm);
      pix_store4(dst, dst0mm);
    }
    else
    {
      dst0mm = _mm_adds_epu8(dst0mm, src0orig);
      pix_store4(dst, dst0mm);
    }

    dst += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i dst0mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    uint32_t msk0 = READ_32(msk);
    if (msk0 != 0xFFFFFFFF)
    {
      pix_expand_mask_2x2W(a0mm, a1mm, msk0);
      pix_multiply_2x2W(a0mm, a0mm, src0mm, a1mm, a1mm, src0mm);
      pix_pack_2x2W(a0mm, a0mm, a1mm);
      dst0mm = _mm_adds_epu8(dst0mm, a0mm);
      pix_store16a(dst, dst0mm);
    }
    else
    {
      dst0mm = _mm_adds_epu8(dst0mm, src0orig);
      pix_store16a(dst, dst0mm);
    }

    dst += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_add_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;

    pix_load4(dst0mm, dst);
    pix_load4(src0mm, src);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_premultiply_1x1W(src0mm, src0mm);
    pix_pack_1x1W(src0mm, src0mm);
    dst0mm = _mm_adds_epu8(dst0mm, src0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm, src1mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
    pix_pack_2x2W(src0mm, src0mm, src1mm);
    dst0mm = _mm_adds_epu8(dst0mm, src0mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_argb32_a8_add_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;
    __m128i a0mm;

    pix_load4(dst0mm, dst);
    pix_load4(src0mm, src);
    pix_unpack_1x1W(src0mm, src0mm);
    pix_premultiply_1x1W(src0mm, src0mm);
    pix_expand_mask_1x1W(a0mm, READ_8(msk));
    pix_multiply_1x1W(src0mm, src0mm, a0mm);
    pix_pack_1x1W(src0mm, src0mm);
    dst0mm = _mm_adds_epu8(dst0mm, src0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);
    pix_unpack_2x2W(src0mm, src1mm, src0mm);
    pix_premultiply_2x2W(src0mm, src0mm, src1mm, src1mm);
    pix_expand_mask_2x2W(a0mm, a1mm, READ_32(msk));
    pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
    pix_pack_2x2W(src0mm, src0mm, src1mm);
    dst0mm = _mm_adds_epu8(dst0mm, src0mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_add_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    pix_load4(dst0mm, dst);
    pix_load4(src0mm, src);
    dst0mm = _mm_adds_epu8(dst0mm, src0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);
    dst0mm = _mm_adds_epu8(dst0mm, src0mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_prgb32_a8_add_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;
    __m128i a0mm;

    pix_load4(dst0mm, dst);
    pix_load4(src0mm, src);

    uint32_t msk0 = READ_8(msk);

    if (msk0 != 0xFF)
    {
      pix_unpack_1x1W(src0mm, src0mm);
      pix_expand_mask_1x1W(a0mm, msk0);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_pack_1x1W(src0mm, src0mm);
    }

    dst0mm = _mm_adds_epu8(dst0mm, src0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);

    uint32_t msk0 = READ_8(msk);

    if (msk0 != 0xFFFFFFFF)
    {
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_expand_mask_2x2W(a0mm, a1mm, msk0);
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
      pix_pack_2x2W(src0mm, src0mm, src1mm);
    }

    dst0mm = _mm_adds_epu8(dst0mm, src0mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb32_add_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  __m128i amask = MaskFF000000FF000000;

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;

    pix_load4(dst0mm, dst);
    pix_load4(src0mm, src);
    src0mm = _mm_or_si128(src0mm, amask);
    dst0mm = _mm_adds_epu8(dst0mm, src0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);
    src0mm = _mm_or_si128(src0mm, amask);
    dst0mm = _mm_adds_epu8(dst0mm, src0mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb32_a8_add_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  __m128i amask = MaskFF000000FF000000;

  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;
    __m128i a0mm;

    pix_load4(dst0mm, dst);
    pix_load4(src0mm, src);
    src0mm = _mm_or_si128(src0mm, amask);

    uint32_t msk0 = READ_8(msk);

    if (msk0 != 0xFF)
    {
      pix_unpack_1x1W(src0mm, src0mm);
      pix_expand_mask_1x1W(a0mm, msk0);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_pack_1x1W(src0mm, src0mm);
    }

    dst0mm = _mm_adds_epu8(dst0mm, src0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    pix_load16u(src0mm, src);
    src0mm = _mm_or_si128(src0mm, amask);

    uint32_t msk0 = READ_8(msk);

    if (msk0 != 0xFFFFFFFF)
    {
      pix_unpack_2x2W(src0mm, src1mm, src0mm);
      pix_expand_mask_2x2W(a0mm, a1mm, msk0);
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
      pix_pack_2x2W(src0mm, src0mm, src1mm);
    }

    dst0mm = _mm_adds_epu8(dst0mm, src0mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb24_add_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;

    pix_load4(dst0mm, dst);
    src0mm = _mm_cvtsi32_si128(PixFmt_RGB24::fetch(src) | 0xFF000000);
    dst0mm = _mm_adds_epu8(dst0mm, src0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 3;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm, src1mm;

    pix_load16a(dst0mm, dst);
    pix_fetch_rgb24_2x2W(src0mm, src1mm, src);
    pix_pack_2x2W(src0mm, src0mm, src1mm);
    dst0mm = _mm_adds_epu8(dst0mm, src0mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 12;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_prgb32_span_composite_rgb24_a8_add_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;
    __m128i a0mm;

    pix_load4(dst0mm, dst);
    src0mm = _mm_cvtsi32_si128(PixFmt_RGB24::fetch(src) | 0xFF000000);

    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0xFF)
    {
      pix_unpack_1x1W(src0mm, src0mm);
      pix_expand_mask_1x1W(a0mm, msk0);
      pix_multiply_1x1W(src0mm, src0mm, a0mm);
      pix_pack_1x1W(src0mm, src0mm);
    }

    dst0mm = _mm_adds_epu8(dst0mm, src0mm);
    pix_store4(dst, dst0mm);

    dst += 4;
    src += 3;
    msk += 1;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm, src1mm;
    __m128i a0mm, a1mm;

    pix_load16a(dst0mm, dst);
    pix_fetch_rgb24_2x2W(src0mm, src1mm, src);

    uint32_t msk0 = READ_8(msk);
    if (msk0 != 0xFFFFFFFF)
    {
      pix_expand_mask_2x2W(a0mm, a1mm, msk0);
      pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
    }

    pix_pack_2x2W(src0mm, src0mm, src1mm);
    dst0mm = _mm_adds_epu8(dst0mm, src0mm);
    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 12;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
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

    pix_unpack_1x1W(src0mm, src);
    pix_load4(dst0mm, dst);
    dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_over_1x1W(dst0mm, src0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);
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

  if ((a != 0xFF) | (msk != 0xFF))
  {
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    pix_unpack_1x1W(src0mm, src);
    pix_expand_mask_1x1W(a0mm, msk);
    pix_load4(dst0mm, dst);
    dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);
    pix_unpack_1x1W(dst0mm, dst0mm);
    pix_multiply_1x1W(src0mm, src0mm, a0mm);
    pix_over_1x1W(dst0mm, src0mm);
    pix_pack_1x1W(dst0mm, dst0mm);
    pix_store4(dst, dst0mm);
  }
  else
  {
    ((uint32_t*)dst)[0] = src;
  }
}

static void FOG_FASTCALL raster_rgb32_span_solid_sse2(
  uint8_t* dst, uint32_t src, sysint_t w)
{
  __m128i src0mm = _mm_cvtsi32_si128(src);
  pix_expand_pixel_1x4B(src0mm, src0mm);

  BLIT_SSE2_INIT(dst, w)

  if ((src >> 24) == 0xFF)
  {
    BLIT_SSE2_SMALL_BEGIN(blt_opaque)
      ((uint32_t*)dst)[0] = src;
      dst += 4;
    BLIT_SSE2_SMALL_END(blt_opaque)

    while (w >= 4)
    {
      _mm_store_si128((__m128i*)(dst), src0mm);
      _mm_store_si128((__m128i*)(dst + 16), src0mm);
      _mm_store_si128((__m128i*)(dst + 32), src0mm);
      _mm_store_si128((__m128i*)(dst + 48), src0mm);

      dst += 64;
      w -= 4;
    }
    switch (w & 3)
    {
      case 3: _mm_store_si128((__m128i*)(dst), src0mm); dst += 16;
      case 2: _mm_store_si128((__m128i*)(dst), src0mm); dst += 16;
      case 1: _mm_store_si128((__m128i*)(dst), src0mm); dst += 16;
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

    BLIT_SSE2_SMALL_BEGIN(blt_trans)
      __m128i dst0mm;

      pix_load4(dst0mm, dst);
      dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_over_ialpha_1x1W(dst0mm, src0mm, ia0mm);
      pix_pack_1x1W(dst0mm, dst0mm);
      pix_store4(dst, dst0mm);

      dst += 4;
    BLIT_SSE2_SMALL_END(blt_trans)

    BLIT_SSE2_LARGE_BEGIN(blt_trans)
      __m128i dst0mm;
      __m128i dst1mm;

      pix_load16a(dst0mm, dst);
      dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);
      pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
      pix_over_ialpha_2x2W(dst0mm, src0mm, ia0mm, dst1mm, src0mm, ia0mm);
      pix_pack_2x2W(dst0mm, dst0mm, dst1mm);
      pix_store16a(dst, dst0mm);

      dst += 16;
    BLIT_SSE2_LARGE_END(blt_trans)
  }
}

static void FOG_FASTCALL raster_rgb32_span_solid_a8_sse2(
  uint8_t* dst, uint32_t src, const uint8_t* msk, sysint_t w)
{
  __m128i src0orig = _mm_cvtsi32_si128(src);
  __m128i src0unpacked;
  pix_expand_pixel_1x4B(src0orig, src0orig);
  pix_unpack_1x2W(src0unpacked, src0orig);

  BLIT_SSE2_INIT(dst, w);

  if ((src >> 24) == 0xFF)
  {
    BLIT_SSE2_SMALL_BEGIN(blt_opaque)
      __m128i dst0mm;
      __m128i src0mm;
      __m128i a0mm;

      uint32_t msk0;
      if ((msk0 = READ_8(msk)) == 0xFF)
      {
        ((uint32_t*)dst)[0] = src;
      }
      else if (msk0)
      {
        pix_load4(dst0mm, dst);
        dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);
        pix_unpack_1x1W(dst0mm, dst0mm);
        pix_expand_mask_1x1W(a0mm, msk0);
        pix_expand_pixel_1x2W(a0mm, a0mm);
        pix_multiply_1x1W(src0mm, src0unpacked, a0mm);
        pix_over_1x1W(dst0mm, src0mm, a0mm);
        pix_pack_1x1W(dst0mm, dst0mm);
        pix_store4(dst, dst0mm);
      }

      dst += 4;
      msk += 1;
    BLIT_SSE2_SMALL_END(blt_opaque)

    BLIT_SSE2_LARGE_BEGIN(blt_opaque)
      __m128i dst0mm, dst1mm;
      __m128i src0mm, src1mm;
      __m128i a0mm, a1mm;

      uint32_t msk0;
      if ((msk0 = ((uint32_t*)msk)[0]))
      {
        if (msk0 == 0xFFFFFFFF)
        {
          _mm_store_si128((__m128i*)dst, src0orig);
        }
        else
        {
          pix_load16a(dst0mm, dst);
          dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);
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
    BLIT_SSE2_LARGE_END(blt_opaque)
  }
  else
  {
    BLIT_SSE2_SMALL_BEGIN(blt_trans)
      __m128i dst0mm;
      __m128i src0mm;
      __m128i a0mm;

      uint32_t msk0;
      if ((msk0 = READ_8(msk)))
      {
        pix_load4(dst0mm, dst);
        dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);
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
    BLIT_SSE2_SMALL_END(blt_trans)

    BLIT_SSE2_LARGE_BEGIN(blt_trans)
      __m128i dst0mm, dst1mm;
      __m128i src0mm, src1mm;
      __m128i a0mm, a1mm;

      uint32_t msk0;
      if ((msk0 = ((uint32_t*)msk)[0]))
      {
        pix_load16a(dst0mm, dst);
        dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);
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
    BLIT_SSE2_LARGE_END(blt_trans)
  }
}

static void FOG_FASTCALL raster_rgb32_span_composite_argb32_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w);

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i dst0mm;
    __m128i src0mm;
    __m128i a0mm;

    uint32_t src0 = READ_32(src);
    uint32_t src0a = src0 >> 24;
    if (src0a != 0x00)
    {
      if (src0a != 0xFF)
      {
        pix_load4(dst0mm, dst);
        dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);
        pix_unpack_1x1W(src0mm, src0);
        pix_unpack_1x1W(dst0mm, dst0mm);

        pix_expand_alpha_1x1W(a0mm, src0mm);
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
    src += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
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
        dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);

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
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_rgb32_span_composite_prgb32_sse2(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t src0 = READ_32(src);

    if (src0)
    {
      pix_load4(dst0mm, dst);
      dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);
      pix_unpack_1x1W(src0mm, src0);
      pix_unpack_1x1W(dst0mm, dst0mm);
      pix_expand_alpha_1x1W(a0mm, src0mm);
      pix_over_1x1W(dst0mm, src0mm, a0mm);
      pix_pack_1x1W(dst0mm, dst0mm);

      pix_store4(dst, dst0mm);
    }

    dst += 4;
    src += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
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
        dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);

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
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_rgb32_span_composite_argb32_a8_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w);

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t src0 = READ_32(src);
    uint32_t src0a = src0 >> 24;
    uint32_t msk0 = READ_8(msk);

    if ((src0a != 0x00) & (msk0 != 0x00))
    {
      msk0 = alphamul(src0a, msk0);

      pix_load4(dst0mm, dst);
      dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);
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
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
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
        dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);
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
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_rgb32_span_composite_prgb32_a8_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t src0 = READ_32(src);
    uint32_t msk0 = READ_8(msk);

    if ((src0 != 0x00000000) & (msk0 != 0x00))
    {
      pix_load4(dst0mm, dst);
      dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);
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
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
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
        dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);

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
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_rgb32_span_composite_rgb32_a8_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
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
        dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);
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
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
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
        dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);
        src0mm = _mm_or_si128(src0mm, MaskFF000000FF000000);

        pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
        pix_unpack_2x2W(src0mm, src1mm, src0mm);
        pix_expand_mask_2x2W(a0mm, a1mm, msk0);
        pix_multiply_2x2W(src0mm, src0mm, a0mm, src1mm, src1mm, a1mm);
        pix_over_2x2W(dst0mm, src0mm, a0mm, dst1mm, src1mm, a1mm);
        pix_pack_2x2W(src0mm, dst0mm, dst1mm);
      }
      pix_store16a(dst, src0mm);
    }

    dst += 16;
    src += 16;
    msk += 4;
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_rgb32_span_composite_rgb24_a8_sse2(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  BLIT_SSE2_INIT(dst, w)

  BLIT_SSE2_SMALL_BEGIN(blt)
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
        dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);
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
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
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
        dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);

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
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_rgb32_span_composite_argb32_a8_const_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk, sysint_t w)
{
  if (FOG_UNLIKELY(msk == 0xFF))
  {
    raster_rgb32_span_composite_argb32_sse2(dst, src, w);
    return;
  }

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_INIT(dst, w);

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t src0 = READ_32(src);
    uint32_t src0a = src0 >> 24;

    if ((src0a != 0x00))
    {
      if ((src0a == 0xFF))
      {
        pix_load4(dst0mm, dst);
        dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);

        pix_unpack_1x1W(src0mm, src0);
        pix_unpack_1x1W(dst0mm, dst0mm);

        pix_multiply_1x1W(src0mm, src0mm, m0mm);
        pix_over_ialpha_1x1W(dst0mm, src0mm, im0mm);
        pix_pack_1x1W(dst0mm, dst0mm);

        pix_store4(dst, dst0mm);
      }
      else
      {
        uint32_t msk0 = alphamul(src0a, msk);

        pix_load4(dst0mm, dst);
        dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);

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
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
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
        dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);

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
        dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);

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
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_rgb32_span_composite_prgb32_a8_const_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk, sysint_t w)
{
  if (FOG_UNLIKELY(msk == 0xFF))
  {
    raster_rgb32_span_composite_argb32_sse2(dst, src, w);
    return;
  }

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_INIT(dst, w);

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;
    __m128i a0mm;

    uint32_t src0 = READ_32(src);
    uint32_t src0a = src0 >> 24;

    if ((src0a != 0x00))
    {
      if ((src0a == 0xFF))
      {
        pix_load4(dst0mm, dst);
        dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);
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
        dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);
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
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
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
        dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);

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
        dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);

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
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_rgb32_span_composite_rgb32_a8_const_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk, sysint_t w)
{
  if (FOG_UNLIKELY(msk == 0xFF))
  {
    convert_memcpy32_sse2(dst, src, w);
    return;
  }

  __m128i m0mm, im0mm;
  __m128i amask = MaskFF000000FF000000;
  pix_expand_mask_1x1W(m0mm, msk);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_INIT(dst, w);

  BLIT_SSE2_SMALL_BEGIN(blt)
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
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
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
  BLIT_SSE2_LARGE_END(blt)
}

static void FOG_FASTCALL raster_rgb32_span_composite_rgb24_a8_const_sse2(
  uint8_t* dst, const uint8_t* src, uint32_t msk, sysint_t w)
{
  if (FOG_UNLIKELY(msk == 0xFF))
  {
    convert_rgb32_from_rgb24_sse2(dst, src, w);
    return;
  }

  __m128i m0mm, im0mm;
  pix_expand_mask_1x1W(m0mm, msk);
  pix_expand_pixel_1x2W(m0mm, m0mm);
  pix_negate_1x2W(im0mm, m0mm);

  BLIT_SSE2_INIT(dst, w);

  BLIT_SSE2_SMALL_BEGIN(blt)
    __m128i src0mm;
    __m128i dst0mm;

    uint32_t src0 = PixFmt_RGB24::fetch(src) | 0xFF000000;

    pix_load4(dst0mm, dst);
    dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);
    pix_unpack_1x1W(src0mm, src0);
    pix_unpack_1x1W(dst0mm, dst0mm);

    pix_multiply_1x1W(src0mm, src0mm, m0mm);
    pix_over_ialpha_1x1W(dst0mm, src0mm, im0mm);
    pix_pack_1x1W(dst0mm, dst0mm);

    pix_store4(dst, dst0mm);

    dst += 4;
    src += 4;
  BLIT_SSE2_SMALL_END(blt)

  BLIT_SSE2_LARGE_BEGIN(blt)
    __m128i src0mm, src1mm;
    __m128i dst0mm, dst1mm;

    pix_load16a(dst0mm, dst);
    dst0mm = _mm_or_si128(dst0mm, MaskFF000000FF000000);
    pix_unpack_2x2W(dst0mm, dst1mm, dst0mm);
    pix_fetch_rgb24_2x2W(src0mm, src1mm, src);
    pix_multiply_2x2W(src0mm, src0mm, m0mm, src1mm, src1mm, m0mm);
    pix_over_ialpha_2x2W(dst0mm, src0mm, im0mm, dst1mm, src1mm, im0mm);
    pix_pack_2x2W(dst0mm, dst0mm, dst1mm);

    pix_store16a(dst, dst0mm);

    dst += 16;
    src += 16;
  BLIT_SSE2_LARGE_END(blt)
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
  Mask000000FF00FF00FF = pix_create_mask_8x2W(0x0000, 0x00FF, 0x00FF, 0x00FF);
  Mask00FF00FF00FF00FF = pix_create_mask_8x2W(0x00FF, 0x00FF, 0x00FF, 0x00FF);
  MaskFF000000000000FF = pix_create_mask_8x2W(0xFF00, 0x0000, 0x0000, 0x00FF);
  Mask0101010101010101 = pix_create_mask_8x2W(0x0101, 0x0101, 0x0101, 0x0101);
  MaskFFFFFFFFFFFFFFFF = pix_create_mask_8x2W(0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF);
  Mask00FF000000000000 = pix_create_mask_8x2W(0x00FF, 0x0000, 0x0000, 0x0000);
  MaskFF000000FF000000 = pix_create_mask_8x2W(0xFF00, 0x0000, 0xFF00, 0x0000);

  sse2_t t;

  t.uq[0] = FOG_UINT64_C(~0x8000000080000000);
  t.uq[1] = FOG_UINT64_C(~0x8000000080000000);
  Mask7FFFFFFF7FFFFFFF = _mm_loadu_ps((float*)&t);

  t.uq[0] = FOG_UINT64_C(~0x8000000000000000);
  t.uq[1] = FOG_UINT64_C(~0x8000000000000000);
  Mask7FFFFFFFFFFFFFFF = _mm_loadu_pd((double*)&t);

  FunctionMap* m = functionMap;

  // [Convert]

  m->convert.memcpy32 = convert_memcpy32_sse2;

  m->convert.prgb32_from_argb32 = convert_prgb32_from_argb32_sse2;
  m->convert.prgb32_from_argb32_bs = convert_prgb32_from_argb32_bs_sse2;
  m->convert.prgb32_bs_from_argb32 = convert_prgb32_bs_from_argb32_sse2;
  m->convert.argb32_from_rgb32 = convert_argb32_from_rgb32_sse2;
  m->convert.rgb32_from_rgb24 = convert_rgb32_from_rgb24_sse2;
  m->convert.rgb32_from_bgr24 = convert_rgb32_from_bgr24_sse2;

  // [Gradient - Gradient]

  m->gradient.gradient_argb32 = gradient_gradient_argb32_sse2;
  m->gradient.gradient_prgb32 = gradient_gradient_prgb32_sse2;

  // [Pattern - Texture]

  m->pattern.texture_fetch_repeat = pattern_texture_fetch_repeat_sse2;
  m->pattern.texture_fetch_reflect = pattern_texture_fetch_reflect_sse2;

  // [Pattern - Radial Gradient]

  m->pattern.radial_gradient_fetch_pad = pattern_radial_gradient_fetch_pad_sse2;

  // [Raster - Prgb32]

  m->raster_argb32[0][CompositeSrc].span_composite[Image::FormatRGB24] = convert_rgb32_from_rgb24_sse2;
  m->raster_argb32[1][CompositeSrc].span_composite[Image::FormatRGB24] = convert_rgb32_from_rgb24_sse2;

  m->raster_argb32[0][CompositeSrcOver].span_composite[Image::FormatRGB24] = convert_rgb32_from_rgb24_sse2;

  m->raster_argb32[1][CompositeSrcOver].pixel = raster_prgb32_pixel_srcover_sse2;
  m->raster_argb32[1][CompositeSrcOver].pixel_a8 = raster_prgb32_pixel_a8_srcover_sse2;
  m->raster_argb32[1][CompositeSrcOver].span_solid = raster_prgb32_span_solid_srcover_sse2;
  m->raster_argb32[1][CompositeSrcOver].span_solid_a8 = raster_prgb32_span_solid_a8_srcover_sse2;
  m->raster_argb32[1][CompositeSrcOver].span_composite[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_srcover_sse2;
  m->raster_argb32[1][CompositeSrcOver].span_composite[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_srcover_sse2;
  m->raster_argb32[1][CompositeSrcOver].span_composite[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_srcover_sse2;
  m->raster_argb32[1][CompositeSrcOver].span_composite[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_srcover_sse2;
  m->raster_argb32[1][CompositeSrcOver].span_composite_a8[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_srcover_sse2;
  m->raster_argb32[1][CompositeSrcOver].span_composite_a8[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_srcover_sse2;
  m->raster_argb32[1][CompositeSrcOver].span_composite_a8[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_srcover_sse2;
  m->raster_argb32[1][CompositeSrcOver].span_composite_a8[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_a8_srcover_sse2;
  m->raster_argb32[1][CompositeSrcOver].span_composite_a8_const[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_const_srcover_sse2;
  m->raster_argb32[1][CompositeSrcOver].span_composite_a8_const[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_const_srcover_sse2;
  m->raster_argb32[1][CompositeSrcOver].span_composite_a8_const[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_const_srcover_sse2;
  m->raster_argb32[1][CompositeSrcOver].span_composite_a8_const[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_a8_const_srcover_sse2;

  m->raster_argb32[1][CompositeDestOver].pixel = raster_prgb32_pixel_dstover_sse2;
  m->raster_argb32[1][CompositeDestOver].pixel_a8 = raster_prgb32_pixel_a8_dstover_sse2;
  m->raster_argb32[1][CompositeDestOver].span_solid = raster_prgb32_span_solid_dstover_sse2;
  m->raster_argb32[1][CompositeDestOver].span_solid_a8 = raster_prgb32_span_solid_a8_dstover_sse2;
  m->raster_argb32[1][CompositeDestOver].span_composite[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_dstover_sse2;
  m->raster_argb32[1][CompositeDestOver].span_composite[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_dstover_sse2;
  m->raster_argb32[1][CompositeDestOver].span_composite[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_dstover_sse2;
  m->raster_argb32[1][CompositeDestOver].span_composite[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_dstover_sse2;
  m->raster_argb32[1][CompositeDestOver].span_composite_a8[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_dstover_sse2;
  m->raster_argb32[1][CompositeDestOver].span_composite_a8[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_dstover_sse2;
  m->raster_argb32[1][CompositeDestOver].span_composite_a8[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_dstover_sse2;
  m->raster_argb32[1][CompositeDestOver].span_composite_a8[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_a8_dstover_sse2;

  m->raster_argb32[1][CompositeSrcIn].pixel = raster_prgb32_pixel_srcin_sse2;
  m->raster_argb32[1][CompositeSrcIn].pixel_a8 = raster_prgb32_pixel_a8_srcin_sse2;
  m->raster_argb32[1][CompositeSrcIn].span_solid = raster_prgb32_span_solid_srcin_sse2;
  m->raster_argb32[1][CompositeSrcIn].span_solid_a8 = raster_prgb32_span_solid_a8_srcin_sse2;
  m->raster_argb32[1][CompositeSrcIn].span_composite[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_srcin_sse2;
  m->raster_argb32[1][CompositeSrcIn].span_composite[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_srcin_sse2;
  m->raster_argb32[1][CompositeSrcIn].span_composite[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_srcin_sse2;
  m->raster_argb32[1][CompositeSrcIn].span_composite[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_srcin_sse2;
  m->raster_argb32[1][CompositeSrcIn].span_composite_a8[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_srcin_sse2;
  m->raster_argb32[1][CompositeSrcIn].span_composite_a8[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_srcin_sse2;
  m->raster_argb32[1][CompositeSrcIn].span_composite_a8[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_srcin_sse2;
  m->raster_argb32[1][CompositeSrcIn].span_composite_a8[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_a8_srcin_sse2;

  m->raster_argb32[1][CompositeDestIn].pixel = raster_prgb32_pixel_dstin_sse2;
  m->raster_argb32[1][CompositeDestIn].pixel_a8 = raster_prgb32_pixel_a8_dstin_sse2;
  m->raster_argb32[1][CompositeDestIn].span_solid = raster_prgb32_span_solid_dstin_sse2;
  m->raster_argb32[1][CompositeDestIn].span_solid_a8 = raster_prgb32_span_solid_a8_dstin_sse2;
  m->raster_argb32[1][CompositeDestIn].span_composite[Image::FormatARGB32] = raster_prgb32_span_composite_axxx32_dstin_sse2;
  m->raster_argb32[1][CompositeDestIn].span_composite[Image::FormatPRGB32] = raster_prgb32_span_composite_axxx32_dstin_sse2;
  m->raster_argb32[1][CompositeDestIn].span_composite_a8[Image::FormatARGB32] = raster_prgb32_span_composite_axxx32_a8_dstin_sse2;
  m->raster_argb32[1][CompositeDestIn].span_composite_a8[Image::FormatPRGB32] = raster_prgb32_span_composite_axxx32_a8_dstin_sse2;
  m->raster_argb32[1][CompositeDestIn].span_composite_a8[Image::FormatRGB32] = raster_prgb32_span_composite_xxxx_a8_dstin_sse2;
  m->raster_argb32[1][CompositeDestIn].span_composite_a8[Image::FormatRGB24] = raster_prgb32_span_composite_xxxx_a8_dstin_sse2;

  m->raster_argb32[1][CompositeSrcOut].pixel = raster_prgb32_pixel_srcout_sse2;
  m->raster_argb32[1][CompositeSrcOut].pixel_a8 = raster_prgb32_pixel_a8_srcout_sse2;
  m->raster_argb32[1][CompositeSrcOut].span_solid = raster_prgb32_span_solid_srcout_sse2;
  m->raster_argb32[1][CompositeSrcOut].span_solid_a8 = raster_prgb32_span_solid_a8_srcout_sse2;
  m->raster_argb32[1][CompositeSrcOut].span_composite[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_srcout_sse2;
  m->raster_argb32[1][CompositeSrcOut].span_composite[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_srcout_sse2;
  m->raster_argb32[1][CompositeSrcOut].span_composite[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_srcout_sse2;
  m->raster_argb32[1][CompositeSrcOut].span_composite[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_srcout_sse2;
  m->raster_argb32[1][CompositeSrcOut].span_composite_a8[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_srcout_sse2;
  m->raster_argb32[1][CompositeSrcOut].span_composite_a8[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_srcout_sse2;
  m->raster_argb32[1][CompositeSrcOut].span_composite_a8[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_srcout_sse2;
  m->raster_argb32[1][CompositeSrcOut].span_composite_a8[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_a8_srcout_sse2;

  m->raster_argb32[1][CompositeDestOut].pixel = raster_prgb32_pixel_dstout_sse2;
  m->raster_argb32[1][CompositeDestOut].pixel_a8 = raster_prgb32_pixel_a8_dstout_sse2;
  m->raster_argb32[1][CompositeDestOut].span_solid = raster_prgb32_span_solid_dstout_sse2;
  m->raster_argb32[1][CompositeDestOut].span_solid_a8 = raster_prgb32_span_solid_a8_dstout_sse2;
  m->raster_argb32[1][CompositeDestOut].span_composite[Image::FormatARGB32] = raster_prgb32_span_composite_axxx32_dstout_sse2;
  m->raster_argb32[1][CompositeDestOut].span_composite[Image::FormatPRGB32] = raster_prgb32_span_composite_axxx32_dstout_sse2;
  m->raster_argb32[1][CompositeDestOut].span_composite_a8[Image::FormatARGB32] = raster_prgb32_span_composite_axxx32_a8_dstout_sse2;
  m->raster_argb32[1][CompositeDestOut].span_composite_a8[Image::FormatPRGB32] = raster_prgb32_span_composite_axxx32_a8_dstout_sse2;
  m->raster_argb32[1][CompositeDestOut].span_composite_a8[Image::FormatRGB32] = raster_prgb32_span_composite_xxxx_a8_dstout_sse2;
  m->raster_argb32[1][CompositeDestOut].span_composite_a8[Image::FormatRGB24] = raster_prgb32_span_composite_xxxx_a8_dstout_sse2;

  m->raster_argb32[1][CompositeSrcAtop].pixel = raster_prgb32_pixel_srcatop_sse2;
  m->raster_argb32[1][CompositeSrcAtop].pixel_a8 = raster_prgb32_pixel_a8_srcatop_sse2;
  m->raster_argb32[1][CompositeSrcAtop].span_solid = raster_prgb32_span_solid_srcatop_sse2;
  m->raster_argb32[1][CompositeSrcAtop].span_solid_a8 = raster_prgb32_span_solid_a8_srcatop_sse2;
  m->raster_argb32[1][CompositeSrcAtop].span_composite[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_srcatop_sse2;
  m->raster_argb32[1][CompositeSrcAtop].span_composite[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_srcatop_sse2;
  m->raster_argb32[1][CompositeSrcAtop].span_composite[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_srcatop_sse2;
  m->raster_argb32[1][CompositeSrcAtop].span_composite[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_srcatop_sse2;
  m->raster_argb32[1][CompositeSrcAtop].span_composite_a8[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_srcatop_sse2;
  m->raster_argb32[1][CompositeSrcAtop].span_composite_a8[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_srcatop_sse2;
  m->raster_argb32[1][CompositeSrcAtop].span_composite_a8[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_srcatop_sse2;
  m->raster_argb32[1][CompositeSrcAtop].span_composite_a8[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_a8_srcatop_sse2;

  m->raster_argb32[1][CompositeDestAtop].pixel = raster_prgb32_pixel_dstatop_sse2;
  m->raster_argb32[1][CompositeDestAtop].pixel_a8 = raster_prgb32_pixel_a8_dstatop_sse2;
  m->raster_argb32[1][CompositeDestAtop].span_solid = raster_prgb32_span_solid_dstatop_sse2;
  m->raster_argb32[1][CompositeDestAtop].span_solid_a8 = raster_prgb32_span_solid_a8_dstatop_sse2;
  m->raster_argb32[1][CompositeDestAtop].span_composite[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_dstatop_sse2;
  m->raster_argb32[1][CompositeDestAtop].span_composite[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_dstatop_sse2;
  m->raster_argb32[1][CompositeDestAtop].span_composite[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_dstatop_sse2;
  m->raster_argb32[1][CompositeDestAtop].span_composite[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_dstatop_sse2;
  m->raster_argb32[1][CompositeDestAtop].span_composite_a8[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_dstatop_sse2;
  m->raster_argb32[1][CompositeDestAtop].span_composite_a8[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_dstatop_sse2;
  m->raster_argb32[1][CompositeDestAtop].span_composite_a8[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_dstatop_sse2;
  m->raster_argb32[1][CompositeDestAtop].span_composite_a8[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_a8_dstatop_sse2;

  m->raster_argb32[1][CompositeXor].pixel = raster_prgb32_pixel_xor_sse2;
  m->raster_argb32[1][CompositeXor].pixel_a8 = raster_prgb32_pixel_a8_xor_sse2;
  m->raster_argb32[1][CompositeXor].span_solid = raster_prgb32_span_solid_xor_sse2;
  m->raster_argb32[1][CompositeXor].span_solid_a8 = raster_prgb32_span_solid_a8_xor_sse2;
  m->raster_argb32[1][CompositeXor].span_composite[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_xor_sse2;
  m->raster_argb32[1][CompositeXor].span_composite[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_xor_sse2;
  m->raster_argb32[1][CompositeXor].span_composite[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_xor_sse2;
  m->raster_argb32[1][CompositeXor].span_composite[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_xor_sse2;
  m->raster_argb32[1][CompositeXor].span_composite_a8[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_xor_sse2;
  m->raster_argb32[1][CompositeXor].span_composite_a8[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_xor_sse2;
  m->raster_argb32[1][CompositeXor].span_composite_a8[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_xor_sse2;
  m->raster_argb32[1][CompositeXor].span_composite_a8[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_a8_xor_sse2;

  m->raster_argb32[1][CompositeAdd].pixel = raster_prgb32_pixel_add_sse2;
  m->raster_argb32[1][CompositeAdd].pixel_a8 = raster_prgb32_pixel_a8_add_sse2;
  m->raster_argb32[1][CompositeAdd].span_solid = raster_prgb32_span_solid_add_sse2;
  m->raster_argb32[1][CompositeAdd].span_solid_a8 = raster_prgb32_span_solid_a8_add_sse2;
  m->raster_argb32[1][CompositeAdd].span_composite[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_add_sse2;
  m->raster_argb32[1][CompositeAdd].span_composite[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_add_sse2;
  m->raster_argb32[1][CompositeAdd].span_composite[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_add_sse2;
  m->raster_argb32[1][CompositeAdd].span_composite[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_add_sse2;
  m->raster_argb32[1][CompositeAdd].span_composite_a8[Image::FormatARGB32] = raster_prgb32_span_composite_argb32_a8_add_sse2;
  m->raster_argb32[1][CompositeAdd].span_composite_a8[Image::FormatPRGB32] = raster_prgb32_span_composite_prgb32_a8_add_sse2;
  m->raster_argb32[1][CompositeAdd].span_composite_a8[Image::FormatRGB32] = raster_prgb32_span_composite_rgb32_a8_add_sse2;
  m->raster_argb32[1][CompositeAdd].span_composite_a8[Image::FormatRGB24] = raster_prgb32_span_composite_rgb24_a8_add_sse2;

  // [Raster - Rgb32]

  m->raster_rgb32.pixel = raster_rgb32_pixel_sse2;
  m->raster_rgb32.pixel_a8 = raster_rgb32_pixel_a8_sse2;
  m->raster_rgb32.span_solid = raster_rgb32_span_solid_sse2;
  m->raster_rgb32.span_solid_a8 = raster_rgb32_span_solid_a8_sse2;
  m->raster_rgb32.span_composite[Image::FormatARGB32] = raster_rgb32_span_composite_argb32_sse2;
  m->raster_rgb32.span_composite[Image::FormatPRGB32] = raster_rgb32_span_composite_prgb32_sse2;
  m->raster_rgb32.span_composite[Image::FormatRGB32] = convert_memcpy32_sse2;
  m->raster_rgb32.span_composite[Image::FormatRGB24] = convert_rgb32_from_rgb24_sse2;
  m->raster_rgb32.span_composite_a8[Image::FormatARGB32] = raster_rgb32_span_composite_argb32_a8_sse2;
  m->raster_rgb32.span_composite_a8[Image::FormatPRGB32] = raster_rgb32_span_composite_prgb32_a8_sse2;
  m->raster_rgb32.span_composite_a8[Image::FormatRGB32] = raster_rgb32_span_composite_rgb32_a8_sse2;
  m->raster_rgb32.span_composite_a8[Image::FormatRGB24] = raster_rgb32_span_composite_rgb24_a8_sse2;
  m->raster_rgb32.span_composite_a8_const[Image::FormatARGB32] = raster_rgb32_span_composite_argb32_a8_const_sse2;
  m->raster_rgb32.span_composite_a8_const[Image::FormatPRGB32] = raster_rgb32_span_composite_prgb32_a8_const_sse2;
  m->raster_rgb32.span_composite_a8_const[Image::FormatRGB32] = raster_rgb32_span_composite_rgb32_a8_const_sse2;
  m->raster_rgb32.span_composite_a8_const[Image::FormatRGB24] = raster_rgb32_span_composite_rgb24_a8_const_sse2;
}
