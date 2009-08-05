// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_PAINTERENGINE_RASTER_H
#define _FOG_GRAPHICS_PAINTERENGINE_RASTER_H

// [Dependencies]
#include <Fog/Graphics/PainterEngine.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

PainterEngine* _getRasterPainterEngine(uint8_t* pixels, int width, int height, sysint_t stride, int format, int hints);

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_PAINTERENGINE_RASTER_H
