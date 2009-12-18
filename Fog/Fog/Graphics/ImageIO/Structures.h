// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_IMAGEIO_STRUCTURES_H
#define _FOG_GRAPHICS_IMAGEIO_STRUCTURES_H

// [Dependencies]
#include <Fog/Build/Build.h>

namespace Fog {
namespace ImageIO {

// ============================================================================
// [Fog::ImageIO::BmpFileHeader]
// ============================================================================

#include <Fog/Core/Pack.h>
  
//! @brief Bitmap file header (always 14 bytes in little endian format).
//!
//! @note Size of BmpFileHeader must be 14 bytes.
struct FOG_PACKED BmpFileHeader
{
  uint8_t magic_B;          //!< @brief Bitmap mime type ('B').
  uint8_t magic_M;          //!< @brief Bitmap mime type ('M').
  uint32_t fileSize;        //!< @brief Bitmap file size in bytes.
  uint16_t reserved1;
  uint16_t reserved2;
  uint32_t imageOffset;     //!< @brief Offset to image data (54, 124, ...).
};

#include <Fog/Core/Unpack.h>

// ============================================================================
// [Fog::ImageIO::BmpOS2V1Header]
// ============================================================================

#include <Fog/Core/Pack.h>

//! @brief Bitmap OS/2 header.
//!
//! @note This header is 12 bytes length.
struct FOG_PACKED BmpOS2V1Header
{
  uint32_t headerSize;      //!< @brief Header size (40, 52).
  uint16_t width;           //!< @brief Bitmap width (16-bit value).
  uint16_t height;          //!< @brief Bitmap height (16-bit value).
  uint16_t planes;          //!< @brief Number of color planes (only 1).
  uint16_t bitsPerPixel;    //!< @brief Bits per pixel (1, 4, 8 or 24).
};

#include <Fog/Core/Unpack.h>

// ============================================================================
// [Fog::ImageIO::BmpWinV3Header]
// ============================================================================

#include <Fog/Core/Pack.h>

//! @brief Bitmap V3 header.
//!
//! @note This header is 40 bytes length.
struct FOG_PACKED BmpWinV3Header
{
  uint32_t headerSize;      //!< @brief Header size (40, 52).
  uint32_t width;           //!< @brief Bitmap width.
  uint32_t height;          //!< @brief Bitmap height.
  uint16_t planes;          //!< @brief Count of planes, always 1.
  uint16_t bitsPerPixel;    //!< @brief Bits per pixel (1, 4, 8, 16, 24 or 32).
  uint32_t compression;     //!< Compression methods used (see @c BMP_BI).
  uint32_t imageSize;       //!< @brief Image data size (in bytes).
  uint32_t horzResolution;  //!< @brief Horizontal resolution in pixels per meter.
  uint32_t vertResolution;  //!< @brief Vertical resolution in pixels per meter.
  uint32_t colorsUsed;      //!< @brief Number of colors in the image.
  uint32_t colorsImportant; //!< @brief Minimum number of important colors.
};

#include <Fog/Core/Unpack.h>

// ============================================================================
// [Fog::ImageIO::BmpWinV4Header]
// ============================================================================

#include <Fog/Core/Pack.h>

//! @brief Bitmap V4 header.
//!
//! @note This header is 108 bytes length.
struct FOG_PACKED BmpWinV4Header
{
  uint32_t headerSize;      //!< @brief Header size (108).
  uint32_t width;           //!< @brief Bitmap width.
  uint32_t height;          //!< @brief Bitmap height.
  uint16_t planes;          //!< @brief Count of planes, always 1.
  uint16_t bitsPerPixel;    //!< @brief Bits per pixel (1, 4, 8, 16, 24 or 32).
  uint32_t compression;     //!< Compression methods used (see @c BMP_BI).
  uint32_t imageSize;       //!< @brief Image data size (in bytes).
  uint32_t horzResolution;  //!< @brief Horizontal resolution in pixels per meter.
  uint32_t vertResolution;  //!< @brief Vertical resolution in pixels per meter.
  uint32_t colorsUsed;      //!< @brief Number of colors in the image.
  uint32_t colorsImportant; //!< @brief Minimum number of important colors.

