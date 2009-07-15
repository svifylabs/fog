// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// For some IDEs to enable code-assist.
#include <Fog/Build/Build.h>
#if defined(FOG_IDE)
#include <Fog/Graphics/Raster/Raster_C_base.cpp.h>
#include <Fog/Graphics/Raster/Raster_C_convert.cpp.h>
#include <Fog/Graphics/Raster/Raster_C_composite.cpp.h>
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
// [Fog::Raster_C::helpers]
// ============================================================================

static FOG_INLINE int clamp255(int val)
{
  return (val > 255) ? 255 : val < 0 ? 0 : val;
}

// Get reciprocal for 16-bit value @a val.
static FOG_INLINE int getReciprocal(int val)
{
  return (65536 + val - 1) / val;
}

// ============================================================================
// [Fog::Raster_C - CopyArea]
// ============================================================================

static void copyArea_32(
  uint8_t* dst, sysint_t dstStride, 
  const uint8_t* src, sysint_t srcStride,
  int width, int height)
{
  sysint_t y;
  sysint_t size = width * 4;

  for (y = height; y; y--, dst += dstStride, src += srcStride)
  {
    memcpy(dst, src, size);
  }
}

static void copyArea_24(
  uint8_t* dst, sysint_t dstStride, 
  const uint8_t* src, sysint_t srcStride,
  int width, int height)
{
  sysint_t y;
  sysint_t size = width * 3;

  for (y = height; y; y--, dst += dstStride, src += srcStride)
  {
    memcpy(dst, src, size);
  }
}

static void copyArea_8(
  uint8_t* dst, sysint_t dstStride, 
  const uint8_t* src, sysint_t srcStride,
  int width, int height)
{
  sysint_t y;
  sysint_t size = width;

  for (y = height; y; y--, dst += dstStride, src += srcStride)
  {
    memcpy(dst, src, size);
  }
}

// ============================================================================
// [Fog::Raster_C - ColorLut]
// ============================================================================

static void FOG_FASTCALL colorLut_prgb32(
  uint8_t* dst, const uint8_t* src, const ColorLut::Table* lut, sysint_t width)
{
  functionMap->convert.argb32_from_prgb32(dst, src, width);
  functionMap->filters.colorLut[Image::FormatARGB32](dst, dst, lut, width);
  functionMap->convert.prgb32_from_argb32(dst, dst, width);
}

static void FOG_FASTCALL colorLut_argb32(
  uint8_t* dst, const uint8_t* src, const ColorLut::Table* lut, sysint_t width)
{
  const uint8_t* rLut = lut->r;
  const uint8_t* gLut = lut->g;
  const uint8_t* bLut = lut->b;
  const uint8_t* aLut = lut->a;

  for (sysint_t i = width; i; i--, dst += 4, src += 4)
  {
    dst[RGB32_RByte] = rLut[src[RGB32_RByte]];
    dst[RGB32_GByte] = gLut[src[RGB32_GByte]];
    dst[RGB32_BByte] = bLut[src[RGB32_BByte]];
    dst[RGB32_AByte] = aLut[src[RGB32_AByte]];
  }
}

static void FOG_FASTCALL colorLut_rgb32(
  uint8_t* dst, const uint8_t* src, const ColorLut::Table* lut, sysint_t width)
{
  const uint8_t* rLut = lut->r;
  const uint8_t* gLut = lut->g;
  const uint8_t* bLut = lut->b;

  for (sysint_t i = width; i; i--, dst += 4, src += 4)
  {
    dst[RGB32_RByte] = rLut[src[RGB32_RByte]];
    dst[RGB32_GByte] = gLut[src[RGB32_GByte]];
    dst[RGB32_BByte] = bLut[src[RGB32_BByte]];
    dst[RGB32_AByte] = src[RGB32_AByte];
  }
}

static void FOG_FASTCALL colorLut_rgb24(
  uint8_t* dst, const uint8_t* src, const ColorLut::Table* lut, sysint_t width)
{
  const uint8_t* rLut = lut->r;
  const uint8_t* gLut = lut->g;
  const uint8_t* bLut = lut->b;

  for (sysint_t i = width; i; i--, dst += 3, src += 3)
  {
    dst[RGB24_RByte] = rLut[src[RGB24_RByte]];
    dst[RGB24_GByte] = gLut[src[RGB24_GByte]];
    dst[RGB24_BByte] = bLut[src[RGB24_BByte]];
  }
}

static void FOG_FASTCALL colorLut_a8(
  uint8_t* dst, const uint8_t* src, const ColorLut::Table* lut, sysint_t width)
{
  const uint8_t* aLut = lut->a;
  for (sysint_t i = width; i; i--, dst += 1, src += 1) dst[0] = aLut[src[0]];
}

// ============================================================================
// [Fog::Raster_C - ColorMatrix]
// ============================================================================

static void FOG_FASTCALL colorMatrix_prgb32(
  uint8_t* dst, const uint8_t* src,
  const ColorMatrix* cm, uint32_t type, sysint_t width)
{
  functionMap->convert.argb32_from_prgb32(dst, src, width);
  functionMap->filters.colorMatrix[Image::FormatARGB32](dst, dst, cm, type, width);
  functionMap->convert.prgb32_from_argb32(dst, dst, width);
}

static void FOG_FASTCALL colorMatrix_argb32(
  uint8_t* dst, const uint8_t* src,
  const ColorMatrix* cm, uint32_t type, sysint_t width)
{
  for (sysint_t i = width; i; i--, dst += 4, src += 4)
  {
    ((uint32_t*)dst)[0] = ((const uint32_t*)src)[0];
    cm->transformRgba((Rgba*)dst);
  }
}

static void FOG_FASTCALL colorMatrix_rgb32(
  uint8_t* dst, const uint8_t* src,
  const ColorMatrix* cm, uint32_t type, sysint_t width)
{
  for (sysint_t i = width; i; i--, dst += 4, src += 4)
  {
    ((uint32_t*)dst)[0] = ((const uint32_t*)src)[0];
    cm->transformRgb((Rgba*)dst);
  }
}

static void FOG_FASTCALL colorMatrix_rgb24(
  uint8_t* dst, const uint8_t* src,
  const ColorMatrix* cm, uint32_t type, sysint_t width)
{
  for (sysint_t i = width; i; i--, dst += 3, src += 3)
  {
    Rgba c = PixFmt_RGB24::fetch(src);
    cm->transformRgb(&c);
    PixFmt_RGB24::store(dst, c);
  }
}

static void FOG_FASTCALL colorMatrix_a8(
  uint8_t* dst, const uint8_t* src,
  const ColorMatrix* cm, uint32_t type, sysint_t width)
{
  for (sysint_t i = width; i; i--, dst += 1, src += 1)
  {
    dst[0] = src[0];
    cm->transformAlpha(dst);
  }
}

// ============================================================================
// [Fog::Raster_C - Transpose]
// ============================================================================

static void FOG_FASTCALL transpose_32(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height)
{
  uint8_t* dstCur;
  const uint8_t* srcCur;

  sysint_t x, y;

  for (y = 0; y < height; y++)
  {
    // Setup raster pointers.
    dstCur = dst;
    srcCur = src;

    for (x = 0; x < width; x++)
    {
      Raster::copy4(dstCur, srcCur);
      dstCur += dstStride;
      srcCur += 4;
    }

    dst += 4;
    src += srcStride;
  }
}

static void FOG_FASTCALL transpose_24(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height)
{
  uint8_t* dstCur;
  const uint8_t* srcCur;

  sysint_t x, y;

  for (y = 0; y < height; y++)
  {
    // Setup raster pointers.
    dstCur = dst;
    srcCur = src;

    for (x = 0; x < width; x++)
    {
      Raster::copy3(dstCur, srcCur);
      dstCur += dstStride;
      srcCur += 4;
    }

    dst += 4;
    src += srcStride;
  }
}

static void FOG_FASTCALL transpose_8(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height)
{
  uint8_t* dstCur;
  const uint8_t* srcCur;

  sysint_t x, y;

  for (y = 0; y < height; y++)
  {
    // Setup raster pointers.
    dstCur = dst;
    srcCur = src;

    for (x = 0; x < width; x++)
    {
      Raster::copy1(dstCur, srcCur);
      dstCur += dstStride;
      srcCur += 4;
    }

    dst += 4;
    src += srcStride;
  }
}

// ============================================================================
// [Fog::Raster_C - FloatScanlineConvolve]
// ============================================================================

