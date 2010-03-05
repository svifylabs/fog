// [Fog-Graphics Library - Public API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_IMAGEFILTER_H
#define _FOG_GRAPHICS_IMAGEFILTER_H

// [Dependencies]
#include <Fog/Core/Memory.h>
#include <Fog/Core/Object.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/String.h>
#include <Fog/Core/Value.h>
#include <Fog/Graphics/Argb.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/ConvolutionMatrix.h>
#include <Fog/Graphics/Image.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct BlurParams;
struct ColorFilter;
struct ColorLut;
struct ColorMatrix;
struct ConvolveParams;
struct ImageFilter;
struct ImageFilterBase;
struct ImageFilterEngine;
struct Rect;
struct ScanlineConvolveParams;

// ============================================================================
// [Typedefs]
// ============================================================================

//! @brief The color filter function prototype.
//!
//! This function is used to filter individual pixels. Each color filter have
//! functions for all major pixel formats. Context pointer is pointer returned
//! by @c ImageFilterEngine::getContext() method.
typedef void (FOG_FASTCALL *ColorFilterFn)(
  const void* context,
  uint8_t* dst,
  const uint8_t* src,
  sysuint_t w);

//! @brief The image filter function prototype.
//!
//! This function is used to filter entire image data. Each image filter have
//! functions for all major pixel formats. Context pointer is pointer returned
//! by @c ImageFilterEngine::getContext() method.
typedef void (FOG_FASTCALL *ImageFilterFn)(
  const void* context,
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  sysuint_t w, sysuint_t h, sysint_t offset);

// ============================================================================
// [Fog::ImageFilterEngine]
// ============================================================================

//! @brief Image filter engine.
//!
//! Purpose of this class is to enable sharing of image filtering concepts
//! between color filters and image filters (but there are big differences
//! anyway).
struct FOG_API ImageFilterEngine
{
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ImageFilterEngine(int type);
  virtual ~ImageFilterEngine();

  // --------------------------------------------------------------------------
  // [Interface - Basics]
  // --------------------------------------------------------------------------

  //! @brief Get whether current filter combination is NOP (no operation).
  virtual bool isNop() const;

  //! @brief Extend a given rectangle by number of pixels needed to successfully
  //! apply this filter to image. This is mainly for convolution type filters
  //! like blurs where resulting image can be larger than source.
  //!
  //! @note For color filters this is always NOP (no extends).
  virtual err_t getExtendedRect(Rect& rect) const;

  // --------------------------------------------------------------------------
  // [Interface - Context]
  // --------------------------------------------------------------------------

  //! @brief Get or create context that will be used as an argument for filter
  //! functions. You must release it using @c releaseContext() method.
  virtual const void* getContext() const;
  //! @brief Release context returned by @c getContext() method.
  virtual void releaseContext(const void* context) const;

  // --------------------------------------------------------------------------
  // [Interface - Filtering]
  // --------------------------------------------------------------------------

  //! @brief Get color filter function for a given @a format.
  //!
  //! @param format Pixel format of data that will be processed.
  //!
  //! @note This is valid only for color filters, otherwise NULL is returned.
  virtual ColorFilterFn getColorFilterFn(int format) const;

  //! @brief Get image filter function for a given @a format.
  //!
  //! @param format Pixel format of data that will be processed.
  //! @param processing Processing flag, see @c IMAGE_FILTER_CHAR.
  //!
  //! Possible processing values are:
  //! - @c IMAGE_FILTER_ENTIRE_PROCESSING - Entire processing (default).
  //! - @c IMAGE_FILTER_VERT_PROCESSING - Vertical processing.
  //! - @c IMAGE_FILTER_HORZ_PROCESSING - Horizontal processing.
  //!
  //! @note This is valid only for image filters, otherwise NULL is returned.
  virtual ImageFilterFn getImageFilterFn(int format, int processing) const;

  // --------------------------------------------------------------------------
  // [Reference Counting]
  // --------------------------------------------------------------------------

  //! @brief Reference the image filter.
  FOG_INLINE ImageFilterEngine* ref() const
  {
    refCount.inc();
    return const_cast<ImageFilterEngine*>(this);
  }

  //! @brief Dereference the image filter. If reference count is decreased to
  //! zero the object is destroyed.
  FOG_INLINE void deref()
  {
    if (refCount.deref()) delete this;
  }

  //! @brief Clone filter with all arguments.
  virtual ImageFilterEngine* clone() const = 0;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Reference count.
  mutable Atomic<sysuint_t> refCount;
  //! @brief filter type, see @c IMAGE_FILTER_TYPE.
  int type;
  //! @brief Filter characteristics, see @c IMAGE_FILTER_CHAR.
  int characteristics;

private:
  FOG_DISABLE_COPY(ImageFilterEngine)
};

// ============================================================================
// [Fog::ImageFilterBase]
// ============================================================================

