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

//! @brief Image compositing operator.
//!
//! @note Many values and formulas are from antigrain and from SVG specification
//! that can be found here: 
//! - http://www.w3.org/TR/2004/WD-SVG12-20041027/rendering.html
enum CompositeOp
{
  //! @brief The source is copied to the destination.
  //!
  //! The source pixel is copied to destination pixel. If destination pixel 
  //! format not supports alpha channel, the source alpha value is ignored. 
  //! If there is alpha channel mask the composition is done using LERP 
  //! operator.
  //!
  //! Formulas for premultiplied colorspace:
  //!   Dca' = Sca.Da + Sca.(1 - Da)
  //!        = Sca
  //!   Da'  = Sa.Da + Sa.(1 - Da)
  //!        = Sa
  CompositeSrc = 0,

  //! @brief The destination is left untouched.
  //!
  //! Destination pixels remains unchanged.
  //!
  //! Formulas for premultiplied colorspace:
  //!   Dca' = Dca.Sa + Dca.(1 - Sa)
  //!        = Dca
  //!   Da'  = Da.Sa + Da.(1 - Sa)
  //!        = Da
  CompositeDest = 1,

  //! @brief The source is composited over the destination.
  //!
  //! Formulas for premultiplied colorspace:
  //!   Dca' = Sca.Da + Sca.(1 - Da) + Dca.(1 - Sa)
  //!        = Sca + Dca.(1 - Sa)
  //!   Da'  = Sa.Da + Sa.(1 - Da) + Da.(1 - Sa)
  //!        = Sa + Da.(1 - Sa)
  //!        = Sa + Da - Sa.Da
  CompositeSrcOver = 2,

  //! @brief The destination is composited over the source and the result 
  //! replaces the destination. 
  //!
  //! Formulas for premultiplied colorspace:
  //!   Dca' = Dca.Sa + Sca.(1 - Da) + Dca.(1 - Sa)
  //!        = Dca + Sca.(1 - Da)
  //!   Da'  = Da.Sa + Sa.(1 - Da) + Da.(1 - Sa)
  //!        = Da + Sa.(1 - Da)
  //!        = Da + Sa - Da.Sa
  CompositeDestOver = 3,

  //! @brief The part of the source lying inside of the destination replaces
  //! the destination. 
  //!
  //! Formulas for premultiplied colorspace:
  //!   Dca' = Sca.Da
  //!   Da'  = Sa.Da
  CompositeSrcIn = 4,

  //! @brief The part of the destination lying inside of the source replaces
  //! the destination. 
  //!
  //! Formulas for premultiplied colorspace:
  //!   Dca' = Dca.Sa
  //!   Da'  = Da.Sa
  CompositeDestIn = 5,

  //! @brief The part of the source lying outside of the destination replaces
  //! the destination. 
  //!
  //! Formulas for premultiplied colorspace:
  //!   Dca' = Sca.(1 - Da)
  //!   Da'  = Sa.(1 - Da)
  CompositeSrcOut = 6,

  //! @brief The part of the destination lying outside of the source replaces
  //! the destination. 
  //!
  //! Formulas for premultiplied colorspace:
  //!   Dca' = Dca.(1 - Sa)
  //!   Da'  = Da.(1 - Sa)
  CompositeDestOut = 7,

  //! @brief The part of the source lying inside of the destination is
  //! composited onto the destination. 
  //!
  //! Formulas for premultiplied colorspace:
  //!   Dca' = Sca.Da + Dca.(1 - Sa)
  //!   Da'  = Sa.Da + Da.(1 - Sa)
  //!        = Da.(Sa + 1 - Sa)
  //!        = Da
  CompositeSrcAtop = 8,

  //! @brief The part of the destination lying inside of the source is 
  //! composited over the source and replaces the destination. 
  //!
  //! Formulas for premultiplied colorspace:
  //!   Dca' = Dca.Sa + Sca.(1 - Da)
  //!   Da'  = Da.Sa + Sa.(1 - Da)
  //!        = Sa.(Da + 1 - Da)
  //!        = Sa
  CompositeDestAtop = 9,

  //! @brief The part of the source that lies outside of the destination is 
  //! combined with the part of the destination that lies outside of the source.
  //!
  //! Formulas for premultiplied colorspace:
  //!   Dca' = Sca.(1 - Da) + Dca.(1 - Sa)
  //!   Da'  = Sa.(1 - Da) + Da.(1 - Sa)
  //!        = Sa + Da - 2.Sa.Da
  CompositeXor = 10,

  //! @brief Clear the destination not using the source.
  //!
  //! Formulas for premultiplied colorspace:
  //!   Dca' = 0
  //!   Da'  = 0
  CompositeClear = 11,

  //! @brief The source is added to the destination and replaces the destination.
  //!
  //! Formulas for premultiplied colorspace:
  //!   Dca' = Sca.Da + Dca.Sa + Sca.(1 - Da) + Dca.(1 - Sa)
  //!        = Sca + Dca
  //!   Da'  = Sa.Da + Da.Sa + Sa.(1 - Da) + Da.(1 - Sa)
  //!        = Sa + Da
  CompositeAdd = 12,

  //! @brief The source is subtracted from the destination and replaces 
  //! the destination.
  //!
  //! Formulas for premultiplied colorspace:
  //!   Dca' = Dca - Sca
  //!   Da'  = 1 - (1 - Sa).(1 - Da)
  //!        = Sa + Da - Sa.Da
  CompositeSubtract = 13,

