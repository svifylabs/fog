// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_UI_UISYSTEM_DEF_H
#define _FOG_UI_UISYSTEM_DEF_H

// [Dependencies]
#include <Fog/Core/Timer.h>
#include <Fog/Core/Vector.h>
#include <Fog/UI/UISystem.h>

//! @addtogroup Fog_UI
//! @{

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct UIWindowDefault;
struct UISystemDefault;

// ============================================================================
// [Fog::UISystem]
// ============================================================================

//! @brief Default UISystem implementation that is shared between Windows and
//! X11 UISystem.
//!
//! The reason to make this class is fact that all UISystems needs similar
//! things and only their API is different. Another reason is that anybody can
//! implement own UISystem not based on this one reimplementing all abstract
//! methods.
struct FOG_API UISystemDefault : public UISystem
{
  FOG_DECLARE_OBJECT(UISystemDefault, UISystem)

  // [Construction / Destruction]

  UISystemDefault();
  virtual ~UISystemDefault();

  // [ID <-> UIWindow]

  //! Widget mapper (ID <-> UIWindow)
  typedef Hash<void*, UIWindow*> WidgetMapper;

  virtual bool mapHandle(void* handle, UIWindow* w);
  virtual bool unmapHandle(void* handle);
  virtual UIWindow* handleToNative(void* handle) const;

  // [Display]

  virtual err_t getDisplayInfo(DisplayInfo* out) const;
  virtual err_t getPaletteInfo(PaletteInfo* out) const;

  // [Caret]

  virtual err_t getCaretStatus(CaretStatus* out) const;

  // [Keyboard]

  virtual err_t getKeyboardStatus(KeyboardStatus* out) const;
  virtual uint32_t getKeyboardModifiers() const;

  // [Mouse]

  virtual err_t getMouseStatus(MouseStatus* out) const;
  virtual err_t getSystemMouseStatus(SystemMouseStatus* out) const;

  virtual void invalidateMouseStatus();
  virtual void updateMouseStatus();
  virtual void changeMouseStatus(Widget* w, const Point& pos);

  virtual void clearSystemMouseStatus();

  virtual bool startButtonRepeat(uint32_t button, 
    bool reset, TimeDelta delay, TimeDelta interval);
  virtual bool stopButtonRepeat(uint32_t button);
  virtual void clearButtonRepeat();

  // [Wheel]

  virtual int getWheelLines() const;
  virtual void setWheelLines(int count);

  // [Timing]

  virtual TimeDelta getRepeatingDelay() const;
  virtual TimeDelta getRepeatingInterval() const;
  virtual TimeDelta getDoubleClickInterval() const;

  // [Windowing System]

  virtual void dispatchEnabled(Widget* w, bool enabled);
  virtual void dispatchVisibility(Widget* w, bool visible);

  virtual void dispatchConfigure(Widget* w, const Rect& rect, bool changedOrientation);

  virtual void widgetDestroyed(Widget* w);

  // [Update]

  virtual void update();
  virtual void doUpdate();
  virtual void doUpdateWindow(UIWindow* window);

  // [UIWindow Create / Destroy]

  virtual UIWindow* createUIWindow(Widget* widget) = 0;
  virtual void destroyUIWindow(UIWindow* native) = 0;

  // [Event Handlers]

  void _onButtonRepeatTimeOut(TimerEvent* e);

  // [Members]

  //! @brief ID <-> UIWindow mapper.
  WidgetMapper _widgetMapper;

  //! @brief Circular list of dirty windows.
  Vector<UIWindowDefault*> _dirtyList;

  //! @brief Display information.
  DisplayInfo _displayInfo;
  //! @brief Palette information.
  PaletteInfo _paletteInfo;

  //! @brief Caret status.
  CaretStatus _caretStatus;

  //! @brief Keyboard status information.
  KeyboardStatus _keyboardStatus;

  //! @brief Count of mouse devices (1 is default).
  int _mouseDevices;
  //! @brief Mouse status information.
  MouseStatus _mouseStatus;
  //! @brief System mouse status information.
  SystemMouseStatus _systemMouseStatus;

  //! @brief Count of lines to scroll through mouse wheel.
  int _wheelLines;

  TimeDelta _repeatingDelay;
  TimeDelta _repeatingInterval;
  TimeDelta _doubleClickInterval;

  Timer _buttonRepeat[3];
  TimeDelta _buttonRepeatInterval[3];
  TimeTicks _buttonTime[3];

  UpdateStatus _updateStatus;

  friend struct UIWindowDefault;
};

// ============================================================================
// [Fog::UIWindow]
// ============================================================================

struct FOG_API UIWindowDefault : public UIWindow
{
  FOG_DECLARE_OBJECT(UIWindowDefault, UIWindow)

  UIWindowDefault(Widget* widget);
  virtual ~UIWindowDefault();

  // [Windowing System]

  virtual void onEnabled(bool enabled);
  virtual void onVisibility(bool visible);

  virtual void onConfigure(const Rect& windowRect, const Rect& clientRect);

  virtual void onMouseHover(int x, int y);
  virtual void onMouseMove(int x, int y);
  virtual void onMouseLeave(int x, int y);

  virtual void onMousePress(uint32_t button, bool repeated);
  virtual void onMouseRelease(uint32_t button);
  virtual void onMouseWheel(uint32_t wheel);

  virtual void onFocus(bool focus);

  virtual bool onKeyPress(uint32_t key, uint32_t modifier, uint32_t systemCode, Char unicode);
  virtual bool onKeyRelease(uint32_t key, uint32_t modifier, uint32_t systemCode, Char unicode);

  virtual void clearFocus();
  virtual void setFocus(Widget* w, uint32_t reason);

  // [Dirty]

  virtual void setDirty();

protected:
  //! @brief Window bound rectangle.
  Rect _windowRect;
  //! @brief Window client rectangle.
  Rect _clientRect;
  //! @brief Window title.
  String _title;
  //! @brief Window resize granularity.
  Point _sizeGranularity;
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_UI_UISYSTEM_DEF_H
