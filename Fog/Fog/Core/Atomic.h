// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_ATOMIC_H
#define _FOG_CORE_ATOMIC_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Core/TypeInfo.h>

#if defined(FOG_CC_MSVC)
#include <intrin.h>
#pragma intrinsic (_InterlockedIncrement)
#pragma intrinsic (_InterlockedDecrement)
#if FOG_ARCH_BITS == 64
#pragma intrinsic (_InterlockedIncrement64)
#pragma intrinsic (_InterlockedDecrement64)
#endif // FOG_ARCH_BITS == 64
#define _MSINTRIN_(func) _##func
#else
#define _MSINTRIN_(func) func
#endif // FOG_CC_MSVC

//! @addtogroup Fog_Core
//! @{

namespace Fog {

// [Fog::AtomicOperation]
template<typename T, sysuint_t Size = sizeof(T)>
struct AtomicOperation {};

template<typename T>
struct AtomicOperation<T, 4>
{
  static FOG_INLINE void set(T* atomic, T value)
  {
    #if defined(FOG_CC_GNU) && (defined(FOG_ARCH_X86) || defined(FOG_ARCH_X86_64))
      __asm__ __volatile__
      (
        "movl %1, %0\n"
          : "=m" (*atomic)
          : "ir" (value), "m" (*atomic)
      );
    #elif defined(FOG_CC_MSVC) || (defined(FOG_CC_BORLAND) && defined(FOG_OS_WINDOWS))
      InterlockedExchange((LONG volatile*)atomic, (LONG)value);
    #else
      *(volatile T *)atomic = value;
    #endif
  }
  
  static FOG_INLINE T setXchg(T* atomic, T value)
  {
    #if defined(FOG_CC_GNU) && (defined(FOG_ARCH_X86) || defined(FOG_ARCH_X86_64))
      __asm__ __volatile__
      (
        "xchgl %0,%1"
          : "=r" (value)
          : "m" (*atomic), "0" (value)
          : "memory"
      );
      return value;
    #elif defined(FOG_CC_MSVC) || (defined(FOG_CC_BORLAND) && defined(FOG_OS_WINDOWS))
      return (T)InterlockedExchange((LONG volatile*)atomic, (LONG)value);
    #else
      #error "Fog::AtomicOperation<T, 4>::setXchg() - missing xchg implementation"
    #endif
  }
  
  static FOG_INLINE bool cmpXchg(T* atomic, T compar, T value)
  {
    #if defined(FOG_CC_GNU) && (defined(FOG_ARCH_X86) || defined(FOG_ARCH_X86_64))
      unsigned char result;
      __asm__ __volatile__
      (
        "lock cmpxchgl %2,%3\n"
        "sete %1\n"
          : "=a" (value), "=qm" (result)
          : "r" (value), "m" (*atomic), "0" (compar)
          : "memory"
      );
      return (bool)result;
    #elif defined(FOG_CC_MSVC) || (defined(FOG_CC_BORLAND) && defined(FOG_OS_WINDOWS))
      return (bool)(InterlockedCompareExchange((LONG*)atomic, (LONG)value, (LONG)compar) == (LONG)compar);
    #else
      #error "Fog::AtomicOperation<T, 4>::cmpXchg() - missing implementation"
    #endif
  }
  
  static FOG_INLINE T get(const T* atomic)
  {
    #if defined(FOG_CC_GNU) && (defined(FOG_ARCH_X86) || defined(FOG_ARCH_X86_64))
      T result;
      __asm__ __volatile__
      (
        "movl %1, %0\n"
          : "=r" (result)
          : "m" (*atomic)
      );
      return result;
    #else
      return *(volatile const T *)atomic;
    #endif
  }

  static FOG_INLINE void inc(T* atomic)
  {
    #if defined(FOG_CC_GNU) && (defined(FOG_ARCH_X86) || defined(FOG_ARCH_X86_64))
      __asm__ __volatile__
      (
        "lock incl %0\n"
          : "=m" (*atomic)
          : "m" (*atomic)
      );
    #elif defined(FOG_CC_MSVC) || (defined(FOG_CC_BORLAND) && defined(FOG_OS_WINDOWS))
      _MSINTRIN_(InterlockedIncrement)((LONG*)atomic);
    #else
      (*(volatile T *)atomic)++;
    #endif
  }
  
