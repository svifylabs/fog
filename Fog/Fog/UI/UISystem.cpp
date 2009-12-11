// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Dependencies]
#include <Fog/UI/UISystem.h>

FOG_IMPLEMENT_OBJECT(Fog::UISystem)
FOG_IMPLEMENT_OBJECT(Fog::UIWindow)

namespace Fog {

// ============================================================================
// [Fog::UISystem]
// ============================================================================

UISystem::UISystem() : 
  _initialized(false)
{
}

UISystem::~UISystem()
{
}

uint32_t UISystem::keyToModifier(uint32_t key) const
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

UIWindow::UIWindow(Widget* widget) : 
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

UIWindow::~UIWindow()
{
}

// ============================================================================
// [Fog::UIBackingStore]
// ============================================================================

UIBackingStore::UIBackingStore()
{
  _clear();
}

UIBackingStore::~UIBackingStore()
{
}

void UIBackingStore::_clear()
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

} // Fog namespace
