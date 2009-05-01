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
#include <Fog/Core/Memory.h>
#include <Fog/Core/Std.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Converter.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/ImageIO.h>

namespace Fog {

// ============================================================================
// [Fog::Image]
// ============================================================================

Static<Image::Data> Image::sharedNull;

Image::Image() : 
  _d(sharedNull.instancep()->REF_ALWAYS())
{
}

Image::Image(Data* d) : 
  _d(d)
{
}

Image::Image(const Image& other) : 
  _d(other._d->REF_INLINE())
{
}

Image::Image(uint32_t w, uint32_t h, const ImageFormat& format) :
  _d(sharedNull.instancep()->REF_ALWAYS())
{
  create(w, h, format);
}

Image::~Image()
{
  _d->DEREF_INLINE();
}

void Image::_detach()
{
  Data* d = _d;
  if (d->flags & Data::IsNull) return;

  if (d->stride == 0)
  {
    AtomicBase::ptr_setXchg(&_d, sharedNull.instancep()->REF_ALWAYS())->deref();
    return;
  }

  if (d->refCount.get() > 1 || d->flags.anyOf(Data::IsReadOnly))
  {
    AtomicBase::ptr_setXchg(&_d, Data::copy(d, AllocCantFail))->deref();
  }
}

bool Image::_tryDetach()
{
  Data* d = _d;

  if (d->flags & Data::IsNull) return true;

  if (d->stride == 0)
  {
    AtomicBase::ptr_setXchg(&_d, sharedNull.instancep()->REF_ALWAYS())->deref();
    return true;
  }

  if (d->refCount.get() > 1 || d->flags.anyOf(Data::IsReadOnly))
  {
    Data* newd = Data::copy(d, AllocCanFail);
    if (newd) AtomicBase::ptr_setXchg(&_d, newd)->deref();
    return newd != 0;
  }

  return true;
}

void Image::free()
{
  AtomicBase::ptr_setXchg(&_d, sharedNull.instancep()->REF_ALWAYS())->deref();
}

bool Image::create(
  uint32_t w, uint32_t h, const ImageFormat& format)
{
  Data* d = _d;

  // Zero dimensions are like Image::free()
  if (w == 0 || h == 0 || format.isInvalid())
  {
    free();
    return false;
  }

  // Always return a new detached image.
  if (d->width == w && d->height == h && d->format.id() == format.id() && d->refCount.get() == 1 && !(d->flags & Data::IsReadOnly))
  {
    return true;
  }

  // Create new data
  Data* newd = Data::create(w, h, format, AllocCanFail);

  if (newd)
  {
    AtomicBase::ptr_setXchg(&_d, newd)->deref();
    return true;
  }
  else
  {
    free();
    return false;
  }
}

bool Image::adopt(
  uint32_t w, uint32_t h, sysint_t stride, 
  const ImageFormat& format, 
  const uint8_t* mem, uint32_t adoptFlags)
{
  Data* d = _d;

  if (w == 0 || h == 0 || format.isInvalid() || mem == NULL)
  {
    free();
    return false;
  }

  if (d->refCount.get() > 1 || ((d->flags & Data::IsDynamic) && d->size != 0))
  {
    Data* newd = Data::create(0, AllocCanFail);

    if (newd)
    {
      AtomicBase::ptr_setXchg(&_d, newd)->deref();
      d = _d;
    }
    else
    {
      // Allocation error
      return false;
    }
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
    d->flags.set(Data::IsReadOnly);
  else
    d->flags.unset(Data::IsReadOnly);

  return true;
}

bool Image::set(const Image& other)
{
  // Reference to itself
  if (_d == other._d) return true;

  if (isStrong() && !other.isEmpty())
  {
    return setDeep(other);
  }
  else
  {
    Data* newd = other._d->REF_INLINE();
    if (newd)
    {
      AtomicBase::ptr_setXchg(&_d, newd)->DEREF_INLINE();
      return true;
    }
    else
      return false;
  }
}

bool Image::setDeep(const Image& other)
{
  // Reference to itself
  if (_d == other._d) return true;

  Data* newd = Data::copy(other._d, AllocCanFail);
  if (newd)
  {
    AtomicBase::ptr_setXchg(&_d, newd)->DEREF_INLINE();
    return true;
  }
  else
    return false;
}

bool Image::convert(const ImageFormat& format)
{
  ImageFormat sourceFormat(_d->format);
  ImageFormat targetFormat(format);

  if (sourceFormat == targetFormat) return true;

  Converter converter;
  if (!converter.setup(targetFormat.id(), sourceFormat.id())) return false;

  Data* d = _d;

  uint32_t w = d->width;
  uint32_t h = d->height;

  if (sourceFormat.depth() == targetFormat.depth() && isDetached() && !isReadOnly())
  {
    converter.convertRect(
      d->first, 0, d->stride,
      d->first, 0, d->stride,
      w, h,
      Point(0, 0));
    return true;
  }
  else
  {
    Data* newd = Data::create(w, h, targetFormat, AllocCanFail);

    if (newd)
    {
      converter.convertRect(
        newd->first, 0, newd->stride,
        d->first, 0, d->stride,
        w, h,
        Point(0, 0));
      AtomicBase::ptr_setXchg(&_d, newd)->deref();
      return true;
    }
    else
      return false;
  }
}

bool Image::setPalette(const Palette& palette)
{
  if (tryDetach()) 
  {
    _d->palette = palette;
    return true;
  }
  else
    return false;
}

bool Image::setPalette(sysuint_t index, sysuint_t count, const Rgba* rgba)
{
  if (tryDetach()) 
  {
    _d->palette.set(index, count, rgba);
    return true;
  }
  else
    return false;
}

// [Swap RGB and RGBA]

bool Image::swapRgb()
{
  if (isEmpty()) return true;

  // These formats have only alpha values
  if (format().id() == ImageFormat::A8)
  {
    return true;
  }

  if (!tryDetach()) return false;

  Data* d = _d;

  uint32_t w = d->width;
  uint32_t h = d->height;

  sysint_t stride = d->stride;
  uint8_t* dest = d->first;

  sysuint_t x, y;

  switch (format().id())
  {
    case ImageFormat::I8:
      dest = (uint8_t*)d->palette.mData();
      y = 1;
      // Go through
    case ImageFormat::ARGB32:
    case ImageFormat::PRGB32:
    case ImageFormat::XRGB32:
#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
      dest += 0;
#else
      dest += 1;
#endif // FOG_BYTE_ORDER
      for (y = h; y; y--, dest += stride)
      {
        uint8_t* destCur = dest;
        for (x = w; x; x--, destCur += 4)
        {
          uint8_t t = destCur[0];
          destCur[0] = destCur[2];
          destCur[2] = t;
        }
      }
      break;

    case ImageFormat::RGB24:
    case ImageFormat::BGR24:
      for (y = h; y; y--, dest += stride)
      {
        uint8_t* destCur = dest;
        for (x = w; x; x--, destCur += 3)
        {
          uint8_t t = destCur[0];
          destCur[0] = destCur[2];
          destCur[2] = t;
        }
      }
      break;
  }

  return this;
}

bool Image::swapRgba()
{
  if (isEmpty()) return true;

  // these formats have only alpha values
  if (format().id() == ImageFormat::A8)
  {
    return true;
  }

  if (format().id() == ImageFormat::RGB24 ||
      format().id() == ImageFormat::BGR24)
  {
    return swapRgb();
  }

  if (!tryDetach()) return false;

  Data* d = _d;

  uint32_t w = (uint)d->width;
  uint32_t h = (uint)d->height;

  sysint_t stride = d->stride;
  uint8_t* dest = d->first;

  sysuint_t x, y;

  switch (format().id())
  {
    case ImageFormat::I8:
      dest = (uint8_t*)d->palette.mData();
      y = 1;
      // Go through
    case ImageFormat::ARGB32:
    case ImageFormat::PRGB32:
    case ImageFormat::XRGB32:
    {
      for (y = h; y; y--, dest += stride)
      {
        uint8_t* destCur = dest;
        for (x = w; x; x--, destCur += 4)
        {
          ((uint32_t *)destCur)[0] = Memory::bswap32(((uint32_t *)destCur)[0]);
        }
      }
      break;
    }
  }

  return true;
}

// [Invert]

bool Image::invert(Image& dest, const Image& src, uint32_t invertMode)
{
  ImageFormat format = src.format();

  // First check for some invertion flags in source image format
  invertMode &= 15;

  if (src.isEmpty() || (invertMode == 0) || src.isEmpty() ||
     (!(invertMode & Image::InvertAlpha) && 
       (format.id() == ImageFormat::A8)) ||
     (!(invertMode & (Image::InvertRgb)) && 
       (format.id() == ImageFormat::XRGB32 || 
        format.id() == ImageFormat::RGB24)) )
  {
    dest = src;
    return true;
  }

  // Destination and source can be equal
  if (dest._d != src._d)
  {
    if (!dest.create(src.width(), src.height(), format)) return false;
  }
  else
  {
    if (!dest.tryDetach()) return false;
  }

  // prepare data
  Data* dest_d = dest._d;
  Data* src_d = src._d;

  sysint_t destStride = dest_d->stride;
  sysint_t srcStride = src_d->stride;

  uint8_t* destPixels = dest_d->first;
  uint8_t* srcPixels = src_d->first;

  uint8_t* destCur;
  uint8_t* srcCur;

  sysuint_t w = dest_d->width;
  sysuint_t h = dest_d->height;
  sysuint_t x;
  sysuint_t y;

  // some pixel formats needs special invertion process
  switch (format.id())
  {
    case ImageFormat::XRGB32:
      invertMode &= ~Image::InvertAlpha;
      // go through
    case ImageFormat::ARGB32:
    case ImageFormat::PRGB32:
    {
      uint32_t mask = 0;

      if (invertMode & Image::InvertRed  ) mask |= Rgba::RedMask;
      if (invertMode & Image::InvertGreen) mask |= Rgba::GreenMask;
      if (invertMode & Image::InvertBlue ) mask |= Rgba::BlueMask;
      if (invertMode & Image::InvertAlpha) mask |= Rgba::AlphaMask;

      for (y = h; y; y--, destPixels += destStride, srcPixels += srcStride)
      {
        destCur = destPixels;
        srcCur = srcPixels;

        for (x = w; x; x--, destCur += 4, srcCur += 4)
        {
          ((uint32_t*)destCur)[0] = ((uint32_t *)srcCur)[0] ^ mask;
        }
      }
      break;
    }

    case ImageFormat::RGB24:
    {
      uint8_t mask0 = 0;
      uint8_t mask1 = 0;
      uint8_t mask2 = 0;

#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
      if (invertMode & Image::InvertRed  ) mask2 |= 0xFF;
      if (invertMode & Image::InvertGreen) mask1 |= 0xFF;
      if (invertMode & Image::InvertBlue ) mask0 |= 0xFF;
#else // FOG_BYTE_ORDER == FOG_BIG_ENDIAN
      if (invertMode & Image::InvertRed  ) mask0 |= 0xFF;
      if (invertMode & Image::InvertGreen) mask1 |= 0xFF;
      if (invertMode & Image::InvertBlue ) mask2 |= 0xFF;
#endif // FOG_BYTE_ORDER

      for (y = h; y; y--, destPixels += destStride, srcPixels += srcStride)
      {
        destCur = destPixels;
        srcCur = srcPixels;

        for (x = w; x; x--, destCur += 3, srcCur += 3)
        {
          destCur[0] = srcCur[0] ^ mask0;
          destCur[1] = srcCur[1] ^ mask1;
          destCur[2] = srcCur[2] ^ mask2;
        }
      }
      break;
    }

    case ImageFormat::A8:
    {
      // should be guaranted that alpha invert is set
      FOG_ASSERT((invertMode & Image::InvertAlpha) != 0);

      for (y = h; y; y--, destPixels += destStride, srcPixels += srcStride)
      {
        destCur = destPixels;
        srcCur = srcPixels;

        x = w;

        // this will make main loop a much faster
        while (((sysuint_t)destCur & 7) != 0 && x)
        {
          *destCur++ = *srcCur++ ^ 0xFF;
          x--;
        }

        while (x >= 8)
        {
#if FOG_ARCH_BITS == 32
          ((uint32_t *)destCur)[0] = ((uint32_t *)srcCur)[0] ^ 0xFFFFFFFF;
          ((uint32_t *)destCur)[1] = ((uint32_t *)srcCur)[1] ^ 0xFFFFFFFF;
#else
          ((uint64_t *)destCur)[0] = ((uint64_t *)srcCur)[0] ^ FOG_UINT64_C(0xFFFFFFFFFFFFFFFF);
#endif // FOG_ARCH_BITS

          destCur += 8;
          srcCur += 8;
          x -= 8;
        }

        switch (x)
        {
          case 7: *destCur++ = *srcCur++ ^ 0xFF;
          case 6: *destCur++ = *srcCur++ ^ 0xFF;
          case 5: *destCur++ = *srcCur++ ^ 0xFF;
          case 4: *destCur++ = *srcCur++ ^ 0xFF;
          case 3: *destCur++ = *srcCur++ ^ 0xFF;
          case 2: *destCur++ = *srcCur++ ^ 0xFF;
          case 1: *destCur++ = *srcCur++ ^ 0xFF;
        }
      }
      break;
    }
  }

  return true;
}

// [Mirror]

typedef void (FOG_FASTCALL *MirrorFunc)(uint8_t*, uint8_t*, sysuint_t);

static void FOG_FASTCALL mirror_copy_src_is_not_dest_32(uint8_t* dest, uint8_t* src, sysuint_t w)
{
  memcpy(dest, src, w << 2);
}

static void FOG_FASTCALL mirror_copy_src_is_not_dest_24(uint8_t* dest, uint8_t* src, sysuint_t w)
{
  memcpy(dest, src, w + w + w);
}

static void FOG_FASTCALL mirror_copy_src_is_not_dest_8(uint8_t* dest, uint8_t* src, sysuint_t w)
{
  memcpy(dest, src, w);
}

static void FOG_FASTCALL mirror_copy_src_is_not_dest_1(uint8_t* dest, uint8_t* src, sysuint_t w)
{
  memcpy(dest, src, (w + 7) >> 3);
}

static void FOG_FASTCALL mirror_flip_src_is_not_dest_32(uint8_t* dest, uint8_t* src, sysuint_t w)
{
  src += ((w - 1) << 2);

  sysuint_t x;
  for (x = w; x; x--, dest += 4, src -= 4)
  {
    Memory::copy4B(dest, src);
  }
}

static void FOG_FASTCALL mirror_flip_src_is_not_dest_24(uint8_t* dest, uint8_t* src, sysuint_t w)
{
  src += w + w + w - 3;

  sysuint_t x;
  for (x = w; x; x--, dest += 3, src -= 3)
  {
    Memory::copy3B(dest, src);
  }
}

static void FOG_FASTCALL mirror_flip_src_is_not_dest_8(uint8_t* dest, uint8_t* src, sysuint_t w)
{
  src += w - 1;

  sysuint_t x;
  for (x = w; x; x--, dest += 3, src -= 3)
  {
    dest[0] = src[0];
  }
}

static void FOG_FASTCALL mirror_flip_src_is_not_dest_1(uint8_t* dest, uint8_t* src, sysuint_t w)
{
  // TODO
}

static void FOG_FASTCALL mirror_copy_src_is_dest_32(uint8_t* dest, uint8_t* src, sysuint_t w)
{
  Memory::xchg(dest, src, w << 2);
}

static void FOG_FASTCALL mirror_copy_src_is_dest_24(uint8_t* dest, uint8_t* src, sysuint_t w)
{
  Memory::xchg(dest, src, w + w + w);
}

static void FOG_FASTCALL mirror_copy_src_is_dest_8(uint8_t* dest, uint8_t* src, sysuint_t w)
{
  Memory::xchg(dest, src, w);
}

static void FOG_FASTCALL mirror_copy_src_is_dest_1(uint8_t* dest, uint8_t* src, sysuint_t w)
{
  Memory::xchg(dest, src, (w + 7) >> 3);
}

static void FOG_FASTCALL mirror_flip_src_is_dest_32(uint8_t* dest, uint8_t* src, sysuint_t w)
{
  sysuint_t x = w;
  if (src == dest) x >>= 1;

  src += ((w - 1) << 2);
  for (; x; x--, dest += 4, src -= 4)
  {
    Memory::xchg4B(dest, src);
  }
}

static void FOG_FASTCALL mirror_flip_src_is_dest_24(uint8_t* dest, uint8_t* src, sysuint_t w)
{
  sysuint_t x = w;
  if (src == dest) x >>= 1;

  src += w + w + w - 3;
  for (; x; x--, dest += 3, src -= 3)
  {
    Memory::xchg2B((uint16_t*)dest, (uint16_t*)src);
    Memory::xchg1B(dest + 2, src + 2);
  }
}

static void FOG_FASTCALL mirror_flip_src_is_dest_8(uint8_t* dest, uint8_t* src, sysuint_t w)
{
  sysuint_t x = w;
  if (src == dest) x >>= 1;

  src += w - 1;
  for (; x; x--, dest += 3, src -= 3)
  {
    Memory::xchg1B(dest, src);
  }
}

static void FOG_FASTCALL mirror_flip_src_is_dest_1(uint8_t* dest, uint8_t* src, sysuint_t w)
{
  // TODO
}

static const MirrorFunc mirror_funcs_copy_src_is_not_dest[] =
{
  mirror_copy_src_is_not_dest_32,
  mirror_copy_src_is_not_dest_32,
  mirror_copy_src_is_not_dest_32,
  mirror_copy_src_is_not_dest_24,
  mirror_copy_src_is_not_dest_24,
  mirror_copy_src_is_not_dest_8,
  mirror_copy_src_is_not_dest_8,
  mirror_copy_src_is_not_dest_1
};

static const MirrorFunc mirror_funcs_flip_src_is_not_dest[] =
{
  mirror_flip_src_is_not_dest_32,
  mirror_flip_src_is_not_dest_32,
  mirror_flip_src_is_not_dest_32,
  mirror_flip_src_is_not_dest_24,
  mirror_flip_src_is_not_dest_24,
  mirror_flip_src_is_not_dest_8,
  mirror_flip_src_is_not_dest_8,
  mirror_flip_src_is_not_dest_1
};

static const MirrorFunc mirror_funcs_copy_src_is_dest[] =
{
  mirror_copy_src_is_dest_32,
  mirror_copy_src_is_dest_32,
  mirror_copy_src_is_dest_32,
  mirror_copy_src_is_dest_24,
  mirror_copy_src_is_dest_24,
  mirror_copy_src_is_dest_8,
  mirror_copy_src_is_dest_8,
  mirror_copy_src_is_dest_1
};

static const MirrorFunc mirror_funcs_flip_src_is_dest[] =
{
  mirror_flip_src_is_dest_32,
  mirror_flip_src_is_dest_32,
  mirror_flip_src_is_dest_32,
  mirror_flip_src_is_dest_24,
  mirror_flip_src_is_dest_24,
  mirror_flip_src_is_dest_8,
  mirror_flip_src_is_dest_8,
  mirror_flip_src_is_dest_1
};

bool Image::mirror(Image& dest, const Image& src, uint32_t mirrorMode)
{
  mirrorMode &= 0x3;

  if (mirrorMode == 0 || src.isEmpty())
  {
    dest = src;
    return true;
  }

  ImageFormat format = src.format();

  if (dest._d != src._d)
  {
    if (!dest.create(src.width(), src.height(), format)) return false;
  }
  else
  {
    if (!dest.tryDetach()) return false;
  }

  Data* dest_d = dest._d;
  Data* src_d = src._d;

  sysint_t destStride = dest_d->stride;
  sysint_t srcStride = src_d->stride;

  uint8_t* destPixels = dest_d->first;
  uint8_t* srcPixels = src_d->first;

  uint32_t w = dest_d->width;
  uint32_t h = dest_d->height;

  MirrorFunc func;

  switch (mirrorMode)
  {
    case MirrorVertical:
    {
      srcPixels += srcStride * ((sysint_t)h - 1);
      srcStride = -srcStride;

      if (dest_d != src_d)
      {
        func = mirror_funcs_copy_src_is_not_dest[format.id()];
      }
      else
      {
        func = mirror_funcs_copy_src_is_dest[format.id()];
        h >>= 1;
      }
      break;
    }
    case MirrorHorizontal:
      if (dest_d != src_d)
      {
        func = mirror_funcs_flip_src_is_not_dest[format.id()];
      }
      else
      {
        func = mirror_funcs_flip_src_is_dest[format.id()];
      }
      break;
    case MirrorBoth:
      srcPixels += srcStride * ((sysint_t)h - 1);
      srcStride = -srcStride;

      if (dest_d != src_d)
      {
        func = mirror_funcs_flip_src_is_not_dest[format.id()];
      }
      else
      {
        func = mirror_funcs_flip_src_is_dest[format.id()];
        h >>= 1;
      }
      break;
  }

  sysuint_t y;
  for (y = h; y; y--, destPixels += destStride, srcPixels += srcStride)
  {
    func(destPixels, srcPixels, w);
  }

  return true;
}

// [Rotate]

bool Image::rotate(Image& dest, const Image& src, uint32_t rotateMode)
{
  if (rotateMode == Rotate0 || src.isEmpty())
  {
    dest = src;
    return true;
  }

  // rotate by 180 degrees has same effect as MirrorBoth, so we will simply 
  // check for this.
  if (rotateMode == Rotate180)
  {
    return mirror(dest, src, Image::MirrorBoth);
  }

  // small check. This case shouldn't happen and it's probabbly for assert()
  if (rotateMode != Rotate90 && rotateMode != Rotate270)
  {
    FOG_ASSERT_NOT_REACHED();
    return false;
  }
  
  // now we have only two possibilities:
  // - rotate by 90
  // - rotate by 270

  // destination == source ?
  if (&dest == &src)
  {
    // we simply call this function again
    Image t(src);
    return rotate(dest, t, rotateMode);
  }

  if (!dest.create(src.height(), src.width(), src.format())) return false;

  Data* dest_d = dest._d;
  Data* src_d = src._d;

  sysint_t destStride = dest_d->stride;
  sysint_t srcStride = src_d->stride;

  uint8_t* destPixels = dest_d->first;
  uint8_t* srcPixels = src_d->first;

  uint8_t* destCur;
  uint8_t* srcCur;

  sysuint_t sz1 = (sysuint_t)src_d->width;
  sysuint_t sz2 = (sysuint_t)src_d->height;

  sysuint_t sz1m1 = sz1-1;
  sysuint_t sz2m1 = sz2-1;

  sysuint_t i;
  sysuint_t j;

  sysuint_t srcInc1;
  sysuint_t srcInc2;

  switch (src.format().depth())
  {
    case 32:
      if (rotateMode == Rotate90)
      {
        srcPixels += ((sysint_t)sz2m1 * srcStride);
        srcInc1 = 4;
        srcInc2 = -srcStride;
      }
      else
      {
        srcPixels += sz1m1 << 2;
        srcInc1 = -4;
        srcInc2 = srcStride;
      }

      for (i = sz1; i; i--, destPixels += destStride, srcPixels += srcInc1)
      {
        destCur = destPixels;
        srcCur = srcPixels;

        for (j = sz2; j; j--, destCur += 4, srcCur += srcInc2)
        {
          Memory::copy4B(destCur, srcCur);
        }
      }
      break;
    case 24:
      if (rotateMode == Rotate90)
      {
        srcPixels += ((sysint_t)sz2m1 * srcStride);
        srcInc1 = 3;
        srcInc2 = -srcStride;
      }
      else
      {
        srcPixels += sz1m1 * 3;
        srcInc1 = -3;
        srcInc2 = srcStride;
      }

      for (i = sz1; i; i--, destPixels += destStride, srcPixels += srcInc1)
      {
        destCur = destPixels;
        srcCur = srcPixels;

        for (j = sz2; j; j--, destCur += 3, srcCur += srcInc2)
        {
          Memory::copy3B(destCur, srcCur);
        }
      }
      break;
    case 8:
      if (rotateMode == Image::Rotate90)
      {
        srcPixels += ((sysint_t)sz2m1 * srcStride);
        srcInc1 = 1;
        srcInc2 = -srcStride;
      }
      else
      {
        srcPixels += sz1m1;
        srcInc1 = -1;
        srcInc2 = srcStride;
      }

      for (i = sz1; i; i--, destPixels += destStride, srcPixels += srcInc1)
      {
        destCur = destPixels;
        srcCur = srcPixels;

        for (j = sz2; j; j--, destCur += 1, srcCur += srcInc2)
        {
          destCur[0] = srcCur[0];
        }
      }
      break;
    case 1:
      // TODO
      break;
  }

  return true;
}

// [Alpha Channel]

bool Image::alphaChannel(Image& dest, const Image& src)
{
  ImageFormat format = src.format();

  if (src.isEmpty() || format.id() == ImageFormat::A8)
  {
    dest = src;
    return true;
  }

  // destination == source ?
  if (&dest == &src)
  {
    // we simply call this function again
    return alphaChannel(dest, Image(src));
  }

  if (!dest.create(src.width(), src.height(), ImageFormat::A8)) return false;

  Data* src_d = src._d;
  Data* dest_d = dest._d;

  sysint_t destStride = dest_d->stride;
  sysint_t srcStride = src_d->stride;

  uint8_t* destPixels = dest_d->first;
  uint8_t* srcPixels = src_d->first;

  uint32_t w = (sysuint_t)src_d->width;
  uint32_t h = (sysuint_t)src_d->height;

  Converter converter;

  converter.setup(ImageFormat::A8, format.id());
  converter.convertRect(
    destPixels, 0, destStride, 
    srcPixels, 0, srcStride, 
    w, h, 
    Point(0, 0));

  return true;
}

// [Windows Specific]

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
  bmi.bmiHeader.biBitCount    = d->format.depth();
  bmi.bmiHeader.biCompression = BI_RGB;

  // TODO: 8 bit greyscale and 1 bit mono images
  // TODO: ARGB32 and PRGB32 images?
  hDibSection = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&raw, NULL, 0);

