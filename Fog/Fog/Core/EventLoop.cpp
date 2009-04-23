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
#include <Fog/Core/AutoLock.h>
#include <Fog/Core/EventLoop.h>
#include <Fog/Core/EventLoop_Def.h>
#include <Fog/Core/Lazy.h>
#include <Fog/Core/ThreadLocalStorage.h>

#if defined(FOG_OS_WINDOWS)
#include <Fog/Core/EventLoop_Win.h>
#endif // FOG_OS_WINDOWS

#include <algorithm>

// A lazily created thread local storage for quick access to a thread's message
// loop, if one exists.  This should be safe and free of static constructors.
static Fog::ThreadLocalPointer<Fog::EventLoop> fog_eventloop_tls;

// Logical events for Histogram profiling. Run with -message-loop-histogrammer
// to get an accounting of messages and actions taken on each thread.
static const int kTaskRunEvent = 0x1;
static const int kTimerEvent = 0x2;

// Provide range of message IDs for use in histogramming and debug display.
static const int kLeastNonZeroMessageId = 1;
static const int kMaxMessageId = 1099;
static const int kNumberOfDistinctMessagesDisplayed = 1100;

#if defined(FOG_OS_WINDOWS)

// Upon a SEH exception in this thread, it restores the original unhandled
// exception filter.
static int SEHFilter(LPTOP_LEVEL_EXCEPTION_FILTER oldFilter)
{
  ::SetUnhandledExceptionFilter(oldFilter);
  return EXCEPTION_CONTINUE_SEARCH;
}

// Retrieves a pointer to the current unhandled exception filter. There
// is no standalone getter method.
static LPTOP_LEVEL_EXCEPTION_FILTER GetTopSEHFilter()
{
  LPTOP_LEVEL_EXCEPTION_FILTER topFilter = NULL;
  topFilter = ::SetUnhandledExceptionFilter(0);
  ::SetUnhandledExceptionFilter(topFilter);
  return topFilter;
}

#endif // FOG_OS_WINDOWS

