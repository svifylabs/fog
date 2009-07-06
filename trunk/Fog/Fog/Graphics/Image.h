// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_IMAGE_H
#define _FOG_GRAPHICS_IMAGE_H

// [Dependencies]
#include <Fog/Core/Assert.h>
#include <Fog/Core/Atomic.h>
#include <Fog/Core/Delegate.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/Stream.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/Graphics/Palette.h>
#include <Fog/Graphics/Rgba.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct ColorLut;
struct ColorMatrix;
struct ImageFilter;

// ============================================================================
// [Fog::Image]
// ============================================================================

struct FOG_API Image
{
  // don't change order of these values, it's very dependent to
  // values that will be initialized in Image.cpp and dependent
  // to Converter.cpp tables.

  //! @brief Pixel format.
  //!
  //! @note All pixel formats are CPU endian dependent. So @c ARGB32 pixels
  //! will be stored differenly in memory on machines with different endianness.
  //!
  //! @c FormatARGB32, @c FormatPRGB32
  //! - Little endian: BBGGRRAA
  //! - Big endian   : AARRGGBB
  //! @c FormatRGB32 formats:
  //! - Little endian: BBGGRRXX
  //! - Big endian   : XXRRGGBB
  //! @c FormatRGB24 format:
  //! - Little endian: BBGGRR
  //! - Big endian   : RRGGBB
  //! @c FormatA8 format:
  //! - no difference: AA (8 bit alpha value)
  //! @c FormatI8 format:
  //! - no difference: II (8 bit index value)
  //!
  //! @note Do not change order of this enum.
  enum Format
  {
    FormatNull = 0,
    //! @brief 32 bit RGBA (equivalent for @c Rgba).
    FormatARGB32 = 1,
    //! @brief 32 bit RGBA premultiplied.
    FormatPRGB32 = 2,
    //! @brief 32 bit RGB (equivalent for @c Rgba without alpha channel - full opaque).
    FormatRGB32 = 3,
    //! @brief 24 bit RGB.
    FormatRGB24 = 4,
    //! @brief 8 bit alpha channel.
    FormatA8 = 5,
    //! @brief 8 bit indexed pixel format.
    FormatI8 = 6,
    //! @brief Count of image formats.
    FormatCount = 7
  };

  // [Data]

  struct FOG_API Data
  {
    enum
    {
      IsNull = (1U << 0),
      IsDynamic = (1U << 1),
      IsSharable = (1U << 2),
      IsStrong = (1U << 3),
      IsReadOnly = (1U << 4)
    };

    Data();
    ~Data();

    // [Ref / Deref]

    Data* ref() const;
    void deref();

    FOG_INLINE Data* refAlways() const { refCount.inc(); return const_cast<Data*>(this); }

    static Data* alloc(sysuint_t size);
    static Data* alloc(int w, int h, int format);
    static void free(Data* d);

    static Data* copy(const Data* other);

    // [Members]

    //! @brief Reference count.
    mutable Atomic<sysuint_t> refCount;
    //! @brief Image flags.
    uint32_t flags;
    //! @brief Image width.
    int width;
    //! @brief Image height.
    int height;
    //! @brief Image pixel format.
    int format;
    //! @brief Image depth.
    int depth;
    //! @brief Image bytes per pixel
    int bytesPerPixel;
    //! @brief Image stride.
    sysint_t stride;
    //! @brief Image palette (only for 8 bit indexed images).
    Palette palette;
    //! @brief Data pointer that points to raw memory received from image device.
    uint8_t* data;
    //! @brief Base pointer to scanline, in most cases it's equal to @c _data.
    uint8_t* first;
    //! @brief Size of @c buffer.
    sysuint_t size;
    //! @brief Buffer for static allocations.
    uint8_t buffer[4];
  };

  static Static<Data> sharedNull;

  // [Constants]

  enum
  {
    //! @brief Maximum image size (in pixels)
    MaxSize = 16384*16384
  };

