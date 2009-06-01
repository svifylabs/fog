// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// [Guard]
#ifndef _FOG_CORE_EVENTLOOP_WIN_H
#define _FOG_CORE_EVENTLOOP_WIN_H

#include <Fog/Build/Build.h>

// [Windows Specific]
#if defined(FOG_OS_WINDOWS)

#include <windows.h>

#include <Fog/Core/EventLoop.h>
#include <Fog/Core/List.h>
#include <Fog/Core/ObserverList.h>
#include <Fog/Core/ScopedHandle.h>
#include <Fog/Core/Time.h>

namespace Fog {

// ============================================================================
// [Fog::EventPumpWin]
// ============================================================================

//! EventPumpWin implements a "traditional" Windows message pump. It contains
//! a nearly infinite loop that peeks out messages, and then dispatches them.
//! Intermixed with those peeks are callouts to DoWork for pending tasks,
//! DoDelayedWork for pending timers, and OnObjectSignaled for signaled objects.
//! When there are no events to be serviced, this pump goes into a wait state.
//! In most cases, this message pump handles all processing.
//!
//! However, when a task, or windows event, invokes on the stack a native dialog
//! box or such, that window typically provides a bare bones (native?) message
//! pump.  That bare-bones message pump generally supports little more than a
//! peek of the Windows message queue, followed by a dispatch of the peeked
//! message.  EventLoop extends that bare-bones message pump to also service
//! Tasks, at the cost of some complexity.
//!
//! The basic structure of the extension (refered to as a sub-pump) is that a
//! special message, kMsgHaveWork, is repeatedly injected into the Windows
//! Message queue.  Each time the kMsgHaveWork message is peeked, checks are
//! made for an extended set of events, including the availability of Tasks to
//! run.
//!
//! After running a task, the special message kMsgHaveWork is again posted to
//! the Windows Message queue, ensuring a future time slice for processing a
//! future event.  To prevent flooding the Windows Message queue, care is taken
//! to be sure that at most one kMsgHaveWork message is EVER pending in the
//! Window's Message queue.
//!
//! There are a few additional complexities in this system where, when there are
//! no Tasks to run, this otherwise infinite stream of messages which drives the
//! sub-pump is halted.  The pump is automatically re-started when Tasks are
//! queued.
//!
//! A second complexity is that the presence of this stream of posted tasks may
//! prevent a bare-bones message pump from ever peeking a WM_PAINT or WM_TIMER.
//! Such paint and timer events always give priority to a posted message, such as
//! kMsgHaveWork messages.  As a result, care is taken to do some peeking in
//! between the posting of each kMsgHaveWork message (i.e., after kMsgHaveWork
//! is peeked, and before a replacement kMsgHaveWork is posted).
//!
//! NOTE: Although it may seem odd that messages are used to start and stop this
//! flow (as opposed to signaling objects, etc.), it should be understood that
//! the native message pump will *only* respond to messages.  As a result, it is
//! an excellent choice.  It is also helpful that the starter messages that are
//! placed in the queue when new task arrive also awakens DoRunLoop.
struct FOG_API EventPumpWin : public EventPump
{
  //! An Observer is an object that receives global notifications from the
  //! EventLoop.
  //!
  //! @note An Observer implementation should be extremely fast!
  struct FOG_API Observer
  {
  public:
    Observer();
    virtual ~Observer();

    //! This method is called before processing a message.
    //! The message may be undefined in which case msg.message is 0
    virtual void willProcessMessage(const MSG& msg) = 0;

    //! This method is called when control returns from processing a UI message.
    //! The message may be undefined in which case msg.message is 0
    virtual void didProcessMessage(const MSG& msg) = 0;
  };

