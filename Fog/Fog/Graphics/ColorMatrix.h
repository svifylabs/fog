// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// AggOO - Version 0.1
// Copyright (c) 2006 Dratek Software
//
// Author of AggOO (Chad M. Draper) agreed relicencing code to the MIT licence
// that is used for Fog (original code was 3 clause BSD licence). Thanks.

// [Guard]
#ifndef _FOG_GRAPHICS_COLORMATRIX_H
#define _FOG_GRAPHICS_COLORMATRIX_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Core/Assert.h>
#include <Fog/Core/Constants.h>
#include <Fog/Graphics/Constants.h>

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct Rgba;
struct Rgba64;

// ============================================================================
// [Fog::ColorMatrix]
// ============================================================================

//! @brief Color matrix
//!
//! The color matrix is a 5x5 matrix that can be used to do color transformations
//! The idea is like affine transformations for vector painter coordinates.
struct FOG_API ColorMatrix
{
  // [Construction / Destruction]

  FOG_INLINE ColorMatrix(_LinkerInitialized _linkerInitialized) {}

  FOG_INLINE ColorMatrix()
  {
    _copyData(this->m, Identity.m);
  }

  FOG_INLINE ColorMatrix(const double src[5][5])
  {
    _copyData(this->m, src);
  }

  FOG_INLINE ColorMatrix(const double src[25])
  {
    _copyData(this->m, src);
  }

  FOG_INLINE ColorMatrix(const ColorMatrix& other)
  {
    _copyData(this->m, other.m);
  }

  FOG_INLINE ColorMatrix(
    double m00, double m01, double m02, double m03, double m04,
    double m10, double m11, double m12, double m13, double m14,
    double m20, double m21, double m22, double m23, double m24,
    double m30, double m31, double m32, double m33, double m34,
    double m40, double m41, double m42, double m43, double m44)
  {
    m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03; m[0][4] = m04;
    m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13; m[1][4] = m14;
    m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23; m[2][4] = m24;
    m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33; m[3][4] = m34;
    m[4][0] = m40; m[4][1] = m41; m[4][2] = m42; m[4][3] = m43; m[4][4] = m44;
  }

  // [Type]

  //! @brief Characteristics of color matrix.
  //!
  //! Characteristics is bit mask.
  enum Part
  {
    //! @brief Matrix contains RGB shear part.
    //!
    //! RGB shear part is illustrated here:
    //!   [n X X n n]
    //!   [X n X n n]
    //!   [X X n n n]
    //!   [n n n n n]
    //!   [n n n n n]
    PartShearRGB = 0x01,

    //! @brief Matrix contains alpha shear part.
    //!
    //! Alpha shear part is illustrated here:
    //!   [n n n X n]
    //!   [n n n X n]
    //!   [n n n X n]
    //!   [X X X n n]
    //!   [n n n n n]
    PartShearAlpha = 0x02,

    //! @brief Matrix contains RGBA shear part.
    //!
    //! RGBA shear part is illustrated here:
    //!   [n X X X n]
    //!   [X n X X n]
    //!   [X X n X n]
    //!   [X X X n n]
    //!   [n n n n n]
    //!
    //! @note RGBA shear is combination of RGB and Alpha shear parts.
    PartShearRGBA = 0x03,

    //! @brief Matrix contains RGB lut part.
    //!
    //! RGB lut part is illustrated here:
    //!   [X n n n n]
    //!   [n X n n n]
    //!   [n n X n n]
    //!   [n n n n n]
    //!   [n n n n n]
    PartLutRGB = 0x04,

    //! @brief Matrix contains RGB lut part.
    //!
    //! Alpha lut part is illustrated here:
    //!   [n n n n n]
    //!   [n n n n n]
    //!   [n n n n n]
    //!   [n n n X n]
    //!   [n n n n n]
    PartLutAlpha = 0x08,

    //! @brief Matrix contains RGBA lut part.
    //!
    //! RGBA lut part is illustrated here:
    //!   [X n n n n]
    //!   [n X n n n]
    //!   [n n X n n]
    //!   [n n n X n]
    //!   [n n n n n]
    //!
    //! @note RGBA lut is combination of RGB and Alpha lut parts.
    PartLutRGBA = 0x0C,

    //! @brief Matrix contains RGB translation part
    //!
    //! RGB translation part is illustrated here:
    //!   [n n n n n]
    //!   [n n n n n]
    //!   [n n n n n]
    //!   [n n n n n]
    //!   [X X X n n]
    PartTranslateRGB  = 0x10,

