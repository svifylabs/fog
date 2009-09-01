// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled Headers]
#ifdef FOG_PRECOMP
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
#include <Fog/Graphics/ColorLut.h>
#include <Fog/Graphics/ColorMatrix.h>
#include <Fog/Graphics/DitherMatrix.h>
#include <Fog/Graphics/Error.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/ImageFilter.h>
#include <Fog/Graphics/Matrix.h>
#include <Fog/Graphics/Path.h>
#include <Fog/Graphics/Path_p.h>
#include <Fog/Graphics/Pattern.h>
#include <Fog/Graphics/Raster.h>
#include <Fog/Graphics/Raster/Raster_C.h>
#include <Fog/Graphics/RgbaAnalyzer.h>

// [Raster_MMX]
#include <Fog/Graphics/Raster/Raster_MMX_base.cpp.h>
#include <Fog/Graphics/Raster/Raster_MMX_adaptor.cpp.h>
#include <Fog/Graphics/Raster/Raster_MMX_convert.cpp.h>
#include <Fog/Graphics/Raster/Raster_MMX_composite.cpp.h>
#include <Fog/Graphics/Raster/Raster_MMX_gradient.cpp.h>
#include <Fog/Graphics/Raster/Raster_MMX_pattern.cpp.h>
#include <Fog/Graphics/Raster/Raster_MMX_filters.cpp.h>
#include <Fog/Graphics/Raster/Raster_MMX_vector.cpp.h>

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE void fog_raster_init_mmx(void)
{
  using namespace Fog;
  using namespace Fog::Raster;

  FunctionMap* m = functionMap;

  m->gradient.gradient_argb32 = gradient_gradient_argb32_MMX;
}

#endif // FOG_ARCH_X86
