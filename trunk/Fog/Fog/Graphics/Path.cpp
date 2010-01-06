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

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Assert.h>
#include <Fog/Core/Constants.h>
#include <Fog/Core/Math.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/Std.h>
#include <Fog/Graphics/Matrix.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Path.h>
#include <Fog/Graphics/PathUtil.h>

namespace Fog {

// ============================================================================
// [Fog::Path::Data]
// ============================================================================

Static<Path::Data> Path::sharedNull;

Path::Data* Path::Data::ref() const
{
  if (flags & IsSharable)
  {
    refCount.inc();
    return const_cast<Data*>(this);
  }
  else
  {
    return copy();
  }
}

void Path::Data::deref()
{
  if (refCount.deref() && (flags & IsDynamic) != 0) 
  {
    Memory::free((void*)this);
  }
}

Path::Data* Path::Data::copy() const
{
  if (!length) return refAlways();

  Data* d = alloc(length);
  if (!d) return NULL;

  d->length = length;
  d->flat = flat;
  Memory::copy(d->data, data, sizeof(PathVertex) * length);

  return d;
}

Path::Data* Path::Data::alloc(sysuint_t capacity)
{
  sysuint_t dsize = 
    sizeof(Data) - sizeof(PathVertex) + capacity * sizeof(PathVertex);

  Data* d = reinterpret_cast<Data*>(Memory::alloc(dsize));
  if (!d) return NULL;

  d->refCount.init(1);
  d->flags = IsDynamic | IsSharable;
  d->flat = true;
  d->capacity = capacity;
  d->length = 0;

  return d;
}

Path::Data* Path::Data::realloc(Data* d, sysuint_t capacity)
{
  FOG_ASSERT(d->length <= capacity);

  if (d->flags & IsDynamic)
  {
    sysuint_t dsize = 
      sizeof(Data) - sizeof(PathVertex) + capacity * sizeof(PathVertex);

    Data* newd = reinterpret_cast<Data*>(Memory::realloc((void*)d, dsize));
    if (!newd) return NULL;

    newd->capacity = capacity;
    return newd;
  }
  else
  {
    Data* newd = alloc(capacity);

    newd->length = d->length;
    newd->flat = d->flat;
    Memory::copy(newd->data, d->data, d->length * sizeof(PathVertex));
    d->deref();
    return newd;
  }
}

// ============================================================================
// [Fog::Path]
// ============================================================================

static FOG_INLINE PathCmd lastCmd(Path::Data* d)
{
  return PathCmd(d->length
    ? PathCmd(d->data[d->length-1].cmd)
    : PathCmd(PATH_CMD_END));
}

static FOG_INLINE double lastX(Path::Data* d)
{
  return d->length ? d->data[d->length-1].x : 0.0;
}

static FOG_INLINE double lastY(Path::Data* d)
{
  return d->length ? d->data[d->length-1].y : 0.0;
}

static FOG_INLINE void relToAbsInline(Path::Data* d, double* x, double* y)
{
  PathVertex* v = d->data + d->length;

  if (d->length && v[-1].cmd.isVertex())
  {
    *x += v[-1].x;
    *y += v[-1].y;
  }
}

static FOG_INLINE void relToAbsInline(Path::Data* d, double* x0, double* y0, double* x1, double* y1)
{
  PathVertex* v = d->data + d->length;

  if (d->length && v[-1].cmd.isVertex())
  {
    *x0 += v[-1].x;
    *y0 += v[-1].y;
    *x1 += v[-1].x;
    *y1 += v[-1].y;
  }
}

static FOG_INLINE void relToAbsInline(Path::Data* d, double* x0, double* y0, double* x1, double* y1, double* x2, double* y2)
{
  PathVertex* v = d->data + d->length;

  if (d->length && v[-1].cmd.isVertex())
  {
    *x0 += v[-1].x;
    *y0 += v[-1].y;
    *x1 += v[-1].x;
    *y1 += v[-1].y;
    *x2 += v[-1].x;
    *y2 += v[-1].y;
  }
}

template<class VertexStorage>
static err_t aggJoinPath(Path* self, VertexStorage& a)
{
  sysuint_t i, len = a.num_vertices();
  PathVertex* v = self->_add(len);
  if (!v) return ERR_RT_OUT_OF_MEMORY;

  a.rewind(0);
  for (i = 0; i < len; i++)
  {
    v[i].cmd = a.vertex(&v[i].x, &v[i].y);
  }

  return ERR_OK;
}

// ============================================================================
// [Fog::Path - Construction / Destruction]
// ============================================================================

Path::Path()
{
  _d = sharedNull->refAlways();
}

Path::Path(const Path& other)
{
  _d = other._d->ref();
}

Path::~Path()
{
  _d->deref();
}

// ============================================================================
// [Fog::Path - Data]
// ============================================================================

err_t Path::reserve(sysuint_t capacity)
{
  if (_d->refCount.get() == 1 && _d->capacity >= capacity) return ERR_OK;

  Data* newd = Data::alloc(capacity);
  if (!newd) return ERR_RT_OUT_OF_MEMORY;

  newd->length = _d->length;
  Memory::copy(newd->data, _d->data, _d->length * sizeof(PathVertex));

  atomicPtrXchg(&_d, newd)->deref();
  return ERR_OK;
}

void Path::squeeze()
{
  if (_d->length == _d->capacity) return;

  if (_d->refCount.get() == 1)
  {
    Data* newd = Data::realloc(_d, _d->length);
    if (!newd) return;

    atomicPtrXchg(&_d, newd);
  }
  else
  {
    Data* newd = _d->copy();
    if (!newd) return;

    atomicPtrXchg(&_d, newd)->deref();
  }
}

PathVertex* Path::_add(sysuint_t count)
{
  sysuint_t length = _d->length;
  sysuint_t remain = _d->capacity - length;

  if (_d->refCount.get() == 1 && count <= remain)
  {
    _d->length += count;
    return _d->data + length;
  }
  else
  {
    sysuint_t optimalCapacity = 
      Std::calcOptimalCapacity(sizeof(Data), sizeof(PathVertex), length, length + count);

    Data* newd = Data::alloc(optimalCapacity);
    if (!newd) return NULL;

    newd->length = length + count;
    newd->flat = _d->flat;
    Memory::copy(newd->data, _d->data, length * sizeof(PathVertex));

    atomicPtrXchg(&_d, newd)->deref();
    return newd->data + length;
  }
}

err_t Path::_detach()
{
  if (isDetached()) return ERR_OK;

  Data* newd = _d->copy();
  if (!newd) return ERR_RT_OUT_OF_MEMORY;

  atomicPtrXchg(&_d, newd)->deref();
  return ERR_OK;
}

err_t Path::set(const Path& other)
{
  Data* self_d = _d;
  Data* other_d = other._d;
  if (self_d == other_d) return ERR_OK;

  if ((self_d->flags & Data::IsStrong) != 0 || 
      (other_d->flags & Data::IsSharable) == 0)
  {
    return setDeep(other);
  }
  else
  {
    atomicPtrXchg(&_d, other_d->ref())->deref();
    return ERR_OK;
  }
}

err_t Path::setDeep(const Path& other)
{
  Data* self_d = _d;
  Data* other_d = other._d;

  if (self_d == other_d) return ERR_OK;
  if (other_d->length == 0) { clear(); return ERR_OK; }

  err_t err = reserve(other_d->length);
  if (err) { clear(); return ERR_RT_OUT_OF_MEMORY; }

  self_d = _d;
  sysuint_t len = other_d->length;

  self_d->length = len;
  self_d->flat = other_d->flat;
  Memory::copy(self_d->data, other_d->data, len * sizeof(PathVertex));

  return ERR_OK;
}

void Path::clear()
{
  if (_d->refCount.get() > 1)
  {
    atomicPtrXchg(&_d, sharedNull->refAlways())->deref();
  }
  else
  {
    _d->length = 0;
    _d->flat = true;
  }
}

void Path::free()
{
  atomicPtrXchg(&_d, sharedNull->refAlways())->deref();
}

// ============================================================================
// [Fog::Path - Bounding Rect]
// ============================================================================

RectD Path::getBoundingRect() const
{
  sysuint_t i = _d->length;
  PathVertex* v = _d->data;

  double x1 = 0.0, y1 = 0.0;
  double x2 = 0.0, y2 = 0.0;

  while (i)
  {
    i--;
    if (v->cmd.isVertex())
    {
      x1 = v->x;
      y1 = v->y;
      x2 = v->x;
      y2 = v->y;
      v++;
      break;
    }
    else
      v++;
  }

  while (i)
  {
    if (v->cmd.isVertex())
    {
      if (x1 > v->x) x1 = v->x;
      if (y1 > v->y) y1 = v->y;
      if (x2 < v->x) x2 = v->x;
      if (y2 < v->y) y2 = v->y;
    }

    i--;
    v++;
  }

  return RectD(x1, y1, x2 - x1, y2 - y1);
}

// ============================================================================
// [Fog::Path - SubPath]
// ============================================================================

sysuint_t Path::getSubPathLength(sysuint_t subPathId) const
{
  const Path::Data* d = _d;

  sysuint_t length = d->length;
  if (subPathId >= length) return 0;

  const PathVertex* cur = d->data + subPathId + 1;;
  const PathVertex* end = d->data + length;

  while (cur < end)
  {
    if (cur->cmd.isMoveTo()) break;
  }

  return (sysuint_t)(cur - d->data);
}

// ============================================================================
// [Fog::Path - Start / End]
// ============================================================================

err_t Path::start(sysuint_t* index)
{
  if (_d->length && !_d->data[_d->length-1].cmd.isStop())
  {
    PathVertex* v = _add(1);
    if (!v) return ERR_RT_OUT_OF_MEMORY;

    v->cmd = PATH_CMD_STOP;
    v->x = 0.0;
    v->y = 0.0;
  }

  if (index) *index = _d->length;
  return ERR_OK;
}

err_t Path::endPoly(uint32_t cmdflags)
{
  if (_d->length && _d->data[_d->length-1].cmd.isVertex())
  {
    PathVertex* v = _add(1);
    if (!v) return ERR_RT_OUT_OF_MEMORY;

    v->cmd = cmdflags | PATH_CMD_END;
    v->x = 0.0;
    v->y = 0.0;
  }

  return ERR_OK;
}

err_t Path::closePolygon(uint32_t cmdflags)
{
  return endPoly(cmdflags | PATH_CFLAG_CLOSE);
}

// ============================================================================
// [Fog::Path - MoveTo]
// ============================================================================

err_t Path::moveTo(double x, double y)
{
  PathVertex* v = _add(1);
  if (!v) return ERR_RT_OUT_OF_MEMORY;

  v->cmd = PATH_CMD_MOVE_TO;
  v->x = x;
  v->y = y;

  return ERR_OK;
}

err_t Path::moveRel(double dx, double dy)
{
  relToAbsInline(_d, &dx, &dy);
  return moveTo(dx, dy);
}

// ============================================================================
// [Fog::Path - LineTo]
// ============================================================================

err_t Path::lineTo(double x, double y)
{
  PathVertex* v = _add(1);
  if (!v) return ERR_RT_OUT_OF_MEMORY;

  v->cmd = PATH_CMD_LINE_TO;
  v->x = x;
  v->y = y;

  return ERR_OK;
}

err_t Path::lineRel(double dx, double dy)
{
  relToAbsInline(_d, &dx, &dy);
  return lineTo(dx, dy);
}

err_t Path::lineTo(const double* x, const double* y, sysuint_t count)
{
  PathVertex* v = _add(count);
  if (!v) return ERR_RT_OUT_OF_MEMORY;

  for (sysuint_t i = 0; i < count; i++)
  {
    v[i].cmd = PATH_CMD_LINE_TO;
    v[i].x = x[i];
    v[i].y = y[i];
  }

  return ERR_OK;
}

err_t Path::lineTo(const PointD* pts, sysuint_t count)
{
  PathVertex* v = _add(count);
  if (!v) return ERR_RT_OUT_OF_MEMORY;

  for (sysuint_t i = 0; i < count; i++)
  {
    v[i].cmd = PATH_CMD_LINE_TO;
    v[i].x = pts[i].x;
    v[i].y = pts[i].y;
  }

  return ERR_OK;
}

err_t Path::hlineTo(double x)
{
  return lineTo(x, lastY(_d));
}

err_t Path::hlineRel(double dx)
{
  double dy = 0.0;
  relToAbsInline(_d, &dx, &dy);
  return lineTo(dx, dy);
}

err_t Path::vlineTo(double y)
{
  return lineTo(lastX(_d), y);
}

err_t Path::vlineRel(double dy)
{
  double dx = 0.0;
  relToAbsInline(_d, &dx, &dy);
  return lineTo(dx, dy);
}

// ============================================================================
// [Fog::Path - ArcTo]
// ============================================================================

static void arcToBezier(
  double cx, double cy,
  double rx, double ry,
  double start,
  double sweep,
  PathVertex* dst)
{
  sweep *= 0.5;

  double x0 = cos(sweep);
  double y0 = sin(sweep);
  double tx = (1.0 - x0) * (4.0 / 3.0);
  double ty = y0 - tx * x0 / y0;
  double px[4];
  double py[4];

  double sn = sin(start + sweep);
  double cs = cos(start + sweep);

  px[0] =  x0;
  py[0] = -y0;
  px[1] =  x0 + tx;
  py[1] = -ty;
  px[2] =  x0 + tx;
  py[2] =  ty;
  px[3] =  x0;
  py[3] =  y0;

  for (sysuint_t i = 0; i < 4; i++)
  {
    dst[i].cmd = PATH_CMD_CURVE_4;
    dst[i].x = cx + rx * (px[i] * cs - py[i] * sn);
    dst[i].y = cy + ry * (px[i] * sn + py[i] * cs);
  }
}

err_t Path::_arcTo(double cx, double cy, double rx, double ry, double start, double sweep, uint initialCommand, bool closePath)
{
  start = fmod(start, 2.0 * M_PI);

  if (sweep >=  2.0 * M_PI) sweep =  2.0 * M_PI;
  if (sweep <= -2.0 * M_PI) sweep = -2.0 * M_PI;

  // Degenerated.
  if (fabs(sweep) < 1e-10)
  {
    PathVertex* v = _add(2);
    if (!v) return ERR_RT_OUT_OF_MEMORY;

    v[0].cmd = initialCommand;
    v[0].x = cx + rx * cos(start);
    v[0].y = cy + ry * sin(start);
    v[1].cmd = PATH_CMD_LINE_TO;
    v[1].x = cx + rx * cos(start + sweep);
    v[1].y = cy + ry * sin(start + sweep);
  }
  else
  {
    PathVertex* v = _add(13);
    if (!v) return ERR_RT_OUT_OF_MEMORY;

    PathVertex* vstart = v;
    PathVertex* vend = v + 13;

    double totalSweep = 0.0;
    double localSweep = 0.0;
    double prevSweep;
    bool done = false;

    v++;

    do {
      if (sweep < 0.0)
      {
        prevSweep   = totalSweep;
        localSweep  = -M_PI * 0.5;
        totalSweep -=  M_PI * 0.5;

        if (totalSweep <= sweep + PathUtil::BEZIER_ARC_ANGLE_EPSILON)
        {
          localSweep = sweep - prevSweep;
          done = true;
        }
      }
      else
      {
        prevSweep   = totalSweep;
        localSweep  = M_PI * 0.5;
        totalSweep += M_PI * 0.5;

        if (totalSweep >= sweep - PathUtil::BEZIER_ARC_ANGLE_EPSILON)
        {
          localSweep = sweep - prevSweep;
          done = true;
        }
      }

      arcToBezier(cx, cy, rx, ry, start, localSweep, v-1);
      v += 3;
      start += localSweep;
    } while (!done && v < vend);

    // Setup initial command, path length and set flat to false.
    vstart[0].cmd = initialCommand;
    _d->length = (sysuint_t)(v - _d->data);
    _d->flat = false;
  }

  if (closePath) closePolygon();
  return ERR_OK;
}

err_t Path::_svgArcTo(
  double rx, double ry,
  double angle,
  bool largeArcFlag,
  bool sweepFlag,
  double x2, double y2,
  uint initialCommand, bool closePath)
{
  // Mark current length (will be position where the first bezier would start).
  sysuint_t mark = getLength();

  bool radiiOk = true;
  double x0, y0;

  // Get initial (x0, y0).
  PathVertex* vertex = _d->data + mark - 1;
  if (!mark || !vertex->cmd.isVertex())
  {
    x0 = 0.0;
    y0 = 0.0;
  }
  else
  {
    x0 = vertex->x;
    y0 = vertex->y;
  }

  // Normalize radius.
  if (rx < 0.0) rx = -rx;
  if (ry < 0.0) ry = -ry;

  // Calculate the middle point between the current and the final points.
  double dx2 = (x0 - x2) / 2.0;
  double dy2 = (y0 - y2) / 2.0;

  double cos_a = cos(angle);
  double sin_a = sin(angle);

  // Calculate (x1, y1).
  double x1 =  cos_a * dx2 + sin_a * dy2;
  double y1 = -sin_a * dx2 + cos_a * dy2;

  // Ensure radii are large enough.
  double prx = rx * rx;
  double pry = ry * ry;
  double px1 = x1 * x1;
  double py1 = y1 * y1;

  // Check that radii are large enough.
  double radiiCheck = px1 / prx + py1 / pry;

  if (radiiCheck > 1.0)
  {
    rx = sqrt(radiiCheck) * rx;
    ry = sqrt(radiiCheck) * ry;
    prx = rx * rx;
    pry = ry * ry;
    if (radiiCheck > 10.0) radiiOk = false;
  }

  // Calculate (cx1, cy1).
  double sign = (largeArcFlag == sweepFlag) ? -1.0 : 1.0;
  double sq   = (prx*pry - prx*py1 - pry*px1) / (prx*py1 + pry*px1);
  double coef = sign * sqrt((sq < 0) ? 0 : sq);
  double cx1  = coef *  ((rx * y1) / ry);
  double cy1  = coef * -((ry * x1) / rx);

  // Calculate (cx, cy) from (cx1, cy1).
  double sx2 = (x0 + x2) / 2.0;
  double sy2 = (y0 + y2) / 2.0;
  double cx = sx2 + (cos_a * cx1 - sin_a * cy1);
  double cy = sy2 + (sin_a * cx1 + cos_a * cy1);

  // Calculate the start_angle (angle1) and the sweep_angle (dangle).
  double ux =  (x1 - cx1) / rx;
  double uy =  (y1 - cy1) / ry;
  double vx = (-x1 - cx1) / rx;
  double vy = (-y1 - cy1) / ry;
  double p, n;

  // Calculate the angle start.
  n = sqrt(ux*ux + uy*uy);
  p = ux; // (1 * ux) + (0 * uy)
  sign = (uy < 0) ? -1.0 : 1.0;
  double v = p / n;
  if (v < -1.0) v = -1.0;
  if (v >  1.0) v =  1.0;
  double start_angle = sign * acos(v);

  // Calculate the sweep angle.
  n = sqrt((ux*ux + uy*uy) * (vx*vx + vy*vy));
  p = ux * vx + uy * vy;
  sign = (ux * vy - uy * vx < 0) ? -1.0 : 1.0;
  v = p / n;
  if (v < -1.0) v = -1.0;
  if (v >  1.0) v =  1.0;
  double sweep_angle = sign * acos(v);

  if (!sweepFlag && sweep_angle > 0)
    sweep_angle -= M_PI * 2.0;
  else if (sweepFlag && sweep_angle < 0)
    sweep_angle += M_PI * 2.0;

  // We can now build and transform the resulting arc.
  Matrix matrix = Matrix::fromRotation(angle);
  matrix.translate(cx, cy, MATRIX_APPEND);

  err_t err = _arcTo(0.0, 0.0, rx, ry, start_angle, sweep_angle, initialCommand, false);
  if (err) return err;

  // This can't fail. Path must be already detached.
  err = applyMatrix(matrix, mark, DETECT_LENGTH);
  FOG_ASSERT(err == ERR_OK);

  // We must make sure that the starting and ending points exactly coincide
  // with the initial (x0, y0) and (x2, y2).
  //
  // This comment is from AntiGrain, we actually need only to fix end point.
  vertex = _d->data + getLength() - 1;
  vertex->x = x2;
  vertex->y = y2;

  if (closePath) closePolygon();
  return ERR_OK;
}

err_t Path::arcTo(double cx, double cy, double rx, double ry, double start, double sweep)
{
  return _arcTo(cx, cy, rx, ry, start, sweep, PATH_CMD_LINE_TO, false);
}

err_t Path::arcRel(double cx, double cy, double rx, double ry, double start, double sweep)
{
  relToAbsInline(_d, &cx, &cy);
  return _arcTo(cx, cy, rx, ry, start, sweep, PATH_CMD_LINE_TO, false);
}

err_t Path::svgArcTo(
  double rx, double ry,
  double angle,
  bool largeArcFlag,
  bool sweepFlag,
  double x2, double y2)
{
  return _svgArcTo(rx, ry, angle, largeArcFlag, sweepFlag, x2, y2, PATH_CMD_LINE_TO, false);
}

err_t Path::svgArcRel(
  double rx, double ry,
  double angle,
  bool largeArcFlag,
  bool sweepFlag,
  double x2, double y2)
{
  relToAbsInline(_d, &x2, &y2);
  return _svgArcTo(rx, ry, angle, largeArcFlag, sweepFlag, x2, y2, PATH_CMD_LINE_TO, false);
}

// ============================================================================
// [Fog::Path - BezierTo]
// ============================================================================

err_t Path::curveTo(double cx, double cy, double tx, double ty)
{
  PathVertex* v = _add(2);
  if (!v) return ERR_RT_OUT_OF_MEMORY;

  v[0].cmd = PATH_CMD_CURVE_3;
  v[0].x = cx;
  v[0].y = cy;
  v[1].cmd = PATH_CMD_CURVE_3;
  v[1].x = tx;
  v[1].y = ty;

  _d->flat = false;
  return ERR_OK;
}

err_t Path::curveRel(double cx, double cy, double tx, double ty)
{
  relToAbsInline(_d, &cx, &cy, &tx, &ty);
  return curveTo(cx, cy, tx, ty);
}

err_t Path::curveTo(double tx, double ty)
{
  PathVertex* endv = _d->data + _d->length;

  if (_d->length && endv[-1].cmd.isVertex())
  {
    double cx = endv[-1].x;
    double cy = endv[-1].y;

    if (_d->length >= 2 && endv[-2].cmd.isCurve())
    {
      cx += endv[-1].x - endv[-2].x;
      cy += endv[-1].y - endv[-2].y;
    }

    return curveTo(cx, cy, tx, ty);
  }
  else
  {
    return ERR_OK;
  }
}

err_t Path::curveRel(double tx, double ty)
{
  relToAbsInline(_d, &tx, &ty);
  return curveTo(tx, ty);
}

// ============================================================================
// [Fog::Path - CubicTo]
// ============================================================================

err_t Path::cubicTo(double cx1, double cy1, double cx2, double cy2, double tx, double ty)
{
  PathVertex* v = _add(3);
  if (!v) return ERR_RT_OUT_OF_MEMORY;

  v[0].cmd = PATH_CMD_CURVE_4;
  v[0].x = cx1;
  v[0].y = cy1;
  v[1].cmd = PATH_CMD_CURVE_4;
  v[1].x = cx2;
  v[1].y = cy2;
  v[2].cmd = PATH_CMD_CURVE_4;
  v[2].x = tx;
  v[2].y = ty;

  _d->flat = false;
  return ERR_OK;
}

err_t Path::cubicRel(double cx1, double cy1, double cx2, double cy2, double tx, double ty)
{
  relToAbsInline(_d, &cx1, &cy1, &cx2, &cy2, &tx, &ty);
  return cubicTo(cx1, cy1, cx2, cy2, tx, ty);
}

err_t Path::cubicTo(double cx2, double cy2, double tx, double ty)
{
  PathVertex* endv = _d->data + _d->length;

  if (_d->length && endv[-1].cmd.isVertex())
  {
    double cx1 = endv[-1].x;
    double cy1 = endv[-1].y;

    if (_d->length >= 2 && endv[-2].cmd.isCurve())
    {
      cx1 += endv[-1].x - endv[-2].x;
      cy1 += endv[-1].y - endv[-2].y;
    }

    return cubicTo(cx1, cy1, cx2, cy2, tx, ty);
  }
  else
  {
    return ERR_OK;
  }
}

err_t Path::cubicRel(double cx2, double cy2, double tx, double ty)
{
  relToAbsInline(_d, &cx2, &cy2, &tx, &ty);
  return cubicTo(cx2, cy2, tx, ty);
}

// ============================================================================
// [Fog::Path - FlipX / FlipY]
// ============================================================================

err_t Path::flipX(double x1, double x2)
{
  if (!_d->length) return ERR_OK;

  err_t err = detach();
  if (err) return err;

  sysuint_t i, len = _d->length;
  PathVertex* v = _d->data;
  
  for (i = 0; i < len; i++)
  {
    if (v[i].cmd.isVertex()) v[i].x = x2 - v[i].x + x1;
  }

  return ERR_OK;
}

err_t Path::flipY(double y1, double y2)
{
  if (!_d->length) return ERR_OK;

  err_t err = detach();
  if (err) return err;

  sysuint_t i, len = _d->length;
  PathVertex* v = _d->data;
  
  for (i = 0; i < len; i++)
  {
    if (v[i].cmd.isVertex()) v[i].y = y2 - v[i].y + y1;
  }

  return ERR_OK;
}

// ============================================================================
// [Fog::Path - Translate]
// ============================================================================

err_t Path::translate(double dx, double dy)
{
  if (!_d->length) return ERR_OK;

  err_t err = detach();
  if (err) return err;
  
  sysuint_t i, len = _d->length;
  PathVertex* v = _d->data;

  for (i = 0; i < len; i++)
  {
    if (v[i].cmd.isVertex())
    {
      v[i].x += dx;
      v[i].y += dy;
    }
  }

  return ERR_OK;
}

err_t Path::translate(double dx, double dy, sysuint_t pathId)
{
  if (!_d->length) return ERR_OK;

  err_t err = detach();
  if (err) return err;

  sysuint_t i, len = _d->length;
  PathVertex* v = _d->data;

  for (i = pathId; i < len; i++)
  {
    if (v[i].cmd.isStop()) break;
    if (v[i].cmd.isVertex())
    {
      v[i].x += dx;
      v[i].y += dy;
    }
  }

  return ERR_OK;
}

// ============================================================================
// [Fog::Path - Scale]
// ============================================================================

err_t Path::scale(double sx, double sy, bool keepStartPos)
{
  if (!_d->length) return ERR_OK;

  err_t err = detach();
  if (err) return err;

  sysuint_t i, len = _d->length;
  PathVertex* v = _d->data;

  if (keepStartPos)
  {
    double px = v[0].x;
    double py = v[0].y;

    for (i = 1; i < len; i++)
    {
      if (v[i].cmd.isVertex())
      {
        if (v[i].x < px) px = v[i].x;
        if (v[i].y < py) py = v[i].y;
      }
    }

    for (i = 0; i < len; i++)
    {
      if (v[i].cmd.isVertex())
      {
        v[i].x = (v[i].x - px) * sx + px;
        v[i].y = (v[i].y - py) * sy + py;
      }
    }
  }
  else
  {
    for (i = 0; i < len; i++)
    {
      if (v[i].cmd.isVertex())
      {
        v[i].x *= sx;
        v[i].y *= sy;
      }
    }
  }

  return ERR_OK;
}

// ============================================================================
// [Fog::Path - ApplyMatrix]
// ============================================================================

err_t Path::applyMatrix(const Matrix& matrix)
{
  if (!_d->length) return ERR_OK;

  err_t err = detach();
  if (err) return err;

  PathUtil::fm.transformVertex(_d->data, _d->length, &matrix);
  return ERR_OK;
}

err_t Path::applyMatrix(const Matrix& matrix, sysuint_t index, sysuint_t length)
{
  if (index >= _d->length) return ERR_OK;

  err_t err = detach();
  if (err) return err;

  PathUtil::fm.transformVertex(_d->data + index, Math::min(length, _d->length - index), &matrix);
  return ERR_OK;
}

// ============================================================================
// [Fog::Path - Add]
// ============================================================================

err_t Path::addRect(const RectD& r)
{
  if (!r.isValid()) return ERR_OK;

  PathVertex* v = _add(5);
  if (!v) return ERR_RT_OUT_OF_MEMORY;

  v[0].cmd = PATH_CMD_MOVE_TO;
  v[0].x = r.getX1();
  v[0].y = r.getY1();
  v[1].cmd = PATH_CMD_LINE_TO;
  v[1].x = r.getX2();
  v[1].y = r.getY1();
  v[2].cmd = PATH_CMD_LINE_TO;
  v[2].x = r.getX2();
  v[2].y = r.getY2();
  v[3].cmd = PATH_CMD_LINE_TO;
  v[3].x = r.getX1();
  v[3].y = r.getY2();
  v[4].cmd = PATH_CMD_END | PATH_CFLAG_CLOSE;
  v[4].x = 0.0;
  v[4].y = 0.0;

  return ERR_OK;
}

err_t Path::addRects(const RectD* r, sysuint_t count)
{
  if (!count) return ERR_OK;
  FOG_ASSERT(r);

  PathVertex* v = _add(count * 5);
  if (!v) return ERR_RT_OUT_OF_MEMORY;

  for (sysuint_t i = 0; i < count; i++, r++)
  {
    if (!r->isValid()) continue;

    v[0].cmd = PATH_CMD_MOVE_TO;
    v[0].x = r->getX1();
    v[0].y = r->getY1();
    v[1].cmd = PATH_CMD_LINE_TO;
    v[1].x = r->getX2();
    v[1].y = r->getY1();
    v[2].cmd = PATH_CMD_LINE_TO;
    v[2].x = r->getX2();
    v[2].y = r->getY2();
    v[3].cmd = PATH_CMD_LINE_TO;
    v[3].x = r->getX1();
    v[3].y = r->getY2();
    v[4].cmd = PATH_CMD_END | PATH_CFLAG_CLOSE;
    v[4].x = 0.0;
    v[4].y = 0.0;

    v += 5;
  }

  // Return and update path length.
  _d->length = (sysuint_t)(v - _d->data);
  return ERR_OK;
}

err_t Path::addRound(const RectD& r, const PointD& radius)
{
  if (!r.isValid()) return ERR_OK;

  double rw2 = r.getWidth() / 2.0;
  double rh2 = r.getHeight() / 2.0;

  double rx = fabs(radius.x);
  double ry = fabs(radius.y);

  if (rx > rw2) rx = rw2;
  if (ry > rh2) ry = rh2;

  if (rx == 0 || ry == 0)
    return addRect(r);

  double x1 = r.getX1();
  double y1 = r.getY1();
  double x2 = r.getX2();
  double y2 = r.getY2();

  err_t err = ERR_OK;

  err |= moveTo(x1 + rx, y1);
  err |= lineTo(x2 - rx, y1);
  err |= arcTo(x2 - rx, y1 + ry, rx, ry, M_PI * 1.5, M_PI * 0.5);

  err |= lineTo(x2, y2 - ry);
  err |= arcTo(x2 - rx, y2 - ry, rx, ry, M_PI * 0.0, M_PI * 0.5);

  err |= lineTo(x1 + rx, y2);
  err |= arcTo(x1 + rx, y2 - ry, rx, ry, M_PI * 0.5, M_PI * 0.5);

  err |= lineTo(x1, y1 + ry);
  err |= arcTo(x1 + rx, y1 + ry, rx, ry, M_PI * 1.0, M_PI * 0.5);

  err |= closePolygon();

  return err;
}

err_t Path::addEllipse(const RectD& r)
{
  if (!r.isValid()) return ERR_OK;

  double rx = r.getWidth() / 2.0;
  double ry = r.getHeight() / 2.0;
  double cx = r.getX() + rx;
  double cy = r.getY() + ry;

  return _arcTo(cx, cy, rx, ry, 0.0, 2.0 * M_PI, PATH_CMD_MOVE_TO, true);
}

err_t Path::addEllipse(const PointD& cp, const PointD& r)
{
  return _arcTo(cp.getX(), cp.getY(), r.getX(), r.getY(), 0.0, 2.0 * M_PI, PATH_CMD_MOVE_TO, true);
}

err_t Path::addArc(const RectD& r, double start, double sweep)
{
  if (!r.isValid()) return ERR_OK;

  double rx = r.getWidth() / 2.0;
  double ry = r.getHeight() / 2.0;
  double cx = r.getX() + rx;
  double cy = r.getY() + ry;

  return _arcTo(cx, cy, rx, ry, start, sweep, PATH_CMD_MOVE_TO, false);
}

err_t Path::addArc(const PointD& cp, const PointD& r, double start, double sweep)
{
  return _arcTo(cp.getX(), cp.getY(), r.getX(), r.getY(), start, sweep, PATH_CMD_MOVE_TO, false);
}

err_t Path::addChord(const RectD& r, double start, double sweep)
{
  if (!r.isValid()) return ERR_OK;

  double rx = r.getWidth() / 2.0;
  double ry = r.getHeight() / 2.0;
  double cx = r.getX() + rx;
  double cy = r.getY() + ry;

  return _arcTo(cx, cy, rx, ry, start, sweep, PATH_CMD_MOVE_TO, true);
}

err_t Path::addChord(const PointD& cp, const PointD& r, double start, double sweep)
{
  return _arcTo(cp.getX(), cp.getY(), r.getX(), r.getY(), start, sweep, PATH_CMD_MOVE_TO, true);
}

err_t Path::addPie(const RectD& r, double start, double sweep)
{
  if (!r.isValid()) return ERR_OK;

  double rx = r.getWidth() / 2.0;
  double ry = r.getHeight() / 2.0;
  double cx = r.getX() + rx;
  double cy = r.getY() + ry;

  return addPie(PointD(cx, cy), PointD(rx, ry), start, sweep);
}

err_t Path::addPie(const PointD& cp, const PointD& r, double start, double sweep)
{
  if (sweep >= M_PI*2.0) return addEllipse(cp, r);

  start = fmod(start, M_PI * 2.0);
  if (start < 0) start += M_PI * 2.0;

  err_t err;

  if ( (err = moveTo(cp.getX(), cp.getY())) ) return err;
  if ( (err = _arcTo(cp.getX(), cp.getY(), r.getX(), r.getY(), start, sweep, PATH_CMD_LINE_TO, true)) ) return err;

  return ERR_OK;
}

err_t Path::addPath(const Path& path)
{
  sysuint_t count = path.getLength();
  if (count == 0) return ERR_OK;

  uint32_t flat = isFlat() & path.isFlat();

  PathVertex* dst = _add(count);
  if (!dst) return ERR_RT_OUT_OF_MEMORY;

  _d->flat = flat;

  const PathVertex* src = path.getData();
  Memory::copy(dst, src, count * sizeof(PathVertex));

  return ERR_OK;
}

err_t Path::addPath(const Path& path, const Matrix& matrix)
{
  if (this == &path)
  {
    Path other(path);
    return addPath(other);
  }

  sysuint_t count = path.getLength();
  if (count == 0) return ERR_OK;

  uint32_t flat = isFlat() & path.isFlat();

  PathVertex* dst = _add(count);
  if (!dst) return ERR_RT_OUT_OF_MEMORY;

  _d->flat = flat;

  const PathVertex* src = path.getData();
  PathUtil::fm.transformVertex2(dst, src, count, &matrix);

  return ERR_OK;
}

// ============================================================================
// [Fog::Path - Flatten]
// ============================================================================

bool Path::isFlat() const
{
  int32_t flat = _d->flat;
  if (flat != -1) return (bool)flat;

  // Detection
  const PathVertex* v = _d->data;
  sysuint_t len = _d->length;

  flat = true;

  for (sysuint_t i = 0; i < len; i++, v++)
  {
    if (v->cmd.cmd() > PATH_CMD_LINE_TO && v->cmd.cmd() < PATH_CMD_MASK) { flat = false; break; }
  }

  _d->flat = flat;
  return (bool)flat;
}

err_t Path::flatten()
{
  return flattenTo(*this, NULL, 1.0);
}

err_t Path::flatten(const Matrix* matrix, double approximationScale)
{
  return flattenTo(*this, NULL, 1.0);
}

static err_t _flattenData(Path& dst, const PathVertex* data, sysuint_t count, const Matrix* matrix, double approximationScale)
{
  dst.clear();
  if (count == 0) return ERR_OK;

  err_t err = ERR_OK;
  sysuint_t grow = count * 4;

  if (grow < count) return ERR_RT_OUT_OF_MEMORY;
  if (dst.reserve(grow)) return ERR_RT_OUT_OF_MEMORY;

  PathVertex* dstv;

  double lastx = 0.0;
  double lasty = 0.0;

  sysuint_t oldLen;
  uint32_t oldCmd;

ensureSpace:
  dstv = dst._add(count);
  if (!dstv) return ERR_RT_OUT_OF_MEMORY;

  do {
    switch (data->cmd.cmd())
    {
      case PATH_CMD_MOVE_TO:
      case PATH_CMD_LINE_TO:
        dstv->x = lastx = data->x;
        dstv->y = lasty = data->y;
        dstv->cmd = data->cmd;

        data++;
        dstv++;
        count--;

        break;

      case PATH_CMD_CURVE_3:
        if (count <= 1) goto invalid;
        if (data[1].cmd.cmd() != PATH_CMD_CURVE_3) goto invalid;

        // Finalize path
        dst._d->length = (sysuint_t)(dstv - dst._d->data);

        // If there was lineTo command, we eat it, because approximateCurve3
        // will add it here again.
        oldLen = INVALID_INDEX;
        if (dst._d->length > 0)
        {
          oldLen = --dst._d->length;
          const PathVertex* lastVertex = &dst._d->data[oldLen];

          if (lastVertex->x == lastx && lastVertex->y == lasty)
            oldCmd = lastVertex->cmd;
        }

        // Approximate curve.
        err = PathUtil::fm.approximateCurve3(dst, lastx, lasty, data[0].x, data[0].y, data[1].x, data[1].y, approximationScale, 0.0);
        if (err) goto end;

        // Part of fix described above.
        if (oldLen != INVALID_INDEX) dst._d->data[oldLen].cmd = oldCmd;

        lastx = data[1].x;
        lasty = data[1].y;

        data += 2;
        count -= 2;

        if (count == 0)
          goto end;
        else
          goto ensureSpace;

      case PATH_CMD_CURVE_4:
        if (count <= 2) goto invalid;
        if (data[1].cmd.cmd() != PATH_CMD_CURVE_4 ||
            data[2].cmd.cmd() != PATH_CMD_CURVE_4) goto invalid;

        // Finalize path
        dst._d->length = (sysuint_t)(dstv - dst._d->data);

        // If there was lineTo command, we eat it, because approximateCurve3
        // will add it here again.
        oldLen = INVALID_INDEX;
        if (dst._d->length > 0)
        {
          oldLen = --dst._d->length;
          const PathVertex* lastVertex = &dst._d->data[oldLen];

          if (lastVertex->x == lastx && lastVertex->y == lasty)
            oldCmd = lastVertex->cmd;
        }

        // Approximate curve.
        err = PathUtil::fm.approximateCurve4(dst, lastx, lasty, data[0].x, data[0].y, data[1].x, data[1].y, data[2].x, data[2].y, approximationScale, 0.0, 0.0);
        if (err) goto end;

        // Part of fix described above.
        if (oldLen != INVALID_INDEX) dst._d->data[oldLen].cmd = oldCmd;

        lastx = data[2].x;
        lasty = data[2].y;

        data += 3;
        count -= 3;

        if (count == 0)
          goto end;
        else
          goto ensureSpace;

      default:
        dstv->x = lastx = 0.0;
        dstv->y = lasty = 0.0;
        dstv->cmd = data->cmd;

        data++;
        dstv++;
        count--;

        break;
    }
  } while(count);

  dst._d->length = (sysuint_t)(dstv - dst._d->data);
end:
  dst._d->flat = true;
  if (matrix) dst.applyMatrix(*matrix);
  return err;

invalid:
  dst._d->length = 0;
  dst._d->flat = true;
  return ERR_PATH_INVALID;
}

err_t Path::flattenTo(Path& dst, const Matrix* matrix, double approximationScale) const
{
  // --------------------------------------------------------------------------
  // Contains only lines (flat path).
  // --------------------------------------------------------------------------

  if (isFlat())
  {
    if (this == &dst)
    {
      if (!matrix) return ERR_OK;
      return dst.applyMatrix(*matrix);
    }
    else
    {
      if (!matrix) return dst.set(*this);

      dst.clear();
      return dst.addPath(*this, *matrix);
    }
  }

  // --------------------------------------------------------------------------
  // Contains curves.
  // --------------------------------------------------------------------------

  // The dst argument is here mainly if we need to flatten path into different
  // one. If paths are equal, we will simply create second path and assign
  // result to first one.
  if (this == &dst)
  {
    Path tmp(*this);
    return _flattenData(dst, tmp.getData(), tmp.getLength(), matrix, approximationScale);
  }
  else
  {
    return _flattenData(dst, getData(), getLength(), matrix, approximationScale);
  }
}

err_t Path::flattenSubPathTo(Path& dst, sysuint_t subPathId, const Matrix* matrix, double approximationScale) const
{
  sysuint_t len = getSubPathLength(subPathId);

  // --------------------------------------------------------------------------
  // No sub-path.
  // --------------------------------------------------------------------------

  if (!len)
  {
    if (this != &dst) dst.clear();
    return ERR_OK;
  }

  // --------------------------------------------------------------------------
  // Contains only lines (flat path).
  // --------------------------------------------------------------------------

  if (isFlat())
  {
    if (this == &dst)
    {
      if (!matrix) return ERR_OK;
      return dst.applyMatrix(*matrix);
    }
    else
    {
      if (!matrix) return dst.set(*this);

      dst.clear();
      return dst.addPath(*this, *matrix);
    }
  }

  // --------------------------------------------------------------------------
  // Contains curves.
  // --------------------------------------------------------------------------

  // The dst argument is here mainly if we need to flatten path into different
  // one. If paths are equal, we will simply create second path and assign
  // result to first one.
  if (this == &dst)
  {
    Path tmp(*this);
    return _flattenData(dst, tmp._d->data + subPathId, len, matrix, approximationScale);
  }
  else
  {
    return _flattenData(dst, _d->data + subPathId, len, matrix, approximationScale);
  }
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_path_init(void)
{
  using namespace Fog;

  Path::Data* d = Path::sharedNull.instancep();

  d->refCount.init(1);
  d->flags |= Path::Data::IsSharable;
  d->flat = true;
  d->capacity = 0;
  d->length = 0;

  d->data[0].cmd._cmd = PATH_CMD_STOP;
  d->data[0].x = 0.0;
  d->data[0].y = 0.0;

  return ERR_OK;
}

FOG_INIT_DECLARE void fog_path_shutdown(void)
{
  using namespace Fog;

  Path::sharedNull->refCount.dec();
}
