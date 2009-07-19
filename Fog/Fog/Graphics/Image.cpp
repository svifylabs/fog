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

#include <Fog/Core/FileSystem.h>
#include <Fog/Core/FileUtil.h>
#include <Fog/Core/MapFile.h>
#include <Fog/Core/Math.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/Misc.h>
#include <Fog/Core/Std.h>
#include <Fog/Graphics/ColorMatrix.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Error.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/ImageFilter.h>
#include <Fog/Graphics/ImageIO.h>
#include <Fog/Graphics/Raster.h>
#include <Fog/Graphics/Raster/Raster_Bresenham.h>
#include <Fog/Graphics/Raster/Raster_C.h>
#include <Fog/Graphics/Reduce.h>

namespace Fog {

// ============================================================================
// [Fog::Image]
// ============================================================================

Static<Image::Data> Image::sharedNull;

Image::Image() : 
  _d(sharedNull->refAlways())
{
}

Image::Image(Data* d) : 
  _d(d)
{
}

Image::Image(const Image& other) : 
  _d(other._d->ref())
{
}

Image::Image(int w, int h, int format) :
  _d(sharedNull->refAlways())
{
  create(w, h, format);
}

Image::~Image()
{
  _d->deref();
}

err_t Image::_detach()
{
  Data* d = _d;
  if (d->flags & Data::IsNull) return Error::Ok;

  if (d->stride == 0)
  {
    AtomicBase::ptr_setXchg(&_d, sharedNull->refAlways())->deref();
    return Error::Ok;
  }

  if (d->refCount.get() > 1 || d->flags & Data::IsReadOnly)
  {
    Data* newd = Data::copy(d);
    if (!newd) return Error::OutOfMemory;

    AtomicBase::ptr_setXchg(&_d, newd)->deref();
  }

  return Error::Ok;
}

void Image::free()
{
  AtomicBase::ptr_setXchg(&_d, sharedNull.instancep()->refAlways())->deref();
}

err_t Image::create(int w, int h, int format)
{
  Data* d = _d;

  // Zero dimensions are like Image::free()
  if (w <= 0 || h <= 0 || format <= 0 || format >= FormatCount)
  {
    free();
    return Error::InvalidArgument;
  }

  // Always return a new detached and writable image.
  if (d->width == w && d->height == h && d->format == format && d->refCount.get() == 1 && !(d->flags & Data::IsReadOnly))
  {
    return Error::Ok;
  }

  // Create new data
  Data* newd = Data::alloc(w, h, format);
  if (!newd)
  {
    free();
    return Error::OutOfMemory;
  }

  AtomicBase::ptr_setXchg(&_d, newd)->deref();
  return Error::Ok;
}

err_t Image::adopt(int w, int h, int format, const uint8_t* mem, sysint_t stride, uint32_t adoptFlags)
{
  Data* d = _d;

  if (w <= 0 || h <= 0 || format <= 0 || format >= FormatCount || mem == NULL)
  {
    free();
    return Error::InvalidArgument;
  }

  if (d->refCount.get() > 1 || ((d->flags & Data::IsDynamic) && d->size != 0))
  {
    Data* newd = Data::alloc(0);
    if (!newd)
    {
      free();
      return Error::OutOfMemory;
    }

    AtomicBase::ptr_setXchg(&_d, newd)->deref();
    d = newd;
  }

  // Fill basic variables
  d->width = w;
  d->height = h;
  d->format = format;

  // Bottom -> Top data
  if (adoptFlags & AdoptReversed)
  {
    d->stride = -stride;
    d->data = (uint8_t*)mem;
    d->first = (uint8_t*)mem + (h - 1) * stride;
  }
  // Top -> Bottom data (default)
  else 
  {
    d->stride = stride;
    d->data = (uint8_t*)mem;
    d->first = (uint8_t*)mem;
  }

  // Read only memory ?
  if ((adoptFlags & AdoptReadOnly) != 0)
    d->flags |= Data::IsReadOnly;
  else
    d->flags &= ~Data::IsReadOnly;

  return Error::Ok;
}

err_t Image::set(const Image& other)
{
  if (_d == other._d) return Error::Ok;

  if (other.isEmpty())
  {
    AtomicBase::ptr_setXchg(&_d, sharedNull->refAlways())->deref();
    return Error::Ok;
  }

  if (isStrong() || !other.isSharable())
  {
    return setDeep(other);
  }
  else
  {
    AtomicBase::ptr_setXchg(&_d, other._d->refAlways())->deref();
    return Error::Ok;
  }
}

err_t Image::setDeep(const Image& other)
{
  if (_d == other._d) return Error::Ok;

  if (other.isEmpty())
  {
    AtomicBase::ptr_setXchg(&_d, sharedNull->refAlways())->deref();
    return Error::Ok;
  }

  if (width() == other.width() &&
      height() == other.height() &&
      depth() == other.depth())
  {
    sysint_t bpl = width() * bytesPerPixel();
    sysint_t h = height();
    _d->format = other._d->format;

    uint8_t* dstCur = _d->first;
    uint8_t* srcCur = other._d->first;

    sysint_t dstStride = _d->stride;
    sysint_t srcStride = other._d->stride;

    for (sysint_t i = 0; i < h; i++, dstCur += dstStride, srcCur += srcStride)
    {
      Memory::copy(dstCur, srcCur, bpl);
    }
    return Error::Ok;
  }

  Data* newd = Data::copy(other._d);
  if (!newd) return Error::OutOfMemory;

  AtomicBase::ptr_setXchg(&_d, newd)->deref();
  return Error::Ok;
}

err_t Image::convert(int format)
{
  Data* d = _d;

  int sourceFormat = _d->format;
  int targetFormat = format;

  if (targetFormat <= 0 || targetFormat >= FormatCount)
    return Error::InvalidArgument;

  if (sourceFormat == targetFormat) return Error::Ok;

  int w = d->width;
  int h = d->height;
  int y;

  void* blitter = NULL;

  // Special cases
  switch (targetFormat)
  {
    case FormatARGB32:
    case FormatPRGB32:
      blitter = (void*)Raster::getRasterOps(targetFormat, CompositeSrc)->span_composite[sourceFormat];
      break;
    case FormatRGB32:
      switch (sourceFormat)
      {
        case FormatARGB32:
        case FormatPRGB32:
          blitter = (void*)Raster::functionMap->convert.rgb32_from_argb32;
          break;
        case FormatRGB32:
          blitter = (void*)Raster::functionMap->convert.memcpy32;
          break;
        case FormatRGB24:
          blitter = (void*)Raster::functionMap->convert.rgb32_from_rgb24;
          break;
        case FormatA8:
          blitter = (void*)Raster::functionMap->convert.rgb32_from_greyscale8;
          break;
        case FormatI8:
          blitter = (void*)Raster::functionMap->convert.rgb32_from_i8;
          break;
      }
      break;
    case FormatRGB24:
      switch (sourceFormat)
      {
        case FormatARGB32:
        case FormatPRGB32:
        case FormatRGB32:
          blitter = (void*)Raster::functionMap->convert.rgb24_from_rgb32;
          break;
        case FormatRGB24:
          blitter = (void*)Raster::functionMap->convert.memcpy24;
          break;
        case FormatA8:
          blitter = (void*)Raster::functionMap->convert.rgb24_from_greyscale8;
          break;
        case FormatI8:
          blitter = (void*)Raster::functionMap->convert.rgb24_from_i8;
          break;
      }
      break;
    case FormatA8:
      switch (sourceFormat)
      {
        case FormatARGB32:
        case FormatPRGB32:
        {
          *this = extractChannel(ChannelAlpha);
          return Error::Ok;
        }
        case FormatRGB32:
           blitter = (void*)Raster::functionMap->convert.greyscale8_from_rgb32;
           break;
        case FormatRGB24:
           blitter = (void*)Raster::functionMap->convert.greyscale8_from_rgb24;
           break;
        case FormatI8:
           blitter = (void*)Raster::functionMap->convert.greyscale8_from_i8;
           break;
      }
      break;
    case FormatI8:
      return to8Bit();
      break;
  }

  // We can optimize converting if we can do conversion to current image data.
  if (depth() == formatToDepth(format) && isDetached() && !isReadOnly())
  {
    uint8_t* dstCur = _d->first;
    sysint_t dstStride = _d->stride;

    if (sourceFormat == FormatI8)
    {
      for (y = 0; y < h; y++, dstCur += dstStride)
        ((Raster::SpanCompositeIndexedFn)blitter)(dstCur, dstCur, w, _d->palette.cData());
    }
    else
    {
      for (y = 0; y < h; y++, dstCur += dstStride)
        ((Raster::SpanCompositeFn)blitter)(dstCur, dstCur, w);
    }

    _d->format = targetFormat;
    return Error::Ok;
  }
  else
  {
    Data* newd = Data::alloc(w, h, targetFormat);
    if (!newd) return Error::OutOfMemory;

    uint8_t* dstCur = newd->first;
    uint8_t* srcCur = _d->first;

    sysint_t dstStride = newd->stride;
    sysint_t srcStride = _d->stride;

    if (sourceFormat == FormatI8)
    {
      for (y = 0; y < h; y++, dstCur += dstStride, srcCur += srcStride)
        ((Raster::SpanCompositeIndexedFn)blitter)(dstCur, srcCur, w, _d->palette.cData());
    }
    else
    {
      for (y = 0; y < h; y++, dstCur += dstStride, srcCur += srcStride)
        ((Raster::SpanCompositeFn)blitter)(dstCur, srcCur, w);
    }

    AtomicBase::ptr_setXchg(&_d, newd)->deref();
    return Error::Ok;
  }
}

err_t Image::to8Bit()
{
  if (isEmpty()) return Error::Ok;
  if (format() == FormatI8) return Error::Ok;

  err_t err;

  if (format() == FormatA8)
  {
    if ((err = forceFormat(FormatI8))) return err;
    if ((err = setPalette(Palette::greyscale()))) return err;

    return Error::Ok;
  }

  Reduce reducer;

  int w = width();
  int h = height();

  Image i;
  if ( (err = i.create(w, h, FormatI8)) ) return err;

  uint8_t* dstBase = i._d->first;
  uint8_t* srcBase = _d->first;

  sysint_t dstStride = i._d->stride;
  sysint_t srcStride = _d->stride;

  int y;

  if (reducer.analyze(*this, true))
  {
    if ( (err = i.setPalette(reducer.toPalette())) ) return err;

    for (y = 0; y < h; y++, dstBase += dstStride, srcBase += srcStride)
    {
      uint8_t* dstCur = dstBase;
      uint8_t* srcCur = srcBase;

      switch (format())
      {
        case FormatARGB32:
        case FormatPRGB32:
          for (int x = 0; x < w; x++, dstCur += 1, srcCur += 4)
          {
            dstCur[0] = reducer.traslate(
              ((uint32_t*)srcCur)[0] | Raster::RGB32_AMask);
          }
          break;
        case FormatRGB24:
          for (int x = 0; x < w; x++, dstCur += 1, srcCur += 3)
          {
            dstCur[0] = reducer.traslate(
              Raster::PixFmt_RGB24::fetch(srcCur) | Raster::RGB32_AMask);
          }
          break;
      }
    }
  }
  else
  {
    Palette pal = Palette::colorCube(4, 8, 4);
    if ( (err = i.setPalette(pal)) ) return err;

    uint8_t palConv[256];
    for (y = 0; y < 256; y++) palConv[y] = y;

    Raster::ConvertDither8Fn converter = NULL;

    switch (format())
    {
      case FormatARGB32:
      case FormatPRGB32:
      case FormatRGB32:
        converter = Raster::functionMap->convert.i8rgb232_from_rgb32_dither;
        break;
      case FormatRGB24:
        converter = Raster::functionMap->convert.i8rgb232_from_rgb24_dither;
        break;
      default:
        FOG_ASSERT_NOT_REACHED();
        break;
    }

    for (y = 0; y < h; y++, dstBase += dstStride, srcBase += srcStride)
    {
      converter(dstBase, srcBase, w, Point(0, y), palConv);
    }
  }

  return set(i);
}

err_t Image::to8Bit(const Palette& pal)
{
  if (isEmpty()) return Error::Ok;

  err_t err;

  int w = width();
  int h = height();

  Image i;
  if ( (err = i.create(w, h, FormatI8)) ) return err;
  if ( (err = i.setPalette(pal)) ) return err;

  uint8_t* dstBase = i._d->first;
  uint8_t* srcBase = _d->first;

  sysint_t dstStride = i._d->stride;
  sysint_t srcStride = _d->stride;

  int y, x;

  switch (format())
  {
    case FormatARGB32:
    case FormatPRGB32:
    case FormatRGB32:
    {
      for (y = 0; y < h; y++, dstBase += dstStride, srcBase += srcStride)
      {
        uint8_t* dstCur = dstBase;
        uint8_t* srcCur = srcBase;
        for (x = 0; x < w; x++, dstCur += 1, srcCur += 4)
        {
          Rgba c = ((uint32_t*)srcCur)[0];
          dstCur[0] = pal.findColor(c.r, c.g, c.b);
        }
      }
      break;
    }
    case FormatRGB24:
    {
      for (y = 0; y < h; y++, dstBase += dstStride, srcBase += srcStride)
      {
        uint8_t* dstCur = dstBase;
        uint8_t* srcCur = srcBase;
        for (x = 0; x < w; x++, dstCur += 1, srcCur += 3)
        {
          Rgba c = Raster::PixFmt_RGB24::fetch(dstCur);
          dstCur[0] = pal.findColor(c.r, c.g, c.b);
        }
      }
      break;
    }
    case FormatI8:
    case FormatA8:
    {
      // Build lookup table.
      uint8_t table[256];

      if (format() == FormatI8)
      {
        for (y = 0; y < 256; y++)
        {
          Rgba c = _d->palette.cAt(y);
          table[y] = pal.findColor(c.r, c.g, c.b);
        }
      }
      else
      {
        for (y = 0; y < 256; y++)
        {
          uint8_t c = _d->palette.cAt(y).grey();
          table[y] = pal.findColor(c, c, c);
        }
      }

      for (y = 0; y < h; y++, dstBase += dstStride, srcBase += srcStride)
      {
        uint8_t* dstCur = dstBase;
        uint8_t* srcCur = srcBase;
        for (x = 0; x < w; x++, dstCur += 1, srcCur += 1)
        {
          dstCur[0] = table[srcCur[0]];
        }
      }
      break;

    }
    default:
      FOG_ASSERT_NOT_REACHED();
      break;
  }

  return set(i);
}

err_t Image::forceFormat(int format)
{
  if (format <= 0 || format >= FormatCount)
    return Error::InvalidArgument;

  if (depth() != formatToDepth(format))
    return Error::InvalidArgument;
  
  if (_d->format == format)
    return Error::Ok;

  if (!isDetached())
  {
    err_t err = detach();
    if (err) return err;
  }

  _d->format = format;

  return Error::Ok;
}

err_t Image::setPalette(const Palette& palette)
{
  err_t err;
  if ( (err = detach()) ) return err;

  _d->palette = palette;
  return Error::Ok;
}

err_t Image::setPalette(sysuint_t index, sysuint_t count, const Rgba* rgba)
{
  if (count == 0) return Error::Ok;

  err_t err;
  if ( (err = detach()) ) return err;

  return _d->palette.set(index, count, rgba);
}

// ============================================================================
// [Fog::Image - GetDib / SetDib]
// ============================================================================

static void image_getdib(const Image* self,
  int x, int y, sysint_t w, void* dst, void* converter)
{
  if (converter == NULL) return;

  Image::Data* d = self->_d;

  if ((uint)x >= (uint)d->width ||
      (uint)y >= (uint)d->height ||
      (sysuint_t)w > (uint)(d->width - x)) return;

  const uint8_t* src = d->first +
    (sysint_t)y * d->stride +
    (sysint_t)x * d->bytesPerPixel;

  if (d->format != Image::FormatI8)
    ((Raster::ConvertPlainFn)converter)((uint8_t*)dst, src, w);
  else
    ((Raster::ConvertIndexedFn)converter)((uint8_t*)dst, src, w, d->palette.cData());
}

static void image_setdib(Image* self,
  int x, int y, sysint_t w, const void* src, void* converter)
{
  if (converter == NULL) return;

  Image::Data* d = self->_d;

  if ((uint)x >= (uint)d->width ||
      (uint)y >= (uint)d->height ||
      (sysuint_t)w > (uint)(d->width - x)) return;

  err_t err = self->detach();
  if (err) return;

  d = self->_d;

  uint8_t* dst = d->first +
    (sysint_t)y * d->stride +
    (sysint_t)x * d->bytesPerPixel;

  ((Raster::ConvertPlainFn)converter)(dst, (const uint8_t*)src, w);
}

void Image::getDibArgb32(int x, int y, sysint_t w, void* dst) const
{
  void* converter = NULL;
  switch (format())
  {
    case FormatARGB32:
      converter = (void*)Raster::functionMap->convert.memcpy32;
      break;
    case FormatPRGB32:
      converter = (void*)Raster::functionMap->convert.argb32_from_prgb32;
      break;
    case FormatRGB32:
      converter = (void*)Raster::functionMap->convert.argb32_from_rgb32;
      break;
    case FormatRGB24:
      converter = (void*)Raster::functionMap->convert.rgb32_from_rgb24;
      break;
    case FormatI8:
      converter = (void*)Raster::functionMap->convert.bgr24_from_i8;
      break;
  }
  image_getdib(this, x, y, w, dst, converter);
}

void Image::getDibArgb32_bs(int x, int y, sysint_t w, void* dst) const
{
  void* converter = NULL;
  switch (format())
  {
    case FormatARGB32:
      converter = (void*)Raster::functionMap->convert.bswap32;
      break;
    case FormatPRGB32:
      converter = (void*)Raster::functionMap->convert.argb32_bs_from_prgb32;
      break;
    case FormatRGB32:
      converter = (void*)Raster::functionMap->convert.argb32_bs_from_rgb32;
      break;
    case FormatRGB24:
      converter = (void*)Raster::functionMap->convert.rgb32_bs_from_rgb24;
      break;
    case FormatI8:
      converter = (void*)Raster::functionMap->convert.bgr24_from_i8;
      break;
  }
  image_getdib(this, x, y, w, dst, converter);
}

void Image::setDibArgb32(int x, int y, sysint_t w, const void* src)
{
  void* converter = NULL;
  switch (format())
  {
    case FormatARGB32:
      converter = (void*)Raster::functionMap->convert.memcpy32;
      break;
    case FormatPRGB32:
      converter = (void*)Raster::functionMap->convert.prgb32_from_argb32;
      break;
    case FormatRGB32:
      converter = (void*)Raster::functionMap->convert.rgb32_from_argb32;
      break;
    case FormatRGB24:
      converter = (void*)Raster::functionMap->convert.rgb24_from_rgb32;
      break;
  }
  image_setdib(this, x, y, w, src, converter);
}

void Image::setDibArgb32_bs(int x, int y, sysint_t w, const void* src)
{
  void* converter = NULL;
  switch (format())
  {
    case FormatARGB32:
      converter = (void*)Raster::functionMap->convert.bswap32;
      break;
    case FormatPRGB32:
      converter = (void*)Raster::functionMap->convert.prgb32_from_argb32_bs;
      break;
    case FormatRGB32:
      converter = (void*)Raster::functionMap->convert.rgb32_from_argb32_bs;
      break;
    case FormatRGB24:
      converter = (void*)Raster::functionMap->convert.rgb24_from_rgb32_bs;
      break;
  }
  image_setdib(this, x, y, w, src, converter);
}

void Image::getDibPrgb32(int x, int y, sysint_t w, void* dst) const
{
  void* converter = NULL;
  switch (format())
  {
    case FormatARGB32:
      converter = (void*)Raster::functionMap->convert.prgb32_from_argb32;
      break;
    case FormatPRGB32:
      converter = (void*)Raster::functionMap->convert.memcpy32;
      break;
    case FormatRGB32:
      converter = (void*)Raster::functionMap->convert.argb32_from_rgb32;
      break;
    case FormatRGB24:
      converter = (void*)Raster::functionMap->convert.rgb32_from_rgb24;
      break;
    case FormatI8:
      converter = (void*)Raster::functionMap->convert.prgb32_from_i8;
      break;
  }
  image_getdib(this, x, y, w, dst, converter);
}

void Image::getDibPrgb32_bs(int x, int y, sysint_t w, void* dst) const
{
  void* converter = NULL;
  switch (format())
  {
    case FormatARGB32:
      converter = (void*)Raster::functionMap->convert.prgb32_bs_from_argb32;
      break;
    case FormatPRGB32:
      converter = (void*)Raster::functionMap->convert.bswap32;
      break;
    case FormatRGB32:
      converter = (void*)Raster::functionMap->convert.argb32_bs_from_rgb32;
      break;
    case FormatRGB24:
      converter = (void*)Raster::functionMap->convert.rgb32_bs_from_rgb24;
      break;
    case FormatI8:
      converter = (void*)Raster::functionMap->convert.prgb32_bs_from_i8;
      break;
  }
  image_getdib(this, x, y, w, dst, converter);
}

void Image::setDibPrgb32(int x, int y, sysint_t w, const void* src)
{
  void* converter = NULL;
  switch (format())
  {
    case FormatARGB32:
      converter = (void*)Raster::functionMap->convert.argb32_from_prgb32;
      break;
    case FormatPRGB32:
      converter = (void*)Raster::functionMap->convert.memcpy32;
      break;
    case FormatRGB32:
      converter = (void*)Raster::functionMap->convert.rgb32_from_argb32;
      break;
    case FormatRGB24:
      converter = (void*)Raster::functionMap->convert.rgb24_from_rgb32;
      break;
  }
  image_setdib(this, x, y, w, src, converter);
}

void Image::setDibPrgb32_bs(int x, int y, sysint_t w, const void* src)
{
  void* converter = NULL;
  switch (format())
  {
    case FormatARGB32:
      converter = (void*)Raster::functionMap->convert.argb32_from_prgb32_bs;
      break;
    case FormatPRGB32:
      converter = (void*)Raster::functionMap->convert.bswap32;
      break;
    case FormatRGB32:
      converter = (void*)Raster::functionMap->convert.rgb32_from_argb32_bs;
      break;
    case FormatRGB24:
      converter = (void*)Raster::functionMap->convert.rgb24_from_rgb32_bs;
      break;
  }
  image_setdib(this, x, y, w, src, converter);
}

void Image::getDibRgb24(int x, int y, sysint_t w, void* dst) const
{
  void* converter = NULL;
  switch (format())
  {
    case FormatARGB32:
    case FormatPRGB32:
    case FormatRGB32:
      converter = (void*)Raster::functionMap->convert.rgb24_from_rgb32;
      break;
    case FormatRGB24:
      converter = (void*)Raster::functionMap->convert.memcpy24;
      break;
    case FormatI8:
      converter = (void*)Raster::functionMap->convert.rgb24_from_i8;
      break;
  }
  image_getdib(this, x, y, w, dst, converter);
}

void Image::getDibRgb24_bs(int x, int y, sysint_t w, void* dst) const
{
  void* converter = NULL;
  switch (format())
  {
    case FormatARGB32:
    case FormatPRGB32:
    case FormatRGB32:
      converter = (void*)Raster::functionMap->convert.bgr24_from_rgb32;
      break;
    case FormatRGB24:
      converter = (void*)Raster::functionMap->convert.bswap24;
      break;
    case FormatI8:
      converter = (void*)Raster::functionMap->convert.bgr24_from_i8;
      break;
  }
  image_getdib(this, x, y, w, dst, converter);
}

void Image::setDibRgb24(int x, int y, sysint_t w, const void* src)
{
  void* converter = NULL;
  switch (format())
  {
    case FormatARGB32:
    case FormatPRGB32:
    case FormatRGB32:
      converter = (void*)Raster::functionMap->convert.rgb32_from_rgb24;
      break;
    case FormatRGB24:
      converter = (void*)Raster::functionMap->convert.memcpy24;
      break;
  }
  image_setdib(this, x, y, w, src, converter);
}

void Image::setDibRgb24_bs(int x, int y, sysint_t w, const void* src)
{
  void* converter = NULL;
  switch (format())
  {
    case FormatARGB32:
    case FormatPRGB32:
    case FormatRGB32:
      converter = (void*)Raster::functionMap->convert.rgb32_from_bgr24;
      break;
    case FormatRGB24:
      converter = (void*)Raster::functionMap->convert.bswap24;
      break;
  }
  image_setdib(this, x, y, w, src, converter);
}

void Image::getDibRgb16_555(int x, int y, sysint_t w, void* dst) const
{
  void* converter = NULL;
  switch (format())
  {
    case FormatARGB32:
    case FormatPRGB32:
    case FormatRGB32:
      converter = (void*)Raster::functionMap->convert.rgb16_5550_from_rgb32;
      break;
    case FormatRGB24:
      converter = (void*)Raster::functionMap->convert.rgb16_5550_from_rgb24;
      break;
    case FormatI8:
      converter = (void*)Raster::functionMap->convert.rgb16_5550_from_i8;
      break;
  }
  image_getdib(this, x, y, w, dst, converter);
}

void Image::getDibRgb16_555_bs(int x, int y, sysint_t w, void* dst) const
{
  void* converter = NULL;
  switch (format())
  {
    case FormatARGB32:
    case FormatPRGB32:
    case FormatRGB32:
      converter = (void*)Raster::functionMap->convert.rgb16_5550_bs_from_rgb32;
      break;
    case FormatRGB24:
      converter = (void*)Raster::functionMap->convert.rgb16_5550_bs_from_rgb24;
      break;
    case FormatI8:
      converter = (void*)Raster::functionMap->convert.rgb16_5550_bs_from_i8;
      break;
  }
  image_getdib(this, x, y, w, dst, converter);
}

void Image::setDibRgb16_555(int x, int y, sysint_t w, const void* src)
{
  void* converter = NULL;
  switch (format())
  {
    case FormatARGB32:
    case FormatPRGB32:
    case FormatRGB32:
      converter = (void*)Raster::functionMap->convert.rgb32_from_rgb16_5550;
      break;
    case FormatRGB24:
      converter = (void*)Raster::functionMap->convert.rgb24_from_rgb16_5550;
      break;
  }
  image_setdib(this, x, y, w, src, converter);
}

void Image::setDibRgb16_555_bs(int x, int y, sysint_t w, const void* src)
{
  void* converter = NULL;
  switch (format())
  {
    case FormatARGB32:
    case FormatPRGB32:
    case FormatRGB32:
      converter = (void*)Raster::functionMap->convert.rgb32_from_rgb16_5550_bs;
      break;
    case FormatRGB24:
      converter = (void*)Raster::functionMap->convert.rgb24_from_rgb16_5550_bs;
      break;
  }
  image_setdib(this, x, y, w, src, converter);
}

void Image::getDibRgb16_565(int x, int y, sysint_t w, void* dst) const
{
  void* converter = NULL;
  switch (format())
  {
    case FormatARGB32:
    case FormatPRGB32:
    case FormatRGB32:
      converter = (void*)Raster::functionMap->convert.rgb16_5650_from_rgb32;
      break;
    case FormatRGB24:
      converter = (void*)Raster::functionMap->convert.rgb16_5650_from_rgb24;
      break;
    case FormatI8:
      converter = (void*)Raster::functionMap->convert.rgb16_5650_from_i8;
      break;
  }
  image_getdib(this, x, y, w, dst, converter);
}

void Image::getDibRgb16_565_bs(int x, int y, sysint_t w, void* dst) const
{
  void* converter = NULL;
  switch (format())
  {
    case FormatARGB32:
    case FormatPRGB32:
    case FormatRGB32:
      converter = (void*)Raster::functionMap->convert.rgb16_5650_bs_from_rgb32;
      break;
    case FormatRGB24:
      converter = (void*)Raster::functionMap->convert.rgb16_5650_bs_from_rgb24;
      break;
    case FormatI8:
      converter = (void*)Raster::functionMap->convert.rgb16_5650_bs_from_i8;
      break;
  }
  image_getdib(this, x, y, w, dst, converter);
}

void Image::setDibRgb16_565(int x, int y, sysint_t w, const void* src)
{
  void* converter = NULL;
  switch (format())
  {
    case FormatARGB32:
    case FormatPRGB32:
    case FormatRGB32:
      converter = (void*)Raster::functionMap->convert.rgb32_from_rgb16_5650;
      break;
    case FormatRGB24:
      converter = (void*)Raster::functionMap->convert.rgb24_from_rgb16_5650;
      break;
  }
  image_setdib(this, x, y, w, src, converter);
}

void Image::setDibRgb16_565_bs(int x, int y, sysint_t w, const void* src)
{
  void* converter = NULL;
  switch (format())
  {
    case FormatARGB32:
    case FormatPRGB32:
    case FormatRGB32:
      converter = (void*)Raster::functionMap->convert.rgb32_from_rgb16_5650_bs;
      break;
    case FormatRGB24:
      converter = (void*)Raster::functionMap->convert.rgb24_from_rgb16_5650_bs;
      break;
  }
  image_setdib(this, x, y, w, src, converter);
}

// ============================================================================
// [Fog::Image - Swap RGB and RGBA]
// ============================================================================

err_t Image::swapRgb()
{
  if (isEmpty()) return Error::Ok;
  if (format() == FormatA8) return Error::Ok;

  err_t err;
  if ( (err = detach()) ) return err;

  Data* d = _d;

  int x, y;
  int w = d->width;
  int h = d->height;

  sysint_t stride = d->stride;
  uint8_t* dst = d->first;

  switch (format())
  {
    case FormatI8:
      dst = (uint8_t*)d->palette.mData();
      if (!dst) return Error::OutOfMemory;
      w = 256;
      h = 1;
      // ... fall through ...

    case FormatARGB32:
    case FormatPRGB32:
    case FormatRGB32:
#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
      dst += 0;
#else
      dst += 1;
#endif // FOG_BYTE_ORDER
      for (y = h; y; y--, dst += stride)
      {
        uint8_t* dstCur = dst;
        for (x = w; x; x--, dstCur += 4)
        {
          uint8_t t = dstCur[0];
          dstCur[0] = dstCur[2];
          dstCur[2] = t;
        }
      }
      break;

    case FormatRGB24:
      for (y = h; y; y--, dst += stride)
      {
        uint8_t* dstCur = dst;
        for (x = w; x; x--, dstCur += 3)
        {
          uint8_t t = dstCur[0];
          dstCur[0] = dstCur[2];
          dstCur[2] = t;
        }
      }
      break;

    default:
      FOG_ASSERT_NOT_REACHED();
      break;
  }

  return Error::Ok;
}

err_t Image::swapRgba()
{
  if (isEmpty()) return Error::Ok;

  // These formats have only alpha values
  if (format() == FormatA8) return Error::Ok;
  if (format() == FormatRGB24) return swapRgb();

  err_t err;
  if ( (err = detach()) ) return err;

  Data* d = _d;

  int x, y;
  int w = d->width;
  int h = d->height;

  sysint_t stride = d->stride;
  uint8_t* dst = d->first;

  switch (format())
  {
    case FormatI8:
      dst = (uint8_t*)d->palette.mData();
      if (!dst) return Error::OutOfMemory;
      w = 256;
      h = 1;
      // ... fall through ...

    case FormatARGB32:
    case FormatPRGB32:
    case FormatRGB32:
    {
      for (y = h; y; y--, dst += stride)
      {
        uint8_t* dstCur = dst;
        for (x = w; x; x--, dstCur += 4)
        {
          ((uint32_t *)dstCur)[0] = Memory::bswap32(((uint32_t *)dstCur)[0]);
        }
      }
      break;
    }

    default:
      FOG_ASSERT_NOT_REACHED();
      break;
  }

  return Error::Ok;
}

// ============================================================================
// [Fog::Image - Premultiply / Demultiply]
// ============================================================================

err_t Image::premultiply()
{
  return convert(FormatPRGB32);
}

err_t Image::demultiply()
{
  if (format() == FormatPRGB32)
    return convert(FormatARGB32);
  else
    return Error::Ok;
}

// ============================================================================
// [Fog::Image - Invert]
// ============================================================================

err_t Image::invert(Image& dst, const Image& src, uint32_t invertMode)
{
  if (invertMode >= 16) return Error::InvalidArgument;

  int format = src.format();

  // First check for some invertion flags in source image format.
  if (src.isEmpty() ||
      invertMode == 0 ||
      (!(invertMode & (InvertRgb)) && (format == FormatRGB32 || format == FormatRGB24)) ||
      (!(invertMode & InvertAlpha) && (format == FormatA8)))
  {
    return dst.set(src);
  }

  // Destination and source can share same data.
  err_t err;
  if (dst._d != src._d)
    err = dst.create(src.width(), src.height(), format);
  else
    err = dst.detach();
  if (err) return err;

  // Prepare data.
  Data* dst_d = dst._d;
  Data* src_d = src._d;

  sysint_t dstStride = dst_d->stride;
  sysint_t srcStride = src_d->stride;

  uint8_t* dstPixels = dst_d->first;
  uint8_t* srcPixels = src_d->first;

  uint8_t* dstCur;
  uint8_t* srcCur;

  int w = dst_d->width;
  int h = dst_d->height;
  int x;
  int y;

  // Don't touch unused channel for XRGB32 format/
  if (format == FormatRGB32) invertMode &= ~InvertAlpha;

  // some pixel formats needs special invertion process
  switch (format)
  {
    case FormatI8:
      dstCur = (uint8_t*)dst_d->palette.mData();
      if (!dstCur) return Error::OutOfMemory;
      srcCur = (uint8_t*)src_d->palette.mData();
      if (!srcCur) return Error::OutOfMemory;
      w = 256;
      h = 1;
      // ... fall through ...

    case FormatARGB32:
    case FormatPRGB32:
    case FormatRGB32:
    {
      uint32_t mask = 0;

      if (invertMode & InvertRed  ) mask |= Rgba::RedMask;
      if (invertMode & InvertGreen) mask |= Rgba::GreenMask;
      if (invertMode & InvertBlue ) mask |= Rgba::BlueMask;
      if (invertMode & InvertAlpha && format == FormatARGB32) mask |= Rgba::AlphaMask;

      if (format == FormatPRGB32)
      {
        for (y = h; y; y--, dstPixels += dstStride, srcPixels += srcStride)
        {
          Raster::functionMap->convert.argb32_from_prgb32(dstPixels, srcPixels, w);

          dstCur = dstPixels;
          for (x = w; x; x--, dstCur += 4) ((uint32_t*)dstCur)[0] ^= mask;

          Raster::functionMap->convert.prgb32_from_argb32(dstPixels, dstPixels, w);
        }
      }
      else
      {
        for (y = h; y; y--, dstPixels += dstStride, srcPixels += srcStride)
        {
          dstCur = dstPixels;
          srcCur = srcPixels;

          for (x = w; x; x--, dstCur += 4, srcCur += 4)
          {
            ((uint32_t*)dstCur)[0] = ((uint32_t *)srcCur)[0] ^ mask;
          }
        }
      }
      break;
    }

    case FormatRGB24:
    {
      uint8_t mask0 = 0;
      uint8_t mask1 = 0;
      uint8_t mask2 = 0;

#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
      if (invertMode & InvertRed  ) mask2 |= 0xFF;
      if (invertMode & InvertGreen) mask1 |= 0xFF;
      if (invertMode & InvertBlue ) mask0 |= 0xFF;
#else // FOG_BYTE_ORDER == FOG_BIG_ENDIAN
      if (invertMode & InvertRed  ) mask0 |= 0xFF;
      if (invertMode & InvertGreen) mask1 |= 0xFF;
      if (invertMode & InvertBlue ) mask2 |= 0xFF;
#endif // FOG_BYTE_ORDER

      for (y = h; y; y--, dstPixels += dstStride, srcPixels += srcStride)
      {
        dstCur = dstPixels;
        srcCur = srcPixels;

        for (x = w; x; x--, dstCur += 3, srcCur += 3)
        {
          dstCur[0] = srcCur[0] ^ mask0;
          dstCur[1] = srcCur[1] ^ mask1;
          dstCur[2] = srcCur[2] ^ mask2;
        }
      }
      break;
    }

    case FormatA8:
    {
      // Should be guaranted that alpha invert is set.
      FOG_ASSERT((invertMode & InvertAlpha) != 0);

      for (y = h; y; y--, dstPixels += dstStride, srcPixels += srcStride)
      {
        dstCur = dstPixels;
        srcCur = srcPixels;

        x = w;

        // this will make main loop a much faster
        while (((sysuint_t)dstCur & 7) != 0 && x)
        {
          *dstCur++ = *srcCur++ ^ 0xFF;
          x--;
        }

        while (x >= 8)
        {
#if FOG_ARCH_BITS == 32
          ((uint32_t *)dstCur)[0] = ((uint32_t *)srcCur)[0] ^ 0xFFFFFFFF;
          ((uint32_t *)dstCur)[1] = ((uint32_t *)srcCur)[1] ^ 0xFFFFFFFF;
#else
          ((uint64_t *)dstCur)[0] = ((uint64_t *)srcCur)[0] ^ FOG_UINT64_C(0xFFFFFFFFFFFFFFFF);
#endif // FOG_ARCH_BITS

          dstCur += 8;
          srcCur += 8;
          x -= 8;
        }

        switch (x)
        {
          case 7: *dstCur++ = *srcCur++ ^ 0xFF;
          case 6: *dstCur++ = *srcCur++ ^ 0xFF;
          case 5: *dstCur++ = *srcCur++ ^ 0xFF;
          case 4: *dstCur++ = *srcCur++ ^ 0xFF;
          case 3: *dstCur++ = *srcCur++ ^ 0xFF;
          case 2: *dstCur++ = *srcCur++ ^ 0xFF;
          case 1: *dstCur++ = *srcCur++ ^ 0xFF;
        }
      }
      break;
    }

    default:
      FOG_ASSERT_NOT_REACHED();
      break;
  }

  return Error::Ok;
}

// ============================================================================
// [Fog::Image - Mirror]
// ============================================================================

typedef void (FOG_FASTCALL *MirrorFunc)(uint8_t*, uint8_t*, sysuint_t);

static void FOG_FASTCALL mirror_copy_src_is_not_dst_32(uint8_t* dst, uint8_t* src, sysuint_t w)
{ Memory::copy(dst, src, Raster::mul4(w)); }

static void FOG_FASTCALL mirror_copy_src_is_not_dst_24(uint8_t* dst, uint8_t* src, sysuint_t w)
{ Memory::copy(dst, src, Raster::mul3(w)); }

static void FOG_FASTCALL mirror_copy_src_is_not_dst_8(uint8_t* dst, uint8_t* src, sysuint_t w)
{ Memory::copy(dst, src, w); }

static void FOG_FASTCALL mirror_flip_src_is_not_dst_32(uint8_t* dst, uint8_t* src, sysuint_t w)
{
  src += Raster::mul4(w - 1);

  sysuint_t x;
  for (x = w; x; x--, dst += 4, src -= 4) Memory::copy4B(dst, src);
}

static void FOG_FASTCALL mirror_flip_src_is_not_dst_24(uint8_t* dst, uint8_t* src, sysuint_t w)
{
  src += Raster::mul3(w) - 3;

  for (sysuint_t x = w; x; x--, dst += 3, src -= 3) Memory::copy3B(dst, src);
}

static void FOG_FASTCALL mirror_flip_src_is_not_dst_8(uint8_t* dst, uint8_t* src, sysuint_t w)
{
  src += w - 1;

  for (sysuint_t x = w; x; x--, dst++, src--) dst[0] = src[0];
}

static void FOG_FASTCALL mirror_copy_src_is_dst_32(uint8_t* dst, uint8_t* src, sysuint_t w)
{
  Memory::xchg(dst, src, Raster::mul4(w));
}

static void FOG_FASTCALL mirror_copy_src_is_dst_24(uint8_t* dst, uint8_t* src, sysuint_t w)
{
  Memory::xchg(dst, src, Raster::mul3(w));
}

static void FOG_FASTCALL mirror_copy_src_is_dst_8(uint8_t* dst, uint8_t* src, sysuint_t w)
{
  Memory::xchg(dst, src, w);
}

static void FOG_FASTCALL mirror_flip_src_is_dst_32(uint8_t* dst, uint8_t* src, sysuint_t w)
{
  sysuint_t x = w;
  if (src == dst) x >>= 1;

  src += ((w - 1) << 2);
  for (; x; x--, dst += 4, src -= 4) Memory::xchg4B(dst, src);
}

static void FOG_FASTCALL mirror_flip_src_is_dst_24(uint8_t* dst, uint8_t* src, sysuint_t w)
{
  sysuint_t x = w;
  if (src == dst) x >>= 1;

  src += Raster::mul3(w) - 3;
  for (; x; x--, dst += 3, src -= 3)
  {
    Memory::xchg2B((uint16_t*)dst, (uint16_t*)src);
    Memory::xchg1B(dst + 2, src + 2);
  }
}

static void FOG_FASTCALL mirror_flip_src_is_dst_8(uint8_t* dst, uint8_t* src, sysuint_t w)
{
  sysuint_t x = w;
  if (src == dst) x >>= 1;

  src += w - 1;
  for (; x; x--, dst += 3, src -= 3) Memory::xchg1B(dst, src);
}

static const MirrorFunc mirror_funcs_copy_src_is_not_dst[] =
{
  NULL,
  mirror_copy_src_is_not_dst_8,
  NULL,
  mirror_copy_src_is_not_dst_24,
  mirror_copy_src_is_not_dst_32
};

static const MirrorFunc mirror_funcs_flip_src_is_not_dst[] =
{
  NULL,
  mirror_flip_src_is_not_dst_8,
  NULL,
  mirror_flip_src_is_not_dst_24,
  mirror_flip_src_is_not_dst_32
};

static const MirrorFunc mirror_funcs_copy_src_is_dst[] =
{
  NULL,
  mirror_copy_src_is_dst_8,
  NULL,
  mirror_copy_src_is_dst_24,
  mirror_copy_src_is_dst_32
};

static const MirrorFunc mirror_funcs_flip_src_is_dst[] =
{
  NULL,
  mirror_flip_src_is_dst_8,
  NULL,
  mirror_flip_src_is_dst_24,
  mirror_flip_src_is_dst_32
};

err_t Image::mirror(Image& dst, const Image& src, uint32_t mirrorMode)
{
  if (src.isEmpty() || mirrorMode == 0) return dst.set(src);
  if (mirrorMode >= 4) return Error::InvalidArgument;

  int format = src.format();

  err_t err;
  if (dst._d != src._d)
    err = dst.create(src.width(), src.height(), format);
  else
    err = dst.detach();
  if (err) return err;

  Data* dst_d = dst._d;
  Data* src_d = src._d;

  sysint_t dstStride = dst_d->stride;
  sysint_t srcStride = src_d->stride;

  uint8_t* dstPixels = dst_d->first;
  uint8_t* srcPixels = src_d->first;

  int w = dst_d->width;
  int h = dst_d->height;

  int bpp = src.bytesPerPixel();

  MirrorFunc func;

  switch (mirrorMode)
  {
    case MirrorVertical:
      srcPixels += srcStride * ((sysint_t)h - 1);
      srcStride = -srcStride;

      if (dst_d != src_d)
      {
        func = mirror_funcs_copy_src_is_not_dst[bpp];
      }
      else
      {
        func = mirror_funcs_copy_src_is_dst[bpp];
        h >>= 1;
      }
      break;

    case MirrorHorizontal:
      if (dst_d != src_d)
      {
        func = mirror_funcs_flip_src_is_not_dst[bpp];
      }
      else
      {
        func = mirror_funcs_flip_src_is_dst[bpp];
      }
      break;

    case MirrorBoth:
      srcPixels += srcStride * ((sysint_t)h - 1);
      srcStride = -srcStride;

      if (dst_d != src_d)
      {
        func = mirror_funcs_flip_src_is_not_dst[bpp];
      }
      else
      {
        func = mirror_funcs_flip_src_is_dst[bpp];
        h >>= 1;
      }
      break;
  }

  for (int y = h; y; y--, dstPixels += dstStride, srcPixels += srcStride)
  {
    func(dstPixels, srcPixels, w);
  }

  return Error::Ok;
}

// ============================================================================
// [Fog::Image - Rotate]
// ============================================================================

err_t Image::rotate(Image& dst, const Image& src, uint32_t rotateMode)
{
  if (rotateMode >= 4) return Error::InvalidArgument;
  if (rotateMode == Rotate0 || src.isEmpty()) return dst.set(src);

  // rotate by 180 degrees has same effect as MirrorBoth, so we will simply 
  // check for this.
  if (rotateMode == Rotate180) return mirror(dst, src, MirrorBoth);

  // Now we have only two possibilities:
  // - rotate by 90
  // - rotate by 270

  // Destination == source?
  if (&dst == &src) return rotate(dst, Image(src), rotateMode);

  err_t err = dst.create(src.height(), src.width(), src.format());
  if (err) return err;

  Data* dst_d = dst._d;
  Data* src_d = src._d;

  sysint_t dstStride = dst_d->stride;
  sysint_t srcStride = src_d->stride;

  uint8_t* dstPixels = dst_d->first;
  uint8_t* srcPixels = src_d->first;

  uint8_t* dstCur;
  uint8_t* srcCur;

  sysint_t sz1 = (sysuint_t)src_d->width;
  sysint_t sz2 = (sysuint_t)src_d->height;

  sysint_t sz1m1 = sz1-1;
  sysint_t sz2m1 = sz2-1;

  sysint_t i;
  sysint_t j;

  sysint_t srcInc1;
  sysint_t srcInc2;

  sysint_t bpp = src.bytesPerPixel();

  if (rotateMode == Image::Rotate90)
  {
    srcPixels += ((sysint_t)sz2m1 * srcStride);
    srcInc1 = bpp;
    srcInc2 = -srcStride;
  }
  else
  {
    srcPixels += sz1m1 * bpp;
    srcInc1 = -bpp;
    srcInc2 = srcStride;
  }

  switch (bpp)
  {
    case 1:
      for (i = sz1; i; i--, dstPixels += dstStride, srcPixels += srcInc1)
      {
        dstCur = dstPixels;
        srcCur = srcPixels;

        for (j = sz2; j; j--, dstCur += 1, srcCur += srcInc2) dstCur[0] = srcCur[0];
      }
      break;
    case 3:
      for (i = sz1; i; i--, dstPixels += dstStride, srcPixels += srcInc1)
      {
        dstCur = dstPixels;
        srcCur = srcPixels;

        for (j = sz2; j; j--, dstCur += 3, srcCur += srcInc2) Memory::copy3B(dstCur, srcCur);
      }
      break;
    case 4:
      for (i = sz1; i; i--, dstPixels += dstStride, srcPixels += srcInc1)
      {
        dstCur = dstPixels;
        srcCur = srcPixels;

        for (j = sz2; j; j--, dstCur += 4, srcCur += srcInc2) Memory::copy4B(dstCur, srcCur);
      }
      break;

    default:
      FOG_ASSERT_NOT_REACHED();
      break;
  }

  return Error::Ok;
}

// ============================================================================
// [Fog::Image - Channel related]
// ============================================================================

Image Image::extractChannel(uint32_t channel) const
{
  Image i;

  // Invalid argument error.
  if (channel >= ChannelCount) return i;
  if ((channel & (channel-1)) != channel) return i;

  // Requested alpha channel on A8 image?
  if (channel == ChannelAlpha && format() == FormatA8)
  {
    i = *this;
    return i;
  }

  // Create image to hold 8 bit channel.
  if (i.create(width(), height(), FormatA8) != Error::Ok) return i;

  // We treat images without alpha channel as full opacity images.
  if (channel == ChannelAlpha && (format() == FormatRGB32 || format() == FormatRGB24))
  {
    i.clear(0xFF);
    return i;
  }

  // We treat A8 images as 0xAA000000 data.
  if (channel != ChannelAlpha && format() == FormatA8)
  {
    i.clear(0x00);
    return i;
  }

  // Conversion...
  sysint_t dstStride = i._d->stride;
  sysint_t srcStride = _d->stride;

  uint8_t* dstBuf = i._d->first;
  const uint8_t* srcBuf = _d->first;

  sysint_t y, h = i._d->height;
  sysint_t srcStart = 0;
  sysint_t srcInc = _d->bytesPerPixel;

  switch (format())
  {
    case FormatARGB32:
    case FormatPRGB32:
    case FormatRGB32:
    case FormatI8:
      if (channel == ChannelRed)
        srcStart = Raster::RGB32_RByte;
      else if (channel == ChannelGreen)
        srcStart = Raster::RGB32_GByte;
      else if (channel == ChannelBlue)
        srcStart = Raster::RGB32_BByte;
      else
        srcStart = Raster::RGB32_AByte;
      break;

    case FormatRGB24:
      if (channel == ChannelRed)
        srcStart = Raster::RGB24_RByte;
      else if (channel == ChannelGreen)
        srcStart = Raster::RGB24_GByte;
      else if (channel == ChannelBlue)
        srcStart = Raster::RGB24_BByte;
      break;
  }

  if (format() != FormatI8)
  {
    for (y = 0; y < h; y++, dstBuf += dstStride, srcBuf += srcStride)
    {
      uint8_t* dstCur = dstBuf;
      const uint8_t* srcCur = srcBuf;

      dstCur[0] = srcCur[0];
      dstCur++;
      srcCur += srcInc;
    }
  }
  else
  {
    const uint8_t* pal = reinterpret_cast<const uint8_t*>(_d->palette.cData()) + srcStart;
    for (y = 0; y < h; y++, dstBuf += dstStride, srcBuf += srcStride)
    {
      uint8_t* dstCur = dstBuf;
      const uint8_t* srcCur = srcBuf;

      dstCur[0] = pal[srcCur[0] << 2];
      dstCur++;
      srcCur++;
    }
  }

  return i;
}

// ============================================================================
// [Fog::Image - Filtering]
// ============================================================================

err_t Image::filter(const ImageFilter& f)
{
  return f.filterImage(*this, *this);
}

err_t Image::filter(const ImageFilter& f, const Rect& r)
{
  int w = width();
  int h = height();

  // Clip.
  int x1 = Math::max(r.x(), 0);
  int y1 = Math::max(r.y(), 0);
  int x2 = Math::min(r.x(), w);
  int y2 = Math::min(r.y(), h);

  if (x1 >= x2 || y1 >= y2) return Error::Ok;

  if (x1 == 0 && y1 == 0 && x2 == w && y2 == h)
  {
    return f.filterImage(*this, *this);
  }
  else
  {
    err_t err = detach();
    if (err) return err;

    return f.filterData(xFirst(), stride(), xFirst(), stride(), w, h, format());
  }
}

err_t Image::filter(const ColorLut& lut)
{
  return filter(lut, Rect(0, 0, width(), height()));
}

err_t Image::filter(const ColorLut& lut, const Rect& r)
{
  int x1 = r.x1();
  int y1 = r.y1();
  int x2 = r.x2();
  int y2 = r.y2();

  int w = _d->width;
  int h = _d->height;
  int fmt = _d->format;

  if (x1 < 0) x1 = 0;
  if (y1 < 0) y1 = 0;
  if (x2 > w) x2 = w;
  if (y2 > h) y2 = h;

  if ((w = x2 - x1) <= 0) return Error::Ok;
  if ((h = y2 - y1) <= 0) return Error::Ok;

  err_t err;
  if ((err = detach())) return err;

  uint8_t* dstPixels = xScanline(y1) + x1 * bytesPerPixel();
  sysint_t dstStride = stride();

  Raster::ColorLutFn converter = Raster::functionMap->filters.colorLut[format()];
  const ColorLut::Table* table = &lut._d->table;

  for (int y = y1; y < y2; y++, dstPixels += dstStride)
  {
    converter(dstPixels, dstPixels, table, w);
  }

  return Error::Ok;
}

err_t Image::filter(const ColorMatrix& mat)
{
  return filter(mat, Rect(0, 0, width(), height()));
}

err_t Image::filter(const ColorMatrix& mat, const Rect& r)
{
  int x1 = r.x1();
  int y1 = r.y1();
  int x2 = r.x2();
  int y2 = r.y2();

  int w = _d->width;
  int h = _d->height;
  int fmt = _d->format;

  if (x1 < 0) x1 = 0;
  if (y1 < 0) y1 = 0;
  if (x2 > w) x2 = w;
  if (y2 > h) y2 = h;

  if ((w = x2 - x1) <= 0) return Error::Ok;
  if ((h = y2 - y1) <= 0) return Error::Ok;

  err_t err;
  if ((err = detach())) return err;

  uint8_t* dstPixels = xScanline(y1) + x1 * bytesPerPixel();
  sysint_t dstStride = stride();

  Raster::ColorMatrixFn converter = Raster::functionMap->filters.colorMatrix[format()];
  uint32_t type = mat.type();

  for (int y = y1; y < y2; y++, dstPixels += dstStride)
  {
    converter(dstPixels, dstPixels, &mat, type, w);
  }

  return Error::Ok;
}

// ============================================================================
// [Fog::Image - Painting]
// ============================================================================

static Raster::SpanSolidFn getSpanSolidBlitter(int format, bool over)
{
  Raster::FunctionMap* m = Raster::functionMap;
  switch (format)
  {
    case Image::FormatARGB32:
      return m->raster_argb32[0][over ? CompositeSrcOver : CompositeSrc].span_solid;
    case Image::FormatPRGB32:
      return m->raster_argb32[1][over ? CompositeSrcOver : CompositeSrc].span_solid;
    case Image::FormatRGB32:
      return m->raster_rgb32.span_solid;
    case Image::FormatRGB24:
      return m->raster_rgb24.span_solid;
    case Image::FormatA8:
    case Image::FormatI8:
    default:
      return NULL;
  }
}

static Raster::SpanCompositeFn getSpanCompositeBlitter(int format, bool over)
{
  Raster::FunctionMap* m = Raster::functionMap;
  switch (format)
  {
    case Image::FormatARGB32:
      return m->raster_argb32[0][over ? CompositeSrcOver : CompositeSrc].span_composite[Image::FormatARGB32];
    case Image::FormatPRGB32:
      return m->raster_argb32[1][over ? CompositeSrcOver : CompositeSrc].span_composite[Image::FormatPRGB32];
    case Image::FormatRGB32:
      return m->raster_rgb32.span_composite[Image::FormatARGB32];
    case Image::FormatRGB24:
      return m->raster_rgb24.span_composite[Image::FormatARGB32];
    case Image::FormatA8:
    case Image::FormatI8:
    default:
      return NULL;
  }
}

err_t Image::clear(uint32_t c0)
{
  return fillRect(Rect(0, 0, width(), height()), c0, false);
}

err_t Image::drawPixel(const Point& pt, uint32_t c0)
{
  if ((uint)pt.x() >= (uint)width() || (uint)pt.y() >= (uint)height())
    return Error::Ok;

  err_t err = detach();
  if (err) return err;

  uint8_t* dstCur = _d->first + pt.y() * _d->stride;

  switch (format())
  {
    case FormatARGB32:
    case FormatPRGB32:
    case FormatRGB32:
      Raster::PixFmt_ARGB32::store(dstCur + Raster::mul4(pt.x()), c0);
      break;
    case FormatRGB24:
      Raster::PixFmt_RGB24::store(dstCur + Raster::mul3(pt.x()), c0);
      break;
    case FormatA8:
    case FormatI8:
      dstCur[Raster::mul1(pt.x())] = (uint8_t)c0;
      break;
  }

  return Error::Ok;
}

template<typename Op>
static void Draw_BresenhamLine(uint8_t* dst, sysint_t dstStride, uint32_t c0, Raster::BresenhamLineIterator& line, bool last)
{
  sysint_t dstIncX = Op::BytesPerPixel;
  sysint_t dstIncY = dstStride;

  if (line.incx < 0) dstIncX = -dstIncX;
  if (line.incy < 0) dstIncY = -dstIncY;

  if (line.hOriented())
  {
    while (!line.hDone())
    {
      Op::store(dst, c0);
      line.hIncPtr(&dst, dstIncX, dstIncY);
    }
  }
  else
  {
    while (!line.vDone())
    {
      Op::store(dst, c0);
      line.vIncPtr(&dst, dstIncX, dstIncY);
    }
  }

  // TODO: When clipped it's omitted ?
  if (last) Op::store(dst, c0);
}

err_t Image::drawLine(const Point& pt0, const Point& pt1, uint32_t c0, bool lastPoint)
{
  if (isEmpty())
    return Error::Ok;

  Raster::BresenhamLineIterator line;
  if (!line.initAndClip(pt0.x(), pt0.y(), pt1.x(), pt1.y(), 0, 0, width()-1, height()-1))
    return Error::Ok;

  err_t err = detach();
  if (err) return err;

  sysint_t dstStride = _d->stride;
  uint8_t* dstCur = _d->first + line.y * dstStride;

  switch (format())
  {
    case FormatARGB32:
    case FormatPRGB32:
    case FormatRGB32:
      dstCur += Raster::mul4(line.x);
      Draw_BresenhamLine<Raster::PixFmt_ARGB32>(dstCur, dstStride, c0, line, lastPoint);
      break;

    case FormatRGB24:
      dstCur += Raster::mul3(line.x);
      Draw_BresenhamLine<Raster::PixFmt_RGB24>(dstCur, dstStride, c0, line, lastPoint);
      break;

    case FormatA8:
    case FormatI8:
      dstCur += Raster::mul1(line.x);
      Draw_BresenhamLine<Raster::PixFmt_A8>(dstCur, dstStride, c0, line, lastPoint);
      break;
  }

  return Error::Ok;
}

err_t Image::fillRect(const Rect& r, uint32_t c0, bool over)
{
  int x1 = r.x1();
  int y1 = r.y1();
  int x2 = r.x2();
  int y2 = r.y2();

  int w = _d->width;
  int h = _d->height;
  int fmt = _d->format;

  int op = CompositeSrc;
  if (over && (c0 & 0xFF000000) != 0xFF000000) op = CompositeSrcOver;

  if (x1 < 0) x1 = 0;
  if (y1 < 0) y1 = 0;
  if (x2 > w) x2 = w;
  if (y2 > h) y2 = h;

  if ((w = x2 - x1) <= 0) return Error::Ok;
  if ((h = y2 - y1) <= 0) return Error::Ok;

  err_t err = detach();
  if (err) return err;

  sysint_t dstStride = _d->stride;
  uint8_t* dstCur = _d->first + y1 * dstStride + x1 * bytesPerPixel();

  Raster::SpanSolidFn blitter = getSpanSolidBlitter(fmt, over);

  for (int i = 0; i < h; i++, dstCur += dstStride)
    blitter(dstCur, c0, w);

  return Error::Ok;
}

// ============================================================================
// [Fog::Image - Painting - Gradients]
// ============================================================================

err_t Image::fillQGradient(const Rect& r, Rgba c0, Rgba c1, Rgba c2, Rgba c3, bool over)
{
  if (format() == FormatI8) return Error::InvalidFunction;

  // Optimized variants.
  if (c0 == c1 && c2 == c3) return fillVGradient(r, c0, c2, over);
  if (c0 == c2 && c1 == c3) return fillHGradient(r, c0, c1, over);

  int i;

  int x1 = r.x1();
  int y1 = r.y1();
  int x2 = r.x2();
  int y2 = r.y2();

  int w = _d->width;
  int h = _d->height;
  int fmt = _d->format;

  int op = CompositeSrc;

  if (over && (
    (c0 & 0xFF000000) != 0xFF000000 ||
    (c1 & 0xFF000000) != 0xFF000000 ||
    (c2 & 0xFF000000) != 0xFF000000 ||
    (c3 & 0xFF000000) != 0xFF000000)) op = CompositeSrcOver;

  if (x1 < 0) x1 = 0;
  if (y1 < 0) y1 = 0;
  if (x2 > w) x2 = w;
  if (y2 > h) y2 = h;

  if ((w = x2 - x1) <= 0) return Error::Ok;
  if ((h = y2 - y1) <= 0) return Error::Ok;

  err_t err = detach();
  if (err) return err;

  sysint_t dstStride = _d->stride;
  uint8_t* dstCur = _d->first + y1 * dstStride + x1 * bytesPerPixel();

  MemoryBuffer<512> memStorage;
  uint8_t* mem = (uint8_t*)memStorage.alloc(h * 2 * sizeof(uint32_t) + w * sizeof(uint32_t));
  if (!mem) return Error::OutOfMemory;

  uint8_t* shade0 = mem;
  uint8_t* shade1 = shade0 + h * sizeof(uint32_t);
  uint8_t* shadeW = shade1 + h * sizeof(uint32_t);

  Raster::GradientSpanFn gradientSpan;

  // Interpolate vertical lines (c0 to c2 and c1 to c3).
  gradientSpan = Raster::functionMap->gradient.gradient_argb32;
  gradientSpan(shade0, c0, c2, h, 0, h);
  gradientSpan(shade1, c1, c3, h, 0, h);

  switch (fmt)
  {
    case FormatPRGB32:
      // We must premultiply if dst is premultiplied. The reason why we are
      // setting it here and not before is that all gradient functions need
      // colors in ARGB colorspace.
      gradientSpan = Raster::functionMap->gradient.gradient_prgb32;
      // ... fall throught ...
    case FormatARGB32:
    case FormatRGB32:
      if (!over)
      {
        for (i = 0; i < h; i++,
          dstCur += dstStride,
          shade0 += sizeof(uint32_t),
          shade1 += sizeof(uint32_t))
        {
          gradientSpan(dstCur, ((uint32_t*)shade0)[0], ((uint32_t*)shade1)[0], w, 0, w);
        }
        break;
      }
      // ... fall thgouht ...
    default:
    {
      Raster::SpanCompositeFn blitter = getSpanCompositeBlitter(fmt, over);

      for (i = 0; i < h; i++,
        dstCur += dstStride,
        shade0 += sizeof(uint32_t),
        shade1 += sizeof(uint32_t))
      {
        gradientSpan(shadeW, ((uint32_t*)shade0)[0], ((uint32_t*)shade1)[0], w, 0, w);
        blitter(dstCur, shadeW, w);
      }
      break;
    }
  }
  return Error::Ok;
}

err_t Image::fillHGradient(const Rect& r, Rgba c0, Rgba c1, bool over)
{
  if (format() == FormatI8) return Error::InvalidFunction;

  if (c0 == c1) return fillRect(r, c0, over);

  int x1 = r.x1();
  int y1 = r.y1();
  int x2 = r.x2();
  int y2 = r.y2();

  int w = _d->width;
  int h = _d->height;
  int fmt = _d->format;

  int op = CompositeSrc;

  if (over && (
    (c0 & 0xFF000000) != 0xFF000000 ||
    (c1 & 0xFF000000) != 0xFF000000)) op = CompositeSrcOver;

  if (x1 < 0) x1 = 0;
  if (y1 < 0) y1 = 0;
  if (x2 > w) x2 = w;
  if (y2 > h) y2 = h;

  if ((w = x2 - x1) <= 0) return Error::Ok;
  if ((h = y2 - y1) <= 0) return Error::Ok;

  err_t err = detach();
  if (err) return err;

  sysint_t dstStride = _d->stride;
  uint8_t* dstCur = _d->first + y1 * dstStride + x1 * bytesPerPixel();

  MemoryBuffer<512> memStorage;
  uint8_t* shade0 = (uint8_t*)memStorage.alloc(w * sizeof(uint32_t));
  if (!shade0) return Error::OutOfMemory;

  Raster::GradientSpanFn gradientSpan =
    fmt == FormatPRGB32
      ? Raster::functionMap->gradient.gradient_prgb32
      : Raster::functionMap->gradient.gradient_argb32;

  gradientSpan(shade0, c0, c1, w, 0, w);

  Raster::SpanCompositeFn blitter = getSpanCompositeBlitter(fmt, over);

  for (int i = 0; i < h; i++, dstCur += dstStride)
  {
    blitter(dstCur, shade0, w);
  }
  return Error::Ok;
}

err_t Image::fillVGradient(const Rect& r, Rgba c0, Rgba c1, bool over)
{
  if (format() == FormatI8) return Error::InvalidFunction;

  if (c0 == c1) return fillRect(r, c0, over);

  int x1 = r.x1();
  int y1 = r.y1();
  int x2 = r.x2();
  int y2 = r.y2();

  int w = _d->width;
  int h = _d->height;
  int fmt = _d->format;

  int op = CompositeSrc;
  if (over && (
    (c0 & 0xFF000000) != 0xFF000000 ||
    (c1 & 0xFF000000) != 0xFF000000)) op = CompositeSrcOver;

  if (x1 < 0) x1 = 0;
  if (y1 < 0) y1 = 0;
  if (x2 > w) x2 = w;
  if (y2 > h) y2 = h;

  if ((w = x2 - x1) <= 0) return Error::Ok;
  if ((h = y2 - y1) <= 0) return Error::Ok;

  err_t err = detach();
  if (err) return err;

  sysint_t dstStride = _d->stride;
  uint8_t* dstCur = _d->first + y1 * dstStride + x1 * bytesPerPixel();

  MemoryBuffer<512> memStorage;
  uint8_t* mem = (uint8_t*)memStorage.alloc(h * sizeof(uint32_t));
  if (!mem) return Error::OutOfMemory;

  uint8_t* shade0 = mem;

  Raster::GradientSpanFn gradientSpan =
    fmt == FormatPRGB32
      ? Raster::functionMap->gradient.gradient_prgb32
      : Raster::functionMap->gradient.gradient_argb32;

  gradientSpan(shade0, c0, c1, h, 0, h);

  Raster::SpanSolidFn blitter = getSpanSolidBlitter(fmt, over);

  for (int i = 0; i < h; i++, dstCur += dstStride, shade0 += sizeof(uint32_t))
  {
    blitter(dstCur, ((uint32_t*)shade0)[0], w);
  }
  return Error::Ok;
}

// ============================================================================
// [Fog::Image - Painting - Blit]
// ============================================================================

static err_t _blitImage(
  Image::Data* dstD, int dstX, int dstY,
  Image::Data* srcD, int srcX, int srcY,
  int w, int h,
  uint32_t op, uint32_t opacity)
{
  sysint_t dstStride = dstD->stride;
  sysint_t srcStride = srcD->stride;

  uint8_t* dstPixels = dstD->first;
  const uint8_t* srcPixels = srcD->first;

  // Special case if dst and src overlaps.
  if (dstD == srcD && dstY >= srcY && (dstY - srcY) <= h)
  {
    dstPixels += (dstY + h - 1) * dstStride;
    srcPixels += (srcY + h - 1) * srcStride;

    dstStride = -dstStride;
    srcStride = -srcStride;
  }
  else
  {
    dstPixels += dstY * dstStride;
    srcPixels += srcY * srcStride;
  }

  dstPixels += dstX * dstD->bytesPerPixel;
  srcPixels += srcX * srcD->bytesPerPixel;

  int y;

  // Support for overlapping images.
  if (dstD == srcD && dstX >= srcX && (dstX - srcX) <= w)
  {
    MemoryBuffer<512> bufStorage;
    sysint_t bufSize = w * srcD->bytesPerPixel;
    uint8_t* buf = (uint8_t*)bufStorage.alloc(bufSize);
    if (!buf) return Error::OutOfMemory;

    Raster::ConvertPlainFn copy = Raster::functionMap->convert.memcpy8;

    if (opacity >= 255)
    {
      Raster::SpanCompositeFn blit = 
        Raster::getRasterOps(dstD->format, op)->span_composite[srcD->format];

      for (y = h; y; y--, dstPixels += dstStride, srcPixels += srcStride)
      {
        copy(buf, srcPixels, bufSize);
        blit(dstPixels, buf, w);
      }
    }
    else
    {
      Raster::SpanCompositeMskConstFn blit = 
        Raster::getRasterOps(dstD->format, op)->span_composite_a8_const[srcD->format];

      for (y = h; y; y--, dstPixels += dstStride, srcPixels += srcStride)
      {
        copy(buf, srcPixels, bufSize);
        blit(dstPixels, buf, opacity, w);
      }
    }
  }
  // Normal case.
  else
  {
    if (srcD->format == Image::FormatI8)
    {
      const Rgba* palette = srcD->palette.cData();

      if (opacity >= 255)
      {
        Raster::SpanCompositeIndexedFn blit = 
          Raster::getRasterOps(dstD->format, op)->span_composite_indexed[srcD->format];

        for (y = h; y; y--, dstPixels += dstStride, srcPixels += srcStride)
        {
          blit(dstPixels, srcPixels, w, palette);
        }
      }
      else
      {
        Raster::SpanCompositeIndexedMskConstFn blit = 
          Raster::getRasterOps(dstD->format, op)->span_composite_indexed_a8_const[srcD->format];

        for (y = h; y; y--, dstPixels += dstStride, srcPixels += srcStride)
        {
          blit(dstPixels, srcPixels, opacity, w, palette);
        }
      }
    }
    else
    {
      if (opacity >= 255)
      {
        Raster::SpanCompositeFn blit = 
          Raster::getRasterOps(dstD->format, op)->span_composite[srcD->format];

        for (y = h; y; y--, dstPixels += dstStride, srcPixels += srcStride)
        {
          blit(dstPixels, srcPixels, w);
        }
      }
      else
      {
        Raster::SpanCompositeMskConstFn blit = 
          Raster::getRasterOps(dstD->format, op)->span_composite_a8_const[srcD->format];

        for (y = h; y; y--, dstPixels += dstStride, srcPixels += srcStride)
        {
          blit(dstPixels, srcPixels, opacity, w);
        }
      }
    }
  }
  return Error::Ok;
}

err_t Image::drawImage(const Point& pt, const Image& src, uint32_t op, uint32_t opacity)
{
  if (op >= CompositeCount) return Error::InvalidArgument;
  if (opacity == 0) return Error::Ok;

  Data* dst_d = _d;
  Data* src_d = src._d;

  int w = dst_d->width;
  int h = dst_d->height;

  int x1 = pt.x();
  int y1 = pt.y();
  int x2 = x1 + src_d->width;
  int y2 = y1 + src_d->height;

  if (x1 < 0) x1 = 0;
  if (y1 < 0) y1 = 0;
  if (x2 > w) x2 = w;
  if (y2 > h) y2 = h;

  if (x1 >= x2 || y1 >= y2) return Error::Ok;

  err_t err = detach();
  if (err) return err;

  return _blitImage(_d, x1, y1, src_d, x1 - pt.x(), y1 - pt.y(), x2 - x1, y2 - y1, op, opacity);
}

err_t Image::drawImage(const Point& pt, const Image& src, const Rect& srcRect, uint32_t op, uint32_t opacity)
{
  if (op >= CompositeCount) return Error::InvalidArgument;
  if (!srcRect.isValid()) return Error::Ok;
  if (opacity == 0) return Error::Ok;

  Data* dst_d = _d;
  Data* src_d = src._d;

  int srcX1 = srcRect.x1();
  int srcY1 = srcRect.y1();
  int srcX2 = srcRect.x2();
  int srcY2 = srcRect.y2();

  if (srcX1 < 0) srcX1 = 0;
  if (srcY1 < 0) srcY1 = 0;
  if (srcX2 > src_d->width) srcX1 = src_d->width;
  if (srcY2 > src_d->height) srcY2 = src_d->height;

  if (srcX1 >= srcX2 || srcY1 >= srcY2) return Error::Ok;

  int dstX1 = pt.x() + (srcX1 - srcRect.x1());
  int dstY1 = pt.y() + (srcY1 - srcRect.y1());
  int dstX2 = dstX1 + (srcX2 - srcX1);
  int dstY2 = dstY1 + (srcY2 - srcY1);

  if (dstX1 < 0) { srcX1 -= dstX1; dstX1 = 0; }
  if (dstY1 < 0) { srcY1 -= dstY1; dstY1 = 0; }
  if (dstX2 > dst_d->width) dstX2 = dst_d->width;
  if (dstY2 > dst_d->height) dstY2 = dst_d->height;

  if (dstX1 >= dstX2 || dstY1 >= dstY2) return Error::Ok;

  err_t err = detach();
  if (err) return err;

  return _blitImage(_d, dstX1, dstY1, src_d, srcX1, srcY1, dstX2 - dstX1, dstY2 - dstY1, op, opacity);
}

// ============================================================================
// [Fog::Image - Painting - Scroll]
// ============================================================================

err_t Image::scroll(int scrollX, int scrollY)
{
  return scroll(scrollX, scrollY, Rect(0, 0, width(), height()));
}

err_t Image::scroll(int scrollX, int scrollY, const Rect& r)
{
  if (scrollX == 0 && scrollY == 0) return Error::Ok;

  Data* d = _d;

  int x1 = r.x1();
  int y1 = r.y1();
  int x2 = r.x2();
  int y2 = r.y2();

  if (x1 < 0) x1 = 0;
  if (y1 < 0) y1 = 0;
  if (x2 > d->width) x1 = d->width;
  if (y2 > d->height) y2 = d->height;

  if (x1 >= x2 || y1 >= y2) return Error::Ok;

  int absX = abs(scrollX);
  int absY = abs(scrollY);

  int scrollW = x2 - x1;
  int scrollH = y2 - y1;

  int srcX, srcY;
  int dstX, dstY;

  if (absX >= scrollW || absY >= scrollH) return Error::Ok;

  err_t err = detach();
  if (err) return err;

  if (scrollX < 0) { srcX = absX; dstX = 0; } else { srcX = 0; dstX = absX; }
  if (scrollY < 0) { srcY = absY; dstY = 0; } else { srcY = 0; dstY = absY; }

  scrollW -= absX;
  scrollH -= absY;

  sysint_t stride = d->stride;
  sysint_t size = scrollW * d->bytesPerPixel;

  uint8_t* dstPixels = d->first + dstX * d->bytesPerPixel;
  uint8_t* srcPixels = d->first + srcX * d->bytesPerPixel;

  dstPixels += dstY * stride;
  srcPixels += srcY * stride;

  if (scrollY > 0)
  {
    dstPixels += (scrollH - 1) * stride;
    srcPixels += (scrollH - 1) * stride;

    stride = -stride;
  }

  for (int y = scrollH; y; y--, dstPixels += stride, srcPixels += stride)
    memmove(dstPixels, srcPixels, size);

  return Error::Ok;
}

// ============================================================================
// [Fog::Image - Windows Specific]
// ============================================================================

#if defined(FOG_OS_WINDOWS)
/*
  Following code can be used to test Image <=> HBITMAP conversion:

  HDC hdc = CreateCompatibleDC(NULL);
  HBITMAP hbmp = CreateCompatibleBitmap(hdc, 320, 200);
  RECT r = {0 , 0, 320, 200 };

  HGDIOBJ old = SelectObject(hdc, (HGDIOBJ)hbmp);
  FillRect(hdc, &r, (HBRUSH)GetStockObject(WHITE_BRUSH));
  SetBkMode(hdc, TRANSPARENT);
  TextOut(hdc, 0, 0, "Abcdefghijklmnop", 16);
  SelectObject(hdc, old);
  DeleteObject(hdc);

  Image i;
  i.fromHBITMAP(hbmp);

  DeleteObject((HGDIOBJ)hbmp);
*/

HBITMAP Image::toHBITMAP()
{
  Data* d = _d;
  uint8_t* raw;

  if (d->width == 0 || d->height == 0) return NULL;

  // define bitmap attributes
  BITMAPINFO bmi;
  // define dib section handle
  HBITMAP hDibSection;

  Memory::zero(&bmi, sizeof(bmi));
  bmi.bmiHeader.biSize        = sizeof(bmi.bmiHeader);
  bmi.bmiHeader.biWidth       = (int)d->width;
  bmi.bmiHeader.biHeight      = -((int)d->height);
  bmi.bmiHeader.biPlanes      = 1;
  bmi.bmiHeader.biBitCount    = d->depth;
  bmi.bmiHeader.biCompression = BI_RGB;

  // TODO: 8 bit greyscale
  // TODO: ARGB32 and PRGB32 images?
  hDibSection = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&raw, NULL, 0);

