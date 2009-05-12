// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_THREADPOOL_H
#define _FOG_CORE_THREADPOOL_H

// [Dependencies]
#include <Fog/Core/Atomic.h>
#include <Fog/Core/Lock.h>
#include <Fog/Core/Thread.h>

namespace Fog {

// ============================================================================
// [Fog::Forward Declarations]
// ============================================================================

struct Thread;

// ============================================================================
// [Fog::ThreadPool]
// ============================================================================

struct FOG_API ThreadPool
{
  // [Construction / Destruction]

  ThreadPool();
  virtual ~ThreadPool();

  // [Thread Management]

  Thread* getThread();
  void releaseThread(Thread* thread);

  int minThreads() const;
  int maxThreads() const;
  int numThreads() const;

  err_t setMaxThreads(int maxThreads);

  struct PoolEntry
  {
    PoolEntry* next;
    Thread* thread;
  };

private:
  virtual Thread* _createThread();

  int _minThreads;
  int _maxThreads;
  int _numThreads;

  Lock _lock;
  PoolEntry* _usedThreads;
  PoolEntry* _unusedThreads;

  FOG_DISABLE_COPY(ThreadPool)
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_CORE_THREADPOOL_H
