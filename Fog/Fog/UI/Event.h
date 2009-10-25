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
  EvMouseOut,
  EvMouseMove,
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
  // [Construction / Destruction]

  ChildEvent(uint32_t code = 0, Widget* child = NULL);

  // [Methods]

  FOG_INLINE Widget* child() const { return _child; }

  // [Members]

  Widget* _child;
};

struct FOG_API LayoutEvent : public Event
{
  // [Construction / Destruction]

  LayoutEvent(uint32_t code = 0);
};

struct FOG_API StateEvent : public Event
{
  // [Construction / Destruction]

  StateEvent(uint32_t code = 0);
};

struct FOG_API VisibilityEvent : public Event
{
  // [Construction / Destruction]

  VisibilityEvent(uint32_t code = 0);
};

struct FOG_API ConfigureEvent : public Event
{
  // [Construction / Destruction]

  ConfigureEvent();

  // [Methods]

  enum Changed
  {
    ChangedPosition = (1 << 0),
    ChangedSize = (1 << 1),
    ChangedWindowPosition = (1 << 2),
    ChangedWindowSize = (1 << 3),
    ChangedOrientation = (1 << 4)
  };

  FOG_INLINE const Rect& rect() const { return _rect; }
  FOG_INLINE uint32_t changed() const { return _changed; }

  FOG_INLINE bool changedPosition() const { return (_changed & ChangedPosition) != 0; }
  FOG_INLINE bool changedSize() const { return (_changed & ChangedSize) != 0; }
  FOG_INLINE bool changedWindowPosition() const { return (_changed & ChangedWindowPosition) != 0; }
  FOG_INLINE bool changedWindowSize() const { return (_changed & ChangedWindowSize) != 0; }
  FOG_INLINE bool changedOrientation() const { return (_changed & ChangedOrientation) != 0; }

  // [Members]

  // TODO: Is this needed?
  Rect _rect;
  uint32_t _changed;
};

struct FOG_API OriginEvent : public Event
{
  // [Construction / Destruction]

  OriginEvent();

  // [Methods]

  FOG_INLINE const Point& getOrigin() const { return _origin; }

  // [Members]

  Point _origin;
};

struct FOG_API FocusEvent : public Event
{
  // [Construction / Destruction]

  FocusEvent(uint32_t code = 0, uint32_t reason = FocusReasonNone);

  // [Members]

  FOG_INLINE uint32_t getReason() const { return _reason; }

  // [Methods]

  uint32_t _reason;
};

struct FOG_API KeyEvent : public Event
{
  // [Construction / Destruction]

  KeyEvent(uint32_t code = 0);

  // [Methods]

  //! @brief Returns key code. See @c KeyEnum for possible ones.
  FOG_INLINE uint32_t getKey() const { return _key; }
  
  //! @brief Returns modifiers. See @c ModifierEnum for possible ones.
  FOG_INLINE uint32_t getModifiers() const { return _modifiers; }
  
  //! @brief Returns key code that depends to windowing system.
  FOG_INLINE uint32_t getSystemCode() const { return _systemCode; }
  
  //! @brief Returns key code translated to unicode character, can be zero if it's not possible to translate it.
  FOG_INLINE Char getUnicode() const { return _unicode; }

  // [Members]

  //! @brief Key code.
  uint32_t _key;
  //! @brief Modifiers.
  uint32_t _modifiers;
  //! @brief System key code (OS dependent, shouldn't be used).
  uint32_t _systemCode;
  //! @brief Key code translated to unicode character, can be zero.
  Char _unicode;
};

struct FOG_API MouseEvent : public Event
{
  // [Construction / Destruction]

  MouseEvent(uint32_t code = 0);

  // [Methods]

  //! @brief Returns mouse button that was pressed or released.
  FOG_INLINE uint32_t getButton() const { return _button; }

  //! @brief Returns modifierd.
  FOG_INLINE uint32_t getModifiers() const { return _modifiers; }

  //! @brief Returns mouse position relative to client or non-client 
  //! area (depends to event type).
  FOG_INLINE const Point& getPosition() const { return _position; }

  //! @brief Get if event was generated outside of widget.
  FOG_INLINE bool isOutside() const { return _isOutside; }

  // [Members]

  //! @brief Mouse button that was pressed or released. 
  //!
  //! Enter, leave and motion events not using this member, so it's (uint32_t)-1.
  uint32_t _button;

  //! @brief Modifiers.
  uint32_t _modifiers;

  //! @brief Relative mouse position.
  Point _position;

  //! @brief True if event is outside of widget (grabbing).
  bool _isOutside;
};

struct FOG_API SelectionEvent : public Event
{
  // [Construction / Destruction]

  SelectionEvent(uint32_t code = 0);
};

struct FOG_API PaintEvent : public Event
{
  // [Construction / Destruction]

  PaintEvent(uint32_t code = 0);

  // [Methods]

  FOG_INLINE Painter* getPainter() const { return _painter; }
  FOG_INLINE bool isParentPainted() const { return _isParentPainted; }

  // [Members]

  Painter* _painter;
  bool _isParentPainted;
};

struct FOG_API CloseEvent : public Event
{
  // [Construction / Destruction]

  CloseEvent();
};

struct FOG_API CheckEvent : public Event
{
  // [Construction / Destruction]

  CheckEvent(uint32_t code = EvCheck);

  // [Methods]

  FOG_INLINE uint getStatus() const { return _status; }

  // [Members]

  uint _status;
};

struct FOG_API ThemeEvent : public Event
{
  // [Construction / Destruction]

  ThemeEvent(uint32_t code);
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_UI_EVENT_H
