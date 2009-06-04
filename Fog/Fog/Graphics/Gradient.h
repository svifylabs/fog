// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_GRADIENT_H
#define _FOG_GRAPHICS_GRADIENT_H

// [Dependencies]
#include <Fog/Graphics/Rgba.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Fog::GradientStop]
// ============================================================================

struct FOG_HIDDEN GradientStop
{
  double offset;
  Rgba rgba;

  FOG_INLINE GradientStop() {}
  FOG_INLINE GradientStop(double offset, Rgba rgba) : 
    offset(offset), rgba(rgba) {}
  FOG_INLINE GradientStop(double offset, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF) : 
    offset(offset), rgba(Rgba(r, g, b, a)) {}
  FOG_INLINE GradientStop(const GradientStop& other) :
    offset(other.offset), rgba(other.rgba) {}

  FOG_INLINE GradientStop& operator=(const GradientStop& other)
  { offset = other.offset; rgba = other.rgba; return *this; }

  FOG_INLINE void normalize()
  { 
    if (offset < 0.0)
      offset = 0.0;
    else if (offset > 1.0)
      offset = 1.0;
  }
};

} // Fog namespace

FOG_DECLARE_TYPEINFO(Fog::GradientStop, Fog::PrimitiveType)

static FOG_INLINE bool operator==(const Fog::GradientStop& a, const Fog::GradientStop& b) { return a.offset == b.offset && a.rgba == b.rgba; }
static FOG_INLINE bool operator!=(const Fog::GradientStop& a, const Fog::GradientStop& b) { return a.offset != b.offset || a.rgba != b.rgba; }
static FOG_INLINE bool operator< (const Fog::GradientStop& a, const Fog::GradientStop& b) { return a.offset <  b.offset || (a.offset == b.offset && a.rgba <  b.rgba); }
static FOG_INLINE bool operator<=(const Fog::GradientStop& a, const Fog::GradientStop& b) { return a.offset <  b.offset || (a.offset == b.offset && a.rgba <= b.rgba); }
static FOG_INLINE bool operator> (const Fog::GradientStop& a, const Fog::GradientStop& b) { return a.offset >  b.offset || (a.offset == b.offset && a.rgba >  b.rgba); }
static FOG_INLINE bool operator>=(const Fog::GradientStop& a, const Fog::GradientStop& b) { return a.offset >  b.offset || (a.offset == b.offset && a.rgba >= b.rgba); }

namespace Fog {

// ============================================================================
// [Fog::GradientStops]
// ============================================================================

// [Typedefs]
typedef Vector<GradientStop> GradientStops;

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_GRADIENT_H
