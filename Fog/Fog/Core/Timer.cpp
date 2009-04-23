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
#include <Fog/Core/EventLoop.h>
#include <Fog/Core/Timer.h>

FOG_IMPLEMENT_OBJECT(Fog::Timer)

namespace Fog {

// ============================================================================
// [Fog::TimerTask]
// ============================================================================

struct FOG_HIDDEN TimerTask : public Task
{
  TimerTask(Timer* timer) : 
    timer(timer)
  {
  }

  virtual void run()
  {
    if (timer)
    {
      // First send event
      TimerEvent e(timer);
      timer->sendEvent(&e);
      
      // Repeat it
      if (timer)
      {
        timer->_task = NULL;
      }
    }
  }

  Timer* timer;
};

// ============================================================================
// [Fog::Timer]
// ============================================================================

Timer::Timer() :
  _task(NULL)
{
}

Timer::~Timer()
{
  stop();
}

bool Timer::isRunning()
{
  return _task != NULL;
}

bool Timer::start()
{
  stop();

  _task = new TimerTask(this);
  thread()->eventLoop()->postDelayedTask(
    _task,
    static_cast<int>(_interval.inMilliseconds()));
  return true;
}

void Timer::stop()
{
  if (_task)
  {
    static_cast<TimerTask *>(_task)->timer = NULL;
    _task = NULL;
  }
}

void Timer::setInterval(TimeDelta interval)
{
  if (_interval != interval)
  {
    _interval = interval;
  }
}

void Timer::onTimer(TimerEvent* e)
{
  // Nothing...
}

} // Fog namespace
