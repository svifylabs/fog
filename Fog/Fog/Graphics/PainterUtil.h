// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_UI_PAINTERUTIL_H
#define _FOG_UI_PAINTERUTIL_H

// [Dependencies]
#include <Fog/Graphics/Painter.h>
#include <Fog/Graphics/Rgba.h>

namespace Fog {

// ============================================================================
// [Fog::PainterUtil]
// ============================================================================

//! @brief Static class that provides painting of basic UI primitives like
//! 3d looking rectangles, etc...
struct FOG_API PainterUtil
{
  static void draw3dRect(Painter* p, const Rect& r, Rgba color1, Rgba color2, bool checked = false);
};

} // Fog namespace

// [Guard]
#endif // _FOG_UI_PAINTERUTIL_H
