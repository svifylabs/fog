// [Fog/Graphics library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_RASTER_C_H
#define _FOG_GRAPHICS_RASTER_C_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/Rgba.h>
#include <Fog/Graphics/Raster.h>

//! @addtogroup Fog_Graphics_Raster
//! @{

// This file is included from all Fog/Graphics/Raster/Raster_.h includes and 
// .cpp files, so in future it may contain generic code for these modules.

namespace Fog {
namespace Raster {

// ============================================================================
// [Fog::Raster - Masks]
// ============================================================================

// [Byte Position]

enum RGB32_BytePos
{
#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
  RGB32_RByte = 2,
  RGB32_GByte = 1,
  RGB32_BByte = 0,
  RGB32_AByte = 3,
#else // FOG_BYTE_ORDER == FOG_BIG_ENDIAN
  RGB32_RByte = 1,
  RGB32_GByte = 2,
  RGB32_BByte = 3,
  RGB32_AByte = 0,
#endif // FOG_BYTE_ORDER
};

enum RGB24_BytePos
{
#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
  RGB24_RByte = 2,
  RGB24_GByte = 1,
  RGB24_BByte = 0,
#else // FOG_BYTE_ORDER == FOG_BIG_ENDIAN
  RGB24_RByte = 0,
  RGB24_GByte = 1,
  RGB24_BByte = 2,
#endif // FOG_BYTE_ORDER
};

// [Shift]

enum RGB32_Shift
{
  RGB32_AShift = 24U,
  RGB32_RShift = 16U,
  RGB32_GShift = 8U,
  RGB32_BShift = 0U
};

// [Mask]

enum RGB32_Mask
{
  RGB32_RMask = 0x000000FFU << RGB32_RShift,
  RGB32_GMask = 0x000000FFU << RGB32_GShift,
  RGB32_BMask = 0x000000FFU << RGB32_BShift,
  RGB32_AMask = 0x000000FFU << RGB32_AShift
};

// ============================================================================
// [Fog::Raster - Copy]
// ============================================================================

static FOG_INLINE void copy1(uint8_t* dest, const uint8_t* src)
{
  *dest = *src;
}

static FOG_INLINE void copy2(uint8_t* dest, const uint8_t* src)
{
  ((uint16_t *)dest)[0] = ((uint16_t *)src)[0];
}

static FOG_INLINE void copy3(uint8_t* dest, const uint8_t* src)
{
  ((uint16_t *)dest)[0] = ((uint16_t *)src)[0];
  ((uint8_t  *)dest)[2] = ((uint8_t  *)src)[2];
}

static FOG_INLINE void copy4(uint8_t* dest, const uint8_t* src)
{
  ((uint32_t *)dest)[0] = ((uint32_t *)src)[0];
}

static FOG_INLINE void copy8(uint8_t* dest, const uint8_t* src)
{
#if FOG_ARCH_BITS == 64
  ((uint64_t *)dest)[0] = ((uint64_t *)src)[0];
#else
  ((uint32_t *)dest)[0] = ((uint32_t *)src)[0];
  ((uint32_t *)dest)[1] = ((uint32_t *)src)[1];
#endif
}

static FOG_INLINE void copy12(uint8_t* dest, const uint8_t* src)
{
#if FOG_ARCH_BITS == 64
  ((uint64_t *)dest)[0] = ((uint64_t *)src)[0];
  ((uint32_t *)dest)[2] = ((uint32_t *)src)[2];
#else
  ((uint32_t *)dest)[0] = ((uint32_t *)src)[0];
  ((uint32_t *)dest)[1] = ((uint32_t *)src)[1];
  ((uint32_t *)dest)[2] = ((uint32_t *)src)[2];
#endif
}

static FOG_INLINE void copy16(uint8_t* dest, const uint8_t* src)
{
#if defined(FOG_HARDCODE_SSE2)
  _mm_storeu_si128(&((__m128i *)dest)[0], _mm_loadu_si128(&((__m128i *)src)[0]));
#elif FOG_ARCH_BITS == 64
  ((uint64_t *)dest)[0] = ((uint64_t *)src)[0];
  ((uint64_t *)dest)[1] = ((uint64_t *)src)[1];
#else
  ((uint32_t *)dest)[0] = ((uint32_t *)src)[0];
  ((uint32_t *)dest)[1] = ((uint32_t *)src)[1];
  ((uint32_t *)dest)[2] = ((uint32_t *)src)[2];
  ((uint32_t *)dest)[3] = ((uint32_t *)src)[3];
#endif
}

static FOG_INLINE void copy24(uint8_t* dest, const uint8_t* src)
{
  copy16(dest, src);
  copy8(dest + 16, src + 16);
}

static FOG_INLINE void copy32(uint8_t* dest, const uint8_t* src)
{
  copy16(dest, src);
  copy16(dest + 16, src + 16);
}

// ============================================================================
// [Fog::Raster - Set]
// ============================================================================

static FOG_INLINE void set4(uint8_t* dest, uint32_t pattern)
{
  ((uint32_t *)dest)[0] = pattern;
}

static FOG_INLINE void set8(uint8_t* dest, uint32_t pattern)
{
  ((uint32_t *)dest)[0] = pattern;
  ((uint32_t *)dest)[1] = pattern;
}

static FOG_INLINE void set12(uint8_t* dest, uint32_t pattern)
{
  ((uint32_t *)dest)[0] = pattern;
  ((uint32_t *)dest)[1] = pattern;
  ((uint32_t *)dest)[2] = pattern;
}

static FOG_INLINE void set16(uint8_t* dest, uint32_t pattern)
{
  ((uint32_t *)dest)[0] = pattern;
  ((uint32_t *)dest)[1] = pattern;
  ((uint32_t *)dest)[2] = pattern;
  ((uint32_t *)dest)[3] = pattern;
}

static FOG_INLINE void set24(uint8_t* dest, uint32_t pattern)
{
  ((uint32_t *)dest)[0] = pattern;
  ((uint32_t *)dest)[1] = pattern;
  ((uint32_t *)dest)[2] = pattern;
  ((uint32_t *)dest)[3] = pattern;
  ((uint32_t *)dest)[4] = pattern;
  ((uint32_t *)dest)[5] = pattern;
}

static FOG_INLINE void set32(uint8_t* dest, uint32_t pattern)
{
  ((uint32_t *)dest)[0] = pattern;
  ((uint32_t *)dest)[1] = pattern;
  ((uint32_t *)dest)[2] = pattern;
  ((uint32_t *)dest)[3] = pattern;
  ((uint32_t *)dest)[4] = pattern;
  ((uint32_t *)dest)[5] = pattern;
  ((uint32_t *)dest)[6] = pattern;
  ((uint32_t *)dest)[7] = pattern;
}

static FOG_INLINE void set8(uint8_t* dest, uint64_t pattern)
{
  ((uint64_t *)dest)[0] = pattern;
}

static FOG_INLINE void set12(uint8_t* dest, uint64_t pattern)
{
  ((uint64_t *)dest)[0] = pattern;
  ((uint32_t *)dest)[2] = (uint32_t)pattern;
}

static FOG_INLINE void set16(uint8_t* dest, uint64_t pattern)
{
  ((uint64_t *)dest)[0] = pattern;
  ((uint64_t *)dest)[1] = pattern;
}

static FOG_INLINE void set24(uint8_t* dest, uint64_t pattern)
{
  ((uint64_t *)dest)[0] = pattern;
  ((uint64_t *)dest)[1] = pattern;
  ((uint64_t *)dest)[2] = pattern;
}

static FOG_INLINE void set32(uint8_t* dest, uint64_t pattern)
{
  ((uint64_t *)dest)[0] = pattern;
  ((uint64_t *)dest)[1] = pattern;
  ((uint64_t *)dest)[2] = pattern;
  ((uint64_t *)dest)[3] = pattern;
}

// ============================================================================
// [Fog::Raster - Mul]
// ============================================================================

template<typename T>
static FOG_INLINE T mul1(T i)
{
  return i;
}

template<typename T>
static FOG_INLINE T mul2(T i)
{
  return i << 1;
}

template<typename T>
static FOG_INLINE T mul3(T i)
{
  return (i << 1) + i;
}

template<typename T>
static FOG_INLINE T mul4(T i)
{
  return i << 2;
}

template<typename T, int Mul>
static FOG_INLINE T mul(T i) { return i * Mul; }

// ============================================================================
// [Fog::Raster - Div255/256]
// ============================================================================

//! @brief Accurate division of 255. The result is equal to <code>(val / 255.0)</code>.
//!
//! @note This template is used for accure alpha blends.
template<typename T>
static FOG_INLINE T div255(T i)
{
    return (T)( (((uint32_t)i << 8U) + ((uint32_t)i + 256U)) >> 16U );
}

//! @brief Accurate division of 256. The result is equal to <code>(val / 256.0)</code>.
template<typename T>
static FOG_INLINE T div256(T i)
{
    return (T)(i >> 8U);
}

// ============================================================================
// [Fog::Raster - Unpack]
// ============================================================================

static FOG_INLINE uint32_t unpackU32ToU32(uint32_t i)
{
  return i;
}

static FOG_INLINE uint64_t unpackU32ToU64(uint32_t i)
{
  return ((uint64_t)i) | ((uint64_t)i << 32); 
}

static FOG_INLINE sysuint_t unpackU32ToSysUInt(uint32_t i)
{
#if FOG_ARCH_BITS == 64
  return unpackU32ToU64(i);
#else
  return unpackU32ToU32(i);
#endif // FOG_ARCH_BITS
}

static FOG_INLINE uint32_t unpackU16ToU32(uint16_t i)
{
  return ((uint32_t)i) | ((uint32_t)i << 16);
}

static FOG_INLINE uint64_t unpackU16ToU64(uint16_t i)
{
  return unpackU32ToU64( ((uint32_t)i) | ((uint32_t)i << 16) );
}

static FOG_INLINE sysuint_t unpackU16ToSysUInt(uint16_t i)
{
#if FOG_ARCH_BITS == 64
  return unpackU16ToU64(i);
#else
  return unpackU16ToU32(i);
#endif // FOG_ARCH_BITS
}

static FOG_INLINE uint32_t unpackU8ToU32(uint8_t i)
{
  return unpackU16ToU32( ((uint16_t)i) | ((uint16_t)i << 8) );
}

static FOG_INLINE uint64_t unpackU8ToU64(uint8_t i)
{
  return unpackU16ToU64( ((uint16_t)i) | ((uint16_t)i << 8) );
}

static FOG_INLINE sysuint_t unpackU8ToSysUInt(uint8_t i)
{
#if FOG_ARCH_BITS == 64
  return unpackU8ToU64(i);
#else
  return unpackU8ToU32(i);
#endif // FOG_ARCH_BITS
}

// ============================================================================
// [Fog::Raster - Alpha]
// ============================================================================

static FOG_INLINE bool isAlpha0xFF(uint32_t c0) { return (c0 >= 0xFF000000); }
static FOG_INLINE bool isAlpha0x00(uint32_t c0) { return (c0 <= 0x00FFFFFF); }

// ============================================================================
// [Fog::Raster - Byte1x2 - 32 Bit Integer SIMD Library]
// ============================================================================

// These function allows to do SIMD with classic 32 bit integers.
//
// Terminology:
// - byte1x2 - unsigned 32 bit integer that contains two bytes at mask
//             _1x2MASK that is 0x00FF00FF.
// - byte2x2 - two byte1x2 values
// - u       - one byte at range 0 to 255
//
// Pixels encoded in byte2x2:
// - x0/y0 (0x00RR00BB)
// - x1/y1 (0x00AA00GG);

typedef uint32_t byte1x2;

enum {
  _1x1MASK          = 0x000000FFU,
  _1x2MASK          = 0x00FF00FFU,
  _1x1HALF          = 0x00000080U,
  _1x2HALF          = 0x00800080U,
  _1x1HALF_PLUS_ONE = 0x00000100U,
  _1x2MASK_PLUS_ONE = 0x10000100U
};

static FOG_INLINE byte1x2 byte1x2_from_u(uint32_t b)
{
  return b | (b << 16);
}

static FOG_INLINE uint32_t byte1x2_lo(byte1x2 x0)
{
  return x0 & 0xFF;
}

static FOG_INLINE uint32_t byte1x2_hi(byte1x2 x0)
{
  return (x0 >> 16);
}

//! @brief Pack x0 and x1 into single DWORD.
static FOG_INLINE uint32_t byte2x2_pack_0213(
  byte1x2 x0, byte1x2 x1)
{
  return x0 | (x1 << 8);
}

//! @brief Unpack 0st and 2nd BYTE of single DWORD into one byte1x2 value.
static FOG_INLINE void byte2x2_unpack_02(
  byte1x2& x0, uint32_t src_c)
{
  x0 = src_c;
  x0 &= _1x2MASK;
}

//! @brief Unpack 1st and 3rd BYTE of single DWORD into one byte1x2 value.
static FOG_INLINE void byte2x2_unpack_13(
  byte1x2& x0, uint32_t src_c)
{
  x0 = src_c >> 8;
  x0 &= _1x2MASK;
}

//! @brief Unpack all bytes in single DWORD into two byte1x2 values.
static FOG_INLINE void byte2x2_unpack_0213(
  byte1x2& x0, byte1x2& x1, uint32_t src_c)
{
  x0 = src_c;
  x1 = src_c >> 8;
  x0 &= _1x2MASK;
  x1 &= _1x2MASK;
}

//! @brief Unpack all bytes in single DWORD into two byte1x2 values.
static FOG_INLINE void byte2x2_unpack_021X(
  byte1x2& x0, byte1x2& x1, uint32_t src_c)
{
  x0 = src_c;
  x1 = src_c >> 8;
  x0 &= _1x2MASK;
  x1 &= _1x1MASK;
}

// x0 = y0hi, y0hi
static FOG_INLINE void byte1x2_expand_hi(
  byte1x2& x0, byte1x2 y0)
{
  x0 = (y0 >> 16);
  x0 |= (y0 & 0x00FF0000);
}

// x0 = y0hi, y0hi
// x1 = y0hi, y0hi
static FOG_INLINE void byte2x2_expand_hi(
  byte1x2& x0, byte1x2& x1, byte1x2 y0)
{
  byte1x2_expand_hi(x0, y0);
  x1 = x0;
}

// x0 = min(x0, 255)
static FOG_INLINE void byte1x2_sat(
  byte1x2& x0)
{
  x0 |= _1x2MASK_PLUS_ONE - ((x0 >> 8) & _1x2MASK);
  x0 &= 0x00FF00FF;
}

// x0 = min(x0, 255)
// x1 = min(x1, 255)
static FOG_INLINE void byte2x2_sat(
  byte1x2& x0, byte1x2& x1)
{
  x0 |= _1x2MASK_PLUS_ONE - ((x0 >> 8) & _1x2MASK);
  x1 |= _1x2MASK_PLUS_ONE - ((x1 >> 8) & _1x2MASK);
  x0 &= 0x00FF00FF;
  x1 &= 0x00FF00FF;
}

// x0 += a
static FOG_INLINE void byte1x2_add_u(
  byte1x2& x0, uint32_t a)
{
  x0 += a | (a << 16);
}

// x0 += a
// x1 += a
static FOG_INLINE void byte2x2_add_u(
  byte1x2& x0, byte1x2& x1, uint32_t a)
{
  a |= a << 16;
  x0 += a;
  x1 += a;
}

// x0 = max(x0 - a, 0)
static FOG_INLINE void byte1x2_adds_u(
  byte1x2& x0, uint32_t a)
{
  x0 += a | (a << 16);
  byte1x2_sat(x0);
}

// x0 = max(x0 - a, 0)
// x0 = max(x0 - a, 0)
static FOG_INLINE void byte2x2_adds_u(
  byte1x2& x0, byte1x2& x1, uint32_t a)
{
  a |= a << 16;
  x0 += a;
  x1 += a;
  byte2x2_sat(x0, x1);
}

// x0 -= a
static FOG_INLINE void byte1x2_sub_u(
  byte1x2& x0, uint32_t a)
{
  a |= a << 16;
  x0 -= (a | a << 16);
}

// x0 -= a
// x1 -= a
static FOG_INLINE void byte2x2_sub_u(
  byte1x2& x0, byte1x2& x1, uint32_t a)
{
  a |= a << 16;
  x0 -= a;
  x1 -= a;
}

// x0 = max(x0 - a, 0)
static FOG_INLINE void byte1x2_subs_u(
  byte1x2& x0, uint32_t a)
{
  x0 ^= _1x2MASK;
  x0 += a | (a << 16);
  byte1x2_sat(x0);
  x0 ^= _1x2MASK;
}

// x0 = max(x0 - a, 0)
// x1 = max(x1 - a, 0)
static FOG_INLINE void byte2x2_subs_u(
  byte1x2& x0, byte1x2& x1, uint32_t a)
{
  a |= a << 16;
  x0 ^= _1x2MASK;
  x1 ^= _1x2MASK;
  x0 += a;
  x1 += a;
  byte2x2_sat(x0, x1);
  x0 ^= _1x2MASK;
  x1 ^= _1x2MASK;
}

// x0 += y0
static FOG_INLINE void byte1x2_add_byte1x2(
  byte1x2& x0, byte1x2 y0)
{
  x0 += y0;
}

// x0 += y0
// x1 += y1
static FOG_INLINE void byte2x2_add_byte2x2(
  byte1x2& x0, byte1x2& x1,
  byte1x2 y0, byte1x2 y1)
{
  x0 += y0;
  x1 += y1;
}

// x0 = min(x0 + y0, 255)
static FOG_INLINE void byte1x2_adds_byte1x2(
  byte1x2& x0, byte1x2 y0)
{
  x0 += y0;
  byte1x2_sat(x0);
}

// x0 = min(x0 + y0, 255)
// x1 = min(x1 + y1, 255)
static FOG_INLINE void byte2x2_adds_byte2x2(
  byte1x2& x0, byte1x2& x1,
  byte1x2 y0, byte1x2 y1)
{
  x0 += y0;
  x1 += y1;
  byte2x2_sat(x0, x1);
}

// x0 -= y0
static FOG_INLINE void byte1x2_sub_byte1x2(
  byte1x2& x0, byte1x2 y0)
{
  x0 -= y0;
}

// x0 -= y0
// x1 -= y1
static FOG_INLINE void byte2x2_sub_byte2x2(
  byte1x2& x0, byte1x2& x1,
  byte1x2 y0, byte1x2 y1)
{
  x0 -= y0;
  x1 -= y1;
}

// x0 = max(x0 - y0, 0)
static FOG_INLINE void byte1x2_subs_byte1x2(
  byte1x2& x0, byte1x2 y0)
{
  x0 ^= _1x2MASK;
  x0 += y0;
  byte1x2_sat(x0);
  x0 ^= _1x2MASK;
}

// x0 = max(x0 - y0, 0)
// x1 = max(x1 - y1, 0)
static FOG_INLINE void byte2x2_subs_byte2x2(
  byte1x2& x0, byte1x2& x1,
  byte1x2 y0, byte1x2 y1)
{
  x0 ^= _1x2MASK;
  x1 ^= _1x2MASK;
  x0 += y0;
  x1 += y1;
  byte2x2_sat(x0, x1);
  x0 ^= _1x2MASK;
  x1 ^= _1x2MASK;
}

// x0 = (x0 * a) / 255
static FOG_INLINE void byte1x2_mul_u(
  byte1x2& x0, uint32_t a)
{
  x0 *= a; x0 = ((x0 + ((x0 >> 8) & _1x2MASK) + _1x2HALF) >> 8) & _1x2MASK;
}

// x0 = (x0 * a) / 255
// x1 = (x1 * a) / 255
static FOG_INLINE void byte2x2_mul_u(
  byte1x2& x0, byte1x2& x1, uint32_t a)
{
  x0 *= a; x0 = ((x0 + ((x0 >> 8) & _1x2MASK) + _1x2HALF) >> 8) & _1x2MASK;
  x1 *= a; x1 = ((x1 + ((x1 >> 8) & _1x2MASK) + _1x2HALF) >> 8) & _1x2MASK;
}

// x0 = (x0 * a) / 255 + y0
// x1 = (x1 * a) / 255 + y1
static FOG_INLINE void byte2x2_muladd_u_byte2x2(
  byte1x2& x0, byte1x2& x1, uint32_t a,
  byte1x2 y0, byte1x2 y1)
{
  byte2x2_mul_u(x0, x1, a);
  byte2x2_adds_byte2x2(x0, x1, y0, y1);
}

// x0 = (x0 * a) / 255 + y0
static FOG_INLINE void byte1x2_muladd_u_byte1x2(
  byte1x2& x0, uint32_t a, byte1x2 y0)
{
  byte1x2_mul_u(x0, a);
  byte1x2_adds_byte1x2(x0, y0);
}

//  x0 = (x0 * a + y0 * b) / 255
//  x1 = (x1 * a + y1 * b) / 255
static FOG_INLINE void byte2x2_addmul_u_byte2x2_u(
  byte1x2& x0, byte1x2& x1, uint32_t a,
  byte1x2 y0, byte1x2 y1, uint32_t b)
{
  byte2x2_mul_u(x0, x1, a);
  byte2x2_mul_u(y0, y1, b);
  byte2x2_adds_byte2x2(x0, x1, y0, y1);
}

// x0 = (x0 * a0) / 255
static FOG_INLINE void byte1x2_mul_byte1x2(
  byte1x2& x0, byte1x2 a0)
{
  x0 = ((x0 & 0x000000FF) * (a0 & 0x000000FF)) | ((x0 & 0x00FF0000) * ((a0 & 0x00FF0000) >> 16)) ;
  x0 = ((x0 + ((x0 >> 8) & _1x2MASK) + _1x2HALF) >> 8) & _1x2MASK;
}

// x0 = (x0 * a0) / 255
// x1 = (x0 * a1) / 255
static FOG_INLINE void byte2x2_mul_byte2x2(
  byte1x2& x0, byte1x2& x1,
  byte1x2 a0, byte1x2 a1)
{
  x0 = ((x0 & 0x000000FF) * (a0 & 0x000000FF)) | ((x0 & 0x00FF0000) * ((a0 & 0x00FF0000) >> 16)) ;
  x1 = ((x1 & 0x000000FF) * (a1 & 0x000000FF)) | ((x1 & 0x00FF0000) * ((a1 & 0x00FF0000) >> 16)) ;
  x0 = ((x0 + ((x0 >> 8) & _1x2MASK) + _1x2HALF) >> 8) & _1x2MASK;
  x1 = ((x1 + ((x1 >> 8) & _1x2MASK) + _1x2HALF) >> 8) & _1x2MASK;
}

// x0 = (x0 * a0) / 255 + y0
// x1 = (x0 * a1) / 255 + y1
static FOG_INLINE void byte2x2_muladd_byte2x2_byte2x2(
  byte1x2& x0, byte1x2& x1,
  byte1x2 a0, byte1x2 a1,
  byte1x2 y0, byte1x2 y1)
{
  byte2x2_mul_byte2x2(x0, x1, a0, a1);
  byte2x2_adds_byte2x2(x0, x1, y0, y1);
}

// x0 = min(x0, y0);
static FOG_INLINE void byte1x2_min(
  byte1x2& x0, byte1x2 y0)
{
  uint32_t x0lo = x0 & 0x000000FF;
  uint32_t x0hi = x0 & 0x00FF0000;

  uint32_t y0lo = y0 & 0x000000FF;
  uint32_t y0hi = y0 & 0x00FF0000;

  // I hope this will be compiled using cmov...
  if (x0lo > y0lo) x0lo = y0lo;
  if (x0hi > y0hi) x0hi = y0hi;

  x0 = x0lo | x0hi;
}

// x0 = min(x0, y0);
// x1 = min(x1, y1);
static FOG_INLINE void byte2x2_min(
  byte1x2& x0, byte1x2& x1,
  byte1x2 y0, byte1x2 y1)
{
  byte1x2_min(x0, y0);
  byte1x2_min(x1, y1);
}

// x0 = max(x0, y0);
static FOG_INLINE void byte1x2_max(
  byte1x2& x0, byte1x2 y0)
{
  uint32_t x0lo = x0 & 0x000000FF;
  uint32_t x0hi = x0 & 0x00FF0000;

  uint32_t y0lo = y0 & 0x000000FF;
  uint32_t y0hi = y0 & 0x00FF0000;

  // I hope this will be compiled using cmov...
  if (x0lo < y0lo) x0lo = y0lo;
  if (x0hi < y0hi) x0hi = y0hi;

  x0 = x0lo | x0hi;
}

// x0 = max(x0, y0);
// x1 = max(x1, y1);
static FOG_INLINE void byte2x2_max(
  byte1x2& x0, byte1x2& x1,
  byte1x2 y0, byte1x2 y1)
{
  byte1x2_max(x0, y0);
  byte1x2_max(x1, y1);
}

// x0 = saturate(x0 + y0 - z0)
static FOG_INLINE void byte1x2_addsub_byte1x2(
  byte1x2& x0, byte1x2 y0, byte1x2 z0)
{
  x0 += y0;

  uint32_t x0lo = (x0 & 0x000001FF);
  uint32_t x0hi = (x0 & 0x01FF0000) >> 16;

  x0lo -= (z0 & 0x0000FFFF);
  x0hi -= (z0 & 0xFFFF0000) >> 16;

  x0lo &= ((~x0lo) & 0xFFFF0000) >> 16;
  x0hi &= ((~x0hi) & 0xFFFF0000) >> 16;

  x0 = x0lo | (x0hi << 16);
  byte1x2_sat(x0);
}

// x0 = saturate(x0 + y0 - z0)
static FOG_INLINE void byte2x2_addsub_byte2x2(
  byte1x2& x0, byte1x2& x1,
  byte1x2 y0, byte1x2 y1,
  byte1x2 z0, byte1x2 z1)
{
  byte1x2_addsub_byte1x2(x0, y0, z0);
  byte1x2_addsub_byte1x2(x1, y1, z1);
}

// x0.lo <<= 1 (B channel)
// x0.hi <<= 1 (R channel)
// x1.lo <<= 1 (G channel)
static FOG_INLINE void byte2x2_lshift_by_1_no_alpha(
  byte1x2& x0, byte1x2& x1)
{
  x0 <<= 1;
  x1 += x1 & 0xFF;
}

static FOG_INLINE void byte1x2_interpolate_u(
  byte1x2& x0, byte1x2& x1, uint32_t a0,
  byte1x2 y0, byte1x2 y1, uint32_t b0)
{
  x0 = x0 * a0 + y0 * b0;
  x1 = x1 * a0 + y1 * b0;

  x0 = x0 + ((x0 >> 8) & 0x00FF00FF) + 0x00800080;
  x1 = x1 + ((x1 >> 8) & 0x00FF00FF) + 0x00800080;

  x0 &= _1x2MASK;
  x1 &= _1x2MASK;
}

static FOG_INLINE void byte1x2_neg(
  byte1x2& x0)
{
  x0 ^= 0x00FF00FF;
}

static FOG_INLINE void byte2x2_neg(
  byte1x2& x0, byte1x2& x1)
{
  x0 ^= 0x00FF00FF;
  x1 ^= 0x00FF00FF;
}

// ============================================================================
// [Fog::Raster - Pixel Operations]
// ============================================================================

static FOG_INLINE uint32_t singlemul(uint32_t x, uint32_t y)
{
  x *= y;
  x = ((x + (x >> 8) + 0x80) >> 8);
  return x;
}

static FOG_INLINE uint32_t singlelerp(uint32_t x, uint32_t y, uint32_t a)
{
  x *= a;
  y *= a ^ 0xFF;
  x += y;
  x = ((x + (x >> 8) + 0x80) >> 8);
  return x;
}

static FOG_INLINE uint32_t singleneg(uint32_t x)
{
  return x ^ 0xFF;
}

static FOG_INLINE uint32_t maskexpand(uint32_t x)
{
  x |= x << 8;
  x |= x << 16;
  return x;
}

static FOG_INLINE uint32_t alphaexpand(uint32_t x)
{
  x >>= 24;
  x |= x << 8;
  x |= x << 16;
  return x;
}

static FOG_INLINE uint32_t alphamul(uint32_t x, uint32_t a)
{
  a *= x >> 24;
  a = ((a + (a >> 8) + 0x80) >> 8);

  return (x & 0x00FFFFFF) | (a << 24);
}

static FOG_INLINE uint32_t alphaneg(uint32_t x)
{
  return x ^ 0xFF;
}

// x_c = (x_c * a) / 255
static FOG_INLINE uint32_t bytemul(uint32_t x, uint32_t a)
{
#if FOG_ARCH_BITS == 64
  uint64_t x0 = ((uint64_t)x | ((uint64_t)x << 24)) & FOG_UINT64_C(0x00FF00FF00FF00FF);
  x0 *= a;
  x0 = (x0 + ((x0 >> 8) & FOG_UINT64_C(0x00FF00FF00FF00FF)) + FOG_UINT64_C(0x0080008000800080)) >> 8;
  x0 &= FOG_UINT64_C(0x00FF00FF00FF00FF);
  return (uint32_t)(x0 | (x0 >> 24));
#else
  uint32_t t0 = ((x & 0x00FF00FF)     ) * a;
  uint32_t t1 = ((x & 0xFF00FF00) >> 8) * a;

  x  = ((t0 + ((t0 >> 8) & 0x00FF00FF) + 0x00800080) >> 8) & 0x00FF00FF;
  x |= ((t1 + ((t1 >> 8) & 0x00FF00FF) + 0x00800080)     ) & 0xFF00FF00;

  return x;
#endif
}

// x_c = x_c * a
// x_a = 0xFF
static FOG_INLINE uint32_t bytemul_full_alpha(uint32_t x, uint32_t a)
{
#if FOG_ARCH_BITS == 64
  uint64_t x0 = ((uint64_t)x | ((uint64_t)x << 24)) & FOG_UINT64_C(0x000000FF00FF00FF);
  x0 *= a;
  x0 = (x0 + ((x0 >> 8) & FOG_UINT64_C(0x000000FF00FF00FF)) + FOG_UINT64_C(0xFF00008000800080)) >> 8;
  x0 &= FOG_UINT64_C(0x00FF00FF00FF00FF);
  return (uint32_t)(x0 | (x0 >> 24));
#else
  uint32_t t0 = ((x & 0x00FF00FF)) * a;
  uint32_t t1 = ((x & 0x0000FF00)) * a;

  x  = ((t0 + ((t0 >> 8) & 0x00FF00FF) + 0x00800080) >> 8) & 0x00FF00FF;
  x |= ((t1 + ((t1 >> 8) & 0x00FF00FF) + 0xFF000080) >> 8) & 0xFF00FF00;

  return x;
#endif
}

// x_c = x_c * a
// x_a = 0x00
static FOG_INLINE uint32_t bytemul_reset_alpha(uint32_t x, uint32_t a)
{
#if FOG_ARCH_BITS == 64
  uint64_t x0 = ((uint64_t)x | ((uint64_t)x << 24)) & FOG_UINT64_C(0x000000FF00FF00FF);
  x0 *= a;
  x0 = (x0 + ((x0 >> 8) & FOG_UINT64_C(0x000000FF00FF00FF)) + FOG_UINT64_C(0x0000008000800080)) >> 8;
  x0 &= FOG_UINT64_C(0x000000FF00FF00FF);
  return (uint32_t)(x0 | (x0 >> 24));
#else
  uint32_t t0 = ((x & 0x00FF00FF)) * a;
  uint32_t t1 = ((x & 0x0000FF00)) * a;

  x  = ((t0 + ((t0 >> 8) & 0x00FF00FF) + 0x00800080) >> 8) & 0x00FF00FF;
  x |= ((t1 + ((t1 >> 8) & 0x00FF00FF) + 0x00000080) >> 8) & 0x0000FF00;

  return x;
#endif
}

// x_c = min(x_c + y_c, 255)
static FOG_INLINE uint32_t byteadd(uint32_t x, uint32_t y)
{
#if FOG_ARCH_BITS == 64
  uint64_t x0 = ((uint64_t)x | ((uint64_t)x << 24)) & FOG_UINT64_C(0x00FF00FF00FF00FF);
  uint64_t y0 = ((uint64_t)y | ((uint64_t)y << 24)) & FOG_UINT64_C(0x00FF00FF00FF00FF);

  x0 += y0;
  x0 |= FOG_UINT64_C(0x0100010001000100) - ((x0 >> 8) & FOG_UINT64_C(0x00FF00FF00FF00FF));
  x0 &= FOG_UINT64_C(0x00FF00FF00FF00FF);

  return (uint32_t)(x0 | (x0 >> 24));
#else
  uint32_t t0 = (x & 0x00FF00FF);
  uint32_t t1 = (x & 0xFF00FF00) >> 8;

  t0 += (y & 0x00FF00FF);
  t1 += (y & 0xFF00FF00) >> 8;

  t0 |= 0x01000100 - ((t0 >> 8) & 0x00FF00FF);
  t1 |= 0x01000100 - ((t1 >> 8) & 0x00FF00FF);

  t0 &= 0x00FF00FF;
  t1 &= 0x00FF00FF;

  return t0 | (t1 << 8);
#endif
}

// x_c = (x_c * a) / 255 + y
static FOG_INLINE uint32_t bytemuladd(uint32_t x, uint32_t a, uint32_t y)
{
#if FOG_ARCH_BITS == 64
  uint64_t x0 = ((uint64_t)x | ((uint64_t)x << 24)) & FOG_UINT64_C(0x00FF00FF00FF00FF);
  uint64_t y0 = ((uint64_t)y | ((uint64_t)y << 24)) & FOG_UINT64_C(0x00FF00FF00FF00FF);

  x0 *= a;
  x0 = (x0 + ((x0 >> 8) & FOG_UINT64_C(0x00FF00FF00FF00FF)) + FOG_UINT64_C(0x0080008000800080)) >> 8;
  x0 &= FOG_UINT64_C(0x00FF00FF00FF00FF);

  x0 += y0;
  x0 |= FOG_UINT64_C(0x0100010001000100) - ((x0 >> 8) & FOG_UINT64_C(0x00FF00FF00FF00FF));
  x0 &= FOG_UINT64_C(0x00FF00FF00FF00FF);

  return (uint32_t)(x0 | (x0 >> 24));
#else
  uint32_t t0 = ((x & 0x00FF00FF)     ) * a;
  uint32_t t1 = ((x & 0xFF00FF00) >> 8) * a;

  t0 = ((t0 + ((t0 >> 8) & 0x00FF00FF) + 0x00800080) >> 8) & 0x00FF00FF;
  t1 = ((t1 + ((t1 >> 8) & 0x00FF00FF) + 0x00800080) >> 8) & 0x00FF00FF;

  t0 += (y & 0x00FF00FF);
  t1 += (y & 0xFF00FF00) >> 8;

  t0 |= 0x01000100 - ((t0 >> 8) & 0x00FF00FF);
  t1 |= 0x01000100 - ((t1 >> 8) & 0x00FF00FF);

  t0 &= 0x00FF00FF;
  t1 &= 0x00FF00FF;

  return t0 | (t1 << 8);
#endif
}

static FOG_INLINE uint32_t byteaddmul(
  uint32_t x0, uint32_t a0,
  uint32_t y0, uint32_t b0)
{
  uint32_t t0 = ((x0 & 0x00FF00FF)     ) * a0 + ((y0 & 0x00FF00FF)     ) * b0;
  uint32_t t1 = ((x0 & 0xFF00FF00) >> 8) * a0 + ((y0 & 0xFF00FF00) >> 8) * b0;

  t0 = (t0 + ((t0 >> 8) & 0x00FF00FF) + 0x00800080);
  t1 = (t1 + ((t1 >> 8) & 0x00FF00FF) + 0x00800080);

  x0  = (t0 & 0xFF00FF00) >> 8;
  x0 |= (t1 & 0xFF00FF00);

  return x0;
}

static FOG_INLINE uint32_t byteaddmul_div256(
  uint32_t x0, uint32_t a0,
  uint32_t y0, uint32_t b0)
{
  uint32_t t0 = ((x0 & 0x00FF00FF)     ) * a0 + ((y0 & 0x00FF00FF)     ) * b0;
  uint32_t t1 = ((x0 & 0xFF00FF00) >> 8) * a0 + ((y0 & 0xFF00FF00) >> 8) * b0;

  x0  = (t0 & 0xFF00FF00) >> 8;
  x0 |= (t1 & 0xFF00FF00);

  return x0;
}

#if 0
// Currently not used.
// x_c = (x_c * a_c) / 255
static FOG_INLINE uint32_t bytemulC(uint32_t x, uint32_t a)
{
  uint32_t t;
  uint32_t r = (x & 0xFF) * (a & 0xFF);
  r |= (x & 0x00FF0000) * ((a >> 16) & 0xFF);
  r += 0x80;
  r = (r + ((r >> 8) & 0x00FF00FF)) >> 8;
  r &= 0x00FF00FF;

  x >>= 8;
  t = (x & 0xFF) * ((a >> 8) & 0xFF);
  t |= (x & 0x00FF0000) * (a >> 24);
  t += 0x80;
  t = t + ((t >> 8) & 0x00FF00FF);

  return r | (t & 0xFF00FF00);
}

// x_c = (x_c * a) / 255 + y
static FOG_INLINE uint32_t bytemuladdC(uint32_t x, uint32_t a, uint32_t y)
{
  uint32_t t;
  uint32_t r = (x & 0xFF) * (a & 0xFF);

  r |= (x & 0x00FF0000) * ((a >> 16) & 0xFF);
  r += 0x80;
  r = (r + ((r >> 8) & 0x00FF00FF)) >> 8;
  r &= 0x00FF00FF;
  r += y & 0x00FF00FF;
  r |= 0x01000100 - ((r >> 8) & 0x00FF00FF);
  r &= 0x00FF00FF;

  x >>= 8;
  t = (x & 0xFF) * ((a >> 8) & 0xFF);
  t |= (x & 0x00FF0000) * (a >> 24);
  t += 0x80;
  t = (t + ((t >> 8) & 0x00FF00FF)) >> 8;
  t &= 0x00FF00FF;
  t += (y >> 8) & 0x00FF00FF;
  t |= 0x01000100 - ((t >> 8) & 0x00FF00FF);
  t &= 0x00FF00FF;

  return r | (t << 8);
}

// x_c = (x_c * a_c + y_c * b) / 255
static FOG_INLINE uint32_t byteaddmulC(uint32_t x, uint32_t a, uint32_t y, uint32_t b)
{
  uint32_t t;
  uint32_t r = (x >> 24) * (a >> 24) +
               (y >> 24) * b;
  r += (r >> 8) + 0x80;
  r >>= 8;

  t = (x & 0x0000FF00) * ((a >> 8) & 0xFF) + (y & 0x0000FF00) * b;
  t += (t >> 8) + (0x80 << 8);
  t >>= 16;

  t |= r << 16;
  t |= 0x01000100 - ((t >> 8) & 0x00FF00FF);
  t &= 0x00FF00FF;
  t <<= 8;

  r = ((x >> 16) & 0xFF) * ((a >> 16) & 0xFF) +
      ((y >> 16) & 0xFF) * b + 0x80;
  r += (r >> 8);
  r >>= 8;

  x = (x & 0xFF) * (a & 0xFF) + (y & 0xFF) * b + 0x80;
  x += (x >> 8);
  x >>= 8;
  x |= r << 16;
  x |= 0x01000100 - ((x >> 8) & 0x00FF00FF);
  x &= 0x00FF00FF;

  return x | t;
}
#endif

// ============================================================================
// [Fog::Raster - Demultiply Reciprocal Table]
// ============================================================================

extern FOG_API const uint32_t demultiply_reciprocal_table_d[256];
extern FOG_API const uint16_t demultiply_reciprocal_table_w[256*4];

// ============================================================================
// [Fog::Raster - Get Red/Green/Blue/Alpha]
// ============================================================================

static FOG_INLINE uint32_t getRed(uint32_t c)
{
  return (c >> RGB32_RShift);
}

static FOG_INLINE uint32_t getGreen(uint32_t c)
{
  return (c >> RGB32_GShift);
}

static FOG_INLINE uint32_t getBlue(uint32_t c)
{
  return (c >> RGB32_BShift);
}

static FOG_INLINE uint32_t getAlpha(uint32_t c)
{
  return (c >> RGB32_AShift);
}

// ============================================================================
// [Fog::Raster - FromRGBA]
// ============================================================================

static FOG_INLINE uint32_t fromRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF)
{
  return ((uint32_t)r << RGB32_RShift) |
         ((uint32_t)g << RGB32_GShift) |
         ((uint32_t)b << RGB32_BShift) |
         ((uint32_t)a << RGB32_AShift) ;
}

// ============================================================================
// [Fog::Raster - FromGrey / ToGrey]
// ============================================================================

static FOG_INLINE uint32_t fromGrey(uint8_t grey, uint8_t alpha = 0xFF)
{
  return fromRGBA(grey, grey, grey, alpha);
}

static FOG_INLINE uint8_t toGrey(uint32_t c)
{
  // We are using this formula:
  //  0.212671 * R + 0.715160 * G + 0.072169 * B;
  // As:
  //  (13938 * R + 46868 * G + 4730 * B) / 65536

  uint32_t grey =
    ((c >> 16) & 0xFF) * 13938 +
    ((c >>  8) & 0xFF) * 46868 +
    ((c      ) & 0xFF) *  4730 ;

  return (uint8_t)(grey >> 16);
}

// ============================================================================
// [Fog::Raster - Premultiply / Demultiply]
// ============================================================================

static FOG_INLINE uint32_t premultiply(uint32_t x)
{
#if FOG_ARCH_BITS == 64 && 0
  uint32_t a = x >> 24;
  uint64_t x0 = ((uint64_t)x | ((uint64_t)x << 24)) & FOG_UINT64_C(0x000000FF00FF00FF);
  x0 |= FOG_UINT64_C(0x00FF000000000000);
  x0 *= a;
  x0 = (x0 + ((x0 >> 8) & FOG_UINT64_C(0x00FF00FF00FF00FF)) + FOG_UINT64_C(0x0080008000800080)) >> 8;
  x0 &= FOG_UINT64_C(0x00FF00FF00FF00FF);
  return (uint32_t)(x0 | (x0 >> 24));
#else
  uint32_t a = x >> 24;
  uint32_t t0 = (x & 0x00FF00FF) * a;
  uint32_t t1 = (x & 0x0000FF00) * a;

  t0 = ((t0 + ((t0 >> 8) & 0x00FF00FF) + 0x00800080) >> 8) & 0x00FF00FF;
  t1 = ((t1 + ((t1 >> 8) & 0x0000FF00) + 0x00008000) >> 8) & 0x0000FF00;

  return t0 | t1 | (a << 24);
#endif
}

static FOG_INLINE uint32_t demultiply(uint32_t x)
{
  uint32_t a = (x >> 24);
  uint32_t recip = demultiply_reciprocal_table_d[a];

  uint32_t r = (x >> 16) & 0xFF;
  uint32_t g = (x >>  8) & 0xFF;
  uint32_t b = (x      ) & 0xFF;

  r = ((r * recip)      ) & 0x00FF0000;
  g = ((g * recip) >>  8) & 0x0000FF00;
  b = ((b * recip) >> 16) & 0x000000FF;

  return r | g | b | (a << 24);
}

// ============================================================================
// [Fog::Raster - Blending]
// ============================================================================

static FOG_INLINE uint32_t combine_alpha(uint32_t a0, uint32_t a1)
{
  return div255(a0 * a1);
}

static FOG_INLINE uint32_t blend_over_nonpremultiplied(uint32_t dst, uint32_t src, uint32_t a)
{
  dst = bytemul_full_alpha(dst, 255 - a);
  src = bytemul_reset_alpha(src, a);
  return dst + src;
}

static FOG_INLINE uint32_t blend_over_srcpremultiplied(uint32_t dst, uint32_t src, uint32_t a)
{
  dst = bytemul_reset_alpha(dst, 255 - a);
  return (dst + src) | 0xFF000000;
}

static FOG_INLINE uint32_t blend_lerp(uint32_t dst, uint32_t src, uint32_t a)
{
  dst = bytemul(dst, 255 - a);
  src = bytemul(src, a);
  return dst + src;
}

// ============================================================================
// [Fog::Raster - PixFmt_ARGB32]
// ============================================================================

struct FOG_HIDDEN PixFmt_ARGB32
{
  enum {
    BytesPerPixel = 4,
    HasRGB = 1,
    HasAlpha = 1,
    IsPremultiplied = 0
  };

