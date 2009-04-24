// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/Assert.h>
#include <Fog/Core/Error.h>
#include <Fog/Core/Math.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/Std.h>
#include <Fog/Graphics/Path.h>

// [Antigrain]
#include <agg_bezier_arc.h>

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
    memcpy(newd->data, d->data, d->length * sizeof(Vertex));
    d->deref();
    return newd;
  }
}

// ============================================================================
// [Fog::Path]
// ============================================================================

static FOG_INLINE double calcDistance(double x1, double y1, double x2, double y2)
{
  double dx = x2 - x1;
  double dy = y2 - y1;
  return sqrt((dx * dx) + (dy * dy));
}

static FOG_INLINE Path::Cmd lastCmd(Path::Data* d)
{
  return Path::Cmd(d->length
    ? d->data[d->length-1].cmd.cmd()
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

template<class VertexStorage>
static void aggJoinPath(Path* self, VertexStorage& a)
{
  sysuint_t i, len = a.num_vertices();
  Path::Vertex* v = self->_add(len);

  a.rewind(0);
  for (i = 0; i < len; i++)
  {
    v[i].cmd = a.vertex(&v[i].x, &v[i].y);
  }
}

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

  if (other.isEmpty()) { clear(); return Error::Ok; }

  err_t err = reserve(other.length());
  if (err) { clear(); return Error::OutOfMemory; }

  sysuint_t len = other._d->length;

  _d->length = len;
  memcpy(_d->data, other._d->data, len * sizeof(Vertex));

  return Error::Ok;
}

void Path::clear()
{
  if (_d->refCount.get() > 1)
    AtomicBase::ptr_setXchg(&_d, sharedNull->refAlways())->deref();
  else
    _d->length = 0;
}

void Path::free()
{
  AtomicBase::ptr_setXchg(&_d, sharedNull->refAlways())->deref();
}

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

err_t Path::arcTo(
  double rx, double ry,
  double angle,
  bool largeArcFlag,
  bool sweepFlag,
  double x, double y)
{
  if (_d->length && _d->data[_d->length-1].cmd.isVertex())
  {
    const double epsilon = 1e-30;
    const Vertex* v = &_d->data[_d->length-1];
    double x0 = v->x;
    double y0 = v->y;

    rx = fabs(rx);
    ry = fabs(ry);

    // Ensure radii are valid
    if (rx < epsilon || ry < epsilon) 
    {
      return lineTo(x, y);
    }
    else if (calcDistance(x0, y0, x, y) < epsilon)
    {
      // If the endpoints (x, y) and (x0, y0) are identical, then this
      // is equivalent to omitting the elliptical arc segment entirely.
      return Error::Ok;
    }
    else
    {
      agg::bezier_arc_svg a(x0, y0, rx, ry, angle, largeArcFlag, sweepFlag, x, y);
      if (a.radii_ok())
      {
        aggJoinPath(this, a);
        return Error::Ok;
      }
      else
      {
        return lineTo(x, y);
      }
    }
  }
  else
  {
    return moveTo(x, y);
  }
}

err_t Path::arcToRel(
  double rx, double ry,
  double angle,
  bool largeArcFlag,
  bool sweepFlag,
  double dx, double dy)
{
  relToAbsInline(_d, &dx, &dy);
  return arcTo(rx, ry, angle, largeArcFlag, sweepFlag, dx, dy);
}

err_t Path::quadraticCurveTo(
  double x_ctrl, double y_ctrl, 
  double x_to,   double y_to)
{
  Vertex* v = _add(2);
  if (!v) return Error::OutOfMemory;

  v[0].cmd = CmdCurve3;
  v[0].x = x_ctrl;
  v[0].y = y_ctrl;
  v[1].cmd = CmdCurve3;
  v[1].x = x_to;
  v[1].y = y_to;

  return Error::Ok;
}

err_t Path::quadraticCurveToRel(
  double dx_ctrl, double dy_ctrl, 
  double dx_to,   double dy_to)
{
  relToAbsInline(_d, &dx_ctrl, &dy_ctrl);
  relToAbsInline(_d, &dx_to,   &dy_to);
  return quadraticCurveTo(dx_ctrl, dy_ctrl, dx_to, dy_to);
}

err_t Path::quadraticCurveTo(
  double x_to,    double y_to)
{
  Vertex* endv = _d->data + _d->length;

  if (_d->length && endv[-1].cmd.isVertex())
  {
    double x_ctrl = endv[-1].x;
    double y_ctrl = endv[-1].y; 

    if (_d->length >= 2 && endv[-2].cmd.isCurve())
    {
      x_ctrl += endv[-1].x - endv[-2].x;
      y_ctrl += endv[-1].y - endv[-2].y;
    }

    return quadraticCurveTo(x_ctrl, y_ctrl, x_to, y_to);
  }
  else
    return Error::Ok;
}

err_t Path::quadraticCurveToRel(
  double dx_to,   double dy_to)
{
  relToAbsInline(_d, &dx_to, &dy_to);
  return quadraticCurveTo(dx_to, dy_to);
}

err_t Path::cubicCurveTo(
  double x_ctrl1, double y_ctrl1,
  double x_ctrl2, double y_ctrl2,
  double x_to,    double y_to)
{
  Vertex* v = _add(3);
  if (!v) return Error::OutOfMemory;

  v[0].cmd = CmdCurve4;
  v[0].x = x_ctrl1;
  v[0].y = y_ctrl1;
  v[1].cmd = CmdCurve4;
  v[1].x = x_ctrl2;
  v[1].y = y_ctrl2;
  v[2].cmd = CmdCurve4;
  v[2].x = x_to;
  v[2].y = y_to;

  return Error::Ok;
}

err_t Path::cubicCurveToRel(
  double dx_ctrl1, double dy_ctrl1, 
  double dx_ctrl2, double dy_ctrl2, 
  double dx_to,    double dy_to)
{
  relToAbsInline(_d, &dx_ctrl1, &dy_ctrl1);
  relToAbsInline(_d, &dx_ctrl2, &dy_ctrl2);
  relToAbsInline(_d, &dx_to,    &dy_to);

  return cubicCurveTo(dx_ctrl1, dy_ctrl1, dx_ctrl2, dy_ctrl2, dx_to, dy_to);
}

err_t Path::cubicCurveTo(
  double x_ctrl2, double y_ctrl2, 
  double x_to,    double y_to)
{
  Vertex* endv = _d->data + _d->length;

  if (_d->length && endv[-1].cmd.isVertex())
  {
    double x_ctrl1 = endv[-1].x;
    double y_ctrl1 = endv[-1].y; 

    if (_d->length >= 2 && endv[-2].cmd.isCurve())
    {
      x_ctrl1 += endv[-1].x - endv[-2].x;
      y_ctrl1 += endv[-1].y - endv[-2].y;
    }

    return cubicCurveTo(x_ctrl1, y_ctrl1, x_ctrl2, y_ctrl2, x_to, y_to);
  }
  else
    return Error::Ok;
}

err_t Path::cubicCurveToRel(
  double dx_ctrl2, double dy_ctrl2, 
  double dx_to,    double dy_to)
{
  relToAbsInline(_d, &dx_ctrl2, &dy_ctrl2);
  relToAbsInline(_d, &dx_to,    &dy_to);

  return cubicCurveTo(dx_ctrl2, dy_ctrl2, dx_to, dy_to);
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

err_t Path::addRect(const RectF& r)
{
  Vertex* v = _add(5);
  if (!v) return Error::OutOfMemory;

  v[0].cmd = CmdMoveTo;
  v[0].x = r.x1();
  v[0].y = r.y1();
  v[1].cmd = CmdLineTo;
  v[1].x = r.x2();
  v[1].y = r.y1();
  v[2].cmd = CmdLineTo;
  v[2].x = r.x2();
  v[2].y = r.y2();
  v[3].cmd = CmdLineTo;
  v[3].x = r.x1();
  v[3].y = r.y2();
  v[4].cmd = CmdEndPoly | CFlagClose;
  v[4].x = 0.0;
  v[4].y = 0.0;

  return Error::Ok;
}

err_t Path::addLineTo(const PointF* pts, sysuint_t count)
{
  Vertex* v = _add(count);
  if (!v) return Error::OutOfMemory;

  for (sysuint_t i = 0; i < count; i++)
  {
    v[i].cmd = CmdLineTo;
    v[i].x = pts[i].x();
    v[i].y = pts[i].y();
  }

  return Error::Ok;
}

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
  Fog::Path::Data* d = Fog::Path::sharedNull.instancep();

  d->refCount.init(1);
  d->flags |= Fog::Path::Data::IsNull | Fog::Path::Data::IsSharable;
  d->capacity = 0;
  d->length = 0;

  d->data[0].cmd._cmd = Fog::Path::CmdStop;
  d->data[0].x = 0.0;
  d->data[0].y = 0.0;

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_path_shutdown(void)
{
  Fog::Path::sharedNull->refCount.dec();
}
