// [Fog library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_RASTER_H
#define _FOG_GRAPHICS_RASTER_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Graphics/ImageFormat.h>
#include <Fog/Graphics/Rgba.h>

namespace Fog {
namespace Raster {

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

//! @brief Accurate division of 255. The result is equal to <code>(val / 255)</code>.
//!
//! @note This template is used for accure alpha blends.
template<typename T>
static FOG_INLINE T Div255(T i)
{
    return (T)( (((uint32_t)i << 8U) + ((uint32_t)i + 256U)) >> 16U );
}

//! @brief Accurate division of 255. The result is equal to <code>(val / 256)</code>.
template<typename T>
static FOG_INLINE T Div256(T i)
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
// [Fog::Raster - Get Red/Green/Blue/Alpha]
// ============================================================================

static FOG_INLINE uint32_t getRed(uint32_t c)
{
  return (c >> ImageFormat::ARGB32_RShift);
}

static FOG_INLINE uint32_t getGreen(uint32_t c)
{
  return (c >> ImageFormat::ARGB32_GShift);
}

static FOG_INLINE uint32_t getBlue(uint32_t c)
{
  return (c >> ImageFormat::ARGB32_BShift);
}

static FOG_INLINE uint32_t getAlpha(uint32_t c)
{
  return (c >> ImageFormat::ARGB32_AShift);
}

// ============================================================================
// [Fog::Raster - FromRGBA]
// ============================================================================

static FOG_INLINE uint32_t fromRGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF)
{
  return ((uint32_t)r << ImageFormat::ARGB32_RShift) |
         ((uint32_t)g << ImageFormat::ARGB32_GShift) |
         ((uint32_t)b << ImageFormat::ARGB32_BShift) |
         ((uint32_t)a << ImageFormat::ARGB32_AShift) ;
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
  return (uint8_t)((
    ((c & ImageFormat::ARGB32_RMask) >> ImageFormat::ARGB32_RShift) *  77U +
    ((c & ImageFormat::ARGB32_GMask) >> ImageFormat::ARGB32_GShift) * 150U +
    ((c & ImageFormat::ARGB32_BMask) >> ImageFormat::ARGB32_BShift) *  29U ) >> 8U);
}

// ============================================================================
// [Fog::Raster - Premultiply / Demultiply]
// ============================================================================

static FOG_INLINE uint32_t premultiply(uint32_t x)
{
  uint32_t a = getAlpha(x);
  uint32_t t = (x & 0x00FF00FF) * a;

  t = (t + ((t >> 8) & 0x00FF00FF) + 0x00800080) >> 8;
  t &= 0x00FF00FF;

  x = ((x >> 8) & 0x000000FF) * a;
  x = (x + ((x >> 8) & 0x000000FF) + 0x00000080);
  x &= 0x0000FF00;
  x |= t | (a << 24);

  return x;
}

static FOG_INLINE uint32_t demultiply(uint32_t x)
{
  uint32_t a = getAlpha(x);

  return (a) 
    ? (((((x & 0x00FF0000) >> 16) * 255) / a) << 16) |
      (((((x & 0x0000FF00) >>  8) * 255) / a) <<  8) |
      (((((x & 0x000000FF)      ) * 255) / a)      ) |
      (a << 24)
    : 0;
}

// ============================================================================
// [Fog::Raster - Fetch/Write ARGB32]
// ============================================================================

static FOG_INLINE uint32_t fetchARGB32(const uint8_t* p)
{
  return ((const uint32_t *)p)[0];
}

static FOG_INLINE void writeARGB32(uint8_t* p, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
  ((uint32_t*)p)[0] = 
    ((uint32_t)r << ImageFormat::ARGB32_RShift) |
    ((uint32_t)g << ImageFormat::ARGB32_GShift) |
    ((uint32_t)b << ImageFormat::ARGB32_BShift) |
    ((uint32_t)a << ImageFormat::ARGB32_AShift) ;
}

static FOG_INLINE void writeARGB32(uint8_t* p, uint32_t s0)
{
  ((uint32_t *)p)[0] = s0;
}

// ============================================================================
// [Fog::Raster - Fetch/Write RGB24]
// ============================================================================

static FOG_INLINE uint32_t fetchRGB24(const uint8_t* p)
{
  return Fog::Rgba(
    p[ImageFormat::RGB24_RBytePos],
    p[ImageFormat::RGB24_GBytePos],
    p[ImageFormat::RGB24_BBytePos]);
}

static FOG_INLINE void writeRGB24(uint8_t* p, uint8_t r, uint8_t g, uint8_t b)
{
  p[ImageFormat::RGB24_RBytePos] = r;
  p[ImageFormat::RGB24_GBytePos] = g;
  p[ImageFormat::RGB24_BBytePos] = b;
}

static FOG_INLINE void writeRGB24(uint8_t* p, uint32_t s0)
{
  writeRGB24(p,
    Rgba::red(s0),
    Rgba::green(s0),
    Rgba::blue(s0));
}

// ============================================================================
// [Fog::Raster - Fetch/Write BGR24]
// ============================================================================

static FOG_INLINE uint32_t fetchBGR24(const uint8_t* p)
{
  return Fog::Rgba(
    p[ImageFormat::BGR24_RBytePos], 
    p[ImageFormat::BGR24_GBytePos], 
    p[ImageFormat::BGR24_BBytePos]);
}

static FOG_INLINE void writeBGR24(uint8_t* p, uint8_t r, uint8_t g, uint8_t b)
{
  p[ImageFormat::BGR24_RBytePos] = r;
  p[ImageFormat::BGR24_GBytePos] = g;
  p[ImageFormat::BGR24_BBytePos] = b;
}

static FOG_INLINE void writeBGR24(uint8_t* p, uint32_t s0)
{
  writeBGR24(p,
    Rgba::red(s0),
    Rgba::green(s0),
    Rgba::blue(s0));
}

} // Raster namespace
} // Fog namespace

// [Guard]
#endif // _FOG_GRAPHICS_RASTER_H
