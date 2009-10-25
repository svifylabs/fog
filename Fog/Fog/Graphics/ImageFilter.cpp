// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled Headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/ManagedString.h>
#include <Fog/Core/Math.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/Misc.h>
#include <Fog/Core/Strings.h>
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

// [Properties]

Static<PropertiesData> ImageFilter::_propertiesData;

int ImageFilter::propertyInfo(int id) const
{
  switch (id)
  {
    case PropertyFilterType:
      return IProperties::Exists | IProperties::ReadOnly;
    default:
      return base::propertyInfo(id);
  }
}

err_t ImageFilter::getProperty(int id, Value& value) const
{
  switch (id)
  {
    case PropertyFilterType:
      return value.setInt32(_type);
    default:
      return base::getProperty(id, value);
  }
}

err_t ImageFilter::setProperty(int id, const Value& value)
{
  switch (id)
  {
    case PropertyFilterType:
      return Error::PropertyIsReadOnly;
    default:
      return base::setProperty(id, value);
  }
}

// [Nop]

bool ImageFilter::isNop() const
{
  return false;
}

// [Filters]

err_t ImageFilter::filterImage(Image& dst, const Image& src) const
{
  int w = src.getWidth();
  int h = src.getHeight();
  int format = src.getFormat();

  if (isNop()) return dst.set(src);

  err_t err = (&dst == &src) ? dst.detach() : dst.create(w, h, format);
  if (err) return err;

  return filterData(dst.xFirst(), dst.getStride(), src.cFirst(), src.getStride(), w, h, format);
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
      demultiply(dstCur, srcCur, width, NULL);
    }

    err_t err = filterPrivate(dst, dstStride, dst, dstStride, width, height, format);

    for (y = height, dstCur = dst; y; y--, dstCur += dstStride)
    {
      premultiply(dstCur, dstCur, width, NULL);
    }

    return err;
  }
  else
  {
    return filterPrivate(dst, dstStride, src, srcStride, width, height, format);
  }
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

BlurImageFilter::BlurImageFilter() :
  _blurType(BlurTypeStack),
  _hRadius(1.0),
  _vRadius(1.0),
  _borderMode(BorderModeExtend),
  _borderColor(0x00000000)
{
  _setupFilterType();
}

