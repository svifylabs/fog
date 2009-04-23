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
#include <Fog/Graphics/Converter.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/ImageIO/ImageIO_PCX.h>

namespace Fog {
namespace ImageIO {

// ============================================================================
// [Fog::ImageIO::PcxProvider]
// ============================================================================

struct PcxProvider : public Provider
{
  PcxProvider();
  virtual ~PcxProvider();

  virtual uint32_t check(const void* mem, sysuint_t length);
  virtual EncoderDevice* createEncoder();
  virtual DecoderDevice* createDecoder();
};

PcxProvider::PcxProvider()
{
  // features
  _features.decoder = true;
  _features.encoder = true;

  _features.mono = true;
  _features.pal1 = true;
  _features.pal4 = true;
  _features.pal8 = true;
  _features.rgb24 = true;
  _features.argb32 = true;
  _features.rle4 = true;
  _features.rle8 = true;

  _features.rgbAlpha = true;

  // name
  _name = fog_strings->get(STR_GRAPHICS_PCX);

  // extensions
  _extensions.reserve(1);
  _extensions.append(fog_strings->get(STR_GRAPHICS_pcx));
}

PcxProvider::~PcxProvider()
{
}

uint32_t PcxProvider::check(const void* mem, sysuint_t length)
{
  if (length == 0) return 0;

  const uint8_t* m = (const uint8_t*)mem;

  if (length >= 3)
  {
    // [0] == manufacturer (0x0A)
    // [1] == version (0x05)
    // [2] == encoding (0x01)
    if (m[0] == 0x0A && m[1] <= 0x05 && m[2] == 0x01) return 90;
  }

  return 0;
}

EncoderDevice* PcxProvider::createEncoder()
{
  return new PcxEncoderDevice();
}

DecoderDevice* PcxProvider::createDecoder()
{
  return new PcxDecoderDevice();
}

// ============================================================================
// [Fog::ImageIO::PCX utils]
// ============================================================================

static void PCX_byteSwapHeader(PcxHeader* h)
{
#if FOG_BYTE_ORDER == FOG_BIG_ENDIAN
  h->xMin          = Memory::bswap16(h->xMin);
  h->yMin          = Memory::bswap16(h->yMin);
  h->xMax          = Memory::bswap16(h->xMax);
  h->yMax          = Memory::bswap16(h->yMax);
  h->horizontalDPI = Memory::bswap16(h->horizontalDPI);
  h->verticalDPI   = Memory::bswap16(h->verticalDPI);
  h->bytesPerLine  = Memory::bswap16(h->bytesPerLine);
  h->paletteInfo   = Memory::bswap16(h->paletteInfo);
  h->hScreenSize   = Memory::bswap16(h->hScreenSize);
  h->vScreenSize   = Memory::bswap16(h->vScreenSize);
#endif
}

// windows did not define strnlen() ?
static uint PCX_strnlen(const uint8_t* str, uint maxlen)
{
  uint n = 0;
  while (n < maxlen && str[n]) n++;
  return n;
}

#define PCX_Strict 1

// decodes scanline compressed by PCX-RLE.
// Returns 0 on success, otherwise error code.
static uint PCX_decodeScanline(
  uint8_t* dest,
  const uint8_t** src, const uint8_t* end, 
  sysuint_t length, sysuint_t ignore, sysuint_t increment)
{
  const uint8_t* cur = *src;
  sysuint_t left = length;
  sysuint_t error = EImageIOTruncated;

  // Decode RLE scanline and write it to 'dest'
  while (left) 
  {
    if (cur == end) goto fail;
    uint8_t b = *cur++;

    if (b >= 192)
    {
      uint count = b & 0x3F;
      uint extra = 0;

      if (cur == end) goto fail;
      b = *cur++;

      if (left < count) 
      {
        extra = count - left;
        count = left;
      }

      left -= count;

      while (count--) 
      {
        *dest = b;
        dest += increment;
      }

      if (extra)
      {
        // This should happen only if 'ignore' is not zero
        if (extra <= ignore)
        {
          ignore -= extra;
        }
        else
        {
          ignore = 0;
          error = EImageIORleError;

          if (PCX_Strict) goto fail;
        }
      }
    }
    else
    {
      *dest = b;
      dest += increment;
      left--;
    }
  }

  // Decode RLE scanline and discard it
  while (ignore)
  {
    if (cur == end) goto fail;
    uint8_t b = *cur++;

    if (b >= 192)
    {
      uint count = b & 0x3F;

      if (cur == end) goto fail;
      b = *cur++;

      if (ignore < count)
      {
        ignore = 0;
        error = EImageIORleError;
        if (PCX_Strict) goto fail;
      }
      else
        ignore -= count;
    }
    else
      ignore--;
  }

  error = Error::Ok;

fail:
  *src = cur;
  return error;
}

// Encode scanline using PCX_RLE compression
static bool PCX_encodeScanline(
  Stream& stream,
  uint8_t* buffer, const uint8_t* dataCur,
  sysuint_t length, sysuint_t alignment, sysuint_t increment)
{
  uint8_t *bufferPtr = buffer;
  sysuint_t rleCandidate;
  sysuint_t rleCount;

  const uint8_t* dataEnd = dataCur + length * increment;

  do {
    // Get equal pixels
    rleCandidate = (sysuint_t)dataCur[0];
    rleCount = 1;
    dataCur += increment;

    while (dataCur != dataEnd && rleCandidate == (sysuint_t)dataCur[0] && rleCount < 62)
    {
      dataCur += increment;
      rleCount++;
    }

    // Write RLE sequence
    if (rleCount > 1)
    {
      // Multiple pixels (Double-byte code)
      bufferPtr[0] = (uint8_t)(0xC0 | (uint8_t)rleCount);
      bufferPtr[1] = (uint8_t)(rleCandidate);
      bufferPtr += 2;
    }
    else
    {
      // Single pixel
      if (((uint8_t)rleCandidate & 0xC0) != 0xC0)
      {
        // Single-byte code
        *bufferPtr++ = (uint8_t)(rleCandidate);
      }
      else
      {
        // Double-byte code
        bufferPtr[0] = (uint8_t)(0xC1);
        bufferPtr[1] = (uint8_t)(rleCandidate);
        bufferPtr += 2;
      }
    }
  } while (dataCur != dataEnd);

  // align to 2 bytes
  // this is in specification, but it's not probabbly used
  // by most applications and libraries. This is correct code.
  if (alignment)
  {
    // if the last bytes are encoded using RLE, we can increment
    // it if it not exceeds 63
    /*if ((sysuint_t)(bufferPtr - buffer) >= 2 &&
        bufferPtr[-1] > 192 &&
        bufferPtr[-1] < 255 )
    {
      bufferPtr[-1]++;
    }*/
    // or just add zero byte
    /*else*/ 
    {
      *bufferPtr++ = 0;
    }
  }

  // Write compressed line into stream and return
  length = (sysuint_t)(bufferPtr - buffer);
  return stream.write((const char *)buffer, length) == length;
}

static void PCX_fillMonoPalette(Rgba* palette)
{
  palette[0] = 0xFF000000;
  palette[1] = 0xFFFFFFFF;
}

static void PCX_fillGreyPalette(Rgba* palette)
{
  uint32_t c = 0xFF000000;
  for (sysuint_t i = 256; i; i--, c += 0x00010101) *palette++ = c;
}

static void PCX_fillEgaPalette(Rgba* palette)
{
  palette[0] = 0xFF000000;
  palette[1] = 0xFF0000AA;
  palette[2] = 0xFF00AA00;
  palette[3] = 0xFF00AAAA;
  palette[4] = 0xFFAA0000;
  palette[5] = 0xFFAA00AA;
  palette[6] = 0xFFAA5500;
  palette[7] = 0xFFAAAAAA;
  palette[8] = 0xFF555555;
  palette[9] = 0xFF5555FF;
  palette[10] = 0xFF55FF55;
  palette[11] = 0xFF55FFFF;
  palette[12] = 0xFFFF5555;
  palette[13] = 0xFFFF55FF;
  palette[14] = 0xFFFFFF55;
  palette[15] = 0xFFFFFFFF;
}

static void PCX_applyPalette(uint8_t* pixels, sysuint_t count, const uint32_t* palette)
{
  ulong i;

  for (i = count; i; i--, pixels += 4)
  {
    ((uint32_t *)pixels)[0] = palette[pixels[0]];
  }
}

static void PCX_convPaletteToPcx(uint8_t* dest, const uint8_t* src, sysuint_t count, sysuint_t entrySize)
{
  for (sysuint_t i = 0; i != count; i++, dest += 3, src += entrySize)
  {
    uint32_t c = *(uint32_t *)src;
    dest[0] = (uint8_t)( (c & 0x00FF0000) >> 16 );
    dest[1] = (uint8_t)( (c & 0x0000FF00) >>  8 );
    dest[2] = (uint8_t)( (c & 0x000000FF)       );
  }
}

// ============================================================================
// [Fog::ImageIO::PcxDecoderDevice]
// ============================================================================

PcxDecoderDevice::PcxDecoderDevice()
{
  zeroall();
}

PcxDecoderDevice::~PcxDecoderDevice()
{
}

// ============================================================================
// [Fog::ImageIO::PcxDecoderDevice::reset]
// ============================================================================

void PcxDecoderDevice::reset()
{
  DecoderDevice::reset();
  zeroall();
}

// ============================================================================
// [Fog::ImageIO::PcxDecoderDevice::readHeader]
// ============================================================================

uint32_t PcxDecoderDevice::readHeader()
{
  // don't read header more than once
  if (headerDone()) return headerResult();

  // mark header as done
  _headerDone = true;

  // read pcx header
  if (stream().read(&_pcxFileHeader, sizeof(PcxHeader)) != sizeof(PcxHeader))
  {
    return EImageIOTruncated;
  }

  // check for correct mime
  if (pcxFileHeader().manufacturer != 10 ||
    pcxFileHeader().version != 5 ||
    pcxFileHeader().encoding != 1)
  {
    return EImageIOMimeNotMatch;
  }

  // byteswap header
  PCX_byteSwapHeader(&_pcxFileHeader);

  int16_t xMin = pcxFileHeader().xMin;
  int16_t yMin = pcxFileHeader().yMin;
  int16_t xMax = pcxFileHeader().xMax;
  int16_t yMax = pcxFileHeader().yMax;

  // size reject 
  if (xMin > xMax || yMin > yMax) return EImageSizeInvalid;

  // decode header
  _width  = (uint)(xMax - xMin) + 1;
  _height = (uint)(yMax - yMin) + 1;
  _depth  = (uint32_t)(pcxFileHeader().bitsPerPixel);
  _planes = (uint32_t)(pcxFileHeader().nPlanes);

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

  // pcx contains only one image
  _actualFrame = 0;
  _framesCount = 1;

  // Accept only supported formats
  switch (depth())
  {
    case 1:
      if (planes() >= 1 && planes() <= 4) break;
      return (_headerResult = EImageIOFormatNotSupported);
    case 4:
      if (planes() == 1) break;
      return (_headerResult = EImageIOFormatNotSupported);
    case 8:
      if ((planes() == 1 || 
         planes() == 3 || 
         planes() == 4) && 
        pcxFileHeader().version >= 4) break;
      // Go through
    default:
      return (_headerResult = EImageIOFormatNotSupported);
  }

  _format.set(ImageFormat::I8);

  // standardize output for Fog::ImageIO
  if (depth() == 8 && planes() == 4)
  {
    _depth = 32;
    _planes = 1;
    _format.set(ImageFormat::ARGB32);
  }
  else if (depth() == 8 && planes() == 3)
  {
    _depth = 24;
    _planes = 1;
    _format.set(ImageFormat::RGB24);
  }

  // success
  return (_headerResult = Error::Ok);
}

// ============================================================================
// [Fog::ImageIO::PcxDecoderDevice::readImage]
// ============================================================================

uint32_t PcxDecoderDevice::readImage(Image& image)
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

