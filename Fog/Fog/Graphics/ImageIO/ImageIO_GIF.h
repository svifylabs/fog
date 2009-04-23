// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_IMAGEIO_GIF_H
#define _FOG_GRAPHICS_IMAGEIO_GIF_H

// [Dependencies]
#include <Fog/Graphics/ImageIO.h>

namespace Fog {

// [Fog::ImageIO::]
namespace ImageIO {

// [Fog::ImageIO::GifDecoderDevice]

struct FOG_API GifDecoderDevice : public Fog::ImageIO::DecoderDevice
{
private:
  void* _context;

  bool openGif();
  void closeGif();

public:
  GifDecoderDevice();
  virtual ~GifDecoderDevice();

  virtual void reset();
  virtual uint32_t readHeader();
  virtual uint32_t readImage(Fog::Image& image);
};

// [Fog::ImageIO::GifEncoderDevice]

struct FOG_API GifEncoderDevice : public Fog::ImageIO::EncoderDevice
{
  GifEncoderDevice();
  virtual ~GifEncoderDevice();

  virtual uint32_t writeImage(const Fog::Image& image);
};

// [Fog::ImageIO::]
}

} // Fog namespace

// [Guard]
#endif // _FOG_GRAPHICS_IMAGEIO_GIF_H