  //! Dispatcher is used during a nested invocation of Run to dispatch events.
  //! If run() is invoked with a non-NULL Dispatcher, EventLoop does not
  //! dispatch events (or invoke TranslateMessage), rather every message is
  //! passed to Dispatcher's Dispatch method for dispatch. It is up to the
  //! Dispatcher to dispatch(), or not, the event.
  //!
  //! The nested loop is exited by either posting a quit, or returning false
  //! from Dispatch.
  struct FOG_API Dispatcher
  {
  public:
    Dispatcher();
    virtual ~Dispatcher();
    //! Dispatches the event. If true is returned processing continues as
    //! normal. If false is returned, the nested loop exits immediately.
    virtual bool dispatch(const MSG& msg) = 0;
  };

  EventPumpWin(const String32& name);
  virtual ~EventPumpWin();

  //! @brief Add an Observer, which will start receiving notifications 
  //! immediately.
  void addObserver(Observer* observer);

  //! @brief Remove an Observer. It is safe to call this method while an 
  //! observer is receiving a notification callback.
  void removeObserver(Observer* observer);

  //! @brief Give a chance to code processing additional messages to notify 
  //! the message loop observers that another message has been processed.
  void willProcessMessage(const MSG& msg);

  //! @sa @c willProcessMessage().
  void didProcessMessage(const MSG& msg);

  //! @brief Applications can call this to encourage us to process all pending 
  //! WM_PAINT messages.  This method will process all paint messages the 
  //! Windows Message queue can provide, up to some fixed number (to avoid any 
  //! infinite loops).
  void pumpOutPendingPaintMessages();

  //! Like EventPump::Run, but MSG objects are routed through dispatcher.
  void runWithDispatcher(Delegate* delegate, Dispatcher* dispatcher);

  // EventPump methods:
  virtual void run(Delegate* delegate) { runWithDispatcher(delegate, NULL); }
  virtual void quit();
  virtual void scheduleWork();
  virtual void scheduleDelayedWork(const Time& delayedWorkTime);

protected:
  static LRESULT CALLBACK WndProcThunk(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

  struct RunState
  {
    Delegate* delegate;
    Dispatcher* dispatcher;

    //! @brief Used to flag that the current run() invocation should return 
    //! ASAP.
    bool shouldQuit;

    //! @brief Used to count how many run() invocations are on the stack.
    int runDepth;
  };

  virtual void doRunLoop() = 0;
  void initMessageWnd();
  void handleWorkMessage();
  void handleTimerMessage();
  bool processNextWindowsMessage();
  bool processMessageHelper(const MSG& msg);
  bool processPumpReplacementMessage();
  int getCurrentDelay() const;

  //! @brief A hidden message-only window.
  HWND _messageHwnd;

  //! @brief List of observers.
  ObserverList<Observer> _observers;

  //! @brief The time at which delayed work should run.
  Time _delayedWorkTime;

  //! @brief A boolean value used to indicate if there is a kMsgDoWork message 
  //! pending in the Windows Message queue.  There is at most one such message,
  //! and it can drive execution of tasks when a native message pump is running.
  LONG _haveWork;

  //! @brief State for the current invocation of run().
  RunState* _state;
};

// ============================================================================
// [Fog::EventPumpWinUI]
// ============================================================================

// EventPumpWinUI extends EventPumpWin with methods that are particular to a
// EventLoop instantiated with TypeUI.
struct FOG_API EventPumpWinUI : public EventPumpWin
{
  EventPumpWinUI();
  virtual ~EventPumpWinUI();

private:
  virtual void doRunLoop();
  void waitForWork();
};

// ============================================================================
// [Fog::EventLoopWinUI]
// ============================================================================

//! EventLoopForUI extends EventLoop with methods that are particular to a
//! EventLoop instantiated with TypeUI.
//!
//! This class is typically used like so:
//!   Fog::EventLoopWinUI::current()->...call some method...
struct FOG_API EventLoopWinUI : public EventLoop
{
public:
  EventLoopWinUI();

  typedef EventPumpWin::Dispatcher Dispatcher;
  typedef EventPumpWin::Observer Observer;