  sysuint_t y;
  uint8_t* dest = raw;
  uint8_t* src = d->first;
  
  DIBSECTION info;
  GetObject(hDibSection, sizeof(DIBSECTION), &info);
  
  sysint_t destStride = info.dsBm.bmWidthBytes;
  sysint_t srcStride = d->stride;

  sysuint_t byteWidth = (d->width * d->depth + 7) >> 3;

  for (y = d->height; y; y--, dest += destStride, src += srcStride)
  {
    memcpy(dest, src, byteWidth);
  }

  return hDibSection;
}

bool Image::fromHBITMAP(HBITMAP hBitmap)
{
  BITMAP bm;       // source
  BITMAPINFO dibi; // target (for GetDIBits() function)
  GetObject(hBitmap, sizeof(BITMAP), &bm);
  
  int format;

  switch (bm.bmBitsPixel)
  {
    case 32:
      format = FormatRGB32;
      break;
    case 24:
    default:
      format = FormatRGB24;
      break;
  }

  if (create(bm.bmWidth, bm.bmHeight, format) != Error::Ok) return false;

  if (bm.bmBits)
  {
    // DIB
    // TODO:
  }
  else 
  {
    // DDB
    HDC hdc = CreateCompatibleDC(NULL);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdc, (HGDIOBJ)hBitmap);

    uint8_t* destPixels = _d->first;
    sysint_t destStride = _d->stride;

    memset(&dibi, 0, sizeof(BITMAPINFO));
    dibi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    dibi.bmiHeader.biCompression = BI_RGB;
    dibi.bmiHeader.biWidth = bm.bmWidth;
    dibi.bmiHeader.biHeight = bm.bmHeight;
    dibi.bmiHeader.biBitCount = depth();
    dibi.bmiHeader.biPlanes = 1;
    // GetDIBits(hdc, hBitmap, 0, 1, NULL, &dibi, DIB_RGB_COLORS);

    sysuint_t y;
    for (y = bm.bmHeight; y; y--, destPixels += destStride)
    {
      GetDIBits(hdc, hBitmap, y, 1, (LPVOID)destPixels, &dibi, DIB_RGB_COLORS);
    }

    SelectObject(hdc, (HGDIOBJ)hOldBitmap);
    DeleteDC(hdc);
  }

  return true;
}
#endif // FOG_OS_WINDOWS

