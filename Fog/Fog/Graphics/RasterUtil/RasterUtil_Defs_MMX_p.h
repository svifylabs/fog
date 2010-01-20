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
// [Fog::RasterUtil::MMX - Defines]
// ============================================================================

// TODO: Duplicated, move to different location which will be shared.
#define READ_8(ptr)  (((const uint8_t *)(ptr))[0])
#define READ_16(ptr) (((const uint16_t*)(ptr))[0])
#define READ_32(ptr) (((const uint32_t*)(ptr))[0])

// ============================================================================
// [Fog::RasterUtil::MMX - Constants]
// ============================================================================

// RGB16 masks
static __m64 Mask_F800F800F800F800;
static __m64 Mask_E000E000E000E000;
static __m64 Mask_7C007C007C007C00;
static __m64 Mask_1F001F001F001F00;
static __m64 Mask_07E007E007E007E0;
static __m64 Mask_03E003E003E003E0;
static __m64 Mask_00F800F800F800F8;
static __m64 Mask_007C007C007C007C;
static __m64 Mask_001F001F001F001F;
static __m64 Mask_0007000700070007;
static __m64 Mask_0003000300030003;

// ============================================================================
// [Fog::RasterUtil::MMX - Helpers]
// ============================================================================

static FOG_INLINE __m128i mmx_create_mask_4x2W(uint16_t m0, uint16_t m1, uint16_t m2, uint16_t m3)
{
  mmx_t t;

  t.uw[3] = m0;
  t.uw[2] = m1;
  t.uw[1] = m2;
  t.uw[0] = m3;

  return t.m64;
}

template<typename T>
static FOG_INLINE void mmx_load8(__m64& dst, const T* src)
{
  return *reinterpret_cast<const __m64*>(src);
}

template<typename T>
static FOG_INLINE void mmx_store8(T* dst, __m64 src)
{
  reinterpret_cast<__m64*>(dst)[0] = src;
}

} // RasterUtil namespace
} // Fog namespace
