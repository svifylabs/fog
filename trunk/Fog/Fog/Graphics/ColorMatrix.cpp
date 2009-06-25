// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// AggOO - Version 0.1
// Copyright (c) 2006 Dratek Software
//
// Author of AggOO (Chad M. Draper) agreed relicencing code to the MIT licence
// that is used for Fog (original code was 3 clause BSD licence). Thanks.

// [Precompiled headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/Error.h>
#include <Fog/Core/Math.h>
#include <Fog/Graphics/ColorMatrix.h>
#include <Fog/Graphics/Rgba.h>

namespace Fog {

// The luminance color weights are close to the values of the NTSC color
// weights, but these values are preferable.
static const double lumR = 0.3086;
static const double lumG = 0.6094;
static const double lumB = 0.0820;

// ============================================================================
// [Fog::ColorMatrix]
// ============================================================================

int ColorMatrix::type() const
{
  int parts = 0;

  // RGB shear part is illustrated here:
  //   [n X X n n]
  //   [X n X n n]
  //   [X X n n n]
  //   [n n n n n]
  //   [n n n n n]
  if (m[0][1] != 0.0 || m[0][2] != 0.0 ||
      m[1][0] != 0.0 || m[1][2] != 0.0 ||
      m[2][0] != 0.0 || m[2][1] != 0.0) parts |= PartShearRGB;

  // Alpha shear part is illustrated here:
  //   [n n n X n]
  //   [n n n X n]
  //   [n n n X n]
  //   [X X X n n]
  //   [n n n n n]
  if (m[0][3] != 0.0 ||
      m[1][3] != 0.0 ||
      m[2][3] != 0.0 ||
      m[2][0] != 0.0 || m[2][1] != 0.0 || m[2][2] != 0.0) parts |= PartShearAlpha;

  // RGB lut part is illustrated here:
  //   [X n n n n]
  //   [n X n n n]
  //   [n n X n n]
  //   [n n n n n]
  //   [n n n n n]
  if (m[0][0] != 0.0 ||
      m[1][1] != 0.0 ||
      m[2][2] != 0.0) parts |= PartLutRGB;

  // Alpha lut part is illustrated here:
  //   [n n n n n]
  //   [n n n n n]
  //   [n n n n n]
  //   [n n n X n]
  //   [n n n n n]
  if (m[3][3] != 0.0) parts |= PartLutAlpha;

  //! @brief Matrix contains const RGB lut part (all cells are set to 1.0).
  //!
  //! Const RGB lut part is illustrated here:
  //!   [1 n n n n]
  //!   [n 1 n n n]
  //!   [n n 1 n n]
  //!   [n n n n n]
  //!   [n n n n n]
  if (m[0][0] == 1.0 &&
      m[1][1] == 1.0 &&
      m[2][2] == 1.0) parts |= PartConstRGB;

  //! @brief Matrix contains const alpha lut part (cell set to 1.0).
  //!
  //! Const alpha lut part is illustrated here:
  //!   [n n n n n]
  //!   [n n n n n]
  //!   [n n n n n]
  //!   [n n n 1 n]
  //!   [n n n n n]
  if (m[3][3] == 1.0) parts |= PartConstAlpha;

  // RGB translation part is illustrated here:
  //   [n n n n n]
  //   [n n n n n]
  //   [n n n n n]
  //   [n n n n n]
  //   [X X X n n]
  if (m[4][0] != 0.0 || m[4][1] != 0.0 || m[4][2] != 0.0) parts |= PartTranslateRGB;

  // Alpha translation part is illustrated here:
  //   [n n n n n]
  //   [n n n n n]
  //   [n n n n n]
  //   [n n n n n]
  //   [n n n X n]
  if (m[4][3] != 0.0) parts |= PartTranslateAlpha;

  return parts;
}

ColorMatrix& ColorMatrix::add(const ColorMatrix& other)
{
  for (sysuint_t i = 0; i < 25; i++)
    arr[i] += other.arr[i];
  return *this;
}

ColorMatrix& ColorMatrix::subtract(const ColorMatrix& other)
{
  for (sysuint_t i = 0; i < 25; i++)
    arr[i] -= other.arr[i];
  return *this;
}

ColorMatrix& ColorMatrix::multiply(const ColorMatrix& other, int order)
{
  ColorMatrix save(*this);

  const ColorMatrix* mat1 = &save;
  const ColorMatrix* mat2 = &other;

  if (this == &other)
  {
    mat2 = &other;
  }
  else if (order != MatrixOrderPrepend)
  {
    mat1 = &other;
    mat2 = &save;
  }

  m[0][0] = mat1->m[0][0] * mat2->m[0][0] + mat1->m[0][1] * mat2->m[1][0] + mat1->m[0][2] * mat2->m[2][0] + mat1->m[0][3] * mat2->m[3][0] + mat1->m[0][4] * mat2->m[4][0];
  m[0][1] = mat1->m[0][0] * mat2->m[0][1] + mat1->m[0][1] * mat2->m[1][1] + mat1->m[0][2] * mat2->m[2][1] + mat1->m[0][3] * mat2->m[3][1] + mat1->m[0][4] * mat2->m[4][1];
  m[0][2] = mat1->m[0][0] * mat2->m[0][2] + mat1->m[0][1] * mat2->m[1][2] + mat1->m[0][2] * mat2->m[2][2] + mat1->m[0][3] * mat2->m[3][2] + mat1->m[0][4] * mat2->m[4][2];
  m[0][3] = mat1->m[0][0] * mat2->m[0][3] + mat1->m[0][1] * mat2->m[1][3] + mat1->m[0][2] * mat2->m[2][3] + mat1->m[0][3] * mat2->m[3][3] + mat1->m[0][4] * mat2->m[4][3];
  m[0][4] = mat1->m[0][0] * mat2->m[0][4] + mat1->m[0][1] * mat2->m[1][4] + mat1->m[0][2] * mat2->m[2][4] + mat1->m[0][3] * mat2->m[3][4] + mat1->m[0][4] * mat2->m[4][4];

  m[1][0] = mat1->m[1][0] * mat2->m[0][0] + mat1->m[1][1] * mat2->m[1][0] + mat1->m[1][2] * mat2->m[2][0] + mat1->m[1][3] * mat2->m[3][0] + mat1->m[1][4] * mat2->m[4][0];
  m[1][1] = mat1->m[1][0] * mat2->m[0][1] + mat1->m[1][1] * mat2->m[1][1] + mat1->m[1][2] * mat2->m[2][1] + mat1->m[1][3] * mat2->m[3][1] + mat1->m[1][4] * mat2->m[4][1];
  m[1][2] = mat1->m[1][0] * mat2->m[0][2] + mat1->m[1][1] * mat2->m[1][2] + mat1->m[1][2] * mat2->m[2][2] + mat1->m[1][3] * mat2->m[3][2] + mat1->m[1][4] * mat2->m[4][2];
  m[1][3] = mat1->m[1][0] * mat2->m[0][3] + mat1->m[1][1] * mat2->m[1][3] + mat1->m[1][2] * mat2->m[2][3] + mat1->m[1][3] * mat2->m[3][3] + mat1->m[1][4] * mat2->m[4][3];
  m[1][4] = mat1->m[1][0] * mat2->m[0][4] + mat1->m[1][1] * mat2->m[1][4] + mat1->m[1][2] * mat2->m[2][4] + mat1->m[1][3] * mat2->m[3][4] + mat1->m[1][4] * mat2->m[4][4];

  m[2][0] = mat1->m[2][0] * mat2->m[0][0] + mat1->m[2][1] * mat2->m[1][0] + mat1->m[2][2] * mat2->m[2][0] + mat1->m[2][3] * mat2->m[3][0] + mat1->m[2][4] * mat2->m[4][0];
  m[2][1] = mat1->m[2][0] * mat2->m[0][1] + mat1->m[2][1] * mat2->m[1][1] + mat1->m[2][2] * mat2->m[2][1] + mat1->m[2][3] * mat2->m[3][1] + mat1->m[2][4] * mat2->m[4][1];
  m[2][2] = mat1->m[2][0] * mat2->m[0][2] + mat1->m[2][1] * mat2->m[1][2] + mat1->m[2][2] * mat2->m[2][2] + mat1->m[2][3] * mat2->m[3][2] + mat1->m[2][4] * mat2->m[4][2];
  m[2][3] = mat1->m[2][0] * mat2->m[0][3] + mat1->m[2][1] * mat2->m[1][3] + mat1->m[2][2] * mat2->m[2][3] + mat1->m[2][3] * mat2->m[3][3] + mat1->m[2][4] * mat2->m[4][3];
  m[2][4] = mat1->m[2][0] * mat2->m[0][4] + mat1->m[2][1] * mat2->m[1][4] + mat1->m[2][2] * mat2->m[2][4] + mat1->m[2][3] * mat2->m[3][4] + mat1->m[2][4] * mat2->m[4][4];

  m[3][0] = mat1->m[3][0] * mat2->m[0][0] + mat1->m[3][1] * mat2->m[1][0] + mat1->m[3][2] * mat2->m[2][0] + mat1->m[3][3] * mat2->m[3][0] + mat1->m[3][4] * mat2->m[4][0];
  m[3][1] = mat1->m[3][0] * mat2->m[0][1] + mat1->m[3][1] * mat2->m[1][1] + mat1->m[3][2] * mat2->m[2][1] + mat1->m[3][3] * mat2->m[3][1] + mat1->m[3][4] * mat2->m[4][1];
  m[3][2] = mat1->m[3][0] * mat2->m[0][2] + mat1->m[3][1] * mat2->m[1][2] + mat1->m[3][2] * mat2->m[2][2] + mat1->m[3][3] * mat2->m[3][2] + mat1->m[3][4] * mat2->m[4][2];
  m[3][3] = mat1->m[3][0] * mat2->m[0][3] + mat1->m[3][1] * mat2->m[1][3] + mat1->m[3][2] * mat2->m[2][3] + mat1->m[3][3] * mat2->m[3][3] + mat1->m[3][4] * mat2->m[4][3];
  m[3][4] = mat1->m[3][0] * mat2->m[0][4] + mat1->m[3][1] * mat2->m[1][4] + mat1->m[3][2] * mat2->m[2][4] + mat1->m[3][3] * mat2->m[3][4] + mat1->m[3][4] * mat2->m[4][4];

  m[4][0] = mat1->m[4][0] * mat2->m[0][0] + mat1->m[4][1] * mat2->m[1][0] + mat1->m[4][2] * mat2->m[2][0] + mat1->m[4][3] * mat2->m[3][0] + mat1->m[4][4] * mat2->m[4][0];
  m[4][1] = mat1->m[4][0] * mat2->m[0][1] + mat1->m[4][1] * mat2->m[1][1] + mat1->m[4][2] * mat2->m[2][1] + mat1->m[4][3] * mat2->m[3][1] + mat1->m[4][4] * mat2->m[4][1];
  m[4][2] = mat1->m[4][0] * mat2->m[0][2] + mat1->m[4][1] * mat2->m[1][2] + mat1->m[4][2] * mat2->m[2][2] + mat1->m[4][3] * mat2->m[3][2] + mat1->m[4][4] * mat2->m[4][2];
  m[4][3] = mat1->m[4][0] * mat2->m[0][3] + mat1->m[4][1] * mat2->m[1][3] + mat1->m[4][2] * mat2->m[2][3] + mat1->m[4][3] * mat2->m[3][3] + mat1->m[4][4] * mat2->m[4][3];
  m[4][4] = mat1->m[4][0] * mat2->m[0][4] + mat1->m[4][1] * mat2->m[1][4] + mat1->m[4][2] * mat2->m[2][4] + mat1->m[4][3] * mat2->m[3][4] + mat1->m[4][4] * mat2->m[4][4];

  return *this;
}

ColorMatrix& ColorMatrix::multiply(double scalar)
{
  for (sysuint_t i = 0; i < 25; i++)
    arr[i] *= scalar;
  return *this;
}

void ColorMatrix::transformVector(double* v) const
{
  double temp[4];

  temp[0] = v[0] * m[0][0] + v[1] * m[1][0] + v[2] * m[2][0] + v[3] * m[3][0] + m[4][0];
  temp[1] = v[0] * m[0][1] + v[1] * m[1][1] + v[2] * m[2][1] + v[3] * m[3][1] + m[4][1];
  temp[2] = v[0] * m[0][2] + v[1] * m[1][2] + v[2] * m[2][2] + v[3] * m[3][2] + m[4][2];
  temp[3] = v[0] * m[0][3] + v[1] * m[1][3] + v[2] * m[2][3] + v[3] * m[3][3] + m[4][3];

  v[0] = temp[0];
  v[1] = temp[1];
  v[2] = temp[2];
  v[3] = temp[3];
}

void ColorMatrix::transformRgb(Rgba* clr) const
{
  double r = (double)clr->r;
  double g = (double)clr->g;
  double b = (double)clr->b;

  int tr = (int)(r * m[0][0] + g * m[1][0] + b * m[2][0] + 255.0 * m[3][0] + m[4][0] * 255.0);
  int tg = (int)(r * m[0][1] + g * m[1][1] + b * m[2][1] + 255.0 * m[3][1] + m[4][1] * 255.0);
  int tb = (int)(r * m[0][2] + g * m[1][2] + b * m[2][2] + 255.0 * m[3][2] + m[4][2] * 255.0);

  if (tr < 0) tr = 0;
  if (tg < 0) tg = 0;
  if (tb < 0) tb = 0;

  if (tr > 255) tr = 255;
  if (tg > 255) tg = 255;
  if (tb > 255) tb = 255;

  clr->r = (uint8_t)tr;
  clr->g = (uint8_t)tg;
  clr->b = (uint8_t)tb;
}

void ColorMatrix::transformRgb64(Rgba64* clr) const
{
  double r = (double)clr->r;
  double g = (double)clr->g;
  double b = (double)clr->b;

  int tr = (int)(r * m[0][0] + g * m[1][0] + b * m[2][0] + 65535.0 * m[3][0] + m[4][0] * 65535.0);
  int tg = (int)(r * m[0][1] + g * m[1][1] + b * m[2][1] + 65535.0 * m[3][1] + m[4][1] * 65535.0);
  int tb = (int)(r * m[0][2] + g * m[1][2] + b * m[2][2] + 65535.0 * m[3][2] + m[4][2] * 65535.0);

  if (tr < 0) tr = 0;
  if (tg < 0) tg = 0;
  if (tb < 0) tb = 0;

  if (tr > 65535) tr = 65535;
  if (tg > 65535) tg = 65535;
  if (tb > 65535) tb = 65535;

  clr->r = (uint16_t)tr;
  clr->g = (uint16_t)tg;
  clr->b = (uint16_t)tb;
}

void ColorMatrix::transformRgba(Rgba* clr) const
{
  double r = (double)clr->r;
  double g = (double)clr->g;
  double b = (double)clr->b;
  double a = (double)clr->a;

  int tr = (int)(r * m[0][0] + g * m[1][0] + b * m[2][0] + a * m[3][0] + m[4][0] * 255.0);
  int tg = (int)(r * m[0][1] + g * m[1][1] + b * m[2][1] + a * m[3][1] + m[4][1] * 255.0);
  int tb = (int)(r * m[0][2] + g * m[1][2] + b * m[2][2] + a * m[3][2] + m[4][2] * 255.0);
  int ta = (int)(r * m[0][3] + g * m[1][3] + b * m[2][3] + a * m[3][3] + m[4][3] * 255.0);

  if (tr < 0) tr = 0;
  if (tg < 0) tg = 0;
  if (tb < 0) tb = 0;
  if (ta < 0) ta = 0;

  if (tr > 255) tr = 255;
  if (tg > 255) tg = 255;
  if (tb > 255) tb = 255;
  if (ta > 255) ta = 255;

  clr->set(tr, tg, tb, ta);
}

void ColorMatrix::transformRgba64(Rgba64* clr) const
{
  double r = (double)clr->r;
  double g = (double)clr->g;
  double b = (double)clr->b;
  double a = (double)clr->a;

  int tr = (int)(r * m[0][0] + g * m[1][0] + b * m[2][0] + a * m[3][0] + m[4][0] * 65535.0);
  int tg = (int)(r * m[0][1] + g * m[1][1] + b * m[2][1] + a * m[3][1] + m[4][1] * 65535.0);
  int tb = (int)(r * m[0][2] + g * m[1][2] + b * m[2][2] + a * m[3][2] + m[4][2] * 65535.0);
  int ta = (int)(r * m[0][3] + g * m[1][3] + b * m[2][3] + a * m[3][3] + m[4][3] * 65535.0);

  if (tr < 0) tr = 0;
  if (tg < 0) tg = 0;
  if (tb < 0) tb = 0;
  if (ta < 0) ta = 0;

  if (tr > 65535) tr = 65535;
  if (tg > 65535) tg = 65535;
  if (tb > 65535) tb = 65535;
  if (ta > 65535) ta = 65535;

  clr->set(tr, tg, tb, ta);
}

void ColorMatrix::transformAlpha(uint8_t* a) const
{
  int ta = (int)((double)*a * m[3][3] + m[4][3] * 255.0);

  if (ta < 0) ta = 0;
  if (ta > 255) ta = 255;

  *a = (uint8_t)ta;
}

ColorMatrix& ColorMatrix::scale(double sr, double sg, double sb, double sa, int order)
{
  ColorMatrix mod;
  mod[0][0] = sr;
  mod[1][1] = sg;
  mod[2][2] = sb;
  mod[3][3] = sa;
  return multiply(mod, order);
}

ColorMatrix& ColorMatrix::translate(double tr, double tg, double tb, double ta, int order)
{
  ColorMatrix mod;
  mod[4][0] = tr;
  mod[4][1] = tg;
  mod[4][2] = tb;
  mod[4][3] = ta;
  return multiply(mod, order);
}

ColorMatrix& ColorMatrix::setSaturation(double sat, int order)
{
  // If the saturation is 1.0, then this matrix remains unchanged.
  // If the saturation is 0.0, each color is scaled by its luminance
  double satComplement  = 1.0 - sat;
  double satComplementR = lumR * satComplement;
  double satComplementG = lumG * satComplement;
  double satComplementB = lumB * satComplement;

  // Create the matrix:
  ColorMatrix mod(
      satComplementR + sat, satComplementR, satComplementR, 0.0, 0.0,
      satComplementG, satComplementG + sat, satComplementG, 0.0, 0.0,
      satComplementB, satComplementB, satComplementB + sat, 0.0, 0.0,
      0.0, 0.0, 0.0, 1.0, 0.0,
      0.0, 0.0, 0.0, 0.0, 1.0 );

  return multiply(mod, order);
}

ColorMatrix& ColorMatrix::setTint(double phi, double amount)
{
  // Rotate the hue, scale the blue and rotate back.
  return rotateHue(phi).scale(1.0, 1.0, 1.0 + amount, 1.0).rotateHue(-phi);
}

ColorMatrix& ColorMatrix::rotateHue(double phi)
{
  // Rotate the gray vector to the blue axis and rotate around the blue axis.
  return multiply(PreHue).rotateBlue(phi).multiply(PostHue);
}

bool ColorMatrix::eq(const ColorMatrix& other, double epsilon) const
{
  for (sysuint_t i = 0; i < 25; i++)
    if (!isEqualEps(arr[i], other.arr[i], epsilon)) return false;

  return true;
}

ColorMatrix& ColorMatrix::_rotateColor(double phi, int x, int y, int order)
{
  double pi_sin = sin(phi);
  double pi_cos = cos(phi);

  ColorMatrix mod;
  mod.m[x][x] =  pi_cos;
  mod.m[x][y] = -pi_sin;
  mod.m[y][x] =  pi_sin;
  mod.m[y][y] =  pi_cos;
  return multiply(mod, order);
}

ColorMatrix& ColorMatrix::_shearColor(int x, int y1, double col1, int y2, double col2, int order)
{
  ColorMatrix mod;
  mod.m[y1][x] = col1;
  mod.m[y2][x] = col2;
  return multiply(mod, order);
}

void ColorMatrix::_copyData(void *_dst, const void *_src)
{
  memcpy(_dst, _src, sizeof(double) * 25);
}

ColorMatrix ColorMatrix::Greyscale(LinkerInitialized);
ColorMatrix ColorMatrix::Identity(LinkerInitialized);
ColorMatrix ColorMatrix::White(LinkerInitialized);
ColorMatrix ColorMatrix::Zero(LinkerInitialized);
ColorMatrix ColorMatrix::PreHue(LinkerInitialized);
ColorMatrix ColorMatrix::PostHue(LinkerInitialized);

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_colormatrix_init(void)
{
  // Grayscale color matrix is modified from the GDI+ FAQ (submitted by
  // Gilles Khouzam) to use the NTSC color values.  The version in the FAQ
  // used 0.3, 0.59, and 0.11, so it was close...
  Fog::ColorMatrix::Greyscale.set(
    Fog::lumR, Fog::lumR, Fog::lumR, 0.0, 0.0,
    Fog::lumG, Fog::lumG, Fog::lumG, 0.0, 0.0,
    Fog::lumB, Fog::lumB, Fog::lumB, 0.0, 0.0,
    0.0      , 0.0      , 0.0      , 1.0, 0.0,
    0.0      , 0.0      , 0.0      , 0.0, 1.0);

  Fog::ColorMatrix::Identity.set(
    1.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 1.0);

  Fog::ColorMatrix::White.set(
    1.0, 1.0, 1.0, 0.0, 0.0,
    1.0, 1.0, 1.0, 0.0, 0.0,
    1.0, 1.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 1.0);

  Fog::ColorMatrix::Zero.set(
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0,
    0.0, 0.0, 0.0, 0.0, 0.0);

  Fog::ColorMatrix::PreHue = Fog::ColorMatrix::Identity;
  Fog::ColorMatrix::PostHue = Fog::ColorMatrix::Identity;

  // [CMD 2006-02-22]
  //
  // The green rotation is the angle where the sin is 1 / sqrt( 3 )
  // and the cosine is sqrt( 2 / 3 ). According to the documentation
  // for QColorMatrix, this is 39.182655, but I came up with
  // 35.26439. Considering that Sjaak Priester explains that 35.0
  // gave better results than 39.182655, I wonder where the higher
  // value came from...  If you want to use 39.182655, simply
  // change the value of greenRotation below.
  const double greenRotation(Fog::deg2rad(35.26439));

  // Rotating the hue requires several matrix multiplications.  To
  // speed things up a bit, use two static matrices (pre and post
  // rotation.)

  // Prepare the preHue matrix
  // Rotate the gray vector in the red plane:
  Fog::ColorMatrix::PreHue.rotateRed(Fog::deg2rad(45.0));

  // Rotate again in the green plane so it coinsides with the
  // blue axis:
  Fog::ColorMatrix::PreHue.rotateGreen(-greenRotation);

  // Shear the blue plane, in order to keep the color luminance
  // constant:
  double lum[4] = { Fog::lumR, Fog::lumG, Fog::lumB, 1.0 };

  // Transform by the luminance vector:
  Fog::ColorMatrix::PreHue.transformVector(lum);

  // Calculate the red and green factors:
  double shearRed   = lum[0] / lum[2];
  double shearGreen = lum[1] / lum[2];

  // Shear the blue plane:
  Fog::ColorMatrix::PreHue.shearBlue(shearRed, shearGreen);

  // Prepare the postHue matrix, which is actually the inverse of the
  // preHue matrix
  Fog::ColorMatrix::PostHue.shearBlue(-shearRed, -shearGreen);
  Fog::ColorMatrix::PostHue.rotateGreen(greenRotation);
  Fog::ColorMatrix::PostHue.rotateRed(Fog::deg2rad(-45.0));

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_colormatrix_shutdown(void)
{
}