// ============================================================================
// [Fog::Image - ImageIO]
// ============================================================================

err_t Image::readFile(const String32& fileName)
{
  err_t err;

  MapFile mapfile;
  Stream stream;
  TemporaryString32<16> extension;

  if ((err = FileUtil::extractExtension(extension, fileName)) ||
      (err = extension.lower()))
  {
    return err;
  }

  if (mapfile.map(fileName, false) == Error::Ok)
  {
    stream.openMemory((void*)mapfile.data(), mapfile.size(), Stream::OpenRead);
    return readStream(stream, extension);
  }
  else
  {
    if ((err = stream.openFile(fileName, Stream::OpenRead))) return err;
    return readStream(stream, extension);
  }
}

err_t Image::readStream(Stream& stream)
{
  if (!stream.isSeekable()) return Error::CantSeek;

  int64_t pos = stream.tell();

  uint8_t mime[128];
  sysuint_t readn = stream.read(mime, 128);

  if (stream.seek(pos, Stream::SeekSet) == -1) return Error::CantSeek;

  ImageIO::Provider* provider;
  ImageIO::DecoderDevice* decoder;

  provider = ImageIO::getProviderByMemory(mime, readn);
  if (!provider) return Error::ImageIO_ProviderNotAvailable;

  decoder = provider->createDecoder();
  if (!decoder) return Error::ImageIO_DecoderNotAvailable;

  err_t err = decoder->readImage(*this);
  delete decoder;

  // Seek to begin if failed
  if (err) stream.seek(pos, Stream::SeekSet);

  return err;
}

