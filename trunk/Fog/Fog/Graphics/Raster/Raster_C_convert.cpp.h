// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// For some IDEs to enable code-assist.
#include <Fog/Build/Build.h>
#if defined(FOG_IDE)
#include <Fog/Graphics/Raster/Raster_C_base.cpp.h>
#endif // FOG_IDE

namespace Fog {
namespace Raster {

// ============================================================================
// [Fog::Raster - Convert - BSwap]
// ============================================================================

static void FOG_FASTCALL convert_bswap16(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  for (sysint_t i = w; i; i--, dst += 2, src += 2)
  {
    ((uint16_t*)dst)[0] = Memory::bswap16(((const uint16_t*)src)[0]);
  }
}

static void FOG_FASTCALL convert_bswap24(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  for (sysint_t i = w; i; i--, dst += 3, src += 3)
  {
    uint8_t s0 = src[0];
    uint8_t s1 = src[1];
    uint8_t s2 = src[2];

    dst[0] = s2;
    dst[1] = s1;
    dst[2] = s0;
  }
}

static void FOG_FASTCALL convert_bswap32(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 4)
  {
    ((uint32_t*)dst)[0] = Memory::bswap32(((const uint32_t*)src)[0]);
  }
}

// ============================================================================
// [Fog::Raster - Convert - MemCpy]
// ============================================================================

static void FOG_FASTCALL convert_memcpy8(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  while ((sysuint_t(dst) & 3))
  {
    copy1(dst, src);
    dst++;
    src++;
    if (--w == 0) return;
  }

  while (w >= 32)
  {
    copy32(dst, src);
    dst += 32;
    src += 32;
    w -= 32;
  }

  while (w >= 4)
  {
    copy4(dst, src);
    dst += 4;
    src += 4;
    w -= 4;
  }

  switch (w & 3)
  {
    case 3: copy1(dst, src); dst += 1; src += 1;
    case 2: copy1(dst, src); dst += 1; src += 1;
    case 1: copy1(dst, src); dst += 1; src += 1;
  }
}

static void FOG_FASTCALL convert_memcpy16(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  convert_memcpy8(dst, src, mul2(w), closure);
}

static void FOG_FASTCALL convert_memcpy24(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  convert_memcpy8(dst, src, mul3(w), closure);
}

static void FOG_FASTCALL convert_memcpy32(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  while (w >= 8)
  {
    copy32(dst, src);
    dst += 32;
    src += 32;
    w -= 8;
  }

  switch (w & 7)
  {
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
// [Fog::Raster - Convert - Axxx32 Dest]
// ============================================================================

static void FOG_FASTCALL convert_axxx32_from_xxxx32(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 4)
  {
    ((uint32_t*)dst)[0] = ((const uint32_t*)src)[0] | 0xFF000000;
  }
}

static void FOG_FASTCALL convert_axxx32_from_a8(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  for (sysint_t i = w; i; i--)
  {
    ((uint32_t*)dst)[0] = (uint32_t)src[0] << 24;

    dst += 4;
    src += 1;
  }
}

static void FOG_FASTCALL convert_axxx32_bs_from_a8(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  for (sysint_t i = w; i; i--)
  {
    ((uint32_t*)dst)[0] = (uint32_t)src[0];

    dst += 4;
    src += 1;
  }
}

// ============================================================================
// [Fog::Raster - Convert - Argb32 Dest]
// ============================================================================

static void FOG_FASTCALL convert_argb32_from_prgb32(
  uint8_t* _dst, const uint8_t* _src, sysint_t w, const Closure* closure)
{
  uint32_t* dst = reinterpret_cast<uint32_t*>(_dst);
  const uint32_t* src = reinterpret_cast<const uint32_t*>(_src);

  sysint_t i = 0;

  while (i < w)
  {
    // We want to identify long runs of constant input pixels and
    // cache the unpremultiplied.
    uint32_t const_in, const_out;

    // Diff is the or of all bitwise differences from const_in
    // during the probe period.  If it is zero after the probe
    // period then every input pixel was identical in the
    // probe.
    unsigned diff = 0;

    // Accumulator for all alphas of the probe period pixels,
    // biased to make the sum zero if the
    unsigned accu = -2*255;

    {
      uint32_t rgba, a, r, g, b, recip;

      rgba = const_in = src[i];
      a = (rgba >> 24);
      accu += a;
      r = (rgba >> 16) & 0xFF;
      g = (rgba >> 8 ) & 0xFF;
      b = (rgba      ) & 0xFF;
      recip = demultiply_reciprocal_table_d[a];

      r = ((r * recip)      ) & 0x00FF0000;
      g = ((g * recip) >>  8) & 0x0000FF00;
      b = ((b * recip) >> 16) & 0x000000FF;

      dst[i] = const_out = r | g | b | (a << 24);
    }

    if (++i == w) return;

    {
      uint32_t rgba, a, r, g, b, recip;
      rgba = src[i];
      a = (rgba >> 24);
      accu += a;
      r = (rgba >> 16) & 0xFF;
      g = (rgba >>  8) & 0xFF;
      b = (rgba      ) & 0xFF;
      recip = demultiply_reciprocal_table_d[a];

      diff = rgba ^ const_in;

      r = ((r * recip)      ) & 0x00FF0000;
      g = ((g * recip) >>  8) & 0x0000FF00;
      b = ((b * recip) >> 16) & 0x000000FF;

      dst[i] = r | g | b | (a << 24);
    }

    if (++i == w) return;

    // Fall into special cases if we have special circumstances.
    if (0 != (accu & diff)) continue;

    if (0 == accu)
    {
      // a run of solid pixels.
      uint32_t in;
      while (0xFF000000 == ((in = src[i]) & 0xFF000000))
      {
        dst[i] = in;
        if (++i == w) return;
      }
    }
    else if (0 == diff)
    {
      // a run of constant pixels.
      while (src[i] == const_in)
      {
        dst[i] = const_out;
        if (++i == w) return;
      }
    }
  }
}

static void FOG_FASTCALL convert_argb32_from_prgb32_bs(
  uint8_t* _dst, const uint8_t* _src, sysint_t w, const Closure* closure)
{
  uint32_t* dst = reinterpret_cast<uint32_t*>(_dst);
  const uint32_t* src = reinterpret_cast<const uint32_t*>(_src);

  sysint_t i = 0;

  while (i < w)
  {
    // We want to identify long runs of constant input pixels and
    // cache the unpremultiplied.
    uint32_t const_in, const_out;

    // Diff is the or of all bitwise differences from const_in
    // during the probe period.  If it is zero after the probe
    // period then every input pixel was identical in the
    // probe.
    unsigned diff = 0;

    // Accumulator for all alphas of the probe period pixels,
    // biased to make the sum zero if the
    unsigned accu = -2*255;

    {
      uint32_t rgba, a, r, g, b, recip;

      rgba = const_in = src[i];
      a = (rgba      ) & 0xFF;
      accu += a;
      r = (rgba >>  8) & 0xFF;
      g = (rgba >> 16) & 0xFF;
      b = (rgba >> 24);
      recip = demultiply_reciprocal_table_d[a];

      r = ((r * recip)      ) & 0x00FF0000;
      g = ((g * recip) >>  8) & 0x0000FF00;
      b = ((b * recip) >> 16) & 0x000000FF;

      dst[i] = const_out = r | g | b | (a << 24);
    }

    if (++i == w) return;

    {
      uint32_t rgba, a, r, g, b, recip;
      rgba = src[i];
      a = (rgba      ) & 0xFF;
      accu += a;
      r = (rgba >>  8) & 0xFF;
      g = (rgba >> 16) & 0xFF;
      b = (rgba >> 24);
      recip = demultiply_reciprocal_table_d[a];

      diff = rgba ^ const_in;

      r = ((r * recip)      ) & 0x00FF0000;
      g = ((g * recip) >>  8) & 0x0000FF00;
      b = ((b * recip) >> 16) & 0x000000FF;

      dst[i] = r | g | b | (a << 24);
    }

    if (++i == w) return;

    // Fall into special cases if we have special circumstances.
    if (0 != (accu & diff)) continue;

    if (0 == accu)
    {
      // a run of solid pixels.
      uint32_t in;
      while (0xFF000000 == ((in = Memory::bswap32(src[i])) & 0xFF000000))
      {
        dst[i] = in;
        if (++i == w) return;
      }
    }
    else if (0 == diff)
    {
      // a run of constant pixels.
      while (src[i] == const_in)
      {
        dst[i] = const_out;
        if (++i == w) return;
      }
    }
  }
}

static void FOG_FASTCALL convert_argb32_from_rgb32_bs(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 4)
  {
    ((uint32_t*)dst)[0] = Memory::bswap32(((const uint32_t*)src)[0]) | 0xFF000000;
  }
}

static void FOG_FASTCALL convert_argb32_from_i8(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const Rgba* srcPal = closure->srcPalette + Palette::IndexARGB32;

  for (sysint_t i = w; i; i--, dst += 4, src += 1)
  {
    ((uint32_t*)dst)[0] = srcPal[src[0]];
  }
}

static void FOG_FASTCALL convert_argb32_bs_from_rgb32(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 4)
  {
    ((uint32_t*)dst)[0] = Memory::bswap32(((const uint32_t*)src)[0]) | 0x000000FF;
  }
}

