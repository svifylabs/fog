// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_STRING_H
#define _FOG_CORE_STRING_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Core/Assert.h>
#include <Fog/Core/Atomic.h>
#include <Fog/Core/Char.h>
#include <Fog/Core/Constants.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/TypeInfo.h>

//! @addtogroup Fog_Core
//! @{

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

// foreign classes
struct Locale;
struct TextCodec;

template<typename T> struct Sequence;
template<typename T> struct Vector;

// local classes defined in Core/String.h or Core/String_gen.h.
struct Stub8;
struct Ascii8;
struct Local8;
struct Utf8;
struct Utf16;
struct Utf32;

struct String8;
struct String16;
struct String32;
struct StringFilter8;
struct StringFilter16;
struct StringFilter32;
struct StringMatcher8;
struct StringMatcher16;
struct StringMatcher32;

// ============================================================================
// [Typedefs]
// ============================================================================

#if FOG_SIZEOF_WCHAR_T == 2
typedef String16 StringW;
typedef Utf16 StubW;
#else
typedef String32 StringW;
typedef Utf32 StubW;
#endif // FOG_SIZEOF_WCHAR_T

// ============================================================================
// [Fog::Stub8]
// ============================================================================

struct FOG_HIDDEN Stub8
{
  // [Construction / Destruction]

  explicit FOG_INLINE Stub8(
    const char* str,
    sysuint_t length = DetectLength) : _str((const Char8*)str), _length(length)
  {}

  explicit FOG_INLINE Stub8(
    const uint8_t* str,
    sysuint_t length = DetectLength) : _str((const Char8*)str), _length(length)
  {}

  explicit FOG_INLINE Stub8(
    const Char8* str,
    sysuint_t length = DetectLength) : _str(str), _length(length)
  {}

  explicit FOG_INLINE Stub8(const String8& str);

  // [Methods]

  FOG_INLINE const Char8* str() const { return _str; }
  FOG_INLINE sysuint_t length() const { return _length; }

  // [Members]

private:
  const Char8* _str;
  sysuint_t _length;
};

// ============================================================================
// [Fog::Ascii8]
// ============================================================================

struct FOG_HIDDEN Ascii8 : public Stub8
{
  // [Construction / Destruction]

  explicit FOG_INLINE Ascii8(
    const char* str,
    sysuint_t length = DetectLength) : Stub8(str, length)
  {}

  explicit FOG_INLINE Ascii8(
    const uint8_t* str,
    sysuint_t length = DetectLength) : Stub8(str, length)
  {}

  explicit FOG_INLINE Ascii8(
    const Char8* str,
    sysuint_t length = DetectLength) : Stub8(str, length)
  {}

  explicit FOG_INLINE Ascii8(const String8& str);
};

// ============================================================================
// [Fog::Local8]
// ============================================================================

struct FOG_HIDDEN Local8 : public Stub8
{
  // [Construction / Destruction]

  explicit FOG_INLINE Local8(
    const char* str,
    sysuint_t length = DetectLength) : Stub8(str, length)
  {}

  explicit FOG_INLINE Local8(
    const uint8_t* str,
    sysuint_t length = DetectLength) : Stub8(str, length)
  {}

  explicit FOG_INLINE Local8(
    const Char8* str,
    sysuint_t length = DetectLength) : Stub8(str, length)
  {}

  explicit FOG_INLINE Local8(const String8& str);
};

// ============================================================================
// [Fog::Utf8]
// ============================================================================

struct FOG_HIDDEN Utf8 : public Stub8
{
  // [Construction / Destruction]

  explicit FOG_INLINE Utf8(
    const char* str,
    sysuint_t length = DetectLength) : Stub8(str, length)
  {}

  explicit FOG_INLINE Utf8(
    const uint8_t* str,
    sysuint_t length = DetectLength) : Stub8(str, length)
  {}

  explicit FOG_INLINE Utf8(
    const Char8* str,
    sysuint_t length = DetectLength) : Stub8(str, length)
  {}

  explicit FOG_INLINE Utf8(const String8& str);
};

// ============================================================================
// [Fog::Utf16]
// ============================================================================

struct FOG_HIDDEN Utf16
{
  // [Construction / Destruction]

