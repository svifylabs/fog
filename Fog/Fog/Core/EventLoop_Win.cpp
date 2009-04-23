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

#include <Fog/Core/Assert.h>
#include <Fog/Core/EventLoop_Win.h>
#include <Fog/Core/OS.h>

#if defined(FOG_OS_WINDOWS)

#include <math.h>

namespace Fog {

// ============================================================================
// [Fog::EventPumpWin]
// ============================================================================

static const WCHAR kWndClass[] = L"Fog::EventPumpWin";

// Message sent to get an additional time slice for pumping (processing) 
// another task (a series of such messages creates a continuous task pump).
static const int kMsgHaveWork = WM_USER + 1;

#ifndef NDEBUG
// Force exercise of polling model.
static const int kMaxWaitObjects = 8;
#else
static const int kMaxWaitObjects = MAXIMUM_WAIT_OBJECTS;
#endif

// ============================================================================
// [Fog::EventPumpWin]
// ============================================================================

EventPumpWin::EventPumpWin(const String32& name) : 
  EventPump(name),
  _haveWork(0),
  _state(NULL)
{
  initMessageWnd();
}

EventPumpWin::~EventPumpWin()
{
  ::DestroyWindow(_messageHwnd);
}

void EventPumpWin::addObserver(Observer* observer)
{
  _observers.addObserver(observer);
}

void EventPumpWin::removeObserver(Observer* observer)
{
  _observers.removeObserver(observer);
}

void EventPumpWin::willProcessMessage(const MSG& msg)
{
  FOR_EACH_OBSERVER(Observer, _observers, willProcessMessage(msg));
}

void EventPumpWin::didProcessMessage(const MSG& msg)
{
  FOR_EACH_OBSERVER(Observer, _observers, didProcessMessage(msg));
}

void EventPumpWin::pumpOutPendingPaintMessages()
{
  // If we are being called outside of the context of Run, then don't try to do
  // any work.
  if (!_state) return;

  // Create a mini-message-pump to force immediate processing of only Windows
  // WM_PAINT messages.  Don't provide an infinite loop, but do enough peeking
  // to get the job done.  Actual common max is 4 peeks, but we'll be a little
  // safe here.
  const int kMaxPeekCount = 20;
  bool win2k = OS::windowsVersion() <= OS::Win_2000;
  int peek_count;
  MSG msg;

  for (peek_count = 0; peek_count < kMaxPeekCount; ++peek_count)
  {
    if (win2k)
    {
      if (!PeekMessage(&msg, NULL, WM_PAINT, WM_PAINT, PM_REMOVE)) break;
    }
    else
    {
      if (!PeekMessage(&msg, NULL, 0, 0, PM_REMOVE | PM_QS_PAINT)) break;
    }

    processMessageHelper(msg);

    if (_state->shouldQuit)
      // Handle WM_QUIT.
      break;
  }
}

void EventPumpWin::runWithDispatcher(Delegate* delegate, Dispatcher* dispatcher)
{
  RunState s;

  s.delegate = delegate;
  s.dispatcher = dispatcher;
  s.shouldQuit = false;
  s.runDepth = _state ? _state->runDepth + 1 : 1;

  RunState* previousState = _state;
  _state = &s;

  doRunLoop();

  _state = previousState;
}

void EventPumpWin::quit()
{
  FOG_ASSERT(_state);
  _state->shouldQuit = true;
}

void EventPumpWin::scheduleWork()
{
  if (InterlockedExchange(&_haveWork, 1))
    // Someone else continued the pumping.
    return;

  // Make sure the EventPump does some work for us.
  PostMessage(_messageHwnd, kMsgHaveWork, reinterpret_cast<WPARAM>(this), 0);
}

void EventPumpWin::scheduleDelayedWork(const Time& delayedWorkTime)
{
  // We would *like* to provide high resolution timers.  Windows timers using
  // SetTimer() have a 10ms granularity.  We have to use WM_TIMER as a wakeup
  // mechanism because the application can enter modal windows loops where it
  // is not running our EventLoop; the only way to have our timers fire in
  // these cases is to post messages there.
  //
  // To provide sub-10ms timers, we process timers directly from our run loop.
  // For the common case, timers will be processed there as the run loop does
  // its normal work.  However, we *also* set the system timer so that WM_TIMER
  // events fire.  This mops up the case of timers not being able to work in
  // modal message loops.  It is possible for the SetTimer to pop and have no
  // pending timers, because they could have already been processed by the
  // run loop itself.
  //
  // We use a single SetTimer corresponding to the timer that will expire
  // soonest.  As new timers are created and destroyed, we update SetTimer.
  // Getting a spurrious SetTimer event firing is benign, as we'll just be
  // processing an empty timer queue.
  //
  _delayedWorkTime = delayedWorkTime;

  int delay_msec = getCurrentDelay();
  FOG_ASSERT(delay_msec >= 0);
  if (delay_msec < USER_TIMER_MINIMUM) delay_msec = USER_TIMER_MINIMUM;

  // Create a WM_TIMER event that will wake us up to check for any pending
  // timers (in case we are running within a nested, external sub-pump).
  SetTimer(_messageHwnd, reinterpret_cast<UINT_PTR>(this), delay_msec, NULL);
}

//-----------------------------------------------------------------------------
// EventPumpWin protected:

// static
LRESULT CALLBACK EventPumpWin::WndProcThunk(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
  switch (message)
  {
    case kMsgHaveWork:
      reinterpret_cast<EventPumpWin*>(wparam)->handleWorkMessage();
      break;
    case WM_TIMER:
      reinterpret_cast<EventPumpWin*>(wparam)->handleTimerMessage();
      break;
  }

  return DefWindowProc(hwnd, message, wparam, lparam);
}

void EventPumpWin::initMessageWnd()
{
  HINSTANCE hinst = GetModuleHandle(NULL);

  WNDCLASSEXW wc;
  ZeroMemory(&wc, sizeof(WNDCLASSEX));
  wc.cbSize = sizeof(wc);
  wc.lpfnWndProc = WndProcThunk;
  wc.hInstance = hinst;
  wc.lpszClassName = kWndClass;
  RegisterClassExW(&wc);

  _messageHwnd = CreateWindowExW(0, kWndClass, 0, 0, 0, 0, 0, 0, HWND_MESSAGE, 0, hinst, 0);
  FOG_ASSERT(_messageHwnd);
}

void EventPumpWin::handleWorkMessage()
{
  // If we are being called outside of the context of Run, then don't try to do
  // any work.  This could correspond to a MessageBox call or something of that
  // sort.
  if (!_state)
  {
    // Since we handled a kMsgHaveWork message, we must still update this flag.
    InterlockedExchange(&_haveWork, 0);
    return;
  }

  // Let whatever would have run had we not been putting messages in the queue
  // run now.  This is an attempt to make our dummy message not starve other
  // messages that may be in the Windows message queue.
  processPumpReplacementMessage();

  // Now give the delegate a chance to do some work.  He'll let us know if he
  // needs to do more work.
  if (_state->delegate->doWork()) scheduleWork();
}

void EventPumpWin::handleTimerMessage()
{
  ::KillTimer(_messageHwnd, reinterpret_cast<UINT_PTR>(this));

  // If we are being called outside of the context of Run, then don't do
  // anything.  This could correspond to a MessageBox call or something of
  // that sort.
  if (!_state) return;

  _state->delegate->doDelayedWork(&_delayedWorkTime);
  if (!_delayedWorkTime.isNull())
  {
    // A bit gratuitous to set _delayedWorkTime again, but oh well.
    scheduleDelayedWork(_delayedWorkTime);
  }
}

bool EventPumpWin::processNextWindowsMessage()
{
  // If there are sent messages in the queue then PeekMessage internally
  // dispatches the message and returns false. We return true in this
  // case to ensure that the message loop peeks again instead of calling
  // MsgWaitForMultipleObjectsEx again.
  bool sentMessagesInQueue = false;
  DWORD queueStatus = ::GetQueueStatus(QS_SENDMESSAGE);
  if (HIWORD(queueStatus) & QS_SENDMESSAGE)
    sentMessagesInQueue = true;

  MSG msg;
  if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) return processMessageHelper(msg);

