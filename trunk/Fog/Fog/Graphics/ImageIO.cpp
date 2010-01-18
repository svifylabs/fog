// [Fog/Graphics Library - Public API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Assert.h>
#include <Fog/Core/AutoLock.h>
#include <Fog/Core/FileSystem.h>
#include <Fog/Core/FileUtil.h>
#include <Fog/Core/List.h>
#include <Fog/Core/Lock.h>
#include <Fog/Core/ManagedString.h>
#include <Fog/Core/MapFile.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/Std.h>
#include <Fog/Core/Stream.h>
#include <Fog/Core/String.h>
#include <Fog/Core/Strings.h>
#include <Fog/Core/TextCodec.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/ImageIO.h>

// [Provider Ininializers]
FOG_CAPI_EXTERN void fog_imageio_init_bmp(void);
FOG_CAPI_EXTERN void fog_imageio_init_gif(void);
FOG_CAPI_EXTERN void fog_imageio_init_ico(void);
FOG_CAPI_EXTERN void fog_imageio_init_jpeg(void);
FOG_CAPI_EXTERN void fog_imageio_init_pcx(void);
FOG_CAPI_EXTERN void fog_imageio_init_png(void);

FOG_IMPLEMENT_OBJECT(Fog::ImageIO::BaseDevice)
FOG_IMPLEMENT_OBJECT(Fog::ImageIO::DecoderDevice)
FOG_IMPLEMENT_OBJECT(Fog::ImageIO::EncoderDevice)

namespace Fog {

// ============================================================================
// [Fog::ImageIO - Local]
// ============================================================================

struct ImageIO_Local
{
  // Critical section for accessing members
  Lock lock;

  // List of providers
  typedef List<ImageIO::Provider*> Providers;

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

  if (imageio_local->providers.indexOf(provider) == INVALID_INDEX)
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

