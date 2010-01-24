// [Fog/Core Library - Public API]
//
// [Licence]
// MIT, See COPYING file in package

// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// [Guard]
#ifndef _FOG_CORE_APPLICATION_H
#define _FOG_CORE_APPLICATION_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Core/Object.h>
#include <Fog/Core/String.h>

//! @addtogroup Fog_Core
//! @{

//! @brief Function that should be called if application arguments are not
//! initialized and Application object not exists or it's not kwown that
//! it will exist in future.
//!
//! This method is called automatically by @c FOG_CORE_MAIN and
//! @c FOG_UI_MAIN macros.
FOG_API void fog_arguments_init(int argc, char* argv[]);

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct Application;
struct EventLoop;

// Declared in Fog/UI
struct UISystem;

// ============================================================================
// [Fog::Application]
// ============================================================================

struct FOG_API Application : public Object
{
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  //! @brief Application constructor.
  Application(const String& type);
  //! @brief Application constructor that allows to set argc and argv[].
  Application(const String& type, int argc, char* argv[]);

  virtual ~Application();

private:
  void _init(const String& type);

public:
  virtual err_t run();
  virtual void quit();

  static FOG_INLINE Application* getInstance() { return _instance; }

  // --------------------------------------------------------------------------
  // [Application Executable / Arguments]
  // --------------------------------------------------------------------------

  static String getApplicationExecutable();
  static List<String> getApplicationArguments();

  // --------------------------------------------------------------------------
  // [Working Directory]
  // --------------------------------------------------------------------------

  static err_t getWorkingDirectory(String& dir);
  static err_t setWorkingDirectory(const String& dir);

  // --------------------------------------------------------------------------
  // [UISystem - Access]
  // --------------------------------------------------------------------------

  //! @brief Return application UI system (can be NULL).
  FOG_INLINE UISystem* getUiSystem() const { return _uiSystem; }

  static String detectUISystem();
  static UISystem* createUISystem(const String& type);

  // --------------------------------------------------------------------------
  // [UISystem - Register / Unregister]
  // --------------------------------------------------------------------------

  typedef UISystem* (*UISystemConstructor)();

  static bool registerUISystem(const String& type, UISystemConstructor ctor);
  static bool unregisterUISystem(const String& type);

  template<typename UISystemT>
  struct _UISystemCtorHelper
  {
    static UISystem* ctor() { return new(std::nothrow) UISystemT(); }
  };

  template<typename UISystemT>
  static FOG_INLINE bool registerUISystemT(const String& type)
  {
    return registerUISystem(type, _UISystemCtorHelper<UISystemT>::ctor);
  }

  // --------------------------------------------------------------------------
  // [EventLoop - Access]
  // --------------------------------------------------------------------------

  //! @brief Return application event loop (can be NULL).
  FOG_INLINE EventLoop* getEventLoop() const { return _eventLoop; }

  static EventLoop* createEventLoop(const String& type);

  // --------------------------------------------------------------------------
  // [EventLoop - Register / Unregister]
  // --------------------------------------------------------------------------

  typedef EventLoop* (*EventLoopConstructor)();

  static bool registerEventLoop(const String& type, EventLoopConstructor ctor);
  static bool unregisterEventLoop(const String& type);

  template<typename EventLoopT>
  struct _EventLoopCtorHelper
  {
    static EventLoop* ctor() { return new(std::nothrow) EventLoopT(); }
  };

  template<typename EventLoopT>
  static FOG_INLINE bool registerEventLoopT(const String& type)
  {
    return registerEventLoop(type, _EventLoopCtorHelper<EventLoopT>::ctor);
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

protected:
  static Application* _instance;

  EventLoop* _eventLoop;
  UISystem* _uiSystem;

private:
  FOG_DISABLE_COPY(Application)
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_CORE_APPLICATION_H
