// [Fog/Graphics Library - Public API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Constants.h>
#include <Fog/Core/ManagedString.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/Misc.h>
#include <Fog/Core/Stream.h>
#include <Fog/Core/String.h>
#include <Fog/Core/Strings.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/ImageIO.h>
#include <Fog/Graphics/ImageIO/Structures_p.h>
#include <Fog/Graphics/RasterUtil_p.h>

namespace Fog { 
namespace ImageIO {

// ============================================================================
// [Fog::ImageIO::IcoDecoderDevice]
// ============================================================================

struct FOG_HIDDEN IcoDecoderDevice : public DecoderDevice
{
  FOG_DECLARE_OBJECT(IcoDecoderDevice, DecoderDevice)

  IcoDecoderDevice(Provider* provider);
  virtual ~IcoDecoderDevice();

  virtual void reset();
  virtual err_t readHeader();
  virtual err_t readImage(Image& image);

private:
  // For determining offsets/sizes of "frames" LE numbers are already
  // converted to BE numbers on BE systems.
  IcoEntry *_framesInfo;
};

// ============================================================================
// [Fog::ImageIO::IcoProvider]
// ============================================================================

struct FOG_HIDDEN IcoProvider : public Provider
{
  IcoProvider();
  virtual ~IcoProvider();

