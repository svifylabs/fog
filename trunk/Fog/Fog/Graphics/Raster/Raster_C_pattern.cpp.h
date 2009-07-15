// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// For some IDEs to enable code-assist.
#include <Fog/Build/Build.h>
#if defined(FOG_IDE)
#include <Fog/Graphics/Raster/Raster_C_base.cpp.h>
#include <Fog/Graphics/Raster/Raster_C_convert.cpp.h>
#endif // FOG_IDE

namespace Fog {
namespace Raster {

// ============================================================================
// [Fog::Raster - Pattern - Texture]
// ============================================================================

static uint8_t* FOG_FASTCALL pattern_texture_fetch_repeat(
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

  for (;;)
  {
    w -= i;

    do {
      ((uint32_t*)dstCur)[0] = ((const uint32_t*)srcCur)[0];
      dstCur += 4;
      srcCur += 4;
    } while (--i);
    if (!w) break;

    i = Math::min(w, tw);
    srcCur = srcBase;
  }

  return dst;
}

static uint8_t* FOG_FASTCALL pattern_texture_fetch_reflect(
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

      do {
        ((uint32_t*)dstCur)[0] = ((const uint32_t*)srcCur)[0];
        dstCur += 4;
        srcCur += 4;
      } while (--i);
    }
  } while (w);

  return dst;
}

static void FOG_FASTCALL pattern_texture_destroy(
  PatternContext* ctx);

static err_t FOG_FASTCALL pattern_texture_init(
  PatternContext* ctx, const Pattern& pattern)
{
  Pattern::Data* d = pattern._d;
  if (d->type != Pattern::Texture) return Error::InvalidArgument;
  if (d->obj.texture->isEmpty()) return Error::ImageSizeIsZero;

  ctx->texture.texture.init(d->obj.texture.instance());
  ctx->texture.dx = double_to_int(d->points[0].x());
  ctx->texture.dy = double_to_int(d->points[0].y());

  ctx->format = ctx->texture.texture->format();
  ctx->depth = ctx->texture.texture->depth();
  
  // Set fetch function.
  switch (d->spread)
  {
    case Pattern::PadSpread:
    case Pattern::RepeatSpread:
      ctx->fetch = functionMap->pattern.texture_fetch_repeat;
      break;
    case Pattern::ReflectSpread:
      ctx->fetch = functionMap->pattern.texture_fetch_reflect;
      break;
    default:
      return Error::InvalidArgument;
  }

  // Set destroy function.
  ctx->destroy = pattern_texture_destroy;

  // Copy texture variables into pattern context.
  ctx->texture.bits = ctx->texture.texture->cData();
  ctx->texture.stride = ctx->texture.texture->stride();
  ctx->texture.w = ctx->texture.texture->width();
  ctx->texture.h = ctx->texture.texture->height();

  ctx->initialized = true;
  return Error::Ok;
}

static void FOG_FASTCALL pattern_texture_destroy(
  PatternContext* ctx)
{
  FOG_ASSERT(ctx->initialized);

  ctx->texture.texture.destroy();
  ctx->initialized = false;
}

// ============================================================================
// [Fog::Raster - Pattern - Gradient - Generic]
// ============================================================================