  static FOG_INLINE uint32_t fetch(const uint8_t* p)
  {
    return ((const uint32_t *)p)[0];
  }

  static FOG_INLINE uint32_t fetchAlpha(const uint8_t* p)
  {
    return p[RGB32_AByte];
  }

  static FOG_INLINE void store(uint8_t* p, uint32_t s0)
  {
    ((uint32_t *)p)[0] = s0;
  }

  static FOG_INLINE void storeRGB(uint8_t* p, uint8_t r, uint8_t g, uint8_t b)
  {
    ((uint32_t*)p)[0] =
      ((uint32_t)r << RGB32_RShift) |
      ((uint32_t)g << RGB32_GShift) |
      ((uint32_t)b << RGB32_BShift) |
      ((uint32_t)RGB32_AMask);
  }
};

// ============================================================================
// [Fog::Raster - PixFmt_PRGB32]
// ============================================================================

struct FOG_HIDDEN PixFmt_PRGB32
{
  enum {
    BytesPerPixel = 4,
    HasRGB = 1,
    HasAlpha = 1,
    IsPremultiplied = 1
  };

  static FOG_INLINE uint32_t fetch(const uint8_t* p)
  {
    return ((const uint32_t *)p)[0];
  }

  static FOG_INLINE uint32_t fetchAlpha(const uint8_t* p)
  {
    return p[RGB32_AByte];
  }