static void FOG_FASTCALL convert_argb32_bs_from_prgb32(
  uint8_t* _dst, const uint8_t* _src, sysint_t w, const Closure* closure)
{
  uint32_t* dst = reinterpret_cast<uint32_t*>(_dst);
  const uint32_t* src = reinterpret_cast<const uint32_t*>(_src);

  sysint_t i = 0;

  while (i < w)
  {
    // We want to identify long runs of constant input pixels and
    // cache the unpremultiplied.
    uint32_t const_in, const_out;

    // Diff is the or of all bitwise differences from const_in
    // during the probe period.  If it is zero after the probe
    // period then every input pixel was identical in the
    // probe.
    unsigned diff = 0;

    // Accumulator for all alphas of the probe period pixels,
    // biased to make the sum zero if the
    unsigned accu = -2*255;

    {
      uint32_t rgba, a, r, g, b, recip;

      rgba = const_in = src[i];
      a = (rgba >> 24);
      accu += a;
      r = (rgba >> 16) & 0xFF;
      g = (rgba >> 8 ) & 0xFF;
      b = (rgba      ) & 0xFF;
      recip = demultiply_reciprocal_table_d[a];

      r = ((r * recip) >>  8) & 0x0000FF00;
      g = ((g * recip)      ) & 0x00FF0000;
      b = ((b * recip) <<  8) & 0xFF000000;

      dst[i] = const_out = r | g | b | a;
    }

    if (++i == w) return;

    {
      uint32_t rgba, a, r, g, b, recip;

      rgba = const_in = src[i];
      a = (rgba >> 24);
      accu += a;
      r = (rgba >> 16) & 0xFF;
      g = (rgba >> 8 ) & 0xFF;
      b = (rgba      ) & 0xFF;
      recip = demultiply_reciprocal_table_d[a];

      diff = rgba ^ const_in;

      r = ((r * recip) >>  8) & 0x0000FF00;
      g = ((g * recip)      ) & 0x00FF0000;
      b = ((b * recip) <<  8) & 0xFF000000;

      dst[i] = r | g | b | a;
    }

    if (++i == w) return;

    // Fall into special cases if we have special circumstances.
    if (0 != (accu & diff)) continue;

    if (0 == accu)
    {
      // a run of solid pixels.
      uint32_t in;
      while (0xFF000000 == ((in = src[i]) & 0xFF000000))
      {
        dst[i] = Memory::bswap32(in);
        if (++i == w) return;
      }
    }
    else if (0 == diff)
    {
      // a run of constant pixels.
      while (src[i] == const_in)
      {
        dst[i] = const_out;
        if (++i == w) return;
      }
    }
  }
}

static void FOG_FASTCALL convert_argb32_bs_from_i8(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const Rgba* srcPal = closure->srcPalette + Palette::IndexARGB32;

  for (sysint_t i = w; i; i--, dst += 4, src += 1)
  {
    ((uint32_t*)dst)[0] = Memory::bswap32(srcPal[src[0]]);
  }
}

// ============================================================================
// [Fog::Raster - Convert - Prgb32 Dest]
// ============================================================================

static void FOG_FASTCALL convert_prgb32_from_argb32(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 4)
  {
    ((uint32_t*)dst)[0] = premultiply(((const uint32_t*)src)[0]);
  }
}

static void FOG_FASTCALL convert_prgb32_from_argb32_bs(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 4)
  {
    ((uint32_t*)dst)[0] = premultiply(Memory::bswap32(((const uint32_t*)src)[0]));
  }
}

static void FOG_FASTCALL convert_prgb32_from_i8(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const Rgba* srcPal = closure->srcPalette + Palette::IndexPRGB32;

  for (sysint_t i = w; i; i--, dst += 4, src += 1)
  {
    ((uint32_t*)dst)[0] = srcPal[src[0]];
  }
}

static void FOG_FASTCALL convert_prgb32_bs_from_argb32(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 4)
  {
    ((uint32_t*)dst)[0] = Memory::bswap32(premultiply(((const uint32_t*)src)[0]));
  }
}

