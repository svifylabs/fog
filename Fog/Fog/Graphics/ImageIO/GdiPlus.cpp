// [Fog/Graphics Library - Public API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Build/Build.h>

#if defined(FOG_OS_WINDOWS)

#include <Fog/Core/Constants.h>
#include <Fog/Core/Library.h>
#include <Fog/Core/ManagedString.h>
#include <Fog/Core/Misc.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/Stream.h>
#include <Fog/Core/String.h>
#include <Fog/Core/Strings.h>
#include <Fog/Core/WinIStream_p.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/ImageIO.h>
#include <Fog/Graphics/ImageIO/GdiPlus_p.h>
#include <Fog/Graphics/RasterEngine_p.h>

namespace Fog {
namespace ImageIO {

// ===========================================================================
// [Fog::ImageIO::GdiPlus - Helpers]
// ===========================================================================

static uint32_t fogFormatFromGpFormat(GpPixelFormat gpFormat)
{
  switch (gpFormat)
  {
    case GpPixelFormat1bppIndexed:
    case GpPixelFormat4bppIndexed:
    case GpPixelFormat8bppIndexed:
      return PIXEL_FORMAT_I8;

    case GpPixelFormat16bppGrayScale:
      return PIXEL_FORMAT_I8;

    case GpPixelFormat16bppRGB555:
    case GpPixelFormat16bppRGB565:
      return PIXEL_FORMAT_XRGB32;

    case GpPixelFormat16bppARGB1555:
      return PIXEL_FORMAT_ARGB32;

    case GpPixelFormat24bppRGB:
    case GpPixelFormat32bppRGB:
      return PIXEL_FORMAT_XRGB32;

    case GpPixelFormat32bppARGB:
      return PIXEL_FORMAT_ARGB32;

    case GpPixelFormat32bppPARGB:
      return PIXEL_FORMAT_PRGB32;

    case GpPixelFormat48bppRGB:
      return PIXEL_FORMAT_XRGB32;

    case GpPixelFormat64bppARGB:
      return PIXEL_FORMAT_ARGB32;

    case GpPixelFormat64bppPARGB:
      return PIXEL_FORMAT_PRGB32;

    default:
      return PIXEL_FORMAT_NULL;
  }
}

static GpPixelFormat gpFormatFromFogFormat(int fogFormat)
{
  switch (fogFormat)
  {
    case PIXEL_FORMAT_PRGB32: return GpPixelFormat32bppPARGB;
    case PIXEL_FORMAT_ARGB32: return GpPixelFormat32bppARGB;
    case PIXEL_FORMAT_XRGB32: return GpPixelFormat32bppRGB;
    case PIXEL_FORMAT_A8    : return GpPixelFormat8bppIndexed;
    case PIXEL_FORMAT_I8    : return GpPixelFormat8bppIndexed;

    default:
      return GpPixelFormatUndefined;
  }
}

// ===========================================================================
// [Fog::ImageIO::GdiPlusLibrary]
// ===========================================================================

GdiPlusLibrary::GdiPlusLibrary() :
  err(0xFFFFFFFF),
  gdiplusToken(0)
{
}

GdiPlusLibrary::~GdiPlusLibrary()
{
  close();
}

err_t GdiPlusLibrary::prepare()
{
  if (err == 0xFFFFFFFF)
  {
    FOG_ONCE_LOCK();
    if (err == 0xFFFFFFFF) err = init();
    FOG_ONCE_UNLOCK();
  }

  return err;
}

err_t GdiPlusLibrary::init()
{
  static const char symbols[] =
    "GdiplusStartup\0"
    "GdiplusShutdown\0"
    "GdipLoadImageFromStream\0"
    "GdipDisposeImage\0"
    "GdipGetImageType\0"
    "GdipGetImageWidth\0"
    "GdipGetImageHeight\0"
    "GdipGetImageFlags\0"
    "GdipGetImagePixelFormat\0"
    "GdipGetImageGraphicsContext\0"
    "GdipImageGetFrameCount\0"
    "GdipImageSelectActiveFrame\0"
    "GdipCreateBitmapFromScan0\0"
    "GdipFlush\0"
    "GdipSetCompositingMode\0"
    "GdipDrawImageI\0"
    "GdipDeleteGraphics\0"
    ;

  if (dll.open(Ascii8("gdiplus")) != ERR_OK)
  {
    // gdiplus.dll not found.
    return ERR_IMAGEIO_GDIPLUS_NOT_LOADED;
  }

  const char* badSymbol;
  if (dll.getSymbols(addr, symbols, FOG_ARRAY_SIZE(symbols), NUM_SYMBOLS, (char**)&badSymbol) != NUM_SYMBOLS)
  {
    // Some symbol failed to load? Inform about it.
    fog_debug("Fog::ImageIO::GdiPlusLibrary::init() - Can't load symbol '%s'.", badSymbol);
    dll.close();
    return ERR_IMAGEIO_GDIPLUS_NOT_LOADED;
  }

  // GdiPlus - Startup.
  GpGdiplusStartupInput startupInput;
  startupInput.GdiplusVersion = 1;
  startupInput.DebugEventCallback = NULL;
  startupInput.SuppressBackgroundThread = false;
  startupInput.SuppressExternalCodecs = false;

  GpStatus status = pGdiplusStartup(&gdiplusToken, &startupInput, NULL);
  if (status != GpOk)
  {
    fog_debug("Fog::ImageIO::GdiPlusLibrary::init() - GdiplusStartup() failed (%u).", status);
    dll.close();
    return ERR_IMAGEIO_GDIPLUS_NOT_LOADED;
  }

  return ERR_OK;
}

void GdiPlusLibrary::close()
{
  // GdiPlus - Shutdown.
  if (err == ERR_OK)
  {
    pGdiplusShutdown(gdiplusToken);
    gdiplusToken = 0;
  }

  dll.close();
  err = 0xFFFFFFFF;
}

// ===========================================================================
// [Fog::ImageIO::GdiPlusProvider]
// ===========================================================================

struct FOG_HIDDEN GdiPlusProvider : public Provider
{
  GdiPlusProvider();
  virtual ~GdiPlusProvider();