  virtual uint32_t check(const void* mem, sysuint_t length);
  virtual DecoderDevice* createDecoder();
};

IcoProvider::IcoProvider()
{
  // Name of ImageIO Provider.
  _name = fog_strings->getString(STR_GRAPHICS_ICO);

  // Supported features.
  _features.decoder = true;
  _features.encoder = false;

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

  // Supported extensions.
  _extensions.append(fog_strings->getString(STR_GRAPHICS_ico));
}

IcoProvider::~IcoProvider()
{
}

uint32_t IcoProvider::check(const void* mem, sysuint_t length)
{
  if (length < sizeof(IcoHeader)) return 0;

  const uint8_t* m = (const uint8_t*)mem;
  
  if (*(const uint16_t*)m != 0x0000) return 0;
  if (Memory::bswap16le(*(const uint16_t*)(m+2)) != 0x0001) return 0;
  
  sysuint_t remaining = length - sizeof(IcoHeader);
  uint16_t count = Memory::bswap16le(*(const uint16_t*)(m+4));
    
  if (remaining < sizeof(IcoEntry) || count == 0)
  {
    // We cannot be much sure, because ICO files have no signature
    return 10;
  }

  uint16_t entriesAvail = 0;
  sysuint_t fOffset, fSize;
  IcoEntry *entry = (IcoEntry*)(m + sizeof(IcoHeader));
  sysuint_t minOffset = sizeof(IcoHeader) + (sysuint_t)count * sizeof(IcoEntry);

  while (remaining >= sizeof(IcoEntry))
  {
    fSize = Memory::bswap32le(entry->size);
    fOffset = Memory::bswap32le(entry->offset);
    
    if (fOffset < minOffset || fSize == 0) return 0;
    
    ++entriesAvail;
    remaining -= sizeof(IcoEntry);
    ++entry;
  }

  // TODO: I don't understand this. Need to saturate some values (count can be
  // large and entries small, maybe min(95, 40 + (entriesAvail) * 5) ?
  return Math::max(20, 80 - (count - entriesAvail) * 5);
}

DecoderDevice* IcoProvider::createDecoder()
{
  return new(std::nothrow) IcoDecoderDevice(this);
}

// ============================================================================
// [Fog::ImageIO::IcoDecoderDevice]
// ============================================================================

IcoDecoderDevice::IcoDecoderDevice(Provider* provider) :
  DecoderDevice(provider)
{
  _framesInfo = NULL;
}

IcoDecoderDevice::~IcoDecoderDevice()
{
  reset();
}

// ============================================================================
// [Fog::ImageIO::IcoDecoderDevice::reset]
// ============================================================================

void IcoDecoderDevice::reset()
{
  DecoderDevice::reset();
  
  if (_framesInfo) Memory::free(_framesInfo);

  _framesInfo = NULL;
}

// ============================================================================
// [Fog::ImageIO::IcoDecoderDevice::readHeader]
// ============================================================================

err_t IcoDecoderDevice::readHeader()
{
  // Don't read header more than once.
  if (isHeaderDone()) return _headerResult;

  // Mark header as done.
  _headerDone = true;

  {
    IcoHeader icoHeader;
    if (getStream().read(&icoHeader, sizeof(IcoHeader)) != sizeof(IcoHeader))
    {
      return (_headerResult = ERR_IMAGEIO_TRUNCATED);
    }

    icoHeader.type = Memory::bswap16le(icoHeader.type);
    icoHeader.count = Memory::bswap16le(icoHeader.count);

    if (icoHeader.reserved != 0 || icoHeader.type != 1 /* don't read cursors (2) */)
    {
      return (_headerResult = ERR_IMAGEIO_MIME_NOT_MATCH);
    }

    _framesCount = icoHeader.count;
    _actualFrame = 0;
    // discard icoHeader now
  }

  // Read frames' info.
  if (_framesCount > 0)
  {
    // Added check for malformed _framesCount value right here. Is it possible
    // to have more than 1024 frames in ICO?
    if (_framesCount > 1024)
    {
      return (_headerResult = ERR_IMAGEIO_MALFORMED_HEADER);
    }

    sysuint_t memSize = _framesCount * sizeof(IcoEntry);
    
    _framesInfo = (IcoEntry*)Memory::alloc(memSize);
    if (_framesInfo == NULL)
    {
      return (_headerResult = ERR_RT_OUT_OF_MEMORY);
    }

    if (getStream().read(_framesInfo, memSize) != memSize)
    {
      Memory::free(_framesInfo);
      _framesInfo = NULL;
      _framesCount = 0;
      return (_headerResult = ERR_IMAGEIO_TRUNCATED);
    }
    
    // _currentOffset is now currently minimal allowed offset for a frame.
    int64_t _currentOffset = sizeof(IcoHeader) + memSize;
    IcoEntry *currentEntry = _framesInfo;
    
    for (sysuint_t i = 0; i < _framesCount; ++i, ++currentEntry)
    {
#if FOG_BYTE_ORDER == FOG_BIG_ENDIAN
      currentEntry->colorPlanes = Memory::bswap16le(currentEntry->colorPlanes);
      currentEntry->bpp         = Memory::bswap16le(currentEntry->bpp);
      currentEntry->size        = Memory::bswap32le(currentEntry->size);
      currentEntry->offset      = Memory::bswap32le(currentEntry->offset);
#endif
      
      if (currentEntry->offset < _currentOffset || currentEntry->size == 0)
      {
        Memory::free(_framesInfo);
        _framesInfo = NULL;
        _framesCount = 0;
        return (_headerResult = ERR_IMAGEIO_MALFORMED_HEADER);
      }
    }
    // TODO:  ? check individual frames if the stream is seekable ?
  }

  // Success.
  return (_headerResult = ERR_OK);
}

// ============================================================================
// [IcoDecoderDevice::readImage]
// ============================================================================

err_t IcoDecoderDevice::readImage(Image& image)
{
  if (readHeader() != ERR_OK) return _headerResult;

  if (_actualFrame == _framesCount || !_framesInfo) return ERR_IMAGEIO_NO_MORE_FRAMES;

  err_t err;

  IcoEntry *entry = _framesInfo + _actualFrame;
  DecoderDevice* decoder = NULL;

  int64_t toSeek = _attachedOffset + entry->offset;
  if (_stream.seek(toSeek, STREAM_SEEK_SET) == -1) return ERR_IO_CANT_SEEK;

  // Stream should be at frame position now, either PNG or BMP data follows...
  //
  // I read carefully documentation about ICO format and its new variants and it
  // seems that PNG image format is only used if WIDTH and HEIGHT in ICO entry
  // were set to zeros. Some documentation says that this means that ICO is
  // 256x256 pixels big, but I thing that correct interpretation is that instead
  // PNG format is used and ICO size can be variant.
  if (entry->width == 0 && entry->height == 0)
  {
    // Here I used createDecoderByExtension(), becuase I don't care which decoder
    // will be used for this (for example it's possible to use GDI+ under Windows).
    decoder = createDecoderByExtension(fog_strings->getString(STR_GRAPHICS_png), &err);
    if (!decoder) goto end;

    decoder->attachStream(_stream);
    err = decoder->readImage(image);
  }
  else
  {
    // Here I used createDecoderByName(), because I want exactly BMP Decoder
    // from Fog. Note the property skipFileHeader that configures this decoder
    // to skip reading of bmp file header, this is important step.
    decoder = createDecoderByName(fog_strings->getString(STR_GRAPHICS_BMP), &err);
    if (!decoder) goto end;

    decoder->attachStream(_stream);
    decoder->setProperty(fog_strings->getString(STR_GRAPHICS_skipFileHeader), Value::fromInt32(1));

    if ((err = decoder->readHeader()) == ERR_OK)
    {
      // There are also AND+XOR masks, we adjust height to be only half of it.
      decoder->_height /= 2;
      err = decoder->readImage(image);
    }

    // 32-bit icons contains alpha-channel.
    if (image.getFormat() == PIXEL_FORMAT_XRGB32) image.forceFormat(PIXEL_FORMAT_ARGB32);
  }

  delete decoder;

end:
  // We always increment actual frame, because if there is no PNG loader we can
  // just skip PNG frames.
  ++_actualFrame;
  return err;
}

} // ImageIO namespace
} // Fog namespace

FOG_IMPLEMENT_OBJECT(Fog::ImageIO::IcoDecoderDevice)

// ============================================================================
// [CAPI]
// ============================================================================

FOG_INIT_DECLARE void fog_imageio_init_ico(void)
{
  using namespace Fog;
  ImageIO::addProvider(new(std::nothrow) ImageIO::IcoProvider());
}
