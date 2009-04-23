// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_IMAGEIO_PNG_H
#define _FOG_GRAPHICS_IMAGEIO_PNG_H

// [Dependencies]
#include <Fog/Graphics/ImageIO.h>

#if defined(FOG_HAVE_PNG_H)

namespace Fog {

// [Fog::ImageIO::]
namespace ImageIO {

// [Fog::ImageIO::PngDecoderDevice]

struct FOG_API PngDecoderDevice : public DecoderDevice
{
public:
  PngDecoderDevice();
  virtual ~PngDecoderDevice();

  virtual void reset();
  virtual uint32_t readHeader();
  virtual uint32_t readImage(Image& image);

private:
  void* _png_ptr;
  void* _png_info_ptr;
  int _png_bit_depth;
  int _png_color_type;
  int _png_interlace_type;

  uint32_t _createPngStream();
  void _deletePngStream();
};

// [Fog::ImageIO::PngEncoderDevice]

struct FOG_API PngEncoderDevice : public EncoderDevice
{
  PngEncoderDevice();
  virtual ~PngEncoderDevice();

  virtual uint32_t writeImage(const Image& image);
};

// [Fog::ImageIO::]
}

} // Fog namespace

#endif // FOG_HAVE_PNG_H

// [Guard]
#endif // _FOG_GRAPHICS_IMAGEIO_PNG_H