  sysuint_t y;
  uint8_t* dest = raw;
  uint8_t* src = d->first;
  
  DIBSECTION info;
  GetObject(hDibSection, sizeof(DIBSECTION), &info);
  
  sysint_t destStride = info.dsBm.bmWidthBytes;
  sysint_t srcStride = d->stride;

  sysuint_t byteWidth = (d->width * d->format.depth() + 7) >> 3;

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
  
  ImageFormat format;

  switch (bm.bmBitsPixel)
  {
    case 32:
      format.set(ImageFormat::XRGB32);
      break;
    case 24:
    default:
      format.set(ImageFormat::RGB24);
      break;
  }

  if (!create(bm.bmWidth, bm.bmHeight, format)) return false;

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
    dibi.bmiHeader.biBitCount = format.depth();
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

// [ImageIO]

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
    uint32_t result;

    if ((provider = ImageIO::getProviderByExtension(extension)))
    {
      if ((decoder = provider->createDecoder()))
      {
        int64_t pos = stream.tell();

        decoder->attachStream(stream);
        result = decoder->readImage(*this);
        delete decoder;

        if (result == Error::Ok)
        {
          // Success
          return Error::Ok;
        }
        else
        {
          // Seek to begin if fail
          if (stream.seek(pos, Stream::SeekSet) == -1)
          {
            return Error::ImageIO_SeekFailure;
          }
        }
      }
    }
  }

  // Fallback to readStream(Stream) without extension support
  return readStream(stream);
}

