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

  String getText() const;
  void setText(const String& text);

  // [Value]

  //! @brief Get whether widget is checked (see @c CheckedState).
  int getChecked() const;
  //! @brief Set checked state of the widget (see @c CheckedState).
  void setChecked(int value);

  //! @brief Set widget state to checked.
  FOG_INLINE void check() { setChecked(true); }
  //! @brief Set widget state to unchecked.
  FOG_INLINE void uncheck() { setChecked(false); }

  // [Interaction]

  bool isMouseOver() const;
  bool isMouseDown() const;
  bool isSpaceDown() const;
  bool isDown() const;

  // [Event Handlers]

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
  String _text;

  int _checked;
  bool _isMouseOver;
  bool _isMouseDown;
  bool _isSpaceDown;
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_UI_BUTTONBASE_H
