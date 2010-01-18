// [Fog/Graphics Library - Public API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif

#include <Fog/Build/Build.h>

// Don't build MMX under 64-bit mode. MSVC complains about MMX in 64-bit mode
// and it's always overriden by SSE2 implementation that is guaranted for every
// 64-bit processor.
#if defined(FOG_ARCH_X86)

// [Dependencies]
#include <Fog/Core/Intrin_MMX.h>
#include <Fog/Core/Math.h>
#include <Fog/Core/Misc.h>
#include <Fog/Graphics/Argb.h>
#include <Fog/Graphics/ArgbUtil.h>
#include <Fog/Graphics/ByteUtil_p.h>
#include <Fog/Graphics/ColorLut.h>
#include <Fog/Graphics/ColorMatrix.h>
#include <Fog/Graphics/Constants_p.h>
#include <Fog/Graphics/DitherMatrix.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/ImageFilter.h>
#include <Fog/Graphics/Matrix.h>
#include <Fog/Graphics/Path.h>
#include <Fog/Graphics/Pattern.h>
#include <Fog/Graphics/RasterUtil.h>
#include <Fog/Graphics/ArgbAnalyzer.h>

// [Raster_MMX]
#include <Fog/Graphics/RasterUtil/RasterUtil_C_p.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Defs_MMX_p.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Composite_MMX_p.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Dib_MMX_p.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Interpolate_MMX_p.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Pattern_MMX_p.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Scale_MMX_p.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Filters_MMX_p.h>

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE void fog_raster_init_mmx(void)
{
  using namespace Fog;
  using namespace Fog::RasterUtil;

  FunctionMap* m = functionMap;

  m->interpolate.gradient[PIXEL_FORMAT_ARGB32] = InterpolateMMX::gradient_argb32;
}

#endif // FOG_ARCH_X86