err_t Image::readMemory(const uint8_t* data, sysuint_t size)
{
  Stream stream;
  stream.openMemory((void*)data, size, Stream::OpenRead);
  return readStream(stream);
}

err_t Image::readMemory(const uint8_t* data, sysuint_t size, const String32& extension)
{
  Stream stream;
  stream.openMemory((void*)data, size, Stream::OpenRead);
  return readStream(stream, extension);
}

#if 0
FOG_CAPI_DECLARE void Fog_Image_writeFile(
  Image* image,
  const String32* fileName,
  Fog_ImageProcessHandler* handler, void* handlerData, Value* result)
{
  Stream stream;

  *result = stream.openFile(*fileName, Stream::Open_Write | Stream::Open_Create | Stream::Open_CreatePath | Stream::Open_Truncate);
  if (result->ok())
  {
    TemporaryString<16> extension;
    Path::extensionTo(*fileName, extension);
    extension.lower();

    Fog_Image_writeStream(image, &stream, &extension, handler, handlerData, result);
  }
}

FOG_CAPI_DECLARE void Fog_Image_writeStream(
  Image* image,
  Stream* stream, const String32* extension,
  Fog_ImageProcessHandler* handler, void* handlerData, Value* result)
{
  Fog_ImageIOEncoder encoder;
  uint error;
  
  if (extension == NULL || extension->length() == 0 || 
    !encoder.getByExtension(*stream, *extension)) 
  {
    error = Fog_ImageError_ExtensionNotMatch;
    goto end;
  }

  error = encoder.funcs->encodeImage(&encoder, image, handler, handlerData);
end:
  if (error == Fog_ImageError_Success)
  {
    result->null();
  }
  else
  {
    result->setError(ErrorDomain_Image, error);
  }
}
#endif

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
  // TODO:
  return Error::Ok;
}