  static FOG_INLINE void store(uint8_t* p, uint32_t s0)
  {
    ((uint32_t *)p)[0] = s0;
  }

  static FOG_INLINE void storeRGB(uint8_t* p, uint8_t r, uint8_t g, uint8_t b)
  {
    ((uint32_t*)p)[0] =
      ((uint32_t)r << RGB32_RShift) |
      ((uint32_t)g << RGB32_GShift) |
      ((uint32_t)b << RGB32_BShift) |
      ((uint32_t)RGB32_AMask);
  }
};

// ============================================================================
// [Fog::Raster - PixFmt_RGB32]
// ============================================================================

struct FOG_HIDDEN PixFmt_RGB32
{
  enum {
    BytesPerPixel = 4,
    HasRGB = 1,
    HasAlpha = 0,
    IsPremultiplied = 0
  };

  static FOG_INLINE uint32_t fetch(const uint8_t* p)
  {
    return ((const uint32_t *)p)[0];
  }

  static FOG_INLINE uint32_t fetchAlpha(const uint8_t* p)
  {
    FOG_UNUSED(p);
    return 0xFF;
  }

  static FOG_INLINE void store(uint8_t* p, uint32_t s0)
  {
    ((uint32_t *)p)[0] = s0;
  }

  static FOG_INLINE void storeRGB(uint8_t* p, uint8_t r, uint8_t g, uint8_t b)
  {
    ((uint32_t*)p)[0] =
      ((uint32_t)r << RGB32_RShift) |
      ((uint32_t)g << RGB32_GShift) |
      ((uint32_t)b << RGB32_BShift) |
      ((uint32_t)RGB32_AMask) ;
  }
};

// ============================================================================
// [Fog::Raster - PixFmt_RGB24]
// ============================================================================

struct FOG_HIDDEN PixFmt_RGB24
{
  enum {
    BytesPerPixel = 3,
    HasRGB = 1,
    HasAlpha = 0,
    IsPremultiplied = 0
  };

