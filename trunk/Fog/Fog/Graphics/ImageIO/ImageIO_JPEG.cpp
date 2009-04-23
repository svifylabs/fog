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
#include <Fog/Core/Stream.h>
#include <Fog/Core/String.h>
#include <Fog/Core/StringCache.h>
#include <Fog/Core/Strings.h>
#include <Fog/Graphics/Converter.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/ImageIO/ImageIO_JPEG.h>

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

// [Fog::ImageIO::]
namespace ImageIO {

// [Fog::ImageIO::JpegProvider]
struct JpegProvider : public Provider
{
  JpegProvider();
  virtual ~JpegProvider();

  virtual uint32_t check(const void* mem, sysuint_t length);
  virtual EncoderDevice* createEncoder();
  virtual DecoderDevice* createDecoder();
};

struct JpegLibrary
{
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

  Fog::Library dll;
  uint32_t ok;

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

    if (dll.open("jpeg", Fog::Library::OpenSystemPrefix | Fog::Library::OpenSystemSuffix).ok() &&
        dll.symbols(addr, symbols, FOG_ARRAY_SIZE(symbols), SymbolsCount, (char**)NULL) == SymbolsCount)
    {
      ok = 1;
    }
  }
};

JpegProvider::JpegProvider()
{
  // features
  _features.decoder = true;
  _features.encoder = true;

  _features.rgb24 = true;

  // name
  _name = graphics_strings->get(STR_GRAPHICS_JPEG);

  // extensions
  _extensions.reserve(4);
  _extensions.append(graphics_strings->get(STR_GRAPHICS_jpg));
  _extensions.append(graphics_strings->get(STR_GRAPHICS_jpeg));
  _extensions.append(graphics_strings->get(STR_GRAPHICS_jfi));
  _extensions.append(graphics_strings->get(STR_GRAPHICS_jfif));
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
  return new JpegEncoderDevice();
}

DecoderDevice* JpegProvider::createDecoder()
{
  return new JpegDecoderDevice();
}

static Fog::Lazy<JpegLibrary> _jpeg;

// [Fog::ImageIO::JpegDecoderDevice]

#define INPUT_BUFFER_SIZE 4096

struct MyJpegSourceMgr
{
  struct jpeg_source_mgr pub;
  Fog::Stream* stream;
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
    src->stream->seek(num_bytes - remain, Fog::Stream::Seek_Cur);

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
  JpegLibrary* jpeg = _jpeg.get();
  if (!jpeg->ok) return;

}

JpegDecoderDevice::~JpegDecoderDevice()
{
  JpegLibrary* jpeg = _jpeg.get();
  if (!jpeg->ok) return;
}

// [Fog::ImageIO::JpegDecoderDevice::reset]

void JpegDecoderDevice::reset()
{
  DecoderDevice::reset();
}

// [Fog::ImageIO::JpegDecoderDevice::readHeader]

uint32_t JpegDecoderDevice::readHeader()
{
  if (_jpeg == NULL)
  {
    return Fog_ImageError_LibjpegMissing;
  }

  struct jpeg_decompress_struct cinfo;
  MyJpegSourceMgr srcmgr;
  MyJpegErrorMgr jerr;

  // create a decompression structure and load the header
  cinfo.err = _jpeg->jpeg_std_error(&jerr.errmgr);
  jerr.errmgr.error_exit = MyJpegErrorExit;
  jerr.errmgr.output_message = MyJpegMessage;

  if (setjmp(jerr.escape))
  {
    // error
    _jpeg->jpeg_destroy_decompress(&cinfo);
    return Fog_ImageError_Libjpeg;
  }

  _jpeg->jpegCreateDecompress(&cinfo, /* version */ 62, sizeof(struct jpeg_decompress_struct));

  cinfo.src = (struct jpeg_source_mgr *)&srcmgr;
  srcmgr.pub.init_source = MyJpegInitSource;
  srcmgr.pub.fill_input_buffer = MyJpegFillInputBuffer;
  srcmgr.pub.skip_input_data = MyJpegSkipInputData;
  srcmgr.pub.resync_to_restart = _jpeg->jpeg_resync_to_restart;
  srcmgr.pub.term_source = MyJpegTermSource;
  srcmgr.pub.next_input_byte = srcmgr.buffer;
  srcmgr.pub.bytes_in_buffer = 0;
  srcmgr.stream = &self->stream;

  _jpeg->jpeg_read_header(&cinfo, true);
  _jpeg->jpeg_calc_output_dimensions(&cinfo);

  self->data.headerRead = true;
  self->data.width = cinfo.output_width;
  self->data.height = cinfo.output_height;
  self->data.planes = 1;
  self->data.actualFrame = 0;
  self->data.framesCount = 1;

  switch (cinfo.out_color_space)
  {
    case JCS_GRAYSCALE:
      self->data.depth = 8;
      break;
    case JCS_RGB:
      self->data.depth = 24;
      break;
    default:
      _jpeg->jpeg_destroy_decompress(&cinfo);
      return Fog_ImageError_UnsupportedFormat;
  }

  _jpeg->jpeg_destroy_decompress(&cinfo);
  return Fog_ImageError_Success;
}

