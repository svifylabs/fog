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
// [Fog::Raster_C - Vector]
// ============================================================================

static void FOG_FASTCALL pathVertexTransform(Path::Vertex* data, sysuint_t length, const Matrix* matrix)
{
  for (sysuint_t i = length; i; i--, data++)
  {
    if (data->cmd.isVertex())
    {
      double nx = data->x * matrix->sx  + data->y * matrix->shx + matrix->tx;
      double ny = data->x * matrix->shy + data->y * matrix->sy  + matrix->ty;

      data->x = nx;
      data->y = ny;
    }
  }
}

#define VERTEX_INITIAL_SIZE 256
#define ADD_VERTEX(_cmd, _x, _y) { v->x = _x; v->y = _y; v->cmd = _cmd; v++; }

enum { ApproximateCurve3RecursionLimit = 32 };
enum { ApproximateCurve4RecursionLimit = 32 };

static err_t FOG_FASTCALL approximateCurve3(
  Path& dst,
  double x1, double y1,
  double x2, double y2,
  double x3, double y3,
  double approximationScale,
  double angleTolerance = 0.0)
{
  double distanceToleranceSquare = 0.5 / approximationScale;
  distanceToleranceSquare *= distanceToleranceSquare;

  sysuint_t level = 0;
  ApproximateCurve3Data stack[ApproximateCurve3RecursionLimit];

  Path::Vertex* v;
  Path::Vertex* vend;

realloc:
  v = dst._add(VERTEX_INITIAL_SIZE);
  if (!v) return Error::OutOfMemory;
  vend = v + VERTEX_INITIAL_SIZE - 1;

  for (;;)
  {
    // Realloc if needed.
    if (v >= vend) { dst._d->length = (sysuint_t)(v - dst._d->data); goto realloc; }

    // Calculate all the mid-points of the line segments
    double x12   = (x1 + x2) / 2.0;
    double y12   = (y1 + y2) / 2.0;
    double x23   = (x2 + x3) / 2.0;
    double y23   = (y2 + y3) / 2.0;
    double x123  = (x12 + x23) / 2.0;
    double y123  = (y12 + y23) / 2.0;

    double dx = x3-x1;
    double dy = y3-y1;
    double d = fabs(((x2 - x3) * dy - (y2 - y3) * dx));
    double da;

    if (d > curveCollinearityEpsilon)
    {
      // Regular case
      if (d * d <= distanceToleranceSquare * (dx*dx + dy*dy))
      {
        // If the curvature doesn't exceed the distanceTolerance value
        // we tend to finish subdivisions.
        if (angleTolerance < curveAngleToleranceEpsilon)
        {
          ADD_VERTEX(Path::CmdLineTo, x123, y123);
          goto ret;
        }

        // Angle & Cusp Condition
        da = fabs(atan2(y3 - y2, x3 - x2) - atan2(y2 - y1, x2 - x1));
        if (da >= M_PI) da = 2.0 * M_PI - da;

        if (da < angleTolerance)
        {
          // Finally we can stop the recursion
          ADD_VERTEX(Path::CmdLineTo, x123, y123);
          goto ret;
        }
      }
    }
    else
    {
      // Collinear case
      da = dx*dx + dy*dy;
      if (da == 0)
      {
        d = calcSqDistance(x1, y1, x2, y2);
      }
      else
      {
        d = ((x2 - x1)*dx + (y2 - y1)*dy) / da;

        if (d > 0 && d < 1)
        {
          // Simple collinear case, 1---2---3
          // We can leave just two endpoints
          goto ret;
        }

        if (d <= 0)
          d = calcSqDistance(x2, y2, x1, y1);
        else if (d >= 1)
          d = calcSqDistance(x2, y2, x3, y3);
        else
          d = calcSqDistance(x2, y2, x1 + d*dx, y1 + d*dy);
      }
      if (d < distanceToleranceSquare)
      {
        ADD_VERTEX(Path::CmdLineTo, x2, y2);
        goto ret;
      }
    }

    // Continue subdivision
    //
    // Original code from antigrain was:
    //   recursive_bezier(x1, y1, x12, y12, x123, y123, level + 1);
    //   recursive_bezier(x123, y123, x23, y23, x3, y3, level + 1);
    //
    // First recursive subdivision will be set into x1, y1, x2, y2, x3, y3,
    // second subdivision will be added into stack.

    if (level < ApproximateCurve3RecursionLimit)
    {
      stack[level].x1 = x123;
      stack[level].y1 = y123;
      stack[level].x2 = x23;
      stack[level].y2 = y23;
      stack[level].x3 = x3;
      stack[level].y3 = y3;
      level++;

      x2 = x12;
      y2 = y12;
      x3 = x123;
      y3 = y123;

      continue;
    }

ret:
    if (level == 0) break;

    level--;
    x1 = stack[level].x1;
    y1 = stack[level].y1;
    x2 = stack[level].x2;
    y2 = stack[level].y2;
    x3 = stack[level].x3;
    y3 = stack[level].y3;
  }

  // Add end point.
  ADD_VERTEX(Path::CmdLineTo, x3, y3);

  dst._d->length = (sysuint_t)(v - dst._d->data);
  FOG_ASSERT(dst._d->capacity >= dst._d->length);

  return Error::Ok;
}

