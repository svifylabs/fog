// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_IMAGEIO_BMP_H
#define _FOG_GRAPHICS_IMAGEIO_BMP_H

// [Dependencies]
#include <Fog/Core/Memory.h>
#include <Fog/Graphics/ImageIO.h>

namespace Fog {
namespace ImageIO {

// ============================================================================
// [Fog::ImageIO::BmpHeader]
// ============================================================================

#include <Fog/Core/Pack.h>
struct FOG_PACKED BmpHeader
{
  // ----- FILE header - 14 bytes -----

  uint16_t magic;         // BM
  uint32_t fileSize;
  uint16_t reserved1;
  uint16_t reserved2;
  uint32_t imageOffset;   // 54

  // ----- BITMAP header - 40 bytes -----

  uint32_t headerSize;    // 40
  uint32_t width;
  uint32_t height;
  uint16_t planes;        // 1
  uint16_t bitsPerPixel;  // 8, 24 or 32
  uint32_t compression;   // BI_RGB
  uint32_t imageSize;
  uint32_t reserved3;
  uint32_t reserved4;
  uint32_t reserved5;
  uint32_t reserved6;
};
#include <Fog/Core/Unpack.h>

// ============================================================================
// [Fog::ImageIO::BMP_BI]
// ============================================================================

enum BMP_BI
{
  BMP_BI_RGB        = 0,
  BMP_BI_RLE8       = 1,
  BMP_BI_RLE4       = 2,
  BMP_BI_BITFIELDS  = 3
};

// ============================================================================
// [Fog::ImageIO::BMP_RLE]
// ============================================================================

enum BMP_RLE
{
  BMP_RLE_NEXT_LINE = 0,
  BMP_RLE_END       = 1,
  BMP_RLE_MOVE      = 2
};

// ============================================================================
// [Fog::ImageIO::BmpDecoderDevice]
// ============================================================================

struct FOG_API BmpDecoderDevice : public DecoderDevice
{
private:
  enum HeaderType
  {
    OS2Header = 0,
    WinHeader = 1
  };

  uint32_t _bmpHeaderType;

  // raw data from stream

  uint8_t _bmpFileHeader[18]; // 14 + 4
  uint8_t _bmpDataHeader[36]; // 40 - 4

  // bmp
  uint32_t _bmpOffset;
  uint32_t _bmpHeaderSize;
  uint32_t _bmpCompression;
  uint32_t _bmpImageSize;
  uint32_t _bmpStride;

  // rgb
  uint32_t _rMask;
  uint32_t _gMask;
  uint32_t _bMask;

  uint32_t _rShift;
  uint32_t _gShift;
  uint32_t _bShift;

  // rgb - only used by converter from 16 BPP
  uint32_t _rLoss;
  uint32_t _gLoss;
  uint32_t _bLoss;

  // true if 16 bpp is byteswapped (big endian machine)
  uint32_t _byteSwapped16;

public:
  BmpDecoderDevice();
  virtual ~BmpDecoderDevice();

  virtual void reset();
  virtual uint32_t readHeader();
  virtual uint32_t readImage(Image& image);

  FOG_INLINE uint32_t bmpHeaderType() const { return _bmpHeaderType; }
  FOG_INLINE uint32_t bmpOffset() const { return _bmpOffset; }
  FOG_INLINE uint32_t bmpHeaderSize() const { return _bmpHeaderSize; }
  FOG_INLINE uint32_t bmpCompression() const { return _bmpCompression; }
  FOG_INLINE uint32_t bmpImageSize() const { return _bmpImageSize; }
  FOG_INLINE uint32_t bmpStride() const { return _bmpStride; }

  FOG_INLINE uint32_t rMask() const { return _rMask; }
  FOG_INLINE uint32_t gMask() const { return _gMask; }
  FOG_INLINE uint32_t bMask() const { return _bMask; }

  FOG_INLINE uint32_t rShift() const { return _rShift; }
  FOG_INLINE uint32_t gShift() const { return _gShift; }
  FOG_INLINE uint32_t bShift() const { return _bShift; }

  FOG_INLINE uint32_t rLoss() const { return _rLoss; }
  FOG_INLINE uint32_t gLoss() const { return _gLoss; }
  FOG_INLINE uint32_t bLoss() const { return _bLoss; }

  FOG_INLINE uint32_t byteSwapped16() const { return _byteSwapped16; }

private:
  // clear everything
  FOG_INLINE void zeroall()
  {
    const ulong ddsize = sizeof(DecoderDevice);

    Memory::zero(
      (uint8_t*)this + ddsize, 
      sizeof(BmpDecoderDevice) - ddsize);
  }
};

// ============================================================================
// [Fog::ImageIO::BmpEncoderDevice]
// ============================================================================

struct FOG_API BmpEncoderDevice : public EncoderDevice
{
  BmpEncoderDevice();
  virtual ~BmpEncoderDevice();

  virtual uint32_t writeImage(const Image& image);
};

} // ImageIO namespace
} // Fog namespace

// [Guard]
#endif // _FOG_GRAPHICS_IMAGEIO_BMP_H