//! @brief Base class for @c ImageFilter and @c ColorFilter.
//!
//! Fog-Framework contains two types of color filtering - image filtering and
//! color filtering. Image filtering is defined as filtering where pixel
//! position and pixel neighbours can affect resulting color of filtered pixel,
//! while color filtering is single color transformation for each pixel. Pixel
//! position or neighbours are not needed to make the transformation.
//!
//! Fog-Framework usually accepts filters as @c ImageFilterBase so you can pass
//! @c ImageFilter or @c ColorFilter to most methods.
struct FOG_API ImageFilterBase
{
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  ImageFilterBase();
  ImageFilterBase(const ImageFilterBase& other);

  explicit ImageFilterBase(const ColorMatrix& colorMatrix);
  explicit ImageFilterBase(const ColorLut& colorLut);
  explicit ImageFilterBase(const ImageFilterEngine* engine);

  ~ImageFilterBase();

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  //! @brief Get filter engine instance (constant, you can't modify it).
  FOG_INLINE const ImageFilterEngine* getEngine() const { return _d; }

  //! @brief Get reference count of wrapped filter engine.
  FOG_INLINE sysuint_t refCount() const { return _d->refCount.get(); }

  //! @brief Get type of filter, see @c IMAGE_FILTER_TYPE.
  FOG_INLINE int getType() const { return _d->type; }

  //! @brief Get characteristics of filter, see @c IMAGE_FILTER_CHAR.
  FOG_INLINE int getCharacteristics() const { return _d->characteristics; }

  // --------------------------------------------------------------------------
  // [ColorFilter / ImageFilter Compatible]
  // --------------------------------------------------------------------------

public:
  // These getters are allowed through @c ImageFilter or @c ColorFilter.

  err_t getColorLut(ColorLut& colorLut) const;
  err_t getColorMatrix(ColorMatrix& colorMatrix) const;

protected:
  // These getters are allowed only through @c ImageFilter.

  err_t getBlur(BlurParams& params) const;

public:
  // These getters are allowed through @c ImageFilter or @c ColorFilter.

  FOG_INLINE err_t setColorFilter(const ColorFilter& colorFilter)
  { return setOther(reinterpret_cast<const ImageFilterBase&>(colorFilter)); }

  err_t setColorLut(const ColorLut& colorLut);
  err_t setColorMatrix(const ColorMatrix& colorMatrix);

protected:
  // These setters are allowed only through @c ImageFilter.

  err_t setBlur(const BlurParams& params);
  err_t setOther(const ImageFilterBase& other);

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  FOG_INLINE ImageFilterBase& operator=(const ImageFilterBase& other) { setOther(other); return *this; }

public:
  FOG_INLINE ImageFilterBase& operator=(const ColorFilter& colorFilter) { setColorFilter(colorFilter); return *this; }
  FOG_INLINE ImageFilterBase& operator=(const ColorMatrix& colorMatrix) { setColorMatrix(colorMatrix); return *this; }
  FOG_INLINE ImageFilterBase& operator=(const ColorLut& colorLut) { setColorLut(colorLut); return *this; }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  FOG_DECLARE_D(ImageFilterEngine)
};

// ============================================================================
// [Fog::ImageFilter]
// ============================================================================

struct FOG_HIDDEN ImageFilter : public ImageFilterBase
{
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  FOG_INLINE ImageFilter() : ImageFilterBase() {}
  FOG_INLINE ImageFilter(const ColorFilter& other) : ImageFilterBase(reinterpret_cast<const ImageFilterBase&>(other)) {}

  FOG_INLINE explicit ImageFilter(const ColorMatrix& colorMatrix) : ImageFilterBase(colorMatrix) {}
  FOG_INLINE explicit ImageFilter(const ColorLut& colorLut) : ImageFilterBase(colorLut) {}
  FOG_INLINE explicit ImageFilter(const BlurParams& params) : ImageFilterBase() { setBlur(params); }
  FOG_INLINE explicit ImageFilter(const ImageFilterEngine* engine) : ImageFilterBase(engine) {}

  FOG_INLINE ~ImageFilter() {}

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  FOG_INLINE err_t getBlur(BlurParams& params) const { return ImageFilterBase::getBlur(params); }
  FOG_INLINE err_t setBlur(const BlurParams& params) { return ImageFilterBase::setBlur(params); }
  FOG_INLINE err_t setOther(const ImageFilterBase& other) { return ImageFilterBase::setOther(other); }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  FOG_INLINE ImageFilter& operator=(const ColorFilter& other) { setOther(reinterpret_cast<const ImageFilterBase&>(other)); return *this; }
  FOG_INLINE ImageFilter& operator=(const ColorMatrix& colorMatrix) { setColorMatrix(colorMatrix); return *this; }
  FOG_INLINE ImageFilter& operator=(const ColorLut& colorLut) { setColorLut(colorLut); return *this; }

