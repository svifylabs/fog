// [Fog/Graphics library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_RASTER_H
#define _FOG_GRAPHICS_RASTER_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Graphics/ColorLut.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/Rgba.h>

namespace Fog {

// Used in function map prototypes.
struct ColorMatrix;
struct Pattern;

namespace Raster {

//! @brief Solid source for raster based compositing.
struct Solid
{
  //! @brief Non-premultiplied RGBA color.
  uint32_t rgba;
  //! @brief Premultiplied RGBA color.
  uint32_t rgbp;
};

//! @brief Closure structure that is passed to each compositing or blitting 
//! method.
struct Closure
{
  //! @brief Compositing object or NULL if not used.
  const void* closure;
  //! @brief If source pixels are indexed, here is link to 256 color entries.
  const Rgba* srcPalette;
  //! @brief If destination pixels are indexed, here is link to 256 color entries.
  const Rgba* dstPalette;
};

// ============================================================================
// [Fog::Raster - Function Map]
// ============================================================================

// Converters
typedef void (FOG_FASTCALL *ConvertPlainFn)(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure);

typedef void (FOG_FASTCALL *ConvertDither8Fn)(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Point& origin, const uint8_t* palConv);

typedef void (FOG_FASTCALL *ConvertDither16Fn)(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Point& origin);

// Gradient
typedef void (FOG_FASTCALL *GradientSpanFn)(
  uint8_t* dst, uint32_t c0, uint32_t c1,
  sysint_t w, sysint_t x1, sysint_t x2);

// Raster
typedef void (FOG_FASTCALL *PixelFn)(
  uint8_t* dst, const Solid* src, const Closure* closure);
typedef void (FOG_FASTCALL *PixelMskFn)(
  uint8_t* dst, const Solid* src, uint32_t msk, const Closure* closure);

typedef void (FOG_FASTCALL *SpanSolidFn)(
  uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure);
typedef void (FOG_FASTCALL *SpanSolidMskFn)(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure);
typedef void (FOG_FASTCALL *SpanSolidMskConstFn)(
  uint8_t* dst, const Solid* src, uint32_t msk, sysint_t w, const Closure* closure);

typedef void (FOG_FASTCALL *SpanCompositeFn)(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure);
typedef void (FOG_FASTCALL *SpanCompositeMskFn)(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure);
typedef void (FOG_FASTCALL *SpanCompositeMskConstFn)(
  uint8_t* dst, const uint8_t* src, uint32_t msk, sysint_t w, const Closure* closure);

// Pattern
struct PatternContext;

typedef err_t (FOG_FASTCALL *PatternContextInitFn)(
  PatternContext* ctx,
  const Pattern& pattern);
typedef uint8_t* (FOG_FASTCALL *PatternContextFetchFn)(
  PatternContext* ctx,
  uint8_t* dst, int x, int y, int w);
typedef void (FOG_FASTCALL *PatternContextDestroyFn)(
  PatternContext* ctx);

struct PatternContext
{
  Atomic<sysuint_t> refCount;

  PatternContextFetchFn fetch;
  PatternContextDestroyFn destroy;

  int initialized;
  int format;
  int depth;

  struct Texture
  {
    int dx;
    int dy;

    // Must be destroyed when context is destroyed and its type is texture.
    Static<Image> texture;

    // Private, initialized when context is created. These variables are here
    // for easier access into texture data (it saves some pointer dereferences,
    // i don't know if this is optimization or not, but it's easier to access
    // it in pattern fetchers).
    const uint8_t* bits;
    sysint_t stride;
    int w;
    int h;
  };

  struct GenericGradient
  {
    int colorsAlloc;
    int colorsLength;

    // Dynamically allocated, must be free when context is destroyed and its
    // type is gradient.
    uint32_t* colors;
  };

  struct LinearGradient
  {
    // See GenericGradient.
    int colorsAlloc;
    int colorsLength;
    uint32_t* colors;

    int64_t dx;
    int64_t dy;

    int64_t ax;
    int64_t ay;
  };

  struct RadialGradient
  {
    // See GenericGradient.
    int colorsAlloc;
    int colorsLength;
    uint32_t* colors;

    double dx;
    double dy;
    double fx;
    double fy;
    double r;
    double r2;
    double fx2;
    double fy2;
    double mul;
  };

  struct ConicalGradient
  {
    // See GenericGradient.
    int colorsAlloc;
    int colorsLength;
    uint32_t* colors;

    double dx;
    double dy;
    double angle;
  };

