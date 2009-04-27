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
#include <Fog/Core/RefData.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/Stream.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/Graphics/ImageFormat.h>
#include <Fog/Graphics/Palette.h>
#include <Fog/Graphics/Rgba.h>

namespace Fog {

// ============================================================================
// [Fog::Image]
// ============================================================================

struct FOG_API Image
{
  // [Data]

  struct FOG_API Data : public Fog::RefData<Data>
  {
    enum
    {
      IsReadOnly = (1U << 8)
    };

    Data();
    ~Data();

    //! @brief Image width.
    uint32_t width;
    //! @brief Image height.
    uint32_t height;
    //! @brief Image stride.
    sysint_t stride;
    //! @brief Image pixel format.
    ImageFormat format;
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

    Data* ref();
    void deref();

    FOG_INLINE void destroy() { this->~Data(); }
    FOG_INLINE void free() { Memory::free(this); }

    static Data* create(sysuint_t size, uint allocPolicy);
    static Data* create(uint32_t w, uint32_t h, const ImageFormat& format, uint allocPolicy);
    static Data* copy(const Data* other, uint allocPolicy);
  };

  static Fog::Static<Data> sharedNull;

  // [Constants]

  /*! @brief Image adopt flags. */
  enum AdoptFlags
  {
    AdoptReadOnly = 0x1,
    AdoptReversed = 0x2,
  };

  enum
  {
    /*! @brief Maximum image size (in pixels) */
    MaxSize = 16384*16384
  };

  // [Construction / Destruction]

  Image();
  explicit Image(Data* d);
  Image(const Image& other);
  Image(uint32_t w, uint32_t h, const ImageFormat& format);
  ~Image();

  // [Implicit Sharing]

  //! @copydoc Doxygen::Implicit::refCount().
  FOG_INLINE sysuint_t refCount() const { return _d->refCount.get(); }
  //! @copydoc Doxygen::Implicit::isDetached().
  FOG_INLINE bool isDetached() const { return refCount() == 1 || isReadOnly(); }
  //! @copydoc Doxygen::Implicit::detach().
  FOG_INLINE void detach() { if (!isDetached()) _detach(); }
  //! @copydoc Doxygen::Implicit::tryDetach().
  FOG_INLINE bool tryDetach() { return (!isDetached()) ? _tryDetach() : true; }
  //! @copydoc Doxygen::Implicit::_detach().
  void _detach();
  //! @copydoc Doxygen::Implicit::_tryDetach().
  bool _tryDetach();
  //! @copydoc Doxygen::Implicit::free().
  void free();

  // [Flags]

  //! @copydoc Doxygen::Implicit::flags().
  FOG_INLINE uint32_t flags() const { return _d->flags; }
  //! @copydoc Doxygen::Implicit::isDynamic().
  FOG_INLINE bool isDynamic() const { return _d->flags.anyOf(Data::IsDynamic); }
  //! @copydoc Doxygen::Implicit::isSharable().
  FOG_INLINE bool isSharable() const { return _d->flags.anyOf(Data::IsSharable); }
  //! @copydoc Doxygen::Implicit::isNull().
  FOG_INLINE bool isNull() const { return _d->flags.anyOf(Data::IsNull); }
  //! @copydoc Doxygen::Implicit::isStrong().
  FOG_INLINE bool isStrong() const { return _d->flags.anyOf(Data::IsStrong); }
  //! @brief Returns true if image is read only.
  FOG_INLINE bool isReadOnly() const { return _d->flags.anyOf(Data::IsReadOnly); }

  // [Data]

  FOG_INLINE const uint8_t* cData()
  {
    return _d->data;
  }

  FOG_INLINE uint8_t* mData()
  {
    detach();
    return _d->data;
  }

  FOG_INLINE uint8_t* xData()
  {
    FOG_ASSERT(isDetached());
    return _d->data;
  }

  FOG_INLINE const uint8_t* cFirst()
  {
    return _d->first;
  }

  FOG_INLINE uint8_t* mFirst()
  {
    detach();
    return _d->first;
  }

  FOG_INLINE uint8_t* xFirst()
  {
    FOG_ASSERT(isDetached());
    return _d->first;
  }

  FOG_INLINE const uint8_t* cScanline(uint32_t i) const
  {
    FOG_ASSERT((sysuint_t)i < (ulong)_d->height);
    return _d->first + (sysint_t)i * _d->stride;
  }

  FOG_INLINE uint8_t* mScanline(uint32_t i)
  {
    FOG_ASSERT((sysuint_t)i < (ulong)_d->height);
    detach();
    return _d->first + (sysint_t)i * _d->stride;
  }

  FOG_INLINE uint8_t* xScanline(uint32_t i)
  {
    FOG_ASSERT((sysuint_t)i < (ulong)_d->height);
    FOG_ASSERT(isDetached());
    return _d->first + (sysint_t)i * _d->stride;
  }