err_t Image::readStream(Stream& stream, const String32& extension)
{
  if (!stream.isSeekable()) return Error::ImageIO_NotSeekableStream;

  // First try to use extension, if fail, fallback to readStream(Stream)
  if (!extension.isEmpty())
  {
    ImageIO::Provider* provider;
    ImageIO::DecoderDevice* decoder;

    if ((provider = ImageIO::getProviderByExtension(extension)))
    {
      if ((decoder = provider->createDecoder()))
      {
        int64_t pos = stream.tell();
        err_t err;

        decoder->attachStream(stream);
        err = decoder->readImage(*this);
        delete decoder;

        if (err == Error::Ok) return err;

        // Seek to begin if fail
        if (stream.seek(pos, Stream::SeekSet) == -1)
          return Error::ImageIO_SeekFailure;
      }
    }
  }

  // Fallback to readStream(Stream) without extension support
  return readStream(stream);
}

err_t Image::readMemory(const void* data, sysuint_t size)
{
  Stream stream;
  stream.openMemory((void*)data, size, Stream::OpenRead);
  return readStream(stream);
}

err_t Image::readMemory(const void* data, sysuint_t size, const String32& extension)
{
  Stream stream;
  stream.openMemory((void*)data, size, Stream::OpenRead);
  return readStream(stream, extension);
}

