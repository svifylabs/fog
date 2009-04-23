// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_UI_LAYOUT_H
#define _FOG_UI_LAYOUT_H

// [Dependencies]
#include <Fog/Core/Event.h>
#include <Fog/Core/Object.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/UI/Constants.h>
#include <Fog/UI/LayoutItem.h>

//! @addtogroup Fog_UI
//! @{

namespace Fog {

// ============================================================================
// [Fog::Layout]
// ============================================================================

//! @brief Base class for all layouts.
struct FOG_API Layout : public LayoutItem
{
  FOG_DECLARE_OBJECT(Layout, LayoutItem)

  // [Construction / Destruction]

  Layout();
  virtual ~Layout();

private:
  FOG_DISABLE_COPY(Layout)
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_UI_LAYOUT_H
