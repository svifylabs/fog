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

//----------------------------------------------------------------------------
// Stack blur and recursive blur algorithms were ported from AntiGrain.
//----------------------------------------------------------------------------
//
// Anti-Grain Geometry - Version 2.4
// Copyright (C) 2002-2005 Maxim Shemanarev (http://www.antigrain.com)
//
// Permission to copy, use, modify, sell and distribute this software 
// is granted provided this copyright notice appears in all copies. 
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
//----------------------------------------------------------------------------
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://www.antigrain.com
//----------------------------------------------------------------------------
//
// The Stack Blur Algorithm was invented by Mario Klingemann, 
// mario@quasimondo.com and described here:
// http://incubator.quasimondo.com/processing/fast_blur_deluxe.php
// (search phrase "Stackblur: Fast But Goodlooking"). 
// The major improvement is that there's no more division table
// that was very expensive to create for large blur radii. Insted, 
// for 8-bit per channel and radius not exceeding 254 the division is 
// replaced by multiplication and shift. 
//
//----------------------------------------------------------------------------

namespace Fog {
namespace Raster {

// ============================================================================
// [Fog::Raster_SSE2 - HelpersMatrix]
// ============================================================================

// Get reciprocal for 16-bit value @a val.
static FOG_INLINE int getReciprocal(int val)
{
  return (65536 + val - 1) / val;
}

// ============================================================================
// [Fog::Raster_SSE2 - ColorMatrix]
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
    dst[0] = (uint8_t)_mm_cvtsi128_si32(pix);
  }
}

// ============================================================================
// [Fog::Raster_C - FloatScanlineConvolve]
// ============================================================================

static void FOG_FASTCALL floatScanlineConvolveH_argb32_sse2(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, const float* kernel, int size, float divisor,
  int borderMode, uint32_t borderColor)
{
  if (size == 0 || width < 2)
  {
    if (dst != src) functionMap->filter.copyArea[Image::FormatARGB32](dst, dstStride, src, srcStride, width, height);
    return;
  }

  sysint_t dym1 = width;
  sysint_t dym2 = height;
  sysint_t max = dym1 - 1;
  sysint_t end = max * 4;

  uint8_t* dstCur;
  const uint8_t* srcCur;

  sysint_t pos1;
  sysint_t pos2;
  sysint_t xp, i;

  int sizeHalf = size >> 1;

  MemoryBuffer<512*sizeof(float)> stackBuffer;
  uint32_t* stack = (uint32_t*)stackBuffer.alloc(size * (sizeof(uint32_t) + sizeof(float)));
  uint32_t* stackEnd = stack + size;
  uint32_t* stackCur;

  if (!stack) return;

  uint32_t lBorderColor = borderColor;
  uint32_t rBorderColor = borderColor;

  // If divisor is not 1.0, we will modify a given kernel to be.
  if (divisor != 1.0f)
  {
    float* k = (float*)( (uint8_t*)stack + size * sizeof(float) );
    Math::vdivs(k, kernel, divisor, size);
    kernel = k;
  }

  __m128 const1 = _mm_set1_ps(1.0f);
  __m128 const0 = _mm_set1_ps(0.0f);
  __m128 const255 = _mm_set1_ps(255.0f);

  for (pos2 = 0; pos2 < dym2; pos2++)
  {
    uint32_t pix0;

    dstCur = dst;
    srcCur = src;
    stackCur = stack;

    if (borderMode == ImageFilter::BorderModeExtend)
    {
      lBorderColor = READ_32(srcCur);
      rBorderColor = READ_32(srcCur + end);
    }

    pix0 = lBorderColor;
    xp = 0;

    for (i = 0; i < sizeHalf; i++)
    {
      stackCur[0] = pix0;
      stackCur++;
    }

    for (i = sizeHalf; i < size; i++)
    {
      if (xp < dym1)
      {
        pix0 = READ_32(srcCur);
        srcCur += 4;
        xp++;
      }
      else
      {
        pix0 = rBorderColor;
      }

      stackCur[0] = pix0;
      stackCur++;
    }

    stackCur = stack;

    for (pos1 = 0; pos1 < dym1; pos1++)
    {
      __m128i pixi;
      __m128 z = const1;

      for (i = 0; i < size; i++)
      {
        __m128 k = _mm_set1_ps(kernel[i]);
        __m128 pixf;

        pix_load4(pixi, (uint8_t*)stackCur);
        pix_unpack_to_float(pixf, pixi);

        pixf = _mm_mul_ps(pixf, k);
        z = _mm_add_ps(z, pixf);

        stackCur++;
        if (stackCur == stackEnd) stackCur = stack;
      }

      z = _mm_max_ps(z, const0);
      z = _mm_min_ps(z, const255);

      pix_pack_from_float(pixi, z);
      pix_store4(dstCur, pixi);
      dstCur += 4;

      if (xp < dym1)
      {
        pix_load4(pixi, srcCur);
        pix_store4((uint8_t*)stackCur, pixi);
        srcCur += 4;
        xp++;
      }
      else
      {
        stackCur[0] = rBorderColor;
      }

      stackCur++;
      if (stackCur == stackEnd) stackCur = stack;
    }

    src += srcStride;
    dst += dstStride;
  }
}

