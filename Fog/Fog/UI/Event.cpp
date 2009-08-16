// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Static.h>
#include <Fog/UI/Event.h>

namespace Fog {

// ============================================================================
// [Fog::ChildEvent]
// ============================================================================

ChildEvent::ChildEvent(uint32_t code, Widget* child) :
  Fog::Event(code),
  _child(child)
{
}

// ============================================================================
// [Fog::LayoutEvent]
// ============================================================================

LayoutEvent::LayoutEvent(uint32_t code) :
  Event(code) 
{
}

// ============================================================================
// [Fog::StateEvent]
// ============================================================================

StateEvent::StateEvent(uint32_t code) :
  Event(code) 
{
}

// ============================================================================
// [Fog::VisibilityEvent]
// ============================================================================

VisibilityEvent::VisibilityEvent(uint32_t code) :
  Event(code) 
{
}

// ============================================================================
// [Fog::ConfigureEvent]
// ============================================================================

ConfigureEvent::ConfigureEvent() : 
  Event(EvConfigure),
  _rect(0, 0, 0, 0),
  _changed(0)
{
}

// ============================================================================
// [Fog::OriginEvent]
// ============================================================================

OriginEvent::OriginEvent() :
  Event(EvOrigin),
  _origin(0, 0)
{
}

// ============================================================================
// [Fog::FocusEvent]
// ============================================================================

FocusEvent::FocusEvent(uint32_t code, uint32_t reason) :
  Event(code), _reason(reason) 
{
}

// ============================================================================
// [Fog::KeyEvent]
// ============================================================================

KeyEvent::KeyEvent(uint32_t code) :
  Event(code),
  _key(0),
  _modifiers(0),
  _systemCode(0),
  _unicode(0)
{
}

// ============================================================================
// [Fog::MouseEvent]
// ============================================================================

MouseEvent::MouseEvent(uint32_t code) : 
  Event(code),
  _button(0),
  _modifiers(0),
  _position(-1, -1),
  _isOutside(false)
{
}

// ============================================================================
// [Fog::SelectionEvent]
// ============================================================================

SelectionEvent::SelectionEvent(uint32_t code) : 
  Event(code)
{
}

// ============================================================================
// [Fog::PaintEvent]
// ============================================================================

PaintEvent::PaintEvent(uint32_t code) : 
  Event(code),
  _painter(NULL),
  _isParentPainted(0)
{
}

// ============================================================================
// [Fog::CloseEvent]
// ============================================================================

CloseEvent::CloseEvent() : 
  Event(EvClose)
{
}

// ============================================================================
// [Fog::CheckEvent]
// ============================================================================

CheckEvent::CheckEvent(uint32_t code) : 
  Event(code),
  _status(code == EvCheck)
{
}

// ============================================================================
// [Fog::ThemeEvent]
// ============================================================================

ThemeEvent::ThemeEvent(uint32_t code) : 
  Event(code)
{
}
} // Fog namespace
