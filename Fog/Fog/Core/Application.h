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
  Application(const String32& type);
  virtual ~Application();

  virtual err_t run();
  virtual void quit();

  static FOG_INLINE Application* instance() { return _instance; }

  //! @brief Return application event loop (can be NULL).
  FOG_INLINE EventLoop* eventLoop() const { return _eventLoop; }

  //! @brief Return application UI system (can be NULL).
  FOG_INLINE UISystem* uiSystem() const { return _uiSystem; }

  static String32 detectUI();
  static UISystem* createUISystem(const String32& type);
  static EventLoop* createEventLoop(const String32& type);

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
