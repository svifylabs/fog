// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Dependencies]
#include <Fog/Core/Error.h>
#include <Fog/Core/Lock.h>

namespace Fog {

#if defined(FOG_OS_POSIX)
FOG_CVAR_DECLARE pthread_mutexattr lock_recursive_attrs;
#endif // FOG_OS_POSIX

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_lock_init(void)
{
#if defined(FOG_OS_POSIX)
  pthread_mutexattr_init(&lock_recursive_attrs);
  pthread_mutexattr_settype(&lock_recursive_attrs, PTHREAD_MUTEX_RECURSIVE);
#endif // FOG_OS_POSIX

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_lock_shutdown(void)
{
#if defined(FOG_OS_POSIX)
  pthread_mutexattr_destroy(&lock_recursive_attrs);
#endif // FOG_OS_POSIX
}
