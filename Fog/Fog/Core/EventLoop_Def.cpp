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

#include <Fog/Core/EventLoop_Def.h>
#include <Fog/Core/Time.h>

namespace Fog {

// ============================================================================
// [Fog::EventPumpDefault]
// ============================================================================

EventPumpDefault::EventPumpDefault() : 
  EventPump(Ascii8("Default")),
  _keepRunning(true),
  _event(false, false)
{
}

EventPumpDefault::~EventPumpDefault()
{
}

void EventPumpDefault::run(EventPump::Delegate* delegate)
{
  FOG_ASSERT(_keepRunning);

  for (;;)
  {
    bool didWork = delegate->doWork();
    if (!_keepRunning) break;

    didWork |= delegate->doDelayedWork(&_delayedWorkTime);
    if (!_keepRunning) break;

    if (didWork) continue;

    didWork = delegate->doIdleWork();
    if (!_keepRunning) break;

    if (didWork) continue;

    if (_delayedWorkTime.isNull())
    {
      _event.wait();
    }
    else
    {
      TimeDelta delay = _delayedWorkTime - Time::now();
      if (delay > TimeDelta())
        _event.timedWait(delay);
      else
        // It looks like delayedWorkTime indicates a time in the past, so we
        // need to call doDelayedWork now.
        _delayedWorkTime = Time();
    }
    // Since _event is auto-reset, we don't need to do anything special here
    // other than service each delegate method.
  }

  _keepRunning = true;
}

void EventPumpDefault::quit()
{
  _keepRunning = false;
}

void EventPumpDefault::scheduleWork()
{
  // Since this can be called on any thread, we need to ensure that our Run
  // loop wakes up.
  _event.signal();
}

void EventPumpDefault::scheduleDelayedWork(const Time& delayedWorkTime)
{
  // We know that we can't be blocked on Wait right now since this method can
  // only be called on the same thread as Run, so we only need to update our
  // record of how long to sleep when we do sleep.
  _delayedWorkTime = delayedWorkTime;
}

// ============================================================================
// [Fog::EventLoopDefault]
// ============================================================================

EventLoopDefault::EventLoopDefault() : 
  EventLoop(new EventPumpDefault())
{
}

} // Fog namespace
