// [Fog/Graphics library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_RASTER_H
#define _FOG_GRAPHICS_RASTER_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Graphics/Argb.h>
#include <Fog/Graphics/ColorLut.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/Path.h>

namespace Fog {

// Used in function map prototypes.
struct BlurParams;
struct ColorMatrix;
struct ConvolveParams;
struct Matrix;
struct Pattern;
struct SymmetricConvolveParamsF;
struct SymmetricConvolveParamsI;

namespace RasterUtil {

// ============================================================================
// [Fog::RasterUtil - Data]
// ============================================================================

extern FOG_API const uint16_t linear_blur8_mul[255]; // [255]
extern FOG_API const uint8_t linear_blur8_shr[255];  // [255]

// ============================================================================
// [Fog::RasterUtil - Structures]
// ============================================================================

//! @brief Solid source for raster based compositing.
struct Solid
{
  // TODO: Remove non-premultiplied, we are completely moved to premultiplied
  // colorspace for painting.
  //! @brief Non-premultiplied RGBA color.
  uint32_t argb;
  //! @brief Premultiplied RGBA color.
  uint32_t prgb;
};

//! @brief Closure structure that is passed to each compositing or blitting 
//! method.
struct Closure
{
  //! @brief Compositing object or NULL if not used.
  const void* closure;
  //! @brief If source pixels are indexed, here is link to 256 color entries.
  const Argb* srcPalette;
  //! @brief If destination pixels are indexed, here is link to 256 color entries.
  const Argb* dstPalette;
};

// ============================================================================
// [Fog::RasterUtil - Prototypes - Convert]
// ============================================================================

typedef void (FOG_FASTCALL *ConvertPlainFn)(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure);

typedef void (FOG_FASTCALL *ConvertDither8Fn)(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Point& origin, const uint8_t* palConv);

typedef void (FOG_FASTCALL *ConvertDither16Fn)(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Point& origin);

// ============================================================================
// [Fog::RasterUtil - Prototypes - Gradient]
// ============================================================================

typedef void (FOG_FASTCALL *GradientSpanFn)(
  uint8_t* dst, uint32_t c0, uint32_t c1,
  sysint_t w, sysint_t x1, sysint_t x2);

// ============================================================================
// [Fog::RasterUtil - Prototypes - Pattern]
// ============================================================================

struct PatternContext;

typedef err_t (FOG_FASTCALL *PatternContextSolidInitFn)(
  PatternContext* ctx, uint32_t prgb);

typedef err_t (FOG_FASTCALL *PatternContextInitFn)(
  PatternContext* ctx,
  const Pattern& pattern, const Matrix& matrix);

typedef uint8_t* (FOG_FASTCALL *PatternContextFetchFn)(
  PatternContext* ctx,
  uint8_t* dst, int x, int y, int w);

typedef void (FOG_FASTCALL *PatternContextDestroyFn)(
  PatternContext* ctx);

typedef err_t (FOG_FASTCALL *ScaleInitFn)(
  PatternContext* ctx,
  const Image* im, int dw, int dh, int filter);

struct PatternContext
{
  //! @brief Reference count.
  Atomic<sysuint_t> refCount;

  //! @brief Fetch function.
  PatternContextFetchFn fetch;
  //! @brief Destroy function (you must call it if reference count was decreased
  //! to zero).
  PatternContextDestroyFn destroy;

  //! @brief true if this context is initialized and must be destroyed using
  //! @c destroy() function.
  int initialized;
  //! @brief Pixel format of this context (pixel format that will be fetched
  //! using @c fetch() function.
  int format;
  //! @brief Depth of this context (this is related to pixel format of course).
  int depth;

  //! @brief True if context is transformed.
  //!
  //! @note This is never set for translation only matrices. Scale or shear is
  //! condition to set this variable to true.
  int isTransformed;

  //! @brief Embedded matrix data (no @c Matrix instance here).
  double m[6];

  //! @brief Data for solid color context.
  struct Solid
  {
    uint32_t prgb;
  };