static void FOG_FASTCALL floatScanlineConvolveV_argb32_sse2(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, const float* kernel, int size, float divisor,
  int borderMode, uint32_t borderColor)
{
  if (size == 0 || height < 2)
  {
    if (dst != src) functionMap->filter.copyArea[Image::FormatARGB32](dst, dstStride, src, srcStride, width, height);
    return;
  }

  sysint_t dym1 = height;
  sysint_t dym2 = width;
  sysint_t max = dym1 - 1;
  sysint_t end = max * srcStride;

  uint8_t* dstCur;
  const uint8_t* srcCur;

  sysint_t pos1;
  sysint_t pos2;
  sysint_t xp, i;

  int sizeHalf = size >> 1;

  MemoryBuffer<512*sizeof(float)> stackBuffer;
  uint32_t* stack = (uint32_t*)stackBuffer.alloc(size * (sizeof(uint32_t) + sizeof(float)));
  uint32_t* stackEnd = stack + size;
  uint32_t* stackCur;

  if (!stack) return;

  uint32_t lBorderColor = borderColor;
  uint32_t rBorderColor = borderColor;

  // If divisor is not 1.0, we will modify a given kernel to be.
  if (divisor != 1.0f)
  {
    float* k = (float*)( (uint8_t*)stack + size * sizeof(uint32_t) );
    Math::vdivs(k, kernel, divisor, size);
    kernel = k;
  }

  __m128 const1 = _mm_set1_ps(1.0f);
  __m128 const0 = _mm_set1_ps(0.0f);
  __m128 const255 = _mm_set1_ps(255.0f);

  for (pos2 = 0; pos2 < dym2; pos2++)
  {
    uint32_t pix0;

    dstCur = dst;
    srcCur = src;
    stackCur = stack;

    if (borderMode == ImageFilter::BorderModeExtend)
    {
      lBorderColor = READ_32(srcCur);
      rBorderColor = READ_32(srcCur + end);
    }

    pix0 = lBorderColor;
    xp = 0;

    for (i = 0; i < sizeHalf; i++)
    {
      stackCur[0] = pix0;
      stackCur++;
    }

    for (i = sizeHalf; i < size; i++)
    {
      if (xp < dym1)
      {
        pix0 = READ_32(srcCur);
        srcCur += srcStride;
        xp++;
      }
      else
      {
        pix0 = rBorderColor;
      }

      stackCur[0] = pix0;
      stackCur++;
    }

    stackCur = stack;

    for (pos1 = 0; pos1 < dym1; pos1++)
    {
      __m128i pixi;
      __m128 z = const1;

      for (i = 0; i < size; i++)
      {
        __m128 k = _mm_set1_ps(kernel[i]);
        __m128 pixf;

        pix_load4(pixi, (uint8_t*)stackCur);
        pix_unpack_to_float(pixf, pixi);

        pixf = _mm_mul_ps(pixf, k);
        z = _mm_add_ps(z, pixf);

        stackCur++;
        if (stackCur == stackEnd) stackCur = stack;
      }

      z = _mm_max_ps(z, const0);
      z = _mm_min_ps(z, const255);

      pix_pack_from_float(pixi, z);
      pix_store4(dstCur, pixi);
      dstCur += dstStride;

      if (xp < dym1)
      {
        pix_load4(pixi, srcCur);
        pix_store4((uint8_t*)stackCur, pixi);
        srcCur += srcStride;
        xp++;
      }
      else
      {
        stackCur[0] = rBorderColor;
      }

      stackCur++;
      if (stackCur == stackEnd) stackCur = stack;
    }

    src += 4;
    dst += 4;
  }
}