sysint_t Image::calcStride(uint32_t width, uint32_t depth)
{
  sysint_t result = 0;

  switch (depth)
  {
    case   1: result = ((sysint_t)width + 7) >> 3; break;
    case   4: result = ((sysint_t)width + 1) >> 1; break;
    // X server can be configured for these!
    case   5:
    case   6:
    case   7:
    case   8: result = (sysint_t)width; break;

    case  15:
    case  16: result = (sysint_t)width *  2; break;
    case  24: result = (sysint_t)width *  3; break;
    case  32: result = (sysint_t)width *  4; break;

    case  48: result = (sysint_t)width *  6; break;
    case  64: result = (sysint_t)width *  8; break;
    case  96: result = (sysint_t)width * 12; break; 
    case 128: result = (sysint_t)width * 16; break;

    // all others are invalid
    default:
      fog_fail("Image::calcStride() - invalid depth %u", depth);
  }

  // Align to 32 bits boudary
  result += 3;
  result &= ~3;

  // Overflow
  if (result < (sysint_t)width) return 0;

  // Success
  return (sysint_t)result;
}

// [Image::Data]

Image::Data::Data() {}
Image::Data::~Data() {}

Image::Data* Image::Data::ref()
{
  return REF_INLINE();
}

void Image::Data::deref()
{
  DEREF_INLINE();
}

