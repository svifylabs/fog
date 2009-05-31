// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/Assert.h>
#include <Fog/Core/AutoLock.h>
#include <Fog/Core/FileSystem.h>
#include <Fog/Core/Lock.h>
#include <Fog/Core/MapFile.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/Std.h>
#include <Fog/Core/Stream.h>
#include <Fog/Core/String.h>
#include <Fog/Core/TextCodec.h>
#include <Fog/Core/Vector.h>

#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/ImageIO.h>
#include <Fog/Graphics/ImageIO/ImageIO_BMP.h>
#include <Fog/Graphics/ImageIO/ImageIO_GIF.h>
#include <Fog/Graphics/ImageIO/ImageIO_JPEG.h>
#include <Fog/Graphics/ImageIO/ImageIO_PCX.h>
#include <Fog/Graphics/ImageIO/ImageIO_PNG.h>

// [Provider Ininializers]
FOG_CAPI_EXTERN void fog_imageio_bmp_init(void);
FOG_CAPI_EXTERN void fog_imageio_bmp_shutdown(void);

FOG_CAPI_EXTERN void fog_imageio_pcx_init(void);
FOG_CAPI_EXTERN void fog_imageio_pcx_shutdown(void);

FOG_CAPI_EXTERN void fog_imageio_gif_init(void);
FOG_CAPI_EXTERN void fog_imageio_gif_shutdown(void);

FOG_CAPI_EXTERN void fog_imageio_png_init(void);
FOG_CAPI_EXTERN void fog_imageio_png_shutdown(void);

FOG_CAPI_EXTERN void fog_imageio_jpeg_init(void);
FOG_CAPI_EXTERN void fog_imageio_jpeg_shutdown(void);

namespace Fog {

// ============================================================================
// [Fog::ImageIO - Local]
// ============================================================================

struct ImageIO_Local
{
  // Critical section for accessing members
  Lock lock;

  // List of providers
  typedef Vector<ImageIO::Provider*> Providers;

