// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_RGBA_H
#define _FOG_GRAPHICS_RGBA_H

// [Dependencies]
#include <Fog/Core/Memory.h>
#include <Fog/Core/Vector.h>

namespace Fog {

// ============================================================================
// [Fog::Rgba]
// ============================================================================

#include <Fog/Core/Pack.h>
struct FOG_PACKED Rgba
{
  // [Enums]

  enum { Size = 4 };

  // [Members]

  union
  {
    struct
    {
#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
      uint8_t b;
      uint8_t g;
      uint8_t r;
      uint8_t a;
#else
      uint8_t a;
      uint8_t r;
      uint8_t g;
      uint8_t b;
#endif // FOG_BYTE_ORDER
    };
    uint32_t i;
  };

  // [Constants]

  static const uint32_t RedMask      = 0x00FF0000U;
  static const uint32_t GreenMask    = 0x0000FF00U;
  static const uint32_t BlueMask     = 0x000000FFU;
  static const uint32_t AlphaMask    = 0xFF000000U;
  static const uint32_t RgbMask      = 0x00FFFFFFU;

  static const uint32_t RedShift     = 16U;
  static const uint32_t GreenShift   = 8U;
  static const uint32_t BlueShift    = 0U;
  static const uint32_t AlphaShift   = 24U;

#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
  static const uint32_t RedBytePos   = 2;
  static const uint32_t GreenBytePos = 1;
  static const uint32_t BlueBytePos  = 0;
  static const uint32_t AlphaBytePos = 3;
#else
  static const uint32_t RedBytePos   = 1;
  static const uint32_t GreenBytePos = 2;
  static const uint32_t BlueBytePos  = 3;
  static const uint32_t AlphaBytePos = 0;
#endif // FOG_BYTE_ORDER

  // [Construction / Destruction]

  FOG_INLINE Rgba() {}
  FOG_INLINE Rgba(uint32_t _i) : i(_i) {}
  FOG_INLINE Rgba(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a = 0xFF) :
    i(
      ((uint32_t)(_r) << RedShift  ) |
      ((uint32_t)(_g) << GreenShift) |
      ((uint32_t)(_b) << BlueShift ) |
      ((uint32_t)(_a) << AlphaShift) )
  {
  }
  FOG_INLINE Rgba(const Rgba& rgba) :
    i(rgba.i)
  {
  }

  // [Set]

  FOG_INLINE Rgba& set(const Rgba& rgba)
  { i = rgba.i; return* this; }
  FOG_INLINE Rgba& set(uint32_t rgba)
  { i = rgba; return* this; }
  FOG_INLINE Rgba& set(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a = 0xFF)
  { 
    i = (
      ((uint32_t)(_r) << RedShift  ) |
      ((uint32_t)(_g) << GreenShift) |
      ((uint32_t)(_b) << BlueShift ) |
      ((uint32_t)(_a) << AlphaShift) );
    return* this;
  }

  // [Channels]

  FOG_INLINE uint32_t red  () const { return r; }
  FOG_INLINE uint32_t gren () const { return g; }
  FOG_INLINE uint32_t blue () const { return b; }
  FOG_INLINE uint32_t alpha() const { return a; }
  FOG_INLINE uint32_t grey () const { return grey(i); }

  // [Swap]

  FOG_INLINE void swapRgb()
  { uint8_t t = r; r = g; g = t; }

  FOG_INLINE void swapRgba() 
  { i = Memory::bswap32(i); }

  // [Overloaded Operators]

  FOG_INLINE operator uint32_t() const { return i; }

  FOG_INLINE Rgba& operator=(const Rgba& rgba) { return set(rgba); }
  FOG_INLINE Rgba& operator=(uint32_t rgba) { return set(rgba); }

  FOG_INLINE Rgba& operator|=(uint32_t _i) { i |= _i; return *this; }
  FOG_INLINE Rgba& operator&=(uint32_t _i) { i &= _i; return *this; }
  FOG_INLINE Rgba& operator^=(uint32_t _i) { i ^= _i; return *this; }
  FOG_INLINE Rgba& operator+=(uint32_t _i) { i += _i; return *this; }
  FOG_INLINE Rgba& operator-=(uint32_t _i) { i -= _i; return *this; }
  FOG_INLINE Rgba& operator*=(uint32_t _i) { i *= _i; return *this; }
  FOG_INLINE Rgba& operator/=(uint32_t _i) { i /= _i; return *this; }

  // [Static]

  static FOG_INLINE uint32_t red  (uint32_t rgba) { return (rgba & RedMask  ) >> RedShift  ; }
  static FOG_INLINE uint32_t green(uint32_t rgba) { return (rgba & GreenMask) >> GreenShift; }
  static FOG_INLINE uint32_t blue (uint32_t rgba) { return (rgba & BlueMask ) >> BlueShift ; }
  static FOG_INLINE uint32_t alpha(uint32_t rgba) { return (rgba & AlphaMask) >> AlphaShift; }