  virtual uint32_t check(const void* mem, sysuint_t length);
  virtual EncoderDevice* createEncoder();
  virtual DecoderDevice* createDecoder();

  GdiPlusLibrary _gdiPlusLibrary;
};

GdiPlusProvider::GdiPlusProvider()
{
  // Name of ImageIO Provider.
  _name = fog_strings->getString(STR_GRAPHICS_GDIPLUS);

  // Supported features.
  _features.decoder = true;
  _features.encoder = true;
  _features.proxy = true;

  // Supported extensions.
  _extensions.reserve(7);
  _extensions.append(fog_strings->getString(STR_GRAPHICS_jpg));
  _extensions.append(fog_strings->getString(STR_GRAPHICS_jpeg));
  _extensions.append(fog_strings->getString(STR_GRAPHICS_jfi));
  _extensions.append(fog_strings->getString(STR_GRAPHICS_jfif));
  _extensions.append(fog_strings->getString(STR_GRAPHICS_png));
  _extensions.append(fog_strings->getString(STR_GRAPHICS_tif));
  _extensions.append(fog_strings->getString(STR_GRAPHICS_tiff));
}

GdiPlusProvider::~GdiPlusProvider()
{
}

uint32_t GdiPlusProvider::check(const void* mem, sysuint_t length)
{
  // Note: GdiPlus proxy provider uses 14 as a base score. This is
  // by one less than all other providers based on external 
  // libraries (libpng, libjpeg, libtiff) and reason is that if
  // available these external libraries are used instead.
  if (length == 0) return 0;

  uint32_t score = 0;
  sysuint_t i;

  // Mime data.
  static const uint8_t mimeJPEG[2]    = { 0xFF, 0xD8 };
  static const uint8_t mimePNG[8]     = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
  static const uint8_t mimeTIFF_LE[4] = { 0x49, 0x49, 0x00, 0x42 };
  static const uint8_t mimeTIFF_BE[4] = { 0x4D, 0x4D, 0x42, 0x00 };

  // JPEG check.
  i = Math::min<sysuint_t>(length, 2);
  if (memcmp(mem, mimeJPEG, i) == 0)
    score = Math::max<uint32_t>(score, 14 + ((uint32_t)i * 40));

  // PNG check.
  i = Math::min<sysuint_t>(length, 8);
  if (memcmp(mem, mimePNG, i) == 0)
    score = Math::max<uint32_t>(score, 14 + ((uint32_t)i * 10));

  // TIFF check.
  i = Math::min<sysuint_t>(length, 4);

  if (memcmp(mem, mimeTIFF_LE, i) == 0 || memcmp(mem, mimeTIFF_BE, i) == 0)
    score = Math::max<uint32_t>(score, 14 + ((uint32_t)i * 20));

  return 0;
}

EncoderDevice* GdiPlusProvider::createEncoder()
{
  return (_gdiPlusLibrary.prepare() == ERR_OK) ? new(std::nothrow) GdiPlusEncoderDevice(this) : NULL;
}

DecoderDevice* GdiPlusProvider::createDecoder()
{
  return (_gdiPlusLibrary.prepare() == ERR_OK) ? new(std::nothrow) GdiPlusDecoderDevice(this) : NULL;
}

// ===========================================================================
// [Fog::ImageIO::GdiPlusDecoderDevice]
// ===========================================================================

GdiPlusDecoderDevice::GdiPlusDecoderDevice(Provider* provider) :
  DecoderDevice(provider),
  _istream(NULL),
  _gpImage(NULL)
{
}

GdiPlusDecoderDevice::~GdiPlusDecoderDevice()
{
}

// ===========================================================================
// [Fog::ImageIO::GdiPlusDecoderDevice::attachStream / detachStream]
// ===========================================================================

void GdiPlusDecoderDevice::attachStream(Stream& stream)
{
  _istream = new(std::nothrow) WinIStream(stream);

  base::attachStream(stream);
}

void GdiPlusDecoderDevice::detachStream()
{
  GdiPlusLibrary& gdp = reinterpret_cast<GdiPlusProvider*>(_provider)->_gdiPlusLibrary;

  if (_gpImage) 
  {
    gdp.pGdipDisposeImage(_gpImage);
    _gpImage = NULL;
  }

  if (_istream)
  {
    _istream->Release();
    _istream = NULL;
  }

  base::detachStream();
}

// ===========================================================================
// [Fog::ImageIO::GdiPlusDecoderDevice::reset]
// ===========================================================================

void GdiPlusDecoderDevice::reset()
{
  DecoderDevice::reset();
}

// ===========================================================================
// [Fog::ImageIO::GdiPlusDecoderDevice::readHeader]
// ===========================================================================

err_t GdiPlusDecoderDevice::readHeader()
{
  // Do not read header more than once.
  if (_headerResult) return _headerResult;

  if (_istream == NULL) return ERR_RT_INVALID_HANDLE;
  GdiPlusLibrary& gdp = reinterpret_cast<GdiPlusProvider*>(_provider)->_gdiPlusLibrary;

  GpStatus status = gdp.pGdipLoadImageFromStream(_istream, &_gpImage);
  if (status != GpOk) return (_headerResult = ERR_IMAGEIO_GDIPLUS_ERROR);

  gdp.pGdipGetImageWidth(_gpImage, &_width);
  gdp.pGdipGetImageHeight(_gpImage, &_height);
  _planes = 1;

  GpPixelFormat pf;
  gdp.pGdipGetImagePixelFormat(_gpImage, &pf);

  _format = fogFormatFromGpFormat(pf);
  _depth = Image::formatToDepth(_format);

  return ERR_OK;
}

// ===========================================================================
// [Fog::ImageIO::GdiPlusDecoderDevice::readImage]
// ===========================================================================

err_t GdiPlusDecoderDevice::readImage(Image& image)
{
  err_t err = ERR_OK;

  if (_istream == NULL) return ERR_RT_INVALID_HANDLE;
  GdiPlusLibrary& gdp = reinterpret_cast<GdiPlusProvider*>(_provider)->_gdiPlusLibrary;

  GpBitmap* bm = NULL;
  GpGraphics* gr = NULL;
  GpStatus status;

  // Read image header.
  if (readHeader() != ERR_OK) return _headerResult;

  // Don't read image more than once.
  if (isReaderDone()) return (_readerResult = ERR_IMAGEIO_NO_MORE_FRAMES);

  // Create image.
  if ((err = image.create(_width, _height, _format)) != ERR_OK) return err;

  // Create GpBitmap that will share raster data with our image.
  status = gdp.pGdipCreateBitmapFromScan0(
    (INT)image.getWidth(),
    (INT)image.getHeight(), 
    (INT)image.getStride(),
    gpFormatFromFogFormat(image.getFormat()),
    (BYTE*)image.getMData(),
    &bm);
  if (status != GpOk) { err = ERR_IMAGEIO_GDIPLUS_ERROR; goto end; }

  // Create GpGraphics context.
  status = gdp.pGdipGetImageGraphicsContext((GpImage*)bm, &gr);
  if (status != GpOk) { err = ERR_IMAGEIO_GDIPLUS_ERROR; goto end; }

  // Set compositing to source copy (we want alpha bits).
  status = gdp.pGdipSetCompositingMode(gr, GpCompositingModeSourceCopy);
  if (status != GpOk) { err = ERR_IMAGEIO_GDIPLUS_ERROR; goto end; }

  // Draw streamed image to GpGraphics context.
  status = gdp.pGdipDrawImageI(gr, _gpImage, 0, 0);
  if (status != GpOk) { err = ERR_IMAGEIO_GDIPLUS_ERROR; goto end; }

  // flush (this step is probably not necessary).
  status = gdp.pGdipFlush(gr, GpFlushIntentionSync);
  if (status != GpOk) { err = ERR_IMAGEIO_GDIPLUS_ERROR; goto end; }

end:
  // Delete created Gdi+ objects.
  if (gr) gdp.pGdipDeleteGraphics(gr);
  if (bm) gdp.pGdipDisposeImage((GpImage*)bm);

  if (err == ERR_OK) updateProgress(1.0);
  return (_readerResult = err);
}

// ===========================================================================
// [Fog::ImageIO::GdiPlusEncoderDevice]
// ===========================================================================

GdiPlusEncoderDevice::GdiPlusEncoderDevice(Provider* provider) :
  EncoderDevice(provider)
{
}

GdiPlusEncoderDevice::~GdiPlusEncoderDevice()
{
}

// ===========================================================================
// [Fog::ImageIO::GdiPlusEncoderDevice::attachStream / detachStream]
// ===========================================================================

void GdiPlusEncoderDevice::attachStream(Stream& stream)
{
  _istream = new(std::nothrow) WinIStream(stream);

  base::attachStream(stream);
}

void GdiPlusEncoderDevice::detachStream()
{
  if (_istream)
  {
    _istream->Release();
    _istream = NULL;
  }

  base::detachStream();
}

// ===========================================================================
// [Fog::ImageIO::GdiPlusEncoderDevice::reset]
// ===========================================================================

void GdiPlusEncoderDevice::reset()
{
  EncoderDevice::reset();
}

// ===========================================================================
// [Fog::ImageIO::GdiPlusEncoderDevice::writeImage]
// ===========================================================================

err_t GdiPlusEncoderDevice::writeImage(const Image& image)
{
  

  return ERR_RT_NOT_IMPLEMENTED;
}

// ===========================================================================
// [Fog::ImageIO::GdiPlusEncoderDevice::getProperty / setProperty]
// ===========================================================================

err_t GdiPlusEncoderDevice::getProperty(const ManagedString& name, Value& value) const
{
  return base::getProperty(name, value);
}

err_t GdiPlusEncoderDevice::setProperty(const ManagedString& name, const Value& value)
{
  return base::setProperty(name, value);
}

} // ImageIO namespace
} // Fog namespace

FOG_IMPLEMENT_OBJECT(Fog::ImageIO::GdiPlusDecoderDevice)
FOG_IMPLEMENT_OBJECT(Fog::ImageIO::GdiPlusEncoderDevice)

// ===========================================================================
// [CAPI]
// ===========================================================================

FOG_INIT_DECLARE void fog_imageio_init_gdiplus(void)
{
  using namespace Fog;

  ImageIO::GdiPlusProvider* provider = new(std::nothrow) ImageIO::GdiPlusProvider();
  if (provider) ImageIO::addProvider(provider);
}

#endif // FOG_OS_WINDOWS
