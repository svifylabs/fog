// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled Headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/ManagedString.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/Misc.h>
#include <Fog/Core/Stream.h>
#include <Fog/Core/String.h>
#include <Fog/Core/Strings.h>
#include <Fog/Graphics/Error.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/ImageIO/ImageIO_BMP.h> // BMP stuff
#include <Fog/Graphics/ImageIO/ImageIO_ICO.h>
#include <Fog/Graphics/Raster.h>
#include <Fog/Graphics/Raster/Raster_C.h>

namespace Fog { 
namespace ImageIO {

// ============================================================================
// [Fog::ImageIO::IcoProvider]
// ============================================================================

struct IcoProvider : public Provider
{
  IcoProvider();
  virtual ~IcoProvider();

  virtual uint32_t check(const void* mem, sysuint_t length);
  virtual DecoderDevice* createDecoder();
};

IcoProvider::IcoProvider()
{
  // features
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

  // name
  _name = Ascii8("image/x-icon");

  // extensions
  _extensions.append(Ascii8("ico"));
}

IcoProvider::~IcoProvider()
{
}

uint32_t IcoProvider::check(const void* mem, sysuint_t length)
{
  if (length < sizeof(IcoHeader)) return 0;

  const uint8_t* m = (const uint8_t*)mem;
  
  if (*(const uint16_t*)m != 0) return 0;
  if (Memory::bswap16le(*(const uint16_t*)(m+2)) != 1) return 0;
  
  sysuint_t remaining = length - sizeof(IcoHeader);
  uint16_t count = Memory::bswap16le(*(const uint16_t*)(m+4));
    
  if (remaining < sizeof(IcoEntry) || count == 0)
  {
    // We cannot be much sure, because ICO files have no signature
    return 10;
  }

  uint16_t entriesAvail = 0;
  sysuint_t fOffset, fSize;
  IcoEntry *entry = (IcoEntry*)(m+sizeof(IcoHeader));
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

  return Math::max(20, 80 - (count - entriesAvail) * 5);
}

DecoderDevice* IcoProvider::createDecoder()
{
  return new IcoDecoderDevice();
}

// ============================================================================
// [Fog::ImageIO::IcoDecoderDevice]
// ============================================================================

IcoDecoderDevice::IcoDecoderDevice()
{
  _framesInfo = 0;
  _currentOffset = 0;
}

IcoDecoderDevice::~IcoDecoderDevice()
{
  if (_framesInfo)
  {
    Memory::free(_framesInfo);
  }
}

// ============================================================================
// [Fog::ImageIO::IcoDecoderDevice::reset]
// ============================================================================

void IcoDecoderDevice::reset()
{
  DecoderDevice::reset();
  
  if (_framesInfo)
  {
    Memory::free(_framesInfo);
    _framesInfo = 0;
  }
  _currentOffset = 0;
}

// ============================================================================
// [Fog::ImageIO::IcoDecoderDevice::readHeader]
// ============================================================================

uint32_t IcoDecoderDevice::readHeader()
{
  // don't read header more than once
  if (headerDone()) return headerResult();
  
  // mark header as done
  _headerDone = true;
  
  {
  	IcoHeader icoHeader;
    if (stream().read(&icoHeader, sizeof(IcoHeader)) != sizeof(IcoHeader))
    {
      return (_headerResult = Error::ImageIO_Truncated);
    }
    
    icoHeader.type = Memory::bswap16le(icoHeader.type);
    icoHeader.count = Memory::bswap16le(icoHeader.count);
    
    if (icoHeader.reserved != 0 || icoHeader.type != 1 /* don't read cursors (2) */)
    {
      return (_headerResult = Error::ImageIO_MimeNotMatch);
    }
    
    _framesCount = icoHeader.count;
    _actualFrame = 0;
    // discard icoHeader now
  }
  
  // read frames' info
  if (_framesCount > 0)
  {
    sysuint_t memSize = _framesCount * sizeof(IcoEntry);
    
    _framesInfo = (IcoEntry*)Memory::alloc(memSize);
    
    if (stream().read(_framesInfo, memSize) != memSize)
    {
      Memory::free(_framesInfo);
      _framesInfo = 0;
      _framesCount = 0;
      return (_headerResult = Error::ImageIO_Truncated);
    }
    
    _currentOffset = sizeof(IcoHeader) + memSize;
    // _currentOffset is now currently minimal allowed offset for a frame
  
    IcoEntry *currentEntry = _framesInfo;
    
    for (sysuint_t i = 0; i < _framesCount; ++i, ++currentEntry)
    {
      #if FOG_BYTE_ORDER == FOG_BIG_ENDIAN
      currentEntry->colorPlanes = Memory::bswap16le(currentEntry->colorPlanes);
      currentEntry->bpp = Memory::bswap16le(currentEntry->bpp);
      currentEntry->size = Memory::bswap32le(currentEntry->size);
      currentEntry->offset = Memory::bswap32le(currentEntry->offset);
      #endif
      
      if (currentEntry->offset < _currentOffset || currentEntry->size == 0)
      {
        Memory::free(_framesInfo);
        _framesCount = 0;
        _framesInfo = 0;
        return (_headerResult = Error::ImageIO_MimeNotMatch);
      }
    }
    // TODO:  ? check individual frames if the stream is seekable ?
  }
  
  // success
  return (_headerResult = Error::Ok);
}

// ============================================================================
// [IcoDecoderDevice::readImage]
// ============================================================================

// Part of ugly hack to allow PNG decoder to read PNG from non-seekable
// stream after we already readed PNG signature
struct IcoStreamReadDevice: public StreamDevice
{
	const uint8_t *_prereadBuffer;
    sysuint_t _prereadSize;
	Stream& _parent;
    int64_t _readFromParent;
    
