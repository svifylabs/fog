// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Memory.h>
#include <Fog/Core/Static.h>
#include <Fog/Graphics/Dither.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/Converter.h>
#include <Fog/Graphics/Raster.h>
#include <Fog/Graphics/Rgba.h>

namespace Fog {

// [Converters]

static const Converter::Format converter_formats[] =
{
  //Depth                Flags            Red mask    Green mask  Blue mask   Alpha mask
  //-----|------------------------------|-----------|-----------|-----------|------------|
  { 32   ,0                             , 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000 },
  { 32   ,Converter::Premultiplied      , 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000 },
  { 32   ,0                             , 0x00FF0000, 0x0000FF00, 0x000000FF, 0x00000000 },

  { 24   ,0                             , 0x00FF0000, 0x0000FF00, 0x000000FF, 0x00000000 },
  { 24   ,0                             , 0x000000FF, 0x0000FF00, 0x00FF0000, 0x00000000 },

  { 8    ,Converter::Indexed            , 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
  { 8    ,0                             , 0x00000000, 0x00000000, 0x00000000, 0x000000FF },
  { 1    ,0                             , 0x00000000, 0x00000000, 0x00000000, 0x00000001 },

  { 32   ,0                             , 0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF },
  { 32   ,Converter::Premultiplied      , 0x0000FF00, 0x00FF0000, 0xFF000000, 0x000000FF },
  { 32   ,0                             , 0x0000FF00, 0x00FF0000, 0xFF000000, 0x00000000 },

  { 16   ,0                             , 0x0000F800, 0x000007E0, 0x0000001F, 0x00000000 },
  { 16   ,Converter::ByteSwap           , 0x0000F800, 0x000007E0, 0x0000001F, 0x00000000 },

  { 16   ,0                             , 0x00007C00, 0x000003E0, 0x0000001F, 0x00000000 },
  { 16   ,Converter::ByteSwap           , 0x00007C00, 0x000003E0, 0x0000001F, 0x00000000 },

  { 16   ,0                             , 0x00007C00, 0x000003E0, 0x0000001F, 0x00008000 },
  { 16   ,Converter::ByteSwap           , 0x00007C00, 0x000003E0, 0x0000001F, 0x00008000 },

  { 4    ,Converter::Indexed            , 0x00000000, 0x00000000, 0x00000000, 0x00000000 },
  { 1    ,Converter::Indexed            , 0x00000000, 0x00000000, 0x00000000, 0x00000000 },

  { 8    ,Converter::Indexed            , 0x00000060, 0x0000001C, 0x00000003, 0x00000000 },
  { 8    ,Converter::Indexed            , 0x00000030, 0x0000000C, 0x00000003, 0x00000000 },
  { 8    ,Converter::Indexed            , 0x00000004, 0x00000002, 0x00000001, 0x00000000 },

  { 4    ,0                             , 0x00000000, 0x00000000, 0x00000000, 0x0000000F }
};

// built-in devices [NxN array]
static void* converter_builtin
  [Converter::Count]
  [Converter::Count];

// [Table]

static Converter::SpanFunc converter_converters
  [Converter::Count]
  [Converter::Count];

// [Macros]

#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
#define __FOG_CONV_16_BPP_0 0
#define __FOG_CONV_16_BPP_1 1
#else
#define __FOG_CONV_16_BPP_0 1
#define __FOG_CONV_16_BPP_1 0
#endif

// [Converters - Copy]

static void FOG_OPTIMIZEDCALL Conv_Copy_32_C(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + Raster::mul4(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul4(params->srcX);

  sysuint_t x;
  sysuint_t width = params->width;

  for (x = width >> 2; x; x--, dest += 16, src += 16)
  {
    Raster::copy16(dest, src);
  }

  for (x = width & 3; x; x--, dest += 4, src += 4)
  {
    Raster::copy4(dest, src);
  }
}

static void FOG_OPTIMIZEDCALL Conv_Copy_32_Swap_C(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + Raster::mul4(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul4(params->srcX);

  sysuint_t x;
  sysuint_t width = params->width;

  for (x = width; x; x--, dest += 4, src += 4)
  {
    ((uint32_t *)dest)[0] = Memory::bswap32(((const uint32_t *)src)[0]);
  }
}

static void FOG_OPTIMIZEDCALL Conv_Copy_24_C(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + Raster::mul3(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul3(params->srcX);

  sysuint_t x;
  // width becomes 'count of bytes to copy'
  sysuint_t width = Raster::mul3(params->width);

  // simple reject
  if (width == 0) return;

  // this core is correct, width was multiplied by 3 and checked for 0,
  // so it will never be lower than 3
  sysuint_t align = ((sysuint_t)dest & 0x3);
  for (x = align, width -= align; x; x--) *dest++ = *src++;

  for (x = width >> 4, width &= 0xF; x; x--)
  {
    Raster::copy16(dest, src);
    dest += 16;
    src += 16;
  }

  for (x = width >> 2, width &= 0x3; x; x--)
  {
    Raster::copy4(dest, src);
    dest += 4;
    src += 4;
  }

  for (x = width; x; x--)
  {
    *dest++ = *src++;
  }
}

static void FOG_OPTIMIZEDCALL Conv_Copy_24_Swap_C(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + Raster::mul3(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul3(params->srcX);

  sysuint_t x;
  // width becomes 'count of bytes to copy'
  sysuint_t width = Raster::mul3(params->width);

  for (x = width; x; x--, dest += 3, src += 3)
  {
    dest[0] = src[2];
    dest[1] = src[1];
    dest[2] = src[0];
  }
}

static void FOG_OPTIMIZEDCALL Conv_Copy_16_C(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + Raster::mul2(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul2(params->srcX);

  sysuint_t x;
  // width becomes 'count of bytes to copy'
  sysuint_t width = Raster::mul2(params->width);

  sysuint_t align = ((sysuint_t)dest & 0x3);
  if (align > width) align = width;
  for (x = align, width -= align; x; x--) *dest++ = *src++;

  for (x = width >> 4, width &= 0xF; x; x--)
  {
    Raster::copy16(dest, src);
    dest += 16;
    src += 16;
  }

  for (x = width >> 2, width &= 0x3; x; x--)
  {
    Raster::copy4(dest, src);
    dest += 4;
    src += 4;
  }

  for (x = width; x; x--)
  {
    *dest++ = *src++;
  }
}

static void FOG_OPTIMIZEDCALL Conv_Copy_16_Swap_C(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + Raster::mul2(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul2(params->srcX);

  sysuint_t x;
  sysuint_t width = params->width;

  for (x = width; x; x--)
  {
    ((uint16_t *)dest)[0] = Memory::bswap16(((uint16_t *)src)[0]);
    dest += 2;
    src += 2;
  }
}

static void FOG_OPTIMIZEDCALL Conv_Copy_8_C(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + Raster::mul2(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul2(params->srcX);

  sysuint_t x;
  sysuint_t width = params->width;

  sysuint_t align = ((sysuint_t)dest & 0x3);
  if (align > width) align = width;

  for (x = align, width -= align; x; x--)
  {
    *dest++ = *src++;
  }

  for (x = width >> 4, width &= 0xF; x; x--)
  {
    Raster::copy32(dest, src);
    dest += 32;
    src += 32;
  }

  for (x = width >> 2, width &= 0x3; x; x--)
  {
    Raster::copy4(dest, src);
    dest += 4;
    src += 4;
  }

  for (x = width; x; x--)
  {
    *dest++ = *src++;
  }
}

// [XRGB32 <- RGB24]

template<uint32_t __Mask>
static void FOG_OPTIMIZEDCALL Conv_XRGB32_From_RGB24(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + Raster::mul4(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul3(params->srcX);

  sysuint_t x = params->width;

  // align
  while (((sysuint_t)src & 0x3) != 0 && x)
  {
    ((uint32_t *)dest)[0] = __Mask |
      (src[ImageFormat::RGB24_RBytePos] << 16) |
      (src[ImageFormat::RGB24_GBytePos] <<  8) |
      (src[ImageFormat::RGB24_BBytePos]      ) ;

    dest += 4;
    src += 3;
    x--;
  }

  // 4 pixels at time
  while (x >= 4)
  {
    uint32_t c0_1231 = ((const uint32_t *)src)[0];
    uint32_t c1_2312 = ((const uint32_t *)src)[1];
    uint32_t c2_3123 = ((const uint32_t *)src)[2];

#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
    // ByteOrder: 0123 0123 0123 0123
    // Source   : BGRB GRBG RBGR
    // Target   : BGRX BGRX BGRX BGRX
    ((uint32_t *)dest)[0] = __Mask | ((c0_1231) & 0x00FFFFFF);
    ((uint32_t *)dest)[1] = __Mask | ((c0_1231) >> 24) | ((c1_2312 & 0x0000FFFF) <<  8);
    ((uint32_t *)dest)[2] = __Mask | ((c1_2312) >> 16) | ((c2_3123 & 0x000000FF) << 16);
    ((uint32_t *)dest)[3] = __Mask | ((c2_3123) >>  8);
#else // FOG_BIG_ENDIAN
    // ByteOrder: 3210 3210 3210 3210
    // Source   : RGBR GBRG BRGB
    // Target   : XRGB XRGB XRGB XRGB
    TODO();
#endif // FOG_BYTE_ORDER

    dest += 16;
    src += 12;
    x -= 4;
  }

  // tail
  while (x)
  {
    ((uint32_t *)dest)[0] = __Mask |
      (src[ImageFormat::RGB24_RBytePos] << 16) |
      (src[ImageFormat::RGB24_GBytePos] <<  8) |
      (src[ImageFormat::RGB24_BBytePos]      ) ;

    dest += 4;
    src += 3;
    x--;
  }
}

// [XRGB32 <- BGR24]

template<uint32_t __Mask>
static void FOG_OPTIMIZEDCALL Conv_XRGB32_From_BGR24(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + Raster::mul4(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul3(params->srcX);

  sysuint_t x = params->width;

  // align
  while (((sysuint_t)src & 0x3) != 0 && x)
  {
    ((uint32_t *)dest)[0] = 
      __Mask |
      (src[ImageFormat::BGR24_RBytePos] << 16) |
      (src[ImageFormat::BGR24_GBytePos] <<  8) |
      (src[ImageFormat::BGR24_BBytePos]      ) ;

    dest += 4;
    src += 3;
    x--;
  }

  // 4 pixels at time
  while (x >= 4)
  {
    uint32_t c0_1231 = ((const uint32_t *)src)[0];
    uint32_t c1_2312 = ((const uint32_t *)src)[1];
    uint32_t c2_3123 = ((const uint32_t *)src)[2];

#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
    // ByteOrder: 0123 0123 0123 0123
    // Source   : RGBR GBRG BRGB
    // Target   : BGRX BGRX BGRX BGRX
    ((uint32_t *)dest)[0] = __Mask | 
      ((c0_1231 & 0x000000FF) << 16) |
      ((c0_1231 & 0x0000FF00)      ) |
      ((c0_1231 & 0x00FF0000) >> 16) ;
    ((uint32_t *)dest)[1] = __Mask | 
      ((c0_1231 & 0xFF000000) >>  8) |
      ((c1_2312 & 0x000000FF) <<  8) |
      ((c1_2312 & 0x0000FF00) >>  8) ;
    ((uint32_t *)dest)[2] = __Mask | 
      ((c1_2312 & 0x00FF0000)      ) |
      ((c1_2312 & 0xFF000000) >> 16) |
      ((c2_3123 & 0x000000FF)      ) ;
    ((uint32_t *)dest)[3] = __Mask | 
      ((c2_3123 & 0x0000FF00) <<  8) |
      ((c2_3123 & 0x00FF0000) >>  8) |
      ((c2_3123 & 0xFF000000) >> 24) ;
#else // FOG_BIG_ENDIAN
    // ByteOrder: 3210 3210 3210 3210
    // Source   : BGRB GRBG RBGR
    // Target   : XRGB XRGB XRGB XRGB
    // TODO:
#endif // FOG_BYTE_ORDER

    dest += 16;
    src += 12;
    x -= 4;
  }

  // tail
  while (x)
  {
    ((uint32_t *)dest)[0] = 
      __Mask |
      (src[ImageFormat::BGR24_RBytePos] << 16) |
      (src[ImageFormat::BGR24_GBytePos] <<  8) |
      (src[ImageFormat::BGR24_BBytePos]      ) ;

    dest += 4;
    src += 3;
    x--;
  }
}

// [XRGB32 <- RGB16_5650]

template<uint32_t __Mask>
static void FOG_OPTIMIZEDCALL Conv_XRGB32_From_RGB16_5650(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + Raster::mul4(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul2(params->srcX);

  sysuint_t x;
  sysuint_t width = params->width;

  // 16 bit 565 R mask: 0x0000F800
  // 16 bit 565 G mask: 0x000007E0
  // 16 bit 565 B mask: 0x0000001F
  for (x = width >> 2; x; x--, dest += 16, src += 8)
  {
    uint32_t c0 = ((uint32_t *)src)[0];
    uint32_t c1 = ((uint32_t *)src)[1];

    ((uint32_t *)dest)[0] = __Mask |
      ((c0 & 0x0000F800) <<  8) |
      ((c0 & 0x000007E0) <<  5) |
      ((c0 & 0x0000001F) <<  3) ;
    ((uint32_t *)dest)[1] = __Mask |
      ((c0 & 0xF8000000) >>  8) |
      ((c0 & 0x07E00000) >> 11) |
      ((c0 & 0x001F0000) >> 13) ;
    ((uint32_t *)dest)[2] = __Mask |
      ((c1 & 0x0000F800) <<  8) |
      ((c1 & 0x000007E0) <<  5) |
      ((c1 & 0x0000001F) <<  3) ;
    ((uint32_t *)dest)[3] = __Mask |
      ((c1 & 0xF8000000) >>  8) |
      ((c1 & 0x07E00000) >> 11) |
      ((c1 & 0x001F0000) >> 13) ;
  }

  for (x = width & 3; x; x--, dest += 4, src += 2)
  {
    uint32_t c0 = ((uint16_t *)src)[0];
    ((uint32_t *)dest)[0] = __Mask |
      ((c0 & 0x0000F800) <<  8) |
      ((c0 & 0x000007E0) <<  5) |
      ((c0 & 0x0000001F) <<  3) ;
  }
}

// [XRGB32 <- RGB16_5550]

template<uint32_t __Mask>
static void FOG_OPTIMIZEDCALL Conv_XRGB32_From_RGB16_5550(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + Raster::mul4(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul2(params->srcX);

  sysuint_t x;
  sysuint_t width = params->width;

  // 16 bit 555 R mask: 0x00007C00
  // 16 bit 555 G mask: 0x000003E0
  // 16 bit 555 B mask: 0x0000001F
  for (x = width >> 2; x; x--, dest += 16, src += 8)
  {
    uint32_t c0 = ((uint32_t *)src)[0];
    uint32_t c1 = ((uint32_t *)src)[1];

    ((uint32_t *)dest)[0] = __Mask |
      ((c0 & 0x00007C00) <<  9) |
      ((c0 & 0x000003E0) <<  6) |
      ((c0 & 0x0000001F) <<  3) ;
    ((uint32_t *)dest)[1] = __Mask |
      ((c0 & 0x7C000000) >>  7) |
      ((c0 & 0x03E00000) >> 10) |
      ((c0 & 0x001F0000) >> 13) ;
    ((uint32_t *)dest)[2] = __Mask |
      ((c1 & 0x00007C00) <<  9) |
      ((c1 & 0x000003E0) <<  6) |
      ((c1 & 0x0000001F) <<  3) ;
    ((uint32_t *)dest)[3] = __Mask |
      ((c1 & 0x7C000000) >>  7) |
      ((c1 & 0x03E00000) >> 10) |
      ((c1 & 0x001F0000) >> 13) ;
  }

  for (x = width & 3; x; x--, dest += 4, src += 2)
  {
    uint32_t c0 = ((uint16_t *)src)[0];
    ((uint32_t *)dest)[0] = __Mask |
      ((c0 & 0x00007C00) <<  9) |
      ((c0 & 0x000003E0) <<  6) |
      ((c0 & 0x0000001F) <<  3) ;
  }
}

// [ARGB32 <- XRGB32]

static void FOG_OPTIMIZEDCALL Conv_ARGB32_From_XRGB32(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + Raster::mul4(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul4(params->srcX);

  sysuint_t x;
  sysuint_t width = params->width;

  for (x = width >> 2; x; x--, dest += 16, src += 16)
  {
#if FOG_ARCH_BITS == 32
    uint32_t c0, c1;

    c0 = ((uint32_t *)src)[0] | 0xFF000000;
    c1 = ((uint32_t *)src)[1] | 0xFF000000;
    ((uint32_t *)dest)[0] = c0;
    ((uint32_t *)dest)[1] = c1;

    c0 = ((uint32_t *)src)[2] | 0xFF000000;
    c1 = ((uint32_t *)src)[3] | 0xFF000000;
    ((uint32_t *)dest)[2] = c0;
    ((uint32_t *)dest)[3] = c1;
#else
    uint64_t c0, c1;

    c0 = ((uint64_t *)src)[0] | FOG_UINT64_C(0xFF000000FF000000);
    c1 = ((uint64_t *)src)[1] | FOG_UINT64_C(0xFF000000FF000000);
    ((uint64_t *)dest)[0] = c0;
    ((uint64_t *)dest)[1] = c1;
#endif
  }

  for (x = width & 3; x; x--, dest += 4, src += 4)
  {
    ((uint32_t *)dest)[0] = ((uint32_t *)src)[0] | 0xFF000000;
  }
}

// [ARGB32 <- PRGB32]

static void FOG_OPTIMIZEDCALL Conv_ARGB32_From_PRGB32(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + Raster::mul4(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul4(params->srcX);

  sysuint_t x;
  sysuint_t width = params->width;

  for (x = width; x; x--, dest += 4, src += 4)
  {
    ((uint32_t *)dest)[0] = Raster::demultiply(((uint32_t *)src)[0]);
  }
}

// [PRGB32 <- ARGB32]

static void FOG_OPTIMIZEDCALL Conv_PRGB32_From_ARGB32(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + Raster::mul4(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul4(params->srcX);

  sysuint_t x;
  sysuint_t width = params->width;

  for (x = width; x; x--, dest += 4, src += 4)
  {
    ((uint32_t *)dest)[0] = Raster::premultiply(((uint32_t *)src)[0]);
  }
}

static void FOG_OPTIMIZEDCALL Conv_PRGB32_From_BGRA32(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + Raster::mul4(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul4(params->srcX);

  sysuint_t x;
  sysuint_t width = params->width;

  for (x = width; x; x--, dest += 4, src += 4)
  {
    ((uint32_t *)dest)[0] = Raster::premultiply(Memory::bswap32(((uint32_t *)src)[0]));
  }
}

static void FOG_OPTIMIZEDCALL Conv_PRGB32_From_XRGB32(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + Raster::mul4(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul4(params->srcX);

  sysuint_t x;
  sysuint_t width = params->width;

  for (x = width >> 2; x; x--, dest += 16, src += 16)
  {
#if FOG_ARCH_BITS == 32
    uint32_t c0, c1;

    c0 = ((uint32_t *)src)[0] | 0xFF000000;
    c1 = ((uint32_t *)src)[1] | 0xFF000000;
    ((uint32_t *)dest)[0] = c0;
    ((uint32_t *)dest)[1] = c1;

    c0 = ((uint32_t *)src)[2] & 0xFF000000;
    c1 = ((uint32_t *)src)[3] & 0xFF000000;
    ((uint32_t *)dest)[2] = c0;
    ((uint32_t *)dest)[3] = c1;
#else
    uint64_t c0, c1;

    c0 = ((uint64_t *)src)[0] | FOG_UINT64_C(0xFF000000FF000000);
    c1 = ((uint64_t *)src)[1] | FOG_UINT64_C(0xFF000000FF000000);
    ((uint64_t *)dest)[0] = c0;
    ((uint64_t *)dest)[1] = c1;
#endif
  }

  for (x = width & 3; x; x--, dest += 4, src += 4)
  {
    ((uint32_t *)dest)[0] = ((uint32_t *)src)[0] & 0xFF000000;
  }
}

// [XRGB32 <- A8]

static void FOG_OPTIMIZEDCALL Conv_XRGB32_From_A8(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + Raster::mul4(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul1(params->srcX);

  sysuint_t x;
  sysuint_t width = params->width;

  for (x = width >> 2; x; x--, dest += 16, src += 4)
  {
    uint32_t s0 = ((uint32_t *)src)[0];

#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
    ((uint32_t *)dest)[0] = Raster::fromGrey(s0 & 0xFF); s0 >>= 8;
    ((uint32_t *)dest)[1] = Raster::fromGrey(s0 & 0xFF); s0 >>= 8;
    ((uint32_t *)dest)[2] = Raster::fromGrey(s0 & 0xFF); s0 >>= 8;
    ((uint32_t *)dest)[3] = Raster::fromGrey(s0);
#else
    ((uint32_t *)dest)[3] = Raster::fromGrey(s0 & 0xFF); s0 >>= 8;
    ((uint32_t *)dest)[2] = Raster::fromGrey(s0 & 0xFF); s0 >>= 8;
    ((uint32_t *)dest)[1] = Raster::fromGrey(s0 & 0xFF); s0 >>= 8;
    ((uint32_t *)dest)[0] = Raster::fromGrey(s0);
#endif
  }

  for (x = width & 3; x; x--, dest += 4, src++)
  {
    ((uint32_t *)dest)[0] = Raster::fromGrey(((uint8_t *)src)[0]);
  }
}

// [RGB24 <- XRGB32]

static void FOG_OPTIMIZEDCALL Conv_RGB24_From_XRGB32(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + Raster::mul3(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul4(params->srcX);

  sysuint_t x;
  sysuint_t width = params->width;

  // align
  while (((sysuint_t)dest & 0x3) != 0 && width)
  {
    uint32_t c0 = ((const uint32_t *)src)[0];

    dest[ImageFormat::RGB24_RBytePos] = Rgba::red(c0);
    dest[ImageFormat::RGB24_GBytePos] = Rgba::green(c0);
    dest[ImageFormat::RGB24_BBytePos] = Rgba::blue(c0);

    dest += 3;
    src += 4;
    width--;
  }

  // 4 pixels at time
  for (x = (width >> 2); x; x--, dest += 12, src += 16)
  {
    uint32_t c0 = ((const uint32_t *)src)[0];
    uint32_t c1 = ((const uint32_t *)src)[1];
    uint32_t c2 = ((const uint32_t *)src)[2];
    uint32_t c3 = ((const uint32_t *)src)[3];

    uint32_t t0_1231;
    uint32_t t1_2312;
    uint32_t t2_3123;

#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
    // ByteOrder: 0123 0123 0123 0123
    // Source   : BGRX BGRX BGRX BGRX
    // Target   : BGRB GRBG RBGR
    t0_1231 = ((c0 & 0x00FFFFFF)      ) | ((c1 & 0x000000FF) << 24) ;
    t1_2312 = ((c1 & 0x00FFFF00) >>  8) | ((c2 & 0x0000FFFF) << 16) ;
    t2_3123 = ((c2 & 0x00FF0000) >> 16) | ((c3 & 0x00FFFFFF) <<  8) ;
#else // FOG_BIG_ENDIAN
    // ByteOrder: 3210 3210 3210 3210
    // Source   : XRGB XRGB XRGB XRGB
    // Target   : RGBR GBRG BRGB
    t0_1231 = ((c0 & 0x00FFFFFF) <<  8) | ((c1 & 0x00FF0000) >> 16) ;
    t1_2312 = ((c1 & 0x0000FFFF)      ) | ((c2 & 0x0000FFFF) << 16) ;
    t2_3123 = ((c2 & 0x00FF0000) >> 16) | ((c3 & 0x00FFFFFF) <<  8) ;
#endif // FOG_BYTE_ORDER

    ((uint32_t *)dest)[0] = t0_1231;
    ((uint32_t *)dest)[1] = t1_2312;
    ((uint32_t *)dest)[2] = t2_3123;
  }

  // tail
  for (x = width & 3; x; x--, dest += 3, src += 4)
  {
    uint32_t c0 = ((const uint32_t *)src)[0];

    dest[ImageFormat::RGB24_RBytePos] = Rgba::red(c0);
    dest[ImageFormat::RGB24_GBytePos] = Rgba::green(c0);
    dest[ImageFormat::RGB24_BBytePos] = Rgba::blue(c0);
  }
}

// [BGR24 <- XRGB32]

static void FOG_OPTIMIZEDCALL Conv_BGR24_From_XRGB32(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + Raster::mul3(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul4(params->srcX);

  sysuint_t x;
  sysuint_t width = params->width;

  // align
  while (((sysuint_t)dest & 0x3) != 0 && width)
  {
    Raster::writeBGR24(dest, Raster::fetchARGB32(src));

    dest += 3;
    src += 4;
    width--;
  }

  // 4 pixels at time
  for (x = (width >> 2); x; x--, dest += 12, src += 16)
  {
    uint32_t c0 = Raster::fetchARGB32(src +  0);
    uint32_t c1 = Raster::fetchARGB32(src +  4);
    uint32_t c2 = Raster::fetchARGB32(src +  8);
    uint32_t c3 = Raster::fetchARGB32(src + 12);

    uint32_t t0_1231;
    uint32_t t1_2312;
    uint32_t t2_3123;

#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
    // ByteOrder: 0123 0123 0123 0123
    // Source   : BGRX BGRX BGRX BGRX
    // Target   : RGBR GBRG BRGB
    t0_1231 = ((c0 & 0x00FF0000) >> 16) | ((c0 & 0x0000FF00)      ) | ((c0 & 0x000000FF) << 16) | ((c1 & 0x00FF0000) <<  8) ;
    t1_2312 = ((c1 & 0x0000FF00) >>  8) | ((c1 & 0x000000FF) <<  8) | ((c2 & 0x00FF0000)      ) | ((c2 & 0x0000FF00) << 16) ;
    t2_3123 = ((c2 & 0x000000FF)      ) | ((c3 & 0x00FF0000) >>  8) | ((c3 & 0x0000FF00) <<  8) | ((c3 & 0x000000FF) << 24) ;
#else // FOG_BIG_ENDIAN
    // ByteOrder: 3210 3210 3210 3210
    // Source   : XRGB XRGB XRGB XRGB
    // Target   : BGRB GRBG RBGR
    t0_1231 = ((c0 & 0x000000FF) << 24) | ((c0 & 0x0000FF00) <<  8) | ((c0 & 0x00FF0000) >>  8) | ((c1 & 0x000000FF)      ) ;
    t1_2312 = ((c1 & 0x0000FF00) << 16) | ((c1 & 0x00FF0000)      ) | ((c2 & 0x000000FF) <<  8) | ((c2 & 0x0000FF00) >>  8) ;
    t2_3123 = ((c2 & 0x00FF0000) <<  8) | ((c3 & 0x000000FF) << 16) | ((c3 & 0x0000FF00)      ) | ((c3 & 0x00FF0000) >> 16) ;
#endif // FOG_BYTE_ORDER

    ((uint32_t *)dest)[0] = t0_1231;
    ((uint32_t *)dest)[1] = t1_2312;
    ((uint32_t *)dest)[2] = t2_3123;
  }

  // tail
  for (x = width & 3; x; x--, dest += 3, src += 4)
  {
    Raster::writeBGR24(dest, Raster::fetchARGB32(src));
  }
}

// [RGB15_5650 <- XRGB32]

static void FOG_OPTIMIZEDCALL Conv_RGB16_5650_From_XRGB32(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + Raster::mul2(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul4(params->srcX);

  sysuint_t x;
  sysuint_t width = params->width;

  for (x = width >> 2; x; x--, dest += 8, src += 16)
  {
    uint32_t c0, c1;

    c0 = ((uint32_t *)src)[0+__FOG_CONV_16_BPP_0];
    c1 = ((uint32_t *)src)[0+__FOG_CONV_16_BPP_1];

    ((uint32_t *)dest)[0] = ((c0 & 0xF80000) >> 8) | ((c0 & 0x00FC00) >>  5) | ((c0 & 0x0000F8) >>  3) |
                   ((c1 & 0xF80000) << 8) | ((c1 & 0x00FC00) << 11) | ((c1 & 0x0000F8) << 13) ;

    c0 = ((uint32_t *)src)[2+__FOG_CONV_16_BPP_0];
    c1 = ((uint32_t *)src)[2+__FOG_CONV_16_BPP_1];

    ((uint32_t *)dest)[1] = ((c0 & 0xF80000) >> 8) | ((c0 & 0x00FC00) >>  5) | ((c0 & 0x0000F8) >>  3) |
                   ((c1 & 0xF80000) << 8) | ((c1 & 0x00FC00) << 11) | ((c1 & 0x0000F8) << 13) ;
  }

  for (x = width & 3; x; x--, dest += 2, src += 4)
  {
    uint32_t c;

    c = ((uint32_t *)src)[0];
    ((uint16_t *)dest)[0] = ((c & 0xF80000) >> 8) | ((c & 0x00FC00) >> 5) | ((c & 0x0000F8) >> 3);
  }
}

static void FOG_OPTIMIZEDCALL Conv_RGB16_5650_From_XRGB32_dither(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + Raster::mul2(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul4(params->srcX);

  sysuint_t x;
  sysuint_t width = params->width;

  const uint8_t* dt = Dither::table[params->origin.y() & Dither::Mask];
  int dx = params->origin.x();

  for (x = width >> 1; x; x--, dx += 2, dest += 4, src += 8)
  {
    uint32_t c0;
    uint32_t c1;
    uint32_t r0, g0, b0, d, dith5, dith6;
    uint32_t r1, g1, b1;

    c0 = ((uint32_t *)src)[0+__FOG_CONV_16_BPP_0];
    c1 = ((uint32_t *)src)[0+__FOG_CONV_16_BPP_1];

    r0 = (c0 & 0x00FF0000) >> 16;
    g0 = (c0 & 0x0000FF00) >> 8;
    b0 = (c0 & 0x000000FF);
    d = dt[(dx + __FOG_CONV_16_BPP_0) & Dither::Mask];
    dith5 = Dither::shf_dither(d, Dither::shf_arg(5));
    dith6 = Dither::shf_dither(d, Dither::shf_arg(6));
    if (((r0 & 7) >= dith5) && (r0 < 0xF8)) r0 += 8;
    if (((g0 & 3) >= dith6) && (g0 < 0xFC)) g0 += 4;
    if (((b0 & 7) >= dith5) && (b0 < 0xF8)) b0 += 8;

    r1 = (c1 & 0x00FF0000) >> 16;
    g1 = (c1 & 0x0000FF00) >> 8;
    b1 = (c1 & 0x000000FF);
    d = dt[(dx + __FOG_CONV_16_BPP_1) & Dither::Mask];
    dith5 = Dither::shf_dither(d, Dither::shf_arg(5));
    dith6 = Dither::shf_dither(d, Dither::shf_arg(6));
    if (((r1 & 7) >= dith5) && (r1 < 0xF8)) r1 += 8;
    if (((g1 & 3) >= dith6) && (g1 < 0xFC)) g1 += 4;
    if (((b1 & 7) >= dith5) && (b1 < 0xF8)) b1 += 8;

    ((uint32_t *)dest)[0] =
      ((r0 & 0xF8) <<  8) | ((g0 & 0xFC) <<  3) | ((b0 & 0xF8) >>  3) |
      ((r1 & 0xF8) << 24) | ((g1 & 0xFC) << 19) | ((b1 & 0xF8) << 13) ;
  }

  if (width & 1)
  {
    uint32_t c0;
    uint32_t r0, g0, b0, d, dith5, dith6;

    c0 = ((uint32_t *)src)[0];

    r0 = (c0 & 0x00FF0000) >> 16;
    g0 = (c0 & 0x0000FF00) >> 8;
    b0 = (c0 & 0x000000FF);
    d = dt[dx & Dither::Mask];
    dith5 = Dither::shf_dither(d, Dither::shf_arg(5));
    dith6 = Dither::shf_dither(d, Dither::shf_arg(6));
    if (((r0 & 7) >= dith5) && (r0 < 0xF8)) r0 += 8;
    if (((g0 & 3) >= dith6) && (g0 < 0xFC)) g0 += 4;
    if (((b0 & 7) >= dith5) && (b0 < 0xF8)) b0 += 8;

    ((uint16_t *)dest)[0] = ((r0 & 0xF8) << 8) | ((g0 & 0xFC) << 3) | ((b0 & 0xF8) >> 3);
  }
}

// [RGB15_5550 <- XRGB32]

static void FOG_OPTIMIZEDCALL Conv_RGB16_5550_From_XRGB32(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + Raster::mul2(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul4(params->srcX);

  sysuint_t x;
  sysuint_t width = params->width;

  for (x = width >> 2; x; x--, dest += 8, src += 16)
  {
    uint32_t c0, c1;

    c0 = ((uint32_t *)src)[0+__FOG_CONV_16_BPP_0];
    c1 = ((uint32_t *)src)[0+__FOG_CONV_16_BPP_1];

    ((uint32_t *)dest)[0] = 
      ((c0 & 0xF80000) >> 9) | ((c0 & 0x00F800) >>  6) | ((c0 & 0x0000F8) >>  3) |
      ((c1 & 0xF80000) << 7) | ((c1 & 0x00F800) << 10) | ((c1 & 0x0000F8) << 13) ;

    c0 = ((uint32_t *)src)[2+__FOG_CONV_16_BPP_0];
    c1 = ((uint32_t *)src)[2+__FOG_CONV_16_BPP_1];

    ((uint32_t *)dest)[1] = 
      ((c0 & 0xF80000) >> 9) | ((c0 & 0x00F800) >>  6) | ((c0 & 0x0000F8) >>  3) |
      ((c1 & 0xF80000) << 7) | ((c1 & 0x00F800) << 10) | ((c1 & 0x0000F8) << 13) ;
  }

  for (x = width & 3; x; x--, dest += 2, src += 4)
  {
    uint32_t c;

    c = ((uint32_t *)src)[0];
    ((uint16_t *)dest)[0] = ((c & 0xF80000) >> 9) | ((c & 0x00F800) >> 6) | ((c & 0x0000F8) >> 3);
  }
}

static void FOG_OPTIMIZEDCALL Conv_RGB16_5550_From_XRGB32_dither(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + Raster::mul2(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul4(params->srcX);

  sysuint_t x;
  sysuint_t width = params->width;

  const uint8_t* dt = Dither::table[params->origin.y() & Dither::Mask];
  int dx = params->origin.x();

  for (x = width >> 1; x; x--, dx += 2, dest += 4, src += 8)
  {
    uint32_t c0;
    uint32_t c1;
    uint32_t r0, g0, b0, dith;
    uint32_t r1, g1, b1;

    c0 = ((uint32_t *)src)[0+__FOG_CONV_16_BPP_0];
    c1 = ((uint32_t *)src)[0+__FOG_CONV_16_BPP_1];

    r0 = (c0 & 0x00FF0000) >> 16;
    g0 = (c0 & 0x0000FF00) >> 8;
    b0 = (c0 & 0x000000FF);
    dith = Dither::shf_dither(dt[(dx + __FOG_CONV_16_BPP_0) & Dither::Mask], Dither::shf_arg(5));
    if (((r0 & 7) >= dith) && (r0 < 0xF8)) r0 += 8;
    if (((g0 & 7) >= dith) && (g0 < 0xF8)) g0 += 8;
    if (((b0 & 7) >= dith) && (b0 < 0xF8)) b0 += 8;

    r1 = (c1 & 0x00FF0000) >> 16;
    g1 = (c1 & 0x0000FF00) >> 8;
    b1 = (c1 & 0x000000FF);
    dith = Dither::shf_dither(dt[(dx + __FOG_CONV_16_BPP_1) & Dither::Mask], Dither::shf_arg(5));
    if (((r1 & 7) >= dith) && (r1 < 0xF8)) r1 += 8;
    if (((g1 & 7) >= dith) && (g1 < 0xF8)) g1 += 8;
    if (((b1 & 7) >= dith) && (b1 < 0xF8)) b1 += 8;

    ((uint32_t *)dest)[0] =
      ((r0 & 0xF8) <<  7) | ((g0 & 0xF8) <<  2) | ((b0 & 0xF8) >>  3) |
      ((r1 & 0xF8) << 23) | ((g1 & 0xF8) << 18) | ((b1 & 0xF8) << 13) ;
  }

  if (width & 1)
  {
    uint32_t c0;
    uint32_t r0, g0, b0, dith;

    c0 = ((uint32_t *)src)[0];

    r0 = (c0 & 0x00FF0000) >> 16;
    g0 = (c0 & 0x0000FF00) >> 8;
    b0 = (c0 & 0x000000FF);
    dith = Dither::shf_dither(dt[dx & Dither::Mask], Dither::shf_arg(5));
    if (((r0 & 7) >= dith) && (r0 < 0xF8)) r0 += 8;
    if (((g0 & 7) >= dith) && (g0 < 0xF8)) g0 += 8;
    if (((b0 & 7) >= dith) && (b0 < 0xF8)) b0 += 8;

    ((uint16_t *)dest)[0] = ((r0 & 0xF8) << 7) | ((g0 & 0xF8) << 2) | ((b0 & 0xF8) >> 3);
  }
}

// [A8 <- Generic]

static void FOG_OPTIMIZEDCALL Conv_A8_Fill_FF(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + Raster::mul1(params->destX);

  sysuint_t x;
  sysuint_t width = params->width;

  for (x = width; x; x--) *dest++ = 0xFF;
}

static void FOG_OPTIMIZEDCALL Conv_A8_Fill_00(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + Raster::mul1(params->destX);

  sysuint_t x;
  sysuint_t width = params->width;

  for (x = width; x; x--) *dest++ = 0x00;
}

// [A8 <- ARGB32]

static void FOG_OPTIMIZEDCALL Conv_A8_From_ARGB32(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + Raster::mul1(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul4(params->srcX);

  sysuint_t x;
  sysuint_t width = params->width;

  // offset
  src += ImageFormat::ARGB32_ABytePos;

  for (x = width; x; x--, dest++, src += 4) *dest = *src;
}

// [A8 <- PRGB32]

static void FOG_OPTIMIZEDCALL Conv_A8_From_PRGB32(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + Raster::mul1(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul4(params->srcX);

  sysuint_t x;
  sysuint_t width = params->width;

  // offset
  src += ImageFormat::ARGB32_ABytePos;

  for (x = width; x; x--, dest++, src += 4) *dest = *src;
}






















#if 0
// BIG TODO:
// [A1 <- Generic]

static void FOG_OPTIMIZEDCALL Conv_A1_Fill_1(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + (params->destX >> 3);
  const uint8_t* src  = params->srcData  + Raster::mul4(params->srcX);

  sysuint_t x = params->destX;
  sysuint_t width = params->width;

  sysuint_t endx = destX + width;
  sysuint_t endx8 = endx & ~7;

  int dx = self->ditherOrigin.x;
  int dith_y = self->ditherOrigin.y;

  const uint8_t* dt = &Dither::table[dith_y & Dither::Mask][0];

  // align
  if ((x & 7) != 0 || x > endx8)
  {
tail:
    uint8_t destBits = dest[0];
    do {
      uint8_t grey = _XRGB32_To_Grey(((uint32_t *)src)[0]);

      uint dith1 = Dither::shf_dither(dt[dx & Dither::Mask], Dither::shf_arg(0));

      if (grey > dith1 || grey == 255)
        destBits |= 1 << (7 - (x & 7));
      else
        destBits &= ~(1 << (7 - (x & 7)));

      x++;
      dx++;
      src += 4;
    } while ((x & 7) != 0 && x != endx);

    *dest++ = destBits;
    if (x == endx) return;
  }

  // 8 pixels loop (1 dest BYTE)
  for (; x != endx8; x += 8)
  {
    uint32_t i;
    uint8_t bits = 0;

    for (i = 128; i != 0; i >>= 1, dx++, src += 4)
    {
      uint8_t grey = _XRGB32_To_Grey(((uint32_t *)src)[0]);
      uint dith1 = Dither::shf_dither(dt[dx & Dither::Mask], Dither::shf_arg(0));

      if (grey > dith1 || grey == 255) bits |= i;
    }
    *dest++ = bits;
  }

  if (x != endx) goto tail;
}

// [A1 <- XRGB32]

static void FOG_OPTIMIZEDCALL Conv_A1_From_XRGB32(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + (params->destX >> 3);
  const uint8_t* src  = params->srcData  + Raster::mul4(params->srcX);

  sysuint_t x = params->destX;
  sysuint_t width = params->width;

  sysuint_t endx = destX + width;
  sysuint_t endx8 = endx & ~7;

  int dx = self->ditherOrigin.x;
  int dith_y = self->ditherOrigin.y;

  const uint8_t* dt = &Dither::table[dith_y & Dither::Mask][0];

  // align
  if ((x & 7) != 0 || x > endx8)
  {
tail:
    uint8_t destBits = dest[0];
    do {
      uint8_t grey = _XRGB32_To_Grey(((uint32_t *)src)[0]);

      uint dith1 = Dither::shf_dither(dt[dx & Dither::Mask], Dither::shf_arg(0));

      if (grey > dith1 || grey == 255)
        destBits |= 0x80 >> (x & 7);
      else
        destBits &= ~(0x80 >> (x & 7));

      x++;
      dx++;
      src += 4;
    } while ((x & 7) != 0 && x != endx);

    *dest++ = destBits;
    if (x == endx) return;
  }

  // 8 pixels loop (1 dest BYTE)
  for (; x != endx8; x += 8)
  {
    uint32_t i;
    uint8_t bits = 0;

    for (i = 128; i != 0; i >>= 1, dx++, src += 4)
    {
      uint8_t grey = _XRGB32_To_Grey(((uint32_t *)src)[0]);
      uint dith1 = Dither::shf_dither(dt[dx & Dither::Mask], Dither::shf_arg(0));

      if (grey > dith1 || grey == 255) bits |= i;
    }
    *dest++ = bits;
  }

  if (x != endx) goto tail;
}
#endif













// [A1 <- A8]

static FOG_INLINE uint8_t __getbit(sysuint_t i)
{
  return 0x80 >> (i);
}

static void FOG_OPTIMIZEDCALL Conv_A1_From_A8(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + (params->destX >> 3);
  const uint8_t* src  = params->srcData  + Raster::mul1(params->srcX);

  sysuint_t x = params->destX;
  sysuint_t width = params->width;
  
  sysuint_t endx = params->destX + width;
  sysuint_t endx8 = endx & ~7;

  // align
  if ((x & 7) != 0 || x > endx8)
  {
tail:
    uint8_t destBits = dest[0];
    do {
      uint8_t s0 = *src++;
      uint8_t bit = __getbit(x & 7);

      if (s0 > 127)
        destBits |= bit;
      else
        destBits &= ~bit;

      x++;
    } while ((x & 7) != 0 && x != endx);

    *dest++ = destBits;
    if (x == endx) return;
  }

  // 8 pixels loop (1 dest BYTE)
  for (; x != endx8; x += 8, src += 8)
  {
    uint8_t bits = 0;

    bits |= (src[0] > 127) << 7;
    bits |= (src[1] > 127) << 6;
    bits |= (src[2] > 127) << 5;
    bits |= (src[3] > 127) << 4;
    bits |= (src[4] > 127) << 3;
    bits |= (src[5] > 127) << 2;
    bits |= (src[6] > 127) << 1;
    bits |= (src[7] > 127) << 0;

    *dest++ = bits;
  }

  if (x != endx) goto tail;
}

static void FOG_OPTIMIZEDCALL Conv_A1_From_A8_dither(const Converter::Data* device, const Converter::Params* params)
{
  uint8_t* dest = params->destData + (params->destX >> 3);
  const uint8_t* src  = params->srcData  + Raster::mul1(params->srcX);

  sysuint_t x = params->destX;
  sysuint_t width = params->width;
  
  sysuint_t endx = params->destX + width;
  sysuint_t endx8 = endx & ~7;

  const uint8_t* dt = &Dither::table[params->origin.y() & Dither::Mask][0];
  int dx = params->origin.x();

  // align
  if ((x & 7) != 0 || x > endx8)
  {
tail:
    uint8_t destBits = dest[0];
    do {
      uint8_t s0 = *src++;
      uint8_t bit = __getbit(x & 7);
      uint dith1 = Dither::shf_dither(dt[dx & Dither::Mask], Dither::shf_arg(0));

      if (s0 > dith1 || s0 == 255)
        destBits |= bit;
      else
        destBits &= ~bit;

      x++;
      dx++;
    } while ((x & 7) != 0 && x != endx);

    *dest++ = destBits;
    if (x == endx) return;
  }

  // 8 pixels loop (1 dest BYTE)
  for (; x != endx8; x += 8)
  {
    uint32_t i;
    uint8_t bits = 0;

    for (i = 128; i != 0; i >>= 1, dx++)
    {
      uint8_t s0 = *src++;
      uint dith1 = Dither::shf_dither(dt[dx & Dither::Mask], Dither::shf_arg(0));

      if (s0 > dith1 || s0 == 255) bits |= i;
    }
    *dest++ = bits;
  }

  if (x != endx) goto tail;
}

// [I8_RGB232 <- XRGB32]

static void FOG_OPTIMIZEDCALL Conv_I8_RGB232_From_XRGB32_dither(const Converter::Data* device, const Converter::Params* params)
{
#if 0
  uint8_t* dest = params->destData + Raster::mul1(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul4(params->srcX);

  sysuint_t x;
  sysuint_t width = params->width;

  const uint8_t* dt = Dither::table[params->origin.y & Dither::Mask];
  int dx = params->origin.x;

  const uint8_t* palConv = params->palConv;

  for (x = width; x; x--, dx++, dest += 1, src += 4)
  {
    uint32_t c0;
    uint32_t r0, g0, b0, d, dith2, dith3;

    c0 = ((uint32_t *)src)[0];
    r0 = (c0 & 0x00FF0000) >> 16;
    g0 = (c0 & 0x0000FF00) >> 8;
    b0 = (c0 & 0x000000FF);
    d = dt[dx & Dither::Mask];
    dith2 = Dither::shf_dither(d, Dither::shf_arg(2));
    dith3 = Dither::shf_dither(d, Dither::shf_arg(3));
    if (((r0 & 0x3F) >= dith2) && (r0 < 0xC0)) r0 += 64;
    if (((g0 & 0x1F) >= dith3) && (g0 < 0xE0)) g0 += 32;
    if (((b0 & 0x3F) >= dith2) && (b0 < 0xC0)) b0 += 64;

    ((uint8_t *)dest)[0] = palConv[((r0 & 0xC0) >> 1) | ((g0 & 0xE0) >> 3) | ((b0 & 0xC0) >> 6)];
  }
#endif
}

// [I8_RGB222 <- XRGB32]

static void FOG_OPTIMIZEDCALL Conv_I8_RGB222_From_XRGB32_dither(const Converter::Data* device, const Converter::Params* params)
{
#if 0
  uint8_t* dest = params->destData + Raster::mul1(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul4(params->srcX);

  sysuint_t x;
  sysuint_t width = params->width;

  const uint8_t* dt = Dither::table[params->origin.y & Dither::Mask];
  int dx = params->origin.x;

  const uint8_t* palConv = params->palConv;

  for (x = width; x; x--, dx++, dest += 1, src += 4)
  {
    uint32_t c0;
    uint32_t r0, g0, b0, dith2;
    uint32_t r0t, g0t, b0t;

    c0 = ((uint32_t *)src)[0];
    r0 = (c0 & 0x00FF0000) >> 16;
    g0 = (c0 & 0x0000FF00) >> 8;
    b0 = (c0 & 0x000000FF);

    r0t = r0 / 85;
    g0t = g0 / 85;
    b0t = b0 / 85;

    dith2 = Dither::shf_dither(dt[dx & Dither::Mask], Dither::shf_arg(2)) * 4 / 3;
    if ((r0 - (r0t * 85)) > dith2) r0t++;
    if ((g0 - (g0t * 85)) > dith2) g0t++;
    if ((b0 - (b0t * 85)) > dith2) b0t++;
    ((uint8_t *)dest)[0] = palConv[(r0t<<4)|(g0t<<2)|(b0t)];
  }
#endif 
}

// [I8_RGB111 <- XRGB32]

static void FOG_OPTIMIZEDCALL Conv_I8_RGB111_From_XRGB32_dither(const Converter::Data* device, const Converter::Params* params)
{
#if 0
  uint8_t* dest = params->destData + Raster::mul1(params->destX);
  const uint8_t* src  = params->srcData  + Raster::mul4(params->srcX);

  sysuint_t x;
  sysuint_t width = params->width;

  const uint8_t* dt = Dither::table[params->origin.y & Dither::Mask];
  int dx = params->origin.x;

  const uint8_t* palConv = params->palConv;

  for (x = width; x; x--, dx++, dest += 1, src += 4)
  {
    uint32_t c0;
    uint32_t r0, g0, b0, dith1;

    c0 = ((uint32_t *)src)[0];
    r0 = (c0 & 0x00FF0000) >> 16;
    g0 = (c0 & 0x0000FF00) >> 8;
    b0 = (c0 & 0x000000FF);
    dith1 = Dither::shf_dither(dt[dx & Dither::Mask], Dither::shf_arg(0));
    if (r0 > dith1) r0 = 255;
    if (g0 > dith1) g0 = 255;
    if (b0 > dith1) b0 = 255;

    ((uint8_t *)dest)[0] = palConv[
      (((r0 + 1) >> 8) << 2) |
      (((g0 + 1) >> 8) << 1) |
      (((b0 + 1) >> 8)     ) ];
  }
#endif
}

// [Multiconverter]

static void FOG_OPTIMIZEDCALL Conv_Multi(const Converter::Data* device, const Converter::Params* params)
{
  Converter* c[2];
  Converter::Params p[2];

  sysuint_t x = params->width;
  uint8_t buffer[2048];

  c[0] = ((Converter*)device->layer[0]);
  c[1] = ((Converter*)device->layer[1]);

  p[0].destData = buffer;
  p[1].destData = params->destData;

  p[0].destX = 0;
  p[1].destX = params->destX;

  p[0].srcData = params->srcData;
  p[1].srcData = buffer;

  p[0].srcX = params->srcX;
  p[1].srcX = 0;

  p[0].origin.clear();
  p[1].origin.set(params->origin);

  for (;;) 
  {
    sysuint_t count = x >= 512 ? 512 : x;
    if (count == 0) break;

    p[0].width = count;
    p[1].width = count;

    p[0].srcX += count;
    p[1].destX += count;

    p[1].origin.translate((int)count, 0);

    c[0]->convert(p[0]);
    c[1]->convert(p[1]);

    x -= count;
  }
}

// [Format]

static uint32_t formatToId(const Converter::Format& format)
{
  // create copy of format
  Converter::Format mod = format;

  // update to format that we can compare it with our built-in formats
  if (mod.flags & Converter::ByteSwap)
  {
    // 32 and 24 bit pixel formats are de facto never byteswapped
    switch (mod.depth)
    {
      case 32:
        mod.rMask = Memory::bswap32(mod.rMask);
        mod.gMask = Memory::bswap32(mod.gMask);
        mod.bMask = Memory::bswap32(mod.bMask);
        mod.aMask = Memory::bswap32(mod.aMask);
        mod.flags &= ~Converter::ByteSwap;
        break;
      case 24:
        // swap R and B channels
        if ((mod.gMask == 0x0000FF00 && mod.aMask == 0x00000000) &&
           ((mod.rMask == 0x00FF0000 && mod.bMask == 0x000000FF) ||
          (mod.rMask == 0x000000FF && mod.bMask == 0x00FF0000) ))
        {
          Fog::Memory::xchg4B(&mod.rMask, &mod.bMask);
          mod.flags &= ~Converter::ByteSwap;
        }
        // invalid
        else
        {
          return Converter::Invalid;
        }
        break;
      case 16:
        break;
      default:
        // only 32, 24 and 16 bit depth ca be byte swapped
        mod.flags &= ~Converter::ByteSwap;
        break;
    }
  }

  sysuint_t i;

#if FOG_ARCH_BITS == 64
  FOG_ASSERT(sizeof(Converter::Format) == sizeof(uint64_t)*3);

  // 64 bit architecture optimizations
  uint64_t mod0 = ((const uint64_t *)&mod)[0];
  uint64_t mod1 = ((const uint64_t *)&mod)[1];
  uint64_t mod2 = ((const uint64_t *)&mod)[2];

  for (i = 0; i != FOG_ARRAY_SIZE(converter_formats); i++)
  {
    if (((const uint64_t *)&converter_formats[i])[0] == mod0 &&
        ((const uint64_t *)&converter_formats[i])[1] == mod1 &&
        ((const uint64_t *)&converter_formats[i])[2] == mod2 )
    {
      return i;
    }
  }
#else
  for (i = 0; i != FOG_ARRAY_SIZE(converter_formats); i++)
  {
    if (converter_formats[i].depth == mod.depth &&
        converter_formats[i].flags == mod.flags &&
        converter_formats[i].rMask == mod.rMask &&
        converter_formats[i].gMask == mod.gMask &&
        converter_formats[i].bMask == mod.bMask &&
        converter_formats[i].aMask == mod.aMask )
    {
      return i;
    }
  }
#endif

  // not found
  return Converter::Invalid;
}

// [Converter]

Fog::Static<Converter::Data> Converter::sharedNull;

Converter::Converter() :
  _d(sharedNull.instancep()->ref())
{
}

Converter::Converter(const Converter& other) :
  _d(other._d->ref())
{
}

Converter::Converter(const Format& dest, const Format& source) :
  _d(sharedNull.instancep()->ref())
{
  setup(dest, source);
}

Converter::Converter(uint32_t dest, uint32_t source) :
  _d(sharedNull.instancep()->ref())
{
  setup(dest, source);
}

Converter::~Converter()
{
  _d->deref();
}

void Converter::_detach()
{
  Data* d = _d;
  Data* newd = new Data();
  
  // copy base
  newd->spanFunc    = d->spanFunc;
  newd->dest        = d->dest;
  newd->source      = d->source;

  // copy extras
  if (d->extraUsed)
  {
    newd->extra.init(d->extra.instance());
    newd->extraUsed = true;
  }

  // copy layers
  if (d->layer[0]) newd->layer[0] = (void *)((Data*)d->layer[0])->ref();
  if (d->layer[1]) newd->layer[1] = (void *)((Data*)d->layer[1])->ref();

  Fog::AtomicBase::ptr_setXchg(&_d, newd)->deref();
}

bool Converter::_tryDetach()
{
  // TODO
  _detach();
  return true;
}

void Converter::free()
{
  Fog::AtomicBase::ptr_setXchg(&_d, sharedNull.instancep()->ref())->deref();
}

void Converter::prepare()
{
  Data* d = _d;

  if (d->refCount.get() > 1)
  {
    Fog::AtomicBase::ptr_setXchg(&_d, new Data())->deref();
  }
  else
  {
    // free device layers
    if (d->layer[0]) ((Data*)d->layer[0])->deref();
    if (d->layer[1]) ((Data*)d->layer[1])->deref();

    // free device extras
    if (d->extraUsed) d->extra.destroy();

    d->layer[0] = NULL;
    d->layer[1] = NULL;
    d->extraUsed = false;
  }
}

void Converter::clear()
{
  Data* d = _d;

  if (d->refCount.get() > 1)
  {
    Fog::AtomicBase::ptr_setXchg(&_d, sharedNull.instancep()->ref())->deref();
  }
  else
  {
    // free device layers
    if (d->layer[0]) ((Data*)d->layer[0])->deref();
    if (d->layer[1]) ((Data*)d->layer[1])->deref();

    // free device extras
    if (d->extraUsed) d->extra.destroy();

    d->layer[0] = NULL;
    d->layer[1] = NULL;
    d->extraUsed = false;
  }
}

bool Converter::setup(const Format& dest, const Format& source)
{
  uint32_t destId = formatToId(dest);
  uint32_t sourceId = formatToId(source);

  return setup(destId, sourceId);
}

bool Converter::setup(uint32_t dest, uint32_t source)
{
  SpanFunc spanFunc;

  if (dest   >= Converter::Count) goto fail;
  if (source >= Converter::Count) goto fail;

  if ((spanFunc = converter_converters[dest][source]))
  {
    prepare();
    _d->spanFunc = spanFunc;
  }
  else
  {
    // TODO: ARGB
    prepare();
    _d->spanFunc = Conv_Multi;
    _d->layer[0] = (void*)( new Data(converter_converters[ImageFormat::XRGB32][source]) );
    _d->layer[1] = (void*)( new Data(converter_converters[dest][ImageFormat::XRGB32]) );

    FOG_ASSERT(((Data*)_d->layer[0])->spanFunc);
    FOG_ASSERT(((Data*)_d->layer[1])->spanFunc);
  }
  return true;

fail:
  clear();
  return false;
}

Converter& Converter::operator=(const Converter& other)
{
  Fog::AtomicBase::ptr_setXchg(&_d, other._d->ref())->deref();
  return *this;
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

using namespace Fog;

#define FILL_FORMAT(dest, src, converter) \
  converter_converters[dest][src] = converter

FOG_INIT_DECLARE err_t fog_converter_init(void)
{
  Fog::Converter::sharedNull.init();

  Fog::Memory::zero(converter_converters, sizeof(Fog::Converter::SpanFunc) * (Fog::Converter::Count * Fog::Converter::Count));

  // Macro    Target pixel format                  Source pixel format                  Function
  // --------|-----------------------------------|------------------------------------|--------------------------------
  FILL_FORMAT(Fog::Converter::ARGB32             , Fog::Converter::ARGB32             , Conv_Copy_32_C);
  FILL_FORMAT(Fog::Converter::ARGB32             , Fog::Converter::PRGB32             , Conv_ARGB32_From_PRGB32);
  FILL_FORMAT(Fog::Converter::ARGB32             , Fog::Converter::XRGB32             , Conv_ARGB32_From_XRGB32);
  FILL_FORMAT(Fog::Converter::ARGB32             , Fog::Converter::RGB24              , Conv_XRGB32_From_RGB24<0xFF000000>);
  FILL_FORMAT(Fog::Converter::ARGB32             , Fog::Converter::BGR24              , Conv_XRGB32_From_BGR24<0xFF000000>);
  FILL_FORMAT(Fog::Converter::ARGB32             , Fog::Converter::RGB16_5650         , Conv_XRGB32_From_RGB16_5650<0xFF000000>);
  FILL_FORMAT(Fog::Converter::ARGB32             , Fog::Converter::RGB16_5550         , Conv_XRGB32_From_RGB16_5550<0xFF000000>);
  FILL_FORMAT(Fog::Converter::ARGB32             , Fog::Converter::BGRA32             , Conv_Copy_32_Swap_C);

  FILL_FORMAT(Fog::Converter::PRGB32             , Fog::Converter::ARGB32             , Conv_PRGB32_From_ARGB32);

  FILL_FORMAT(Fog::Converter::PRGB32             , Fog::Converter::PRGB32             , Conv_Copy_32_C);
  FILL_FORMAT(Fog::Converter::PRGB32             , Fog::Converter::XRGB32             , Conv_PRGB32_From_XRGB32);
  FILL_FORMAT(Fog::Converter::PRGB32             , Fog::Converter::RGB24              , Conv_XRGB32_From_RGB24<0x00000000>);
  FILL_FORMAT(Fog::Converter::PRGB32             , Fog::Converter::BGR24              , Conv_XRGB32_From_BGR24<0x00000000>);
  FILL_FORMAT(Fog::Converter::PRGB32             , Fog::Converter::RGB16_5650         , Conv_XRGB32_From_RGB16_5650<0x00000000>);
  FILL_FORMAT(Fog::Converter::PRGB32             , Fog::Converter::RGB16_5550         , Conv_XRGB32_From_RGB16_5550<0x00000000>);
  FILL_FORMAT(Fog::Converter::PRGB32             , Fog::Converter::BGRA32             , Conv_PRGB32_From_BGRA32);

  FILL_FORMAT(Fog::Converter::XRGB32             , Fog::Converter::ARGB32             , Conv_Copy_32_C);
  FILL_FORMAT(Fog::Converter::XRGB32             , Fog::Converter::PRGB32             , Conv_ARGB32_From_PRGB32);
  FILL_FORMAT(Fog::Converter::XRGB32             , Fog::Converter::XRGB32             , Conv_Copy_32_C);
  FILL_FORMAT(Fog::Converter::XRGB32             , Fog::Converter::RGB24              , Conv_XRGB32_From_RGB24<0xFF000000>);
  FILL_FORMAT(Fog::Converter::XRGB32             , Fog::Converter::BGR24              , Conv_XRGB32_From_BGR24<0xFF000000>);
  FILL_FORMAT(Fog::Converter::XRGB32             , Fog::Converter::RGB16_5650         , Conv_XRGB32_From_RGB16_5650<0xFF000000>);
  FILL_FORMAT(Fog::Converter::XRGB32             , Fog::Converter::RGB16_5550         , Conv_XRGB32_From_RGB16_5550<0xFF000000>);

//FILL_FORMAT(Fog::Converter::XRGB32             , Fog::Converter::A8                 , Conv_XRGB32_From_A8);

  FILL_FORMAT(Fog::Converter::RGB24              , Fog::Converter::ARGB32             , Conv_RGB24_From_XRGB32);
  FILL_FORMAT(Fog::Converter::RGB24              , Fog::Converter::XRGB32             , Conv_RGB24_From_XRGB32);
  FILL_FORMAT(Fog::Converter::RGB24              , Fog::Converter::RGB24              , Conv_Copy_24_C);
  FILL_FORMAT(Fog::Converter::RGB24              , Fog::Converter::BGR24              , Conv_Copy_24_Swap_C);

  FILL_FORMAT(Fog::Converter::BGR24              , Fog::Converter::ARGB32             , Conv_BGR24_From_XRGB32);
  FILL_FORMAT(Fog::Converter::BGR24              , Fog::Converter::XRGB32             , Conv_BGR24_From_XRGB32);
  FILL_FORMAT(Fog::Converter::BGR24              , Fog::Converter::BGR24              , Conv_Copy_24_C);
  FILL_FORMAT(Fog::Converter::BGR24              , Fog::Converter::RGB24              , Conv_Copy_24_Swap_C);

//FILL_FORMAT(Fog::Converter::A8                 , Fog::Converter::XRGB32             , Conv_A8_Fill_FF);
//FILL_FORMAT(Fog::Converter::A8                 , Fog::Converter::A8                 , Conv_Copy_8_C);

//FILL_FORMAT(Fog::Converter::A1                 , Fog::Converter::XRGB32             , Conv_A1_Fill_1);
//FILL_FORMAT(Fog::Converter::A1                 , Fog::Converter::A8                 , Conv_A1_From_A8);

//FILL_FORMAT(Fog::Converter::BGRA32             , Fog::Converter::XRGB32             , Conv_XXXX32_To_XXXX32_Swap);
  FILL_FORMAT(Fog::Converter::BGRA32             , Fog::Converter::ARGB32             , Conv_Copy_32_Swap_C);
//FILL_FORMAT(Fog::Converter::BGRA32             , Fog::Converter::BGRX32             , Conv_XXXX32_To_XXXX32);

//FILL_FORMAT(Fog::Converter::BGRP32             , Fog::Converter::XRGB32             , Conv_XXXX32_To_XXXX32_Swap);
  FILL_FORMAT(Fog::Converter::BGRP32             , Fog::Converter::PRGB32             , Conv_Copy_32_Swap_C);
//FILL_FORMAT(Fog::Converter::BGRP32             , Fog::Converter::BGRP32             , Conv_XXXX32_To_XXXX32);

  FILL_FORMAT(Fog::Converter::BGRX32             , Fog::Converter::XRGB32             , Conv_Copy_32_Swap_C);
//FILL_FORMAT(Fog::Converter::BGRX32             , Fog::Converter::PRGB32             , Conv_XXXX32_To_XXXX32_Swap);
//FILL_FORMAT(Fog::Converter::BGRX32             , Fog::Converter::BGRX32             , Conv_XXXX32_To_XXXX32);

  FILL_FORMAT(Fog::Converter::RGB16_5650         , Fog::Converter::ARGB32             , Conv_RGB16_5650_From_XRGB32);
  FILL_FORMAT(Fog::Converter::RGB16_5650         , Fog::Converter::XRGB32             , Conv_RGB16_5650_From_XRGB32);
  FILL_FORMAT(Fog::Converter::RGB16_5650         , Fog::Converter::RGB16_5650         , Conv_Copy_16_C);
  FILL_FORMAT(Fog::Converter::RGB16_5650         , Fog::Converter::RGB16_5650_ByteSwap, Conv_Copy_16_Swap_C);

  FILL_FORMAT(Fog::Converter::RGB16_5550         , Fog::Converter::ARGB32             , Conv_RGB16_5550_From_XRGB32);
  FILL_FORMAT(Fog::Converter::RGB16_5550         , Fog::Converter::XRGB32             , Conv_RGB16_5550_From_XRGB32);
  FILL_FORMAT(Fog::Converter::RGB16_5550         , Fog::Converter::RGB16_5550         , Conv_Copy_16_C);
  FILL_FORMAT(Fog::Converter::RGB16_5550         , Fog::Converter::RGB16_5550_ByteSwap, Conv_Copy_16_Swap_C);

  FILL_FORMAT(Fog::Converter::RGB16_5650_ByteSwap, Fog::Converter::RGB16_5650         , Conv_Copy_16_Swap_C);
  FILL_FORMAT(Fog::Converter::RGB16_5650_ByteSwap, Fog::Converter::RGB16_5650_ByteSwap, Conv_Copy_16_C);

  FILL_FORMAT(Fog::Converter::RGB16_5550_ByteSwap, Fog::Converter::RGB16_5550         , Conv_Copy_16_Swap_C);
  FILL_FORMAT(Fog::Converter::RGB16_5550_ByteSwap, Fog::Converter::RGB16_5550_ByteSwap, Conv_Copy_16_C);

  FILL_FORMAT(Fog::Converter::I8_RGB232          , Fog::Converter::ARGB32             , Conv_I8_RGB232_From_XRGB32_dither);
  FILL_FORMAT(Fog::Converter::I8_RGB232          , Fog::Converter::XRGB32             , Conv_I8_RGB232_From_XRGB32_dither);

  FILL_FORMAT(Fog::Converter::I8_RGB222          , Fog::Converter::ARGB32             , Conv_I8_RGB222_From_XRGB32_dither);
  FILL_FORMAT(Fog::Converter::I8_RGB222          , Fog::Converter::XRGB32             , Conv_I8_RGB222_From_XRGB32_dither);

  FILL_FORMAT(Fog::Converter::I8_RGB111          , Fog::Converter::ARGB32             , Conv_I8_RGB111_From_XRGB32_dither);
  FILL_FORMAT(Fog::Converter::I8_RGB111          , Fog::Converter::XRGB32             , Conv_I8_RGB111_From_XRGB32_dither);

  Fog::Memory::zero(converter_builtin, 
    sizeof(void*) * Fog::Converter::Count
                  * Fog::Converter::Count
  );

  return Error::Ok;
}
#undef FILL_FORMAT

FOG_INIT_DECLARE void fog_converter_shutdown(void)
{
  Fog::Converter::sharedNull.destroy();
}