Image::Data* Image::Data::create(sysuint_t size, uint allocPolicy)
{
  sysuint_t dsize = sizeof(Data) - sizeof(uint32_t) + size;
  Data* d = (Data*)Memory::alloc(dsize);

  if (d)
  {
    new (d) Data();

    d->refCount.init(1);
    d->flags.init(Data::IsDynamic | Data::IsSharable);
    d->width = 0;
    d->height = 0;
    d->stride = 0;
    d->data = (size != 0) ? d->buffer : NULL;
    d->first = d->data;
    d->size = size;
  }
  else if (allocPolicy == AllocCantFail)
  {
    fog_out_of_memory_fatal_format(
      "Image::Data", "create", 
      "Couldn't allocate %lu bytes of memory for image data", (ulong)dsize);
  }

  return d;
}

Image::Data* Image::Data::create(uint32_t w, uint32_t h, const ImageFormat& format, uint allocPolicy)
{
  Data* d;
  sysint_t stride;
  uint64_t size;

  // Zero or negative coordinates are invalid
  if (w == 0 || h == 0) return 0;

  // Prevent multiply overflow (64 bit int type)
  size = (uint64_t)w * h;
  if (size > (uint64_t)Image::MaxSize) return 0;

  // Calculate stride
  if ((stride = calcStride(w, format.depth())) == 0) return 0;

  // Try to alloc data
  d = create((sysuint_t)(h * stride), allocPolicy);

  if (d)
  {
    d->width = w;
    d->height = h;
    d->stride = stride;
    d->format = format;
  }

  return d;
}