  static FOG_INLINE void dec(T* atomic)
  {
    #if defined(FOG_CC_GNU) && (defined(FOG_ARCH_X86) || defined(FOG_ARCH_X86_64))
      __asm__ __volatile__
      (
        "lock decl %0\n"
          : "=m" (*atomic)
          : "m" (*atomic)
      );
    #elif defined(FOG_CC_MSVC) || (defined(FOG_CC_BORLAND) && defined(FOG_OS_WINDOWS))
      _MSINTRIN_(InterlockedDecrement)((LONG*)atomic);
    #else
      (*(volatile T *)atomic)--;
    #endif
  }
  
  static FOG_INLINE void add(T* atomic, T value)
  {
    #if defined(FOG_CC_GNU) && (defined(FOG_ARCH_X86) || defined(FOG_ARCH_X86_64))
      __asm__ __volatile__
      (
        "lock addl %1, %0\n"
          : "=m" (*atomic)
          : "ir" (value), "m" (*atomic)
      );
    #elif defined(FOG_CC_MSVC) || (defined(FOG_CC_BORLAND) && defined(FOG_OS_WINDOWS))
      InterlockedExchangeAdd((LONG*)atomic, (LONG)value);
    #else
      (*(volatile T *)atomic) += value;
    #endif
  }
  
  static FOG_INLINE void sub(T* atomic, T value)
  {
    #if defined(FOG_CC_GNU) && (defined(FOG_ARCH_X86) || defined(FOG_ARCH_X86_64))
      __asm__ __volatile__
      (
        "lock subl %1, %0\n"
          : "=m" (*atomic)
          : "ir" (value), "m" (*atomic)
      );
    #elif defined(FOG_CC_MSVC) || (defined(FOG_CC_BORLAND) && defined(FOG_OS_WINDOWS))
      InterlockedExchangeAdd((LONG*)atomic, (LONG)((value ^ 0xFFFFFFFF) + 1U));
    #else
      (*(volatile T *)atomic) -= value;
    #endif
  }
  
  static FOG_INLINE T addXchg(T* atomic, T value)
  {
    #if defined(FOG_CC_GNU) && (defined(FOG_ARCH_X86) || defined(FOG_ARCH_X86_64))
      T result;
      __asm__ __volatile__
      (
        "lock xaddl %0, %1\n"
          : "=r" (result), "=m" (*atomic)
          : "0" (value), "m" (*atomic)
      );
      return result;
    #elif defined(FOG_CC_MSVC) || (defined(FOG_CC_BORLAND) && defined(FOG_OS_WINDOWS))
      return (T)InterlockedExchangeAdd((LONG*)atomic, (LONG)value);
    #else
      #error "Fog::AtomicOperation<T, 4>::addXchg() - missing implementation"
    #endif
  }
  
  static FOG_INLINE T subXchg(T* atomic, T value)
  {
    #if defined(FOG_CC_GNU) && (defined(FOG_ARCH_X86) || defined(FOG_ARCH_X86_64))
      T result;
      __asm__ __volatile__
      (
        "lock xaddl %0, %1\n"
          : "=r" (result), "=m" (*atomic)
          : "0" ((value ^ 0xFFFFFFFF) + 1U), "m" (*atomic)
      );
      return result;
    #elif defined(FOG_CC_MSVC) || (defined(FOG_CC_BORLAND) && defined(FOG_OS_WINDOWS))
      return (T)InterlockedExchangeAdd((LONG*)atomic, (LONG)((value ^ 0xFFFFFFFF) + 1U));
    #else
      #error "Fog::AtomicOperation<T, 4>::subXchg() - missing implementation"
    #endif
  }
  
  static FOG_INLINE bool deref(T* atomic)
  {
    #if defined(FOG_CC_GNU) && (defined(FOG_ARCH_X86) || defined(FOG_ARCH_X86_64))
      char result;
      
      __asm__ __volatile__
      (
        "lock decl %0\n"
        "setne %1\n"
          : "=m" (*atomic), "=qm" (result)
          : "m" (*atomic)
      );
      return !result;
    #elif defined(FOG_CC_MSVC) || (defined(FOG_CC_BORLAND) && defined(FOG_OS_WINDOWS))
      return _MSINTRIN_(InterlockedDecrement)((LONG*)atomic) == 0;
    #else
      return (*(volatile T *)atomic)-- == 0;
    #endif
  }
};

template<typename T>
struct AtomicOperation<T, 8>
{
  static FOG_INLINE void set(T* atomic, T value)
  {
    #if defined(FOG_CC_GNU)
      __asm__ __volatile__
      (
        "movq %1, %0\n"
          : "=m" (*atomic)
          : "ir" (value), "m" (*atomic)
      );
    #elif (defined(FOG_CC_MSVC) || (defined(FOG_CC_BORLAND) && defined(FOG_OS_WINDOWS)))
      InterlockedExchange64((int64_t*)atomic, (int64_t)value);
    #else
      (*(volatile T *)atomic) = value;
    #endif
  }
  
