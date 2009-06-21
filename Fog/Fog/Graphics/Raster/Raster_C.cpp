// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/Math.h>
#include <Fog/Graphics/DitherMatrix.h>
#include <Fog/Graphics/Error.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/Pattern.h>
#include <Fog/Graphics/Raster.h>
#include <Fog/Graphics/Raster/Raster_C.h>
#include <Fog/Graphics/Raster/Raster_ByteOp.h>
#include <Fog/Graphics/Raster/Raster_PixelOp.h>
#include <Fog/Graphics/RgbaAnalyzer.h>

#include <math.h>

namespace Fog {
namespace Raster {

// ============================================================================
// [Fog::Raster - Defines]
// ============================================================================

#define READ_MASK_A8(ptr) ((const uint8_t *)ptr)[0]
#define READ_MASK_C8(ptr) ((const uint32_t*)ptr)[0]

static FOG_INLINE int double_to_int(double d) { return (int)d; }

static FOG_INLINE int64_t double_to_fixed48x16(double d) { return (int64_t)(d * 65536.0); }

// ============================================================================
// [Fog::Raster - Demultiply Reciprocal Table]
// ============================================================================

// Reciprocal table created by material provided by M Joonas Pihlaja, see
// <http://cgit.freedesktop.org/~joonas/unpremultiply/tree/README>
//
// Copyright (c) 2009  M Joonas Pihlaja
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

// The reciprocal_table[i] entries are defined by
//
//   0    when i = 0
//  255 / i    when i > 0
//
// represented in fixed point format with RECIPROCAL_BITS of
// precision and errors rounded up.
//
// #define ceil_div(a,b) ((a) + (b)-1) / (b)
//
// for each:
//   (i) ? ceil_div(255 * (1 << 16), (i)) : 0) : 0

uint32_t const demultiply_reciprocal_table[256] =
{
  0x00000000, 0x00FF0000, 0x007F8000, 0x00550000, 0x003FC000, 0x00330000, 0x002A8000, 0x00246DB7,
  0x001FE000, 0x001C5556, 0x00198000, 0x00172E8C, 0x00154000, 0x00139D8A, 0x001236DC, 0x00110000,
  0x000FF000, 0x000F0000, 0x000E2AAB, 0x000D6BCB, 0x000CC000, 0x000C2493, 0x000B9746, 0x000B1643,
  0x000AA000, 0x000A3334, 0x0009CEC5, 0x000971C8, 0x00091B6E, 0x0008CB09, 0x00088000, 0x000839CF,
  0x0007F800, 0x0007BA2F, 0x00078000, 0x00074925, 0x00071556, 0x0006E454, 0x0006B5E6, 0x000689D9,
  0x00066000, 0x00063832, 0x0006124A, 0x0005EE24, 0x0005CBA3, 0x0005AAAB, 0x00058B22, 0x00056CF0,
  0x00055000, 0x0005343F, 0x0005199A, 0x00050000, 0x0004E763, 0x0004CFB3, 0x0004B8E4, 0x0004A2E9,
  0x00048DB7, 0x00047944, 0x00046585, 0x00045271, 0x00044000, 0x00042E2A, 0x00041CE8, 0x00040C31,
  0x0003FC00, 0x0003EC4F, 0x0003DD18, 0x0003CE55, 0x0003C000, 0x0003B217, 0x0003A493, 0x00039770,
  0x00038AAB, 0x00037E40, 0x0003722A, 0x00036667, 0x00035AF3, 0x00034FCB, 0x000344ED, 0x00033A55,
  0x00033000, 0x000325EE, 0x00031C19, 0x00031282, 0x00030925, 0x00030000, 0x0002F712, 0x0002EE59,
  0x0002E5D2, 0x0002DD7C, 0x0002D556, 0x0002CD5D, 0x0002C591, 0x0002BDF0, 0x0002B678, 0x0002AF29,
  0x0002A800, 0x0002A0FE, 0x00029A20, 0x00029365, 0x00028CCD, 0x00028657, 0x00028000, 0x000279CA,
  0x000273B2, 0x00026DB7, 0x000267DA, 0x00026218, 0x00025C72, 0x000256E7, 0x00025175, 0x00024C1C,
  0x000246DC, 0x000241B3, 0x00023CA2, 0x000237A7, 0x000232C3, 0x00022DF3, 0x00022939, 0x00022493,
  0x00022000, 0x00021B82, 0x00021715, 0x000212BC, 0x00020E74, 0x00020A3E, 0x00020619, 0x00020205,
  0x0001FE00, 0x0001FA0C, 0x0001F628, 0x0001F253, 0x0001EE8C, 0x0001EAD4, 0x0001E72B, 0x0001E38F,
  0x0001E000, 0x0001DC80, 0x0001D90C, 0x0001D5A4, 0x0001D24A, 0x0001CEFB, 0x0001CBB8, 0x0001C881,
  0x0001C556, 0x0001C235, 0x0001BF20, 0x0001BC15, 0x0001B915, 0x0001B61F, 0x0001B334, 0x0001B052,
  0x0001AD7A, 0x0001AAAB, 0x0001A7E6, 0x0001A52A, 0x0001A277, 0x00019FCC, 0x00019D2B, 0x00019A91,
  0x00019800, 0x00019578, 0x000192F7, 0x0001907E, 0x00018E0D, 0x00018BA3, 0x00018941, 0x000186E6,
  0x00018493, 0x00018246, 0x00018000, 0x00017DC2, 0x00017B89, 0x00017958, 0x0001772D, 0x00017508,
  0x000172E9, 0x000170D1, 0x00016EBE, 0x00016CB2, 0x00016AAB, 0x000168AA, 0x000166AF, 0x000164B9,
  0x000162C9, 0x000160DE, 0x00015EF8, 0x00015D18, 0x00015B3C, 0x00015966, 0x00015795, 0x000155C8,
  0x00015400, 0x0001523E, 0x0001507F, 0x00014EC5, 0x00014D10, 0x00014B5F, 0x000149B3, 0x0001480B,
  0x00014667, 0x000144C7, 0x0001432C, 0x00014194, 0x00014000, 0x00013E71, 0x00013CE5, 0x00013B5D,
  0x000139D9, 0x00013859, 0x000136DC, 0x00013563, 0x000133ED, 0x0001327B, 0x0001310C, 0x00012FA1,
  0x00012E39, 0x00012CD5, 0x00012B74, 0x00012A16, 0x000128BB, 0x00012763, 0x0001260E, 0x000124BD,
  0x0001236E, 0x00012223, 0x000120DA, 0x00011F94, 0x00011E51, 0x00011D11, 0x00011BD4, 0x00011A99,
  0x00011962, 0x0001182C, 0x000116FA, 0x000115CA, 0x0001149D, 0x00011372, 0x0001124A, 0x00011124,
  0x00011000, 0x00010EE0, 0x00010DC1, 0x00010CA5, 0x00010B8B, 0x00010A73, 0x0001095E, 0x0001084B,
  0x0001073A, 0x0001062C, 0x0001051F, 0x00010415, 0x0001030D, 0x00010207, 0x00010103, 0x00010000
};

/*
static void FOG_FASTCALL demultiply_argb(uint32_t* dst, uint32_t const * src, sysuint_t w)
{
  for (sysuint_t i = w; i; i--, dst++, src++)
  {
    uint32_t rgba = src[0];
    uint32_t a = (rgba >> 24);
    uint32_t r = (rgba >> 16) & 255;
    uint32_t g = (rgba >>  8) & 255;
    uint32_t b = (rgba      ) & 255;
    uint32_t recip = demultiply_reciprocal_table[a];

    r = ((r * recip)      ) & 0x00FF0000;
    g = ((g * recip) >>  8) & 0x0000FF00;
    b = ((b * recip) >> 16) & 0x000000FF;

    dst[0] = r | g | b | (a << 24);
  }
}
*/

// ============================================================================
// [Fog::Raster - Convert - BSwap]
// ============================================================================

static void FOG_FASTCALL convert_bswap16(uint8_t* dst, const uint8_t* src, sysint_t w)
{
  for (sysint_t i = w; i; i--, dst += 2, src += 2)
  {
    ((uint16_t*)dst)[0] = Memory::bswap16(((const uint16_t*)src)[0]);
  }
}

static void FOG_FASTCALL convert_bswap24(uint8_t* dst, const uint8_t* src, sysint_t w)
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

static void FOG_FASTCALL convert_bswap32(uint8_t* dst, const uint8_t* src, sysint_t w)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 4)
  {
    ((uint32_t*)dst)[0] = Memory::bswap32(((const uint32_t*)src)[0]);
  }
}

// ============================================================================
// [Fog::Raster - Convert - MemCpy]
// ============================================================================

static void FOG_FASTCALL convert_memcpy8(uint8_t* dst, const uint8_t* src, sysint_t w)
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

static void FOG_FASTCALL convert_memcpy16(uint8_t* dst, const uint8_t* src, sysint_t w)
{
  convert_memcpy8(dst, src, mul2(w));
}

static void FOG_FASTCALL convert_memcpy24(uint8_t* dst, const uint8_t* src, sysint_t w)
{
  convert_memcpy8(dst, src, mul3(w));
}

static void FOG_FASTCALL convert_memcpy32(uint8_t* dst, const uint8_t* src, sysint_t w)
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
// [Fog::Raster - Convert - Argb32 Dest]
// ============================================================================

static void FOG_FASTCALL convert_argb32_from_prgb32(uint8_t* _dst, const uint8_t* _src, sysint_t w)
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
      recip = demultiply_reciprocal_table[a];

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
      recip = demultiply_reciprocal_table[a];

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

static void FOG_FASTCALL convert_argb32_from_prgb32_bs(uint8_t* _dst, const uint8_t* _src, sysint_t w)
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
      recip = demultiply_reciprocal_table[a];

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
      recip = demultiply_reciprocal_table[a];

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

static void FOG_FASTCALL convert_argb32_from_rgb32(uint8_t* dst, const uint8_t* src, sysint_t w)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 4)
  {
    ((uint32_t*)dst)[0] = ((const uint32_t*)src)[0] | 0xFF000000;
  }
}

static void FOG_FASTCALL convert_argb32_from_rgb32_bs(uint8_t* dst, const uint8_t* src, sysint_t w)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 4)
  {
    ((uint32_t*)dst)[0] = Memory::bswap32(((const uint32_t*)src)[0]) | 0xFF000000;
  }
}

static void FOG_FASTCALL convert_argb32_bs_from_rgb32(uint8_t* dst, const uint8_t* src, sysint_t w)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 4)
  {
    ((uint32_t*)dst)[0] = Memory::bswap32(((const uint32_t*)src)[0]) | 0x000000FF;
  }
}

static void FOG_FASTCALL convert_argb32_bs_from_prgb32(uint8_t* _dst, const uint8_t* _src, sysint_t w)
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
      recip = demultiply_reciprocal_table[a];

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
      recip = demultiply_reciprocal_table[a];

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

// ============================================================================
// [Fog::Raster - Convert - Prgb32 Dest]
// ============================================================================

static void FOG_FASTCALL convert_prgb32_from_argb32(uint8_t* dst, const uint8_t* src, sysint_t w)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 4)
  {
    ((uint32_t*)dst)[0] = premultiply(((const uint32_t*)src)[0]);
  }
}

static void FOG_FASTCALL convert_prgb32_from_argb32_bs(uint8_t* dst, const uint8_t* src, sysint_t w)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 4)
  {
    ((uint32_t*)dst)[0] = premultiply(Memory::bswap32(((const uint32_t*)src)[0]));
  }
}

static void FOG_FASTCALL convert_prgb32_from_i8(uint8_t* dst, const uint8_t* src, sysint_t w, const Rgba* pal)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 1)
  {
    ((uint32_t*)dst)[0] = premultiply(pal[src[0]]);
  }
}

static void FOG_FASTCALL convert_prgb32_bs_from_argb32(uint8_t* dst, const uint8_t* src, sysint_t w)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 4)
  {
    ((uint32_t*)dst)[0] = Memory::bswap32(premultiply(((const uint32_t*)src)[0]));
  }
}

static void FOG_FASTCALL convert_prgb32_bs_from_i8(uint8_t* dst, const uint8_t* src, sysint_t w, const Rgba* pal)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 1)
  {
    ((uint32_t*)dst)[0] = Memory::bswap32(premultiply(pal[src[0]]));
  }
}

// ============================================================================
// [Fog::Raster - Convert - Rgb32 Dest]
// ============================================================================

static void FOG_FASTCALL convert_rgb32_from_argb32(uint8_t* dst, const uint8_t* src, sysint_t w)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 4)
  {
    ((uint32_t*)dst)[0] = ((const uint32_t*)src)[0] | 0xFF000000;
  }
}

static void FOG_FASTCALL convert_rgb32_from_argb32_bs(uint8_t* dst, const uint8_t* src, sysint_t w)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 4)
  {
    ((uint32_t*)dst)[0] = Memory::bswap32(((const uint32_t*)src)[0] | 0xFF000000);
  }
}

static void FOG_FASTCALL convert_rgb32_from_rgb24(uint8_t* dst, const uint8_t* src, sysint_t w)
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

static void FOG_FASTCALL convert_rgb32_from_bgr24(uint8_t* dst, const uint8_t* src, sysint_t w)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 3)
  {
    ((uint32_t*)dst)[0] = PixFmt_BGR24::fetch(src) | 0xFF000000;
  }
}

static void FOG_FASTCALL convert_rgb32_from_rgb16_5550(uint8_t* dst, const uint8_t* src, sysint_t w)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 2)
  {
    ((uint32_t*)dst)[0] = PixFmt_RGB16_555::fetch(src) | 0xFF000000;
  }
}

static void FOG_FASTCALL convert_rgb32_from_rgb16_5550_bs(uint8_t* dst, const uint8_t* src, sysint_t w)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 2)
  {
    ((uint32_t*)dst)[0] = PixFmt_RGB16_555_BS::fetch(src) | 0xFF000000;
  }
}

static void FOG_FASTCALL convert_rgb32_from_rgb16_5650(uint8_t* dst, const uint8_t* src, sysint_t w)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 2)
  {
    ((uint32_t*)dst)[0] = PixFmt_RGB16_565::fetch(src) | 0xFF000000;
  }
}

static void FOG_FASTCALL convert_rgb32_from_rgb16_5650_bs(uint8_t* dst, const uint8_t* src, sysint_t w)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 2)
  {
    ((uint32_t*)dst)[0] = PixFmt_RGB16_565_BS::fetch(src) | 0xFF000000;
  }
}

static void FOG_FASTCALL convert_rgb32_from_i8(uint8_t* dst, const uint8_t* src, sysint_t w, const Rgba* pal)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 1)
  {
    ((uint32_t*)dst)[0] = pal[src[0]] | 0xFF000000;
  }
}

