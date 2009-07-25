// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// TODO: This header is deprecated and will be removed.

// [Guard]
#ifndef _FOG_CORE_REFDATA_H
#define _FOG_CORE_REFDATA_H

// [Dependencies]
#include <Fog/Core/Assert.h>
#include <Fog/Core/Atomic.h>
#include <Fog/Core/Constants.h>

namespace Fog {

template<typename T>
struct RefDataSimple
{
  //! @brief Reference count.
  mutable Atomic<sysuint_t> refCount;

  //! @brief Used to reference data if we know that flag 
  //! IsSharable is set.
  FOG_INLINE T* REF_ALWAYS()
  {
    refCount.inc();
    return static_cast<T*>(this);
  }

  FOG_INLINE void DEREF_INLINE()
  {
    if (refCount.deref()) static_cast<T*>(this)->free();
  }
};

template<typename T>
struct RefData
{
  //! @brief Basic RefData<T> flags that supports most classes.
  enum
  {
    //! @brief Null 'd' object. 
    //!
    //! This is very likely object that's shared between all null objects. So
    //! normally only one data instance can has this flag set on.
    IsNull = (1U << 0),

    //! @brief Object is created on the heap. 
    //!
    //! Object is created by function like @c malloc() or Fog::Memory::alloc() or
    //! by @c new operator. It this flag is not set, you can't delete object from
    //! the heap and object is probabbly only temporary (short life).
    IsDynamic = (1U << 1),

    //! @brief Object is shareable.
    //!
    //! Object can be directly referenced by internal method @c ref(). Sharable
    //! objects are usually created on the heap and together with this flag is set
    //! also @c IsDynamic, but it couldn't be.
    IsSharable = (1U << 2),

    //! @brief Prevents destroying from assigning operations.
    //!
    //! This flag is usually only for @c Fog::String family classes and it
    //! it means 
    //!   "Don't assign other object to me, instead, copy it to me directly!".
    IsStrong = (1U << 3)
  };

  //! @brief Reference count.
  mutable Atomic<sysuint_t> refCount;

  //! @brief Flags that contains informations about data, see @c Flags.
  uint32_t flags;

#if FOG_ARCH_BITS == 64
  uint32_t _data_padding;
#endif // FOG_ARCH_BITS == 64

  //! @brief Used to reference data by inlining.
  FOG_INLINE T* REF_INLINE()
  {
    if (flags & IsSharable)
    {
      refCount.inc();
      return static_cast<T*>(this);
    }
    else
    {
      return T::copy(static_cast<T*>(this), AllocCantFail);
    }
  }

  //! @brief Used to reference data if we know that flag IsSharable is set.
  FOG_INLINE T* REF_ALWAYS()
  {
    refCount.inc();
    return static_cast<T*>(this);
  }

  FOG_INLINE void DEREF_INLINE()
  {
    if (refCount.deref())
    {
      static_cast<T*>(this)->destroy();
      if (flags & IsDynamic) static_cast<T*>(this)->free();
    }
  }
};

} // Fog namespace

// [Guard]
#endif // _FOG_CORE_REFDATA_H