  uint32_t rMask;           //!< @brief Mask identifying bits of red component.
  uint32_t gMask;           //!< @brief Mask identifying bits of green component.
  uint32_t bMask;           //!< @brief Mask identifying bits of blue component.
  uint32_t aMask;           //!< @brief Mask identifying bits of alpha component.
  uint32_t colorspace;      //!< @brief Color space type.
  uint32_t rX;              //!< @brief X coordinate of red endpoint.
  uint32_t rY;              //!< @brief Y coordinate of red endpoint.
  uint32_t rZ;              //!< @brief Z coordinate of red endpoint.
  uint32_t gX;              //!< @brief X coordinate of green endpoint.
  uint32_t gY;              //!< @brief Y coordinate of green endpoint.
  uint32_t gZ;              //!< @brief Z coordinate of green endpoint.
  uint32_t bX;              //!< @brief X coordinate of blue endpoint.
  uint32_t bY;              //!< @brief Y coordinate of blue endpoint.
  uint32_t bZ;              //!< @brief Z coordinate of blue endpoint.
  uint32_t rGamma;          //!< @brief Gamma red coordinate scale value.
  uint32_t gGamma;          //!< @brief Gamma green coordinate scale value.
  uint32_t bGamma;          //!< @brief Gamma blue coordinate scale value.
};

#include <Fog/Core/Unpack.h>

// ============================================================================
// [Fog::ImageIO::BmpWinV5Header]
// ============================================================================

#include <Fog/Core/Pack.h>

//! @brief Bitmap V5 header.
//!
//! @note This header is 124 bytes length.
struct FOG_PACKED BmpWinV5Header
{
  uint32_t headerSize;      //!< @brief Header size (108).
  uint32_t width;           //!< @brief Bitmap width.
  uint32_t height;          //!< @brief Bitmap height.
  uint16_t planes;          //!< @brief Count of planes, always 1.
  uint16_t bitsPerPixel;    //!< @brief Bits per pixel (1, 4, 8, 16, 24 or 32).
  uint32_t compression;     //!< Compression methods used (see @c BMP_BI).
  uint32_t imageSize;       //!< @brief Image data size (in bytes).
  uint32_t horzResolution;  //!< @brief Horizontal resolution in pixels per meter.
  uint32_t vertResolution;  //!< @brief Vertical resolution in pixels per meter.
  uint32_t colorsUsed;      //!< @brief Number of colors in the image.
  uint32_t colorsImportant; //!< @brief Minimum number of important colors.

  uint32_t rMask;           //!< @brief Mask identifying bits of red component.
  uint32_t gMask;           //!< @brief Mask identifying bits of green component.
  uint32_t bMask;           //!< @brief Mask identifying bits of blue component.
  uint32_t aMask;           //!< @brief Mask identifying bits of alpha component.
  uint32_t colorspace;      //!< @brief Color space type.
  uint32_t rX;              //!< @brief X coordinate of red endpoint.
  uint32_t rY;              //!< @brief Y coordinate of red endpoint.
  uint32_t rZ;              //!< @brief Z coordinate of red endpoint.
  uint32_t gX;              //!< @brief X coordinate of green endpoint.
  uint32_t gY;              //!< @brief Y coordinate of green endpoint.
  uint32_t gZ;              //!< @brief Z coordinate of green endpoint.
  uint32_t bX;              //!< @brief X coordinate of blue endpoint.
  uint32_t bY;              //!< @brief Y coordinate of blue endpoint.
  uint32_t bZ;              //!< @brief Z coordinate of blue endpoint.
  uint32_t rGamma;          //!< @brief Gamma red coordinate scale value.
  uint32_t gGamma;          //!< @brief Gamma green coordinate scale value.
  uint32_t bGamma;          //!< @brief Gamma blue coordinate scale value.

  uint32_t intent;          //!< @brief Rendering intent for bitmap.
  uint32_t profileData;     //!< @brief The offset, in bytes, from the beginning
                            //!< of the BmpWinV5Header structure to the start of the profile data.
  uint32_t profileSize;     //!< @brief Size, in bytes, of embedded profile data.
  uint32_t reserved;        //!< @brief Reserved, should be zero.
};

#include <Fog/Core/Unpack.h>

// ============================================================================
// [Fog::ImageIO::BmpDataHeader]
// ============================================================================

//! @brief All bitmap headers in one union.
union BmpDataHeader
{
  uint32_t headerSize;

