// [Fog/Graphics library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_BYTEUTIL_H
#define _FOG_GRAPHICS_BYTEUTIL_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Graphics/Constants.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {
namespace ByteUtil {

// ============================================================================
// [Fog::ByteUtil - Mul]
// ============================================================================

template<typename T, int Mul>
static FOG_INLINE T mul(T i) { return i * Mul; }

template<typename T>
static FOG_INLINE T mul1(T i) { return i; }

template<typename T>
static FOG_INLINE T mul2(T i) { return i * 2; }

template<typename T>
static FOG_INLINE T mul3(T i) { return i * 3; }

template<typename T>
static FOG_INLINE T mul4(T i) { return i * 4; }

// ============================================================================
// [Fog::ByteUtil - Div255/256]
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
//!
//! @not This template is simply implemented as a right shift by 8 bits.
template<typename T>
static FOG_INLINE T div256(T i)
{
  return (T)(i >> 8U);
}

// ============================================================================
// [Fog::ByteUtil - Byte1x2 - 32-Bit Integer SIMD Library]
// ============================================================================

// These function allows to do SIMD with classic 32 bit integers.
//
// Terminology:
// - byte1x2 - unsigned 32 bit integer that contains two bytes at mask
//             BYTE_1x2MASK that is 0x00FF00FF.
// - byte2x2 - two byte1x2 values
// - u       - scalar at range 0 to 255 (or 0 to 256 depending to the context).
//
// Pixels are usually encoded in byte2x2 using this formula:
// - byte1x2 b0 (0x00RR00BB)
// - byte1x2 b1 (0x00AA00GG);

typedef uint32_t byte1x2;

static const uint32_t BYTE_1x1MASK          = 0x000000FFU;
static const uint32_t BYTE_1x2MASK          = 0x00FF00FFU;
static const uint32_t BYTE_1x1HALF          = 0x00000080U;
static const uint32_t BYTE_1x2HALF          = 0x00800080U;
static const uint32_t BYTE_1x1HALF_PLUS_ONE = 0x00000100U;
static const uint32_t BYTE_1x2MASK_PLUS_ONE = 0x10000100U;

// ============================================================================
// [Fog::ByteUtil - Byte1x2 - From / To / Packing / Unpacking]
// ============================================================================

static FOG_INLINE byte1x2 byte1x2_from_u(uint32_t b) { return b | (b << 16); }

static FOG_INLINE uint32_t byte1x2_lo(byte1x2 x0) { return (x0 & 0xFF); }
static FOG_INLINE uint32_t byte1x2_hi(byte1x2 x0) { return (x0 >> 16); }

//! @brief Pack x0 and x1 into single DWORD.
static FOG_INLINE uint32_t byte2x2_pack_0213(byte1x2 x0, byte1x2 x1) { return x0 | (x1 << 8); }

//! @brief Unpack 0st and 2nd BYTE of single DWORD into one byte1x2 value.
static FOG_INLINE void byte2x2_unpack_02(byte1x2& x0, uint32_t src_c) { x0 = (src_c & BYTE_1x2MASK); }

//! @brief Unpack 1st and 3rd BYTE of single DWORD into one byte1x2 value.
static FOG_INLINE void byte2x2_unpack_13(byte1x2& x0, uint32_t src_c) { x0 = (src_c >> 8) & BYTE_1x2MASK; }

//! @brief Unpack all bytes in single DWORD into two byte1x2 values.
static FOG_INLINE void byte2x2_unpack_0213(byte1x2& x0, byte1x2& x1, uint32_t src_c)
{
  x0 = src_c;
  x1 = src_c >> 8;
  x0 &= BYTE_1x2MASK;
  x1 &= BYTE_1x2MASK;
}

//! @brief Unpack all bytes in single DWORD into two byte1x2 values.
static FOG_INLINE void byte2x2_unpack_021X(byte1x2& x0, byte1x2& x1, uint32_t src_c)
{
  x0 = src_c;
  x1 = src_c >> 8;
  x0 &= BYTE_1x2MASK;
  x1 &= BYTE_1x1MASK;
}

// ============================================================================
// [Fog::ByteUtil - Byte1x2 - Expand]
// ============================================================================

// x0 = y0hi, y0hi
static FOG_INLINE void byte1x2_expand_hi(byte1x2& x0, byte1x2 y0)
{
  x0 = (y0 >> 16);
  x0 |= (y0 & 0x00FF0000);
}

// x0 = y0hi, y0hi
// x1 = y0hi, y0hi
static FOG_INLINE void byte2x2_expand_hi(byte1x2& x0, byte1x2& x1, byte1x2 y0)
{
  byte1x2_expand_hi(x0, y0);
  x1 = x0;
}

// ============================================================================
// [Fog::ByteUtil - Byte1x2 - Saturate]
// ============================================================================

// x0 = min(x0, 255)
static FOG_INLINE void byte1x2_sat(byte1x2& x0)
{
  x0 |= BYTE_1x2MASK_PLUS_ONE - ((x0 >> 8) & BYTE_1x2MASK);
  x0 &= 0x00FF00FF;
}

// x0 = min(x0, 255)
// x1 = min(x1, 255)
static FOG_INLINE void byte2x2_sat(byte1x2& x0, byte1x2& x1)
{
  x0 |= BYTE_1x2MASK_PLUS_ONE - ((x0 >> 8) & BYTE_1x2MASK);
  x1 |= BYTE_1x2MASK_PLUS_ONE - ((x1 >> 8) & BYTE_1x2MASK);
  x0 &= 0x00FF00FF;
  x1 &= 0x00FF00FF;
}

// ============================================================================
// [Fog::ByteUtil - Byte1x2 - Add / Subtract]
// ============================================================================

// x0 += a
static FOG_INLINE void byte1x2_add_u(byte1x2& x0, uint32_t a)
{
  x0 += a | (a << 16);
}

// x0 += a
// x1 += a
static FOG_INLINE void byte2x2_add_u(byte1x2& x0, byte1x2& x1, uint32_t a)
{
  a |= a << 16;
  x0 += a;
  x1 += a;
}

// x0 += y0
static FOG_INLINE void byte1x2_add_byte1x2(byte1x2& x0, byte1x2 y0)
{
  x0 += y0;
}

// x0 += y0
// x1 += y1
static FOG_INLINE void byte2x2_add_byte2x2(byte1x2& x0, byte1x2& x1, byte1x2 y0, byte1x2 y1)
{
  x0 += y0;
  x1 += y1;
}

// x0 = max(x0 - a, 0)
static FOG_INLINE void byte1x2_addus_u(byte1x2& x0, uint32_t a)
{
  x0 += a | (a << 16);
  byte1x2_sat(x0);
}

// x0 = max(x0 - a, 0)
// x0 = max(x0 - a, 0)
static FOG_INLINE void byte2x2_addus_u(byte1x2& x0, byte1x2& x1, uint32_t a)
{
  a |= a << 16;
  x0 += a;
  x1 += a;
  byte2x2_sat(x0, x1);
}

// x0 = min(x0 + y0, 255)
static FOG_INLINE void byte1x2_addus_byte1x2(byte1x2& x0, byte1x2 y0)
{
  x0 += y0;
  byte1x2_sat(x0);
}

// x0 = min(x0 + y0, 255)
// x1 = min(x1 + y1, 255)
static FOG_INLINE void byte2x2_addus_byte2x2(byte1x2& x0, byte1x2& x1, byte1x2 y0, byte1x2 y1)
{
  x0 += y0;
  x1 += y1;
  byte2x2_sat(x0, x1);
}

// x0 -= a
static FOG_INLINE void byte1x2_sub_u(byte1x2& x0, uint32_t a)
{
  a |= a << 16;
  x0 -= (a | a << 16);
}

// x0 -= a
// x1 -= a
static FOG_INLINE void byte2x2_sub_u(byte1x2& x0, byte1x2& x1, uint32_t a)
{
  a |= a << 16;
  x0 -= a;
  x1 -= a;
}

// x0 -= y0
static FOG_INLINE void byte1x2_sub_byte1x2(byte1x2& x0, byte1x2 y0)
{
  x0 -= y0;
}

// x0 -= y0
// x1 -= y1
static FOG_INLINE void byte2x2_sub_byte2x2(byte1x2& x0, byte1x2& x1, byte1x2 y0, byte1x2 y1)
{
  x0 -= y0;
  x1 -= y1;
}

// x0 = max(x0 - a, 0)
static FOG_INLINE void byte1x2_subus_u(byte1x2& x0, uint32_t a)
{
  x0 ^= BYTE_1x2MASK;
  x0 += a | (a << 16);
  byte1x2_sat(x0);
  x0 ^= BYTE_1x2MASK;
}

// x0 = max(x0 - a, 0)
// x1 = max(x1 - a, 0)
static FOG_INLINE void byte2x2_subus_u(byte1x2& x0, byte1x2& x1, uint32_t a)
{
  a |= a << 16;
  x0 ^= BYTE_1x2MASK;
  x1 ^= BYTE_1x2MASK;
  x0 += a;
  x1 += a;
  byte2x2_sat(x0, x1);
  x0 ^= BYTE_1x2MASK;
  x1 ^= BYTE_1x2MASK;
}

// x0 = max(x0 - y0, 0)
static FOG_INLINE void byte1x2_subus_byte1x2(byte1x2& x0, byte1x2 y0)
{
  x0 ^= BYTE_1x2MASK;
  x0 += y0;
  byte1x2_sat(x0);
  x0 ^= BYTE_1x2MASK;
}

// x0 = max(x0 - y0, 0)
// x1 = max(x1 - y1, 0)
static FOG_INLINE void byte2x2_subus_byte2x2(byte1x2& x0, byte1x2& x1, byte1x2 y0, byte1x2 y1)
{
  x0 ^= BYTE_1x2MASK;
  x1 ^= BYTE_1x2MASK;
  x0 += y0;
  x1 += y1;
  byte2x2_sat(x0, x1);
  x0 ^= BYTE_1x2MASK;
  x1 ^= BYTE_1x2MASK;
}

// ============================================================================
// [Fog::ByteUtil - Byte1x2 - AddSub]
// ============================================================================

// x0 = saturate(x0 + y0 - z0)
static FOG_INLINE void byte1x2_addsubus_byte1x2(
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
static FOG_INLINE void byte2x2_addsubus_byte2x2(
  byte1x2& x0, byte1x2& x1,
  byte1x2 y0, byte1x2 y1,
  byte1x2 z0, byte1x2 z1)
{
  byte1x2_addsubus_byte1x2(x0, y0, z0);
  byte1x2_addsubus_byte1x2(x1, y1, z1);
}

// ============================================================================
// [Fog::ByteUtil - Byte1x2 - MulDiv]
// ============================================================================

// TODO: Add here simple mul (old code must be first corrected).

// x0 = (x0 * a) / 255
static FOG_INLINE void byte1x2_muldiv255_u(
  byte1x2& x0, uint32_t a)
{
  x0 *= a; x0 = ((x0 + ((x0 >> 8) & BYTE_1x2MASK) + BYTE_1x2HALF) >> 8) & BYTE_1x2MASK;
}

// x0 = (x0 * a) / 255
// x1 = (x1 * a) / 255
static FOG_INLINE void byte2x2_muldiv255_u(
  byte1x2& x0, byte1x2& x1, uint32_t a)
{
  x0 *= a; x0 = ((x0 + ((x0 >> 8) & BYTE_1x2MASK) + BYTE_1x2HALF) >> 8) & BYTE_1x2MASK;
  x1 *= a; x1 = ((x1 + ((x1 >> 8) & BYTE_1x2MASK) + BYTE_1x2HALF) >> 8) & BYTE_1x2MASK;
}

// x0 = (x0 * a) / 255 + y0
static FOG_INLINE void byte1x2_muldiv255_u_addus_byte1x2(
  byte1x2& x0, uint32_t a, byte1x2 y0)
{
  byte1x2_muldiv255_u(x0, a);
  byte1x2_addus_byte1x2(x0, y0);
}

// x0 = (x0 * a) / 255 + y0
// x1 = (x1 * a) / 255 + y1
static FOG_INLINE void byte2x2_muldiv255_u_addus_byte2x2(
  byte1x2& x0, byte1x2& x1, uint32_t a,
  byte1x2 y0, byte1x2 y1)
{
  byte2x2_muldiv255_u(x0, x1, a);
  byte2x2_addus_byte2x2(x0, x1, y0, y1);
}

//  x0 = (x0 * a + y0 * b) / 255
//  x1 = (x1 * a + y1 * b) / 255
static FOG_INLINE void byte2x2_muldiv255_u_addus_byte2x2_u(
  byte1x2& x0, byte1x2& x1, uint32_t a,
  byte1x2 y0, byte1x2 y1, uint32_t b)
{
  byte2x2_muldiv255_u(x0, x1, a);
  byte2x2_muldiv255_u(y0, y1, b);
  byte2x2_addus_byte2x2(x0, x1, y0, y1);
}

// x0 = (x0 * a0) / 255
static FOG_INLINE void byte1x2_muldiv255_byte1x2(
  byte1x2& x0, byte1x2 a0)
{
  x0 = ((x0 & 0x000000FF) * (a0 & 0x000000FF)) | ((x0 & 0x00FF0000) * ((a0 & 0x00FF0000) >> 16)) ;
  x0 = ((x0 + ((x0 >> 8) & BYTE_1x2MASK) + BYTE_1x2HALF) >> 8) & BYTE_1x2MASK;
}

// x0 = (x0 * a0) / 255
// x1 = (x0 * a1) / 255
static FOG_INLINE void byte2x2_muldiv255_byte2x2(
  byte1x2& x0, byte1x2& x1,
  byte1x2 a0, byte1x2 a1)
{
  x0 = ((x0 & 0x000000FF) * (a0 & 0x000000FF)) | ((x0 & 0x00FF0000) * ((a0 & 0x00FF0000) >> 16)) ;
  x1 = ((x1 & 0x000000FF) * (a1 & 0x000000FF)) | ((x1 & 0x00FF0000) * ((a1 & 0x00FF0000) >> 16)) ;
  x0 = ((x0 + ((x0 >> 8) & BYTE_1x2MASK) + BYTE_1x2HALF) >> 8) & BYTE_1x2MASK;
  x1 = ((x1 + ((x1 >> 8) & BYTE_1x2MASK) + BYTE_1x2HALF) >> 8) & BYTE_1x2MASK;
}

// x0 = (x0 * a0) / 255 + y0
// x1 = (x0 * a1) / 255 + y1
static FOG_INLINE void byte2x2_muldiv255_byte2x2_addus_byte2x2(
  byte1x2& x0, byte1x2& x1,
  byte1x2 a0, byte1x2 a1,
  byte1x2 y0, byte1x2 y1)
{
  byte2x2_muldiv255_byte2x2(x0, x1, a0, a1);
  byte2x2_addus_byte2x2(x0, x1, y0, y1);
}

// ============================================================================
// [Fog::ByteUtil - Byte1x2 - Min / Max]
// ============================================================================

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

// ============================================================================
// [Fog::ByteUtil - Byte1x2 - Shift]
// ============================================================================

static FOG_INLINE void byte1x2_slli(
  byte1x2& x0, uint32_t by)
{
  x0 <<= by;
}

static FOG_INLINE void byte2x2_slli(
  byte1x2& x0, byte1x2& x1, uint32_t by)
{
  x0 <<= by;
  x1 <<= by;
}

static FOG_INLINE void byte1x2_slli_by1(
  byte1x2& x0)
{
  x0 += x0;
}

static FOG_INLINE void byte2x2_slli_by1(
  byte1x2& x0, byte1x2& x1)
{
  x0 += x0;
  x1 += x1;
}

static FOG_INLINE void byte1x2_slli_by1_lo(
  byte1x2& x0)
{
  x0 += (x0 & 0x000000FF);
}

static FOG_INLINE void byte2x2_slli_by1_lo(
  byte1x2& x0, byte1x2& x1)
{
  x0 += (x0 & 0x000000FF);
  x1 += (x1 & 0x000000FF);
}

static FOG_INLINE void byte1x2_slli_by1_hi(
  byte1x2& x0)
{
  x0 += (x0 & 0x00FF0000);
}

static FOG_INLINE void byte2x2_slli_by1_hi(
  byte1x2& x0, byte1x2& x1)
{
  x0 += (x0 & 0x00FF0000);
  x1 += (x1 & 0x00FF0000);
}

static FOG_INLINE void byte1x2_srli(
  byte1x2& x0, uint32_t by)
{
  x0 >>= by;
}

static FOG_INLINE void byte2x2_srli(
  byte1x2& x0, byte1x2& x1, uint32_t by)
{
  x0 >>= by;
  x1 >>= by;
}

// ============================================================================
// [Fog::ByteUtil - Byte1x2 - Negate]
// ============================================================================

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
// [Fog::ByteUtil - Byte1x2 - Interpolate]
// ============================================================================

static FOG_INLINE void byte1x2_interpolate_u_255(
  byte1x2& x0, byte1x2& x1, uint32_t a0,
  byte1x2 y0, byte1x2 y1, uint32_t b0)
{
  x0 = x0 * a0 + y0 * b0;
  x1 = x1 * a0 + y1 * b0;

  x0 = x0 + ((x0 >> 8) & 0x00FF00FF) + 0x00800080;
  x1 = x1 + ((x1 >> 8) & 0x00FF00FF) + 0x00800080;

  x0 &= BYTE_1x2MASK;
  x1 &= BYTE_1x2MASK;
}

static FOG_INLINE void byte1x2_interpolate_u_256(
  byte1x2& x0, byte1x2& x1, uint32_t a0,
  byte1x2 y0, byte1x2 y1, uint32_t b0)
{
  x0 = x0 * a0 + y0 * b0;
  x1 = x1 * a0 + y1 * b0;

  x0 >>= 8;
  x1 >>= 8;

  x0 &= BYTE_1x2MASK;
  x1 &= BYTE_1x2MASK;
}

static FOG_INLINE uint32_t byte1x2_interpolate_u_255_pack_0213(
  byte1x2 x0, byte1x2 x1, uint32_t a0,
  byte1x2 y0, byte1x2 y1, uint32_t b0)
{
  x0 = x0 * a0 + y0 * b0;
  x1 = x1 * a0 + y1 * b0;

  x0 = (x0 + ((x0 >> 8) & 0x00FF00FF) + 0x00800080);
  x1 = (x1 + ((x1 >> 8) & 0x00FF00FF) + 0x00800080) << 8;

  x0 &= 0x00FF00FF;
  x1 &= 0xFF00FF00;

  return x0 | x1;
}

static FOG_INLINE uint32_t byte1x2_interpolate_u_256_pack_0213(
  byte1x2 x0, byte1x2 x1, uint32_t a0,
  byte1x2 y0, byte1x2 y1, uint32_t b0)
{
  x0 = x0 * a0 + y0 * b0;
  x1 = x1 * a0 + y1 * b0;

  x0 >>= 8;

  x0 &= 0x00FF00FF;
  x1 &= 0xFF00FF00;

  return x0 | x1;
}

// ============================================================================
// [Fog::ByteUtil - Byte1x4 - 64-Bit Integer SIMD Library]
// ============================================================================

// These function allows to do SIMD with classic 64-bit integers.
//
// Terminology:
// - byte142 - unsigned 64 bit integer that contains four bytes at mask
//             BYTE_1x4MASK that is 0x00FF00FF00FF00FF.
// - u       - scalar at range 0 to 255 (or 0 to 256 depending to the context).
//
// Pixels are usually encoded in byte1x4 using this formula:
// - byte1x4 b0 (0x00AA00GG00RR00BB)

typedef uint64_t byte1x4;

static const uint64_t BYTE_1x3MASK          = FOG_UINT64_C(0x000000FF00FF00FF);
static const uint64_t BYTE_1x3HALF          = FOG_UINT64_C(0x0000008000800080);
static const uint64_t BYTE_1x3MASK_PLUS_ONE = FOG_UINT64_C(0x0000010010000100);

static const uint64_t BYTE_1x4MASK          = FOG_UINT64_C(0x00FF00FF00FF00FF);
static const uint64_t BYTE_1x4HALF          = FOG_UINT64_C(0x0080008000800080);
static const uint64_t BYTE_1x4MASK_PLUS_ONE = FOG_UINT64_C(0x1000010010000100);

// ============================================================================
// [Fog::ByteUtil - Byte1x4 - From / To / Packing / Unpacking]
// ============================================================================

//! @brief Unpack all bytes in single DWORD into one byte1x4 value.
static FOG_INLINE void byte1x4_unpack_0213(
  byte1x4& x0, uint32_t src_c)
{
  x0 = ((byte1x4)src_c | ((byte1x4)src_c << 24)) & BYTE_1x4MASK;
}

//! @brief Unpack all bytes in single DWORD into one byte1x4 value.
static FOG_INLINE void byte1x4_unpack_021X(
  byte1x4& x0, uint32_t src_c)
{
  x0 = ((byte1x4)src_c | ((byte1x4)src_c << 24)) & BYTE_1x3MASK;
}

//! @brief Pack byte1x4 value to DWORD.
static FOG_INLINE uint32_t byte1x4_pack_0213(byte1x4 x0)
{
  return (uint32_t)(x0 | (x0 >> 24));
}

// ============================================================================
// [Fog::ByteUtil - Scalar-Byte OPS]
// ============================================================================

// Result = (x * y) / 255
static FOG_INLINE uint32_t single_muldiv255(uint32_t x, uint32_t y)
{
  x *= y;
  x = ((x + (x >> 8) + 0x80) >> 8);
  return x;
}

// Result = {(x * a) + (y * (255 - a))} / 255
static FOG_INLINE uint32_t single_lerp255(uint32_t x, uint32_t y, uint32_t a)
{
  x *= a;
  y *= a ^ 0xFF;
  x += y;
  x = ((x + (x >> 8) + 0x80) >> 8);
  return x;
}

// Result = 255 - x
static FOG_INLINE uint32_t single_neg(uint32_t x)
{
  return x ^ 0xFF;
}

// Result = saturate(x + y)
static FOG_INLINE uint32_t single_addus(uint32_t x, uint32_t y)
{
  x += y;
  x |= 0x0100U - ((x >> 8) & 0x00FFU);
  x &= 0x00FFU;
  return x;
}

// Result = x | (x << 8) | (x << 16) | (x << 24)
static FOG_INLINE uint32_t single_expand(uint32_t x)
{
  x |= (x << 8);
  x |= (x << 16);
  return x;
}

// Result = ((x * a) / 255) + ((x * b) / 255)
static FOG_INLINE uint32_t single_mulab_add(uint32_t x, uint32_t a, uint32_t b)
{
  uint32_t t0 = a | (b << 16);
  t0 = ((t0 + ((t0 >> 8) & 0x00FF00FF) + 0x00800080) >> 8);

  return (t0 & 0xFF) + (t0 >> 16);
}

// ============================================================================
// [Fog::ByteUtil - Packed-Byte OPS]
// ============================================================================

// NOTE: These functions are included here, because there are somethimes useful,
// but note that each function here needs first to unpack input values, do
// operation and them pack result back. This can be in some situations overhead
// and you should consider to use pure byte1x2 or byte1x4 solution instead.

// x_c' = (x_c * a) / 255
static FOG_INLINE uint32_t packed_muldiv255(uint32_t x, uint32_t a)
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

// x_c' = x_c * a
// x_a' = 0xFF
static FOG_INLINE uint32_t packed_muldiv255_Fxxx(uint32_t x, uint32_t a)
{
#if FOG_ARCH_BITS == 64
  // TODO: I'm not sure about this.
  uint64_t x0 = ((uint64_t)x | ((uint64_t)x << 24)) & FOG_UINT64_C(0x000000FF00FF00FF);
  x0 *= a;
  x0 = (x0 + ((x0 >> 8) & FOG_UINT64_C(0x000000FF00FF00FF)) + FOG_UINT64_C(0xFF00008000800080)) >> 8;
  x0 &= FOG_UINT64_C(0x00FF00FF00FF00FF);
  return (uint32_t)(x0 | (x0 >> 24));
#else
  // TODO: I'm not sure about this.
  uint32_t t0 = ((x & 0x00FF00FF)     ) * a;
  uint32_t t1 = ((x & 0x0000FF00) >> 8) * a;

  x  = ((t0 + ((t0 >> 8) & 0x00FF00FF) + 0x00800080) >> 8) & 0x00FF00FF;
  x |= ((t1 + ((t1 >> 8) & 0x00FF00FF) + 0xFF000080)     ) & 0xFF00FF00;

  return x;
#endif
}

// x_c' = x_c * a
// x_a' = 0x00
static FOG_INLINE uint32_t packed_muldiv255_0xxx(uint32_t x, uint32_t a)
{
#if FOG_ARCH_BITS == 64
  // TODO: I'm not sure about this.
  uint64_t x0 = ((uint64_t)x | ((uint64_t)x << 24)) & FOG_UINT64_C(0x000000FF00FF00FF);
  x0 *= a;
  x0 = (x0 + ((x0 >> 8) & FOG_UINT64_C(0x000000FF00FF00FF)) + FOG_UINT64_C(0x0000008000800080)) >> 8;
  x0 &= FOG_UINT64_C(0x000000FF00FF00FF);
  return (uint32_t)(x0 | (x0 >> 24));
#else
  uint32_t t0 = ((x & 0x00FF00FF)     ) * a;
  uint32_t t1 = ((x & 0x0000FF00) >> 8) * a;

  x  = ((t0 + ((t0 >> 8) & 0x00FF00FF) + 0x00800080) >> 8) & 0x00FF00FF;
  x |= ((t1 + ((t1 >> 8) & 0x000000FF) + 0x00000080)     ) & 0x0000FF00;

  return x;
#endif
}

// x_c' = min(x_c + y_c, 255)
static FOG_INLINE uint32_t packed_addus(uint32_t x, uint32_t y)
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

// x_c' = (x_c * a) / 255 + y
static FOG_INLINE uint32_t packed_muldiv255_addus(uint32_t x, uint32_t a, uint32_t y)
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

// TODO: Better name, audit
static FOG_INLINE uint32_t packed_addmul(
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

// TODO: Better name, audit
static FOG_INLINE uint32_t packed_addmul_div256(
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

} // ByteUtil namespace
} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_BYTEUTIL_H