BlurImageFilter::BlurImageFilter(int blurType, double hRadius, double vRadius, int borderMode, uint32_t borderColor) :
  _blurType(blurType),
  _hRadius(hRadius),
  _vRadius(vRadius),
  _borderMode(borderMode),
  _borderColor(borderColor)
{
  _setupFilterType();
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

Static<PropertiesData> BlurImageFilter::_propertiesData;

err_t BlurImageFilter::getProperty(int id, Value& value) const
{
  switch (id)
  {
    case PropertyBorderMode:
      return value.setInt32(_borderMode);
    case PropertyBorderColor:
      return value.setInt32(_borderColor);
    case PropertyBlurType:
      return value.setInt32(_blurType);
    case PropertyHorizontalRadius:
      return value.setDouble(_hRadius);
    case PropertyVerticalRadius:
      return value.setDouble(_vRadius);
    default:
      return base::getProperty(id, value);
  }
}

err_t BlurImageFilter::setProperty(int id, const Value& value)
{
  err_t err;
  int i;
  double d;

  switch (id)
  {
    case PropertyBorderMode:
      if ((err = value.getInt32(&i))) return err;
      return setBorderMode(i);
    case PropertyBorderColor:
      if ((err = value.getInt32(&i))) return err;
      return setBorderColor((uint32_t)i);
    case PropertyBlurType:
      if ((err = value.getInt32(&i))) return err;
      return setBlurType(i);
    case PropertyHorizontalRadius:
      if ((err = value.getDouble(&d))) return err;
      return setHorizontalRadius(d);
    case PropertyVerticalRadius:
      if ((err = value.getDouble(&d))) return err;
      return setVerticalRadius(d);
    default:
      return base::setProperty(id, value);
  }
}

// [Blur Type]

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

// [Border Mode]

err_t BlurImageFilter::setBorderMode(int borderMode)
{
  if (borderMode >= BorderModeInvalid) return Error::InvalidPropertyValue;

  _borderMode = borderMode;
  return Error::Ok;
}

// [Border Color]

err_t BlurImageFilter::setBorderColor(uint32_t borderColor)
{
  _borderColor = borderColor;
  return Error::Ok;
}

// [Nop]

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

// [Filters]

err_t BlurImageFilter::filterPrivate(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int format) const
{
  if (isNop())
  {
    if (dst != src) Raster::functionMap->filter.copyArea[format](dst, dstStride, src, srcStride, width, height);
    return Error::Ok;
  }

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
        convolveH = Raster::functionMap->filter.boxBlurConvolveH[format];
        convolveV = Raster::functionMap->filter.boxBlurConvolveV[format];
      }
      else
      {
        convolveH = Raster::functionMap->filter.stackBlurConvolveH[format];
        convolveV = Raster::functionMap->filter.stackBlurConvolveV[format];
      }

      convolveH(dst, dstStride, src, srcStride, width, height, hRadiusInt, _borderMode, _borderColor);
      convolveV(dst, dstStride, dst, dstStride, width, height, vRadiusInt, _borderMode, _borderColor);
      break;
    }

    case BlurTypeGaussian:
    {
      sysint_t bufStride = height * Image::formatToBytesPerPixel(format);
      if (bufStride == 0) return Error::InvalidArgument;

      float* hKernel;
      float* vKernel;

      float hKernelDiv;
      float vKernelDiv;

      MemoryBuffer<512> kernelMemory;
      hKernel = (float*)kernelMemory.alloc((hKernelSize + vKernelSize) * sizeof(float));
      if (!hKernel) return Error::OutOfMemory;
      vKernel = hKernel + hKernelSize;

      Raster::FloatScanlineConvolveFn convolveH = Raster::functionMap->filter.floatScanlineConvolveH[format];
      Raster::FloatScanlineConvolveFn convolveV = Raster::functionMap->filter.floatScanlineConvolveV[format];

      hKernelDiv = makeGaussianBlurKernel(hKernel, hRadius, hKernelSize);
      vKernelDiv = makeGaussianBlurKernel(vKernel, vRadius, vKernelSize);

      convolveH(dst, dstStride, src, srcStride,
        width, height, hKernel, hKernelSize, hKernelDiv, _borderMode, _borderColor);
      convolveV(dst, dstStride, dst, dstStride,
        width, height, vKernel, vKernelSize, vKernelDiv, _borderMode, _borderColor);

      break;
    }
  }

  return err;
}

void BlurImageFilter::_setupFilterType()
{
  _type = FilterTypeBlur;
  _flags = OnlyNonPremultiplied | TwoPasses;
}

// ============================================================================
// [Fog::IntConvolutionMatrix]
// ============================================================================

Static<IntConvolutionMatrix::Data> IntConvolutionMatrix::sharedNull;

IntConvolutionMatrix::IntConvolutionMatrix() :
  _d(sharedNull->ref())
{
}

IntConvolutionMatrix::IntConvolutionMatrix(const IntConvolutionMatrix& other) :
  _d(other._d->ref())
{
}

IntConvolutionMatrix::~IntConvolutionMatrix()
{
  _d->deref();
}

err_t IntConvolutionMatrix::_detach()
{
  if (_d->refCount.get() == 1) return Error::Ok;

  Data* newd = Data::create(_d->width, _d->height);
  if (!newd) return Error::OutOfMemory;

  Data::copy(newd, 0, 0, _d, 0, 0, _d->width, _d->height);
  return Error::Ok;
}