static void FOG_FASTCALL floatScanlineConvolve_argb32(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, const float* kernel, int size, float divisor,
  int borderMode, uint32_t borderColor)
{
  if (kernel == NULL || size == 0)
  {
    functionMap->filters.transpose[Image::FormatARGB32](dst, dstStride, src, srcStride, width, height);
    return;
  }

  MemoryBuffer<512> kernelTemporary;

  uint8_t* dstCur;
  const uint8_t* srcCur;

  int radiusInt = size >> 1;

  // Hot inner loops.
  bool tooSmallImage = width <= size;
  int firstLoopStop = (tooSmallImage) ? width : radiusInt;
  int secondLoopStop = (tooSmallImage) ? 0 : (width - radiusInt);

  sysint_t i, j, kpos;
  sysint_t x, y;

  float leftEdgeR = (float)(int)((borderColor >> 16) & 0xFF);
  float leftEdgeG = (float)(int)((borderColor >>  8) & 0xFF);
  float leftEdgeB = (float)(int)((borderColor      ) & 0xFF);
  float leftEdgeA = (float)(int)((borderColor >> 24)       );

  float rightEdgeR = leftEdgeR;
  float rightEdgeG = leftEdgeG;
  float rightEdgeB = leftEdgeB;
  float rightEdgeA = leftEdgeA;

  // Modify kernel to remove extra division / multiplication from main loop
  if (divisor != 1.0f)
  {
    float *k = (float*)kernelTemporary.alloc(sizeof(float) * size);
    if (!k) return;

    for (i = 0; i < size; i++) k[i] = kernel[i] / divisor;
    kernel = k;
  }

  for (y = 0; y < height; y++)
  {
    // Setup raster pointers.
    dstCur = dst;
    srcCur = src;

    // Setup borders if needed.
    if (borderMode == ImageFilter::ExtendBorderMode)
    {
      uint32_t pix;

      pix = ((const uint32_t*)srcCur)[0];
      leftEdgeR = (float)(int32_t)((pix >> 16) & 0xFF);
      leftEdgeG = (float)(int32_t)((pix >>  8) & 0xFF);
      leftEdgeB = (float)(int32_t)((pix      ) & 0xFF);
      leftEdgeA = (float)(int32_t)((pix >> 24)       );

      pix = ((const uint32_t*)srcCur)[width-1];
      rightEdgeR = (float)(int32_t)((pix >> 16) & 0xFF);
      rightEdgeG = (float)(int32_t)((pix >>  8) & 0xFF);
      rightEdgeB = (float)(int32_t)((pix      ) & 0xFF);
      rightEdgeA = (float)(int32_t)((pix >> 24)       );
    }

    x = 0;
    j = firstLoopStop;

    // Loop with bound checking supporting borders.
again:
    while (x < j)
    {
      float cr = 0.5f;
      float cg = 0.5f;
      float cb = 0.5f;
      float ca = 0.5f;

      for (kpos = 0, i = x - radiusInt; kpos < size; kpos++, i++)
      {
        float k = kernel[kpos];

        if (i < 0)
        {
          cr += leftEdgeR * k;
          cg += leftEdgeG * k;
          cb += leftEdgeB * k;
          ca += leftEdgeA * k;
        }
        else if (i >= width)
        {
          cr += rightEdgeR * k;
          cg += rightEdgeG * k;
          cb += rightEdgeB * k;
          ca += rightEdgeA * k;
        }
        else
        {
          uint32_t pix = ((const uint32_t*)src)[i];
          cr += (float)(int32_t)((pix >> 16) & 0xFF) * k;
          cg += (float)(int32_t)((pix >>  8) & 0xFF) * k;
          cb += (float)(int32_t)((pix      ) & 0xFF) * k;
          ca += (float)(int32_t)((pix >> 24)       ) * k;
        }
      }

      ((uint32_t*)dstCur)[0] = 
        ((uint32_t)clamp255((int32_t)cr) << 16) | 
        ((uint32_t)clamp255((int32_t)cg) <<  8) |
        ((uint32_t)clamp255((int32_t)cb)      ) |
        ((uint32_t)clamp255((int32_t)ca) << 24) ;

      dstCur += dstStride;
      srcCur += 4;
      x++;
    }

    // Loop without bound checking not supporting borders.
    if (x != width)
    {
      j = secondLoopStop;

      while (x < j)
      {
        float cr = 0.5f;
        float cg = 0.5f;
        float cb = 0.5f;
        float ca = 0.5f;

        for (kpos = 0, i = -radiusInt; kpos < size; kpos++, i++)
        {
          float k = kernel[kpos];

          uint32_t pix = ((const uint32_t*)srcCur)[i];
          cr += (float)(int32_t)((pix >> 16) & 0xFF) * k;
          cg += (float)(int32_t)((pix >>  8) & 0xFF) * k;
          cb += (float)(int32_t)((pix      ) & 0xFF) * k;
          ca += (float)(int32_t)((pix >> 24)       ) * k;
        }

        ((uint32_t*)dstCur)[0] = 
          ((uint32_t)clamp255((int32_t)cr) << 16) | 
          ((uint32_t)clamp255((int32_t)cg) <<  8) |
          ((uint32_t)clamp255((int32_t)cb)      ) |
          ((uint32_t)clamp255((int32_t)ca) << 24) ;

        dstCur += dstStride;
        srcCur += 4;
        x++;
      }

      j = width;
      goto again;
    }

    dst += 4;
    src += srcStride;
  }
}

static void FOG_FASTCALL floatScanlineConvolve_rgb24(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, const float* kernel, int size, float divisor,
  int borderMode, uint32_t borderColor)
{
  if (kernel == NULL || size == 0)
  {
    functionMap->filters.transpose[Image::FormatRGB24](dst, dstStride, src, srcStride, width, height);
    return;
  }

  MemoryBuffer<512> kernelTemporary;

  uint8_t* dstCur;
  const uint8_t* srcCur;

  int radiusInt = size >> 1;

  // Hot inner loops.
  bool tooSmallImage = width <= size;
  int firstLoopStop = (tooSmallImage) ? width : radiusInt;
  int secondLoopStop = (tooSmallImage) ? 0 : (width - radiusInt);

  sysint_t i, j, kpos;
  sysint_t x, y;

  float leftEdgeR = (float)(int)((borderColor >> 16) & 0xFF);
  float leftEdgeG = (float)(int)((borderColor >>  8) & 0xFF);
  float leftEdgeB = (float)(int)((borderColor      ) & 0xFF);

  float rightEdgeR = leftEdgeR;
  float rightEdgeG = leftEdgeG;
  float rightEdgeB = leftEdgeB;

  // Modify kernel to remove extra division / multiplication from main loop
  if (divisor != 1.0f)
  {
    float *k = (float*)kernelTemporary.alloc(sizeof(float) * size);
    if (!k) return;

    for (i = 0; i < size; i++) k[i] = kernel[i] / divisor;
    kernel = k;
  }

  for (y = 0; y < height; y++)
  {
    // Setup raster pointers.
    dstCur = dst;
    srcCur = src;

    // Setup borders if needed.
    if (borderMode == ImageFilter::ExtendBorderMode)
    {
      leftEdgeR = (float)(int32_t)(srcCur[Raster::RGB24_RByte]);
      leftEdgeG = (float)(int32_t)(srcCur[Raster::RGB24_GByte]);
      leftEdgeB = (float)(int32_t)(srcCur[Raster::RGB24_BByte]);

      rightEdgeR = (float)(int32_t)(srcCur[(width-1)*3 + Raster::RGB24_RByte]);
      rightEdgeG = (float)(int32_t)(srcCur[(width-1)*3 + Raster::RGB24_GByte]);
      rightEdgeB = (float)(int32_t)(srcCur[(width-1)*3 + Raster::RGB24_BByte]);
    }

    x = 0;
    j = firstLoopStop;

    // Loop with bound checking supporting borders.
again:
    while (x < j)
    {
      float cr = 0.5f;
      float cg = 0.5f;
      float cb = 0.5f;

      const uint8_t* srcCurInner = srcCur - radiusInt*3;
      for (kpos = 0, i = x - radiusInt; kpos < size; kpos++, i++, srcCurInner += 3)
      {
        float k = kernel[kpos];

        if (i < 0)
        {
          cr += leftEdgeR * k;
          cg += leftEdgeG * k;
          cb += leftEdgeB * k;
        }
        else if (i >= width)
        {
          cr += rightEdgeR * k;
          cg += rightEdgeG * k;
          cb += rightEdgeB * k;
        }
        else
        {
          cr += (float)(int32_t)(srcCurInner[Raster::RGB24_RByte]) * k;
          cg += (float)(int32_t)(srcCurInner[Raster::RGB24_GByte]) * k;
          cb += (float)(int32_t)(srcCurInner[Raster::RGB24_BByte]) * k;
        }
      }

      dstCur[Raster::RGB24_RByte] = (uint32_t)clamp255((int32_t)cr);
      dstCur[Raster::RGB24_GByte] = (uint32_t)clamp255((int32_t)cg);
      dstCur[Raster::RGB24_BByte] = (uint32_t)clamp255((int32_t)cb);

      dstCur += dstStride;
      srcCur += 3;
      x++;
    }

    // Loop without bound checking not supporting borders.
    if (x != width)
    {
      j = secondLoopStop;

      while (x < j)
      {
        float cr = 0.5f;
        float cg = 0.5f;
        float cb = 0.5f;

        const uint8_t* srcCurInner = srcCur - radiusInt*3;
        for (kpos = 0; kpos < size; kpos++, srcCurInner += 3)
        {
          float k = kernel[kpos];

          cr += (float)(int32_t)(srcCurInner[Raster::RGB24_RByte]) * k;
          cg += (float)(int32_t)(srcCurInner[Raster::RGB24_GByte]) * k;
          cb += (float)(int32_t)(srcCurInner[Raster::RGB24_BByte]) * k;
        }

        dstCur[Raster::RGB24_RByte] = (uint32_t)clamp255((int32_t)cr);
        dstCur[Raster::RGB24_GByte] = (uint32_t)clamp255((int32_t)cg);
        dstCur[Raster::RGB24_BByte] = (uint32_t)clamp255((int32_t)cb);

        dstCur += dstStride;
        srcCur += 3;
        x++;
      }

      j = width;
      goto again;
    }

    dst += 3;
    src += srcStride;
  }
}