  static FOG_INLINE uint32_t fetch(const uint8_t* p)
  {
#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
    return ( (uint32_t)(((const uint16_t*)(p))[0]) | ((uint32_t )p[2] << 16) );
#else
    return ((uint32_t)p[RGB24_RByte] << RGB32_RShift) |
           ((uint32_t)p[RGB24_GByte] << RGB32_GShift) |
           ((uint32_t)p[RGB24_BByte] << RGB32_BShift) ;
#endif
  }

  static FOG_INLINE uint32_t fetchAlpha(const uint8_t* p)
  {
    FOG_UNUSED(p);
    return 0xFF;
  }

  static FOG_INLINE void store(uint8_t* p, uint32_t s0)
  {
#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
    ((uint16_t*)(p))[0] = (uint16_t)s0;
    p[2] = (uint8_t)(s0 >> 16);
#else
    p[RGB24_RByte] = (uint8_t)(s0 >> RGB32_RShift);
    p[RGB24_GByte] = (uint8_t)(s0 >> RGB32_GShift);
    p[RGB24_BByte] = (uint8_t)(s0 >> RGB32_BShift);
#endif
  }

  static FOG_INLINE void storeRGB(uint8_t* p, uint8_t r, uint8_t g, uint8_t b)
  {
    p[RGB24_RByte] = r;
    p[RGB24_GByte] = g;
    p[RGB24_BByte] = b;
  }
};

// ============================================================================
// [Fog::Raster - PixFmt_BGR24]
// ============================================================================

struct FOG_HIDDEN PixFmt_BGR24
{
  enum {
    BytesPerPixel = 3,
    HasRGB = 1,
    HasAlpha = 0,
    IsPremultiplied = 0
  };

