// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_OBJECT_H
#define _FOG_CORE_OBJECT_H

// [Dependencies]
#include <Fog/Core/Atomic.h>
#include <Fog/Build/Build.h>
#include <Fog/Core/Char.h>
#include <Fog/Core/Class.h>
#include <Fog/Core/Hash.h>
#include <Fog/Core/HashUtil.h>
#include <Fog/Core/Delegate.h>
#include <Fog/Core/Event.h>
#include <Fog/Core/Thread.h>
#include <Fog/Core/Vector.h>

// [Forward Declarations]
namespace Fog { struct Event; }
namespace Fog { struct MetaClass; }
namespace Fog { struct Object; }

// [Fog::Object CAPI]

FOG_CVAR_EXTERN Fog::Lock* fog_object_lock;

FOG_CAPI_EXTERN void* fog_object_cast_helper(Fog::Object* self, const Fog::MetaClass* targetMetaClass);
FOG_CAPI_EXTERN void* fog_object_cast_string(Fog::Object* self, const char* className);

//! @addtogroup Fog_Core
//! @{

// [Fog::Object macros]

//!
//! @brief This macro should be used for each class declaration that inherits
//! from @c Fog::Object class to use object dynamic features. 
//!
//! This macro creates needed virtual methods and it's used together with
//! @c FOG_IMPLEMENT_OBJECT, see example:
//!
//! @verbatim
//! // header file
//!
//! // some namespace
//! namespace Namespace
//! {
//!   struct MyObject : public Fog::Object
//!   {
//!     // declare that object is MyObject and is derived from Fog::Object
//!     FOG_DECLARE_OBJECT(MyObject, Fog::Object)
//!
//!     MyObject();
//!     ~MyObject();
//!
//!     // ... object members and methods
//!   };
//! }
//!
//! // initialize object info, always use full name with namespace
//! // or meta class information will be invalid!
//! FOG_IMPLEMENT_OBJECT(Namespace::MyObject)
//!
//! // source file
//! namespace Namespace
//! {
//!   MyObject::MyObject()
//!   {
//!   }
//!
//!   MyObject::~MyObject()
//!   {
//!   }
//! }
//! @endverbatim
//!
//! @c MyObject from example inherits @c Core_Object and now it's able to use
//! Wde features like dynamic object identification and properties.
#define FOG_DECLARE_OBJECT(selftype, basetype) \
public: \
  /* [Typedefs] */ \
  \
  /*! @brief Self type of class. */ \
  typedef selftype __selftype__; \
  /*! @brief Base type of class. */ \
  typedef basetype __basetype__; \
  /*! @brief Base type of class. */ \
  typedef __basetype__ base; \
  \
  /* [Meta Class] */ \
  \
  /*! @brief Static method that's used for retrieving meta class information. */ \
  static const Fog::MetaClass* staticMetaClass(); \
  /*! @brief Virtual method that's used for retrieving meta class information. */ \
  virtual const Fog::MetaClass* metaClass() const;

#define FOG_IMPLEMENT_OBJECT(selftype) \
const Fog::MetaClass* selftype::staticMetaClass() \
{ \
  /* [Static Variables] */ \
  static Fog::MetaClass metaClass; \
  static const char metaClassName[] = #selftype; \
  \
  /* [Initialization States] */ \
  /* 0 - Not initialized     */ \
  /* 1 - Initializing        */ \
  /* 2 - Initialized         */ \
  static uint32_t initialized = 0; \
  \
  /* [Initialized Meta Class] */ \
  if (Fog::AtomicOperation<uint32_t>::get(&initialized) == 2) \
  { \
    return &metaClass; \
  } \
  \
  /* [Or Initialize It] */ \
  if (Fog::AtomicOperation<uint32_t>::cmpXchg(&initialized, 0, 1)) \
  { \
    metaClass.base = base::staticMetaClass(); \
    metaClass.name = metaClassName; \
    metaClass.hash = Fog::HashUtil::hashString(metaClassName, FOG_ARRAY_SIZE(metaClassName)-1); \
    \
    Fog::AtomicOperation<uint32_t>::set(&initialized, 1); \
    return &metaClass; \
  } \
  \
  /* [Race - Another thread is initializing Meta Class] */ \
  while (Fog::AtomicOperation<uint32_t>::get(&initialized) != 2) \
  { \
    Fog::Thread::_yield(); \
  } \
  \
  return &metaClass; \
} \
\
const Fog::MetaClass* selftype::metaClass() const \
{ \
  return staticMetaClass(); \
}

