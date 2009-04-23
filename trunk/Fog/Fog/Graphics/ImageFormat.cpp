// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Graphics/ImageFormat.h>

namespace Fog {

// [Fog::ImageFormat]

#define __MAKE_FMT(__name__) #__name__, ImageFormat::__name__

static const ImageFormat::Data formats_storage[] =
{
  //          Name           |BPP|                  Mask value                   |  Shift value  | Byte position | Premul | Indexed |
  //                         |   |     R     |     G     |     B     |     A     | R | G | B | A | R | G | B | A |        |         |
#if FOG_BYTE_ORDER == FOG_LITTLE_ENDIAN
  { __MAKE_FMT(ARGB32)       , 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000, 16, 8 , 0 , 24, 2 , 1 , 0 , 3 , false  , false },
  { __MAKE_FMT(PRGB32)       , 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000, 16, 8 , 0 , 24, 2 , 1 , 0 , 3 , true   , false },
  { __MAKE_FMT(XRGB32)       , 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0x00000000, 16, 8 , 0 , 0 , 2 , 1 , 0 , 3 , false  , false },
  { __MAKE_FMT(RGB24)        , 24, 0x00FF0000, 0x0000FF00, 0x000000FF, 0x00000000, 16, 8 , 0 , 0 , 2 , 1 , 0 , 0 , false  , false },
  { __MAKE_FMT(BGR24)        , 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0x00000000, 16, 8 , 0 , 0 , 2 , 1 , 0 , 0 , false  , false },
#else
  { __MAKE_FMT(ARGB32)       , 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000, 16, 8 , 0 , 24, 1 , 2 , 3 , 0 , false  , false },
  { __MAKE_FMT(PRGB32)       , 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000, 16, 8 , 0 , 24, 1 , 2 , 3 , 0 , true   , false },
  { __MAKE_FMT(XRGB32)       , 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0x00000000, 16, 8 , 0 , 0 , 1 , 2 , 3 , 0 , false  , false },
  { __MAKE_FMT(RGB24)        , 24, 0x00FF0000, 0x0000FF00, 0x000000FF, 0x00000000, 16, 8 , 0 , 0 , 0 , 1 , 2 , 0 , false  , false },
  { __MAKE_FMT(BGR24)        , 24, 0x000000FF, 0x0000FF00, 0x00FF0000, 0x00000000, 16, 8 , 0 , 0 , 2 , 1 , 0 , 0 , false  , false },
#endif // FOG_BYTE_ORDER
  { __MAKE_FMT(I8)           , 8 , 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , false  , true  },
  { __MAKE_FMT(A8)           , 8 , 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , false  , false },
  { __MAKE_FMT(A1)           , 1 , 0x00000000, 0x00000000, 0x00000000, 0x00000001, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , false  , false },
  { __MAKE_FMT(Invalid)      , 0 , 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , false  , false }
};

#undef __MAKE_FMT

ImageFormat::Data* ImageFormat::_formats;

ImageFormat::Data* ImageFormat::_getById(uint32_t id)
{
  if (id < Count)
    return &_formats[id];
  else
    return &_formats[Count];
}

ImageFormat::Data* ImageFormat::_getByName(const Fog::String32& name)
{
  Data* fmt = _formats;

  do {
    if (name == StubAscii8(fmt->name)) break;
  } while ((++fmt)->id != Count);

  return fmt;
}

} // Fog namespace

using namespace Fog;

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_imageformat_init(void)
{
  ImageFormat::_formats = const_cast<ImageFormat::Data*>(formats_storage);

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_imageformat_shutdown(void)
{
}
