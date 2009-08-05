// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Graphics/Error.h>
#include <Fog/Graphics/Painter.h>
#include <Fog/Graphics/PainterEngine.h>
#include <Fog/Graphics/PainterEngine_Null.h>
#include <Fog/Graphics/PainterEngine_Raster.h>

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
  if (pixels == NULL) return Error::InvalidArgument;

  // Never initialize painter for zero image.
  if (width <= 0 || height <= 0)
  {
    if (width == 0 && height == 0)
      return Error::ImageSizeIsZero;
    else
      return Error::ImageSizeIsInvalid;
  }

  // Check for valid image format.
  if (format == Image::FormatNull || format >= Image::FormatCount) return Error::InvalidArgument;
  if (format == Image::FormatI8) return Error::ImageFormatNotSupported;

  PainterEngine* d = _getRasterPainterEngine(pixels, width, height, stride, format, hints);
  if (!d) return Error::OutOfMemory;

  _engine = d;
  return Error::Ok;
}

err_t Painter::begin(Image& image, int hints)
{
  int format = image.format();

  end();

  err_t err = image.detach();
  if (err) return err;

  if (image.isEmpty()) return Error::ImageSizeIsZero;

  // If image is not empty, the format can't be Null.
  FOG_ASSERT(format != Image::FormatNull);
  if (format == Image::FormatI8) return Error::ImageFormatNotSupported;

  uint8_t* data = image.mData();
  if (!data) return Error::OutOfMemory;

  PainterEngine* d = _getRasterPainterEngine(data, image.width(), image.height(), image.stride(), format, hints);
  if (!d) return Error::OutOfMemory;

  _engine = d;
  return Error::Ok;
}

void Painter::end()
{
  if (_engine != sharedNull) delete _engine;
  _engine = sharedNull;
}

} // Fog namespace