err_t Image::writeFile(const String32& fileName)
{
  Stream stream;

  err_t err = stream.openFile(fileName, 
    Stream::OpenWrite      | 
    Stream::OpenCreate     | 
    Stream::OpenCreatePath | 
    Stream::OpenTruncate   );
  if (err) return err;

  TemporaryString32<16> extension;
  if ((err = FileUtil::extractExtension(extension, fileName)) ||
      (err = extension.lower()))
  {
    return err;
  }

  err = writeStream(stream, extension);
  return err;
}

err_t Image::writeStream(Stream& stream, const String32& extension)
{
  ImageIO::Provider* provider;
  ImageIO::EncoderDevice* encoder;

  if ((provider = ImageIO::getProviderByExtension(extension)))
  {
    if ((encoder = provider->createEncoder()))
    {
      err_t err;

      encoder->attachStream(stream);
      err = encoder->writeImage(*this);
      delete encoder;
      return err;
    }
    else
      return Error::ImageIO_EncoderNotAvailable;
  }
  else
    return Error::ImageIO_ProviderNotAvailable;
}

// ============================================================================
// [Fog::Image - Stride]
// ============================================================================

sysint_t Image::calcStride(int width, int depth)
{
  sysint_t result = 0;

  switch (depth)
  {
    case   1: result = (width + 7) >> 3; break;
    case   4: result = (width + 1) >> 1; break;
    // X server can be configured for these!
    case   5:
    case   6:
    case   7:
    case   8: result = width; break;

    case  15:
    case  16: result = width *  2; break;
    case  24: result = width *  3; break;
    case  32: result = width *  4; break;

    // Future?
    case  48: result = width *  6; break;
    case  64: result = width *  8; break;
    case  96: result = width * 12; break; 
    case 128: result = width * 16; break;

    // all others are invalid
    default:
      fog_fail("Image::calcStride() - invalid depth %d", depth);
  }

  // Align to 32 bits boudary
  result += 3;
  result &= ~3;

  // Overflow
  if (result < width) return 0;

  // Success
  return result;
}