  // Source
  String8 dataArray;
  const uint8_t* dataCur;
  const uint8_t* dataEnd;

  // Destination
  uint8_t *pixelsCur;

  // Temporary plane data
  MemoryBuffer<1024> temporary;
  uint8_t* mem;

  // Bytes per line
  sysuint_t bytesPerLine = (sysuint_t)pcxFileHeader().bytesPerLine;

  // Loop variables
  uint x, y;

  // Secondary palette
  bool info_secPalette = false;
  uint info_secPaletteColors = 0;

  // Image text
  if (pcxFileHeader().unused[0])
  {
    const uint8_t* str = (const uint8_t*)pcxFileHeader().unused;
    _comment.set(StubAscii8(str, PCX_strnlen(str, 54)));
  }
  else
    _comment.free();

  stream().readAll(dataArray);
  dataCur = (const uint8_t*)dataArray.cData();
  dataEnd = dataCur + dataArray.length();

  if (!image.create(width(), height(), format()))
  {
    error = Error::OutOfMemory;
    goto end;
  }

  // BitsPP: 1
  // Planes: 1, 2, 3, 4
  if (depth() == 1)
  {
    sysuint_t plane;
    uint8_t b;

    if (!temporary.alloc(bytesPerLine))
    {
      error = Error::OutOfMemory;
      goto end;
    }

    for (y = 0; y != height(); y++) 
    {
      // Expand planes to 8 BPP
      for (plane = 0; plane != planes(); plane++) 
      {
        pixelsCur = image.xScanline(y);
        mem = (uint8_t*)temporary.mem();

        if ((error = PCX_decodeScanline(mem, &dataCur, dataEnd, bytesPerLine, 0, 1)) != Error::Ok) goto end;

        for (x = 0; x != width(); x++, pixelsCur++)
        {
          if ((x & 7) == 0) 
            b = *mem++;
          else
            b <<= 1;

          pixelsCur[0] |= ((b >> 7) & 1) << plane;
        }
      }
      if ((y & 15) == 0) updateProgress(y, height());
    }
  }

