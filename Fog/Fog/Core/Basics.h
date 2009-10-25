// [Fog/Core Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_BASICS_H
#define _FOG_CORE_BASICS_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Core/Assert.h>
#include <Fog/Core/Constants.h>
#include <Fog/Core/Char.h>
#include <Fog/Core/TypeInfo.h>

//! @addtogroup Fog_Core
//! @{

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct ByteArray;
struct Char;
struct String;

// ============================================================================
// [Fog::Str8]
// ============================================================================

struct FOG_HIDDEN Str8
{
  // [Construction / Destruction]

  explicit FOG_INLINE Str8(const char* data, sysuint_t length = DetectLength) : _data(data), _length(length) {}
  explicit FOG_INLINE Str8(const uint8_t* data, sysuint_t length = DetectLength) : _data(reinterpret_cast<const char*>(data)), _length(length) {}

  // [Methods]

  FOG_INLINE const char* getData() const { return _data; }
  FOG_INLINE sysuint_t getLength() const { return _length; }

  // [Operator overload]

  FOG_INLINE char operator[](sysuint_t index) const
  {
    FOG_ASSERT_X(index <= _length, "Fog::Str8::operator[] - Index out of range");
    return _data[index];
  }

  // [Members]

private:
  const char* _data;
  sysuint_t _length;
};

// ============================================================================
// [Fog::Ascii8]
// ============================================================================

struct FOG_HIDDEN Ascii8 : public Str8
{
  // [Construction / Destruction]

  explicit FOG_INLINE Ascii8(const char* data, sysuint_t length = DetectLength) : Str8(data, length) {}
  explicit FOG_INLINE Ascii8(const uint8_t* data, sysuint_t length = DetectLength) : Str8(data, length) {}
};

// ============================================================================
// [Fog::Local8]
// ============================================================================

struct FOG_HIDDEN Local8 : public Str8
{
  // [Construction / Destruction]

  explicit FOG_INLINE Local8(const char* data, sysuint_t length = DetectLength) : Str8(data, length) {}
  explicit FOG_INLINE Local8(const uint8_t* data, sysuint_t length = DetectLength) : Str8(data, length) {}
};

// ============================================================================
// [Fog::Utf8]
// ============================================================================

struct FOG_HIDDEN Utf8 : public Str8
{
  // [Construction / Destruction]

  explicit FOG_INLINE Utf8(const char* data, sysuint_t length = DetectLength) : Str8(data, length) {}
  explicit FOG_INLINE Utf8(const uint8_t* data, sysuint_t length = DetectLength) : Str8(data, length) {}
};

// ============================================================================
// [Fog::Utf16]
// ============================================================================

struct FOG_HIDDEN Utf16
{
  // [Construction / Destruction]

  explicit FOG_INLINE Utf16(const uint16_t* data, sysuint_t length = DetectLength) : _data(reinterpret_cast<const Char*>(data)), _length(length) {}
  explicit FOG_INLINE Utf16(const Char* data, sysuint_t length = DetectLength) : _data(data), _length(length) {}

#if FOG_SIZEOF_WCHAR_T == 2
  explicit FOG_INLINE Utf16(const wchar_t* data, sysuint_t length = DetectLength) : _data(reinterpret_cast<const Char*>(data)), _length(length) {}
#endif // FOG_SIZEOF_WCHAR_T == 2

  // [Methods]

  FOG_INLINE const Char* getData() const { return _data; }
  FOG_INLINE sysuint_t getLength() const { return _length; }

  // [Operator overload]

  FOG_INLINE const Char& operator[](sysuint_t index) const
  {
    FOG_ASSERT_X(index <= _length, "Fog::Utf16::operator[] - Index out of range");
    return _data[index];
  }

  // [Members]

private:
  const Char* _data;
  sysuint_t _length;
};

// ============================================================================
// [Fog::FormatFlags]
// ============================================================================

struct FOG_HIDDEN FormatFlags
{
  // [Construction / Destruction]

  FOG_INLINE FormatFlags(sysuint_t precision = NoPrecision, sysuint_t width = NoWidth, uint32_t flags = NoFlags) :
    precision(precision), width(width), flags(flags)
  {
  }

  // [Members]

  sysuint_t precision;
  sysuint_t width;
  uint32_t flags;

  static const sysuint_t NoPrecision = (sysuint_t)-1;
  static const sysuint_t NoWidth     = (sysuint_t)-1;

  //! @brief Number format flags.
  enum Flags
  {
    //! @brief Show group separators (sprintf ' flag).
    ThousandsGroup = (1 << 0),
    //! @brief Align to left (sprintf '-' flag).
    LeftAdjusted = (1 << 1),
    //! @brief Always show sign (sprintf '+' flag).
    ShowSign = (1 << 2),
    //! @brief Blank character instead of positive sign (sprintf ' ' flag).
    BlankPositive = (1 << 3),
    //! @brief Use alternate form (sprintf '#' flag).
    Alternate = (1 << 4),
    //! @brief Zero padded (sprintf '0' flag).
    ZeroPadded = (1 << 5),
    //! @brief Capitalize number output.
    Capitalize = (1 << 6),
    //! @brief Capitalize E or X in number output.
    CapitalizeEOrX = (1 << 7)
  };
};

// ============================================================================
// [Fog::Range]
// ============================================================================

struct FOG_HIDDEN Range
{
  // [Construction / Destruction]

  FOG_INLINE Range(sysuint_t index = 0, sysuint_t length = DetectLength) : index(index), length(length) {}

  // [Members]

  sysuint_t index;
  sysuint_t length;
};

} // Fog namespace

//! @}

// ============================================================================
// [Fog::TypeInfo<T>]
// ============================================================================

FOG_DECLARE_TYPEINFO(Fog::Str8       , Fog::PrimitiveType)
FOG_DECLARE_TYPEINFO(Fog::Ascii8     , Fog::PrimitiveType)
FOG_DECLARE_TYPEINFO(Fog::Local8     , Fog::PrimitiveType)
FOG_DECLARE_TYPEINFO(Fog::Utf8       , Fog::PrimitiveType)
FOG_DECLARE_TYPEINFO(Fog::Utf16      , Fog::PrimitiveType)

FOG_DECLARE_TYPEINFO(Fog::FormatFlags, Fog::PrimitiveType)
FOG_DECLARE_TYPEINFO(Fog::Range      , Fog::PrimitiveType)

// [Guard]
#endif // _FOG_CORE_BASICS_H