    //! @brief Matrix contains alpha translation part
    //!
    //! Alpha translation part is illustrated here:
    //!   [n n n n n]
    //!   [n n n n n]
    //!   [n n n n n]
    //!   [n n n n n]
    //!   [n n n X n]
    PartTranslateAlpha = 0x20,

    //! @brief Matrix contains RGBA translation part
    //!
    //! RGBA translation part is illustrated here:
    //!   [n n n n n]
    //!   [n n n n n]
    //!   [n n n n n]
    //!   [n n n n n]
    //!   [X X X X n]
    //!
    //! @note RGBA translation is combination of RGB and Alpha translation parts.
    PartTranslateRGBA = 0x30
  };

  //! @brief Determines type of color matrix.
  //!
  //! Type of color matrix is important part of optimization that can be done
  //! in blitters. The main adventage of color matrix class is that many color
  //! operations can be defined with it. But these operations usually not uses
  //! all matrix cells, so the type() checks for zero and one values and
  //! returns type value that can be used by optimized blitters.
  //!
  //! @see @c Type for type possibilities and its descriptions.
  int type() const;

  // [Operations]

  FOG_INLINE ColorMatrix& set(const double src[5][5])
  {
    _copyData(this->m, src);
    return *this;
  }

  FOG_INLINE ColorMatrix& set(const double src[25])
  {
    _copyData(this->m, src);
    return *this;
  }

  FOG_INLINE ColorMatrix& set(const ColorMatrix& other)
  {
    _copyData(this->m, other.m);
    return *this;
  }

  FOG_INLINE ColorMatrix& set(
    double m00, double m01, double m02, double m03, double m04,
    double m10, double m11, double m12, double m13, double m14,
    double m20, double m21, double m22, double m23, double m24,
    double m30, double m31, double m32, double m33, double m34,
    double m40, double m41, double m42, double m43, double m44)
  {
    m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03; m[0][4] = m04;
    m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13; m[1][4] = m14;
    m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23; m[2][4] = m24;
    m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33; m[3][4] = m34;
    m[4][0] = m40; m[4][1] = m41; m[4][2] = m42; m[4][3] = m43; m[4][4] = m44;
    return *this;
  }

  //! @brief Add other matrix into this matrix.
  ColorMatrix& add(const ColorMatrix& other);
  //! @brief Subtract other matrix from this matrix.
  ColorMatrix& subtract(const ColorMatrix& other);
  //! @brief Multiply this matrix with other matrix.
  ColorMatrix& multiply(const ColorMatrix& other, int order = MatrixOrderPrepend);
  //! @brief Multiply this matrix with scalar.
  ColorMatrix& multiply(double scalar);

  //! @brief Transform a 1x4 vector by the matrix.
  void transformVector(double* v) const;

  //! @brief Transform a @a rgb color.
  void transformRgb(Rgba* rgb) const;

  //! @brief Transform a @a rgb color.
  void transformRgb64(Rgba64* rgb) const;

  //! @brief Transform a @a rgba color.
  void transformRgba(Rgba* rgba) const;

  //! @brief Transform a @a rgba color.
  void transformRgba64(Rgba64* rgba) const;

  //! @brief Transform alpha value @a a.
  void transformAlpha(uint8_t* a) const;

  //! @brief Scale the color components of the matrix.
  ColorMatrix& scale(double sr, double sg, double sb, double sa = 1.0, int order = MatrixOrderPrepend);

  FOG_INLINE ColorMatrix& scaleColors(double scalar, int order = MatrixOrderPrepend)
  { return scale(scalar, scalar, scalar, 1.0, order );}

  FOG_INLINE ColorMatrix& scaleOpacity(double scalar, int order = MatrixOrderPrepend)
  { return scale(1.0, 1.0, 1.0, scalar, order ); }

  //! @brief Translate the color components of the matrix.
  ColorMatrix& translate(double tr, double tg, double tb, double ta = 0.0, int order = MatrixOrderPrepend);

  FOG_INLINE ColorMatrix& translateColors(double transVal, int order = MatrixOrderPrepend)
  { return translate( transVal, transVal, transVal, 0.0, order ); }

  FOG_INLINE ColorMatrix& translateOpacity(double transVal, int order = MatrixOrderPrepend)
  { return translate( 1.0, 1.0, 1.0, transVal, order ); }

