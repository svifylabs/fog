// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// [Guard]
#ifndef _FOG_CORE_REFCOUNTED_H
#define _FOG_CORE_REFCOUNTED_H

// [Dependencies]
#include <Fog/Core/Atomic.h>
#include <Fog/Build/Build.h>
#include <Fog/Core/Class.h>

namespace Fog {

/*! @brief Base class for Fog::RefCounted<T> template. */
struct RefCountedBase
{
protected:
  Atomic<sysuint_t> _refCount;

  FOG_INLINE RefCountedBase() { _refCount.init(0); }
  FOG_INLINE ~RefCountedBase() {}

  FOG_INLINE sysint_t refCount() const { return _refCount.get(); }

  FOG_INLINE void addRef() { _refCount.inc(); }

  // Returns true if the object should self-delete.
  FOG_INLINE bool release() { return _refCount.deref(); }

private:
  FOG_DISABLE_COPY(RefCountedBase)
};

/*!
  A base class for reference counted classes.  Otherwise, known as a cheap
  knock-off of WebKit's RefCounted<T> class.  To use this guy just extend your
  class from it like so:

    class MyFoo : public Fog::RefCounted<MyFoo> {
      ...
    };
*/
template <typename T>
struct RefCounted : public RefCountedBase
{
public:
  FOG_INLINE RefCounted() {}
  FOG_INLINE ~RefCounted() {}

  FOG_INLINE void addRef()
  {
    RefCountedBase::addRef();
  }

  FOG_INLINE void release()
  {
    if (RefCountedBase::release()) delete static_cast<T*>(this);
  }

private:
  FOG_DISABLE_COPY(RefCounted<T>)
};

/*! @brief Base class for Fog::RefClass<T> template. */
struct FOG_API RefClassBase : public Class
{
public:
  FOG_INLINE sysint_t refCount() const { return _refCount.get(); }

  FOG_INLINE void addRef() { _refCount.inc(); }

  // Returns true if the object should self-delete.
  FOG_INLINE bool release() { return _refCount.deref(); }

protected:
  Atomic<sysuint_t> _refCount;
  Flags<uint32_t> _flags;

  // [Fog::RefClassBase]
  FOG_INLINE RefClassBase() : 
    Class(reinterpret_cast<uint32_t*>(&_flags))
  {
    _refCount.init(0);
  }

  FOG_INLINE ~RefClassBase()
  {
  }

private:
  FOG_DISABLE_COPY(RefClassBase)
};

template <typename T>
struct RefClass : public RefClassBase
{
public:
  FOG_INLINE void addRef()
  {
    RefClassBase::addRef();
  }

  FOG_INLINE void release()
  {
    if (RefClassBase::release()) delete static_cast<T*>(this);
  }
};

// A smart pointer class for reference counted objects.  Use this class instead
// of calling AddRef and Release manually on a reference counted object to
// avoid common memory leaks caused by forgetting to Release an object
// reference.  Sample usage:
//
//   class MyFoo : public RefCounted<MyFoo> {
//    ...
//   };
//
//   void some_function() {
//     RefPtr<MyFoo> foo = new MyFoo();
//     foo->Method(param);
//     // |foo| is released when this function returns
//   }
//
//   void some_other_function() {
//     RefPtr<MyFoo> foo = new MyFoo();
//     ...
//     foo = NULL;  // explicitly releases |foo|
//     ...
//     if (foo)
//       foo->Method(param);
//   }
//
// The above examples show how RefPtr<T> acts like a pointer to T.
// Given two RefPtr<T> classes, it is also possible to exchange
// references between the two objects, like so:
//
//   {
//     RefPtr<MyFoo> a = new MyFoo();
//     RefPtr<MyFoo> b;
//
//     b.swap(a);
//     // now, |b| references the MyFoo object, and |a| references NULL.
//   }
//
// To make both |a| and |b| in the above example reference the same MyFoo
// object, simply use the assignment operator:
//
//   {
//     RefPtr<MyFoo> a = new MyFoo();
//     RefPtr<MyFoo> b;
//
//     b = a;
//     // now, |a| and |b| each own a reference to the same MyFoo object.
//   }
//
template <typename T>
struct RefPtr
{
public:
  FOG_INLINE RefPtr() : _ptr(NULL)
  {
  }

  FOG_INLINE RefPtr(T* p) : _ptr(p)
  {
    if (_ptr) _ptr->addRef();
  }

  FOG_INLINE RefPtr(const RefPtr<T>& r) : _ptr(r._ptr)
  {
    if (_ptr) _ptr->addRef();
  }

  FOG_INLINE ~RefPtr()
  {
    if (_ptr) _ptr->release();
  }

  FOG_INLINE T* get() const { return _ptr; }
  FOG_INLINE operator T*() const { return _ptr; }
  FOG_INLINE T* operator->() const { return _ptr; }

  RefPtr<T>& operator=(T* p)
  {
    // addRef first so that self assignment should work
    if (p) p->addRef();
    if (_ptr) _ptr ->release();
    _ptr = p;
    return *this;
  }

  FOG_INLINE RefPtr<T>& operator=(const RefPtr<T>& r)
  {
    return *this = r._ptr;
  }

  FOG_INLINE T* take()
  {
    T* p = _ptr;
    _ptr = NULL;
    return p;
  }

  FOG_INLINE void swap(T** pp)
  {
    T* p = _ptr;
    _ptr = *pp;
    *pp = p;
  }

  FOG_INLINE void swap(RefPtr<T>& r)
  {
    swap(&r._ptr);
  }

 private:
  T* _ptr;
};

} // Fog namespace

#endif // _FOG_CORE_REFCOUNTED_H
