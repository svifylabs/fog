// [Core library - T++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// [Guard]
#ifndef _FOG_CORE_SCOPEDPTR_H
#define _FOG_CORE_SCOPEDPTR_H

// [Dependencies]
#include <Fog/Build/Build.h>

#include <stdlib.h>

namespace Fog {

// [Fog::ScopedPtr<T>]

// Scopers help you manage ownership of a pointer, helping you easily manage the
// a pointer within a scope, and automatically destroying the pointer at the
// end of a scope.  There are two main classes you will use, which coorespond
// to the operators new/delete and new[]/delete[].
//
// Example usage (ScopedPtr):
//   {
//     ScopedPtr<Foo> foo(new Foo("wee"));
//   }  // foo goes out of scope, releasing the pointer with it.
//
//   {
//     ScopedPtr<Foo> foo;          // No pointer managed.
//     foo.reset(new Foo("wee"));    // Now a pointer is managed.
//     foo.reset(new Foo("wee2"));   // Foo("wee") was destroyed.
//     foo.reset(new Foo("wee3"));   // Foo("wee2") was destroyed.
//     foo->method();                // Foo::method() called.
//     foo.get()->method();          // Foo::method() called.
//     SomeFunc(foo.release());      // SomeFunc takes owernship, foo no longer
//                                   // manages a pointer.
//     foo.reset(new Foo("wee4"));   // foo manages a pointer again.
//     foo.reset();                  // Foo("wee4") destroyed, foo no longer
//                                   // manages a pointer.
//   }  // foo wasn't managing a pointer, so nothing was destroyed.
//
// Example usage (ScopedArray):
//   {
//     ScopedArray<Foo> foo(new Foo[100]);
//     foo.get()->method();  // Foo::method on the 0th element.
//     foo[10].method();     // Foo::method on the 10th element.
//   }

// This is an implementation designed to match the anticipated future TR2
// implementation of the ScopedPtr class, and its closely-related brethren,
// ScopedArray.

// A ScopedPtr<T> is like a T*, except that the destructor of ScopedPtr<T>
// automatically deletes the pointer it holds (if any).
// That is, ScopedPtr<T> owns the T object that it points to.
// Like a T*, a ScopedPtr<T> may hold either NULL or a pointer to a T object.
// Also like T*, ScopedPtr<T> is thread-compatible, and once you
// dereference it, you get the threadsafety guarantees of T.
//
// The size of a ScopedPtr is small:
// sizeof(ScopedPtr<T>) == sizeof(T*)
template <typename T>
struct ScopedPtr
{
public:
  // Constructor.  Defaults to intializing with NULL.
  // There is no way to create an uninitialized ScopedPtr.
  // The input parameter must be allocated with new.
  FOG_INLINE explicit ScopedPtr(T* p = NULL) : _ptr(p) {}

  // Destructor.  If there is a T object, delete it.
  // We don't need to test _ptr == NULL because T++ does that for us.
  FOG_INLINE ~ScopedPtr()
  {
    enum { type_must_be_complete = sizeof(T) };
    delete _ptr;
  }

  // Reset.  Deletes the current owned object, if any.
  // Then takes ownership of a new object, if given.
  // this->reset(this->get()) works.
  FOG_INLINE void reset(T* p = NULL)
  {
    if (p != _ptr)
    {
      enum { type_must_be_complete = sizeof(T) };
      delete _ptr;
      _ptr = p;
    }
  }

  // Accessors to get the owned object.
  // operator* and operator-> will assert if there is no current object.
  FOG_INLINE T& operator*() const
  {
    FOG_ASSERT(_ptr != NULL);
    return *_ptr;
  }
  FOG_INLINE T* operator->() const
  {
    FOG_ASSERT(_ptr != NULL);
    return _ptr;
  }
  FOG_INLINE T* get() const { return _ptr; }

  // Comparison operators.
  // These return whether two ScopedPtr refer to the same object, not just to
  // two different but equal objects.
  FOG_INLINE bool operator==(T* p) const { return _ptr == p; }
  FOG_INLINE bool operator!=(T* p) const { return _ptr != p; }

  // Swap two scoped pointers.
  FOG_INLINE void swap(ScopedPtr& p2)
  {
    T* tmp = _ptr;
    _ptr = p2._ptr;
    p2._ptr = tmp;
  }

  // Release a pointer.
  // The return value is the current pointer held by this object.
  // If this object holds a NULL pointer, the return value is NULL.
  // After this operation, this object will hold a NULL pointer,
  // and will not own the object any more.
  FOG_INLINE T* release()
  {
    T* ret = _ptr;
    _ptr = NULL;
    return ret;
  }

private:
  T* _ptr;

