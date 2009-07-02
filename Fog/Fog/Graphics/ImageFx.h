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
// [Fog::ImageFx]
// ============================================================================

enum EdgeMode
{
  EdgeModeAuto,
  EdgeModeColor
};

FOG_API err_t convolveSymmetricFloat(Image& dst, const Image& src, 
  const float* hKernel, int hKernelSize,
  const float* vKernel, int vKernelSize,
  int edgeMode, uint32_t edgeColor);

FOG_API err_t boxBlur(Image& dst, const Image& src, int hRadius, int vRadius, int edgeMode, uint32_t edgeColor);
FOG_API err_t stackBlur(Image& dst, const Image& src, int hRadius, int vRadius, int edgeMode, uint32_t edgeColor);
FOG_API err_t gaussianBlur(Image& dst, const Image& src, double hRadius, double vRadius, int edgeMode, uint32_t edgeColor);

} // ImageFx namespace
} // Fog namespae

#endif _FOG_GRAPHICS_IMAGEFX_H
