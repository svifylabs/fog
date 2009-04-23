// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _CHAR_CHAR_H
#define _CHAR_CHAR_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/TypeInfo.h>

// ============================================================================
// [Ascii and Unicode Data]
// ============================================================================

FOG_CVAR_EXTERN const uint8_t Core_CType_Ascii_ctypeTable[256];
FOG_CVAR_EXTERN const uint8_t Core_CType_Ascii_toLowerTable[256];
FOG_CVAR_EXTERN const uint8_t Core_CType_Ascii_toUpperTable[256];

FOG_CVAR_EXTERN const uint32_t Core_CType_Unicode_infoTable[2048];
FOG_CAPI_EXTERN uint32_t Core_CType_Unicode_combine(uint32_t uc, uint32_t comb);

//! @defgroup Core
//! @{

namespace Fog {

#include <Fog/Core/Pack.h>

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct Char8;
struct Char16;
struct Char32;

// ============================================================================
// [Fog::CType]
// ============================================================================

//! @brief CType namespace
struct CType
{
  // [Ascii]

  enum AsciiCType
  {
    //! @brief Mask for all lowercase characters: <code>abcdefghijklmnopqrstuvwxyz</code>.
    AsciiCType_Lower = (1 << 0),
    //! @brief Mask for all uppercase characters: <code>ABCDEFGHIJKLMNOPQRSTUVWXYZ</code>.
    AsciiCType_Upper = (1 << 1),
    //! @brief Mask for all hex characters except digits: <code>abcdefABCDEF</code>.
    AsciiCType_Hex = (1 << 2),
    //! @brief Mask for all ascii digits: <code>0123456789</code>
    AsciiCType_Digit = (1 << 3),
    //! @brief Mask for underscore character: <code>_</code>
    AsciiCType_Underscore = (1 << 4),
    //! @brief Mask for all whitespaces: <code>space \\n \\r \\t \\f \\v</code>
    AsciiCType_Space = (1 << 5),
    //! @brief Mask for all puncuation characters.
    AsciiCType_Punct = (1 << 6),

    //! @brief Mask for all alpha characters, combines @c AsciiCType_Lower and @c AsciiCType_Upper.
    AsciiCType_Alpha = AsciiCType_Lower | AsciiCType_Upper,
    //! @brief Mask for all alpha and numeric characters, combines @c AsciiCType_MaskAlpha and @c AsciiCType_MaskDigit.
    AsciiCType_Alnum = AsciiCType_Alpha | AsciiCType_Digit,
    //! @brief Mask for all graphics characters, combines @c AsciiCType_Punct and @c AsciiCType_Alnum.
    AsciiCType_Graph = AsciiCType_Punct | AsciiCType_Alnum,
    //! @brief Mask for all hex characters, combines @c AsciiCType_Digit and @c AsciiCType_Hex.
    AsciiCType_XDigit = AsciiCType_Digit | AsciiCType_Hex
  };

#define __FOG_CTYPE_ASCII_OVERLOAD(type, conditional, use) \
  static FOG_INLINE bool isAsciiAlpha(type ch) { return (conditional) && (Core_CType_Ascii_ctypeTable[use] & AsciiCType_Alpha) != 0; } \
  static FOG_INLINE bool isAsciiAlnum(type ch) { return (conditional) && (Core_CType_Ascii_ctypeTable[use] & AsciiCType_Alnum) != 0; } \
  static FOG_INLINE bool isAsciiLower(type ch) { return (conditional) && (Core_CType_Ascii_ctypeTable[use] & AsciiCType_Lower) != 0; } \
  static FOG_INLINE bool isAsciiUpper(type ch) { return (conditional) && (Core_CType_Ascii_ctypeTable[use] & AsciiCType_Upper) != 0; } \
  static FOG_INLINE bool isAsciiDigit(type ch) { return (conditional) && (Core_CType_Ascii_ctypeTable[use] & AsciiCType_Digit) != 0; } \
  static FOG_INLINE bool isAsciiXDigit(type ch){ return (conditional) && (Core_CType_Ascii_ctypeTable[use] & AsciiCType_XDigit) != 0; } \
  static FOG_INLINE bool isAsciiSpace(type ch) { return (conditional) && (Core_CType_Ascii_ctypeTable[use] & AsciiCType_Space) != 0; } \
  static FOG_INLINE bool isAsciiBlank(type ch) { return (conditional) && (ch == ' ' || ch == '\t'); } \
  static FOG_INLINE bool isAsciiPunct(type ch) { return (conditional) && (Core_CType_Ascii_ctypeTable[use] & AsciiCType_Punct) != 0; } \
  static FOG_INLINE bool isAsciiGraph(type ch) { return (conditional) && (Core_CType_Ascii_ctypeTable[use] & AsciiCType_Graph) != 0; } \
  static FOG_INLINE bool isAsciiPrint(type ch) { return (conditional) && ((Core_CType_Ascii_ctypeTable[use] & AsciiCType_Graph) != 0 || ch == ' '); } \
  static FOG_INLINE bool isAsciiCntrl(type ch) { return !isAsciiPrint(ch); } \
  static FOG_INLINE type toAsciiLower(type ch) { return (conditional) ? (type)Core_CType_Ascii_toLowerTable[use] : ch; } \
  static FOG_INLINE type toAsciiUpper(type ch) { return (conditional) ? (type)Core_CType_Ascii_toUpperTable[use] : ch; }

