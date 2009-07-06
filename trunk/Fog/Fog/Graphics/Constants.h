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

static const double defaultEpsilon = 1e-14; 

//! @brief Image operators.
enum CompositeOp
{
  CompositeSrc,
  CompositeDest,
  CompositeSrcOver,
  CompositeDestOver,
  CompositeSrcIn,
  CompositeDestIn,
  CompositeSrcOut,
  CompositeDestOut,
  CompositeSrcAtop,
  CompositeDestAtop,
  CompositeXor,
  CompositeClear,

  CompositeAdd,
  CompositeSubtract,
  CompositeMultiply,
  CompositeScreen,
  CompositeDarken,
  CompositeLighten,
  CompositeDifference,
  CompositeExclusion,
  CompositeInvert,
  CompositeInvertRgb,

  CompositeCount
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

//! @brief Rgba color channels.
enum Channel
{
  ChannelRed = 0x1,
  ChannelGreen = 0x2,
  ChannelBlue = 0x4,
  ChannelAlpha = 0x8,
  ChannelRGB = ChannelRed | ChannelGreen | ChannelBlue,
  ChannelRGBA = ChannelRGB | ChannelAlpha,

  ChannelCount = ChannelRGBA + 1
};

//! @brief Matrix multiply ordering.
enum MatrixOrder
{
  MatrixOrderPrepend = 0,
  MatrixOrderAppend = 1
};

//! @brief Fill mode.
enum FillMode
{
  FillNonZero = 0,
  FillEvenOdd = 1
};

//! @brief Line cap.
enum LineCap
{
  LineCapButt = 0,
  LineCapSquare,
  LineCapRound
};

//! @brief Line join.
enum LineJoin
{
  LineJoinMiter = 0,
  LineJoinMiterRevert,
  LineJoinRound,
  LineJoinBevel,
  LineJoinMiterRound
};

//! @brief Inner join.
enum InnerJoin
{
  InnerJoinBevel = 0,
  InnerJoinMiter,
  InnerJoinJag,
  InnerJoinRound
};

//! @brief Viewport option.
enum ViewportOption
{
  ViewAnisotropic,
  ViewXMinYMin,
  ViewXMidYMin,
  ViewXMaxYMin,
  ViewXMinYMid,
  ViewXMidYMid,
  ViewXMaxYMid,
  ViewXMinYMax,
  ViewXMidYMax,
  ViewXMaxYMax
};

//! @brief Text alignment
enum TextAlign
{
  TextAlignLeft        = 0x01,
  TextAlignRight       = 0x02,
  TextAlignHCenter     = 0x03,
  TextAlignHMask       = 0x03,

  TextAlignTop         = 0x10,
  TextAlignBottom      = 0x20,
  TextAlignVCenter     = 0x30,
  TextAlignVMask       = 0x30,

  TextAlignCenter      = TextAlignVCenter | TextAlignHCenter
};

} // Fog namespace

// [Guard]
#endif // _FOG_GRAPHICS_CONSTANTS_H