  // Forbid comparison of ScopedPtr types.  If C2 != T, it totally doesn't
  // make sense, and if C2 == T, it still doesn't make sense because you should
  // never have the same object owned by two different ref ptrs.
  template <class C2> bool operator==(ScopedPtr<C2> const& p2) const;
  template <class C2> bool operator!=(ScopedPtr<C2> const& p2) const;

  FOG_DISABLE_COPY(ScopedPtr<T>)
};

} // Fog namespace

// Free functions
template <typename T>
FOG_INLINE void swap(Fog::ScopedPtr<T>& p1, Fog::ScopedPtr<T>& p2)
{
  p1.swap(p2);
}

template <typename T>
FOG_INLINE bool operator==(T* p1, const Fog::ScopedPtr<T>& p2)
{
  return p1 == p2.get();
}

template <typename T>
FOG_INLINE bool operator!=(T* p1, const Fog::ScopedPtr<T>& p2)
{
  return p1 != p2.get();
}

namespace Fog {

// [Fog::ScopedArray<T>]

// ScopedArray<T> is like ScopedPtr<T>, except that the caller must allocate
// with new [] and the destructor deletes objects with delete [].
//
// As with ScopedPtr<T>, a ScopedArray<T> either points to an object
// or is NULL.  A ScopedArray<T> owns the object that it points to.
// ScopedArray<T> is thread-compatible, and once you index into it,
// the returned objects have only the threadsafety guarantees of T.
//
// Size: sizeof(ScopedArray<T>) == sizeof(T*)
template <typename T>
struct ScopedArray
{
public:
  // Constructor.  Defaults to intializing with NULL.
  // There is no way to create an uninitialized ScopedArray.
  // The input parameter must be allocated with new [].
  FOG_INLINE explicit ScopedArray(T* p = NULL) : _array(p) { }

  // Destructor.  If there is a T object, delete it.
  // We don't need to test _ptr == NULL because T++ does that for us.
  FOG_INLINE ~ScopedArray()
  {
    enum { type_must_be_complete = sizeof(T) };
    delete[] _array;
  }

  // Reset.  Deletes the current owned object, if any.
  // Then takes ownership of a new object, if given.
  // this->reset(this->get()) works.
  FOG_INLINE void reset(T* p = NULL)
  {
    if (p != _array)
    {
      enum { type_must_be_complete = sizeof(T) };
      delete[] _array;
      _array = p;
    }
  }

  // Get one element of the current object.
  // Will assert if there is no current object, or index i is negative.
  FOG_INLINE T& operator[](sysuint_t i) const
  {
    FOG_ASSERT(_array != NULL);
    return _array[i];
  }

  // Get a pointer to the zeroth element of the current object.
  // If there is no current object, return NULL.
  FOG_INLINE T* get() const { return _array; }

  // Comparison operators.
  // These return whether two ScopedArray refer to the same object, not just to
  // two different but equal objects.
  FOG_INLINE bool operator==(T* p) const { return _array == p; }
  FOG_INLINE bool operator!=(T* p) const { return _array != p; }

  // Swap two scoped arrays.
  FOG_INLINE void swap(ScopedArray& p2)
  {
    T* tmp = _array;
    _array = p2._array;
    p2._array = tmp;
  }

  // Release an array.
  // The return value is the current pointer held by this object.
  // If this object holds a NULL pointer, the return value is NULL.
  // After this operation, this object will hold a NULL pointer,
  // and will not own the object any more.
  FOG_INLINE T* release()
  {
    T* retVal = _array;
    _array = NULL;
    return retVal;
  }

private:
  T* _array;

  // Forbid comparison of different ScopedArray types.
  template <class C2> bool operator==(ScopedArray<C2> const& p2) const;
  template <class C2> bool operator!=(ScopedArray<C2> const& p2) const;

  // Disallow evil constructors
  ScopedArray(const ScopedArray&);
  void operator=(const ScopedArray&);
};

} // Fog namespace

// Free functions
template <typename T>
FOG_INLINE void swap(Fog::ScopedArray<T>& p1, Fog::ScopedArray<T>& p2)
{
  p1.swap(p2);
}

template <typename T>
FOG_INLINE bool operator==(T* p1, const Fog::ScopedArray<T>& p2)
{
  return p1 == p2.get();
}

template <typename T>
FOG_INLINE bool operator!=(T* p1, const Fog::ScopedArray<T>& p2)
{
  return p1 != p2.get();
}

// [Guard]
#endif // _FOG_CORE_SCOPEDPTR_H