  static FOG_INLINE uint8_t grey(uint32_t rgba)
  {
    return (
      red  (rgba) *  77U +
      green(rgba) * 150U +
      blue (rgba) *  29U ) >> 8U;
  }

  static FOG_INLINE uint32_t half(uint32_t color1, uint32_t color2)
  {
    return (((((color1) ^ (color2)) & 0xFEFEFEFEU) >> 1) + ((color1) & (color2)));
  }

  static FOG_INLINE uint32_t add(uint32_t color1, uint32_t color2)
  {
#if defined(FOG_HARDCODE_SSE2)
    __m128i xmm0 = _mm_cvtsi32_si128((int)color1);
    __m128i xmm1 = _mm_cvtsi32_si128((int)(color2 & 0x00FFFFFF));
    return _mm_cvtsi128_si32(_mm_adds_epu8(xmm0, xmm1));
#else
    register uint32_t c13 = (color1 & 0x00FF00FF) + (color2 & 0x00FF00FF);
    register uint32_t c24 = (color1 & 0xFF00FF00) + (color2 & 0x0000FF00);

    if (c13 & 0xFF000000) c13 |= 0x00FF0000;
    if (c24 & 0x00FF0000) c24 |= 0x0000FF00;
    if (c13 & 0x0000FF00) c13 |= 0x000000FF;

    return (c13 & 0x00FF00FF) | (c24 & 0xFF00FF00);
#endif // FOG_HARDCODE_SSE2
  }

  static FOG_INLINE uint32_t sub(uint32_t color1, uint32_t color2)
  {
#if defined(FOG_HARDCODE_SSE2)
    __m128i xmm0 = _mm_cvtsi32_si128((int)color1);
    __m128i xmm1 = _mm_cvtsi32_si128((int)(color2 & 0x00FFFFFF));
    return _mm_cvtsi128_si32(_mm_subs_epu8(xmm0, xmm1));
#else
    register uint32_t c1 = (color1 & 0x00FF0000) - (color2 & 0x00FF0000);
    register uint32_t c2 = (color1 & 0x0000FF00) - (color2 & 0x0000FF00);
    register uint32_t c3 = (color1 & 0x000000FF) - (color2 & 0x000000FF);

    if (c1 & 0xFF00FFFF) c1 = 0;
    if (c2 & 0xFFFF00FF) c2 = 0;
    if (c3 & 0xFFFFFF00) c3 = 0;

    return (color1 & 0xFF000000) | c1 | c2 | c3;
#endif // FOG_HARDCODE_SSE2
  }

  static FOG_INLINE uint32_t make(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xFF)
  {
    return
      ((uint32_t)(r) << RedShift  ) |
      ((uint32_t)(g) << GreenShift) |
      ((uint32_t)(b) << BlueShift ) |
      ((uint32_t)(a) << AlphaShift) ;
  }
};
#include <Fog/Core/Unpack.h>

// ============================================================================
// [Fog::Rgba64]
// ============================================================================

#include <Fog/Core/Pack.h>
struct Rgba64
{
  // [Enums]

  enum { Size = 8 };

  // [Members]

  union
  {
    struct
    {
#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
      uint16_t a;
      uint16_t r;
      uint16_t g;
      uint16_t b;
#else
      uint16_t b;
      uint16_t g;
      uint16_t r;
      uint16_t a;
#endif // FOG_BYTE_ORDER
    };
    uint64_t i;
  };

  // [Constants]

  static const uint64_t RedMask      = FOG_UINT64_C(0x0000FFFF00000000);
  static const uint64_t GreenMask    = FOG_UINT64_C(0x00000000FFFF0000);
  static const uint64_t BlueMask     = FOG_UINT64_C(0x000000000000FFFF);
  static const uint64_t AlphaMask    = FOG_UINT64_C(0xFFFF000000000000);
  static const uint64_t RgbMask      = FOG_UINT64_C(0x0000FFFFFFFFFFFF);

  static const uint64_t RedShift     = FOG_UINT64_C(32);
  static const uint64_t GreenShift   = FOG_UINT64_C(16);
  static const uint64_t BlueShift    = FOG_UINT64_C(0 );
  static const uint64_t AlphaShift   = FOG_UINT64_C(48);

#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
  static const uint32_t RedWordPos   = 2;
  static const uint32_t GreenWordPos = 1;
  static const uint32_t BlueWordPos  = 0;
  static const uint32_t AlphaWordPos = 3;
#else
  static const uint32_t RedWordPos   = 1;
  static const uint32_t GreenWordPos = 2;
  static const uint32_t BlueWordPos  = 3;
  static const uint32_t AlphaWordPos = 0;
#endif // FOG_BYTE_ORDER

  // [Construction / Destruction]