  __FOG_CTYPE_ASCII_OVERLOAD(int8_t, true, (uint8_t)(ch))
  __FOG_CTYPE_ASCII_OVERLOAD(uint8_t, true, ch)
  __FOG_CTYPE_ASCII_OVERLOAD(int16_t, (uint32_t)(ch) < 256 , (uint8_t)(ch))
  __FOG_CTYPE_ASCII_OVERLOAD(uint16_t, (uint32_t)(ch) < 256 , (uint8_t)(ch))
  __FOG_CTYPE_ASCII_OVERLOAD(int32_t, (uint32_t)(ch) < 256 , (uint8_t)(ch))
  __FOG_CTYPE_ASCII_OVERLOAD(uint32_t, (uint32_t)(ch) < 256 , (uint8_t)(ch))
  __FOG_CTYPE_ASCII_OVERLOAD(wchar_t, (uint32_t)(ch) < 256 , (uint8_t)(ch))
#undef __FOG_CTYPE_ASCII_OVERLOAD

// [Unicode]

#define __FOG_CTYPE_UNICODE_OVERLOAD(type, conditional, use) \
  static FOG_INLINE bool isAlpha(type ch) { return ch < 2048 ? (Core_CType_Unicode_infoTable[ch] & 0xC0000000) != 0 : false; } \
  static FOG_INLINE bool isLower(type ch) { return ch < 2048 ? (Core_CType_Unicode_infoTable[ch] & 0x80000000) != 0 : false; } \
  static FOG_INLINE bool isUpper(type ch) { return ch < 2048 ? (Core_CType_Unicode_infoTable[ch] & 0x40000000) != 0 : false; } \
  static FOG_INLINE type toLower(type ch) { return type(ch < 2048 ? (Core_CType_Unicode_infoTable[ch]      ) & 2047 : ch); } \
  static FOG_INLINE type toUpper(type ch) { return type(ch < 2048 ? (Core_CType_Unicode_infoTable[ch] >> 11) & 2047 : ch); } \
  static FOG_INLINE type toAscii(type ch) { return type(ch < 2048 ? (Core_CType_Unicode_infoTable[ch] >> 22) & 0x7F : 0 ); } \
  \
  static FOG_INLINE bool isSpace(type ch) { return (isAsciiSpace(ch)); } \
  static FOG_INLINE bool isBlank(type ch) { return (isAsciiBlank(ch)); } \
  static FOG_INLINE bool isDigit(type ch) { return (isAsciiDigit(ch)); } \
  static FOG_INLINE bool isAlnum(type ch) { return (isAlpha(ch) || isDigit(ch)); } \
  static FOG_INLINE bool isXDigit(type ch){ return (isAsciiXDigit(ch)); } \
  static FOG_INLINE bool isPunct(type ch) { return (ch >= type(' ') && !isAlnum(ch)); } \
  static FOG_INLINE bool isGraph(type ch) { return (isAlnum(ch) || isPunct(ch)); } \
  static FOG_INLINE bool isPrint(type ch) { return (isGraph(ch) || isSpace(ch)); } \
  static FOG_INLINE bool isCntrl(type ch) { return !isPrint(ch); }

  __FOG_CTYPE_UNICODE_OVERLOAD(uint16_t, (ch < 2048), ch)
  __FOG_CTYPE_UNICODE_OVERLOAD(uint32_t, (ch < 2048), ch)
#undef __FOG_CTYPE_UNICODE_OVERLOAD
};

// ============================================================================
// [Typedefs]
// ============================================================================

#if FOG_SIZEOF_WCHAR_T == 2
typedef Char16 CharW;
#else
typedef Char32 CharW;
#endif // FOG_SIZEOF_WCHAR_T

// FIXME: Disable this to make sources clean
// typedef Char32 Char;

// ============================================================================
// [Unicode Constants]
// ============================================================================

enum UnicodeConstants
{
  // BOM