  FOG_INLINE ColorMatrix& rotateRed(double phi, int order = MatrixOrderPrepend)
  { return _rotateColor(phi, 2, 1, order); }

  FOG_INLINE ColorMatrix& rotateGreen(double phi, int order = MatrixOrderPrepend)
  { return _rotateColor(phi, 0, 2, order); }

  FOG_INLINE ColorMatrix& rotateBlue(double phi, int order = MatrixOrderPrepend)
  { return _rotateColor(phi, 1, 0, order); }

  FOG_INLINE ColorMatrix& shearRed(double g, double b, int order = MatrixOrderPrepend)
  { return _shearColor(0, 1, g, 2, b, order); }

  FOG_INLINE ColorMatrix& shearGreen( double r, double b, int order = MatrixOrderPrepend)
  { return _shearColor(1, 0, r, 2, b, order); }

  FOG_INLINE ColorMatrix& shearBlue(double r, double g, int order = MatrixOrderPrepend)
  { return _shearColor(2, 0, r, 1, g, order); }

  //! @brief Set the saturation of the matrix
  //! @remark Saturation of 0.0 yields black & white, 1.0 is neutral.
  ColorMatrix& setSaturation(double sat, int order = MatrixOrderPrepend);

  //! @brief Scale the tint.
  //! @param phi [-180..180, 0 = blue]
  //! @param amount [-1.0..1.0, 0 = neutral]
  ColorMatrix& setTint(double phi, double amount);

  //! @brief Rotate the hue about the gray axis.
  //! @remark Luminance is fixed, other colors are rotated.
  ColorMatrix& rotateHue(double phi);

  // [Operator Overload]

  //! @brief Assignment operator.
  FOG_INLINE ColorMatrix& operator=(const ColorMatrix& other)
  { _copyData(this->m, other.m); return *this; }

  bool eq(const ColorMatrix& other, double epsilon = defaultEpsilon) const;

  //! @brief Equality operator.
  FOG_INLINE bool operator==(const ColorMatrix& other)
  { return eq(other); }

  //! @brief Inequality operator.
  FOG_INLINE bool operator!=(const ColorMatrix& other)
  { return !eq(other); }

  FOG_INLINE ColorMatrix& operator+=(const ColorMatrix& other) { return add(other); }
  FOG_INLINE ColorMatrix& operator-=(const ColorMatrix& other) { return subtract(other); }
  FOG_INLINE ColorMatrix& operator*=(const ColorMatrix& other) { return multiply(other); }

  FOG_INLINE ColorMatrix operator+(const ColorMatrix& other) { ColorMatrix t(*this); t.add(other); return t; }
  FOG_INLINE ColorMatrix operator-(const ColorMatrix& other) { ColorMatrix t(*this); t.subtract(other); return t; }
  FOG_INLINE ColorMatrix operator*(const ColorMatrix& other) { ColorMatrix t(*this); t.multiply(other); return t; }

  //! @brief Overload the [] operator for access to a row. This will enable
  //! access to the elements by using [r][c].
  FOG_INLINE double* operator[](sysuint_t row)
  {
    FOG_ASSERT(row < 5);
    return m[row];
  }

  //! @overload.
  FOG_INLINE const double* const operator[](sysuint_t row) const
  {
    FOG_ASSERT(row < 5);
    return m[row];
  }

  // [Members]

  union
  {
    double m[5][5];
    double arr[25];
  };

  // [Statics]

  static ColorMatrix Greyscale;
  static ColorMatrix Identity;
  static ColorMatrix White;
  static ColorMatrix Zero;
  static ColorMatrix PreHue;
  static ColorMatrix PostHue;

  // [Private]

private:
  //! @brief Rotate the matrix about a color axis.
  //! @note The color of the axis is unchanged, while the others are rotated 
  //! in color space.
  ColorMatrix& _rotateColor(double phi, int x, int y, int order = MatrixOrderPrepend);

  //! @brief Shear the matrix in one of the color planes
  ColorMatrix& _shearColor(int x, int y1, double col1, int y2, double col2, int order = MatrixOrderPrepend);

  //! @brief Copy matrix data from @a srt to @a dst.
  static void _copyData(void* dst, const void* src);
};

} // Fog namespace

// [Guard]
#endif // _FOG_GRAPHICS_COLORMATRIX_H
