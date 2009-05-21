// [Fog/Graphics library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_RASTER_H
#define _FOG_GRAPHICS_RASTER_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/Rgba.h>

namespace Fog {

// Used in function map prototypes.
struct Pattern;

namespace Raster {

// ============================================================================
// [Fog::Raster - Function Map]
// ============================================================================

typedef void (FOG_FASTCALL *ConvertPlainFn)(
  uint8_t* dst, const uint8_t* src, sysint_t w);

typedef void (FOG_FASTCALL *ConvertDither8Fn)(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Point& origin, const uint8_t* palConv);

typedef void (FOG_FASTCALL *ConvertDither16Fn)(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Point& origin);

typedef void (FOG_FASTCALL *ConvertIndexedFn)(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Rgba* pal);

typedef void (FOG_FASTCALL *GradientSpanFn)(
  uint8_t* dst, uint32_t c0, uint32_t c1,
  sysint_t w, sysint_t x1, sysint_t x2);

typedef void (FOG_FASTCALL *PixelFn)(
  uint8_t* dst, uint32_t src);
typedef void (FOG_FASTCALL *PixelMskFn)(
  uint8_t* dst, uint32_t src, uint32_t msk);

typedef void (FOG_FASTCALL *SpanSolidFn)(
  uint8_t* dst, uint32_t src, sysint_t w);
typedef void (FOG_FASTCALL *SpanSolidMskFn)(
  uint8_t* dst, uint32_t src, const uint8_t* msk, sysint_t w);

typedef void (FOG_FASTCALL *RectSolidFn)(
  uint8_t* dst, uint32_t src,
  sysint_t dstStride, sysint_t srcStride, sysint_t w);
typedef void (FOG_FASTCALL *RectSolidMskFn)(
  uint8_t* dst, uint32_t src, const uint8_t* msk,
  sysint_t dstStride, sysint_t srcStride, sysint_t mskStride, sysint_t w);

typedef void (FOG_FASTCALL *SpanCompositeFn)(
  uint8_t* dst, const uint8_t* src, sysint_t w);
typedef void (FOG_FASTCALL *SpanCompositeMskFn)(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w);
typedef void (FOG_FASTCALL *SpanCompositeMskConstFn)(
  uint8_t* dst, const uint8_t* src, uint32_t msk, sysint_t w);

typedef void (FOG_FASTCALL *SpanCompositeIndexedFn)(
  uint8_t* dst, const uint8_t* src, sysint_t w,
  const Rgba* pal);
typedef void (FOG_FASTCALL *SpanCompositeIndexedMskFn)(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w,
  const Rgba* pal);
typedef void (FOG_FASTCALL *SpanCompositeIndexedMskConstFn)(
  uint8_t* dst, const uint8_t* src, uint32_t msk, sysint_t w,
  const Rgba* pal);

typedef void (FOG_FASTCALL *RectCompositeFn)(
  uint8_t* dst, const uint8_t* src,
  sysint_t dstStride, sysint_t srcStride, sysint_t w);
typedef void (FOG_FASTCALL *RectCompositeMskFn)(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk,
  sysint_t dstStride, sysint_t srcStride, sysint_t mskStride, sysint_t w);
typedef void (FOG_FASTCALL *RectCompositeMskConstFn)(
  uint8_t* dst, const uint8_t* src, uint32_t msk,
  sysint_t dstStride, sysint_t srcStride, sysint_t mskStride, sysint_t w);

typedef void (FOG_FASTCALL *RectCompositeIndexedFn)(
  uint8_t* dst, const uint8_t* src,
  sysint_t dstStride, sysint_t srcStride, sysint_t w,
  const Rgba* pal);
typedef void (FOG_FASTCALL *RectCompositeIndexedMskFn)(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk,
  sysint_t dstStride, sysint_t srcStride, sysint_t mskStride, sysint_t w,
  const Rgba* pal);
typedef void (FOG_FASTCALL *RectCompositeIndexedMskConstFn)(
  uint8_t* dst, const uint8_t* src, uint32_t msk,
  sysint_t dstStride, sysint_t srcStride, sysint_t mskStride, sysint_t w,
  const Rgba* pal);

struct PatternContext;

typedef err_t (FOG_FASTCALL *PatternContextInitFn)(
  PatternContext* ctx, const Pattern& pattern);

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

    // Must be dereferenced when context is destroyed: d->deref().
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
    uint32_t* colors;
  };

  struct LinearGradient
  {
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
    int colorsAlloc;
    int colorsLength;
    uint32_t* colors;

    double px;
    double py;
    double fx;
    double fy;
    double r;
    double r2;
    double fx2;
    double fy2;
    double mul;
  };

  union
  {
    Texture texture;
    GenericGradient genericGradient;
    LinearGradient linearGradient;
    RadialGradient radialGradient;
  };
};

struct FunctionMap
{
  // [Convert Table]