err_t IntConvolutionMatrix::create(int width, int height)
{
  if (_d->width == width && _d->height == height) return Error::Ok;

  Data* newd = Data::create(width, height);
  if (!newd) return Error::OutOfMemory;

  AtomicBase::ptr_setXchg(&_d, newd)->deref();
  return Error::Ok;
}

err_t IntConvolutionMatrix::extend(int width, int height, ValueType value)
{
  if (_d->width == width && _d->height == height) return Error::Ok;

  Data* newd = Data::create(width, height);
  if (!newd) return Error::OutOfMemory;
  
  int copyw = Math::min(_d->width, width);
  int copyh = Math::min(_d->height, height);
  int zerow = width - copyw;

  int x, y;

  ValueType *dstCur = newd->m;
  ValueType* srcM = _d->m;

  for (y = copyh; y; y--, srcM += _d->width)
  {
    ValueType *srcCur = srcM;

    for (x = copyw; x; x--, dstCur++, srcCur++) *dstCur++ = *srcCur++;
    for (x = zerow; x; x--) *dstCur++ = value;
  }

  for (y = copyh; y; y--)
  {
    for (x = width; x; x--) *dstCur++ = value;
  }

  AtomicBase::ptr_setXchg(&_d, newd)->deref();
  return Error::Ok;
}

int IntConvolutionMatrix::get(int x, int y) const
{
  Data* d = _d;
  if ((uint)x >= (uint)d->width || (uint)y >= (uint)_d->height) return 0;

  return d->m[y * d->width + x];
}

void IntConvolutionMatrix::set(int x, int y, int val)
{
  Data* d = _d;
  if ((uint)x >= (uint)d->width || (uint)y >= (uint)_d->height) return;

  if (FOG_UNLIKELY(!isDetached()))
  {
    if (_detach()) return;
    d = _d;
  }

  d->m[y * d->width + x] = val;
}

IntConvolutionMatrix& IntConvolutionMatrix::operator=(const IntConvolutionMatrix& other)
{
  AtomicBase::ptr_setXchg(&_d, other._d->ref())->deref();
  return *this;
}

int* IntConvolutionMatrix::operator[](int y)
{
  Data* d = _d;
  if ((uint)y >= (uint)_d->height) return NULL;

  if (FOG_UNLIKELY(!isDetached()))
  {
    if (_detach()) return NULL;
    d = _d;
  }

  return d->m + y * d->width;
}

IntConvolutionMatrix::Data* IntConvolutionMatrix::Data::create(int w, int h)
{
  if (w == 0 || h == 0) return sharedNull->ref();

  Data* d = (Data*)Memory::alloc(sizeof(Data) + (w * h) * sizeof(ValueType));
  if (!d) return NULL;

  d->refCount.init(1);
  d->width = w;
  d->height = h;
  return d;
}

void IntConvolutionMatrix::Data::copy(Data* dst, int dstX, int dstY, Data* src, int srcX, int srcY, int w, int h)
{
  ValueType* dstM = dst->m + dstY * dst->width + dstX;
  ValueType* srcM = src->m + srcY * src->width + srcX;

  for (int y = h; y; y--, dstM += dst->width, srcM += src->width)
  {
    ValueType* dstCur = dstM;
    ValueType* srcCur = srcM;

    for (int x = w; x; x--, dstCur++, srcCur++) *dstCur++ = *srcCur++;
  }
}

// ============================================================================
// [Fog::IntConvolutionImageFilter]
// ============================================================================

IntConvolutionImageFilter::IntConvolutionImageFilter() :
  _borderMode(BorderModeExtend),
  _borderColor(0x00000000)
{
  _setupFilterType();
}

IntConvolutionImageFilter::IntConvolutionImageFilter(const IntConvolutionMatrix& kernel, int borderMode, uint32_t borderColor) :
  _kernel(kernel),
  _borderMode(borderMode),
  _borderColor(borderColor)
{
  _setupFilterType();
}

IntConvolutionImageFilter::~IntConvolutionImageFilter()
{
}

