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
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/Assert.h>
#include <Fog/Core/Error.h>
#include <Fog/Core/Math.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/Std.h>
#include <Fog/Graphics/Matrix.h>
#include <Fog/Graphics/Error.h>
#include <Fog/Graphics/Path.h>
#include <Fog/Graphics/Path_p.h>
#include <Fog/Graphics/Raster.h>

// [Antigrain]
#include "agg_basics.h"
#include "agg_conv_dash.h"
#include "agg_conv_stroke.h"
#include "agg_vcgen_dash.h"

namespace Fog {

// ============================================================================
// [AntiGrain Wrappers]
// ============================================================================

// Wraps Fog::Path to antigrain like vertex storage.
struct FOG_HIDDEN AggPath
{
  FOG_INLINE AggPath(const Path& path)
  {
    d = path._d;
    rewind(0);
  }

  FOG_INLINE ~AggPath()
  {
  }

  FOG_INLINE void rewind(unsigned index)
  {
    vCur = d->data + index;
    vEnd = d->data + d->length;
  }

  FOG_INLINE unsigned vertex(double* x, double* y)
  {
    if (vCur == vEnd) return Path::CmdStop;

    *x = vCur->x;
    *y = vCur->y;

    uint command = vCur->cmd.cmd();
    vCur++;
    return command;
  }

private:
  const Path::Data* d;
  const Path::Vertex* vCur;
  const Path::Vertex* vEnd;
};

template<typename VertexStorage>
static err_t concatToPath(Path& dst, VertexStorage& src, unsigned path_id = 0)
{
  sysuint_t i, len = dst.getLength();
  sysuint_t step = 1024;

  Path::Vertex* v;
  err_t err;

  src.rewind(path_id);

  for (;;)
  {
    if ( (err = dst.reserve(len + step)) ) return err;
    v = dst._d->data + len;

    // Concat vertexes.
    for (i = step; i; i--, v++)
    {
      if ((v->cmd._cmd = src.vertex(&v->x, &v->y)) == Path::CmdStop)
        goto done;
    }

    // If we are here it's needed to alloc more memory (really big path).
    len += step;
    dst._d->length = len;

    // Double step until we reach 1MB.
    if (step < 1024*1024) step <<= 1;
  }

done:
  dst._d->length = (sysuint_t)(v - dst._d->data);
  return Error::Ok;
}

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
  d->type = type;
  memcpy(d->data, data, sizeof(Path::Vertex) * length);

  return d;
}

Path::Data* Path::Data::alloc(sysuint_t capacity)
{
  sysuint_t dsize = 
    sizeof(Data) - sizeof(Vertex) + capacity * sizeof(Vertex);

  Data* d = reinterpret_cast<Data*>(Memory::alloc(dsize));
  if (!d) return NULL;

  d->refCount.init(1);
  d->flags = IsDynamic | IsSharable;
  d->type = Path::LineType;
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
      sizeof(Data) - sizeof(Vertex) + capacity * sizeof(Vertex);

    Data* newd = reinterpret_cast<Data*>(Memory::realloc((void*)d, dsize));
    if (!newd) return NULL;

    newd->capacity = capacity;
    return newd;
  }
  else
  {
    Data* newd = alloc(capacity);

    newd->length = d->length;
    newd->type = d->type;
    memcpy(newd->data, d->data, d->length * sizeof(Vertex));
    d->deref();
    return newd;
  }
}

// ============================================================================
// [Fog::Path]
// ============================================================================

static FOG_INLINE Path::Cmd lastCmd(Path::Data* d)
{
  return Path::Cmd(d->length
    ? Path::Cmd(d->data[d->length-1].cmd)
    : Path::Cmd(Path::CmdEndPoly));
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
  Path::Vertex* v = d->data + d->length;

  if (d->length && v[-1].cmd.isVertex())
  {
    *x += v[-1].x;
    *y += v[-1].y;
  }
}

static FOG_INLINE void relToAbsInline(Path::Data* d, double* x0, double* y0, double* x1, double* y1)
{
  Path::Vertex* v = d->data + d->length;

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
  Path::Vertex* v = d->data + d->length;

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
  Path::Vertex* v = self->_add(len);
  if (!v) return Error::OutOfMemory;

  a.rewind(0);
  for (i = 0; i < len; i++)
  {
    v[i].cmd = a.vertex(&v[i].x, &v[i].y);
  }

  return Error::Ok;
}

// ============================================================================
// [Fog::Path - Construction / Destruction]
// ============================================================================

Path::Path()
{
  _d = sharedNull->refAlways();
}

Path::Path(Data* d)
{
  _d = d;
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
// [Fog::Path - Type]
// ============================================================================

uint32_t Path::getType() const
{
  uint32_t t = _d->type;
  if (t != 0) return t;

  // Detection
  const Vertex* v = _d->data;
  sysuint_t len = _d->length;

  t = LineType;
  for (sysuint_t i = 0; i < len; i++, v++)
  {
    if (v->cmd.cmd() > CmdLineTo && v->cmd.cmd() < CmdMask)
    {
      t = CurveType;
      break;
    }
  }

  _d->type = t;
  return t;
}

// ============================================================================
// [Fog::Path - Data]
// ============================================================================

err_t Path::reserve(sysuint_t capacity)
{
  if (_d->refCount.get() == 1 && _d->capacity >= capacity) return Error::Ok;

  Data* newd = Data::alloc(capacity);
  if (!newd) return Error::OutOfMemory;

  newd->length = _d->length;
  memcpy(newd->data, _d->data, _d->length * sizeof(Vertex));

  AtomicBase::ptr_setXchg(&_d, newd)->deref();
  return Error::Ok;
}

Path::Vertex* Path::_add(sysuint_t count)
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
      Std::calcOptimalCapacity(sizeof(Data), sizeof(Vertex), _d->length, _d->length + count);

    Data* newd = Data::alloc(optimalCapacity);
    if (!newd) return NULL;

    newd->length = length + count;
    newd->type = _d->type;
    memcpy(newd->data, _d->data, length * sizeof(Vertex));

    AtomicBase::ptr_setXchg(&_d, newd)->deref();
    return newd->data + length;
  }
}

err_t Path::_detach()
{
  if (isDetached()) return Error::Ok;

  Data* newd = _d->copy();
  if (!newd) return Error::OutOfMemory;

  AtomicBase::ptr_setXchg(&_d, newd)->deref();
  return Error::Ok;
}

err_t Path::set(const Path& other)
{
  Data* self_d = _d;
  Data* other_d = other._d;
  if (self_d == other_d) return Error::Ok;

  if ((self_d->flags & Data::IsStrong) != 0 || 
      (other_d->flags & Data::IsSharable) == 0)
  {
    return setDeep(other);
  }
  else
  {
    AtomicBase::ptr_setXchg(&_d, other_d->ref())->deref();
    return Error::Ok;
  }
}

err_t Path::setDeep(const Path& other)
{
  Data* self_d = _d;
  Data* other_d = other._d;

  if (self_d == other_d) return Error::Ok;
  if (other_d->length == 0) { clear(); return Error::Ok; }

  err_t err = reserve(other_d->length);
  if (err) { clear(); return Error::OutOfMemory; }

  self_d = _d;
  sysuint_t len = other_d->length;

  self_d->length = len;
  self_d->type = other_d->type;
  memcpy(self_d->data, other_d->data, len * sizeof(Vertex));

  return Error::Ok;
}

void Path::clear()
{
  if (_d->refCount.get() > 1)
  {
    AtomicBase::ptr_setXchg(&_d, sharedNull->refAlways())->deref();
  }
  else
  {
    _d->length = 0;
    _d->type = LineType;
  }
}

void Path::free()
{
  AtomicBase::ptr_setXchg(&_d, sharedNull->refAlways())->deref();
}

