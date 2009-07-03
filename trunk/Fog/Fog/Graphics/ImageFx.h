// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

#ifndef _FOG_GRAPHICS_IMAGEFX_H
#define _FOG_GRAPHICS_IMAGEFX_H

#include <Fog/Graphics/Image.h>

namespace Fog {
namespace ImageFx {

// ============================================================================
// [Fog::ImageFx::EdgeMode]
// ============================================================================

enum EdgeMode
{
  EdgeModeAuto,
  EdgeModeColor
};

// ============================================================================
// [Fog::ImageFx::FunctionMap]
// ============================================================================

typedef void (*ConvolveLineIntegerFn)(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, const int* kernel, int size, int divide,
  int edgeMode, uint32_t edgeColor);

typedef void (*ConvolveLineFloatFn)(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, const float* kernel, int size, float divide,
  int edgeMode, uint32_t edgeColor);

struct FunctionMap
{
  ConvolveLineIntegerFn convolveLineInteger[Image::FormatCount];
  ConvolveLineFloatFn convolveLineFloat[Image::FormatCount];
};

// ============================================================================
// [Fog::ImageFx::Filters]
// ============================================================================

FOG_API err_t convolveSymmetricFloat(Image& dst, const Image& src, 
  const float* hKernel, int hKernelSize, float hKernelDiv,
  const float* vKernel, int vKernelSize, float vKernelDiv,
  int edgeMode, uint32_t edgeColor);

FOG_API err_t boxBlur(Image& dst, const Image& src, int hRadius, int vRadius, int edgeMode, uint32_t edgeColor);
FOG_API err_t stackBlur(Image& dst, const Image& src, int hRadius, int vRadius, int edgeMode, uint32_t edgeColor);
FOG_API err_t gaussianBlur(Image& dst, const Image& src, double hRadius, double vRadius, int edgeMode, uint32_t edgeColor);

} // ImageFx namespace
} // Fog namespae

#endif // _FOG_GRAPHICS_IMAGEFX_H