  // [Dimensions]

  FOG_INLINE int width() const { return _d->width; }
  FOG_INLINE int height() const { return _d->height; }
  FOG_INLINE sysint_t stride() const { return _d->stride; }
  FOG_INLINE bool isEmpty() const { return _d->width == 0 || _d->height == 0; }

  // [Format]

  FOG_INLINE const ImageFormat& format() const { return _d->format; }
  FOG_INLINE bool isIndexed() const { return format().isIndexed(); }
  FOG_INLINE bool hasAlpha() const { return format().hasAlpha(); }

  // [Create / Adopt]

  bool create(
    uint32_t w, uint32_t h, const ImageFormat& format);
  
  bool adopt(
    uint32_t w, uint32_t h, sysint_t stride, 
    const ImageFormat& format, 
    const uint8_t* mem, uint32_t adoptFlags);

  // [Set]

  bool set(const Image& other);
  bool setDeep(const Image& other);

  // [Convert]

  bool convert(const ImageFormat& format);

  // [Palette]

  FOG_INLINE const Palette& palette() const { return _d->palette; }

  bool setPalette(const Palette& palette);
  bool setPalette(sysuint_t index, sysuint_t count, const Rgba* rgba);

  // [Swap RGB and RGBA]

  bool swapRgb();
  bool swapRgba();

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

  FOG_INLINE Image& invert(uint32_t invertMode)
  {
    invert(*this, *this, invertMode);
    return *this;
  }

  static bool invert(Image& dest, const Image& src, uint32_t invertMode);

  // [Mirror]

  /*! @brief Invert modes used together with @c Image::mirror() and @c Image::mirrored() methods. */
  enum MirrorMode
  {
    MirrorNone       = 0x0,
    MirrorHorizontal = 0x1,
    MirrorVertical   = 0x2,
    MirrorBoth       = 0x3
  };

  FOG_INLINE Image& mirror(uint32_t mirrorMode)
  {
    mirror(*this, *this, mirrorMode);
    return *this;
  }

  static bool mirror(Image& dest, const Image& src, uint32_t mirrorMode);

  // [Rotate]

  /*! @brief Invert modes used together with @c Image::rotate() and @c Image::rotated() methods. */
  enum RotateMode
  {
    Rotate0,
    Rotate90,
    Rotate180,
    Rotate270
  };

  FOG_INLINE Image& rotate(uint32_t rotateMode)
  {
    rotate(*this, *this, rotateMode);
    return *this;
  }

  static bool rotate(Image& dest, const Image& src, uint32_t rotateMode);

  // [Blur]

  /*! @brief Blur modes used together with @c Image::blur() and @c @c Image::blured() methods. */
  enum BlurMode
  {
    /*! @brief Fastest, but very low quality blur that can produce incorrect
      results for some special images.

      This blur mode can be used to blur glyphs.

      The blur matrix is:
        0 1 0
        1 0 1 
        0 1 0
    */
    Blur0x4,

    /*! @brief Slower blur with not so good results.

      This blur mode can be used to blur glyphs.

      The blur matrix is:
        0 1 0
        1 1 1 
        0 1 0
    */
    Blur1x4,
    /*! @brief Slower blur, but with optimal results. Edges are blured better,
      because center point is like two points with same color. 

      This method is slower than @c Blur0x4 and @c Blur1x4, but produces
      best results.

      The blur matrix is:
        0 1 0
        1 2 1 
        0 1 0
    */
    Blur2x4
  };

  FOG_INLINE Image& blur(uint32_t blurMode)
  {
    blur(*this, *this, blurMode);
    return *this;
  }

  static bool blur(Image& dest, const Image& src, uint32_t blurMode);

  // [Alpha channel related]

  FOG_INLINE Image alphaChannel() const
  {
    Image result;
    alphaChannel(result, *this);
    return result;
  }

  static bool alphaChannel(Image& dest, const Image& src);

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

  err_t readFile(const Fog::String32& fileName);
  err_t readStream(Fog::Stream& stream);
  err_t readStream(Fog::Stream& stream, const Fog::String32& extension);
  err_t readMemory(const uint8_t* data, sysuint_t size);
  err_t readMemory(const uint8_t* data, sysuint_t size, const Fog::String32& extension);

  // [Write]

  err_t writeFile(const Fog::String32& fileName);
  err_t writeStream(Fog::Stream& stream, const Fog::String32& extension);

  // [Overloaded Operators]

  FOG_INLINE Image& operator=(const Image& other)
  { set(other); return *this; }

  // [Statics]

  static sysint_t calcStride(uint32_t width, uint32_t depth);

  // [Members]

  FOG_DECLARE_D(Data)
};

} // Fog namespace

// [Guard]
#endif // _FOG_GRAPHICS_IMAGE_H