static void FOG_FASTCALL gradient_stops(
  uint8_t* dst, const GradientStops& stops,
  GradientSpanFn gradientSpan,
  int offset, int size, int w,
  bool reverse)
{
  // Sanity check.
  FOG_ASSERT(w <= size || offset <= (size - w));

  sysint_t count = (sysint_t)stops.length();
  sysint_t end = offset + w;

  if (count == 0 || w == 0) return;

  if (count == 1 || size == 1)
  {
    //Rgba color = stops.cAt(0).color;
    //render(dst, color, color, 0, w, w, &cpuState);
  }
  else
  {
    sysint_t i = (reverse) ? count - 1 : 0;
    sysint_t iinc = (reverse) ? -1 : 1;

    Rgba primaryStopColor = stops.cAt(i).rgba;
    Rgba secondaryStopRgba;

    double primaryStopOffset = 0.0;
    double secondaryStopOffset;
    long x1 = 0;
    long x2 = 0;

    for (; i < count && (sysint_t)x1 < end; i += iinc,
      primaryStopOffset = secondaryStopOffset,
      primaryStopColor = secondaryStopRgba,
      x1 = x2)
    {
      secondaryStopOffset = stops.cAt(i).offset;
      secondaryStopRgba = stops.cAt(i).rgba;

      // Stop offset can be at range from 0.0 to 1.0 including.
      if (secondaryStopOffset < 0.0) secondaryStopOffset = 0.0;
      if (secondaryStopOffset > 1.0) secondaryStopOffset = 1.0;
      if (reverse) secondaryStopOffset = 1.0 - secondaryStopOffset;

      // Don't trust input data...
      if (secondaryStopOffset < primaryStopOffset) return;

      // Skip all siblings and the first one.
      if (secondaryStopOffset == primaryStopOffset) continue;

      // get pixel coordinates and skip that caller not wants
      x2 = (sysint_t)((double)size * secondaryStopOffset);
      if (x2 < (sysint_t)offset) continue; // not reached the beggining
      if (x2 > (sysint_t)size) return;     // reached the end

      sysint_t cx1 = x1; if (cx1 < (sysint_t)offset) cx1 = (sysint_t)offset;
      sysint_t cx2 = x2; if (cx2 > (sysint_t)end) cx2 = (sysint_t)end;

      if (cx2 - cx1)
      {
        gradientSpan(
          // pointer to destination, it's needed to decrease it by 'offset'
          dst + (sysint_t)(cx1 - offset) * 4,
          // primary and secondary colors
          primaryStopColor, secondaryStopRgba,
          // width, x1, x2
          cx2 - cx1, cx1 - x1, x2 - x1);
      }
    }

    // TODO: draw last point
    // if (size == width) ((uint32_t*)dst)[size-1] = secondaryStopRgba;
  }
}

static err_t FOG_FASTCALL pattern_generic_gradient_init(
  PatternContext* ctx, const GradientStops& stops, sysint_t gLength, bool reflect)
{
  bool hasAlpha = RgbaAnalyzer::analyzeAlpha(stops) != 0xFF;

  // Alloc twice memory for reflect spread.
  sysint_t gAlloc = gLength;
  if (reflect) gAlloc <<= 1;

  // One pixel for interpolation.
  gAlloc += 1;

  // alloc space for pattern or use reserved buffer.
  if ((ctx->genericGradient.colors = (uint32_t*)Memory::alloc(gAlloc << 2)) == NULL)
  {
    return Error::OutOfMemory;
  }

  ctx->format = hasAlpha 
    ? Image::FormatPRGB32
    : Image::FormatRGB32;
  ctx->depth = 32;

  gradient_stops(
    (uint8_t*)ctx->genericGradient.colors, stops,
    hasAlpha ? functionMap->gradient.gradient_prgb32
             : functionMap->gradient.gradient_rgb32,
    0, (int)gLength, (int)gLength, false);

  ctx->genericGradient.colorsAlloc = gAlloc;
  ctx->genericGradient.colorsLength = gAlloc-1;

  // Create mirror for reflect spread.
  if (reflect)
  {
    uint32_t* patternTo = ctx->genericGradient.colors + gLength;
    uint32_t* patternFrom = patternTo - 1;

    size_t i;
    for (i = (size_t)gLength; i; i--, patternTo++, patternFrom--)
    {
      *patternTo = *patternFrom;
    }
  }

  // Interpolation.
  ctx->genericGradient.colors[gAlloc-1] = ctx->genericGradient.colors[0];

  return Error::Ok;
}

static void FOG_FASTCALL pattern_generic_gradient_destroy(
  PatternContext* ctx)
{
  FOG_ASSERT(ctx->initialized);

  Memory::free(ctx->linearGradient.colors);
  ctx->initialized = false;
}

// ============================================================================
// [Fog::Raster - Pattern - Gradient - Linear]
// ============================================================================

