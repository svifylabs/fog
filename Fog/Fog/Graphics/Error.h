// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_ERROR_H
#define _FOG_GRAPHICS_ERROR_H

// [Dependencies]
#include <Fog/Build/Build.h>

//! @addtogroup Fog_Graphics
//! @{

// ============================================================================
// [Error]
// ============================================================================

namespace Error {

//! @brief Fog/Graphics library error codes.
enum GraphicsError
{
  // [Errors Range]
  _GraphicsErrorStart = 0x00011100,
  _GraphicsErrorLast  = 0x000111FF,

  ImageSizeIsZero = _GraphicsErrorStart,
  ImageSizeIsInvalid,
  ImageSizeIsTooLarge,

  ImageIO_ProviderNotAvailable,
  ImageIO_DecoderNotAvailable,
  ImageIO_EncoderNotAvailable,
  ImageIO_ConverterNotAvailable,
  ImageIO_FormatNotSupported,
  ImageIO_Terminated,
  ImageIO_Truncated,
  ImageIO_ReadFailure,
  ImageIO_WriteFailure,
  ImageIO_SeekFailure,
  ImageIO_NotSeekableStream,
  ImageIO_MimeNotMatch,
  ImageIO_RleError,
  ImageIO_Internal,
  ImageIO_NotAnimationFormat,

  ImageIO_JpegLibraryNotFound,
  ImageIO_JpegSymbolNotFound,
  ImageIO_JpegError,

  ImageIO_PngLibraryNotFound,
  ImageIO_PngError,

  FontCantLoadDefaultFace,

  FontConfigLibraryNotFound,
  FontConfigSymbolNotFound,
  FontConfigInitFailed,

  FreeTypeLibraryNotFound,
  FreeTypeSymbolNotFound,
  FreeTypeInitFailed
};

} // Error namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_ERROR_H
