// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// For some IDEs to enable code-assist.
#include <Fog/Build/Build.h>
#if defined(FOG_IDE)
#include <Fog/Graphics/Raster/Raster_SSE2_base.cpp.h>
#include <Fog/Graphics/Raster/Raster_SSE2_convert.cpp.h>
#include <Fog/Graphics/Raster/Raster_SSE2_composite.cpp.h>
#endif // FOG_IDE

namespace Fog {
namespace Raster {

// ============================================================================
// [Fog::Raster_C - ColorMatrix]
// ============================================================================

static FOG_INLINE __m128 _loadColorMatrixRow(const double* src)
{
  __m128 t0 = _mm_cvtpd_ps(_mm_loadu_pd(src));
  __m128 t1 = _mm_cvtpd_ps(_mm_loadu_pd(src + 2));

  t0 = _mm_shuffle_ps(t0, t1, _MM_SHUFFLE(1, 0, 1, 0));
  t0 = _mm_shuffle_ps(t0, t0, _MM_SHUFFLE(3, 0, 1, 2));
  return t0;
}

static void FOG_FASTCALL colorMatrix_argb32_sse2(
  uint8_t* dst, const uint8_t* src,
  const ColorMatrix* cm, uint32_t type, sysint_t width)
{
  __m128 cf0 = _mm_set1_ps(0.0f);
  __m128 cf255 = _mm_set1_ps(255.0f);

  __m128 pr = _loadColorMatrixRow(cm->m[0]);
  __m128 pg = _loadColorMatrixRow(cm->m[1]);
  __m128 pb = _loadColorMatrixRow(cm->m[2]);
  __m128 pa = _loadColorMatrixRow(cm->m[3]);
  __m128 pt = _mm_mul_ps(_loadColorMatrixRow(cm->m[4]), cf255);

  for (sysint_t i = width; i; i--, dst += 4, src += 4)
  {
    __m128i pix;
    __m128 pixf;
    __m128 pixt;
    __m128 result = pt;

    pix_load4(pix, src);
    pix_unpack_to_float(pixf, pix);

    pixt = _mm_shuffle_ps(pixf, pixf, _MM_SHUFFLE(2, 2, 2, 2));
    pixt = _mm_mul_ps(pixt, pr);
    result = _mm_add_ps(result, pixt);

    pixt = _mm_shuffle_ps(pixf, pixf, _MM_SHUFFLE(1, 1, 1, 1));
    pixt = _mm_mul_ps(pixt, pg);
    result = _mm_add_ps(result, pixt);

    pixt = _mm_shuffle_ps(pixf, pixf, _MM_SHUFFLE(0, 0, 0, 0));
    pixt = _mm_mul_ps(pixt, pb);
    result = _mm_add_ps(result, pixt);

    pixt = _mm_shuffle_ps(pixf, pixf, _MM_SHUFFLE(3, 3, 3, 3));
    pixt = _mm_mul_ps(pixt, pa);
    result = _mm_add_ps(result, pixt);

    result = _mm_max_ps(result, cf0);
    result = _mm_min_ps(result, cf255);

    pix_pack_from_float(pix, result);
    pix_store4(dst, pix);
  }
}

static void FOG_FASTCALL colorMatrix_rgb32_sse2(
  uint8_t* dst, const uint8_t* src,
  const ColorMatrix* cm, uint32_t type, sysint_t width)
{
  __m128 cf0 = _mm_set_ps(255.0f, 0.0f, 0.0f, 0.0f);
  __m128 cf255 = _mm_set1_ps(255.0f);

  __m128 pr = _loadColorMatrixRow(cm->m[0]);
  __m128 pg = _loadColorMatrixRow(cm->m[1]);
  __m128 pb = _loadColorMatrixRow(cm->m[2]);
  __m128 pa = _loadColorMatrixRow(cm->m[3]);
  __m128 pt = _mm_mul_ps(_loadColorMatrixRow(cm->m[4]), cf255);

  pt = _mm_add_ps(pt, _mm_mul_ps(pa, cf255));

  for (sysint_t i = width; i; i--, dst += 4, src += 4)
  {
    __m128i pix;
    __m128 pixf;
    __m128 pixt;
    __m128 result = pt;

    pix_load4(pix, src);
    pix = _mm_or_si128(pix, MaskFF000000FF000000);
    pix_unpack_to_float(pixf, pix);

    pixt = _mm_shuffle_ps(pixf, pixf, _MM_SHUFFLE(2, 2, 2, 2));
    pixt = _mm_mul_ps(pixt, pr);
    result = _mm_add_ps(result, pixt);

    pixt = _mm_shuffle_ps(pixf, pixf, _MM_SHUFFLE(1, 1, 1, 1));
    pixt = _mm_mul_ps(pixt, pg);
    result = _mm_add_ps(result, pixt);

    pixt = _mm_shuffle_ps(pixf, pixf, _MM_SHUFFLE(0, 0, 0, 0));
    pixt = _mm_mul_ps(pixt, pb);
    result = _mm_add_ps(result, pixt);

    result = _mm_max_ps(result, cf0);
    result = _mm_min_ps(result, cf255);

    pix_pack_from_float(pix, result);
    pix_store4(dst, pix);
  }
}

static void FOG_FASTCALL colorMatrix_rgb24_sse2(
  uint8_t* dst, const uint8_t* src,
  const ColorMatrix* cm, uint32_t type, sysint_t width)
{
  __m128 cf0 = _mm_set_ps(255.0f, 0.0f, 0.0f, 0.0f);
  __m128 cf255 = _mm_set1_ps(255.0f);

  __m128 pr = _loadColorMatrixRow(cm->m[0]);
  __m128 pg = _loadColorMatrixRow(cm->m[1]);
  __m128 pb = _loadColorMatrixRow(cm->m[2]);
  __m128 pa = _loadColorMatrixRow(cm->m[3]);
  __m128 pt = _mm_mul_ps(_loadColorMatrixRow(cm->m[4]), cf255);

  pt = _mm_add_ps(pt, _mm_mul_ps(pa, cf255));

  for (sysint_t i = width; i; i--, dst += 3, src += 3)
  {
    __m128i pix;
    __m128 pixf;
    __m128 pixt;
    __m128 result = pt;

    pix = _mm_cvtsi32_si128(PixFmt_RGB24::fetch(dst));
    pix = _mm_or_si128(pix, MaskFF000000FF000000);
    pix_unpack_to_float(pixf, pix);

    pixt = _mm_shuffle_ps(pixf, pixf, _MM_SHUFFLE(2, 2, 2, 2));
    pixt = _mm_mul_ps(pixt, pr);
    result = _mm_add_ps(result, pixt);

    pixt = _mm_shuffle_ps(pixf, pixf, _MM_SHUFFLE(1, 1, 1, 1));
    pixt = _mm_mul_ps(pixt, pg);
    result = _mm_add_ps(result, pixt);

    pixt = _mm_shuffle_ps(pixf, pixf, _MM_SHUFFLE(0, 0, 0, 0));
    pixt = _mm_mul_ps(pixt, pb);
    result = _mm_add_ps(result, pixt);

    result = _mm_max_ps(result, cf0);
    result = _mm_min_ps(result, cf255);

    pix_pack_from_float(pix, result);
    PixFmt_RGB24::store(dst, _mm_cvtsi128_si32(pix));
  }
}

static void FOG_FASTCALL colorMatrix_a8_sse2(
  uint8_t* dst, const uint8_t* src,
  const ColorMatrix* cm, uint32_t type, sysint_t width)
{
  sysint_t i;

  __m128 cf0 = _mm_set_ps(0.0f, 0.0f, 0.0f, 0.0f);
  __m128 cf255 = _mm_set1_ps(255.0f);

  __m128 pa = _mm_set1_ps((float)(cm->m[3][3]));
  __m128 pt = _mm_set1_ps((float)(cm->m[4][3]));

  pt = _mm_mul_ps(pa, cf255);

  for (i = width >> 2; i; i--, dst += 4, src += 4)
  {
    __m128i pix;
    __m128 pixf;

    pix_load4(pix, src);
    pix_unpack_to_float(pixf, pix);

    pixf = _mm_mul_ps(pixf, pa);
    pixf = _mm_add_ps(pixf, pt);

    pixf = _mm_max_ps(pixf, cf0);
    pixf = _mm_min_ps(pixf, cf255);

    pix_pack_from_float(pix, pixf);
    pix_store4(dst, pix);
  }

  for (i = width & 3; i; i--, dst += 1, src += 1)
  {
    __m128i pix;
    __m128 pixf;

    pix = _mm_cvtsi32_si128(src[0]);
    pix_unpack_to_float(pixf, pix);

    pixf = _mm_mul_ss(pixf, pa);
    pixf = _mm_add_ss(pixf, pt);

    pixf = _mm_max_ss(pixf, cf0);
    pixf = _mm_min_ss(pixf, cf255);

    pix_pack_from_float(pix, pixf);
    dst[0] = _mm_cvtsi128_si32(pix);
  }
}

} // Raster namespace
} // Fog namespace