static uint8_t* FOG_FASTCALL pattern_linear_gradient_fetch_pad(
  PatternContext* ctx,
  uint8_t* dst, int x, int y, int w)
{
  FOG_ASSERT(w);

  uint8_t* dstCur = dst;

  const uint32_t* colors = (const uint32_t*)ctx->linearGradient.colors;
  sysint_t colorsLength = ctx->linearGradient.colorsLength;

  int64_t cx = ((int64_t)x << 16) - ctx->linearGradient.dx;
  int64_t cy = ((int64_t)y << 16) - ctx->linearGradient.dy;

  int64_t ax = ctx->linearGradient.ax;
  int64_t ay = ctx->linearGradient.ay;

  int64_t yy = (cx * ax + cy * ay) >> 16;
  int64_t yy_max = (int64_t)colorsLength << 16;

  uint32_t color0 = colors[0];
  uint32_t color1 = colors[colorsLength-1];

  // There are two directions of gradient (ax > 0 or ax < 0):
  if (ax < 0)
  {
    while (yy >= yy_max)
    {
      ((uint32_t*)dstCur)[0] = color1;
      if (!(--w)) goto end;

      dstCur += 4;
      yy += ax;
    }

    while (yy >= 0)
    {
      ((uint32_t*)dstCur)[0] = colors[(sysint_t)(yy >> 16)];
      if (!(--w)) goto end;

      dstCur += 4;
      yy += ax;
    }

    do {
      ((uint32_t*)dstCur)[0] = color0;
      if (!(--w)) goto end;
      dstCur += 4;
    } while (true);
  }
  else
  {
    while (yy <= 0)
    {
      ((uint32_t*)dstCur)[0] = color0;
      if (!(--w)) goto end;

      dstCur += 4;
      yy += ax;
    }

    while (yy < yy_max)
    {
      ((uint32_t*)dstCur)[0] = colors[(sysint_t)(yy >> 16)];
      if (!(--w)) goto end;

      dstCur += 4;
      yy += ax;
    }

    do {
      ((uint32_t*)dstCur)[0] = color1;
      if (!(--w)) goto end;
      dstCur += 4;
    } while (true);
  }

end:
  return dst;
}

static uint8_t* FOG_FASTCALL pattern_linear_gradient_fetch_repeat(
  PatternContext* ctx,
  uint8_t* dst, int x, int y, int w)
{
  FOG_ASSERT(w);

  uint8_t* dstCur = dst;

  const uint32_t* colors = (const uint32_t*)ctx->linearGradient.colors;
  sysint_t colorsLength = ctx->linearGradient.colorsLength;

  int64_t cx = ((int64_t)x << 16) - ctx->linearGradient.dx;
  int64_t cy = ((int64_t)y << 16) - ctx->linearGradient.dy;

  int64_t ax = ctx->linearGradient.ax;
  int64_t ay = ctx->linearGradient.ay;

  int64_t yy_max = (int64_t)colorsLength << 16;
  int64_t yy = ((cx * ax + cy * ay) >> 16) % yy_max;

  uint32_t color0 = colors[0];
  uint32_t color1 = colors[colorsLength-1];

  if (yy < 0) yy += yy_max;

  // There are two directions of gradient (ax > 0 or ax < 0):
  if (ax < 0)
  {
    do {
      ((uint32_t*)dstCur)[0] = colors[(sysint_t)(yy >> 16)];

      if (!(--w)) goto end;

      dstCur += 4;
      yy += ax;
      if (yy < 0) yy += yy_max;
    } while (true);
  }
  else
  {
    do {
      ((uint32_t*)dstCur)[0] = colors[(sysint_t)(yy >> 16)];

      if (!(--w)) goto end;

      dstCur += 4;
      yy += ax;
      if (yy >= yy_max) yy -= yy_max;
    } while (true);
  }

end:
  return dst;
}

