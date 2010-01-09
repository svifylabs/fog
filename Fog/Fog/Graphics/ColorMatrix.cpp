// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// AggOO - Version 0.1
// Copyright (c) 2006 Dratek Software
//
// Author of AggOO (Chad M. Draper) agreed relicencing code to the MIT licence
// that is used for Fog (original code was 3 clause BSD licence). Thanks.

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Constants.h>
#include <Fog/Core/Math.h>
#include <Fog/Graphics/Argb.h>
#include <Fog/Graphics/ColorMatrix.h>

namespace Fog {

// The luminance color weights are close to the values of the NTSC color
// weights, but these values are preferable.
static const float lumR = 0.3086f;
static const float lumG = 0.6094f;
static const float lumB = 0.0820f;

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
  if (m[0][1] != 0.0f || m[0][2] != 0.0f ||
      m[1][0] != 0.0f || m[1][2] != 0.0f ||
      m[2][0] != 0.0f || m[2][1] != 0.0f) parts |= TYPE_SHEAR_RGB;

  // Alpha shear part is illustrated here:
  //   [n n n X n]
  //   [n n n X n]
  //   [n n n X n]
  //   [X X X n n]
  //   [n n n n n]
  if (m[0][3] != 0.0f ||
      m[1][3] != 0.0f ||
      m[2][3] != 0.0f ||
      m[2][0] != 0.0f || m[2][1] != 0.0f || m[2][2] != 0.0f) parts |= TYPE_SHEAR_ALPHA;

  // RGB lut part is illustrated here:
  //   [X n n n n]
  //   [n X n n n]
  //   [n n X n n]
  //   [n n n n n]
  //   [n n n n n]
  if (m[0][0] != 0.0f ||
      m[1][1] != 0.0f ||
      m[2][2] != 0.0f) parts |= TYPE_LUT_RGB;

  // Alpha lut part is illustrated here:
  //   [n n n n n]
  //   [n n n n n]
  //   [n n n n n]
  //   [n n n X n]
  //   [n n n n n]
  if (m[3][3] != 0.0f) parts |= TYPE_LUT_ALPHA;

  //! @brief Matrix contains const RGB lut part (all cells are set to 1.0).
  //!
  //! Const RGB lut part is illustrated here:
  //!   [1 n n n n]
  //!   [n 1 n n n]
  //!   [n n 1 n n]
  //!   [n n n n n]
  //!   [n n n n n]
  if (m[0][0] == 1.0f &&
      m[1][1] == 1.0f &&
      m[2][2] == 1.0f) parts |= TYPE_CONST_RGB;

  //! @brief Matrix contains const alpha lut part (cell set to 1.0).
  //!
  //! Const alpha lut part is illustrated here:
  //!   [n n n n n]
  //!   [n n n n n]
  //!   [n n n n n]
  //!   [n n n 1 n]
  //!   [n n n n n]
  if (m[3][3] == 1.0f) parts |= TYPE_CONST_ALPHA;

  // RGB translation part is illustrated here:
  //   [n n n n n]
  //   [n n n n n]
  //   [n n n n n]
  //   [n n n n n]
  //   [X X X n n]
  if (m[4][0] != 0.0f || m[4][1] != 0.0f || m[4][2] != 0.0f) parts |= TYPE_TRANSLATE_RGB;

  // Alpha translation part is illustrated here:
  //   [n n n n n]
  //   [n n n n n]
  //   [n n n n n]
  //   [n n n n n]
  //   [n n n X n]
  if (m[4][3] != 0.0f) parts |= TYPE_TRANSLATE_ALPHA;

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

  const ColorMatrix* FOG_RESTRICT mat1 = &save;
  const ColorMatrix* FOG_RESTRICT mat2 = &other;

