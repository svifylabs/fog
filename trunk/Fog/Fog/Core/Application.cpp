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

#if defined(FOG_OS_POSIX)
#include <Fog/Core/EventLoop_Libevent.h>
#endif // FOG_OS_POSIX

namespace Fog {

// ============================================================================
// [Fog::Application - Local]
// ============================================================================

struct FOG_HIDDEN Application_Local
{
  typedef EventLoop* (*NewEventLoopFn)(void);
  typedef Hash<String32, NewEventLoopFn> ELHash;

  Lock lock;
  ELHash elHash;

  template<typename EventLoopT>
  struct CtorHelper
  {
    static EventLoop* ctor() { return new EventLoopT(); }
  };

  template<typename EventLoopT>
  void addEventLoopType(const String32& type)
  {
    elHash.put(type, CtorHelper<EventLoopT>::ctor);
  }

  void removeEventLoopType(const String32& type)
  {
    elHash.remove(type);
  }

  EventLoop* createEventLoop(const String32& type)
  {
    NewEventLoopFn ctor = elHash.value(type, NULL);
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
  if (type.startsWith(StubAscii8("UI")))
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
  // Delete EventLoop if associated.
  if (_eventLoop)
  {
    delete _eventLoop;
    _eventLoop = NULL;
    Thread::_mainThread->_eventLoop = NULL;
  }

  // Delete UISystem if associated.
  if (_uiSystem)
  {
    delete _uiSystem;
    _uiSystem = NULL;
  }

  // Unset main thread event loop (this is safe, we will destroy it later if it
  // exists).
  Thread::mainThread()->_eventLoop = NULL;

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

String32 Application::detectUI()
{
#if defined(FOG_OS_WINDOWS)
  return StubAscii8("UI::Windows");
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
  return StubAscii8("UI::X11");
#endif // FOG_OS_POSIX
}

typedef UISystem* (*UISystemConstructor)(void);

UISystem* Application::createUISystem(const String32& _type)
{
  String32 type(_type);

  // First try to detect UISystem if not specified
  if (type == StubAscii8("UI")) type = detectUI();

  // UI::Windows is built-in
#if defined(FOG_OS_WINDOWS)
  if (type == StubAscii8("UI::Windows"))
    return new UISystemWin();
#endif // FOG_OS_WINDOWS

  // All other UI systems are dynamic linked libraries
  if (!type.startsWith(StubAscii8("UI::"))) return NULL;
  {
    Library lib;
    err_t err = lib.openPlugin(StubAscii8("UI"), type.substring(Range(4)));
    if (err) return NULL;

    UISystemConstructor ctor = 
      (UISystemConstructor)lib.symbol(StubAscii8("createUISystem"));
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
  if (type == StubAscii8("UI")) type = detectUI();

  return application_local->createEventLoop(type);
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_application_init(void)
{
  Fog::application_local.init();

  Fog::application_local->addEventLoopType
    <Fog::EventLoopDefault>(Fog::StubAscii8("Default"));

#if defined(FOG_OS_WINDOWS)
  Fog::application_local->addEventLoopType
    <Fog::EventLoopWinUI>(Fog::StubAscii8("UI::Windows"));
  Fog::application_local->addEventLoopType
    <Fog::EventLoopWinIO>(Fog::StubAscii8("IO::Windows"));
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
  Fog::application_local->addEventLoopType
    <Fog::EventLoopLibevent>(Fog::StubAscii8("IO::LibEvent"));
#endif // FOG_OS_POSIX

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_application_shutdown(void)
{
  Fog::application_local.destroy();
}