  return sentMessagesInQueue;
}

bool EventPumpWin::processMessageHelper(const MSG& msg)
{
  if (WM_QUIT == msg.message)
  {
    // Repost the QUIT message so that it will be retrieved by the primary
    // GetMessage() loop.
    _state->shouldQuit = true;
    ::PostQuitMessage(static_cast<int>(msg.wParam));
    return false;
  }

  // While running our main message pump, we discard kMsgHaveWork messages.
  if (msg.message == kMsgHaveWork && msg.hwnd == _messageHwnd)
    return processPumpReplacementMessage();

  willProcessMessage(msg);

  if (_state->dispatcher) {
    if (!_state->dispatcher->dispatch(msg))
      _state->shouldQuit = true;
  } else {
    ::TranslateMessage(&msg);
    ::DispatchMessage(&msg);
  }

  didProcessMessage(msg);
  return true;
}

bool EventPumpWin::processPumpReplacementMessage()
{
  // When we encounter a kMsgHaveWork message, this method is called to peek
  // and process a replacement message, such as a WM_PAINT or WM_TIMER.  The
  // goal is to make the kMsgHaveWork as non-intrusive as possible, even though
  // a continuous stream of such messages are posted.  This method carefully
  // peeks a message while there is no chance for a kMsgHaveWork to be pending,
  // then resets the _haveWork flag (allowing a replacement kMsgHaveWork to
  // possibly be posted), and finally dispatches that peeked replacement.  Note
  // that the re-post of kMsgHaveWork may be asynchronous to this thread!!

  MSG msg;
  bool have_message = (0 != PeekMessage(&msg, NULL, 0, 0, PM_REMOVE));
  FOG_ASSERT(
    !have_message ||
    kMsgHaveWork != msg.message ||
    msg.hwnd != _messageHwnd);
  
  // Since we discarded a kMsgHaveWork message, we must update the flag.
  InterlockedExchange(&_haveWork, 0);

  // TODO(darin,jar): There is risk of being lost in a sub-pump within the call
  // to processMessageHelper(), which could result in no longer getting a
  // kMsgHaveWork message until the next out-of-band call to scheduleWork().

  return have_message && processMessageHelper(msg);
}