  union
  {
    Texture texture;
    GenericGradient genericGradient;
    LinearGradient linearGradient;
    RadialGradient radialGradient;
    ConicalGradient conicalGradient;
  };
};

// Color Filters
typedef void (FOG_FASTCALL *ColorLutFn)(
  uint8_t* dst, const uint8_t* src,
  const ColorLut::Table* lut,
  sysint_t width);

typedef void (FOG_FASTCALL *ColorMatrixFn)(
  uint8_t* dst, const uint8_t* src,
  const ColorMatrix* cm, uint32_t type,
  sysint_t width);

// Image Filters
typedef void (FOG_FASTCALL *CopyAreaFn)(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height);

typedef void (FOG_FASTCALL *TransposeFn)(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height);

typedef void (FOG_FASTCALL *FloatScanlineConvolveFn)(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, const float* kernel, int size, float divisor,
  int borderMode, uint32_t borderColor);

typedef void (FOG_FASTCALL *IntegerScanlineConvolveFn)(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, const int* kernel, int size, int divisor,
  int borderMode, uint32_t borderColor);

typedef void (FOG_FASTCALL *BlurConvolveFn)(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int radius,
  int borderMode, uint32_t borderColor);

//! @brief Function map contains all low-level raster based manipulation methods.
//!
//! Function map is designed for overriding functions by their better 
//! implementation. For example if processor supports MMX or SSE2, some
//! functions are replaced by MMX/SSE2 versions. This way means that there
//! is not needed to check for these features in higher level API.
struct FunctionMap
{
  // [Convert Table]

  struct ConvertFuncs
  {
    // [ByteSwap]

    ConvertPlainFn bswap16;
    ConvertPlainFn bswap24;
    ConvertPlainFn bswap32;

    // [MemCpy]

    ConvertPlainFn memcpy8;
    ConvertPlainFn memcpy16;
    ConvertPlainFn memcpy24;
    ConvertPlainFn memcpy32;

    // [Argb32 Dest]

    ConvertPlainFn argb32_from_prgb32;
    ConvertPlainFn argb32_from_prgb32_bs;
    ConvertPlainFn argb32_from_rgb32;
    ConvertPlainFn argb32_from_rgb32_bs;

    ConvertPlainFn argb32_bs_from_rgb32;
    ConvertPlainFn argb32_bs_from_prgb32;

    // [Prgb32 Dest]

    ConvertPlainFn prgb32_from_argb32;
    ConvertPlainFn prgb32_from_argb32_bs;
    ConvertPlainFn prgb32_from_i8;

    ConvertPlainFn prgb32_bs_from_argb32;
    ConvertPlainFn prgb32_bs_from_i8;

    // [Rgb32 Dest]

    ConvertPlainFn rgb32_from_argb32;
    ConvertPlainFn rgb32_from_argb32_bs;
    ConvertPlainFn rgb32_from_rgb24;
    ConvertPlainFn rgb32_from_bgr24;
    ConvertPlainFn rgb32_from_rgb16_5550;
    ConvertPlainFn rgb32_from_rgb16_5550_bs;
    ConvertPlainFn rgb32_from_rgb16_5650;
    ConvertPlainFn rgb32_from_rgb16_5650_bs;
    ConvertPlainFn rgb32_from_i8;

    ConvertPlainFn rgb32_bs_from_rgb24;

    // [Rgb24/Bgr24 Dest]

    ConvertPlainFn rgb24_from_rgb32;
    ConvertPlainFn rgb24_from_rgb32_bs;
    ConvertPlainFn rgb24_from_rgb16_5550;
    ConvertPlainFn rgb24_from_rgb16_5550_bs;
    ConvertPlainFn rgb24_from_rgb16_5650;
    ConvertPlainFn rgb24_from_rgb16_5650_bs;
    ConvertPlainFn rgb24_from_i8;

    ConvertPlainFn bgr24_from_rgb32;
    ConvertPlainFn bgr24_from_i8;

    // [Rgb16 Dest]

    ConvertPlainFn rgb16_5550_from_rgb32;
    ConvertPlainFn rgb16_5550_from_rgb24;
    ConvertPlainFn rgb16_5550_from_i8;

    ConvertPlainFn rgb16_5650_from_rgb32;
    ConvertPlainFn rgb16_5650_from_rgb24;
    ConvertPlainFn rgb16_5650_from_i8;

    ConvertPlainFn rgb16_5550_bs_from_rgb32;
    ConvertPlainFn rgb16_5550_bs_from_rgb24;
    ConvertPlainFn rgb16_5550_bs_from_i8;

    ConvertPlainFn rgb16_5650_bs_from_rgb32;
    ConvertPlainFn rgb16_5650_bs_from_rgb24;
    ConvertPlainFn rgb16_5650_bs_from_i8;

    // [Greyscale]

    ConvertPlainFn greyscale8_from_rgb32;
    ConvertPlainFn greyscale8_from_rgb24;
    ConvertPlainFn greyscale8_from_i8;

    ConvertPlainFn rgb32_from_greyscale8;
    ConvertPlainFn rgb24_from_greyscale8;