// ============================================================================
// [Fog::Path - Bounding Rect]
// ============================================================================

RectF Path::boundingRect() const
{
  sysuint_t i = _d->length;
  Vertex* v = _d->data;

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
      break;
    }

    i--;
    v++;
  }

  return RectF(x1, y1, x2 - x1, y2 - y1);
}

// ============================================================================
// [Fog::Path - Start / End]
// ============================================================================

err_t Path::start(sysuint_t* index)
{
  if (_d->length && !_d->data[_d->length-1].cmd.isStop())
  {
    Vertex* v = _add(1);
    if (!v) return Error::OutOfMemory;

    v->cmd = CmdStop;
    v->x = 0.0;
    v->y = 0.0;
  }

  if (index) *index = _d->length;
  return Error::Ok;
}

err_t Path::endPoly(uint32_t cmdflags)
{
  if (_d->length && _d->data[_d->length-1].cmd.isVertex())
  {
    Vertex* v = _add(1);
    if (!v) return Error::OutOfMemory;

    v->cmd = cmdflags | CmdEndPoly;
    v->x = 0.0;
    v->y = 0.0;
  }

  return Error::Ok;
}

err_t Path::closePolygon(uint32_t cmdflags)
{
  return endPoly(cmdflags | CFlagClose);
}

// ============================================================================
// [Fog::Path - MoveTo]
// ============================================================================

err_t Path::moveTo(double x, double y)
{
  Vertex* v = _add(1);
  if (!v) return Error::OutOfMemory;

  v->cmd = CmdMoveTo;
  v->x = x;
  v->y = y;

  return Error::Ok;
}

err_t Path::moveToRel(double dx, double dy)
{
  relToAbsInline(_d, &dx, &dy);
  return moveTo(dx, dy);
}

// ============================================================================
// [Fog::Path - LineTo]
// ============================================================================

err_t Path::lineTo(double x, double y)
{
  Vertex* v = _add(1);
  if (!v) return Error::OutOfMemory;

  v->cmd = CmdLineTo;
  v->x = x;
  v->y = y;

  return Error::Ok;
}

err_t Path::lineToRel(double dx, double dy)
{
  relToAbsInline(_d, &dx, &dy);
  return lineTo(dx, dy);
}

err_t Path::lineTo(const double* x, const double* y, sysuint_t count)
{
  Vertex* v = _add(count);
  if (!v) return Error::OutOfMemory;

  for (sysuint_t i = 0; i < count; i++)
  {
    v[i].cmd = CmdLineTo;
    v[i].x = x[i];
    v[i].y = y[i];
  }

  return Error::Ok;
}

err_t Path::lineTo(const PointF* pts, sysuint_t count)
{
  Vertex* v = _add(count);
  if (!v) return Error::OutOfMemory;

  for (sysuint_t i = 0; i < count; i++)
  {
    v[i].cmd = CmdLineTo;
    v[i].x = pts[i].x;
    v[i].y = pts[i].y;
  }

  return Error::Ok;
}

err_t Path::hlineTo(double x)
{
  return lineTo(x, lastY(_d));
}

err_t Path::hlineToRel(double dx)
{
  double dy = 0.0;
  relToAbsInline(_d, &dx, &dy);
  return lineTo(dx, dy);
}

err_t Path::vlineTo(double y)
{
  return lineTo(lastX(_d), y);
}

err_t Path::vlineToRel(double dy)
{
  double dx = 0.0;
  relToAbsInline(_d, &dx, &dy);
  return lineTo(dx, dy);
}

// ============================================================================
// [Fog::Path - ArcTo]
// ============================================================================

static void arc_to_bezier(
  double cx, double cy,
  double rx, double ry,
  double start,
  double sweep,
  Path::Vertex* dst)
{
  sweep /= 2.0;

  double x0 = cos(sweep);
  double y0 = sin(sweep);
  double tx = (1.0 - x0) * (4.0 / 3.0);
  double ty = y0 - tx * x0 / y0;
  double px[4];
  double py[4];

  px[0] =  x0;
  py[0] = -y0;
  px[1] =  x0 + tx;
  py[1] = -ty;
  px[2] =  x0 + tx;
  py[2] =  ty;
  px[3] =  x0;
  py[3] =  y0;

  double sn = sin(start + sweep);
  double cs = cos(start + sweep);

  for (sysuint_t i = 0; i < 4; i++)
  {
    dst[i].cmd = Path::CmdCurve4;
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
    Vertex* v = _add(2);
    if (!v) return Error::OutOfMemory;

    v[0].cmd = initialCommand;
    v[0].x = cx + rx * cos(start);
    v[0].y = cx + ry * sin(start);
    v[1].cmd = CmdLineTo;
    v[1].x = cx + rx * cos(start + sweep);
    v[1].y = cx + ry * sin(start + sweep);
  }
  else
  {
    Vertex* v = _add(13);
    if (!v) return Error::OutOfMemory;

    Vertex* vstart = v;
    Vertex* vend = v + 13;

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

        if (totalSweep <= sweep + bezierArcAngleEpsilon)
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

        if (totalSweep >= sweep - bezierArcAngleEpsilon)
        {
          localSweep = sweep - prevSweep;
          done = true;
        }
      }

      arc_to_bezier(cx, cy, rx, ry, start, localSweep, v-1);
      v += 3;
      start += localSweep;
    } while (!done && v < vend);

    // Setup initial command, path length and set type to CurveType.
    vstart[0].cmd = initialCommand;
    _d->length = (sysuint_t)(v - _d->data);
    _d->type = CurveType;
  }

  if (closePath) closePolygon();
  return Error::Ok;
}

err_t Path::arcTo(double cx, double cy, double rx, double ry, double start, double sweep)
{
  return _arcTo(cx, cy, rx, ry, start, sweep, CmdLineTo, false);
}

err_t Path::arcToRel(double cx, double cy, double rx, double ry, double start, double sweep)
{
  relToAbsInline(_d, &cx, &cy);
  return _arcTo(cx, cy, rx, ry, start, sweep, CmdLineTo, false);
}

// ============================================================================
// [Fog::Path - BezierTo]
// ============================================================================

err_t Path::curveTo(double cx, double cy, double tx, double ty)
{
  Vertex* v = _add(2);
  if (!v) return Error::OutOfMemory;

  v[0].cmd = CmdCurve3;
  v[0].x = cx;
  v[0].y = cy;
  v[1].cmd = CmdCurve3;
  v[1].x = tx;
  v[1].y = ty;

  _d->type = CurveType;
  return Error::Ok;
}

err_t Path::curveToRel(double cx, double cy, double tx, double ty)
{
  relToAbsInline(_d, &cx, &cy, &tx, &ty);
  return curveTo(cx, cy, tx, ty);
}

err_t Path::curveTo(double tx, double ty)
{
  Vertex* endv = _d->data + _d->length;

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
    return Error::Ok;
  }
}

err_t Path::curveToRel(double tx, double ty)
{
  relToAbsInline(_d, &tx, &ty);
  return curveTo(tx, ty);
}

// ============================================================================
// [Fog::Path - CubicTo]
// ============================================================================

err_t Path::cubicTo(double cx1, double cy1, double cx2, double cy2, double tx, double ty)
{
  Vertex* v = _add(3);
  if (!v) return Error::OutOfMemory;

  v[0].cmd = CmdCurve4;
  v[0].x = cx1;
  v[0].y = cy1;
  v[1].cmd = CmdCurve4;
  v[1].x = cx2;
  v[1].y = cy2;
  v[2].cmd = CmdCurve4;
  v[2].x = tx;
  v[2].y = ty;

  _d->type = CurveType;
  return Error::Ok;
}