  //! @brief Data for texture context.
  struct Texture
  {
    // Private, initialized when context is created. These variables are here
    // for easier access into texture data (it saves some pointer dereferences,
    // i don't know if this is optimization or not, but it's easier to access
    // it in pattern fetchers).
    const uint8_t* bits;
    int w;
    int h;
    sysint_t stride;

    // Must be destroyed when context is destroyed and its type is texture.
    Static<Image> texture;

    // Increments, f...
    int dx;
    int dy;

    // Subpixel offset at 1.0 scale ratio (used if isTransformed is false).
    uint fY0X0;
    uint fY0X1;
    uint fY1X0;
    uint fY1X1;
  };

  struct Scale
  {
    int dx;
    int dy;

    int sw;
    int sh;
    int dw;
    int dh;

    int* xpoints;
    uint32_t** ypoints;
    int* xapoints;
    int* yapoints;
    int xup_yup;
  };

  //! @brief Shared data for linear, radial and conical gradients.
  struct GenericGradient
  {
    // Dynamically allocated, must be free when context is destroyed and its
    // type is gradient.
    uint32_t* colors;
    // Capacity of 'colors' array.
    int colorsAlloc;
    // Count of colors in 'colors' array.
    int colorsLength;

    double dx;
    double dy;
  };

  //! @brief Linear gradient data.
  struct LinearGradient
  {
    // See GenericGradient.
    uint32_t* colors;
    int colorsAlloc;
    int colorsLength;

    double dx;
    double dy;

    double ax;
    double ay;

    double dist;
  };

  //! @brief Radial gradient data.
  struct RadialGradient
  {
    // See GenericGradient.
    uint32_t* colors;
    int colorsAlloc;
    int colorsLength;

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

  //! @brief Conical gradient data.
  struct ConicalGradient
  {
    // See GenericGradient.
    uint32_t* colors;
    int colorsAlloc;
    int colorsLength;

    double dx;
    double dy;

    double angle;
  };

  union
  {
    Solid solid;

    Texture texture;
    Scale scale;

    GenericGradient genericGradient;
    LinearGradient linearGradient;
    RadialGradient radialGradient;
    ConicalGradient conicalGradient;
  };
};

// ============================================================================
// [Fog::RasterUtil - Prototypes - Filter]
// ============================================================================

typedef void (FOG_FASTCALL *ColorLutFn)(
  uint8_t* dst, const uint8_t* src, sysuint_t width, const ColorLutData* lut);

typedef void (FOG_FASTCALL *ColorMatrixFn)(
  uint8_t* dst, const uint8_t* src, sysuint_t width, const float m[5][5]);

typedef void (FOG_FASTCALL *CopyAreaFn)(
  uint8_t* dst, sysint_t dstStride, const uint8_t* src, sysint_t srcStride,
  sysuint_t w, sysuint_t h, sysint_t offset, const void* context);

typedef void (FOG_FASTCALL *BlurFn)(
  uint8_t* dst, sysint_t dstStride, const uint8_t* src, sysint_t srcStride,
  sysuint_t w, sysuint_t h, sysint_t offset, const BlurParams* params);

typedef void (FOG_FASTCALL *SymmetricConvolveFloatFn)(
  uint8_t* dst, sysint_t dstStride, const uint8_t* src, sysint_t srcStride,
  sysuint_t w, sysuint_t h, sysint_t offset, const SymmetricConvolveParamsF* params);

typedef void (FOG_FASTCALL *SymmetricConvolveIntFn)(
  uint8_t* dst, sysint_t dstStride, const uint8_t* src, sysint_t srcStride,
  sysuint_t w, sysuint_t h, sysint_t offset, const SymmetricConvolveParamsI* params);

// ============================================================================
// [Fog::RasterUtil - Prototypes - Composite]
// ============================================================================

typedef void (FOG_FASTCALL *PixelFn)(
  uint8_t* dst, const Solid* src, const Closure* closure);
typedef void (FOG_FASTCALL *PixelMskFn)(
  uint8_t* dst, const Solid* src, uint32_t msk, const Closure* closure);

typedef void (FOG_FASTCALL *CSpanFn)(
  uint8_t* dst, const Solid* src, sysint_t w, const Closure* closure);
typedef void (FOG_FASTCALL *CSpanMskFn)(
  uint8_t* dst, const Solid* src, const uint8_t* msk, sysint_t w, const Closure* closure);
typedef void (FOG_FASTCALL *CSpanMskConstFn)(
  uint8_t* dst, const Solid* src, uint32_t msk, sysint_t w, const Closure* closure);

typedef void (FOG_FASTCALL *VSpanFn)(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Closure* closure);
typedef void (FOG_FASTCALL *VSpanMskFn)(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Closure* closure);
typedef void (FOG_FASTCALL *VSpanMskConstFn)(
  uint8_t* dst, const uint8_t* src, uint32_t msk, sysint_t w, const Closure* closure);

// ============================================================================
// [Fog::RasterUtil - Function Map]
// ============================================================================

//! @brief Function map contains all low-level raster based manipulation methods.
//!
//! Function map is designed for overriding functions by their better 
//! implementation. For example if processor supports MMX or SSE2, some
//! functions are replaced by MMX/SSE2 versions. This way means that there
//! is not needed to check for these features in higher level API.
struct FunctionMap
{
  // [Convert]

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