static void FOG_FASTCALL convert_prgb32_bs_from_i8(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const Rgba* srcPal = closure->srcPalette + Palette::IndexPRGB32;

  for (sysint_t i = w; i; i--, dst += 4, src += 1)
  {
    ((uint32_t*)dst)[0] = Memory::bswap32(srcPal[src[0]]);
  }
}

// ============================================================================
// [Fog::Raster - Convert - Rgb32 Dest]
// ============================================================================

static void FOG_FASTCALL convert_rgb32_from_argb32(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 4)
  {
    ((uint32_t*)dst)[0] = premultiply(((const uint32_t*)src)[0]) | 0xFF000000;
  }
}

static void FOG_FASTCALL convert_rgb32_from_argb32_bs(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 4)
  {
    ((uint32_t*)dst)[0] = Memory::bswap32(premultiply(((const uint32_t*)src)[0]) | 0xFF000000);
  }
}

static void FOG_FASTCALL convert_rgb32_from_rgb24(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  sysint_t i = w;

#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
  while (i >= 4)
  {
    // Following table illustrates how this works on Little Endian:
    //
    // [B G R B] [G R B G] [R B G R]
    // [B G R X] [B G R X] [B G R X] [B G R X]

    uint32_t s0 = ((const uint32_t*)src)[0];
    uint32_t s1 = ((const uint32_t*)src)[1];
    uint32_t s2 = ((const uint32_t*)src)[2];

    ((uint32_t*)dst)[0] = (s0      )              | 0xFF000000;
    ((uint32_t*)dst)[1] = (s0 >> 24) | (s1 <<  8) | 0xFF000000;
    ((uint32_t*)dst)[2] = (s1 >> 16) | (s2 << 16) | 0xFF000000;
    ((uint32_t*)dst)[3] =              (s2 >>  8) | 0xFF000000;

    dst += 16;
    src += 12;
    i -= 4;
  }
#endif // FOG_BYTE_ORDER

  while (i)
  {
    ((uint32_t*)dst)[0] = PixFmt_RGB24::fetch(src) | 0xFF000000;
    dst += 4;
    src += 3;
    i--;
  }
}

static void FOG_FASTCALL convert_rgb32_from_bgr24(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 3)
  {
    ((uint32_t*)dst)[0] = PixFmt_BGR24::fetch(src) | 0xFF000000;
  }
}

static void FOG_FASTCALL convert_rgb32_from_rgb16_5550(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 2)
  {
    ((uint32_t*)dst)[0] = PixFmt_RGB16_555::fetch(src) | 0xFF000000;
  }
}

static void FOG_FASTCALL convert_rgb32_from_rgb16_5550_bs(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 2)
  {
    ((uint32_t*)dst)[0] = PixFmt_RGB16_555_BS::fetch(src) | 0xFF000000;
  }
}

static void FOG_FASTCALL convert_rgb32_from_rgb16_5650(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 2)
  {
    ((uint32_t*)dst)[0] = PixFmt_RGB16_565::fetch(src) | 0xFF000000;
  }
}

static void FOG_FASTCALL convert_rgb32_from_rgb16_5650_bs(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 2)
  {
    ((uint32_t*)dst)[0] = PixFmt_RGB16_565_BS::fetch(src) | 0xFF000000;
  }
}

static void FOG_FASTCALL convert_rgb32_from_i8(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const Rgba* srcPal = closure->srcPalette + Palette::IndexPRGB32;

  for (sysint_t i = w; i; i--, dst += 4, src += 1)
  {
    ((uint32_t*)dst)[0] = srcPal[src[0]] | 0xFF000000;
  }
}

static void FOG_FASTCALL convert_rgb32_bs_from_rgb24(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 3)
  {
    ((uint32_t*)dst)[0] = Memory::bswap32(PixFmt_RGB24::fetch(src) | 0xFF000000);
  }
}

// ============================================================================
// [Fog::Raster - Convert - Rgb24/Bgr24 Dest]
// ============================================================================

static void FOG_FASTCALL convert_rgb24_from_rgb32(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  sysint_t i = w;

#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
  // Align.
  while (i && (sysuint_t)dst & 0x3)
  {
    uint32_t s0 = ((const uint32_t*)src)[0];

    dst[RGB24_RByte] = (uint8_t)(s0 >> 16);
    dst[RGB24_GByte] = (uint8_t)(s0 >>  8);
    dst[RGB24_BByte] = (uint8_t)(s0);

    dst += 3;
    src += 4;
    i--;
  }

  // 4 Pixels per time.
  while (i >= 4)
  {
    // Following table illustrates how this works on Little Endian:
    //
    // [B G R X] [B G R X] [B G R X] [B G R X]
    // [B G R B] [G R B G] [R B G R]
    uint32_t s0 = ((const uint32_t*)src)[0];
    uint32_t s1 = ((const uint32_t*)src)[1];
    uint32_t s2 = ((const uint32_t*)src)[2];
    uint32_t s3 = ((const uint32_t*)src)[3];

    ((uint32_t*)dst)[0] = (s0 & 0x00FFFFFF) | (s1 << 24); s1 >>= 8;
    ((uint32_t*)dst)[1] = (s1 & 0x0000FFFF) | (s2 << 16); s2 >>= 16;
    ((uint32_t*)dst)[2] = (s2 & 0x000000FF) | (s3 <<  8);

    dst += 12;
    src += 16;
    i -= 4;
  }
#endif // FOG_BYTE_ORDER

  while (i)
  {
    uint32_t s0 = ((const uint32_t*)src)[0];

    dst[RGB24_RByte] = (uint8_t)(s0 >> 16);
    dst[RGB24_GByte] = (uint8_t)(s0 >>  8);
    dst[RGB24_BByte] = (uint8_t)(s0);

    dst += 3;
    src += 4;
    i--;
  }
}

static void FOG_FASTCALL convert_rgb24_from_rgb32_bs(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  sysint_t i = w;

#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
  // Align.
  while (i && (sysuint_t)dst & 0x3)
  {
    uint32_t s0 = ((const uint32_t*)src)[0];

    dst[RGB24_RByte] = (uint8_t)(s0 >> 8);
    dst[RGB24_GByte] = (uint8_t)(s0 >> 16);
    dst[RGB24_BByte] = (uint8_t)(s0 >> 24);

    dst += 3;
    src += 4;
    i--;
  }

  // 4 Pixels per time.
  while (i >= 4)
  {
    // Following table illustrates how this works on Little Endian:
    //
    // [X R G B] [X R G B] [X R G B] [X R G B]
    // [B G R B] [G R B G] [R B G R]
    uint32_t s0 = Memory::bswap32(((const uint32_t*)src)[0]);
    uint32_t s1 = Memory::bswap32(((const uint32_t*)src)[1]);
    uint32_t s2 = Memory::bswap32(((const uint32_t*)src)[2]);
    uint32_t s3 = Memory::bswap32(((const uint32_t*)src)[3]);

    ((uint32_t*)dst)[0] = (s0 & 0x00FFFFFF) | (s1 << 24); s1 >>= 8;
    ((uint32_t*)dst)[1] = (s1 & 0x0000FFFF) | (s2 << 16); s2 >>= 16;
    ((uint32_t*)dst)[2] = (s2 & 0x000000FF) | (s3 <<  8);

    dst += 12;
    src += 16;
    i -= 4;
  }
#endif // FOG_BYTE_ORDER

  while (i)
  {
    uint32_t s0 = ((const uint32_t*)src)[0];

    dst[RGB24_RByte] = (uint8_t)(s0 >> 8);
    dst[RGB24_GByte] = (uint8_t)(s0 >> 16);
    dst[RGB24_BByte] = (uint8_t)(s0 >> 24);

    dst += 3;
    src += 4;
    i--;
  }
}

