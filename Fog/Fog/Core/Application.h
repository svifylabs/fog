// [Fog/Core Library - C++ API]
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
#include <Fog/Core/Class.h>
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
FOG_API void fog_application_initArguments(int argc, char* argv[]);

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
  //! @brief Application constructor.
  Application(const String& type);
  //! @brief Application constructor that allows to set argc and argv[].
  Application(const String& type, int argc, char* argv[]);

private:
  void _init(const String& type);

public:
  virtual ~Application();

  virtual err_t run();
  virtual void quit();

  //! @brief Return application event loop (can be NULL).
  FOG_INLINE EventLoop* getEventLoop() const { return _eventLoop; }

  //! @brief Return application UI system (can be NULL).
  FOG_INLINE UISystem* getUiSystem() const { return _uiSystem; }

  // [Application Executable / Arguments]

  static String getApplicationExecutable();
  static Vector<String> getApplicationArguments();

  // [Working Directory]

  static err_t getWorkingDirectory(String& dir);
  static err_t setWorkingDirectory(const String& dir);

  // [Add / Remove Event Loop]

  typedef EventLoop* (*EventLoopConstructor)();

  static bool addEventLoopType(const String& type, EventLoopConstructor ctor);
  static bool removeEventLoopType(const String& type);

  template<typename EventLoopT>
  struct _EventLoopCtorHelper
  {
    static EventLoop* ctor() { return new EventLoopT(); }
  };

  template<typename EventLoopT>
  static FOG_INLINE bool addEventLoopTypeT(const String& type)
  {
    return addEventLoopType(type, _EventLoopCtorHelper<EventLoopT>::ctor);
  }

  // [UI / UISystem]

  static FOG_INLINE Application* getInstance() { return _instance; }

  static String detectUI();
  static UISystem* createUISystem(const String& type);
  static EventLoop* createEventLoop(const String& type);

  // [Members]

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