  // BitsPP: 4
  // Planes: 1
  else if (depth() == 4)
  {
    if (!temporary.alloc(bytesPerLine))
    {
      error = Error::OutOfMemory;
      goto end;
    }

    for (y = 0; y != height(); y++) 
    {
      pixelsCur = image.xScanline(y);
      mem = (uint8_t*)temporary.mem();

      if ((error = PCX_decodeScanline(mem, &dataCur, dataEnd, bytesPerLine, 0, 1)) != Error::Ok) goto end;

      for (x = 0; x != width(); x++, pixelsCur++) 
      {
        pixelsCur[0] = (!(x & 1)) ? ((*mem & 0xF0) >> 4) : (*mem++ & 0xF);
      }

      if (y % 15) updateProgress(y, height());
    }
  }

  // BitsPP: 8
  // Planes: 1, 3, 4
  else
  {
    sysuint_t pos[4];
    sysuint_t ignore = bytesPerLine - width();
    sysuint_t increment;
    sysuint_t plane;

    switch (depth())
    {
      case 8:
        pos[0] = 0;
        increment = 1;
        break;
      case 24:
        pos[0] = ImageFormat::RGB24_RBytePos;
        pos[1] = ImageFormat::RGB24_GBytePos;
        pos[2] = ImageFormat::RGB24_BBytePos;
        increment = 3;
      case 32:
        pos[0] = ImageFormat::ARGB32_RBytePos;
        pos[1] = ImageFormat::ARGB32_GBytePos;
        pos[2] = ImageFormat::ARGB32_BBytePos;
        pos[3] = ImageFormat::ARGB32_ABytePos;
        increment = 4;
        break;
      default:
        FOG_ASSERT_NOT_REACHED();
    }

    for (y = 0; y != height(); y++) 
    {
      pixelsCur = image.xScanline(y);
      for (plane = 0; plane < planes(); plane++)
      {
        if ((error = PCX_decodeScanline(pixelsCur + pos[plane], &dataCur, dataEnd, width(), ignore, increment)) != Error::Ok) goto end;
      }

      if (y % 15) updateProgress(y, height());
    }
  }