  // [Construction / Destruction]

  Image();
  explicit Image(Data* d);
  Image(const Image& other);
  Image(int w, int h, int format);
  ~Image();

  // [Implicit Sharing]

  //! @copydoc Doxygen::Implicit::refCount().
  FOG_INLINE sysuint_t refCount() const { return _d->refCount.get(); }
  //! @copydoc Doxygen::Implicit::isDetached().
  FOG_INLINE bool isDetached() const { return refCount() == 1 || isReadOnly(); }
  //! @copydoc Doxygen::Implicit::detach().
  FOG_INLINE err_t detach() { return !isDetached() ? _detach() : (err_t)Error::Ok; }
  //! @copydoc Doxygen::Implicit::_detach().
  err_t _detach();
  //! @copydoc Doxygen::Implicit::free().
  void free();

  // [Flags]

  //! @copydoc Doxygen::Implicit::flags().
  FOG_INLINE uint32_t flags() const { return _d->flags; }
  //! @copydoc Doxygen::Implicit::isDynamic().
  FOG_INLINE bool isDynamic() const { return _d->flags & Data::IsDynamic; }
  //! @copydoc Doxygen::Implicit::isSharable().
  FOG_INLINE bool isSharable() const { return _d->flags & Data::IsSharable; }
  //! @copydoc Doxygen::Implicit::isNull().
  FOG_INLINE bool isNull() const { return _d->flags & Data::IsNull; }
  //! @copydoc Doxygen::Implicit::isStrong().
  FOG_INLINE bool isStrong() const { return _d->flags & Data::IsStrong; }
  //! @brief Returns true if image is read only.
  FOG_INLINE bool isReadOnly() const { return _d->flags & Data::IsReadOnly; }

  // [Data]

  FOG_INLINE const uint8_t* cData() const
  {
    return _d->data;
  }

  FOG_INLINE uint8_t* mData()
  {
    return (detach() == Error::Ok) ? _d->data : NULL;
  }

  FOG_INLINE uint8_t* xData()
  {
    FOG_ASSERT(isDetached());
    return _d->data;
  }

  FOG_INLINE const uint8_t* cFirst() const
  {
    return _d->first;
  }

  FOG_INLINE uint8_t* mFirst()
  {
    return (detach() == Error::Ok) ? _d->first : NULL;
  }

  FOG_INLINE uint8_t* xFirst()
  {
    FOG_ASSERT(isDetached());
    return _d->first;
  }

  FOG_INLINE const uint8_t* cScanline(uint32_t i) const
  {
    FOG_ASSERT((sysuint_t)i < (sysuint_t)_d->height);
    return _d->first + (sysint_t)i * _d->stride;
  }

  FOG_INLINE uint8_t* mScanline(uint32_t i)
  {
    FOG_ASSERT((sysuint_t)i < (sysuint_t)_d->height);
    return (detach() == Error::Ok) ? _d->first + (sysint_t)i * _d->stride : NULL;
  }

  FOG_INLINE uint8_t* xScanline(uint32_t i)
  {
    FOG_ASSERT((sysuint_t)i < (sysuint_t)_d->height);
    FOG_ASSERT(isDetached());
    return _d->first + (sysint_t)i * _d->stride;
  }

  // [Dimensions]

  FOG_INLINE int width() const { return _d->width; }
  FOG_INLINE int height() const { return _d->height; }
  FOG_INLINE bool isEmpty() const { return _d->stride == 0; }
  FOG_INLINE sysint_t stride() const { return _d->stride; }

  // [Format]

  FOG_INLINE int format() const { return _d->format; }
  FOG_INLINE int depth() const { return _d->depth; }
  FOG_INLINE int bytesPerPixel() const { return _d->bytesPerPixel; }

  FOG_INLINE bool isIndexed() const
  { return _d->format == FormatI8; }