  static FOG_INLINE uint32_t fetch(const uint8_t* p)
  {
    return ((uint32_t)p[2-RGB24_RByte] << RGB32_RShift) |
           ((uint32_t)p[2-RGB24_GByte] << RGB32_GShift) |
           ((uint32_t)p[2-RGB24_BByte] << RGB32_BShift) ;
  }

  static FOG_INLINE uint32_t fetchAlpha(const uint8_t* p)
  {
    FOG_UNUSED(p);
    return 0xFF;
  }

  static FOG_INLINE void store(uint8_t* p, uint32_t s0)
  {
    p[2-RGB24_RByte] = (uint8_t)(s0 >> RGB32_RShift);
    p[2-RGB24_GByte] = (uint8_t)(s0 >> RGB32_GShift);
    p[2-RGB24_BByte] = (uint8_t)(s0 >> RGB32_BShift);
  }

  static FOG_INLINE void storeRGB(uint8_t* p, uint8_t r, uint8_t g, uint8_t b)
  {
    p[2-RGB24_RByte] = r;
    p[2-RGB24_GByte] = g;
    p[2-RGB24_BByte] = b;
  }
};

// ============================================================================
// [Fog::Raster - PixFmt_RGB16_555]
// ============================================================================

struct FOG_HIDDEN PixFmt_RGB16_555
{
  enum {
    BytesPerPixel = 2,
    HasRGB = 1,
    HasAlpha = 0,
    IsPremultiplied = 0
  };

