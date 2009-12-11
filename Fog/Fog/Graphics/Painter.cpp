// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Painter.h>
#include <Fog/Graphics/PainterEngine.h>

#include <Fog/Graphics/PainterEngine/Null.h>
#include <Fog/Graphics/PainterEngine/Raster.h>

namespace Fog {

// ============================================================================
// [Fog::Painter]
// ============================================================================

PainterEngine* Painter::sharedNull;

Painter::Painter()
{
  _engine = sharedNull;
}

Painter::Painter(Image& image, int hints)
{
  _engine = sharedNull;
  begin(image, hints);
}

Painter::~Painter()
{
  if (_engine != sharedNull) delete _engine;
}

err_t Painter::begin(uint8_t* pixels, int width, int height, sysint_t stride, int format, int hints)
{
  end();

  // Check for invalid arguments.
  if (pixels == NULL) return ERR_RT_INVALID_ARGUMENT;

  // Never initialize painter for zero image.
  if (width <= 0 || height <= 0)
  {
    if (width == 0 && height == 0)
      return ERR_IMAGE_ZERO_SIZE;
    else
      return ERR_IMAGE_INVALID_SIZE;
  }

  // Check for valid image format.
  if (format == PIXEL_FORMAT_NULL || (uint)format >= PIXEL_FORMAT_COUNT) return ERR_RT_INVALID_ARGUMENT;
  if (format == PIXEL_FORMAT_I8) return ERR_IMAGE_FORMAT_NOT_SUPPORTED;

  PainterEngine* d = _getRasterPainterEngine(pixels, width, height, stride, format, hints);
  if (!d) return ERR_RT_OUT_OF_MEMORY;

  _engine = d;
  return ERR_OK;
}

err_t Painter::begin(Image& image, int hints)
{
  int format = image.getFormat();

  end();

  err_t err = image.detach();
  if (err) return err;

  if (image.isEmpty()) return ERR_IMAGE_ZERO_SIZE;

  // If image is not empty, the format can't be Null.
  FOG_ASSERT(format != PIXEL_FORMAT_NULL);
  if (format == PIXEL_FORMAT_I8) return ERR_IMAGE_FORMAT_NOT_SUPPORTED;

  uint8_t* data = image.getMData();
  if (!data) return ERR_RT_OUT_OF_MEMORY;

  PainterEngine* d = _getRasterPainterEngine(
    data, image.getWidth(), image.getHeight(), image.getStride(), format, hints);
  if (!d) return ERR_RT_OUT_OF_MEMORY;

  _engine = d;
  return ERR_OK;
}

void Painter::end()
{
  if (_engine != sharedNull) delete _engine;
  _engine = sharedNull;
}

} // Fog namespace