static void FOG_FASTCALL floatScanlineConvolve_a8(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, const float* kernel, int size, float divisor,
  int borderMode, uint32_t borderColor)
{
  if (kernel == NULL || size == 0)
  {
    functionMap->filters.transpose[Image::FormatA8](dst, dstStride, src, srcStride, width, height);
    return;
  }

  MemoryBuffer<512> kernelTemporary;

  uint8_t* dstCur;
  const uint8_t* srcCur;

  int radiusInt = size >> 1;

  // Hot inner loops.
  bool tooSmallImage = width <= size;
  int firstLoopStop = (tooSmallImage) ? width : radiusInt;
  int secondLoopStop = (tooSmallImage) ? 0 : (width - radiusInt);

  sysint_t i, j, kpos;
  sysint_t x, y;

  float leftEdge = (float)(int)(borderColor & 0xFF);
  float rightEdge = leftEdge;

  // Modify kernel to remove extra division / multiplication from main loop
  if (divisor != 1.0f)
  {
    float *k = (float*)kernelTemporary.alloc(size * sizeof(float));
    if (!k) return;

    for (i = 0; i < size; i++) k[i] = kernel[i] / divisor;
    kernel = k;
  }

  for (y = 0; y < height; y++)
  {
    // Setup raster pointers.
    dstCur = dst;
    srcCur = src;

    // Setup borders if needed.
    if (borderMode == ImageFilter::ExtendBorderMode)
    {
      leftEdge = (float)(int32_t)(srcCur[0]);
      rightEdge = (float)(int32_t)(srcCur[width-1]);
    }

    x = 0;
    j = firstLoopStop;

    // Loop with bound checking supporting borders.
again:
    while (x < j)
    {
      float ca = 0.5f;

      const uint8_t* srcCurInner = srcCur - radiusInt;
      for (kpos = 0, i = x - radiusInt; kpos < size; kpos++, i++, srcCurInner++)
      {
        float k = kernel[kpos];

        if (i < 0)
        {
          ca += leftEdge * k;
        }
        else if (i >= width)
        {
          ca += rightEdge * k;
        }
        else
        {
          ca += (float)(int32_t)(srcCurInner[0]) * k;
        }
      }

      dstCur[0] = (uint32_t)clamp255((int32_t)ca);

      dstCur += dstStride;
      srcCur += 1;
      x++;
    }

    // Loop without bound checking not supporting borders.
    if (x != width)
    {
      j = secondLoopStop;

      while (x < j)
      {
        float ca = 0.5f;

        const uint8_t* srcCurInner = srcCur - radiusInt;
        for (kpos = 0; kpos < size; kpos++, srcCurInner++)
        {
          float k = kernel[kpos];
          ca += (float)(int32_t)(srcCurInner[0]) * k;
        }

        dstCur[0] = (uint32_t)clamp255((int32_t)ca);

        dstCur += dstStride;
        srcCur += 1;
        x++;
      }

      j = width;
      goto again;
    }

    dst += 1;
    src += srcStride;
  }
}

// ============================================================================
// [Fog::Raster_C - BoxBlur]
// ============================================================================