  //! @brief The source is multiplied by the destination and replaces 
  //! the destination
  //!
  //! The resultant color is always at least as dark as either of the two 
  //! constituent colors. Multiplying any color with black produces black.
  //! Multiplying any color with white leaves the original color unchanged.
  //!
  //! Formulas for premultiplied colorspace:
  //!   Dca' = Sca.Dca + Sca.(1 - Da) + Dca.(1 - Sa)
  //!   Da'  = Sa.Da + Sa.(1 - Da) + Da.(1 - Sa)
  //!        = Sa + Da - Sa.Da
 CompositeMultiply = 14,

  //! @brief The source and destination are complemented and then multiplied 
  //! and then replace the destination
  //!
  //! Formulas for premultiplied colorspace:
  //!   Dca' = (Sca.Da + Dca.Sa - Sca.Dca) + Sca.(1 - Da) + Dca.(1 - Sa)
  //!        = Sca + Dca - Sca.Dca
  //!   Da'  = Sa + Da - Sa.Da
  CompositeScreen = 15,

  //! @brief Selects the darker of the destination and source colors. The 
  //! destination is replaced with the source when the source is darker,
  //! otherwise it is left unchanged.
  //!
  //! Formulas for premultiplied colorspace:
  //!   Dca' = min(Sca.Da, Dca.Sa) + Sca.(1 - Da) + Dca.(1 - Sa)
  //!   Da'  = min(Sa.Da, Da.Sa) + Sa.(1 - Da) + Da.(1 - Sa)
  //!        = Sa.Da + Sa - Sa.Da + Da - Sa.Da
  //!        = Sa + Da - Sa.Da
  //!
  //!   OR: if (Sca.Da < Dca.Sa) Src-Over() else Dst-Over()
  CompositeDarken = 16,

  //! @brief Selects the lighter of the destination and source colors. The
  //! destination is replaced with the source when the source is lighter, 
  //! otherwise it is left unchanged.
  //!
  //! Formulas for premultiplied colorspace:
  //!   Dca' = max(Sca.Da, Dca.Sa) + Sca.(1 - Da) + Dca.(1 - Sa)
  //!   Da'  = max(Sa.Da, Da.Sa) + Sa.(1 - Da) + Da.(1 - Sa)
  //!        = Sa.Da + Sa - Sa.Da + Da - Sa.Da
  //!        = Sa + Da - Sa.Da
  //!
  //!   OR: if (Sca.Da > Dca.Sa) Src-Over() else Dst-Over()
  CompositeLighten = 17,

  //! @brief Subtracts the darker of the two constituent colors from the 
  //! lighter. Painting with white inverts the destination color. Painting
  //! with black produces no change.
  //!
  //! Formulas for premultiplied colorspace:
  //!   Dca' = abs(Dca.Sa - Sca.Da) + Sca.(1 - Da) + Dca.(1 - Sa)
  //!        = Sca + Dca - 2.min(Sca.Da, Dca.Sa)
  //!   Da'  = Sa + Da - min(Sa.Da, Da.Sa)
  //!        = Sa + Da - Sa.Da
  CompositeDifference = 18,

  //! @brief Produces an effect similar to that of 'difference', but appears
  //! as lower contrast. Painting with white inverts the destination color.
  //! Painting with black produces no change.
  //!
  //! Formulas for premultiplied colorspace:
  //!   Dca' = (Sca.Da + Dca.Sa - 2.Sca.Dca) + Sca.(1 - Da) + Dca.(1 - Sa)
  //!          Sca + Dca - 2.Sca.Dca
  //!   Da'  = (Sa.Da + Da.Sa - 2.Sa.Da) + Sa.(1 - Da) + Da.(1 - Sa)
  //!        = Sa - Sa.Da + Da - Da.Sa = Sa + Da - 2.Sa.Da
  //!          [Substitute 2.Sa.Da with Sa.Da]
  //!        = Sa + Da - Sa.Da
  CompositeExclusion = 19,

  //! @brief Invert.
  //!
  //! Formulas for premultiplied colorspace:
  //!   Dca' = (Da - Dca) * Sa + Dca.(1 - Sa)
  //!   Da'  = Sa + (Da - Da) * Sa + Da - Sa.Da
  //!        = Sa + Da - Sa.Da
  //!
  //! For calculation this formula is best:
  //!   Dca' = (Da - Dca) * Sa + Dca.(1 - Sa)
  //!   Da'  = (1 + Da - Da) * Sa + Da.(1 - Sa)
  CompositeInvert = 20,

  //! @brief Invert RGB.
  //!
  //! Formulas for premultiplied colorspace:
  //!   Dca' = (Da - Dca) * Sca + Dca.(1 - Sa)
  //!   Da'  = Sa + (Da - Da) * Sa + Da - Da.Sa
  //!        = Sa + Da - Sa.Da
  //!
  //! For calculation this formula is best:
  //!   Dca' = (Da - Dca) * Sca + Dca.(1 - Sa)
  //!   Da'  = (1 + Da - Da) * Sa + Da.(1 - Sa)
  CompositeInvertRgb = 21,

  //! @brief Count of compositing operators (this is not a valid operator).
  CompositeCount = 22,

  //! @brief Count of compositing operators built in Fog/Raster engine.
  CompositeBuiltIn = 12
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