// ============================================================================
// [Fog::Raster_SSE2 - BoxBlur]
// ============================================================================

static void FOG_FASTCALL boxBlurConvolveH_argb32_sse2(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int radius,
  int borderMode, uint32_t borderColor)
{
  if (radius == 0 || width < 2)
  {
    if (dst != src) functionMap->filter.copyArea[Image::FormatARGB32](dst, dstStride, src, srcStride, width, height);
    return;
  }

  if (radius > 254) radius = 254;

  sysint_t dym1 = width;
  sysint_t dym2 = height;
  sysint_t max = dym1 - 1;
  sysint_t end = max * 4;

  uint8_t* dstCur;
  const uint8_t* srcCur;

  sysint_t pos1;
  sysint_t pos2;
  sysint_t xp, i;

  uint size = (uint)radius * 2 + 1;

  __m128i mmMul = _mm_cvtsi32_si128(getReciprocal(size));
  __m128i mmShr = _mm_cvtsi32_si128(16);
  pix_expand_pixel_1x4B(mmMul, mmMul);

  uint32_t stack[512];
  uint32_t* stackEnd = stack + size;
  uint32_t* stackCur;

  uint32_t lBorderColor = borderColor;
  uint32_t rBorderColor = borderColor;

  for (pos2 = 0; pos2 < dym2; pos2++)
  {
    uint32_t pix32;

    __m128i pix;
    __m128i sum = _mm_setzero_si128();

    srcCur = src;

    if (borderMode == ImageFilter::BorderModeExtend)
    {
      lBorderColor = READ_32(srcCur);
      rBorderColor = READ_32(srcCur + end);
    }

    pix32 = lBorderColor;
    pix_unpack_1x1D(pix, pix32);

    stackCur = stack;

    for (i = 0; i < radius; i++)
    {
      stackCur[0] = pix32;
      stackCur++;

      sum = _mm_add_epi32(sum, pix);
    }

    pix32 = READ_32(srcCur);
    stackCur[0] = pix32;
    stackCur++;

    pix_unpack_1x1D(pix, pix32);
    sum = _mm_add_epi32(sum, pix);

    for (i = 1; i <= radius; i++)
    {
      if (i <= max)
      {
        srcCur += 4;
        pix32 = READ_32(srcCur);
      }
      else 
      {
        pix32 = rBorderColor;
      }

      stackCur[0] = pix32;
      stackCur++;

      pix_unpack_1x1D(pix, pix32);
      sum = _mm_add_epi32(sum, pix);
    }

    xp = Math::min((sysint_t)radius, max);

    srcCur = src + xp * 4;
    dstCur = dst;

    stackCur = stack;

    for (pos1 = 0; pos1 < dym1; pos1++)
    {
      sse2_mul_const_4D(pix, sum, mmMul);
      pix = _mm_srl_epi32(pix, mmShr);
      pix_pack_1x1D(pix, pix);
      pix_store4(dstCur, pix);

      dstCur += 4;

      pix32 = stackCur[0];

      pix_unpack_1x1D(pix, pix32);
      sum = _mm_sub_epi32(sum, pix);

      if (xp < max)
      {
        ++xp;
        srcCur += 4;
        pix32 = READ_32(srcCur);
      }
      else
      {
        pix32 = rBorderColor;
      }

      stackCur[0] = pix32;

      pix_unpack_1x1D(pix, pix32);
      sum = _mm_add_epi32(sum, pix);

      stackCur += 1;
      if (stackCur == stackEnd) stackCur = stack;
    }

    src += srcStride;
    dst += dstStride;
  }
}

