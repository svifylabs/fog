// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Class.h>
#include <Fog/Core/ThreadLocalStorage.h>

// [Fog::Class]

static Fog::ThreadLocalPointer<void> tls_ptr;

FOG_CAPI_DECLARE void* Core_Class_create(sysuint_t size)
{
  void* addr = Fog::Memory::xalloc(size);
  if (addr) tls_ptr.set(addr);
  return addr;
}

FOG_CAPI_DECLARE void Core_Class_setup32(void* self, uint32_t* flags)
{
  if (self == tls_ptr.get())
  {
    tls_ptr.set(NULL);
    *flags = Fog::Class::IsDynamic;
  }
  else
    *flags = 0;
}

FOG_CAPI_DECLARE void Core_Class_setup64(void* self, uint64_t* flags)
{
  if (self == tls_ptr.get())
  {
    tls_ptr.set(NULL);
    *flags = (uint64_t)Fog::Class::IsDynamic;
  }
  else
    *flags = FOG_UINT64_C(0);
}
