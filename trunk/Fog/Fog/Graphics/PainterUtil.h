// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_PAINTERUTIL_H
#define _FOG_GRAPHICS_PAINTERUTIL_H

// [Dependencies]
#include <Fog/Graphics/Argb.h>
#include <Fog/Graphics/Painter.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Fog::PainterUtil]
// ============================================================================

//! @brief Static class that provides painting of basic UI primitives like
//! 3d looking rectangles, etc...
struct FOG_API PainterUtil
{
  static void draw3dRect(Painter* p, const Rect& r, Argb color1, Argb color2, bool checked = false);
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_PAINTERUTIL_H
