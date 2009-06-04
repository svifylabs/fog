// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_UI_BUTTONBASE_H
#define _FOG_UI_BUTTONBASE_H

// [Dependencies]
#include <Fog/UI/Widget.h>

//! @addtogroup Fog_UI
//! @{

namespace Fog {

// ============================================================================
// [Fog::ButtonBase]
// ============================================================================

struct FOG_API ButtonBase : public Widget
{
  FOG_DECLARE_OBJECT(ButtonBase, Widget)

  // [Construction / Destruction]

  ButtonBase();
  virtual ~ButtonBase();

  // [Text]

  String32 text() const;
  void setText(const String32& text);

  // [Value]

  uint checked() const;
  void setChecked(uint value);

  FOG_INLINE void check() { setChecked(true); }
  FOG_INLINE void uncheck() { setChecked(false); }

  // [Interaction]

  bool isMouseOver() const;
  bool isMouseDown() const;
  bool isSpaceDown() const;
  bool isDown() const;

  // [Events]

  // Fog::Widget Events.
  virtual void onFocus(FocusEvent* e);
  virtual void onKey(KeyEvent* e);
  virtual void onMouse(MouseEvent* e);

  // Fog::ButtonBase Events.
  virtual void onCheck(CheckEvent* e);
  virtual void onUncheck(CheckEvent* e);

  // [Event Map]

  fog_event_begin()
    fog_event(EvCheck            , onCheck           , CheckEvent     , Override)
    fog_event(EvUncheck          , onUncheck         , CheckEvent     , Override)
  fog_event_end()

protected:
  String32 _text;

  uint _checked;
  bool _isMouseOver;
  bool _isMouseDown;
  bool _isSpaceDown;
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_UI_BUTTONBASE_H