// [Clone]

ImageFilter* IntConvolutionImageFilter::clone() const
{
  return new(std::nothrow) IntConvolutionImageFilter(_kernel, _borderMode, _borderColor);
}

// [Properties]

Static<PropertiesData> IntConvolutionImageFilter::_propertiesData;

err_t IntConvolutionImageFilter::getProperty(int id, Value& value) const
{
  switch (id)
  {
    case PropertyBorderMode:
      return value.setInt32(_borderMode);
    case PropertyBorderColor:
      return value.setInt32(_borderColor);
    case PropertyKernel:
      return Error::NotImplemented;
    default:
      return base::getProperty(id, value);
  }
}

err_t IntConvolutionImageFilter::setProperty(int id, const Value& value)
{
  err_t err;
  int i;

  switch (id)
  {
    case PropertyBorderMode:
      if ((err = value.getInt32(&i))) return err;
      return setBorderMode(i);
    case PropertyBorderColor:
      if ((err = value.getInt32(&i))) return err;
      return setBorderColor((uint32_t)i);
    case PropertyKernel:
      return Error::NotImplemented;
    default:
      return base::setProperty(id, value);
  }
}

// [Kernel]

err_t IntConvolutionImageFilter::setKernel(const IntConvolutionMatrix& kernel)
{
  _kernel = kernel;
  return Error::Ok;
}

// [Border Type]

err_t IntConvolutionImageFilter::setBorderMode(int borderMode)
{
  if (borderMode >= BorderModeInvalid) return Error::InvalidPropertyValue;

  _borderMode = borderMode;
  return Error::Ok;
}

// [Border Color]

err_t IntConvolutionImageFilter::setBorderColor(uint32_t borderColor)
{
  _borderColor = borderColor;
  return Error::Ok;
}

// [Nop]

bool IntConvolutionImageFilter::isNop() const
{
  return _kernel.isEmpty();
}

// [Filters]

err_t IntConvolutionImageFilter::filterPrivate(
  uint8_t* dst, sysint_t dstStride,
  const uint8_t* src, sysint_t srcStride,
  int width, int height, int format) const
{
  return Error::NotImplemented;
}

void IntConvolutionImageFilter::_setupFilterType()
{
  _type = FilterTypeConvolution;
  _flags = OnlyNonPremultiplied | OnePass;
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_imagefilter_init(void)
{
  using namespace Fog;

  Vector<String> properties;

  properties.clear();
  properties.append(fog_strings->getString(STR_GRAPHICS_filterType));
  FOG_INIT_PROPERTIES_CONTAINER(ImageFilter, ImageFilter::base, properties);

  properties.clear();
  properties.append(fog_strings->getString(STR_GRAPHICS_borderMode));
  properties.append(fog_strings->getString(STR_GRAPHICS_borderColor));
  properties.append(fog_strings->getString(STR_GRAPHICS_blurType));
  properties.append(fog_strings->getString(STR_GRAPHICS_horizontalRadius));
  properties.append(fog_strings->getString(STR_GRAPHICS_verticalRadius));
  FOG_INIT_PROPERTIES_CONTAINER(BlurImageFilter, BlurImageFilter::base, properties);

  properties.clear();
  properties.append(fog_strings->getString(STR_GRAPHICS_borderMode));
  properties.append(fog_strings->getString(STR_GRAPHICS_borderColor));
  properties.append(fog_strings->getString(STR_GRAPHICS_kernel));
  FOG_INIT_PROPERTIES_CONTAINER(IntConvolutionImageFilter, IntConvolutionImageFilter::base, properties);

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_imagefilter_shutdown(void)
{
  using namespace Fog;

  FOG_DESTROY_PROPERTIES_CONTAINER(IntConvolutionImageFilter);
  FOG_DESTROY_PROPERTIES_CONTAINER(BlurImageFilter);
  FOG_DESTROY_PROPERTIES_CONTAINER(ImageFilter);
}
