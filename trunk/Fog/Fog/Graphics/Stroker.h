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
#ifndef _FOG_GRAPHICS_STROKER_H
#define _FOG_GRAPHICS_STROKER_H

// [Dependencies]
#include <Fog/Core/Atomic.h>
#include <Fog/Core/List.h>
#include <Fog/Core/Static.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Geometry.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct Matrix;
struct Path;
struct StrokerPrivate;

// ============================================================================
// [Fog::StrokeParams]
// ============================================================================

struct FOG_API StrokeParams
{
  // [Construction / Destruction]

  StrokeParams();
  StrokeParams(const StrokeParams& other);
  ~StrokeParams();

  // [Operator Overload]

  StrokeParams& operator=(const StrokeParams& other);

  // [Reset]

  void reset();

  // [Getters / Setters]

  FOG_INLINE double getLineWidth() const { return _lineWidth; }
  FOG_INLINE double getMiterLimit() const { return _miterLimit; }

  FOG_INLINE int getLineCap() const { return _lineCap; }
  FOG_INLINE int getLineJoin() const { return _lineJoin; }

  FOG_INLINE const List<double>& getDashes() const { return _dashes; }
  FOG_INLINE double getDashOffset() const { return _dashOffset; }

  FOG_INLINE void setLineWidth(double lineWidth) { _lineWidth = lineWidth; }
  FOG_INLINE void setMiterLimit(double miterLimit) { _miterLimit = miterLimit; }
  FOG_INLINE void setInnerLimit(double innerLimit) { _innerLimit = innerLimit; }

  FOG_INLINE void setLineCap(int lineCap) { _lineCap = lineCap; }
  FOG_INLINE void setLineJoin(int lineJoin) { _lineJoin = lineJoin; }
  FOG_INLINE void setInnerJoin(int innerJoin) { _innerJoin = innerJoin; }

  FOG_INLINE void setDashes(const List<double>& dashes) { _dashes = dashes; }
  FOG_INLINE void setDashes(const double* dashes, sysuint_t length)
  {
    _dashes.clear();
    _dashes.append(dashes, length);
  }
  FOG_INLINE void setDashOffset(double dashOffset) { _dashOffset = dashOffset; }

  // [Members]

  double _lineWidth;
  double _miterLimit;
  double _innerLimit;

  int _lineCap;
  int _lineJoin;
  int _innerJoin;

  List<double> _dashes;
  double _dashOffset;
};

// ============================================================================
// [Fog::Stroker]
// ============================================================================

struct FOG_API Stroker
{
  // [Construction / Destruction]

  Stroker();
  Stroker(const StrokeParams& params, double approximationScale = 1.0);
  ~Stroker();

  // [Getters / Setters]

  FOG_INLINE const StrokeParams& getParams() const { return _params; }
  FOG_INLINE double getApproximationScale() const { return _approximationScale; }

  void setParams(const StrokeParams& params);
  void setApproximationScale(double approximationScale);

  // [Stroking]

  err_t stroke(Path& dst) const;
  err_t stroke(Path& dst, const Path& src) const;

  // [Members]

protected:
  void _update();

  StrokeParams _params;
  double _approximationScale;

  double _w;
  double _wAbs;
  double _wEps;
  double _da;
  int _wSign;

private:
  friend struct StrokerPrivate;

  FOG_DISABLE_COPY(Stroker)
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_STROKER_H