static err_t FOG_FASTCALL approximateCurve4(
  Path& dst,
  double x1, double y1,
  double x2, double y2,
  double x3, double y3,
  double x4, double y4,
  double approximationScale,
  double angleTolerance = 0.0,
  double cuspLimit = 0.0)
{
  double distanceToleranceSquare = 0.5 / approximationScale;
  distanceToleranceSquare *= distanceToleranceSquare;

  sysuint_t level = 0;
  ApproximateCurve4Data stack[ApproximateCurve4RecursionLimit];

  Path::Vertex* v;
  Path::Vertex* vend;

realloc:
  v = dst._add(VERTEX_INITIAL_SIZE);
  if (!v) return Error::OutOfMemory;
  vend = v + VERTEX_INITIAL_SIZE - 2;

  for (;;)
  {
    // Realloc if needed.
    if (v >= vend) { dst._d->length = (sysuint_t)(v - dst._d->data); goto realloc; }

    // Calculate all the mid-points of the line segments
    double x12   = (x1 + x2) / 2.0;
    double y12   = (y1 + y2) / 2.0;
    double x23   = (x2 + x3) / 2.0;
    double y23   = (y2 + y3) / 2.0;
    double x34   = (x3 + x4) / 2.0;
    double y34   = (y3 + y4) / 2.0;
    double x123  = (x12 + x23) / 2.0;
    double y123  = (y12 + y23) / 2.0;
    double x234  = (x23 + x34) / 2.0;
    double y234  = (y23 + y34) / 2.0;
    double x1234 = (x123 + x234) / 2.0;
    double y1234 = (y123 + y234) / 2.0;

    // Try to approximate the full cubic curve by a single straight line
    double dx = x4 - x1;
    double dy = y4 - y1;

    double d2 = fabs(((x2 - x4) * dy - (y2 - y4) * dx));
    double d3 = fabs(((x3 - x4) * dy - (y3 - y4) * dx));
    double da1, da2, k;

    switch ((int(d2 > curveCollinearityEpsilon) << 1) +
             int(d3 > curveCollinearityEpsilon))
    {
      // All collinear OR p1==p4
      case 0:
        k = dx*dx + dy*dy;
        if (k == 0)
        {
          d2 = calcSqDistance(x1, y1, x2, y2);
          d3 = calcSqDistance(x4, y4, x3, y3);
        }
        else
        {
          k   = 1 / k;
          da1 = x2 - x1;
          da2 = y2 - y1;
          d2  = k * (da1 * dx + da2 * dy);
          da1 = x3 - x1;
          da2 = y3 - y1;
          d3  = k * (da1 * dx + da2 * dy);

          if (d2 > 0 && d2 < 1 && d3 > 0 && d3 < 1)
          {
            // Simple collinear case, 1---2---3---4
            // We can leave just two endpoints
            goto ret;
          }

          if (d2 <= 0)
            d2 = calcSqDistance(x2, y2, x1, y1);
          else if (d2 >= 1)
            d2 = calcSqDistance(x2, y2, x4, y4);
          else
            d2 = calcSqDistance(x2, y2, x1 + d2*dx, y1 + d2*dy);

          if (d3 <= 0)
            d3 = calcSqDistance(x3, y3, x1, y1);
          else if (d3 >= 1)
            d3 = calcSqDistance(x3, y3, x4, y4);
          else
            d3 = calcSqDistance(x3, y3, x1 + d3*dx, y1 + d3*dy);
        }

        if (d2 > d3)
        {
          if (d2 < distanceToleranceSquare)
          {
            ADD_VERTEX(Path::CmdLineTo, x2, y2);
            goto ret;
          }
        }
        else
        {
          if (d3 < distanceToleranceSquare)
          {
            ADD_VERTEX(Path::CmdLineTo, x3, y3);
            goto ret;
          }
        }
        break;

    // p1,p2,p4 are collinear, p3 is significant
    case 1:
      if (d3 * d3 <= distanceToleranceSquare * (dx*dx + dy*dy))
      {
        if (angleTolerance < curveAngleToleranceEpsilon)
        {
          ADD_VERTEX(Path::CmdLineTo, x23, y23);
          goto ret;
        }

        // Angle Condition
        da1 = fabs(atan2(y4 - y3, x4 - x3) - atan2(y3 - y2, x3 - x2));
        if (da1 >= M_PI) da1 = 2.0 * M_PI - da1;

        if (da1 < angleTolerance)
        {
          ADD_VERTEX(Path::CmdLineTo, x2, y2);
          ADD_VERTEX(Path::CmdLineTo, x3, y3);
          goto ret;
        }

        if (cuspLimit != 0.0)
        {
          if (da1 > cuspLimit)
          {
            ADD_VERTEX(Path::CmdLineTo, x3, y3);
            goto ret;
          }
        }
      }
      break;

    // p1,p3,p4 are collinear, p2 is significant
    case 2:
      if (d2 * d2 <= distanceToleranceSquare * (dx*dx + dy*dy))
      {
        if (angleTolerance < curveAngleToleranceEpsilon)
        {
          ADD_VERTEX(Path::CmdLineTo, x23, y23);
          goto ret;
        }

        // Angle Condition
        da1 = fabs(atan2(y3 - y2, x3 - x2) - atan2(y2 - y1, x2 - x1));
        if (da1 >= M_PI) da1 = 2.0 * M_PI - da1;

        if (da1 < angleTolerance)
        {
          ADD_VERTEX(Path::CmdLineTo, x2, y2);
          ADD_VERTEX(Path::CmdLineTo, x3, y3);
          goto ret;
        }

        if (cuspLimit != 0.0)
        {
          if (da1 > cuspLimit)
          {
            ADD_VERTEX(Path::CmdLineTo, x3, y3);
            goto ret;
          }
        }
      }
      break;

    // Regular case
    case 3:
      if ((d2 + d3)*(d2 + d3) <= distanceToleranceSquare * (dx*dx + dy*dy))
      {
        // If the curvature doesn't exceed the distance_tolerance value
        // we tend to finish subdivisions.
        if (angleTolerance < curveAngleToleranceEpsilon)
        {
          ADD_VERTEX(Path::CmdLineTo, x23, y23);
          goto ret;
        }

        // Angle & Cusp Condition
        k   = atan2(y3 - y2, x3 - x2);
        da1 = fabs(k - atan2(y2 - y1, x2 - x1));
        da2 = fabs(atan2(y4 - y3, x4 - x3) - k);
        if (da1 >= M_PI) da1 = 2.0 * M_PI - da1;
        if (da2 >= M_PI) da2 = 2.0 * M_PI - da2;

        if (da1 + da2 < angleTolerance)
        {
          // Finally we can stop the recursion
          ADD_VERTEX(Path::CmdLineTo, x23, y23);
          goto ret;
        }

        if (cuspLimit != 0.0)
        {
          if (da1 > cuspLimit)
          {
            ADD_VERTEX(Path::CmdLineTo, x2, y2);
            goto ret;
          }

          if (da2 > cuspLimit)
          {
            ADD_VERTEX(Path::CmdLineTo, x3, y3);
            goto ret;
          }
        }
      }
      break;
    }

    // Continue subdivision
    //
    // Original antigrain code:
    //   recursive_bezier(x1, y1, x12, y12, x123, y123, x1234, y1234, level + 1);
    //   recursive_bezier(x1234, y1234, x234, y234, x34, y34, x4, y4, level + 1);
    //
    // First recursive subdivision will be set into x1, y1, x2, y2, x3, y3,
    // second subdivision will be added into stack.
    if (level < ApproximateCurve4RecursionLimit)
    {
      stack[level].x1 = x1234;
      stack[level].y1 = y1234;
      stack[level].x2 = x234;
      stack[level].y2 = y234;
      stack[level].x3 = x34;
      stack[level].y3 = y34;
      stack[level].x4 = x4;
      stack[level].y4 = y4;
      level++;

      x2 = x12;
      y2 = y12;
      x3 = x123;
      y3 = y123;
      x4 = x1234;
      y4 = y1234;

      continue;
    }

ret:
    if (level == 0) break;

    level--;
    x1 = stack[level].x1;
    y1 = stack[level].y1;
    x2 = stack[level].x2;
    y2 = stack[level].y2;
    x3 = stack[level].x3;
    y3 = stack[level].y3;
    x4 = stack[level].x4;
    y4 = stack[level].y4;
  }

  // Add end point.
  ADD_VERTEX(Path::CmdLineTo, x4, y4);

  dst._d->length = (sysuint_t)(v - dst._d->data);
  FOG_ASSERT(dst._d->capacity >= dst._d->length);

  return Error::Ok;
}

#undef ADD_VERTEX

} // Raster namespace
} // Fog namespace
