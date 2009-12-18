// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_ARGBANALYZER_H
#define _FOG_GRAPHICS_ARGBANALYZER_H

// [Dependencies]
#include <Fog/Core/List.h>
#include <Fog/Graphics/Argb.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Fog::ArgbAnalyzer]
// ============================================================================

struct FOG_API ArgbAnalyzer
{
  static int analyzeAlpha(const uint8_t* data, sysuint_t count, sysuint_t alphapos, sysuint_t _inc);

  static FOG_INLINE int analyzeAlpha(
    const uint32_t* data,
    sysuint_t count,
    sysuint_t alphapos = ARGB32_ABYTE,
    sysuint_t inc = 4)
  {
    return analyzeAlpha((const uint8_t*)data, count, alphapos, inc);
  }

  static FOG_INLINE int analyzeAlpha(
    const Argb* data,
    size_t count)
  {
    return analyzeAlpha((const uint8_t*)data, count, ARGB32_ABYTE, sizeof(Argb));
  }

  static FOG_INLINE int analyzeAlpha(
    const ArgbStop* data,
    sysuint_t count)
  {
    return analyzeAlpha((const uint8_t*)data, count,
      FOG_OFFSET_OF(ArgbStop, rgba) + ARGB32_ABYTE,
      sizeof(ArgbStop));
  }

  static FOG_INLINE int analyzeAlpha(const List<ArgbStop>& stops)
  { return analyzeAlpha(stops.getData(), stops.getLength()); }
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_ARGBANALYZER_H
