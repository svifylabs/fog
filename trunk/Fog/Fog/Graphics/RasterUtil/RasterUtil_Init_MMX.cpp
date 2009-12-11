// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif

#include <Fog/Build/Build.h>

// Don't build MMX under 64-bit mode. MSVC complains about MMX in 64-bit mode
// and it's always overriden by SSE2 implementation that is guaranted for any
// 64-bit processor.
#if defined(FOG_ARCH_X86)

// [Dependencies]
#include <Fog/Core/Intrin_MMX.h>
#include <Fog/Core/Math.h>
#include <Fog/Core/Misc.h>
#include <Fog/Graphics/Argb.h>
#include <Fog/Graphics/ArgbUtil.h>
#include <Fog/Graphics/ColorLut.h>
#include <Fog/Graphics/ColorMatrix.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/DitherMatrix.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/ImageFilter.h>
#include <Fog/Graphics/Matrix.h>
#include <Fog/Graphics/Path.h>
#include <Fog/Graphics/Pattern.h>
#include <Fog/Graphics/RasterUtil.h>
#include <Fog/Graphics/ArgbAnalyzer.h>

// [Raster_MMX]
#include <Fog/Graphics/RasterUtil/RasterUtil_C.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Defs_MMX.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Convert_MMX.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Composite_MMX.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Gradient_MMX.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Pattern_MMX.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Scale_MMX.h>
#include <Fog/Graphics/RasterUtil/RasterUtil_Filters_MMX.h>

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE void fog_raster_init_mmx(void)
{
  using namespace Fog;
  using namespace Fog::RasterUtil;

  FunctionMap* m = functionMap;

  m->gradient.gradient_argb32 = GradientMMX::gradient_argb32;
}

#endif // FOG_ARCH_X86