static void FOG_FASTCALL convert_rgb32_bs_from_rgb24(uint8_t* dst, const uint8_t* src, sysint_t w)
{
  for (sysint_t i = w; i; i--, dst += 4, src += 3)
  {
    ((uint32_t*)dst)[0] = Memory::bswap32(PixFmt_RGB24::fetch(src) | 0xFF000000);
  }
}

// ============================================================================
// [Fog::Raster - Convert - Rgb24/Bgr24 Dest]
// ============================================================================

static void FOG_FASTCALL convert_rgb24_from_rgb32(uint8_t* dst, const uint8_t* src, sysint_t w)
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

static void FOG_FASTCALL convert_rgb24_from_rgb32_bs(uint8_t* dst, const uint8_t* src, sysint_t w)
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

static void FOG_FASTCALL convert_rgb24_from_rgb16_5550(uint8_t* dst, const uint8_t* src, sysint_t w)
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

static void FOG_FASTCALL convert_rgb24_from_rgb16_5550_bs(uint8_t* dst, const uint8_t* src, sysint_t w)
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

static void FOG_FASTCALL convert_rgb24_from_rgb16_5650(uint8_t* dst, const uint8_t* src, sysint_t w)
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

static void FOG_FASTCALL convert_rgb24_from_rgb16_5650_bs(uint8_t* dst, const uint8_t* src, sysint_t w)
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

static void FOG_FASTCALL convert_rgb24_from_i8(uint8_t* dst, const uint8_t* src, sysint_t w, const Rgba* pal)
{
  sysint_t i = w;

#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
  // Align.
  while (i && (sysuint_t)dst & 0x3)
  {
    uint32_t s0 = pal[src[0]];

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
    uint32_t s0 = pal[src[0]];
    uint32_t s1 = pal[src[1]];
    uint32_t s2 = pal[src[2]];
    uint32_t s3 = pal[src[3]];

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
    uint32_t s0 = pal[src[0]];

    dst[RGB24_RByte] = (uint8_t)(s0 >> 8);
    dst[RGB24_GByte] = (uint8_t)(s0 >> 16);
    dst[RGB24_BByte] = (uint8_t)(s0 >> 24);

    dst += 3;
    src += 1;
    i--;
  }
}

static void FOG_FASTCALL convert_bgr24_from_rgb32(uint8_t* dst, const uint8_t* src, sysint_t w)
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

static void FOG_FASTCALL convert_bgr24_from_i8(uint8_t* dst, const uint8_t* src, sysint_t w, const Rgba* pal)
{
  sysint_t i = w;

  while (i)
  {
    uint32_t s0 = pal[src[0]];

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

static void FOG_FASTCALL convert_rgb16_5550_from_rgb32(uint8_t* dst, const uint8_t* src, sysint_t w)
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

static void FOG_FASTCALL convert_rgb16_5550_from_rgb24(uint8_t* dst, const uint8_t* src, sysint_t w)
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

static void FOG_FASTCALL convert_rgb16_5550_from_i8(uint8_t* dst, const uint8_t* src, sysint_t w, const Rgba* pal)
{
  sysint_t i = w;

  while (i)
  {
    uint32_t s0 = pal[src[0]];

    ((uint16_t*)dst)[0] =
      ((s0 >> 9) & 0x7C00) |
      ((s0 >> 6) & 0x03E0) |
      ((s0 >> 3)         ) ;

    dst += 2;
    src += 1;
    i--;
  }
}

static void FOG_FASTCALL convert_rgb16_5650_from_rgb32(uint8_t* dst, const uint8_t* src, sysint_t w)
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

static void FOG_FASTCALL convert_rgb16_5650_from_rgb24(uint8_t* dst, const uint8_t* src, sysint_t w)
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

static void FOG_FASTCALL convert_rgb16_5650_from_i8(uint8_t* dst, const uint8_t* src, sysint_t w, const Rgba* pal)
{
  sysint_t i = w;

  while (i)
  {
    uint32_t s0 = pal[src[0]];

    ((uint16_t*)dst)[0] =
      ((s0 >> 8) & 0xF800) |
      ((s0 >> 5) & 0x07E0) |
      ((s0 >> 3)         ) ;

    dst += 2;
    src += 1;
    i--;
  }
}

static void FOG_FASTCALL convert_rgb16_5550_bs_from_rgb32(uint8_t* dst, const uint8_t* src, sysint_t w)
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

static void FOG_FASTCALL convert_rgb16_5550_bs_from_rgb24(uint8_t* dst, const uint8_t* src, sysint_t w)
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

static void FOG_FASTCALL convert_rgb16_5550_bs_from_i8(uint8_t* dst, const uint8_t* src, sysint_t w, const Rgba* pal)
{
  sysint_t i = w;

  while (i)
  {
    uint32_t s0 = pal[src[0]];

    ((uint16_t*)dst)[0] = Memory::bswap16(
      ((s0 >> 9) & 0x7C00) |
      ((s0 >> 6) & 0x03E0) |
      ((s0 >> 3)         ) );

    dst += 2;
    src += 1;
    i--;
  }
}

static void FOG_FASTCALL convert_rgb16_5650_bs_from_rgb32(uint8_t* dst, const uint8_t* src, sysint_t w)
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

static void FOG_FASTCALL convert_rgb16_5650_bs_from_rgb24(uint8_t* dst, const uint8_t* src, sysint_t w)
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

static void FOG_FASTCALL convert_rgb16_5650_bs_from_i8(uint8_t* dst, const uint8_t* src, sysint_t w, const Rgba* pal)
{
  sysint_t i = w;

  while (i)
  {
    uint32_t s0 = pal[src[0]];

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
// [Fog::Raster - Convert - Greyscale]
// ============================================================================

// We are using this formula:
//  0.212671 * R + 0.715160 * G + 0.072169 * B;
// As:
//  (13938 * R + 46868 * G + 4730 * B) / 65536
static void FOG_FASTCALL convert_greyscale8_from_rgb32(uint8_t* dst, const uint8_t* src, sysint_t w)
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

static void FOG_FASTCALL convert_greyscale8_from_rgb24(uint8_t* dst, const uint8_t* src, sysint_t w)
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

static void FOG_FASTCALL convert_greyscale8_from_i8(uint8_t* dst, const uint8_t* src, sysint_t w, const Rgba* pal)
{
  sysint_t i = w;

  while (i)
  {
    uint32_t s0 = pal[src[0]];

    uint32_t grey =
      ((s0 >> 16) & 0xFF) * 13938 +
      ((s0 >>  8) & 0xFF) * 46868 +
      ((s0      ) & 0xFF) *  4730 ;

    dst[0] = grey >> 16;

    dst += 1;
    src += 1;
    i--;
  }
}

static void FOG_FASTCALL convert_rgb32_from_greyscale8(uint8_t* dst, const uint8_t* src, sysint_t w)
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

static void FOG_FASTCALL convert_rgb24_from_greyscale8(uint8_t* dst, const uint8_t* src, sysint_t w)
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

  const uint8_t* dt = DitherMatrix::matrix[origin.y() & DitherMatrix::Mask];
  int dx = origin.x();

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

  const uint8_t* dt = DitherMatrix::matrix[origin.y() & DitherMatrix::Mask];
  int dx = origin.x();

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

  const uint8_t* dt = DitherMatrix::matrix[origin.y() & DitherMatrix::Mask];
  int dx = origin.x();

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

  const uint8_t* dt = DitherMatrix::matrix[origin.y() & DitherMatrix::Mask];
  int dx = origin.x();

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

  const uint8_t* dt = DitherMatrix::matrix[origin.y() & DitherMatrix::Mask];
  int dx = origin.x();

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

  const uint8_t* dt = DitherMatrix::matrix[origin.y() & DitherMatrix::Mask];
  int dx = origin.x();

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

  const uint8_t* dt = DitherMatrix::matrix[origin.y() & DitherMatrix::Mask];
  int dx = origin.x();

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

  const uint8_t* dt = DitherMatrix::matrix[origin.y() & DitherMatrix::Mask];
  int dx = origin.x();

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

  const uint8_t* dt = DitherMatrix::matrix[origin.y() & DitherMatrix::Mask];
  int dx = origin.x();

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

  const uint8_t* dt = DitherMatrix::matrix[origin.y() & DitherMatrix::Mask];
  int dx = origin.x();

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

  const uint8_t* dt = DitherMatrix::matrix[origin.y() & DitherMatrix::Mask];
  int dx = origin.x();

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

  const uint8_t* dt = DitherMatrix::matrix[origin.y() & DitherMatrix::Mask];
  int dx = origin.x();

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

  const uint8_t* dt = DitherMatrix::matrix[origin.y() & DitherMatrix::Mask];
  int dx = origin.x();

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

  const uint8_t* dt = DitherMatrix::matrix[origin.y() & DitherMatrix::Mask];
  int dx = origin.x();

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

// ============================================================================
// [Fog::Raster - Gradient - gradient]
// ============================================================================

static void FOG_FASTCALL gradient_gradient_argb32(uint8_t* dst, uint32_t c0, uint32_t c1, sysint_t w, sysint_t x1, sysint_t x2)
{
  // This function is using fixed point 9.23 calculation to interpolate between
  // packed 8 bit integers. 9 bits are needed to extend 8 bit value with sign.
  uint8_t* dstCur = dst;

  // Sanity checks.
  FOG_ASSERT(w >= 0 && x1 <= x2);

  sysint_t xw = (x2 - x1);
  if (xw == 0) return;

  // Width is decreased by 1 to fit our gradient schema that first and last
  // points in interpolation are always equal to c0 and c1 recpectively.
  if (w) w--;

  // Loop counter.
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
    int grCur[4]; // Current value for each color.
    int grStp[4]; // Step for each color.

    // Calculate differences and step values.
    grCur[0] =   (int)((c0      ) & 0xFF) << 23;
    grStp[0] = (((int)((c1      ) & 0xFF) << 23) - grCur[0]) / (int)w;
    grCur[0] += grStp[0] * (int)x1;

    grCur[1] =   (int)((c0 >>  8) & 0xFF) << 23;
    grStp[1] = (((int)((c1 >>  8) & 0xFF) << 23) - grCur[1]) / (int)w;
    grCur[1] += grStp[1] * (int)x1;

    grCur[2] =   (int)((c0 >> 16) & 0xFF) << 23;
    grStp[2] = (((int)((c1 >> 16) & 0xFF) << 23) - grCur[2]) / (int)w;
    grCur[2] += grStp[2] * (int)x1;

    grCur[3] =   (int)((c0 >> 24)       ) << 23;
    grStp[3] = (((int)((c1 >> 24)       ) << 23) - grCur[3]) / (int)w;
    grCur[3] += grStp[3] * (int)x1;

    i = fog_min(w + 1, x2) - x1;
    x1 += i;

    do {
      set4(dstCur,
        ((uint32_t)(grCur[0] & (0xFF << 23)) >> 23) |
        ((uint32_t)(grCur[1] & (0xFF << 23)) >> 15) |
        ((uint32_t)(grCur[2] & (0xFF << 23)) >>  7) |
        ((uint32_t)(grCur[3] & (0xFF << 23)) <<  1) );
      dstCur += 4;

      grCur[0] += grStp[0];
      grCur[1] += grStp[1];
      grCur[2] += grStp[2];
      grCur[3] += grStp[3];
    } while (--i);

    if (x1 == x2) return;
  }

  // Fill c1 after gradient end.
  {
    i = x2 - x1;
    do { set4(dstCur, c1); dstCur += 4; } while (--i);
  }
}

static void FOG_FASTCALL gradient_gradient_rgb32(uint8_t* dst, uint32_t c0, uint32_t c1, sysint_t w, sysint_t x1, sysint_t x2);

static void FOG_FASTCALL gradient_gradient_prgb32(uint8_t* dst, uint32_t c0, uint32_t c1, sysint_t w, sysint_t x1, sysint_t x2)
{
  // Ooptimization: Do not premultiply if alpha is 0xFF
  if ((c0 & 0xFF000000) == 0xFF000000 && (c1 & 0xFF000000) == 0xFF000000)
  {
    gradient_gradient_rgb32(dst, c0, c1, w, x1, x2);
    return;
  }

  // This function is using fixed point 9.23 calculation to interpolate between
  // packed 8 bit integers. 9 bits are needed to extend 8 bit value with sign.
  uint8_t* dstCur = dst;

  // Sanity checks.
  FOG_ASSERT(w >= 0 && x1 <= x2);

  sysint_t xw = (x2 - x1);
  if (xw == 0) return;

  // Width is decreased by 1 to fit our gradient schema that first and last
  // points in interpolation are always equal to c0 and c1 recpectively.
  if (w) w--;

  // Loop counter.
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
    int grCur[4]; // Current value for each color.
    int grStp[4]; // Step for each color.

    // Calculate differences and step values.
    grCur[0] =   (int)((c0      ) & 0xFF) << 23;
    grStp[0] = (((int)((c1      ) & 0xFF) << 23) - grCur[0]) / (int)w;
    grCur[0] += grStp[0] * (int)x1;

    grCur[1] =   (int)((c0 >>  8) & 0xFF) << 23;
    grStp[1] = (((int)((c1 >>  8) & 0xFF) << 23) - grCur[1]) / (int)w;
    grCur[1] += grStp[1] * (int)x1;

    grCur[2] =   (int)((c0 >> 16) & 0xFF) << 23;
    grStp[2] = (((int)((c1 >> 16) & 0xFF) << 23) - grCur[2]) / (int)w;
    grCur[2] += grStp[2] * (int)x1;

    grCur[3] =   (int)((c0 >> 24)       ) << 23;
    grStp[3] = (((int)((c1 >> 24)       ) << 23) - grCur[3]) / (int)w;
    grCur[3] += grStp[3] * (int)x1;

    i = fog_min(w + 1, x2) - x1;
    x1 += i;

    do {
      uint32_t ta = ((uint32_t)(grCur[3] & (0xFF << 23)) >> 23) ;
      uint32_t t0 = ((uint32_t)(grCur[0] & (0xFF << 23)) >> 23) |
                    ((uint32_t)(grCur[2] & (0xFF << 23)) >>  7) ;
      uint32_t t1 = ((uint32_t)(grCur[1] & (0xFF << 23)) >> 15) ;

      t0 *= ta;
      t1 *= ta;

      t0 = ((t0 + ((t0 >> 8) & 0x00FF00FF) + 0x00800080) >> 8) & 0x00FF00FF;
      t1 = ((t1 + ((t1 >> 8) & 0x0000FF00) + 0x00008000) >> 8) & 0x0000FF00;

      set4(dstCur, t0 | t1 | (ta << 24));
      dstCur += 4;

      grCur[0] += grStp[0];
      grCur[1] += grStp[1];
      grCur[2] += grStp[2];
      grCur[3] += grStp[3];
    } while (--i);

    if (x1 == x2) return;
  }

  // Fill c1 after gradient end.
  {
    uint32_t c1p = premultiply(c1);
    i = x2 - x1;
    do { set4(dstCur, c1p); dstCur += 4; } while (--i);
  }
}

static void FOG_FASTCALL gradient_gradient_rgb32(uint8_t* dst, uint32_t c0, uint32_t c1, sysint_t w, sysint_t x1, sysint_t x2)
{
  // This function is using fixed point 9.23 calculation to interpolate between
  // packed 8 bit integers. 9 bits are needed to extend 8 bit value with sign.
  uint8_t* dstCur = dst;

  // Sanity checks.
  FOG_ASSERT(w >= 0 && x1 <= x2);

  sysint_t xw = (x2 - x1);
  if (xw == 0) return;

  // Width is decreased by 1 to fit our gradient schema that first and last
  // points in interpolation are always equal to c0 and c1 recpectively.
  if (w) w--;

  c0 |= 0xFF000000;
  c1 |= 0xFF000000;

  // Loop counter.
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
    int grCur[3]; // Current value for each color.
    int grStp[3]; // Step for each color.

    // Calculate differences and step values.
    grCur[0] =   (int)((c0      ) & 0xFF) << 23;
    grStp[0] = (((int)((c1      ) & 0xFF) << 23) - grCur[0]) / (int)w;
    grCur[0] += grStp[0] * (int)x1;

    grCur[1] =   (int)((c0 >>  8) & 0xFF) << 23;
    grStp[1] = (((int)((c1 >>  8) & 0xFF) << 23) - grCur[1]) / (int)w;
    grCur[1] += grStp[1] * (int)x1;

    grCur[2] =   (int)((c0 >> 16) & 0xFF) << 23;
    grStp[2] = (((int)((c1 >> 16) & 0xFF) << 23) - grCur[2]) / (int)w;
    grCur[2] += grStp[2] * (int)x1;

    i = fog_min(w + 1, x2) - x1;
    x1 += i;

    do {
      set4(dstCur,
        ((uint32_t)(grCur[0] & (0xFF << 23)) >> 23) |
        ((uint32_t)(grCur[1] & (0xFF << 23)) >> 15) |
        ((uint32_t)(grCur[2] & (0xFF << 23)) >>  7) |
        0xFF000000);
      dstCur += 4;

      grCur[0] += grStp[0];
      grCur[1] += grStp[1];
      grCur[2] += grStp[2];
    } while (--i);

    if (x1 == x2) return;
  }

  // Fill c1 after gradient end.
  {
    i = x2 - x1;
    do { set4(dstCur, c1); dstCur += 4; } while (--i);
  }
}

static void FOG_FASTCALL gradient_gradient_rgb24(uint8_t* dst, uint32_t c0, uint32_t c1, sysint_t w, sysint_t x1, sysint_t x2)
{
  // This function is using fixed point 9.23 calculation to interpolate between
  // packed 8 bit integers. 9 bits are needed to extend 8 bit value with sign.
  uint8_t* dstCur = dst;

  // Sanity checks.
  FOG_ASSERT(w >= 0 && x1 <= x2);

  sysint_t xw = (x2 - x1);
  if (xw == 0) return;

  // Width is decreased by 1 to fit our gradient schema that first and last
  // points in interpolation are always equal to c0 and c1 recpectively.
  if (w) w--;

  // Loop counter.
  sysint_t i;

  // Fill c0 before gradient start.
  if (x1 < 0)
  {
    i = fog_min((sysint_t)0, x2) - x1;

    xw -= i;
    x1 = 0;

    uint8_t c0r = c0 >> 16;
    uint8_t c0g = c0 >> 8;
    uint8_t c0b = c0;

    do {
      dstCur[RGB24_RByte] = c0r;
      dstCur[RGB24_GByte] = c0g;
      dstCur[RGB24_BByte] = c0b;
      dstCur += 3;
    } while (--i);

    if (xw == 0) return;
  }

  // Fill c0 to c1 using linear interpolation.
  if (x1 < w)
  {
    int grCur[3]; // Current value for each color.
    int grStp[3]; // Step for each color.

    // Calculate differences and step values.
    grCur[0] =   (int)((c0      ) & 0xFF) << 23;
    grStp[0] = (((int)((c1      ) & 0xFF) << 23) - grCur[0]) / (int)w;
    grCur[0] += grStp[0] * (int)x1;

    grCur[1] =   (int)((c0 >>  8) & 0xFF) << 23;
    grStp[1] = (((int)((c1 >>  8) & 0xFF) << 23) - grCur[1]) / (int)w;
    grCur[1] += grStp[1] * (int)x1;

    grCur[2] =   (int)((c0 >> 16) & 0xFF) << 23;
    grStp[2] = (((int)((c1 >> 16) & 0xFF) << 23) - grCur[2]) / (int)w;
    grCur[2] += grStp[2] * (int)x1;

    i = fog_min(w + 1, x2) - x1;
    x1 += i;

    do {
      dstCur[RGB24_RByte] = (uint8_t)(grCur[0] >> 23);
      dstCur[RGB24_GByte] = (uint8_t)(grCur[1] >> 23);
      dstCur[RGB24_BByte] = (uint8_t)(grCur[2] >> 23);
      dstCur += 3;

      grCur[0] += grStp[0];
      grCur[1] += grStp[1];
      grCur[2] += grStp[2];
    } while (--i);

    if (x1 == x2) return;
  }

  // Fill c1 after gradient end.
  {
    i = x2 - x1;

    uint8_t c1r = c1 >> 16;
    uint8_t c1g = c1 >> 8;
    uint8_t c1b = c1;

    do {
      dstCur[RGB24_RByte] = c1r;
      dstCur[RGB24_GByte] = c1g;
      dstCur[RGB24_BByte] = c1b;
      dstCur += 3;
    } while (--i);
  }
}

static void FOG_FASTCALL gradient_gradient_a8(uint8_t* dst, uint32_t c0, uint32_t c1, sysint_t w, sysint_t x1, sysint_t x2)
{
  // This function is using fixed point 9.23 calculation to interpolate between
  // packed 8 bit integers. 9 bits are needed to extend 8 bit value with sign.
  uint8_t* dstCur = dst;

  // Sanity checks.
  FOG_ASSERT(w >= 0 && x1 <= x2);

  sysint_t xw = (x2 - x1);
  if (xw == 0) return;

  // Width is decreased by 1 to fit our gradient schema that first and last
  // points in interpolation are always equal to c0 and c1 recpectively.
  if (w) w--;

  // Loop counter.
  sysint_t i;

  // Fill c0 before gradient start.
  if (x1 < 0)
  {
    i = fog_min((sysint_t)0, x2) - x1;

    xw -= i;
    x1 = 0;

    do { *dstCur++ = c0; } while (--i);
    if (xw == 0) return;
  }

  // Fill c0 to c1 using linear interpolation.
  if (x1 < w)
  {
    int grCur; // Current value for each color.
    int grStp; // Step for each color.

    // Calculate differences and step values.
    grCur =   (int)((c0      ) & 0xFF) << 23;
    grStp = (((int)((c1      ) & 0xFF) << 23) - grCur) / (int)w;
    grCur += grStp * (int)x1;

    i = fog_min(w + 1, x2) - x1;
    x1 += i;

    do {
      *dstCur++ = (uint32_t)(grCur >> 23);
      grCur += grStp;
    } while (--i);

    if (x1 == x2) return;
  }

  // Fill c1 after gradient end.
  {
    i = x2 - x1;
    do { *dstCur++ = c1; } while (--i);
  }
}

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
  i = fog_min(tw - x, w);
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

    i = fog_min(w, tw);
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
      int i = fog_min(tw2 - x, w);

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
      int i = fog_min(tw - x, w);

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

  double dxd = fog_abs(d->points[1].x() - d->points[0].x());
  double dyd = fog_abs(d->points[1].y() - d->points[0].y());
  double sqrtxxyy = sqrt(dxd * dxd + dyd * dyd);

  int64_t dx = double_to_fixed48x16(dxd);
  int64_t dy = double_to_fixed48x16(dyd);
  int64_t dmax = fog_max(dx, dy);
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
/*
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

  double angle;
  double ainc;
  int i;
 
reconfigure:
  if (dx < 0.0)
  {
    double stop;

    angle = atan2(dy, dx);
    stop = atan2(dy, -0.0000001);
    ainc = (stop - angle) / (-dx);

    i = (int)-dx;
    if (i < 1) i = 1;
    if (i > w) i = w;
  }
  else
  {
    double stop;

    angle = atan2(dy, 0.0000001);
    stop = atan2(dy, dx + (double)w);
    ainc = (stop - angle) / (dx + (double)w);
    angle += ainc * dx;

    i = w;
  }

  if (i > 16) i = 16;
  w -= i;
  dx += (double)i;
  angle += add;

  do {
    index = (int)(angle * scale);
    if (index >= colorsLength) index -= colorsLength;

    ((uint32_t*)dstCur)[0] = colors[index];
    dstCur += 4;

    angle += ainc;
  } while (--i);
  
  if (w) goto reconfigure;

  return dst;
*/
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

// ============================================================================
// [Fog::Raster - Operator - Base Macros and Generic Implementation]
// ============================================================================

#define BEGIN_OPERATOR_IMPL(OP_NAME, OP_INHERITS) \
  template<typename DstFmt, typename SrcFmt> \
  struct OP_NAME : public OP_INHERITS<OP_NAME<DstFmt, SrcFmt>, DstFmt, SrcFmt> \
  { \
    typedef OP_INHERITS<OP_NAME<DstFmt, SrcFmt>, DstFmt, SrcFmt> INHERITED; \
    using INHERITED::normalize_src_fetch; \
    using INHERITED::normalize_dst_fetch; \
    using INHERITED::normalize_dst_store;

#define END_OPERATOR_IMPL \
  }

template<typename Operator, typename DstFmt, typename SrcFmt>
struct Operator_Base
{
  static FOG_INLINE uint32_t normalize_src_fetch(uint32_t x)
  {
    if (!SrcFmt::HasAlpha && DstFmt::HasAlpha)
    {
      return x |= 0xFF000000;
    }
    else if (SrcFmt::HasAlpha && DstFmt::HasAlpha && !SrcFmt::IsPremultiplied)
    {
      return premultiply(x);
    }
    else
    {
      return x;
    }
  }

  static FOG_INLINE uint32_t normalize_dst_fetch(uint32_t x)
  {
    if (SrcFmt::HasAlpha && !DstFmt::HasAlpha)
    {
      return x |= 0xFF000000;
    }
    else if (SrcFmt::HasAlpha && DstFmt::HasAlpha && !DstFmt::IsPremultiplied)
    {
      return premultiply(x);
    }
    else
    {
      return x;
    }
  }

  static FOG_INLINE uint32_t normalize_dst_store(uint32_t x)
  {
    if (DstFmt::HasAlpha && !DstFmt::IsPremultiplied)
    {
      return demultiply(x);
    }
    else
    {
      return x;
    }
  }

  static FOG_INLINE void span_composite(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    for (sysint_t i = w; i; i--, dst += DstFmt::BytesPerPixel, src += SrcFmt::BytesPerPixel)
    {
      Operator::pixel(dst, src);
    }
  }

  static FOG_INLINE void span_composite_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    for (sysint_t i = w; i; i--, dst += DstFmt::BytesPerPixel, src += SrcFmt::BytesPerPixel, msk += 1)
    {
      Operator::pixel_a8(dst, src, READ_MASK_A8(msk));
    }
  }
};

