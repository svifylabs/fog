// [Fog-Core Library - Public API]
//
// [Licence]
// MIT, See COPYING file in package

#if !defined(_FOG_BUILD_BUILD_H)
# error "You can include this header only after including Fog/Build/Build.h or any other Fog header."
#endif // _FOG_BUILD_BUILD_H

// [GCC]
#if defined(FOG_CC_GNU)
# define FOG_PACKED __attribute__((packed))
// [MSVC and BORLAND]
#else
# define FOG_PACKED
# pragma pack(push)
# pragma pack(1)
#endif
