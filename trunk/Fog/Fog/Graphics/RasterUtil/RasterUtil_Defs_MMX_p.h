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

// ============================================================================
// [Fog::RasterUtil::MMX - Constants]
// ============================================================================

// RGB16 masks
static __m64 SSE2_GET_CONST_PI(0000F8000000F800);
static __m64 SSE2_GET_CONST_PI(0000E0000000E000);
static __m64 SSE2_GET_CONST_PI(00007C0000007C00);
static __m64 SSE2_GET_CONST_PI(00001F0000001F00);
static __m64 SSE2_GET_CONST_PI(000007E0000007E0);
static __m64 SSE2_GET_CONST_PI(000003E0000003E0);
static __m64 SSE2_GET_CONST_PI(000000F8000000F8);
static __m64 SSE2_GET_CONST_PI(0000007C0000007C);
static __m64 SSE2_GET_CONST_PI(0000001F0000001F);
static __m64 SSE2_GET_CONST_PI(0000000700000007);
static __m64 SSE2_GET_CONST_PI(0000000300000003);

// ============================================================================
// [Fog::RasterUtil::MMX - Helpers]
// ============================================================================

static FOG_INLINE __m64 mmx_create_mask_4x2W(uint16_t m0, uint16_t m1, uint16_t m2, uint16_t m3)
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
  dst = reinterpret_cast<const __m64*>(src)[0];
}

template<typename T>
static FOG_INLINE void mmx_store8(T* dst, __m64 src)
{
  reinterpret_cast<__m64*>(dst)[0] = src;
}

} // RasterUtil namespace
} // Fog namespace
