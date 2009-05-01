// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/AutoLock.h>
#include <Fog/Core/Error.h>
#include <Fog/Core/Event.h>
#include <Fog/Core/EventLoop.h>
#include <Fog/Core/Hash.h>
#include <Fog/Core/Object.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/String.h>
#include <Fog/Core/Strings.h>
#include <Fog/Core/StringUtil.h>
#include <Fog/Core/Thread.h>
#include <Fog/Core/Vector.h>

FOG_CVAR_DECLARE Fog::Lock* fog_object_lock;

// [Forward declarations]
// static uint32_t Core_Object_signalHash(const char* signal);

// [Fog::Object - fog_object_local variables]
struct Core_Object_Local
{
  FOG_INLINE Core_Object_Local()
  {
    static const char object_string[] = "Fog::Object";

    fog_object_lock = &object_lock;

    object_metaClass.base = NULL;
    object_metaClass.name = (const Fog::Char8*)object_string;
    object_metaClass.hash = Fog::hashString(
      (const Fog::Char8*)object_string, FOG_ARRAY_SIZE(object_string)-1);
  }
  FOG_INLINE ~Core_Object_Local()
  {
    fog_object_lock = NULL;
  }

  Fog::Lock object_lock;
  Fog::MetaClass object_metaClass;
};
static Fog::Static<Core_Object_Local> fog_object_local;

// [Fog::Object - cast]

FOG_CAPI_DECLARE void* fog_object_cast_helper(Fog::Object* self, const Fog::MetaClass* targetMetaClass)
{
  FOG_ASSERT(self);
  register const Fog::MetaClass* selfMetaClass = self->metaClass();

  for (;;)
  {
    // compare meta classes for match. Each class has only one meta class,
    // so pointers comparision is the best way
    if (selfMetaClass == targetMetaClass) return (void*)self;
    // iterate over base classes and return if there is no one
    if ((selfMetaClass = selfMetaClass->base) == NULL) return NULL;
  }
}

FOG_CAPI_DECLARE void* fog_object_cast_string(Fog::Object* self, const Fog::Char8* className)
{
  FOG_ASSERT(self);

  const Fog::MetaClass* metaClass = self->metaClass();
  uint32_t classHash = Fog::hashString(className, Fog::DetectLength);

  for (;;)
  {
    // Compare hashes and string class names.
    if (metaClass->hash == classHash && strcmp(
      (const char*)metaClass->name, 
      (const char*)className) == 0)
    {
      return (void*)self;
    }

    // Iterate over base classes and return if there is no one.
    if ((metaClass = metaClass->base) == NULL) return NULL;
  }
}