int Image::formatToDepth(int format)
{
  switch (format)
  {
    case FormatARGB32:
    case FormatPRGB32:
    case FormatRGB32:
      return 32;
    case FormatRGB24:
      return 24;
    case FormatA8:
    case FormatI8:
      return 8;
    // Everything else is invalid.
    default:
      return 0;
  }
}

int Image::formatToBytesPerPixel(int format)
{
  switch (format)
  {
    case FormatARGB32:
    case FormatPRGB32:
    case FormatRGB32:
      return 4;
    case FormatRGB24:
      return 3;
    case FormatA8:
    case FormatI8:
      return 1;
    // Everything else is invalid.
    default:
      return 0;
  }
}

// ============================================================================
// [Fog::Image::Data]
// ============================================================================

Image::Data::Data() {}
Image::Data::~Data() {}

Image::Data* Image::Data::ref() const
{
  if (flags & IsSharable)
    return refAlways();
  else
    return copy(this);
}

void Image::Data::deref()
{
  if (refCount.deref())
  {
    bool dynamic = (flags & IsDynamic) != 0;
    this->~Data();
    if (dynamic) Memory::free(this);
  }
}

Image::Data* Image::Data::alloc(sysuint_t size)
{
  sysuint_t dsize = sizeof(Data) - sizeof(uint32_t) + size;
  Data* d = (Data*)Memory::alloc(dsize);
  if (!d) return NULL;

  new (d) Data();
  d->refCount.init(1);
  d->flags = Data::IsDynamic | Data::IsSharable;
  d->width = 0;
  d->height = 0;
  d->format = 0;
  d->depth = 0;
  d->bytesPerPixel = 0;
  d->stride = 0;
  d->data = (size != 0) ? d->buffer : NULL;
  d->first = d->data;
  d->size = size;

  return d;
}

