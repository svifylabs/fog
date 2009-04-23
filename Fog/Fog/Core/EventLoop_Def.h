// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// [Guard]
#ifndef _FOG_CORE_EVENTLOOP_DEF_H
#define _FOG_CORE_EVENTLOOP_DEF_H

// [Dependencies]
#include <Fog/Core/EventLoop.h>
#include <Fog/Core/ThreadEvent.h>
#include <Fog/Core/Time.h>

//! @addtogroup Fog_Core
//! @{

namespace Fog {

// ============================================================================
// [Fog::EventPumpDefault]
// ============================================================================

//! @brief Default event pump that not uses operating system (or UI system)
//! dependent event loop.
struct FOG_API EventPumpDefault : public EventPump
{
public:
  EventPumpDefault();
  virtual ~EventPumpDefault();

  // EventPump methods:
  virtual void run(Delegate* delegate);
  virtual void quit();
  virtual void scheduleWork();
  virtual void scheduleDelayedWork(const Time& delayedWorkTime);

private:
  //! @brief This flag is set to false when Run should return.
  bool _keepRunning;

  //! @brief Used to sleep until there is more work to do.
  ThreadEvent _event;

  //! @brief The time at which we should call @c scheduleDelayedWork().
  Time _delayedWorkTime;

  FOG_DISABLE_COPY(EventPumpDefault)
};

// ============================================================================
// [Fog::EventLoopDefault]
// ============================================================================

struct FOG_API EventLoopDefault : public EventLoop
{
  EventLoopDefault();
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_CORE_EVENTLOOP_DEF_H