static void FOG_FASTCALL boxBlurConvolveV_argb32_sse2(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int radius,
  int borderMode, uint32_t borderColor)
{
  if (radius == 0 || height < 2)
  {
    if (dst != src) functionMap->filter.copyArea[Image::FormatARGB32](dst, dstStride, src, srcStride, width, height);
    return;
  }

  if (radius > 254) radius = 254;

  sysint_t dym1 = height;
  sysint_t dym2 = width;
  sysint_t max = dym1 - 1;
  sysint_t end = max * srcStride;

  uint8_t* dstCur;
  const uint8_t* srcCur;

  sysint_t pos1;
  sysint_t pos2;
  sysint_t xp, i;

  uint size = (uint)radius * 2 + 1;

  __m128i mmMul = _mm_cvtsi32_si128(getReciprocal(size));
  __m128i mmShr = _mm_cvtsi32_si128(16);
  pix_expand_pixel_1x4B(mmMul, mmMul);

  uint32_t stack[512];
  uint32_t* stackEnd = stack + size;
  uint32_t* stackCur;

  uint32_t lBorderColor = borderColor;
  uint32_t rBorderColor = borderColor;

  for (pos2 = 0; pos2 < dym2; pos2++)
  {
    uint32_t pix32;

    __m128i pix;
    __m128i sum = _mm_setzero_si128();

    srcCur = src;

    if (borderMode == ImageFilter::BorderModeExtend)
    {
      lBorderColor = READ_32(srcCur);
      rBorderColor = READ_32(srcCur + end);
    }

    pix32 = lBorderColor;
    pix_unpack_1x1D(pix, pix32);

    stackCur = stack;

    for (i = 0; i < radius; i++)
    {
      stackCur[0] = pix32;
      stackCur++;

      sum = _mm_add_epi32(sum, pix);
    }

    pix32 = READ_32(srcCur);
    stackCur[0] = pix32;
    stackCur++;

    pix_unpack_1x1D(pix, pix32);
    sum = _mm_add_epi32(sum, pix);

    for (i = 1; i <= radius; i++)
    {
      if (i <= max)
      {
        srcCur += srcStride;
        pix32 = READ_32(srcCur);
      }
      else 
      {
        pix32 = rBorderColor;
      }

      stackCur[0] = pix32;
      stackCur++;

      pix_unpack_1x1D(pix, pix32);
      sum = _mm_add_epi32(sum, pix);
    }

    xp = Math::min((sysint_t)radius, max);

    srcCur = src + xp * srcStride;
    dstCur = dst;

    stackCur = stack;

    for (pos1 = 0; pos1 < dym1; pos1++)
    {
      sse2_mul_const_4D(pix, sum, mmMul);
      pix = _mm_srl_epi32(pix, mmShr);
      pix_pack_1x1D(pix, pix);
      pix_store4(dstCur, pix);

      dstCur += dstStride;

      pix32 = stackCur[0];

      pix_unpack_1x1D(pix, pix32);
      sum = _mm_sub_epi32(sum, pix);

      if (xp < max)
      {
        ++xp;
        srcCur += srcStride;
        pix32 = READ_32(srcCur);
      }
      else
      {
        pix32 = rBorderColor;
      }

      stackCur[0] = pix32;

      pix_unpack_1x1D(pix, pix32);
      sum = _mm_add_epi32(sum, pix);

      stackCur += 1;
      if (stackCur == stackEnd) stackCur = stack;
    }

    src += 4;
    dst += 4;
  }
}

// ============================================================================
// [Fog::Raster_SSE2 - StackBlur]
// ============================================================================