  FOG_INLINE Rgba64() {}
  FOG_INLINE Rgba64(uint64_t _i) : i(_i) {}
  FOG_INLINE Rgba64(uint16_t _r, uint16_t _g, uint16_t _b, uint16_t _a = 0xFFFF)
  { set(_r, _g, _b, _a); }
  FOG_INLINE Rgba64(const Rgba64& other)
  { set(other); }
  FOG_INLINE explicit Rgba64(const Rgba& other)
  { set(other); }

  // [Set]

  FOG_INLINE Rgba64& set(uint64_t rgba)
  { i = rgba; return* this; }

  FOG_INLINE Rgba64& set(uint16_t _r, uint16_t _g, uint16_t _b, uint16_t _a = 0xFFFF)
  { 
    r = _r;
    g = _g;
    b = _b;
    a = _a;
    return* this;
  }

  FOG_INLINE Rgba64& set(const Rgba64& rgba)
  { i = rgba.i; return* this; }

  FOG_INLINE Rgba64& set(const Rgba& other)
  {
    r = (uint16_t)other.r << 8 | (uint16_t)other.r;
    g = (uint16_t)other.g << 8 | (uint16_t)other.g;
    b = (uint16_t)other.b << 8 | (uint16_t)other.b;
    a = (uint16_t)other.a << 8 | (uint16_t)other.a;

    return *this;
  }

  // [Channels]

  FOG_INLINE uint16_t red  () const { return r; }
  FOG_INLINE uint16_t gren () const { return g; }
  FOG_INLINE uint16_t blue () const { return b; }
  FOG_INLINE uint16_t alpha() const { return a; }
  FOG_INLINE uint16_t grey () const { return grey(i); }

  // [Swap]

  FOG_INLINE void swapRgb()
  {
    uint16_t t = r;
    r = g; 
    g = t; 
  }

  FOG_INLINE void swapRgba()
  {
    uint16_t t0 = b;
    uint16_t t1 = g;
    b = a;
    g = r;
    r = t1;
    a = t0;
  }

  // [Overloaded Operators]

  // FOG_INLINE operator uint32_t() const { return i; }
  FOG_INLINE operator uint64_t() const { return i; }

  FOG_INLINE Rgba64& operator=(const Rgba64& rgba) { return set(rgba); }
  FOG_INLINE Rgba64& operator=(uint64_t rgba) { return set(rgba); }

  FOG_INLINE Rgba64& operator|=(uint64_t _i) { i |= _i; return *this; }
  FOG_INLINE Rgba64& operator&=(uint64_t _i) { i &= _i; return *this; }
  FOG_INLINE Rgba64& operator^=(uint64_t _i) { i ^= _i; return *this; }
  FOG_INLINE Rgba64& operator+=(uint64_t _i) { i += _i; return *this; }
  FOG_INLINE Rgba64& operator-=(uint64_t _i) { i -= _i; return *this; }
  FOG_INLINE Rgba64& operator*=(uint64_t _i) { i *= _i; return *this; }
  FOG_INLINE Rgba64& operator/=(uint64_t _i) { i /= _i; return *this; }

  // [Static]

  static FOG_INLINE uint16_t red  (uint64_t rgba) { return (uint16_t)((rgba & RedMask  ) >> RedShift  ); }
  static FOG_INLINE uint16_t green(uint64_t rgba) { return (uint16_t)((rgba & GreenMask) >> GreenShift); }
  static FOG_INLINE uint16_t blue (uint64_t rgba) { return (uint16_t)((rgba & BlueMask ) >> BlueShift ); }
  static FOG_INLINE uint16_t alpha(uint64_t rgba) { return (uint16_t)((rgba & AlphaMask) >> AlphaShift); }

  static FOG_INLINE uint16_t grey(uint64_t rgba)
  {
    return (
      (uint32_t)(red  (rgba)) *  77U +
      (uint32_t)(green(rgba)) * 150U +
      (uint32_t)(blue (rgba)) *  29U ) >> 8U;
  }

  static FOG_INLINE uint64_t half(uint64_t color1, uint64_t color2)
  {
    return (((((color1) ^ (color2)) & FOG_UINT64_C(0xFEFEFEFEFEFEFEFE)) >> 1) + ((color1) & (color2)));
  }

  static FOG_INLINE uint64_t make(uint16_t r, uint16_t g, uint16_t b, uint16_t a = 0xFFFF)
  {
    return
      ((uint64_t)(r) << RedShift  ) |
      ((uint64_t)(g) << GreenShift) |
      ((uint64_t)(b) << BlueShift ) |
      ((uint64_t)(a) << AlphaShift) ;
  }
};
#include <Fog/Core/Unpack.h>

} // Fog namespace

FOG_DECLARE_TYPEINFO(Fog::Rgba, Fog::PrimitiveType)
FOG_DECLARE_TYPEINFO(Fog::Rgba64, Fog::PrimitiveType)

// [Guard]
#endif // _FOG_GRAPHICS_RGBA_H
