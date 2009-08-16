// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled Headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/Lazy.h>
#include <Fog/Core/Library.h>
#include <Fog/Core/ManagedString.h>
#include <Fog/Core/Misc.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/Stream.h>
#include <Fog/Core/String.h>
#include <Fog/Core/Strings.h>
#include <Fog/Graphics/Error.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/ImageIO/ImageIO_PNG.h>
#include <Fog/Graphics/Raster.h>

#if defined(FOG_HAVE_PNG_H)
#define FOG_HAVE_PNG_HEADERS
#endif // FOG_HAVE_PNG_H

#if defined(FOG_HAVE_PNG_HEADERS)
#include <stdio.h>
#include <string.h>

#include <png.h>
#include <setjmp.h>
#endif // FOG_HAVE_PNG_HEADERS

#if defined(FOG_HAVE_PNG_HEADERS)

namespace Fog {
namespace ImageIO {

// ============================================================================
// [Fog::ImageIO::PngLibrary]
// ============================================================================

struct PngLibrary
{
  PngLibrary() : ok(false)
  {
    static const char symbols[] =
      "png_error\0"
      "png_create_read_struct\0"
      "png_destroy_read_struct\0"
      "png_create_info_struct\0"
      "png_read_info\0"
      "png_read_rows\0"
      "png_read_image\0"
      "png_read_end\0"
      "png_create_write_struct\0"
      "png_destroy_write_struct\0"
      "png_write_info\0"
      "png_write_rows\0"
      "png_write_end\0"
      "png_set_gray_1_2_4_to_8\0"
      "png_set_gray_to_rgb\0"
      "png_set_strip_16\0"
      "png_set_swap_alpha\0"
      "png_set_filler\0"
      "png_set_packing\0"
      "png_set_packswap\0"
      "png_set_shift\0"
      "png_set_error_fn\0"
      "png_set_read_fn\0"
      "png_set_write_fn\0"
      "png_set_bgr\0"
      "png_set_expand\0"
      "png_set_interlace_handling\0"
      "png_set_compression_level\0"
      "png_set_IHDR\0"
      "png_set_sBIT\0"
      "png_get_valid\0"
      "png_get_bit_depth\0"
      "png_get_IHDR\0";

    if (dll.open(Ascii8("png"), Library::OpenSystemPrefix | Library::OpenSystemSuffix) != Error::Ok)
    {
      return;
    }

    const char* badSymbol;
    if (dll.getSymbols(addr, symbols, FOG_ARRAY_SIZE(symbols), SymbolsCount, (char**)NULL) != SymbolsCount)
    {
      // Some symbol failed to load? Inform about it.
      fog_debug("Fog::ImageIO::PngLibrary() - Can't load symbol '%s' from libpng", badSymbol);
      dll.close();
      return;
    }

    ok = 1;
  }

  enum { SymbolsCount = 33 };

