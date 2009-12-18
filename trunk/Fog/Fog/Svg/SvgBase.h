// [Fog/Svg Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_SVG_SVGBASE_H
#define _FOG_SVG_SVGBASE_H

// [Dependencies]
#include <Fog/Core/Char.h>
#include <Fog/Core/List.h>
#include <Fog/Core/ManagedString.h>
#include <Fog/Core/String.h>
#include <Fog/Graphics/Argb.h>
#include <Fog/Graphics/Path.h>

namespace Fog {

//! @addtogroup Fog_Svg
//! @{

// ============================================================================
// [Fog::SvgCoord]
// ============================================================================

#include <Fog/Core/Pack.h>
struct FOG_HIDDEN FOG_PACKED SvgCoord
{
  FOG_INLINE SvgCoord() {}
  FOG_INLINE SvgCoord(double value, uint32_t unit) : value(value), unit(unit) {}

  double value;
  uint32_t unit;
};
#include <Fog/Core/Unpack.h>

//! @}

} // Fog namespace

FOG_DECLARE_TYPEINFO(Fog::SvgCoord, Fog::TYPEINFO_PRIMITIVE)

// [Guard]
#endif // _FOG_SVG_SVGBASE_H