static void FOG_FASTCALL convert_rgb24_from_rgb16_5550(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  sysint_t i = w;

  while (i)
  {
    uint32_t x = ((uint16_t*)src)[0];

    dst[RGB24_RByte] = ((x & 0x7C00) | ((x & 0x7000) >> 5)) >> 7;
    dst[RGB24_GByte] = ((x & 0x03E0) | ((x & 0x0380) >> 5)) >> 2;
    dst[RGB24_BByte] = ((x & 0x001C) | ((x & 0x001F) << 5)) >> 2;

    dst += 3;
    src += 2;
    i--;
  }
}

static void FOG_FASTCALL convert_rgb24_from_rgb16_5550_bs(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  sysint_t i = w;

  while (i)
  {
    uint32_t x = Memory::bswap16(((uint16_t*)src)[0]);

    dst[RGB24_RByte] = ((x & 0x7C00) | ((x & 0x7000) >> 5)) >> 7;
    dst[RGB24_GByte] = ((x & 0x03E0) | ((x & 0x0380) >> 5)) >> 2;
    dst[RGB24_BByte] = ((x & 0x001C) | ((x & 0x001F) << 5)) >> 2;

    dst += 3;
    src += 2;
    i--;
  }
}

static void FOG_FASTCALL convert_rgb24_from_rgb16_5650(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  sysint_t i = w;

  while (i)
  {
    uint32_t x = ((uint16_t*)src)[0];

    dst[RGB24_RByte] = ((x & 0xF800) | ((x & 0xE000) >> 5)) >> 8;
    dst[RGB24_GByte] = ((x & 0x07E0) | ((x & 0x0600) >> 6)) >> 3;
    dst[RGB24_RByte] = ((x & 0x001C) | ((x & 0x001F) << 5)) >> 2;

    dst += 3;
    src += 2;
    i--;
  }
}

static void FOG_FASTCALL convert_rgb24_from_rgb16_5650_bs(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  sysint_t i = w;

  while (i)
  {
    uint32_t x = Memory::bswap16(((uint16_t*)src)[0]);

    dst[RGB24_RByte] = ((x & 0xF800) | ((x & 0xE000) >> 5)) >> 8;
    dst[RGB24_GByte] = ((x & 0x07E0) | ((x & 0x0600) >> 6)) >> 3;
    dst[RGB24_RByte] = ((x & 0x001C) | ((x & 0x001F) << 5)) >> 2;

    dst += 3;
    src += 2;
    i--;
  }
}

static void FOG_FASTCALL convert_rgb24_from_i8(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const Rgba* srcPal = closure->srcPalette + Palette::IndexPRGB32;
  sysint_t i = w;

#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
  // Align.
  while (i && (sysuint_t)dst & 0x3)
  {
    uint32_t s0 = srcPal[src[0]];

    dst[RGB24_RByte] = (uint8_t)(s0 >> 8);
    dst[RGB24_GByte] = (uint8_t)(s0 >> 16);
    dst[RGB24_BByte] = (uint8_t)(s0 >> 24);

    dst += 3;
    src += 1;
    i--;
  }

  // 4 Pixels per time.
  while (i >= 4)
  {
    // Following table illustrates how this works on Little Endian:
    //
    // [X R G B] [X R G B] [X R G B] [X R G B]
    // [B G R B] [G R B G] [R B G R]
    uint32_t s0 = srcPal[src[0]];
    uint32_t s1 = srcPal[src[1]];
    uint32_t s2 = srcPal[src[2]];
    uint32_t s3 = srcPal[src[3]];

    ((uint32_t*)dst)[0] = (s0 & 0x00FFFFFF) | (s1 << 24); s1 >>= 8;
    ((uint32_t*)dst)[1] = (s1 & 0x0000FFFF) | (s2 << 16); s2 >>= 16;
    ((uint32_t*)dst)[2] = (s2 & 0x000000FF) | (s3 <<  8);

    dst += 12;
    src += 4;
    i -= 4;
  }
#endif // FOG_BYTE_ORDER

  while (i)
  {
    uint32_t s0 = srcPal[src[0]];

    dst[RGB24_RByte] = (uint8_t)(s0 >> 8);
    dst[RGB24_GByte] = (uint8_t)(s0 >> 16);
    dst[RGB24_BByte] = (uint8_t)(s0 >> 24);

    dst += 3;
    src += 1;
    i--;
  }
}

static void FOG_FASTCALL convert_bgr24_from_rgb32(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  sysint_t i = w;

  while (i)
  {
    uint32_t s0 = ((const uint32_t*)src)[0];

    dst[2 - RGB24_RByte] = (uint8_t)(s0 >> 16);
    dst[2 - RGB24_GByte] = (uint8_t)(s0 >>  8);
    dst[2 - RGB24_BByte] = (uint8_t)(s0);

    dst += 3;
    src += 4;
    i--;
  }
}

static void FOG_FASTCALL convert_bgr24_from_i8(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const Rgba* srcPal = closure->srcPalette + Palette::IndexPRGB32;
  sysint_t i = w;

  while (i)
  {
    uint32_t s0 = srcPal[src[0]];

    dst[2 - RGB24_RByte] = (uint8_t)(s0 >> 8);
    dst[2 - RGB24_GByte] = (uint8_t)(s0 >> 16);
    dst[2 - RGB24_BByte] = (uint8_t)(s0 >> 24);

    dst += 3;
    src += 1;
    i--;
  }
}

// ============================================================================
// [Fog::Raster - Convert - Rgb16 Dest]
// ============================================================================

static void FOG_FASTCALL convert_rgb16_5550_from_rgb32(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  sysint_t i = w;

  while (i)
  {
    uint32_t s0 = ((const uint32_t*)src)[0];

    ((uint16_t*)dst)[0] =
      ((s0 >> 9) & 0x7C00) |
      ((s0 >> 6) & 0x03E0) |
      ((s0 >> 3)         ) ;

    dst += 2;
    src += 4;
    i--;
  }
}