  UTF16_BOM = 0xFEFF,
  UTF16_BOM_Swapped = 0xFFFE,
  UTF32_BOM = 0x0000FEFF,
  UTF32_BOM_Swapped = 0x0000FEFF,

  // Leading (high) surrogates are from 0xD800 - 0xDBFF

  //! @brief Leading (high) surrogate minimum (0xD800)
  LeadSurrogateMin = 0xD800U,
  //! @brief Leading (high) surrogate maximum (0xDBFF)
  LeadSurrogateMax = 0xDBFFU,

  // Trailing (low) surrogates are from 0xDC00 - 0xDFFF

  //! @brief Trailing (low) surrogate minimum (0xDC00)
  TrailSurrogateMin = 0xDC00U,
  //! @brief Trailing (low) surrogate maximum (0xDFFF)
  TrailSurrogateMax = 0xDFFFU,

  // Offsets

  LeadOffset       = LeadSurrogateMin - (0x10000U >> 10),
  SurrogateOffset  = 0x10000U - (LeadSurrogateMin << 10) - TrailSurrogateMin,

  //! @brief Maximum valid value for a Unicode code point
  MaxCodePoint     = 0x0010FFFFU
};

// ============================================================================
// [Fog::StringUtil::Utf8 Length Table]
// ============================================================================

// UTF-8 Length table.
//
// From UTF-8 to Unicode UCS-4:
// Let's take a UTF-8 byte sequence. The first byte in a new sequence will tell us how long the sequence is.
// Let's call the subsequent decimal bytes z y x w v u.
// - If z is between and including 0 - 127, then there is 1 byte z.
//   The decimal Unicode value ud = the value of z.
// - If z is between and including 192 - 223, then there are 2 bytes z y;
//   ud = (z-192)*64 + (y-128)
// - If z is between and including 224 - 239, then there are 3 bytes z y x;
//   ud = (z-224)*4096 + (y-128)*64 + (x-128)
// - If z is between and including 240 - 247, then there are 4 bytes z y x w;
//   ud = (z-240)*262144 + (y-128)*4096 + (x-128)*64 + (w-128)
// - If z is between and including 248 - 251, then there are 5 bytes z y x w v;
//   ud = (z-248)*16777216 + (y-128)*262144 + (x-128)*4096 + (w-128)*64 + (v-128)
// - If z is 252 or 253, then there are 6 bytes z y x w v u;
//   ud = (z-252)*1073741824 + (y-128)*16777216 + (x-128)*262144 + (w-128)*4096 + (v-128)*64 + (u-128)
// - If z = 254 or 255 then there is something wrong!
//
// The table looks like this:
//   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
//   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
//   2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
//   3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
//   4, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0
// - Zeros are invalid UTF-8 characters
extern FOG_API const uint8_t utf8LengthTable[256];

// ============================================================================
// [Fog::Char8]
// ============================================================================

//! @brief 8-bit character.
struct FOG_HIDDEN FOG_PACKED Char8
{
  // [Construction / Destruction]

  FOG_INLINE Char8() {}
  FOG_INLINE Char8(const Char8&  c) : _ch(c._ch) {}

  FOG_INLINE explicit Char8(int8_t   c) : _ch((uint8_t)c) {}
  FOG_INLINE explicit Char8(uint8_t  c) : _ch(c) {}
  FOG_INLINE explicit Char8(int16_t  c) : _ch((uint8_t)(uint16_t)c) {}
  FOG_INLINE explicit Char8(uint16_t c) : _ch((uint8_t)c) {}
  FOG_INLINE explicit Char8(int32_t  c) : _ch((uint8_t)(uint32_t)c) {}
  FOG_INLINE explicit Char8(uint32_t c) : _ch((uint8_t)c) {}

  // [Character]

  //! @brief Return 8-bit character value.
  FOG_INLINE uint8_t ch() const { return _ch; }

  // [Char::operator=]

  FOG_INLINE Char8& operator=(const int8_t&    ch) { _ch = (uint8_t )ch         ; return *this; }
  FOG_INLINE Char8& operator=(const uint8_t&   ch) { _ch = ch                   ; return *this; }
  FOG_INLINE Char8& operator=(const int16_t&   ch) { _ch = (uint8_t)(uint16_t)ch; return *this; }
  FOG_INLINE Char8& operator=(const uint16_t&  ch) { _ch = (uint8_t)ch          ; return *this; }
  FOG_INLINE Char8& operator=(const int32_t&   ch) { _ch = (uint8_t)(uint32_t)ch; return *this; }
  FOG_INLINE Char8& operator=(const uint32_t&  ch) { _ch = (uint8_t)ch          ; return *this; }
  FOG_INLINE Char8& operator=(const Char8&     ch) { _ch = ch._ch               ; return *this; }

