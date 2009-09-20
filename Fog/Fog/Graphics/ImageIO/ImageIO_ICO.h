// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_IMAGEIO_ICO_H
#define _FOG_GRAPHICS_IMAGEIO_ICO_H

// [Dependencies]
#include <Fog/Core/Memory.h>
#include <Fog/Graphics/ImageIO.h>

//! @addtogroup Fog_Graphics_ImageIO
//! @{

namespace Fog {
namespace ImageIO {

#include <Fog/Core/Pack.h>

struct FOG_PACKED IcoHeader
{
  uint16_t reserved;
  uint16_t type;
  uint16_t count;
  // sizeof(IcoHeader) = 6
};

struct FOG_PACKED IcoEntry
{
  uint8_t width;         // byte offset 0
  uint8_t height;        // byte offset 1
  uint8_t colorCount;    // byte offset 2
  uint8_t reserved;      // byte offset 3
  uint16_t colorPlanes;  // byte offset 4
  uint16_t bpp;          // byte offset 6
  uint32_t size;         // byte offset 8
  uint32_t offset;       // byte offset 12
  // sizeof(IcoEntry) = 16
};

#include <Fog/Core/Unpack.h>

// ============================================================================
// [Fog::ImageIO::IcoDecoderDevice]
// ============================================================================

struct FOG_API IcoDecoderDevice : public DecoderDevice
{
  IcoDecoderDevice();
  virtual ~IcoDecoderDevice();

  virtual void reset();
  virtual uint32_t readHeader();
  virtual uint32_t readImage(Image& image);

private:
  //! For determining offsets/sizes of "frames"
  //! LE numbers are already converted to BE numbers
  //! on BE systems
  IcoEntry *_framesInfo;

  //! Current position in the stream, to subtract from frame offset
  //! Needed for reading from non-seekable streams and to support
  //! loading ICO from current non-zero stream position
  int64_t _currentOffset;
};

} // ImageIO namespace
} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_IMAGEIO_ICO_H