  if (index != INVALID_INDEX)
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
  return imageio_local->providers.indexOf(provider) != INVALID_INDEX;
}

FOG_API List<Provider*> getProviders()
{
  AutoLock locked(imageio_local->lock);
  return imageio_local->providers;
}

FOG_API Provider* getProviderByName(const String& name)
{
  AutoLock locked(imageio_local->lock);
  ImageIO_Local::Providers::ConstIterator it(imageio_local->providers);

  for (; it.isValid(); it.toNext())
  {
    if (it.value()->getName() == name) return it.value();
  }

  return 0;
}

FOG_API Provider* getProviderByExtension(const String& extension)
{
  AutoLock locked(imageio_local->lock);
  ImageIO_Local::Providers::ConstIterator it(imageio_local->providers);

  // Convert extension to lower case
  TemporaryString<16> e;
  e.set(extension);
  e.lower();

  for (it.toStart(); it.isValid(); it.toNext())
  {
    if (it.value()->getExtensions().indexOf(e) != INVALID_INDEX) return it.value();
  }

  return 0;
}

FOG_API Provider* getProviderByMime(void* mem, sysuint_t len)
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

FOG_API DecoderDevice* createDecoderByName(const String& name, err_t* err_)
{
  err_t err = ERR_OK;
  Provider* provider = getProviderByName(name);
  DecoderDevice* decoder = NULL;

  if (!provider) { err = ERR_IMAGEIO_NOT_AVAILABLE_PROVIDER; goto end; }

  decoder = provider->createDecoder();
  if (!decoder) { err = ERR_IMAGEIO_NOT_AVAILABLE_DECODER; goto end; }

end:
  if (err_) *err_ = err;
  return decoder;
}

FOG_API DecoderDevice* createDecoderByExtension(const String& extension, err_t* err_)
{
  err_t err = ERR_OK;
  Provider* provider = getProviderByExtension(extension);
  DecoderDevice* decoder = NULL;

  if (!provider) { err = ERR_IMAGEIO_NOT_AVAILABLE_PROVIDER; goto end; }

  decoder = provider->createDecoder();
  if (!decoder) { err = ERR_IMAGEIO_NOT_AVAILABLE_DECODER; goto end; }

end:
  if (err_) *err_ = err;
  return decoder;
}

FOG_API DecoderDevice* createDecoderForFile(const String& fileName, err_t* err_)
{
  err_t err;

  Stream stream;
  String extension;
  DecoderDevice* decoder = NULL;

  err = stream.openMMap(fileName, false);
  // MMap failed?, try to open in by normal way...
  if (err != ERR_OK) err = stream.openFile(fileName, STREAM_OPEN_READ);

  // If err is not ERR_OK then file can't be open.
  if (err != ERR_OK) goto end;

  // Extract extension to help opening from stream...
  if ((err = FileUtil::extractExtension(extension, fileName)) || (err = extension.lower())) goto end;

  // Finally, open device...
  decoder = createDecoderForStream(stream, extension, &err);

end:
  if (err_) *err_ = err;
  return decoder;
}

FOG_API DecoderDevice* createDecoderForStream(Stream& stream, const String& extension, err_t* err_)
{
  err_t err = ERR_OK;

  Provider* provider = NULL;
  DecoderDevice* decoder = NULL;

  int64_t pos;

  // This is default, 128 bytes should be enough for any image file.
  uint8_t mime[128];
  sysuint_t readn;

  if (!stream.isSeekable()) { err = ERR_IO_CANT_SEEK; goto end; }

  pos = stream.tell();
  readn = stream.read(mime, 128);

  if (stream.seek(pos, STREAM_SEEK_SET) == -1) { err =  ERR_IO_CANT_SEEK; goto end; }

  // First try to use extension, if fail, fallback to readStream(Stream)
  if (!extension.isEmpty()) provider = getProviderByExtension(extension);

  if (!provider) provider = ImageIO::getProviderByMime(mime, readn);
  if (!provider) { err = ERR_IMAGEIO_NOT_AVAILABLE_PROVIDER; goto end; }

  decoder = provider->createDecoder();
  if (!decoder) { err = ERR_IMAGEIO_NOT_AVAILABLE_DECODER; goto end; }

  decoder->attachStream(stream);

end:
  if (err_) *err_ = err;

  // Seek to begin if failed.
  if (err) stream.seek(pos, STREAM_SEEK_SET);

  return decoder;
}

FOG_API EncoderDevice* createEncoderByName(const String& name, err_t* err_)
{
  err_t err = ERR_OK;
  Provider* provider = getProviderByName(name);
  EncoderDevice* decoder = NULL;

  if (!provider) { err = ERR_IMAGEIO_NOT_AVAILABLE_PROVIDER; goto end; }

  decoder = provider->createEncoder();
  if (!decoder) { err = ERR_IMAGEIO_NOT_AVAILABLE_ENCODER; goto end; }

end:
  if (err_) *err_ = err;
  return decoder;
}

FOG_API EncoderDevice* createEncoderByExtension(const String& extension, err_t* err_)
{
  err_t err = ERR_OK;
  Provider* provider = getProviderByExtension(extension);
  EncoderDevice* decoder = NULL;

  if (!provider) { err = ERR_IMAGEIO_NOT_AVAILABLE_PROVIDER; goto end; }

  decoder = provider->createEncoder();
  if (!decoder) { err = ERR_IMAGEIO_NOT_AVAILABLE_ENCODER; goto end; }

end:
  if (err_) *err_ = err;
  return decoder;
}

// ============================================================================
// [Fog::ImageIO::Provider]
// ============================================================================

Provider::Provider()
{
  memset(&_features, 0, sizeof(_features));
  _id = IMAGEIO_FILE_NONE;
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

BaseDevice::BaseDevice(Provider* provider) :
  _provider(provider),
  _deviceType(IMAGEIO_DEVICE_NONE),
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

// [Properties]

err_t BaseDevice::getProperty(const ManagedString& name, Value& value) const
{
  if (name == fog_strings->getString(STR_GRAPHICS_width      )) return value.setInt32(_width);
  if (name == fog_strings->getString(STR_GRAPHICS_height     )) return value.setInt32(_height);
  if (name == fog_strings->getString(STR_GRAPHICS_depth      )) return value.setInt32(_depth);
  if (name == fog_strings->getString(STR_GRAPHICS_planes     )) return value.setInt32(_planes);
  if (name == fog_strings->getString(STR_GRAPHICS_actualFrame)) return value.setInt32(_actualFrame);
  if (name == fog_strings->getString(STR_GRAPHICS_framesCount)) return value.setInt32(_framesCount);
  if (name == fog_strings->getString(STR_GRAPHICS_progress   )) return value.setDouble(_progress);

  return base::getProperty(name, value);
}

err_t BaseDevice::setProperty(const ManagedString& name, const Value& value)
{
  if (name == fog_strings->getString(STR_GRAPHICS_width) ||
      name == fog_strings->getString(STR_GRAPHICS_height) ||
      name == fog_strings->getString(STR_GRAPHICS_depth) ||
      name == fog_strings->getString(STR_GRAPHICS_planes) ||
      name == fog_strings->getString(STR_GRAPHICS_actualFrame) ||
      name == fog_strings->getString(STR_GRAPHICS_framesCount) ||
      name == fog_strings->getString(STR_GRAPHICS_progress))
  {
    return ERR_PROPERTY_IS_READ_ONLY;
  }

  return base::setProperty(name, value);
}

// [Progress]

void BaseDevice::updateProgress(float value)
{
  _progress = value;
}

void BaseDevice::updateProgress(uint32_t y, uint32_t height)
{
  updateProgress((float)( (double)y / (double)height ));
}

// [Dimensions]

bool BaseDevice::areDimensionsZero() const 
{ 
  return _width == 0 || _height == 0;
}

bool BaseDevice::areDimensionsTooLarge() const 
{ 
  // check individual coordinates
  const uint32_t side = 256*256*128;
  if (_width > side || _height > side) return true;

  // check total count of pixels
  uint64_t total = (uint64_t)_width * (uint64_t)_height;
  if (total >= FOG_UINT64_C(1000000000)) return true;

  // ok
  return false;
}

// [Stream]

void BaseDevice::attachStream(Stream& stream)
{
  // detach current stream first
  if (_stream.isOpen()) detachStream();

  _stream = stream;
  _attachedOffset = (uint64_t)_stream.tell();
}

void BaseDevice::detachStream()
{
  if (_stream.isOpen())
  {
    reset();
    _stream.close();
  }
}

// [Reset]

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

  _format = PIXEL_FORMAT_NULL;
  _palette.free();
  _comment.free();
}

// ============================================================================
// [Fog::ImageIO::DecoderDevice]
// ============================================================================

DecoderDevice::DecoderDevice(Provider* provider) :
  BaseDevice(provider),
  _headerDone(false),
  _readerDone(false),
  _headerResult(ERR_OK),
  _readerResult(ERR_OK)
{
  _deviceType = IMAGEIO_DECIDE_DECODER;
}

DecoderDevice::~DecoderDevice()
{
}

// [Reset]

void DecoderDevice::reset()
{
  BaseDevice::reset();

  _headerDone = false;
  _readerDone = false;

  _headerResult = ERR_OK;
  _readerResult = ERR_OK;
}

// ============================================================================
// [EncoderDevice]
// ============================================================================

EncoderDevice::EncoderDevice(Provider* provider) :
  BaseDevice(provider),
  _headerDone(false),
  _writerDone(false)
{
  _deviceType = IMAGEIO_DEVICE_ENCODER;
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
  using namespace Fog;

  // Init imageio_local.
  imageio_local.init();

  // Init all built-in providers.
  fog_imageio_init_bmp();
  fog_imageio_init_gif();
  fog_imageio_init_ico();
  fog_imageio_init_jpeg();
  fog_imageio_init_pcx();
  fog_imageio_init_png();

  return ERR_OK;
}

FOG_INIT_DECLARE void fog_imageio_shutdown(void)
{
  using namespace Fog;

  // Do not need to lock, because we are shutting down. All threads should
  // been already destroyed.
  //
  // Remove (and delete) all providers
  {
    ImageIO_Local::Providers::ConstIterator it(imageio_local->providers);
    for (it.toStart(); it.isValid(); it.toNext()) delete it.value();
  }

  // Shutdown imageio_local.
  imageio_local.destroy();
}
