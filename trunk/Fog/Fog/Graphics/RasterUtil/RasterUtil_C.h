// [Fog/Graphics library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_RASTER_C_H
#define _FOG_GRAPHICS_RASTER_C_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Graphics/Argb.h>
#include <Fog/Graphics/ArgbUtil.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/RasterUtil.h>

//! @addtogroup Fog_Graphics_Raster
//! @{

// This file is included from all Fog/Graphics/RasterUtil/RasterUtil_.h includes and
// .cpp files, so in future it may contain generic code for these modules.

namespace Fog {
namespace RasterUtil {

// ============================================================================
// [Fog::Raster - Masks]
// ============================================================================

// [Byte Position]
// TODO: Duplicated, see Constants.h
enum RGB32_BytePos
{
#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
  RGB32_RByte = 2,
  RGB32_GByte = 1,
  RGB32_BByte = 0,
  RGB32_AByte = 3
#else // FOG_BYTE_ORDER == FOG_BIG_ENDIAN
  RGB32_RByte = 1,
  RGB32_GByte = 2,
  RGB32_BByte = 3,
  RGB32_AByte = 0
#endif // FOG_BYTE_ORDER
};

enum RGB24_BytePos
{
#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
  RGB24_RByte = 2,
  RGB24_GByte = 1,
  RGB24_BByte = 0
#else // FOG_BYTE_ORDER == FOG_BIG_ENDIAN
  RGB24_RByte = 0,
  RGB24_GByte = 1,
  RGB24_BByte = 2
#endif // FOG_BYTE_ORDER
};

enum BGR24_BytePos
{
  BGR24_RByte = 2 - RGB24_RByte,
  BGR24_GByte = 2 - RGB24_GByte,
  BGR24_BByte = 2 - RGB24_BByte
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

// TODO: Ideally move to Fog::Memory...

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

// TODO: Ideally move to Fog::Memory...

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

} // RasterUtil namespace
} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_RASTER_C_H
