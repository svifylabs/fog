// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/CpuInfo.h>
#include <Fog/Graphics/Raster.h>

namespace Fog {
namespace Raster {

// ============================================================================
// [Fog::Raster - FunctionMap]
// ============================================================================

static FunctionMap functionMapData;

FunctionMap* functionMap;

// ============================================================================
// [Fog::Raster - getRasterOps]
// ============================================================================

FunctionMap::RasterFuncs* getRasterOps(int format, int op)
{
  FOG_ASSERT(format < Image::FormatCount);
  FOG_ASSERT(op < CompositeCount);

  return &functionMap->raster[op][format];
}

} // Raster namespace
} // Fog namespace

// ============================================================================
// [Adaptor Initializers]
// ============================================================================

static void fog_raster_init_adaptors()
{
  using namespace Fog;
  using namespace Fog::Raster;

  FunctionMap* m = functionMap;
  int i, dstf, srcf;

  for (i = 0; i < CompositeCount; i++)
  {
    for (dstf = 1; dstf < Image::FormatCount; dstf++)
    {
      // Currently FormatI8 is last valid format, but this can be changed.
      if (dstf == Image::FormatI8) continue;

      FunctionMap::RasterFuncs* r = &m->raster[i][dstf];

      // Set closure to current compositor pointer.
      r->closure = (void*)m->raster[i];

      // Set wrappers to all non assigned functions.

      // Solid pixel adaptors.
      if (r->pixel == NULL)
      {
        r->pixel = m->adaptor[dstf].pixel;
        FOG_ASSERT(r->pixel);
      }

      if (r->pixel_a8 == NULL)
      {
        r->pixel_a8 = m->adaptor[dstf].pixel_a8;
        FOG_ASSERT(r->pixel_a8);
      }

      // Solid span adaptors.
      if (r->span_solid == NULL)
      {
        r->span_solid = m->adaptor[dstf].span_solid;
        FOG_ASSERT(r->span_solid);
      }
      
      if (r->span_solid_a8 == NULL)
      {
        r->span_solid_a8 = m->adaptor[dstf].span_solid_a8;
        FOG_ASSERT(r->span_solid_a8);
      }
      
      if (r->span_solid_a8_const == NULL)
      {
        r->span_solid_a8_const = m->adaptor[dstf].span_solid_a8_const;
        FOG_ASSERT(r->span_solid_a8_const);
      }

      // Composite adaptors
      for (srcf = 1; srcf < Image::FormatCount; srcf++)
      {
        if (r->span_composite[srcf] == NULL)
        {
          r->span_composite[srcf] = m->adaptor[dstf].span_composite[srcf];
          FOG_ASSERT(r->span_composite[srcf]);
        }
      }

      for (srcf = 1; srcf < Image::FormatCount; srcf++)
      {
        if (r->span_composite_a8[srcf] == NULL)
        {
          r->span_composite_a8[srcf] = m->adaptor[dstf].span_composite_a8[srcf];
          FOG_ASSERT(r->span_composite_a8[srcf]);
        }
      }

      for (srcf = 1; srcf < Image::FormatCount; srcf++)
      {
        if (r->span_composite_a8_const[srcf] == NULL)
        {
          r->span_composite_a8_const[srcf] = m->adaptor[dstf].span_composite_a8_const[srcf];
          FOG_ASSERT(r->span_composite_a8_const[srcf]);
        }
      }
    }
  }
}

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_EXTERN void fog_raster_init_c(void);
FOG_INIT_EXTERN void fog_raster_init_mmx(void);
FOG_INIT_EXTERN void fog_raster_init_sse2(void);

FOG_INIT_DECLARE err_t fog_raster_init(void)
{
  using namespace Fog;
  using namespace Fog::Raster;

  FunctionMap* m = functionMap = &functionMapData;

  // [Install C optimized code (default)]
  {
    fog_raster_init_c();
  }

  // [Install MMX optimized code if supported]
  if (cpuInfo->hasFeature(CpuInfo::Feature_MMX))
  {
    fog_raster_init_mmx();
  }

  // [Install SSE2 optimized code if supported]
  if (cpuInfo->hasFeature(CpuInfo::Feature_SSE2))
  {
    fog_raster_init_sse2();
  }

  // [Install Adaptors (slow paths)]
  {
    fog_raster_init_adaptors();
  }

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_raster_shutdown(void)
{
}