template<typename Operator, typename DstFmt, typename SrcFmt>
struct Operator_Complex : public Operator_Base<Operator, DstFmt, SrcFmt>
{
  using Operator_Base<Operator, DstFmt, SrcFmt>::normalize_src_fetch;
  using Operator_Base<Operator, DstFmt, SrcFmt>::normalize_dst_fetch;
  using Operator_Base<Operator, DstFmt, SrcFmt>::normalize_dst_store;

  static FOG_INLINE void pixel(uint8_t* dst, const uint8_t* src)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));

    byte1x2 src0lo, src0hi;
    byte1x2 dst0lo, dst0hi;

    byte2x2_unpack_0213(dst0lo, dst0hi, dst0);
    byte2x2_unpack_0213(src0lo, src0hi, src0);

    Operator::op(dst0lo, dst0hi, src0lo, src0hi);

    DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(dst0lo, dst0hi)));
  }

  static FOG_INLINE void pixel_a8(uint8_t* dst, const uint8_t* src, uint32_t msk)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));

    byte1x2 src0lo, src0hi;
    byte1x2 dst0lo, dst0hi;

    byte2x2_unpack_0213(dst0lo, dst0hi, dst0);
    byte2x2_unpack_0213(src0lo, src0hi, src0);
    byte2x2_mul_u(src0lo, src0hi, msk);

    Operator::op(dst0lo, dst0hi, src0lo, src0hi);

    DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(dst0lo, dst0hi)));
  }

  static FOG_INLINE void span_solid(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    uint32_t dst0;

    byte1x2 src0lo, src0hi;
    byte1x2 dst0lo, dst0hi;

    byte2x2_unpack_0213(src0lo, src0hi, src0);

    do {
      dst0 = normalize_dst_fetch(DstFmt::fetch(dst));

      byte2x2_unpack_0213(dst0lo, dst0hi, dst0);
      Operator::op(dst0lo, dst0hi, src0lo, src0hi);
      DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(dst0lo, dst0hi)));

      dst += DstFmt::BytesPerPixel;
    } while(--w);
  }

  static FOG_INLINE void span_solid_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    uint32_t dst0;

    byte1x2 src0lo, src0hi;
    byte1x2 dst0lo, dst0hi;

    byte2x2_unpack_0213(src0lo, src0hi, src0);

    do {
      dst0 = normalize_dst_fetch(DstFmt::fetch(dst));

      byte2x2_unpack_0213(dst0lo, dst0hi, dst0);
      Operator::op(dst0lo, dst0hi, src0lo, src0hi);
      DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(dst0lo, dst0hi)));

      dst += DstFmt::BytesPerPixel;
    } while(--w);
  }

  static FOG_INLINE void span_composite(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    do {
      uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
      uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));

      byte1x2 dst0lo, dst0hi;
      byte1x2 src0lo, src0hi;

      byte2x2_unpack_0213(dst0lo, dst0hi, dst0);
      byte2x2_unpack_0213(src0lo, src0hi, src0);
      Operator::op(dst0lo, dst0hi, src0lo, src0hi);
      DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(dst0lo, dst0hi)));

      dst += DstFmt::BytesPerPixel;
      src += DstFmt::BytesPerPixel;
    } while (--w);
  }

  static FOG_INLINE void span_composite_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    do {
      uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
      uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
      uint32_t msk0 = READ_MASK_A8(msk);

      byte1x2 dst0lo, dst0hi;
      byte1x2 src0lo, src0hi;

      byte2x2_unpack_0213(dst0lo, dst0hi, dst0);
      byte2x2_unpack_0213(src0lo, src0hi, src0);
      if (msk0 != 0xFF) byte2x2_mul_u(src0lo, src0hi, msk0);
      Operator::op(dst0lo, dst0hi, src0lo, src0hi);
      DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(dst0lo, dst0hi)));

      dst += DstFmt::BytesPerPixel;
      src += DstFmt::BytesPerPixel;
      msk += 1;
    } while (--w);
  }
};