static err_t FOG_FASTCALL pattern_linear_gradient_init(
  PatternContext* ctx, const Pattern& pattern)
{
  Pattern::Data* d = pattern._d;
  if (d->type != Pattern::LinearGradient) return Error::InvalidArgument;

  double dxd = Math::abs(d->points[1].x() - d->points[0].x());
  double dyd = Math::abs(d->points[1].y() - d->points[0].y());
  double sqrtxxyy = sqrt(dxd * dxd + dyd * dyd);

  int64_t dx = double_to_fixed48x16(dxd);
  int64_t dy = double_to_fixed48x16(dyd);
  int64_t dmax = Math::max(dx, dy);
  sysint_t maxSize = d->obj.gradientStops->length() * 256;

  sysint_t gLength = (sysint_t)((dmax >> 16) << 2);

  if (gLength > maxSize) gLength = maxSize;
  if (gLength > 4096) gLength = 4096;

  double scale = gLength ? sqrtxxyy / (double)gLength : 1;

  ctx->linearGradient.dx = double_to_fixed48x16(d->points[0].x());
  ctx->linearGradient.dy = double_to_fixed48x16(d->points[0].y());

  ctx->linearGradient.ax = double_to_fixed48x16(dxd / (scale * sqrtxxyy));
  ctx->linearGradient.ay = double_to_fixed48x16(dyd / (scale * sqrtxxyy));

  if (d->points[0].x() > d->points[1].x()) ctx->linearGradient.ax = -ctx->linearGradient.ax;
  if (d->points[0].y() > d->points[1].y()) ctx->linearGradient.ay = -ctx->linearGradient.ay;

  err_t err = pattern_generic_gradient_init(ctx, 
    d->obj.gradientStops.instance(), gLength,
    d->spread == Pattern::ReflectSpread);
  if (err) return err;

  // Set fetch function.
  switch (d->spread)
  {
    case Pattern::PadSpread:
      ctx->fetch = functionMap->pattern.linear_gradient_fetch_pad;
      break;
    case Pattern::RepeatSpread:
      ctx->fetch = functionMap->pattern.linear_gradient_fetch_repeat;
      break;
    case Pattern::ReflectSpread:
      ctx->fetch = functionMap->pattern.linear_gradient_fetch_repeat;
      break;
    default:
      FOG_ASSERT_NOT_REACHED();
  }

  // Set destroy function.
  ctx->destroy = pattern_generic_gradient_destroy;

  ctx->initialized = true;
  return Error::Ok;
}

// ============================================================================
// [Fog::Raster - Pattern - Gradient - Radial]
// ============================================================================

/*
  This is reference implementation

static uint8_t* FOG_FASTCALL pattern_radial_gradient_fetch_pad(
  PatternContext* ctx,
  uint8_t* dst, int x, int y, int w)
{
  uint8_t* dstCur = dst;

  const uint32_t* colors = (const uint32_t*)ctx->radialGradient.colors;
  sysint_t colorsLength = ctx->radialGradient.colorsLength;

  uint32_t color0 = colors[0];
  uint32_t color1 = colors[colorsLength-1];

  int index;

  double dx = (double)x - ctx->radialGradient.dx;
  double dy = (double)y - ctx->radialGradient.dy;

  double fx = ctx->radialGradient.fx;
  double fy = ctx->radialGradient.fy;
  double r2 = ctx->radialGradient.r2;
  double scale = ctx->radialGradient.mul;

  double dyfx = dy * fx;
  double dyfy = dy * dy;
  double dydy = dy * dy;

  do {
    double d2 = dx * fy - dyfx;
    double d3 = r2 * (dx * dx + dydy) - d2 * d2;

    index = (int) ((dx * fx + dyfy + sqrt(fabs(d3))) * scale);

    if (index < 0)
      ((uint32_t*)dstCur)[0] = color0;
    else if (index >= colorsLength)
      ((uint32_t*)dstCur)[0] = color1;
    else
      ((uint32_t*)dstCur)[0] = colors[index];

    dstCur += 4;
    dx += 1.0;
  } while (--w);

  return dst;
}
*/

static uint8_t* FOG_FASTCALL pattern_radial_gradient_fetch_pad(
  PatternContext* ctx,
  uint8_t* dst, int x, int y, int w)
{
  FOG_ASSERT(w);

  uint8_t* dstCur = dst;

  const uint32_t* colors = (const uint32_t*)ctx->radialGradient.colors;
  sysint_t colorsLength = ctx->radialGradient.colorsLength;

  uint32_t color0 = colors[0];
  uint32_t color1 = colors[colorsLength-1];

  int index;

  double dx = (double)x - ctx->radialGradient.dx;
  double dy = (double)y - ctx->radialGradient.dy;

  double fx = ctx->radialGradient.fx;
  double fy = ctx->radialGradient.fy;
  double r2 = ctx->radialGradient.r2;
  double scale = ctx->radialGradient.mul;

  double dyfx = dy * fx;
  double dyfy = dy * fy;
  double dydy = dy * dy;

  double dxdx = dx * dx;
  double dxfx = dx * fx;
  double dxfy = dx * fy;

  double dxfx_p_dyfy = dxfx + dyfy;
  double dxfy_m_dyfx = dxfy - dyfx;

  double cc = (dydy + dxdx) * r2 - (dxfy_m_dyfx * dxfy_m_dyfx);
  double cx = (dx * r2)          - (dxfy_m_dyfx * fy);
  double ci = r2                 - (fy * fy);

  double dd = (dxfx_p_dyfy);
  double di = fx;

  do {
    index = (int)((dd + sqrt(fabs(cc))) * scale);

    if (index < 0)
      ((uint32_t*)dstCur)[0] = color0;
    else if (index >= colorsLength)
      ((uint32_t*)dstCur)[0] = color1;
    else
      ((uint32_t*)dstCur)[0] = colors[index];
    dstCur += 4;

    // cc += cx + cx + ci
    // cx += ci
    cc += cx;
    cx += ci;
    cc += cx;

    dd += di;
  } while (--w);

  return dst;
}