int EventPumpWin::getCurrentDelay() const
{
  if (_delayedWorkTime.isNull()) return -1;

  // Be careful here.  TimeDelta has a precision of microseconds, but we want a
  // value in milliseconds.  If there are 5.5ms left, should the delay be 5 or
  // 6?  It should be 6 to avoid executing delayed work too early.
  double timeout = ceil((_delayedWorkTime - Time::now()).inMillisecondsF());

  // If this value is negative, then we need to run delayed work soon.
  int delay = static_cast<int>(timeout);
  if (delay < 0) delay = 0;

  return delay;
}

// ============================================================================
// [Fog::EventPumpWin::Observer]
// ============================================================================

EventPumpWin::Observer::Observer() {}
EventPumpWin::Observer::~Observer() {}

// ============================================================================
// [Fog::EventPumpWin::Dispatcher]
// ============================================================================

EventPumpWin::Dispatcher::Dispatcher() {}
EventPumpWin::Dispatcher::~Dispatcher() {}

// ============================================================================
// [Fog::EventPumpWinUI]
// ============================================================================

EventPumpWinUI::EventPumpWinUI() :
  EventPumpWin(StubAscii8("WinIO"))
{
}

EventPumpWinUI::~EventPumpWinUI()
{
}

void EventPumpWinUI::doRunLoop()
{
  // If this was just a simple PeekMessage() loop (servicing all possible work
  // queues), then Windows would try to achieve the following order according
  // to MSDN documentation about PeekMessage with no filter):
  //    * Sent messages
  //    * Posted messages
  //    * Sent messages (again)
  //    * WM_PAINT messages
  //    * WM_TIMER messages
  //
  // Summary: none of the above classes is starved, and sent messages has twice
  // the chance of being processed (i.e., reduced service time).

  for (;;) {
    // If we do any work, we may create more messages etc., and more work may
    // possibly be waiting in another task group.  When we (for example)
    // processNextWindowsMessage(), there is a good chance there are still more
    // messages waiting.  On the other hand, when any of these methods return
    // having done no work, then it is pretty unlikely that calling them again
    // quickly will find any work to do.  Finally, if they all say they had no
    // work, then it is a good time to consider sleeping (waiting) for more
    // work.

    bool moreWorkIsPlausible = processNextWindowsMessage();
    if (_state->shouldQuit) break;

    moreWorkIsPlausible |= _state->delegate->doWork();
    if (_state->shouldQuit) break;

    moreWorkIsPlausible |= _state->delegate->doDelayedWork(&_delayedWorkTime);
    // If we did not process any delayed work, then we can assume that our
    // existing WM_TIMER if any will fire when delayed work should run.  We
    // don't want to disturb that timer if it is already in flight.  However,
    // if we did do all remaining delayed work, then lets kill the WM_TIMER.
    if (moreWorkIsPlausible && _delayedWorkTime.isNull())
    {
      KillTimer(_messageHwnd, reinterpret_cast<UINT_PTR>(this));
    }
    if (_state->shouldQuit) break;

    if (moreWorkIsPlausible) continue;

    moreWorkIsPlausible = _state->delegate->doIdleWork();
    if (_state->shouldQuit) break;
    if (moreWorkIsPlausible) continue;

    waitForWork();  // Wait (sleep) until we have work to do again.
  }
}

