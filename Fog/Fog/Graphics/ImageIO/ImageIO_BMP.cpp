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
#include <Fog/Graphics/Converter.h>

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
    return (_headerResult = EImageIOTruncated);
  }

  // try to match bmp mime "BM"
  if (*(uint16_t*)(_bmpFileHeader) != FOG_MAKE_UINT16_SEQ('B', 'M')) 
  { 
    return (_headerResult = EImageIOMimeNotMatch);
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
      return (_headerResult = EImageIOTruncated);
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
      return (_headerResult = EImageIOTruncated);
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
    return (_headerResult = EImageIOFormatNotSupported);
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
      return (_headerResult = EImageIOFormatNotSupported);
  }

  // check for zero dimensions
  if (areDimensionsZero())
  {
    return (_headerResult = EImageSizeIsZero);
  }

  // check for too large dimensions
  if (areDimensionsTooLarge())
  {
    return (_headerResult = EImageSizeTooLarge);
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
          return EImageIOTruncated;
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
          return EImageIOTruncated;
        }

#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
        for (i = 0; i != nColors; i++, pdestCur++) *pdestCur |= 0xFF000000;
#else
        for (i = 0; i != nColors; i++, pdestCur++) *pdestCur = Memory::bswap32(pdestCur[i]) | 0xFF000000;
#endif
      }

      _format.set(ImageFormat::I8);
      break;
    }

    // setup rgb
    case 16:
      _rMask = 0x7C00; _rShift = 10; _rLoss = 3;
      _gMask = 0x03E0; _gShift =  5; _gLoss = 3;
      _bMask = 0x001F; _bShift =  0; _bLoss = 3;

#if FOG_BYTE_ORDER == FOG_BIG_ENDIAN
      _byteSwapped16 = true
