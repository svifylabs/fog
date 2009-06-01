// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/Lazy.h>
#include <Fog/Core/Library.h>
#include <Fog/Core/Misc.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/Stream.h>
#include <Fog/Core/String.h>
#include <Fog/Core/StringCache.h>
#include <Fog/Core/Strings.h>
#include <Fog/Graphics/Error.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/ImageIO/ImageIO_JPEG.h>
#include <Fog/Graphics/Raster.h>

#if defined(FOG_HAVE_JPEGLIB_H)
#define FOG_HAVE_JPEG_HEADERS
#endif // FOG_HAVE_JPEGLIB_H

#if defined FOG_HAVE_JPEG_HEADERS

#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#include <jpeglib.h>
#include <jerror.h>

namespace Fog {
namespace ImageIO {

// ===========================================================================
// [Fog::ImageIO::JpegLibrary]
// ===========================================================================

struct JpegLibrary
{
  JpegLibrary() : ok(false)
  {
    static const char symbols[] =
      "jpeg_write_scanlines\0"
      "jpeg_read_scanlines\0"
      "jpeg_set_defaults\0"
      "jpeg_set_quality\0"
      "jpeg_std_error\0"
      "jpeg_read_header\0"
      "jpeg_calc_output_dimensions\0"
      "jpeg_start_compress\0"
      "jpeg_start_decompress\0"
      "jpeg_CreateCompress\0"
      "jpeg_CreateDecompress\0"
      "jpeg_finish_compress\0"
      "jpeg_finish_decompress\0"
      "jpeg_resync_to_restart\0"
      "jpeg_destroy_compress\0"
      "jpeg_destroy_decompress\0";

    if (dll.open(StubAscii8("jpeg"), Library::OpenSystemPrefix | Library::OpenSystemSuffix) != Error::Ok)
    {
      // No JPEG library found.
      return;
    }

    const char* badSymbol;
    if (dll.symbols(addr, symbols, FOG_ARRAY_SIZE(symbols), SymbolsCount, (char**)&badSymbol) != SymbolsCount)
    {
      // Some symbol failed to load? Inform about it.
      fog_debug("Fog::ImageIO::JpegLibrary() - Can't load symbol '%s' from libjpeg", badSymbol);
      dll.close();
      return;
    }

    ok = 1;
  }

  enum { SymbolsCount = 16 };

  union
  {
    struct
    {
      FOG_CDECL JDIMENSION (*jpeg_write_scanlines)(jpeg_compress_struct*, uint8_t**, unsigned int);
      FOG_CDECL JDIMENSION (*jpeg_read_scanlines)(jpeg_decompress_struct*, uint8_t**, unsigned int);
      FOG_CDECL void (*jpeg_set_defaults)(jpeg_compress_struct*);
      FOG_CDECL void (*jpeg_set_quality)(jpeg_compress_struct*, int /* quality */, int /* force_baseline*/);
      FOG_CDECL struct jpeg_error_mgr* (*jpeg_std_error)(jpeg_error_mgr*);
      FOG_CDECL int (*jpeg_read_header)(jpeg_decompress_struct*, int);
      FOG_CDECL void (*jpeg_calc_output_dimensions)(jpeg_decompress_struct*);
      FOG_CDECL int (*jpeg_start_compress)(jpeg_compress_struct*, int);
      FOG_CDECL int (*jpeg_start_decompress)(jpeg_decompress_struct*);
      FOG_CDECL JDIMENSION (*jpegCreateCompress)(jpeg_compress_struct*, int, size_t);
      FOG_CDECL JDIMENSION (*jpegCreateDecompress)(jpeg_decompress_struct*, int, size_t);
      FOG_CDECL int (*jpeg_finish_compress)(jpeg_compress_struct*);
      FOG_CDECL int (*jpeg_finish_decompress)(jpeg_decompress_struct*);
      FOG_CDECL int (*jpeg_resync_to_restart)(jpeg_decompress_struct*, int);
      FOG_CDECL void (*jpeg_destroy_compress)(jpeg_compress_struct*);
      FOG_CDECL void (*jpeg_destroy_decompress)(jpeg_decompress_struct*);
    };
    void *addr[SymbolsCount];
  };