// ============================================================================
// [Fog::Raster - Operator_Src]
// ============================================================================

// Dca' = Sca
// Da'  = Sa
BEGIN_OPERATOR_IMPL(Operator_Src, Operator_Base)
  static FOG_INLINE void pixel(uint8_t* dst, const uint8_t* src)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    DstFmt::store(dst, normalize_dst_store(src0));
  }

  static FOG_INLINE void pixel_a8(uint8_t* dst, const uint8_t* src, uint32_t msk)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    if (DstFmt::HasAlpha && msk != 0xFF) src0 = bytemul(src0, msk);
    DstFmt::store(dst, normalize_dst_store(src0));
  }

  static FOG_INLINE void span_solid(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    for (sysint_t i = w; i; i--, dst += DstFmt::BytesPerPixel)
    {
      DstFmt::store(dst, normalize_dst_store(src0));
    }
  }

  static FOG_INLINE void span_solid_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    if (DstFmt::HasAlpha)
    {
      uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
      byte1x2 pix0;
      byte1x2 pix1;
      byte2x2_unpack_0213(pix0, pix1, src0);

      for (sysint_t i = w; i; i--, dst += DstFmt::BytesPerPixel, msk += 1)
      {
        uint32_t p = src0;
        uint32_t m = READ_MASK_A8(msk);

        if (m != 0xFF)
        {
          byte1x2 p0 = pix0;
          byte1x2 p1 = pix1;
          byte2x2_mul_u(p0, p1, m);
          p = byte2x2_pack_0213(p0, p1);
        }

        DstFmt::store(dst, normalize_dst_store(p));
      }
    }
    else
    {
      span_solid(dst, src, w);
    }
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Dst]
// ============================================================================

// Dca' = Dca
// Da'  = Da
BEGIN_OPERATOR_IMPL(Operator_Dst, Operator_Base)
  static FOG_INLINE void pixel(uint8_t* dst, const uint8_t* src)
  {
  }

  static FOG_INLINE void pixel_a8(uint8_t* dst, const uint8_t* src, uint32_t msk)
  {
  }

  static FOG_INLINE void span_solid(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
  }

  static FOG_INLINE void span_solid_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Over]
// ============================================================================

// Dca' = Sca + Dca.(1 - Sa)
// Da'  = Sa + Da.(1 - Sa)
//      = Sa + Da - Sa.Da
BEGIN_OPERATOR_IMPL(Operator_Over, Operator_Base)
  static FOG_INLINE void pixel(uint8_t* dst, const uint8_t* src)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    uint32_t a0 = getAlpha(~src0);

    if (a0 != 0x00) src0 = bytemuladd(normalize_dst_fetch(DstFmt::fetch(dst)), a0, src0);

    DstFmt::store(dst, normalize_dst_store(src0));
  }

  static FOG_INLINE void pixel_a8(uint8_t* dst, const uint8_t* src, uint32_t msk)
  {
    if (msk == 0) return;

    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    byte1x2 src0lo, src0hi;

    byte2x2_unpack_0213(src0lo, src0hi, src0);
    byte2x2_mul_u(src0lo, src0hi, msk);

    uint32_t src0a = byte1x2_hi(src0hi);
    if (src0a == 0xFF)
    {
      DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(src0lo, src0hi)));
    }
    else
    {
      uint32_t dst0 = DstFmt::fetch(dst);
      byte1x2 dst0lo, dst0hi;

      byte2x2_unpack_0213(dst0lo, dst0hi, src0);
      byte2x2_muladd_u_byte2x2(dst0lo, dst0hi, 255 - src0a, src0lo, src0hi);
      DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(dst0lo, dst0hi)));
    }
  }

  static FOG_INLINE void span_solid(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    if (!SrcFmt::HasAlpha)
    {
      Operator_Src<DstFmt, SrcFmt>::span_solid(dst, src, w);
      return;
    }
    else
    {
      uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
      uint32_t dst0;
      uint32_t ia0 = (~src0) >> 24;

      if (ia0 == 0x00)
      {
        Operator_Src<DstFmt, SrcFmt>::span_solid(dst, src, w);
      }
      else
      {
        byte1x2 src0lo, src0hi;
        byte1x2 dst0lo, dst0hi;
        byte2x2_unpack_0213(src0lo, src0hi, src0);

        do {
          dst0 = normalize_dst_fetch(DstFmt::fetch(dst));

          byte2x2_unpack_0213(dst0lo, dst0hi, dst0);
          byte2x2_muladd_u_byte2x2(dst0lo, dst0hi, ia0, src0lo, src0hi);
          DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(dst0lo, dst0hi)));

          dst += DstFmt::BytesPerPixel;
        } while (--w);
      }
    }
  }

  static FOG_INLINE void span_solid_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    if (!SrcFmt::HasAlpha)
    {
      Operator_Src<DstFmt, SrcFmt>::span_solid_a8(dst, src, msk, w);
      return;
    }
    else
    {
      uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
      uint32_t dst0;
      uint32_t a0 = src0 >> 24;
      uint32_t msk0;

      byte1x2 src0lo, src0hi;
      byte1x2 dst0lo, dst0hi;
      byte1x2 pix0lo, pix0hi;

      byte2x2_unpack_0213(src0lo, src0hi, src0);

      if (a0 == 0xFF)
      {
        do {
          if ((msk0 = READ_MASK_A8(msk)) != 0)
          {
            dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
            pix0lo = src0lo;
            pix0hi = src0hi;
            byte2x2_unpack_0213(dst0lo, dst0hi, dst0);
            byte2x2_mul_u(pix0lo, pix0hi, msk0);
            byte2x2_muladd_u_byte2x2(dst0lo, dst0hi, 255 - msk0, pix0lo, pix0hi);
            DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(dst0lo, dst0hi)));
          }

          dst += DstFmt::BytesPerPixel;
          msk += 1;
        } while (--w);
      }
      else
      {
        do {
          if ((msk0 = READ_MASK_A8(msk)) != 0)
          {
            dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
            pix0lo = src0lo;
            pix0hi = src0hi;
            byte2x2_unpack_0213(dst0lo, dst0hi, dst0);
            byte2x2_mul_u(pix0lo, pix0hi, msk0);
            byte2x2_muladd_u_byte2x2(dst0lo, dst0hi, 255 - byte1x2_hi(pix0hi), dst0lo, dst0hi);
            DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(dst0lo, dst0hi)));
          }

          dst += DstFmt::BytesPerPixel;
          msk += 1;
        } while (--w);
      }
    }
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_OverReverse]
// ============================================================================

// Dca' = Dca + Sca.(1 - Da)
// Da'  = Da + Sa.(1 - Da)
//      = Da + Sa - Da.Sa
BEGIN_OPERATOR_IMPL(Operator_OverReverse, Operator_Base)
  static FOG_INLINE void pixel(uint8_t* dst, const uint8_t* src)
  {
    uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    if (src0)
    {
      uint32_t dst0ia = 255 - getAlpha(dst0);
      DstFmt::store(dst, normalize_dst_store(bytemuladd(src0, dst0ia, dst0)));
    }
  }

  static FOG_INLINE void pixel_a8(uint8_t* dst, const uint8_t* src, uint32_t msk)
  {
    if (msk == 0x00) return;

    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));

    if (src0)
    {
      byte1x2 dst0lo, dst0hi;
      byte1x2 src0lo, src0hi;

      uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
      uint32_t dst0a = getAlpha(dst0);

      byte2x2_unpack_0213(dst0lo, dst0hi, dst0);
      byte2x2_unpack_0213(src0lo, src0hi, src0);
      if (msk != 0xFF) byte2x2_mul_u(src0lo, src0hi, msk);
      byte2x2_muladd_u_byte2x2(src0lo, src0hi, 255 - dst0a, dst0lo, dst0hi);
      DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(dst0lo, dst0hi)));
    }
  }

  static FOG_INLINE void span_solid(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    uint32_t dst0;
    uint32_t a0;

    if (!src0) return;

    byte1x2 src0lo, src0hi;
    byte1x2 dst0lo, dst0hi;
    byte1x2 pix0lo, pix0hi;

    byte2x2_unpack_0213(src0lo, src0hi, src0);

    do {
      dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
      a0 = dst0 >> 24;

      if (a0 != 0xFF)
      {
        byte2x2_unpack_0213(dst0lo, dst0hi, dst0);
        pix0lo = src0lo;
        pix0hi = src0hi;
        byte2x2_muladd_u_byte2x2(pix0lo, pix0hi, 255 - a0, dst0lo, dst0hi);
        DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(pix0lo, pix0hi)));
      }

      dst += DstFmt::BytesPerPixel;
    } while (--w);
  }

  static FOG_INLINE void span_solid_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    uint32_t dst0;
    uint32_t a0;
    uint32_t msk0;

    if (!src0) return;

    byte1x2 src0lo, src0hi;
    byte1x2 dst0lo, dst0hi;
    byte1x2 pix0lo, pix0hi;

    byte2x2_unpack_0213(src0lo, src0hi, src0);

    do {
      if ((msk0 = READ_MASK_A8(msk)) != 0)
      {
        dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
        a0 = dst0 >> 24;

        if (a0 != 0xFF)
        {
          byte2x2_unpack_0213(dst0lo, dst0hi, dst0);
          pix0lo = src0lo;
          pix0hi = src0hi;
          byte2x2_mul_u(pix0lo, pix0hi, msk0);
          byte2x2_muladd_u_byte2x2(pix0lo, pix0hi, 255 - a0, dst0lo, dst0hi);
          DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(pix0lo, pix0hi)));
        }
      }

      dst += DstFmt::BytesPerPixel;
      msk += 1;
    } while (--w);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_In]
// ============================================================================

// Dca' = Sca.Da
// Da'  = Sa.Da
BEGIN_OPERATOR_IMPL(Operator_In, Operator_Base)
  static FOG_INLINE void pixel(uint8_t* dst, const uint8_t* src)
  {
    uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
    if (dst0)
    {
      uint32_t a0 = getAlpha(dst0);
      uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
      DstFmt::store(dst, normalize_dst_store(bytemul(src0, a0)));
    }
  }

  static FOG_INLINE void pixel_a8(uint8_t* dst, const uint8_t* src, uint32_t msk)
  {
    uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
    if (dst0)
    {
      uint32_t a0 = getAlpha(dst0);
      uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
      if (msk != 0xFF) a0 = div255(a0 * msk);
      DstFmt::store(dst, normalize_dst_store(bytemul(src0, a0)));
    }
  }

  static FOG_INLINE void span_solid(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    sysint_t i = w;
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));

    do {
      uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
      if (dst0)
      {
        uint32_t a0 = getAlpha(dst0);
        DstFmt::store(dst, normalize_dst_store(bytemul(src0, a0)));
      }

      dst += DstFmt::BytesPerPixel;
    } while (--i);
  }

  static FOG_INLINE void span_solid_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    sysint_t i = w;
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));

    do {
      uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
      if (dst0)
      {
        uint32_t a0 = getAlpha(dst0);
        uint32_t m0 = READ_MASK_A8(msk);
        uint32_t s0 = src0;
        if (m0 != 0xFF) s0 = bytemul(s0, m0);
        DstFmt::store(dst, normalize_dst_store(bytemul(s0, a0)));
      }

      dst += DstFmt::BytesPerPixel;
      msk += 1;
    } while (--i);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_InReverse]
// ============================================================================

// Dca' = Dca.Sa
// Da'  = Da.Sa
BEGIN_OPERATOR_IMPL(Operator_InReverse, Operator_Base)
  static FOG_INLINE void pixel(uint8_t* dst, const uint8_t* src)
  {
    if (SrcFmt::HasAlpha)
    {
      uint32_t a0 = SrcFmt::fetchAlpha(src);
      if (a0 != 0xFF)
      {
        uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
        DstFmt::store(dst, normalize_dst_store(bytemul(dst0, a0)));
      }
    }
  }

  static FOG_INLINE void pixel_a8(uint8_t* dst, const uint8_t* src, uint32_t msk)
  {
    if (SrcFmt::HasAlpha)
    {
      uint32_t a0 = div255(SrcFmt::fetchAlpha(src) * msk);
      if (a0 != 0xFF)
      {
        uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
        DstFmt::store(dst, normalize_dst_store(bytemul(dst0, a0)));
      }
    }
    else
    {
      if (msk != 0xFF)
      {
        uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
        DstFmt::store(dst, normalize_dst_store(bytemul(dst0, msk)));
      }
    }
  }

  static FOG_INLINE void span_solid(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    if (SrcFmt::HasAlpha)
    {
      sysint_t i = w;
      uint32_t a0 = SrcFmt::fetchAlpha(src);
      if (a0 == 0xFF) return;

      do {
        uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
        DstFmt::store(dst, normalize_dst_store(bytemul(dst0, a0)));

        dst += DstFmt::BytesPerPixel;
      } while (--i);
    }
  }

  static FOG_INLINE void span_solid_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    if (SrcFmt::HasAlpha)
    {
      sysint_t i = w;
      uint32_t a0 = SrcFmt::fetchAlpha(src);

      if (a0 == 0xFF)
      {
        do {
          uint32_t m0 = READ_MASK_A8(msk);
          if (m0 != 0xFF)
          {
            uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
            DstFmt::store(dst, normalize_dst_store(bytemul(dst0, m0)));
          }

          dst += DstFmt::BytesPerPixel;
          msk += 1;
        } while (--i);
      }
      else
      {
        do {
          uint32_t m0 = div255(a0 * READ_MASK_A8(msk));
          uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
          DstFmt::store(dst, normalize_dst_store(bytemul(dst0, m0)));

          dst += DstFmt::BytesPerPixel;
          msk += 1;
        } while (--i);
      }
    }
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Out]
// ============================================================================