  if (order == MATRIX_APPEND)
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

ColorMatrix& ColorMatrix::multiply(float scalar)
{
  for (sysuint_t i = 0; i < 25; i++)
    arr[i] *= scalar;
  return *this;
}

void ColorMatrix::transformVector(float* FOG_RESTRICT v) const
{
  float temp[4];

  temp[0] = v[0] * m[0][0] + v[1] * m[1][0] + v[2] * m[2][0] + v[3] * m[3][0] + m[4][0];
  temp[1] = v[0] * m[0][1] + v[1] * m[1][1] + v[2] * m[2][1] + v[3] * m[3][1] + m[4][1];
  temp[2] = v[0] * m[0][2] + v[1] * m[1][2] + v[2] * m[2][2] + v[3] * m[3][2] + m[4][2];
  temp[3] = v[0] * m[0][3] + v[1] * m[1][3] + v[2] * m[2][3] + v[3] * m[3][3] + m[4][3];

  v[0] = temp[0];
  v[1] = temp[1];
  v[2] = temp[2];
  v[3] = temp[3];
}

void ColorMatrix::transformRgb(Argb* clr) const
{
  float r = (float)clr->r;
  float g = (float)clr->g;
  float b = (float)clr->b;

  int tr = Math::iround(r * m[0][0] + g * m[1][0] + b * m[2][0] + 255.0f * m[3][0] + m[4][0] * 255.0f);
  int tg = Math::iround(r * m[0][1] + g * m[1][1] + b * m[2][1] + 255.0f * m[3][1] + m[4][1] * 255.0f);
  int tb = Math::iround(r * m[0][2] + g * m[1][2] + b * m[2][2] + 255.0f * m[3][2] + m[4][2] * 255.0f);

  tr = Math::bound<int>(tr, 0, 255);
  tg = Math::bound<int>(tg, 0, 255);
  tb = Math::bound<int>(tb, 0, 255);

  clr->r = (uint8_t)tr;
  clr->g = (uint8_t)tg;
  clr->b = (uint8_t)tb;
}

void ColorMatrix::transformArgb(Argb* clr) const
{
  float r = (float)clr->r;
  float g = (float)clr->g;
  float b = (float)clr->b;
  float a = (float)clr->a;

  int tr = Math::iround(r * m[0][0] + g * m[1][0] + b * m[2][0] + a * m[3][0] + m[4][0] * 255.0f);
  int tg = Math::iround(r * m[0][1] + g * m[1][1] + b * m[2][1] + a * m[3][1] + m[4][1] * 255.0f);
  int tb = Math::iround(r * m[0][2] + g * m[1][2] + b * m[2][2] + a * m[3][2] + m[4][2] * 255.0f);
  int ta = Math::iround(r * m[0][3] + g * m[1][3] + b * m[2][3] + a * m[3][3] + m[4][3] * 255.0f);

  tr = Math::bound<int>(tr, 0, 255);
  tg = Math::bound<int>(tg, 0, 255);
  tb = Math::bound<int>(tb, 0, 255);
  ta = Math::bound<int>(ta, 0, 255);

  clr->set(tr, tg, tb, ta);
}

void ColorMatrix::transformAlpha(uint8_t* a) const
{
  int ta = Math::iround((float)*a * m[3][3] + m[4][3] * 255.0f);

  ta = Math::bound<int>(ta, 0, 255);

  *a = (uint8_t)ta;
}

ColorMatrix& ColorMatrix::scale(float sa, float sr, float sg, float sb, int order)
{
  ColorMatrix mod;
  mod[0][0] = sr;
  mod[1][1] = sg;
  mod[2][2] = sb;
  mod[3][3] = sa;
  return multiply(mod, order);
}

ColorMatrix& ColorMatrix::translate(float ta, float tr, float tg, float tb, int order)
{
  ColorMatrix mod;
  mod[4][0] = tr;
  mod[4][1] = tg;
  mod[4][2] = tb;
  mod[4][3] = ta;
  return multiply(mod, order);
}

ColorMatrix& ColorMatrix::setSaturation(float sat, int order)
{
  // If the saturation is 1.0, then this matrix remains unchanged.
  // If the saturation is 0.0, each color is scaled by its luminance
  float satComplement  = 1.0f - sat;
  float satComplementR = lumR * satComplement;
  float satComplementG = lumG * satComplement;
  float satComplementB = lumB * satComplement;

  // Create the matrix:
  ColorMatrix mod(
      satComplementR + sat, satComplementR, satComplementR, 0.0f, 0.0f,
      satComplementG, satComplementG + sat, satComplementG, 0.0f, 0.0f,
      satComplementB, satComplementB, satComplementB + sat, 0.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
      0.0f, 0.0f, 0.0f, 0.0f, 1.0f );

  return multiply(mod, order);
}

ColorMatrix& ColorMatrix::setTint(float phi, float amount)
{
  // Rotate the hue, scale the blue and rotate back.
  return rotateHue(phi).scale(1.0f, 1.0f, 1.0f, 1.0f + amount).rotateHue(-phi);
}

ColorMatrix& ColorMatrix::rotateHue(float phi)
{
  // Rotate the gray vector to the blue axis and rotate around the blue axis.
  return multiply(PreHue).rotateBlue(phi).multiply(PostHue);
}

bool ColorMatrix::eq(const ColorMatrix& other, float epsilon) const
{
  for (sysuint_t i = 0; i < 25; i++)
    if (!Math::feq(arr[i], other.arr[i], epsilon)) return false;

  return true;
}

ColorMatrix& ColorMatrix::_rotateColor(float phi, int x, int y, int order)
{
  float phiSin;
  float phiCos;
  Math::sincos(phi, &phiSin, &phiCos);

  ColorMatrix mod;
  mod.m[x][x] =  phiCos;
  mod.m[x][y] = -phiSin;
  mod.m[y][x] =  phiSin;
  mod.m[y][y] =  phiCos;
  return multiply(mod, order);
}

ColorMatrix& ColorMatrix::_shearColor(int x, int y1, float col1, int y2, float col2, int order)
{
  ColorMatrix mod;
  mod.m[y1][x] = col1;
  mod.m[y2][x] = col2;
  return multiply(mod, order);
}

void ColorMatrix::_copyData(void *_dst, const void *_src)
{
  memcpy(_dst, _src, sizeof(float) * 25);
}

ColorMatrix ColorMatrix::Greyscale(DONT_INITIALIZE);
ColorMatrix ColorMatrix::Identity(DONT_INITIALIZE);
ColorMatrix ColorMatrix::White(DONT_INITIALIZE);
ColorMatrix ColorMatrix::Zero(DONT_INITIALIZE);
ColorMatrix ColorMatrix::PreHue(DONT_INITIALIZE);
ColorMatrix ColorMatrix::PostHue(DONT_INITIALIZE);

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_colormatrix_init(void)
{
  using namespace Fog;

  // Grayscale color matrix is modified from the GDI+ FAQ (submitted by
  // Gilles Khouzam) to use the NTSC color values.  The version in the FAQ
  // used 0.3, 0.59, and 0.11, so it was close...
  ColorMatrix::Greyscale.set(
    lumR, lumR, lumR, 0.0f, 0.0f,
    lumG, lumG, lumG, 0.0f, 0.0f,
    lumB, lumB, lumB, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

  ColorMatrix::Identity.set(
    1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

  ColorMatrix::White.set(
    1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

  ColorMatrix::Zero.set(
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

  ColorMatrix::PreHue = ColorMatrix::Identity;
  ColorMatrix::PostHue = ColorMatrix::Identity;

  // [CMD 2006-02-22]
  //
  // The green rotation is the angle where the sin is 1 / sqrt( 3 )
  // and the cosine is sqrt( 2 / 3 ). According to the documentation
  // for QColorMatrix, this is 39.182655, but I came up with
  // 35.26439. Considering that Sjaak Priester explains that 35.0
  // gave better results than 39.182655, I wonder where the higher
  // value came from...  If you want to use 39.182655, simply
  // change the value of greenRotation below.
  const float greenRotation(Math::deg2rad(35.26439f));

  // Rotating the hue requires several matrix multiplications.  To
  // speed things up a bit, use two static matrices (pre and post
  // rotation.)

  // Prepare the preHue matrix
  // Rotate the gray vector in the red plane:
  ColorMatrix::PreHue.rotateRed(Math::deg2rad(45.0f));

  // Rotate again in the green plane so it coinsides with the
  // blue axis:
  ColorMatrix::PreHue.rotateGreen(-greenRotation);

  // Shear the blue plane, in order to keep the color luminance
  // constant:
  float lum[4] = { lumR, lumG, lumB, 1.0f };

  // Transform by the luminance vector:
  ColorMatrix::PreHue.transformVector(lum);

  // Calculate the red and green factors:
  float shearRcp = 1.0f / lum[2];
  float shearRed   = lum[0] * shearRcp;
  float shearGreen = lum[1] * shearRcp;

  // Shear the blue plane:
  ColorMatrix::PreHue.shearBlue(shearRed, shearGreen);

  // Prepare the postHue matrix, which is actually the inverse of the
  // preHue matrix
  ColorMatrix::PostHue.shearBlue(-shearRed, -shearGreen);
  ColorMatrix::PostHue.rotateGreen(greenRotation);
  ColorMatrix::PostHue.rotateRed(Math::deg2rad(-45.0f));

  return ERR_OK;
}

FOG_INIT_DECLARE void fog_colormatrix_shutdown(void)
{
}
