// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_STRINGCACHE_H
#define _FOG_CORE_STRINGCACHE_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Core/String.h>

//! @addtogroup Fog_Core
//! @{

namespace Fog {

// ============================================================================
// [Fog::StringCache]
// ============================================================================

//! @brief String cache that's used to pre-cache strings.
struct FOG_API StringCache
{
private:
  //! @brief Private constructur used from @c StringCache::create().
  StringCache(
    const char* strings,
    sysuint_t length,
    sysuint_t count);

  //! @brief Private destructur used from @c StringCache::destroy().
  ~StringCache();

public:
  //! @brief Returns cache name.
  //!
  //! @sa StringCache::getCacheByName()
  FOG_INLINE const String32& name() { return _name; }

  //! @brief Returns count of strings in cache.
  FOG_INLINE sysuint_t count() const { return _count; }

  //! @brief Returns all strings.
  FOG_INLINE const String32* data() const { return (String32 *)_data; }

  //! @brief Returns string that has id @c i from cache.
  FOG_INLINE const String32& get(sysuint_t i) const
  {
    FOG_ASSERT(i < _count);
    return ((String32 *)_data)[i];
  }

  //! @brief Official constructor to create @c Fog::StringCache.
  //! @param strings Array of strings to create. Each string ends with zero terminator.
  //! @param length Total length of @a strings with all zero terminators.
  //! @param count Count of zero terminated strings in @a strings.
  //! @param name Optional name of this collection for loadable libraries.
 static StringCache* create(
    const char* strings, 
    sysuint_t length,
    sysuint_t count,
    const String32& name = String32());

  //! @brief Official destructor for @c Fog::StringCache.
  static void destroy(StringCache* cache);

  static StringCache* getCacheByName(const String32& name);

private:
  //! @brief Cache name.
  String32 _name;

  //! @brief Count of strings in cache.
  sysuint_t _count;
  //! @brief Continuous cache memory.
  uint8_t _data[sizeof(void*)];

  FOG_DISABLE_COPY(StringCache)
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_CORE_STRINGCACHE_H
