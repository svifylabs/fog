// [Fog/Graphics Library - Private API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_PAINTERENGINE_RASTER_P_H
#define _FOG_GRAPHICS_PAINTERENGINE_RASTER_P_H

// [Dependencies]
#include <Fog/Graphics/PaintEngine.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

FOG_HIDDEN PaintEngine* _getRasterPaintEngine(const ImageBuffer& buffer, int hints);

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_PAINTERENGINE_RASTER_P_H
