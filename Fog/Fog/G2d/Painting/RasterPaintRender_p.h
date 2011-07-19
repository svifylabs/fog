// [Fog-G2d]
//
// [License]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_G2D_PAINTING_RASTERPAINTRENDER_P_H
#define _FOG_G2D_PAINTING_RASTERPAINTRENDER_P_H

// [Dependencies]
#include <Fog/Core/Global/Global.h>
#include <Fog/G2d/Painting/RasterPaintConstants_p.h>
#include <Fog/G2d/Painting/RasterPaintStructs_p.h>

namespace Fog {

//! @addtogroup Fog_G2d_Painting_Raster
//! @{

// ============================================================================
// [Fog::RasterRender - VTable]
// ============================================================================

struct FOG_NO_EXPORT RasterRenderVTable
{
  // --------------------------------------------------------------------------
  // [Types]
  // --------------------------------------------------------------------------

  typedef void (FOG_FASTCALL *FillNormalizedBoxI)(RasterContext& ctx, const BoxI& box);
  typedef void (FOG_FASTCALL *FillNormalizedBoxF)(RasterContext& ctx, const BoxF& box);
  typedef void (FOG_FASTCALL *FillNormalizedBoxD)(RasterContext& ctx, const BoxD& box);
  typedef void (FOG_FASTCALL *FillNormalizedPathF)(RasterContext& ctx, const PathF& path, uint32_t fillRule);
  typedef void (FOG_FASTCALL *FillNormalizedPathD)(RasterContext& ctx, const PathD& path, uint32_t fillRule);
  typedef void (FOG_FASTCALL *FillRasterizedShape)(RasterContext& ctx, void* _rasterizer);

  typedef void (FOG_FASTCALL *BlitNormalizedImageI)(RasterContext& ctx, const PointI& pt, const Image& srcImage, const RectI& srcFragment);

  // --------------------------------------------------------------------------
  // [Funcs]
  // --------------------------------------------------------------------------

  FillNormalizedBoxI fillNormalizedBoxI;
  FillNormalizedBoxF fillNormalizedBoxF;
  FillNormalizedBoxD fillNormalizedBoxD;
  FillNormalizedPathF fillNormalizedPathF;
  FillNormalizedPathD fillNormalizedPathD;
  FillRasterizedShape fillRasterizedShape;

  BlitNormalizedImageI blitNormalizedImageI;
};

//! @internal
//!
//! @brief @c RasterPaintEngine VTable instances.
//!
//! For performance reasons there are vtable for these combinations:
//!
//!   1. Threading model - ST(Singlethreaded) and MT(Multithreaded).
//!   2. Precision - BYTE and WORD.
//!   3. Clipping - BOX, REGION and MASK.
extern FOG_NO_EXPORT RasterRenderVTable _RasterRender_vtable[RASTER_MODE_COUNT][IMAGE_PRECISION_COUNT][RASTER_CLIP_COUNT];

// ============================================================================
// [Fog::RasterRender - Initializer]
// ============================================================================

FOG_NO_EXPORT void Painter_initRasterRender(void);

//! @}

} // Fog namespace

// [Guard]
#endif // _FOG_G2D_PAINTING_RASTERPAINTRENDER_P_H