//! @brief Declare event handler for @c Fog::Object based class.
//!
//! This declaration overrides class virtual event handler. Virtual
//! event handler is fastest way to use event cathing and it's preferred
//! way of communication between @c Fog::Object based objects.
//!
//! This macro creates a fast compiled static table that can't be
//! overriden by addListener() or removeListener() methods.
#define fog_event_begin() \
virtual FOG_NO_INLINE void onEvent(Fog::Event* e) \
{ \
  switch (e->getCode()) \
  {



//! @brief Add event into event handled declared by 
//! @c fog_event_begin() macro.
//!
//! @param __event_code__ Event code, see event codes.
//! @param __event_class__ Event class (@c Fog::Event or derived)
//! @param __event_handler__ Event handler method (expression without parenthesis).
//! @param __event_type__ Event type, use @c override or @c cascade.
//!
//! Event type should be @c Override, because event handlers are usually 
//! virtual methods and these methods are more elegant than declaring 
//! them by other way.
#define fog_event(__event_code__, __event_handler__, __event_class__, __event_policy__) \
    case __event_code__: \
      if ((__event_policy__) == Reverse) base::onEvent(e); \
      \
      __event_handler__(reinterpret_cast<__event_class__*>(e)); \
      \
      if ((__event_policy__) == Cascade) \
        break; \
      else \
        return; 

//! @brief End of @c fog_event_begin() macro.
#define fog_event_end() \
    default: \
      /* event not found here or declared event is cascaded */ \
      /* into event handler. */ \
      break; \
  } \
  base::onEvent(e); \
}

// [fog_object_cast<>]

//! @brief Cast @a Core_Object* to @a T type.
template <typename T>
FOG_INLINE T fog_object_cast(Fog::Object* object)
{
  return static_cast<T>( (Fog::Object*) fog_object_cast_helper(object, ((T)NULL)->staticMetaClass()) );
}

//! @brief Cast const @a Core_Object* to @a T type.
template <typename T>
FOG_INLINE T fog_object_cast(const Fog::Object* object)
{
  return static_cast<T>( (const Fog::Object*) fog_object_cast_helper((Fog::Object*)object, ((T)NULL)->staticMetaClass()) );
}

namespace Fog {

// [Fog::ObjectConnection]

struct ObjectConnection
{
  ObjectConnection* next;
  Object* attachedObject;
  Object* listener;
  union {
    Static< Delegate0<> > delegateVoid;
    Static< Delegate1<Event*> > delegateEvent;
  };
  uint32_t type;
  uint32_t code;
};

// [Fog::MetaClass]

//! @brief Object record.
struct MetaClass
{
  //! @brief Base meta class. Only @c Fog::Object have this value NULL.
  const MetaClass* base;
  //! @brief Object class name. 
  const char* name;
  //! @brief Object class name hash.
  uint32_t hash;
};

// [Fog::Object]

struct FOG_API Object : public Class
{
  // don't use FOG_DECLARE_OBJECT macro here, this base class is special.

  //! @brief Self object type, always filled by @c FOG_DECLARE_OBJECT macro.
  typedef Object __selftype__;

  // [Construction & Destruction]

  Object();
  virtual ~Object();

  //! @brief Posts delete later event.
  void deleteLater();

  FOG_INLINE bool postedDeleteLaterEvent() const { return (_flags & PostedDeleteLaterEvent) != 0; }

  //! @brief Destroys object if it's allocated on the heap (calls delete)
  //!
  //! Destroy method is called from @c deref() or from @c Fog::Wrap
  //! template. This is only correct way to delete object from memory,
  //! because destroying process needs to send Fog::DestroyEvent thats
  //! only possible before object destructor is called.
  void destroy();

  // [Flags]

  //! @brief Returns object flags.
  FOG_INLINE uint32_t getFlags() const { return _flags; }
  
  //! @brief Returns @c true for dynamic instances (checked by @c Fog::Class).
  FOG_INLINE bool isDynamic() const { return (_flags & IsDynamic) != 0; }
  
  //! @brief Returns @c true if object is @c Fog::Widget.
  FOG_INLINE bool isWidget() const { return (_flags & IsWidget) != 0; }