err_t Path::cubicToRel(double cx1, double cy1, double cx2, double cy2, double tx, double ty)
{
  relToAbsInline(_d, &cx1, &cy1, &cx2, &cy2, &tx, &ty);
  return cubicTo(cx1, cy1, cx2, cy2, tx, ty);
}

err_t Path::cubicTo(double cx2, double cy2, double tx, double ty)
{
  Vertex* endv = _d->data + _d->length;

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
    return Error::Ok;
  }
}

err_t Path::cubicToRel(double cx2, double cy2, double tx, double ty)
{
  relToAbsInline(_d, &cx2, &cy2, &tx, &ty);
  return cubicTo(cx2, cy2, tx, ty);
}

// ============================================================================
// [Fog::Path - FlipX / FlipY]
// ============================================================================

err_t Path::flipX(double x1, double x2)
{
  if (!_d->length) return Error::Ok;

  err_t err = detach();
  if (err) return err;

  sysuint_t i, len = _d->length;
  Vertex* v = _d->data;
  
  for (i = 0; i < len; i++)
  {
    if (v[i].cmd.isVertex()) v[i].x = x2 - v[i].x + x1;
  }

  return Error::Ok;
}

err_t Path::flipY(double y1, double y2)
{
  if (!_d->length) return Error::Ok;

  err_t err = detach();
  if (err) return err;

  sysuint_t i, len = _d->length;
  Vertex* v = _d->data;
  
  for (i = 0; i < len; i++)
  {
    if (v[i].cmd.isVertex()) v[i].y = y2 - v[i].y + y1;
  }

  return Error::Ok;
}

// ============================================================================
// [Fog::Path - Translate]
// ============================================================================

err_t Path::translate(double dx, double dy)
{
  if (!_d->length) return Error::Ok;

  err_t err = detach();
  if (err) return err;
  
  sysuint_t i, len = _d->length;
  Vertex* v = _d->data;

  for (i = 0; i < len; i++)
  {
    if (v[i].cmd.isVertex())
    {
      v[i].x += dx;
      v[i].y += dy;
    }
  }

  return Error::Ok;
}

err_t Path::translate(double dx, double dy, sysuint_t pathId)
{
  if (!_d->length) return Error::Ok;

  err_t err = detach();
  if (err) return err;

  sysuint_t i, len = _d->length;
  Vertex* v = _d->data;

  for (i = pathId; i < len; i++)
  {
    if (v[i].cmd.isStop()) break;
    if (v[i].cmd.isVertex())
    {
      v[i].x += dx;
      v[i].y += dy;
    }
  }

  return Error::Ok;
}

// ============================================================================
// [Fog::Path - Scale]
// ============================================================================

err_t Path::scale(double sx, double sy, bool keepStartPos)
{
  if (!_d->length) return Error::Ok;

  err_t err = detach();
  if (err) return err;

  sysuint_t i, len = _d->length;
  Vertex* v = _d->data;

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

  return Error::Ok;
}

// ============================================================================
// [Fog::Path - ApplyMatrix]
// ============================================================================

err_t Path::applyMatrix(const Matrix& matrix)
{
  if (!_d->length) return Error::Ok;

  err_t err = detach();
  if (err) return err;

  Raster::functionMap->vector.pathVertexTransform(_d->data, _d->length, &matrix);

  return Error::Ok;
}

// ============================================================================
// [Fog::Path - Add]
// ============================================================================

err_t Path::addRect(const RectF& r)
{
  if (!r.isValid()) return Error::Ok;

  Vertex* v = _add(5);
  if (!v) return Error::OutOfMemory;

  v[0].cmd = CmdMoveTo;
  v[0].x = r.getX1();
  v[0].y = r.getY1();
  v[1].cmd = CmdLineTo;
  v[1].x = r.getX2();
  v[1].y = r.getY1();
  v[2].cmd = CmdLineTo;
  v[2].x = r.getX2();
  v[2].y = r.getY2();
  v[3].cmd = CmdLineTo;
  v[3].x = r.getX1();
  v[3].y = r.getY2();
  v[4].cmd = CmdEndPoly | CFlagClose;
  v[4].x = 0.0;
  v[4].y = 0.0;

  return Error::Ok;
}

err_t Path::addRects(const RectF* r, sysuint_t count)
{
  if (!count) return Error::Ok;
  FOG_ASSERT(r);

  Vertex* v = _add(count * 5);
  if (!v) return Error::OutOfMemory;

  for (sysuint_t i = 0; i < count; i++, r++)
  {
    if (!r->isValid()) continue;

    v[0].cmd = CmdMoveTo;
    v[0].x = r->getX1();
    v[0].y = r->getY1();
    v[1].cmd = CmdLineTo;
    v[1].x = r->getX2();
    v[1].y = r->getY1();
    v[2].cmd = CmdLineTo;
    v[2].x = r->getX2();
    v[2].y = r->getY2();
    v[3].cmd = CmdLineTo;
    v[3].x = r->getX1();
    v[3].y = r->getY2();
    v[4].cmd = CmdEndPoly | CFlagClose;
    v[4].x = 0.0;
    v[4].y = 0.0;

    v += 5;
  }

  // Return and update path length.
  _d->length = (sysuint_t)(v - _d->data);
  return Error::Ok;
}

