// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/Math.h>
#include <Fog/Core/Misc.h>
#include <Fog/Graphics/ImageFilter.h>
#include <Fog/Graphics/Raster.h>

namespace Fog {

// ============================================================================
// [Fog::ImageFilter]
// ============================================================================

ImageFilter::ImageFilter() : 
  _type(FilterTypeNone),
  _flags(0)
{
}

ImageFilter::~ImageFilter()
{
}

bool ImageFilter::isNop() const
{
  return false;
}

err_t ImageFilter::filterImage(Image& dst, const Image& src) const
{
  int w = src.width();
  int h = src.height();
  int format = src.format();

  if (isNop()) return dst.set(src);

  err_t err = (&dst == &src) ? dst.detach() : dst.create(w, h, format);
  if (err) return err;

  return filterData(dst.xFirst(), dst.stride(), src.cFirst(), src.stride(), w, h, format);
}

err_t ImageFilter::filterData(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int format) const
{
  if (format == Image::FormatNull || format >= Image::FormatCount)
  {
    return Error::InvalidArgument;
  }

  // If filter doesn't support premultiplied pixel format, we will demultiply,
  // filter and back premultiply. The temporary buffer is dst.
  if ((_flags & OnlyNonPremultiplied) != 0 && format == Image::FormatPRGB32)
  {
    Raster::ConvertPlainFn premultiply = Raster::functionMap->convert.argb32_from_prgb32;
    Raster::ConvertPlainFn demultiply = Raster::functionMap->convert.prgb32_from_argb32;

    int y;
    uint8_t* dstCur;
    const uint8_t* srcCur;

    for (y = height, srcCur = src, dstCur = dst; y; y--, srcCur += srcStride, dstCur += dstStride)
    {
      demultiply(dstCur, srcCur, width);
    }

    err_t err = filterPrivate(dst, dstStride, dst, dstStride, width, height, format);

    for (y = height, dstCur = dst; y; y--, dstCur += dstStride)
    {
      premultiply(dstCur, dstCur, width);
    }

    return err;
  }
  else
  {
    return filterPrivate(dst, dstStride, src, srcStride, width, height, format);
  }
}

err_t ImageFilter::setProperty(const String32& name, const Value& value)
{
  return Error::InvalidPropertyName;
}

Value ImageFilter::getProperty(const String32& name) const
{
  return Value();
}

// ============================================================================
// [Fog::BlurImageFilter]
// ============================================================================

// This function is designed to make kernel for gaussian blur matrix. See
// wikipedia (http://en.wikipedia.org/wiki/Gaussian_function) for equations.
static float makeGaussianBlurKernel(float* dst, double radius, int size)
{
  FOG_ASSERT(dst);

  int i, pos;

  double sigma = radius / 3.0;
  double sigma2 = sigma * sigma;
  double radius2 = radius * radius;

  // Reciprocals.
  double re = 1.0 / (2.0 * sigma2);
  double rs = 1.0 / (float)sqrt(2.0 * M_PI * sigma);

  double total = 0;

  for (i = 0, pos = -(int)ceil(radius); i < size; i++, pos++)
  {
    double dist = (double)pos * (double)pos;
    double val = exp(-dist * re) * rs;
    if (val < 0.0) val = 0.0;
    total += val;
    dst[i] = (float)val;
  }

  // Error correction.
  if (total >= 0.001) total -= 0.001;
  return (float)total;
}
/*
static err_t convolveSymmetricFloat(
  uint8_t* dst, sysint_t dstStride, 
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int format,
  const float* hKernel, int hKernelSize, float hKernelDiv,
  const float* vKernel, int vKernelSize, float vKernelDiv,
  int borderMode, uint32_t borderColor)
{
  // Kernel sizes must be odd.
  if ((hKernelSize & 1) == 0 || (vKernelSize & 1) == 0) return Error::InvalidArgument;

  uint8_t* buf = NULL;
  sysint_t bufStride = height * 4;

  buf = (uint8_t*)Memory::alloc(width * height * 4);
  if (!buf) return Error::OutOfMemory;

  Raster::FloatScanlineConvolveFn convolve = Raster::functionMap->filters.floatScanlineConvolve[format];

  convolve(buf, bufStride, src, srcStride,
    width, height, hKernel, hKernelSize, hKernelDiv, borderMode, borderColor);
  convolve(dst, dstStride, buf, bufStride,
    height, width, vKernel, vKernelSize, vKernelDiv, borderMode, borderColor);

  Memory::free(buf);
  return Error::Ok;
}
*/
BlurImageFilter::BlurImageFilter() :
  _blurType(BlurTypeStack),
  _hRadius(1.0),
  _vRadius(1.0),
  _borderMode(BorderModeExtend),
  _borderColor(0x00000000)
{
  _setupFilter();
}

BlurImageFilter::BlurImageFilter(int blurType, double hRadius, double vRadius, int borderMode, uint32_t borderColor) :
  _blurType(blurType),
  _hRadius(hRadius),
  _vRadius(vRadius),
  _borderMode(borderMode),
  _borderColor(borderColor)
{
  _setupFilter();
}

BlurImageFilter::~BlurImageFilter()
{
}

// [Clone]

ImageFilter* BlurImageFilter::clone() const
{
  return new(std::nothrow) BlurImageFilter(_blurType, _hRadius, _vRadius, _borderMode, _borderColor);
}

// [Properties]

err_t BlurImageFilter::setProperty(const String32& name, const Value& value)
{
  // TODO
  return Error::NotImplemented;
}

Value BlurImageFilter::getProperty(const String32& name) const
{
  // TODO
  return Value();
}

err_t BlurImageFilter::setBlurType(int blurType)
{
  if (blurType >= BlurTypeInvalid) return Error::InvalidPropertyValue;

  _blurType = blurType;
  return Error::Ok;
}

err_t BlurImageFilter::setHorizontalRadius(double hr)
{
  _hRadius = hr;
  return Error::Ok;
}

err_t BlurImageFilter::setVerticalRadius(double vr)
{
  _vRadius = vr;
  return Error::Ok;
}

err_t BlurImageFilter::setBorderMode(int borderMode)
{
  if (borderMode >= BorderModeInvalid) return Error::InvalidPropertyValue;

  _borderMode = borderMode;
  return Error::Ok;
}

err_t BlurImageFilter::setBorderColor(uint32_t borderColor)
{
  _borderColor = borderColor;
  return Error::Ok;
}

bool BlurImageFilter::isNop() const
{
  double hRadius = fabs(_hRadius);
  double vRadius = fabs(_vRadius);

  switch (_blurType)
  {
    case BlurTypeBox:
    case BlurTypeStack:
      return hRadius < 1.0 && vRadius < 1.0;
    case BlurTypeGaussian:
      return hRadius < 0.63 && vRadius < 0.63;
    default:
      FOG_ASSERT_NOT_REACHED();
      return true;
  }
}

err_t BlurImageFilter::filterPrivate(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int format) const
{
  double hRadius = fabs(_hRadius);
  double vRadius = fabs(_vRadius);

  if (hRadius > 254.0) hRadius = 254.0;
  if (vRadius > 254.0) vRadius = 254.0;

  int hRadiusInt = (int)ceil(hRadius);
  int vRadiusInt = (int)ceil(vRadius);

  if (hRadiusInt < 1) hRadiusInt = 1;
  if (vRadiusInt < 1) vRadiusInt = 1;

  int hKernelSize = hRadiusInt * 2 + 1;
  int vKernelSize = vRadiusInt * 2 + 1;

  switch (_blurType)
  {
    case BlurTypeBox:
    case BlurTypeStack:
    {
      if (hRadiusInt == 0 && vRadiusInt == 0)
      {
        if (dst != src) Raster::functionMap->filters.copyArea[format](dst, dstStride, src, srcStride, width, height);
        return Error::Ok;
      }
      break;
    }
    case BlurTypeGaussian:
    {
      if (vRadius <= 0.63 && hRadius <= 0.63)
      {
        if (dst != src) Raster::functionMap->filters.copyArea[format](dst, dstStride, src, srcStride, width, height);
        return Error::Ok;
      }
      break;
    }
    default:
      FOG_ASSERT_NOT_REACHED();
  }

  err_t err = Error::Ok;

  switch (_blurType)
  {
    case BlurTypeBox:
    case BlurTypeStack: 
    {
      Raster::BlurConvolveFn convolveH;
      Raster::BlurConvolveFn convolveV;

      if (_blurType == BlurTypeBox)
      {
        convolveH = Raster::functionMap->filters.boxBlurConvolveH[format];
        convolveV = Raster::functionMap->filters.boxBlurConvolveV[format];
      }
      else
      {
        convolveH = Raster::functionMap->filters.stackBlurConvolveH[format];
        convolveV = Raster::functionMap->filters.stackBlurConvolveV[format];
      }

      convolveH(dst, dstStride, src, srcStride, width, height, hRadiusInt, _borderMode, _borderColor);
      convolveV(dst, dstStride, dst, dstStride, width, height, vRadiusInt, _borderMode, _borderColor);
      break;
    }

    case BlurTypeGaussian:
    {
      sysint_t bufStride = height * Image::formatToBytesPerPixel(format);
      if (bufStride == 0) return Error::InvalidArgument;

      uint8_t* buf = (uint8_t*)Memory::alloc(width * bufStride);
      if (!buf) return Error::OutOfMemory;

      float* hKernel;
      float* vKernel;

      float hKernelDiv;
      float vKernelDiv;

      MemoryBuffer<512> kernelMemory;
      hKernel = (float*)kernelMemory.alloc((hKernelSize + vKernelSize) * sizeof(float));
      if (!hKernel) return Error::OutOfMemory;
      vKernel = hKernel + hKernelSize;

      hKernelDiv = makeGaussianBlurKernel(hKernel, hRadius, hKernelSize);
      vKernelDiv = makeGaussianBlurKernel(vKernel, vRadius, vKernelSize);

      Raster::FloatScanlineConvolveFn convolve = Raster::functionMap->filters.floatScanlineConvolve[format];

      convolve(buf, bufStride, src, srcStride,
        width, height, hKernel, hKernelSize, hKernelDiv, _borderMode, _borderColor);
      convolve(dst, dstStride, buf, bufStride,
        height, width, vKernel, vKernelSize, vKernelDiv, _borderMode, _borderColor);

      Memory::free(buf);
      break;
    }
  }

  return err;
}

void BlurImageFilter::_setupFilter()
{
  _type = FilterTypeBlur;
  _flags = OnlyNonPremultiplied | TwoPasses;
}

} // Fog namespace
