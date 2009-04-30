// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// [Guard]
#ifndef _FOG_CORE_EVENTLOOP_LIBEVENT_H
#define _FOG_CORE_EVENTLOOP_LIBEVENT_H

#include <Fog/Build/Build.h>

// [Unix Specific]
#if defined(FOG_OS_POSIX)

// [Dependencies]
#include <Fog/Core/EventLoop.h>
#include <Fog/Core/ThreadEvent.h>

#include <event.h>

//! @addtogroup Fog_Core
//! @{

namespace Fog {

// ============================================================================
// [Fog::EventPumpLibevent]
// ============================================================================

// Class to monitor sockets and issue callbacks when sockets are ready for I/O
// TODO(dkegel): add support for background file IO somehow
struct EventPumpLibevent : public EventPump
{
 public:
  // Used with WatchObject to asynchronously monitor the I/O readiness of a
  // socket.
  struct Watcher
  {
  public:
    virtual ~Watcher() {}
    // Called from EventLoop::Run when a ready socket is detected.
    virtual void onSocketReady(short eventmask) = 0;
  };

  EventPumpLibevent();
  virtual ~EventPumpLibevent();

  // Have the current thread's message loop watch for a ready socket.
  // Caller must provide a struct event for this socket for libevent's use.
  // The event and interest_mask fields are defined in libevent.
  // Returns true on success.  
  void watchSocket(int socket, short interestMask, event* e, Watcher*);

  // Stop watching a socket.
  // Event was previously initialized by WatchSocket.
  void unwatchSocket(event* e);

  // EventPump methods:
  virtual void run(Delegate* delegate);
  virtual void quit();
  virtual void scheduleWork();
  virtual void scheduleDelayedWork(const Time& delayedWorkTime);

private:

  // Risky part of constructor.  Returns true on success.
  bool init();

  // This flag is set to false when Run should return.
  bool _keepRunning;

  // This flag is set when inside Run.
  bool _inRun;

  // The time at which we should call doDelayedWork.
  Time _delayedWorkTime;

  // Libevent dispatcher.  Watches all sockets registered with it, and sends 
  // readiness callbacks when a socket is ready for I/O.
  event_base* _eventBase;

  // Called by libevent to tell us a registered socket is ready
  static void onReadinessNotification(int socket, short flags, void* context);

  // Unix pipe used to implement ScheduleWork()
  // ... callback; called by libevent inside Run() when pipe is ready to read
  static void onWakeup(int socket, short flags, void* context);
  // ... write end; ScheduleWork() writes a single byte to it 
  int _wakeupPipeIn;
  // ... read end; OnWakeup reads it and then breaks Run() out of its sleep
  int _wakeupPipeOut;
  // ... libevent wrapper for read end
  event* _wakeupEvent;

  FOG_DISABLE_COPY(EventPumpLibevent)
};

// ============================================================================
// [Fog::EventLoopLibevent]
// ============================================================================

// Fog::EventLoopForIO extends Fog::EventLoop with methods that are particular 
// to a Fog::EventLoop instantiated with TypeIO.
//
// This class is typically used like so:
//   Fog::EventLoopForIO::current()->...call some method...
//
struct FOG_API EventLoopLibevent : public EventLoop
{
public:
  EventLoopLibevent();

  typedef EventPumpLibevent::Watcher Watcher;

  // Please see EventPumpLibevent for definitions of these methods.
  void watchSocket(int socket, short interest_mask, struct event* e, Watcher* watcher);
  void unwatchSocket(struct event* e);
};

} // Fog namespace

//! @}

#endif // FOG_OS_POSIX

// [Guard]
#endif // _FOG_CORE_EVENTLOOP_LIBEVENT_H