static void FOG_FASTCALL convert_rgb16_5550_from_rgb24(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  sysint_t i = w;

  while (i)
  {
    uint32_t s0 = ((const uint32_t*)src)[0];

    ((uint16_t*)dst)[0] =
      (((uint32_t)src[RGB24_RByte] << 7) & 0x7C00) |
      (((uint32_t)src[RGB24_GByte] << 2) & 0x03E0) |
      (((uint32_t)src[RGB24_BByte] >> 3)         ) ;

    dst += 2;
    src += 3;
    i--;
  }
}

static void FOG_FASTCALL convert_rgb16_5550_from_i8(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const Rgba* srcPal = closure->srcPalette + Palette::IndexPRGB32;
  sysint_t i = w;

  while (i)
  {
    uint32_t s0 = srcPal[src[0]];

    ((uint16_t*)dst)[0] =
      ((s0 >> 9) & 0x7C00) |
      ((s0 >> 6) & 0x03E0) |
      ((s0 >> 3)         ) ;

    dst += 2;
    src += 1;
    i--;
  }
}

static void FOG_FASTCALL convert_rgb16_5650_from_rgb32(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  sysint_t i = w;

  while (i)
  {
    uint32_t s0 = ((const uint32_t*)src)[0];

    ((uint16_t*)dst)[0] =
      ((s0 >> 8) & 0xF800) |
      ((s0 >> 5) & 0x07E0) |
      ((s0 >> 3)         ) ;

    dst += 2;
    src += 4;
    i--;
  }
}

static void FOG_FASTCALL convert_rgb16_5650_from_rgb24(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  sysint_t i = w;

  while (i)
  {
    uint32_t s0 = ((const uint32_t*)src)[0];

    ((uint16_t*)dst)[0] =
      (((uint32_t)src[RGB24_RByte] << 8) & 0xF800) |
      (((uint32_t)src[RGB24_GByte] << 3) & 0x07E0) |
      (((uint32_t)src[RGB24_BByte] >> 3)         ) ;

    dst += 2;
    src += 3;
    i--;
  }
}

static void FOG_FASTCALL convert_rgb16_5650_from_i8(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const Rgba* srcPal = closure->srcPalette + Palette::IndexPRGB32;
  sysint_t i = w;

  while (i)
  {
    uint32_t s0 = srcPal[src[0]];

    ((uint16_t*)dst)[0] =
      ((s0 >> 8) & 0xF800) |
      ((s0 >> 5) & 0x07E0) |
      ((s0 >> 3)         ) ;

    dst += 2;
    src += 1;
    i--;
  }
}

static void FOG_FASTCALL convert_rgb16_5550_bs_from_rgb32(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  sysint_t i = w;

  while (i)
  {
    uint32_t s0 = ((const uint32_t*)src)[0];

    ((uint16_t*)dst)[0] = Memory::bswap16(
      ((s0 >> 9) & 0x7C00) |
      ((s0 >> 6) & 0x03E0) |
      ((s0 >> 3)         ) );

    dst += 2;
    src += 4;
    i--;
  }
}

static void FOG_FASTCALL convert_rgb16_5550_bs_from_rgb24(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  sysint_t i = w;

  while (i)
  {
    uint32_t s0 = ((const uint32_t*)src)[0];

    ((uint16_t*)dst)[0] = Memory::bswap16(
      (((uint32_t)src[RGB24_RByte] << 7) & 0x7C00) |
      (((uint32_t)src[RGB24_GByte] << 2) & 0x03E0) |
      (((uint32_t)src[RGB24_BByte] >> 3)         ) );

    dst += 2;
    src += 3;
    i--;
  }
}

static void FOG_FASTCALL convert_rgb16_5550_bs_from_i8(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const Rgba* srcPal = closure->srcPalette + Palette::IndexPRGB32;
  sysint_t i = w;

  while (i)
  {
    uint32_t s0 = srcPal[src[0]];

    ((uint16_t*)dst)[0] = Memory::bswap16(
      ((s0 >> 9) & 0x7C00) |
      ((s0 >> 6) & 0x03E0) |
      ((s0 >> 3)         ) );

    dst += 2;
    src += 1;
    i--;
  }
}

static void FOG_FASTCALL convert_rgb16_5650_bs_from_rgb32(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  sysint_t i = w;

  while (i)
  {
    uint32_t s0 = ((const uint32_t*)src)[0];

    ((uint16_t*)dst)[0] = Memory::bswap16(
      ((s0 >> 8) & 0xF800) |
      ((s0 >> 5) & 0x07E0) |
      ((s0 >> 3)         ) );

    dst += 2;
    src += 4;
    i--;
  }
}

static void FOG_FASTCALL convert_rgb16_5650_bs_from_rgb24(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  sysint_t i = w;

  while (i)
  {
    uint32_t s0 = ((const uint32_t*)src)[0];

    ((uint16_t*)dst)[0] = Memory::bswap16(
      (((uint32_t)src[RGB24_RByte] << 8) & 0xF800) |
      (((uint32_t)src[RGB24_GByte] << 3) & 0x07E0) |
      (((uint32_t)src[RGB24_BByte] >> 3)         ) );

    dst += 2;
    src += 3;
    i--;
  }
}

static void FOG_FASTCALL convert_rgb16_5650_bs_from_i8(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const Rgba* srcPal = closure->srcPalette + Palette::IndexPRGB32;
  sysint_t i = w;

  while (i)
  {
    uint32_t s0 = srcPal[src[0]];

    ((uint16_t*)dst)[0] = Memory::bswap16(
      ((s0 >> 8) & 0xF800) |
      ((s0 >> 5) & 0x07E0) |
      ((s0 >> 3)         ) );

    dst += 2;
    src += 1;
    i--;
  }
}

// ============================================================================
// [Fog::Raster - Convert - A8 Dest]
// ============================================================================

static void FOG_FASTCALL convert_a8_from_axxx32(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  src += RGB32_AByte;

  for (sysint_t i = w; i; i--)
  {
    dst[0] = src[0];

    dst += 1;
    src += 4;
  }
}

static void FOG_FASTCALL convert_a8_from_i8(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const Rgba* srcPal = closure->srcPalette + Palette::IndexARGB32;

  for (sysint_t i = w; i; i--)
  {
    dst[0] = srcPal[src[0]].a;

    dst += 1;
    src += 1;
  }
}

// ============================================================================
// [Fog::Raster - Convert - Greyscale]
// ============================================================================

