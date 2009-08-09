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
  typedef DecoderDevice base;

  // [Construction / Destruction]

  JpegDecoderDevice();
  virtual ~JpegDecoderDevice();

  // [Virtuals]

  virtual void reset();
  virtual uint32_t readHeader();
  virtual uint32_t readImage(Image& image);
};

// ============================================================================
// [Fog::ImageIO::JpegEncoderDevice]
// ============================================================================

struct FOG_API JpegEncoderDevice : public EncoderDevice
{
  typedef EncoderDevice base;

  // [Construction / Destruction]

  JpegEncoderDevice();
  virtual ~JpegEncoderDevice();

  // [Virtuals]

  virtual uint32_t writeImage(const Image& image);

  // [Properties]

  FOG_DECLARE_PROPERTIES_CONTAINER()

  enum PropertyId
  {
    PropertyQuality = base::PropertyLast,

    PropertyLast
  };

  virtual err_t getProperty(int id, Value& value) const;
  virtual err_t setProperty(int id, const Value& value);

private:
  int _quality;
};

} // ImageIO namespace
} // Fog namespace

//! @}

#endif // FOG_HAVE_JPEGLIB_H

// [Guard]
#endif // _FOG_GRAPHICS_IMAGEIO_JPEG_H