void EventPumpWinUI::waitForWork()
{
  // Wait until a message is available, up to the time needed by the timer
  // manager to fire the next set of timers.
  int delay = getCurrentDelay();

  if (delay < 0) 
    // Negative value means no timers waiting.
    delay = INFINITE;

  DWORD result;
  result = MsgWaitForMultipleObjectsEx(0, NULL, delay, QS_ALLINPUT,
                                       MWMO_INPUTAVAILABLE);

  if (WAIT_OBJECT_0 == result)
  {
    // A WM_* message is available.
    // If a parent child relationship exists between windows across threads
    // then their thread inputs are implicitly attached.
    // This causes the MsgWaitForMultipleObjectsEx API to return indicating
    // that messages are ready for processing (specifically mouse messages
    // intended for the child window. Occurs if the child window has capture)
    // The subsequent PeekMessages call fails to return any messages thus
    // causing us to enter a tight loop at times.
    // The WaitMessage call below is a workaround to give the child window
    // sometime to process its input messages.

    // Here was bug that dalayed NC messages. Thank guys from #chromium-dev 
    // for help with catching it. The problem is that QS_MOUSE will return
    // true also for NC messages, so there should be two PeekMessage() calls,
    // first for standard messages and second for NC messages.
    static const DWORD ncMouseFirst = WM_NCLBUTTONDOWN;
    static const DWORD ncMouseLast  = WM_NCMBUTTONDBLCLK;

    MSG msg = {0};
    DWORD queue_status = GetQueueStatus(QS_MOUSE);

    if ((HIWORD(queue_status) & QS_MOUSE) &&
        !PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_NOREMOVE) &&
        !PeekMessage(&msg, NULL, ncMouseFirst , ncMouseLast , PM_NOREMOVE) )
    {
      WaitMessage();
    }
    
    return;
  }

  FOG_ASSERT(result != WAIT_FAILED);
}

// ============================================================================
// [Fog::EventLoopWinUI]
// ============================================================================

EventLoopWinUI::EventLoopWinUI() : 
  EventLoop(new EventPumpWinUI())
{
}

void EventLoopWinUI::run(Dispatcher* dispatcher)
{
  AutoRunState saveState(this);
  _state->dispatcher = dispatcher;
  runHandler();
}

void EventLoopWinUI::addObserver(Observer* observer)
{
  static_cast<EventPumpWinUI*>(_pump.get())->addObserver(observer);
}

void EventLoopWinUI::removeObserver(Observer* observer)
{
  static_cast<EventPumpWinUI*>(_pump.get())->removeObserver(observer);
}

void EventLoopWinUI::willProcessMessage(const MSG& message)
{
  static_cast<EventPumpWinUI*>(_pump.get())->willProcessMessage(message);
}

void EventLoopWinUI::didProcessMessage(const MSG& message)
{
  static_cast<EventPumpWinUI*>(_pump.get())->didProcessMessage(message);
}

void EventLoopWinUI::pumpOutPendingPaintMessages()
{
  static_cast<EventPumpWinUI*>(_pump.get())->pumpOutPendingPaintMessages();
}