  //! @brief Returns @c true if object is @c Fog::Layout.
  FOG_INLINE bool isLayout() const { return (_flags & IsLayout) != 0; }

  //! @brief Returns @c true if basic event @c Fog::EvCreate was posted.
  //FOG_INLINE bool postedCreateEvent() const { return (_flags & Flag_PostedCreateEvent) != 0; }
  //! @brief Returns @c true if basic event @c Fog::EvDeleteLater was posted.
  //FOG_INLINE bool postedDeleteLaterEvent() const { return (_flags & Flag_PostedDeleteLaterEvent) != 0; }
  //! @brief Returns @c true if basic event @c Fog::EvUpdate was posted.
  //FOG_INLINE bool postedUpdateEvent() const { return (_flags & Flag_PostedUpdateEvent) != 0; }

  // [Id]

  //! @brief Returns object id.
  FOG_INLINE uint32_t getObjectId() const  { return _objectId; }

  //! @brief Sets object id.
  void setObjectId(uint32_t objectId);

  // [Name]

  //! @brief Returns object name.
  FOG_INLINE String getObjectName() const { return _objectName; }

  //! @brief Sets object name.
  void setObjectName(const String& objectName);

  // [Threading]

  //! @brief Returns thread id where object lives.
  FOG_INLINE Thread* getThread() const { return _thread; }

  // [RTTI]

  //! @brief Static method that's used for retrieving dynamic class info.
  static const MetaClass* staticMetaClass();
  //! @brief Virtual method that's used for retrieving class info.
  virtual const MetaClass* getMetaClass() const;

  //! @brief Returns class name of this object.
  FOG_INLINE const char* getClassName() const { return getMetaClass()->name; }

  //! @brief Returns @c true if class can be casted to @a T.
  template<typename T>
  FOG_INLINE bool isClassOf() const { return fog_object_cast<T>((Object*)this) != NULL; }

  //! @brief Returns @c true if class can be casted to @a className.
  FOG_INLINE bool isClassOf(const char* className) const
  { return fog_object_cast_string((Object*)this, className) != NULL; }

  // [Event Handlers]

  //! @brief Add a pure C function listener @a fn.
  bool addListener(uint32_t code, void (*fn)(Event*));
  //! @overload
  bool addListener(uint32_t code, void (*fn)());

  //! @brief Removes a pure C function listener @a fn.
  bool removeListener(uint32_t code, void (*fn)(Event*));
  //! @overload
  bool removeListener(uint32_t code, void (*fn)());

  //! @brief Adds @c a Fog::Object* based function listener @a fn.
  template<class X, class Y, class Z>
  FOG_INLINE bool addListener(uint32_t code, Y *target, void (X::*fn)(Z*))
  {
    Delegate1<Z*> del(target, fn);
    return _addListener(code, target, (const void*)&del, EventDelegate);
  }

  //! @overload
  template<class X, class Y>
  FOG_INLINE bool addListener(uint32_t code, Y *target, void (X::*fn)())
  {
    Delegate0<> del(target, fn);
    return _addListener(code, target, (const void*)&del, VoidDelegate);
  }

  //! @brief Removes a @c Fog::Object* based function listener @a fn.
  template<class X, class Y, class Z>
  FOG_INLINE bool removeListener(uint32_t code, Y *target, void (X::*fn)(Z*))
  {
    Delegate1<Z*> del(target, fn);
    return _removeListener(code, target, (const void*)&del, EventDelegate);
  }

  //! @brief Removes a @c Fog::Object* based function listener @a fn.
  template<class X, class Y>
  FOG_INLINE bool removeListener(uint32_t code, Y *target, void (X::*fn)())
  {
    Delegate0<> del(target, fn);
    return _removeListener(code, target, (const void*)&del, VoidDelegate);
  }

  uint removeListener(Object* receiver);
  uint removeAllListeners();

  //! @brief Posts event @a e that will be processed by objects thread main loop. 
  //!
  //! Posted event will be sent by main loop by @c sendEvent() method.
  void postEvent(Event* e);
  //! @brief Posts simple event (only event id).
  void postEvent(uint32_t eventId);

  //! @brief Sends event @a e immediately.
  void sendEvent(Event* e);
  //! @brief Sends simple event (only event id) immediately.
  void sendEventById(uint32_t eventId);