  union
  {
    struct
    {
      FOG_CDECL void (*png_error)(png_structp png_ptr, png_const_charp error_message);
      FOG_CDECL png_structp (*png_create_read_struct)(png_const_charp user_png_ver, png_voidp error_ptr, png_error_ptr error_fn, png_error_ptr warn_fn);
      FOG_CDECL void (*png_destroy_read_struct)(png_structpp png_ptr_ptr, png_infopp info_ptr_ptr, png_infopp end_info_ptr_ptr);
      FOG_CDECL png_infop (*png_create_info_struct)(png_structp png_ptr);
      FOG_CDECL void (*png_read_info)(png_structp png_ptr, png_infop info_ptr);
      FOG_CDECL void(*png_read_rows)(png_structp png_ptr, png_bytepp row, png_bytepp display_row, png_uint_32 num_rows);
      FOG_CDECL void (*png_read_image)(png_structp png_ptr, png_bytepp image);
      FOG_CDECL void (*png_read_end)(png_structp png_ptr, png_infop info_ptr);
      FOG_CDECL png_structp (*png_create_write_struct)(png_const_charp user_png_ver, png_voidp error_ptr, png_error_ptr error_fn, png_error_ptr warn_fn);
      FOG_CDECL void (*png_destroy_write_struct)(png_structpp png_ptr_ptr, png_infopp info_ptr_ptr);
      FOG_CDECL void (*png_write_info)(png_structp png_ptr, png_infop info_ptr);
      FOG_CDECL void (*png_write_rows)(png_structp png_ptr, png_bytepp row, png_uint_32 num_rows);
      FOG_CDECL void (*png_write_end)(png_structp png_ptr, png_infop info_ptr);
      FOG_CDECL void (*png_set_gray_1_2_4_to_8)(png_structp png_ptr);
      FOG_CDECL void (*png_set_gray_to_rgb)(png_structp png_ptr);
      FOG_CDECL void (*png_set_strip_16)(png_structp png_ptr);
      FOG_CDECL void (*png_set_swap_alpha)(png_structp png_ptr);
      FOG_CDECL void (*png_set_filler)(png_structp png_ptr, png_uint_32 filler, int flags);
      FOG_CDECL void (*png_set_packing)(png_structp png_ptr);
      FOG_CDECL void (*png_set_packswap)(png_structp png_ptr);
      FOG_CDECL void (*png_set_shift)(png_structp png_ptr, png_color_8p true_bits);
      FOG_CDECL void (*png_set_error_fn)(png_structp png_ptr, png_voidp error_ptr, png_error_ptr error_fn, png_error_ptr warning_fn);
      FOG_CDECL void (*png_set_read_fn)(png_structp png_ptr, png_voidp io_ptr, png_rw_ptr read_data_fn);
      FOG_CDECL void (*png_set_write_fn)(png_structp png_ptr, png_voidp io_ptr, png_rw_ptr write_data_fn, png_flush_ptr output_flush_fn);
      FOG_CDECL void (*png_set_bgr)(png_structp png_ptr);
      FOG_CDECL void (*png_set_expand)(png_structp png_ptr);
      FOG_CDECL int (*png_set_interlace_handling)(png_structp png_ptr);
      FOG_CDECL void (*png_set_compression_level)(png_structp png_ptr, int level);
      FOG_CDECL void (*png_set_IHDR)(png_structp png_ptr,
        png_infop info_ptr, png_uint_32 width, png_uint_32 height, int bit_depth,
        int color_type, int interlace_method, int compression_method,
        int filter_method);
      FOG_CDECL void (*png_set_sBIT)(png_structp png_ptr, png_infop info_ptr, png_color_8p sig_bit);

      FOG_CDECL png_uint_32 (*png_get_valid)(png_structp png_ptr, png_infop info_ptr, png_uint_32 flag);
      FOG_CDECL png_byte (*png_get_bit_depth)(png_structp png_ptr, png_infop info_ptr);
      FOG_CDECL png_uint_32 (*png_get_IHDR)(png_structp png_ptr,
        png_infop info_ptr, png_uint_32 *width, png_uint_32 *height,
        int *bit_depth, int *color_type, int *interlace_method,
        int *compression_method, int *filter_method);
    };
    void *addr[SymbolsCount];
  };

  Library dll;
  uint32_t ok;
};

static Static< Lazy<PngLibrary> > _png;

// ============================================================================
// [Fog::ImageIO::PngProvider]
// ============================================================================

struct PngProvider : public Provider
{
  PngProvider();
  virtual ~PngProvider();

  virtual uint32_t check(const void* mem, sysuint_t length);
  virtual EncoderDevice* createEncoder();
  virtual DecoderDevice* createDecoder();
};

PngProvider::PngProvider()
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
  _features.compressionTypeAdjust = true;

  // name
  _name = fog_strings->getString(STR_GRAPHICS_PNG);

  // extensions
  _extensions.reserve(1);
  _extensions.append(fog_strings->getString(STR_GRAPHICS_png));
}

PngProvider::~PngProvider()
{
}

uint32_t PngProvider::check(const void* mem, sysuint_t length)
{
  const uint8_t mime[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
  sysuint_t s = (length < 8) ? length : 8;

  if (memcmp(mem, mime, s) == 0) 
    return 10 + (s * 10);
  else
    return 0;
}

EncoderDevice* PngProvider::createEncoder()
{
  return _png->get()->ok ? new(std::nothrow) PngEncoderDevice() : NULL;
}

DecoderDevice* PngProvider::createDecoder()
{
  return _png->get()->ok ? new(std::nothrow) PngDecoderDevice() : NULL;
}

static void user_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
  if (((Stream *)png_ptr->io_ptr)->read(data, length) != length)
  {
    _png->get()->png_error(png_ptr, "Read Error");
  }
}

static void user_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
  if (((Stream *)png_ptr->io_ptr)->write(data, length) != length)
  {
    _png->get()->png_error(png_ptr, "Write Error");
  }
}