    // [Dithering]

    ConvertDither8Fn i8rgb232_from_rgb32_dither;
    ConvertDither8Fn i8rgb222_from_rgb32_dither;
    ConvertDither8Fn i8rgb111_from_rgb32_dither;

    ConvertDither8Fn i8rgb232_from_rgb24_dither;
    ConvertDither8Fn i8rgb222_from_rgb24_dither;
    ConvertDither8Fn i8rgb111_from_rgb24_dither;

    ConvertDither16Fn rgb16_5550_from_rgb32_dither;
    ConvertDither16Fn rgb16_5550_from_rgb24_dither;

    ConvertDither16Fn rgb16_5650_from_rgb32_dither;
    ConvertDither16Fn rgb16_5650_from_rgb24_dither;

    ConvertDither16Fn rgb16_5550_bs_from_rgb32_dither;
    ConvertDither16Fn rgb16_5550_bs_from_rgb24_dither;

    ConvertDither16Fn rgb16_5650_bs_from_rgb32_dither;
    ConvertDither16Fn rgb16_5650_bs_from_rgb24_dither;
  };

  ConvertFuncs convert;

  // [Gradient Table]

  struct GradientFuncs
  {
    // [GradientSpan]

    GradientSpanFn gradient_argb32;
    GradientSpanFn gradient_prgb32;
    GradientSpanFn gradient_rgb32;
    GradientSpanFn gradient_rgb24;
    GradientSpanFn gradient_a8;
  };

  GradientFuncs gradient;

  // [Pattern Table]

  struct PatternFuncs
  {
    // [Texture]

    PatternContextInitFn texture_init;

    //PatternContextFetchFn texture_fetch;
    //PatternContextFetchFn texture_fetch_scaled;
    PatternContextFetchFn texture_fetch_repeat;
    PatternContextFetchFn texture_fetch_reflect;
    //PatternContextFetchFn texture_fetch_repeat_scaled;

    // [Linear Gradient]

    PatternContextInitFn linear_gradient_init;
    PatternContextFetchFn linear_gradient_fetch_pad;
    PatternContextFetchFn linear_gradient_fetch_repeat;

    // [Radial Gradient]

    PatternContextInitFn radial_gradient_init;
    PatternContextFetchFn radial_gradient_fetch_pad;
    PatternContextFetchFn radial_gradient_fetch_repeat;

    // [Conical Gradient]

    PatternContextInitFn conical_gradient_init;
    PatternContextFetchFn conical_gradient_fetch;
  };

  PatternFuncs pattern;

  // [Raster Table]

  struct RasterFuncs
  {
    const void* closure;

    // [Pixel]

    PixelFn             pixel;
    PixelMskFn          pixel_a8;

    // [Span Solid]

    SpanSolidFn         span_solid;
    SpanSolidMskFn      span_solid_a8;
    SpanSolidMskConstFn span_solid_a8_const;

    // [Span Composite]

    SpanCompositeFn span_composite[Image::FormatCount];
    SpanCompositeMskFn span_composite_a8[Image::FormatCount];
    SpanCompositeMskConstFn span_composite_a8_const[Image::FormatCount];
  };

  RasterFuncs raster[Image::FormatCount][CompositeBuiltIn];

  // [Filters Table]

  struct FiltersFuncs
  {
    // [ColorLut / ColorMatrix]

    ColorLutFn colorLut[Image::FormatCount];
    ColorMatrixFn colorMatrix[Image::FormatCount];

    // [Copy Area]

    CopyAreaFn copyArea[Image::FormatCount];

    // [FloatConvolve / IntegerConvolve]

    FloatScanlineConvolveFn floatScanlineConvolveH[Image::FormatCount];
    FloatScanlineConvolveFn floatScanlineConvolveV[Image::FormatCount];

    IntegerScanlineConvolveFn integerScanlineConvolveH[Image::FormatCount];
    IntegerScanlineConvolveFn integerScanlineConvolveV[Image::FormatCount];

    // [Box Blur]

    BlurConvolveFn boxBlurConvolveH[Image::FormatCount];
    BlurConvolveFn boxBlurConvolveV[Image::FormatCount];

    // [Stack Blur]

    const uint16_t* stackBlur8Mul; // [255]
    const uint8_t* stackBlur8Shr;  // [255]

    BlurConvolveFn stackBlurConvolveH[Image::FormatCount];
    BlurConvolveFn stackBlurConvolveV[Image::FormatCount];
  };

  FiltersFuncs filters;
};

extern FOG_API FunctionMap* functionMap;

FOG_API FunctionMap::RasterFuncs* getRasterOps(int format, int op);

} // Raster namespace
} // Fog namespace

// [Guard]
#endif // _FOG_GRAPHICS_RASTER_H