  //! @brief Generic event handler.
  virtual void onEvent(Event* e);

  //! @brief Create object event.
  virtual void onCreate(CreateEvent* e);
  //! @brief Destroy object event.
  virtual void onDestroy(DestroyEvent* e);
  //! @brief Property changed event.
  virtual void onPropertyChanged(PropertyChangedEvent* e);

  // [OFlags constants]

  enum OFlags
  {
    // [Fog::Class]

    // IsDynamic = (1 << 0),

    // [Event handlers flags - optimization]

    //HasChildAddedEventHandler = (1 << 2),
    //HasChildRemovedEventHandler = (1 << 3),
    //HasParentChangedEventHandler = (1 << 4),

    //! @brief Object was posted 'Create' event.
    PostedCreateEvent = (1 << 5),
    //! @brief Object was posted 'DeleteLater' event.
    PostedDeleteLaterEvent = (1 << 6),

    // [Fog::UI flags]

    //! @brief Object type is @c Fog::Widget.
    IsWidget = (1 << 16),
    IsLayout = (1 << 17)
  };

  // [Event Policy]

  enum EventPolicy
  {
    Reverse  = 0,
    Cascade  = 1,
    Override = 2
  };

  // [Delegate Type]

  enum DelegateType
  {
    VoidDelegate = 0,
    EventDelegate = 1
  };

protected:
  //! @brief Object flags
  uint32_t _flags;

  //! @brief Object id.
  //!
  //! Object id is not unique and you can't get global object by it's
  //! id. But it can help you identifying your objects by very fast
  //! way and ability to use C/C++ switch() statement.
  //! 
  //! @sa _name
  uint32_t _objectId;

  //! @brief Object name.
  //!
  //! @sa _objectId
  String _objectName;

  //! @brief Attached listeners to this object.
  //!
  //! @note Access to this structure must be always locked by
  //! @c fog_object_lock.
  Hash<uint32_t, ObjectConnection*> _connection;

  //! @brief Back reference to connections.
  //!
  //! @note Access to this structure must be always locked by
  //! @c fog_object_lock.
  Vector<ObjectConnection*> _backref;

  //! @brief Object thread where it lives.
  //!
  //! NULL if object is created in anonymous thread (thread that's not created
  //! using Fog::Thread class. In that case event system is unusable for that
  //! thread.
  Thread* _thread;

  //! @brief Link to last pending event or NULL if there are
  //! no pending events for this object.
  //! 
  //! @note Access to this structure must be always locked by @c fog_object_lock.
  Event* _events;

private:
  bool _addListener(uint32_t code, Fog::Object* receiver, const void* del, uint32_t type);
  bool _removeListener(uint32_t code, Fog::Object* receiver, const void* del, uint32_t type);
  void _callListeners(Event* e);

  friend struct Event;

  FOG_DISABLE_COPY(Object)
};

} // Fog namespace

// [fog_object_cast<Fog::Widget*>]
namespace Fog { struct Layout; }
namespace Fog { struct Widget; }

// fog_object_cast<Fog::Widget*> specialization.
template<>
FOG_INLINE Fog::Widget* fog_object_cast(Fog::Object* object)
{
  FOG_ASSERT(object);
  return (Fog::Widget*)(object->isWidget() ? object : NULL);
}

template<>
FOG_INLINE const Fog::Widget* fog_object_cast(const Fog::Object* object)
{
  FOG_ASSERT(object);
  return (Fog::Widget*)(object->isWidget() ? object : NULL);
}

// fog_object_cast<Fog::Layout*> specialization.
template<>
FOG_INLINE Fog::Layout* fog_object_cast(Fog::Object* object)
{
  FOG_ASSERT(object);
  return (Fog::Layout*)(object->isLayout() ? object : NULL);
}

template<>
FOG_INLINE const Fog::Layout* fog_object_cast(const Fog::Object* object)
{
  FOG_ASSERT(object);
  return (Fog::Layout*)(object->isLayout() ? object : NULL);
}

//! @}

// ============================================================================
// [Fog::TypeInfo<>]
// ============================================================================

FOG_DECLARE_TYPEINFO(Fog::ObjectConnection, Fog::PrimitiveType)

// [Guard]
#endif // _FOG_CORE_OBJECT_H