  FOG_INLINE bool isPremultiplied() const
  { return _d->format == FormatPRGB32; }

  // [Create / Adopt]

  err_t create(
    int w, int h, int format);
  
  //! @brief Image adopt flags.
  enum AdoptFlags
  {
    NoAdoptFlags = 0x0,
    AdoptReadOnly = 0x1,
    AdoptReversed = 0x2,
  };

  err_t adopt(
    int w, int h, int format,
    const uint8_t* mem, sysint_t stride,
    uint32_t adoptFlags = NoAdoptFlags);

  // [Set]

  err_t set(const Image& other);
  err_t setDeep(const Image& other);

  // [Convert]

  //! @brief Convert image to @a format.
  err_t convert(int format);

  //! @brief Convert image to 8 bit using optimized palette or dithering.
  err_t to8Bit();

  //! @brief Convert image to 8 bit using custom palette @a pal.
  err_t to8Bit(const Palette& pal);

  //! @brief Sets image format without doing any conversion. This can be only
  //! used for formats with equal bit depths.
  //!
  //! @note You can use this method for example to bypass premultiplication
  //! or demultiplication on 32 bit images.
  err_t forceFormat(int format);

  // [Palette]

  FOG_INLINE const Palette& palette() const { return _d->palette; }

  err_t setPalette(const Palette& palette);
  err_t setPalette(sysuint_t index, sysuint_t count, const Rgba* rgba);

  // [GetDib / SetDib]

  typedef void (Image::*GetDibFunc)(int x, int y, sysint_t w, void* dst) const;
  typedef void (Image::*SetDibFunc)(int x, int y, sysint_t w, const void* src);

  void getDibArgb32(int x, int y, sysint_t w, void* dst) const;
  void getDibArgb32_bs(int x, int y, sysint_t w, void* dst) const;

  void setDibArgb32(int x, int y, sysint_t w, const void* src);
  void setDibArgb32_bs(int x, int y, sysint_t w, const void* src);

  void getDibPrgb32(int x, int y, sysint_t w, void* dst) const;
  void getDibPrgb32_bs(int x, int y, sysint_t w, void* dst) const;

  void setDibPrgb32(int x, int y, sysint_t w, const void* src);
  void setDibPrgb32_bs(int x, int y, sysint_t w, const void* src);

  void getDibRgb24(int x, int y, sysint_t w, void* dst) const;
  void getDibRgb24_bs(int x, int y, sysint_t w, void* dst) const;

  void setDibRgb24(int x, int y, sysint_t w, const void* src);
  void setDibRgb24_bs(int x, int y, sysint_t w, const void* src);

  void getDibRgb16_555(int x, int y, sysint_t w, void* dst) const;
  void getDibRgb16_555_bs(int x, int y, sysint_t w, void* dst) const;

  void setDibRgb16_555(int x, int y, sysint_t w, const void* src);
  void setDibRgb16_555_bs(int x, int y, sysint_t w, const void* src);

  void getDibRgb16_565(int x, int y, sysint_t w, void* dst) const;
  void getDibRgb16_565_bs(int x, int y, sysint_t w, void* dst) const;

  void setDibRgb16_565(int x, int y, sysint_t w, const void* src);
  void setDibRgb16_565_bs(int x, int y, sysint_t w, const void* src);

#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
# define DIBFN_LE(fn) fn
# define DIBFN_BE(fn) fn##_bs
#else
# define DIBFN_LE(fn) fn##_bs
# define DIBFN_BE(fn) fn
#endif

  FOG_INLINE void getDibArgb32_le(int x, int y, sysint_t w, void* dst) const { DIBFN_LE(getDibArgb32)(x, y, w, dst); }
  FOG_INLINE void getDibArgb32_be(int x, int y, sysint_t w, void* dst) const { DIBFN_BE(getDibArgb32)(x, y, w, dst); }