  FOG_INLINE ImageFilter& operator=(const ImageFilter& other) { setOther(other); return *this; }
};

// ============================================================================
// [Fog::ColorFilter]
// ============================================================================

struct FOG_HIDDEN ColorFilter : public ImageFilterBase
{
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  FOG_INLINE ColorFilter() : ImageFilterBase() {}
  FOG_INLINE ColorFilter(const ColorFilter& other) : ImageFilterBase(other) {}

  FOG_INLINE explicit ColorFilter(const ColorMatrix& colorMatrix) : ImageFilterBase(colorMatrix) {}
  FOG_INLINE explicit ColorFilter(const ColorLut& colorLut) : ImageFilterBase(colorLut) {}
  FOG_INLINE explicit ColorFilter(const ImageFilterEngine* engine) : ImageFilterBase(engine) {}

  FOG_INLINE ~ColorFilter() {}

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  FOG_INLINE ColorFilter& operator=(const ColorFilter& colorFilter) { setColorFilter(colorFilter); return *this; }
  FOG_INLINE ColorFilter& operator=(const ColorMatrix& colorMatrix) { setColorMatrix(colorMatrix); return *this; }
  FOG_INLINE ColorFilter& operator=(const ColorLut& colorLut) { setColorLut(colorLut); return *this; }
};

// ============================================================================
// [Fog::ImageBlurParams]
// ============================================================================

struct FOG_HIDDEN BlurParams
{
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  FOG_INLINE BlurParams() :
    blur(BLUR_LINEAR),
    hRadius(1.0),
    vRadius(1.0),
    borderExtend(BORDER_EXTEND_PAD),
    borderColor(0x00000000)
  {
  }

  FOG_INLINE BlurParams(
    int blur,
    float hRadius,
    float vRadius,
    int borderExtend = BORDER_EXTEND_PAD,
    uint32_t borderColor = 0x00000000)
    :
    blur(blur),
    hRadius(hRadius),
    vRadius(vRadius),
    borderExtend(borderExtend),
    borderColor(borderColor)
  {
  }

  FOG_INLINE ~BlurParams()
  {
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Blur type, see @c BLUR_TYPE.
  int blur;

  //! @brief Horizontal blur radius.
  float hRadius;
  //! @brief Vertical blur radius.
  float vRadius;

  //! @brief Border extend mode, see @c BORDER_EXTEND_MODE.
  int borderExtend;
  //! @brief Border color (non-premultiplied).
  uint32_t borderColor;
};

// ============================================================================
// [Fog::ScanlineConvolveParamsF]
// ============================================================================

struct FOG_HIDDEN SymmetricConvolveParamsF
{
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  FOG_INLINE SymmetricConvolveParamsF() :
    borderExtend(BORDER_EXTEND_PAD),
    borderColor(0x00000000)
  {
  }

  FOG_INLINE SymmetricConvolveParamsF(const SymmetricConvolveParamsF& other) :
    hMatrix(other.hMatrix),
    vMatrix(other.vMatrix),
    borderExtend(other.borderExtend),
    borderColor(other.borderColor)
  {
  }

  FOG_INLINE SymmetricConvolveParamsF(
    const List<float>& hMatrix,
    const List<float>& vMatrix,
    int borderExtend = BORDER_EXTEND_PAD,
    uint32_t borderColor = 0x00000000)
    :
    hMatrix(hMatrix),
    vMatrix(vMatrix),
    borderExtend(BORDER_EXTEND_PAD),
    borderColor(0x00000000)
  {
  }

  FOG_INLINE ~SymmetricConvolveParamsF()
  {
  }

  SymmetricConvolveParamsF& operator=(const SymmetricConvolveParamsF& other)
  {
    hMatrix = other.hMatrix;
    vMatrix = other.vMatrix;
    borderExtend = other.borderExtend;
    borderColor = other.borderColor;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  List<float> hMatrix;
  List<float> vMatrix;

  //! @brief Border extend mode, see @c BORDER_EXTEND_MODE.
  int borderExtend;
  //! @brief Border color (non-premultiplied).
  uint32_t borderColor;
};

} // Fog namespace

//! @}

// ============================================================================
// [Fog::TypeInfo<>]
// ============================================================================

FOG_DECLARE_TYPEINFO(Fog::BlurParams, Fog::TYPEINFO_PRIMITIVE)
FOG_DECLARE_TYPEINFO(Fog::ColorFilter, Fog::TYPEINFO_MOVABLE)
FOG_DECLARE_TYPEINFO(Fog::ConvolveParams, Fog::TYPEINFO_MOVABLE)
FOG_DECLARE_TYPEINFO(Fog::ImageFilter, Fog::TYPEINFO_MOVABLE)
FOG_DECLARE_TYPEINFO(Fog::SymmetricConvolveParamsF, Fog::TYPEINFO_MOVABLE)

// [Guard]
#endif // _FOG_GRAPHICS_IMAGEFILTER_H
