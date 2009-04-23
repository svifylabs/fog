// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_IMAGEIO_PCX_H
#define _FOG_GRAPHICS_IMAGEIO_PCX_H

// [Dependencies]
#include <Fog/Core/Memory.h>
#include <Fog/Core/String.h>
#include <Fog/Graphics/ImageIO.h>

namespace Fog {

// [Fog::ImageIO::]
namespace ImageIO {

// [Fog::ImageIO::PcxHeader]

#include <Fog/Core/Pack.h>
struct FOG_PACKED PcxHeader
{
  uint8_t manufacturer;
  uint8_t version;
  uint8_t encoding;
  uint8_t bitsPerPixel;
  int16_t xMin;
  int16_t yMin;
  int16_t xMax;
  int16_t yMax;
  int16_t horizontalDPI;
  int16_t verticalDPI;
  uint8_t colorMap[48];
  uint8_t reserved;
  uint8_t nPlanes;
  int16_t bytesPerLine;
  int16_t paletteInfo;
  int16_t hScreenSize;
  int16_t vScreenSize;
  uint8_t unused[54];
};
#include <Fog/Core/Unpack.h>

// [Fog::ImageIO::PcxDecoderDevice]

struct FOG_API PcxDecoderDevice : public Fog::ImageIO::DecoderDevice
{
private:
  Fog::ImageIO::PcxHeader _pcxFileHeader;

public:
  PcxDecoderDevice();
  virtual ~PcxDecoderDevice();

  virtual void reset();
  virtual uint32_t readHeader();
  virtual uint32_t readImage(Fog::Image& image);

  FOG_INLINE const Fog::ImageIO::PcxHeader& pcxFileHeader() const { return _pcxFileHeader; }

private:

  // clear everything
  FOG_INLINE void zeroall()
  {
    const ulong ddsize = sizeof(Fog::ImageIO::DecoderDevice);

    Fog::Memory::zero(
      (uint8_t*)this + ddsize, 
      sizeof(Fog::ImageIO::PcxDecoderDevice) - ddsize);
  }
};

// [Fog::ImageIO::PcxEncoder]

struct FOG_API PcxEncoderDevice : public Fog::ImageIO::EncoderDevice
{
  PcxEncoderDevice();
  virtual ~PcxEncoderDevice();

  virtual uint32_t writeImage(const Fog::Image& image);
};


// [Fog::ImageIO::]
}

} // Fog namespace

// [Guard]
#endif // _FOG_GRAPHICS_IMAGEIO_PCX_H
