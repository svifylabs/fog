// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Atomic.h>
#include <Fog/Core/AutoLock.h>
#include <Fog/Core/Event.h>
#include <Fog/Core/Lock.h>
#include <Fog/Core/Object.h>

namespace Fog {

// [Fog::Event]

static Atomic<uint32_t> uid_val = FOG_ATOMIC_SET(EvUID);

void Event::run()
{
  if (!wasDeleted())
  {
    Object* r = receiver();

    // Remove object from its event queue
    {
      Fog::AutoLock locked(*fog_object_lock);

      if (this == r->_events)
      {
        // This is LAST event
        FOG_ASSERT(_prev == NULL);
        FOG_ASSERT(_next == NULL);

        r->_events = NULL;
      }
      else
      {
        // This is not last event
        FOG_ASSERT(_prev == NULL);
        FOG_ASSERT(_next);

        _next->_prev = NULL;
        _next = NULL;
      }
    }

    r->onEvent(this);
  }
}

Event* Event::clone() const 
{
  return ( new Event(code(), flags()) )->_cloned(receiver() );
}

uint32_t Event::uid()
{
  uint32_t id = uid_val.addXchg(1);

  if (id == (uint32_t)-1)
  {
    fog_fail("Fog::Event::uid() - Event counter reached to the end.");
  }

  return id;
}

// [Fog::CreateEvent]

CreateEvent::CreateEvent() : Event(EvCreate)
{
}

// [Fog::DestroyEvent]

DestroyEvent::DestroyEvent()  : Event(EvDestroy)
{
}

void DestroyEvent::run()
{
  Event::run();
}

// [Fog::TimerEvent]

TimerEvent::TimerEvent(Timer* timer) : 
  Event(EvTimer),
  _timer(timer)
{
}

Event* TimerEvent::clone()
{
  return ( new TimerEvent(timer()) )->_cloned(receiver());
}

// [Fog::PropertyChangedEvent]

PropertyChangedEvent::PropertyChangedEvent(const String32& name) :
  Event(EvPropertyChanged),
  _name(name)
{
}

PropertyChangedEvent::~PropertyChangedEvent()
{
}

Event* PropertyChangedEvent::clone() const
{ 
  return ( new PropertyChangedEvent(name()) )->_cloned(receiver());
}

} // Fog namespace
