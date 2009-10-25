// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Application.h>
#include <Fog/Core/Assert.h>
#include <Fog/Core/AutoLock.h>
#include <Fog/Core/ByteArray.h>
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
#include <Fog/Core/TextCodec.h>
#include <Fog/Core/Thread.h>
#include <Fog/Core/WinUtil.h>
#include <Fog/UI/UISystem.h>

#if defined(FOG_OS_WINDOWS)
#include <Fog/Core/EventLoop_Win.h>
#include <Fog/UI/UISystem_Win.h>
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX) && defined(FOG_BUILD_MODULE_X11_INTERNAL)
#include <Fog/UI/UISystem_X11.h>
#include <unistd.h>
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

static void unescapeArgument(String& s)
{
  Char* beg = s.mData();
  Char* cur = beg;
  Char* dst = beg;

  sysuint_t remain = s.getLength();

  while (remain)
  {
    if (cur[0] == Char('\\') && remain > 1 && (cur[1] == Char('\"') ||
                                                 cur[1] == Char('\'') ||
                                                 cur[1] == Char(' ')) )
    {
      cur++;
      remain--;
    }
    *dst++ = *cur++;
    remain--;
  }

  s.resize((sysuint_t)(dst - beg));
}

#if defined(FOG_OS_WINDOWS)
static void parseWinCmdLine(const String& cmdLine, Vector<String>& dst)
{
  const Char* cur = cmdLine.cData();
  const Char* end = cur + cmdLine.getLength();

  const Char* mark;
  Char quote;
  bool isEscaped;
  sysuint_t len;

  for (;;)
  {
    // Skip spaces.
    for (;;) {
      if (cur == end) goto end;
      if (cur->isSpace()) cur++;
      else break;
    }
    
    // Zero character means end.
    if (cur->ch() == 0) goto end;

    // Parse quote character (if it's here).
    mark = cur;
    quote = 0;

    if (cur[0] == Char('\"') ||
        cur[0] == Char('\'') )
    {
      quote = cur[0];
    }
    cur++;

    // Parse argument
    isEscaped = false;

    for (;;)
    {
      if (cur == end) goto parsed;

      Char c = cur[0];
      if (c.ch() == 0) goto parsed;
      if (c.isSpace() && quote.ch() == 0) goto parsed;

      // Quotes.
      if (c == quote)
      {
        cur++;
        goto parsed;
      }

      // Escape sequence.
      if (c == Char('\\'))
      {
        if (cur < end - 1 && (cur[1] == Char('\"') || 
                              cur[1] == Char('\'') || 
                              cur[1] == Char(' ')) )
        {
          cur++;
          isEscaped = true;
        }
      }

      cur++;
    }

parsed:
    len = (sysuint_t)(cur - mark);
    if (quote.ch() != 0)
    {
      mark++;
      len--;
      if (len && mark[len-1] == quote) len--;
    }

    {
      String arg(mark, len);
      if (isEscaped) unescapeArgument(arg);
      dst.append(arg);
    }
  }

end:
  return;
}
#endif // FOG_OS_WINDOWS

struct FOG_HIDDEN Application_Local
{
  typedef Hash<String, Application::EventLoopConstructor> ELHash;

  Lock lock;
  ELHash elHash;

  String applicationExecutable;
  Vector<String> applicationArguments;

  Application_Local();
  ~Application_Local();

  EventLoop* createEventLoop(const String& type);
  void applicationArgumentsWasSet();
};

Application_Local::Application_Local()
{
#if defined(FOG_OS_WINDOWS)
  String applicationCommand;

  applicationCommand.set(StubW(::GetCommandLineW()));
  parseWinCmdLine(applicationCommand, applicationArguments);

  applicationArgumentsWasSet();
#endif // FOG_OS_WINDOWS
}

Application_Local::~Application_Local()
{
}

EventLoop* Application_Local::createEventLoop(const String& type)
{
  AutoLock locked(lock);

  Application::EventLoopConstructor ctor = elHash.value(type, NULL);
  return ctor ? ctor() : NULL;
}

void Application_Local::applicationArgumentsWasSet()
{
  applicationExecutable = applicationArguments.cAt(0);
  FileUtil::toAbsolutePath(applicationExecutable, String(), applicationExecutable);

  String applicationDirectory;
  FileUtil::extractDirectory(applicationDirectory, applicationExecutable);

  // Application directory usually contains plugins and library itself under
  // Windows, but we will add it also for posix OSes. It can help if application
  // is started from user home directory.
  Library::addPath(applicationDirectory, Library::PathPrepend);
}