  // Read palette
  if (depth() <= 8)
  {
    Rgba* pdest = _palette.mData();
    uint nColors = 1 << (depth() * planes());
    bool read = true;

    // Setup basic palette settings if something will be wrong
    // or image not contains the palette. 
    Memory::zero(pdest, sizeof(Rgba) * 4);

    if (depth() == 1 && planes() == 1)
    {
      PCX_fillMonoPalette(pdest);
      if (pcxFileHeader().version == 2) read = false;
    }
    else
    {
      PCX_fillEgaPalette(pdest);
    }

    // 256 color palette
    if (depth() == 8)
    {
      // Find 0x0C marker
      while (dataCur != dataEnd)
      {
        if (*dataCur++ == 0x0C) 
        {
          info_secPalette = true;
          break;
        }
      }

      if (dataCur == dataEnd)
      {
        // Marker not found, so use greyscale ?
        // if (pcxFileHeader().paletteInfo == 2)
        // {
        PCX_fillGreyPalette(pdest);
        read = false;
        // }
      }
      else
      {
        ulong n = (ulong)(dataEnd - dataCur) / 3;
        if (n < nColors) nColors = n;
      }
      info_secPaletteColors = nColors;
    }
    else if (pcxFileHeader().version != 3)
    {
      dataCur = pcxFileHeader().colorMap;
      if (nColors > 16)
      {
        error = EImageIOFormatNotSupported;
        goto end;
      }
    }
    else
    {
      read = false;
    }

    // Read primary or secondary palette (from PCX header or end of file)
    if (read)
    {
      for (x = 0; x < nColors; x++) 
      {
        pdest[x].set(dataCur[0], dataCur[1], dataCur[2]);
        dataCur += 3;
      }
    }
  }

