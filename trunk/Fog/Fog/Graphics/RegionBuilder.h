// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_REGIONBUILDER_H
#define _FOG_GRAPHICS_REGIONBUILDER_H

// [Dependencies]
#include <Fog/Graphics/Geometry.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct Region;

// ============================================================================
// [Fog::RegionBuilder]
// ============================================================================

struct FOG_API RegionBuilder
{
  // [Construction / Destruction]

  RegionBuilder();
  ~RegionBuilder();

  // [Members]

private:
  FOG_DISABLE_COPY(RegionBuilder)
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_REGIONBUILDER_H
