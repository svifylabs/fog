// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Assert.h>
#include <Fog/Core/AutoLock.h>
#include <Fog/Core/Thread.h>
#include <Fog/Core/ThreadPool.h>

namespace Fog {

// ============================================================================
// [Fog::ThreadPool]
// ============================================================================

ThreadPool::ThreadPool() :
  // Safe defaults
  _minThreads(1),
  _maxThreads(32),
  _numThreads(0),
  _usedThreads(NULL),
  _unusedThreads(NULL)
{
}

ThreadPool::~ThreadPool()
{
}

Thread* ThreadPool::getThread(int workId)
{
  AutoLock locked(_lock);
  PoolEntry* pe = NULL;

  if (_unusedThreads)
  {
    // First try to find workId if specified
    if (workId >= 0)
    {
      PoolEntry* cur = _unusedThreads;
      PoolEntry* prev = NULL;
      
      while (cur)
      {
        if (cur->workId == workId)
        {
          if (prev)
            prev->next = cur->next;
          else
            _unusedThreads = cur->next;
          pe = cur;
          break;
        }

        prev = cur;
        cur = cur->next;
      }
    }
    
    if (pe == NULL)
    {
      pe = _unusedThreads;
      pe->workId = workId;
      _unusedThreads = pe->next;
    }
  }
  else if (_numThreads < _maxThreads)
  {
    pe = (PoolEntry*)Memory::alloc(sizeof(PoolEntry));
    if (!pe) return NULL;

    Thread* thread = _createThread();
    if (!thread)
    {
      Memory::free(pe);
      return NULL;
    }

    if (!thread->start(Ascii8("Default")))
    {
      delete thread;
      Memory::free(pe);
      return NULL;
    }

    pe->thread = thread;
    pe->workId = -1;
  }
  else
  {
    return NULL;
  }

  pe->next = _usedThreads;
  _usedThreads = pe;

  return pe->thread;
}

void ThreadPool::releaseThread(Thread* thread, int workId)
{
  AutoLock locked(_lock);

  PoolEntry* cur = _usedThreads;
  PoolEntry* prev = NULL;

  while (cur)
  {
    if (cur->thread == thread)
    {
      if (prev)
        prev->next = cur->next;
      else
        _usedThreads = cur->next;

      cur->next = _unusedThreads;
      if (workId != -1) cur->workId = workId;
      _unusedThreads = cur;

      return;
    }

    prev = cur;
    cur = cur->next;
  }

  fog_debug(
    "Fog::ThreadPool::releaseThread() - Releasing thread (%p) that is not in thread list", thread);
}

int ThreadPool::minThreads() const
{
  return _minThreads;
}

int ThreadPool::maxThreads() const
{
  return _maxThreads;
}

int ThreadPool::numThreads() const
{
  return _numThreads;
}

err_t ThreadPool::setMaxThreads(int maxThreads)
{
  _maxThreads = maxThreads;
  return Error::Ok;
}

Thread* ThreadPool::_createThread()
{
  String32 threadName;
  threadName.format("Fog::ThreadPool #", _numThreads);

  Thread* thread = new(std::nothrow) Thread(threadName);
  if (!thread) return NULL;

  // Update statistics
  _numThreads++;

  return thread;
}

} // Fog namespace