static uint8_t* FOG_FASTCALL pattern_radial_gradient_fetch_repeat(
  PatternContext* ctx,
  uint8_t* dst, int x, int y, int w)
{
  FOG_ASSERT(w);

  uint8_t* dstCur = dst;

  const uint32_t* colors = (const uint32_t*)ctx->radialGradient.colors;
  sysint_t colorsLength = ctx->radialGradient.colorsLength;

  uint32_t color0 = colors[0];
  uint32_t color1 = colors[colorsLength-1];

  int index;

  double dx = (double)x - ctx->radialGradient.dx;
  double dy = (double)y - ctx->radialGradient.dy;

  double fx = ctx->radialGradient.fx;
  double fy = ctx->radialGradient.fy;
  double r2 = ctx->radialGradient.r2;
  double scale = ctx->radialGradient.mul;

  double dyfx = dy * fx;
  double dyfy = dy * fy;
  double dydy = dy * dy;

  double dxdx = dx * dx;
  double dxfx = dx * fx;
  double dxfy = dx * fy;

  double dxfx_p_dyfy = dxfx + dyfy;
  double dxfy_m_dyfx = dxfy - dyfx;

  double cc = (dydy + dxdx) * r2 - (dxfy_m_dyfx * dxfy_m_dyfx);
  double cx = (dx * r2)          - (dxfy_m_dyfx * fy);
  double ci = r2                 - (fy * fy);

  double dd = (dxfx_p_dyfy);
  double di = fx;

  do {
    index = (int)((dd + sqrt(fabs(cc))) * scale) % colorsLength;
    if (index < 0) index += colorsLength;

    ((uint32_t*)dstCur)[0] = colors[index];
    dstCur += 4;

    // cc += cx + cx + ci
    // cx += ci
    cc += cx;
    cx += ci;
    cc += cx;

    dd += di;
  } while (--w);

  return dst;
}