#endif
      _format.set(ImageFormat::RGB24);
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
      _format.set(ImageFormat::RGB24);
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
      _format.set(ImageFormat::XRGB32);
      break;
  }

  // Bitfields
  if (bmpCompression() == BMP_BI_BITFIELDS && 
      (depth() == 16 || depth() == 32))
  {
    uint8_t masks[12];
    int bit;

    uint rSize = 0;
    uint gSize = 0;
    uint bSize = 0;

    if (stream().read(masks, 12) != 12) 
    {
      return (_headerResult = EImageIOTruncated);
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
  if (readerDone()) return (_readerResult = EImageIONotAnimationFormat);

  // error code (default is success)
  uint32_t error = Error::Ok;

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
  if (!image.create(width(), height(), format())) goto outOfMemory;

  stride = image.stride();
  pixelsBegin = image.xFirst();
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
    /* ==== 4 BIT RLE DECOMPRESSION BEGIN ==== */
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
    /* ==== 4 BIT RLE DECOMPRESSION END ==== */

    /* ==== 4 BIT RAW BEGIN ==== */
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
    /* ==== 4 BIT RAW END ==== */
  }

  /* -------------------- Conversion from 8 bit depth -------------------- */

  else if (depth() == 8)
  {
    /* ==== 8 BIT RLE DECOMPRESSION BEGIN ==== */
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
    /* ==== 8 BIT RLE DECOMPRESSION END ==== */

    /* ==== 8 BIT RAW BEGIN ==== */
    else if (bmpCompression() == BMP_BI_RGB)
    {
      uint8_t* bufferCur;

      for (y = 0; y < height(); y++)
      {
        if (stream().read(buffer, bmpStride()) != bmpStride()) goto truncated;
        bufferCur = buffer;
        pixelsCur = pixelsBegin + (height() - y - 1) * stride;

        for (i = width(); i; i--) *pixelsCur++ = *bufferCur++;
        if ((y & 15) == 0) updateProgress(y, height());
      }
    }
    /* ==== 8 BIT RAW END ==== */
  }

  /* --------------- Conversion from 16, 24 and 32 bit depth -------------- */

  // direct loading
  else if (
    (depth() == 32 && format().id() == ImageFormat::XRGB32 && format().hasMasks(rMask(), gMask(), bMask())) ||
    (depth() == 32 && format().id() == ImageFormat::ARGB32 && format().hasMasks(rMask(), gMask(), bMask())) ||
    (depth() == 24 && format().id() == ImageFormat::RGB24  && format().hasMasks(rMask(), gMask(), bMask())) )
  {
    ulong readBytes = width() * (depth() >> 3);
    ulong tailBytes = bmpStride() - readBytes;

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
  // no direct loading - use converter
  else
  {
    Converter converter;
    Converter::Format destFormat;
    Converter::Format srcFormat;

    destFormat.depth = format().depth();
    destFormat.flags = 0;
    destFormat.rMask = format().rMask();
    destFormat.gMask = format().gMask();
    destFormat.bMask = format().bMask();
    destFormat.aMask = format().aMask();

    srcFormat.depth = depth();
    srcFormat.flags = byteSwapped16() ? Converter::ByteSwap : 0;
    srcFormat.rMask = rMask();
    srcFormat.gMask = gMask();
    srcFormat.bMask = bMask();
    srcFormat.aMask = 0x00000000;

    if (!converter.setup(destFormat, srcFormat))
    {
      error = EImageIOConverterNotAvailable;
      goto end;
    }

    for (y = 0; y < height(); y++)
    {
      if (stream().read(buffer, bmpStride()) != bmpStride()) goto truncated;
      converter.convertSpan(pixelsBegin + (height() - y - 1) * stride, 0, buffer, 0, width());
      if ((y & 15) == 0) updateProgress(y, height());
    }
  }

  // apply palette if needed
  if (depth() <= 8) image.setPalette(palette());

  goto end;

truncated:
  error = EImageIOTruncated;
  goto end;
rleError:
  error = EImageIORleError;
  goto end;
outOfMemory:
  error = Error::OutOfMemory;
  goto end;
end:
  if (error == Error::Ok) updateProgress(1.0);
  return (_readerResult = error);
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
  // TODO
  return 0;
}




















































































#if 0

// ---------------------------------------------------------------------------
// Fog_ImageIO_BMP decode
// ---------------------------------------------------------------------------

static uint Fog_ImageIO_BMP_decodeHeader(Fog_ImageIODecoder* self)
{
  // BMP file header (14 bytes) + header size (4 bytes)
  uint8_t bmpFileHeader[18];
  uint8_t bmpDataHeader[40];

  /*
    Read header
  */
  if (self->stream().seek(self->data.headerOffset, Fog::Stream::Seek_Set) == -1 ||
    self->stream().read(bmpFileHeader, 14+4) != 14+4)
  {
    return Fog_ImageError_Truncated;
  }

  if (*(uint16_t*)(bmpFileHeader) != FOG_MAKE_UINT16_SEQ('B', 'M')) 
  { 
    return Fog_ImageError_MimeNotMatch; 
  }

  uint32_t headerSize = Fog::Memory::bswap32le( *(const uint32_t *)(bmpFileHeader + 14) );

  /*
    Old BMP header (OS2)
  */
  if (headerSize == 12)
  {
    if (self->stream().read(bmpDataHeader, 12-4) != 12-4) return Fog_ImageError_Truncated;

    self->data.headerRead   = true;
    self->data.width        = Fog::Memory::bswap16le( *(const uint16_t *)(bmpDataHeader + 4  - 4) );
    self->data.height       = Fog::Memory::bswap16le( *(const uint16_t *)(bmpDataHeader + 6  - 4) );
    self->data.planes       = Fog::Memory::bswap16le( *(const uint16_t *)(bmpDataHeader + 8  - 4) );
    self->data.depth        = Fog::Memory::bswap16le( *(const uint16_t *)(bmpDataHeader + 10 - 4) );
    self->data.actualFrame = 0;
    self->data.framesCount = 1;

    return Fog_ImageError_Success;
  }

  /* 
    Win BMP header 
  */
  else if (headerSize == 40) 
  {
    if (self->stream.read(bmpDataHeader, 40-4) != 40-4) return Fog_ImageError_Truncated;

    self->data.headerRead   = true;
    self->data.width        = Fog::Memory::bswap32le( *(const uint32_t *)(bmpDataHeader +  4 - 4) );
    self->data.height       = Fog::Memory::bswap32le( *(const uint32_t *)(bmpDataHeader +  8 - 4) );
    self->data.planes       = Fog::Memory::bswap16le( *(const uint16_t *)(bmpDataHeader + 12 - 4) );
    self->data.depth        = Fog::Memory::bswap16le( *(const uint16_t *)(bmpDataHeader + 14 - 4) );
    self->data.actualFrame = 0;
    self->data.framesCount = 1;

    return Fog_ImageError_Success;
  }
  else
  {
    return Fog_ImageError_UnsupportedFormat;
  }
}

static uint Fog_ImageIO_BMP_decodeImage(Fog_ImageIODecoder* self, Fog::Image* image, Fog_ImageProcessHandler* handler, void* handlerData)
{
  if (handler && handler->isEmpty()) handler = NULL;
  #define CALL_HANDLER(y, ytotal) \
  if (handler && !((*handler)(y, ytotal, handlerData)) ) \
  { \
    error = Fog_ImageError_Terminated; \
    goto end; \
  }

  Fog::Stream& stream = self->stream;
  uint32_t error = Fog_ImageError_Success;

  // BMP file header (14 bytes) + header size (4 bytes)
  uint8_t bmpFileHeader[18];
  uint8_t bmpDataHeader[40];

  /*
    Buffer
  */
  uint8_t* pixelsBegin;
  uint8_t* pixelsCur;

  /*
    Header
  */
  uint32_t headerSize;
  uint32_t offset;
  uint32_t width;
  uint32_t height;
  uint16_t planes;
  uint16_t bitCount;
  uint32_t compression;
  uint32_t imageSize;
  uint32_t bmpStride;

  uint rMask = 0x00FF0000;
  uint gMask = 0x0000FF00;
  uint bMask = 0x000000FF;
  uint rShift = 0;
  uint gShift = 0;
  uint bShift = 0;
  uint byteSwapped16 = 0;

  /*
    Only used by converter from 16 BPP
  */
  uint rLoss = 0;
  uint gLoss = 0;
  uint bLoss = 0;

  /*
    Reader variables
  */
  uint x = 0;
  uint y = 0;
  uint i;
  uint stride;
  uint formatId = Fog::ImageFormat::XRGB32;

  uint32_t palette[256];
  Fog::MemoryBuffer<4096> bufferStorage;
  uint8_t* buffer;
  uint8_t* rleBuffer = NULL;

  /*
    Read header
  */
  if (stream.seek(self->data.headerOffset, Fog::Stream::Seek_Set) == -1) goto truncated;
  if (stream.read(bmpFileHeader, 14+4) != 14+4) goto truncated;

  if (*(uint16_t*)(bmpFileHeader) != FOG_MAKE_UINT16_SEQ('B', 'M')) 
  { 
    error = Fog_ImageError_MimeNotMatch; 
    goto end; 
  }

  offset     = Fog::Memory::bswap32le( *(const uint32_t *)(bmpFileHeader + 10) );
  headerSize = Fog::Memory::bswap32le( *(const uint32_t *)(bmpFileHeader + 14) );

  /*
    Old BMP header (OS2)
  */
  if (headerSize == 12)
  {
    if (stream.read(bmpDataHeader, 12-4) != 12-4) goto truncated;

    width       = Fog::Memory::bswap16le( *(const uint16_t *)(bmpDataHeader + 4  - 4) );
    height      = Fog::Memory::bswap16le( *(const uint16_t *)(bmpDataHeader + 6  - 4) );
    planes      = Fog::Memory::bswap16le( *(const uint16_t *)(bmpDataHeader + 8  - 4) );
    bitCount    = Fog::Memory::bswap16le( *(const uint16_t *)(bmpDataHeader + 10 - 4) );
    compression = Fog_ImageIO_BMP_BI_RGB;
  }

  /* 
    Win BMP header 
  */
  else if (headerSize == 40) 
  {
    if (stream.read(bmpDataHeader, 40-4) != 40-4) goto truncated;

    width       = Fog::Memory::bswap32le( *(const uint32_t *)(bmpDataHeader +  4 - 4) );
    height      = Fog::Memory::bswap32le( *(const uint32_t *)(bmpDataHeader +  8 - 4) );
    planes      = Fog::Memory::bswap16le( *(const uint16_t *)(bmpDataHeader + 12 - 4) );

    bitCount    = Fog::Memory::bswap16le( *(const uint16_t *)(bmpDataHeader + 14 - 4) );
    compression = Fog::Memory::bswap32le( *(const uint32_t *)(bmpDataHeader + 16 - 4) );
    imageSize   = Fog::Memory::bswap32le( *(const uint32_t *)(bmpDataHeader + 20 - 4) );
  }
  else
  {
    error = Fog_ImageError_UnsupportedFormat;
    goto end;
  }

  /*
    Size reject
  */
  if (width == 0 || height == 0) 
  { 
    error = Fog_ImageError_InvalidBounds; 
    goto end;
  }
  else if (width > Fog_Image_MaxWidth || height > Fog_Image_MaxHeight) 
  { 
    error = Fog_ImageError_BoundsTooLarge; 
    goto end;
  }

  bmpStride = (((width * bitCount + 7) >> 3) + 3) & ~3;
  buffer = (uint8_t *)bufferStorage.alloc(bmpStride);

  /*
    Setup pixel format in image 
  */
  if (bitCount < 16)
  {
    uint32_t nColors = (offset - headerSize - 14);

    if (headerSize == 12)
    {
      uint8_t palette24[768];
      uint8_t* palette24Cur = palette24;

      nColors /= 3;
      if (nColors > 256) nColors = 256;

      if (stream.read(palette24, nColors*3) != nColors*3) goto truncated;

      for (i = 0; i != nColors; i++)
      {
        palette[i] = (uint32_t)0xFF000000 | 
          (uint32_t)(palette24Cur[2] << 16) | 
          (uint32_t)(palette24Cur[1] <<  8) | 
          (uint32_t)(palette24Cur[0]);
        palette24Cur++;
      }
    }
    else
    {
      nColors /= 4;
      if (nColors > 256) nColors = 256;

      if (stream.read(palette, nColors*4) != nColors*4) goto truncated;

#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
      for (i = 0; i != nColors; i++) palette[i] |= 0xFF000000;
#else
      for (i = 0; i != nColors; i++) palette[i] = Fog::Memory::bswap32(palette[i]) | 0xFF000000;
#endif
    }

    // If there are only greys, we can use A8 instead of XRGB32.
    if (Fog_Image_isGreyPalette(palette, nColors)) 
    {
      formatId = Fog::ImageFormat::A8;
    }
  }
  else if (bitCount == 16)
  {
    rMask = 0x7C00;
    gMask = 0x03E0;
    bMask = 0x001F;
    rShift = 10;
    gShift = 5;
    bShift = 0;
    rLoss = 3;
    gLoss = 3;
    bLoss = 3;
#if FOG_BYTE_ORDER == FOG_BIG_ENDIAN
    byteSwapped16 = true;
#endif
  }
  else if (bitCount == 24)
  {
#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
    rMask = 0x00FF0000;
    gMask = 0x0000FF00;
    bMask = 0x000000FF;
    rShift = 16;
    gShift = 8;
    bShift = 0;
#else
    rMask = 0x000000FF;
    gMask = 0x0000FF00;
    bMask = 0x00FF0000;
    rShift = 0;
    gShift = 8;
    bShift = 16;
#endif // FOG_BYTE_ORDER
  }
  else if (bitCount == 32)
  {
#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
    rMask = 0x00FF0000;
    gMask = 0x0000FF00;
    bMask = 0x000000FF;
    rShift = 16;
    gShift = 8;
    bShift = 0;
#else
    rMask = 0x0000FF00;
    gMask = 0x00FF0000;
    bMask = 0xFF000000;
    rShift = 8;
    gShift = 16;
    bShift = 24;
#endif // FOG_BYTE_ORDER
  }
  else
  {
    error = Fog_ImageError_UnsupportedFormat;
    goto end;
  }

  // BI_BITFIELDS compression must set correct masks
  if (compression == Fog_ImageIO_BMP_BI_BITFIELDS && (bitCount == 16 || bitCount == 32))
  {
    uint8_t masks[12];
    int bit;

    uint rSize = 0;
    uint gSize = 0;
    uint bSize = 0;

    if (stream.read(masks, 12) != 12) goto truncated;

    rMask = Fog::Memory::bswap32le( *((const uint32_t *)(masks + 0)) );
    gMask = Fog::Memory::bswap32le( *((const uint32_t *)(masks + 4)) );
    bMask = Fog::Memory::bswap32le( *((const uint32_t *)(masks + 8)) );

    for (bit = (int)(bitCount) - 1; bit >= 0; bit--)
    {
      if (bMask & (1 << bit)) { bShift = bit; bSize++; }
      if (gMask & (1 << bit)) { gShift = bit; gSize++; }
      if (rMask & (1 << bit)) { rShift = bit; rSize++; }
    }

    rLoss = 8 - rSize;
    gLoss = 8 - gSize;
    bLoss = 8 - bSize;
  }

  /* Create image */
  if (!image->resize(width, height, formatId))
  {
    error = Fog_ImageError_OutOfMemory;
    goto end;
  }

  stride = image->_d->_stride;
  pixelsBegin = (uint8_t *)image->_d->_base;
  pixelsCur = pixelsBegin + stride * (height - 1);

  /* -------------------- Conversion from 1 bit depth -------------------- */

  if (bitCount == 1)
  {
    if (compression == Fog_ImageIO_BMP_BI_RGB)
    {
      uint8_t* bufferCur;
      uint32_t b;

      /* ==== 1 BIT TO 32 BIT BEGIN ==== */
      if (formatId != Fog::ImageFormat::A8)
      {
        for (y = 0; y != height; y++)
        {
          if (stream.read(buffer, bmpStride) != bmpStride) goto truncated;
          bufferCur = buffer;
          pixelsCur = pixelsBegin + (height - y - 1) * stride;

          for (i = width; i >= 8; i -= 8, pixelsCur += 32, bufferCur++)
          {
            b = (uint32_t)(*bufferCur);
            ((uint32_t *)pixelsCur)[0] = palette[uint8_t((b >> 7) & 1)];
            ((uint32_t *)pixelsCur)[1] = palette[uint8_t((b >> 6) & 1)];
            ((uint32_t *)pixelsCur)[2] = palette[uint8_t((b >> 5) & 1)];
            ((uint32_t *)pixelsCur)[3] = palette[uint8_t((b >> 4) & 1)];
            ((uint32_t *)pixelsCur)[4] = palette[uint8_t((b >> 3) & 1)];
            ((uint32_t *)pixelsCur)[5] = palette[uint8_t((b >> 2) & 1)];
            ((uint32_t *)pixelsCur)[6] = palette[uint8_t((b >> 1) & 1)];
            ((uint32_t *)pixelsCur)[7] = palette[uint8_t((b     ) & 1)];
          }
          if (i)
          {
            b = (uint32_t)(*bufferCur);
            for (; i; i--, pixelsCur += 4, b <<= 1)
            {
              ((uint32_t *)pixelsCur)[0] = palette[uint8_t((b >> 7) & 1)];
            }
          }
          CALL_HANDLER(y, height);
        }
      }
      /* ==== 1 BIT TO 32 BIT END ==== */
      
      /* ==== 1 BIT TO 8 BIT BEGIN ==== */
      else
      {
        for (y = 0; y != height; y++)
        {
          if (stream.read(buffer, bmpStride) != bmpStride) goto truncated;
          bufferCur = buffer;
          pixelsCur = pixelsBegin + (height - y - 1) * stride;

          for (i = width; i >= 8; i -= 8, pixelsCur += 32, bufferCur++)
          {
            b = (uint32_t)(*bufferCur);
            ((uint8_t *)pixelsCur)[0] = palette[uint8_t((b >> 7) & 1)] & 0xFF;
            ((uint8_t *)pixelsCur)[1] = palette[uint8_t((b >> 6) & 1)] & 0xFF;
            ((uint8_t *)pixelsCur)[2] = palette[uint8_t((b >> 5) & 1)] & 0xFF;
            ((uint8_t *)pixelsCur)[3] = palette[uint8_t((b >> 4) & 1)] & 0xFF;
            ((uint8_t *)pixelsCur)[4] = palette[uint8_t((b >> 3) & 1)] & 0xFF;
            ((uint8_t *)pixelsCur)[5] = palette[uint8_t((b >> 2) & 1)] & 0xFF;
            ((uint8_t *)pixelsCur)[6] = palette[uint8_t((b >> 1) & 1)] & 0xFF;
            ((uint8_t *)pixelsCur)[7] = palette[uint8_t((b     ) & 1)] & 0xFF;
          }
          if (i)
          {
            b = (uint32_t)(*bufferCur);
            for (; i; i--, pixelsCur += 4, b <<= 1)
            {
              ((uint8_t *)pixelsCur)[0] = palette[uint8_t((b >> 7) & 1)] & 0xFF;
            }
          }
          CALL_HANDLER(y, height);
        }
      }
      /* ==== 1 BIT TO 8 BIT END ==== */
    }
  }

  /* -------------------- Conversion from 4 bit depth -------------------- */
  
  else if (bitCount == 4)
  {
    /* ==== 4 BIT RLE DECOMPRESSION BEGIN ==== */
    if (compression == Fog_ImageIO_BMP_BI_RLE4)
    {
      uint8_t* rleCur;
      uint8_t* rleEnd;
      uint b0;
      uint b1;

      if ((rleBuffer = (uint8_t *)Fog::Memory::alloc(imageSize)) == NULL)
      {
        error = Fog_ImageError_OutOfMemory;
        goto end;
      }
      if (stream.read(rleBuffer, imageSize) != imageSize)
      {
        goto truncated;
      }

      rleCur = rleBuffer;
      rleEnd = rleBuffer + imageSize;

BI_RLE_4_BEGIN:
      if (x >= width || y >= height) goto rleError;

      pixelsCur = pixelsBegin + (height - y - 1) * stride;
      CALL_HANDLER(y, height)

      for (;;)
      {
        if (rleCur + 2 > rleEnd) goto truncated;
        b0 = uint(rleCur[0]);
        b1 = uint(rleCur[1]);
        rleCur += 2;

        if (b0)
        {
          /*
            b0 = Length
            b1 = 2 Colors
          */
          uint clr1 = palette[b1 >> 4];
          uint clr2 = palette[b1 & 0xF];

          i = b0;
          if (i > width - x) goto rleError;

          /* ==== 4 TO 32 BIT BEGIN ==== */
          if (formatId != Fog::ImageFormat::A8)
          {
            while (i >= 2)
            {
              ((uint32_t *)pixelsCur)[0] = clr1;
              ((uint32_t *)pixelsCur)[1] = clr2;
              pixelsCur += 8; i -= 2;
            }
            if (i) { *(uint32_t *)pixelsCur = clr1; pixelsCur += 4; }
          }
          /* ==== 4 TO 32 BIT END ==== */

          /* ==== 4 TO 8 BIT BEGIN ==== */
          else
          {
            clr1 &= 0xFF;
            clr2 &= 0xFF;

            while (i >= 2)
            {
              ((uint8_t *)pixelsCur)[0] = clr1;
              ((uint8_t *)pixelsCur)[1] = clr2;
              pixelsCur += 2; i -= 2;
            }
            if (i) { *(uint8_t *)pixelsCur = clr1; pixelsCur += 4; }
          }
          /* ==== 4 TO 8 BIT END ==== */
        }
        else
        {
          switch (b1)
          {
            case Fog_ImageIO_BMP_RLE_NEXT_LINE: x = 0; y++; goto BI_RLE_4_BEGIN;
            case Fog_ImageIO_BMP_RLE_END: goto end;
            case Fog_ImageIO_BMP_RLE_MOVE:
              if (rleCur + 2 > rleEnd) goto truncated;
              x += rleCur[0];
              y += rleCur[1]; rleCur += 2;
              goto BI_RLE_4_BEGIN;
            /* FILL BITS (b1 == length) */
            default:
              i = b1;

              if (i > width - x) goto rleError;
              if (rleCur + (i >> 1) + (i & 1) > rleEnd) goto truncated;

              x += i;

              /* ==== 4 TO 32 BIT BEGIN ==== */
              if (formatId != Fog::ImageFormat::A8)
              {
                while (i >= 2)
                {
                  b0 = uint(*rleCur++);
                  ((uint32_t *)pixelsCur)[0] = palette[b0  >> 4];
                  ((uint32_t *)pixelsCur)[1] = palette[b0 & 0xF]; pixelsCur += 8;
                  i -= 2;
                }
                if (i) {
                  ((uint32_t *)pixelsCur)[0] = palette[uint(*rleCur++) >> 4]; pixelsCur += 4;
                }
              }
              /* ==== 4 TO 32 BIT END ==== */

              /* ==== 4 TO 8 BIT BEGIN ==== */
              else
              {
                while (i >= 2)
                {
                  b0 = uint(*rleCur++);
                  ((uint8_t *)pixelsCur)[0] = palette[b0  >> 4] & 0xFF;
                  ((uint8_t *)pixelsCur)[1] = palette[b0 & 0xF] & 0xFF; pixelsCur += 2;
                  i -= 2;
                }
                if (i)
                {
                  ((uint8_t *)pixelsCur)[0] = palette[uint(*rleCur++) >> 4] & 0xFF; pixelsCur += 1;
                }
              }
              /* ==== 4 TO 8 BIT END ==== */

              if ((b1 & 3) == 1) rleCur += 2;
              else if ((b1 & 3) == 2) rleCur++;

              break;
          }
        }
      }
    }
    /* ==== 4 BIT RLE DECOMPRESSION END ==== */
    
    /* ==== 4 BIT RAW BEGIN ==== */
    else if (compression == Fog_ImageIO_BMP_BI_RGB)
    {
      uint8_t* bufferCur;
      uint b;

      /* ==== 4 BIT TO 32 BIT BEGIN ==== */
      if (formatId != Fog::ImageFormat::A8)
      {
        for (y = 0; y != height; y++)
        {
          if (stream.read(buffer, bmpStride) != bmpStride) goto truncated;
          bufferCur = buffer;
          pixelsCur = pixelsBegin + (height - y - 1) * stride;

          for (x = 0; x + 2 <= width; x += 2, pixelsCur += 8)
          {
            b = (uint)(*bufferCur++);
            ((uint32_t *)pixelsCur)[0] = palette[b  >> 4];
            ((uint32_t *)pixelsCur)[1] = palette[b & 0xF];
          }
          if (x < width)
          {
            ((uint32_t *)pixelsCur)[0] = palette[uint(*bufferCur++) >> 4];
          }
          CALL_HANDLER(y, height)
        }
      }
      /* ==== 4 BIT TO 32 BIT END ==== */
      
      /* ==== 4 BIT TO 8 BIT BEGIN ==== */
      else
      {
        for (y = 0; y != height; y++)
        {
          if (stream.read(buffer, bmpStride) != bmpStride) goto truncated;
          bufferCur = buffer;
          pixelsCur = pixelsBegin + (height - y - 1) * stride;

          for (x = 0; x + 2 <= width; x += 2, pixelsCur += 2)
          {
            b = (uint)(*bufferCur++);
            ((uint8_t *)pixelsCur)[0] = palette[b  >> 4] & 0xFF;
            ((uint8_t *)pixelsCur)[1] = palette[b & 0xF] & 0xFF;
          }
          if (x < width)
          {
            ((uint8_t *)pixelsCur)[0] = palette[(uint)(*bufferCur) >> 4] & 0xFF;
          }
          CALL_HANDLER(y, height)
        }
      }
      /* ==== 4 BIT TO 8 BIT END ==== */
    }
    /* ==== 4 BIT RAW END ==== */
  }
  
  /* -------------------- Conversion from 8 bit depth -------------------- */
  
  else if (bitCount == 8)
  {
    /* ==== 8 BIT RLE DECOMPRESSION BEGIN ==== */
    if (compression == Fog_ImageIO_BMP_BI_RLE8)
    {
      uint8_t* rleCur;
      uint8_t* rleEnd;
      uint b0;
      uint b1;

      if ((rleBuffer = (uint8_t *)Fog::Memory::alloc(imageSize)) == NULL)
      {
        error = Fog_ImageError_OutOfMemory;
        goto end;
      }
      if (stream.read(rleBuffer, imageSize) != imageSize)
      {
        goto truncated;
      }

      rleCur = rleBuffer;
      rleEnd = rleBuffer + imageSize;

BI_RLE_8_BEGIN:
      if (x >= width || y >= height) goto rleError;

      pixelsCur = pixelsBegin + (height - y - 1) * stride;
      CALL_HANDLER(y, height)

      for (;;)
      {
        if (rleCur + 2 > rleEnd) goto truncated;
        b0 = uint(rleCur[0]);
        b1 = uint(rleCur[1]);
        rleCur += 2;

        if (b0)
        {
          // b0 = Length
          // b1 = Color
          i = b0;
          if (i > width - x) goto rleError;

          /* ==== 8 BIT TO 32 BIT BEGIN ==== */
          if (formatId != Fog::ImageFormat::A8)
          {
            while (i--)
            {
              *(uint32_t *)pixelsCur = palette[b1];
              pixelsCur += 4;
            }
          }
          /* ==== 8 BIT TO 32 BIT END ==== */

          /* ==== 8 BIT TO 8 BIT BEGIN ==== */
          else
          {
            while (i--) *pixelsCur++ = palette[b1] & 0xFF;
          }
          /* ==== 8 BIT TO 8 BIT END ==== */
        }
        else 
        {
          /* b1 = Chunk type */
          switch (b1)
          {
            case Fog_ImageIO_BMP_RLE_NEXT_LINE: x = 0; y++; goto BI_RLE_8_BEGIN;
            case Fog_ImageIO_BMP_RLE_END: goto end;
            case Fog_ImageIO_BMP_RLE_MOVE:
              if (rleCur + 2 > rleEnd) goto truncated;
              x += rleCur[0];
              y += rleCur[1]; rleCur += 2;
              goto BI_RLE_8_BEGIN;
            /* FILL BITS (b1 == length) */
            default:
              i = b1;

              if (i > width - x) goto rleError;
              if (rleCur + i > rleEnd) goto truncated;

              /* 8 BIT TO 32 BIT BEGIN */
              if (formatId != Fog::ImageFormat::A8)
              {
                while (i--)
                {
                  *(uint32_t *)pixelsCur = palette[*rleCur++];
                  pixelsCur += 4;
                }
              }
              /* 8 BIT TO 32 BIT END */

              /* 8 BIT TO 8 BIT BEGIN */
              else
              {
                while (i--) *pixelsCur++ = palette[*rleCur++] & 0xFF;
              }
              /* 8 BIT TO 8 BIT END */

              if (b1 & 1) rleCur++;
              x += i;
              break;
          }
        }
      }
      CALL_HANDLER(y, height);
    }
    /* ==== 8 BIT RLE DECOMPRESSION END ==== */

    /* ==== 8 BIT RAW BEGIN ==== */
    else if (compression == Fog_ImageIO_BMP_BI_RGB)
    {
      uint8_t* bufferCur;
      /* ==== 8 BIT TO 32 BIT BEGIN ==== */
      if (formatId != Fog::ImageFormat::A8)
      {
        for (y = 0; y < height; y++)
        {
          if (stream.read(buffer, bmpStride) != bmpStride) goto truncated;
          bufferCur = buffer;
          pixelsCur = pixelsBegin + (height - y - 1) * stride;

          for (i = width; i; i--, pixelsCur += 4) 
          {
            *(uint32_t *)pixelsCur = palette[*bufferCur++];
          }
          CALL_HANDLER(y, height)
        }
      }
      /* ==== 8 BIT TO 32 BIT END ==== */

      /* ==== 8 BIT TO 8 BIT BEGIN ==== */
      else
      {
        for (y = 0; y < height; y++)
        {
          if (stream.read(buffer, bmpStride) != bmpStride) goto truncated;
          bufferCur = buffer;
          pixelsCur = pixelsBegin + (height - y - 1) * stride;

          for (i = width; i; i--) 
          {
            *pixelsCur++ = palette[*bufferCur++] & 0xFF;
          }
          CALL_HANDLER(y, height)
        }
      }
      /* ==== 8 BIT TO 8 BIT END ==== */
    }
    /* ==== 8 BIT RAW END ==== */
  }

  /* --------------- Conversion from 16, 24 and 32 bit depth -------------- */
  // direct loading
  else if (
    (bitCount == 32 && formatId == Fog::ImageFormat::XRGB32 && Fog::ImageFormat::instance(Fog::ImageFormat::XRGB32).hasMasks(rMask, gMask, bMask)) ||
    (bitCount == 32 && formatId == Fog::ImageFormat::ARGB32 && Fog::ImageFormat::instance(Fog::ImageFormat::ARGB32).hasMasks(rMask, gMask, bMask)) ||
    (bitCount == 24 && formatId == Fog::ImageFormat::RGB24  && Fog::ImageFormat::instance(Fog::ImageFormat::RGB24 ).hasMasks(rMask, gMask, bMask)) )
  {
    ulong readBytes = width * (bitCount >> 3);
    ulong tailBytes = bmpStride - readBytes;

    for (y = 0; y < height; y++)
    {
      pixelsCur = pixelsBegin + (height - y - 1) * stride;
      if (stream.read(pixelsCur, readBytes) != readBytes ||
        stream.read(buffer, tailBytes) != tailBytes)
      {
        goto truncated;
      }
      CALL_HANDLER(y, height)
    }
  }
  // no direct loading possible, so use pixel converter to convert pixels
  // from bitmap into Fog::Image
  else
  {
    Fog::Converter converter;
    Fog::ConverterDescription targetDesc;
    Fog::ConverterDescription sourceDesc;

    converter.dither = 1;

    targetDesc.depth = Fog::ImageFormat::instance(formatId).depth();
    targetDesc.rMask = Fog::ImageFormat::instance(formatId).rMask();
    targetDesc.gMask = Fog::ImageFormat::instance(formatId).gMask();
    targetDesc.bMask = Fog::ImageFormat::instance(formatId).bMask();
    targetDesc.aMask = Fog::ImageFormat::instance(formatId).aMask();
    targetDesc.byteSwapped = false;

    sourceDesc.depth = bitCount;
    sourceDesc.rMask = rMask;
    sourceDesc.gMask = gMask;
    sourceDesc.bMask = bMask;
    sourceDesc.aMask = 0x00000000;
    sourceDesc.byteSwapped = byteSwapped16;

    if (!converter.init(targetDesc, sourceDesc))
    {
      error = Fog_ImageError_CantCreateConverter;
      goto end;
    }

    for (y = 0; y < height; y++)
    {
      if (stream.read(buffer, bmpStride) != bmpStride) goto truncated;
      converter.convertSpan(pixelsBegin + (height - y - 1) * stride, 0, buffer, 0, width);
      CALL_HANDLER(y, height)
    }
  }

end:
  if (rleBuffer) Fog::Memory::free(rleBuffer);
  CALL_HANDLER(height, height)
  return error;

truncated:
  error = Fog_ImageError_Truncated;
  goto end;

rleError:
  error = Fog_ImageError_RLEDecompressionFail;
  goto end;

  #undef CALL_HANDLER
}

FOG_CVAR_DECLARE const Fog_ImageIODecoderFuncs Fog_ImageIO_BMP_decoderFuncs =
{
  Fog_ImageIO_BMP_decodeHeader,
  Fog_ImageIO_BMP_decodeImage,
  Fog_ImageIODecoder_closeGeneric
};

// ---------------------------------------------------------------------------
// Fog_ImageIO_BMP encode
// ---------------------------------------------------------------------------

static uint Fog_ImageIO_BMP_encodeImage(Fog_ImageIOEncoder* self, Fog::Image* image, Fog_ImageProcessHandler* handler, void* handlerData)
{
  static const uint8_t tailBytes[4] = { 0, 0, 0, 0 };

  if (handler && handler->isEmpty()) handler = NULL;
  #define CALL_HANDLER(y, ytotal) \
  if (handler && !((*handler)(y, ytotal, handlerData)) ) \
  { \
    error = Fog_ImageError_Terminated; \
    goto end; \
  }

  Fog::Stream& stream = self->stream;
  uint error = Fog_ImageError_Success;

  Fog_ImageIO_BMPHeader bmpHeader;

  Fog::ImageData* d = image->_d;
  uint width = d->_width;
  uint height = d->_height;
  long stride = d->_stride;
  uint32_t formatId = d->_formatId;
  uint bpl;
  uint skip;

  Fog::MemoryBuffer<4096> bufferLocal;

  if (!width || !height)
  {
    error = Fog_ImageError_InvalidBounds;
    goto end;
  }

  // We can write with alpha channel or without it

  // file bmpHeader
  bmpHeader.magic = FOG_MAKE_UINT16_SEQ('B', 'M');
  bmpHeader.reserved1 = 0x0000;
  bmpHeader.reserved2 = 0x0000;
  bmpHeader.imageOffset = Fog::Memory::bswap32le(54);

  // BMP bmpHeader
  bmpHeader.headerSize = Fog::Memory::bswap32le(40);
  bmpHeader.width = Fog::Memory::bswap32le(width);
  bmpHeader.height = Fog::Memory::bswap32le(height);
  bmpHeader.planes = Fog::Memory::bswap16le(1);
  bmpHeader.compression = Fog::Memory::bswap32le(Fog_ImageIO_BMP_BI_RGB);
  bmpHeader.reserved3 = 0x00000000;
  bmpHeader.reserved4 = 0x00000000;
  bmpHeader.reserved5 = 0x00000000;
  bmpHeader.reserved6 = 0x00000000;

  switch (formatId)
  {
    case Fog::ImageFormat::ARGB32:
    case Fog::ImageFormat::PRGB32:
    {
      uint imageSize;

      bpl = width * 4;
      skip = 0;
      imageSize = bpl * height;

      bmpHeader.fileSize = Fog::Memory::bswap32le(54 + imageSize);
      bmpHeader.bitsPerPixel = Fog::Memory::bswap16le(32);
      bmpHeader.imageSize = Fog::Memory::bswap32le(imageSize);

      break;
    }

    case Fog::ImageFormat::XRGB32:
    case Fog::ImageFormat::RGB24:
    {
      uint imageSize;

      bpl = width * 3;
      skip = (4 - (bpl & 3)) & 3;
      imageSize = (bpl + skip) * height;

      bmpHeader.fileSize = Fog::Memory::bswap32le(54 + imageSize);
      bmpHeader.bitsPerPixel = Fog::Memory::bswap16le(24);
      bmpHeader.imageSize = Fog::Memory::bswap32le(imageSize);

      break;
    }
#if 0
    // not supported now, but may be image io encoder choice ?
    case Fog::ImageFormat::RGB16_5550:
    case Fog::ImageFormat::RGB16_5650:
    {
      uint imageSize;

      bpl = width << 1;
      skip = (bpl) & 3;
      imageSize = (bpl + skip) * height;

      bmpHeader.fileSize = Fog::Memory::bswap32le(54 + imageSize);
      bmpHeader.bitsPerPixel = Fog::Memory::bswap16le(16);
      bmpHeader.imageSize = Fog::Memory::bswap32le(imageSize);
      bmpHeader.compression = Fog_ImageIO_BMP_BI_BITFIELDS;
      bmpHeader.fileSize += 12;
      bmpHeader.imageOffset += 12;

      break;
    }
#endif
    case Fog::ImageFormat::A8:
    {
      uint imageSize;

      bpl = width;
      skip = (4 - (bpl & 3)) & 3;
      imageSize = (bpl + skip) * height;

      // 1024 == palette size (4 * 256)
      bmpHeader.fileSize = Fog::Memory::bswap32le(54 + imageSize + 1024);
      bmpHeader.bitsPerPixel = Fog::Memory::bswap16le(8);
      bmpHeader.imageSize = Fog::Memory::bswap32le(imageSize);

      bmpHeader.imageOffset = Fog::Memory::bswap32le(1024 + 54);
      break;
    }

    case Fog::ImageFormat::A1:
    {
      uint imageSize;

      bpl = (width + 7) / 8;
      skip = (4 - (bpl & 3)) & 3;
      imageSize = (bpl + skip) * height;

      // 8 == palette size (4 * 2)
      bmpHeader.fileSize = Fog::Memory::bswap32le(54 + imageSize + 8);
      bmpHeader.bitsPerPixel = Fog::Memory::bswap16le(1);
      bmpHeader.imageSize = Fog::Memory::bswap32le(imageSize);

      bmpHeader.imageOffset = Fog::Memory::bswap32le(8 + 54);
      break;
    }
  }

  // Write file and bmp bmpHeader
  if (stream.write((const void *)&bmpHeader, 54) != 54)
  {
    goto fail;
  }

  switch (formatId)
  {
    case Fog::ImageFormat::ARGB32:
    {
      const uint8_t* dataBase = (const uint8_t *)d->_base;
      const uint8_t* dataCur;

#if FOG_BYTE_ORDER == FOG_BIG_ENDIAN
      uint8_t *buffer = (uint8_t *)bufferLocal.alloc(bpl);
      uint8_t *bufferCur;
      uint x;
#endif // FOG_BYTE_ORDER
      uint y;

      // Write 32 bit BMP data
      for (y = 0; y != height; y++)
      {
        dataCur = dataBase + (height - 1 - y) * stride;
#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
        if (stream.write((const void *)dataCur, bpl) != bpl) goto fail;
#else // FOG_BYTE_ORDER == FOG_BIG_ENDIAN
        for (x = 0, bufferCur = buffer; x != width; x++, bufferCur += 4, dataCur += 4)
        {
          *(uint32_t *)bufferCur = Fog::Memory::bswap32(*(const uint32_t *)dataCur);
        }
        if (stream.write((const void *)buffer, bpl) != bpl) goto fail;
#endif // FOG_BYTE_ORDER
        CALL_HANDLER(y, height)
      }
      break;
    }

    case Fog::ImageFormat::XRGB32:
#if FOG_BYTE_ORDER == FOG_BIG_ENDIAN
    case Fog::ImageFormat::RGB24:
#endif // FOG_BYTE_ORDER
    {
      const uint8_t* dataBase = (const uint8_t *)d->_base;
      uint8_t* buffer = (uint8_t *)bufferLocal.alloc(bpl + skip);

      uint y;

      Fog::Converter converter;
#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
      if (!converter.init(Fog::Converter::RGB24, formatId))
#else // FOG_BYTE_ORDER == FOG_BIG_ENDIAN
      if (!converter.init(Fog::Converter::BGR24, formatId))
#endif // FOG_BYTE_ORDER
      {
        error = Fog_ImageError_CantCreateConverter;
        goto end;
      }

      // cleanup tail
      memset(buffer + bpl, 0, skip);

      bpl += skip;

      for (y = 0; y != height; y++)
      {
        converter.convertSpan(buffer, 0, dataBase + (height - 1 - y) * stride, 0, width);
        if (stream.write(buffer, bpl) != (sysuint_t)bpl) goto fail;
        CALL_HANDLER(y, height)
      }
      break;
    }

#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
    case Fog::ImageFormat::RGB24:
#endif // FOG_BYTE_ORDER
    {
      const uint8_t* dataBase = (const uint8_t *)d->_base;
      uint y;

      for (y = 0; y != height; y++)
      {
        if (stream.write(dataBase + (height - 1 - y) * stride, bpl) != bpl) goto fail;
        if (stream.write(tailBytes, skip) != skip) goto fail;

        CALL_HANDLER(y, height)
      }
      break;
    }
#if 0
    // now supported now
    case Fog::ImageFormat::RGB16_5550:
    case Fog::ImageFormat::RGB16_5650:
    {
      const uint8_t* dataBase = (const uint8_t *)d->_base;
      const uint8_t* dataPtr;
      uint y;
#if FOG_BYTE_ORDER != FOG_LITTLE_ENDIAN
      uint8_t* buffer = (uint8_t *)bufferLocal.alloc(bpl + skip);
      uint x;
#endif

      // write bitfields
      {
        uint32_t bitfieldsMask[3];

        if (formatId == Fog::ImageFormat::RGB16_5650)
        {
          bitfieldsMask[0] = Fog::Memory::bswap32le(0x0000F800);
          bitfieldsMask[1] = Fog::Memory::bswap32le(0x000007E0);
          bitfieldsMask[2] = Fog::Memory::bswap32le(0x0000001F);
        }
        else
        {
          bitfieldsMask[0] = Fog::Memory::bswap32le(0x00007C00);
          bitfieldsMask[1] = Fog::Memory::bswap32le(0x000003E0);
          bitfieldsMask[2] = Fog::Memory::bswap32le(0x0000001F);
        }

        if (stream.write(bitfieldsMask, 12) != 12) goto fail;
      }

      for (y = 0; y != height; y++)
      {
        dataPtr = dataBase + (height - 1 - y) * stride;
#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
        if (stream.write(dataPtr, bpl) != bpl) goto fail;
#else
        for (x = 0; x != width; x++)
        {
          ((uint16 *)buffer)[x] = Fog::Memory::bswap16(((uint16*)dataPtr)[x]);
        }
        if (stream.write(buffer, bpl) != bpl) goto fail;
#endif
        if (stream.write(tailBytes, skip) != skip) goto fail;

        CALL_HANDLER(y, height)
      }
      break;
    }
#endif
    case Fog::ImageFormat::A8:
    {
      const uint8_t *dataBase = (const uint8_t *)d->_base;
      const uint8_t *dataPtr;
      
      uint y;

      // write greyscale palette for A8 formats
      {
        uint8_t pl[1024];
        uint i, j;

        for (i = 0, j = 0; i != 1024; i += 4, j++)
        {
          pl[i+0] = j;
          pl[i+1] = j;
          pl[i+2] = j;
          pl[i+3] = 0xFF;
        }
        if (stream.write((const void*)pl, 1024) != 1024) goto fail;
      }

      // Write 8 bit BMP data
      for (y = 0; y != height; y++)
      {
        dataPtr = dataBase + (height - 1 - y) * stride;
        if (stream.write((const void *)dataPtr, width) != width) goto fail;
        if (stream.write((const void *)tailBytes, skip) != skip) goto fail;
        CALL_HANDLER(y, height)
      }
      break;
    }

    case Fog::ImageFormat::A1:
    {
      const uint8_t *dataBase = (const uint8_t *)d->_base;
      const uint8_t *dataPtr;
      
      uint y;

      // write palette
      static const uint8_t pl[8] = { 0, 0, 0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
      if (stream.write((const void*)pl, 8) != 8) goto fail;

      // Write 1 bit BMP data
      for (y = 0; y != height; y++)
      {
        dataPtr = dataBase + (height - 1 - y) * stride;
        if (stream.write((const void *)dataPtr, bpl) != bpl) goto fail;
        if (stream.write((const void *)tailBytes, skip) != skip) goto fail;
        CALL_HANDLER(y, height)
      }
      break;
    }
  }

end:
  return error;

fail:
  return Fog_ImageError_StreamError;

  #undef CALL_HANDLER
}

FOG_CVAR_DECLARE const Fog_ImageIOEncoderFuncs Fog_ImageIO_BMP_encoderFuncs = 
{
  Fog_ImageIO_BMP_encodeImage,
  Fog_ImageIOEncoder_closeGeneric
};




#endif

} // ImageIO namespace
} // Fog namespace

// ============================================================================
// [CAPI]
// ============================================================================

FOG_CAPI_DECLARE Fog::ImageIO::Provider* fog_imageio_getBmpProvider(void)
{
  return new Fog::ImageIO::BmpProvider();
}
