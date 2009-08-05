// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// For some IDEs to enable code-assist.
#include <Fog/Build/Build.h>
#if defined(FOG_IDE)
#include <Fog/Graphics/Raster/Raster_SSE2_base.cpp.h>
#include <Fog/Graphics/Raster/Raster_SSE2_convert.cpp.h>
#endif // FOG_IDE

namespace Fog {
namespace Raster {

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
  i = Math::min(tw - x, w);
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

    i = Math::min(w, tw);
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
      int i = Math::min(tw2 - x, w);

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
      int i = Math::min(tw - x, w);

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

  idxf = _mm_and_ps(cc, Mask_7FFFFFFF7FFFFFFF_7FFFFFFF7FFFFFFF); // abs()
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
    idxf = _mm_and_ps(cc, Mask_7FFFFFFF7FFFFFFF_7FFFFFFF7FFFFFFF); // abs()
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

} // Raster namespace
} // Fog namespace
