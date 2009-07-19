// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// [Guard]
#ifndef _FOG_CORE_EVENTLOOP_H
#define _FOG_CORE_EVENTLOOP_H

// [Dependencies]
#include <Fog/Core/Lock.h>
#include <Fog/Core/ObserverList.h>
#include <Fog/Core/RefCounted.h>
#include <Fog/Core/String.h>
#include <Fog/Core/Task.h>
#include <Fog/Core/Time.h>

// TODO: Remove STL dependency
#include <deque>
#include <queue>

//! @addtogroup Fog_Core
//! @{

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct EventLoop;
struct EventPump;
struct Time;

// ============================================================================
// [Fog::EventPump]
// ============================================================================

struct FOG_API EventPump : public RefCounted<EventPump>
{
public:
  //! @brief Please see the comments above the Run method for an illustration 
  //! of how these delegate methods are used.
  struct FOG_API Delegate
  {
  public:
    Delegate();
    virtual ~Delegate();

    //! @brief Called from within Run in response to @c scheduleWork() or when
    //! the message pump would otherwise call @c doDelayedWork(). Returns true
    //! to indicate that work was done. @c doDelayedWork() will not be called
    //! if @a doWork() returns true.
    virtual bool doWork() = 0;

    //! @brief Called from within @c run() in response to @c scheduleDelayedWork()
    //! or when the message pump would otherwise sleep waiting for more work.
    //!
    //! Returns true to indicate that delayed work was done. doIdleWork() will 
    //! not be called if doDelayedWork() returns true. Upon return 
    //! @a nextDelayedWorkTime indicates the time when DoDelayedWork should be 
    //! called again. If @a nextDelayedWorkTime is null (per Time::isNull), then 
    //! the queue of future delayed work (timer events) is currently empty, and 
    //! no additional calls to this function need to be scheduled.
    virtual bool doDelayedWork(Time* nextDelayedWorkTime) = 0;

    //! @brief Called from within Run just before the message pump goes to 
    //! sleep. Returns true to indicate that idle work was done.
    virtual bool doIdleWork() = 0;
  };

  EventPump(const String32& name);
  virtual ~EventPump();

  //! @brief The Run method is called to enter the message pump's run loop.
  //!
  //! Within the method, the message pump is responsible for processing native
  //! messages as well as for giving cycles to the delegate periodically.  The
  //! message pump should take care to mix delegate callbacks with native
  //! message processing so neither type of event starves the other of cycles.
  //!
  //! The anatomy of a typical run loop:
  //!
  //! @code
  //! for (;;) {
  //!   bool did_work = doInternalWork();
  //!   if (should_quit_)
  //!     break;
  //!
  //!   did_work |= delegate_->doWork();
  //!   if (should_quit_)
  //!     break;
  //!
  //!   did_work |= delegate_->doDelayedWork();
  //!   if (should_quit_)
  //!     break;
  //!
  //!   if (did_work)
  //!     continue;
  //!
  //!   did_work = delegate_->doIdleWork();
  //!   if (should_quit_)
  //!     break;
  //!
  //!   if (did_work)
  //!     continue;
  //!
  //!   waitForWork();
  //! }
  //! @endcode
  //!
  //! Here, doInternalWork is some private method of the message pump that is
  //! responsible for dispatching the next UI message or notifying the next IO
  //! completion (for example).  WaitForWork is a private method that simply
  //! blocks until there is more work of any type to do.
  //!
  //! Notice that the run loop cycles between calling doInternalWork, doWork,
  //! and doDelayedWork methods. This helps ensure that neither work queue
  //! starves the other. This is important for message pumps that are used to
  //! drive animations, for example.
  //!
  //! Notice also that after each callout to foreign code, the run loop checks
  //! to see if it should quit. The Quit method is responsible for setting this
  //! flag. No further work is done once the quit flag is set.
  //!
  //! @note Care must be taken to handle Run being called again from within any
  //! of the callouts to foreign code. Native message pumps may also need to
  //! deal with other native message pumps being run outside their control
  //! (e.g., the MessageBox API on Windows pumps UI messages!). To be specific,
  //! the callouts (scheduleWork and scheduleDelayedWork) MUST still be provided 
  //! even in nested sub-loops that are "seemingly" outside the control of this 
  //! message pump.  DoWork in particular must never be starved for time slices 
  //! unless it returns false (meaning it has run out of things to do).
  virtual void run(Delegate* d) = 0;