  explicit FOG_INLINE Utf16(
    const uint16_t* str,
    sysuint_t length = DetectLength) : _str((const Char16*)str), _length(length)
  {}

  explicit FOG_INLINE Utf16(
    const Char16* str,
    sysuint_t length = DetectLength) : _str(str), _length(length)
  {}

#if FOG_SIZEOF_WCHAR_T == 2
  explicit FOG_INLINE Utf16(
    const wchar_t* str,
    sysuint_t length = DetectLength) : _str((const Char16*)str), _length(length)
  {}
#endif

  explicit FOG_INLINE Utf16(const String16& str);

  // [Methods]

  FOG_INLINE const Char16* str() const { return _str; }
  FOG_INLINE sysuint_t length() const { return _length; }

  // [Members]

private:
  const Char16* _str;
  sysuint_t _length;
};

// ============================================================================
// [Fog::Utf32]
// ============================================================================

struct FOG_HIDDEN Utf32
{
  // [Construction / Destruction]

  explicit FOG_INLINE Utf32(const uint32_t* str, sysuint_t length = DetectLength) :
    _str((const Char32*)str), _length(length)
  {}

  explicit FOG_INLINE Utf32(const Char32* str, sysuint_t length = DetectLength) :
    _str(str), _length(length)
  {}

#if FOG_SIZEOF_WCHAR_T == 4
  explicit FOG_INLINE Utf32(const wchar_t* str, sysuint_t length = DetectLength) :
    _str((const Char32*)str), _length(length)
  {}
#endif

  explicit FOG_INLINE Utf32(const String32& str);

  // [Methods]

  FOG_INLINE const Char32* str() const { return _str; }
  FOG_INLINE sysuint_t length() const { return _length; }

  // [Members]

private:
  const Char32* _str;
  sysuint_t _length;
};

// ============================================================================
// [Fog::FormatFlags]
// ============================================================================

struct FOG_HIDDEN FormatFlags
{
  // [Construction / Destruction]

  FOG_INLINE FormatFlags(
    sysuint_t precision = NoPrecision,
    sysuint_t width = NoWidth,
    uint32_t flags = NoFlags) : 
      precision(precision), width(width), flags(flags)
  {
  }

  // [Members]

  sysuint_t precision;
  sysuint_t width;
  uint32_t flags;

  static const sysuint_t NoPrecision   = (sysuint_t)-1;
  static const sysuint_t NoWidth       = (sysuint_t)-1;

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

  FOG_INLINE Range(sysuint_t index = 0, sysuint_t length = DetectLength) :
    index(index), length(length)
  {
  }

  // [Members]

  sysuint_t index;
  sysuint_t length;
};

} // Fog namespace

// ============================================================================
// [Generator]
// ============================================================================

#define __G_GENERATE

#define __G_SIZE 1
#include <Fog/Core/String_gen.h>
#undef __G_SIZE

#define __G_SIZE 2
#include <Fog/Core/String_gen.h>
#undef __G_SIZE

#define __G_SIZE 4
#include <Fog/Core/String_gen.h>
#undef __G_SIZE

#undef __G_GENERATE

namespace Fog {

FOG_INLINE Stub8::Stub8(const String8& str) :
  _str(str.cData()), _length(str.length()) {}

FOG_INLINE Ascii8::Ascii8(const String8& str) :
  Stub8(str.cData(), str.length()) {}

FOG_INLINE Local8::Local8(const String8& str) :
  Stub8(str.cData(), str.length()) {}

FOG_INLINE Utf8::Utf8(const String8& str) :
  Stub8(str.cData(), str.length()) {}

FOG_INLINE Utf16::Utf16(const String16& str) :
  _str(str.cData()), _length(str.length()) {}

FOG_INLINE Utf32::Utf32(const String32& str) :
  _str(str.cData()), _length(str.length()) {}

} // Fog namespace

//! @}

// ============================================================================
// [Fog::TypeInfo<T>]
// ============================================================================

FOG_DECLARE_TYPEINFO(Fog::FormatFlags, Fog::PrimitiveType)
FOG_DECLARE_TYPEINFO(Fog::Range      , Fog::PrimitiveType)

// [Guard]
#endif // _FOG_CORE_STRING_H
