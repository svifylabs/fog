// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_CONSTANTS_H
#define _FOG_GRAPHICS_CONSTANTS_H

// [Dependencies]
#include <Fog/Build/Build.h>

namespace Fog {

//! @brief Image operators.
enum Op
{
  // These are for images without alpha value, but
  // Can be used on images with alpha value too!
  OpCombine = 0,

  // Internal SubOp. Used for OpCombine in situation that source has
  // alpha channel, so the result is alpha-blending.
  _OpCombineBlend = 1,
  // Internal SubOp. Used for OpCombine in situation that source is full
  // opaque (alpha == 255 or simply there is no alpha channel)
  _OpCombineCopy = 2,

  // These are for images with alpha value
  // (can't be used on NON alpha images)
  OpCompositeClear,
  OpCompositeSrc,
  OpCompositeDest,
  OpCompositeOver,
  OpCompositeOverReverse,
  OpCompositeIn,
  OpCompositeInReverse,
  OpCompositeOut,
  OpCompositeOutReverse,
  OpCompositeAtop,
  OpCompositeAtopReverse,
  OpCompositeXor,
  OpCompositeAdd,
  OpCompositeSaturate,

  // for Fog
  _OpCompositeEnd,
  _OpCompositeBegin = OpCompositeClear,

  _OpCombineCount = 3,
  _OpEnd = _OpCompositeEnd
};

//! @brief Type of image masks.
enum MaskType
{
  MaskA8 = 0,
  MaskXRGB32 = 1,

  // fog Rog
  _MaskCount = 2
};

//! @brief Results from some color analyze functions.
//!
//! Usualy used to optimize image processing, because algorithm
//! for full opaque image is always better that generic algorithm
//! for image with alpha channel.
enum
{
  //! @brief Alpha values are not constant.
  VariableAlpha = -1,
  //! @brief All alpha values are transparent (0).
  TransparentAlpha = 0,
  //! @brief All alpha values are opaque (255).
  OpaqueAlpha = 255
};

// [ImageFile IDs]

//! @brief Image file IDs.
enum ImageFile
{
  ImageFileNone = 0,

  ImageFileANI,
  ImageFileAPNG,
  ImageFileBMP,
  ImageFileFLI,
  ImageFileFLC,
  ImageFileGIF,
  ImageFileICO,
  ImageFileJPEG,
  ImageFileLBM,
  ImageFileMNG,
  ImageFilePCX,
  ImageFilePNG,
  ImageFilePNM,
  ImageFileTGA,
  ImageFileTIFF,
  ImageFileXBM,
  ImageFileXPM,

  ImageFileOther = 65536
};

// [Error codes]

//! @brief Fog library error codes.
enum ErrorCode
{
  EImageSizeIsZero,
  EImageSizeInvalid,
  EImageSizeTooLarge,

  EImageIOProviderNotAvailable,
  EImageIODecoderNotAvailable,
  EImageIOEncoderNotAvailable,
  EImageIOConverterNotAvailable,
  EImageIOFormatNotSupported,
  EImageIOTerminated,
  EImageIOTruncated,
  EImageIOReadError,
  EImageIOWriteError,
  EImageIOSeekError,
  EImageIONotSeekableStream,
  EImageIOMimeNotMatch,
  EImageIORleError,
  EImageIOInternal,
  EImageIONotAnimationFormat,

  EImageIOJpegLibraryNotFound,
  EImageIOJpegError,
  EImageIOPngLibraryNotFound,
  EImageIOPngError,

  EFontCantLoadDefaultFace,

  EFontConfigLibraryNotFound,
  EFontConfigSymbolNotFound,
  EFontConfigInitFailed,

  EFreeTypeLibraryNotFound,
  EFreeTypeSymbolNotFound,
  EFreeTypeInitFailed
};

//! @brief Fill rule
enum FillRule
{
  FillRuleWinding = 0,
  FillRuleEvenOdd = 1
};

//! @brief Line cap
enum LineCap
{
  LineCapButt = 0,
  LineCapSquare,
  LineCapRound
};

//! @brief Line join
enum LineJoin
{
  LineJoinMiter = 0,
  LineJoinMiterRevert,
  LineJoinRound,
  LineJoinBevel
};

} // Fog namespace

// [Guard]
#endif // _FOG_GRAPHICS_CONSTANTS_H
