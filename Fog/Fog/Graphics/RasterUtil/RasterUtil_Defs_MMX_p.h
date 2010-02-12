// [Fog/Graphics Library - Private API]
//
// [Licence]
// MIT, See COPYING file in package

// For some IDEs to enable code-assist.
#include <Fog/Build/Build.h>

#if defined(FOG_IDE)
#include <Fog/Graphics/RasterUtil/RasterUtil_Defs_C_p.h>
#endif // FOG_IDE

namespace Fog {
namespace RasterUtil {

// ============================================================================
// [Fog::RasterUtil::MMX - Constants]
// ============================================================================

#define MMX_DECLARE_CONST_PI8_VAR(name, val0, val1, val2, val3, val4, val5, val6, val7) \
  FOG_ALIGNED_VAR(static const uint8_t, _sse2_const_##name[8], 8) = \
  { \
    (uint8_t)(val7), \
    (uint8_t)(val6), \
    (uint8_t)(val5), \
    (uint8_t)(val4), \
    (uint8_t)(val3), \
    (uint8_t)(val2), \
    (uint8_t)(val1), \
    (uint8_t)(val0)  \
  }

#define MMX_DECLARE_CONST_PI8_SET(name, val0) \
  FOG_ALIGNED_VAR(static const uint8_t, _mmx_const_##name[8], 8) = \
  { \
    (uint8_t)(val0), \
    (uint8_t)(val0), \
    (uint8_t)(val0), \
    (uint8_t)(val0), \
    (uint8_t)(val0), \
    (uint8_t)(val0), \
    (uint8_t)(val0), \
    (uint8_t)(val0)  \
  }

#define MMX_DECLARE_CONST_PI16_VAR(name, val0, val1, val2, val3) \
  FOG_ALIGNED_VAR(static const uint16_t, _mmx_const_##name[4], 8) = \
  { \
    (uint16_t)(val3), \
    (uint16_t)(val2), \
    (uint16_t)(val1), \
    (uint16_t)(val0)  \
  }

#define MMX_DECLARE_CONST_PI16_SET(name, val0) \
  FOG_ALIGNED_VAR(static const uint16_t, _mmx_const_##name[4], 8) = \
  { \
    (uint16_t)(val0), \
    (uint16_t)(val0), \
    (uint16_t)(val0), \
    (uint16_t)(val0)  \
  }

#define MMX_DECLARE_CONST_PI32_VAR(name, val0, val1) \
  FOG_ALIGNED_VAR(static const uint32_t, _mmx_const_##name[2], 8) = \
  { \
    (uint32_t)(val1), \
    (uint32_t)(val0)  \
  }

#define MMX_DECLARE_CONST_PI32_SET(name, val0) \
  FOG_ALIGNED_VAR(static const uint32_t, _mmx_const_##name[2], 8) = \
  { \
    (uint32_t)(val0), \
    (uint32_t)(val0)  \
  }

#define MMX_DECLARE_CONST_PI64(name, val0, val1) \
  FOG_ALIGNED_VAR(static const uint64_t, _mmx_const_##name[1], 8) = \
  { \
    (uint64_t)(val0)  \
  }

#define MMX_GET_CONST(name) (*(const __m64*)_mmx_const_##name)

// ============================================================================
// [Fog::RasterUtil::MMX - Constants]
// ============================================================================

// RGB16 masks.
MMX_DECLARE_CONST_PI16_VAR(0000F8000000F800, 0x0000, 0xF800, 0x0000, 0xF800);
MMX_DECLARE_CONST_PI16_VAR(0000E0000000E000, 0x0000, 0xE000, 0x0000, 0xE000);
MMX_DECLARE_CONST_PI16_VAR(00007C0000007C00, 0x0000, 0x7C00, 0x0000, 0x7C00);
MMX_DECLARE_CONST_PI16_VAR(00001F0000001F00, 0x0000, 0x1F00, 0x0000, 0x1F00);
MMX_DECLARE_CONST_PI16_VAR(000007E0000007E0, 0x0000, 0x07E0, 0x0000, 0x07E0);
MMX_DECLARE_CONST_PI16_VAR(000003E0000003E0, 0x0000, 0x03E0, 0x0000, 0x03E0);
MMX_DECLARE_CONST_PI16_VAR(000000F8000000F8, 0x0000, 0x00F8, 0x0000, 0x00F8);
MMX_DECLARE_CONST_PI16_VAR(0000007C0000007C, 0x0000, 0x007C, 0x0000, 0x007C);
MMX_DECLARE_CONST_PI16_VAR(0000001F0000001F, 0x0000, 0x001F, 0x0000, 0x001F);
MMX_DECLARE_CONST_PI16_VAR(0000000700000007, 0x0000, 0x0007, 0x0000, 0x0007);
MMX_DECLARE_CONST_PI16_VAR(0000000300000003, 0x0000, 0x0003, 0x0000, 0x0003);

// ============================================================================
// [Fog::RasterUtil::MMX - Helpers]
// ============================================================================

template<typename T>
static FOG_INLINE void mmx_load8(__m64& dst, const T* src)
{
  dst = reinterpret_cast<const __m64*>(src)[0];
}

template<typename T>
static FOG_INLINE void mmx_store8(T* dst, __m64 src)
{
  reinterpret_cast<__m64*>(dst)[0] = src;
}

} // RasterUtil namespace
} // Fog namespace
