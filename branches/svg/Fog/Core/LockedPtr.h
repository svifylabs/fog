// [Core library - T++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// [Guard]
#ifndef _FOG_CORE_LOCKEDPTR_H
#define _FOG_CORE_LOCKEDPTR_H

// [Dependencies]
#include <Fog/Core/Atomic.h>
#include <Fog/Build/Build.h>

#include <stdlib.h>

namespace Fog {

// ============================================================================
// [Fog::LockedPtr]
// ============================================================================

//! @brief LockedPtr is like @c ScopedPtr and others, but it's designed to
//! working with locked data that must be unlocked after you don't need that.
//!
//! When this class is destructed, the data will be automatically unlocked by
//! calling release() method.
template<typename T>
struct LockedPtr
{
public:
  //! Constructor.  Defaults to intializing with NULL.
  //! There is no way to create an uninitialized LockedPtr.
  //! The input parameter must be allocated with new.
  FOG_INLINE explicit LockedPtr(T* p = NULL) : _ptr(p) {}

  //! Destructor.  If there is a T object, delete it.
  //! We don't need to test _ptr == NULL because T++ does that for us.
  FOG_INLINE ~LockedPtr()
  {
    if (_ptr) _ptr->release();
  }

  //! Reset.  Deletes the current owned object, if any.
  //! Then takes ownership of a new object, if given.
  //! this->reset(this->get()) works.
  FOG_INLINE void reset(T* p = NULL)
  {
    if (p != _ptr)
    {
      if (_ptr) _ptr->release();
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

  // These return whether two LockedPtr refer to the same object, not just to
  // two different but equal objects.

  FOG_INLINE bool operator==(T* p) const { return _ptr == p; }
  FOG_INLINE bool operator!=(T* p) const { return _ptr != p; }

  //! Swap two locked pointers.
  FOG_INLINE void swap(LockedPtr& p2)
  {
    T* tmp = _ptr;
    _ptr = p2._ptr;
    p2._ptr = tmp;
  }

  //! Release a pointer using release() method. This is like destructor
  FOG_INLINE void release()
  {
    if (_ptr)
    { 
      _ptr->release();
      _ptr = NULL;
    }
  }

private:
  T* _ptr;

  // Forbid comparison of LockedPtr types.  If C2 != T, it totally doesn't
  // make sense, and if C2 == T, it still doesn't make sense because you should
  // never have the same object owned by two different ref ptrs.
  template <class C2> bool operator==(LockedPtr<C2> const& p2) const;
  template <class C2> bool operator!=(LockedPtr<C2> const& p2) const;

  FOG_DISABLE_COPY(LockedPtr<T>)
};

} // Fog namespace

// [Guard]
#endif // _FOG_CORE_LOCKEDPTR_H