  // [Implicit Conversion]

  FOG_INLINE operator bool() const { return _ch != 0; }
  FOG_INLINE operator int8_t() const { return (int8_t)_ch; }
  FOG_INLINE operator uint8_t() const { return (uint8_t)_ch; }

  // [Ascii CTypes]

  FOG_INLINE bool isAsciiAlpha() const { return CType::isAsciiAlpha(_ch); }
  FOG_INLINE bool isAsciiAlnum() const { return CType::isAsciiAlnum(_ch); }
  FOG_INLINE bool isAsciiLower() const { return CType::isAsciiLower(_ch); }
  FOG_INLINE bool isAsciiUpper() const { return CType::isAsciiUpper(_ch); }
  FOG_INLINE bool isAsciiDigit() const { return CType::isAsciiDigit(_ch); }
  FOG_INLINE bool isAsciiXDigit()const { return CType::isAsciiXDigit(_ch); }
  FOG_INLINE bool isAsciiSpace() const { return CType::isAsciiSpace(_ch); }
  FOG_INLINE bool isAsciiBlank() const { return CType::isAsciiSpace(_ch); }
  FOG_INLINE bool isAsciiPunct() const { return CType::isAsciiPunct(_ch); }
  FOG_INLINE bool isAsciiGraph() const { return CType::isAsciiGraph(_ch); }
  FOG_INLINE bool isAsciiPrint() const { return CType::isAsciiPrint(_ch); }

  FOG_INLINE Char8 toAsciiLower() const { return Char8(CType::toAsciiLower(_ch)); }
  FOG_INLINE Char8 toAsciiUpper() const { return Char8(CType::toAsciiUpper(_ch)); }

  // [Pseudo CTypes - Save as Ascii CTypes]

  FOG_INLINE bool isAlpha() const { return CType::isAsciiAlpha(_ch); }
  FOG_INLINE bool isLower() const { return CType::isAsciiLower(_ch); }
  FOG_INLINE bool isUpper() const { return CType::isAsciiUpper(_ch); }
  FOG_INLINE Char8 toLower() const { return Char8(CType::toAsciiLower(_ch)); }
  FOG_INLINE Char8 toUpper() const { return Char8(CType::toAsciiUpper(_ch)); }

  FOG_INLINE bool isSpace() const { return CType::isAsciiSpace(_ch); }
  FOG_INLINE bool isBlank() const { return CType::isAsciiBlank(_ch); }
  FOG_INLINE bool isDigit() const { return CType::isAsciiDigit(_ch); }
  FOG_INLINE bool isAlnum() const { return CType::isAsciiAlnum(_ch); }
  FOG_INLINE bool isXDigit() const { return CType::isAsciiXDigit(_ch); }
  FOG_INLINE bool isPunct() const { return CType::isAsciiPunct(_ch); }
  FOG_INLINE bool isGraph() const { return CType::isAsciiGraph(_ch); }
  FOG_INLINE bool isPrint() const { return CType::isAsciiPrint(_ch); }
  FOG_INLINE bool isCntrl() const { return CType::isAsciiCntrl(_ch); }

  // [Unicode Helpers]

  FOG_INLINE bool isValidUtf8() const { return isValidUtf8(_ch); }
  FOG_INLINE uint8_t utf8Len() const { return utf8Len(_ch); }

  static FOG_INLINE bool isValidUtf8(uint8_t ch) 
  { return ch < 0x80 && ch >= 0xC0; }
  
  static FOG_INLINE uint8_t utf8Len(uint8_t ch)
  { return utf8LengthTable[ch]; }

  // [Members]

  //! @brief Unicode character value.
  uint8_t _ch;
};

// ============================================================================
// [Fog::Char16]
// ============================================================================

//! @brief 16-bit unicode character.
struct FOG_HIDDEN FOG_PACKED Char16
{
  // [Construction / Destruction]

  FOG_INLINE Char16() {}
  FOG_INLINE Char16(const Char16& c) : _ch(c._ch) {}

