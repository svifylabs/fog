// [Fog/UI Library - Public API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/UI/ButtonBase.h>
#include <Fog/UI/Constants.h>

FOG_IMPLEMENT_OBJECT(Fog::ButtonBase)

namespace Fog {

// ============================================================================
// [Fog::ButtonBase]
// ============================================================================

ButtonBase::ButtonBase() : 
  Widget(),
  _checked(CHECKED_OFF),
  _isMouseOver(false),
  _isMouseDown(false),
  _isSpaceDown(false)
{
}

ButtonBase::~ButtonBase()
{
}

String ButtonBase::getText() const
{
  return _text;
}

void ButtonBase::setText(const String& text)
{
  _text = text;

  invalidateLayout();
  repaint(RepaintWidget);
}

int ButtonBase::getChecked() const
{
  return _checked;
}

void ButtonBase::setChecked(int value)
{
  if (_checked == value) return;
  if (value > 2) return;

  _checked = value;

  CheckEvent e(value ? EV_CHECK : EV_UNCHECK);
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

void ButtonBase::onFocusIn(FocusEvent* e)
{
  base::onFocusIn(e);
}

void ButtonBase::onFocusOut(FocusEvent* e)
{
  if (_isSpaceDown)
  {
    _isSpaceDown = false;
    repaint(RepaintWidget);
  }

  base::onFocusOut(e);
}

void ButtonBase::onKeyPress(KeyEvent* e)
{
  if ((e->getKey() & KEY_MASK) == KEY_SPACE)
  {
    _isSpaceDown = true;
    repaint(RepaintWidget);
    e->accept();
    return;
  }

  base::onKeyPress(e);
}

void ButtonBase::onKeyRelease(KeyEvent* e)
{
  if ((e->getKey() & KEY_MASK) == KEY_SPACE)
  {
    _isSpaceDown = false;
    repaint(RepaintWidget);
    e->accept();
    return;
  }

  base::onKeyRelease(e);
}

void ButtonBase::onMouseIn(MouseEvent* e)
{
  _isMouseOver = true;
  repaint(RepaintWidget);

  base::onMouseIn(e);
}

void ButtonBase::onMouseOut(MouseEvent* e)
{
  _isMouseOver = false;
  repaint(RepaintWidget);

  base::onMouseOut(e);
}

void ButtonBase::onMousePress(MouseEvent* e)
{
  if (e->getButton() == BUTTON_LEFT)
  {
    _isMouseDown = true;
    repaint(RepaintWidget);
  }

  base::onMousePress(e);
}

void ButtonBase::onMouseRelease(MouseEvent* e)
{
  if (e->getButton() == BUTTON_LEFT)
  {
    _isMouseDown = false;
    repaint(RepaintWidget);
  }

  base::onMouseRelease(e);
}

void ButtonBase::onCheck(CheckEvent* e)
{
}

void ButtonBase::onUncheck(CheckEvent* e)
{
}

} // Fog namespace