// [Fog::ImageIO::JpegDecoderDevice::readImage]

uint32_t JpegDecoderDevice::readImage(Fog::Image& image)
{
  if (_jpeg == NULL)
  {
    return Fog_ImageError_LibjpegMissing;
  }

  struct jpeg_decompress_struct cinfo;
  MyJpegSourceMgr srcmgr;
  MyJpegErrorMgr jerr;
  JSAMPROW rowptr[1];
  Fog::ImageData* image_d;
  uint error = Fog_ImageError_Success;
  uint format = Fog::ImageFormat::RGB24;

  // create a decompression structure and load the header
  cinfo.err = _jpeg->jpeg_std_error(&jerr.errmgr);
  jerr.errmgr.error_exit = MyJpegErrorExit;
  jerr.errmgr.output_message = MyJpegMessage;

  if (setjmp(jerr.escape))
  {
    // error
    _jpeg->jpeg_destroy_decompress(&cinfo);
    return Fog_ImageError_Libjpeg;
  }

  _jpeg->jpegCreateDecompress(&cinfo, /* version */ 62, sizeof(struct jpeg_decompress_struct));

  cinfo.src = (struct jpeg_source_mgr *)&srcmgr;
  srcmgr.pub.init_source = MyJpegInitSource;
  srcmgr.pub.fill_input_buffer = MyJpegFillInputBuffer;
  srcmgr.pub.skip_input_data = MyJpegSkipInputData;
  srcmgr.pub.resync_to_restart = _jpeg->jpeg_resync_to_restart;
  srcmgr.pub.term_source = MyJpegTermSource;
  srcmgr.pub.next_input_byte = srcmgr.buffer;
  srcmgr.pub.bytes_in_buffer = 0;
  srcmgr.stream = &self->stream;

  _jpeg->jpeg_read_header(&cinfo, true);
  _jpeg->jpeg_calc_output_dimensions(&cinfo);
  _jpeg->jpeg_start_decompress(&cinfo);

  // set 8 or 24-bit output
  if (cinfo.out_color_space == JCS_GRAYSCALE)
  {
    if (cinfo.output_components != 1)
    {
      error = Fog_ImageError_UnsupportedFormat;
      goto end;
    }
    format = Fog::ImageFormat::A8;
  }
  else if (cinfo.out_color_space == JCS_RGB)
  {
    if (cinfo.output_components != 3)
    {
      error = Fog_ImageError_UnsupportedFormat;
      goto end;
    }
  }
  else
  {
    cinfo.out_color_space = JCS_RGB;
    cinfo.quantize_colors = false;
  }

  // resize our image to jpeg size
  if (!image->resize(cinfo.output_width, cinfo.output_height, format))
  {
    _jpeg->jpeg_destroy_decompress(&cinfo);
    error = Fog_ImageError_OutOfMemory;
    goto end;
  }
  image_d = image->_d;

  while (cinfo.output_scanline < cinfo.output_height)
  {
    rowptr[0] = (JSAMPROW)(uint8_t *)image_d->_base + cinfo.output_scanline * image_d->_stride;
    _jpeg->jpeg_read_scanlines(&cinfo, rowptr, (JDIMENSION)1);

    // Need to swap R and B values for little endian architecture
#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
    uint8_t* rowtmp = (uint8_t*)rowptr[0];
    ulong x;
    for (x = cinfo.output_width; x; x--, rowtmp += 3)
    {
      uint8_t t = rowtmp[0];
      rowtmp[0] = rowtmp[2];
      rowtmp[2] = t;
    }
#endif // FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN

    CALL_HANDLER(cinfo.output_scanline, cinfo.output_height)
  }
  _jpeg->jpeg_finish_decompress(&cinfo);

end:
  // Success
  _jpeg->jpeg_destroy_decompress(&cinfo);
  return error;
}

// [Fog::ImageIO::JpegEncoder]

#define OUTPUT_BUF_SIZE 4096

struct MyJpegDestMgr
{
  struct jpeg_destination_mgr pub;

