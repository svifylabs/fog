// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled Headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Graphics/PainterEngine.h>

namespace Fog {

// ============================================================================
// [Fog::LineParams]
// ============================================================================

LineParams::LineParams()
{
  lineWidth = 1.0;
  lineCap = LineCapButt;
  lineJoin = LineJoinMiter;
  miterLimit = 4.0;
  dashOffset = 0.0;
}

LineParams::LineParams(const LineParams& other) :
  lineWidth(other.lineWidth),
  lineCap(other.lineCap),
  lineJoin(other.lineJoin),
  miterLimit(other.miterLimit),
  dashes(other.dashes),
  dashOffset(other.dashOffset)
{
}

LineParams::~LineParams()
{
}

LineParams& LineParams::operator=(const LineParams& other)
{
  lineWidth = other.lineWidth;
  lineCap = other.lineCap;
  lineJoin = other.lineJoin;
  miterLimit = other.miterLimit;
  dashes = other.dashes;
  dashOffset = other.dashOffset;

  return *this;
}

// ============================================================================
// [Fog::PainterEngine]
// ============================================================================

PainterEngine::PainterEngine() {}
PainterEngine::~PainterEngine() {}

} // Fog namespace