// ============================================================================
// [Fog::EventPumpWinIO]
// ============================================================================

class HandlerData : public Fog::EventPumpWinIO::Watcher
{
public:
  typedef Fog::EventPumpWinIO::IOHandler IOHandler;

  HandlerData(OVERLAPPED* context, IOHandler* handler) :
    _context(context), _handler(handler) 
  {
  }
  ~HandlerData() {}

  virtual void onObjectSignaled(HANDLE object);

private:
  OVERLAPPED* _context;
  IOHandler* _handler;

  FOG_DISABLE_COPY(HandlerData)
};

void HandlerData::onObjectSignaled(HANDLE object)
{
  FOG_ASSERT(object == _context->hEvent);

  DWORD transfered;
  DWORD error = ERROR_SUCCESS;
  BOOL ret = GetOverlappedResult(NULL, _context, &transfered, FALSE);

  if (!ret)
  {
    error = GetLastError();
    FOG_ASSERT(error == ERROR_HANDLE_EOF || error == ERROR_BROKEN_PIPE);
    transfered = 0;
  }

  ResetEvent(_context->hEvent);
  _handler->onIOCompleted(_context, transfered, error);
}

EventPumpWinIO::EventPumpWinIO() : 
  EventPumpWin(StubAscii8("WinIO"))
{
}

EventPumpWinIO::~EventPumpWinIO()
{
}

void EventPumpWinIO::watchObject(HANDLE object, Watcher* watcher)
{
  FOG_ASSERT(object);
  FOG_ASSERT(object != INVALID_HANDLE_VALUE);

  sysuint_t i = _objects.indexOf(object);

  if (watcher)
  {
    if (i == InvalidIndex)
    {
     static sysuint_t warning_multiple = 1;

     if (_objects.length() >= warning_multiple * MAXIMUM_WAIT_OBJECTS / 2)
     {
       // LOG(INFO) << "More than " << warning_multiple * MAXIMUM_WAIT_OBJECTS / 2
       //           << " objects being watched";
       // This DCHECK() is an artificial limitation, meant to warn us if we
       // start creating too many objects.  It can safely be raised to a higher
       // level, and the program is designed to handle much larger values.
       // Before raising this limit, make sure that there is a very good reason
       // (in your debug testing) to be watching this many objects.
       FOG_ASSERT(2 <= warning_multiple);
       ++warning_multiple;
      }

      _objects.append(object);
      _watchers.append(watcher);
    }
    else
    {
      _watchers.set(i, watcher);
    }
  }
  else if (i != InvalidIndex)
  {
    _objects.removeAt(i);
    _watchers.removeAt(i);
  }
}

void EventPumpWinIO::registerIOHandler(HANDLE fileHandle, IOHandler* handler)
{
#if 0
  // TODO(rvargas): This is just to give an idea of what this code will look
  // like when we actually move to completion ports. Of course, we cannot
  // do this without calling getQueuedCompletionStatus().
  ULONG_PTR key = reinterpret_cast<ULONG_PTR>(handler);
  HANDLE port = CreateIoCompletionPort(fileHandle, _port, key, 1);
  if (!_port.IsValid())
    _port.Set(port);
#endif
}

void EventPumpWinIO::registerIOContext(OVERLAPPED* context, IOHandler* handler)
{
  FOG_ASSERT(context->hEvent);

  if (handler)
  {
    HandlerData* watcher = new HandlerData(context, handler);
    watchObject(context->hEvent, watcher);
  }
  else
  {
    sysuint_t i = _objects.indexOf(context->hEvent);
    FOG_ASSERT(i != InvalidIndex);

    delete _watchers.cAt(i);
    _objects.removeAt(i);
    _watchers.removeAt(i);
  }
}