  BmpOS2V1Header os2v1;
  BmpWinV3Header winv3;
  BmpWinV4Header winv4;
  BmpWinV5Header winv5;
};

// ============================================================================
// [Fog::ImageIO::BMP_BI]
// ============================================================================

enum BMP_HEADER_TYPE
{
  BMP_HEADER_OS2_V1 = 12,
  BMP_HEADER_WIN_V3 = 40,
  BMP_HEADER_WIN_V4 = 108,
  BMP_HEADER_WIN_V5 = 124
};

// ============================================================================
// [Fog::ImageIO::BMP_BI]
// ============================================================================

enum BMP_BI
{
  BMP_BI_RGB        = 0,
  BMP_BI_RLE8       = 1,
  BMP_BI_RLE4       = 2,
  BMP_BI_BITFIELDS  = 3,
  BMP_BI_JPEG       = 4,
  BMP_BI_PNG        = 5
};

// ============================================================================
// [Fog::ImageIO::BMP_RLE]
// ============================================================================

enum BMP_RLE
{
  BMP_RLE_NEXT_LINE = 0,
  BMP_RLE_END       = 1,
  BMP_RLE_MOVE      = 2
};

// ============================================================================
// [Fog::ImageIO::BMP_COLORSPACE_TYPE]
// ============================================================================

enum BMP_COLORSPACE_TYPE
{
  BMP_COLORSPACE_CALIBRATED_RGB = 0,
  BMP_COLORSPACE_DD_RGB = 1,
  BMP_COLORSPACE_DD_CMYK = 2
};

// ============================================================================
// [Fog::ImageIO::PcxHeader]
// ============================================================================

#include <Fog/Core/Pack.h>

struct FOG_PACKED PcxHeader
{
  uint8_t manufacturer;
  uint8_t version;
  uint8_t encoding;
  uint8_t bitsPerPixel;
  int16_t xMin;
  int16_t yMin;
  int16_t xMax;
  int16_t yMax;
  int16_t horizontalDPI;
  int16_t verticalDPI;
  uint8_t colorMap[48];
  uint8_t reserved;
  uint8_t nPlanes;
  int16_t bytesPerLine;
  int16_t paletteInfo;
  int16_t hScreenSize;
  int16_t vScreenSize;
  uint8_t unused[54];
};

#include <Fog/Core/Unpack.h>

// ============================================================================
// [Fog::ImageIO::IcoHeader]
// ============================================================================

#include <Fog/Core/Pack.h>

//! @brief Size of IcoHeader == 6
struct FOG_PACKED IcoHeader
{
  uint16_t reserved;
  uint16_t type;
  uint16_t count;
};

#include <Fog/Core/Unpack.h>

// ============================================================================
// [Fog::ImageIO::IcoEntry]
// ============================================================================

#include <Fog/Core/Pack.h>

//! @brief Size of IcoEntry == 16.
struct FOG_PACKED IcoEntry
{
  uint8_t width;
  uint8_t height;
  uint8_t colorCount;
  uint8_t reserved;
  uint16_t planes;
  uint16_t bpp;
  uint32_t size;
  uint32_t offset;
};

#include <Fog/Core/Unpack.h>

} // ImageIO namespace
} // Fog namespace

// ============================================================================
// [Fog::TypeInfo<>]
// ============================================================================

FOG_DECLARE_TYPEINFO(Fog::ImageIO::BmpFileHeader, Fog::TYPEINFO_PRIMITIVE)
FOG_DECLARE_TYPEINFO(Fog::ImageIO::BmpOS2V1Header, Fog::TYPEINFO_PRIMITIVE)
FOG_DECLARE_TYPEINFO(Fog::ImageIO::BmpWinV3Header, Fog::TYPEINFO_PRIMITIVE)
FOG_DECLARE_TYPEINFO(Fog::ImageIO::BmpWinV4Header, Fog::TYPEINFO_PRIMITIVE)
FOG_DECLARE_TYPEINFO(Fog::ImageIO::BmpWinV5Header, Fog::TYPEINFO_PRIMITIVE)
FOG_DECLARE_TYPEINFO(Fog::ImageIO::BmpDataHeader, Fog::TYPEINFO_PRIMITIVE)

FOG_DECLARE_TYPEINFO(Fog::ImageIO::PcxHeader, Fog::TYPEINFO_PRIMITIVE)

FOG_DECLARE_TYPEINFO(Fog::ImageIO::IcoHeader, Fog::TYPEINFO_PRIMITIVE)
FOG_DECLARE_TYPEINFO(Fog::ImageIO::IcoEntry, Fog::TYPEINFO_PRIMITIVE)

// [Guard]
#endif // _FOG_GRAPHICS_IMAGEIO_STRUCTURES_H
