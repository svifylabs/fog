// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/Memory.h>
#include <Fog/Core/Misc.h>
#include <Fog/Core/Stream.h>
#include <Fog/Core/String.h>
#include <Fog/Core/StringCache.h>
#include <Fog/Core/Strings.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/ImageIO/ImageIO_BMP.h>
#include <Fog/Graphics/Raster_p.h>

namespace Fog { 
namespace ImageIO {

// ============================================================================
// [Fog::ImageIO::BmpProvider]
// ============================================================================

struct BmpProvider : public Provider
{
  BmpProvider();
  virtual ~BmpProvider();

  virtual uint32_t check(const void* mem, sysuint_t length);
  virtual EncoderDevice* createEncoder();
  virtual DecoderDevice* createDecoder();
};

BmpProvider::BmpProvider()
{
  // features
  _features.decoder = true;
  _features.encoder = true;

  _features.mono = true;
  _features.pal1 = true;
  _features.pal4 = true;
  _features.pal8 = true;
  _features.rgb15 = true;
  _features.rgb16 = true;
  _features.rgb24 = true;
  _features.argb32 = true;
  _features.rle4 = true;
  _features.rle8 = true;

  _features.rgbAlpha = true;

  // name
  _name = fog_strings->get(STR_GRAPHICS_BMP);

  // extensions
  _extensions.reserve(2);
  _extensions.append(fog_strings->get(STR_GRAPHICS_bmp));
  _extensions.append(fog_strings->get(STR_GRAPHICS_ras));
}

BmpProvider::~BmpProvider()
{
}

uint32_t BmpProvider::check(const void* mem, sysuint_t length)
{
  if (length == 0) return 0;

  const uint8_t* m = (const uint8_t*)mem;

  // Check for 'BM' mime
  if (length >= 1 && m[0] != (uint8_t)'B') return 0;
  if (length >= 2 && m[1] != (uint8_t)'M') return 0;

  // Check for correct header size
  if (length >= 18)
  {
    uint32_t headerSize = Memory::bswap32le( *(const uint32_t *)(m + 14) );
    if (headerSize != 12 || headerSize != 40) return 0;

    return 90;
  }
  else
    return 75;
}

EncoderDevice* BmpProvider::createEncoder()
{
  return new BmpEncoderDevice();
}

DecoderDevice* BmpProvider::createDecoder()
{
  return new BmpDecoderDevice();
}

// ============================================================================
// [Fog::ImageIO::BmpDecoderDevice]
// ============================================================================

BmpDecoderDevice::BmpDecoderDevice()
{
  zeroall();
}

BmpDecoderDevice::~BmpDecoderDevice()
{
}

// ============================================================================
// [Fog::ImageIO::BmpDecoderDevice::reset]
// ============================================================================

void BmpDecoderDevice::reset()
{
  DecoderDevice::reset();
  zeroall();
}

// ============================================================================
// [Fog::ImageIO::BmpDecoderDevice::readHeader]
// ============================================================================

uint32_t BmpDecoderDevice::readHeader()
{
  // don't read header more than once
  if (headerDone()) return headerResult();

  // mark header as done
  _headerDone = true;

  // read bmp header
  if (stream().read(_bmpFileHeader, 14+4) != 14+4) 
  {
    return (_headerResult = Error::ImageIO_Truncated);
  }

  // try to match bmp mime "BM"
  if (*(uint16_t*)(_bmpFileHeader) != FOG_MAKE_UINT16_SEQ('B', 'M')) 
  { 
    return (_headerResult = Error::ImageIO_MimeNotMatch);
  }

  // File header structure looks like this (extracted from bmp header):
  // {
  //   uint16_t magic;         // 00-02 | "BM"
  //   uint32_t fileSize;      // 02-06 |
  //   uint16_t reserved1;     // 06-08 |
  //   uint16_t reserved2;     // 08-10 |
  //   uint32_t imageOffset;   // 10-14 | 54
  //   uint32_t headerSize;    // 14-18 | 40
  // };

  _bmpOffset     = Memory::bswap32le( *(const uint32_t *)(_bmpFileHeader + 10) );
  _bmpHeaderSize = Memory::bswap32le( *(const uint32_t *)(_bmpFileHeader + 14) );

  // old bmp header (OS2)
  if (bmpHeaderSize() == 12)
  {
    _bmpHeaderType = OS2Header;

    if (stream().read(_bmpDataHeader, 12-4) != 12-4) 
    {
      return (_headerResult = Error::ImageIO_Truncated);
    }

    _width          = Memory::bswap16le( *(const uint16_t *)(_bmpDataHeader + 4  - 4) );
    _height         = Memory::bswap16le( *(const uint16_t *)(_bmpDataHeader + 6  - 4) );
    _depth          = Memory::bswap16le( *(const uint16_t *)(_bmpDataHeader + 10 - 4) );
    _planes         = Memory::bswap16le( *(const uint16_t *)(_bmpDataHeader + 8  - 4) );

    _bmpCompression = BMP_BI_RGB;
  }

  // Windows bmp header 
  else if (bmpHeaderSize() == 40) 
  {
    _bmpHeaderType = WinHeader;

    if (stream().read(_bmpDataHeader, 40-4) != 40-4) 
    {
      return (_headerResult = Error::ImageIO_Truncated);
    }

    _width          = Memory::bswap32le( *(const uint32_t *)(_bmpDataHeader +  4 - 4) );
    _height         = Memory::bswap32le( *(const uint32_t *)(_bmpDataHeader +  8 - 4) );
    _depth          = Memory::bswap16le( *(const uint16_t *)(_bmpDataHeader + 14 - 4) );
    _planes         = Memory::bswap16le( *(const uint16_t *)(_bmpDataHeader + 12 - 4) );

    _bmpCompression = Memory::bswap32le( *(const uint32_t *)(_bmpDataHeader + 16 - 4) );
    _bmpImageSize   = Memory::bswap32le( *(const uint32_t *)(_bmpDataHeader + 20 - 4) );
  }

  // Malformed or invalid header size
  else
  {
    return (_headerResult = Error::ImageIO_FormatNotSupported);
  }

  // check for correct depth
  switch(depth())
  {
    case 1:
    case 4:
    case 8:
    case 16:
    case 24:
    case 32:
      break;
    default:
      return (_headerResult = Error::ImageIO_FormatNotSupported);
  }

  // check for zero dimensions
  if (areDimensionsZero())
  {
    return (_headerResult = Error::ImageSizeIsZero);
  }

  // check for too large dimensions
  if (areDimensionsTooLarge())
  {
    return (_headerResult = Error::ImageSizeIsTooLarge);
  }

  // bmp contains only one image
  _actualFrame = 0;
  _framesCount = 1;

  _bmpStride = (((width() * depth() + 7) >> 3) + 3) & ~3;

  // os2 header didn't define image size
  if (bmpImageSize() == 0) _bmpImageSize = bmpStride() * height();

  switch (depth())
  {
    // setup palette
    case 1:
    case 4:
    case 8:
    {
      Rgba* pdestCur = _palette.mData();
      sysuint_t i, nColors = (bmpOffset() - bmpHeaderSize() - 14);

      if (bmpHeaderSize() == 12)
      {
        uint8_t psrc24[768];
        uint8_t* psrc24Cur = psrc24;

        nColors /= 3;
        if (nColors > 256) nColors = 256;

        if (stream().read(psrc24, nColors*3) != nColors*3)
        {
          return Error::ImageIO_Truncated;
        }

        for (i = nColors; i; i--, pdestCur++, psrc24Cur += 3)
        {
          pdestCur->set(psrc24Cur[2], psrc24Cur[1], psrc24Cur[0]);
        }
      }
      else
      {
        nColors /= 4;
        if (nColors > 256) nColors = 256;

        if (stream().read(pdestCur, nColors*4) != nColors*4) 
        {
          return Error::ImageIO_Truncated;
        }

#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
        for (i = 0; i != nColors; i++, pdestCur++) *pdestCur |= 0xFF000000;
#else
        for (i = 0; i != nColors; i++, pdestCur++) *pdestCur = Memory::bswap32(pdestCur[i]) | 0xFF000000;
#endif
      }

      _format = Image::FormatI8;
      break;
    }

    // setup rgb
    case 16:
      _rMask = 0x7C00; _rShift = 10; _rLoss = 3;
      _gMask = 0x03E0; _gShift =  5; _gLoss = 3;
      _bMask = 0x001F; _bShift =  0; _bLoss = 3;

      _format = Image::FormatRGB24;
      break;

    case 24:
#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
      _rMask = 0x00FF0000; _rShift = 16;
      _gMask = 0x0000FF00; _gShift =  8;
      _bMask = 0x000000FF; _bShift =  0;
#else
      _rMask = 0x000000FF; _rShift =  0;
      _gMask = 0x0000FF00; _gShift =  8;
      _bMask = 0x00FF0000; _bShift = 16;
#endif // FOG_BYTE_ORDER
      _format = Image::FormatRGB24;
      break;

    case 32:
#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
      _rMask = 0x00FF0000; _rShift = 16;
      _gMask = 0x0000FF00; _gShift =  8;
      _bMask = 0x000000FF; _bShift =  0;
#else
      _rMask = 0x0000FF00; _rShift =  8;
      _gMask = 0x00FF0000; _gShift = 16;
      _bMask = 0xFF000000; _bShift = 24;
#endif // FOG_BYTE_ORDER
      _format = Image::FormatRGB32;
      break;
  }

  // Bitfields
  if (bmpCompression() == BMP_BI_BITFIELDS && (depth() == 16 || depth() == 32))
  {
    uint8_t masks[12];
    int bit;

    uint rSize = 0;
    uint gSize = 0;
    uint bSize = 0;

    if (stream().read(masks, 12) != 12) 
    {
      return (_headerResult = Error::ImageIO_Truncated);
    }

    _rMask = Memory::bswap32le( *((const uint32_t *)(masks + 0)) );
    _gMask = Memory::bswap32le( *((const uint32_t *)(masks + 4)) );
    _bMask = Memory::bswap32le( *((const uint32_t *)(masks + 8)) );

    for (bit = (int)(depth()) - 1; bit >= 0; bit--)
    {
      if (bMask() & (1 << bit)) { _bShift = bit; bSize++; }
      if (gMask() & (1 << bit)) { _gShift = bit; gSize++; }
      if (rMask() & (1 << bit)) { _rShift = bit; rSize++; }
    }

    _rLoss = 8 - rSize;
    _gLoss = 8 - gSize;
    _bLoss = 8 - bSize;
  }

  // success
  return (_headerResult = Error::Ok);
}

// ============================================================================
// [BmpDecoderDevice::readImage]
// ============================================================================

uint32_t BmpDecoderDevice::readImage(Image& image)
{
  // read bmp header
  if (readHeader() != Error::Ok)
  {
    return headerResult();
  }

  // don't read image more than once
  if (readerDone()) return (_readerResult = Error::ImageIO_NotAnimationFormat);

  // error code (default is success)
  uint32_t err = Error::Ok;

  // buffer
  uint8_t* pixelsBegin;
  uint8_t* pixelsCur;

  // reader variables
  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t i;
  uint32_t stride;

  MemoryBuffer<4096> bufferStorage;
  MemoryBuffer<4096> rleBufferStorage;
  uint8_t* buffer = (uint8_t *)bufferStorage.alloc(bmpStride());
  uint8_t* rleBuffer = NULL;

  // create image
  if ( (err = image.create(width(), height(), format())) ) goto end;

  stride = image.stride();
  pixelsBegin = image._d->first;
  pixelsCur = pixelsBegin + stride * (height() - 1);

  // -------------------- Conversion from 1 bit depth --------------------

  if (depth() == 1)
  {
    if (bmpCompression() == BMP_BI_RGB)
    {
      uint8_t* bufferCur;
      uint32_t b;

      for (y = 0; y != height(); y++)
      {
        if (stream().read(buffer, bmpStride()) != bmpStride()) goto truncated;
        bufferCur = buffer;
        pixelsCur = pixelsBegin + (height() - y - 1) * stride;

        for (i = width(); i >= 8; i -= 8, pixelsCur += 8, bufferCur++)
        {
          b = (uint32_t)(*bufferCur);
          ((uint8_t *)pixelsCur)[0] = (uint8_t)((b >> 7) & 1);
          ((uint8_t *)pixelsCur)[1] = (uint8_t)((b >> 6) & 1);
          ((uint8_t *)pixelsCur)[2] = (uint8_t)((b >> 5) & 1);
          ((uint8_t *)pixelsCur)[3] = (uint8_t)((b >> 4) & 1);
          ((uint8_t *)pixelsCur)[4] = (uint8_t)((b >> 3) & 1);
          ((uint8_t *)pixelsCur)[5] = (uint8_t)((b >> 2) & 1);
          ((uint8_t *)pixelsCur)[6] = (uint8_t)((b >> 1) & 1);
          ((uint8_t *)pixelsCur)[7] = (uint8_t)((b     ) & 1);
        }
        if (i)
        {
          b = (uint32_t)(*bufferCur);
          for (; i; i--, pixelsCur += 1, b <<= 1)
          {
            ((uint8_t *)pixelsCur)[0] = (uint8_t)((b >> 7) & 1);
          }
        }
        if ((y & 15) == 0) { updateProgress(y, height()); }
      }
    }
  }

  // -------------------- Conversion from 4 bit depth --------------------

  else if (depth() == 4)
  {
    // ==== 4 BIT RLE DECOMPRESSION BEGIN ====
    if (bmpCompression() == BMP_BI_RLE4)
    {
      uint8_t* rleCur;
      uint8_t* rleEnd;
      uint8_t b0;
      uint8_t b1;

      if ((rleBuffer = (uint8_t *)rleBufferStorage.alloc(bmpImageSize())) == NULL) goto outOfMemory;
      if (stream().read(rleBuffer, bmpImageSize()) != bmpImageSize()) goto truncated;

      rleCur = rleBuffer;
      rleEnd = rleBuffer + bmpImageSize();

BI_RLE_4_BEGIN:
      if (x >= width() || y >= height()) goto rleError;

      pixelsCur = pixelsBegin + (height() - y - 1) * stride;
      updateProgress(y, height());

      for (;;)
      {
        if (rleCur + 2 > rleEnd) goto truncated;
        b0 = *rleCur++;
        b1 = *rleCur++;

        if (b0)
        {
          // b0 = Length
          // b1 = 2 Colors
          uint8_t c0 = b1 >> 4;
          uint8_t c1 = b1 & 0xF;

          i = b0;
          if (i > width() - x) goto rleError;

          while (i >= 2)
          {
            *pixelsCur++ = c0;
            *pixelsCur++ = c1;
            i -= 2;
          }
          if (i) *pixelsCur++ = c0;
        }
        else
        {
          switch (b1)
          {
            case BMP_RLE_NEXT_LINE: x = 0; y++; goto BI_RLE_4_BEGIN;
            case BMP_RLE_END: goto end;
            case BMP_RLE_MOVE:
              if (rleCur + 2 > rleEnd) goto truncated;
              x += *rleCur++;
              y += *rleCur++;
              goto BI_RLE_4_BEGIN;
            // FILL BITS (b1 == length)
            default:
              i = b1;

              if (i > width() - x) goto rleError;
              if (rleCur + (i >> 1) + (i & 1) > rleEnd) goto truncated;

              x += i;

              while (i >= 2)
              {
                b0 = *rleCur++;
                *pixelsCur++ = b0 >> 4;
                *pixelsCur++ = b0 & 0xF;
                i -= 2;
              }
              if (i)
              {
                *pixelsCur++ = *rleCur++ >> 4;
              }

              if ((b1 & 3) == 1) rleCur += 2;
              else if ((b1 & 3) == 2) rleCur++;

              break;
          }
        }
      }
    }

    // ==== 4 BIT RAW BEGIN ====
    else if (bmpCompression() == BMP_BI_RGB)
    {
      uint8_t* bufferCur;
      uint8_t b;

      for (y = 0; y != height(); y++)
      {
        if (stream().read(buffer, bmpStride()) != bmpStride()) goto truncated;
        bufferCur = buffer;
        pixelsCur = pixelsBegin + (height() - y - 1) * stride;

        for (x = 0; x + 2 <= width(); x += 2)
        {
          b = *bufferCur++;
          *pixelsCur++ = b >> 4;
          *pixelsCur++ = b & 0xF;
        }
        if (x < width())
        {
          *pixelsCur = *bufferCur >> 4;
        }
        if ((y & 15) == 0) updateProgress(y, height());
      }
    }
  }

  // -------------------- Conversion from 8 bit depth --------------------

  else if (depth() == 8)
  {
    // ==== 8 BIT RLE DECOMPRESSION BEGIN ====
    if (bmpCompression() == BMP_BI_RLE8)
    {
      uint8_t* rleCur;
      uint8_t* rleEnd;
      uint8_t b0;
      uint8_t b1;

      if ((rleBuffer = (uint8_t *)rleBufferStorage.alloc(bmpImageSize())) == NULL) goto outOfMemory;
      if (stream().read(rleBuffer, bmpImageSize()) != bmpImageSize()) goto truncated;

      rleCur = rleBuffer;
      rleEnd = rleBuffer + bmpImageSize();

BI_RLE_8_BEGIN:
      if (x >= width() || y >= height()) goto rleError;

      pixelsCur = pixelsBegin + (height() - y - 1) * stride;
      updateProgress(y, height());

      for (;;)
      {
        if (rleCur + 2 > rleEnd) goto truncated;
        b0 = *rleCur++;
        b1 = *rleCur++;

        if (b0)
        {
          // b0 = Length
          // b1 = Color
          i = b0;
          if (i > width() - x) goto rleError;

          while (i--) *pixelsCur++ = b1;
        }
        else 
        {
          // b1 = Chunk type
          switch (b1)
          {
            case BMP_RLE_NEXT_LINE: x = 0; y++; goto BI_RLE_8_BEGIN;
            case BMP_RLE_END: goto end;
            case BMP_RLE_MOVE:
              if (rleCur + 2 > rleEnd) goto truncated;
              x += *rleCur++;
              y += *rleCur++; 
              goto BI_RLE_8_BEGIN;
            // FILL BITS (b1 == length)
            default:
              i = b1;

              if (i > width() - x) goto rleError;
              if (rleCur + i > rleEnd) goto truncated;

              while (i--) *pixelsCur++ = *rleCur++;

              if (b1 & 1) rleCur++;
              x += i;
              break;
          }
        }
      }
      updateProgress(y, height());
    }

    // ==== 8 BIT RAW BEGIN ====
    else if (bmpCompression() == BMP_BI_RGB)
    {
      for (y = 0; y < height(); y++)
      {
        pixelsCur = pixelsBegin + (height() - y - 1) * stride;
        if (stream().read(pixelsCur, bmpStride()) != bmpStride()) goto truncated;
        if ((y & 15) == 0) updateProgress(y, height());
      }
    }
  }

  // --------------- Conversion from 16, 24 and 32 bit depth --------------

#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
  // Direct loading.
  else if (
    ((depth() == 32 && format() == Image::FormatARGB32) ||
     (depth() == 32 && format() == Image::FormatRGB32)  ||
     (depth() == 24 && format() == Image::FormatRGB24)) &&
     rMask() == Raster::RGB32_RMask &&
     gMask() == Raster::RGB32_GMask &&
     bMask() == Raster::RGB32_BMask)
  {
    sysuint_t readBytes = width() * (depth() >> 3);
    sysuint_t tailBytes = bmpStride() - readBytes;

    for (y = 0; y < height(); y++)
    {
      pixelsCur = pixelsBegin + (height() - y - 1) * stride;
      if (stream().read(pixelsCur, readBytes) != readBytes ||
        stream().read(buffer, tailBytes) != tailBytes)
      {
        goto truncated;
      }
      if ((y & 15) == 0) updateProgress(y, height());
    }
  }
#endif // FOG_LITTLE_ENDIAN
  // Buffered loading.
  else
  {
    uint32_t m;
    uint32_t rShift = 0, rSize = 0;
    uint32_t gShift = 0, gSize = 0;
    uint32_t bShift = 0, bSize = 0;
    uint32_t aShift = 0, aSize = 0;

    uint32_t _aMask;

    for (m = _rMask; (m & 1) == 0; m >>= 1) rShift++;
    while ((m & 1) == 1) rSize++;

    for (m = _gMask; (m & 1) == 0; m >>= 1) gShift++;
    while ((m & 1) == 1) gSize++;

    for (m = _bMask; (m & 1) == 0; m >>= 1) bShift++;
    while ((m & 1) == 1) bSize++;

    if (depth() == 32)
    {
      _aMask = (_rMask | _gMask | _bMask) ^ 0xFFFFFFFF;
      for (m = _bMask; (m & 1) == 0; m >>= 1) aShift++;
      while ((m & 1) == 1) aSize++;
    }

    for (y = 0; y < _height; y++, pixelsCur += stride)
    {
      uint8_t* bufferCur = buffer;
      pixelsCur = pixelsBegin + (height() - y - 1) * stride;
      if (stream().read(bufferCur, _bmpStride) != _bmpStride) goto truncated;

      switch (depth())
      {
        case 16:
          for (m = 0; m < _width; m++, pixelsCur += 3, bufferCur += 2)
          {
            uint32_t pix = Memory::bswap16le(((const uint16_t*)bufferCur)[0]);
            uint32_t pixr = ((pix & _rMask) >> rShift) << (8 - rSize);
            uint32_t pixg = ((pix & _gMask) >> gShift) << (8 - gSize);
            uint32_t pixb = ((pix & _bMask) >> bShift) << (8 - bSize);

            pixelsCur[Raster::RGB24_RByte] = pixr |= (pixr >> rSize);
            pixelsCur[Raster::RGB24_GByte] = pixg |= (pixg >> gSize);
            pixelsCur[Raster::RGB24_BByte] = pixb |= (pixb >> bSize);
          }
          break;
        case 24:
          // 24 bit fields are not supported, standard format is BGR.
          for (m = 0; m < _width; m++, pixelsCur += 3, bufferCur += 3)
          {
            pixelsCur[Raster::RGB24_BByte] = bufferCur[0];
            pixelsCur[Raster::RGB24_GByte] = bufferCur[1];
            pixelsCur[Raster::RGB24_RByte] = bufferCur[2];
          }
          break;
        case 32:
          for (m = 0; m < _width; m++, pixelsCur += 4, bufferCur += 4)
          {
            uint32_t pix = Memory::bswap32le(((const uint32_t*)bufferCur)[0]);
            uint32_t pixr = ((pix & _rMask) >> rShift);
            uint32_t pixg = ((pix & _gMask) >> gShift);
            uint32_t pixb = ((pix & _bMask) >> bShift);
            uint32_t pixa = ((pix & _aMask) >> aShift);

            ((uint32_t*)pixelsCur)[0] =
              (pixr << Raster::RGB32_RShift) |
              (pixg << Raster::RGB32_GShift) |
              (pixb << Raster::RGB32_BShift) |
              (pixa << Raster::RGB32_AShift) ;
          }
          break;
        default:
          FOG_ASSERT_NOT_REACHED();
          break;
      }

      if ((y & 15) == 0) updateProgress(y, height());
    }
  }

  // apply palette if needed
  if (depth() <= 8) image.setPalette(palette());

  goto end;

truncated:
  err = Error::ImageIO_Truncated;
  goto end;
rleError:
  err = Error::ImageIO_RleError;
  goto end;
outOfMemory:
  err = Error::OutOfMemory;
  goto end;
end:
  if (err == Error::Ok) updateProgress(1.0);
  return (_readerResult = err);
}

// ============================================================================
// [Fog::ImageIO::BmpEncoder]
// ============================================================================

BmpEncoderDevice::BmpEncoderDevice()
{
}

BmpEncoderDevice::~BmpEncoderDevice()
{
}

uint32_t BmpEncoderDevice::writeImage(const Image& image)
{
  static const uint8_t tailBytes[4] = { 0, 0, 0, 0 };
  err_t err = Error::Ok;

  Image::Data* d = image._d;
  int width = d->width;
  int height = d->height;
  int format = d->format;
  int bpl;
  int skip;

  BmpHeader bmpHeader;

  MemoryBuffer<4096> bufferLocal;

  if (!width || !height)
  {
    err = Error::ImageSizeIsInvalid;
    goto end;
  }

  // file bmpHeader
  bmpHeader.magic       = FOG_MAKE_UINT16_SEQ('B', 'M');
  bmpHeader.reserved1   = 0x0000;
  bmpHeader.reserved2   = 0x0000;
  bmpHeader.imageOffset = Memory::bswap32le(54);

  // BMP bmpHeader
  bmpHeader.headerSize  = Memory::bswap32le(40);
  bmpHeader.width       = Memory::bswap32le(width);
  bmpHeader.height      = Memory::bswap32le(height);
  bmpHeader.planes      = Memory::bswap16le(1);
  bmpHeader.compression = Memory::bswap32le(BMP_BI_RGB);
  bmpHeader.reserved3   = 0x00000000;
  bmpHeader.reserved4   = 0x00000000;
  bmpHeader.reserved5   = 0x00000000;
  bmpHeader.reserved6   = 0x00000000;

  switch (format)
  {
    case Image::FormatARGB32:
    case Image::FormatPRGB32:
    {
      uint imageSize;

      bpl = width * 4;
      skip = 0;
      imageSize = bpl * height;

      bmpHeader.fileSize     = Memory::bswap32le(54 + imageSize);
      bmpHeader.bitsPerPixel = Memory::bswap16le(32);
      bmpHeader.imageSize    = Memory::bswap32le(imageSize);

      break;
    }

    case Image::FormatRGB32:
    case Image::FormatRGB24:
    {
      uint imageSize;

      bpl = width * 3;
      skip = (4 - (bpl & 3)) & 3;
      imageSize = (bpl + skip) * height;

      bmpHeader.fileSize     = Memory::bswap32le(54 + imageSize);
      bmpHeader.bitsPerPixel = Memory::bswap16le(24);
      bmpHeader.imageSize    = Memory::bswap32le(imageSize);

      break;
    }

    case Image::FormatA8:
    case Image::FormatI8:
    {
      uint imageSize;

      bpl = width;
      skip = (4 - (bpl & 3)) & 3;
      imageSize = (uint)(bpl + skip) * (uint)height;

      // 1024 == palette size (4 * 256)
      bmpHeader.fileSize     = Memory::bswap32le(54 + imageSize + 1024);
      bmpHeader.bitsPerPixel = Memory::bswap16le(8);
      bmpHeader.imageSize    = Memory::bswap32le(imageSize);

      bmpHeader.imageOffset  = Memory::bswap32le(1024 + 54);
      break;
    }
  }

  // Write file and bmp bmpHeader.
  if (stream().write((const void *)&bmpHeader, 54) != 54)
  {
    goto fail;
  }

  switch (format)
  {
    case Image::FormatARGB32:
    case Image::FormatPRGB32:
    {
      uint8_t *buffer = (uint8_t *)bufferLocal.alloc(bpl);
      if (!buffer) { err = Error::OutOfMemory; goto end; }

      // Write 32 bit BMP data
      for (uint y = 0; y != height; y++)
      {
        image.getDibArgb32_le(0, y, width, buffer);
        if (stream().write((const void *)buffer, bpl) != bpl) goto fail;
        if ((y & 15) == 0) updateProgress(y, height);
      }
      break;
    }

    case Image::FormatRGB32:
    case Image::FormatRGB24:
    {
      uint8_t* buffer = (uint8_t *)bufferLocal.alloc(bpl + skip);
      if (!buffer) { err = Error::OutOfMemory; goto end; }

      // cleanup tail
      memset(buffer + bpl, 0, skip);
      bpl += skip;

      for (int y = 0; y != height; y++)
      {
        image.getDibRgb24_le(0, y, width, buffer);
        if (stream().write(buffer, bpl) != (sysuint_t)bpl) goto fail;
        if ((y & 15) == 0) updateProgress(y, height);
      }
      break;
    }

    case Image::FormatA8:
    case Image::FormatI8:
    {
      // Palette
      if (format == Image::FormatA8)
      {
        // Write greyscale palette for A8 formats.
        uint32_t pl[256];
        for (uint i = 0; i != 256; i += 4)
        {
          pl[i] = Memory::bswap32le(i | (i << 8) | (i << 16) | (0xFF << 24));
        }
        if (stream().write((const void*)pl, 1024) != 1024) goto fail;
      }
      else
      {
        // Standard palette.
        if (stream().write(image.palette().cData(), 1024) != 1024) goto fail;
      }

      // Write 8 bit BMP data.
      for (int y = 0; y != height; y++)
      {
        if (stream().write((const void *)image.cScanline(height - 1 - y), width) != width) goto fail;
        if (stream().write((const void *)tailBytes, skip) != skip) goto fail;
        if ((y & 15) == 0) updateProgress(y, height);
      }
      break;
    }
  }

end:
  return err;

fail:
  return Error::ImageIO_WriteFailure;
}

} // ImageIO namespace
} // Fog namespace

// ============================================================================
// [CAPI]
// ============================================================================

FOG_CAPI_DECLARE Fog::ImageIO::Provider* fog_imageio_getBmpProvider(void)
{
  return new Fog::ImageIO::BmpProvider();
}
