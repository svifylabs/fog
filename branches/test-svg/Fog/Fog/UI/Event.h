// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_UI_EVENT_H
#define _FOG_UI_EVENT_H

// [Dependencies]
#include <Fog/Core/Event.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/UI/Constants.h>

//! @addtogroup Fog_UI
//! @{

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct Widget;
struct Painter;

// ============================================================================
// [Fog::Event IDs]
// ============================================================================

enum EvGui
{
  // [ChildEvent]
  EvChildAdd = 100,
  EvChildRemove,

  // [LayoutEvent]

  EvLayoutSet,
  EvLayoutRemove,

  EvLayoutItemAdd,
  EvLayoutItemRemove,

  // [StateEvent]
  EvEnable,
  EvDisable,
  EvDisableByParent,

  // [VisibilityEvent]
  EvShow,
  EvHide,
  EvHideByParent,

  // [ConfigureEvent]
  EvConfigure,

  // [OriginEvent]
  EvOrigin,

  // [FocusEvent]
  EvFocusIn,
  EvFocusOut,

  // [KeyEvent]
  EvKeyPress,
  EvKeyRelease,

  // [MouseEvent]
  EvMouseIn,
  EvMouseMove,
  EvMouseOutside,
  EvMouseOut,
  EvMousePress,
  EvMousePressRepeated,
  EvMouseRelease,
  EvClick,
  EvDoubleClick,
  EvWheel,

  // [SelectionEvent]
  EvClearSelection,
  EvSelectionRequired,

  // [PaintEvent]
  EvPaint,

  // [CloseEvent]
  EvClose,

  // [CheckEvent]

  EvCheck,
  EvUncheck,

  // [ThemeEvent]

  EvThemeChange,

  // Custom events, each application or library can use this space
  // for their internal events.
  EvCustomBegin = 65536,
};

struct FOG_API ChildEvent : public Event
{
  ChildEvent(uint32_t code = 0, Widget* child = NULL);

  FOG_INLINE Widget* child() const
  { return _child; }

  Widget* _child;
};

struct FOG_API LayoutEvent : public Event
{
  LayoutEvent(uint32_t code = 0);
};

struct FOG_API StateEvent : public Event
{
  StateEvent(uint32_t code = 0);
};

struct FOG_API VisibilityEvent : public Event
{
  VisibilityEvent(uint32_t code = 0);
};

struct FOG_API ConfigureEvent : public Event
{
  ConfigureEvent();

  enum Changed
  {
    ChangedPosition = (1 << 0),
    ChangedSize = (1 << 1),
    ChangedWindowPosition = (1 << 2),
    ChangedWindowSize = (1 << 3),
    ChangedOrientation = (1 << 4)
  };

  FOG_INLINE const Rect& rect() const
  { return _rect; }
  
  FOG_INLINE uint32_t changed() const
  { return _changed; }
  
  FOG_INLINE bool changedPosition() const
  { return (_changed & ChangedPosition) != 0; }
  
  FOG_INLINE bool changedSize() const
  { return (_changed & ChangedSize) != 0; }
  
  FOG_INLINE bool changedWindowPosition() const
  { return (_changed & ChangedWindowPosition) != 0; }

  FOG_INLINE bool changedWindowSize() const
  { return (_changed & ChangedWindowSize) != 0; }

  FOG_INLINE bool changedOrientation() const
  { return (_changed & ChangedOrientation) != 0; }

  // TODO: Is this needed?
  Rect _rect;
  uint32_t _changed;
};

struct FOG_API OriginEvent : public Event
{
  OriginEvent();

  FOG_INLINE const Point& origin() const
  { return _origin; }

  Point _origin;
};

struct FOG_API FocusEvent : public Event
{
  FocusEvent(uint32_t code = 0, uint32_t reason = FocusReasonNone);

  FOG_INLINE uint32_t reason() const
  { return _reason; }

  uint32_t _reason;
};

struct FOG_API KeyEvent : public Event
{
  KeyEvent(uint32_t code = 0);

  //! @brief Returns key code. See @c KeyEnum for possible ones.
  FOG_INLINE uint32_t key() const { return _key; }
  
  //! @brief Returns modifiers. See @c ModifierEnum for possible ones.
  FOG_INLINE uint32_t modifiers() const { return _modifiers; }
  
  //! @brief Returns key code that depends to windowing system.
  FOG_INLINE uint32_t systemCode() const { return _systemCode; }
  
  //! @brief Returns key code translated to unicode character, can be zero if it's not possible to translate it.
  FOG_INLINE Char32 unicode() const { return _unicode; }

  //! @brief Key code.
  uint32_t _key;
  //! @brief Modifiers.
  uint32_t _modifiers;
  //! @brief System key code (OS dependent, shouldn't be used).
  uint32_t _systemCode;
  //! @brief Key code translated to unicode character, can be zero.
  Char32 _unicode;
};

struct FOG_API MouseEvent : public Event
{
  MouseEvent(uint32_t code = 0);

  //! @brief Returns mouse button that was pressed or released.
  FOG_INLINE uint32_t button() const
  { return _button; }

  //! @brief Returns modifierd.
  FOG_INLINE uint32_t modifiers() const
  { return _modifiers; }

  //! @brief Returns mouse position relative to client or non-client 
  //! area (depends to event type).
  FOG_INLINE const Point& position() const
  { return _position; }

  //! @brief Mouse button that was pressed or released. 
  //!
  //! Enter, leave and motion events not using this member, so it's (uint32_t)-1.
  uint32_t _button;

  //! @brief Modifiers.
  uint32_t _modifiers;

  //! @brief Relative mouse position.
  Point _position;
};

struct FOG_API SelectionEvent : public Event
{
  SelectionEvent(uint32_t code = 0);
};

struct FOG_API PaintEvent : public Event
{
  PaintEvent(uint32_t code = 0);

  FOG_INLINE Painter* painter() const
  { return _painter; }

  FOG_INLINE bool parentPainted() const
  { return _parentPainted; }

  Painter* _painter;
  bool _parentPainted;
};

struct FOG_API CloseEvent : public Event
{
  CloseEvent();
};

struct FOG_API CheckEvent : public Event
{
  CheckEvent(uint32_t code = EvCheck);

  FOG_INLINE uint checked() const
  { return _checked; }

  uint _checked;
};

struct FOG_API ThemeEvent : public Event
{
  ThemeEvent(uint32_t code);
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_UI_EVENT_H