// Dca' = Sca.(1 - Da)
// Da'  = Sa.(1 - Da)
BEGIN_OPERATOR_IMPL(Operator_Out, Operator_Base)
  static FOG_INLINE void pixel(uint8_t* dst, const uint8_t* src)
  {
    uint32_t a0 = 255 - DstFmt::fetchAlpha(dst);
    uint32_t src0 = 0;
    if (a0 != 0) src0 = bytemul(normalize_src_fetch(SrcFmt::fetch(src)), a0);
    DstFmt::store(dst, normalize_dst_store(src0));
  }

  static FOG_INLINE void pixel_a8(uint8_t* dst, const uint8_t* src, uint32_t msk)
  {
    uint32_t a0 = 255 - DstFmt::fetchAlpha(dst);
    uint32_t src0 = bytemul(SrcFmt::fetch(src), div255(a0 * msk));
    DstFmt::store(dst, normalize_dst_store(src0));
  }

  static FOG_INLINE void span_solid(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    sysint_t i = w;

    do {
      uint32_t a0 = 255 - DstFmt::fetchAlpha(dst);
      DstFmt::store(dst, normalize_dst_store(bytemul(src0, a0)));

      dst += DstFmt::BytesPerPixel;
    } while (--i);
  }

  static FOG_INLINE void span_solid_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    sysint_t i = w;

    do {
      uint32_t a0 = 255 - div255(DstFmt::fetchAlpha(dst) * READ_MASK_A8(msk));
      DstFmt::store(dst, normalize_dst_store(bytemul(src0, a0)));

      dst += DstFmt::BytesPerPixel;
      msk += 1;
    } while (--i);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_OutReverse]
// ============================================================================

// Dca' = Dca.(1 - Sa)
// Da'  = Da.(1 - Sa)
BEGIN_OPERATOR_IMPL(Operator_OutReverse, Operator_Base)
  static FOG_INLINE void pixel(uint8_t* dst, const uint8_t* src)
  {
    uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
    if (dst0)
    {
      uint32_t ia0 = 255 - SrcFmt::fetchAlpha(src);
      DstFmt::store(dst, normalize_dst_store(bytemul(dst0, ia0)));
    }
  }

  static FOG_INLINE void pixel_a8(uint8_t* dst, const uint8_t* src, uint32_t msk)
  {
    uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
    if (dst0)
    {
      uint32_t ia0 = div255((255 - SrcFmt::fetchAlpha(src)) * msk);
      DstFmt::store(dst, normalize_dst_store(bytemul(dst0, ia0)));
    }
  }

  static FOG_INLINE void span_solid(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    uint32_t ia0 = 255 - SrcFmt::fetchAlpha(src);
    sysint_t i = w;

    if (ia0 == 0xFF) return;

    do {
      uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
      if (dst0)
      {
        DstFmt::store(dst, normalize_dst_store(bytemul(dst0, ia0)));
      }

      dst += DstFmt::BytesPerPixel;
    } while (--i);
  }

  static FOG_INLINE void span_solid_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    uint32_t a0 = SrcFmt::fetchAlpha(src);
    sysint_t i = w;

    if (a0 == 0x00) return;

    if (a0 == 0xFF)
    {
      do {
        uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
        if (dst0)
        {
          DstFmt::store(dst, normalize_dst_store(bytemul(dst0, 255 - READ_MASK_A8(msk))));
        }

        dst += DstFmt::BytesPerPixel;
        msk += 1;
      } while (--i);
    }
    else
    {
      do {
        uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
        if (dst0)
        {
          DstFmt::store(dst, normalize_dst_store(bytemul(dst0, 255 - div255(a0 * READ_MASK_A8(msk)))));
        }

        dst += DstFmt::BytesPerPixel;
        msk += 1;
      } while (--i);
    }
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Atop]
// ============================================================================

// Dca' = Sca.Da + Dca.(1 - Sa)
// Da'  = Sa.Da + Da.(1 - Sa)
//      = Da.(Sa + 1 - Sa)
//      = Da
BEGIN_OPERATOR_IMPL(Operator_Atop, Operator_Base)
  static FOG_INLINE void pixel(uint8_t* dst, const uint8_t* src)
  {
    uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
    if (dst0)
    {
      uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
      uint32_t dsta0 = getAlpha(dst0);
      uint32_t srci0 = getAlpha(~src0);
      DstFmt::store(dst, normalize_dst_store(byteaddmul(src0, dsta0, dst0, srci0)));
    }
  }

  static FOG_INLINE void pixel_a8(uint8_t* dst, const uint8_t* src, uint32_t msk)
  {
    uint32_t dst0 = DstFmt::fetch(dst);
    if (dst0)
    {
      uint32_t src0 = SrcFmt::fetch(src);
      if (msk != 0xFF) src0 = bytemul(src0, msk);
      uint32_t dsta0 = getAlpha(dst0);
      uint32_t srci0 = getAlpha(~src0);
      DstFmt::store(dst, normalize_dst_store(byteaddmul(src0, dsta0, dst0, srci0)));
    }
  }

  static FOG_INLINE void span_solid(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    sysint_t i = w;
    uint32_t src0 = SrcFmt::fetch(src);
    uint32_t srci0 = getAlpha(~src0);

    do {
      uint32_t dst0 = DstFmt::fetch(dst);
      if (dst0)
      {
        uint32_t dsta0 = getAlpha(dst0);
        DstFmt::store(dst, normalize_dst_store(byteaddmul(src0, dsta0, dst0, srci0)));
      }

      dst += DstFmt::BytesPerPixel;
    } while(--i);
  }

  static FOG_INLINE void span_solid_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    sysint_t i = w;
    uint32_t src0 = SrcFmt::fetch(src);

    do {
      uint32_t dst0 = DstFmt::fetch(dst);
      if (dst0)
      {
        uint32_t srcm0 = bytemul(src0, READ_MASK_A8(msk));
        uint32_t srci0 = getAlpha(~srcm0);
        uint32_t dsta0 = getAlpha(dst0);
        DstFmt::store(dst, normalize_dst_store(byteaddmul(srcm0, dsta0, dst0, srci0)));
      }

      dst += DstFmt::BytesPerPixel;
      msk += 1;
    } while(--i);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_AtopReverse]
// ============================================================================

// Dca' = Dca.Sa + Sca.(1 - Da)
// Da'  = Da.Sa + Sa.(1 - Da)
//      = Sa.(Da + 1 - Da)
//      = Sa
BEGIN_OPERATOR_IMPL(Operator_AtopReverse, Operator_Base)
  static FOG_INLINE void pixel(uint8_t* dst, const uint8_t* src)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
    uint32_t srca0 = getAlpha(src0);
    uint32_t dsti0 = getAlpha(~dst0);
    DstFmt::store(dst, normalize_dst_store(byteaddmul(src0, dsti0, dst0, srca0)));
  }

  static FOG_INLINE void pixel_a8(uint8_t* dst, const uint8_t* src, uint32_t msk)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
    if (msk != 0xFF) src0 = bytemul(src0, msk);
    uint32_t srca0 = getAlpha(src0);
    uint32_t dsti0 = getAlpha(~dst0);
    DstFmt::store(dst, normalize_dst_store(byteaddmul(src0, dsti0, dst0, srca0)));
  }

  static FOG_INLINE void span_solid(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    sysint_t i = w;

    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    uint32_t srca0 = getAlpha(src0);

    do {
      uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
      uint32_t dsti0 = getAlpha(~dst0);
      DstFmt::store(dst, normalize_dst_store(byteaddmul(src0, dsti0, dst0, srca0)));

      dst += DstFmt::BytesPerPixel;
    } while (--i);
  }

  static FOG_INLINE void span_solid_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    sysint_t i = w;

    do {
      uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
      uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
      uint32_t m0 = READ_MASK_A8(msk);
      if (m0 != 0xFF) src0 = bytemul(src0, m0);
      uint32_t srca0 = getAlpha(src0);
      uint32_t dsti0 = getAlpha(~dst0);
      DstFmt::store(dst, normalize_dst_store(byteaddmul(src0, dsti0, dst0, srca0)));

      dst += DstFmt::BytesPerPixel;
      msk += 1;
    } while (--i);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Xor]
// ============================================================================

// Dca' = Sca.(1 - Da) + Dca.(1 - Sa)
// Da'  = Sa.(1 - Da) + Da.(1 - Sa)
//      = Sa + Da - 2.Sa.Da
BEGIN_OPERATOR_IMPL(Operator_Xor, Operator_Base)
  static FOG_INLINE void pixel(uint8_t* dst, const uint8_t* src)
  {
    if (SrcFmt::HasAlpha)
    {
      uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
      uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
      uint32_t srci0 = getAlpha(~src0);
      uint32_t dsti0 = getAlpha(~dst0);
      DstFmt::store(dst, normalize_dst_store(byteaddmul(src0, dsti0, dst0, srci0)));
    }
    else
    {
      Operator_Over<DstFmt, SrcFmt>::pixel(dst, src);
    }
  }

  static FOG_INLINE void pixel_a8(uint8_t* dst, const uint8_t* src, uint32_t msk)
  {
    uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    if (msk != 0xFF) src0 = bytemul(src0, msk);
    uint32_t srci0 = getAlpha(~src0);
    uint32_t dsti0 = getAlpha(~dst0);
    DstFmt::store(dst, normalize_dst_store(byteaddmul(src0, dsti0, dst0, srci0)));
  }

  static FOG_INLINE void span_solid(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    if (SrcFmt::HasAlpha)
    {
      sysint_t i = w;

      uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
      uint32_t srci0 = getAlpha(~src0);

      do {
        uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
        uint32_t dsti0 = getAlpha(~dst0);
        DstFmt::store(dst, normalize_dst_store(byteaddmul(src0, dsti0, dst0, srci0)));
        dst += DstFmt::BytesPerPixel;
      } while (--i);
    }
    else
    {
      Operator_Over<DstFmt, SrcFmt>::span_solid(dst, src, w);
    }
  }

  static FOG_INLINE void span_solid_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    sysint_t i = w;

    do {
      uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
      uint32_t srcm0 = src0;
      uint32_t m0 = READ_MASK_A8(msk);
      if (m0 != 0xFF) srcm0 = bytemul(srcm0, m0);
      uint32_t srci0 = getAlpha(~srcm0);
      uint32_t dsti0 = getAlpha(~dst0);
      DstFmt::store(dst, normalize_dst_store(byteaddmul(srcm0, dsti0, dst0, srci0)));
      dst += DstFmt::BytesPerPixel;
      msk += 1;
    } while (--i);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Clear]
// ============================================================================

// Dca' = 0
// Da'  = 0
BEGIN_OPERATOR_IMPL(Operator_Clear, Operator_Base)
  static FOG_INLINE void pixel(uint8_t* dst, const uint8_t* src)
  {
    DstFmt::store(dst, 0);
  }

  static FOG_INLINE void pixel_a8(uint8_t* dst, const uint8_t* src, uint32_t msk)
  {
    DstFmt::store(dst, 0);
  }

  static FOG_INLINE void span_solid(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    sysint_t i = w;
    do {
      DstFmt::store(dst, 0);
    } while (--i);
  }

  static FOG_INLINE void span_solid_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    sysint_t i = w;
    do {
      DstFmt::store(dst, 0);
    } while (--i);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Add]
// ============================================================================

// Dca' = Sca + Dca
// Da'  = Sa + Da
BEGIN_OPERATOR_IMPL(Operator_Add, Operator_Complex)
  static FOG_INLINE void op(byte1x2& dst0lo, byte1x2& dst0hi, byte1x2 src0lo, byte1x2 src0hi)
  {
    byte2x2_adds_byte2x2(dst0lo, dst0hi, src0lo, src0hi);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Subtract]
// ============================================================================

// Dca' = Dca - Sca
// Da'  = 1 - (1 - Sa).(1 - Da)
BEGIN_OPERATOR_IMPL(Operator_Subtract, Operator_Complex)
  static FOG_INLINE void op(byte1x2& dst0lo, byte1x2& dst0hi, byte1x2 src0lo, byte1x2 src0hi)
  {
    uint32_t a0 = 255 - div255(byte1x2_hi(~dst0hi) * byte1x2_hi(~src0hi));

    byte2x2_subs_byte2x2(dst0lo, dst0hi, src0lo, src0hi);
    dst0hi &= 0x00FF0000;
    dst0hi |= a0 << 16;
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Multiply]
// ============================================================================

// Dca' = Sca.Dca + Sca.(1 - Da) + Dca.(1 - Sa)
// Da'  = Sa.Da + Sa.(1 - Da) + Da.(1 - Sa)
//      = Sa + Da - Sa.Da
BEGIN_OPERATOR_IMPL(Operator_Multiply, Operator_Complex)
  static FOG_INLINE void op(byte1x2& dst0lo, byte1x2& dst0hi, byte1x2 src0lo, byte1x2 src0hi)
  {
    byte1x2 res0lo = dst0lo;
    byte1x2 res0hi = dst0hi;
    byte2x2_mul_byte2x2(res0lo, res0hi, src0lo, src0hi);

    uint32_t srcia = 255 - byte1x2_hi(src0hi);
    uint32_t dstia = 255 - byte1x2_hi(dst0hi);

    byte2x2_mul_u(dst0lo, dst0hi, srcia);
    byte2x2_mul_u(src0lo, src0hi, dstia);

    byte2x2_adds_byte2x2(dst0lo, dst0hi, res0lo, res0hi);
    byte2x2_adds_byte2x2(dst0lo, dst0hi, src0lo, src0hi);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Screen]
// ============================================================================

// Dca' = (Sca.Da + Dca.Sa - Sca.Dca) + Sca.(1 - Da) + Dca.(1 - Sa)
//      = Sca + Dca - Sca.Dca
// Da'  = Sa + Da - Sa.Da
BEGIN_OPERATOR_IMPL(Operator_Screen, Operator_Complex)
  static FOG_INLINE void op(byte1x2& dst0lo, byte1x2& dst0hi, byte1x2 src0lo, byte1x2 src0hi)
  {
    byte1x2 res0lo = dst0lo, res0hi = dst0hi;
    byte2x2_mul_byte2x2(res0lo, res0hi, src0lo, src0hi);
    byte2x2_addsub_byte2x2(dst0lo, dst0hi, src0lo, src0hi, res0lo, res0hi);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Darken]
// ============================================================================