  static FOG_INLINE uint32_t fetch(const uint8_t* p)
  {
    uint32_t x = ((const uint16_t*)p)[0];
    uint32_t r = ((x & 0x7C00) | ((x & 0x7000) >> 5)) << 9;
    uint32_t g = ((x & 0x03E0) | ((x & 0x0380) >> 5)) << 6;
    uint32_t b = ((x & 0x001C) | ((x & 0x001F) << 5)) >> 2;
    return r | g | b;
  }

  static FOG_INLINE uint32_t fetchAlpha(const uint8_t* p)
  {
    FOG_UNUSED(p);
    return 0xFF;
  }

  static FOG_INLINE void store(uint8_t* p, uint32_t s0)
  {
    ((uint16_t*)p)[0] = (uint16_t)(
      ((s0 >> 3) & 0x001F) |
      ((s0 >> 6) & 0x03E0) |
      ((s0 >> 9) & 0x7C00));
  }
};

struct FOG_HIDDEN PixFmt_RGB16_555_BS
{
  enum {
    BytesPerPixel = 2,
    HasRGB = 1,
    HasAlpha = 0,
    IsPremultiplied = 0
  };

  static FOG_INLINE uint32_t fetch(const uint8_t* p)
  {
    uint32_t x = Memory::bswap16(((const uint16_t*)p)[0]);
    uint32_t r = ((x & 0x7C00) | ((x & 0x7000) >> 5)) << 9;
    uint32_t g = ((x & 0x03E0) | ((x & 0x0380) >> 5)) << 6;
    uint32_t b = ((x & 0x001C) | ((x & 0x001F) << 5)) >> 2;
    return r | g | b;
  }