namespace Fog {

// ============================================================================
// [Fog::EventPump]
// ============================================================================

EventPump::EventPump(const String32& name) : 
  _name(name)
{
}

EventPump::~EventPump()
{
}

// ============================================================================
// [Fog::EventPump::Delegate]
// ============================================================================

EventPump::Delegate::Delegate()
{
}

EventPump::Delegate::~Delegate()
{
}

// ============================================================================
// [Fog::EventPump]
// ============================================================================

String32 EventLoop::type() const
{
  return _pump->_name;
}

// Optional call to connect the thread name with this loop.
void EventLoop::setThreadName(const String32& threadName)
{
#if defined DEBUG
  if (!_threadName.isEmpty())
  {
    fog_debug("Fog::EventLoop::setThreadName() - Should not rename this thread!");
  }
#endif // DEBUG
  _threadName = threadName;
}

const String32& EventLoop::threadName() const
{
  return _threadName;
}

// static
EventLoop* EventLoop::current()
{
  // TODO(darin): sadly, we cannot enable this yet since people call us even
  // when they have no intention of using us.

  return fog_eventloop_tls.get();
}

EventLoop::EventLoop(EventPump* pump) : 
  _nestableTasksAllowed(true),
  _exceptionRestoration(false),
  _state(NULL),
  _nextSequenceNum(0)
{
  // "should only have one event loop per thread"
  FOG_ASSERT(!current());
  fog_eventloop_tls.set(this);

  if (!pump) pump = new EventPumpDefault();
  _pump = pump;
}

EventLoop::~EventLoop()
{
  FOG_ASSERT(this == current());

  // Let interested parties have one last shot at accessing this.
  FOR_EACH_OBSERVER(DestructionObserver, _destructionObservers,
                    willDestroyCurrentEventLoop());

  FOG_ASSERT(!_state);

  // Clean up any unprocessed tasks, but take care: deleting a task could
  // result in the addition of more tasks (e.g., via DeleteSoon).  We set a
  // limit on the number of times we will allow a deleted task to generate more
  // tasks.  Normally, we should only pass through this loop once or twice.  If
  // we end up hitting the loop limit, then it is probably due to one task that
  // is being stubborn.  Inspect the queues to see who is left.
  bool didWork;

  for (int i = 0; i < 100; ++i)
  {
    deletePendingTasks();
    reloadWorkQueue();
    // If we end up with empty queues, then break out of the loop.
    didWork = deletePendingTasks();
    if (!didWork) break;
  }

  FOG_ASSERT(!didWork);

  // OK, now make it so that no one can find us.
  fog_eventloop_tls.set(NULL);
}

void EventLoop::addDestructionObserver(DestructionObserver *obs)
{
  FOG_ASSERT(this == current());
  _destructionObservers.addObserver(obs);
}

void EventLoop::removeDestructionObserver(DestructionObserver *obs)
{
  FOG_ASSERT(this == current());
  _destructionObservers.removeObserver(obs);
}

void EventLoop::run()
{
  AutoRunState saveState(this);
  runHandler();
}

void EventLoop::runAllPending()
{
  AutoRunState saveState(this);
  _state->quitReceived = true;  // Means run until we would otherwise block.
  runHandler();
}

// Runs the loop in two different SEH modes:
// enable_SEH_restoration_ = false : any unhandled exception goes to the last
// one that calls SetUnhandledExceptionFilter().
// enable_SEH_restoration_ = true : any unhandled exception goes to the filter
// that was existed before the loop was run.
void EventLoop::runHandler() 
{
#if defined(FOG_OS_WINDOWS)
  if (_exceptionRestoration)
  {
    LPTOP_LEVEL_EXCEPTION_FILTER currentFilter = GetTopSEHFilter();

    __try
    {
      runInternal();
    }
    __except(SEHFilter(currentFilter))
    {
    }
    return;
  }
#endif // FOG_OS_WINDOWS

  runInternal();
}

//------------------------------------------------------------------------------

void EventLoop::runInternal()
{
  FOG_ASSERT(this == current());

#if defined(FOG_OS_WINDOWS)
  if (_state->dispatcher)
  {
    EventPumpWin* p = static_cast<EventPumpWin*>(_pump.get());
    p->runWithDispatcher(this, 
      static_cast<EventPumpWin::Dispatcher*>(_state->dispatcher));
  }
  else
  {
#endif // FOG_OS_WINDOWS
    _pump->run(this);
#if defined(FOG_OS_WINDOWS)
  }
#endif // FOG_OS_WINDOWS
}

//------------------------------------------------------------------------------
// Wrapper functions for use in above event loop framework.

bool EventLoop::processNextDelayedNonNestableTask()
{
  if (_state->runDepth != 1) return false;
  if (_deferredNonNestableWorkQueue.empty()) return false;
  
  Task* task = _deferredNonNestableWorkQueue.front().task;
  _deferredNonNestableWorkQueue.pop();
  
  runTask(task);
  return true;
}

//------------------------------------------------------------------------------

void EventLoop::quit()
{
  FOG_ASSERT(current() == this);

  if (_state)
  {
    _state->quitReceived = true;
  }
  else
  {
    // Must be inside run() to call quit()
    FOG_ASSERT_NOT_REACHED();
  }
}

void EventLoop::postTask(Task* task)
{
  postTask_Helper(task, 0, true);
}

void EventLoop::postDelayedTask(Task* task, uint32_t delay_ms)
{
  postTask_Helper(task, delay_ms, true);
}

void EventLoop::postNonNestableTask(Task* task)
{
  postTask_Helper(task, 0, false);
}

void EventLoop::postNonNestableDelayedTask(Task* task, uint32_t delay_ms)
{
  postTask_Helper(task, delay_ms, false);
}

// Possibly called on a background thread!
void EventLoop::postTask_Helper(Task* task, uint32_t delay_ms, bool nestable)
{
  PendingTask pendingTask(task, nestable);

  if (delay_ms > 0)
  {
    pendingTask.delayedRunTime = Time::now() + TimeDelta::fromMilliseconds(delay_ms);
  }

  // Warning: Don't try to short-circuit, and handle this thread's tasks more
  // directly, as it could starve handling of foreign threads.  Put every task
  // into this queue.

  RefPtr<EventPump> pump;
  {
    AutoLock locked(_incomingQueueLock);

    bool wasEmpty = _incomingQueue.empty();
    _incomingQueue.push(pendingTask);

    if (!wasEmpty)
      // Someone else should have started the sub-pump.
      return;

    pump = _pump;
  }

  // Since the _incomingQueue may contain a task that destroys this message
  // loop, we cannot exit _incomingQueueLock until we are done with |this|.
  // We use a stack-based reference to the message pump so that we can call
  // ScheduleWork outside of _incomingQueueLock.

  pump->scheduleWork();
}

void EventLoop::setNestableTasksAllowed(bool allowed)
{
  if (_nestableTasksAllowed != allowed)
  {
    _nestableTasksAllowed = allowed;
    if (!_nestableTasksAllowed)
      return;

    // Start the native pump if we are not already pumping.
    _pump->scheduleWork();
  }
}

bool EventLoop::nestableTasksAllowed() const
{
  return _nestableTasksAllowed;
}

//------------------------------------------------------------------------------

void EventLoop::runTask(Task* task)
{
  FOG_ASSERT(_nestableTasksAllowed);

  // Execute the task and assume the worst: It is probably not reentrant.
  _nestableTasksAllowed = false;

  task->run();
  if (task->deleteOnFinish()) delete task;

  _nestableTasksAllowed = true;
}

bool EventLoop::deferOrRunPendingTask(const PendingTask& pendingTask)
{
  if (pendingTask.nestable || _state->runDepth == 1)
  {
    runTask(pendingTask.task);
    // Show that we ran a task (Note: a new one might arrive as a
    // consequence!).
    return true;
  }

  // We couldn't run the task now because we're in a nested event loop
  // and the task isn't nestable.
  _deferredNonNestableWorkQueue.push(pendingTask);
  return false;
}

void EventLoop::addToDelayedWorkQueue(const PendingTask& pendingTask)
{
  // Move to the delayed work queue.  Initialize the sequence number
  // before inserting into the _delayedWorkQueue.  The sequence number
  // is used to faciliate FIFO sorting when two tasks have the same
  // delayedRunTime value.
  PendingTask newPendingTask(pendingTask);
  newPendingTask.sequenceNum = _nextSequenceNum++;
  _delayedWorkQueue.push(newPendingTask);
}

void EventLoop::reloadWorkQueue()
{
  // We can improve performance of our loading tasks from _incomingQueue to
  // _workQueue by waiting until the last minute (_workQueue is empty) to
  // load.  That reduces the number of locks-per-task significantly when our
  // queues get large.
  if (!_workQueue.empty())
    // Wait till we *really* need to lock and load.
    return;

  // Acquire all we can from the inter-thread queue with one lock acquisition.
  {
    AutoLock lock(_incomingQueueLock);
    if (_incomingQueue.empty()) return;
    std::swap(_incomingQueue, _workQueue);
    FOG_ASSERT(_incomingQueue.empty());
  }
}

bool EventLoop::deletePendingTasks()
{
  bool didWork = !_workQueue.empty();
  while (!_workQueue.empty())
  {
    PendingTask pendingTask = _workQueue.front();
    _workQueue.pop();
    if (!pendingTask.delayedRunTime.isNull())
    {
      // We want to delete delayed tasks in the same order in which they would
      // normally be deleted in case of any funny dependencies between delayed
      // tasks.
      addToDelayedWorkQueue(pendingTask);
    }
    else
    {
      // TODO(darin): Delete all tasks once it is safe to do so.
      //delete task;
    }
  }
  didWork |= !_deferredNonNestableWorkQueue.empty();
  while (!_deferredNonNestableWorkQueue.empty())
  {
    // TODO(darin): Delete all tasks once it is safe to do so.
    //Task* task = deferredNonNestableWorkQueue.front().task;
    _deferredNonNestableWorkQueue.pop();
    //delete task;
  }
  didWork |= !_delayedWorkQueue.empty();
  while (!_delayedWorkQueue.empty())
  {
    Task* task = _delayedWorkQueue.top().task;
    _delayedWorkQueue.pop();
    delete task;
  }
  return didWork;
}

bool EventLoop::doWork()
{
  if (!_nestableTasksAllowed)
  {
    // Task can't be executed right now.
    return false;
  }

  for (;;)
  {
    reloadWorkQueue();
    if (_workQueue.empty()) break;

    // Execute oldest task.
    do {
      PendingTask pendingTask = _workQueue.front();
      _workQueue.pop();
      if (!pendingTask.delayedRunTime.isNull())
      {
        addToDelayedWorkQueue(pendingTask);
        // If we changed the topmost task, then it is time to re-schedule.
        if (_delayedWorkQueue.top().task == pendingTask.task)
          _pump->scheduleDelayedWork(pendingTask.delayedRunTime);
      } 
      else
      {
        if (deferOrRunPendingTask(pendingTask)) 
          return true;
      }
    } while (!_workQueue.empty());
  }

  // Nothing happened.
  return false;
}

bool EventLoop::doDelayedWork(Time* nextDelayedWorkTime)
{
  if (!_nestableTasksAllowed || _delayedWorkQueue.empty())
  {
    *nextDelayedWorkTime = Time();
    return false;
  }
  
  if (_delayedWorkQueue.top().delayedRunTime > Time::now())
  {
    *nextDelayedWorkTime = _delayedWorkQueue.top().delayedRunTime;
    return false;
  }

  PendingTask pendingTask = _delayedWorkQueue.top();
  _delayedWorkQueue.pop();
  
  if (!_delayedWorkQueue.empty())
    *nextDelayedWorkTime = _delayedWorkQueue.top().delayedRunTime;

  return deferOrRunPendingTask(pendingTask);
}

bool EventLoop::doIdleWork()
{
  if (processNextDelayedNonNestableTask()) return true;
  if (_state->quitReceived) _pump->quit();
  return false;
}

// ============================================================================
// [Fog::EventLoop::DestructionObserver]
// ============================================================================

EventLoop::DestructionObserver::DestructionObserver()
{
}

EventLoop::DestructionObserver::~DestructionObserver()
{
}

// ============================================================================
// [Fog::EventLoop::AutoRunState]
// ============================================================================

EventLoop::AutoRunState::AutoRunState(EventLoop* loop) : _loop(loop)
{
  // Make the loop reference us.
  _previousState = _loop->_state;

  if (_previousState)
    runDepth = _previousState->runDepth + 1;
  else
    runDepth = 1;

  _loop->_state = this;

  // Initialize the other fields:
  quitReceived = false;

  dispatcher = NULL;
}

EventLoop::AutoRunState::~AutoRunState()
{
  _loop->_state = _previousState;
}

//------------------------------------------------------------------------------
// EventLoop::PendingTask

bool EventLoop::PendingTask::operator<(const PendingTask& other) const
{
  // Since the top of a priority queue is defined as the "greatest" element, we
  // need to invert the comparison here.  We want the smaller time to be at the
  // top of the heap.

  if (delayedRunTime < other.delayedRunTime)
    return false;

  if (delayedRunTime > other.delayedRunTime)
    return true;

  // If the times happen to match, then we use the sequence number to decide.
  // Compare the difference to support integer roll-over.
  return (sequenceNum - other.sequenceNum) > 0;
}

} // Fog namespace
