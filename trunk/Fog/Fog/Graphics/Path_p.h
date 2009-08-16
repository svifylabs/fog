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

// [Guard]
#ifndef _FOG_GRAPHICS_PATH_P_H
#define _FOG_GRAPHICS_PATH_P_H

// [Dependencies]
#include <Fog/Core/Math.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// Coinciding points maximal distance (Epsilon).
static const double pathVertexDistEpsilon = 1.0e-14;

// See calcIntersection (Epsilon).
static const double intersectionEpsilon = 1.0e-30;

// This epsilon is used to prevent us from adding degenerate curves
// (converging to a single point).
// The value isn't very critical. Function arc_to_bezier() has a limit
// of the sweep_angle. If fabs(sweep_angle) exceeds pi/2 the curve
// becomes inaccurate. But slight exceeding is quite appropriate.
static const double bezierArcAngleEpsilon = 0.01;

static const double curveDistanceEpsilon = 1e-30;
static const double curveCollinearityEpsilon = 1e-30;
static const double curveAngleToleranceEpsilon = 0.01;

static FOG_INLINE double calcDistance(
  double x1, double y1, double x2, double y2)
{
  double dx = x2 - x1;
  double dy = y2 - y1;
  return sqrt(dx * dx + dy * dy);
}

static FOG_INLINE double calcSqDistance(
  double x1, double y1, double x2, double y2)
{
  double dx = x2 - x1;
  double dy = y2 - y1;
  return dx * dx + dy * dy;
}

static FOG_INLINE bool calcIntersection(
  double ax, double ay, double bx, double by,
  double cx, double cy, double dx, double dy,
  double* x, double* y)
{
  double num = (ay-cy) * (dx-cx) - (ax-cx) * (dy-cy);
  double den = (bx-ax) * (dy-cy) - (by-ay) * (dx-cx);
  if (fabs(den) < intersectionEpsilon) return false;
  double r = num / den;
  *x = ax + r * (bx-ax);
  *y = ay + r * (by-ay);
  return true;
}

static FOG_INLINE double crossProduct(
  double x1, double y1,
  double x2, double y2,
  double x,  double y)
{
  return (x - x2) * (y2 - y1) - (y - y2) * (x2 - x1);
}

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_PATH_P_H