  FOG_INLINE explicit Char16(int8_t   c) : _ch((uint8_t)c) {}
  FOG_INLINE explicit Char16(uint8_t  c) : _ch(c) {}
  FOG_INLINE explicit Char16(int16_t  c) : _ch((uint16_t)c) {}
  FOG_INLINE explicit Char16(uint16_t c) : _ch(c) {}
  FOG_INLINE explicit Char16(int32_t  c) : _ch((uint16_t)(uint32_t)c) {}
  FOG_INLINE explicit Char16(uint32_t c) : _ch((uint16_t)c) {}
  FOG_INLINE explicit Char16(const Char8&  c) : _ch(c._ch) {}

  // [Character]

  //! @brief Return 16-bit character value.
  FOG_INLINE uint16_t ch() const { return _ch; }

  // [Char::operator=]

  FOG_INLINE Char16& operator=(const int8_t&   ch) { _ch = (uint8_t )ch          ; return *this; }
  FOG_INLINE Char16& operator=(const uint8_t&  ch) { _ch = ch                    ; return *this; }
  FOG_INLINE Char16& operator=(const int16_t&  ch) { _ch = (uint16_t)ch          ; return *this; }
  FOG_INLINE Char16& operator=(const uint16_t& ch) { _ch = ch                    ; return *this; }
  FOG_INLINE Char16& operator=(const int32_t&  ch) { _ch = (uint16_t)(uint32_t)ch; return *this; }
  FOG_INLINE Char16& operator=(const uint32_t& ch) { _ch = (uint16_t)ch          ; return *this; }
  FOG_INLINE Char16& operator=(const Char8&    ch) { _ch = ch._ch                ; return *this; }
  FOG_INLINE Char16& operator=(const Char16&   ch) { _ch = ch._ch                ; return *this; }

  // [Implicit Conversion]

  FOG_INLINE operator bool() const { return _ch != 0; }
  FOG_INLINE operator int16_t() const { return (int16_t)_ch; }
  FOG_INLINE operator uint16_t() const { return (uint16_t)_ch; }

  // [Ascii CTypes]

  FOG_INLINE bool isAsciiAlpha() const { return CType::isAsciiAlpha(_ch); }
  FOG_INLINE bool isAsciiAlnum() const { return CType::isAsciiAlnum(_ch); }
  FOG_INLINE bool isAsciiLower() const { return CType::isAsciiLower(_ch); }
  FOG_INLINE bool isAsciiUpper() const { return CType::isAsciiUpper(_ch); }
  FOG_INLINE bool isAsciiDigit() const { return CType::isAsciiDigit(_ch); }
  FOG_INLINE bool isAsciiXDigit()const { return CType::isAsciiXDigit(_ch); }
  FOG_INLINE bool isAsciiSpace() const { return CType::isAsciiSpace(_ch); }
  FOG_INLINE bool isAsciiBlank() const { return CType::isAsciiSpace(_ch); }
  FOG_INLINE bool isAsciiPunct() const { return CType::isAsciiPunct(_ch); }
  FOG_INLINE bool isAsciiGraph() const { return CType::isAsciiGraph(_ch); }
  FOG_INLINE bool isAsciiPrint() const { return CType::isAsciiPrint(_ch); }

  FOG_INLINE Char16 toAsciiLower() const { return Char16(CType::toAsciiLower(_ch)); }
  FOG_INLINE Char16 toAsciiUpper() const { return Char16(CType::toAsciiUpper(_ch)); }

  // [Unicode CTypes]

  FOG_INLINE bool isAlpha() const { return CType::isAlpha(_ch); }
  FOG_INLINE bool isLower() const { return CType::isLower(_ch); }
  FOG_INLINE bool isUpper() const { return CType::isUpper(_ch); }
  FOG_INLINE Char16 toLower() const { return Char16(CType::toLower(_ch)); }
  FOG_INLINE Char16 toUpper() const { return Char16(CType::toUpper(_ch)); }
  FOG_INLINE Char16 toAscii() const { return Char16(CType::toAscii(_ch)); }

  FOG_INLINE bool isSpace() const { return CType::isSpace(_ch); }
  FOG_INLINE bool isBlank() const { return CType::isBlank(_ch); }
  FOG_INLINE bool isDigit() const { return CType::isDigit(_ch); }
  FOG_INLINE bool isAlnum() const { return CType::isAlnum(_ch); }
  FOG_INLINE bool isXDigit() const { return CType::isXDigit(_ch); }
  FOG_INLINE bool isPunct() const { return CType::isPunct(_ch); }
  FOG_INLINE bool isGraph() const { return CType::isGraph(_ch); }
  FOG_INLINE bool isPrint() const { return CType::isPrint(_ch); }
  FOG_INLINE bool isCntrl() const { return CType::isCntrl(_ch); }

