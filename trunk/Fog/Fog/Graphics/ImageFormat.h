// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_IMAGEFORMAT_H
#define _FOG_GRAPHICS_IMAGEFORMAT_H

// [Dependencies]
#include <Fog/Core/Assert.h>
#include <Fog/Core/Atomic.h>
#include <Fog/Core/String.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Rgba.h>

//! @defgroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Fog::ImageFormat]
// ============================================================================

struct FOG_API ImageFormat
{
  // [Enums]

  // don't change order of these values, it's very dependent to
  // values that will be initialized in Image.cpp and dependent
  // to Converter.cpp tables.

  //! @brief Pixel format. 
  //!
  //! @note All pixel formats are CPU endian dependent. So @c ARGB32 pixels
  //! will be stored differenly in memory on machines with different endianness.
  //!
  //! @c ARGB32, @c PRGB32 and @c XRGB32 formats:
  //! - Little endian: BBGGRRAA
  //! - Big endian   : AARRGGBB
  //! @c RGB24 format:
  //! - Little endian: BBGGRR
  //! - Big endian   : RRGGBB
  //! @c BGR24 format:
  //! - Little endian: RRGGBB
  //! - Big endian   : BBGGRR
  //! @c I8 format:
  //! - no difference, 1 BYTE per pixel (index to palette)
  //! @c A8 format:
  //! - no difference, 1 BYTE per pixel
  //! @c A1 format:
  //! - no difference, 1 BIT per pixel
  enum Format
  {
    // NOTE: 
    // - ARGB32 format is expected to be 0 !
    // - PRGB32 format is expected to be 1 !
    // - XRGB32 format is expected to be 2 !

    //! @brief 32 bit RGBA (equivalent for @c Rgba).
    ARGB32 = 0,
    //! @brief 32 bit RGBA premultiplied.
    PRGB32 = 1,
    //! @brief 32 bit RGB (equivalent for @c Rgba without alpha channel - full opaque).
    XRGB32 = 2,

    //! @brief 24 bit RGB.
    RGB24 = 3,
    //! @brief 24 bit BGR.
    BGR24 = 4,

    //! @brief 8 bit indexed pixel format.
    I8 = 5,
    //! @brief 8 bit alpha channel.
    A8 = 6,
    //! @brief 1 bit alpha channel (mask).
    A1 = 7,

    //! @brief Count of Fog pixel formats.
    Count = 8,

    //! @brief Invalid pixel format.
    Invalid = 0xFFFFFFFF
  };

  enum BytePos
  {
#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
    ARGB32_RBytePos = 2U,
    ARGB32_GBytePos = 1U,
    ARGB32_BBytePos = 0U,
    ARGB32_ABytePos = 3U,

    RGB24_RBytePos = 2U,
    RGB24_GBytePos = 1U,
    RGB24_BBytePos = 0U,

    BGR24_RBytePos = 0U,
    BGR24_GBytePos = 1U,
    BGR24_BBytePos = 2U,
#else // FOG_BYTE_ORDER == FOG_BIG_ENDIAN
    ARGB32_RBytePos = 1U,
    ARGB32_GBytePos = 2U,
    ARGB32_BBytePos = 3U,
    ARGB32_ABytePos = 0U,

    RGB24_RBytePos = 0U,
    RGB24_GBytePos = 1U,
    RGB24_BBytePos = 2U,

    BGR24_RBytePos = 2U,
    BGR24_GBytePos = 1U,
    BGR24_BBytePos = 0U,
#endif // FOG_BYTE_ORDER

    XRGB32_RBytePos = ARGB32_RBytePos,
    XRGB32_GBytePos = ARGB32_GBytePos,
    XRGB32_BBytePos = ARGB32_BBytePos,
    XRGB32_XBytePos = ARGB32_ABytePos
  };

  enum Shift
  {
    ARGB32_AShift = 24U,
    ARGB32_RShift = 16U,
    ARGB32_GShift = 8U,
    ARGB32_BShift = 0U
  };

  enum Mask
  {
    ARGB32_RMask = 0x000000FFU << ARGB32_RShift,
    ARGB32_GMask = 0x000000FFU << ARGB32_GShift,
    ARGB32_BMask = 0x000000FFU << ARGB32_BShift,
    ARGB32_AMask = 0x000000FFU << ARGB32_AShift
  };

  // [Data]

  struct Data
  {
    char name[16];
    uint32_t id;

    uint32_t depth;

    uint32_t rMask;
    uint32_t gMask;
    uint32_t bMask;
    uint32_t aMask;

    uint32_t rShift;
    uint32_t gShift;
    uint32_t bShift;
    uint32_t aShift;

    uint8_t rBytePos;
    uint8_t gBytePos;
    uint8_t bBytePos;
    uint8_t aBytePos;

    uint32_t isPremultiplied : 1;
    uint32_t isIndexed : 1;
  };

  static Data* _formats;
  static Data* _getById(uint32_t id);
  static Data* _getByName(const Fog::String32& name);

  // [Construction / Destruction]

  FOG_INLINE ImageFormat()
    : _d(&_formats[ImageFormat::Count])
  {
  }

  FOG_INLINE ImageFormat(Data* d)
    : _d(d)
  {
  }

  FOG_INLINE ImageFormat(const ImageFormat& other)
    : _d(other._d)
  {
  }

  FOG_INLINE ImageFormat(uint32_t format)
    : _d(_getById(format))
  {
  }

  FOG_INLINE ImageFormat(const Fog::String32& name)
    : _d(_getByName(name))
  {
  }

  FOG_INLINE ~ImageFormat()
  {
  }

  // [Members access]