static void FOG_FASTCALL stackBlurConvolveH_argb32_sse2(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int radius,
  int borderMode, uint32_t borderColor)
{
  if (radius == 0 || width < 2)
  {
    if (dst != src) functionMap->filter.copyArea[Image::FormatARGB32](dst, dstStride, src, srcStride, width, height);
    return;
  }

  if (radius > 254) radius = 254;

  sysint_t dym1 = width;
  sysint_t dym2 = height;
  sysint_t max = dym1 - 1;
  sysint_t end = max * 4;

  uint8_t* dstCur;
  const uint8_t* srcCur;

  sysint_t pos1;
  sysint_t pos2;
  sysint_t xp, i;

  __m128i mmMul = _mm_cvtsi32_si128(functionMap->filter.stackBlur8Mul[radius]);
  __m128i mmShr = _mm_cvtsi32_si128(functionMap->filter.stackBlur8Shr[radius]);
  pix_expand_pixel_1x4B(mmMul, mmMul);

  uint32_t stack[512];
  uint32_t* stackEnd = stack + (radius * 2 + 1);
  uint32_t* stackLeft;
  uint32_t* stackRight;

  uint32_t lBorderColor = borderColor;
  uint32_t rBorderColor = borderColor;

  for (pos2 = 0; pos2 < dym2; pos2++)
  {
    uint32_t pix32;

    __m128i pix;
    __m128i sum    = _mm_setzero_si128();
    __m128i sumIn  = _mm_setzero_si128();
    __m128i sumOut = _mm_setzero_si128();
    __m128i mi;
    __m128i m1;

    srcCur = src;

    if (borderMode == ImageFilter::BorderModeExtend)
    {
      lBorderColor = READ_32(srcCur);
      rBorderColor = READ_32(srcCur + end);
    }

    pix32 = lBorderColor;
    pix_unpack_1x1D(pix, pix32);
    mi = pix;

    stackLeft = stack;

    for (i = 0; i < radius; i++)
    {
      stackLeft[0] = pix32;
      stackLeft++;

      sum    = _mm_add_epi32(sum   , mi);
      sumOut = _mm_add_epi32(sumOut, pix);
      mi     = _mm_add_epi32(mi    , pix);
    }

    pix32 = READ_32(srcCur);
    pix_unpack_1x1D(pix, pix32);

    stackLeft[0] = pix32;
    stackLeft++;

    pix_expand_mask_1x1D(mi, radius + 1);
    pix_expand_mask_1x1D(m1, 1);

    sumOut = _mm_add_epi32(sumOut, pix);

    sse2_mul_const_4D(pix, pix, mi);
    sum = _mm_add_epi32(sum, pix);

    for (i = 1; i <= radius; i++)
    {
      if (i <= max)
      {
        srcCur += 4;
        pix32 = READ_32(srcCur);
      }
      else 
      {
        pix32 = rBorderColor;
      }

      stackLeft[0] = pix32;
      stackLeft++;

      pix_unpack_1x1D(pix, pix32);

      mi = _mm_sub_epi32(mi, m1);
      sumIn = _mm_add_epi32(sumIn, pix);
      pix = _mm_mullo_epi16(pix, mi);
      sum = _mm_add_epi32(sum, pix);
    }

    xp = Math::min((sysint_t)radius, max);

    srcCur = src + xp * 4;
    dstCur = dst;

    stackLeft = stack;
    stackRight = stack + radius;

    for (pos1 = 0; pos1 < dym1; pos1++)
    {
      sse2_mul_const_4D(pix, sum, mmMul);
      pix = _mm_srl_epi32(pix, mmShr);
      pix_pack_1x1D(pix, pix);
      pix_store4(dstCur, pix);

      dstCur += 4;

      sum = _mm_sub_epi32(sum, sumOut);

      pix_unpack_1x1D(pix, stackLeft[0]);
      sumOut = _mm_sub_epi32(sumOut, pix);

      if (xp < max)
      {
        ++xp;
        srcCur += 4;
        pix32 = READ_32(srcCur);
      }
      else
      {
        pix32 = rBorderColor;
      }

      stackLeft[0] = pix32;
      pix_unpack_1x1D(pix, pix32);

      sumIn = _mm_add_epi32(sumIn, pix);
      sum = _mm_add_epi32(sum, sumIn);

      stackLeft += 1;
      stackRight += 1;

      if (stackLeft == stackEnd) stackLeft = stack;
      if (stackRight == stackEnd) stackRight = stack;

      pix_unpack_1x1D(pix, stackRight[0]);

      sumOut = _mm_add_epi32(sumOut, pix);
      sumIn = _mm_sub_epi32(sumIn, pix);
    }

    src += srcStride;
    dst += dstStride;
  }
}