void EventPumpWinIO::doRunLoop()
{
  // If this was just a simple PeekMessage() loop (servicing all possible work
  // queues), then Windows would try to achieve the following order according
  // to MSDN documentation about PeekMessage with no filter):
  //    * Sent messages
  //    * Posted messages
  //    * Sent messages (again)
  //    * WM_PAINT messages
  //    * WM_TIMER messages
  //
  // Summary: none of the above classes is starved, and sent messages has twice
  // the chance of being processed (i.e., reduced service time).

  for (;;)
  {
    // If we do any work, we may create more messages etc., and more work may
    // possibly be waiting in another task group.  When we (for example)
    // ProcessNextWindowsMessage(), there is a good chance there are still more
    // messages waiting (same thing for ProcessNextObject(), which responds to
    // only one signaled object; etc.).  On the other hand, when any of these
    // methods return having done no work, then it is pretty unlikely that
    // calling them again quickly will find any work to do.  Finally, if they
    // all say they had no work, then it is a good time to consider sleeping
    // (waiting) for more work.

    bool moreWorkIsPlausible = processNextWindowsMessage();
    if (_state->shouldQuit) break;

    moreWorkIsPlausible |= _state->delegate->doWork();
    if (_state->shouldQuit) break;

    moreWorkIsPlausible |= processNextObject();
    if (_state->shouldQuit) break;

    moreWorkIsPlausible |=
        _state->delegate->doDelayedWork(&_delayedWorkTime);
    // If we did not process any delayed work, then we can assume that our
    // existing WM_TIMER if any will fire when delayed work should run.  We
    // don't want to disturb that timer if it is already in flight.  However,
    // if we did do all remaining delayed work, then lets kill the WM_TIMER.
    if (moreWorkIsPlausible && _delayedWorkTime.isNull())
      ::KillTimer(_messageHwnd, reinterpret_cast<UINT_PTR>(this));
    if (_state->shouldQuit)
      break;

    if (moreWorkIsPlausible)
      continue;

    moreWorkIsPlausible = _state->delegate->doIdleWork();
    if (_state->shouldQuit)
      break;

    if (moreWorkIsPlausible)
      continue;

    // We service APCs in WaitForWork, without returning.
    waitForWork();  // Wait (sleep) until we have work to do again.
  }
}

// If we handle more than the OS limit on the number of objects that can be
// waited for, we'll need to poll (sequencing through subsets of the objects
// that can be passed in a single OS wait call).  The following is the polling
// interval used in that (unusual) case. (I don't have a lot of justifcation
// for the specific value, but it needed to be short enough that it would not
// add a lot of latency, and long enough that we wouldn't thrash the CPU for no
// reason... especially considering the silly user probably has a million tabs
// open, etc.)
static const int kMultipleWaitPollingInterval = 20;

void EventPumpWinIO::waitForWork()
{
  // Wait until either an object is signaled or a message is available.  Handle
  // (without returning) any APCs (only the IO thread currently has APCs.)

  // We do not support nested message loops when we have watched objects.  This
  // is to avoid messy recursion problems.
  // "Cannot nest a message loop when there are watched objects!";
  FOG_ASSERT(_objects.isEmpty() || _state->runDepth == 1);

  int wait_flags = MWMO_ALERTABLE | MWMO_INPUTAVAILABLE;

  bool use_polling = false;  // Poll if too many objects for one OS Wait call.
  for (;;) {
    // Do initialization here, in case APC modifies object list.
    sysuint_t total_objs = _objects.length();

    // The first unprocessed object index.
    sysuint_t polling_index = 0;
    int delay;

    do {
      sysuint_t objs_len =
          (polling_index < total_objs) ? total_objs - polling_index : 0;
      if (objs_len >= MAXIMUM_WAIT_OBJECTS) {
        objs_len = MAXIMUM_WAIT_OBJECTS - 1;
        use_polling = true;
      }
      HANDLE* objs = objs_len ? (HANDLE*)_objects.cData() + polling_index : NULL;

      // Only wait up to the time needed by the timer manager to fire the next
      // set of timers.
      delay = getCurrentDelay();
      if (use_polling && delay > kMultipleWaitPollingInterval)
        delay = kMultipleWaitPollingInterval;
      if (delay < 0)  // Negative value means no timers waiting.
        delay = INFINITE;

      DWORD result;
      result = MsgWaitForMultipleObjectsEx(static_cast<DWORD>(objs_len), objs,
                                           delay, QS_ALLINPUT, wait_flags);

      if (WAIT_IO_COMPLETION == result) {
        // We'll loop here when we service an APC.  At it currently stands,
        // *ONLY* the IO thread uses *any* APCs, so this should have no impact
        // on the UI thread.
        break;  // Break to outer loop, and waitforwork() again.
      }

      // Use unsigned type to simplify range detection;
      sysuint_t signaled_index = result - WAIT_OBJECT_0;
      if (signaled_index < objs_len)
      {
        signalWatcher(polling_index + signaled_index);
        return;  // We serviced a signaled object.
      }

      if (objs_len == signaled_index)
        return;  // A WM_* message is available.

      FOG_ASSERT(result != WAIT_FAILED);
      FOG_ASSERT(!objs || result == WAIT_TIMEOUT);

      if (!use_polling) return;
      polling_index += objs_len;
    } while (polling_index < total_objs);
    // For compatibility, we didn't return sooner.  This made us do *some* wait
    // call(s) before returning. This will probably change in next rev.
    if (!delay || !getCurrentDelay())
      return;  // No work done, but timer is ready to fire.
  }
}