// We are using this formula:
//  0.212671 * R + 0.715160 * G + 0.072169 * B;
// As:
//  (13938 * R + 46868 * G + 4730 * B) / 65536
static void FOG_FASTCALL convert_greyscale8_from_rgb32(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  sysint_t i = w;

  while (i)
  {
    uint32_t s0 = ((const uint32_t*)src)[0];

    uint32_t grey =
      ((s0 >> 16) & 0xFF) * 13938 +
      ((s0 >>  8) & 0xFF) * 46868 +
      ((s0      ) & 0xFF) *  4730 ;

    dst[0] = grey >> 16;

    dst += 1;
    src += 4;
    i--;
  }
}

static void FOG_FASTCALL convert_greyscale8_from_rgb24(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  sysint_t i = w;

  while (i)
  {
    uint32_t grey =
      (uint32_t)src[RGB24_RByte] * 13938 +
      (uint32_t)src[RGB24_GByte] * 46868 +
      (uint32_t)src[RGB24_BByte] *  4730 ;

    dst[0] = grey >> 16;

    dst += 1;
    src += 3;
    i--;
  }
}

static void FOG_FASTCALL convert_greyscale8_from_bgr24(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  sysint_t i = w;

  while (i)
  {
    uint32_t grey =
      (uint32_t)src[BGR24_RByte] * 13938 +
      (uint32_t)src[BGR24_GByte] * 46868 +
      (uint32_t)src[BGR24_BByte] *  4730 ;

    dst[0] = grey >> 16;

    dst += 1;
    src += 3;
    i--;
  }
}

static void FOG_FASTCALL convert_greyscale8_from_i8(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  const Rgba* srcPal = closure->srcPalette + Palette::IndexPRGB32;
  sysint_t i = w;

  while (i)
  {
    uint32_t s0 = srcPal[src[0]];
    uint32_t grey = ((s0 >> 16) & 0xFF) * 13938 +
                    ((s0 >>  8) & 0xFF) * 46868 +
                    ((s0      ) & 0xFF) *  4730 ;
    dst[0] = grey >> 16;

    dst += 1;
    src += 1;
    i--;
  }
}

static void FOG_FASTCALL convert_rgb32_from_greyscale8(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  sysint_t i = w;

  while (i)
  {
    uint32_t grey = src[0];
    ((uint32_t*)dst)[0] = grey | (grey << 8) | (grey << 16) | 0xFF000000;

    dst += 4;
    src += 1;
    i--;
  }
}

static void FOG_FASTCALL convert_rgb24_from_greyscale8(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure)
{
  sysint_t i = w;

  while (i)
  {
    uint8_t grey = src[0];

    dst[0] = grey;
    dst[1] = grey;
    dst[2] = grey;

    dst += 3;
    src += 1;
    i--;
  }
}

// ============================================================================
// [Fog::Raster - Convert - Dithering]
// ============================================================================

static void FOG_FASTCALL convert_i8rgb232_from_rgb32_dither(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Point& origin, const uint8_t* palConv)
{
  sysint_t i;

  const uint8_t* dt = DitherMatrix::matrix[origin.getY() & DitherMatrix::Mask];
  int dx = origin.getX();

  for (i = w; i; i--, dx++, dst += 1, src += 4)
  {
    uint32_t c0;
    uint32_t r0, g0, b0, d, dith2, dith3;

    d = dt[dx & DitherMatrix::Mask];

    dith2 = DitherMatrix::shf_dither(d, DitherMatrix::shf_arg(2));
    dith3 = DitherMatrix::shf_dither(d, DitherMatrix::shf_arg(3));

    c0 = ((uint32_t *)src)[0];
    r0 = (c0 & 0x00FF0000) >> 16;
    g0 = (c0 & 0x0000FF00) >> 8;
    b0 = (c0 & 0x000000FF);

    if (((r0 & 0x3F) >= dith2) && (r0 < 0xC0)) r0 += 64;
    if (((g0 & 0x1F) >= dith3) && (g0 < 0xE0)) g0 += 32;
    if (((b0 & 0x3F) >= dith2) && (b0 < 0xC0)) b0 += 64;

    ((uint8_t *)dst)[0] = palConv[((r0 & 0xC0) >> 1) | ((g0 & 0xE0) >> 3) | ((b0 & 0xC0) >> 6)];
  }
}

static void FOG_FASTCALL convert_i8rgb222_from_rgb32_dither(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Point& origin, const uint8_t* palConv)
{
  sysint_t i;

  const uint8_t* dt = DitherMatrix::matrix[origin.getY() & DitherMatrix::Mask];
  int dx = origin.getX();

  for (i = w; i; i--, dx++, dst += 1, src += 4)
  {
    uint32_t c0;
    uint32_t r0, g0, b0, dith2;
    uint32_t r0t, g0t, b0t;

    dith2 = DitherMatrix::shf_dither(dt[dx & DitherMatrix::Mask], DitherMatrix::shf_arg(2)) * 4 / 3;

    c0 = ((uint32_t *)src)[0];
    r0 = (c0 & 0x00FF0000) >> 16;
    g0 = (c0 & 0x0000FF00) >> 8;
    b0 = (c0 & 0x000000FF);

    // c = c / 85
    r0t = (r0 * 0x00030000) >> 16;
    g0t = (g0 * 0x00030000) >> 16;
    b0t = (b0 * 0x00030000) >> 16;

    if ((r0 - (r0t * 85)) > dith2) r0t++;
    if ((g0 - (g0t * 85)) > dith2) g0t++;
    if ((b0 - (b0t * 85)) > dith2) b0t++;
    ((uint8_t *)dst)[0] = palConv[(r0t<<4)|(g0t<<2)|(b0t)];
  }
}

static void FOG_FASTCALL convert_i8rgb111_from_rgb32_dither(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Point& origin, const uint8_t* palConv)
{
  sysint_t i;

  const uint8_t* dt = DitherMatrix::matrix[origin.getY() & DitherMatrix::Mask];
  int dx = origin.getX();

  for (i = w; i; i--, dx++, dst += 1, src += 4)
  {
    uint32_t c0;
    uint32_t r0, g0, b0, dith1;

    dith1 = DitherMatrix::shf_dither(dt[dx & DitherMatrix::Mask], DitherMatrix::shf_arg(0));

    c0 = ((uint32_t *)src)[0];
    r0 = (c0 & 0x00FF0000) >> 16;
    g0 = (c0 & 0x0000FF00) >> 8;
    b0 = (c0 & 0x000000FF);

    if (r0 > dith1) r0 = 255;
    if (g0 > dith1) g0 = 255;
    if (b0 > dith1) b0 = 255;

    ((uint8_t *)dst)[0] = palConv[
      (((r0 + 1) >> 8) << 2) |
      (((g0 + 1) >> 8) << 1) |
      (((b0 + 1) >> 8)     ) ];
  }
}

