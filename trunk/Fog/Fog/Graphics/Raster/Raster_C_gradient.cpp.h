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

} // Raster namespace
} // Fog namespace