  FOG_INLINE void setDibArgb32_le(int x, int y, sysint_t w, const void* src) { DIBFN_LE(setDibArgb32)(x, y, w, src); }
  FOG_INLINE void setDibArgb32_be(int x, int y, sysint_t w, const void* src) { DIBFN_BE(setDibArgb32)(x, y, w, src); }

  FOG_INLINE void getDibPrgb32_le(int x, int y, sysint_t w, void* dst) const { DIBFN_LE(getDibPrgb32)(x, y, w, dst); }
  FOG_INLINE void getDibPrgb32_be(int x, int y, sysint_t w, void* dst) const { DIBFN_BE(getDibPrgb32)(x, y, w, dst); }

  FOG_INLINE void setDibPrgb32_le(int x, int y, sysint_t w, const void* src) { DIBFN_LE(setDibPrgb32)(x, y, w, src); }
  FOG_INLINE void setDibPrgb32_be(int x, int y, sysint_t w, const void* src) { DIBFN_BE(setDibPrgb32)(x, y, w, src); }

  FOG_INLINE void getDibRgb24_le(int x, int y, sysint_t w, void* dst) const { DIBFN_LE(getDibRgb24)(x, y, w, dst); }
  FOG_INLINE void getDibRgb24_be(int x, int y, sysint_t w, void* dst) const { DIBFN_BE(getDibRgb24)(x, y, w, dst); }

  FOG_INLINE void setDibRgb24_le(int x, int y, sysint_t w, const void* src) { DIBFN_LE(setDibRgb24)(x, y, w, src); }
  FOG_INLINE void setDibRgb24_be(int x, int y, sysint_t w, const void* src) { DIBFN_BE(setDibRgb24)(x, y, w, src); }

  FOG_INLINE void getDibRgb16_555_le(int x, int y, sysint_t w, void* dst) const { DIBFN_LE(getDibRgb16_555)(x, y, w, dst); }
  FOG_INLINE void getDibRgb16_555_be(int x, int y, sysint_t w, void* dst) const { DIBFN_BE(getDibRgb16_555)(x, y, w, dst); }

  FOG_INLINE void setDibRgb16_555_le(int x, int y, sysint_t w, const void* src) { DIBFN_LE(setDibRgb16_555)(x, y, w, src); }
  FOG_INLINE void setDibRgb16_555_be(int x, int y, sysint_t w, const void* src) { DIBFN_BE(setDibRgb16_555)(x, y, w, src); }

  FOG_INLINE void getDibRgb16_565_le(int x, int y, sysint_t w, void* dst) const { DIBFN_LE(getDibRgb16_565)(x, y, w, dst); }
  FOG_INLINE void getDibRgb16_565_be(int x, int y, sysint_t w, void* dst) const { DIBFN_BE(getDibRgb16_565)(x, y, w, dst); }

  FOG_INLINE void setDibRgb16_565_le(int x, int y, sysint_t w, const void* src) { DIBFN_LE(setDibRgb16_565)(x, y, w, src); }
  FOG_INLINE void setDibRgb16_565_be(int x, int y, sysint_t w, const void* src) { DIBFN_BE(setDibRgb16_565)(x, y, w, src); }

#undef DIBFN_LE
#undef DIBFN_BE

  // [Swap RGB and RGBA]

  err_t swapRgb();
  err_t swapRgba();

  // [Premultiply / Demultiply]

  err_t premultiply();
  err_t demultiply();

  // [Invert]

  //! @brief Invert modes used together with @c Image::invert() and @c Image::inverted() methods.
  enum InvertMode
  {
    InvertNone  = 0x0,
    InvertRed   = 0x1,
    InvertGreen = 0x2,
    InvertBlue  = 0x4,
    InvertAlpha = 0x8,
    InvertRgb   = InvertRed | InvertGreen | InvertBlue,
    InvertRgba  = InvertRgb | InvertAlpha
  };

  static err_t invert(Image& dest, const Image& src, uint32_t invertMode);

