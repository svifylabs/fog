// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [GCC]
#if defined(FOG_CC_GNU)
# define FOG_PACKED __attribute__((packed))
// [MSVC and BORLAND]
#else
# define FOG_PACKED
# pragma pack(push)
# pragma pack(1)
#endif
