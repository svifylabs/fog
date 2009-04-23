// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_FLAGS_H
#define _FOG_CORE_FLAGS_H

// [Dependencies]
#include <Fog/Build/Build.h>

//! @addtogroup Fog_Core
//! @{

namespace Fog {

// [Fog::Flags]

//! @brief Simple template to accesing bits in T type.
template<typename T>
struct Flags
{
  T flags;

  FOG_INLINE Flags& clear() { flags = 0; return *this; }

  FOG_INLINE Flags& init(T i) { flags = i; return *this; }
  FOG_INLINE Flags& set(T i) { flags |= i; return *this; }
  FOG_INLINE Flags& unset(T i) { flags &= ~i; return *this; }
  FOG_INLINE Flags& eor(T i) { flags ^= i; return *this; }

  FOG_INLINE Flags& operator=(T i) { return init(i); }
  FOG_INLINE Flags& operator|=(T i) { return set(i); }
  FOG_INLINE Flags& operator&=(T i) { return unset(i); }
  FOG_INLINE Flags& operator^=(T i) { return eor(i); }

  FOG_INLINE Flags& setBit(uint32_t bit) { flags |= ((T)1 << bit); return *this; }
  FOG_INLINE Flags& unsetBit(uint32_t bit) { flags &= ~((T)1 << bit); return *this; }
  FOG_INLINE bool testBit(uint32_t bit) const { return (flags & ((T)1 << bit)) != 0; }

  FOG_INLINE bool anyOf(T i) const { return (flags & i) != 0; }
  FOG_INLINE bool allOf(T i) const { return (flags & i) == i; }
  FOG_INLINE bool noneOf(T i) const { return (flags & i) == 0; }

  FOG_INLINE operator T() const { return flags; }
};

typedef Flags<uint8_t> Flags8;
typedef Flags<uint16_t> Flags16;
typedef Flags<uint32_t> Flags32;
typedef Flags<uint64_t> Flags64;

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_CORE_FLAGS_H
