// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_IMAGEIO_JPEG_H
#define _FOG_GRAPHICS_IMAGEIO_JPEG_H

// [Dependencies]
#include <Fog/Graphics/ImageIO.h>

#if defined(FOG_HAVE_JPEGLIB_H)

//! @addtogroup Fog_Graphics_ImageIO
//! @{

namespace Fog {
namespace ImageIO {

// ============================================================================
// [Fog::ImageIO::JpegDecoderDevice]
// ============================================================================

struct FOG_API JpegDecoderDevice : public DecoderDevice
{
public:
  JpegDecoderDevice();
  virtual ~JpegDecoderDevice();

  virtual void reset();
  virtual uint32_t readHeader();
  virtual uint32_t readImage(Image& image);
};

// ============================================================================
// [Fog::ImageIO::JpegEncoderDevice]
// ============================================================================

struct FOG_API JpegEncoderDevice : public EncoderDevice
{
  JpegEncoderDevice();
  virtual ~JpegEncoderDevice();

  virtual uint32_t writeImage(const Image& image);

  // [Properties]

  virtual err_t setProperty(const String32& name, const Value& value);
  virtual Value getProperty(const String32& name);

private:
  int _quality;
};

} // ImageIO namespace
} // Fog namespace

//! @}

#endif // FOG_HAVE_JPEGLIB_H

// [Guard]
#endif // _FOG_GRAPHICS_IMAGEIO_JPEG_H
