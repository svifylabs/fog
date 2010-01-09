// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_GRADIENT_H
#define _FOG_GRAPHICS_GRADIENT_H

// [Dependencies]
#include <Fog/Core/Static.h>
#include <Fog/Graphics/Argb.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Geometry.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct Pattern;

// ============================================================================
// [Fog::Gradient]
// ============================================================================

struct FOG_HIDDEN Gradient
{
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  FOG_INLINE Gradient() :
    _type(PATTERN_LINEAR_GRADIENT),
    _spread(SPREAD_PAD),
    _start(0.0, 0.0),
    _end(0.0, 0.0),
    _radius(0.0)
  {
  }

  FOG_INLINE Gradient(const Gradient& other) :
    _type(other._type),
    _spread(other._spread),
    _start(other._start),
    _end(other._end),
    _radius(other._radius),
    _stops(other._stops)
  {
  }

  FOG_INLINE Gradient(int type, int spread, const PointD& start, const PointD& end, const List<ArgbStop>& stops) :
    _type((type < PATTERN_LINEAR_GRADIENT) | (type > PATTERN_CONICAL_GRADIENT) ? PATTERN_LINEAR_GRADIENT : type),
    _spread((uint)spread >= SPREAD_INVALID ? SPREAD_PAD : spread),
    _start(start),
    _end(end),
    _radius(0.0),
    _stops(stops)
  {
  }

  FOG_INLINE Gradient(int type, int spread, const PointD& start, const PointD& end, double radius, const List<ArgbStop>& stops) :
    _type((type < PATTERN_LINEAR_GRADIENT) | (type > PATTERN_CONICAL_GRADIENT) ? PATTERN_LINEAR_GRADIENT : type),
    _spread((uint)spread >= SPREAD_INVALID ? SPREAD_PAD : spread),
    _start(start),
    _end(end),
    _radius(radius),
    _stops(stops)
  {
  }

  FOG_INLINE ~Gradient()
  {
  }

  // --------------------------------------------------------------------------
  // [Type]
  // --------------------------------------------------------------------------

  FOG_INLINE int getType() const
  {
    return _type;
  }

  FOG_INLINE void setType(int type)
  {
    if ((type < PATTERN_LINEAR_GRADIENT) | (type > PATTERN_CONICAL_GRADIENT)) return;
    _type = type;
  }

  // --------------------------------------------------------------------------
  // [Spread]
  // --------------------------------------------------------------------------

  FOG_INLINE int getSpread() const
  {
    return _spread;
  }

  FOG_INLINE void setSpread(int spread)
  {
    if ((uint)spread >= SPREAD_INVALID) return;
    _spread = spread;
  }

  // --------------------------------------------------------------------------
  // [Points]
  // --------------------------------------------------------------------------

  FOG_INLINE const PointD& getStartPoint() const { return _start; }
  FOG_INLINE const PointD& getEndPoint() const { return _end; }

  FOG_INLINE void setStartPoint(const Point& pt) { _start = pt; }
  FOG_INLINE void setStartPoint(const PointD& pt) { _start = pt; }

  FOG_INLINE void setEndPoint(const Point& pt) { _end = pt; }
  FOG_INLINE void setEndPoint(const PointD& pt) { _end = pt; }

  FOG_INLINE void setPoints(const Point& startPt, const Point& endPt) { _start = startPt; _end = endPt; }
  FOG_INLINE void setPoints(const PointD& startPt, const PointD& endPt) { _start = startPt; _end = endPt; }

  // --------------------------------------------------------------------------
  // [Radius]
  // --------------------------------------------------------------------------

  FOG_INLINE double getRadius() const { return _radius; }
  FOG_INLINE void setRadius(double r) { _radius = r; }

  // --------------------------------------------------------------------------
  // [Stops]
  // --------------------------------------------------------------------------

  FOG_INLINE const List<ArgbStop>& getStops() const { return _stops; }
  FOG_INLINE void setStops(const List<ArgbStop>& stops) { _stops = stops; }
  FOG_INLINE void resetStops() { _stops.clear(); }

  FOG_INLINE err_t addStop(const ArgbStop& stop) { return _stops.append(stop); }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  FOG_INLINE Gradient& operator=(const Gradient& other)
  {
    _type = other._type;
    _spread = other._spread;
    _start = other._start;
    _end = other._end;
    _radius = other._radius;
    _stops = other._stops;

    return *this;
  }

private:
  int _type;
  int _spread;
  PointD _start;
  PointD _end;
  double _radius;
  List<ArgbStop> _stops;

  friend struct Pattern;
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_GRADIENT_H