    // TODO: These conversion functions should be in some [][] array, an enum
    // describing foreign formats should be introduced. This leads to switch of
    // doom.

    // [Axxx32 Dest]

    ConvertPlainFn axxx32_from_xxxx32;
    ConvertPlainFn axxx32_from_a8;
    ConvertPlainFn axxx32_bs_from_a8;

    // [Argb32 Dest]

    ConvertPlainFn argb32_from_prgb32;
    ConvertPlainFn argb32_from_prgb32_bs;
    ConvertPlainFn argb32_from_rgb32;
    ConvertPlainFn argb32_from_rgb32_bs;
    ConvertPlainFn argb32_from_i8;

    ConvertPlainFn argb32_bs_from_rgb32;
    ConvertPlainFn argb32_bs_from_prgb32;
    ConvertPlainFn argb32_bs_from_i8;

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

    // [A8 Dest]

    ConvertPlainFn a8_from_axxx32;
    ConvertPlainFn a8_from_axxx32_bs;
    ConvertPlainFn a8_from_i8;

    // [Greyscale]

    ConvertPlainFn greyscale8_from_rgb32;
    ConvertPlainFn greyscale8_from_rgb24;
    ConvertPlainFn greyscale8_from_bgr24;
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

  // [Gradient]

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

  // [Pattern]

  struct PatternFuncs
  {
    // [Solid]

    // NOTE: Here is solid_fetch function, but it's helper for other patterns.
    // It's possible that some pattern is badly initialized or it's allowed
    // null data so the solid color will be used. Also for images with ultra
    // scaling the solid color is good choice.
    PatternContextSolidInitFn solid_init;
    PatternContextFetchFn solid_fetch;

    // [Texture]

    PatternContextInitFn texture_init;
    // Exact, no transform.
    PatternContextFetchFn texture_fetch_exact_repeat[PIXEL_FORMAT_COUNT];
    PatternContextFetchFn texture_fetch_exact_reflect[PIXEL_FORMAT_COUNT];
    // Subpixel accurate, no transform.
    PatternContextFetchFn texture_fetch_subx0_repeat[PIXEL_FORMAT_COUNT];
    PatternContextFetchFn texture_fetch_subx0_reflect[PIXEL_FORMAT_COUNT];
    PatternContextFetchFn texture_fetch_sub0y_repeat[PIXEL_FORMAT_COUNT];
    PatternContextFetchFn texture_fetch_sub0y_reflect[PIXEL_FORMAT_COUNT];
    PatternContextFetchFn texture_fetch_subxy_repeat[PIXEL_FORMAT_COUNT];
    PatternContextFetchFn texture_fetch_subxy_reflect[PIXEL_FORMAT_COUNT];
    // Transform, nearest.
    PatternContextFetchFn texture_fetch_transform_nearest_repeat[PIXEL_FORMAT_COUNT];
    // Transform, bilinear.
    PatternContextFetchFn texture_fetch_transform_bilinear_repeat[PIXEL_FORMAT_COUNT];