  Library dll;
  uint32_t ok;
};

static Static< Lazy<JpegLibrary> > _jpeg;

// ===========================================================================
// [Fog::ImageIO::JpegProvider]
// ===========================================================================

struct JpegProvider : public Provider
{
  JpegProvider();
  virtual ~JpegProvider();

  virtual uint32_t check(const void* mem, sysuint_t length);
  virtual EncoderDevice* createEncoder();
  virtual DecoderDevice* createDecoder();
};

JpegProvider::JpegProvider()
{
  // features
  _features.decoder = true;
  _features.encoder = true;

  _features.rgb24 = true;

  // name
  _name = fog_strings->get(STR_GRAPHICS_JPEG);

  // extensions
  _extensions.reserve(4);
  _extensions.append(fog_strings->get(STR_GRAPHICS_jpg));
  _extensions.append(fog_strings->get(STR_GRAPHICS_jpeg));
  _extensions.append(fog_strings->get(STR_GRAPHICS_jfi));
  _extensions.append(fog_strings->get(STR_GRAPHICS_jfif));
}

JpegProvider::~JpegProvider()
{
}

uint32_t JpegProvider::check(const void* mem, sysuint_t length)
{
  if (length == 0) return 0;

  const uint8_t* m = (const uint8_t*)mem;

  if (length >= 2)
  {
    if (m[0] == 0xFF && m[1] == 0xD8) return 90;
  }

  return 0;
}

EncoderDevice* JpegProvider::createEncoder()
{
  return _jpeg->get()->ok ? new(std::nothrow) JpegEncoderDevice() : NULL;
}

DecoderDevice* JpegProvider::createDecoder()
{
  return _jpeg->get()->ok ? new(std::nothrow) JpegDecoderDevice() : NULL;
}

// ===========================================================================
// [Fog::ImageIO::JpegDecoderDevice]
// ===========================================================================

#define INPUT_BUFFER_SIZE 4096

struct MyJpegSourceMgr
{
  struct jpeg_source_mgr pub;
  Stream* stream;
  uint8_t buffer[INPUT_BUFFER_SIZE];
};

// Initialize source
// called by jpeg_read_header before any data is actually read.
static FOG_CDECL void MyJpegInitSource(j_decompress_ptr cinfo)
{
  // We don't actually need to do anything
  return;
}

static FOG_CDECL int MyJpegFillInputBuffer(j_decompress_ptr cinfo)
{
  MyJpegSourceMgr* src = (MyJpegSourceMgr*)cinfo->src;
  sysuint_t nbytes;

  nbytes = src->stream->read(src->buffer, INPUT_BUFFER_SIZE);
  if (nbytes <= 0)
  {
    // insert a fake EOI marker
    src->buffer[0] = (JOCTET)0xFF;
    src->buffer[1] = (JOCTET)JPEG_EOI;
    nbytes = 2;
  }

  src->pub.next_input_byte = src->buffer;
  src->pub.bytes_in_buffer = nbytes;

  return true;
}

// Skip data
// used to skip over a potentially large amount of uninteresting data (such
// as an APPn marker).
//
// Writers of suspendable-input applications must note that skip_input_data
// is not granted the right to give a suspension return.  If the skip extends
// beyond the data currently in the buffer, the buffer can be marked empty so
// that the next read will cause a fill_input_buffer call that can suspend.
// Arranging for additional bytes to be discarded before reloading the input
// buffer is the application writer's problem.
static FOG_CDECL void MyJpegSkipInputData(j_decompress_ptr cinfo, long num_bytes)
{
  MyJpegSourceMgr* src = (MyJpegSourceMgr*) cinfo->src;
  sysint_t remain = (sysint_t)( (uint8_t *)src->buffer + INPUT_BUFFER_SIZE - (uint8_t*)src->pub.next_input_byte );

  if (num_bytes < remain)
  {
    src->pub.next_input_byte += num_bytes;
    src->pub.bytes_in_buffer -= num_bytes;
  }
  else
  {
    src->stream->seek(num_bytes - remain, Stream::SeekCur);

    sysuint_t nbytes = src->stream->read(src->buffer, INPUT_BUFFER_SIZE);
    src->pub.next_input_byte = src->buffer;
    src->pub.bytes_in_buffer = nbytes;
  }
}

// Terminate source
// called by jpeg_finish_decompress after all data has been read.
static FOG_CDECL void MyJpegTermSource(j_decompress_ptr cinfo)
{
  // we don't actually need to do anything
  return;
}

struct MyJpegErrorMgr
{
  struct jpeg_error_mgr errmgr;
  jmp_buf escape;
};

static FOG_CDECL void MyJpegErrorExit(j_common_ptr cinfo)
{
  MyJpegErrorMgr* err = (MyJpegErrorMgr*)cinfo->err;
  longjmp(err->escape, 1);
}

static FOG_CDECL void MyJpegMessage(j_common_ptr cinfo)
{
  FOG_UNUSED(cinfo);
}

JpegDecoderDevice::JpegDecoderDevice()
{
}

JpegDecoderDevice::~JpegDecoderDevice()
{
}

// ===========================================================================
// [Fog::ImageIO::JpegDecoderDevice::reset]
// ===========================================================================

void JpegDecoderDevice::reset()
{
  DecoderDevice::reset();
}

// ===========================================================================
// [Fog::ImageIO::JpegDecoderDevice::readHeader]
// ===========================================================================

uint32_t JpegDecoderDevice::readHeader()
{
  // Don't read header more than once.
  if (headerDone()) return headerResult();

  JpegLibrary* jpeg = _jpeg->get();
  if (!jpeg->ok)
  {
    return (_headerResult = Error::ImageIO_JpegLibraryNotFound);
  }

  struct jpeg_decompress_struct cinfo;
  MyJpegSourceMgr srcmgr;
  MyJpegErrorMgr jerr;

  // create a decompression structure and load the header
  cinfo.err = jpeg->jpeg_std_error(&jerr.errmgr);
  jerr.errmgr.error_exit = MyJpegErrorExit;
  jerr.errmgr.output_message = MyJpegMessage;

  if (setjmp(jerr.escape))
  {
    // error
    jpeg->jpeg_destroy_decompress(&cinfo);
    return (_headerResult = Error::ImageIO_JpegError);
  }

  jpeg->jpegCreateDecompress(&cinfo, /* version */ 62, sizeof(struct jpeg_decompress_struct));

  cinfo.src = (struct jpeg_source_mgr *)&srcmgr;
  srcmgr.pub.init_source = MyJpegInitSource;
  srcmgr.pub.fill_input_buffer = MyJpegFillInputBuffer;
  srcmgr.pub.skip_input_data = MyJpegSkipInputData;
  srcmgr.pub.resync_to_restart = jpeg->jpeg_resync_to_restart;
  srcmgr.pub.term_source = MyJpegTermSource;
  srcmgr.pub.next_input_byte = srcmgr.buffer;
  srcmgr.pub.bytes_in_buffer = 0;
  srcmgr.stream = &_stream;

  jpeg->jpeg_read_header(&cinfo, true);
  jpeg->jpeg_calc_output_dimensions(&cinfo);

  _headerDone = true;
  _width = cinfo.output_width;
  _height = cinfo.output_height;
  _planes = 1;
  _actualFrame = 0;
  _framesCount = 1;

  switch (cinfo.out_color_space)
  {
    case JCS_GRAYSCALE:
      _depth = 8;
      break;
    case JCS_RGB:
      _depth = 24;
      break;
    default:
      jpeg->jpeg_destroy_decompress(&cinfo);
      return (_headerResult = Error::ImageIO_FormatNotSupported);
  }

  jpeg->jpeg_destroy_decompress(&cinfo);
  return (_headerResult = Error::Ok);
}

// ===========================================================================
// [Fog::ImageIO::JpegDecoderDevice::readImage]
// ===========================================================================

uint32_t JpegDecoderDevice::readImage(Image& image)
{
  JpegLibrary* jpeg = _jpeg->get();
  if (!jpeg->ok)
  {
    return Error::ImageIO_JpegLibraryNotFound;
  }

  struct jpeg_decompress_struct cinfo;
  MyJpegSourceMgr srcmgr;
  MyJpegErrorMgr jerr;
  JSAMPROW rowptr[1];
  Image::Data* image_d;
  err_t error = Error::Ok;
  int format = Image::FormatRGB24;

  // Create a decompression structure and load the header.
  cinfo.err = jpeg->jpeg_std_error(&jerr.errmgr);
  jerr.errmgr.error_exit = MyJpegErrorExit;
  jerr.errmgr.output_message = MyJpegMessage;

  if (setjmp(jerr.escape))
  {
    // error
    jpeg->jpeg_destroy_decompress(&cinfo);
    return Error::ImageIO_JpegError;
  }

  jpeg->jpegCreateDecompress(&cinfo, /* version */ 62, sizeof(struct jpeg_decompress_struct));

  cinfo.src = (struct jpeg_source_mgr *)&srcmgr;
  srcmgr.pub.init_source = MyJpegInitSource;
  srcmgr.pub.fill_input_buffer = MyJpegFillInputBuffer;
  srcmgr.pub.skip_input_data = MyJpegSkipInputData;
  srcmgr.pub.resync_to_restart = jpeg->jpeg_resync_to_restart;
  srcmgr.pub.term_source = MyJpegTermSource;
  srcmgr.pub.next_input_byte = srcmgr.buffer;
  srcmgr.pub.bytes_in_buffer = 0;
  srcmgr.stream = &_stream;

  jpeg->jpeg_read_header(&cinfo, true);
  jpeg->jpeg_calc_output_dimensions(&cinfo);
  jpeg->jpeg_start_decompress(&cinfo);

  // set 8 or 24-bit output
  if (cinfo.out_color_space == JCS_GRAYSCALE)
  {
    if (cinfo.output_components != 1)
    {
      error = Error::ImageIO_FormatNotSupported;
      goto end;
    }
    format = Image::FormatA8;
  }
  else if (cinfo.out_color_space == JCS_RGB)
  {
    if (cinfo.output_components != 3)
    {
      error = Error::ImageIO_FormatNotSupported;
      goto end;
    }
  }
  else
  {
    cinfo.out_color_space = JCS_RGB;
    cinfo.quantize_colors = false;
  }

  // Resize our image to jpeg size.
  if ((error = image.create(cinfo.output_width, cinfo.output_height, format))) 
    goto end;

  image_d = image._d;

  while (cinfo.output_scanline < cinfo.output_height)
  {
    rowptr[0] = (JSAMPROW)(uint8_t *)image_d->first + cinfo.output_scanline * image_d->stride;
    jpeg->jpeg_read_scanlines(&cinfo, rowptr, (JDIMENSION)1);

    // JPEG data are in big endian format.
#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
    Raster::functionMap->convert.bswap24(rowptr[0], rowptr[0], cinfo.output_width);
#endif // FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN

    if ((cinfo.output_scanline & 15) == 0)
      updateProgress(cinfo.output_scanline, cinfo.output_height);
  }
  jpeg->jpeg_finish_decompress(&cinfo);

end:
  jpeg->jpeg_destroy_decompress(&cinfo);
  return error;
}

// ===========================================================================
// [Fog::ImageIO::JpegEncoder]
// ===========================================================================

#define OUTPUT_BUF_SIZE 4096

struct MyJpegDestMgr
{
  struct jpeg_destination_mgr pub;

