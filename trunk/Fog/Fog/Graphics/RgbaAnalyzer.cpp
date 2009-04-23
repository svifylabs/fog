// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/RgbaAnalyzer.h>

namespace Fog {

// [Fog::RgbaAnalyzer]

int RgbaAnalyzer::analyzeAlpha(const uint8_t* data, sysuint_t count, sysuint_t alphapos, sysuint_t _inc)
{
  register sysuint_t i = count;
  register sysuint_t inc = _inc;
  register uint8_t mask;

  if (!i) return Fog::TransparentAlpha;

  data += alphapos;
  mask = data[0];

  while (--i)
  {
    data += inc;
    if (data[0] != mask) return Fog::VariableAlpha;
  }

  return (int)mask;
}

} // Fog namespace