  static FOG_INLINE T setXchg(T* atomic, T value)
  {
    #if defined(FOG_CC_GNU)
      __asm__ __volatile__
      (
        "xchgq %0,%1"
          : "=r" (value)
          : "m" (*atomic), "0" (value)
          : "memory"
      );
      return value;
    #elif (defined(FOG_CC_MSVC) || (defined(FOG_CC_BORLAND) && defined(FOG_OS_WINDOWS)))
      return (T)InterlockedExchange64((int64_t*)atomic, (int64_t)value);
    #else
      #error "Fog::AtomicOperation<T, 8>::setXchg() - missing implementation"
    #endif
  }
  
  static FOG_INLINE bool cmpXchg(T* atomic, T compar, T value)
  {
    #if defined(FOG_CC_GNU)
      unsigned char result;
      __asm__ __volatile__
      (
        "lock cmpxchgq %2,%3\n"
        "sete %1\n"
          : "=a" (value), "=qm" (result)
          : "r" (value), "m" (*atomic), "0" (compar)
          : "memory"
      );
      return (bool)result;
    #elif (defined(FOG_CC_MSVC) || (defined(FOG_CC_BORLAND) && defined(FOG_OS_WINDOWS)))
      return (bool)(InterlockedCompareExchange64((int64_t*)atomic, (int64_t)value, (int64_t)compar) == (int64_t)compar);
    #else
      #error "Fog::AtomicOperation<T, 8>::cmpXchg() - missing implementation"
    #endif
  }
  
  static FOG_INLINE T get(T* atomic)
  {
    #if defined(FOG_CC_GNU)
      T result;
      __asm__ __volatile__
      (
        "movq %1, %0\n"
          : "=r" (result)
          : "m" (*atomic)
      );
      return result;
    #else
      return *(volatile T *)atomic;
    #endif
  }
  
  static FOG_INLINE void inc(T* atomic)
  {
    #if defined(FOG_CC_GNU)
      __asm__ __volatile__
      (
        "lock incq %0\n"
          : "=m" (*atomic)
          : "m" (*atomic)
      );
    #elif (defined(FOG_CC_MSVC) || (defined(FOG_CC_BORLAND) && defined(FOG_OS_WINDOWS)))
      _MSINTRIN_(InterlockedIncrement64)((int64_t*)atomic);
    #else
      (*(volatile T *)atomic)++;
    #endif
  }
  
  static FOG_INLINE void dec(T* atomic)
  {
    #if defined(FOG_CC_GNU) && defined(FOG_ARCH_X86_64)
      __asm__ __volatile__
      (
        "lock decq %0\n"
          : "=m" (*atomic)
          : "m" (*atomic)
      );
    #elif (defined(FOG_CC_MSVC) || (defined(FOG_CC_BORLAND) && defined(FOG_OS_WINDOWS)))
      _MSINTRIN_(InterlockedDecrement64)((int64_t*)atomic);
    #else
      (*(volatile T *)atomic)--;
    #endif
  }
  
  static FOG_INLINE void add(T* atomic, T value)
  {
    #if defined(FOG_CC_GNU)
      __asm__ __volatile__
      (
        "lock addq %1, %0\n"
          : "=m" (*atomic)
          : "ir" (value), "m" (*atomic)
      );
    #elif (defined(FOG_CC_MSVC) || (defined(FOG_CC_BORLAND) && defined(FOG_OS_WINDOWS)))
      InterlockedExchangeAdd64((int64_t*)atomic, (int64_t)value);
    #else
      (*(volatile T *)atomic) += value;
    #endif
  }
  
  static FOG_INLINE void sub(T* atomic, T value)
  {
    #if defined(FOG_CC_GNU)
      __asm__ __volatile__
      (
        "lock subq %1, %0\n"
          : "=m" (*atomic)
          : "ir" (value), "m" (*atomic)
      );
    #elif (defined(FOG_CC_MSVC) || (defined(FOG_CC_BORLAND) && defined(FOG_OS_WINDOWS)))
      InterlockedExchangeAdd64((int64_t*)atomic, (int64_t)((value ^ FOG_UINT64_C(0xFFFFFFFFFFFFFFFF)) + FOG_UINT64_C(1)));
    #else
      (*(volatile T *)atomic) -= value;
    #endif
  }
  