  FOG_INLINE err_t invert(uint32_t invertMode)
  { return invert(*this, *this, invertMode); }

  // [Mirror]

  //! @brief Invert modes used together with @c Image::mirror() and @c Image::mirrored() methods.
  enum MirrorMode
  {
    MirrorNone       = 0x0,
    MirrorHorizontal = 0x1,
    MirrorVertical   = 0x2,
    MirrorBoth       = 0x3
  };

  static err_t mirror(Image& dest, const Image& src, uint32_t mirrorMode);

  FOG_INLINE err_t mirror(uint32_t mirrorMode)
  { return mirror(*this, *this, mirrorMode); }

  // [Rotate]

  //! @brief Invert modes used together with @c Image::rotate() and @c Image::rotated() methods.
  enum RotateMode
  {
    Rotate0 = 0,
    Rotate90,
    Rotate180,
    Rotate270
  };

  static err_t rotate(Image& dest, const Image& src, uint32_t rotateMode);

  FOG_INLINE err_t rotate(uint32_t rotateMode)
  { return rotate(*this, *this, rotateMode); }

  // [Channel related]

  Image extractChannel(uint32_t channel) const;

  // [Filtering]

  err_t filter(const ImageFilter& f);
  err_t filter(const ImageFilter& f, const Rect& r);

  err_t filter(const ColorLut& lut);
  err_t filter(const ColorLut& lut, const Rect& r);

  err_t filter(const ColorMatrix& mat);
  err_t filter(const ColorMatrix& mat, const Rect& r);

  // [Painting]

  err_t clear(uint32_t c0);

  err_t drawPixel(const Point& pt, uint32_t c0);
  err_t drawLine(const Point& pt0, const Point& pt1, uint32_t c0, bool lastPoint = true);

  err_t fillRect(const Rect& r, uint32_t c0, bool over = true);

  err_t fillQGradient(const Rect& r, Rgba c0, Rgba c1, Rgba c2, Rgba c3, bool over = true);
  err_t fillHGradient(const Rect& r, Rgba c0, Rgba c1, bool over = true);
  err_t fillVGradient(const Rect& r, Rgba c0, Rgba c1, bool over = true);

  err_t drawImage(const Point& pt, const Image& src, uint32_t op = CompositeSrcOver, uint32_t opacity = 255);
  err_t drawImage(const Point& pt, const Image& src, const Rect& srcRect, uint32_t op = CompositeSrcOver, uint32_t opacity = 255);

  err_t scroll(int x, int y);
  err_t scroll(int x, int y, const Rect& r);

  // [Misc]

  FOG_INLINE bool hasPoint(int x, int y) 
  { 
    return (uint)x < (uint)width() && (uint)y < (uint)height(); 
  }
  
  FOG_INLINE bool hasPoint(const Point& pt) 
  { 
    return (uint)pt.x() < (uint)width() && (uint)pt.y() < (uint)height(); 
  }

  // [WinAPI functions]
#if defined(FOG_OS_WINDOWS)
  HBITMAP toHBITMAP();
  bool fromHBITMAP(HBITMAP hBitmap);
#endif // FOG_OS_WINDOWS

  // [Read]

  err_t readFile(const String32& fileName);
  err_t readStream(Stream& stream);
  err_t readStream(Stream& stream, const String32& extension);
  err_t readMemory(const void* data, sysuint_t size);
  err_t readMemory(const void* data, sysuint_t size, const String32& extension);

  // [Write]

  err_t writeFile(const String32& fileName);
  err_t writeStream(Stream& stream, const String32& extension);

  // [Overloaded Operators]

  FOG_INLINE Image& operator=(const Image& other)
  { set(other); return *this; }

  // [Statics]

  static sysint_t calcStride(int width, int depth);
  static int formatToDepth(int format);

  // [Members]

  FOG_DECLARE_D(Data)
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_IMAGE_H
