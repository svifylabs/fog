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

} // Raster namespace
} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_RASTER_C_H
