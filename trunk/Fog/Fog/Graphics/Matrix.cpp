// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

//----------------------------------------------------------------------------
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
// Affine transformations
//
//----------------------------------------------------------------------------

#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/Graphics/Matrix.h>

namespace Fog {

Matrix Matrix::fromRotation(double a)
{
  double aSin = sin(a);
  double aCos = cos(a);
  return Matrix(aCos, aSin, -aSin, aCos, 0.0, 0.0);
}

Matrix Matrix::fromScale(double s)
{
  return Matrix(s, 0.0, 0.0, s, 0.0, 0.0);
}

Matrix Matrix::fromScale(double x, double y)
{
  return Matrix(x, 0.0, 0.0, y, 0.0, 0.0);
}

Matrix Matrix::fromTranslation(double x, double y)
{
  return Matrix(1.0, 0.0, 0.0, 1.0, x, y);
}

Matrix Matrix::fromSkew(double x, double y)
{
  return Matrix(1.0, tan(y), tan(x), 1.0, 0.0, 0.0);
}

Matrix Matrix::fromLineSegment(double x1, double y1, double x2, double y2, double dist)
{
  Matrix m;

  double dx = x2 - x1;
  double dy = y2 - y1;
  if (dist > 0.0)
  {
    m.multiply(fromScale(sqrt(dx * dx + dy * dy) / dist));
  }
  m.multiply(fromRotation(atan2(dy, dx)));
  m.multiply(fromTranslation(x1, y1));

  return m;
}

Matrix Matrix::fromReflectionUnit(double ux, double uy)
{
  double ux2 = ux * 2;
  double uy2 = uy * 2;

  return Matrix(ux2 * ux - 1.0, ux2 * uy, ux2 * uy, uy2 * uy - 1.0, 0.0, 0.0);
}

Matrix Matrix::fromReflection(double a)
{
  return fromReflectionUnit(cos(a), sin(a));
}

Matrix Matrix::fromReflection(double x, double y)
{
  double t = sqrt(x * x + y * y);
  return fromReflectionUnit(x / t, y / t);
}

Matrix& Matrix::parlToParl(const double* src, const double* dst)
{
  sx  = src[2] - src[0];
  shy = src[3] - src[1];
  shx = src[4] - src[0];
  sy  = src[5] - src[1];
  tx  = src[0];
  ty  = src[1];

  invert();

  multiply(
    Matrix(
      dst[2] - dst[0],
      dst[3] - dst[1],
      dst[4] - dst[0],
      dst[5] - dst[1],
      dst[0], dst[1]));

  return *this;
}

Matrix& Matrix::rectToParl(double x1, double y1, double x2, double y2, const double* parl)
{
  double src[6];

  src[0] = x1; src[1] = y1;
  src[2] = x2; src[3] = y1;
  src[4] = x2; src[5] = y2;
  parlToParl(src, parl);

  return *this;
}

Matrix& Matrix::parlToRect(const double* parl, double x1, double y1, double x2, double y2)
{
  double dst[6];

  dst[0] = x1; dst[1] = y1;
  dst[2] = x2; dst[3] = y1;
  dst[4] = x2; dst[5] = y2;
  parlToParl(parl, dst);

  return *this;
}

Matrix& Matrix::reset()
{
  sx  = sy  = 1.0;
  shy = shx = tx = ty = 0.0;

  return *this;
}

Matrix& Matrix::translate(double x, double y)
{
  tx += x;
  ty += y; 
  return *this;
}

Matrix& Matrix::rotate(double a)
{
  double ca = cos(a); 
  double sa = sin(a);
  double t0 = sx  * ca - shy * sa;
  double t2 = shx * ca - sy * sa;
  double t4 = tx  * ca - ty * sa;
  shy = sx  * sa + shy * ca;
  sy  = shx * sa + sy * ca; 
  ty  = tx  * sa + ty * ca;
  sx  = t0;
  shx = t2;
  tx  = t4;
  return *this;
}

Matrix& Matrix::scale(double s)
{
  sx  *= s;
  shx *= s;
  tx  *= s;
  shy *= s;
  sy  *= s;
  ty  *= s;
  return *this;
}

Matrix& Matrix::scale(double x, double y)
{
  sx  *= x;
  shx *= x;
  tx  *= x;
  shy *= y;
  sy  *= y;
  ty  *= y;
  return *this;
}

Matrix& Matrix::skew(double x, double y)
{
  return multiply(Matrix::fromSkew(x, y));
}

Matrix& Matrix::multiply(const Matrix& m, int order)
{
  if (order == MATRIX_APPEND)
  {
    double t0 = sx  * m.sx + shy * m.shx;
    double t2 = shx * m.sx + sy  * m.shx;
    double t4 = tx  * m.sx + ty  * m.shx + m.tx;

    shy = sx  * m.shy + shy * m.sy;
    sy  = shx * m.shy + sy  * m.sy;
    ty  = tx  * m.shy + ty  * m.sy + m.ty;

    sx  = t0;
    shx = t2;
    tx  = t4;
  }
  else
  {
    double t0 = m.sx  * sx + m.shy * shx;
    double t2 = m.shx * sx + m.sy  * shx;
    double t4 = m.tx  * sx + m.ty  * shx + tx;

    shy = m.sx  * shy + m.shy * sy;
    sy  = m.shx * shy + m.sy  * sy;
    ty  = m.tx  * shy + m.ty  * sy + ty;

    sx  = t0;
    shx = t2;
    tx  = t4;
  }

  return *this;
}

Matrix Matrix::multiplied(const Matrix& m, int order) const
{
  Matrix result(DONT_INITIALIZE);

  const Matrix* m0 = this;
  const Matrix* m1 = &m;

  if (order == MATRIX_PREPEND)
  {
    m0 = &m;
    m1 = this;
  }

  result.sx  = m0->sx  * m1->sx  + m0->shy * m1->shx;
  result.shy = m0->sx  * m1->shy + m0->shy * m1->sy;

  result.shx = m0->shx * m1->sx  + m0->sy  * m1->shx;
  result.sy  = m0->shx * m1->shy + m0->sy  * m1->sy;

  result.tx  = m0->tx  * m1->sx  + m0->ty  * m1->shx + m1->tx;
  result.ty  = m0->tx  * m1->shy + m0->ty  * m1->sy  + m1->ty;

  return result;
}

Matrix& Matrix::multiplyInv(const Matrix& m, int order)
{
  return multiply(m.inverted(), order);
}

Matrix& Matrix::invert()
{
  double d, t0, t4;

  d  = determinantReciprocal();

  t0  =  sy  * d;
  sy  =  sx  * d;
  shy = -shy * d;
  shx = -shx * d;

  t4 = -tx * t0  - ty * shx;
  ty = -tx * shy - ty * sy;

  sx = t0;
  tx = t4;

  return *this;
}

Matrix Matrix::inverted() const
{
  Matrix result(DONT_INITIALIZE);
  double d;

  d  = determinantReciprocal();

  result.sx  =  sy  * d;
  result.sy  =  sx  * d;
  result.shy = -shy * d;
  result.shx = -shx * d;

  result.tx = -tx * result.sx  - ty * result.shx;
  result.ty = -tx * result.shy - ty * result.sy;

  return result;
}

Matrix& Matrix::flipX()
{
  sx  = -sx;
  shy = -shy;
  tx  = -tx;

  return *this;
}

Matrix& Matrix::flipY()
{
  shx = -shx;
  sy  = -sy;
  ty  = -ty;

  return *this;
}

int Matrix::getType() const
{
  int type = 0;

  if (Math::feq(tx, 0.0, Math::DEFAULT_EPSILON)) type |= MATRIX_TYPE_TRANSLATE;
  if (Math::feq(ty, 0.0, Math::DEFAULT_EPSILON)) type |= MATRIX_TYPE_TRANSLATE;

  if (Math::feq(sx, 1.0, Math::DEFAULT_EPSILON)) type |= MATRIX_TYPE_SCALE;
  if (Math::feq(sy, 1.0, Math::DEFAULT_EPSILON)) type |= MATRIX_TYPE_SCALE;

  if (Math::feq(shx, 1.0, Math::DEFAULT_EPSILON)) type |= MATRIX_TYPE_SHEAR;
  if (Math::feq(shy, 1.0, Math::DEFAULT_EPSILON)) type |= MATRIX_TYPE_SHEAR;

  return type;
}

void Matrix::transform(double* x, double* y) const
{
  double _x = *x;
  double _y = *y;

  *x = _x * sx  + _y * shx + tx;
  *y = _x * shy + _y * sy  + ty;
}

void Matrix::transform2x2(double* x, double* y) const
{
  double _x = *x;
  double _y = *y;

  *x = _x * sx  + _y * shx;
  *y = _x * shy + _y * sy;
}

void Matrix::transformInv(double* x, double* y) const
{
  double d = determinantReciprocal();
  double a = (*x - tx) * d;
  double b = (*y - ty) * d;

  *x = a * sy - b * shx;
  *y = b * sx - a * shy;
}

void Matrix::transformPoints(PointD* dst, const PointD* src, sysuint_t count) const
{
  for (sysuint_t i = 0; i < count; i++)
  {
    double x = src[i].x;
    double y = src[i].y;

    dst[i].x = x * sx  + y * shx + tx;
    dst[i].y = x * shy + y * sy  + ty;
  }
}

double Matrix::determinantReciprocal() const
{
  return 1.0 / (sx * sy - shy * shx);
}

double Matrix::scale() const
{
  double x = 0.707106781 * sx  + 0.707106781 * shx;
  double y = 0.707106781 * shy + 0.707106781 * sy;
  return sqrt(x * x + y * y);
}

bool Matrix::isValid(double epsilon) const
{
  return fabs(sx) > epsilon && fabs(sy) > epsilon;
}

bool Matrix::isIdentity(double epsilon) const
{
  return Math::feq(sx,  1.0, epsilon) &&
         Math::feq(shy, 0.0, epsilon) &&
         Math::feq(shx, 0.0, epsilon) &&
         Math::feq(sy,  1.0, epsilon) &&
         Math::feq(tx,  0.0, epsilon) &&
         Math::feq(ty,  0.0, epsilon);
}

bool Matrix::isEqual(const Matrix& m, double epsilon) const
{
  return Math::feq(sx,  m.sx,  epsilon) &&
         Math::feq(shy, m.shy, epsilon) &&
         Math::feq(shx, m.shx, epsilon) && 
         Math::feq(sy,  m.sy,  epsilon) &&
         Math::feq(tx,  m.tx,  epsilon) &&
         Math::feq(ty,  m.ty,  epsilon);
}

double Matrix::rotation() const
{
  double x1 = 0.0;
  double y1 = 0.0;
  double x2 = 1.0;
  double y2 = 0.0;
  transform(&x1, &y1);
  transform(&x2, &y2);

  return atan2(y2-y1, x2-x1);
}

void Matrix::translation(double* dx, double* dy) const
{
  *dx = tx;
  *dy = ty;
}

void Matrix::scaling(double* x, double* y) const
{
  double x1 = 0.0;
  double y1 = 0.0;
  double x2 = 1.0;
  double y2 = 1.0;

  Matrix t(*this);
  t *= fromRotation(-rotation());
  t.transform(&x1, &y1);
  t.transform(&x2, &y2);
  *x = x2 - x1;
  *y = y2 - y1;
}

void Matrix::scalingAbs(double* x, double* y) const
{
  // Used to calculate scaling coefficients in image resampling. 
  // When there is considerable shear this method gives us much
  // better estimation than just sx, sy.
  *x = sqrt(sx  * sx  + shx * shx);
  *y = sqrt(shy * shy + sy  * sy);
}

} // Fog namespace