static void FOG_FASTCALL stackBlurConvolveV_argb32_sse2(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int radius,
  int borderMode, uint32_t borderColor)
{
  if (radius == 0 || height < 2)
  {
    if (dst != src) functionMap->filter.copyArea[Image::FormatARGB32](dst, dstStride, src, srcStride, width, height);
    return;
  }

  if (radius > 254) radius = 254;

  sysint_t dym1 = height;
  sysint_t dym2 = width;
  sysint_t max = dym1 - 1;
  sysint_t end = max * srcStride;

  uint8_t* dstCur;
  const uint8_t* srcCur;

  sysint_t pos1;
  sysint_t pos2;
  sysint_t xp, i;

  __m128i mmMul = _mm_cvtsi32_si128(functionMap->filter.stackBlur8Mul[radius]);
  __m128i mmShr = _mm_cvtsi32_si128(functionMap->filter.stackBlur8Shr[radius]);
  pix_expand_pixel_1x4B(mmMul, mmMul);

  uint32_t stack[512];
  uint32_t* stackEnd = stack + (radius * 2 + 1);
  uint32_t* stackLeft;
  uint32_t* stackRight;

  uint32_t lBorderColor = borderColor;
  uint32_t rBorderColor = borderColor;

  for (pos2 = 0; pos2 < dym2; pos2++)
  {
    uint32_t pix32;

    __m128i pix;
    __m128i sum    = _mm_setzero_si128();
    __m128i sumIn  = _mm_setzero_si128();
    __m128i sumOut = _mm_setzero_si128();
    __m128i mi;
    __m128i m1;

    srcCur = src;

    if (borderMode == ImageFilter::BorderModeExtend)
    {
      lBorderColor = READ_32(srcCur);
      rBorderColor = READ_32(srcCur + end);
    }

    pix32 = lBorderColor;
    pix_unpack_1x1D(pix, pix32);
    mi = pix;

    stackLeft = stack;

    for (i = 0; i < radius; i++)
    {
      stackLeft[0] = pix32;
      stackLeft++;

      sum    = _mm_add_epi32(sum   , mi);
      sumOut = _mm_add_epi32(sumOut, pix);
      mi     = _mm_add_epi32(mi    , pix);
    }

    pix32 = READ_32(srcCur);
    pix_unpack_1x1D(pix, pix32);

    stackLeft[0] = pix32;
    stackLeft++;

    pix_expand_mask_1x1D(mi, radius + 1);
    pix_expand_mask_1x1D(m1, 1);

    sumOut = _mm_add_epi32(sumOut, pix);

    sse2_mul_const_4D(pix, pix, mi);
    sum = _mm_add_epi32(sum, pix);

    for (i = 1; i <= radius; i++)
    {
      if (i <= max)
      {
        srcCur += srcStride;
        pix32 = READ_32(srcCur);
      }
      else 
      {
        pix32 = rBorderColor;
      }

      stackLeft[0] = pix32;
      stackLeft++;

      pix_unpack_1x1D(pix, pix32);

      mi = _mm_sub_epi32(mi, m1);
      sumIn = _mm_add_epi32(sumIn, pix);
      pix = _mm_mullo_epi16(pix, mi);
      sum = _mm_add_epi32(sum, pix);
    }

    xp = Math::min((sysint_t)radius, max);

    srcCur = src + xp * srcStride;
    dstCur = dst;

    stackLeft = stack;
    stackRight = stack + radius;

    for (pos1 = 0; pos1 < dym1; pos1++)
    {
      sse2_mul_const_4D(pix, sum, mmMul);
      pix = _mm_srl_epi32(pix, mmShr);
      pix_pack_1x1D(pix, pix);
      pix_store4(dstCur, pix);

      dstCur += dstStride;

      sum = _mm_sub_epi32(sum, sumOut);

      pix_unpack_1x1D(pix, stackLeft[0]);
      sumOut = _mm_sub_epi32(sumOut, pix);

      if (xp < max)
      {
        ++xp;
        srcCur += srcStride;
        pix32 = READ_32(srcCur);
      }
      else
      {
        pix32 = rBorderColor;
      }

      stackLeft[0] = pix32;
      pix_unpack_1x1D(pix, pix32);

      sumIn = _mm_add_epi32(sumIn, pix);
      sum = _mm_add_epi32(sum, sumIn);

      stackLeft += 1;
      stackRight += 1;

      if (stackLeft == stackEnd) stackLeft = stack;
      if (stackRight == stackEnd) stackRight = stack;

      pix_unpack_1x1D(pix, stackRight[0]);

      sumOut = _mm_add_epi32(sumOut, pix);
      sumIn = _mm_sub_epi32(sumIn, pix);
    }

    src += 4;
    dst += 4;
  }
}

} // Raster namespace
} // Fog namespace
