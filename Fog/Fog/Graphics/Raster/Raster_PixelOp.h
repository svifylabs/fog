// [Fog/Graphics library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_RASTER_PIXELOP_H
#define _FOG_GRAPHICS_RASTER_PIXELOP_H

// [Dependencies]
#include <Fog/Graphics/Raster/Raster_C.h>
#include <Fog/Graphics/Raster/Raster_ByteOp.h>

namespace Fog {
namespace Raster {

// ============================================================================
// [Fog::Raster - Demultiply Reciprocal Table]
// ============================================================================

extern FOG_API uint32_t const demultiply_reciprocal_table[256];


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

  return grey >> 16;
}

// ============================================================================
// [Fog::Raster - Premultiply / Demultiply]
// ============================================================================

static FOG_INLINE uint32_t premultiply(uint32_t x)
{
#if FOG_ARCH_BITS == 64
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
  uint32_t recip = demultiply_reciprocal_table[a];

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
    return ((uint32_t)p[RGB24_RByte] << RGB32_RShift) |
           ((uint32_t)p[RGB24_GByte] << RGB32_GShift) |
           ((uint32_t)p[RGB24_BByte] << RGB32_BShift) ;
  }

  static FOG_INLINE uint32_t fetchAlpha(const uint8_t* p)
  {
    return 0xFF;
  }

  static FOG_INLINE void store(uint8_t* p, uint32_t s0)
  {
    p[RGB24_RByte] = (uint8_t)(s0 >> RGB32_RShift);
    p[RGB24_GByte] = (uint8_t)(s0 >> RGB32_GShift);
    p[RGB24_BByte] = (uint8_t)(s0 >> RGB32_BShift);
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
    return 0xFF;
  }

  static FOG_INLINE void store(uint8_t* p, uint32_t s0)
  {
    ((uint16_t*)p)[0] =
      ((s0 >> 3) & 0x001F) |
      ((s0 >> 6) & 0x03E0) |
      ((s0 >> 9) & 0x7C00) ;
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
    return 0xFF;
  }

  static FOG_INLINE void store(uint8_t* p, uint32_t s0)
  {
    ((uint16_t*)p)[0] = Memory::bswap16(
      ((s0 >> 3) & 0x001F) |
      ((s0 >> 6) & 0x03E0) |
      ((s0 >> 9) & 0x7C00) );
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
    return 0xFF;
  }

  static FOG_INLINE void store(uint8_t* p, uint32_t s0)
  {
    ((uint16_t*)p)[0] =
      ((s0 >> 3) & 0x001F) |
      ((s0 >> 5) & 0x07E0) |
      ((s0 >> 8) & 0xF800) ;
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
    return 0xFF;
  }

  static FOG_INLINE void store(uint8_t* p, uint32_t s0)
  {
    ((uint16_t*)p)[0] = Memory::bswap16(
      ((s0 >> 3) & 0x001F) |
      ((s0 >> 5) & 0x07E0) |
      ((s0 >> 8) & 0xF800) );
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

// [Guard]
#endif // _FOG_GRAPHICS_PIXELOP_H