// Dca' = min(Sca.Da, Dca.Sa) + Sca.(1 - Da) + Dca.(1 - Sa)
// Da'  = min(Sa.Da, Da.Sa) + Sa.(1 - Da) + Da.(1 - Sa)
//      = Sa.Da + Sa - Sa.Da + Da - Sa.Da
//      = Sa + Da - Sa.Da
BEGIN_OPERATOR_IMPL(Operator_Darken, Operator_Complex)
  static FOG_INLINE void op(byte1x2& dst0lo, byte1x2& dst0hi, byte1x2 src0lo, byte1x2 src0hi)
  {
    byte1x2 res0lo = src0lo, res0hi = src0hi;
    byte1x2 res1lo = dst0lo, res1hi = dst0hi;

    byte2x2_mul_u(res0lo, res0hi, byte1x2_hi(dst0hi));
    byte2x2_mul_u(res1lo, res1hi, byte1x2_hi(src0hi));
    byte2x2_min(res0lo, res0hi, res1lo, res1hi);

    uint32_t dstia = 255 - byte1x2_hi(dst0hi);
    uint32_t srcia = 255 - byte1x2_hi(src0hi);

    byte2x2_mul_u(dst0lo, dst0hi, srcia);
    byte2x2_mul_u(src0lo, src0hi, dstia);

    byte2x2_adds_byte2x2(dst0lo, dst0hi, src0lo, src0hi);
    byte2x2_adds_byte2x2(dst0lo, dst0hi, res0lo, res0hi);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Lighten]
// ============================================================================

// Dca' = max(Sca.Da, Dca.Sa) + Sca.(1 - Da) + Dca.(1 - Sa)
// Da'  = max(Sa.Da, Da.Sa) + Sa.(1 - Da) + Da.(1 - Sa)
//      = Sa.Da + Sa - Sa.Da + Da - Sa.Da
//      = Sa + Da - Sa.Da
BEGIN_OPERATOR_IMPL(Operator_Lighten, Operator_Complex)
  static FOG_INLINE void op(byte1x2& dst0lo, byte1x2& dst0hi, byte1x2 src0lo, byte1x2 src0hi)
  {
    byte1x2 res0lo = src0lo, res0hi = src0hi;
    byte1x2 res1lo = dst0lo, res1hi = dst0hi;

    byte2x2_mul_u(res0lo, res0hi, byte1x2_hi(dst0hi));
    byte2x2_mul_u(res1lo, res1hi, byte1x2_hi(src0hi));
    byte2x2_max(res0lo, res0hi, res1lo, res1hi);

    uint32_t dstia = 255 - byte1x2_hi(dst0hi);
    uint32_t srcia = 255 - byte1x2_hi(src0hi);

    byte2x2_mul_u(dst0lo, dst0hi, srcia);
    byte2x2_mul_u(src0lo, src0hi, dstia);

    byte2x2_adds_byte2x2(dst0lo, dst0hi, src0lo, src0hi);
    byte2x2_adds_byte2x2(dst0lo, dst0hi, res0lo, res0hi);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Difference]
// ============================================================================

// Dca' = Sca + Dca - 2.min(Sca.Da, Dca.Sa)
// Da'  = Sa + Da - min(Sa.Da, Da.Sa)
//      = Sa + Da - Sa.Da
BEGIN_OPERATOR_IMPL(Operator_Difference, Operator_Complex)
  static FOG_INLINE void op(byte1x2& dst0lo, byte1x2& dst0hi, byte1x2 src0lo, byte1x2 src0hi)
  {
    byte1x2 res0lo = src0lo, res0hi = src0hi;
    byte1x2 res1lo = dst0lo, res1hi = dst0hi;

    byte2x2_mul_u(res0lo, res0hi, byte1x2_hi(dst0hi));
    byte2x2_mul_u(res1lo, res1hi, byte1x2_hi(src0hi));
    byte2x2_min(res0lo, res0hi, res1lo, res1hi);
    byte2x2_lshift_by_1_no_alpha(res0lo, res0hi);
    byte2x2_addsub_byte2x2(dst0lo, dst0hi, src0lo, src0hi, res0lo, res0hi);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Exclusion]
// ============================================================================

// Dca' = (Sca.Da + Dca.Sa - 2.Sca.Dca) + Sca.(1 - Da) + Dca.(1 - Sa)
// Dca' = Sca + Dca - 2.Sca.Dca
//
// Da'  = (Sa.Da + Da.Sa - 2.Sa.Da) + Sa.(1 - Da) + Da.(1 - Sa)
//      = Sa - Sa.Da + Da - Da.Sa = Sa + Da - 2.Sa.Da
// Substitute 2.Sa.Da with Sa.Da
//
// Da'  = Sa + Da - Sa.Da
BEGIN_OPERATOR_IMPL(Operator_Exclusion, Operator_Complex)
  static FOG_INLINE void op(byte1x2& dst0lo, byte1x2& dst0hi, byte1x2 src0lo, byte1x2 src0hi)
  {
    byte1x2 res0lo = dst0lo, res0hi = dst0hi;
    byte2x2_mul_byte2x2(res0lo, res0hi, src0lo, src0hi);
    byte2x2_lshift_by_1_no_alpha(res0lo, res0hi);
    byte2x2_addsub_byte2x2(dst0lo, dst0hi, src0lo, src0hi, res0lo, res0hi);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Invert]
// ============================================================================

// Dca' = (Da - Dca) * Sa + Dca.(1 - Sa)
// Da'  = Sa + (Da - Da) * Sa + Da - Sa.Da
//      = Sa + Da - Sa.Da
//
// For calculation this formula is best:
// Dca' = (Da - Dca) * Sa + Dca.(1 - Sa)
// Da'  = (1 + Da - Da) * Sa + Da.(1 - Sa)
BEGIN_OPERATOR_IMPL(Operator_Invert, Operator_Complex)
  static FOG_INLINE void op(byte1x2& dst0lo, byte1x2& dst0hi, byte1x2 src0lo, byte1x2 src0hi)
  {
    byte1x2 dstalo;
    byte1x2 dstahi;

    byte2x2_expand_hi(dstalo, dstahi, dst0hi); dstahi += 0x00FF0000;
    byte2x2_sub_byte2x2(dstalo, dstahi, dst0lo, dst0hi);
    byte2x2_mul_u(dstalo, dstahi, byte1x2_hi(src0hi));
    byte2x2_mul_u(dst0lo, dst0hi, 255 - byte1x2_hi(src0hi));
    byte2x2_adds_byte2x2(dst0lo, dst0hi, dstalo, dstahi);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_InvertRgb]
// ============================================================================

// Dca' = (Da - Dca) * Sca + Dca.(1 - Sa)
// Da'  = Sa + (Da - Da) * Sa + Da - Da.Sa
//      = Sa + Da - Sa.Da
//
// For calculation this formula is best:
// Dca' = (Da - Dca) * Sca + Dca.(1 - Sa)
// Da'  = (1 + Da - Da) * Sa + Da.(1 - Sa)
BEGIN_OPERATOR_IMPL(Operator_InvertRgb, Operator_Complex)
  static FOG_INLINE void op(byte1x2& dst0lo, byte1x2& dst0hi, byte1x2 src0lo, byte1x2 src0hi)
  {
    byte1x2 dstalo;
    byte1x2 dstahi;

    byte2x2_expand_hi(dstalo, dstahi, dst0hi); dstahi += 0x00FF0000;
    byte2x2_sub_byte2x2(dstalo, dstahi, dst0lo, dst0hi);
    byte2x2_mul_byte2x2(dstalo, dstahi, src0lo, src0hi);
    byte2x2_mul_u(dst0lo, dst0hi, 255 - byte1x2_hi(src0hi));
    byte2x2_adds_byte2x2(dst0lo, dst0hi, dstalo, dstahi);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Raster - NOP]
// ============================================================================

static void FOG_FASTCALL raster_pixel_nop(
  uint8_t* dst, uint32_t src)
{
}

static void FOG_FASTCALL raster_pixel_a8_nop(
  uint8_t* dst, uint32_t src, uint32_t msk)
{
}

static void FOG_FASTCALL raster_span_solid_nop(
  uint8_t* dst, uint32_t src, sysint_t w)
{
}

static void FOG_FASTCALL raster_span_solid_a8_nop(
  uint8_t* dst, uint32_t src, const uint8_t* msk, sysint_t w)
{
}

static void FOG_FASTCALL raster_span_composite_nop(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
}

static void FOG_FASTCALL raster_span_composite_a8_nop(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
}

static void FOG_FASTCALL raster_span_composite_indexed_nop(
  uint8_t* dst, const uint8_t* src, sysint_t w,
  const Rgba* pal)
{
}

static void FOG_FASTCALL raster_span_composite_indexed_a8_nop(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w,
  const Rgba* pal)
{
}

// ============================================================================
// [Fog::Raster - Raster - General]
// ============================================================================

template <typename Operator>
static void FOG_FASTCALL raster_pixel(
  uint8_t* dst, uint32_t src)
{
  Operator::pixel(dst, (const uint8_t*)&src);
}

template <typename Operator>
static void FOG_FASTCALL raster_pixel_a8(
  uint8_t* dst, uint32_t src, uint32_t msk)
{
  Operator::pixel_a8(dst, (const uint8_t*)&src, msk);
}

template <typename Operator>
static void FOG_FASTCALL raster_span_solid(
  uint8_t* dst, uint32_t src, sysint_t w)
{
  Operator::span_solid(dst, (const uint8_t*)&src, w);
}

template <typename Operator>
static void FOG_FASTCALL raster_span_solid_a8(
  uint8_t* dst, uint32_t src, const uint8_t* msk, sysint_t w)
{
  Operator::span_solid_a8(dst, (const uint8_t*)&src, msk, w);
}

template <typename Operator>
static void FOG_FASTCALL raster_span_composite(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  Operator::span_composite(dst, src, w);
}

template <typename Operator>
static void FOG_FASTCALL raster_span_composite_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  Operator::span_composite_a8(dst, src, msk, w);
}

// ============================================================================
// [Fog::Raster - Raster - Agb32 - SrcOver]
// ============================================================================
#if 0
static void FOG_FASTCALL raster_argb32_pixel_srcover(
  uint8_t* dst, uint32_t src)
{
  uint32_t a = src >> 24;

  if (a != 0xFF)
    src = blend_srcover_nonpremultiplied(((uint32_t*)dst)[0], src, a);

  ((uint32_t*)dst)[0] = src;
}

static void FOG_FASTCALL raster_argb32_pixel_a8_srcover(
  uint8_t* dst, uint32_t src, uint32_t msk)
{
  uint32_t a = div255((src >> 24) * msk);

  if (a != 0xFF)
    src = blend_srcover_nonpremultiplied(((uint32_t*)dst)[0], src, a);

  ((uint32_t*)dst)[0] = src;
}

static void FOG_FASTCALL raster_argb32_span_solid_srcover(
  uint8_t* dst, uint32_t src, sysint_t w)
{
  sysint_t i = w;
  uint32_t a = src >> 24;

  if (a != 0xFF)
  {
    src = bytemul(src, a);
    a = 255 - a;

    do {
      ((uint32_t*)dst)[0] = bytemul(((uint32_t*)dst)[0], a) + src;
      dst += 4;
    } while (--w);
  }
  else
  {
    do {
      ((uint32_t*)dst)[0] = src;
      dst += 4;
    } while (--w);
  }
}

static void FOG_FASTCALL raster_argb32_span_solid_a8_srcover(
  uint8_t* dst, uint32_t src, const uint8_t* msk, sysint_t w)
{
  sysint_t i = w;
  uint32_t a = src >> 24;
  uint32_t m;

  if (a != 0xFF)
  {
    src = bytemul(src, a);
    a = 255 - a;

    do {
      if ((m = READ_MASK_A8(msk)) == 0xFF)
      {
        ((uint32_t*)dst)[0] = bytemul(((uint32_t*)dst)[0], a) + src;
      }
      else if (m)
      {
        uint32_t src0 = bytemul(src0, m);
      }
      dst += 4;
      msk += 1;
    } while (--w);
  }
  else
  {
    do {
      if ((m = READ_MASK_A8(msk)) == 0xFF)
      {
        ((uint32_t*)dst)[0] = src;
      }
      else if (m)
      {
        ((uint32_t*)dst)[0] = bytemul(((uint32_t*)dst)[0], 255 - m) + bytemul(src, m);
      }
      dst += 4;
      msk += 1;
    } while (--w);
  }
}
#endif

// ============================================================================
// [Fog::Raster - Raster - Rgb32]
// ============================================================================

static void FOG_FASTCALL raster_rgb32_pixel(
  uint8_t* dst, uint32_t src)
{
  uint32_t a = src >> 24;

  if (a != 0xFF)
  {
    src = blend_over_srcpremultiplied(((uint32_t*)dst)[0], src, a);
  }

  ((uint32_t*)dst)[0] = src;
}

static void FOG_FASTCALL raster_rgb32_pixel_a8(
  uint8_t* dst, uint32_t src, uint32_t msk)
{
  uint32_t a = src >> 24;

  if (a != 0xFF || msk != 0xFF)
  {
    src = bytemul_reset_alpha(src, msk);
    src = blend_over_srcpremultiplied(((uint32_t*)dst)[0], src, src >> 24);
  }

  ((uint32_t*)dst)[0] = src;
}

static void FOG_FASTCALL raster_rgb32_span_solid(
  uint8_t* dst, uint32_t src, sysint_t w)
{
  sysint_t i = w;
  uint32_t a = src >> 24;

  if (a != 0xFF)
  {
    src |= 0xFF000000;
    a = 255 - a;

    do {
      ((uint32_t*)dst)[0] = bytemul_reset_alpha(((uint32_t*)dst)[0], a) + src;
      dst += 4;
    } while (--w);
  }
  else
  {
    // This is C optimized version of memfill for 32-bit/64-bit architectures.
    // On most x86 modern systems it will be always replaced by MMX / SSE2
    // version, so it's mainly for other architectures.
    sysuint_t src0 = unpackU32ToSysUInt(src);

#if FOG_ARCH_BITS == 64
    // Align.
    if ((sysuint_t)dst & 0x7)
    {
      ((uint32_t*)dst)[0] = src;
      dst += 4;
      i--;
    }
#endif

    while (i >= 8)
    {
      set32(dst, src0);
      dst += 32;
      i -= 8;
    }

    switch (i)
    {
      case 7: ((uint32_t*)dst)[0] = src; dst += 4;
      case 6: ((uint32_t*)dst)[0] = src; dst += 4;
      case 5: ((uint32_t*)dst)[0] = src; dst += 4;
      case 4: ((uint32_t*)dst)[0] = src; dst += 4;
      case 3: ((uint32_t*)dst)[0] = src; dst += 4;
      case 2: ((uint32_t*)dst)[0] = src; dst += 4;
      case 1: ((uint32_t*)dst)[0] = src; dst += 4;
    }
  }
}

