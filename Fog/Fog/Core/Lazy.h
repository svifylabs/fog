// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_LAZY_H
#define _FOG_CORE_LAZY_H

// [Dependencies]
#include <Fog/Core/Atomic.h>
#include <Fog/Build/Build.h>
#include <Fog/Core/Constants.h>

//! @addtogroup Fog_Core
//! @{

namespace Fog {

// ============================================================================
// [Fog::Lazy_Abstract]
// ============================================================================

struct FOG_API Lazy_Abstract
{
public:
  enum State
  {
    Null = 0x0,
    Creating = 0x1
  };

  Lazy_Abstract();
  virtual ~Lazy_Abstract();

protected:
  virtual void* _create() = 0;
  virtual void* _get();

  void* _ptr;

  FOG_DISABLE_COPY(Lazy_Abstract)
};

// ============================================================================
// [Fog::Lazy<T>]
// ============================================================================

template<typename T>
struct Lazy : public Lazy_Abstract
{
  Lazy()
  {
  }

  ~Lazy()
  {
    // Delete and clear pointer, because Lazy_Abstract checks for it
    // and can assert if it's not NULL
    if (_ptr) { delete reinterpret_cast<T*>(_ptr); _ptr = NULL; }
  }

  FOG_INLINE T* get()
  {
    return reinterpret_cast<T*>(Lazy_Abstract::_get());
  }

  FOG_INLINE T* operator->()
  {
    return get();
  }

protected:
  virtual void* _create()
  {
    return new T();
  }
};

} // Fog namespace

// @}

// [Guard]
#endif // _FOG_CORE_LAZY_H