  Fog::Stream* stream;
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

JpegEncoderDevice::JpegEncoderDevice()
{
}

JpegEncoderDevice::~JpegEncoderDevice()
{
}

uint32_t JpegEncoderDevice::writeImage(const Fog::Image& image_)
{
  if (_jpeg == NULL)
  {
    return Fog_ImageError_LibjpegMissing;
  }

  Fog::MemoryBuffer<4096> bufferLocal;
  uint8_t* buffer;
  long bufferStride;

  uint error = Fog_ImageError_Success;

  Fog::ImageData* d = image->_d;
  uint width = d->_width;
  uint height = d->_height;
  uint formatId = d->_formatId;

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

  // Converter
  Fog::Converter converter;
  converter.dither = 1;

  if (setjmp(jerr.escape))
  {
    // error
    _jpeg->jpeg_destroy_compress(&cinfo);
    return Fog_ImageError_Libjpeg;
  }

  // Step 0: Simple reject
  if (!width || !height)
  {
    error = Fog_ImageError_InvalidBounds;
    goto end;
  }

  // Step 1: allocate and initialize JPEG compression object

  // We have to set up the error handler first, in case the initialization
  // step fails.  (Unlikely, but it could happen if you are out of memory.)
  // This routine fills in the contents of struct jerr, and returns jerr's
  // address which we place into the link field in cinfo.
  cinfo.err = _jpeg->jpeg_std_error((jpeg_error_mgr *)&jerr);
  // Now we can initialize the JPEG compression object.
  _jpeg->jpegCreateCompress(&cinfo, 62, sizeof(cinfo));

  // Step 2: specify data destination (eg, a file)
  cinfo.dest = (jpeg_destination_mgr*)&destmgr;
  destmgr.pub.next_output_byte = (JOCTET*)destmgr.buffer;
  destmgr.pub.free_in_buffer = OUTPUT_BUF_SIZE;
  destmgr.pub.init_destination = MyJpegInitDestination;
  destmgr.pub.empty_output_buffer = MyJpegEmptyOutputBuffer;
  destmgr.pub.term_destination = MyJpegTermDestination;
  destmgr.stream = &self->stream;

  // Step 3: set parameters for compression

  // First we supply a description of the input image.
  // Four fields of the cinfo struct must be filled in:
  cinfo.image_width = width;           // image width in pixels
  cinfo.image_height = height;         // image height in pixels

  // JSAMPLEs per row in image_buffer
  if (formatId == Fog::ImageFormat::A1 || formatId == Fog::ImageFormat::A8)
  {
    cinfo.input_components = 1;          // # of color components per pixel
    cinfo.in_color_space = JCS_GRAYSCALE;// colorspace of input image

    converter.init(Fog::ImageFormat::A8, formatId);

    bufferStride = width;
    buffer = (uint8_t*)bufferLocal.alloc(bufferStride);
  }
  else
  {
    cinfo.input_components = 3;          // # of color components per pixel
    cinfo.in_color_space = JCS_RGB;      // colorspace of input image

#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
    converter.init(Fog::Converter::BGR24, formatId);
#else // FOG_BIG_ENDIAN
    converter.init(Fog::Converter::RGB24, formatId);
#endif // FOG_BYTE_ORDER

    bufferStride = width * 3;
    buffer = (uint8_t*)bufferLocal.alloc(bufferStride);
  }

  // Now use the library's routine to set default compression parameters.
  // (You must set at least cinfo.in_color_space before calling this,
  // since the defaults depend on the source color space.)
  _jpeg->jpeg_set_defaults(&cinfo);

  // Now you can set any non-default parameters you wish to.
  // Here we just illustrate the use of quality (quantization table) scaling:
  _jpeg->jpeg_set_quality(&cinfo, self->options.quality, true /* limit to baseline-JPEG values */);

  // Step 4: Start compressor

  // true ensures that we will write a complete interchange-JPEG file.
  // Pass true unless you are very sure of what you're doing.
  _jpeg->jpeg_start_compress(&cinfo, true);

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
    converter.ditherOrigin.set(0, cinfo.next_scanline);
    converter.convertSpan(buffer, 0, d->_base + cinfo.next_scanline * d->_stride, 0, width);
    _jpeg->jpeg_write_scanlines(&cinfo, row, 1);
  }

  // Step 6: Finish compression

  _jpeg->jpeg_finish_compress(&cinfo);

  // Step 7: release JPEG compression object

  // This is an important step since it will release a good deal of memory.
  _jpeg->jpeg_destroy_compress(&cinfo);

  // And we're done!

end:
  return error;
}

// [Fog::ImageIO::]
}

} // Fog namespace

// [CAPI]
FOG_CAPI_DECLARE Fog::ImageIO::Provider* fog_imageio_getJpegProvider(void)
{
  return _jpeg.get()->ok ? new Fog::ImageIO::JpegProvider() : NULL;
}

#else

FOG_CAPI_DECLARE Fog::ImageIO::Provider* fog_imageio_getJpegProvider(void)
{
  return NULL;
}

#endif // FOG_HAVE_JPEG_HEADERS