bool EventPumpWinIO::processNextObject()
{
  sysuint_t total_objs = _objects.length();
  if (!total_objs) return false;

  // The first unprocessed object index.
  sysuint_t polling_index = 0;

  do {
    FOG_ASSERT(polling_index < total_objs);
    sysuint_t objs_len = total_objs - polling_index;
    if (objs_len >= kMaxWaitObjects) objs_len = kMaxWaitObjects - 1;
    HANDLE* objs = (HANDLE*)_objects.cData() + polling_index;

    // Identify 1 pending object, or allow an IO APC to be completed.
    DWORD result = WaitForMultipleObjectsEx(static_cast<DWORD>(objs_len), objs,
                                            FALSE,    // 1 signal is sufficient.
                                            0,        // Wait 0ms.
                                            false);   // Not alertable (no APC).

    // Use unsigned type to simplify range detection;
    sysuint_t signaled_index = result - WAIT_OBJECT_0;
    if (signaled_index < objs_len)
    {
      signalWatcher(polling_index + signaled_index);
      return true;  // We serviced a signaled object.
    }

    // If an handle is invalid, it will be WAIT_FAILED.
    FOG_ASSERT(result != WAIT_TIMEOUT);
    polling_index += objs_len;
  } while (polling_index < total_objs);

  // We serviced nothing.
  return false;
}

bool EventPumpWinIO::signalWatcher(sysuint_t objectIndex)
{
  // Signal the watcher corresponding to the given index.
  FOG_ASSERT(_objects.length() > objectIndex);

  // On reception of OnObjectSignaled() to a Watcher object, it may call
  // WatchObject(). _watchers and _objects will be modified. This is expected,
  // so don't be afraid if, while tracing a OnObjectSignaled() function, the
  // corresponding _watchers[result] is non-existant.
  _watchers[objectIndex]->onObjectSignaled(_objects[objectIndex]);

  // Signaled objects tend to be removed from the watch list, and then added
  // back (appended).  As a result, they move to the end of the _objects array,
  // and this should make their service "fair" (no HANDLEs should be starved).

  return true;
}

// ============================================================================
// [Fog::EventPumpWinIO::Watcher]
// ============================================================================

EventPumpWinIO::Watcher::Watcher() {}
EventPumpWinIO::Watcher::~Watcher() {}

// ============================================================================
// [Fog::EventPumpWinIO::IOHandler]
// ============================================================================

EventPumpWinIO::IOHandler::IOHandler() {}
EventPumpWinIO::IOHandler::~IOHandler() {}

// ============================================================================
// [Fog::EventLoopWinIO]
// ============================================================================

EventLoopWinIO::EventLoopWinIO() : 
  EventLoop(new EventPumpWinIO())
{
}

void EventLoopWinIO::watchObject(HANDLE object, Watcher* watcher)
{
  static_cast<EventPumpWinIO*>(_pump.get())->watchObject(object, watcher);
}

void EventLoopWinIO::registerIOHandler(HANDLE file, IOHandler* handler)
{
  static_cast<EventPumpWinIO*>(_pump.get())->registerIOHandler(file, handler);
}

void EventLoopWinIO::registerIOContext(OVERLAPPED* context, IOHandler* handler) 
{
  static_cast<EventPumpWinIO*>(_pump.get())->registerIOContext(context, handler);
}

} // Fog namespace

#endif // FOG_OS_WINDOWS
