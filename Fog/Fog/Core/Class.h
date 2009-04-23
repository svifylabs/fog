// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_CLASS_H
#define _FOG_CORE_CLASS_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Core/Constants.h>
#include <Fog/Core/Flags.h>
#include <Fog/Core/Memory.h>

//! @addtogroup Fog_Core_CAPI
//! @{

FOG_CAPI_EXTERN void* Core_Class_create(sysuint_t size);
FOG_CAPI_EXTERN void Core_Class_setup32(void* addr, uint32_t* flags);
FOG_CAPI_EXTERN void Core_Class_setup64(void* addr, uint64_t* flags);

//! @}

//! @addtogroup Fog_Core
//! @{

namespace Fog {

// ============================================================================
// [Fog::Class]
// ============================================================================

//! @brief Dynamic class implementation.
//!
//! This class is used to get information about allocation of class instance.
//! Allocation can be static (on the stack) or dynamic (on the heap). If class
//! instance is allocated on the heap, @c Fog::Class constructor will set
//! @c Fog::Class::IsDynamic flag to a given pointer.
//!
//! This class is very low level and not contains methods or variables
//!
//! See new example for usability:
//! @verbatim
//! struct MyClass : public Fog::Class
//! {
//! protected:
//!   uint32_t flags;
//!
//! public:
//!   MyClass() : Fog::Class(&flags) {}
//!   ~MyClass() {}
//!
//!   // check if class is dynamic instance
//!   inline bool isDynamic() const 
//!   { 
//!     return flags & IsDynamic;
//!   }
//!
//!   // delete only dynamic instance
//!   inline void deleteDynamic()
//!   {
//!     if (isDynamic()) delete this;
//!   }
//! };
//! // create static instance
//! MyClass objectS;
//!
//! // object1 and object2 are both pointer based types, but object2 is
//! // allocated on the heap (see new operator)
//! MyClass* object1 = &objectS;
//! MyClass* object2 = new MyClass;
//!
//! // test for dynamic instances
//! object1->isDynamic(); // == false
//! object2->isDynamic(); // == true
//!
//! // ...some code here...
//!
//! // now, we have two objects and all of them should be destroyed, we can
//! // simply use @c deleteDynamic() to do this job
//! object1->deleteDynamic(); // will not be destroyed
//! object2->deleteDynamic(); // will be destroyed
//! @endverbatim
struct Class
{
  // declare dynamic class checking that allows to mix static and dynamic 
  // instances together

  // Overloaded new operators - new[] operator will not mark class as dynamic!
  FOG_INLINE void* operator new  (size_t size) { return Core_Class_create(size); }
  FOG_INLINE void* operator new[](size_t size) { return Memory::xalloc(size); }

  // Overloaded delete operators - just call Fog::Memory::free(), nothing special to do
  FOG_INLINE void  operator delete  (void* addr) { Memory::free(addr); }
  FOG_INLINE void  operator delete[](void* addr) { Memory::free(addr); }

  // Placement new and delete
  FOG_INLINE void* operator new  (size_t, void* ptr) { return ptr; }
  FOG_INLINE void* operator new[](size_t, void* ptr) { return ptr; }

  FOG_INLINE void  operator delete  (void*, void*) {}
  FOG_INLINE void  operator delete[](void*, void*) {}

  //! @brief Dynamic class checker constructor.
  //!
  //! This constructor guarantes that correct behavior will be set to @c flag
  //! variable.
  FOG_INLINE Class(uint32_t* flags) { Core_Class_setup32((void*)this, flags); }
  FOG_INLINE Class(uint64_t* flags) { Core_Class_setup64((void*)this, flags); }
  //! @overload
  FOG_INLINE Class(Flags32* flags) { Core_Class_setup32((void*)this, (uint32_t *)flags); }
  FOG_INLINE Class(Flags64* flags) { Core_Class_setup64((void*)this, (uint64_t *)flags); }

  //! @brief @c Fog::Class enumeration that has only @c IsDynamic member.
  enum
  {
    //! @brief @c true if class is allocated on the heap (dynamic instance).
    IsDynamic = (1 << 0)
  };
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_CORE_CLASS_H