namespace Fog {

// [Fog::Object]

Object::Object() :
  Class(reinterpret_cast<uint32_t*>(&_flags)),
  _objectId(0),
  _thread(Thread::current()),
  _events(NULL)
{
}

Object::~Object()
{
  // Delete all connections
  if (!_connection.isEmpty()) removeAllListeners();

  // Demove all posted events.
  if (_events)
  {
    AutoLock locked(*fog_object_lock);

    // set "wasDeleted" in all pending events
    Event* e = _events;
    while (e)
    {
      e->_wasDeleted = 1;
      // Go to previous, because newest pending events are stored
      // first
      e = e->_prev;
    }
  }
}

void Object::deleteLater()
{
  if (!(_flags & PostedDeleteLaterEvent))
  {
    if (!thread()) 
    {
      fog_stderr_msg("Fog::Object", "deleteLater", "Can't post DeleteLater event, because object thread has no event loop");
      return;
    }

    _flags |= PostedDeleteLaterEvent;
    postEvent(new DestroyEvent());
  }
}

void Object::destroy()
{
  if (isDynamic()) delete this;
}

const MetaClass* Object::staticMetaClass()
{
  return &fog_object_local.instance().object_metaClass;
}

const MetaClass* Object::metaClass() const
{
  return &fog_object_local.instance().object_metaClass;
}

// [Fog::Object - id]

void Object::setObjectId(uint32_t objectId)
{
  if (_objectId != objectId)
  {
    _objectId = objectId;
  }
}

// [Fog::Object - name]

void Object::setObjectName(const String32& objectName)
{
  if (_objectName != objectName)
  {
    _objectName = objectName;
    _objectName.squeeze();
  }
}

// [Fog::Object - listeners]

bool Object::addListener(uint32_t code, void (*fn)(Event*))
{
  Delegate1<Event*> del(fn);
  return _addListener(code, NULL, (const void*)&del, EventDelegate);
}

bool Object::addListener(uint32_t code, void (*fn)())
{
  Delegate0<> del(fn);
  return _addListener(code, NULL, (const void*)&del, VoidDelegate);
}

bool Object::removeListener(uint32_t code, void (*fn)(Event*))
{
  Delegate1<Event*> del(fn);
  return _removeListener(code, NULL, (const void*)&del, EventDelegate);
}

bool Object::removeListener(uint32_t code, void (*fn)())
{
  Delegate0<> del(fn);
  return _removeListener(code, NULL, (const void*)&del, VoidDelegate);
}

uint Object::removeListener(Object* listener)
{
  AutoLock locked(*fog_object_lock);
  uint result = 0;

  ObjectConnection* prev;
  ObjectConnection* conn;
  ObjectConnection* next;

  Hash<uint32_t, ObjectConnection*>::MutableIterator it(_connection);

  for (it.toBegin(); it.isValid(); )
  {
    prev = NULL;
    conn = it.value();

    do {
      next = conn->next;
      if (conn->listener == listener)
      {
        result++;

        if (listener)
        {
          listener->_backref.remove(conn);
        }

        delete conn;

        if (prev)
          prev->next = next;
        else if (next)
          it.value() = next;
        else
        {
          it.remove();
          goto removed;
        }
      }

      prev = conn;
      conn = next;
    } while (conn);

    it.toNext();
removed:
    continue;
  }

  return result;
}

uint Object::removeAllListeners()
{
  AutoLock locked(*fog_object_lock);

  uint result = 0;

  ObjectConnection* conn;
  ObjectConnection* next;

  Hash<uint32_t, ObjectConnection*>::MutableIterator it(_connection);

  for (it.toBegin(); it.isValid(); it.toNext())
  {
    conn = it.value();

    do {
      next = conn->next;
      result++;

      if (conn->listener)
      {
        conn->listener->_backref.remove(conn);
      }

      delete conn;
      conn = next;
    } while (conn);
  }

  _connection.clear();
  return result;
}

// private
bool Object::_addListener(uint32_t code, Object* listener, const void* del, uint32_t type)
{
  AutoLock locked(*fog_object_lock);

  ObjectConnection* prev = NULL;
  ObjectConnection* conn = _connection.value(code, NULL);
  Delegate0<> d = *(const Delegate0<> *)del;

  if (conn)
  {
    do {
      if (conn->delegateVoid.instance() == d) return false;
      prev = conn;
      conn = conn->next;
    } while (conn);
  }

  conn = new ObjectConnection();
  conn->next = NULL;
  conn->attachedObject = this;
  conn->listener = listener;
  conn->delegateVoid.instance() = d;
  conn->type = type;
  conn->code = code;

  if (prev)
    prev->next = conn;
  else
    _connection.put(code, conn);

  if (listener) listener->_backref.append(conn);
  return true;
}

bool Object::_removeListener(uint32_t code, Object* listener, const void* del, uint32_t type)
{
  AutoLock locked(*fog_object_lock);

  ObjectConnection* prev = NULL;
  ObjectConnection* conn = _connection.value(code, NULL);
  if (!conn) return false;
  Delegate0<> d = *(const Delegate0<> *)del;

  do {
    ObjectConnection* next = conn->next;

    if (conn->delegateVoid.instance() == d)
    {
      FOG_ASSERT(conn->listener == listener);

      if (prev)
        prev->next = next;
      else if (next)
        _connection.put(code, next);
      else
        _connection.remove(code);

      if (listener) listener->_backref.remove(conn);
      
      delete conn;
      return true;
    }

    prev = conn;
    conn = next;
  } while (conn);

  return false;
}

// TODO: Add removing possibility for currently called listeners
void Object::_callListeners(Event* e)
{
  uint32_t code = e->code();

  ObjectConnection* conn = _connection.value(code);
  if (!conn) return;

  do {
    if (conn->type == VoidDelegate)
      conn->delegateVoid.instance()();
    else
      conn->delegateEvent.instance()(e);
  } while ((conn = conn->next));
}

// [Fog::Object - events]

void Object::postEvent(Event* e)
{
  EventLoop* el = EventLoop::current();
  
  if (el)
  {
    // Add event to its object queue
    {
      Fog::AutoLock locker(*fog_object_lock);
      e->_prev = this->_events;
      this->_events = e;

      e->_flags |= Event::IsPosted;
    }
    // Post event (event is posted as task)
    el->postTask(e);
  }
  else
  {
    fog_stderr_msg("Fog::Object", "postEvent", "Can't post event to object which thread hasn't event loop.");
    delete e;
  }
}

void Object::postEvent(uint32_t code)
{
  postEvent(new Event(code, 0));
}

void Object::sendEvent(Event* e)
{
  // First set receiver to self
  e->_receiver = this;

  // Send event to object onEvent()
  onEvent(e);

  // Send event to listeners
  _callListeners(e);
}

void Object::sendEventById(uint32_t code)
{
  Event e(code, 0);
  sendEvent(&e);
}

// event handlers

// onEvent is normally defined by fog_event_begin() and fog_event_end() macro,
// but Fog::Object is exception
void Object::onEvent(Event* e)
{
  switch (e->code())
  {
    case Fog::EvCreate:
      onCreate(reinterpret_cast<CreateEvent*>(e));
      break;
    case Fog::EvDestroy:
      onDestroy(reinterpret_cast<DestroyEvent*>(e));
      break;
  }
}

void Object::onCreate(CreateEvent* e)
{
}

void Object::onDestroy(DestroyEvent* e)
{
}

void Object::onPropertyChanged(PropertyChangedEvent* e)
{
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_object_init(void)
{
  fog_object_local.init();
  return Error::Ok;
}

FOG_INIT_DECLARE void fog_object_shutdown(void)
{
  fog_object_local.destroy();
}
