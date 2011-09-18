// [Fog-Gui]
//
// [License]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GUI_WIDGET_BUTTON_H
#define _FOG_GUI_WIDGET_BUTTON_H

// [Dependencies]
#include <Fog/Core/Global/Global.h>
#include <Fog/Gui/Widget/ButtonBase.h>

namespace Fog {

//! @addtogroup Fog_Gui_Widget
//! @{

// ============================================================================
// [Fog::Button]
// ============================================================================

//! @brief Button.
struct FOG_API Button : public ButtonBase
{
  FOG_DECLARE_OBJECT(Button, ButtonBase)

  // [Construction / Destruction]

  Button();
  virtual ~Button();

  virtual SizeI getSizeHint() const;

  // [Event Handlers]

  // Fog::Widget Events.
  virtual void onPaint(PaintEvent* e);
};

//! @}

} // Fog namespace

// [Guard]
#endif // _FOG_GUI_WIDGET_BUTTON_H