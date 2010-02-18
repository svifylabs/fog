// [Fog/Gui Library - Public API]
//
// [Licence]
// MIT, See COPYING file in package

// [Dependencies]
#include <Fog/Gui/GuiEngine.h>

FOG_IMPLEMENT_OBJECT(Fog::GuiEngine)
FOG_IMPLEMENT_OBJECT(Fog::GuiWindow)

namespace Fog {

// ============================================================================
// [Fog::GuiEngine]
// ============================================================================

GuiEngine::GuiEngine() :
  _initialized(false)
{
}

GuiEngine::~GuiEngine()
{
}

uint32_t GuiEngine::keyToModifier(uint32_t key) const
{
  switch (key)
  {
    case KEY_LEFT_SHIFT : return MODIFIER_LEFT_SHIFT ;
    case KEY_RIGHT_SHIFT: return MODIFIER_RIGHT_SHIFT;
    case KEY_LEFT_CTRL  : return MODIFIER_LEFT_CTRL  ;
    case KEY_RIGHT_CTRL : return MODIFIER_RIGHT_CTRL ;
    case KEY_LEFT_ALT   : return MODIFIER_LEFT_ALT   ;
    case KEY_RIGHT_ALT  : return MODIFIER_RIGHT_ALT  ;
  }
  return 0;
}

// ============================================================================
// [Fog::UIWindow]
// ============================================================================

GuiWindow::GuiWindow(Widget* widget) :
  _widget(widget),
  _handle(NULL),
  _backingStore(NULL),
  _enabled(true),
  _visible(true),
  _hasFocus(false),
  _needBlit(true),
  _isDirty(false)
{
}

GuiWindow::~GuiWindow()
{
}

// ============================================================================
// [Fog::UIBackingStore]
// ============================================================================

GuiBackBuffer::GuiBackBuffer()
{
  _clear();
}

GuiBackBuffer::~GuiBackBuffer()
{
}

void GuiBackBuffer::_clear()
{
  _type = 0;
  _pixels = NULL;
  _width = 0;
  _height = 0;
  _format = PIXEL_FORMAT_NULL;
  _depth = 0;
  _stride = 0;

  _widthOrig = 0;
  _heightOrig = 0;

  _pixelsPrimary = NULL;
  _stridePrimary = 0;

  _pixelsSecondary = NULL;
  _strideSecondary = 0;

  _convertFunc = NULL;
  _convertDepth = 0;

  _createdTime = TimeTicks();
  _expireTime = TimeTicks();
}

bool GuiBackBuffer::expired(TimeTicks now) const
{
  return (_width != _widthOrig || _height != _heightOrig) && (now >= _expireTime);
}

} // Fog namespace