static void FOG_FASTCALL raster_rgb32_span_solid_a8(
  uint8_t* dst, uint32_t src, const uint8_t* msk, sysint_t w)
{
  sysint_t i = w;
  uint32_t a = src >> 24;
  uint32_t m;

  if (a != 0xFF)
  {
    uint32_t FFsrc = src | 0xFF000000;
    uint32_t ia = 255 - a;

    do {
      if ((m = READ_MASK_A8(msk)) == 0xFF)
      {
        ((uint32_t*)dst)[0] = bytemul_reset_alpha(((uint32_t*)dst)[0], ia) + FFsrc;
      }
      else if (m)
      {
        uint32_t srcm = bytemul(src, m);
        ((uint32_t*)dst)[0] = blend_over_srcpremultiplied(((uint32_t*)dst)[0], srcm, srcm >> 24);
      }
      dst += 4;
      msk += 1;
    } while (--w);
  }
  else
  {
    do {
      if ((m = READ_MASK_A8(msk)) == 0xFF)
      {
        ((uint32_t*)dst)[0] = src;
      }
      else if (m)
      {
        ((uint32_t*)dst)[0] = blend_over_nonpremultiplied(((uint32_t*)dst)[0], src, m);
      }
      dst += 4;
      msk += 1;
    } while (--w);
  }
}

static void FOG_FASTCALL raster_rgb32_span_composite_argb32(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  do {
    uint32_t src0 = ((uint32_t*)src)[0];
    uint32_t a0 = src0 >> 24;

    if (a0 == 0xFF)
    {
      ((uint32_t*)dst)[0] = src0;
    }
    else if (a0)
    {
      ((uint32_t*)dst)[0] = bytemul(((uint32_t*)dst)[0], 255 - a0) + bytemul(src0, a0);
    }
    dst += 4;
    src += 4;
  } while (--w);
}

static void FOG_FASTCALL raster_rgb32_span_composite_prgb32(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  do {
    uint32_t src0 = ((uint32_t*)src)[0];
    uint32_t a0 = src0 >> 24;

    if (a0 == 0xFF)
    {
      ((uint32_t*)dst)[0] = src0;
    }
    else if (a0)
    {
      ((uint32_t*)dst)[0] = bytemul(((uint32_t*)dst)[0], 255 - a0) + src0;
    }
    dst += 4;
    src += 4;
  } while (--w);
}

static void FOG_FASTCALL raster_rgb32_span_composite_indexed(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Rgba* pal)
{
  do {
    uint32_t src0 = pal[src[0]];
    uint32_t a0 = src0 >> 24;
    if (a0)
    {
      if (a0 != 0xFF) src0 = bytemul(((uint32_t*)dst)[0], 255 - a0) + bytemul(src0, a0);
      ((uint32_t*)dst)[0] = src0;
    }

    dst += 4;
    src += 1;
  } while (--w);
}

static void FOG_FASTCALL raster_rgb32_span_composite_argb32_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  do {
    uint32_t m;
    if ((m = READ_MASK_A8(msk)))
    {
      uint32_t src0 = ((uint32_t*)src)[0];
      uint32_t a0 = src0 >> 24;
      if (m != 0xFF) a0 = div255(a0 * m);
      ((uint32_t*)dst)[0] = bytemul(((uint32_t*)dst)[0], 255 - a0) + bytemul(src0, a0);
    }

    dst += 4;
    src += 4;
    msk += 1;
  } while (--w);
}

static void FOG_FASTCALL raster_rgb32_span_composite_prgb32_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  do {
    uint32_t m;
    if ((m = READ_MASK_A8(msk)))
    {
      uint32_t src0 = ((uint32_t*)src)[0];
      if (m != 0xFF) src0 = bytemul(src0, m);
      ((uint32_t*)dst)[0] = blend_over_srcpremultiplied(((uint32_t*)dst)[0], src0, src0 >> 24);
    }

    dst += 4;
    src += 4;
    msk += 1;
  } while (--w);
}

static void FOG_FASTCALL raster_rgb32_span_composite_rgb32_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  do {
    uint32_t m;
    if ((m = READ_MASK_A8(msk)))
    {
      uint32_t src0 = ((uint32_t*)src)[0];
      if (m != 0xFF) src0 = bytemul(((uint32_t*)dst)[0], 255 - m) + bytemul(src0, m);
      ((uint32_t*)dst)[0] = src0;
    }

    dst += 4;
    src += 4;
    msk += 1;
  } while (--w);
}

static void FOG_FASTCALL raster_rgb32_span_composite_rgb24_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  do {
    uint32_t m;
    if ((m = READ_MASK_A8(msk)))
    {
      uint32_t src0 = PixFmt_RGB24::fetch(src);
      if (m != 0xFF) src0 = bytemul(((uint32_t*)dst)[0], 255 - m) + bytemul(src0, m);
      ((uint32_t*)dst)[0] = src0;
    }

    dst += 4;
    src += 3;
    msk += 1;
  } while (--w);
}

static void FOG_FASTCALL raster_rgb32_span_composite_indexed_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Rgba* pal)
{
  do {
    uint32_t m;
    if ((m = READ_MASK_A8(msk)))
    {
      uint32_t src0 = pal[src[0]];
      uint32_t a0 = src0 >> 24;
      if (m != 0xFF) a0 = div255(a0 * m);
      ((uint32_t*)dst)[0] = bytemul(((uint32_t*)dst)[0], 255 - a0) + bytemul(src0, a0);
    }

    dst += 4;
    src += 1;
    msk += 1;
  } while (--w);
}

// ============================================================================
// [Fog::Raster - Raster - Rgb24]
// ============================================================================

static void FOG_FASTCALL raster_rgb24_pixel(
  uint8_t* dst, uint32_t src)
{
}

static void FOG_FASTCALL raster_rgb24_pixel_a8(
  uint8_t* dst, uint32_t src, uint32_t msk)
{
}

static void FOG_FASTCALL raster_rgb24_span_solid(
  uint8_t* dst, uint32_t src, sysint_t w)
{
}

static void FOG_FASTCALL raster_rgb24_span_solid_a8(
  uint8_t* dst, uint32_t src, const uint8_t* msk, sysint_t w)
{
}

static void FOG_FASTCALL raster_rgb24_span_composite_argb32(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
}

static void FOG_FASTCALL raster_rgb24_span_composite_prgb32(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
}

static void FOG_FASTCALL raster_rgb24_span_composite_argb32_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
}

static void FOG_FASTCALL raster_rgb24_span_composite_prgb32_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
}

static void FOG_FASTCALL raster_rgb24_span_composite_rgb32_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
}

static void FOG_FASTCALL raster_rgb24_span_composite_rgb24_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
}

} // Raster namespace
} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

#define TODO_NOT_IMPLEMENTED NULL