  Stream* stream;
  uint8_t buffer[OUTPUT_BUF_SIZE * sizeof(JOCTET)];
};

static FOG_CDECL void MyJpegInitDestination(j_compress_ptr cinfo)
{
  // We don't actually need to do anything
  return;
}

static int MyJpegEmptyOutputBuffer(j_compress_ptr cinfo)
{
  MyJpegDestMgr* dest = (MyJpegDestMgr*) cinfo->dest;

  if (dest->stream->write(dest->buffer, OUTPUT_BUF_SIZE) != OUTPUT_BUF_SIZE)
  {
    return false;
  }
  else
  {
    dest->pub.next_output_byte = dest->buffer;
    dest->pub.free_in_buffer = OUTPUT_BUF_SIZE;

    return true;
  }
}

static void FOG_CDECL MyJpegTermDestination(j_compress_ptr cinfo)
{
  MyJpegDestMgr* dest = (MyJpegDestMgr*) cinfo->dest;
  sysuint_t count = OUTPUT_BUF_SIZE - dest->pub.free_in_buffer;

  if (count > 0)
  {
    if (dest->stream->write(dest->buffer, count) != count)
    {
    }
  }
}

JpegEncoderDevice::JpegEncoderDevice() :
  _quality(90)
{
}

JpegEncoderDevice::~JpegEncoderDevice()
{
}

uint32_t JpegEncoderDevice::writeImage(const Image& image)
{
  JpegLibrary* jpeg = _jpeg->get();
  if (!jpeg->ok)
  {
    return Error::ImageIO_JpegLibraryNotFound;
  }

  MemoryBuffer<4096> bufferLocal;
  uint8_t* buffer;
  sysint_t bufferStride;

  err_t err = Error::Ok;

  Image::Data* d = image._d;
  int width = d->width;
  int height = d->height;
  int format = d->format;

  // This struct contains the JPEG compression parameters and pointers to
  // working space (which is allocated as needed by the JPEG library).
  // It is possible to have several such structures, representing multiple
  // compression/decompression processes, in existence at once.  We refer
  // to any one struct (and its associated working data) as a "JPEG object".
  struct jpeg_compress_struct cinfo;

  // This struct represents a JPEG error handler.  It is declared separately
  // because applications often want to supply a specialized error handler
  // (see the second half of this file for an example).  But here we just
  // take the easy way out and use the standard error handler, which will
  // print a message on stderr and call exit() if compression fails.
  // Note that this struct must live as long as the main JPEG parameter
  // struct, to avoid dangling-pointer problems.
  MyJpegErrorMgr jerr;
  jerr.errmgr.error_exit = MyJpegErrorExit;
  jerr.errmgr.output_message = MyJpegMessage;

  // destination manager
  MyJpegDestMgr destmgr;

  // pointer to JSAMPLE row[s]
  JSAMPROW row[1];

  if (setjmp(jerr.escape))
  {
    err = Error::ImageIO_JpegError;
    goto end;
  }

  // Step 0: Simple reject
  if (!width || !height) return Error::ImageSizeIsInvalid;

  // Step 1: allocate and initialize JPEG compression object

  // We have to set up the error handler first, in case the initialization
  // step fails.  (Unlikely, but it could happen if you are out of memory.)
  // This routine fills in the contents of struct jerr, and returns jerr's
  // address which we place into the link field in cinfo.
  cinfo.err = jpeg->jpeg_std_error((jpeg_error_mgr *)&jerr);
  // Now we can initialize the JPEG compression object.
  jpeg->jpegCreateCompress(&cinfo, 62, sizeof(cinfo));

  // Step 2: specify data destination (eg, a file)
  cinfo.dest = (jpeg_destination_mgr*)&destmgr;
  destmgr.pub.next_output_byte = (JOCTET*)destmgr.buffer;
  destmgr.pub.free_in_buffer = OUTPUT_BUF_SIZE;
  destmgr.pub.init_destination = MyJpegInitDestination;
  destmgr.pub.empty_output_buffer = MyJpegEmptyOutputBuffer;
  destmgr.pub.term_destination = MyJpegTermDestination;
  destmgr.stream = &_stream;

  // Step 3: set parameters for compression

  // First we supply a description of the input image.
  // Four fields of the cinfo struct must be filled in:
  cinfo.image_width = width;           // image width in pixels
  cinfo.image_height = height;         // image height in pixels

  // JSAMPLEs per row in image_buffer
  if (format == Image::FormatA8)
  {
    cinfo.input_components = 1;          // # of color components per pixel
    cinfo.in_color_space = JCS_GRAYSCALE;// colorspace of input image

    bufferStride = width;
    buffer = (uint8_t*)bufferLocal.alloc(bufferStride);
  }
  else
  {
    cinfo.input_components = 3;          // # of color components per pixel
    cinfo.in_color_space = JCS_RGB;      // colorspace of input image

    bufferStride = width * 3;
    buffer = (uint8_t*)bufferLocal.alloc(bufferStride);
  }

  if (!buffer) { err = Error::OutOfMemory; goto end; }

  // Now use the library's routine to set default compression parameters.
  // (You must set at least cinfo.in_color_space before calling this,
  // since the defaults depend on the source color space.)
  jpeg->jpeg_set_defaults(&cinfo);

  // Now you can set any non-default parameters you wish to.
  // Here we just illustrate the use of quality (quantization table) scaling:
  jpeg->jpeg_set_quality(&cinfo, _quality, true /* limit to baseline-JPEG values */);

  // This idea is from enlightenment. Make pixel UV sampling 1x1 if quality
  // is high (90 and higher)
  if (_quality >= 90)
  {
    cinfo.comp_info[0].h_samp_factor = 1;
    cinfo.comp_info[0].v_samp_factor = 1;
    cinfo.comp_info[1].h_samp_factor = 1;
    cinfo.comp_info[1].v_samp_factor = 1;
    cinfo.comp_info[2].h_samp_factor = 1;
    cinfo.comp_info[2].v_samp_factor = 1;
  }

  // Step 4: Start compressor

  // true ensures that we will write a complete interchange-JPEG file.
  // Pass true unless you are very sure of what you're doing.
  jpeg->jpeg_start_compress(&cinfo, true);

  // Step 5: while (scan lines remain to be written)

  // Here we use the library's state variable cinfo.next_scanline as the
  // loop counter, so that we don't have to keep track ourselves.
  // To keep things simple, we pass one scanline per call; you can pass
  // more if you wish, though.

  // jpeg_write_scanlines expects an array of pointers to scanlines.
  // Here the array is only one element long, but you could pass
  // more than one scanline at a time if that's more convenient.
  row[0] = buffer;

  while (cinfo.next_scanline < cinfo.image_height)
  {
    if (format == Image::FormatA8)
    {
      row[0] = (JSAMPLE*)image.cScanline(cinfo.next_scanline);
    }
    else
    {
      image.getDibRgb24_be(0, cinfo.next_scanline, width, buffer);
    }

    jpeg->jpeg_write_scanlines(&cinfo, row, 1);
    if (cinfo.next_scanline & 15) updateProgress(cinfo.next_scanline, cinfo.image_height);
  }

  // Step 6: Finish compression

  jpeg->jpeg_finish_compress(&cinfo);

  // Step 7: release JPEG compression object
end:

  // This is an important step since it will release a good deal of memory.
  jpeg->jpeg_destroy_compress(&cinfo);

  return err;
}

err_t JpegEncoderDevice::setProperty(const String32& name, const Value& value)
{
  err_t err;

  if (name == StubAscii8("quality"))
  {
    int32_t q;
    if ((err = value.toInt32(&q))) return err;

    if (q < 0) q = 0;
    if (q > 100) q = 100;

    _quality = q;
    return Error::Ok;
  }
  else
  {
    return EncoderDevice::setProperty(name, value);
  }
}

Value JpegEncoderDevice::getProperty(const String32& name)
{
  if (name == StubAscii8("quality"))
  {
    return Value::fromInt32(_quality);
  }
  else
  {
    return EncoderDevice::getProperty(name);
  }
}

} // ImageIO namespace
} // Fog namespace

// ===========================================================================
// [CAPI]
// ===========================================================================

FOG_INIT_DECLARE void fog_imageio_jpeg_init(void)
{
  Fog::ImageIO::_jpeg.init();
  Fog::ImageIO::addProvider(new(std::nothrow) Fog::ImageIO::JpegProvider());
}

FOG_INIT_DECLARE void fog_imageio_jpeg_shutdown(void)
{
  // Provider is destroyed by Fog::ImageIO, need only to destroy JpegLibrary
  // if open.
  Fog::ImageIO::_jpeg.destroy();
}

#else

FOG_INIT_DECLARE void fog_imageio_jpeg_init(void) {}
FOG_INIT_DECLARE void fog_imageio_jpeg_shutdown(void) {}

#endif // FOG_HAVE_JPEG_HEADERS