static void user_flush_data(png_structp png_ptr)
{
}

static void user_error_fn(png_structp png_ptr, png_const_charp error_msg)
{
  longjmp(png_ptr->jmpbuf, 1);
}

// ============================================================================
// [Fog::ImageIO::PngDecoderDevice]
// ============================================================================

PngDecoderDevice::PngDecoderDevice() :
  _png_ptr(NULL),
  _png_info_ptr(NULL)
{
}

PngDecoderDevice::~PngDecoderDevice()
{
  _deletePngStream();
}

// ============================================================================
// [Fog::ImageIO::PngDecoderDevice::reset]
// ============================================================================

void PngDecoderDevice::reset()
{
  _deletePngStream();
  DecoderDevice::reset();
}

// ============================================================================
// [Fog::ImageIO::PngDecoderDevice::readHeader]
// ============================================================================

uint32_t PngDecoderDevice::readHeader()
{
  // Don't read header more than once.
  if (headerDone()) return headerResult();

  // Png library pointer
  PngLibrary* lib = _png->get();
  if (!lib->ok) return Error::ImageIO_PngLibraryNotFound;

  // don't read header more than once
  if (headerDone()) return headerResult();

  // mark header as done
  _headerDone = true;

  png_uint_32 w32, h32;

  if ((_headerResult = _createPngStream()) != Error::Ok)
  {
    return headerResult();
  }

  if (setjmp(((png_structp)_png_ptr)->jmpbuf))
  {
    return (_headerResult = Error::ImageIO_PngError);
  }

  lib->png_read_info((png_structp)_png_ptr, (png_infop)_png_info_ptr);
  lib->png_get_IHDR((png_structp)_png_ptr, (png_infop)_png_info_ptr,
    (png_uint_32 *)(&w32),
    (png_uint_32 *)(&h32),
    &_png_bit_depth,
    &_png_color_type,
    &_png_interlace_type, NULL, NULL);

  _width = w32;
  _height = h32;
  _depth = _png_bit_depth;
  _planes = 1;

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

  // png contains only one image
  _actualFrame = 0;
  _framesCount = 1;

  // Setup pixel format.
  png_infop info_ptr = (png_infop)_png_info_ptr;

  if (info_ptr->color_type == PNG_COLOR_TYPE_PALETTE)
  {
    _format = Image::FormatI8;
  }
  else if (
      info_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA ||
      info_ptr->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
  {
    _format = Image::FormatARGB32;
  }
  else
  {
    _format = Image::FormatRGB32;
  }

  // Success
  return (_headerResult = Error::Ok);
}

// ============================================================================
// [Fog::ImageIO::PngDecoderDevice::readImage]
// ============================================================================

uint32_t PngDecoderDevice::readImage(Image& image)
{
  // Png library pointer
  PngLibrary* lib = _png->get();
  if (!lib->ok) return Error::ImageIO_PngLibraryNotFound;

  // read png header
  if (readHeader() != Error::Ok) return headerResult();

  // don't read image more than once
  if (readerDone()) return (_readerResult = Error::ImageIO_NotAnimationFormat);

  // error code (default is success)
  uint32_t err = Error::Ok;

  // variables
  png_structp png_ptr = (png_structp)_png_ptr;
  png_infop info_ptr = (png_infop)_png_info_ptr;
  png_uint_32 i;
  bool hasAlpha = false;
  bool hasGrey = false;
  uint8_t *p;

  if (setjmp(png_ptr->jmpbuf))
  {
    return Error::ImageIO_PngError;
  }

  // Change the order of packed pixels to least significant bit first.
  lib->png_set_packswap(png_ptr);

  // Tell libpng to strip 16 bit/color files down to 8 bits/color.
  lib->png_set_strip_16(png_ptr);

  if (info_ptr->color_type == PNG_COLOR_TYPE_RGB_ALPHA)
  {
    hasAlpha = true;
  }

  if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
  {
    hasAlpha = true;
    hasGrey = true;
  }

  if (info_ptr->color_type == PNG_COLOR_TYPE_GRAY)
  {
    hasGrey = true;
  }

  if (hasAlpha) lib->png_set_expand(png_ptr);

  // Extract multiple pixels with bit depths of 1, 2, and 4 from a single
  // byte into separate bytes (useful for paletted and grayscale images).
  lib->png_set_packing(png_ptr);

  if (info_ptr->color_type != PNG_COLOR_TYPE_PALETTE)
  {
    // We want ARGB
#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
    lib->png_set_bgr(png_ptr);
    lib->png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
#else
    lib->png_set_swap_alpha(png_ptr);
    lib->png_set_filler(png_ptr, 0xFF, PNG_FILLER_BEFORE);
#endif // FOG_BYTE_ORDER

    if (lib->png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
    {
      lib->png_set_expand(png_ptr);
    }

    if (hasGrey)
    {
      lib->png_set_gray_to_rgb(png_ptr);
      if (lib->png_get_bit_depth(png_ptr, info_ptr) < 8)
      {
        lib->png_set_gray_1_2_4_to_8(png_ptr);
      }
    }
  }

  if ((err = image.create(width(), height(), _format))) goto end;

  {
    int pass, number_passes = lib->png_set_interlace_handling(png_ptr);
    int y = 0;
    int yi = 0;
    int ytotal = number_passes * height();

    for (pass = 0; pass < number_passes; pass++)
    {
      for (i = 0; i < height(); i++, y++, yi++)
      {
        uint8_t* row = image._d->first + i * image.getStride();
        lib->png_read_rows(png_ptr, &row, NULL, 1);
        if ((yi & 15) == 0) updateProgress(yi, ytotal);
      }
    }
  }

  // TODO: PNG - Read palette

end:
  lib->png_read_end(png_ptr, info_ptr);

  updateProgress(1.0);
  return err;
}

uint32_t PngDecoderDevice::_createPngStream()
{
  // already created ?
  if (_png_ptr) return Error::Ok;

  // Png library pointer
  PngLibrary* lib = _png->get();
  // Should be checked earlier
  FOG_ASSERT(lib->ok);

  // create png structure
  if ((_png_ptr = lib->png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL)
  {
    return Error::ImageIO_PngError;
  }

  // create info structure
  if ((_png_info_ptr = lib->png_create_info_struct((png_structp)_png_ptr)) == NULL)
  {
    goto fail;
  }

  // custom io
  lib->png_set_read_fn((png_structp)_png_ptr, &_stream, (png_rw_ptr)user_read_data);

  // success
  return Error::Ok;

fail:
  _deletePngStream();
  return Error::ImageIO_PngError;
}

void PngDecoderDevice::_deletePngStream()
{
  if (_png_ptr)
  {
    // Png library pointer
    PngLibrary* lib = _png->get();
    // Should be checked earlier
    FOG_ASSERT(lib->ok);

    lib->png_destroy_read_struct((png_structp*)&_png_ptr, (png_infop*)&_png_info_ptr, (png_infopp)NULL);
  }
}

// ============================================================================
// [Fog::ImageIO::PngEncoder]
// ============================================================================

PngEncoderDevice::PngEncoderDevice()
{
}

PngEncoderDevice::~PngEncoderDevice()
{
}

uint32_t PngEncoderDevice::writeImage(const Image& image)
{
  // Png library pointer
  PngLibrary* lib = _png->get();
  if (!lib->ok) return Error::ImageIO_PngLibraryNotFound;

  uint32_t err = Error::Ok;

  int  format = image.getFormat();

  MemoryBuffer<2048> bufferStorage;
  uint8_t* buffer = NULL;

  png_structp  png_ptr;
  png_infop    info_ptr;
  int          width = image.getWidth();
  int          height = image.getHeight();
  int          y;
  png_bytep    row_ptr;
  int          compression;

  // Step 0: Simple reject
  if (!width || !height)
  {
    err = Error::ImageSizeIsInvalid;
    goto end;
  }

  if ((png_ptr = lib->png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL)
  {
    err = Error::ImageIO_PngError;
    goto end;
  }

  if ((info_ptr = lib->png_create_info_struct(png_ptr)) == NULL)
  {
    err = Error::ImageIO_PngError;
    goto end_destroy_write_struct;
  }

  if (setjmp(png_ptr->jmpbuf))
  {
    err = Error::ImageIO_PngError;
    goto end_destroy_write_struct;
  }

  // Use custom I/O functions.
  lib->png_set_write_fn(png_ptr, &_stream, (png_rw_ptr)user_write_data, (png_flush_ptr)user_flush_data);

  png_color_8 sig_bit;
  memset(&sig_bit, 0, sizeof(sig_bit));

  switch (format)
  {
    case Image::FormatARGB32:
    case Image::FormatPRGB32:
    {
      lib->png_set_IHDR(png_ptr, info_ptr, width, height, 8,
        PNG_COLOR_TYPE_RGB_ALPHA, png_ptr->interlaced,
        PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
      lib->png_set_bgr(png_ptr);
#else
      lib->png_set_swap_alpha(png_ptr);
#endif

      sig_bit.red = 8;
      sig_bit.green = 8;
      sig_bit.blue = 8;
      sig_bit.alpha = 8;
      sig_bit.gray = 0;
      lib->png_set_sBIT(png_ptr, info_ptr, &sig_bit);

      buffer = (uint8_t*)bufferStorage.alloc(width * 4);
      if (!buffer) { err = Error::OutOfMemory; goto end; }
      break;
    }
    case Image::FormatRGB32:
    case Image::FormatRGB24:
    {
      lib->png_set_IHDR(png_ptr, info_ptr, width, height, 8,
        PNG_COLOR_TYPE_RGB, png_ptr->interlaced,
        PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

      sig_bit.red = 8;
      sig_bit.green = 8;
      sig_bit.blue = 8;
      sig_bit.alpha = 0;
      sig_bit.gray = 0;
      lib->png_set_sBIT(png_ptr, info_ptr, &sig_bit);

      buffer = (uint8_t*)bufferStorage.alloc(width * 3);
      if (!buffer) { err = Error::OutOfMemory; goto end; }
      break;
    }

    case Image::FormatA8:
    case Image::FormatI8:
      // TODO: PNG - Write A8 and I8 images
      break;
  }

  compression = 5;

  if (compression < 0) compression = 0;
  if (compression > 9) compression = 9;

  lib->png_set_compression_level(png_ptr, compression);
  lib->png_write_info(png_ptr, info_ptr);
  lib->png_set_shift(png_ptr, &sig_bit);
  lib->png_set_packing(png_ptr);

  for (y = 0; y < height; y++)
  {
    switch (format)
    {
      case Image::FormatARGB32:
      case Image::FormatPRGB32:
        row_ptr = (png_bytep)buffer;
        image.getDibArgb32(0, y, width, buffer);
        break;
      case Image::FormatRGB32:
      case Image::FormatRGB24:
        row_ptr = (png_bytep)buffer;
        image.getDibRgb24_be(0, y, width, buffer);
        break;
      case Image::FormatI8:
        row_ptr = (png_bytep)image.cFirst() + y * image.getStride();
        break;
    }

    lib->png_write_rows(png_ptr, &row_ptr, 1);

    if ((y & 15) == 0) updateProgress(y, height);
  }

  lib->png_write_end(png_ptr, info_ptr);

end_destroy_write_struct:
  lib->png_destroy_write_struct(&png_ptr, (png_infopp)&info_ptr);
end:
  updateProgress(1.0);
  return err;
}

} // ImageIO namespace
} // Fog namespace

// ============================================================================
// [CAPI]
// ============================================================================

FOG_INIT_DECLARE void fog_imageio_png_init(void)
{
  using namespace Fog;

  ImageIO::_png.init();
  ImageIO::addProvider(new(std::nothrow) ImageIO::PngProvider());
}

FOG_INIT_DECLARE void fog_imageio_png_shutdown(void)
{
  using namespace Fog;

  // Provider is destroyed by Fog::ImageIO, need only to destroy PngLibrary
  // if open.
  ImageIO::_png.destroy();
}

#else

FOG_INIT_DECLARE void fog_imageio_png_init(void) {}
FOG_INIT_DECLARE void fog_imageio_png_shutdown(void) {}

#endif // FOG_HAVE_PNG_HEADERS
