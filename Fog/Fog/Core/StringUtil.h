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

#include <string.h>

//! @addtogroup Fog_Core
//! @{

namespace Fog {
namespace StringUtil {

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
// [Fog::StringUtil::Unicode Tools]
// ============================================================================

FOG_API err_t validateUtf8(const char* str, sysuint_t len, sysuint_t* invalidPos = NULL);
FOG_API err_t validateUtf16(const Char* str, sysuint_t len, sysuint_t* invalidPos = NULL);

FOG_API err_t getNumUtf8Chars(const char* str, sysuint_t len, sysuint_t* charsCount);
FOG_API err_t getNumUtf16Chars(const Char* str, sysuint_t len, sysuint_t* charsCount);

FOG_API bool unicodeToLatin1(char* dst, const Char* src, sysuint_t length);

// ============================================================================
// [Fog::StringUtil::Mem]
// ============================================================================

FOG_INLINE void copy(char* dst, const char* src, sysuint_t length) { ::memcpy(dst, src, length); }
FOG_INLINE void move(char* dst, const char* src, sysuint_t length) { ::memmove(dst, src, length); }
FOG_INLINE void fill(char* dst, char ch, sysuint_t length) { ::memset(dst, (uint8_t)ch, length); }

FOG_INLINE sysuint_t len(const char* s) { return ::strlen(s); }
// TODO: strnlen is not defined by mingw.
// #if FOG_HAVE_STRNLEN
FOG_INLINE sysuint_t nlen(const char* s, sysuint_t maxlen) { return ::strnlen(s, maxlen); }
// #else
// #endif

FOG_API void copy(Char* dst, const Char* src, sysuint_t length);
FOG_API void copy(Char* dst, const char* src, sysuint_t length);
FOG_API void move(Char* dst, const Char* src, sysuint_t length);
FOG_API void fill(Char* dst, Char ch, sysuint_t length);

FOG_API sysuint_t len(const Char* str);
FOG_API sysuint_t nlen(const Char* str, sysuint_t maxlen);

FOG_API sysuint_t len(const uint32_t* str);
FOG_API sysuint_t nlen(const uint32_t* str, sysuint_t maxlen);

FOG_API bool eq(const char* a, const char* b, sysuint_t length, uint cs = CaseSensitive);
FOG_API bool eq(const Char* a, const Char* b, sysuint_t length, uint cs = CaseSensitive);
FOG_API bool eq(const Char* a, const char* b, sysuint_t length, uint cs = CaseSensitive);

FOG_API sysuint_t countOf(const char* str, sysuint_t length, char ch, uint cs = CaseSensitive);
FOG_API sysuint_t countOf(const Char* str, sysuint_t length, Char ch, uint cs = CaseSensitive);

FOG_API sysuint_t indexOf(const char* str, sysuint_t length, char ch, uint cs = CaseSensitive);
FOG_API sysuint_t indexOf(const Char* str, sysuint_t length, Char ch, uint cs = CaseSensitive);

FOG_API sysuint_t indexOf(const char* aStr, sysuint_t aLength, const char* bStr, sysuint_t bLength, uint cs = CaseSensitive);
FOG_API sysuint_t indexOf(const Char* aStr, sysuint_t aLength, const Char* bStr, sysuint_t bLength, uint cs = CaseSensitive);

FOG_API sysuint_t indexOfAny(const char* str, sysuint_t length, const char* ch, sysuint_t count, uint cs = CaseSensitive);
FOG_API sysuint_t indexOfAny(const Char* str, sysuint_t length, const Char* ch, sysuint_t count, uint cs = CaseSensitive);

FOG_API sysuint_t lastIndexOf(const char* str, sysuint_t length, char ch, uint cs = CaseSensitive);
FOG_API sysuint_t lastIndexOf(const Char* str, sysuint_t length, Char ch, uint cs = CaseSensitive);

FOG_API sysuint_t lastIndexOfAny(const char* str, sysuint_t length, const char* ch, sysuint_t count, uint cs = CaseSensitive);
FOG_API sysuint_t lastIndexOfAny(const Char* str, sysuint_t length, const Char* ch, sysuint_t count, uint cs = CaseSensitive);

// ============================================================================
// [Fog::StringUtil::NTOA]
// ============================================================================

struct NTOAOut
{
  //! @brief Output (pointer to non null terminated string in @c buf).
  char* result;
  //! @brief Output length.
  uint32_t length;
  //! @brief Decimal point position (for dtoa).
  uint32_t decpt;
  //! @brief output flags.
  uint32_t negative;
  //! @brief Output buffer that must fit the largest possible result.
  char buffer[256];
};

FOG_API void itoa(int64_t n, int base, bool uppercase, NTOAOut* out);
FOG_API void utoa(uint64_t n, int base, bool uppercase, NTOAOut* out);
FOG_API void dtoa(double d, int mode, int ndigits, NTOAOut* out);

// ============================================================================
// [Fog::StringUtil::Ato[b|i|u|f|d]]
// ============================================================================

FOG_API err_t atob(const char* str, sysuint_t length, bool* dst, sysuint_t* end = NULL, uint32_t* parserFlags = NULL);

FOG_API err_t atoi8(const char* str, sysuint_t length, int8_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL);
FOG_API err_t atou8(const char* str, sysuint_t length, uint8_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL);
FOG_API err_t atoi16(const char* str, sysuint_t length, int16_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL);
FOG_API err_t atou16(const char* str, sysuint_t length, uint16_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL);
FOG_API err_t atoi32(const char* str, sysuint_t length, int32_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL);
FOG_API err_t atou32(const char* str, sysuint_t length, uint32_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL);
FOG_API err_t atoi64(const char* str, sysuint_t length, int64_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL);
FOG_API err_t atou64(const char* str, sysuint_t length, uint64_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL);

FOG_API err_t atof(const char* str, sysuint_t length, float* dst, char decimalPoint = '.', sysuint_t* end = NULL, uint32_t* parserFlags = NULL);
FOG_API err_t atod(const char* str, sysuint_t length, double* dst, char decimalPoint = '.', sysuint_t* end = NULL, uint32_t* parserFlags = NULL);

FOG_API err_t atob(const Char* str, sysuint_t length, bool* dst, sysuint_t* end = NULL, uint32_t* parserFlags = NULL);

FOG_API err_t atoi8(const Char* str, sysuint_t length, int8_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL);
FOG_API err_t atou8(const Char* str, sysuint_t length, uint8_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL);
FOG_API err_t atoi16(const Char* str, sysuint_t length, int16_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL);
FOG_API err_t atou16(const Char* str, sysuint_t length, uint16_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL);
FOG_API err_t atoi32(const Char* str, sysuint_t length, int32_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL);
FOG_API err_t atou32(const Char* str, sysuint_t length, uint32_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL);
FOG_API err_t atoi64(const Char* str, sysuint_t length, int64_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL);
FOG_API err_t atou64(const Char* str, sysuint_t length, uint64_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL);

FOG_API err_t atof(const Char* str, sysuint_t length, float* dst, Char decimalPoint = Char('.'), sysuint_t* end = NULL, uint32_t* parserFlags = NULL);
FOG_API err_t atod(const Char* str, sysuint_t length, double* dst, Char decimalPoint = Char('.'), sysuint_t* end = NULL, uint32_t* parserFlags = NULL);

} // StringUtil namespace
} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_CORE_STRINGUTIL_H