  //! @brief Returns pixel format name, for example \"ARGB32\".
  FOG_INLINE const char* name() const { return _d->name; }
  //! @brief Returns pixel format id, see @c Format enumeration.
  FOG_INLINE uint32_t id() const { return _d->id; }

  //! @brief Returns @c true if pixel format is not invalid.
  FOG_INLINE bool ok() const { return _d->id != Invalid; }
  //! @brief Returns @c true if pixel format is invalid.
  FOG_INLINE bool isInvalid() const { return _d->id == Invalid; }

  //! @brief Returns depth (bits per pixel).
  FOG_INLINE uint32_t depth() const { return _d->depth; }

  //! @brief Returns red mask.
  FOG_INLINE uint32_t rMask() const { return _d->rMask; }
  //! @brief Returns green mask.
  FOG_INLINE uint32_t gMask() const { return _d->gMask; }
  //! @brief Returns blue mask.
  FOG_INLINE uint32_t bMask() const { return _d->bMask; }
  //! @brief Returns alpha mask.
  FOG_INLINE uint32_t aMask() const { return _d->aMask; }

  //! @brief Returns red shift.
  FOG_INLINE uint32_t rShift() const { return _d->rShift; }
  //! @brief Returns green shift.
  FOG_INLINE uint32_t gShift() const { return _d->gShift; }
  //! @brief Returns blue shift.
  FOG_INLINE uint32_t bShift() const { return _d->bShift; }
  //! @brief Returns alpha shift.
  FOG_INLINE uint32_t aShift() const { return _d->aShift; }

  //! @brief Returns red byte position.
  FOG_INLINE uint8_t rBytePos() const { return _d->rBytePos; }
  //! @brief Returns green byte position.
  FOG_INLINE uint8_t gBytePos() const { return _d->gBytePos; }
  //! @brief Returns blue byte position.
  FOG_INLINE uint8_t bBytePos() const { return _d->bBytePos; }
  //! @brief Returns alpha byte position.
  FOG_INLINE uint8_t aBytePos() const { return _d->aBytePos; }

  //! @brief Returns @c true if pixel format is premultiplied (only @c ImageFormat::PRGB32 format has this set to @c true).
  FOG_INLINE uint32_t isPremultiplied() const { return _d->isPremultiplied; }

  //! @brief Returns @c true if pixel format is indexed (palette based). Only @c I8 format.
  FOG_INLINE bool isIndexed() const { return _d->isIndexed; }

  //! @brief Returns @c true if pixel format contains alpha channel.
  FOG_INLINE bool hasAlpha() const { return _d->aMask != 0x00000000; }

  //! @brief Returns @c true if pixel format has same masks as @c rMask, @c gMask and @c bMask.
  FOG_INLINE bool hasMasks(uint32_t rMask, uint32_t gMask, uint32_t bMask) const
  {
    return rMask == _d->rMask &&
           gMask == _d->gMask &&
           bMask == _d->bMask;
  }

  //! @brief Returns @c true if pixel format has same masks as @c rMask, @c gMask, @c bMask and @c aMask.
  FOG_INLINE bool hasMasks(uint32_t rMask, uint32_t gMask, uint32_t bMask, uint32_t aMask) const
  {
    return rMask == _d->rMask &&
           gMask == _d->gMask &&
           bMask == _d->bMask &&
           aMask == _d->aMask;
  }

  // [Set]

  FOG_INLINE ImageFormat& set(uint32_t id)
  { _d = _getById(id); return *this; }

  FOG_INLINE ImageFormat& set(const Fog::String32& name)
  { _d = _getByName(name); return *this; }

  // [Overloaded Operators]

  FOG_INLINE ImageFormat& operator=(const ImageFormat& other)
  { _d = other._d; return *this;}

  FOG_INLINE ImageFormat& operator=(uint32_t id)
  { return set(id); }

  FOG_INLINE ImageFormat& operator=(const Fog::String32& name)
  { return set(name); }

  // [Static]

  static FOG_INLINE ImageFormat getById(uint32_t id)
  { return ImageFormat(_getById(id)); }

  static FOG_INLINE ImageFormat getByName(const Fog::String32& name)
  { return ImageFormat(_getByName(name)); }

  static FOG_INLINE ImageFormat argb32()  { return ImageFormat(&_formats[ARGB32]); }
  static FOG_INLINE ImageFormat prgb32()  { return ImageFormat(&_formats[PRGB32]); }
  static FOG_INLINE ImageFormat xrgb32()  { return ImageFormat(&_formats[XRGB32]); }
  static FOG_INLINE ImageFormat rgb24()   { return ImageFormat(&_formats[RGB24 ]); }
  static FOG_INLINE ImageFormat bgr24()   { return ImageFormat(&_formats[BGR24 ]); }
  static FOG_INLINE ImageFormat i8()      { return ImageFormat(&_formats[I8    ]); }
  static FOG_INLINE ImageFormat a8()      { return ImageFormat(&_formats[A8    ]); }
  static FOG_INLINE ImageFormat a1()      { return ImageFormat(&_formats[A1    ]); }
  static FOG_INLINE ImageFormat invalid() { return ImageFormat(&_formats[Count ]); }

  // [Members]

  FOG_DECLARE_D(Data)
};

} // Fog namespace

FOG_INLINE bool operator==(const Fog::ImageFormat& a, const Fog::ImageFormat& b) { return a.id() == b.id(); }
FOG_INLINE bool operator!=(const Fog::ImageFormat& a, const Fog::ImageFormat& b) { return a.id() != b.id(); }

//! @}

FOG_DECLARE_TYPEINFO(Fog::ImageFormat, Fog::MoveableType)

// [Guard]
#endif // _FOG_GRAPHICS_IMAGEFORMAT_H