static err_t FOG_FASTCALL pattern_radial_gradient_init(
  PatternContext* ctx, const Pattern& pattern)
{
  Pattern::Data* d = pattern._d;
  if (d->type != Pattern::RadialGradient) return Error::InvalidArgument;

  sysint_t gLength = 256 * d->obj.gradientStops->length();
  if (gLength > 4096) gLength = 4096;
  sysint_t gAlloc = gLength;

  // This calculation is based on AntiGrain 2.4 <www.antigrain.com>
  // ----------------------------------------------------------------
  // Calculate the invariant values. In case the focal center
  // lies exactly on the gradient circle the divisor degenerates
  // into zero. In this case we just move the focal center by
  // one subpixel unit possibly in the direction to the origin (0,0)
  // and calculate the values again.
  //-----------------------------------------------------------------
  ctx->radialGradient.dx = d->points[1].x();
  ctx->radialGradient.dy = d->points[1].y();
  ctx->radialGradient.fx = d->points[1].x() - d->points[0].x();
  ctx->radialGradient.fy = d->points[1].y() - d->points[0].y();
  ctx->radialGradient.r = d->gradientRadius;

  ctx->radialGradient.r2  = ctx->radialGradient.r  * ctx->radialGradient.r;
  ctx->radialGradient.fx2 = ctx->radialGradient.fx * ctx->radialGradient.fx;
  ctx->radialGradient.fy2 = ctx->radialGradient.fy * ctx->radialGradient.fy;
  double dd = (ctx->radialGradient.r2 - (ctx->radialGradient.fx2 + ctx->radialGradient.fy2));

  if (dd == 0.0)
  {
    if (ctx->radialGradient.fx) { if (ctx->radialGradient.fx < 0) ctx->radialGradient.fx += 1.0; else ctx->radialGradient.fx -= 1.0; }
    if (ctx->radialGradient.fy) { if (ctx->radialGradient.fy < 0) ctx->radialGradient.fy += 1.0; else ctx->radialGradient.fy -= 1.0; }
    ctx->radialGradient.fx2 = ctx->radialGradient.fx * ctx->radialGradient.fx;
    ctx->radialGradient.fy2 = ctx->radialGradient.fy * ctx->radialGradient.fy;
    dd = (ctx->radialGradient.r2 - (ctx->radialGradient.fx2 + ctx->radialGradient.fy2));
  }

  // Alloc twice memory for reflect spread.
  gAlloc = gLength;
  if (d->spread == Pattern::ReflectSpread) gAlloc <<= 1;

  ctx->radialGradient.mul = (double)gLength / dd;

  err_t err = pattern_generic_gradient_init(ctx, 
    d->obj.gradientStops.instance(), gLength,
    d->spread == Pattern::ReflectSpread);
  if (err) return err;

  // Set fetch function.
  switch (d->spread)
  {
    case Pattern::PadSpread:
      ctx->fetch = functionMap->pattern.radial_gradient_fetch_pad;
      break;
    case Pattern::RepeatSpread:
      ctx->fetch = functionMap->pattern.radial_gradient_fetch_repeat;
      break;
    case Pattern::ReflectSpread:
      ctx->fetch = functionMap->pattern.radial_gradient_fetch_repeat;
      break;
    default:
      FOG_ASSERT_NOT_REACHED();
  }

  // Set destroy function.
  ctx->destroy = pattern_generic_gradient_destroy;

  ctx->initialized = true;
  return Error::Ok;
}

// ============================================================================
// [Fog::Raster - Pattern - Gradient - Conical]
// ============================================================================

static uint8_t* FOG_FASTCALL pattern_conical_gradient_fetch(
  PatternContext* ctx,
  uint8_t* dst, int x, int y, int w)
{

  FOG_ASSERT(w);

  uint8_t* dstCur = dst;

  const uint32_t* colors = (const uint32_t*)ctx->radialGradient.colors;
  sysint_t colorsLength = ctx->radialGradient.colorsLength;

  int index;

  double dx = (double)x - ctx->conicalGradient.dx;
  double dy = (double)y - ctx->conicalGradient.dy;
  double scale = (double)colorsLength / (M_PI * 2.0);
  double add = ctx->conicalGradient.angle;
  if (add < M_PI) add += M_PI * 2.0;

  do {
    index = (int)((atan2(dy, dx) + add) * scale);
    if (index >= colorsLength) index -= colorsLength;

    ((uint32_t*)dstCur)[0] = colors[index];
    dstCur += 4;
    dx += 1.0;
  } while (--w);

  return dst;
}

static err_t FOG_FASTCALL pattern_conical_gradient_init(
  PatternContext* ctx, const Pattern& pattern)
{
  Pattern::Data* d = pattern._d;
  if (d->type != Pattern::ConicalGradient) return Error::InvalidArgument;

  sysint_t gLength = 256 * d->obj.gradientStops->length();
  if (gLength > 4096) gLength = 4096;

  ctx->conicalGradient.dx = d->points[0].x();
  ctx->conicalGradient.dy = d->points[0].y();
  ctx->conicalGradient.angle = atan2(
    (d->points[0].x() - d->points[1].x()),
    (d->points[0].y() - d->points[1].y())) + (M_PI/2.0);

  err_t err = pattern_generic_gradient_init(ctx,
    d->obj.gradientStops.instance(), gLength,
    d->spread == Pattern::ReflectSpread);
  if (err) return err;

  // Set fetch function.
  ctx->fetch = functionMap->pattern.conical_gradient_fetch;

  // Set destroy function.
  ctx->destroy = pattern_generic_gradient_destroy;

  ctx->initialized = true;
  return Error::Ok;
}

} // Raster namespace
} // Fog namespace
