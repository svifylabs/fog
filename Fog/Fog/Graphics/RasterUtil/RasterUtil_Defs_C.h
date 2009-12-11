// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// For some IDEs to enable code-assist.
#include <Fog/Build/Build.h>

#if defined(FOG_IDE)
#include <Fog/Graphics/RasterUtil/RasterUtil_C.h>
#endif // FOG_IDE

namespace Fog {
namespace RasterUtil {

// ============================================================================
// [Fog::Raster - Defines]
// ============================================================================

// TODO: Duplicated, move to different location which will be shared.
#define READ_8(ptr)  (((const uint8_t *)(ptr))[0])
#define READ_16(ptr) (((const uint16_t*)(ptr))[0])
#define READ_32(ptr) (((const uint32_t*)(ptr))[0])

static FOG_INLINE int double_to_int(double d) { return (int)d; }
static FOG_INLINE int32_t double_to_fixed16x16(double d) { return (int32_t)(d * 65536.0); }
static FOG_INLINE int64_t double_to_fixed48x16(double d) { return (int64_t)(d * 65536.0); }

} // RasterUtil namespace
} // Fog namespace
