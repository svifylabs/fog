// [Fog/Core Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_HASHUTIL_H
#define _FOG_CORE_HASHUTIL_H

// [Dependencies]
#include <Fog/Core/Assert.h>
#include <Fog/Core/Atomic.h>
#include <Fog/Build/Build.h>
#include <Fog/Core/Char.h>
#include <Fog/Core/Error.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/SequenceInfo.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/Std.h>
#include <Fog/Core/TypeInfo.h>
#include <Fog/Core/Vector.h>

namespace Fog {
namespace HashUtil {

//! @addtogroup Fog_Core
//! @{

// ============================================================================
// [Fog::HashUtil::hashData/hashString]
// ============================================================================

FOG_API uint32_t hashData(const void* data, sysuint_t size);

// STANDARD hash functions for 8 bit and 16 bit NULL terminated strings.
// 8 bit and 16 bit hashes are not compatible for strings that represents same
// unicode data.
// TODO: Invalid, hashes must be compatible between Latin1 and Utf16 strings.
FOG_API uint32_t hashString(const char* key, sysuint_t length);
FOG_API uint32_t hashString(const Char* key, sysuint_t length);

//! @brief Combine two hash values into one.
static FOG_INLINE uint32_t combineHash(uint32_t hash1, uint32_t hash2) { return hash1 + hash2; }

// ============================================================================
// [Fog::getHashCode]
// ============================================================================

// Default action is to call getHashCode() method
template<typename T>
FOG_INLINE uint32_t getHashCode(const T& key) { return key.getHashCode(); }

#define FOG_DECLARE_HASHABLE(__type__, __execute__) \
template<> \
FOG_INLINE uint32_t getHashCode<__type__>(const __type__& key) __execute__

#define FOG_DECLARE_HASHABLE_PTR(__type__, __execute__) \
template<> \
FOG_INLINE uint32_t getHashCode<__type__ &>(__type__& key) __execute__ \
\
template<> \
FOG_INLINE uint32_t getHashCode<const __type__ &>(const __type__& key) __execute__

FOG_DECLARE_HASHABLE(char, { return (uint8_t)key; })
FOG_DECLARE_HASHABLE(uint8_t, { return (uint8_t)key; })
FOG_DECLARE_HASHABLE(int16_t, { return (uint16_t)key; })
FOG_DECLARE_HASHABLE(uint16_t, { return (uint16_t)key; })
FOG_DECLARE_HASHABLE(int32_t, { return (uint32_t)key; })
FOG_DECLARE_HASHABLE(uint32_t, { return (uint32_t)key; })
FOG_DECLARE_HASHABLE(int64_t, { return ((uint32_t)((uint64_t)(key) >> 31)) ^ (uint32_t)(key); })
FOG_DECLARE_HASHABLE(uint64_t, { return ((uint32_t)((uint64_t)(key) >> 31)) ^ (uint32_t)(key); })
/*
FOG_DECLARE_HASHABLE_PTR(char*, { return hashString(key, DetectLength); })
FOG_DECLARE_HASHABLE_PTR(uint8_t*, { return hashString(reinterpret_cast<const char*>(key), DetectLength); })
FOG_DECLARE_HASHABLE_PTR(Char*, { return hashString(key, DetectLength); })
*/
#if FOG_ARCH_BITS == 32
template<typename T>
FOG_INLINE uint32_t getHashCode(T* key) { return (uint32_t)key; }
#else
template<typename T>
FOG_INLINE uint32_t getHashCode(T* key) { return ((uint32_t)((uint64_t)(key) >> 31)) ^ (uint32_t)(uint64_t)(key); }
#endif // FOG_ARCH_BITS

//! @}

} // HashUtil namespace
} // Fog namespace

// [Guard]
#endif // _FOG_CORE_HASHUTIL_H
