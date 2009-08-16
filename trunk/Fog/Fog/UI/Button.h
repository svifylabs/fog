// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_UI_BUTTON_H
#define _FOG_UI_BUTTON_H

// [Dependencies]
#include <Fog/UI/ButtonBase.h>

//! @addtogroup Fog_UI
//! @{

namespace Fog {

// ============================================================================
// [Fog::Button]
// ============================================================================

struct FOG_API Button : public ButtonBase
{
  FOG_DECLARE_OBJECT(Button, ButtonBase)

  // [Construction / Destruction]

  Button();
  virtual ~Button();

  // [Event Handlers]

  // Fog::Widget Events.
  virtual void onPaint(PaintEvent* e);
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_UI_BUTTON_H
