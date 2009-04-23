// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_UI_WINDOW_H
#define _FOG_UI_WINDOW_H

// [Dependencies]
#include <Fog/UI/Widget.h>

//! @addtogroup Fog_UI
//! @{

namespace Fog {

// [Fog::Window]

//! @brief Top level window.
struct FOG_API Window : public Widget
{
  FOG_DECLARE_OBJECT(Window, Widget)

  Window(uint32_t createFlags = 0);
  virtual ~Window();

  virtual void onPaint(PaintEvent* e);
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_UI_WINDOW_H