static void FOG_FASTCALL boxBlurConvolveH_argb32(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int radius,
  int borderMode, uint32_t borderColor)
{
  if (radius == 0 || width < 2)
  {
    if (dst != src) functionMap->filters.copyArea[Image::FormatARGB32](dst, dstStride, src, srcStride, width, height);
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

  uint sumMul = getReciprocal(size);
  uint sumShr = 16;

  uint32_t stack[512];
  uint32_t* stackEnd = stack + size;
  uint32_t* stackCur;

  uint32_t lBorderColor = borderColor;
  uint32_t rBorderColor = borderColor;

  for (pos2 = 0; pos2 < dym2; pos2++)
  {
    uint32_t pix0;
    uint32_t pixR;
    uint32_t pixG;
    uint32_t pixB;
    uint32_t pixA;

    uint32_t sumR = 0;
    uint32_t sumG = 0;
    uint32_t sumB = 0;
    uint32_t sumA = 0;

    srcCur = src;

    if (borderMode == ImageFilter::ExtendBorderMode)
    {
      lBorderColor = READ_32(srcCur);
      rBorderColor = READ_32(srcCur + end);
    }

    pix0 = lBorderColor;
    pixR = (pix0 >> 16) & 0xFF;
    pixG = (pix0 >>  8) & 0xFF;
    pixB = (pix0      ) & 0xFF;
    pixA = (pix0 >> 24);

    stackCur = stack;

    for (i = 0; i < radius; i++)
    {
      stackCur[0] = pix0;
      stackCur++;

      sumR += pixR;
      sumG += pixG;
      sumB += pixB;
      sumA += pixA;
    }

    pix0 = READ_32(srcCur);
    stackCur[0] = pix0;
    stackCur++;

    pixR = (pix0 >> 16) & 0xFF;
    pixG = (pix0 >>  8) & 0xFF;
    pixB = (pix0      ) & 0xFF;
    pixA = (pix0 >> 24);

    sumR += pixR;
    sumG += pixG;
    sumB += pixB;
    sumA += pixA;

    for (i = 1; i <= radius; i++)
    {
      if (i <= max)
      {
        srcCur += 4;
        pix0 = READ_32(srcCur);
      }
      else 
      {
        pix0 = rBorderColor;
      }

      stackCur[0] = pix0;
      stackCur++;

      pixR = (pix0 >> 16) & 0xFF;
      pixG = (pix0 >>  8) & 0xFF;
      pixB = (pix0      ) & 0xFF;
      pixA = (pix0 >> 24);

      sumR += pixR;
      sumG += pixG;
      sumB += pixB;
      sumA += pixA;
    }

    xp = fog_min(radius, max);

    srcCur = src + xp * 4;
    dstCur = dst;

    stackCur = stack;

    for (pos1 = 0; pos1 < dym1; pos1++)
    {
      ((uint32_t*)dstCur)[0] = 
        (((sumR * sumMul) >> sumShr) << 16) |
        (((sumG * sumMul) >> sumShr) <<  8) |
        (((sumB * sumMul) >> sumShr)      ) |
        (((sumA * sumMul) >> sumShr) << 24) ;
      dstCur += 4;

      pix0 = stackCur[0];

      pixR = (pix0 >> 16) & 0xFF;
      pixG = (pix0 >>  8) & 0xFF;
      pixB = (pix0      ) & 0xFF;
      pixA = (pix0 >> 24);

      sumR -= pixR;
      sumG -= pixG;
      sumB -= pixB;
      sumA -= pixA;

      if (xp < max)
      {
        ++xp;
        srcCur += 4;
        pix0 = READ_32(srcCur);
      }
      else
      {
        pix0 = rBorderColor;
      }

      stackCur[0] = pix0;

      pixR    = (pix0 >> 16) & 0xFF;
      pixG    = (pix0 >>  8) & 0xFF;
      pixB    = (pix0      ) & 0xFF;
      pixA    = (pix0 >> 24);

      sumR += pixR;
      sumG += pixG;
      sumB += pixB;
      sumA += pixA;

      stackCur += 1;
      if (stackCur == stackEnd) stackCur = stack;
    }

    src += srcStride;
    dst += dstStride;
  }
}

static void FOG_FASTCALL boxBlurConvolveV_argb32(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int radius,
  int borderMode, uint32_t borderColor)
{
  if (radius == 0 || height < 2)
  {
    if (dst != src) functionMap->filters.copyArea[Image::FormatARGB32](dst, dstStride, src, srcStride, width, height);
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

  uint sumMul = getReciprocal(size);
  uint sumShr = 16;

  uint32_t stack[512];
  uint32_t* stackEnd = stack + size;
  uint32_t* stackCur;

  uint32_t lBorderColor = borderColor;
  uint32_t rBorderColor = borderColor;

  for (pos2 = 0; pos2 < dym2; pos2++)
  {
    uint32_t pix0;
    uint32_t pixR;
    uint32_t pixG;
    uint32_t pixB;
    uint32_t pixA;

    uint32_t sumR = 0;
    uint32_t sumG = 0;
    uint32_t sumB = 0;
    uint32_t sumA = 0;

    srcCur = src;

    if (borderMode == ImageFilter::ExtendBorderMode)
    {
      lBorderColor = READ_32(srcCur);
      rBorderColor = READ_32(srcCur + end);
    }

    pix0 = lBorderColor;
    pixR = (pix0 >> 16) & 0xFF;
    pixG = (pix0 >>  8) & 0xFF;
    pixB = (pix0      ) & 0xFF;
    pixA = (pix0 >> 24);

    stackCur = stack;

    for (i = 0; i < radius; i++)
    {
      stackCur[0] = pix0;
      stackCur++;

      sumR += pixR;
      sumG += pixG;
      sumB += pixB;
      sumA += pixA;
    }

    pix0 = READ_32(srcCur);
    stackCur[0] = pix0;
    stackCur++;

    pixR = (pix0 >> 16) & 0xFF;
    pixG = (pix0 >>  8) & 0xFF;
    pixB = (pix0      ) & 0xFF;
    pixA = (pix0 >> 24);

    sumR += pixR;
    sumG += pixG;
    sumB += pixB;
    sumA += pixA;

    for (i = 1; i <= radius; i++)
    {
      if (i <= max)
      {
        srcCur += srcStride;
        pix0 = READ_32(srcCur);
      }
      else 
      {
        pix0 = rBorderColor;
      }

      stackCur[0] = pix0;
      stackCur++;

      pixR = (pix0 >> 16) & 0xFF;
      pixG = (pix0 >>  8) & 0xFF;
      pixB = (pix0      ) & 0xFF;
      pixA = (pix0 >> 24);

      sumR += pixR;
      sumG += pixG;
      sumB += pixB;
      sumA += pixA;
    }

    xp = fog_min(radius, max);

    srcCur = src + xp * srcStride;
    dstCur = dst;

    stackCur = stack;

    for (pos1 = 0; pos1 < dym1; pos1++)
    {
      ((uint32_t*)dstCur)[0] = 
        (((sumR * sumMul) >> sumShr) << 16) |
        (((sumG * sumMul) >> sumShr) <<  8) |
        (((sumB * sumMul) >> sumShr)      ) |
        (((sumA * sumMul) >> sumShr) << 24) ;
      dstCur += dstStride;

      pix0 = stackCur[0];

      pixR = (pix0 >> 16) & 0xFF;
      pixG = (pix0 >>  8) & 0xFF;
      pixB = (pix0      ) & 0xFF;
      pixA = (pix0 >> 24);

      sumR -= pixR;
      sumG -= pixG;
      sumB -= pixB;
      sumA -= pixA;

      if (xp < max)
      {
        ++xp;
        srcCur += srcStride;
        pix0 = READ_32(srcCur);
      }
      else
      {
        pix0 = rBorderColor;
      }

      stackCur[0] = pix0;

      pixR    = (pix0 >> 16) & 0xFF;
      pixG    = (pix0 >>  8) & 0xFF;
      pixB    = (pix0      ) & 0xFF;
      pixA    = (pix0 >> 24);

      sumR += pixR;
      sumG += pixG;
      sumB += pixB;
      sumA += pixA;

      stackCur += 1;
      if (stackCur == stackEnd) stackCur = stack;
    }

    src += 4;
    dst += 4;
  }
}

static void FOG_FASTCALL boxBlurConvolveH_rgb24(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int radius,
  int borderMode, uint32_t borderColor)
{
  if (radius == 0 || width < 2)
  {
    if (dst != src) functionMap->filters.copyArea[Image::FormatRGB24](dst, dstStride, src, srcStride, width, height);
    return;
  }

  if (radius > 254) radius = 254;

  sysint_t dym1 = width;
  sysint_t dym2 = height;
  sysint_t max = dym1 - 1;
  sysint_t end = max * 3;

  uint8_t* dstCur;
  const uint8_t* srcCur;

  sysint_t pos1;
  sysint_t pos2;
  sysint_t xp, i;

  uint size = (uint)radius * 2 + 1;

  uint sumMul = getReciprocal(size);
  uint sumShr = 16;

  uint8_t stack[512*3];
  uint8_t* stackEnd = stack + size * 3;
  uint8_t* stackCur;

  uint32_t lBorderColorR = (borderColor >> 16) & 0xFF;
  uint32_t lBorderColorG = (borderColor >>  8) & 0xFF;
  uint32_t lBorderColorB = (borderColor      ) & 0xFF;
  uint32_t rBorderColorR = lBorderColorR;
  uint32_t rBorderColorG = lBorderColorG;
  uint32_t rBorderColorB = lBorderColorB;

  for (pos2 = 0; pos2 < dym2; pos2++)
  {
    uint32_t pixR;
    uint32_t pixG;
    uint32_t pixB;

    uint32_t sumR = 0;
    uint32_t sumG = 0;
    uint32_t sumB = 0;

    srcCur = src;

    if (borderMode == ImageFilter::ExtendBorderMode)
    {
      lBorderColorR = srcCur[RGB24_RByte];
      lBorderColorG = srcCur[RGB24_GByte];
      lBorderColorB = srcCur[RGB24_BByte];

      rBorderColorR = srcCur[RGB24_RByte + end];
      rBorderColorG = srcCur[RGB24_GByte + end];
      rBorderColorB = srcCur[RGB24_BByte + end];
    }

    pixR = lBorderColorR;
    pixG = lBorderColorG;
    pixB = lBorderColorB;

    stackCur = stack;

    for (i = 0; i < radius; i++)
    {
      stackCur[RGB24_RByte] = pixR;
      stackCur[RGB24_GByte] = pixR;
      stackCur[RGB24_BByte] = pixR;
      stackCur += 3;

      sumR += pixR;
      sumG += pixG;
      sumB += pixB;
    }

    pixR = srcCur[RGB24_RByte];
    pixG = srcCur[RGB24_GByte];
    pixB = srcCur[RGB24_BByte];

    stackCur[RGB24_RByte] = pixR;
    stackCur[RGB24_GByte] = pixG;
    stackCur[RGB24_BByte] = pixB;
    stackCur += 3;

    sumR += pixR;
    sumG += pixG;
    sumB += pixB;

    for (i = 1; i <= radius; i++)
    {
      if (i <= max)
      {
        srcCur += 3;
        pixR = srcCur[RGB24_RByte];
        pixG = srcCur[RGB24_GByte];
        pixB = srcCur[RGB24_BByte];
      }
      else 
      {
        pixR = rBorderColorR;
        pixG = rBorderColorG;
        pixB = rBorderColorB;
      }

      stackCur[RGB24_RByte] = pixR;
      stackCur[RGB24_GByte] = pixG;
      stackCur[RGB24_BByte] = pixB;
      stackCur += 3;

      sumR += pixR;
      sumG += pixG;
      sumB += pixB;
    }

    xp = fog_min(radius, max);

    srcCur = src + xp * 3;
    dstCur = dst;

    stackCur = stack;

    for (pos1 = 0; pos1 < dym1; pos1++)
    {
      dstCur[RGB24_RByte] = (sumR * sumMul) >> sumShr;
      dstCur[RGB24_GByte] = (sumG * sumMul) >> sumShr;
      dstCur[RGB24_BByte] = (sumB * sumMul) >> sumShr;
      dstCur += 3;

      pixR = stackCur[RGB24_RByte];
      pixG = stackCur[RGB24_GByte];
      pixB = stackCur[RGB24_BByte];

      sumR -= pixR;
      sumG -= pixG;
      sumB -= pixB;

      if (xp < max)
      {
        ++xp;
        srcCur += 3;
        pixR = srcCur[RGB24_RByte];
        pixG = srcCur[RGB24_GByte];
        pixB = srcCur[RGB24_BByte];
      }
      else
      {
        pixR = rBorderColorR;
        pixG = rBorderColorG;
        pixB = rBorderColorB;
      }

      stackCur[RGB24_RByte] = pixR;
      stackCur[RGB24_GByte] = pixG;
      stackCur[RGB24_BByte] = pixB;
      stackCur += 3;

      sumR += pixR;
      sumG += pixG;
      sumB += pixB;

      stackCur += 3;
      if (stackCur == stackEnd) stackCur = stack;
    }

    src += srcStride;
    dst += dstStride;
  }
}

static void FOG_FASTCALL boxBlurConvolveV_rgb24(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int radius,
  int borderMode, uint32_t borderColor)
{
  if (radius == 0 || height < 2)
  {
    if (dst != src) functionMap->filters.copyArea[Image::FormatRGB24](dst, dstStride, src, srcStride, width, height);
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

  uint sumMul = getReciprocal(size);
  uint sumShr = 16;

  uint8_t stack[512*3];
  uint8_t* stackEnd = stack + size * 3;
  uint8_t* stackCur;

  uint32_t lBorderColorR = (borderColor >> 16) & 0xFF;
  uint32_t lBorderColorG = (borderColor >>  8) & 0xFF;
  uint32_t lBorderColorB = (borderColor      ) & 0xFF;
  uint32_t rBorderColorR = lBorderColorR;
  uint32_t rBorderColorG = lBorderColorG;
  uint32_t rBorderColorB = lBorderColorB;

  for (pos2 = 0; pos2 < dym2; pos2++)
  {
    uint32_t pixR;
    uint32_t pixG;
    uint32_t pixB;

    uint32_t sumR = 0;
    uint32_t sumG = 0;
    uint32_t sumB = 0;

    srcCur = src;

    if (borderMode == ImageFilter::ExtendBorderMode)
    {
      lBorderColorR = srcCur[RGB24_RByte];
      lBorderColorG = srcCur[RGB24_GByte];
      lBorderColorB = srcCur[RGB24_BByte];

      rBorderColorR = srcCur[RGB24_RByte + end];
      rBorderColorG = srcCur[RGB24_GByte + end];
      rBorderColorB = srcCur[RGB24_BByte + end];
    }

    pixR = lBorderColorR;
    pixG = lBorderColorG;
    pixB = lBorderColorB;

    stackCur = stack;

    for (i = 0; i < radius; i++)
    {
      stackCur[RGB24_RByte] = pixR;
      stackCur[RGB24_GByte] = pixR;
      stackCur[RGB24_BByte] = pixR;
      stackCur += 3;

      sumR += pixR;
      sumG += pixG;
      sumB += pixB;
    }

    pixR = srcCur[RGB24_RByte];
    pixG = srcCur[RGB24_GByte];
    pixB = srcCur[RGB24_BByte];

    stackCur[RGB24_RByte] = pixR;
    stackCur[RGB24_GByte] = pixG;
    stackCur[RGB24_BByte] = pixB;
    stackCur += 3;

    sumR += pixR;
    sumG += pixG;
    sumB += pixB;

    for (i = 1; i <= radius; i++)
    {
      if (i <= max)
      {
        srcCur += srcStride;
        pixR = srcCur[RGB24_RByte];
        pixG = srcCur[RGB24_GByte];
        pixB = srcCur[RGB24_BByte];
      }
      else 
      {
        pixR = rBorderColorR;
        pixG = rBorderColorG;
        pixB = rBorderColorB;
      }

      stackCur[RGB24_RByte] = pixR;
      stackCur[RGB24_GByte] = pixG;
      stackCur[RGB24_BByte] = pixB;
      stackCur += 3;

      sumR += pixR;
      sumG += pixG;
      sumB += pixB;
    }

    xp = fog_min(radius, max);

    srcCur = src + xp * srcStride;
    dstCur = dst;

    stackCur = stack;

    for (pos1 = 0; pos1 < dym1; pos1++)
    {
      dstCur[RGB24_RByte] = (sumR * sumMul) >> sumShr;
      dstCur[RGB24_GByte] = (sumG * sumMul) >> sumShr;
      dstCur[RGB24_BByte] = (sumB * sumMul) >> sumShr;
      dstCur += dstStride;

      pixR = stackCur[RGB24_RByte];
      pixG = stackCur[RGB24_GByte];
      pixB = stackCur[RGB24_BByte];

      sumR -= pixR;
      sumG -= pixG;
      sumB -= pixB;

      if (xp < max)
      {
        ++xp;
        srcCur += srcStride;
        pixR = srcCur[RGB24_RByte];
        pixG = srcCur[RGB24_GByte];
        pixB = srcCur[RGB24_BByte];
      }
      else
      {
        pixR = rBorderColorR;
        pixG = rBorderColorG;
        pixB = rBorderColorB;
      }

      stackCur[RGB24_RByte] = pixR;
      stackCur[RGB24_GByte] = pixG;
      stackCur[RGB24_BByte] = pixB;
      stackCur += 3;

      sumR += pixR;
      sumG += pixG;
      sumB += pixB;

      stackCur += 3;
      if (stackCur == stackEnd) stackCur = stack;
    }

    src += 3;
    dst += 3;
  }
}

static void FOG_FASTCALL boxBlurConvolveH_a8(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int radius,
  int borderMode, uint32_t borderColor)
{
  if (radius == 0 || width < 2)
  {
    if (dst != src) functionMap->filters.copyArea[Image::FormatA8](dst, dstStride, src, srcStride, width, height);
    return;
  }

  if (radius > 254) radius = 254;

  sysint_t dym1 = width;
  sysint_t dym2 = height;
  sysint_t max = dym1 - 1;
  sysint_t end = max;

  uint8_t* dstCur;
  const uint8_t* srcCur;

  sysint_t pos1;
  sysint_t pos2;
  sysint_t xp, i;

  uint size = (uint)radius * 2 + 1;

  uint sumMul = getReciprocal(size);
  uint sumShr = 16;

  uint8_t stack[512];
  uint8_t* stackEnd = stack + size;
  uint8_t* stackCur;

  uint32_t lBorderColor = borderColor;
  uint32_t rBorderColor = borderColor;

  for (pos2 = 0; pos2 < dym2; pos2++)
  {
    uint32_t pix;
    uint32_t sum = 0;

    srcCur = src;

    if (borderMode == ImageFilter::ExtendBorderMode)
    {
      lBorderColor = srcCur[0];
      rBorderColor = srcCur[end];
    }

    pix = lBorderColor;

    stackCur = stack;

    for (i = 0; i < radius; i++)
    {
      stackCur[0] = pix;
      stackCur++;

      sum += pix;
    }

    pix = srcCur[0];
    stackCur[0] = pix;
    stackCur++;

    sum += pix;

    for (i = 1; i <= radius; i++)
    {
      if (i <= max)
      {
        srcCur += 1;
        pix = srcCur[0];
      }
      else 
      {
        pix = rBorderColor;
      }

      stackCur[0] = pix;
      stackCur++;

      sum += pix;
    }

    xp = fog_min(radius, max);

    srcCur = src + xp;
    dstCur = dst;

    stackCur = stack;

    for (pos1 = 0; pos1 < dym1; pos1++)
    {
      dstCur[0] = (sum * sumMul) >> sumShr;
      dstCur += 1;

      pix = stackCur[0];
      sum -= pix;

      if (xp < max)
      {
        ++xp;
        srcCur += 1;
        pix = srcCur[0];
      }
      else
      {
        pix = rBorderColor;
      }

      stackCur[0] = pix;
      sum += pix;

      stackCur += 1;
      if (stackCur == stackEnd) stackCur = stack;
    }

    src += srcStride;
    dst += dstStride;
  }
}

static void FOG_FASTCALL boxBlurConvolveV_a8(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int radius,
  int borderMode, uint32_t borderColor)
{
  if (radius == 0 || height < 2)
  {
    if (dst != src) functionMap->filters.copyArea[Image::FormatA8](dst, dstStride, src, srcStride, width, height);
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

  uint sumMul = getReciprocal(size);
  uint sumShr = 16;

  uint8_t stack[512];
  uint8_t* stackEnd = stack + size;
  uint8_t* stackCur;

  uint32_t lBorderColor = borderColor;
  uint32_t rBorderColor = borderColor;

  for (pos2 = 0; pos2 < dym2; pos2++)
  {
    uint32_t pix;
    uint32_t sum = 0;

    srcCur = src;

    if (borderMode == ImageFilter::ExtendBorderMode)
    {
      lBorderColor = srcCur[0];
      rBorderColor = srcCur[end];
    }

    pix = lBorderColor;
    stackCur = stack;

    for (i = 0; i < radius; i++)
    {
      stackCur[0] = pix;
      stackCur++;

      sum += pix;
    }

    pix = srcCur[0];
    stackCur[0] = pix;
    stackCur++;

    sum += pix;

    for (i = 1; i <= radius; i++)
    {
      if (i <= max)
      {
        srcCur += srcStride;
        pix = srcCur[0];
      }
      else 
      {
        pix = rBorderColor;
      }

      stackCur[0] = pix;
      stackCur++;

      sum += pix;
    }

    xp = fog_min(radius, max);

    srcCur = src + xp * srcStride;
    dstCur = dst;

    stackCur = stack;

    for (pos1 = 0; pos1 < dym1; pos1++)
    {
      dstCur[0] = (sum * sumMul) >> sumShr;
      dstCur += dstStride;

      pix = stackCur[0];
      sum -= pix;

      if (xp < max)
      {
        ++xp;
        srcCur += srcStride;
        pix = srcCur[0];
      }
      else
      {
        pix = rBorderColor;
      }

      stackCur[0] = pix;
      sum += pix;

      stackCur += 1;
      if (stackCur == stackEnd) stackCur = stack;
    }

    src += 1;
    dst += 1;
  }
}

// ============================================================================
// [Fog::Raster_C - StackBlur]
// ============================================================================

static const uint16_t stackBlur8Mul[255] = 
{
  512, 512, 456, 512, 328, 456, 335, 512, 405, 328, 271, 456, 388, 335, 292, 512,
  454, 405, 364, 328, 298, 271, 496, 456, 420, 388, 360, 335, 312, 292, 273, 512,
  482, 454, 428, 405, 383, 364, 345, 328, 312, 298, 284, 271, 259, 496, 475, 456,
  437, 420, 404, 388, 374, 360, 347, 335, 323, 312, 302, 292, 282, 273, 265, 512,
  497, 482, 468, 454, 441, 428, 417, 405, 394, 383, 373, 364, 354, 345, 337, 328,
  320, 312, 305, 298, 291, 284, 278, 271, 265, 259, 507, 496, 485, 475, 465, 456,
  446, 437, 428, 420, 412, 404, 396, 388, 381, 374, 367, 360, 354, 347, 341, 335,
  329, 323, 318, 312, 307, 302, 297, 292, 287, 282, 278, 273, 269, 265, 261, 512,
  505, 497, 489, 482, 475, 468, 461, 454, 447, 441, 435, 428, 422, 417, 411, 405,
  399, 394, 389, 383, 378, 373, 368, 364, 359, 354, 350, 345, 341, 337, 332, 328,
  324, 320, 316, 312, 309, 305, 301, 298, 294, 291, 287, 284, 281, 278, 274, 271,
  268, 265, 262, 259, 257, 507, 501, 496, 491, 485, 480, 475, 470, 465, 460, 456,
  451, 446, 442, 437, 433, 428, 424, 420, 416, 412, 408, 404, 400, 396, 392, 388,
  385, 381, 377, 374, 370, 367, 363, 360, 357, 354, 350, 347, 344, 341, 338, 335,
  332, 329, 326, 323, 320, 318, 315, 312, 310, 307, 304, 302, 299, 297, 294, 292,
  289, 287, 285, 282, 280, 278, 275, 273, 271, 269, 267, 265, 263, 261, 259
};

static const uint8_t stackBlur8Shr[255] = 
{
  9 , 11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17, 
  17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 
  19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20,
  20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21,
  21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
  21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 
  22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
  22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23, 
  23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
  23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
  23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 
  23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 
  24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
  24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
  24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
  24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24
};

static void FOG_FASTCALL stackBlurConvolveH_argb32(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int radius,
  int borderMode, uint32_t borderColor)
{
  if (radius == 0 || width < 2)
  {
    if (dst != src) functionMap->filters.copyArea[Image::FormatARGB32](dst, dstStride, src, srcStride, width, height);
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

  uint sumMul = functionMap->filters.stackBlur8Mul[radius];
  uint sumShr = functionMap->filters.stackBlur8Shr[radius];

  uint32_t stack[512];
  uint32_t* stackEnd = stack + (radius * 2 + 1);
  uint32_t* stackLeft;
  uint32_t* stackRight;

  uint32_t lBorderColor = borderColor;
  uint32_t rBorderColor = borderColor;

  for (pos2 = 0; pos2 < dym2; pos2++)
  {
    uint32_t pix0;
    uint32_t pixR;
    uint32_t pixG;
    uint32_t pixB;
    uint32_t pixA;

    uint32_t sumR = 0;
    uint32_t sumG = 0;
    uint32_t sumB = 0;
    uint32_t sumA = 0;

    uint32_t sumInR = 0;
    uint32_t sumInG = 0;
    uint32_t sumInB = 0;
    uint32_t sumInA = 0;

    uint32_t sumOutR = 0;
    uint32_t sumOutG = 0;
    uint32_t sumOutB = 0;
    uint32_t sumOutA = 0;

    srcCur = src;

    if (borderMode == ImageFilter::ExtendBorderMode)
    {
      lBorderColor = READ_32(srcCur);
      rBorderColor = READ_32(srcCur + end);
    }

    pix0 = lBorderColor;
    pixR = (pix0 >> 16) & 0xFF;
    pixG = (pix0 >>  8) & 0xFF;
    pixB = (pix0      ) & 0xFF;
    pixA = (pix0 >> 24);

    stackLeft = stack;

    for (i = 0; i < radius; i++)
    {
      stackLeft[0] = pix0;
      stackLeft++;

      sumR            += pixR * (i + 1);
      sumG            += pixG * (i + 1);
      sumB            += pixB * (i + 1);
      sumA            += pixA * (i + 1);

      sumOutR         += pixR;
      sumOutG         += pixG;
      sumOutB         += pixB;
      sumOutA         += pixA;
    }

    pix0 = READ_32(srcCur);
    stackLeft[0] = pix0;
    stackLeft++;

    pixR = (pix0 >> 16) & 0xFF;
    pixG = (pix0 >>  8) & 0xFF;
    pixB = (pix0      ) & 0xFF;
    pixA = (pix0 >> 24);

    sumR            += pixR * (radius + 1);
    sumG            += pixG * (radius + 1);
    sumB            += pixB * (radius + 1);
    sumA            += pixA * (radius + 1);

    sumOutR         += pixR;
    sumOutG         += pixG;
    sumOutB         += pixB;
    sumOutA         += pixA;

    for (i = 1; i <= radius; i++)
    {
      if (i <= max)
      {
        srcCur += 4;
        pix0 = READ_32(srcCur);
      }
      else 
      {
        pix0 = rBorderColor;
      }

      pixR = (pix0 >> 16) & 0xFF;
      pixG = (pix0 >>  8) & 0xFF;
      pixB = (pix0      ) & 0xFF;
      pixA = (pix0 >> 24);

      stackLeft[0] = pix0;
      stackLeft++;

      sumR            += pixR * (radius + 1 - i);
      sumG            += pixG * (radius + 1 - i);
      sumB            += pixB * (radius + 1 - i);
      sumA            += pixA * (radius + 1 - i);

      sumInR          += pixR;
      sumInG          += pixG;
      sumInB          += pixB;
      sumInA          += pixA;
    }

    xp = fog_min(radius, max);

    srcCur = src + xp * 4;
    dstCur = dst;

    stackLeft = stack;
    stackRight = stack + radius;

    for (pos1 = 0; pos1 < dym1; pos1++)
    {
      ((uint32_t*)dstCur)[0] = 
        (((sumR * sumMul) >> sumShr) << 16) |
        (((sumG * sumMul) >> sumShr) <<  8) |
        (((sumB * sumMul) >> sumShr)      ) |
        (((sumA * sumMul) >> sumShr) << 24) ;
      dstCur += 4;

      sumR -= sumOutR;
      sumG -= sumOutG;
      sumB -= sumOutB;
      sumA -= sumOutA;

      pix0 = stackLeft[0];

      sumOutR -= (pix0 >> 16) & 0xFF;
      sumOutG -= (pix0 >>  8) & 0xFF;
      sumOutB -= (pix0      ) & 0xFF;
      sumOutA -= (pix0 >> 24);

      if (xp < max)
      {
        ++xp;
        srcCur += 4;
        pix0 = READ_32(srcCur);
      }
      else
      {
        pix0 = rBorderColor;
      }

      pixR    = (pix0 >> 16) & 0xFF;
      pixG    = (pix0 >>  8) & 0xFF;
      pixB    = (pix0      ) & 0xFF;
      pixA    = (pix0 >> 24);

      stackLeft[0] = pix0;

      sumInR += pixR;
      sumInG += pixG;
      sumInB += pixB;
      sumInA += pixA;

      sumR   += sumInR;
      sumG   += sumInG;
      sumB   += sumInB;
      sumA   += sumInA;

      stackLeft += 1;
      stackRight += 1;

      if (stackLeft == stackEnd) stackLeft = stack;
      if (stackRight == stackEnd) stackRight = stack;

      pix0    = stackRight[0];
      pixR    = (pix0 >> 16) & 0xFF;
      pixG    = (pix0 >>  8) & 0xFF;
      pixB    = (pix0      ) & 0xFF;
      pixA    = (pix0 >> 24);

      sumOutR += pixR;
      sumOutG += pixG;
      sumOutB += pixB;
      sumOutA += pixA;

      sumInR  -= pixR;
      sumInG  -= pixG;
      sumInB  -= pixB;
      sumInA  -= pixA;
    }

    src += srcStride;
    dst += dstStride;
  }
}

static void FOG_FASTCALL stackBlurConvolveV_argb32(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int radius,
  int borderMode, uint32_t borderColor)
{
  if (radius == 0 || height < 2)
  {
    if (dst != src) functionMap->filters.copyArea[Image::FormatARGB32](dst, dstStride, src, srcStride, width, height);
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

  uint sumMul = functionMap->filters.stackBlur8Mul[radius];
  uint sumShr = functionMap->filters.stackBlur8Shr[radius];

  uint32_t stack[512];
  uint32_t* stackEnd = stack + (radius * 2 + 1);
  uint32_t* stackLeft;
  uint32_t* stackRight;

  uint32_t lBorderColor = borderColor;
  uint32_t rBorderColor = borderColor;

  for (pos2 = 0; pos2 < dym2; pos2++)
  {
    uint32_t pix0;
    uint32_t pixR;
    uint32_t pixG;
    uint32_t pixB;
    uint32_t pixA;

    uint32_t sumR = 0;
    uint32_t sumG = 0;
    uint32_t sumB = 0;
    uint32_t sumA = 0;

    uint32_t sumInR = 0;
    uint32_t sumInG = 0;
    uint32_t sumInB = 0;
    uint32_t sumInA = 0;

    uint32_t sumOutR = 0;
    uint32_t sumOutG = 0;
    uint32_t sumOutB = 0;
    uint32_t sumOutA = 0;

    srcCur = src;

    if (borderMode == ImageFilter::ExtendBorderMode)
    {
      lBorderColor = READ_32(srcCur);
      rBorderColor = READ_32(srcCur + end);
    }

    pix0 = lBorderColor;
    pixR = (pix0 >> 16) & 0xFF;
    pixG = (pix0 >>  8) & 0xFF;
    pixB = (pix0      ) & 0xFF;
    pixA = (pix0 >> 24);

    stackLeft = stack;

    for (i = 0; i < radius; i++)
    {
      stackLeft[0] = pix0;
      stackLeft++;

      sumR            += pixR * (i + 1);
      sumG            += pixG * (i + 1);
      sumB            += pixB * (i + 1);
      sumA            += pixA * (i + 1);

      sumOutR         += pixR;
      sumOutG         += pixG;
      sumOutB         += pixB;
      sumOutA         += pixA;
    }

    pix0 = READ_32(srcCur);
    stackLeft[0] = pix0;
    stackLeft++;

    pixR = (pix0 >> 16) & 0xFF;
    pixG = (pix0 >>  8) & 0xFF;
    pixB = (pix0      ) & 0xFF;
    pixA = (pix0 >> 24);

    sumR            += pixR * (radius + 1);
    sumG            += pixG * (radius + 1);
    sumB            += pixB * (radius + 1);
    sumA            += pixA * (radius + 1);

    sumOutR         += pixR;
    sumOutG         += pixG;
    sumOutB         += pixB;
    sumOutA         += pixA;

    for (i = 1; i <= radius; i++)
    {
      if (i <= max)
      {
        srcCur += srcStride;
        pix0 = READ_32(srcCur);
      }
      else 
      {
        pix0 = rBorderColor;
      }

      pixR = (pix0 >> 16) & 0xFF;
      pixG = (pix0 >>  8) & 0xFF;
      pixB = (pix0      ) & 0xFF;
      pixA = (pix0 >> 24);

      stackLeft[0] = pix0;
      stackLeft++;

      sumR            += pixR * (radius + 1 - i);
      sumG            += pixG * (radius + 1 - i);
      sumB            += pixB * (radius + 1 - i);
      sumA            += pixA * (radius + 1 - i);

      sumInR          += pixR;
      sumInG          += pixG;
      sumInB          += pixB;
      sumInA          += pixA;
    }

    xp = fog_min(radius, max);

    srcCur = src + xp * srcStride;
    dstCur = dst;

    stackLeft = stack;
    stackRight = stack + radius;

    for (pos1 = 0; pos1 < dym1; pos1++)
    {
      ((uint32_t*)dstCur)[0] = 
        (((sumR * sumMul) >> sumShr) << 16) |
        (((sumG * sumMul) >> sumShr) <<  8) |
        (((sumB * sumMul) >> sumShr)      ) |
        (((sumA * sumMul) >> sumShr) << 24) ;
      dstCur += dstStride;

      sumR -= sumOutR;
      sumG -= sumOutG;
      sumB -= sumOutB;
      sumA -= sumOutA;

      pix0 = stackLeft[0];

      sumOutR -= (pix0 >> 16) & 0xFF;
      sumOutG -= (pix0 >>  8) & 0xFF;
      sumOutB -= (pix0      ) & 0xFF;
      sumOutA -= (pix0 >> 24);

      if (xp < max) 
      {
        srcCur += srcStride;
        ++xp;
        pix0 = ((const uint32_t*)srcCur)[0];
      }
      else
      {
        pix0 = rBorderColor;
      }

      pixR    = (pix0 >> 16) & 0xFF;
      pixG    = (pix0 >>  8) & 0xFF;
      pixB    = (pix0      ) & 0xFF;
      pixA    = (pix0 >> 24);

      stackLeft[0] = pix0;

      sumInR += pixR;
      sumInG += pixG;
      sumInB += pixB;
      sumInA += pixA;

      sumR   += sumInR;
      sumG   += sumInG;
      sumB   += sumInB;
      sumA   += sumInA;

      stackLeft += 1;
      stackRight += 1;

      if (stackLeft == stackEnd) stackLeft = stack;
      if (stackRight == stackEnd) stackRight = stack;

      pix0    = stackRight[0];
      pixR    = (pix0 >> 16) & 0xFF;
      pixG    = (pix0 >>  8) & 0xFF;
      pixB    = (pix0      ) & 0xFF;
      pixA    = (pix0 >> 24);

      sumOutR += pixR;
      sumOutG += pixG;
      sumOutB += pixB;
      sumOutA += pixA;

      sumInR  -= pixR;
      sumInG  -= pixG;
      sumInB  -= pixB;
      sumInA  -= pixA;
    }

    src += 4;
    dst += 4;
  }
}

static void FOG_FASTCALL stackBlurConvolveH_rgb24(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int radius,
  int borderMode, uint32_t borderColor)
{
  if (radius == 0 || width < 2)
  {
    if (dst != src) functionMap->filters.copyArea[Image::FormatRGB24](dst, dstStride, src, srcStride, width, height);
    return;
  }

  if (radius > 254) radius = 254;

  sysint_t dym1 = width;
  sysint_t dym2 = height;
  sysint_t max = dym1 - 1;
  sysint_t end = max * 3;

  uint8_t* dstCur;
  const uint8_t* srcCur;

  sysint_t pos1;
  sysint_t pos2;
  sysint_t xp, i;

  uint sumMul = functionMap->filters.stackBlur8Mul[radius];
  uint sumShr = functionMap->filters.stackBlur8Shr[radius];

  uint8_t stack[512*3];
  uint8_t* stackEnd = stack + (radius * 2 + 1) * 3;
  uint8_t* stackLeft;
  uint8_t* stackRight;

  uint32_t lBorderColor = borderColor;
  uint32_t rBorderColor = borderColor;

  for (pos2 = 0; pos2 < dym2; pos2++)
  {
    uint32_t pixR;
    uint32_t pixG;
    uint32_t pixB;

    uint32_t sumR = 0;
    uint32_t sumG = 0;
    uint32_t sumB = 0;

    uint32_t sumInR = 0;
    uint32_t sumInG = 0;
    uint32_t sumInB = 0;

    uint32_t sumOutR = 0;
    uint32_t sumOutG = 0;
    uint32_t sumOutB = 0;

    uint32_t lBorderColorR;
    uint32_t lBorderColorG;
    uint32_t lBorderColorB;

    uint32_t rBorderColorR;
    uint32_t rBorderColorG;
    uint32_t rBorderColorB;

    srcCur = src;

    if (borderMode == ImageFilter::ExtendBorderMode)
    {
      lBorderColorR = srcCur[RGB24_RByte];
      lBorderColorG = srcCur[RGB24_GByte];
      lBorderColorB = srcCur[RGB24_BByte];

      rBorderColorR = srcCur[end + RGB24_RByte];
      rBorderColorG = srcCur[end + RGB24_GByte];
      rBorderColorB = srcCur[end + RGB24_BByte];
    }
    else
    {
      lBorderColorR = (borderColor >> 16) & 0xFF;
      lBorderColorG = (borderColor >>  8) & 0xFF;
      lBorderColorB = (borderColor      ) & 0xFF;

      rBorderColorR = lBorderColorR;
      rBorderColorG = lBorderColorG;
      rBorderColorB = lBorderColorB;
    }

    pixR = lBorderColorR;
    pixG = lBorderColorG;
    pixB = lBorderColorB;

    stackLeft = stack;

    for (i = 0; i < radius; i++)
    {
      stackLeft[RGB24_RByte] = pixR;
      stackLeft[RGB24_GByte] = pixG;
      stackLeft[RGB24_BByte] = pixB;
      stackLeft += 3;

      sumR            += pixR * (i + 1);
      sumG            += pixG * (i + 1);
      sumB            += pixB * (i + 1);

      sumOutR         += pixR;
      sumOutG         += pixG;
      sumOutB         += pixB;
    }

    pixR = srcCur[RGB24_RByte];
    pixG = srcCur[RGB24_GByte];
    pixB = srcCur[RGB24_BByte];

    stackLeft[RGB24_RByte] = pixR;
    stackLeft[RGB24_GByte] = pixG;
    stackLeft[RGB24_BByte] = pixB;
    stackLeft += 3;

    sumR            += pixR * (radius + 1);
    sumG            += pixG * (radius + 1);
    sumB            += pixB * (radius + 1);

    sumOutR         += pixR;
    sumOutG         += pixG;
    sumOutB         += pixB;

    for (i = 1; i <= radius; i++)
    {
      if (i <= max) 
      {
        srcCur += 3;
        pixR = srcCur[RGB24_RByte];
        pixG = srcCur[RGB24_GByte];
        pixB = srcCur[RGB24_BByte];
      }
      else 
      {
        pixR = rBorderColorR;
        pixG = rBorderColorG;
        pixB = rBorderColorB;
      }

      stackLeft[RGB24_RByte] = pixR;
      stackLeft[RGB24_GByte] = pixG;
      stackLeft[RGB24_BByte] = pixB;
      stackLeft += 3;

      sumR            += pixR * (radius + 1 - i);
      sumG            += pixG * (radius + 1 - i);
      sumB            += pixB * (radius + 1 - i);

      sumInR          += pixR;
      sumInG          += pixG;
      sumInB          += pixB;
    }

    xp = fog_min(radius, max);

    srcCur = src + xp * 3;
    dstCur = dst;

    stackLeft = stack;
    stackRight = stack + radius * 3;

    for (pos1 = 0; pos1 < dym1; pos1++)
    {
      dstCur[RGB24_RByte] = ((sumR * sumMul) >> sumShr);
      dstCur[RGB24_GByte] = ((sumG * sumMul) >> sumShr);
      dstCur[RGB24_BByte] = ((sumB * sumMul) >> sumShr);
      dstCur += 3;

      sumR -= sumOutR;
      sumG -= sumOutG;
      sumB -= sumOutB;

      sumOutR -= stackLeft[RGB24_RByte];
      sumOutG -= stackLeft[RGB24_GByte];
      sumOutB -= stackLeft[RGB24_BByte];

      if (xp < max) 
      {
        srcCur += 3;
        ++xp;

        pixR = srcCur[RGB24_RByte];
        pixG = srcCur[RGB24_GByte];
        pixB = srcCur[RGB24_BByte];
      }
      else
      {
        pixR = rBorderColorR;
        pixG = rBorderColorG;
        pixB = rBorderColorB;
      }

      stackLeft[RGB24_RByte] = pixR;
      stackLeft[RGB24_GByte] = pixG;
      stackLeft[RGB24_BByte] = pixB;

      sumInR += pixR;
      sumInG += pixG;
      sumInB += pixB;

      sumR   += sumInR;
      sumG   += sumInG;
      sumB   += sumInB;

      stackLeft += 3;
      stackRight += 3;

      if (stackLeft == stackEnd) stackLeft = stack;
      if (stackRight == stackEnd) stackRight = stack;

      pixR    = stackRight[RGB24_RByte];
      pixG    = stackRight[RGB24_GByte];
      pixB    = stackRight[RGB24_BByte];

      sumOutR += pixR;
      sumOutG += pixG;
      sumOutB += pixB;

      sumInR  -= pixR;
      sumInG  -= pixG;
      sumInB  -= pixB;
    }

    src += srcStride;
    dst += dstStride;
  }
}

static void FOG_FASTCALL stackBlurConvolveV_rgb24(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int radius,
  int borderMode, uint32_t borderColor)
{
  if (radius == 0 || height < 2)
  {
    if (dst != src) functionMap->filters.copyArea[Image::FormatRGB24](dst, dstStride, src, srcStride, width, height);
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

  uint sumMul = functionMap->filters.stackBlur8Mul[radius];
  uint sumShr = functionMap->filters.stackBlur8Shr[radius];

  uint8_t stack[512*3];
  uint8_t* stackEnd = stack + (radius * 2 + 1) * 3;
  uint8_t* stackLeft;
  uint8_t* stackRight;

  uint32_t lBorderColor = borderColor;
  uint32_t rBorderColor = borderColor;

  for (pos2 = 0; pos2 < dym2; pos2++)
  {
    uint32_t pixR;
    uint32_t pixG;
    uint32_t pixB;

    uint32_t sumR = 0;
    uint32_t sumG = 0;
    uint32_t sumB = 0;

    uint32_t sumInR = 0;
    uint32_t sumInG = 0;
    uint32_t sumInB = 0;

    uint32_t sumOutR = 0;
    uint32_t sumOutG = 0;
    uint32_t sumOutB = 0;

    uint32_t lBorderColorR;
    uint32_t lBorderColorG;
    uint32_t lBorderColorB;

    uint32_t rBorderColorR;
    uint32_t rBorderColorG;
    uint32_t rBorderColorB;

    srcCur = src;

    if (borderMode == ImageFilter::ExtendBorderMode)
    {
      lBorderColorR = srcCur[RGB24_RByte];
      lBorderColorG = srcCur[RGB24_GByte];
      lBorderColorB = srcCur[RGB24_BByte];

      rBorderColorR = srcCur[end + RGB24_RByte];
      rBorderColorG = srcCur[end + RGB24_GByte];
      rBorderColorB = srcCur[end + RGB24_BByte];
    }
    else
    {
      lBorderColorR = (borderColor >> 16) & 0xFF;
      lBorderColorG = (borderColor >>  8) & 0xFF;
      lBorderColorB = (borderColor      ) & 0xFF;

      rBorderColorR = lBorderColorR;
      rBorderColorG = lBorderColorG;
      rBorderColorB = lBorderColorB;
    }

    pixR = lBorderColorR;
    pixG = lBorderColorG;
    pixB = lBorderColorB;

    stackLeft = stack;

    for (i = 0; i < radius; i++)
    {
      stackLeft[RGB24_RByte] = pixR;
      stackLeft[RGB24_GByte] = pixG;
      stackLeft[RGB24_BByte] = pixB;
      stackLeft += 3;

      sumR            += pixR * (i + 1);
      sumG            += pixG * (i + 1);
      sumB            += pixB * (i + 1);

      sumOutR         += pixR;
      sumOutG         += pixG;
      sumOutB         += pixB;
    }

    pixR = srcCur[RGB24_RByte];
    pixG = srcCur[RGB24_GByte];
    pixB = srcCur[RGB24_BByte];

    stackLeft[RGB24_RByte] = pixR;
    stackLeft[RGB24_GByte] = pixG;
    stackLeft[RGB24_BByte] = pixB;
    stackLeft += 3;

    sumR            += pixR * (radius + 1);
    sumG            += pixG * (radius + 1);
    sumB            += pixB * (radius + 1);

    sumOutR         += pixR;
    sumOutG         += pixG;
    sumOutB         += pixB;

    for (i = 1; i <= radius; i++)
    {
      if (i <= max) 
      {
        srcCur += srcStride;
        pixR = srcCur[RGB24_RByte];
        pixG = srcCur[RGB24_GByte];
        pixB = srcCur[RGB24_BByte];
      }
      else 
      {
        pixR = rBorderColorR;
        pixG = rBorderColorG;
        pixB = rBorderColorB;
      }

      stackLeft[RGB24_RByte] = pixR;
      stackLeft[RGB24_GByte] = pixG;
      stackLeft[RGB24_BByte] = pixB;
      stackLeft += 3;

      sumR            += pixR * (radius + 1 - i);
      sumG            += pixG * (radius + 1 - i);
      sumB            += pixB * (radius + 1 - i);

      sumInR          += pixR;
      sumInG          += pixG;
      sumInB          += pixB;
    }

    xp = fog_min(radius, max);

    srcCur = src + xp * srcStride;
    dstCur = dst;

    stackLeft = stack;
    stackRight = stack + radius * 3;

    for (pos1 = 0; pos1 < dym1; pos1++)
    {
      dstCur[RGB24_RByte] = ((sumR * sumMul) >> sumShr);
      dstCur[RGB24_GByte] = ((sumG * sumMul) >> sumShr);
      dstCur[RGB24_BByte] = ((sumB * sumMul) >> sumShr);
      dstCur += dstStride;

      sumR -= sumOutR;
      sumG -= sumOutG;
      sumB -= sumOutB;

      sumOutR -= stackLeft[RGB24_RByte];
      sumOutG -= stackLeft[RGB24_GByte];
      sumOutB -= stackLeft[RGB24_BByte];

      if (xp < max) 
      {
        srcCur += srcStride;
        ++xp;

        pixR = srcCur[RGB24_RByte];
        pixG = srcCur[RGB24_GByte];
        pixB = srcCur[RGB24_BByte];
      }
      else
      {
        pixR = rBorderColorR;
        pixG = rBorderColorG;
        pixB = rBorderColorB;
      }

      stackLeft[RGB24_RByte] = pixR;
      stackLeft[RGB24_GByte] = pixG;
      stackLeft[RGB24_BByte] = pixB;

      sumInR += pixR;
      sumInG += pixG;
      sumInB += pixB;

      sumR   += sumInR;
      sumG   += sumInG;
      sumB   += sumInB;

      stackLeft += 3;
      stackRight += 3;

      if (stackLeft == stackEnd) stackLeft = stack;
      if (stackRight == stackEnd) stackRight = stack;

      pixR    = stackRight[RGB24_RByte];
      pixG    = stackRight[RGB24_GByte];
      pixB    = stackRight[RGB24_BByte];

      sumOutR += pixR;
      sumOutG += pixG;
      sumOutB += pixB;

      sumInR  -= pixR;
      sumInG  -= pixG;
      sumInB  -= pixB;
    }

    src += 3;
    dst += 3;
  }
}

static void FOG_FASTCALL stackBlurConvolveH_a8(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int radius,
  int borderMode, uint32_t borderColor)
{
  if (radius == 0 || width < 2)
  {
    if (dst != src) functionMap->filters.copyArea[Image::FormatA8](dst, dstStride, src, srcStride, width, height);
    return;
  }

  if (radius > 254) radius = 254;

  sysint_t dym1 = width;
  sysint_t dym2 = height;
  sysint_t max = dym1 - 1;
  sysint_t end = max;

  uint8_t* dstCur;
  const uint8_t* srcCur;

  sysint_t pos1;
  sysint_t pos2;
  sysint_t xp, i;

  uint sumMul = functionMap->filters.stackBlur8Mul[radius];
  uint sumShr = functionMap->filters.stackBlur8Shr[radius];

  uint8_t stack[512];
  uint8_t* stackEnd = stack + (radius * 2 + 1);
  uint8_t* stackLeft;
  uint8_t* stackRight;

  uint32_t lBorderColor = borderColor;
  uint32_t rBorderColor = borderColor;

  for (pos2 = 0; pos2 < dym2; pos2++)
  {
    uint32_t pix;
    uint32_t sum = 0;
    uint32_t sumIn = 0;
    uint32_t sumOut = 0;

    srcCur = src;

    if (borderMode == ImageFilter::ExtendBorderMode)
    {
      lBorderColor = srcCur[0];
      rBorderColor = srcCur[end];
    }

    pix = lBorderColor;

    stackLeft = stack;

    for (i = 0; i < radius; i++)
    {
      stackLeft[0] = pix;
      stackLeft += 1;

      sum            += pix * (i + 1);
      sumOut         += pix;
    }

    pix = srcCur[0];

    stackLeft[0] = pix;
    stackLeft += 1;

    sum            += pix * (radius + 1);
    sumOut         += pix;

    for (i = 1; i <= radius; i++)
    {
      if (i <= max) 
      {
        srcCur += 1;
        pix = srcCur[0];
      }
      else 
      {
        pix = rBorderColor;
      }

      stackLeft[0] = pix;
      stackLeft += 1;

      sum            += pix * (radius + 1 - i);
      sumIn          += pix;
    }

    xp = fog_min(radius, max);

    srcCur = src + xp;
    dstCur = dst;

    stackLeft = stack;
    stackRight = stack + radius;

    for (pos1 = 0; pos1 < dym1; pos1++)
    {
      dstCur[0] = ((sum * sumMul) >> sumShr);
      dstCur += 1;

      sum -= sumOut;
      sumOut -= stackLeft[0];

      if (xp < max) 
      {
        srcCur += 1;
        ++xp;

        pix = srcCur[0];
      }
      else
      {
        pix = rBorderColor;
      }

      stackLeft[0] = pix;

      sumIn += pix;
      sum   += sumIn;

      stackLeft += 1;
      stackRight += 1;

      if (stackLeft == stackEnd) stackLeft = stack;
      if (stackRight == stackEnd) stackRight = stack;

      pix    = stackRight[0];

      sumOut += pix;
      sumIn  -= pix;
    }

    src += srcStride;
    dst += dstStride;
  }
}

static void FOG_FASTCALL stackBlurConvolveV_a8(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int radius,
  int borderMode, uint32_t borderColor)
{
  if (radius == 0 || height < 2)
  {
    if (dst != src) functionMap->filters.copyArea[Image::FormatA8](dst, dstStride, src, srcStride, width, height);
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

  uint sumMul = functionMap->filters.stackBlur8Mul[radius];
  uint sumShr = functionMap->filters.stackBlur8Shr[radius];

  uint8_t stack[512];
  uint8_t* stackEnd = stack + (radius * 2 + 1);
  uint8_t* stackLeft;
  uint8_t* stackRight;

  uint32_t lBorderColor = borderColor;
  uint32_t rBorderColor = borderColor;

  for (pos2 = 0; pos2 < dym2; pos2++)
  {
    uint32_t pix;
    uint32_t sum = 0;
    uint32_t sumIn = 0;
    uint32_t sumOut = 0;

    srcCur = src;

    if (borderMode == ImageFilter::ExtendBorderMode)
    {
      lBorderColor = srcCur[0];
      rBorderColor = srcCur[end];
    }
    else
    {
      lBorderColor = (borderColor >> 24) & 0xFF;
      rBorderColor = lBorderColor;
    }

    pix = lBorderColor;
    stackLeft = stack;

    for (i = 0; i <= radius; i++)
    {
      stackLeft[0] = pix;
      stackLeft += 1;

      sum            += pix * (i + 1);
      sumOut         += pix;
    }

    pix = srcCur[0];

    stackLeft[0] = pix;
    stackLeft += 1;

    sum            += pix * (radius + 1);
    sumOut         += pix;

    for (i = 1; i < radius; i++)
    {
      if (i <= max) 
      {
        srcCur += srcStride;
        pix = srcCur[0];
      }
      else 
      {
        pix = rBorderColor;
      }

      stackLeft[0] = pix;
      stackLeft += 1;

      sum            += pix * (radius + 1 - i);
      sumIn          += pix;
    }

    xp = fog_min(radius, max);

    srcCur = src + xp * srcStride;
    dstCur = dst;

    stackLeft = stack;
    stackRight = stack + radius;

    for (pos1 = 0; pos1 < dym1; pos1++)
    {
      dstCur[0] = ((sum * sumMul) >> sumShr);
      dstCur += dstStride;

      sum -= sumOut;
      sumOut -= stackLeft[0];

      if (xp < max) 
      {
        srcCur += srcStride;
        ++xp;

        pix = srcCur[0];
      }
      else
      {
        pix = rBorderColor;
      }

      stackLeft[0] = pix;

      sumIn += pix;
      sum   += sumIn;

      stackLeft += 1;
      stackRight += 1;

      if (stackLeft == stackEnd) stackLeft = stack;
      if (stackRight == stackEnd) stackRight = stack;

      pix    = stackRight[0];
      sumOut += pix;
      sumIn  -= pix;
    }

    src += 1;
    dst += 1;
  }
}

} // Raster namespace
} // Fog namespace