  //! @brief Quit immediately from the most recently entered run loop. This 
  //! method may only be used on the thread that called @c run().
  virtual void quit() = 0;

  //! @brief Schedule a @c doWork() callback to happen reasonably soon. Does
  //! nothing if a @c doWork() callback is already scheduled. This method may
  //! be called from any thread. Once this call is made, @c doWork() should
  //! not be "starved" at least until it returns a value of false.
  virtual void scheduleWork() = 0;

  //! @brief Schedule a @c doDelayedWork() callback to happen at the specified
  //! time, cancelling any pending @c doDelayedWork() callback. This method may
  //! only be used on the thread that called @c run().
  virtual void scheduleDelayedWork(const Time& delayedWorkTime) = 0;

protected:
  String32 _name;

  friend struct EventLoop;
};

// ============================================================================
// [Fog::EventLoop]
// ============================================================================

//! A EventLoop is used to process events for a particular thread.  There is
//! at most one EventLoop instance per thread.
//!
//! Events include at a minimum Task instances submitted to PostTask or those
//! managed by TimerManager.  Depending on the type of message pump used by the
//! EventLoop other events such as UI messages may be processed.  On Windows
//! APC calls (as time permits) and signals sent to a registered set of HANDLEs
//! may also be processed.
//!
//! NOTE: Unless otherwise specified, a EventLoop's methods may only be called
//! on the thread where the EventLoop's Run method executes.
//!
//! NOTE: EventLoop has task reentrancy protection.  This means that if a
//! task is being processed, a second task cannot start until the first task is
//! finished.  Reentrancy can happen when processing a task, and an inner
//! message pump is created.  That inner pump then processes native messages
//! which could implicitly start an inner task.  Inner message pumps are created
//! with dialogs (DialogBox), common dialogs (GetOpenFileName), OLE functions
//! (DoDragDrop), printer functions (StartDoc) and *many* others.
//!
//! Sample workaround when inner task processing is needed:
//!   bool old_state = EventLoop::current()->NestableTasksAllowed();
//!   EventLoop::current()->SetNestableTasksAllowed(true);
//!   HRESULT hr = DoDragDrop(...); // Implicitly runs a modal event loop here.
//!   EventLoop::current()->SetNestableTasksAllowed(old_state);
//!   // Process hr  (the result returned by DoDragDrop().
//!
//! Please be SURE your task is reentrant (nestable) and all global variables
//! are stable and accessible before calling setNestableTasksAllowed(true).
struct FOG_API EventLoop : public EventPump::Delegate
{
public:
  // A DestructionObserver is notified when the current EventLoop is being
  // destroyed.  These obsevers are notified prior to EventLoop::current()
  // being changed to return NULL.  This gives interested parties the chance to
  // do final cleanup that depends on the EventLoop.
  //
  // NOTE: Any tasks posted to the EventLoop during this notification will
  // not be run.  Instead, they will be deleted.
  //
  struct FOG_API DestructionObserver
  {
  public:
    DestructionObserver();
    virtual ~DestructionObserver();
    virtual void willDestroyCurrentEventLoop() = 0;
  };

  // Add a DestructionObserver, which will start receiving notifications
  // immediately.
  void addDestructionObserver(DestructionObserver* destruction_observer);

  // Remove a DestructionObserver.  It is safe to call this method while a
  // DestructionObserver is receiving a notification callback.
  void removeDestructionObserver(DestructionObserver* destruction_observer);

  //! The "PostTask" family of methods call the task's Run method asynchronously
  //! from within a event loop at some point in the future.
  //!
  //! With the PostTask variant, tasks are invoked in FIFO order, inter-mixed
  //! with normal UI or IO event processing.  With the PostDelayedTask variant,
  //! tasks are called after at least approximately 'delay_ms' have elapsed.
  //!
  //! The NonNestable variants work similarly except that they promise never to
  //! dispatch the task from a nested invocation of EventLoop::Run.  Instead,
  //! such tasks get deferred until the top-most EventLoop::Run is executing.
  //!
  //! The EventLoop takes ownership of the Task, and deletes it after it has
  //! been Run().
  //!
  //! NOTE: These methods may be called on any thread.  The Task will be invoked
  //! on the thread that executes EventLoop::Run().
  void postTask(Task* task);

