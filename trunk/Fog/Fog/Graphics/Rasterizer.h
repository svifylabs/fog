// [Fog/Graphics library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_RASTERIZER_H
#define _FOG_GRAPHICS_RASTERIZER_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/Graphics/Path.h>

namespace Fog {

// ============================================================================
// [Fog::Rasterizer]
// ============================================================================

struct FOG_API Rasterizer
{
  Rasterizer();
  virtual ~Rasterizer();
  
private:
  FOG_DISABLE_COPY(Rasterizer)
};

} // Fog namespace

// [Guard]
#endif // _FOG_GRAPHICS_RASTERIZER_H
