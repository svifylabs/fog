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
#include <Fog/Core/FileUtil.h>
#include <Fog/Core/Hash.h>
#include <Fog/Core/Library.h>
#include <Fog/Core/Misc.h>
#include <Fog/Core/Object.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/String.h>
#include <Fog/Core/Thread.h>
#include <Fog/Core/WinUtil.h>
#include <Fog/UI/UISystem.h>

#if defined(FOG_OS_WINDOWS)
#include <Fog/Core/EventLoop_Win.h>
#include <Fog/UI/UISystem_Win.h>
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX) && defined(FOG_BUILD_MODULE_X11_INTERNAL)
#include <Fog/UI/UISystem_X11.h>
#endif // FOG_OS_POSIX

#if defined(FOG_OS_WINDOWS)
// windows.h is already included in Fog/Build/Build.h
#include <io.h>
#else
#include <errno.h>
#if defined(FOG_HAVE_UNISTD_H)
#include <unistd.h>
#endif
#endif

namespace Fog {

// ============================================================================
// [Fog::Application - Local]
// ============================================================================

struct FOG_HIDDEN Application_Local
{
  typedef Hash<String32, Application::EventLoopConstructor> ELHash;

  Lock lock;
  ELHash elHash;

  String32 applicationExecutable;
  String32 applicationDirectory;
  String32 applicationBaseName;
  String32 applicationCommand;

  Application_Local();
  ~Application_Local();

  EventLoop* createEventLoop(const String32& type)
  {
    AutoLock locked(lock);
    Application::EventLoopConstructor ctor = elHash.value(type, NULL);
    return ctor ? ctor() : NULL;
  }
};

Application_Local::Application_Local()
{
#if defined(FOG_OS_WINDOWS)
  WCHAR* cmdLine = GetCommandLineW();
  applicationCommand.set(StubW(cmdLine));
  applicationCommand.slashesToPosix();
  applicationCommand.squeeze();

  //WinUtil::getModuleFileName(NULL, applicationExecutable);
  int count;
  WCHAR** args = CommandLineToArgvW(cmdLine, &count);

  applicationExecutable.set(StubW(args[0]));
  applicationExecutable.slashesToPosix();
  applicationExecutable.squeeze();

  FileUtil::extractDirectory(applicationDirectory, applicationExecutable);
  FileUtil::extractFile(applicationBaseName, applicationExecutable);
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
  // TODO
#endif // FOG_OS_POSIX

  // Application directory usually contains plugins and library itself under
  // Windows, but we will add it also for posix OSes. It can help if application
  // is started from user home directory.
  Library::addPath(applicationDirectory, Library::PathPrepend);
}

Application_Local::~Application_Local()
{
}

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
  if (type.startsWith(Ascii8("UI"))) _uiSystem = createUISystem(type);

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

    // Unset main thread event loop (this is safe, we will destroy it later
    // if it exists).
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

// ============================================================================
// [Fog::Application - Application Directory]
// ============================================================================

err_t Application::getApplicationExecutable(String32& dst)
{
  return dst.set(application_local->applicationExecutable);
}

err_t Application::getApplicationBaseName(String32& dst)
{
  return dst.set(application_local->applicationBaseName);
}

err_t Application::getApplicationDirectory(String32& dst)
{
  return dst.set(application_local->applicationDirectory);
}

err_t Application::getApplicationCommand(String32& dst)
{
  return dst.set(application_local->applicationCommand);
}

// ============================================================================
// [Fog::Application - Working Directory]
// ============================================================================

#if defined(FOG_OS_WINDOWS)
err_t Application::getWorkingDirectory(String32& dst)
{
  err_t err;
  TemporaryString16<TemporaryLength> dirW;

  for (;;)
  {
    DWORD size = GetCurrentDirectoryW(dirW.capacity()+1, dirW.mStrW());
    if (size >= dirW.capacity())
    {
      if ( (err = dirW.reserve(size)) ) return err;
      continue;
    }
    else
    {
      if ((err = dst.set(dirW)) ) return err;
      return dst.slashesToPosix();
    }
  }
}

err_t Application::setWorkingDirectory(const String32& dir)
{
  err_t err;
  TemporaryString16<TemporaryLength> dirW;

  if ((err = dirW.set(dir)) ||
      (err = dirW.slashesToWin()))
  {
    return err;
  }

  if (SetCurrentDirectoryW(dirW.cStrW()) == 0)
    return Error::Ok;
  else
    return GetLastError();
}
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
err_t Application::getWorkingDirectory(String32& dst)
{
  err_t err;
  TemporaryString8<TemporaryLength> dir8;

  dst.clear();
  for (;;)
  {
    const char* ptr = ::getcwd(dir8.mStr(), dir8.capacity()+1);
    if (ptr)
    {
      dst.set(Local8(ptr));
      return Error::Ok;
    }
    if (errno != ERANGE) return errno;

    // Alloc more...
    if ((err = dir8.reserve(dir8.capacity() + 4096))) return err;
  }
}

err_t Application::setWorkingDirectory(const String32& dir)
{
  err_t err;
  TemporaryString8<TemporaryLength> dir8;

  if ( (err = dir8.set(dir, TextCodec::local8())) ) return err;

  if (::chdir(dir8.cStr()) == 0)
    return Error::Ok;
  else
    return errno;
}
#endif // FOG_OS_POSIX

// ============================================================================
// [Fog::Application - Add / Remove Event Loop]
// ============================================================================

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

// ============================================================================
// [Fog::Application - UI / UISystem]
// ============================================================================

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
