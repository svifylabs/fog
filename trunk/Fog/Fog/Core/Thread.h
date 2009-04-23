// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// [Guard]
#ifndef _FOG_CORE_THREAD_H
#define _FOG_CORE_THREAD_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Core/Atomic.h>
#include <Fog/Core/Class.h>
#include <Fog/Core/EventLoop.h>
#include <Fog/Core/Flags.h>
#include <Fog/Core/String.h>
#include <Fog/Core/ThreadEvent.h>

#if defined(FOG_OS_POSIX)
#include <pthread.h>
#endif

//! @addtogroup Fog_Core_CAPI
//! @{

// [FOG_THREAD_VAR]

//!
//! @def FOG_THREAD_VAR
//! @brief Per thread variable declaration.
//!
//! Variable declared by @c FOG_THREAD_VAR uses specific compiler
//! keywords and must be always declared as static
//!
//! @note This specific keyword is not supported by all compilers and it's
//! very unportable to use it directly. See @c Fog::ThreadLocalStorage if
//! you want to use per thread variables.
#if defined(FOG_CC_GNU) && !defined(__MINGW32__)
#define FOG_THREAD_VAR __thread
#elif defined(FOG_CC_MSVC)
#define FOG_THREAD_VAR __declspec(thread)
#endif

//! @}

//! @addtogroup Fog_Core
//! @{

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct Application;

// ============================================================================
// [Fog::Thread]
// ============================================================================

//! A simple thread abstraction that establishes a EventLoop on a new thread.
//! The consumer uses the EventLoop of the thread to cause code to execute on
//! the thread.  When this object is destroyed the thread is terminated.  All
//! pending tasks queued on the thread's message loop will run to completion
//! before the thread is terminated.
//!
//! If you want to run backgroun thread with no event loop. Use 
//! Fog::EventLoop::TypeNone and override main() method.
struct FOG_API Thread : public Class
{
public:
  // Fog::Thread::Handle should not be assumed to be a numeric type, 
  // since the standard intends to allow pthread_t to be a structure. This 
  // means you should not initialize it to a value, like 0.  If it's a member 
  // variable, the constructor can safely "value initialize" using () in the 
  // initializer list.
#if defined(FOG_OS_WINDOWS)
  /*! @brief Thread system handle. */
  typedef void* Handle;
#elif defined(FOG_OS_POSIX)
  /*! @brief Thread system handle. */
  typedef pthread_t Handle;
#endif

  // Static methods for current thread

  /*! @brief Gets the current thread id, which may be useful for logging purposes. */
  static uint32_t _tid();

  /*! @brief Yield the current thread so another thread can be scheduled. */
  static void _yield();

  /*! @brief Sleeps for the specified duration (units are milliseconds). */
  static void _sleep(uint32_t ms);

  /*! @brief Sets the thread name visible to a debugger. This has no effect otherwise. */
  static void _setName(const String32& name);

  //! @brief Creates a new thread.  The @c stackSize parameter can be 0 to 
  //! indicate that the default stack size should be used. Upon success,
  //! @c *handle will be assigned a handle to the newly created thread, and
  //! @c d's main() method will be executed on the newly created thread.
  //!
  //! NOTE: When you are done with the thread handle, you must call join() to
  //! release system resources associated with the thread. You must ensure that
  //! the Delegate object outlives the thread.
  static bool _create(sysuint_t stackSize, Thread* thread);

  //! @brief Joins with a thread created via the create() function. This 
  //! function blocks the caller until the designated thread exits. This
  //! will invalidate @c handle.
  static void _join(Thread* thread);

  // Main

  static FOG_INLINE Thread* mainThread()
  { return _mainThread; }

  static FOG_INLINE uint32_t mainThreadId()
  { return _mainThreadId; }

  static FOG_INLINE bool isMainThread()
  { return _tid() == _mainThreadId; }

  // Current

  static Thread* current();

  //! Constructor.
  //! name is a display string to identify the thread.
  explicit Thread(const String32& name = String32());

  //! Destroys the thread, stopping it if necessary.
  //!
  //! NOTE: If you are subclassing from Thread, and you wish for your cleanUp
  //! method to be called, then you need to call stop() from your destructor.
  virtual ~Thread();