Image::Data* Image::Data::copy(const Data* other, uint allocPolicy)
{
  Data* d;

  if (other->width && other->height)
  {
    d = create(other->width, other->height, other->format, allocPolicy);

    if (d)
    {
      uint8_t *destPixels = d->first;
      const uint8_t *srcPixels = other->first;

      // pixel format returns 1, 8, 16, 24 or 32
      sysuint_t bytesPerLine = (sysuint_t)( ((uint64_t)d->width * (uint64_t)d->format.depth() + FOG_UINT64_C(7)) / FOG_UINT64_C(8) );

      for (sysuint_t y = d->height; y; y--, 
        destPixels += d->stride,
        srcPixels += other->stride)
      {
        Memory::copy(destPixels, srcPixels, bytesPerLine);
      }
    }
  }
  else
  {
    d = Image::sharedNull.instancep()->REF_ALWAYS();
  }

  return d;
}

} // Fog namespace

#if 0
FOG_CAPI_DECLARE void Fog_Image_writeStream(
  Image* image,
  Stream* stream, const String32* extension,
  Fog_ImageProcessHandler* handler, void* handlerData, Value* result)
{
  Fog_ImageIOEncoder encoder;
  uint error;
  
  if (extension == NULL || extension->length() == 0 || 
    !encoder.getByExtension(*stream, *extension)) 
  {
    error = Fog_ImageError_ExtensionNotMatch;
    goto end;
  }

  error = encoder.funcs->encodeImage(&encoder, image, handler, handlerData);
end:
  if (error == Fog_ImageError_Success)
  {
    result->null();
  }
  else
  {
    result->setError(ErrorDomain_Image, error);
  }
}
#endif

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_image_init(void)
{
  Fog::Image::sharedNull.init();
  Fog::Image::Data* d = Fog::Image::sharedNull.instancep();
  d->refCount.init(1);
  d->flags.init(
      Fog::Image::Data::IsSharable |
      Fog::Image::Data::IsNull);

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_image_shutdown(void)
{
  Fog::Image::sharedNull.instancep()->refCount.dec();
  Fog::Image::sharedNull.destroy();
}
