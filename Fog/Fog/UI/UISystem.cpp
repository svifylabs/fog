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
    case KeyLeftShift : return ModifierLeftShift;
    case KeyRightShift: return ModifierRightShift;
    case KeyLeftCtrl  : return ModifierLeftCtrl;
    case KeyRightCtrl : return ModifierRightCtrl;
    case KeyLeftAlt   : return ModifierLeftAlt;
    case KeyRightAlt  : return ModifierRightAlt;
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
  _focus(false),
  _blit(true),
  _dirty(false)
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
  _stride = 0;
  _format.set(Fog::ImageFormat::Invalid);
  _widthOrig = 0;
  _heightOrig = 0;
  _pixelsPrimary = NULL;
  _stridePrimary = 0;
  _pixelsSecondary = NULL;
  _strideSecondary = 0;
  _usingConverter = false;
}

} // Fog namespace
