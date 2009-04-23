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

#include <Fog/Build/Build.h>

// [Unix Specific]
#if defined(FOG_OS_POSIX)

#include <Fog/Core/EventLoop_libevent.h>

#include <event.h>
#include <fcntl.h>
#include <stdio.h>

#if defined(FOG_HAVE_UNISTD_H)
#include <unistd.h>
#endif // FOG_HAVE_UNISTD_H

namespace Fog {

// ============================================================================
// [Fog::EventPumpLibevent]
// ============================================================================

// Return 0 on success
// Too small a function to bother putting in a library?
static int setNonBlocking(int fd)
{
  int flags = fcntl(fd, F_GETFL, 0);
  if (-1 == flags) flags = 0;
  return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// Called if a byte is received on the wakeup pipe.
void EventPumpLibevent::onWakeup(int socket, short flags, void* context)
{
  EventPumpLibevent* that = static_cast<EventPumpLibevent*>(context);
  FOG_ASSERT(that->_wakeupPipeOut == socket);

  // Remove and discard the wakeup byte.
  char buf;
  int nread = read(socket, &buf, 1);
  FOG_ASSERT(nread == 1);

  // Tell libevent to break out of inner loop.
  event_base_loopbreak(that->_eventBase);
}

EventPumpLibevent::EventPumpLibevent() : 
  EventPump(StubAscii8("Libevent")),
  _keepRunning(true),
  _inRun(false),
  _eventBase(event_base_new()),
  _wakeupPipeIn(-1),
  _wakeupPipeOut(-1)
{
  if (!init())
     FOG_ASSERT_NOT_REACHED();
}

bool EventPumpLibevent::init()
{
  int fds[2];
  if (pipe(fds))
    return false;
  if (setNonBlocking(fds[0]))
    return false;
  if (setNonBlocking(fds[1]))
    return false;
  _wakeupPipeOut = fds[0];
  _wakeupPipeIn = fds[1];

  _wakeupEvent = new event;
  event_set(_wakeupEvent, _wakeupPipeOut, EV_READ | EV_PERSIST, onWakeup, this);
  event_base_set(_eventBase, _wakeupEvent);

  if (event_add(_wakeupEvent, 0))
    return false;
  else
    return true;
}

EventPumpLibevent::~EventPumpLibevent()
{
  FOG_ASSERT(_wakeupEvent);
  FOG_ASSERT(_eventBase);
  event_del(_wakeupEvent);
  delete _wakeupEvent;
  event_base_free(_eventBase);
}

void EventPumpLibevent::watchSocket(int socket, short interestMask, event* e, Watcher* watcher)
{

  // Set current interest mask and message pump for this event
  event_set(e, socket, interestMask, onReadinessNotification, watcher);

  // Tell libevent which message pump this socket will belong to when we add it.
  event_base_set(_eventBase, e);

  // Add this socket to the list of monitored sockets.
  if (event_add(e, NULL))
    FOG_ASSERT_NOT_REACHED();
}

void EventPumpLibevent::unwatchSocket(event* e)
{
  // Remove this socket from the list of monitored sockets.
  if (event_del(e))
    FOG_ASSERT_NOT_REACHED();
}

void EventPumpLibevent::onReadinessNotification(int socket, short flags, void* context)
{
  // The given socket is ready for I/O.
  // Tell the owner what kind of I/O the socket is ready for.
  Watcher* watcher = static_cast<Watcher*>(context);
  watcher->onSocketReady(flags);
}

// Reentrant!
void EventPumpLibevent::run(Delegate* delegate)
{
  FOG_ASSERT(_keepRunning);

  bool oldInRun = _inRun;
  _inRun = true;

  for (;;) {
    // ScopedNSAutoreleasePool autorelease_pool;

    bool didWork = delegate->doWork();
    if (!_keepRunning)
      break;

    didWork |= delegate->doDelayedWork(&_delayedWorkTime);
    if (!_keepRunning)
      break;

    if (didWork)
      continue;

    didWork = delegate->doIdleWork();
    if (!_keepRunning)
      break;

    if (didWork)
      continue;

    // EVLOOP_ONCE tells libevent to only block once,
    // but to service all pending events when it wakes up.
    if (_delayedWorkTime.isNull())
    {
      event_base_loop(_eventBase, EVLOOP_ONCE);
    }
    else
    {
      TimeDelta delay = _delayedWorkTime - Time::now();
      if (delay > TimeDelta())
      {
        struct timeval poll_tv;
        poll_tv.tv_sec = delay.inSeconds();
        poll_tv.tv_usec = delay.inMicroseconds() % Time::MicrosecondsPerSecond;
        event_base_loopexit(_eventBase, &poll_tv);
        event_base_loop(_eventBase, EVLOOP_ONCE);
      }
      else
      {
        // It looks like _delayedWorkTime indicates a time in the past, so we
        // need to call doDelayedWork now.
        _delayedWorkTime = Time();
      }
    }
  }

  _keepRunning = true;
  _inRun = oldInRun;
}

void EventPumpLibevent::quit()
{
  FOG_ASSERT(_inRun);
  // Tell both libevent and run() that they should break out of their loops.
  _keepRunning = false;
  scheduleWork();
}

void EventPumpLibevent::scheduleWork()
{
  // Tell libevent (in a threadsafe way) that it should break out of its loop.
  char buf = 0;
  int nwrite = write(_wakeupPipeIn, &buf, 1);
  FOG_ASSERT(nwrite == 1);
}

void EventPumpLibevent::scheduleDelayedWork(const Time& delayedWorkTime)
{
  // We know that we can't be blocked on Wait right now since this method can
  // only be called on the same thread as Run, so we only need to update our
  // record of how long to sleep when we do sleep.
  _delayedWorkTime = delayedWorkTime;
}

// ============================================================================
// Fog::EventLoopLibevent
// ============================================================================

EventLoopLibevent::EventLoopLibevent() :
  EventLoop(new EventPumpLibevent()) 
{
}

void EventLoopLibevent::watchSocket(
  int socket, short interest_mask, 
  struct event* e, Watcher* watcher)
{
  static_cast<EventPumpLibevent*>(_pump.get())->watchSocket(socket, interest_mask, e, watcher);
}

void EventLoopLibevent::unwatchSocket(struct event* e)
{
  static_cast<EventPumpLibevent*>(_pump.get())->unwatchSocket(e);
}

} // Fog namespace

#endif // FOG_OS_POSIX