  // Options

  //! Specifies the maximum stack size that the thread is allowed to use.
  //! This does not necessarily correspond to the thread's initial stack size.
  //! A value of 0 indicates that the default maximum should be used.
  void setStackSize(sysuint_t ssize);

  //! @brief Get thread stack size (0 means default).
  FOG_INLINE sysuint_t stackSize() const { return _stackSize; }

  //! Starts the thread.  Returns true if the thread was successfully started;
  //! otherwise, returns false.  Upon successful return, the eventLoop()
  //! getter will return non-null.
  //!
  //! Note: This function can't be called on Windows with the loader lock held;
  //! i.e. during a DllMain, global object construction or destruction, atexit()
  //! callback.
  virtual bool start(const String32& eventLoopType);

  //! Signals the thread to exit and returns once the thread has exited.  After
  //! this method returns, the Thread object is completely reset and may be used
  //! as if it were newly constructed (i.e., Start may be called again).
  //!
  //! Stop may be called multiple times and is simply ignored if the thread is
  //! already stopped.
  //!
  //! NOTE: This method is optional.  It is not strictly necessary to call this
  //! method as the Thread's destructor will take care of stopping the thread if
  //! necessary.
  virtual void stop();

  //! Signals the thread to exit in the near future.
  //!
  //! WARNING: This function is not meant to be commonly used. Use at your own
  //! risk. Calling this function will cause eventLoop() to become invalid in
  //! the near future. This function was created to workaround a specific
  //! deadlock on Windows with printer worker thread. In any other case, stop()
  //! should be used.
  //!
  //! StopSoon should not be called multiple times as it is risky to do so. It
  //! could cause a timing issue in eventLoop() access. Call stop() to reset
  //! the thread object once it is known that the thread has quit.
  void stopSoon();

  //! Thread flags
  FOG_INLINE uint32_t flags() const { return _flags; }

  //! The native thread handle.
  FOG_INLINE Handle& handle() { return _handle; }
  FOG_INLINE const Handle& handle() const { return _handle; }

  //! The native thread id
  FOG_INLINE uint32_t id() const { return _id; }

  //! Get the name of this thread (for display in debugger too).
  FOG_INLINE const String32& name() const { return _name; }

  //! Returns the message loop for this thread.  Use the EventLoop's
  //! postTask methods to execute code on the thread.  This only returns
  //! non-null after a successful call to Start.  After Stop has been called,
  //! this will return NULL.
  //!
  //! NOTE: You must not call this EventLoop's quit() method directly.  Use
  //! the Thread's stop() method instead.
  FOG_INLINE EventLoop* eventLoop() const { return _eventLoop; }

  //! Called just prior to starting the message loop
  virtual void init();

  virtual void main();
  virtual void finished();

  //! Called just after the message loop ends
  virtual void cleanUp();

private:
  //! We piggy-back on the _startupData member to know if we successfully
  //! started the thread.  This way we know that we need to call Join.
  FOG_INLINE bool _threadWasStarted() const { return _startupData != NULL; }

  //! The thread's handle.
  Handle _handle;

  //! Thread flags
  Flags<uint32_t> _flags;

  //! Thread id. Used for debugging purposes.
  uint32_t _id;

  //! The name of the thread. Used for debugging purposes.
  String32 _name;

  //! Stack size.
  sysuint_t _stackSize;

  //! Used to pass data to threadMain. This structure is allocated on the stack
  //! from within start().
  struct StartupData
  {
    //! @brief Event loop type (string).
    String32 eventLoopType;

    //! @brief Used to synchronize thread startup.
    ThreadEvent event;

    StartupData(const String32& eventLoopType) : 
      eventLoopType(eventLoopType),
      event(false, false)
    {
    }
  };

  StartupData* _startupData;

  //! The thread's message loop.  Valid only while the thread is alive.  Set
  //! by the created thread.
  EventLoop* _eventLoop;

  static Thread* _mainThread;
  static uint32_t _mainThreadId;

  friend struct ThreadQuitTask;
  friend struct MainThread;

private:
  friend struct Application;

  FOG_DISABLE_COPY(Thread)
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_CORE_THREAD_H