static Static<Application_Local> application_local;

// ============================================================================
// [Fog::Application]
// ============================================================================

Application* Application::_instance = NULL;

Application::Application(const String& type)
{
  _init(type);
}

Application::Application(const String& type, int argc, char* argv[])
{
  fog_application_initArguments(argc, argv);

  _init(type);
}

void Application::_init(const String& type)
{
  _eventLoop = NULL;
  _uiSystem = NULL;

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
// [Fog::Application - Application Executable / Arguments]
// ============================================================================

String Application::getApplicationExecutable()
{
  return application_local->applicationExecutable;
}

Vector<String> Application::getApplicationArguments()
{
  return application_local->applicationArguments;
}

// ============================================================================
// [Fog::Application - Working Directory]
// ============================================================================

#if defined(FOG_OS_WINDOWS)
err_t Application::getWorkingDirectory(String& dst)
{
  err_t err;

  dst.prepare(256);
  for (;;)
  {
    DWORD size = GetCurrentDirectoryW(dst.getCapacity()+1, reinterpret_cast<wchar_t*>(dst.xData()));
    if (size >= dst.getCapacity())
    {
      if ((err = dst.reserve(size))) return err;
      continue;
    }
    else
    {
      return dst.slashesToPosix();
    }
  }
}

err_t Application::setWorkingDirectory(const String& _dir)
{
  err_t err;
  String dir = _dir;

  if ((err = dir.slashesToWin())) return err;

  if (SetCurrentDirectoryW(reinterpret_cast<wchar_t*>(dir.cData())) == 0)
    return Error::Ok;
  else
    return GetLastError();
}
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
err_t Application::getWorkingDirectory(String& dst)
{
  err_t err;
  TemporaryByteArray<TemporaryLength> dir8;

  dst.clear();
  for (;;)
  {
    const char* ptr = ::getcwd(dir8.xData(), dir8.getCapacity() + 1);
    if (ptr) return TextCodec::local8().appendToUnicode(dst, ptr);

    if (errno != ERANGE) return errno;

    // Alloc more...
    if ((err = dir8.reserve(dir8.getCapacity() + 4096))) return err;
  }
}

err_t Application::setWorkingDirectory(const String& dir)
{
  err_t err;
  TemporaryByteArray<TemporaryLength> dir8;

  if ((err = TextCodec::local8().appendFromUnicode(dir8, dir))) return err;

  if (::chdir(dir8.cData()) == 0)
    return Error::Ok;
  else
    return errno;
}
#endif // FOG_OS_POSIX

// ============================================================================
// [Fog::Application - Add / Remove Event Loop]
// ============================================================================

bool Application::addEventLoopType(const String& type, EventLoopConstructor ctor)
{
  AutoLock locked(application_local->lock);
  return application_local->elHash.put(type, ctor);
}

bool Application::removeEventLoopType(const String& type)
{
  AutoLock locked(application_local->lock);
  return application_local->elHash.remove(type);
}

// ============================================================================
// [Fog::Application - UI / UISystem]
// ============================================================================

String Application::detectUI()
{
#if defined(FOG_OS_WINDOWS)
  return Ascii8("UI.Windows");
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
  return Ascii8("UI.X11");
#endif // FOG_OS_POSIX
}

typedef UISystem* (*UISystemConstructor)(void);

UISystem* Application::createUISystem(const String& _type)
{
  String type(_type);

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

    UISystemConstructor ctor = (UISystemConstructor)lib.getSymbol(Ascii8("createUISystem"));
    if (!ctor) return NULL;

    UISystem* uis = ctor();
    if (!uis) return NULL;

    // Success
    uis->_library = lib;
    return uis;
  }
}

EventLoop* Application::createEventLoop(const String &_type)
{
  String type(_type);

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

// ============================================================================
// [Fog::Application - initArguments]
// ============================================================================

void fog_application_initArguments(int argc, char* argv[])
{
  using namespace Fog;

  if (argc < 1) return;

  AutoLock locked(application_local->lock);

  Vector<String>& arguments = application_local->applicationArguments;
  if (arguments.getLength() != 0) return;

  for (int i = 0; i < argc; i++)
  {
    String arg;
    TextCodec::local8().appendToUnicode(arg, argv[i]);
    arguments.append(arg);
  }

  application_local->applicationArgumentsWasSet();
}