  // [Unicode Helpers]

  FOG_INLINE bool isLeadSurrogate() const { return isLeadSurrogate(_ch); }
  FOG_INLINE bool isTrailSurrogate() const { return isTrailSurrogate(_ch); }
  FOG_INLINE bool isPair() const { return isPair(_ch); }
  FOG_INLINE bool isValid() const { return isValid(_ch); }

  static FOG_INLINE bool isLeadSurrogate(uint16_t ch)
  { return (ch >= LeadSurrogateMin && ch <= LeadSurrogateMax); }

  static FOG_INLINE bool isTrailSurrogate(uint16_t ch)
  { return (ch >= TrailSurrogateMin && ch <= TrailSurrogateMax); }

  static FOG_INLINE bool isPair(uint16_t ch)
  { return (ch >= LeadSurrogateMin && ch <= TrailSurrogateMax); }

  static FOG_INLINE bool isValid(uint16_t ch)
  { return (ch < 0xFFFE); }

  FOG_INLINE Char16& bswap() { _ch = Memory::bswap16(_ch); return *this; }

  // [Combine]

  static FOG_INLINE Char16 combine(Char16 ch, Char16 comb)
  { return Char16((uint16_t)Core_CType_Unicode_combine(ch._ch, comb._ch)); }

  // [Members]

  //! @brief Unicode character value.
  uint16_t _ch;
};

// ============================================================================
// [Fog::Char32]
// ============================================================================

//! @brief 32-bit unicode character.
struct FOG_HIDDEN FOG_PACKED Char32
{
  // [Construction / Destruction]

  FOG_INLINE Char32() {}
  FOG_INLINE Char32(const Char32& c) : _ch(c._ch) {}

  FOG_INLINE explicit Char32(int8_t   c) : _ch((uint8_t)c) {}
  FOG_INLINE explicit Char32(uint8_t  c) : _ch(c) {}
  FOG_INLINE explicit Char32(int16_t  c) : _ch((uint16_t)c) {}
  FOG_INLINE explicit Char32(uint16_t c) : _ch(c) {}
  FOG_INLINE explicit Char32(int32_t  c) : _ch((uint32_t)c) {}
  FOG_INLINE explicit Char32(uint32_t c) : _ch(c) {}
  FOG_INLINE explicit Char32(wchar_t  c) : _ch((uint32_t)c) {}
  FOG_INLINE explicit Char32(const Char8&  c) : _ch(c._ch) {}
  FOG_INLINE explicit Char32(const Char16& c) : _ch(c._ch) {}

  // [Character]

  //! @brief Return unicode 32-bit character value.
  FOG_INLINE uint32_t ch() const { return _ch; }

  // [Char::operator=]

  FOG_INLINE Char32& operator=(const int8_t&   ch) { _ch = (uint8_t )ch; return *this; }
  FOG_INLINE Char32& operator=(const uint8_t&  ch) { _ch = ch          ; return *this; }
  FOG_INLINE Char32& operator=(const int16_t&  ch) { _ch = (uint16_t)ch; return *this; }
  FOG_INLINE Char32& operator=(const uint16_t& ch) { _ch = ch          ; return *this; }
  FOG_INLINE Char32& operator=(const int32_t&  ch) { _ch = (uint32_t)ch; return *this; }
  FOG_INLINE Char32& operator=(const uint32_t& ch) { _ch = ch          ; return *this; }
  FOG_INLINE Char32& operator=(const wchar_t&  ch) { _ch = (uint32_t)ch; return *this; }
  FOG_INLINE Char32& operator=(const Char8&    ch) { _ch = ch._ch      ; return *this; }
  FOG_INLINE Char32& operator=(const Char16&   ch) { _ch = ch._ch      ; return *this; }
  FOG_INLINE Char32& operator=(const Char32&   ch) { _ch = ch._ch      ; return *this; }

  // [Implicit Conversion]

  FOG_INLINE operator bool() const { return _ch != 0; }
  FOG_INLINE operator int32_t() const { return (int32_t)_ch; }
  FOG_INLINE operator uint32_t() const { return (uint32_t)_ch; }

  // [Ascii CTypes]