FOG_INIT_DECLARE void fog_raster_init_c(void)
{
  using namespace Fog;
  using namespace Fog::Raster;

  sysint_t i;
  FunctionMap* m = functionMap;

  // [Convert - ByteSwap]

  m->convert.bswap16 = convert_bswap16;
  m->convert.bswap24 = convert_bswap24;
  m->convert.bswap32 = convert_bswap32;

  // [Convert - MemCpy]

  m->convert.memcpy8 = convert_memcpy8;
  m->convert.memcpy16 = convert_memcpy16;
  m->convert.memcpy24 = convert_memcpy24;
  m->convert.memcpy32 = convert_memcpy32;

  // [Convert - Argb32 Dest]

  m->convert.argb32_from_prgb32 = convert_argb32_from_prgb32;
  m->convert.argb32_from_prgb32_bs = convert_argb32_from_prgb32_bs;
  m->convert.argb32_from_rgb32 = convert_argb32_from_rgb32;
  m->convert.argb32_from_rgb32_bs = convert_argb32_from_rgb32_bs;

  m->convert.argb32_bs_from_rgb32 = convert_argb32_bs_from_rgb32;
  m->convert.argb32_bs_from_prgb32 = convert_argb32_bs_from_prgb32;

  // [Convert - Prgb32 Dest]

  m->convert.prgb32_from_argb32 = convert_prgb32_from_argb32;
  m->convert.prgb32_from_argb32_bs = convert_prgb32_from_argb32_bs;
  m->convert.prgb32_from_i8 = convert_prgb32_from_i8;

  m->convert.prgb32_bs_from_argb32 = convert_prgb32_bs_from_argb32;
  m->convert.prgb32_bs_from_i8 = convert_prgb32_bs_from_i8;

  // [Convert - Rgb32 Dest]

  m->convert.rgb32_from_argb32 = convert_rgb32_from_argb32;
  m->convert.rgb32_from_argb32_bs = convert_rgb32_from_argb32_bs;
  m->convert.rgb32_from_rgb24 = convert_rgb32_from_rgb24;
  m->convert.rgb32_from_bgr24 = convert_rgb32_from_bgr24;
  m->convert.rgb32_from_rgb16_5550 = convert_rgb32_from_rgb16_5550;
  m->convert.rgb32_from_rgb16_5550_bs = convert_rgb32_from_rgb16_5550_bs;
  m->convert.rgb32_from_rgb16_5650 = convert_rgb32_from_rgb16_5650;
  m->convert.rgb32_from_rgb16_5650_bs = convert_rgb32_from_rgb16_5650_bs;
  m->convert.rgb32_from_i8 = convert_rgb32_from_i8;

  m->convert.rgb32_bs_from_rgb24 = convert_rgb32_bs_from_rgb24;

  // [Convert - Rgb24/Bgr24 Dest]

  m->convert.rgb24_from_rgb32 = convert_rgb24_from_rgb32;
  m->convert.rgb24_from_rgb32_bs = convert_rgb24_from_rgb32_bs;
  m->convert.rgb24_from_rgb16_5550 = convert_rgb24_from_rgb16_5550;
  m->convert.rgb24_from_rgb16_5550_bs = convert_rgb24_from_rgb16_5550_bs;
  m->convert.rgb24_from_rgb16_5650 = convert_rgb24_from_rgb16_5650;
  m->convert.rgb24_from_rgb16_5650_bs = convert_rgb24_from_rgb16_5650_bs;
  m->convert.rgb24_from_i8 = convert_rgb24_from_i8;

  m->convert.bgr24_from_rgb32 = convert_bgr24_from_rgb32;
  m->convert.bgr24_from_i8 = convert_bgr24_from_i8;

  // [Convert - Rgb16 Dest]

  m->convert.rgb16_5550_from_rgb32 = convert_rgb16_5550_from_rgb32;
  m->convert.rgb16_5550_from_rgb24 = convert_rgb16_5550_from_rgb24;
  m->convert.rgb16_5550_from_i8 = convert_rgb16_5550_from_i8;

  m->convert.rgb16_5650_from_rgb32 = convert_rgb16_5650_from_rgb32;
  m->convert.rgb16_5650_from_rgb24 = convert_rgb16_5650_from_rgb24;
  m->convert.rgb16_5650_from_i8 = convert_rgb16_5650_from_i8;

  m->convert.rgb16_5550_bs_from_rgb32 = convert_rgb16_5550_bs_from_rgb32;
  m->convert.rgb16_5550_bs_from_rgb24 = convert_rgb16_5550_bs_from_rgb24;
  m->convert.rgb16_5550_bs_from_i8 = convert_rgb16_5550_bs_from_i8;

  m->convert.rgb16_5650_bs_from_rgb32 = convert_rgb16_5650_bs_from_rgb32;
  m->convert.rgb16_5650_bs_from_rgb24 = convert_rgb16_5650_bs_from_rgb24;
  m->convert.rgb16_5650_bs_from_i8 = convert_rgb16_5650_bs_from_i8;

  // [Convert - Greyscale]

  m->convert.greyscale8_from_rgb32 = convert_greyscale8_from_rgb32;
  m->convert.greyscale8_from_rgb24 = convert_greyscale8_from_rgb24;
  m->convert.greyscale8_from_i8 = convert_greyscale8_from_i8;

  m->convert.rgb32_from_greyscale8 = convert_rgb32_from_greyscale8;
  m->convert.rgb24_from_greyscale8 = convert_rgb24_from_greyscale8;

  // [Convert - Dithering]

  m->convert.i8rgb232_from_rgb32_dither = convert_i8rgb232_from_rgb32_dither;
  m->convert.i8rgb222_from_rgb32_dither = convert_i8rgb222_from_rgb32_dither;
  m->convert.i8rgb111_from_rgb32_dither = convert_i8rgb111_from_rgb32_dither;

  m->convert.i8rgb232_from_rgb24_dither = convert_i8rgb232_from_rgb24_dither;
  m->convert.i8rgb222_from_rgb24_dither = convert_i8rgb222_from_rgb24_dither;
  m->convert.i8rgb111_from_rgb24_dither = convert_i8rgb111_from_rgb24_dither;

  m->convert.rgb16_5550_from_rgb32_dither = convert_rgb16_5550_from_rgb32_dither;
  m->convert.rgb16_5550_from_rgb24_dither = convert_rgb16_5550_from_rgb24_dither;

  m->convert.rgb16_5650_from_rgb32_dither = convert_rgb16_5650_from_rgb32_dither;
  m->convert.rgb16_5650_from_rgb24_dither = convert_rgb16_5650_from_rgb24_dither;

  m->convert.rgb16_5550_bs_from_rgb32_dither = convert_rgb16_5550_bs_from_rgb32_dither;
  m->convert.rgb16_5550_bs_from_rgb24_dither = convert_rgb16_5550_bs_from_rgb24_dither;

  m->convert.rgb16_5650_bs_from_rgb32_dither = convert_rgb16_5650_bs_from_rgb32_dither;
  m->convert.rgb16_5650_bs_from_rgb24_dither = convert_rgb16_5650_bs_from_rgb24_dither;

  // [Gradient - Gradient]

  m->gradient.gradient_argb32 = gradient_gradient_argb32;
  m->gradient.gradient_prgb32 = gradient_gradient_prgb32;
  m->gradient.gradient_rgb32 = gradient_gradient_rgb32;
  m->gradient.gradient_rgb24 = gradient_gradient_rgb24;
  m->gradient.gradient_a8 = gradient_gradient_a8;

  // [Pattern - Texture]

  m->pattern.texture_init = pattern_texture_init;
  m->pattern.texture_fetch_repeat = pattern_texture_fetch_repeat;
  m->pattern.texture_fetch_reflect = pattern_texture_fetch_reflect;

  // [Pattern - Linear Gradient]

  m->pattern.linear_gradient_init = pattern_linear_gradient_init;
  m->pattern.linear_gradient_fetch_pad = pattern_linear_gradient_fetch_pad;
  m->pattern.linear_gradient_fetch_repeat = pattern_linear_gradient_fetch_repeat;

  // [Pattern - Radial Gradient]

  m->pattern.radial_gradient_init = pattern_radial_gradient_init;
  m->pattern.radial_gradient_fetch_pad = pattern_radial_gradient_fetch_pad;
  m->pattern.radial_gradient_fetch_repeat = pattern_radial_gradient_fetch_repeat;

  // [Pattern - Conical Gradient]

  m->pattern.conical_gradient_init = pattern_conical_gradient_init;
  m->pattern.conical_gradient_fetch = pattern_conical_gradient_fetch;

  // [Raster]

  // TODO: Write correct versions and remove

#define SET_RASTER_ARGB32(DST_ID, DST_CLASS, SRC_SOLID_CLASS, OP_ID, OP_CLASS) \
  m->raster_argb32[DST_ID][OP_ID].pixel = raster_pixel< OP_CLASS<DST_CLASS, SRC_SOLID_CLASS> >; \
  m->raster_argb32[DST_ID][OP_ID].pixel_a8 = raster_pixel_a8< OP_CLASS<DST_CLASS, SRC_SOLID_CLASS> >; \
  \
  m->raster_argb32[DST_ID][OP_ID].span_solid = raster_span_solid< OP_CLASS<DST_CLASS, SRC_SOLID_CLASS> >; \
  m->raster_argb32[DST_ID][OP_ID].span_solid_a8 = raster_span_solid_a8< OP_CLASS<DST_CLASS, SRC_SOLID_CLASS> >; \
  \
  m->raster_argb32[DST_ID][OP_ID].span_composite   [Image::FormatARGB32] = raster_span_composite   < OP_CLASS<DST_CLASS, PixFmt_ARGB32> >; \
  m->raster_argb32[DST_ID][OP_ID].span_composite_a8[Image::FormatARGB32] = raster_span_composite_a8< OP_CLASS<DST_CLASS, PixFmt_ARGB32> >; \
  \
  m->raster_argb32[DST_ID][OP_ID].span_composite   [Image::FormatPRGB32] = raster_span_composite   < OP_CLASS<DST_CLASS, PixFmt_PRGB32> >; \
  m->raster_argb32[DST_ID][OP_ID].span_composite_a8[Image::FormatPRGB32] = raster_span_composite_a8< OP_CLASS<DST_CLASS, PixFmt_PRGB32> >; \
  \
  m->raster_argb32[DST_ID][OP_ID].span_composite   [Image::FormatRGB32] = raster_span_composite   < OP_CLASS<DST_CLASS, PixFmt_RGB32> >; \
  m->raster_argb32[DST_ID][OP_ID].span_composite_a8[Image::FormatRGB32] = raster_span_composite_a8< OP_CLASS<DST_CLASS, PixFmt_RGB32> >; \
  \
  m->raster_argb32[DST_ID][OP_ID].span_composite   [Image::FormatRGB24] = raster_span_composite   < OP_CLASS<DST_CLASS, PixFmt_RGB24> >; \
  m->raster_argb32[DST_ID][OP_ID].span_composite_a8[Image::FormatRGB24] = raster_span_composite_a8< OP_CLASS<DST_CLASS, PixFmt_RGB24> >

  // [Raster - Argb32 / Prgb32]
#if 1
  SET_RASTER_ARGB32(0, PixFmt_ARGB32, PixFmt_ARGB32, CompositeSrc, Operator_Src);
  SET_RASTER_ARGB32(0, PixFmt_ARGB32, PixFmt_ARGB32, CompositeSrcOver, Operator_Over);
  SET_RASTER_ARGB32(0, PixFmt_ARGB32, PixFmt_ARGB32, CompositeDestOver, Operator_OverReverse);
  SET_RASTER_ARGB32(0, PixFmt_ARGB32, PixFmt_ARGB32, CompositeSrcIn, Operator_In);
  SET_RASTER_ARGB32(0, PixFmt_ARGB32, PixFmt_ARGB32, CompositeDestIn, Operator_InReverse);
  SET_RASTER_ARGB32(0, PixFmt_ARGB32, PixFmt_ARGB32, CompositeSrcOut, Operator_Out);
  SET_RASTER_ARGB32(0, PixFmt_ARGB32, PixFmt_ARGB32, CompositeDestOut, Operator_OutReverse);
  SET_RASTER_ARGB32(0, PixFmt_ARGB32, PixFmt_ARGB32, CompositeSrcAtop, Operator_Atop);
  SET_RASTER_ARGB32(0, PixFmt_ARGB32, PixFmt_ARGB32, CompositeDestAtop, Operator_AtopReverse);
  SET_RASTER_ARGB32(0, PixFmt_ARGB32, PixFmt_ARGB32, CompositeXor, Operator_Xor);
  SET_RASTER_ARGB32(0, PixFmt_ARGB32, PixFmt_ARGB32, CompositeClear, Operator_Clear);

  SET_RASTER_ARGB32(1, PixFmt_PRGB32, PixFmt_PRGB32, CompositeSrc, Operator_Src);
  SET_RASTER_ARGB32(1, PixFmt_PRGB32, PixFmt_PRGB32, CompositeSrcOver, Operator_Over);
  SET_RASTER_ARGB32(1, PixFmt_PRGB32, PixFmt_PRGB32, CompositeDestOver, Operator_OverReverse);
  SET_RASTER_ARGB32(1, PixFmt_PRGB32, PixFmt_PRGB32, CompositeSrcIn, Operator_In);
  SET_RASTER_ARGB32(1, PixFmt_PRGB32, PixFmt_PRGB32, CompositeDestIn, Operator_InReverse);
  SET_RASTER_ARGB32(1, PixFmt_PRGB32, PixFmt_PRGB32, CompositeSrcOut, Operator_Out);
  SET_RASTER_ARGB32(1, PixFmt_PRGB32, PixFmt_PRGB32, CompositeDestOut, Operator_OutReverse);
  SET_RASTER_ARGB32(1, PixFmt_PRGB32, PixFmt_PRGB32, CompositeSrcAtop, Operator_Atop);
  SET_RASTER_ARGB32(1, PixFmt_PRGB32, PixFmt_PRGB32, CompositeDestAtop, Operator_AtopReverse);
  SET_RASTER_ARGB32(1, PixFmt_PRGB32, PixFmt_PRGB32, CompositeXor, Operator_Xor);
  SET_RASTER_ARGB32(1, PixFmt_PRGB32, PixFmt_PRGB32, CompositeClear, Operator_Clear);

  SET_RASTER_ARGB32(0, PixFmt_ARGB32, PixFmt_ARGB32, CompositeAdd, Operator_Add);
  SET_RASTER_ARGB32(0, PixFmt_ARGB32, PixFmt_ARGB32, CompositeSubtract, Operator_Subtract);
  SET_RASTER_ARGB32(0, PixFmt_ARGB32, PixFmt_ARGB32, CompositeMultiply, Operator_Multiply);
  SET_RASTER_ARGB32(0, PixFmt_ARGB32, PixFmt_ARGB32, CompositeScreen, Operator_Screen);
  SET_RASTER_ARGB32(0, PixFmt_ARGB32, PixFmt_ARGB32, CompositeDarken, Operator_Darken);
  SET_RASTER_ARGB32(0, PixFmt_ARGB32, PixFmt_ARGB32, CompositeLighten, Operator_Lighten);
  SET_RASTER_ARGB32(0, PixFmt_ARGB32, PixFmt_ARGB32, CompositeDifference, Operator_Difference);
  SET_RASTER_ARGB32(0, PixFmt_ARGB32, PixFmt_ARGB32, CompositeExclusion, Operator_Exclusion);
  SET_RASTER_ARGB32(0, PixFmt_ARGB32, PixFmt_ARGB32, CompositeInvert, Operator_Invert);
  SET_RASTER_ARGB32(0, PixFmt_ARGB32, PixFmt_ARGB32, CompositeInvertRgb, Operator_InvertRgb);

  SET_RASTER_ARGB32(1, PixFmt_PRGB32, PixFmt_PRGB32, CompositeAdd, Operator_Add);
  SET_RASTER_ARGB32(1, PixFmt_PRGB32, PixFmt_PRGB32, CompositeSubtract, Operator_Subtract);
  SET_RASTER_ARGB32(1, PixFmt_PRGB32, PixFmt_PRGB32, CompositeMultiply, Operator_Multiply);
  SET_RASTER_ARGB32(1, PixFmt_PRGB32, PixFmt_PRGB32, CompositeScreen, Operator_Screen);
  SET_RASTER_ARGB32(1, PixFmt_PRGB32, PixFmt_PRGB32, CompositeDarken, Operator_Darken);
  SET_RASTER_ARGB32(1, PixFmt_PRGB32, PixFmt_PRGB32, CompositeLighten, Operator_Lighten);
  SET_RASTER_ARGB32(1, PixFmt_PRGB32, PixFmt_PRGB32, CompositeDifference, Operator_Difference);
  SET_RASTER_ARGB32(1, PixFmt_PRGB32, PixFmt_PRGB32, CompositeExclusion, Operator_Exclusion);
  SET_RASTER_ARGB32(1, PixFmt_PRGB32, PixFmt_PRGB32, CompositeInvert, Operator_Invert);
  SET_RASTER_ARGB32(1, PixFmt_PRGB32, PixFmt_PRGB32, CompositeInvertRgb, Operator_InvertRgb);
#endif
#undef SET_RASTER_ARGB32

  // [Raster - Argb32 / Prgb32 - SrcOver]

  // [Raster - Argb32 / Prgb32 - NOP]

  for (i = 0; i < 2; i++)
  {
    m->raster_argb32[i][CompositeDest].pixel = raster_pixel_nop;
    m->raster_argb32[i][CompositeDest].pixel_a8 = raster_pixel_a8_nop;
    m->raster_argb32[i][CompositeDest].span_solid = raster_span_solid_nop;
    m->raster_argb32[i][CompositeDest].span_solid_a8 = raster_span_solid_a8_nop;

    m->raster_argb32[i][CompositeDest].span_composite[Image::FormatARGB32] = raster_span_composite_nop;
    m->raster_argb32[i][CompositeDest].span_composite[Image::FormatPRGB32] = raster_span_composite_nop;
    m->raster_argb32[i][CompositeDest].span_composite[Image::FormatRGB32] = raster_span_composite_nop;
    m->raster_argb32[i][CompositeDest].span_composite[Image::FormatRGB24] = raster_span_composite_nop;
    m->raster_argb32[i][CompositeDest].span_composite[Image::FormatA8] = raster_span_composite_nop;
    m->raster_argb32[i][CompositeDest].span_composite_indexed[Image::FormatI8] = raster_span_composite_indexed_nop;

    m->raster_argb32[i][CompositeDest].span_composite_a8[Image::FormatARGB32] = raster_span_composite_a8_nop;
    m->raster_argb32[i][CompositeDest].span_composite_a8[Image::FormatPRGB32] = raster_span_composite_a8_nop;
    m->raster_argb32[i][CompositeDest].span_composite_a8[Image::FormatRGB32] = raster_span_composite_a8_nop;
    m->raster_argb32[i][CompositeDest].span_composite_a8[Image::FormatRGB24] = raster_span_composite_a8_nop;
    m->raster_argb32[i][CompositeDest].span_composite_a8[Image::FormatA8] = raster_span_composite_a8_nop;
    m->raster_argb32[i][CompositeDest].span_composite_indexed_a8[Image::FormatI8] = raster_span_composite_indexed_a8_nop;
  }

  // [Raster - Rgb32]

  m->raster_rgb32.pixel = raster_rgb32_pixel;
  m->raster_rgb32.pixel_a8 = raster_rgb32_pixel_a8;
  m->raster_rgb32.span_solid = raster_rgb32_span_solid;
  m->raster_rgb32.span_solid_a8 = raster_rgb32_span_solid_a8;

  m->raster_rgb32.span_composite[Image::FormatARGB32] = raster_rgb32_span_composite_argb32;
  m->raster_rgb32.span_composite[Image::FormatPRGB32] = raster_rgb32_span_composite_prgb32;
  m->raster_rgb32.span_composite[Image::FormatRGB32] = convert_memcpy32;
  m->raster_rgb32.span_composite[Image::FormatRGB24] = convert_rgb32_from_rgb24;
  m->raster_rgb32.span_composite[Image::FormatA8] = raster_span_composite_nop;
  m->raster_rgb32.span_composite_indexed[Image::FormatI8] = raster_rgb32_span_composite_indexed;

  m->raster_rgb32.span_composite_a8[Image::FormatARGB32] = raster_rgb32_span_composite_argb32_a8;
  m->raster_rgb32.span_composite_a8[Image::FormatPRGB32] = raster_rgb32_span_composite_prgb32_a8;
  m->raster_rgb32.span_composite_a8[Image::FormatRGB32] = raster_rgb32_span_composite_rgb32_a8;
  m->raster_rgb32.span_composite_a8[Image::FormatRGB24] = raster_rgb32_span_composite_rgb24_a8;
  m->raster_rgb32.span_composite_a8[Image::FormatA8] = raster_span_composite_a8_nop;
  m->raster_rgb32.span_composite_indexed_a8[Image::FormatI8] = raster_rgb32_span_composite_indexed_a8;

  // [Raster - Rgb24]

  m->raster_rgb24.pixel = raster_rgb24_pixel;
  m->raster_rgb24.pixel_a8 = raster_rgb24_pixel_a8;
  m->raster_rgb24.span_solid = raster_rgb24_span_solid;
  m->raster_rgb24.span_solid_a8 = raster_rgb24_span_solid_a8;

  m->raster_rgb24.span_composite[Image::FormatARGB32] = raster_rgb24_span_composite_argb32;
  m->raster_rgb24.span_composite[Image::FormatPRGB32] = raster_rgb24_span_composite_prgb32;
  m->raster_rgb24.span_composite[Image::FormatRGB32] = convert_rgb24_from_rgb32;
  m->raster_rgb24.span_composite[Image::FormatRGB24] = convert_memcpy24;
  m->raster_rgb24.span_composite[Image::FormatA8] = raster_span_composite_nop;
  m->raster_rgb24.span_composite_indexed[Image::FormatI8] = TODO_NOT_IMPLEMENTED;

  m->raster_rgb24.span_composite_a8[Image::FormatARGB32] = raster_rgb24_span_composite_argb32_a8;
  m->raster_rgb24.span_composite_a8[Image::FormatPRGB32] = raster_rgb24_span_composite_prgb32_a8;
  m->raster_rgb24.span_composite_a8[Image::FormatRGB32] = raster_rgb24_span_composite_rgb32_a8;
  m->raster_rgb24.span_composite_a8[Image::FormatRGB24] = raster_rgb24_span_composite_rgb24_a8;
  m->raster_rgb24.span_composite_a8[Image::FormatA8] = raster_span_composite_a8_nop;
  m->raster_rgb24.span_composite_indexed_a8[Image::FormatI8] = TODO_NOT_IMPLEMENTED;
}