  static FOG_INLINE T addXchg(T* atomic, T value)
  {
    #if defined(FOG_CC_GNU)
      T result;
      __asm__ __volatile__
      (
        "lock xaddq %0, %1\n"
          : "=r" (result), "=m" (*atomic)
          : "0" (value), "m" (*atomic)
      );
      return result;
    #elif (defined(FOG_CC_MSVC) || (defined(FOG_CC_BORLAND) && defined(FOG_OS_WINDOWS)))
      return (T)InterlockedExchangeAdd64((int64_t*)atomic, (int64_t)value);
    #else
      #error "Fog::AtomicOperation<T, 8>::addXchg() - missing implementation"
    #endif
  }
  
  static FOG_INLINE T subXchg(T* atomic, T value)
  {
    #if defined(FOG_CC_GNU)
      T result;
      __asm__ __volatile__
      (
        "lock xaddq %0, %1\n"
          : "=r" (result), "=m" (*atomic)
          : "0" ((value ^ FOG_UINT64_C(0xFFFFFFFFFFFFFFFF)) + FOG_UINT64_C(1)), "m" (*atomic)
      );
      return result;
    #elif (defined(FOG_CC_MSVC) || (defined(FOG_CC_BORLAND) && defined(FOG_OS_WINDOWS)))
      return (T)InterlockedExchangeAdd64((int64_t*)atomic, (int64_t)((value ^ FOG_UINT64_C(0xFFFFFFFFFFFFFFFF)) + FOG_UINT64_C(1)));
    #else
      #error "Fog::AtomicOperation<T, 8>::subXchg() - missing implementation"
    #endif
  }
  
  static FOG_INLINE bool deref(T* atomic)
  {
    #if defined(FOG_CC_GNU)
      char result;
      
      __asm__ __volatile__
      (
        "lock decq %0\n"
        "setne %1\n"
          : "=m" (*atomic), "=qm" (result)
          : "m" (*atomic)
      );
      return !result;
    #elif (defined(FOG_CC_MSVC) || (defined(FOG_CC_BORLAND) && defined(FOG_OS_WINDOWS)))
      return _MSINTRIN_(InterlockedDecrement64)((int64_t*)atomic) == FOG_INT64_C(0);
    #else
      return (*(volatile T *)atomic)-- == 0UL;
    #endif
  }
};

// [Fog::AtomicBase]
struct AtomicBase
{
  // [T* type]

  template<typename T>
  static FOG_INLINE T* ptr_setXchg(T** atomic, T* value)
  {
    return (T*)(void*)AtomicOperation<void*>::setXchg((void**)atomic, (void*)value);
  }
};

// [Fog::Atomic<T>]
template<typename T>
struct Atomic
{
  typedef AtomicOperation<T> Op;

  mutable T _atomic;

  inline void init(T value)              { _atomic = value; }
  inline void set(T value)               { Op::set(&_atomic, value); }
  inline T setXchg(T value)              { return Op::setXchg(&_atomic, value); }
  inline bool cmpXchg(T compar, T value) { return Op::cmpXchg(&_atomic, compar, value); }
  inline T get() const                   { return Op::get(&_atomic); }
  inline void inc()                      { Op::inc(&_atomic); }
  inline void dec()                      { Op::dec(&_atomic); }
  inline void add(T value)               { Op::add(&_atomic, value); }
  inline void sub(T value)               { Op::sub(&_atomic, value); }
  inline T addXchg(T value)              { return Op::addXchg(&_atomic, value); }
  inline T subXchg(T value)              { return Op::subXchg(&_atomic, value); }
  inline bool deref()                    { return Op::deref(&_atomic); }
};

//! @brief Macro to help setting @c Fog::Atomic structure in static data.
#define FOG_ATOMIC_SET(value) { value }

// [Cleanup]
#undef _MSINTRIN_

} // Fog namespace

//! @}

// [TypeInfo]
FOG_DECLARE_TYPEINFO_TEMPLATE1(Fog::Atomic, typename, T, Fog::MoveableType)

// [Guard]
#endif // _FOG_CORE_ATOMIC_H
