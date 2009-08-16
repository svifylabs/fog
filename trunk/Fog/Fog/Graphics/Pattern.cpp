// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Graphics/Pattern.h>

namespace Fog {

// ============================================================================
// [Fog::Pattern]
// ============================================================================

Static<Pattern::Data> Pattern::sharedNull;

Pattern::Pattern() : 
  _d(sharedNull->ref())
{
}

Pattern::Pattern(const Pattern& other) :
  _d(other._d->ref())
{
}

Pattern::~Pattern()
{
  _d->deref();
}

err_t Pattern::_detach()
{
  if (_d->refCount.get() == 1) return Error::Ok;

  Data* newd = new(std::nothrow) Data(*_d);
  if (!newd) return Error::OutOfMemory;

  AtomicBase::ptr_setXchg(&_d, newd)->deref();
  return Error::Ok;
}

void Pattern::free()
{
  AtomicBase::ptr_setXchg(&_d, sharedNull->ref())->deref();
}

err_t Pattern::setType(uint32_t type)
{
  err_t err;
  if (_d->type == type) return Error::Ok;

  if (!(type <= 0x2 || (type >= TypeLinearGradient && type <= TypeConicalGradient)))
    return Error::InvalidArgument;

  // Optimize gradient switching.
  if ((_d->type & TypeGradientMask) != 0 && (type & TypeGradientMask) != 0)
  {
    if ( (err = detach()) ) return err;
    _d->type = type;
    return Error::Ok;
  }

  // Optimize switching to different pattern type by using previously allocated
  // memory if it's not shared between another Pattern.
  if (_d->refCount.get() == 1)
  {
    _d->deleteResources();
    _d->type = type;

    if (type == TypeTexture)
      _d->obj.texture.init();
    else if (type & TypeGradientMask)
      _d->obj.gradientStops.init();
  }
  else
  {
    Data* newd = new(std::nothrow) Data();
    if (!newd) return Error::OutOfMemory;

    newd->type = type;
    newd->spread = _d->spread;
    newd->points[0] = _d->points[0];
    newd->points[1] = _d->points[1];
    newd->gradientRadius = 0.0;
    
    if (type == TypeTexture)
      newd->obj.texture.init();
    else if (type & TypeGradientMask)
      newd->obj.gradientStops.init();

    AtomicBase::ptr_setXchg(&_d, newd)->deref();
  }
  return Error::Ok;
}

err_t Pattern::setSpread(uint32_t spread)
{
  if (_d->spread == spread) return Error::Ok;
  if (spread > 2) return Error::InvalidArgument;

  err_t err;
  if ( (err = detach()) ) return err;

  _d->spread = spread;
  return Error::Ok;
}

void Pattern::setNull()
{
  if (_d->refCount.get() > 1)
  {
    AtomicBase::ptr_setXchg(&_d, sharedNull->ref())->deref();
  }
  else
  {
    _d->deleteResources();
    _d->type = TypeNull;
    _d->spread = SpreadPad;
    _d->points[0].set(0.0, 0.0);
    _d->points[1].set(0.0, 0.0);
    _d->gradientRadius = 0.0;
    _d->obj.rgba.instance().set(0x00000000);
  }
}

err_t Pattern::setStartPoint(const Point& pt)
{
  return setStartPoint(
    PointF((double)pt.getX() + 0.5, (double)pt.getY() + 0.5));
}

err_t Pattern::setStartPoint(const PointF& pt)
{
  if (_d->points[0] == pt) return Error::Ok;

  err_t err;
  if ( (err = detach()) ) return err;

  _d->points[0] = pt;
  return Error::Ok;
}

err_t Pattern::setEndPoint(const Point& pt)
{
  return setEndPoint(
    PointF((double)pt.getX() + 0.5, (double)pt.getY() + 0.5));
}

err_t Pattern::setEndPoint(const PointF& pt)
{
  if (_d->points[1] == pt) return Error::Ok;

  err_t err;
  if ( (err = detach()) ) return err;

  _d->points[1] = pt;
  return Error::Ok;
}

err_t Pattern::setPoints(const Point& startPt, const Point& endPt)
{
  return setPoints(
    PointF((double)startPt.getX() + 0.5, (double)startPt.getY() + 0.5),
    PointF((double)endPt.getX()   + 0.5, (double)endPt.getY()   + 0.5));
}

err_t Pattern::setPoints(const PointF& startPt, const PointF& endPt)
{
  if (_d->points[0] == startPt && _d->points[1] == endPt) return Error::Ok;

  err_t err;
  if ( (err = detach()) ) return err;

  _d->points[0] = startPt;
  _d->points[1] = endPt;
  return Error::Ok;
}

// [Solid]

Rgba Pattern::getColor() const
{
  if (isSolid())
    return _d->obj.rgba.instance();
  else
    return Rgba(0x00000000);
}

err_t Pattern::setColor(const Rgba& rgba)
{
  err_t err;

  if (isSolid())
  {
    if (_d->obj.rgba.instance() == rgba) return Error::Ok;
    if ( (err = detach()) ) return err;

    _d->obj.rgba->set(rgba);
    return Error::Ok;
  }
  else
  {
    Data* newd = new(std::nothrow) Data();
    if (!newd) return Error::OutOfMemory;

    newd->type = TypeSolid;
    newd->spread = _d->spread;
    newd->points[0] = _d->points[0];
    newd->points[1] = _d->points[1];
    newd->gradientRadius = 0.0;
    newd->obj.rgba->set(rgba);

    AtomicBase::ptr_setXchg(&_d, newd)->deref();
    return Error::Ok;
  }
}

// [Texture]

Image Pattern::getTexture() const
{
  if (isTexture())
    return _d->obj.texture.instance();
  else
    return Image();
}

err_t Pattern::setTexture(const Image& texture)
{
  if (_d->refCount.get() == 1)
  {
    _d->deleteResources();
    _d->obj.texture.init(texture);

    return Error::Ok;
  }
  else
  {
    Data* newd = new(std::nothrow) Data();
    if (!newd) return Error::OutOfMemory;

    newd->type = TypeTexture;
    newd->spread = _d->spread;
    newd->points[0] = _d->points[0];
    newd->points[1] = _d->points[1];
    newd->gradientRadius = 0.0;
    newd->obj.texture.init(texture);

    AtomicBase::ptr_setXchg(&_d, newd)->deref();
    return Error::Ok;
  }
}

// [Gradient]

err_t Pattern::setGradientRadius(double r)
{
  if (!isGradient()) return Error::InvalidFunction;
  if (_d->gradientRadius == r) return Error::Ok;

  err_t err;
  if ( (err = detach()) ) return err;

  _d->gradientRadius = r;
  return Error::Ok;
}

err_t Pattern::setGradientStops(const GradientStops& stops)
{
  if (!isGradient()) return Error::InvalidFunction;
  if (_d->obj.gradientStops->_d == stops._d) return Error::Ok;

  err_t err;
  if ( (err = detach()) ) return err;

  _d->obj.gradientStops.instance() = stops;
  _d->obj.gradientStops->sort();
  return Error::Ok;
}

err_t Pattern::resetGradientStops()
{
  if (!isGradient()) return Error::InvalidFunction;

  _d->obj.gradientStops->free();
  return Error::Ok;
}

err_t Pattern::addGradientStop(const GradientStop& stop)
{
  if (!isGradient()) return Error::InvalidFunction;

  err_t err;
  if ( (err = detach()) ) return err;

  GradientStop s(stop);
  s.normalize();

  GradientStops::MutableIterator it(_d->obj.gradientStops.instance());
  for (it.toStart(); it.isValid(); it.toNext())
  {
    if (it.value().offset == s.offset)
    {
      it.value().rgba = s.rgba;
      return Error::Ok;
    }
    else if (it.value().offset > s.offset)
    {
      break;
    }
  }
  _d->obj.gradientStops->insert(it.index(), s);

  return Error::Ok;
}

// [Operator Overload]

Pattern& Pattern::operator=(const Pattern& other)
{
  AtomicBase::ptr_setXchg(&_d, other._d->ref())->deref();
  return *this;
}

Pattern& Pattern::operator=(const Rgba& rgba)
{
  setColor(rgba);
  return *this;
}

// ============================================================================
// [Fog::Gradient::Data]
// ============================================================================

Pattern::Data::Data()
{
  refCount.init(1);
  type = Pattern::TypeNull;
  spread = Pattern::SpreadPad;
  points[0].set(0.0, 0.0);
  points[1].set(0.0, 0.0);
  gradientRadius = 0.0;
  obj.rgba->set(0x00000000);
}

Pattern::Data::Data(const Data& other)
{
  refCount.init(1);
  type = other.type;
  spread = other.spread;
  points[0] = other.points[0];
  points[1] = other.points[1];
  gradientRadius = other.gradientRadius;

  if (type == TypeTexture)
    obj.texture.init(other.obj.texture.instance());
  else if (type & TypeGradientMask)
    obj.gradientStops.init(other.obj.gradientStops.instance());
}

Pattern::Data::~Data()
{
  deleteResources();
}

Pattern::Data* Pattern::Data::copy()
{
  return new(std::nothrow) Data(*this);
}

void Pattern::Data::deleteResources()
{
  if (type == Pattern::TypeTexture)
    obj.texture.destroy();
  else if (type & Pattern::TypeGradientMask)
    obj.gradientStops.destroy();
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_pattern_init(void)
{
  using namespace Fog;

  Pattern::Data* d = Pattern::sharedNull.instancep();
  d->refCount.init(1);
  d->type = Pattern::TypeNull;
  d->spread = Pattern::SpreadPad;
  d->points[0].set(0.0, 0.0);
  d->points[1].set(0.0, 0.0);
  d->gradientRadius = 0.0;
  d->obj.rgba.instance().set(0x00000000);

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_pattern_shutdown(void)
{
  using namespace Fog;

  Pattern::sharedNull.instancep()->refCount.dec();
}
