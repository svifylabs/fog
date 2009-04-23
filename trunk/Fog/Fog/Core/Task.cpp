// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/EventLoop.h>
#include <Fog/Core/Task.h>
#include <Fog/Core/Thread.h>

namespace Fog {

// [Fog::Task]

Task::Task() : _deleteOnFinish(true)
{
}

Task::~Task()
{
}

void QuitTask::run()
{
  EventLoop::current()->quit();
}

void ThreadQuitTask::run()
{
  EventLoop::current()->quit();
}

//[Fog::]
}