static void FOG_FASTCALL convert_i8rgb232_from_rgb24_dither(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Point& origin, const uint8_t* palConv)
{
  sysint_t i;

  const uint8_t* dt = DitherMatrix::matrix[origin.getY() & DitherMatrix::Mask];
  int dx = origin.getX();

  for (i = w; i; i--, dx++, dst += 1, src += 3)
  {
    uint32_t r0, g0, b0, d, dith2, dith3;

    d = dt[dx & DitherMatrix::Mask];

    dith2 = DitherMatrix::shf_dither(d, DitherMatrix::shf_arg(2));
    dith3 = DitherMatrix::shf_dither(d, DitherMatrix::shf_arg(3));

    r0 = src[RGB24_RByte];
    g0 = src[RGB24_GByte];
    b0 = src[RGB24_BByte];

    if (((r0 & 0x3F) >= dith2) && (r0 < 0xC0)) r0 += 64;
    if (((g0 & 0x1F) >= dith3) && (g0 < 0xE0)) g0 += 32;
    if (((b0 & 0x3F) >= dith2) && (b0 < 0xC0)) b0 += 64;

    ((uint8_t *)dst)[0] = palConv[((r0 & 0xC0) >> 1) | ((g0 & 0xE0) >> 3) | ((b0 & 0xC0) >> 6)];
  }
}

static void FOG_FASTCALL convert_i8rgb222_from_rgb24_dither(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Point& origin, const uint8_t* palConv)
{
  sysint_t i;

  const uint8_t* dt = DitherMatrix::matrix[origin.getY() & DitherMatrix::Mask];
  int dx = origin.getX();

  for (i = w; i; i--, dx++, dst += 1, src += 3)
  {
    uint32_t r0, g0, b0, dith2;
    uint32_t r0t, g0t, b0t;

    dith2 = DitherMatrix::shf_dither(dt[dx & DitherMatrix::Mask], DitherMatrix::shf_arg(2)) * 4 / 3;

    r0 = src[RGB24_RByte];
    g0 = src[RGB24_GByte];
    b0 = src[RGB24_BByte];

    // c = c / 85
    r0t = (r0 * 0x00030000) >> 16;
    g0t = (g0 * 0x00030000) >> 16;
    b0t = (b0 * 0x00030000) >> 16;

    if ((r0 - (r0t * 85)) > dith2) r0t++;
    if ((g0 - (g0t * 85)) > dith2) g0t++;
    if ((b0 - (b0t * 85)) > dith2) b0t++;
    ((uint8_t *)dst)[0] = palConv[(r0t<<4)|(g0t<<2)|(b0t)];
  }
}

static void FOG_FASTCALL convert_i8rgb111_from_rgb24_dither(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Point& origin, const uint8_t* palConv)
{
  sysint_t i;

  const uint8_t* dt = DitherMatrix::matrix[origin.getY() & DitherMatrix::Mask];
  int dx = origin.getX();

  for (i = w; i; i--, dx++, dst += 1, src += 3)
  {
    uint32_t r0, g0, b0, dith1;

    dith1 = DitherMatrix::shf_dither(dt[dx & DitherMatrix::Mask], DitherMatrix::shf_arg(0));

    r0 = src[RGB24_RByte];
    g0 = src[RGB24_GByte];
    b0 = src[RGB24_BByte];

    if (r0 > dith1) r0 = 255;
    if (g0 > dith1) g0 = 255;
    if (b0 > dith1) b0 = 255;

    ((uint8_t *)dst)[0] = palConv[
      (((r0 + 1) >> 8) << 2) |
      (((g0 + 1) >> 8) << 1) |
      (((b0 + 1) >> 8)     ) ];
  }
}

static void FOG_FASTCALL convert_rgb16_5550_from_rgb32_dither(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Point& origin)
{
  sysint_t i;

  const uint8_t* dt = DitherMatrix::matrix[origin.getY() & DitherMatrix::Mask];
  int dx = origin.getX();

  for (i = w; i; i--, dx++, dst += 2, src += 4)
  {
    uint32_t c0;
    uint32_t r0, g0, b0, dith;

    c0 = ((uint32_t *)src)[0];
    r0 = (c0 & 0x00FF0000) >> 16;
    g0 = (c0 & 0x0000FF00) >> 8;
    b0 = (c0 & 0x000000FF);

    dith = DitherMatrix::shf_dither(dt[dx & DitherMatrix::Mask], DitherMatrix::shf_arg(5));
    if (((r0 & 7) >= dith) && (r0 < 0xF8)) r0 += 8;
    if (((g0 & 7) >= dith) && (g0 < 0xF8)) g0 += 8;
    if (((b0 & 7) >= dith) && (b0 < 0xF8)) b0 += 8;

    ((uint16_t *)dst)[0] = ((r0 & 0xF8) << 7) | ((g0 & 0xF8) << 2) | ((b0 & 0xF8) >> 3);
  }
}

static void FOG_FASTCALL convert_rgb16_5550_from_rgb24_dither(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Point& origin)
{
  sysint_t i;

  const uint8_t* dt = DitherMatrix::matrix[origin.getY() & DitherMatrix::Mask];
  int dx = origin.getX();

  for (i = w; i; i--, dx++, dst += 2, src += 3)
  {
    uint32_t r0, g0, b0, dith;

    r0 = src[RGB24_RByte];
    g0 = src[RGB24_GByte];
    b0 = src[RGB24_BByte];

    dith = DitherMatrix::shf_dither(dt[dx & DitherMatrix::Mask], DitherMatrix::shf_arg(5));
    if (((r0 & 7) >= dith) && (r0 < 0xF8)) r0 += 8;
    if (((g0 & 7) >= dith) && (g0 < 0xF8)) g0 += 8;
    if (((b0 & 7) >= dith) && (b0 < 0xF8)) b0 += 8;

    ((uint16_t *)dst)[0] = ((r0 & 0xF8) << 7) | ((g0 & 0xF8) << 2) | ((b0 & 0xF8) >> 3);
  }
}

static void FOG_FASTCALL convert_rgb16_5650_from_rgb32_dither(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Point& origin)
{
  sysint_t i;

  const uint8_t* dt = DitherMatrix::matrix[origin.getY() & DitherMatrix::Mask];
  int dx = origin.getX();

  for (i = w; i; i--, dx++, dst += 2, src += 4)
  {
    uint32_t c0;
    uint32_t r0, g0, b0, d, dith5, dith6;

    c0 = ((uint32_t *)src)[0];
    r0 = (c0 & 0x00FF0000) >> 16;
    g0 = (c0 & 0x0000FF00) >> 8;
    b0 = (c0 & 0x000000FF);

    d = dt[dx & DitherMatrix::Mask];
    dith5 = DitherMatrix::shf_dither(d, DitherMatrix::shf_arg(5));
    dith6 = DitherMatrix::shf_dither(d, DitherMatrix::shf_arg(6));
    if (((r0 & 7) >= dith5) && (r0 < 0xF8)) r0 += 8;
    if (((g0 & 3) >= dith6) && (g0 < 0xFC)) g0 += 4;
    if (((b0 & 7) >= dith5) && (b0 < 0xF8)) b0 += 8;

    ((uint16_t *)dst)[0] = ((r0 & 0xF8) << 8) | ((g0 & 0xFC) << 3) | ((b0 & 0xF8) >> 3);
  }
}

