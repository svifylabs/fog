// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Std.h>
#include <Fog/Graphics/Image.h>
#include <Fog/UI/UISystem.h>
#include <Fog/UI/Window.h>

#include <stdio.h>
#include <stdlib.h>

FOG_IMPLEMENT_OBJECT(Fog::Window)

namespace Fog {

// [Fog::Window]

Window::Window(uint32_t createFlags) :
  Fog::Composite(createFlags | UIWindow::CreateUIWindow)
{
  setFocusPolicy(StrongFocus);
}

Window::~Window()
{
}

void Window::onPaint(PaintEvent* e)
{
}

} // Fog namespace