  FOG_INLINE bool isAsciiAlpha() const { return CType::isAsciiAlpha(_ch); }
  FOG_INLINE bool isAsciiAlnum() const { return CType::isAsciiAlnum(_ch); }
  FOG_INLINE bool isAsciiLower() const { return CType::isAsciiLower(_ch); }
  FOG_INLINE bool isAsciiUpper() const { return CType::isAsciiUpper(_ch); }
  FOG_INLINE bool isAsciiDigit() const { return CType::isAsciiDigit(_ch); }
  FOG_INLINE bool isAsciiXDigit()const { return CType::isAsciiXDigit(_ch); }
  FOG_INLINE bool isAsciiSpace() const { return CType::isAsciiSpace(_ch); }
  FOG_INLINE bool isAsciiBlank() const { return CType::isAsciiSpace(_ch); }
  FOG_INLINE bool isAsciiPunct() const { return CType::isAsciiPunct(_ch); }
  FOG_INLINE bool isAsciiGraph() const { return CType::isAsciiGraph(_ch); }
  FOG_INLINE bool isAsciiPrint() const { return CType::isAsciiPrint(_ch); }

  FOG_INLINE Char32 toAsciiLower() const { return Char32(CType::toAsciiLower(_ch)); }
  FOG_INLINE Char32 toAsciiUpper() const { return Char32(CType::toAsciiUpper(_ch)); }

  // [Unicode CTypes]

  FOG_INLINE bool isAlpha() const { return CType::isAlpha(_ch); }
  FOG_INLINE bool isLower() const { return CType::isLower(_ch); }
  FOG_INLINE bool isUpper() const { return CType::isUpper(_ch); }
  FOG_INLINE Char32 toLower() const { return Char32(CType::toLower(_ch)); }
  FOG_INLINE Char32 toUpper() const { return Char32(CType::toUpper(_ch)); }
  FOG_INLINE Char32 toAscii() const { return Char32(CType::toAscii(_ch)); }

  FOG_INLINE bool isSpace() const { return CType::isSpace(_ch); }
  FOG_INLINE bool isBlank() const { return CType::isBlank(_ch); }
  FOG_INLINE bool isDigit() const { return CType::isDigit(_ch); }
  FOG_INLINE bool isAlnum() const { return CType::isAlnum(_ch); }
  FOG_INLINE bool isXDigit() const { return CType::isXDigit(_ch); }
  FOG_INLINE bool isPunct() const { return CType::isPunct(_ch); }
  FOG_INLINE bool isGraph() const { return CType::isGraph(_ch); }
  FOG_INLINE bool isPrint() const { return CType::isPrint(_ch); }
  FOG_INLINE bool isCntrl() const { return CType::isCntrl(_ch); }

  // [Unicode Helpers]

  FOG_INLINE bool isValid() const { return isValid(_ch); }

  static FOG_INLINE bool isValid(uint32_t ch)
  { 
    return ((ch <= MaxCodePoint) && 
            (ch < LeadSurrogateMin || ch > LeadSurrogateMax) && 
            (ch != 0xFFFE && ch != 0xFFFF));
  }

  FOG_INLINE Char32& bswap() { _ch = Memory::bswap32(_ch); return *this; }

  // [Combine]

  static FOG_INLINE Char32 combine(Char32 ch, Char32 comb)
  { return Char32(Core_CType_Unicode_combine(ch._ch, comb._ch)); }

  // [Members]

  //! @brief Unicode character value.
  uint32_t _ch;
};

#include <Fog/Core/Unpack.h>

} // Fog namespace

// [Overloads]
#define __FOG_CHAR_COMBINE_OVERLOAD(TYPE) \
  static FOG_INLINE bool operator==(TYPE a, TYPE b) { return a.ch() == b.ch(); } \
  static FOG_INLINE bool operator!=(TYPE a, TYPE b) { return a.ch() != b.ch(); } \
  static FOG_INLINE bool operator<=(TYPE a, TYPE b) { return a.ch() <= b.ch(); } \
  static FOG_INLINE bool operator>=(TYPE a, TYPE b) { return a.ch() >= b.ch(); } \
  static FOG_INLINE bool operator< (TYPE a, TYPE b) { return a.ch() <  b.ch(); } \
  static FOG_INLINE bool operator> (TYPE a, TYPE b) { return a.ch() >  b.ch(); } \
  static FOG_INLINE TYPE operator+ (TYPE a, TYPE b) { return TYPE(a.ch() + b.ch()); } \
  static FOG_INLINE TYPE operator- (TYPE a, TYPE b) { return TYPE(a.ch() - b.ch()); }