err_t Path::addRound(const RectF& r, const PointF& radius)
{
  if (!r.isValid()) return Error::Ok;

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

  err_t err = Error::Ok;

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

err_t Path::addEllipse(const RectF& r)
{
  if (!r.isValid()) return Error::Ok;

  double rx = r.getWidth() / 2.0;
  double ry = r.getHeight() / 2.0;
  double cx = r.getX() + rx;
  double cy = r.getY() + ry;

  return _arcTo(cx, cy, rx, ry, 0.0, 2.0 * M_PI, CmdMoveTo, true);
}

err_t Path::addEllipse(const PointF& cp, const PointF& r)
{
  return _arcTo(cp.getX(), cp.getY(), r.getX(), r.getY(), 0.0, 2.0 * M_PI, CmdMoveTo, true);
}

err_t Path::addArc(const RectF& r, double start, double sweep)
{
  if (!r.isValid()) return Error::Ok;

  double rx = r.getWidth() / 2.0;
  double ry = r.getHeight() / 2.0;
  double cx = r.getX() + rx;
  double cy = r.getY() + ry;

  return _arcTo(cx, cy, rx, ry, start, sweep, CmdMoveTo, false);
}

err_t Path::addArc(const PointF& cp, const PointF& r, double start, double sweep)
{
  return _arcTo(cp.getX(), cp.getY(), r.getX(), r.getY(), start, sweep, CmdMoveTo, false);
}

err_t Path::addChord(const RectF& r, double start, double sweep)
{
  if (!r.isValid()) return Error::Ok;

  double rx = r.getWidth() / 2.0;
  double ry = r.getHeight() / 2.0;
  double cx = r.getX() + rx;
  double cy = r.getY() + ry;

  return _arcTo(cx, cy, rx, ry, start, sweep, CmdMoveTo, true);
}

err_t Path::addChord(const PointF& cp, const PointF& r, double start, double sweep)
{
  return _arcTo(cp.getX(), cp.getY(), r.getX(), r.getY(), start, sweep, CmdMoveTo, true);
}

err_t Path::addPie(const RectF& r, double start, double sweep)
{
  if (!r.isValid()) return Error::Ok;

  double rx = r.getWidth() / 2.0;
  double ry = r.getHeight() / 2.0;
  double cx = r.getX() + rx;
  double cy = r.getY() + ry;

  return addPie(PointF(cx, cy), PointF(rx, ry), start, sweep);
}

err_t Path::addPie(const PointF& cp, const PointF& r, double start, double sweep)
{
  if (sweep >= M_PI*2.0) return addEllipse(cp, r);

  start = fmod(start, M_PI * 2.0);
  if (start < 0) start += M_PI * 2.0;

  err_t err;

  if ( (err = moveTo(cp.getX(), cp.getY())) ) return err;
  if ( (err = _arcTo(cp.getX(), cp.getY(), r.getX(), r.getY(), start, sweep, CmdLineTo, true)) ) return err;

  return Error::Ok;
}

err_t Path::addPath(const Path& path)
{
  sysuint_t count = path.getLength();
  if (count == 0) return Error::Ok;

  uint32_t t = Math::max(getType(), path.getType());

  Vertex* v = _add(count);
  if (!v) return Error::OutOfMemory;

  const Vertex* src = path.cData();

  for (sysuint_t i = 0; i < count; i++, v++, src++)
  {
    v->cmd = src->cmd;
    v->x   = src->x;
    v->y   = src->y;
  }
  _d->type = t;

  return Error::Ok;
}

// ============================================================================
// [Fog::Path - Flatten]
// ============================================================================

err_t Path::flatten()
{
  return flatten(NULL, 1.0);
}

err_t Path::flatten(const Matrix* matrix, double approximationScale)
{
  if (getType() == LineType) return Error::Ok;
  return flattenTo(*this, matrix, approximationScale);
}

err_t Path::flattenTo(Path& dst, const Matrix* matrix, double approximationScale) const
{
  // --------------------------------------------------------------------------
  // Contains only lines (already flattened)
  // --------------------------------------------------------------------------

  if (getType() == LineType)
  {
    if (this == &dst)
    {
      if (matrix != NULL) dst.applyMatrix(*matrix);
      return Error::Ok;
    }
    else
    {
      dst = *this;
      if (matrix != NULL) dst.applyMatrix(*matrix);
      return Error::Ok;
    }
  }

  // --------------------------------------------------------------------------
  // Contains curves
  // --------------------------------------------------------------------------

  // The dst argument is here mainly if we need to flatten path into different
  // one. If paths are equal, we will simply create second path and assign
  // result to first one.
  if (this == &dst)
  {
    Path tmp;
    err_t err = flattenTo(tmp, matrix, approximationScale);
    dst = tmp;
    return err;
  }

  dst.clear();

  sysuint_t n = getLength();
  if (n == 0) return Error::Ok;
  if (dst.reserve(n * 8)) return Error::OutOfMemory;

  double lastx = 0.0;
  double lasty = 0.0;

  const Vertex* v = cData();
  Vertex* dstv;
  err_t err;

ensureSpace:
  dstv = dst._add(n);
  if (!dstv) return Error::OutOfMemory;

  do {
    switch (v->cmd.cmd())
    {
      case CmdMoveTo:
      case CmdLineTo:
        dstv->x = lastx = v->x;
        dstv->y = lasty = v->y;
        dstv->cmd = v->cmd;

        v++;
        dstv++;
        n--;

        break;

      case CmdCurve3:
        if (n <= 1) goto invalid;
        if (v[1].cmd.cmd() != CmdCurve3) goto invalid;

        // Finalize path
        dst._d->length = (sysuint_t)(dstv - dst._d->data);

        // Approximate curve.
        err = Raster::functionMap->vector.approximateCurve3(
          dst, lastx, lasty, v[0].x, v[0].y, v[1].x, v[1].y, approximationScale, 0.0);
        if (err) return err;

        lastx = v[1].x;
        lasty = v[1].y;

        v += 2;
        n -= 2;

        if (n == 0)
          return Error::Ok;
        else
          goto ensureSpace;

      case CmdCurve4:
        if (n <= 2) goto invalid;
        if (v[1].cmd.cmd() != CmdCurve4 ||
            v[2].cmd.cmd() != CmdCurve4) goto invalid;

        // Finalize path
        dst._d->length = (sysuint_t)(dstv - dst._d->data);

        // Approximate curve.
        err = Raster::functionMap->vector.approximateCurve4(
          dst, lastx, lasty, v[0].x, v[0].y, v[1].x, v[1].y, v[2].x, v[2].y, approximationScale, 0.0, 0.0);
        if (err) return err;

        lastx = v[2].x;
        lasty = v[2].y;

        v += 3;
        n -= 3;

        if (n == 0)
          return Error::Ok;
        else
          goto ensureSpace;

      case CmdCatrom:
      {
        if (n <= 2) goto invalid;
        if (v[1].cmd.cmd() != CmdCatrom ||
            v[2].cmd.cmd() != CmdCatrom) goto invalid;

        // Finalize path
        dst._d->length = (sysuint_t)(dstv - dst._d->data);

        double x1 = lastx;
        double y1 = lasty;
        double x2 = v[0].x;
        double y2 = v[0].y;
        double x3 = v[1].x;
        double y3 = v[1].y;
        double x4 = v[2].x;
        double y4 = v[2].y;

        // Trans. matrix Catmull-Rom to Bezier
        //
        //  0       1       0       0
        //  -1/6    1       1/6     0
        //  0       1/6     1       -1/6
        //  0       0       1       0
        err = Raster::functionMap->vector.approximateCurve4(dst,
          x2,
          y2,
          (-x1 + 6.0 * x2 + x3) / 6.0,
          (-y1 + 6.0 * y2 + y3) / 6.0,
          ( x2 + 6.0 * x3 - x4) / 6.0,
          ( y2 + 6.0 * y3 - y4) / 6.0,
          x3,
          y3,
          approximationScale, 0.0, 0.0);

        lastx = x4;
        lasty = y4;

        v += 3;
        n -= 3;

        if (n == 0)
          return Error::Ok;
        else
          goto ensureSpace;
      }

      case CmdUBSpline:
      {
        if (n <= 2) goto invalid;
        if (v[1].cmd.cmd() != CmdUBSpline ||
            v[2].cmd.cmd() != CmdUBSpline) goto invalid;

        // Finalize path
        dst._d->length = (sysuint_t)(dstv - dst._d->data);

        double x1 = lastx;
        double y1 = lasty;
        double x2 = v[0].x;
        double y2 = v[0].y;
        double x3 = v[1].x;
        double y3 = v[1].y;
        double x4 = v[2].x;
        double y4 = v[2].y;

        lastx = (    x2 + 4.0 * x3 + x4) / 6.0;
        lasty = (    y2 + 4.0 * y3 + y4) / 6.0;

        // Trans. matrix Uniform BSpline to Bezier
        //
        //  1/6     4/6     1/6     0
        //  0       4/6     2/6     0
        //  0       2/6     4/6     0
        //  0       1/6     4/6     1/6
        err = Raster::functionMap->vector.approximateCurve4(dst,
          (    x1 + 4.0 * x2 + x3) / 6.0,
          (    y1 + 4.0 * y2 + y3) / 6.0,
          (4 * x2 + 2.0 * x3     ) / 6.0,
          (4 * y2 + 2.0 * y3     ) / 6.0,
          (2 * x2 + 4.0 * x3     ) / 6.0,
          (2 * y2 + 4.0 * y3     ) / 6.0,
          lastx, lasty,
          approximationScale, 0.0, 0.0);

        v += 3;
        n -= 3;

        if (n == 0)
          return Error::Ok;
        else
          goto ensureSpace;
      }

      default:
        dstv->x = lastx = 0.0;
        dstv->y = lasty = 0.0;
        dstv->cmd = v->cmd;

        v++;
        dstv++;
        n--;

        break;
    }
  } while(n);

  dst._d->length = (sysuint_t)(dstv - dst._d->data);
  dst._d->type = LineType;
  if (matrix) dst.applyMatrix(*matrix);
  return Error::Ok;

invalid:
  dst._d->length = 0;
  dst._d->type = LineType;
  return Error::InvalidPath;
}

// ============================================================================
// [Fog::Path - Dash]
// ============================================================================

err_t Path::dash(const Vector<double>& dashes, double startOffset, double approximationScale)
{
  return dashTo(*this, dashes, startOffset, approximationScale);
}

err_t Path::dashTo(Path& dst, const Vector<double>& dashes, double startOffset, double approximationScale)
{
  if (getType() != LineType)
  {
    Path tmp;
    flattenTo(tmp, NULL, approximationScale);
    return tmp.dashTo(dst, dashes, startOffset, approximationScale);
  }
  else
  {
    AggPath src(*this);

    agg::conv_dash<AggPath, agg::vcgen_dash> dasher(src);

    Vector<double>::ConstIterator it(dashes);
    for (;;)
    {
      double d1 = it.value(); it.toNext();
      if (!it.isValid()) break;
      double d2 = it.value(); it.toNext();
      dasher.add_dash(d1, d2);
      if (!it.isValid()) break;
    }
    dasher.dash_start(startOffset);

    if (this == &dst)
    {
      Path tmp;
      err_t err = concatToPath(tmp, dasher);
      if (err) return err;
      return dst.set(tmp);
    }
    else
    {
      dst.clear();
      return concatToPath(dst, dasher);
    }
  }
}

// ============================================================================
// [Fog::Path - Stroke]
// ============================================================================

// Vertex (x, y) with the distance to the next one. The last vertex has
// distance between the last and the first points if the polygon is closed
// and 0.0 if it's a polyline.
struct FOG_HIDDEN PathVertexDist
{
  FOG_INLINE PathVertexDist() {}
  FOG_INLINE PathVertexDist(double x_, double y_) :
    x(x_),
    y(y_),
    dist(0.0)
  {
  }

  bool operator() (const PathVertexDist& val)
  {
    bool ret = (dist = calcDistance(x, y, val.x, val.y)) > pathVertexDistEpsilon;
    if (!ret) dist = 1.0 / pathVertexDistEpsilon;
    return ret;
  }

  double x;
  double y;
  double dist;
};

struct FOG_HIDDEN PathStroker
{
  PathStroker(Path& dst, const StrokeParams& params, double approximateScale = 1.0);
  ~PathStroker();

  FOG_INLINE void add_vertex(double x, double y)
  {
  }

  void calcCap(
    const PathVertexDist& v0,
    const PathVertexDist& v1,
    double len);

  void calcJoin(
    const PathVertexDist& v0,
    const PathVertexDist& v1,
    const PathVertexDist& v2,
    double len1,
    double len2);

  void calcArc(
    double x,   double y,
    double dx1, double dy1,
    double dx2, double dy2);

  void calcMiter(
    const PathVertexDist& v0,
    const PathVertexDist& v1,
    const PathVertexDist& v2,
    double dx1, double dy1,
    double dx2, double dy2,
    uint32_t lj,
    double mlimit,
    double dbevel);

  Path& dst;

  double _approximateScale;
  double _width;
  double _widthAbs;
  double _widthEps;
  double _da;
  int _widthSign;
  double _miterLimit;
  double _innerMiterLimit;
  uint32_t _lineCap;
  uint32_t _lineJoin;
  uint32_t _innerJoin;
};

PathStroker::PathStroker(Path& dst, const StrokeParams& params, double approximateScale) :
  dst(dst)
{
  _approximateScale = approximateScale;
  _width = params.lineWidth * 0.5;

  if (_width < 0)
  {
    _widthAbs  = -_width;
    _widthSign = -1;
  }
  else
  {
    _widthAbs  = _width;
    _widthSign = 1;
  }
  _widthEps = _width / 1024.0;

  _da = acos(_widthAbs / (_widthAbs + 0.125 / _approximateScale)) * 2;

  // TODO: Add inner join and inner miter limit to painter and to StrokeParams.
  _miterLimit = params.miterLimit; // TODO: Default is 4, add this to painter
  _innerMiterLimit = 1.01;
  _lineCap = params.lineCap;
  _lineJoin = params.lineJoin;
  _innerJoin = InnerJoinMiter;
}

PathStroker::~PathStroker()
{
}

void PathStroker::calcArc(
  double x,   double y,
  double dx1, double dy1,
  double dx2, double dy2)
{
  double a1 = atan2(dy1 * _widthSign, dx1 * _widthSign);
  double a2 = atan2(dy2 * _widthSign, dx2 * _widthSign);
  double da = _da;
  int i, n;

  add_vertex(x + dx1, y + dy1);
  if (_widthSign > 0)
  {
    if (a1 > a2) a2 += 2.0 * M_PI;
    n = int((a2 - a1) / da);
    da = (a2 - a1) / (n + 1);
    a1 += da;

    for (i = 0; i < n; i++)
    {
      add_vertex(x + cos(a1) * _width, y + sin(a1) * _width);
      a1 += da;
    }
  }
  else
  {
    if (a1 < a2) a2 -= 2.0 * M_PI;
    n = int((a1 - a2) / da);
    da = (a1 - a2) / (n + 1);
    a1 -= da;

    for (i = 0; i < n; i++)
    {
      add_vertex(x + cos(a1) * _width, y + sin(a1) * _width);
      a1 -= da;
    }
  }
  add_vertex(x + dx2, y + dy2);
}

//-----------------------------------------------------------------------
void PathStroker::calcMiter(
  const PathVertexDist& v0,
  const PathVertexDist& v1,
  const PathVertexDist& v2,
  double dx1, double dy1,
  double dx2, double dy2,
  uint32_t lj,
  double mlimit,
  double dbevel)
{
  double xi  = v1.x;
  double yi  = v1.y;
  double di  = 1;
  double lim = _widthAbs * mlimit;
  bool miter_limit_exceeded = true; // Assume the worst
  bool intersection_failed  = true; // Assume the worst

  if (calcIntersection(v0.x + dx1, v0.y - dy1,
                       v1.x + dx1, v1.y - dy1,
                       v1.x + dx2, v1.y - dy2,
                       v2.x + dx2, v2.y - dy2,
                       &xi, &yi))
  {
    // Calculation of the intersection succeeded
    //---------------------
    di = calcDistance(v1.x, v1.y, xi, yi);
    if (di <= lim)
    {
      // Inside the miter limit
      //---------------------
      add_vertex(xi, yi);
      miter_limit_exceeded = false;
    }
    intersection_failed = false;
  }
  else
  {
    // Calculation of the intersection failed, most probably
    // the three points lie one straight line.
    // First check if v0 and v2 lie on the opposite sides of vector:
    // (v1.x, v1.y) -> (v1.x+dx1, v1.y-dy1), that is, the perpendicular
    // to the line determined by vertices v0 and v1.
    // This condition determines whether the next line segments continues
    // the previous one or goes back.
    //----------------
    double x2 = v1.x + dx1;
    double y2 = v1.y - dy1;
    if ((crossProduct(v0.x, v0.y, v1.x, v1.y, x2, y2) < 0.0) ==
        (crossProduct(v1.x, v1.y, v2.x, v2.y, x2, y2) < 0.0))
    {
      // This case means that the next segment continues
      // the previous one (straight line)
      //-----------------
      add_vertex(v1.x + dx1, v1.y - dy1);
      miter_limit_exceeded = false;
    }
  }

  if (miter_limit_exceeded)
  {
    // Miter limit exceeded.
    switch(lj)
    {
      case LineJoinMiterRevert:
        // For the compatibility with SVG, PDF, etc, we use a simple bevel 
        // join instead of "smart" bevel.
        add_vertex(v1.x + dx1, v1.y - dy1);
        add_vertex(v1.x + dx2, v1.y - dy2);
        break;

      case LineJoinMiterRound:
        calcArc(v1.x, v1.y, dx1, -dy1, dx2, -dy2);
        break;

      default:
        // If no miter-revert, calculate new dx1, dy1, dx2, dy2.
        if (intersection_failed)
        {
          mlimit *= _widthSign;
          add_vertex(v1.x + dx1 + dy1 * mlimit, v1.y - dy1 + dx1 * mlimit);
          add_vertex(v1.x + dx2 - dy2 * mlimit, v1.y - dy2 - dx2 * mlimit);
        }
        else
        {
          double x1 = v1.x + dx1;
          double y1 = v1.y - dy1;
          double x2 = v1.x + dx2;
          double y2 = v1.y - dy2;
          di = (lim - dbevel) / (di - dbevel);
          add_vertex(x1 + (xi - x1) * di, y1 + (yi - y1) * di);
          add_vertex(x2 + (xi - x2) * di, y2 + (yi - y2) * di);
        }
        break;
    }
  }
}

void PathStroker::calcCap(
  const PathVertexDist& v0,
  const PathVertexDist& v1,
  double len)
{
  // TODO
  //vc.remove_all();
  double ilen = 1.0 / len;

  double dx1 = (v1.y - v0.y) * ilen;
  double dy1 = (v1.x - v0.x) * ilen;
  double dx2 = 0;
  double dy2 = 0;

  dx1 *= _width;
  dy1 *= _width;

  if (_lineCap != LineCapRound)
  {
    if (_lineCap == LineCapSquare)
    {
      dx2 = dy1 * _widthSign;
      dy2 = dx1 * _widthSign;
    }
    add_vertex(v0.x - dx1 - dx2, v0.y + dy1 - dy2);
    add_vertex(v0.x + dx1 - dx2, v0.y - dy1 - dy2);
  }
  else
  {
    int i;
    int n = int(M_PI / _da);
    double da = M_PI / (n + 1);
    double a1;

    add_vertex(v0.x - dx1, v0.y + dy1);

    if (_widthSign > 0)
    {
      a1 = atan2(dy1, -dx1);
      a1 += da;
      for (i = 0; i < n; i++)
      {
        add_vertex(v0.x + cos(a1) * _width, v0.y + sin(a1) * _width);
        a1 += da;
      }
    }
    else
    {
      a1 = atan2(-dy1, dx1);
      a1 -= da;
      for (i = 0; i < n; i++)
      {
        add_vertex(v0.x + cos(a1) * _width, v0.y + sin(a1) * _width);
        a1 -= da;
      }
    }
    add_vertex(v0.x + dx1, v0.y - dy1);
  }
}

void PathStroker::calcJoin(
  const PathVertexDist& v0,
  const PathVertexDist& v1,
  const PathVertexDist& v2,
  double len1,
  double len2)
{
  double wilen1 = (_width / len1);
  double wilen2 = (_width / len2);

  double dx1 = (v1.y - v0.y) * wilen1;
  double dy1 = (v1.x - v0.x) * wilen1;
  double dx2 = (v2.y - v1.y) * wilen2;
  double dy2 = (v2.x - v1.x) * wilen2;

  // TODO:
  //vc.remove_all();

  double cp = crossProduct(v0.x, v0.y, v1.x, v1.y, v2.x, v2.y);

  if (cp != 0 && (cp > 0) == (_width > 0))
  {
    // Inner join
    //---------------
    double limit = ((len1 < len2) ? len1 : len2) / _widthAbs;
    if (limit < _innerMiterLimit) limit = _innerMiterLimit;

    switch (_innerJoin)
    {
      case InnerJoinBevel:
        add_vertex(v1.x + dx1, v1.y - dy1);
        add_vertex(v1.x + dx2, v1.y - dy2);
        break;

      case InnerJoinMiter:
        calcMiter(v0, v1, v2, dx1, dy1, dx2, dy2, LineJoinMiterRevert, limit, 0);
        break;

      case InnerJoinJag:
      case InnerJoinRound:
        cp = (dx1-dx2) * (dx1-dx2) + (dy1-dy2) * (dy1-dy2);
        if (cp < len1 * len1 && cp < len2 * len2)
        {
          calcMiter(v0, v1, v2, dx1, dy1, dx2, dy2, LineJoinMiterRevert, limit, 0);
        }
        else
        {
          if (_innerJoin == InnerJoinJag)
          {
            add_vertex(v1.x + dx1, v1.y - dy1);
            add_vertex(v1.x,       v1.y      );
            add_vertex(v1.x + dx2, v1.y - dy2);
          }
          else
          {
            add_vertex(v1.x + dx1, v1.y - dy1);
            add_vertex(v1.x,       v1.y      );
            calcArc(v1.x, v1.y, dx2, -dy2, dx1, -dy1);
            add_vertex(v1.x,       v1.y      );
            add_vertex(v1.x + dx2, v1.y - dy2);
          }
        }
        break;

      default:
        FOG_ASSERT_NOT_REACHED();
    }
  }
  else
  {
    // Outer join
    //---------------

    // Calculate the distance between v1 and
    // the central point of the bevel line segment
    //---------------
    double dx = (dx1 + dx2) / 2;
    double dy = (dy1 + dy2) / 2;
    double dbevel = sqrt(dx * dx + dy * dy);

    if (_lineJoin == LineJoinRound || _lineJoin == LineJoinBevel)
    {
      // This is an optimization that reduces the number of points
      // in cases of almost collinear segments. If there's no
      // visible difference between bevel and miter joins we'd rather
      // use miter join because it adds only one point instead of two.
      //
      // Here we calculate the middle point between the bevel points
      // and then, the distance between v1 and this middle point.
      // At outer joins this distance always less than stroke width,
      // because it's actually the height of an isosceles triangle of
      // v1 and its two bevel points. If the difference between this
      // width and this value is small (no visible bevel) we can
      // add just one point.
      //
      // The constant in the expression makes the result approximately
      // the same as in round joins and caps. You can safely comment
      // out this entire "if".
      //-------------------
      if (_approximateScale * (_widthAbs - dbevel) < _widthEps)
      {
        if (calcIntersection(v0.x + dx1, v0.y - dy1,
                             v1.x + dx1, v1.y - dy1,
                             v1.x + dx2, v1.y - dy2,
                             v2.x + dx2, v2.y - dy2,
                             &dx, &dy))
        {
          add_vertex(dx, dy);
        }
        else
        {
          add_vertex(v1.x + dx1, v1.y - dy1);
        }
        return;
      }
    }

    switch (_lineJoin)
    {
      case LineJoinMiter:
      case LineJoinMiterRevert:
      case LineJoinMiterRound:
        calcMiter(v0, v1, v2, dx1, dy1, dx2, dy2, _lineJoin, _miterLimit, dbevel);
        break;

      case LineJoinRound:
        calcArc(v1.x, v1.y, dx1, -dy1, dx2, -dy2);
        break;

      case LineJoinBevel:
        add_vertex(v1.x + dx1, v1.y - dy1);
        add_vertex(v1.x + dx2, v1.y - dy2);
        break;

      default:
        FOG_ASSERT_NOT_REACHED();
    }
  }
}



















#if 0

namespace agg
{

    //------------------------------------------------------------------------
    vcgen_stroke::vcgen_stroke() :
        m_stroker(),
        m_src_vertices(),
        m_out_vertices(),
        m_shorten(0.0),
        m_closed(0),
        m_status(initial),
        m_src_vertex(0),
        m_out_vertex(0)
    {
    }

    //------------------------------------------------------------------------
    void vcgen_stroke::remove_all()
    {
        m_src_vertices.remove_all();
        m_closed = 0;
        m_status = initial;
    }

    //------------------------------------------------------------------------
    void vcgen_stroke::add_vertex(double x, double y, unsigned cmd)
    {
        m_status = initial;
        if (is_move_to(cmd))
        {
            m_src_vertices.modify_last(PathVertexDist(x, y));
        }
        else
        {
            if (is_vertex(cmd))
            {
                m_src_vertices.add(PathVertexDist(x, y));
            }
            else
            {
                m_closed = get_close_flag(cmd);
            }
        }
    }

    //------------------------------------------------------------------------
    void vcgen_stroke::rewind(unsigned)
    {
        if (m_status == initial)
        {
            m_src_vertices.close(m_closed != 0);
            shorten_path(m_src_vertices, m_shorten, m_closed);
            if (m_src_vertices.size() < 3) m_closed = 0;
        }
        m_status = ready;
        m_src_vertex = 0;
        m_out_vertex = 0;
    }


    //------------------------------------------------------------------------
    unsigned vcgen_stroke::vertex(double* x, double* y)
    {
        unsigned cmd = path_cmd_line_to;
        while(!is_stop(cmd))
        {
            switch(m_status)
            {
            case initial:
                rewind(0);

            case ready:
                if (m_src_vertices.size() < 2 + unsigned(m_closed != 0))
                {
                    cmd = path_cmd_stop;
                    break;
                }
                m_status = m_closed ? outline1 : cap1;
                cmd = path_cmd_move_to;
                m_src_vertex = 0;
                m_out_vertex = 0;
                break;

            case cap1:
                m_stroker.calcCap(m_out_vertices,
                                  m_src_vertices[0],
                                  m_src_vertices[1],
                                  m_src_vertices[0].dist);
                m_src_vertex = 1;
                m_prev_status = outline1;
                m_status = out_vertices;
                m_out_vertex = 0;
                break;

            case cap2:
                m_stroker.calcCap(m_out_vertices,
                                  m_src_vertices[m_src_vertices.size() - 1],
                                  m_src_vertices[m_src_vertices.size() - 2],
                                  m_src_vertices[m_src_vertices.size() - 2].dist);
                m_prev_status = outline2;
                m_status = out_vertices;
                m_out_vertex = 0;
                break;

            case outline1:
                if (m_closed)
                {
                    if (m_src_vertex >= m_src_vertices.size())
                    {
                        m_prev_status = close_first;
                        m_status = end_poly1;
                        break;
                    }
                }
                else
                {
                    if (m_src_vertex >= m_src_vertices.size() - 1)
                    {
                        m_status = cap2;
                        break;
                    }
                }
                m_stroker.calcJoin(m_out_vertices,
                                   m_src_vertices.prev(m_src_vertex),
                                   m_src_vertices.curr(m_src_vertex),
                                   m_src_vertices.next(m_src_vertex),
                                   m_src_vertices.prev(m_src_vertex).dist,
                                   m_src_vertices.curr(m_src_vertex).dist);
                ++m_src_vertex;
                m_prev_status = m_status;
                m_status = out_vertices;
                m_out_vertex = 0;
                break;

            case close_first:
                m_status = outline2;
                cmd = path_cmd_move_to;

            case outline2:
                if (m_src_vertex <= unsigned(m_closed == 0))
                {
                    m_status = end_poly2;
                    m_prev_status = stop;
                    break;
                }

                --m_src_vertex;
                m_stroker.calcJoin(m_out_vertices,
                                   m_src_vertices.next(m_src_vertex),
                                   m_src_vertices.curr(m_src_vertex),
                                   m_src_vertices.prev(m_src_vertex),
                                   m_src_vertices.curr(m_src_vertex).dist,
                                   m_src_vertices.prev(m_src_vertex).dist);

                m_prev_status = m_status;
                m_status = out_vertices;
                m_out_vertex = 0;
                break;

            case out_vertices:
                if (m_out_vertex >= m_out_vertices.size())
                {
                    m_status = m_prev_status;
                }
                else
                {
                    const point_d& c = m_out_vertices[m_out_vertex++];
                    *x = c.x;
                    *y = c.y;
                    return cmd;
                }
                break;

            case end_poly1:
                m_status = m_prev_status;
                return path_cmd_end_poly | path_flags_close | path_flags_ccw;

            case end_poly2:
                m_status = m_prev_status;
                return path_cmd_end_poly | path_flags_close | path_flags_cw;

            case stop:
                cmd = path_cmd_stop;
                break;
            }
        }
        return cmd;
    }
}
#endif


































#if 0

namespace agg
{

    //============================================================vcgen_stroke
    //
    // See Implementation agg_vcgen_stroke.cpp
    // Stroke generator
    //
    //------------------------------------------------------------------------
    class vcgen_stroke
    {
        enum status_e
        {
            initial,
            ready,
            cap1,
            cap2,
            outline1,
            close_first,
            outline2,
            out_vertices,
            end_poly1,
            end_poly2,
            stop
        };

    public:
        typedef vertex_sequence<PathVertexDist, 6> vertex_storage;
        typedef pod_bvector<point_d, 6>         coord_storage;

        vcgen_stroke();

        void line_cap(line_cap_e lc)     { m_stroker.line_cap(lc); }
        void line_join(line_join_e lj)   { m_stroker.line_join(lj); }
        void inner_join(inner_join_e ij) { m_stroker.inner_join(ij); }

        line_cap_e   line_cap()   const { return m_stroker.line_cap(); }
        line_join_e  line_join()  const { return m_stroker.line_join(); }
        inner_join_e inner_join() const { return m_stroker.inner_join(); }

        void width(double w) { m_stroker.width(w); }
        void miter_limit(double ml) { m_stroker.miter_limit(ml); }
        void miter_limit_theta(double t) { m_stroker.miter_limit_theta(t); }
        void inner_miter_limit(double ml) { m_stroker.inner_miter_limit(ml); }
        void approximation_scale(double as) { m_stroker.approximation_scale(as); }

        double width() const { return m_stroker.width(); }
        double miter_limit() const { return m_stroker.miter_limit(); }
        double inner_miter_limit() const { return m_stroker.inner_miter_limit(); }
        double approximation_scale() const { return m_stroker.approximation_scale(); }

        void shorten(double s) { m_shorten = s; }
        double shorten() const { return m_shorten; }

        // Vertex Generator Interface
        void remove_all();
        void add_vertex(double x, double y, unsigned cmd);

        // Vertex Source Interface
        void     rewind(unsigned path_id);
        unsigned vertex(double* x, double* y);

    private:
        vcgen_stroke(const vcgen_stroke&);
        const vcgen_stroke& operator = (const vcgen_stroke&);

        math_stroke<coord_storage> m_stroker;
        vertex_storage             m_src_vertices;
        coord_storage              m_out_vertices;
        double                     m_shorten;
        unsigned                   m_closed;
        status_e                   m_status;
        status_e                   m_prev_status;
        unsigned                   m_src_vertex;
        unsigned                   m_out_vertex;
    };
}
#endif










#if 0

namespace agg
{

    //===========================================================shorten_path
    template<class VertexSequence>
    void shorten_path(VertexSequence& vs, double s, unsigned closed = 0)
    {
        typedef typename VertexSequence::value_type vertex_type;

        if (s > 0.0 && vs.size() > 1)
        {
            double d;
            int n = int(vs.size() - 2);
            while(n)
            {
                d = vs[n].dist;
                if (d > s) break;
                vs.remove_last();
                s -= d;
                --n;
            }
            if (vs.size() < 2)
            {
                vs.remove_all();
            }
            else
            {
                n = vs.size() - 1;
                vertex_type& prev = vs[n-1];
                vertex_type& last = vs[n];
                d = (prev.dist - s) / prev.dist;
                double x = prev.x + (last.x - prev.x) * d;
                double y = prev.y + (last.y - prev.y) * d;
                last.x = x;
                last.y = y;
                if (!prev(last)) vs.remove_last();
                vs.close(closed != 0);
            }
        }
    }


}

#endif






#if 0

#include "agg_basics.h"
#include "agg_vcgen_stroke.h"
#include "agg_conv_adaptor_vcgen.h"

namespace agg
{

    //-------------------------------------------------------------conv_stroke
    template<class VertexSource, class Markers=null_markers>
    struct conv_stroke :
    public conv_adaptor_vcgen<VertexSource, vcgen_stroke, Markers>
    {
        typedef Markers marker_type;
        typedef conv_adaptor_vcgen<VertexSource, vcgen_stroke, Markers> base_type;

        conv_stroke(VertexSource& vs) :
            conv_adaptor_vcgen<VertexSource, vcgen_stroke, Markers>(vs)
        {
        }

        void line_cap(line_cap_e lc)     { base_type::generator().line_cap(lc);  }
        void line_join(line_join_e lj)   { base_type::generator().line_join(lj); }
        void inner_join(inner_join_e ij) { base_type::generator().inner_join(ij); }

        line_cap_e   line_cap()   const { return base_type::generator().line_cap();  }
        line_join_e  line_join()  const { return base_type::generator().line_join(); }
        inner_join_e inner_join() const { return base_type::generator().inner_join(); }

        void width(double w) { base_type::generator().width(w); }
        void miter_limit(double ml) { base_type::generator().miter_limit(ml); }
        void miter_limit_theta(double t) { base_type::generator().miter_limit_theta(t); }
        void inner_miter_limit(double ml) { base_type::generator().inner_miter_limit(ml); }
        void approximation_scale(double as) { base_type::generator().approximation_scale(as); }

        double width() const { return base_type::generator().width(); }
        double miter_limit() const { return base_type::generator().miter_limit(); }
        double inner_miter_limit() const { return base_type::generator().inner_miter_limit(); }
        double approximation_scale() const { return base_type::generator().approximation_scale(); }

        void shorten(double s) { base_type::generator().shorten(s); }
        double shorten() const { return base_type::generator().shorten(); }

    private:
       conv_stroke(const conv_stroke<VertexSource, Markers>&);
       const conv_stroke<VertexSource, Markers>&
           operator = (const conv_stroke<VertexSource, Markers>&);

    };

}

#endif



#if 0
err_t Path::strokeTo(Path& dst, const StrokeParams& strokeParams, double approximationScale) const
{
  if (type() != LineType)
  {
    Path tmp;
    flattenTo(tmp, NULL, approximationScale);
    return tmp.strokeTo(dst, strokeParams, approximationScale);
  }
  else
  {
    if (this == &dst)
    {
      Path tmp;
      err_t err = dst.strokeTo(tmp, strokeParams, approximationScale);
      dst = tmp;
      return err;
    }

    dst.clear();

    for (;;)
    {
      unsigned cmd = path_cmd_line_to;

      while (!is_stop(cmd))
      {
        switch(m_status)
        {
          case initial:
            rewind(0);

          case ready:
            if (m_src_vertices.size() < 2 + unsigned(m_closed != 0))
            {
              cmd = path_cmd_stop;
              break;
            }
            m_status = m_closed ? outline1 : cap1;
            cmd = path_cmd_move_to;
            m_src_vertex = 0;
            m_out_vertex = 0;
            break;

          case cap1:
            m_stroker.calcCap(m_out_vertices,
                              m_src_vertices[0],
                              m_src_vertices[1],
                              m_src_vertices[0].dist);
            m_src_vertex = 1;
            m_prev_status = outline1;
            m_status = out_vertices;
            m_out_vertex = 0;
            break;

          case cap2:
            m_stroker.calcCap(m_out_vertices,
                              m_src_vertices[m_src_vertices.size() - 1],
                              m_src_vertices[m_src_vertices.size() - 2],
                              m_src_vertices[m_src_vertices.size() - 2].dist);
            m_prev_status = outline2;
            m_status = out_vertices;
            m_out_vertex = 0;
            break;

          case outline1:
            if (m_closed)
            {
              if (m_src_vertex >= m_src_vertices.size())
              {
                m_prev_status = close_first;
                m_status = end_poly1;
                break;
              }
            }
            else
            {
              if (m_src_vertex >= m_src_vertices.size() - 1)
              {
                m_status = cap2;
                break;
              }
            }
            m_stroker.calcJoin(m_out_vertices,
                               m_src_vertices.prev(m_src_vertex),
                               m_src_vertices.curr(m_src_vertex),
                               m_src_vertices.next(m_src_vertex),
                               m_src_vertices.prev(m_src_vertex).dist,
                               m_src_vertices.curr(m_src_vertex).dist);
            ++m_src_vertex;
            m_prev_status = m_status;
            m_status = out_vertices;
            m_out_vertex = 0;
            break;

          case close_first:
            m_status = outline2;
            cmd = path_cmd_move_to;

          case outline2:
            if (m_src_vertex <= unsigned(m_closed == 0))
            {
              m_status = end_poly2;
              m_prev_status = stop;
              break;
            }

            --m_src_vertex;
            m_stroker.calcJoin(m_out_vertices,
                               m_src_vertices.next(m_src_vertex),
                               m_src_vertices.curr(m_src_vertex),
                               m_src_vertices.prev(m_src_vertex),
                               m_src_vertices.curr(m_src_vertex).dist,
                               m_src_vertices.prev(m_src_vertex).dist);

            m_prev_status = m_status;
            m_status = out_vertices;
            m_out_vertex = 0;
            break;

          case out_vertices:
            if (m_out_vertex >= m_out_vertices.size())
            {
              m_status = m_prev_status;
            }
            else
            {
              const point_d& c = m_out_vertices[m_out_vertex++];
              *x = c.x;
              *y = c.y;
              return cmd;
            }
            break;

          case end_poly1:
            m_status = m_prev_status;
            return path_cmd_end_poly | path_flags_close | path_flags_ccw;

          case end_poly2:
            m_status = m_prev_status;
            return path_cmd_end_poly | path_flags_close | path_flags_cw;

          case stop:
            cmd = path_cmd_stop;
            break;
        }
      }
command:
      return cmd;
    }
  }
}
#endif










err_t Path::stroke(const StrokeParams& strokeParams, double approximationScale)
{
  return strokeTo(*this, strokeParams);
}

#if 1
err_t Path::strokeTo(Path& dst, const StrokeParams& strokeParams, double approximationScale) const
{
  if (getType() != LineType)
  {
    Path tmp;
    flattenTo(tmp, NULL, approximationScale);
    return tmp.strokeTo(dst, strokeParams, approximationScale);
  }
  else
  {
    AggPath src(*this);

    agg::conv_stroke<AggPath> stroker(src);
    stroker.width(strokeParams.lineWidth);
    stroker.miter_limit(strokeParams.miterLimit);
    stroker.line_join(static_cast<agg::line_join_e>(strokeParams.lineJoin));
    stroker.line_cap(static_cast<agg::line_cap_e>(strokeParams.lineCap));
    stroker.approximation_scale(approximationScale);

    if (this == &dst)
    {
      Path tmp;
      err_t err = concatToPath(tmp, stroker);
      if (err) return err;
      return dst.set(tmp);
    }
    else
    {
      dst.clear();
      return concatToPath(dst, stroker);
    }
  }
}
#endif

// ============================================================================
// [Fog::Path - Operator Overload]
// ============================================================================

Path& Path::operator=(const Path& other)
{
  set(other);
  return *this;
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
  d->flags |= Path::Data::IsNull | Path::Data::IsSharable;
  d->type = Path::LineType;
  d->capacity = 0;
  d->length = 0;

  d->data[0].cmd._cmd = Path::CmdStop;
  d->data[0].x = 0.0;
  d->data[0].y = 0.0;

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_path_shutdown(void)
{
  using namespace Fog;

  Path::sharedNull->refCount.dec();
}