  struct Convert
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
    ConvertIndexedFn prgb32_from_i8;

    ConvertPlainFn prgb32_bs_from_argb32;
    ConvertIndexedFn prgb32_bs_from_i8;

    // [Rgb32 Dest]

    ConvertPlainFn rgb32_from_argb32;
    ConvertPlainFn rgb32_from_argb32_bs;
    ConvertPlainFn rgb32_from_rgb24;
    ConvertPlainFn rgb32_from_bgr24;
    ConvertPlainFn rgb32_from_rgb16_5550;
    ConvertPlainFn rgb32_from_rgb16_5550_bs;
    ConvertPlainFn rgb32_from_rgb16_5650;
    ConvertPlainFn rgb32_from_rgb16_5650_bs;
    ConvertIndexedFn rgb32_from_i8;

    ConvertPlainFn rgb32_bs_from_rgb24;

    // [Rgb24/Bgr24 Dest]

    ConvertPlainFn rgb24_from_rgb32;
    ConvertPlainFn rgb24_from_rgb32_bs;
    ConvertPlainFn rgb24_from_rgb16_5550;
    ConvertPlainFn rgb24_from_rgb16_5550_bs;
    ConvertPlainFn rgb24_from_rgb16_5650;
    ConvertPlainFn rgb24_from_rgb16_5650_bs;
    ConvertIndexedFn rgb24_from_i8;

    ConvertPlainFn bgr24_from_rgb32;
    ConvertIndexedFn bgr24_from_i8;

    // [Rgb16 Dest]

    ConvertPlainFn rgb16_5550_from_rgb32;
    ConvertPlainFn rgb16_5550_from_rgb24;
    ConvertIndexedFn rgb16_5550_from_i8;

    ConvertPlainFn rgb16_5650_from_rgb32;
    ConvertPlainFn rgb16_5650_from_rgb24;
    ConvertIndexedFn rgb16_5650_from_i8;

    ConvertPlainFn rgb16_5550_bs_from_rgb32;
    ConvertPlainFn rgb16_5550_bs_from_rgb24;
    ConvertIndexedFn rgb16_5550_bs_from_i8;

    ConvertPlainFn rgb16_5650_bs_from_rgb32;
    ConvertPlainFn rgb16_5650_bs_from_rgb24;
    ConvertIndexedFn rgb16_5650_bs_from_i8;

    // [Greyscale]

    ConvertPlainFn greyscale8_from_rgb32;
    ConvertPlainFn greyscale8_from_rgb24;
    ConvertIndexedFn greyscale8_from_i8;

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

  Convert convert;

  // [Gradient Table]

  struct Gradient
  {
    // [GradientSpan]

    GradientSpanFn gradient_argb32;
    GradientSpanFn gradient_prgb32;
    GradientSpanFn gradient_rgb32;
    GradientSpanFn gradient_rgb24;
    GradientSpanFn gradient_a8;
  };

  Gradient gradient;

  // [Pattern Table]

  struct Pattern_
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
  };

  Pattern_ pattern;

  // [Raster Table]

  struct Raster
  {
    // [Pixel]

    PixelFn             pixel;
    PixelMskFn          pixel_a8;

    // [Span Solid]

    SpanSolidFn         span_solid;
    SpanSolidMskFn      span_solid_a8;

    // [Rect Solid]

    RectSolidFn         rect_solid;
    RectSolidMskFn      rect_solid_a8;

    // [Span Composite]

    // NOTE: There are two versions of funtions, but you can call only one
    // of them. The indexed version is only for Image::FormatI8. Painter and
    // Fog library knows about this. The indexed version is only used when
    // blitting or tiling image to destination, all transformations
    // and other operations uses 24 bit or 32 bit formats, because it's needed
    // for antialiasing.

    union {
      SpanCompositeFn span_composite[Image::FormatCount];
      SpanCompositeIndexedFn span_composite_indexed[Image::FormatCount];
    };

    union {
      SpanCompositeMskFn span_composite_a8[Image::FormatCount];
      SpanCompositeIndexedMskFn span_composite_indexed_a8[Image::FormatCount];
    };

    // [Rect Composite]

    union {
      RectCompositeFn rect_composite[Image::FormatCount];
      RectCompositeIndexedFn rect_composite_indexed[Image::FormatCount];
    };

    union {
      RectCompositeMskFn rect_composite_a8[Image::FormatCount];
      RectCompositeIndexedMskFn rect_composite_indexed_a8[Image::FormatCount];
    };
  };
  
  // 0 = ARGB32, 1 = ARGB32 premultiplied.
  Raster raster_argb32[2][CompositeCount];
  Raster raster_rgb32;
  Raster raster_rgb24;
};

extern FOG_API FunctionMap* functionMap;

FOG_API FunctionMap::Raster* getRasterOps(int format, int op);

} // Raster namespace
} // Fog namespace

// [Guard]
#endif // _FOG_GRAPHICS_RASTER_H
