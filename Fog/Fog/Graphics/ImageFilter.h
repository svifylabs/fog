// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_IMAGEFILTER_H
#define _FOG_GRAPHICS_IMAGEFILTER_H

// [Dependencies]
#include <Fog/Core/Object.h>
#include <Fog/Core/String.h>
#include <Fog/Core/Value.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Image.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Fog::ImageFilter]
// ============================================================================

struct FOG_API ImageFilter
{
  // [Construction / Destruction]

  ImageFilter();
  virtual ~ImageFilter();

  // [Filter Type]

  //! @brief Image filter types.
  enum FilterType
  {
    FilterTypeNone = 0,
    FilterTypeBlur,
    FilterTypeConvolution,
  };

  // [Filter Flags]

  //! @brief Image filter flags.
  enum FilterFlags
  {
    //! @brief Flag that means that filter is doing only color transformations.
    //!
    //! This flag is set by @c ColorFilter class.
    OnlyColorFiltering = (1U << 0),

    //! @brief Filter doesn't support premultiplied colorspace.
    //!
    //! This flag is set very likely and means that filter will first 
    //! demultiply source image, then applies filter and resulting image
    //! is premultiplied again.
    OnlyNonPremultiplied = (1U << 1),

    //! @brief Indexed format is not supported.
    OnlyNonIndexed = (1U << 2),

    //! @brief Filter needs only one pass to filter input data.
    //!
    //! @note All color filters are using one pass. Can't be combined with @c TwoPasses.
    OnePass = (1U << 3),

    //! @brief Filter needs two passes to filter input data.
    //!
    //! @note All blur / convolution filters are using two pases. Can't be combined with
    //! @c OnePass.
    TwoPasses = (1U << 4)
  };

  // [Border Mode]

  //! @brief Edge mode used in image filtering (convolution and blurs).
  enum BorderMode
  {
    //! @brief Borders are extended.
    //!
    //! To get Overflow or underflow pixel the first or last pixel
    //! in scanline is used.
    BorderModeExtend = 0,

    //! @brief All borders contains @c borderColor value.
    BorderModeColor,

    //! @brief Used to detect invalid arguments.
    BorderModeInvalid
  };

  // [Blur Type]

  //! @brief Type of blur.
  enum BlurType
  {
    //! @brief Box blur type.
    //!
    //! Box blur is very bad looking blur, but it's fastest blur implemented
    //! in Fog library. Fog small radius it's quite good looking one. Box blur
    //! is also most agressive blur in Fog library.
    BlurTypeBox,

    //! @brief Stack blur (the default one).
    //!
    //! Stack blur provides very good looking blur with optimal speed.
    BlurTypeStack,

    //! @brief Gaussian blur type.
    //!
    //! Gaussian blur uses gaussian function to setup convolution matrix. It's
    //! slowest blur in Fog library, but the quality is excellent.
    BlurTypeGaussian,

    //! @brief Used to detect invalid arguments.
    BlurTypeInvalid
  };

  // [Type and Flags]

  //! @brief Get type of image filter, see @c Type.
  FOG_INLINE int type() const { return _type; }
  //! @brief Get image filter flags, see @c Flags.
  FOG_INLINE int flags() const { return _flags; }

  // [Clone]

  virtual ImageFilter* clone() const = 0;

  // [Properties]

  virtual err_t setProperty(const String32& name, const Value& value);
  virtual Value getProperty(const String32& name) const;

  // [Nop]

  //! @brief Return @c true if current filter state is nop (no operation).
  virtual bool isNop() const;

  // [Filtering]

  virtual err_t filterImage(Image& dst, const Image& src) const;
  virtual err_t filterData(
    uint8_t* dst, sysint_t dstStride,
    const uint8_t* src, sysint_t srcStride,
    int width, int height, int format) const;

  // [Filtering]

protected:
  virtual err_t filterPrivate(
    uint8_t* dst, sysint_t dstStride,
    const uint8_t* src, sysint_t srcStride,
    int width, int height, int format) const = 0;

  // [Members]

  int _type;
  int _flags;

private:
  FOG_DISABLE_COPY(ImageFilter)
};

// ============================================================================
// [Fog::BlurImageFilter]
// ============================================================================

struct FOG_API BlurImageFilter : public ImageFilter
{
  // [Construction / Destruction]

  BlurImageFilter();
  BlurImageFilter(
    int blurType, double hRadius, double vRadius,
    int borderMode = BorderModeExtend, uint32_t borderColor = 0x00000000);
  virtual ~BlurImageFilter();

  // [Clone]

  virtual ImageFilter* clone() const;

  // [Properties]

  virtual err_t setProperty(const String32& name, const Value& value);
  virtual Value getProperty(const String32& name) const;

  FOG_INLINE int blurType() const { return _blurType; }
  err_t setBlurType(int blurType);

  // [Radius]

  FOG_INLINE double horizontalRadius() const { return _hRadius; }
  FOG_INLINE double verticalRadius() const { return _vRadius; }

  err_t setHorizontalRadius(double hr);
  err_t setVerticalRadius(double vr);

  // [Border Type]

  FOG_INLINE int borderMode() const { return _borderMode; }
  err_t setBorderMode(int borderMode);

  // [Border Color]

  FOG_INLINE uint32_t borderColor() const { return _borderColor; }
  err_t setBorderColor(uint32_t borderColor);

  // [Nop]

  virtual bool isNop() const;

  // [Members]

protected:
  int _blurType;
  double _hRadius;
  double _vRadius;
  int _borderMode;
  uint32_t _borderColor;

  // [Filtering]

  virtual err_t filterPrivate(
    uint8_t* dst, sysint_t dstStride,
    const uint8_t* src, sysint_t srcStride,
    int width, int height, int format) const;

private:
  void _setupFilterType();

  FOG_DISABLE_COPY(BlurImageFilter)
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_IMAGEFILTER_H