  // Please see EventPumpWin for definitions of these methods.
  void run(Dispatcher* dispatcher);
  void addObserver(Observer* observer);
  void removeObserver(Observer* observer);
  void willProcessMessage(const MSG& message);
  void didProcessMessage(const MSG& message);
  void pumpOutPendingPaintMessages();
};

// ============================================================================
// [Fog::EventPumpWinIO]
// ============================================================================

// EventPumpWinIO extends EventPumpWin with methods that are particular to a
// EventLoop instantiated with TypeIO.
struct FOG_API EventPumpWinIO : public EventPumpWin
{
  // Used with WatchObject to asynchronously monitor the signaled state of a
  // HANDLE object.
  struct FOG_API Watcher
  {
  public:
    Watcher();
    virtual ~Watcher();

    // Called from EventLoop::Run when a signalled object is detected.
    virtual void onObjectSignaled(HANDLE object) = 0;
  };

  // Clients interested in receiving OS notifications when asynchronous IO
  // operations complete should implement this interface and register themselves
  // with the message pump.
  struct FOG_API IOHandler
  {
  public:
    IOHandler();
    virtual ~IOHandler();

    // This will be called once the pending IO operation associated with
    // |context| completes. |error| is the Win32 error code of the IO operation
    // (ERROR_SUCCESS if there was no error). |bytesTransfered| will be zero
    // on error.
    virtual void onIOCompleted(OVERLAPPED* context, DWORD bytesTransfered, DWORD error) = 0;
  };

  EventPumpWinIO();
  virtual ~EventPumpWinIO();

  // Have the current thread's message loop watch for a signaled object.
  // Pass a null watcher to stop watching the object.
  void watchObject(HANDLE, Watcher*);

  // Register the handler to be used when asynchronous IO for the given file
  // completes. The registration persists as long as |fileHandle| is valid, so
  // |handler| must be valid as long as there is pending IO for the given file.
  void registerIOHandler(HANDLE fileHandle, IOHandler* handler);

  // This is just a throw away function to ease transition to completion ports.
  // Pass NULL for handler to stop tracking this request. WARNING: cancellation
  // correctness is the responsibility of the caller. |context| must contain a
  // valid manual reset event, but the caller should not interact directly with
  // it. The registration can live across a single IO operation, or it can live
  // across multiple IO operations without having to reset it after each IO
  // completion callback. Internally, there will be a WatchObject registration
  // alive as long as this context registration is in effect. It is an error
  // to unregister a context that has not been registered before.
  void registerIOContext(OVERLAPPED* context, IOHandler* handler);

private:
  virtual void doRunLoop();
  void waitForWork();
  bool processNextObject();
  bool signalWatcher(sysuint_t objectIndex);

  // A vector of objects (and corresponding watchers) that are routinely
  // serviced by this message pump.
  List<HANDLE> _objects;
  List<Watcher*> _watchers;

  // The completion port associated with this thread.
  ScopedHANDLE _port;
};

// ============================================================================
// [Fog::EventLoopWinIO]
// ============================================================================

// Fog::EventLoopWinIO extends Fog::EventLoop with methods that are particular 
// to a Fog::EventLoop instantiated with TypeIO.
//
// This class is typically used like so:
//   Fog::EventLoopWinIO::current()->...call some method...
struct FOG_API EventLoopWinIO : public EventLoop
{
public:
  EventLoopWinIO();

  typedef EventPumpWinIO::Watcher Watcher;
  typedef EventPumpWinIO::IOHandler IOHandler;

  // Please see EventPumpWin for definitions of these methods.
  void watchObject(HANDLE object, Watcher* watcher);
  void registerIOHandler(HANDLE file_handle, IOHandler* handler);
  void registerIOContext(OVERLAPPED* context, IOHandler* handler);
};

} // Fog namespace

#endif // FOG_OS_WINDOWS

// [Guard]
#endif // _FOG_CORE_EVENTLOOP_WIN_H