    IcoStreamReadDevice(const uint8_t *buf, sysuint_t bufsize, Stream& strm):
    	_prereadBuffer(buf),
        _prereadSize(bufsize),
        _parent(strm),
        _readFromParent(0)
    {
    	flags = Stream::IsReadable | Stream::IsOpen;
    }
    
    virtual ~IcoStreamReadDevice() { }
    
    virtual int64_t seek(int64_t offset, int whence) { return 0; }
    virtual int64_t tell() const { return 0; }
    virtual sysuint_t read(void* buffer, sysuint_t size)
    {
    	uint8_t *mem = (uint8_t*)buffer;
        
    	if (size == 0 || mem == 0) return 0;
        
        sysuint_t readTotal = 0;
        sysuint_t readParent = 0;
        
        if (_prereadSize > 0)
        {
        	sysuint_t fromPreread = Math::min(size, _prereadSize);
            Memory::copy(mem, _prereadBuffer, fromPreread);
            _prereadSize -= fromPreread;
            _prereadBuffer += fromPreread;
            mem += fromPreread;
            size -= fromPreread;
            readTotal += fromPreread;
        }
        
        if (size > 0)
        {
        	readParent = _parent.read(mem, size);
            readTotal += readParent;
            _readFromParent += (int64_t)readParent;
        }
        
        return readTotal;
    }
    
    virtual sysuint_t write(const void* buffer, sysuint_t size)
    {
      return 0;
    }
    
    // FIXME
    virtual err_t truncate(int64_t offset) { return Error::Ok; }
    