  static FOG_INLINE uint32_t fetchAlpha(const uint8_t* p)
  {
    FOG_UNUSED(p);
    return 0xFF;
  }

  static FOG_INLINE void store(uint8_t* p, uint32_t s0)
  {
    ((uint16_t*)p)[0] = Memory::bswap16((uint16_t)(
      ((s0 >> 3) & 0x001F) |
      ((s0 >> 6) & 0x03E0) |
      ((s0 >> 9) & 0x7C00)));
  }
};

// ============================================================================
// [Fog::Raster - PixFmt_RGB16_565]
// ============================================================================

// TODO: Not correct
struct FOG_HIDDEN PixFmt_RGB16_565
{
  enum {
    BytesPerPixel = 2,
    HasRGB = 1,
    HasAlpha = 0,
    IsPremultiplied = 0
  };

  static FOG_INLINE uint32_t fetch(const uint8_t* p)
  {
    uint32_t x = ((const uint16_t*)p)[0];
    uint32_t r = ((x & 0xF800) | ((x & 0xE000) >> 5)) << 8;
    uint32_t g = ((x & 0x07E0) | ((x & 0x0600) >> 6)) << 5;
    uint32_t b = ((x & 0x001C) | ((x & 0x001F) << 5)) >> 2;
    return r | g | b;
  }

