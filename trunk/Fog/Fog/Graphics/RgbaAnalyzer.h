// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_RGBAANALYZER_H
#define _FOG_GRAPHICS_RGBAANALYZER_H

// [Dependencies]
#include <Fog/Core/Sequence.h>
#include <Fog/Graphics/Gradient.h>
#include <Fog/Graphics/Rgba.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Fog::RgbaAnalyzer]
// ============================================================================

struct FOG_API RgbaAnalyzer
{
  static int analyzeAlpha(const uint8_t* data, sysuint_t count, sysuint_t alphapos, sysuint_t _inc);

  static FOG_INLINE int analyzeAlpha(
    const uint32_t* data,
    sysuint_t count,
    sysuint_t alphapos = Rgba::AlphaBytePos,
    sysuint_t inc = 4)
  {
    return analyzeAlpha(
      (const uint8_t*)data,
      count,
      alphapos,
      inc);
  }

  static FOG_INLINE int analyzeAlpha(
    const Rgba* data,
    size_t count)
  {
    return analyzeAlpha(
      (const uint8_t*)data,
      count,
      Rgba::AlphaBytePos,
      sizeof(Rgba));
  }

  static FOG_INLINE int analyzeAlpha(
    const GradientStop* data,
    sysuint_t count)
  {
    return analyzeAlpha(
      (const uint8_t*)data,
      count, 
      FOG_OFFSET_OF(GradientStop, rgba) + Rgba::AlphaBytePos, 
      sizeof(GradientStop));
  }

  static FOG_INLINE int analyzeAlpha(const Sequence<GradientStop>& stops)
  { return analyzeAlpha(stops.cData(), stops.getLength()); }

  static FOG_INLINE int analyzeAlpha(const Vector<GradientStop>& stops)
  { return analyzeAlpha(stops.cData(), stops.getLength()); }
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_RGBAANALYZER_H