    virtual void close() {}
};

uint32_t IcoDecoderDevice::readImage(Image& image)
{
   if (readHeader() != Error::Ok)
   {
     return headerResult();
   }
   
   if (_actualFrame == _framesCount || !_framesInfo)
   {
     // TODO: return some nicer error code here
     return Error::ImageIO_NotAnimationFormat;
   }
   
   Stream& strm = stream();
   
   IcoEntry *entry = _framesInfo + _actualFrame;
   
   int64_t fOffset = entry->offset;
   int64_t fSize = entry->size;
   
   if (strm.isSeekable())
   {
     int64_t toSeek = fOffset - _currentOffset;
     strm.seek(toSeek, Stream::SeekCur); // TODO: check the seek
     _currentOffset += toSeek;
   }
   else
   {
     if (_currentOffset > fOffset)
     {
       // No way to seek backwards
       return Error::ImageIO_SeekFailure;
     }
     else
     {
       // Let's seek by reading and discarding bytes
       
       int64_t toSeek = fOffset - _currentOffset;
       sysuint_t toRead;
       uint8_t seekBuffer[4096];
       while (toSeek > 0)
       {
         // following typecast should be safe as we never read more than 4096 bytes
         toRead = (sysuint_t)Math::min((int64_t)4096, toSeek);
         
         if (strm.read(seekBuffer, toRead) != toRead)
         {
           return Error::ImageIO_SeekFailure;
         }
         
         // again, this should be safe
         toSeek -= (int64_t)toRead;
       }
       _currentOffset = fOffset;
     }
   }
   
   // stream should be at frame position now,
   // either PNG or BMP data follows
   
   err_t err = Error::Ok;
   
   {
     static const uint8_t pngMagic[8] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
     
     union
     {
       uint8_t pngTest[8];
       BmpV3Header bitmapHeader;
     };
     
     if (strm.read(pngTest, 8) != 8)
     {
       // problem here :-(
       return 9664; // TODO: return some nice error code here
     }
     
     _currentOffset += 8;
     
     if (Memory::eq8B(pngMagic, pngTest))
     {
       // it could be PNG, use PNG decoder to read this
       // unfortunately we get libpng depency here for ico loader :-(
       // ? Write non-libpng based decoder ?
       
       // Find png provider
       
       Provider* pngProvider = getProviderByExtension(Ascii8("png"));
       if (!pngProvider)
       {
         err = Error::ImageIO_ProviderNotAvailable;
         goto __ret;
       }
       
       // Fing png decoder
       
       DecoderDevice* pngDecoder = pngProvider->createDecoder();
       if (!pngDecoder)
       {
         err = Error::ImageIO_DecoderNotAvailable;
         goto __ret;
       }
       
       // create stream wrapper so that PNG decoder
       // will get png header too (_ugly hack_)
       IcoStreamReadDevice *readDevice = new IcoStreamReadDevice(pngTest, 8, strm);
       Stream pngStream(readDevice->ref());
       pngDecoder->attachStream(pngStream);
       // decode PNG
       err = pngDecoder->readImage(image);
       pngDecoder->detachStream();
       // free decoder
       delete pngDecoder;
       // add bytes readed as PNG data to current stream offset
       _currentOffset += readDevice->_readFromParent;
       readDevice->deref();
     }
     else
     {
       // it could be bitmap, not supported for now as
       // there is no way to use BMP decoder because of
       // lack of bitmap file header (and even if that would
       // be hardcoded in similar way as previous PNG signature
       // there would be no way to load AND bitmap
       // ? Probably read it without BMP decoder by outselves ?
       
       // TODO: read the rest of bitmap header
       // TODO: decode bitmap
       err = Error::ImageIO_DecoderNotAvailable;
     }
     
     if (err == Error::Ok)
     {
       // check the image against the entry if
       // it is correct
       int entryWidth = (int)entry->width;
       int entryHeight = (int)entry->height;
       
       if (entryWidth == 0) entryWidth = 256;
       if (entryHeight == 0) entryHeight = 256;
       
       if (entryWidth != image.getWidth() || entryHeight != image.getHeight())
       {
         // indicate an error and destroy the image
         err = 56995; // TODO: nicer error code
         image = Image();
       }
     }
   }
   
__ret:
   // advance one frame even if we failed, because next
   // frame might be read successfuly
   ++_actualFrame;
   return err;
}

} // ImageIO namespace
} // Fog namespace

// ============================================================================
// [CAPI]
// ============================================================================

FOG_INIT_DECLARE void fog_imageio_ico_init(void)
{
  using namespace Fog;

  ImageIO::addProvider(new(std::nothrow) ImageIO::IcoProvider());
}

FOG_INIT_DECLARE void fog_imageio_ico_shutdown(void)
{
}