  static FOG_INLINE uint32_t fetchAlpha(const uint8_t* p)
  {
    FOG_UNUSED(p);
    return 0xFF;
  }

  static FOG_INLINE void store(uint8_t* p, uint32_t s0)
  {
    ((uint16_t*)p)[0] = (uint16_t)(
      ((s0 >> 3) & 0x001F) |
      ((s0 >> 5) & 0x07E0) |
      ((s0 >> 8) & 0xF800));
  }
};

// TODO: Not correct
struct FOG_HIDDEN PixFmt_RGB16_565_BS
{
  enum {
    BytesPerPixel = 2,
    HasRGB = 1,
    HasAlpha = 0,
    IsPremultiplied = 0
  };

  static FOG_INLINE uint32_t fetch(const uint8_t* p)
  {
    uint32_t x = Memory::bswap16(((const uint16_t*)p)[0]);
    uint32_t r = ((x & 0xF800) | ((x & 0xE000) >> 5)) << 8;
    uint32_t g = ((x & 0x07E0) | ((x & 0x0600) >> 6)) << 5;
    uint32_t b = ((x & 0x001C) | ((x & 0x001F) << 5)) >> 2;
    return r | g | b;
  }

  static FOG_INLINE uint32_t fetchAlpha(const uint8_t* p)
  {
    FOG_UNUSED(p);
    return 0xFF;
  }

  static FOG_INLINE void store(uint8_t* p, uint32_t s0)
  {
    ((uint16_t*)p)[0] = Memory::bswap16((uint16_t)(
      ((s0 >> 3) & 0x001F) |
      ((s0 >> 5) & 0x07E0) |
      ((s0 >> 8) & 0xF800)));
  }
};

// ============================================================================
// [Fog::Raster - PixFmt_A8]
// ============================================================================

struct FOG_HIDDEN PixFmt_A8
{
  enum {
    BytesPerPixel = 2,
    HasRGB = 1,
    HasAlpha = 0,
    IsPremultiplied = 0
  };

  static FOG_INLINE uint32_t fetch(const uint8_t* p)
  {
    return p[0];
  }

  static FOG_INLINE uint32_t fetchAlpha(const uint8_t* p)
  {
    return p[0];
  }

  static FOG_INLINE void store(uint8_t* p, uint32_t s0)
  {
    p[0] = (uint8_t)s0;
  }
};

} // Raster namespace
} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_RASTER_C_H