  void postDelayedTask(Task* task, uint32_t delay_ms);
  void postNonNestableTask(Task* task);
  void postNonNestableDelayedTask(Task* task, uint32_t delay_ms);

  //! A variant on PostTask that deletes the given object.  This is useful
  //! if the object needs to live until the next run of the EventLoop (for
  //! example, deleting a RenderProcessHost from within an IPC callback is not
  //! good).
  //!
  //! NOTE: This method may be called on any thread.  The object will be deleted
  //! on the thread that executes EventLoop::run().  If this is not the same
  //! as the thread that calls postDelayedTask(), then T MUST inherit
  //! from RefCountedThreadSafe<T>!
  template <typename T>
  FOG_INLINE void deleteSoon(T* object)
  {
    postNonNestableTask(new DeleteTask<T>(object));
  }

  //! A variant on PostTask that releases the given reference counted object
  //! (by calling its Release method).  This is useful if the object needs to
  //! live until the next run of the EventLoop, or if the object needs to be
  //! released on a particular thread.
  //!
  //! NOTE: This method may be called on any thread. The object will be
  //! released (and thus possibly deleted) on the thread that executes
  //! EventLoop::run(). If this is not the same as the thread that calls
  //! postDelayedTask(), then T MUST inherit from RefCountedThreadSafe<T>!
  template <typename T>
  FOG_INLINE void releaseSoon(T* object)
  {
    postNonNestableTask(new ReleaseTask<T>(object));
  }

  //! @brief Run the event loop.
  void run();

  //! Process all pending tasks, windows messages, etc., but don't wait/sleep.
  //! Return as soon as all items that can be run are taken care of.
  void runAllPending();

  //! Signals the Run method to return after it is done processing all pending
  //! messages.  This method may only be called on the same thread that called
  //! Run, and Run must still be on the call stack.
  //!
  //! Use QuitTask if you need to Quit another thread's EventLoop, but note
  //! that doing so is fairly dangerous if the target thread makes nested calls
  //! to EventLoop::Run.  The problem being that you won't know which nested
  //! run loop you are quiting, so be careful!
  void quit();

  // Normally, it is not necessary to instantiate a EventLoop. Instead, it
  // is typical to make use of the current thread's EventLoop instance.
  explicit EventLoop(EventPump* pump = NULL);
  ~EventLoop();

  //! @brief Returns the type passed to the constructor.
  String32 type() const;

  //! @brief Optional call to connect the thread name with this loop.
  void setThreadName(const String32& threadName);

  //! @brief Return thread name.
  const String32& threadName() const;

  //! @brief Return the EventLoop object for the current thread, or null if 
  //! none.
  static EventLoop* current();

  // Enables or disables the recursive task processing. This happens in the case
  // of recursive message loops. Some unwanted event loop may occurs when
  // using common controls or printer functions. By default, recursive task
  // processing is disabled.
  //
  // The specific case where tasks get queued is:
  // - The thread is running a event loop.
  // - It receives a task #1 and execute it.
  // - The task #1 implicitly start a event loop, like a MessageBox in the
  //   unit test. This can also be StartDoc or GetSaveFileName.
  // - The thread receives a task #2 before or while in this second message
  //   loop.
  // - With NestableTasksAllowed set to true, the task #2 will run right away.
  //   Otherwise, it will get executed right after task #1 completes at "thread
  //   event loop level".
  void setNestableTasksAllowed(bool allowed);
  bool nestableTasksAllowed() const;

  // Enables or disables the restoration during an exception of the unhandled
  // exception filter that was active when run() was called. This can happen
  // if some third party code call setUnhandledExceptionFilter() and never
  // restores the previous filter.
  void setExceptionRestoration(bool restore)
  {
    _exceptionRestoration = restore;
  }

  //----------------------------------------------------------------------------
protected:
  struct FOG_API RunState
  {
    // Used to count how many run() invocations are on the stack.
    int runDepth;

    // Used to record that quit() was called, or that we should quit the pump
    // once it becomes idle.
    bool quitReceived;

    void* dispatcher;
  };

  struct FOG_API AutoRunState : public RunState
  {
    AutoRunState(EventLoop* loop);
    ~AutoRunState();

  private:
    EventLoop* _loop;
    RunState* _previousState;
  };

