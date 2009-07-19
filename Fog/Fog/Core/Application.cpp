// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// [Precompiled headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Application.h>
#include <Fog/Core/Assert.h>
#include <Fog/Core/AutoLock.h>
#include <Fog/Core/Error.h>
#include <Fog/Core/EventLoop.h>
#include <Fog/Core/EventLoop_Def.h>
#include <Fog/Core/Hash.h>
#include <Fog/Core/Library.h>
#include <Fog/Core/Object.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/String.h>
#include <Fog/Core/Thread.h>
#include <Fog/UI/UISystem.h>

#if defined(FOG_OS_WINDOWS)
#include <Fog/Core/EventLoop_Win.h>
#include <Fog/UI/UISystem_Win.h>
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX) && defined(FOG_BUILD_MODULE_X11_INTERNAL)
#include <Fog/UI/UISystem_X11.h>
#endif // FOG_OS_POSIX

namespace Fog {

// ============================================================================
// [Fog::Application - Local]
// ============================================================================

struct FOG_HIDDEN Application_Local
{
  typedef Hash<String32, Application::EventLoopConstructor> ELHash;

  Lock lock;
  ELHash elHash;

  EventLoop* createEventLoop(const String32& type)
  {
    AutoLock locked(lock);
    Application::EventLoopConstructor ctor = elHash.value(type, NULL);
    return ctor ? ctor() : NULL;
  }
};

static Static<Application_Local> application_local;

// ============================================================================
// [Fog::Application]
// ============================================================================

Application* Application::_instance = NULL;

Application::Application(const String32& type) :
  _eventLoop(NULL),
  _uiSystem(NULL)
{
  // Create UISystem by type.
  if (type.startsWith(Ascii8("UI")))
    _uiSystem = createUISystem(type);

  // Create EventLoop by type.
  _eventLoop = createEventLoop(type);
  if (_eventLoop) Thread::_mainThread->_eventLoop = _eventLoop;

  // Set global application instance (singleton).
  if (_instance == NULL) _instance = this;

  // Set main thread event loop (can be NULL if no success).
  Thread::mainThread()->_eventLoop = _eventLoop;
}

Application::~Application()
{
  // We will unload library here, not by UISystem destructor, because 
  // EventLoop may be also created by UISystem.
  Library uiToClose;

  // Delete UISystem if associated.
  if (_uiSystem)
  {
    uiToClose = _uiSystem->_library;

    delete _uiSystem;
    _uiSystem = NULL;
  }

  // Delete EventLoop if associated.
  if (_eventLoop)
  {
    delete _eventLoop;
    _eventLoop = NULL;

    // Unset main thread event loop (this is safe, we will destroy it later if it
    // exists).
    Thread::_mainThread->_eventLoop = NULL;
  }

  // Clear global application instance (singleton).
  if (_instance == this) _instance = NULL;
}

err_t Application::run()
{
  if (!_eventLoop) return Error::InvalidHandle;

  _eventLoop->run();
  return Error::Ok;
}

void Application::quit()
{
  if (!_eventLoop) return;

  _eventLoop->quit();
}

bool Application::addEventLoopType(const String32& type, EventLoopConstructor ctor)
{
  AutoLock locked(application_local->lock);
  return application_local->elHash.put(type, ctor);
}

bool Application::removeEventLoopType(const String32& type)
{
  AutoLock locked(application_local->lock);
  return application_local->elHash.remove(type);
}

String32 Application::detectUI()
{
#if defined(FOG_OS_WINDOWS)
  return Ascii8("UI.Windows");
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
  return Ascii8("UI.X11");
#endif // FOG_OS_POSIX
}

typedef UISystem* (*UISystemConstructor)(void);

UISystem* Application::createUISystem(const String32& _type)
{
  String32 type(_type);

  // First try to detect UISystem if not specified
  if (type == Ascii8("UI")) type = detectUI();

  // UI.Windows is built-in
#if defined(FOG_OS_WINDOWS)
  if (type == Ascii8("UI.Windows"))
    return new UISystemWin();
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX) && defined(FOG_BUILD_MODULE_X11_INTERNAL)
  if (type == Ascii8("UI.X11"))
    return new UISystemX11();
#endif // FOG_OS_X11 && FOG_BUILD_MODULE_X11_INTERNAL

  // All other UI systems are dynamic linked libraries
  if (!type.startsWith(Ascii8("UI."))) return NULL;
  {
    Library lib;
    err_t err = lib.openPlugin(Ascii8("FogUI"), type.substring(Range(3)));
    if (err) return NULL;

    UISystemConstructor ctor = (UISystemConstructor)lib.symbol(Ascii8("createUISystem"));
    if (!ctor) return NULL;

    UISystem* uis = ctor();
    if (!uis) return NULL;

    // Success
    uis->_library = lib;
    return uis;
  }
}

EventLoop* Application::createEventLoop(const String32 &_type)
{
  String32 type(_type);

  // First try to detect UISystem if not specified
  if (type == Ascii8("UI")) type = detectUI();

  return application_local->createEventLoop(type);
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_application_init(void)
{
  using namespace Fog;

  application_local.init();

  Application::addEventLoopTypeT<EventLoopDefault>(Ascii8("Default"));

#if defined(FOG_OS_WINDOWS)
  Application::addEventLoopTypeT<EventLoopWinUI>(Ascii8("UI.Windows"));
  Application::addEventLoopTypeT<EventLoopWinIO>(Ascii8("IO.Windows"));
#endif // FOG_OS_WINDOWS

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_application_shutdown(void)
{
  using namespace Fog;

  application_local.destroy();
}
