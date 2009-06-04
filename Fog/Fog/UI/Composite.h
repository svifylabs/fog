// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_UI_COMPOSITE_H
#define _FOG_UI_COMPOSITE_H

// [Dependencies]
#include <Fog/UI/Widget.h>

//! @addtogroup Fog_UI
//! @{

namespace Fog {

// ============================================================================
// [Fog::Composite]
// ============================================================================

struct FOG_API Composite : public Widget
{
  FOG_DECLARE_OBJECT(Composite, Widget)

  // [Construction / Destruction]

  Composite(uint32_t createFlags = 0);
  virtual ~Composite();
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_UI_COMPOSITE_H