  // This structure is copied around by value.
  struct PendingTask
  {
    Task* task;              // The task to run.
    Time  delayedRunTime;    // The time when the task should be run.
    int   sequenceNum;       // Used to facilitate sorting by run time.
    bool  nestable;          // True if OK to dispatch from a nested loop.

    PendingTask(Task* task, bool nestable) :
      task(task),
      sequenceNum(0),
      nestable(nestable)
    {
    }
    
    // Used to support sorting.
    bool operator<(const PendingTask& other) const;
  };

  typedef std::queue<PendingTask> TaskQueue;
  typedef std::priority_queue<PendingTask> DelayedTaskQueue;

  //! A function to encapsulate all the exception handling capability in the
  //! stacks around the running of a main event loop.  It will run the message
  //! loop in a SEH try block or not depending on the set_SEH_restoration()
  //! flag.
  void runHandler();

  //! A surrounding stack frame around the running of the event loop that
  //! supports all saving and restoring of state, as is needed for any/all (ugly)
  //! recursive calls.
  void runInternal();

  //! Called to process any delayed non-nestable tasks.
  bool processNextDelayedNonNestableTask();

  // --------------------------------------------------------------------------
  // Run a _workQueue task or new_task, and delete it (if it was processed by
  // PostTask). If there are queued tasks, the oldest one is executed and
  // new_task is queued. new_task is optional and can be NULL. In this NULL
  // case, the method will run one pending task (if any exist). Returns true if
  // it executes a task.  Queued tasks accumulate only when there is a
  // non-nestable task currently processing, in which case the new_task is
  // appended to the list _workQueue.  Such re-entrancy generally happens when
  // an unrequested message pump (typical of a native dialog) is executing in
  // the context of a task.
  bool queueOrRunTask(Task* new_task);

  // Runs the specified task and deletes it.
  void runTask(Task* task);

  // Calls RunTask or queues the pending_task on the deferred task list if it
  // cannot be run right now.  Returns true if the task was run.
  bool deferOrRunPendingTask(const PendingTask& pending_task);

  // Adds the pending task to _delayedWorkQueue.
  void addToDelayedWorkQueue(const PendingTask& pending_task);

  // Load tasks from the _incomingQueue into _workQueue if the latter is
  // empty.  The former requires a lock to access, while the latter is directly
  // accessible on this thread.
  void reloadWorkQueue();

  // Delete tasks that haven't run yet without running them.  Used in the
  // destructor to make sure all the task's destructors get called.  Returns
  // true if some work was done.
  bool deletePendingTasks();

  // Post a task to our incomming queue.
  void postTask_Helper(Task* task, uint32_t delay_ms, bool nestable);

  // EventPump::Delegate methods:
  virtual bool doWork();
  virtual bool doDelayedWork(Time* nextDelayedWorkTime);
  virtual bool doIdleWork();

  // Event loop and pump type.
  String32 _type;

  // A list of tasks that need to be processed by this instance.  Note that
  // this queue is only accessed (push/pop) by our current thread.
  TaskQueue _workQueue;
  
  // Contains delayed tasks, sorted by their 'delayedRunTime' property.
  DelayedTaskQueue _delayedWorkQueue;

  // A queue of non-nestable tasks that we had to defer because when it came
  // time to execute them we were in a nested event loop.  They will execute
  // once we're out of nested message loops.
  TaskQueue _deferredNonNestableWorkQueue;

  RefPtr<EventPump> _pump;
  ObserverList<DestructionObserver> _destructionObservers;

  // A recursion block that prevents accidentally running additonal tasks when
  // insider a (accidentally induced?) nested message pump.
  bool _nestableTasksAllowed;

  bool _exceptionRestoration;

  String32 _threadName;

  // A null terminated list which creates an incoming_queue of tasks that are
  // aquired under a mutex for processing on this instance's thread. These tasks
  // have not yet been sorted out into items for our _workQueue vs items that
  // will be handled by the TimerManager.
  TaskQueue _incomingQueue;
  // Protect access to _incomingQueue.
  Lock _incomingQueueLock;

  RunState* _state;

  // The next sequence number to use for delayed tasks.
  int _nextSequenceNum;

private:
  FOG_DISABLE_COPY(EventLoop)
};

} // Fog namespace

//! @}

#endif // _FOG_CORE_EVENTLOOP_H
