// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/Error.h>
#include <Fog/Core/Math.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/Thread.h>
#include <Fog/Cpu/CpuInfo.h>
#include <Fog/Graphics/AffineMatrix.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Font.h>
#include <Fog/Graphics/Glyph.h>
#include <Fog/Graphics/GlyphSet.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/Painter.h>
#include <Fog/Graphics/Raster.h>
#include <Fog/Graphics/Raster/Raster_ByteOp.h>
#include <Fog/Graphics/Raster/Raster_PixelOp.h>
#include <Fog/Graphics/Rgba.h>

// [AntiGrain]
#include "agg_alpha_mask_u8.h"
#include "agg_basics.h"
#include "agg_path_storage.h"
#include "agg_conv_contour.h"
#include "agg_conv_curve.h"
#include "agg_conv_dash.h"
#include "agg_conv_stroke.h"
#include "agg_conv_smooth_poly1.h"
#include "agg_conv_transform.h"
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_rasterizer_outline.h"
#include "agg_rounded_rect.h"
#include "agg_scanline_p.h"
#include "agg_scanline_u.h"
#include "agg_scanline_bin.h"
#include "agg_trans_affine.h"
#include "agg_trans_viewport.h"
#include "agg_vcgen_dash.h"

// [BlitJit]
#include <BlitJit/BlitJit.h>

// ============================================================================
// [Fog::PainterDevice]
// ============================================================================

namespace Fog {

PainterDevice::PainterDevice() {}
PainterDevice::~PainterDevice() {}

}

// ============================================================================
// [Fog::NullPainterDevice]
// ============================================================================

#include "Painter_Null.cpp.h"

// ============================================================================
// [Fog::RasterPainterDevice]
// ============================================================================

#include "Painter_Raster.cpp.h"

// ============================================================================
// [Fog::Painter]
// ============================================================================

namespace Fog {

PainterDevice* Painter::sharedNull;

Painter::Painter()
{
  _d = sharedNull;
}

Painter::Painter(uint8_t* pixels, int width, int height, sysint_t stride, int format)
{
  _d = sharedNull;
  begin(pixels, width, height, stride, format);
}

Painter::Painter(Image& image)
{
  _d = sharedNull;
  begin(image);
}

Painter::~Painter()
{
  if (_d != sharedNull) delete _d;
}

err_t Painter::begin(uint8_t* pixels, int width, int height, sysint_t stride, int format)
{
  end();

  if (width <= 0 || height <= 0) return Error::InvalidArgument;

  PainterDevice* d = new(std::nothrow) RasterPainterDevice(
    pixels, width, height, stride, format);
  if (!d) return Error::OutOfMemory;

  _d = d;
  return Error::Ok;
}

err_t Painter::begin(Image& image)
{
  err_t err = image.detach();
  if (err) return err;

  return begin(image.mData(), image.width(), image.height(), image.stride(), image.format());
}

void Painter::end()
{
  if (_d != sharedNull) delete _d;
  _d = sharedNull;
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_painter_init(void)
{
  static Fog::NullPainterDevice sharedNullDevice;
  Fog::Painter::sharedNull = &sharedNullDevice;

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_painter_shutdown(void)
{
}