    // [Scale]

    ScaleInitFn scale_init;
    PatternContextFetchFn scale_fetch_nearest[PIXEL_FORMAT_COUNT];
    PatternContextFetchFn scale_fetch_bilinear[PIXEL_FORMAT_COUNT];

    // [Linear Gradient]

    PatternContextInitFn linear_gradient_init;
    // Exact is non-antialiased gradient rendering, also used in antialiased
    // mode for vertical and horizontal gradients.
    PatternContextFetchFn linear_gradient_fetch_exact_pad;
    PatternContextFetchFn linear_gradient_fetch_exact_repeat;
    // Subpixel accurate gradient rendering.
    PatternContextFetchFn linear_gradient_fetch_subxy_pad;
    PatternContextFetchFn linear_gradient_fetch_subxy_repeat;

    // [Radial Gradient]

    PatternContextInitFn radial_gradient_init;
    PatternContextFetchFn radial_gradient_fetch_pad;
    PatternContextFetchFn radial_gradient_fetch_repeat;

    // [Conical Gradient]

    PatternContextInitFn conical_gradient_init;
    PatternContextFetchFn conical_gradient_fetch;
  };

  PatternFuncs pattern;

  // [Filter]

  struct FilterFuncs
  {
    // [ColorLut / ColorMatrix]

    ColorLutFn color_lut[PIXEL_FORMAT_COUNT];
    ColorMatrixFn color_matrix[PIXEL_FORMAT_COUNT];

    // [Copy Area]

    CopyAreaFn copy_area[PIXEL_FORMAT_COUNT];

    // [Box Blur]

    BlurFn box_blur_h[PIXEL_FORMAT_COUNT];
    BlurFn box_blur_v[PIXEL_FORMAT_COUNT];

    // [Linear Blur]

    BlurFn linear_blur_h[PIXEL_FORMAT_COUNT];
    BlurFn linear_blur_v[PIXEL_FORMAT_COUNT];

    // [Symmetric Convolve]

    SymmetricConvolveFloatFn symmetric_convolve_float_h[PIXEL_FORMAT_COUNT];
    SymmetricConvolveFloatFn symmetric_convolve_float_v[PIXEL_FORMAT_COUNT];

    SymmetricConvolveIntFn symmetric_convolve_int_h[PIXEL_FORMAT_COUNT];
    SymmetricConvolveIntFn symmetric_convolve_int_v[PIXEL_FORMAT_COUNT];

    // [Generic Convolve]
  };

  FilterFuncs filter;

  // [Composite]

  struct RasterFuncs
  {
    // [Closure]

    const void* closure;

    // [Pixel]

    PixelFn pixel;
    PixelMskFn pixel_a8;

    // [Span Solid]

    CSpanFn cspan;
    CSpanMskFn cspan_a8;
    CSpanMskConstFn cspan_a8_const;

    // [Span Composite]

    VSpanFn vspan[PIXEL_FORMAT_COUNT];
    VSpanMskFn vspan_a8[PIXEL_FORMAT_COUNT];
    VSpanMskConstFn vspan_a8_const[PIXEL_FORMAT_COUNT];
  };

  RasterFuncs raster[COMPOSITE_COUNT][PIXEL_FORMAT_COUNT];
};

extern FOG_API FunctionMap* functionMap;

FOG_API FunctionMap::RasterFuncs* getRasterOps(int format, int op);

} // RasterUtil namespace
} // Fog namespace

// [Guard]
#endif // _FOG_GRAPHICS_RASTER_H
