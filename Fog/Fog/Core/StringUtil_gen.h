// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Generator]
// - __G_CHAR - Char type (Char8, Char16, Char32)
// - __G_FRIEND - Friend type (char, uint16_t, uint32_t)
// - __G_SIZE - Char size (1, 2, 4)
#if defined(__G_GENERATE)

#if __G_SIZE == 1
# define __G_CHAR Char8
# define __G_FRIEND char
#elif __G_SIZE == 2
# define __G_CHAR Char16
# define __G_FRIEND uint16_t
#else
# define __G_CHAR Char32
# define __G_FRIEND uint32_t
#endif

namespace Fog {
namespace StringUtil {

// ============================================================================
// [Fog::StringUtil::Raw]
// ============================================================================

FOG_API void copy(__G_CHAR* dst, const __G_CHAR* src, sysuint_t length);
FOG_API void move(__G_CHAR* dst, const __G_CHAR* src, sysuint_t length);
FOG_API void fill(__G_CHAR* dst, __G_CHAR ch, sysuint_t length);

FOG_API sysuint_t len(const __G_CHAR* str);
FOG_API sysuint_t nlen(const __G_CHAR* str, sysuint_t maxlen);

FOG_API bool eq(const __G_CHAR* a, const __G_CHAR* b, sysuint_t length, uint cs = CaseSensitive);

FOG_API sysuint_t countOf(const __G_CHAR* str, sysuint_t length, __G_CHAR ch, uint cs = CaseSensitive);
FOG_API sysuint_t indexOf(const __G_CHAR* str, sysuint_t length, __G_CHAR ch, uint cs = CaseSensitive);
FOG_API sysuint_t indexOf(const __G_CHAR* aStr, sysuint_t aLength, const __G_CHAR* bStr, sysuint_t bLength, uint cs = CaseSensitive);
FOG_API sysuint_t indexOfAny(const __G_CHAR* str, sysuint_t length, const __G_CHAR* ch, sysuint_t count, uint cs = CaseSensitive);

FOG_API sysuint_t lastIndexOf(const __G_CHAR* str, sysuint_t length, __G_CHAR ch, uint cs = CaseSensitive);
FOG_API sysuint_t lastIndexOfAny(const __G_CHAR* str, sysuint_t length, const __G_CHAR* ch, sysuint_t count, uint cs = CaseSensitive);

// ============================================================================
// [Fog::StringUtil::ato...]
// ============================================================================

FOG_API err_t atob(const __G_CHAR* str, sysuint_t length, bool* dst, sysuint_t* end = NULL, uint32_t* parserFlags = NULL);

FOG_API err_t atoi8(const __G_CHAR* str, sysuint_t length, int8_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL);
FOG_API err_t atou8(const __G_CHAR* str, sysuint_t length, uint8_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL);
FOG_API err_t atoi16(const __G_CHAR* str, sysuint_t length, int16_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL);
FOG_API err_t atou16(const __G_CHAR* str, sysuint_t length, uint16_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL);
FOG_API err_t atoi32(const __G_CHAR* str, sysuint_t length, int32_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL);
FOG_API err_t atou32(const __G_CHAR* str, sysuint_t length, uint32_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL);
FOG_API err_t atoi64(const __G_CHAR* str, sysuint_t length, int64_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL);
FOG_API err_t atou64(const __G_CHAR* str, sysuint_t length, uint64_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL);

FOG_API err_t atof(const __G_CHAR* str, sysuint_t length, float* dst, __G_CHAR decimalPoint = __G_CHAR('.'), sysuint_t* end = NULL, uint32_t* parserFlags = NULL);
FOG_API err_t atod(const __G_CHAR* str, sysuint_t length, double* dst, __G_CHAR decimalPoint = __G_CHAR('.'), sysuint_t* end = NULL, uint32_t* parserFlags = NULL);

// ============================================================================
// [Fog::StringUtil::Wrappers]
// ============================================================================

static FOG_INLINE void copy(__G_FRIEND* dst, const __G_FRIEND* src, sysuint_t length)
{ copy((__G_CHAR*)dst, (const __G_CHAR*)src, length); }

static FOG_INLINE void move(__G_FRIEND* dst, const __G_FRIEND* src, sysuint_t length)
{ move((__G_CHAR*)dst, (const __G_CHAR*)src, length); }

static FOG_INLINE void fill(__G_FRIEND* dst, __G_FRIEND ch, sysuint_t length)
{ fill((__G_CHAR*)dst, __G_CHAR(ch), length); }

static FOG_INLINE sysuint_t len(const __G_FRIEND* str)
{ return len((const __G_CHAR *)str); }

static FOG_INLINE sysuint_t nlen(const __G_FRIEND* str, sysuint_t maxlen)
{ return nlen((const __G_CHAR *)str, maxlen); }

static FOG_INLINE bool eq(const __G_FRIEND* a, const __G_FRIEND* b, sysuint_t length, uint cs = CaseSensitive)
{ return eq((const __G_CHAR *)a, (const __G_CHAR *)b, length, cs); }

static FOG_INLINE sysuint_t countOf(const __G_FRIEND* str, sysuint_t length, __G_FRIEND ch, uint cs = CaseSensitive)
{ return countOf((const __G_CHAR *)str, length, __G_CHAR(ch), cs); }

static FOG_INLINE sysuint_t indexOf(const __G_FRIEND* str, sysuint_t length, __G_FRIEND ch, uint cs = CaseSensitive)
{ return indexOf((const __G_CHAR *)str, length, __G_CHAR(ch), cs); }

static FOG_INLINE sysuint_t indexOfAny(const __G_FRIEND* str, sysuint_t length, const __G_FRIEND* ch, sysuint_t count, uint cs = CaseSensitive)
{ return indexOfAny((const __G_CHAR *)str, length, (const __G_CHAR*)ch, count, cs); }

static FOG_INLINE sysuint_t lastIndexOf(const __G_FRIEND* str, sysuint_t length, __G_FRIEND ch, uint cs = CaseSensitive)
{ return lastIndexOf((const __G_CHAR *)str, length, __G_CHAR(ch), cs); }

static FOG_INLINE sysuint_t lastIndexOfAny(const __G_FRIEND* str, sysuint_t length, const __G_FRIEND* ch, sysuint_t count, uint cs = CaseSensitive)
{ return lastIndexOfAny((const __G_CHAR *)str, length, (const __G_CHAR*)ch, count, cs); }

static FOG_INLINE err_t atob(const __G_FRIEND* str, sysuint_t length, bool* dst, sysuint_t* end = NULL, uint32_t* parserFlags = NULL)
{ return atob((const __G_CHAR *)str, length, dst, end, parserFlags); }

static FOG_INLINE err_t atoi8(const __G_FRIEND* str, sysuint_t length, int8_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL)
{ return atoi8((const __G_CHAR *)str, length, dst, base, end, parserFlags); }

static FOG_INLINE err_t atou8(const __G_FRIEND* str, sysuint_t length, uint8_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL)
{ return atou8((const __G_CHAR *)str, length, dst, base, end, parserFlags); }

static FOG_INLINE err_t atoi16(const __G_FRIEND* str, sysuint_t length, int16_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL)
{ return atoi16((const __G_CHAR *)str, length, dst, base, end, parserFlags); }

static FOG_INLINE err_t atou16(const __G_FRIEND* str, sysuint_t length, uint16_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL)
{ return atou16((const __G_CHAR *)str, length, dst, base, end, parserFlags); }

static FOG_INLINE err_t atoi32(const __G_FRIEND* str, sysuint_t length, int32_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL)
{ return atoi32((const __G_CHAR *)str, length, dst, base, end, parserFlags); }

static FOG_INLINE err_t atou32(const __G_FRIEND* str, sysuint_t length, uint32_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL)
{ return atou32((const __G_CHAR *)str, length, dst, base, end, parserFlags); }

static FOG_INLINE err_t atoi64(const __G_FRIEND* str, sysuint_t length, int64_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL)
{ return atoi64((const __G_CHAR *)str, length, dst, base, end, parserFlags); }

static FOG_INLINE err_t atou64(const __G_FRIEND* str, sysuint_t length, uint64_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL)
{ return atou64((const __G_CHAR *)str, length, dst, base, end, parserFlags); }

static FOG_INLINE err_t atof(const __G_FRIEND* str, sysuint_t length, float* dst, __G_CHAR decimalPoint = __G_CHAR('.'), sysuint_t* end = NULL, uint32_t* parserFlags = NULL)
{ return atof((const __G_CHAR *)str, length, dst, decimalPoint, end, parserFlags); }

static FOG_INLINE err_t atod(const __G_FRIEND* str, sysuint_t length, double* dst, __G_CHAR decimalPoint = __G_CHAR('.'), sysuint_t* end = NULL, uint32_t* parserFlags = NULL)
{ return atod((const __G_CHAR *)str, length, dst, decimalPoint, end, parserFlags); }

} // StringUtil namespace
} // Fog namespace

#undef __G_FRIEND
#undef __G_CHAR

// [Generator]
#endif // __G_GENERATE
