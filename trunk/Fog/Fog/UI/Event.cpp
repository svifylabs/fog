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

ChildEvent::~ChildEvent()
{
}

// ============================================================================
// [Fog::LayoutEvent]
// ============================================================================

LayoutEvent::LayoutEvent(uint32_t code) :
  Event(code) 
{
}

LayoutEvent::~LayoutEvent()
{
}

// ============================================================================
// [Fog::StateEvent]
// ============================================================================

StateEvent::StateEvent(uint32_t code) :
  Event(code) 
{
}

StateEvent::~StateEvent()
{
}

// ============================================================================
// [Fog::VisibilityEvent]
// ============================================================================

VisibilityEvent::VisibilityEvent(uint32_t code) :
  Event(code) 
{
}

VisibilityEvent::~VisibilityEvent()
{
}

// ============================================================================
// [Fog::ConfigureEvent]
// ============================================================================

ConfigureEvent::ConfigureEvent() : 
  Event(EV_CONFIGURE),
  _rect(0, 0, 0, 0),
  _changed(0)
{
}

ConfigureEvent::~ConfigureEvent()
{
}

// ============================================================================
// [Fog::OriginEvent]
// ============================================================================

OriginEvent::OriginEvent() :
  Event(EV_ORIGIN),
  _origin(0, 0)
{
}

OriginEvent::~OriginEvent()
{
}

// ============================================================================
// [Fog::FocusEvent]
// ============================================================================

FocusEvent::FocusEvent(uint32_t code, uint32_t reason) :
  Event(code), _reason(reason) 
{
}

FocusEvent::~FocusEvent()
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
  _unicode(0),
  _isRepeated(false)
{
}

KeyEvent::~KeyEvent()
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
  _isOutside(false),
  _isRepeated(false)
{
}

MouseEvent::~MouseEvent()
{
}

// ============================================================================
// [Fog::SelectionEvent]
// ============================================================================

SelectionEvent::SelectionEvent(uint32_t code) : 
  Event(code)
{
}

SelectionEvent::~SelectionEvent()
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

PaintEvent::~PaintEvent()
{
}

// ============================================================================
// [Fog::CloseEvent]
// ============================================================================

CloseEvent::CloseEvent() : 
  Event(EV_CLOSE)
{
}

CloseEvent::~CloseEvent()
{
}

// ============================================================================
// [Fog::CheckEvent]
// ============================================================================

CheckEvent::CheckEvent(uint32_t code) : 
  Event(code),
  _status(code == EV_CHECK)
{
}

CheckEvent::~CheckEvent()
{
}

// ============================================================================
// [Fog::ThemeEvent]
// ============================================================================

ThemeEvent::ThemeEvent(uint32_t code) : 
  Event(code)
{
}

ThemeEvent::~ThemeEvent()
{
}

} // Fog namespace