  Providers providers;
};

static Static<ImageIO_Local> imageio_local;

// [Fog::ImageIO::]
namespace ImageIO {

// ============================================================================
// [Fog::ImageIO - Functions]
// ============================================================================

FOG_API bool addProvider(Provider* provider)
{
  AutoLock locked(imageio_local->lock);

  if (imageio_local->providers.indexOf(provider) == InvalidIndex)
  {
    imageio_local->providers.append(provider);
    return true;
  }
  else 
    return false;
}

FOG_API bool removeProvider(Provider* provider)
{
  AutoLock locked(imageio_local->lock);
  sysuint_t index = imageio_local->providers.indexOf(provider);

  if (index != InvalidIndex)
  {
    imageio_local->providers.removeAt(index);
    return true;
  }
  else 
    return false;
}

FOG_API bool hasProvider(Provider* provider)
{
  AutoLock locked(imageio_local->lock);
  return imageio_local->providers.indexOf(provider) != InvalidIndex;
}

FOG_API Vector<Provider*> getProviders()
{
  AutoLock locked(imageio_local->lock);
  return imageio_local->providers;
}

FOG_API Provider* getProviderByName(const String32& name)
{
  AutoLock locked(imageio_local->lock);
  ImageIO_Local::Providers::ConstIterator it(imageio_local->providers);

  for (; it.isValid(); it.toNext())
  {
    if (it.value()->name() == name) return it.value();
  }

  return 0;
}

FOG_API Provider* getProviderByExtension(const String32& extension)
{
  AutoLock locked(imageio_local->lock);
  ImageIO_Local::Providers::ConstIterator it(imageio_local->providers);

  // Convert extension to lower case
  TemporaryString32<16> e;
  e.set(extension);
  e.lower();

  for (it.toStart(); it.isValid(); it.toNext())
  {
    if (it.value()->extensions().indexOf(e) != InvalidIndex) return it.value();
  }

  return 0;
}

FOG_API Provider* getProviderByMemory(void* mem, sysuint_t len)
{
  if (!mem || len == 0) return 0;

  AutoLock locked(imageio_local->lock);
  ImageIO_Local::Providers::ConstIterator it(imageio_local->providers);

  Provider* bestProvider = NULL;
  uint32_t bestScore = 0;

  for (it.toStart(); it.isValid(); it.toNext())
  {
    uint32_t score = it.value()->check(mem, len);
    if (score > bestScore)
    {
      bestProvider = it.value();
      bestScore = score;
    }
  }

  return bestProvider;
}

// ============================================================================
// [Fog::ImageIO::Provider]
// ============================================================================

Provider::Provider()
{
  memset(&_features, 0, sizeof(_features));
  _id = ImageFileNone;
}

Provider::~Provider()
{
}

EncoderDevice* Provider::createEncoder()
{
  return 0;
}

DecoderDevice* Provider::createDecoder()
{
  return 0;
}

// ============================================================================
// [Fog::ImageIO::BaseDevice]
// ============================================================================

BaseDevice::BaseDevice() :
  _provider(NULL),
  _deviceType(BaseDevice::None),
  _attachedOffset(FOG_UINT64_C(0)),
  _stream(),
  _width(0),
  _height(0),
  _depth(0),
  _planes(0),
  _actualFrame(0),
  _framesCount(0),
  _progress(0.0f)
{
}

BaseDevice::~BaseDevice()
{
}

void BaseDevice::updateProgress(float value)
{
  _progress = value;
}

void BaseDevice::updateProgress(uint32_t y, uint32_t height)
{
  updateProgress((float)( (double)y / (double)height ));
}

bool BaseDevice::areDimensionsZero() const 
{ 
  return width() == 0 || height() == 0; 
}

bool BaseDevice::areDimensionsTooLarge() const 
{ 
  // check individual coordinates
  const uint32_t side = 256*256*128;
  if (width() > side || height() > side) return true;

  // check total count of pixels
  uint64_t total = (uint64_t)width() * (uint64_t)height();
  if (total >= FOG_UINT64_C(1000000000)) return true;

  // ok
  return false;
}

void BaseDevice::attachStream(Stream& stream)
{
  // detach current stream first
  if (_stream.isOpen()) detachStream();

  _stream = stream;
  _attachedOffset = (uint64_t)_stream.tell();
}

void BaseDevice::detachStream()
{
  if (stream().isOpen())
  {
    reset();
    _stream.close();
  }
}

err_t BaseDevice::setProperty(const String32& name, const Value& value)
{
  return Error::InvalidProperty;
}

Value BaseDevice::getProperty(const String32& name)
{
  return Value();
}

void BaseDevice::reset()
{
  _attachedOffset = FOG_UINT64_C(0);
  _width = 0;
  _height = 0;
  _depth = 0;
  _planes = 0;
  _actualFrame = 0;
  _framesCount = 0;
  _progress = 0.0f;

  _format = Image::FormatNull;
  _palette.free();
  _comment.free();
}

// ============================================================================
// [Fog::ImageIO::DecoderDevice]
// ============================================================================

DecoderDevice::DecoderDevice() :
  BaseDevice(),
  _headerDone(false),
  _readerDone(false),
  _headerResult(Error::Ok),
  _readerResult(Error::Ok)
{
  _deviceType = DecoderType;
}

DecoderDevice::~DecoderDevice()
{
}

void DecoderDevice::reset()
{
  BaseDevice::reset();

  _headerDone = false;
  _readerDone = false;

  _headerResult = Error::Ok;
  _readerResult = Error::Ok;
}

// ============================================================================
// [EncoderDevice]
// ============================================================================

EncoderDevice::EncoderDevice() :
  BaseDevice(),
  _headerDone(false),
  _writerDone(false)
{
  _deviceType = EncoderType;
}

EncoderDevice::~EncoderDevice()
{
}

void EncoderDevice::detachStream()
{
  finalize();

  BaseDevice::detachStream();
}

void EncoderDevice::reset()
{
  BaseDevice::reset();

  _headerDone = false;
  _writerDone = false;
}

void EncoderDevice::finalize()
{
  _headerDone = false;
  _writerDone = false;
}

} // ImageIO namespace
} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_imageio_init(void)
{
  Fog::imageio_local.init();

  fog_imageio_bmp_init();
  fog_imageio_pcx_init();
  fog_imageio_gif_init();
  fog_imageio_png_init();
  fog_imageio_jpeg_init();

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_imageio_shutdown(void)
{
  // Do not need to lock, because we are shutting down. All threads should
  // been already joined.
  Fog::ImageIO_Local::Providers::ConstIterator it(
    Fog::imageio_local->providers);

  // Remove (and delete) all providers
  for (it.toStart(); it.isValid(); it.toNext()) delete it.value();

  fog_imageio_jpeg_shutdown();
  fog_imageio_png_shutdown();
  fog_imageio_gif_shutdown();
  fog_imageio_pcx_shutdown();
  fog_imageio_bmp_shutdown();

  Fog::imageio_local.destroy();
}
