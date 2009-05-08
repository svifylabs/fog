// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/UI/ButtonBase.h>

FOG_IMPLEMENT_OBJECT(Fog::ButtonBase)

namespace Fog {

// ============================================================================
// [Fog::ButtonBase]
// ============================================================================

ButtonBase::ButtonBase() : 
  Widget(),
  _checked(false),
  _isMouseOver(false),
  _isMouseDown(false),
  _isSpaceDown(false)
{
}

ButtonBase::~ButtonBase()
{
}

String32 ButtonBase::text() const
{
  return _text;
}

void ButtonBase::setText(const String32& text)
{
  _text = text;
  repaint(RepaintWidget);
}

uint ButtonBase::checked() const
{
  return _checked;
}

void ButtonBase::setChecked(uint value)
{
  if (_checked == value) return;
  if (value > 2) return;

  _checked = value;

  CheckEvent e(value ? EvCheck : EvUncheck);
  sendEvent(&e);
}

bool ButtonBase::isMouseOver() const
{
  return _isMouseOver;
}

bool ButtonBase::isMouseDown() const
{
  return _isMouseDown;
}

bool ButtonBase::isSpaceDown() const
{
  return _isSpaceDown;
}

bool ButtonBase::isDown() const
{
  return (_isMouseDown && _isMouseOver) || _isSpaceDown;
}

void ButtonBase::onFocus(FocusEvent* e)
{
  if (e->code() == EvFocusOut && _isSpaceDown) 
  {
    _isSpaceDown = false;
    repaint(RepaintWidget);
  }

  base::onFocus(e);
}

void ButtonBase::onKey(KeyEvent* e)
{
  switch (e->code())
  {
    case EvMousePress:
      if ((e->key() & KeyMask) == KeySpace)
      {
        _isSpaceDown = true;
        repaint(RepaintWidget);
        e->accept();
        return;
      }
      break;
    case EvMouseRelease:
      if ((e->key() & KeyMask) == KeySpace)
      {
        _isSpaceDown = false;
        repaint(RepaintWidget);
        e->accept();
        return;
      }
      break;
  }

  base::onKey(e);
}

void ButtonBase::onMouse(MouseEvent* e)
{
  switch (e->code())
  {
    case EvMouseIn:
      _isMouseOver = true;
      repaint(RepaintWidget);
      break;
    case EvMouseOut:
      _isMouseOver = false;
      repaint(RepaintWidget);
      break;
    case EvMousePress:
      if (e->button() == ButtonLeft)
      {
        _isMouseDown = true;
        repaint(RepaintWidget);
      }
      break;
    case EvMouseRelease:
      if (e->button() == ButtonLeft)
      {
        _isMouseDown = false;
        repaint(RepaintWidget);
      }
      break;
  }

  base::onMouse(e);
}

void ButtonBase::onCheck(CheckEvent* e) {}
void ButtonBase::onUncheck(CheckEvent* e) {}

} // Fog namespace