static void FOG_FASTCALL convert_rgb16_5650_from_rgb24_dither(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Point& origin)
{
  sysint_t i;

  const uint8_t* dt = DitherMatrix::matrix[origin.getY() & DitherMatrix::Mask];
  int dx = origin.getX();

  for (i = w; i; i--, dx++, dst += 2, src += 3)
  {
    uint32_t r0, g0, b0, d, dith5, dith6;

    r0 = src[RGB24_RByte];
    g0 = src[RGB24_GByte];
    b0 = src[RGB24_BByte];

    d = dt[dx & DitherMatrix::Mask];
    dith5 = DitherMatrix::shf_dither(d, DitherMatrix::shf_arg(5));
    dith6 = DitherMatrix::shf_dither(d, DitherMatrix::shf_arg(6));
    if (((r0 & 7) >= dith5) && (r0 < 0xF8)) r0 += 8;
    if (((g0 & 3) >= dith6) && (g0 < 0xFC)) g0 += 4;
    if (((b0 & 7) >= dith5) && (b0 < 0xF8)) b0 += 8;

    ((uint16_t *)dst)[0] = ((r0 & 0xF8) << 8) | ((g0 & 0xFC) << 3) | ((b0 & 0xF8) >> 3);
  }
}

static void FOG_FASTCALL convert_rgb16_5550_bs_from_rgb32_dither(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Point& origin)
{
  sysint_t i;

  const uint8_t* dt = DitherMatrix::matrix[origin.getY() & DitherMatrix::Mask];
  int dx = origin.getX();

  for (i = w; i; i--, dx++, dst += 2, src += 4)
  {
    uint32_t c0;
    uint32_t r0, g0, b0, dith;

    c0 = ((uint32_t *)src)[0];
    r0 = (c0 & 0x00FF0000) >> 16;
    g0 = (c0 & 0x0000FF00) >> 8;
    b0 = (c0 & 0x000000FF);

    dith = DitherMatrix::shf_dither(dt[dx & DitherMatrix::Mask], DitherMatrix::shf_arg(5));
    if (((r0 & 7) >= dith) && (r0 < 0xF8)) r0 += 8;
    if (((g0 & 7) >= dith) && (g0 < 0xF8)) g0 += 8;
    if (((b0 & 7) >= dith) && (b0 < 0xF8)) b0 += 8;

    ((uint16_t *)dst)[0] = Memory::bswap16(((r0 & 0xF8) << 7) | ((g0 & 0xF8) << 2) | ((b0 & 0xF8) >> 3));
  }
}

static void FOG_FASTCALL convert_rgb16_5550_bs_from_rgb24_dither(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Point& origin)
{
  sysint_t i;

  const uint8_t* dt = DitherMatrix::matrix[origin.getY() & DitherMatrix::Mask];
  int dx = origin.getX();

  for (i = w; i; i--, dx++, dst += 2, src += 3)
  {
    uint32_t r0, g0, b0, dith;

    r0 = src[RGB24_RByte];
    g0 = src[RGB24_GByte];
    b0 = src[RGB24_BByte];

    dith = DitherMatrix::shf_dither(dt[dx & DitherMatrix::Mask], DitherMatrix::shf_arg(5));
    if (((r0 & 7) >= dith) && (r0 < 0xF8)) r0 += 8;
    if (((g0 & 7) >= dith) && (g0 < 0xF8)) g0 += 8;
    if (((b0 & 7) >= dith) && (b0 < 0xF8)) b0 += 8;

    ((uint16_t *)dst)[0] = Memory::bswap16(((r0 & 0xF8) << 7) | ((g0 & 0xF8) << 2) | ((b0 & 0xF8) >> 3));
  }
}

static void FOG_FASTCALL convert_rgb16_5650_bs_from_rgb32_dither(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Point& origin)
{
  sysint_t i;

  const uint8_t* dt = DitherMatrix::matrix[origin.getY() & DitherMatrix::Mask];
  int dx = origin.getX();

  for (i = w; i; i--, dx++, dst += 2, src += 4)
  {
    uint32_t c0;
    uint32_t r0, g0, b0, d, dith5, dith6;

    c0 = ((uint32_t *)src)[0];
    r0 = (c0 & 0x00FF0000) >> 16;
    g0 = (c0 & 0x0000FF00) >> 8;
    b0 = (c0 & 0x000000FF);

    d = dt[dx & DitherMatrix::Mask];
    dith5 = DitherMatrix::shf_dither(d, DitherMatrix::shf_arg(5));
    dith6 = DitherMatrix::shf_dither(d, DitherMatrix::shf_arg(6));
    if (((r0 & 7) >= dith5) && (r0 < 0xF8)) r0 += 8;
    if (((g0 & 3) >= dith6) && (g0 < 0xFC)) g0 += 4;
    if (((b0 & 7) >= dith5) && (b0 < 0xF8)) b0 += 8;

    ((uint16_t *)dst)[0] = Memory::bswap16(((r0 & 0xF8) << 8) | ((g0 & 0xFC) << 3) | ((b0 & 0xF8) >> 3));
  }
}

static void FOG_FASTCALL convert_rgb16_5650_bs_from_rgb24_dither(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Point& origin)
{
  sysint_t i;

  const uint8_t* dt = DitherMatrix::matrix[origin.getY() & DitherMatrix::Mask];
  int dx = origin.getX();

  for (i = w; i; i--, dx++, dst += 2, src += 3)
  {
    uint32_t r0, g0, b0, d, dith5, dith6;

    r0 = src[RGB24_RByte];
    g0 = src[RGB24_GByte];
    b0 = src[RGB24_BByte];

    d = dt[dx & DitherMatrix::Mask];
    dith5 = DitherMatrix::shf_dither(d, DitherMatrix::shf_arg(5));
    dith6 = DitherMatrix::shf_dither(d, DitherMatrix::shf_arg(6));
    if (((r0 & 7) >= dith5) && (r0 < 0xF8)) r0 += 8;
    if (((g0 & 3) >= dith6) && (g0 < 0xFC)) g0 += 4;
    if (((b0 & 7) >= dith5) && (b0 < 0xF8)) b0 += 8;

    ((uint16_t *)dst)[0] = Memory::bswap16(((r0 & 0xF8) << 8) | ((g0 & 0xFC) << 3) | ((b0 & 0xF8) >> 3));
  }
}

} // Raster namespace
} // Fog namespace
