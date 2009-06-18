// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_STRINGUTIL_H
#define _FOG_CORE_STRINGUTIL_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Core/Char.h>

//! @addtogroup Fog_Core
//! @{

namespace Fog {
namespace StringUtil {

// ============================================================================
// [Fog::StringUtil::Unicode Tools]
// ============================================================================

FOG_API sysuint_t utf8Validate(const Char8* str, sysuint_t len);
FOG_API sysuint_t utf16Validate(const Char16* str, sysuint_t len);
FOG_API sysuint_t utf32Validate(Char32* str, sysuint_t len);

FOG_API sysuint_t utf8ToUtf16(Char16* dst, const Char8* src, sysuint_t srcLen);
FOG_API sysuint_t utf8ToUtf16Len(const Char8* str, sysuint_t len);

FOG_API sysuint_t utf8ToUtf32(Char32* dst, const Char8* src, sysuint_t srcLen);
FOG_API sysuint_t utf8ToUtf32Len(const Char8* str, sysuint_t len);

FOG_API sysuint_t utf16ToUtf8(Char8* dst, const Char16* src, sysuint_t srcLen);
FOG_API sysuint_t utf16ToUtf8Len(const Char16* str, sysuint_t len);

FOG_API sysuint_t utf16ToUtf32(Char32* dst, const Char16* src, sysuint_t srcLen);
FOG_API sysuint_t utf16ToUtf32Len(const Char16* str, sysuint_t len);

FOG_API sysuint_t utf32ToUtf8(Char8* dst, const Char32* src, sysuint_t srcLen);
FOG_API sysuint_t utf32ToUtf8Len(const Char32* src, sysuint_t len);

FOG_API sysuint_t utf32ToUtf16(Char16* dst, const Char32* src, sysuint_t srcLen);
FOG_API sysuint_t utf32ToUtf16Len(const Char32* src, sysuint_t srcLen);

FOG_API bool utf16ToLatin(Char8* dst, const Char16* src, sysuint_t length);
FOG_API bool utf32ToLatin(Char8* dst, const Char32* src, sysuint_t length);

// ============================================================================
// [Fog::StringUtil::ParserFlags]
// ============================================================================

enum ParserFlags
{
  //! @brief Some spaces was parsed and skipped in input.
  ParsedSpaces = (1 << 1),
  //! @brief Sign was parsed.
  ParsedSign = (1 << 2),
  //! @brief @c "0" octal prefix was parsed.
  ParsedOctalPrefix = (1 << 3),
  //! @brief @c "0x" or @c "0X" hexadecimal prefix was parsed.
  ParsedHexPrefix = (1 << 4),
  //! @brief Decimal point was parsed.
  ParsedDecimalPoint = (1 << 5),
  //! @brief Exponent (@c "E" or @c "e") was parsed.
  ParsedExponent = (1 << 6)
};

// ============================================================================
// [Fog::StringUtil::NTOA]
// ============================================================================

struct NTOAOut
{
  //! @brief Output (pointer to non null terminated string in @c buf).
  uint8_t* result;
  //! @brief Output length.
  uint32_t length;
  //! @brief Decimal point position (for dtoa).
  uint32_t decpt;
  //! @brief output flags.
  uint32_t negative;
  //! @brief Output buffer that must fit the largest possible result.
  uint8_t buffer[256];
};

FOG_API void itoa(int64_t n, int base, bool uppercase, NTOAOut* out);
FOG_API void utoa(uint64_t n, int base, bool uppercase, NTOAOut* out);
FOG_API void dtoa(double d, int mode, int ndigits, NTOAOut* out);

// ============================================================================
// [Fog::StringUtil::Raw]
// ============================================================================

FOG_API void copy(Char16* dst, const Char8* src, sysuint_t length);
FOG_API void copy(Char32* dst, const Char8* src, sysuint_t length);

FOG_API bool eq(const Char16* a, const Char8* b, sysuint_t length, uint cs = CaseSensitive);
FOG_API bool eq(const Char32* a, const Char8* b, sysuint_t length, uint cs = CaseSensitive);

// ============================================================================
// [Fog::StringUtil::Wrappers]
// ============================================================================

static FOG_INLINE void copy(uint16_t* dst, const char* src, sysuint_t length)
{ return copy((Char16*)dst, (const Char8*)src, length); }

static FOG_INLINE void copy(uint32_t* dst, const char* src, sysuint_t length)
{ return copy((Char32*)dst, (const Char8*)src, length); }

} // StringUtil namespace
} // Fog namespace

// ============================================================================
// [Fog::StringUtil::Generator]
// ============================================================================

#define __G_GENERATE

#define __G_SIZE 1
#include <Fog/Core/StringUtil_gen.h>
#undef __G_SIZE

#define __G_SIZE 2
#include <Fog/Core/StringUtil_gen.h>
#undef __G_SIZE

#define __G_SIZE 4
#include <Fog/Core/StringUtil_gen.h>
#undef __G_SIZE

#undef __G_GENERATE

//! @}

// [Guard]
#endif // _FOG_CORE_STRINGUTIL_H