Image::Data* Image::Data::alloc(int w, int h, int format)
{
  Data* d;
  sysint_t stride;
  uint64_t size;

  // Zero or negative coordinates are invalid
  if (w == 0 || h == 0) return 0;

  // Prevent multiply overflow (64 bit int type)
  size = (uint64_t)w * h;
  if (size > (uint64_t)Image::MaxSize) return 0;

  int depth = formatToDepth(format);

  // Calculate stride
  if ((stride = calcStride(w, depth)) == 0) return 0;

  // Try to alloc data
  d = alloc((sysuint_t)(h * stride));
  if (!d) return NULL;

  d->width = w;
  d->height = h;
  d->format = format;
  d->depth = depth;
  d->bytesPerPixel = depth >> 3;
  d->stride = stride;

  return d;
}

Image::Data* Image::Data::copy(const Data* other)
{
  Data* d;

  if (other->width && other->height)
  {
    d = alloc(other->width, other->height, other->format);
    if (!d) return NULL;

    uint8_t *destPixels = d->first;
    const uint8_t *srcPixels = other->first;

    sysint_t w = d->width;
    Raster::ConvertPlainFn copy;

    switch (d->depth)
    {
      case 32:
        copy = Raster::functionMap->convert.memcpy32;
        break;
      case 24:
        copy = Raster::functionMap->convert.memcpy24;
        break;
      case 8:
        copy = Raster::functionMap->convert.memcpy8;
        break;
      default:
        FOG_ASSERT_NOT_REACHED();
    }

    for (sysuint_t y = d->height; y; y--,
      destPixels += d->stride,
      srcPixels += other->stride)
    {
      copy(destPixels, srcPixels, w);
    }

    d->palette = other->palette;
  }
  else
  {
    d = Image::sharedNull->refAlways();
  }

  return d;
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_image_init(void)
{
  Fog::Image::sharedNull.init();
  Fog::Image::Data* d = Fog::Image::sharedNull.instancep();
  d->refCount.init(1);
  d->flags = Fog::Image::Data::IsSharable | Fog::Image::Data::IsNull;

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_image_shutdown(void)
{
  Fog::Image::sharedNull.instancep()->refCount.dec();
  Fog::Image::sharedNull.destroy();
}