  // apply palette if needed
  if (depth() <= 8) image.setPalette(palette());

end:
  if (error == Error::Ok) updateProgress(1.0);
  return error;
}

// ============================================================================
// [Fog::ImageIO::PcxEncoder]
// ============================================================================

PcxEncoderDevice::PcxEncoderDevice()
{
}

PcxEncoderDevice::~PcxEncoderDevice()
{
}

uint32_t PcxEncoderDevice::writeImage(const Image& image_)
{
  Image image = image_;

  PcxHeader pcx;
  Memory::zero((void*)&pcx, sizeof(pcx));

  uint width = image.width();
  uint height = image.height();
  uint version;
  uint bitsPerPixel;
  uint nPlanes;
  uint bpl;
  uint alignment;
  uint y;

  MemoryBuffer<1024> rleenc;
  MemoryBuffer<1024> row;

  const uint8_t* pixels;

  if (!width || !height) return EImageSizeIsZero;

  version = 5;
  bitsPerPixel = 8;
  nPlanes = 1;
  bpl = width;

  if (image.format().isIndexed())
  {
    image.convert(ImageFormat::i8());
#if 0
    if (nColors <= 2)
    {
      bitsPerPixel = 1;
      bpl = (width + 7) >> 3;
      PCX_convPaletteToPcx(pcxFileHeader().colorMap, (const uint8_t*)&reduce.entities[0].rgba, 2, sizeof(Reduce::Color));
    }
    else if (nColors <= 16)
    {
      bitsPerPixel = 1;
      nPlanes = 4;
      bpl = (width + 7) >> 3;
      PCX_convPaletteToPcx(pcxFileHeader().colorMap, (const uint8_t *)&reduce.entities[0].rgba, 16, sizeof(Reduce::Color));
    }
#endif
  }
  else if (!image.format().hasAlpha())
  {
    image.convert(ImageFormat::rgb24());
    nPlanes = 3;
  }
  else if (image.format().hasAlpha())
  {
    image.convert(ImageFormat::argb32());
    nPlanes = 4;
  }

  // align bpl to 16 bits
  alignment = bpl & 1; 

  pcx.manufacturer = 0x10;
  pcx.version = version;
  pcx.encoding = 1;
  pcx.bitsPerPixel = bitsPerPixel;
  pcx.xMax = width - 1;
  pcx.yMax = height - 1;
  pcx.hScreenSize = width;
  pcx.vScreenSize = height;
  pcx.nPlanes = nPlanes;
  pcx.bytesPerLine = bpl + alignment;
  // color or mono
  pcx.paletteInfo = 1;

  // Try to write comment
  if (!comment().isEmpty())
  {
    sysuint_t length = comment().length();
    if (length > 54) length = 54;
    memcpy(pcx.unused, (const char*)comment().cData(), length);
  }

  PCX_byteSwapHeader(&pcx);

  // Write pcx header
  if (stream().write((const char *)(&pcx), sizeof(PcxHeader)) != sizeof(PcxHeader)) goto fail;

  // Alloc buffers
  if (!rleenc.alloc((width << 1) + 2) || !row.alloc(bpl)) return Error::OutOfMemory;

  // Write 8 bit image
  if (image.format().isIndexed())
  {
#if 0
    // Write 1 bit MONO image
    if (nColors <= 2)
    {
      for (y = 0; y != height; y++)
      {
        pixels = image.cScanLine(y);

        memset(row.mem(), 0, bpl);
        for (x = 0; x != width; x++, pixels += 4)
        {
          if (reduce.index[*(uint32_t *)pixels] == 1)
            row.mem()[x >> 3] |= 0x80 >> (x & 7);
        }

        if (!PCX_encodeScanline(device, rleenc.mem(), row.mem(), bpl, alignment, 1)) goto fail;
      }
    }
    // Write 4 bit COLOR image
    else if (nColors <= 16)
    {
      // this is quite unusual, but we needs 4 row buffers 
      // (for each plane)
      MemoryBuffer<1024> rows[4];
      uint plane;

      for (plane = 0; plane != 4; plane++) 
      {
        if (!rows[plane].alloc(bpl))
        {
          return Error::OutOfMemory;
        }
      }

      for (y = 0; y != height; y++)
      {
        pixels = (uint8_t *)image.scanLine(y);

        for (plane = 0; plane != 4; plane++) memset(rows[plane].mem(), 0, bpl);

        for (x = 0; x != width; x++, pixels += 4)
        {
          uint8_t c = reduce.index[*(uint32_t *)pixels];
          for (plane = 0; plane != 4; plane++)
          {
            if (c & (1 << plane))
              ((uint8_t *)rows[plane].mem())[x >> 3] |= 0x80 >> (x & 7);
          }
        }

        for (plane = 0; plane != 4; plane++)
        {
          PCX_encodeScanline(device, rleenc.mem(), rows[plane].mem(), bpl, alignment, 1);
        }
      }
    }
    // Write 8 bit COLOR image
    else
#endif
    {
      for (y = 0; y != height; y++)
      {
        pixels = image.cScanline(y);
        if (!PCX_encodeScanline(stream(), (uint8_t*)rleenc.mem(), pixels, bpl, alignment, 1)) goto fail;
      }

      // Write secondary palette
      {
        uint8_t palette[768+1];
        palette[0] = 0xC;
        PCX_convPaletteToPcx(palette + 1, (uint8_t*)image.palette().cData(), 256, sizeof(Rgba));
        if (stream().write((const char*)palette, 768+1) != 768+1) goto fail;
      }
      if ((y & 15) == 0) updateProgress(y, height);
    }
  }
  // Write 24 or 32 bit RGB image
  else
  {
    sysuint_t pos[4];
    sysuint_t plane;
    sysuint_t planes = 3 + image.format().hasAlpha();
    sysuint_t increment = image.format().depth() >> 3;

    pos[0] = image.format().rBytePos();
    pos[1] = image.format().gBytePos();
    pos[2] = image.format().bBytePos();
    pos[3] = image.format().aBytePos();

    for (y = 0; y != height; y++)
    {
      pixels = image.cScanline(y);
      for (plane = 0; plane != planes; plane++)
      {
        if (!PCX_encodeScanline(stream(), (uint8_t*)rleenc.mem(), pixels + pos[plane], bpl, alignment, increment)) goto fail;
      }
      if ((y & 15) == 0) updateProgress(y, height);
    }
  }

  updateProgress(1.0);
  return Error::Ok;
fail:
  updateProgress(1.0);
  return EImageIOWriteError;
}

} // ImageIO namespace
} // Fog namespace

// ============================================================================
// [CAPI]
// ============================================================================

FOG_CAPI_DECLARE Fog::ImageIO::Provider* fog_imageio_getPcxProvider(void)
{
  return new Fog::ImageIO::PcxProvider();
}