#define __FOG_CHAR_OPERATOR_OVERLOAD(TYPEA, GETA, TYPEB, GETB) \
  static FOG_INLINE bool operator==(TYPEA a, TYPEB b) { return GETA == GETB; } \
  static FOG_INLINE bool operator==(TYPEB b, TYPEA a) { return GETB == GETA; } \
  static FOG_INLINE bool operator!=(TYPEA a, TYPEB b) { return GETA != GETB; } \
  static FOG_INLINE bool operator!=(TYPEB b, TYPEA a) { return GETB != GETA; } \
  static FOG_INLINE bool operator<=(TYPEA a, TYPEB b) { return GETA <= GETB; } \
  static FOG_INLINE bool operator<=(TYPEB b, TYPEA a) { return GETB <= GETA; } \
  static FOG_INLINE bool operator>=(TYPEA a, TYPEB b) { return GETA >= GETB; } \
  static FOG_INLINE bool operator>=(TYPEB b, TYPEA a) { return GETB >= GETA; } \
  static FOG_INLINE bool operator< (TYPEA a, TYPEB b) { return GETA <  GETB; } \
  static FOG_INLINE bool operator< (TYPEB b, TYPEA a) { return GETB <  GETA; } \
  static FOG_INLINE bool operator> (TYPEA a, TYPEB b) { return GETA >  GETB; } \
  static FOG_INLINE bool operator> (TYPEB b, TYPEA a) { return GETB >  GETA; } \
  static FOG_INLINE TYPEA operator+(TYPEA a, TYPEB b) { return TYPEA(GETA + GETB); } \
  static FOG_INLINE TYPEA operator+(TYPEB b, TYPEA a) { return TYPEA(GETB + GETA); } \
  static FOG_INLINE TYPEA operator-(TYPEA a, TYPEB b) { return TYPEA(GETA - GETB); } \
  static FOG_INLINE TYPEA operator-(TYPEB b, TYPEA a) { return TYPEA(GETB - GETA); }

__FOG_CHAR_COMBINE_OVERLOAD(Fog::Char8)
__FOG_CHAR_OPERATOR_OVERLOAD(Fog::Char8 , (a.ch()), int8_t      , (uint32_t)(b))
__FOG_CHAR_OPERATOR_OVERLOAD(Fog::Char8 , (a.ch()), uint8_t     , (uint32_t)(b))

__FOG_CHAR_COMBINE_OVERLOAD(Fog::Char16)
__FOG_CHAR_OPERATOR_OVERLOAD(Fog::Char16, (a.ch()), int8_t      , (uint8_t)(b))
__FOG_CHAR_OPERATOR_OVERLOAD(Fog::Char16, (a.ch()), uint8_t     , (b))
__FOG_CHAR_OPERATOR_OVERLOAD(Fog::Char16, (a.ch()), int16_t     , (uint16_t)(b))
__FOG_CHAR_OPERATOR_OVERLOAD(Fog::Char16, (a.ch()), uint16_t    , (b))
__FOG_CHAR_OPERATOR_OVERLOAD(Fog::Char16, (a.ch()), Fog::Char8 , (b.ch()))

__FOG_CHAR_COMBINE_OVERLOAD(Fog::Char32)
__FOG_CHAR_OPERATOR_OVERLOAD(Fog::Char32, (a.ch()), int8_t      , (uint8_t)(b))
__FOG_CHAR_OPERATOR_OVERLOAD(Fog::Char32, (a.ch()), uint8_t     , (b))
__FOG_CHAR_OPERATOR_OVERLOAD(Fog::Char32, (a.ch()), int16_t     , (uint16_t)(b))
__FOG_CHAR_OPERATOR_OVERLOAD(Fog::Char32, (a.ch()), uint16_t    , (b))
__FOG_CHAR_OPERATOR_OVERLOAD(Fog::Char32, (a.ch()), int32_t     , (uint32_t)(b))
__FOG_CHAR_OPERATOR_OVERLOAD(Fog::Char32, (a.ch()), uint32_t    , (b))
__FOG_CHAR_OPERATOR_OVERLOAD(Fog::Char32, (a.ch()), Fog::Char8 , (b.ch()))
__FOG_CHAR_OPERATOR_OVERLOAD(Fog::Char32, (a.ch()), Fog::Char16, (b.ch()))

#undef __FOG_CHAR_OPERATOR_OVERLOAD
#undef __FOG_CHAR_COMBINE_OVERLOAD

//! @}

// [Fog::TypeInfo<>]
FOG_DECLARE_TYPEINFO(Fog::Char8, Fog::PrimitiveType)
FOG_DECLARE_TYPEINFO(Fog::Char16, Fog::PrimitiveType)
FOG_DECLARE_TYPEINFO(Fog::Char32, Fog::PrimitiveType)

#endif // _FOG_CORE_CHAR_H
