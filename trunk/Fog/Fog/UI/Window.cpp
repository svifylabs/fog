// [Fog/UI Library - Public API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled Headers]
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
  setFocusPolicy(FOCUS_STRONG);
}

Window::~Window()
{
}

void Window::onPaint(PaintEvent* e)
{
}

} // Fog namespace
