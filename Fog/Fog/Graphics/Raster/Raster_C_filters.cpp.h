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
// [Fog::Raster_C::transpose]
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
// [Fog::Raster_C::floatScanlineConvolve]
// ============================================================================

static void FOG_FASTCALL floatScanlineConvolve_argb32(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, const float* kernel, int size, float divide,
  int borderMode, uint32_t borderColor)
{
  if (kernel == NULL || size == 0)
  {
    transpose_32(dst, dstStride, src, srcStride, width, height);
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
  if (divide != 1.0f)
  {
    float *k = (float*)kernelTemporary.alloc(sizeof(float) * size);
    if (!k) return;

    for (i = 0; i < size; i++) k[i] = kernel[i] / divide;
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
  int width, int height, const float* kernel, int size, float divide,
  int borderMode, uint32_t borderColor)
{
  if (kernel == NULL || size == 0)
  {
    transpose_24(dst, dstStride, src, srcStride, width, height);
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
  if (divide != 1.0f)
  {
    float *k = (float*)kernelTemporary.alloc(sizeof(float) * size);
    if (!k) return;

    for (i = 0; i < size; i++) k[i] = kernel[i] / divide;
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
  int width, int height, const float* kernel, int size, float divide,
  int borderMode, uint32_t borderColor)
{
  if (kernel == NULL || size == 0)
  {
    transpose_8(dst, dstStride, src, srcStride, width, height);
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
  if (divide != 1.0f)
  {
    float *k = (float*)kernelTemporary.alloc(size * sizeof(float));
    if (!k) return;

    for (i = 0; i < size; i++) k[i] = kernel[i] / divide;
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
// [Fog::Raster_C::boxBlur]
// ============================================================================

static void FOG_FASTCALL boxBlurConvolve_argb32(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int radius,
  int borderMode, uint32_t borderColor)
{
  if (radius == 0)
  {
    transpose_32(dst, dstStride, src, srcStride, width, height);
    return;
  }

  uint8_t* dstCur;
  const uint8_t* srcCur;

  // Setup box blur mask.
  int radius2 = radius * 2;
  int size = radius2 + 1;
  int widthMax = width - radius - 1;

  // Accumulate loop sizes.
  int accumulateUnderflow = radius;
  int accumulateLoopSize = fog_min(width, radius + 1);
  int accumulateOverflow = radius + 1 - accumulateLoopSize;

  // Hot inner loops.
  bool tooSmallImage = width <= size;
  int firstLoopStop = (tooSmallImage) ? width : radius;
  int secondLoopStop = (tooSmallImage) ? 0 : (width - radius);

  sysint_t i, j;
  sysint_t x, y;

  uint32_t reciprocal = getReciprocal(size);

  uint32_t leftEdgeR = (borderColor >> 16) & 0xFF;
  uint32_t leftEdgeG = (borderColor >>  8) & 0xFF;
  uint32_t leftEdgeB = (borderColor      ) & 0xFF;
  uint32_t leftEdgeA = (borderColor >> 24);

  uint32_t rightEdgeR = leftEdgeR;
  uint32_t rightEdgeG = leftEdgeG;
  uint32_t rightEdgeB = leftEdgeB;
  uint32_t rightEdgeA = leftEdgeA;

  for (y = 0; y < height; y++)
  {
    // Accumulators.
    uint32_t cr, cg;
    uint32_t cb, ca;

    // Setup raster pointers.
    dstCur = dst;
    srcCur = src;

    // Setup borders if needed.
    if (borderMode == ImageFilter::ExtendBorderMode)
    {
      uint32_t pix;

      pix = ((const uint32_t*)srcCur)[0];
      leftEdgeR = (pix >> 16) & 0xFF;
      leftEdgeG = (pix >>  8) & 0xFF;
      leftEdgeB = (pix      ) & 0xFF;
      leftEdgeA = (pix >> 24);

      pix = ((const uint32_t*)srcCur)[width-1];
      rightEdgeR = (pix >> 16) & 0xFF;
      rightEdgeG = (pix >>  8) & 0xFF;
      rightEdgeB = (pix      ) & 0xFF;
      rightEdgeA = (pix >> 24);
    }

    // Accumulate left border values.
    cr = leftEdgeR * accumulateUnderflow;
    cg = leftEdgeG * accumulateUnderflow;
    cb = leftEdgeB * accumulateUnderflow;
    ca = leftEdgeA * accumulateUnderflow;

    // Accumulate inner pixels.
    // In normal case accumulateLoopSize is equal to radius+1.
    for (i = 0; i < accumulateLoopSize; i++)
    {
      uint32_t pix = ((const uint32_t*)srcCur)[i];
      cr += (pix >> 16) & 0xFF;
      cg += (pix >>  8) & 0xFF;
      cb += (pix      ) & 0xFF;
      ca += (pix >> 24);
    }

    // Accumulate right border values. 
    // This happen only if blur radius is larger or equal as width.
    cr += rightEdgeR * accumulateOverflow;
    cg += rightEdgeG * accumulateOverflow;
    cb += rightEdgeB * accumulateOverflow;
    ca += rightEdgeA * accumulateOverflow;

    x = 0;
    j = firstLoopStop;

    // Loop with bound checking supporting borders.
again:
    while (x < j)
    {
      ((uint32_t*)dstCur)[0] = 
        (((cr * reciprocal) & 0x00FF0000)      ) | 
        (((cg * reciprocal) & 0x00FF0000) >>  8) |
        (((cb * reciprocal) & 0x00FF0000) >> 16) |
        (((ca * reciprocal) & 0x00FF0000) <<  8) ;

      if (x >= radius)
      {
        uint32_t pix = ((const uint32_t*)srcCur)[-radius];
        cr -= (pix >> 16) & 0xFF;
        cg -= (pix >>  8) & 0xFF;
        cb -= (pix      ) & 0xFF;
        ca -= (pix >> 24);
      }
      else
      {
        cr -= leftEdgeR;
        cg -= leftEdgeG;
        cb -= leftEdgeB;
        ca -= leftEdgeA;
      }

      if (x < widthMax)
      {
        uint32_t pix = ((const uint32_t*)srcCur)[radius+1];
        cr += (pix >> 16) & 0xFF;
        cg += (pix >>  8) & 0xFF;
        cb += (pix      ) & 0xFF;
        ca += (pix >> 24);
      }
      else
      {
        cr += rightEdgeR;
        cg += rightEdgeG;
        cb += rightEdgeB;
        ca += rightEdgeA;
      }

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
        uint32_t pix;

        ((uint32_t*)dstCur)[0] = 
          (((cr * reciprocal) & 0x00FF0000)      ) | 
          (((cg * reciprocal) & 0x00FF0000) >>  8) |
          (((cb * reciprocal) & 0x00FF0000) >> 16) |
          (((ca * reciprocal) & 0x00FF0000) <<  8) ;

        pix = ((const uint32_t*)srcCur)[-radius];
        cr -= (pix >> 16) & 0xFF;
        cg -= (pix >>  8) & 0xFF;
        cb -= (pix      ) & 0xFF;
        ca -= (pix >> 24);

        pix = ((const uint32_t*)srcCur)[radius+1];
        cr += (pix >> 16) & 0xFF;
        cg += (pix >>  8) & 0xFF;
        cb += (pix      ) & 0xFF;
        ca += (pix >> 24);

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

static void FOG_FASTCALL boxBlurConvolve_rgb24(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int radius,
  int borderMode, uint32_t borderColor)
{
  if (radius == 0)
  {
    transpose_32(dst, dstStride, src, srcStride, width, height);
    return;
  }

  uint8_t* dstCur;
  const uint8_t* srcCur;

  // Setup box blur mask.
  int radius2 = radius * 2;
  int size = radius2 + 1;
  int widthMax = width - radius - 1;

  // Accumulate loop sizes.
  int accumulateUnderflow = radius;
  int accumulateLoopSize = fog_min(width, radius + 1);
  int accumulateOverflow = radius + 1 - accumulateLoopSize;

  // Hot inner loops.
  bool tooSmallImage = width <= size;
  int firstLoopStop = (tooSmallImage) ? width : radius;
  int secondLoopStop = (tooSmallImage) ? 0 : (width - radius);

  sysint_t i, j;
  sysint_t x, y;

  uint32_t reciprocal = getReciprocal(size);

  uint32_t leftEdgeR = (borderColor >> 16) & 0xFF;
  uint32_t leftEdgeG = (borderColor >>  8) & 0xFF;
  uint32_t leftEdgeB = (borderColor      ) & 0xFF;

  uint32_t rightEdgeR = leftEdgeR;
  uint32_t rightEdgeG = leftEdgeG;
  uint32_t rightEdgeB = leftEdgeB;

  for (y = 0; y < height; y++)
  {
    // Accumulators.
    uint32_t cr, cg, cb;

    // Setup raster pointers.
    dstCur = dst;
    srcCur = src;

    // Setup borders if needed.
    if (borderMode == ImageFilter::ExtendBorderMode)
    {
      leftEdgeR = srcCur[Raster::RGB24_RByte];
      leftEdgeG = srcCur[Raster::RGB24_GByte];
      leftEdgeB = srcCur[Raster::RGB24_BByte];

      rightEdgeR = srcCur[(width-1)*3 + Raster::RGB24_RByte];
      rightEdgeG = srcCur[(width-1)*3 + Raster::RGB24_GByte];
      rightEdgeB = srcCur[(width-1)*3 + Raster::RGB24_BByte];
    }

    // Accumulate left border values.
    cr = leftEdgeR * accumulateUnderflow;
    cg = leftEdgeG * accumulateUnderflow;
    cb = leftEdgeB * accumulateUnderflow;

    // Accumulate inner pixels.
    // In normal case accumulateLoopSize is equal to radius+1.
    {
      const uint8_t* srcCurInner = srcCur;
      for (i = 0; i < accumulateLoopSize; i++, srcCurInner += 3)
      {
        cr = srcCurInner[Raster::RGB24_RByte];
        cg = srcCurInner[Raster::RGB24_GByte];
        cb = srcCurInner[Raster::RGB24_BByte];
      }
    }

    // Accumulate right border values.
    // This happen only if blur radius is larger or equal as width.
    cr += rightEdgeR * accumulateOverflow;
    cg += rightEdgeG * accumulateOverflow;
    cb += rightEdgeB * accumulateOverflow;

    x = 0;
    j = firstLoopStop;

    // Loop with bound checking supporting borders.
again:
    while (x < j)
    {
      dstCur[Raster::RGB24_RByte] = (cr * reciprocal) >> 16;
      dstCur[Raster::RGB24_GByte] = (cg * reciprocal) >> 16;
      dstCur[Raster::RGB24_BByte] = (cb * reciprocal) >> 16;

      if (x >= radius)
      {
        const uint8_t* srcCurInner = srcCur - radius*3;
        cr -= srcCurInner[Raster::RGB24_RByte];
        cg -= srcCurInner[Raster::RGB24_GByte];
        cb -= srcCurInner[Raster::RGB24_BByte];
      }
      else
      {
        cr -= leftEdgeR;
        cg -= leftEdgeG;
        cb -= leftEdgeB;
      }

      if (x < widthMax)
      {
        const uint8_t* srcCurInner = srcCur + ((radius+1)*3);
        cr += srcCurInner[Raster::RGB24_RByte];
        cg += srcCurInner[Raster::RGB24_GByte];
        cb += srcCurInner[Raster::RGB24_BByte];
      }
      else
      {
        cr += rightEdgeR;
        cg += rightEdgeG;
        cb += rightEdgeB;
      }

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
        dstCur[Raster::RGB24_RByte] = (cr * reciprocal) >> 16;
        dstCur[Raster::RGB24_GByte] = (cg * reciprocal) >> 16;
        dstCur[Raster::RGB24_BByte] = (cb * reciprocal) >> 16;

        const uint8_t* srcCurInner;

        srcCurInner = srcCur - radius*3;
        cr -= srcCurInner[Raster::RGB24_RByte];
        cg -= srcCurInner[Raster::RGB24_GByte];
        cb -= srcCurInner[Raster::RGB24_BByte];

        srcCurInner = srcCur + ((radius+1)*3);
        cr += srcCurInner[Raster::RGB24_RByte];
        cg += srcCurInner[Raster::RGB24_GByte];
        cb += srcCurInner[Raster::RGB24_BByte];

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

static void FOG_FASTCALL boxBlurConvolve_a8(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int radius,
  int borderMode, uint32_t borderColor)
{
  if (radius == 0)
  {
    transpose_32(dst, dstStride, src, srcStride, width, height);
    return;
  }

  uint8_t* dstCur;
  const uint8_t* srcCur;

  // Setup box blur mask.
  int radius2 = radius * 2;
  int size = radius2 + 1;
  int widthMax = width - radius - 1;

  // Accumulate loop sizes.
  int accumulateUnderflow = radius;
  int accumulateLoopSize = fog_min(width, radius + 1);
  int accumulateOverflow = radius + 1 - accumulateLoopSize;

  // Hot inner loops.
  bool tooSmallImage = width <= size;
  int firstLoopStop = (tooSmallImage) ? width : radius;
  int secondLoopStop = (tooSmallImage) ? 0 : (width - radius);

  sysint_t i, j;
  sysint_t x, y;

  uint32_t reciprocal = getReciprocal(size);

  uint32_t leftEdge = borderColor & 0xFF;
  uint32_t rightEdge = leftEdge;

  for (y = 0; y < height; y++)
  {
    // Accumulators.
    uint32_t ca;

    // Setup raster pointers.
    dstCur = dst;
    srcCur = src;

    // Setup borders if needed.
    if (borderMode == ImageFilter::ExtendBorderMode)
    {
      leftEdge = srcCur[0];
      rightEdge = srcCur[width-1];
    }

    // Accumulate left border values.
    ca = leftEdge * accumulateUnderflow;

    // Accumulate inner pixels.
    // In normal case accumulateLoopSize is equal to radius+1.
    {
      const uint8_t* srcCurInner = srcCur;
      for (i = 0; i < accumulateLoopSize; i++, srcCurInner++)
      {
        ca = srcCurInner[0];
      }
    }

    // Accumulate right border values.
    // This happen only if blur radius is larger or equal as width.
    ca += rightEdge * accumulateOverflow;

    x = 0;
    j = firstLoopStop;

    // Loop with bound checking supporting borders.
again:
    while (x < j)
    {
      dstCur[0] = (ca * reciprocal) >> 16;

      if (x >= radius)
      {
        const uint8_t* srcCurInner = srcCur - radius;
        ca -= srcCurInner[0];
      }
      else
      {
        ca -= leftEdge;
      }

      if (x < widthMax)
      {
        const uint8_t* srcCurInner = srcCur + (radius+1);
        ca += srcCurInner[0];
      }
      else
      {
        ca += rightEdge;
      }

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
        dstCur[0] = (ca * reciprocal) >> 16;

        ca -= srcCur[-radius];

        dstCur += dstStride;
        srcCur += 1;

        ca += srcCur[radius];

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
// [Fog::Raster_C::stackBlur]
// ============================================================================

static const uint16_t stack_blur8_mul[255] = 
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

static const uint8_t stack_blur8_shr[255] = 
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

static void FOG_FASTCALL stackBlurConvolve_argb32(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int radius,
  int borderMode, uint32_t borderColor)
{
  if (radius == 0)
  {
    transpose_32(dst, dstStride, src, srcStride, width, height);
    return;
  }

  uint x, y, xp, i;
  uint stackPtr;
  uint stackStart;

  uint8_t* dstCur;
  const uint8_t* srcCur;
  uint32_t* stackCur;

  uint w   = (uint)width;
  uint h   = (uint)height;
  uint wm  = w - 1;
  uint hm  = h - 1;

  uint div;
  uint sumMul;
  uint sumShr;

  if (radius > 254) radius = 254;
  div = radius * 2 + 1;
  sumMul = stack_blur8_mul[radius];
  sumShr = stack_blur8_shr[radius];

  uint32_t stack[512];
  uint32_t leftEdgeColor = borderColor;
  uint32_t rightEdgeColor = borderColor;

  for (y = 0; y < h; y++)
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
      leftEdgeColor = ((const uint32_t*)srcCur)[0];
      rightEdgeColor = ((const uint32_t*)srcCur)[wm];
    }

    pix0 = leftEdgeColor;
    pixR = (pix0 >> 16) & 0xFF;
    pixG = (pix0 >>  8) & 0xFF;
    pixB = (pix0      ) & 0xFF;
    pixA = (pix0 >> 24);

    for (i = 0; i <= (uint)radius; i++)
    {
      stackCur    = &stack[i];
      stackCur[0] = pix0;

      sumR            += pixR * (i + 1);
      sumG            += pixG * (i + 1);
      sumB            += pixB * (i + 1);
      sumA            += pixA * (i + 1);

      sumOutR         += pixR;
      sumOutG         += pixG;
      sumOutB         += pixB;
      sumOutA         += pixA;
    }

    for (i = 1; i <= (uint)radius; i++)
    {
      if (i <= wm) 
      {
        srcCur += 4;
        pix0 = ((const uint32_t*)srcCur)[0];
      }
      else 
      {
        pix0 = rightEdgeColor;
      }

      pixR = (pix0 >> 16) & 0xFF;
      pixG = (pix0 >>  8) & 0xFF;
      pixB = (pix0      ) & 0xFF;
      pixA = (pix0 >> 24);

      stackCur    = &stack[i + radius];
      stackCur[0] = pix0;

      sumR            += pixR * (radius + 1 - i);
      sumG            += pixG * (radius + 1 - i);
      sumB            += pixB * (radius + 1 - i);
      sumA            += pixA * (radius + 1 - i);

      sumInR          += pixR;
      sumInG          += pixG;
      sumInB          += pixB;
      sumInA          += pixA;
    }

    stackPtr = radius;
    xp = radius;
    if (xp > wm) xp = wm;

    srcCur = src + xp * 4;
    dstCur = dst;

    for (x = 0; x < w; x++)
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

      stackStart = stackPtr + div - radius;
      if (stackStart >= div) stackStart -= div;
      stackCur = &stack[stackStart];

      pix0 = stackCur[0];
      sumOutR -= (pix0 >> 16) & 0xFF;
      sumOutG -= (pix0 >>  8) & 0xFF;
      sumOutB -= (pix0      ) & 0xFF;
      sumOutA -= (pix0 >> 24);

      if (xp < wm) 
      {
        srcCur += 4;
        ++xp;
        pix0 = ((const uint32_t*)srcCur)[0];
      }
      else
      {
        pix0 = rightEdgeColor;
      }

      pixR    = (pix0 >> 16) & 0xFF;
      pixG    = (pix0 >>  8) & 0xFF;
      pixB    = (pix0      ) & 0xFF;
      pixA    = (pix0 >> 24);

      stackCur[0] = pix0;

      sumInR += pixR;
      sumInG += pixG;
      sumInB += pixB;
      sumInA += pixA;

      sumR   += sumInR;
      sumG   += sumInG;
      sumB   += sumInB;
      sumA   += sumInA;

      ++stackPtr;
      if(stackPtr >= div) stackPtr = 0;
      stackCur = &stack[stackPtr];

      pix0    = stackCur[0];
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
    dst += 4;
  }
}

static void FOG_FASTCALL stackBlurConvolve_rgb24(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int radius,
  int borderMode, uint32_t borderColor)
{
  if (radius == 0)
  {
    transpose_24(dst, dstStride, src, srcStride, width, height);
    return;
  }

  uint x, y, xp, i;
  uint stackPtr;
  uint stackStart;

  uint8_t* dstCur;
  const uint8_t* srcCur;
  uint8_t* stackCur;

  uint w   = (uint)width;
  uint h   = (uint)height;
  uint wm  = w - 1;
  uint hm  = h - 1;

  uint div;
  uint sumMul;
  uint sumShr;

  if (radius > 254) radius = 254;
  div = radius * 2 + 1;
  sumMul = stack_blur8_mul[radius];
  sumShr = stack_blur8_shr[radius];

  uint8_t stack[512*3];

  for (y = 0; y < h; y++)
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

    uint32_t leftEdgeColorR;
    uint32_t leftEdgeColorG;
    uint32_t leftEdgeColorB;

    uint32_t rightEdgeColorR;
    uint32_t rightEdgeColorG;
    uint32_t rightEdgeColorB;

    srcCur = src;

    if (borderMode == ImageFilter::ExtendBorderMode)
    {
      leftEdgeColorR = srcCur[RGB24_RByte];
      leftEdgeColorG = srcCur[RGB24_GByte];
      leftEdgeColorB = srcCur[RGB24_BByte];

      rightEdgeColorR = srcCur[wm*3 + RGB24_RByte];
      rightEdgeColorG = srcCur[wm*3 + RGB24_GByte];
      rightEdgeColorB = srcCur[wm*3 + RGB24_BByte];
    }
    else
    {
      leftEdgeColorR = (borderColor >> 16) & 0xFF;
      leftEdgeColorG = (borderColor >>  8) & 0xFF;
      leftEdgeColorB = (borderColor      ) & 0xFF;

      rightEdgeColorR = leftEdgeColorR;
      rightEdgeColorG = leftEdgeColorG;
      rightEdgeColorB = leftEdgeColorB;
    }

    pixR = leftEdgeColorR;
    pixG = leftEdgeColorG;
    pixB = leftEdgeColorB;

    for (i = 0; i <= (uint)radius; i++)
    {
      stackCur    = &stack[i*3];
      stackCur[RGB24_RByte] = pixR;
      stackCur[RGB24_GByte] = pixG;
      stackCur[RGB24_BByte] = pixB;

      sumR            += pixR * (i + 1);
      sumG            += pixG * (i + 1);
      sumB            += pixB * (i + 1);

      sumOutR         += pixR;
      sumOutG         += pixG;
      sumOutB         += pixB;
    }

    for (i = 1; i <= (uint)radius; i++)
    {
      if (i <= wm) 
      {
        srcCur += 3;
        pixR = srcCur[RGB24_RByte];
        pixG = srcCur[RGB24_GByte];
        pixB = srcCur[RGB24_BByte];
      }
      else 
      {
        pixR = rightEdgeColorR;
        pixG = rightEdgeColorG;
        pixB = rightEdgeColorB;
      }

      stackCur    = &stack[(i + radius) * 3];
      stackCur[RGB24_RByte] = pixR;
      stackCur[RGB24_GByte] = pixG;
      stackCur[RGB24_BByte] = pixB;

      sumR            += pixR * (radius + 1 - i);
      sumG            += pixG * (radius + 1 - i);
      sumB            += pixB * (radius + 1 - i);

      sumInR          += pixR;
      sumInG          += pixG;
      sumInB          += pixB;
    }

    stackPtr = radius;
    xp = radius;
    if (xp > wm) xp = wm;

    srcCur = src + xp * 3;
    dstCur = dst;

    for (x = 0; x < w; x++)
    {
      dstCur[RGB24_RByte] = ((sumR * sumMul) >> sumShr);
      dstCur[RGB24_GByte] = ((sumG * sumMul) >> sumShr);
      dstCur[RGB24_BByte] = ((sumB * sumMul) >> sumShr);
      dstCur += dstStride;

      sumR -= sumOutR;
      sumG -= sumOutG;
      sumB -= sumOutB;

      stackStart = stackPtr + div - radius;
      if (stackStart >= div) stackStart -= div;
      stackCur = &stack[stackStart*3];

      sumOutR -= stackCur[RGB24_RByte];
      sumOutG -= stackCur[RGB24_GByte];
      sumOutB -= stackCur[RGB24_BByte];

      if (xp < wm) 
      {
        srcCur += 3;
        pixR = srcCur[RGB24_RByte];
        pixG = srcCur[RGB24_GByte];
        pixB = srcCur[RGB24_BByte];
        ++xp;
      }
      else
      {
        pixR = rightEdgeColorR;
        pixG = rightEdgeColorG;
        pixB = rightEdgeColorB;
      }

      stackCur[RGB24_RByte] = pixR;
      stackCur[RGB24_GByte] = pixG;
      stackCur[RGB24_BByte] = pixB;

      sumInR += pixR;
      sumInG += pixG;
      sumInB += pixB;

      sumR   += sumInR;
      sumG   += sumInG;
      sumB   += sumInB;

      ++stackPtr;
      if (stackPtr >= div) stackPtr = 0;
      stackCur = &stack[stackPtr*3];

      pixR    = stackCur[RGB24_RByte];
      pixG    = stackCur[RGB24_GByte];
      pixB    = stackCur[RGB24_BByte];

      sumOutR += pixR;
      sumOutG += pixG;
      sumOutB += pixB;

      sumInR  -= pixR;
      sumInG  -= pixG;
      sumInB  -= pixB;
    }

    src += srcStride;
    dst += 3;
  }
}

static void FOG_FASTCALL stackBlurConvolve_a8(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int radius,
  int borderMode, uint32_t borderColor)
{
  if (radius == 0)
  {
    transpose_8(dst, dstStride, src, srcStride, width, height);
    return;
  }

  uint x, y, xp, i;
  uint stackPtr;
  uint stackStart;

  uint8_t* dstCur;
  const uint8_t* srcCur;
  uint8_t* stackCur;

  uint w   = (uint)width;
  uint h   = (uint)height;
  uint wm  = w - 1;
  uint hm  = h - 1;

  uint div;
  uint sumMul;
  uint sumShr;

  if (radius > 254) radius = 254;
  div = radius * 2 + 1;
  sumMul = stack_blur8_mul[radius];
  sumShr = stack_blur8_shr[radius];

  uint8_t stack[512];

  for (y = 0; y < h; y++)
  {
    uint32_t pix;
    uint32_t sum = 0;
    uint32_t sumIn = 0;
    uint32_t sumOut = 0;

    uint32_t leftEdgeColor;
    uint32_t rightEdgeColor;

    srcCur = src;

    if (borderMode == ImageFilter::ExtendBorderMode)
    {
      leftEdgeColor = srcCur[0];
      rightEdgeColor = srcCur[wm];
    }
    else
    {
      leftEdgeColor = (borderColor >> 16) & 0xFF;
      rightEdgeColor = leftEdgeColor;
    }

    pix = leftEdgeColor;

    for (i = 0; i <= (uint)radius; i++)
    {
      stackCur    = &stack[i];
      stackCur[0] = pix;

      sum            += pix * (i + 1);
      sumOut         += pix;
    }

    for (i = 1; i <= (uint)radius; i++)
    {
      if (i <= wm) 
      {
        srcCur += 1;
        pix = srcCur[0];
      }
      else 
      {
        pix = rightEdgeColor;
      }

      stackCur    = &stack[i + radius];
      stackCur[0] = pix;

      sum            += pix * (radius + 1 - i);
      sumIn          += pix;
    }

    stackPtr = radius;
    xp = radius;
    if (xp > wm) xp = wm;

    srcCur = src + xp;
    dstCur = dst;

    for (x = 0; x < w; x++)
    {
      dstCur[0] = ((sum * sumMul) >> sumShr);
      dstCur += dstStride;

      sum -= sumOut;

      stackStart = stackPtr + div - radius;
      if (stackStart >= div) stackStart -= div;
      stackCur = &stack[stackStart];

      sumOut -= stackCur[0];

      if (xp < wm) 
      {
        srcCur += 1;
        pix = srcCur[0];
        ++xp;
      }
      else
      {
        pix = rightEdgeColor;
      }

      stackCur[0] = pix;

      sumIn += pix;
      sum   += sumIn;

      ++stackPtr;
      if (stackPtr >= div) stackPtr = 0;
      stackCur = &stack[stackPtr];

      pix    = stackCur[0];

      sumOut += pix;
      sumIn  -= pix;
    }

    src += srcStride;
    dst += 1;
  }
}

} // Raster namespace
} // Fog namespace
