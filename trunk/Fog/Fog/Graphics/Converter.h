// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_CONVERTER_H
#define _FOG_GRAPHICS_CONVERTER_H

// [Dependencies]
#include <Fog/Core/Atomic.h>
#include <Fog/Core/Cpu.h>
#include <Fog/Core/RefData.h>
#include <Fog/Core/Static.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/Graphics/ImageFormat.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Fog::Converter]
// ============================================================================

//! @brief Pixel converter is powerful conversion tool between different
//! pixel formats.
struct FOG_API Converter
{
  // Forward
  struct Data;
  struct Params;

  //! @brief Flags that can be set in @c Fog::Converter::Format::flags.
  enum Flags
  {
    //! @brief No flags.
    NoFlags = 0,
    //! @brief Indexed pixel format.
    Indexed = (1 << 0),
    //! @brief Premultiplied rgba format.
    Premultiplied = (1 << 1),
    //! @brief Byteswap input / output.
    ByteSwap = (1 << 2)
  };

  //! @brief Standard pixel converter format IDs.
  //!
  //! Pixel converter is able to process more pixel formats, but formats
  //! defined here are de facto standard. In rare situations are needed
  //! something more.
  //!
  //! Of course, Fog not contains all-to-all coverters, so some converters
  //! uses two-pass process that is slower, but implemented.
  //!
  //! Most useful converters are implemented in one-pass process and also
  //! by MMX or SSE2 instructions, so it's very fast to use converter
  //! instead of own solutions.
  enum IdEnum
  {
    // Image formats compatible

    //! @brief 32 bit ARGB format with non-premultiplited alpha values.
    ARGB32              = ImageFormat::ARGB32,
    //! @brief 32 bit ARGB format with premultiplited alpha values.
    PRGB32              = ImageFormat::PRGB32,
    //! @brief 32 bit RGB format without alpha channel.
    XRGB32              = ImageFormat::XRGB32,

    //! @brief 24 bit RGB format.
    RGB24               = ImageFormat::RGB24,
    //! @brief 24 bit BGR format.
    BGR24               = ImageFormat::BGR24,

    //! @brief 8 bit indexed format.
    I8                  = ImageFormat::I8,
    //! @brief 8 bit alpha channel.
    A8                  = ImageFormat::A8,
    //! @brief 1 bit alpha channel (mask).
    A1                  = ImageFormat::A1,

    // Extended formats

    //! Similar to @c Fog::Converter::ARGB32, but byteswapped.
    BGRA32              = ImageFormat::Count + 0,
    //! Similar to @c Fog::Converter::PRGB32, but byteswapped.
    BGRP32              = ImageFormat::Count + 1,
    //! Similar to @c Fog::Converter::XRGB32, but byteswapped.
    BGRX32              = ImageFormat::Count + 2,

    //! 16 bit pixel format. 5 values for red, 6 for green and 5 for blue.
    RGB16_5650          = ImageFormat::Count + 3,
    //! 16 bit pixel format. 5 values for red, 5 for green and 5 for blue.
    RGB16_5550          = ImageFormat::Count + 4,
    //! 16 bit pixel format. 5 values for red, 5 for green, 5 for blue and 1 for alpha channel (mask).
    RGB16_5551          = ImageFormat::Count + 5,

    //! Similar to @c Fog::Converter::RGB16_5650, but byteswapped.
    RGB16_5650_ByteSwap = ImageFormat::Count + 6,
    //! Similar to @c Fog::Converter::RGB16_5550, but byteswapped.
    RGB16_5550_ByteSwap = ImageFormat::Count + 7,
    //! Similar to @c Fog::Converter::RGB16_5551, but byteswapped.
    RGB16_5551_ByteSwap = ImageFormat::Count + 8,

    //! 4 bit indexed format.
    I4                  = ImageFormat::Count + 9,
    //! 1 bit indexed format.
    I1                  = ImageFormat::Count + 10,

    //! 8 bit indexed format with rgb 2x3x2 color cube.
    I8_RGB232           = ImageFormat::Count + 11,
    //! 8 bit indexed format with rgb 2x2x2 color cube.
    I8_RGB222           = ImageFormat::Count + 12,
    //! 8 bit indexed format with rgb 1x1x1 color cube.
    I8_RGB111           = ImageFormat::Count + 13,

    //! @brief 4 bit alpha channel.
    A4                  = ImageFormat::Count + 14,

    //! @brief Count of image converter built-in formats.
    Count               = ImageFormat::Count + 15,

    //! @brief Uninitialized pixel converter.
    Invalid             = 0xFFFFFFFE,

    //! @brief Image converter extension registered in runtime.
    Extension           = 0xFFFFFFFF
  };

  // [SpanFunc]

  //! @brief Prototype for pixel converter function.
  typedef void (FOG_OPTIMIZEDCALL *SpanFunc)
  (
    //! @brief Pixel converter data.
    const Data* d,
    //! @brief Pixel converter parameters.
    const Params* params
  );

  // [Params]

  struct Params
  {
    //! @brief Pointer to destination pixels.
    uint8_t* destData;
    //! @brief Destination horizotal offset.
    sysuint_t destX;

    //! @brief Pointer to source pixels.
    const uint8_t* srcData;
    //! @brief Source horizotal offset.
    sysuint_t srcX;

    //! @brief Width to convert.
    sysuint_t width;

    //! @brief Origin for dithering.
    Point origin;
  };

  // [Format]

  //! @Brief Pixel converter format.
  struct Format
  {
    // base parameters

    //! @brief Pixel converter format depth (bits per pixel).
    uint32_t depth;
    //! @brief Pixel converter format flags, see @c Fog::Converter::Flags.
    uint32_t flags;

    // masks

    //! @brief Red mask, must be zero for all indexed and alpha-only formats.
    uint32_t rMask;
    //! @brief Green mask, must be zero for all indexed and alpha-only formats.
    uint32_t gMask;
    //! @brief Blue mask, must be zero for all indexed and alpha-only formats.
    uint32_t bMask;
    //! @brief Alpha mask, must be zero for all indexed formats.
    uint32_t aMask;
  };

  // [Extra]

  //! @brief Pixel converter extra data.
  struct Extra
  {
  };

  // [Data]

  //! @brief Converter data directly used by @c Fog::Converter.
  struct Data : public Fog::RefDataSimple<Data>
  {
    // [Members]

    SpanFunc spanFunc;

    Format dest;
    Format source;

    // [Data layers]

    void* layer[2];

    // [Data extras]

    Fog::Static<Extra> extra;
    bool extraUsed;

    // [Construction / Destruction]

    Data()
    {
      Fog::Memory::zero(this, sizeof(Data));
      refCount.init(1);
    }

    Data(SpanFunc _spanFunc)
    {
      Fog::Memory::zero(this, sizeof(Data));
      spanFunc = _spanFunc;
    }

    ~Data()
    { 
      // free device layers
      if (layer[0]) ((Data*)layer[0])->deref();
      if (layer[1]) ((Data*)layer[1])->deref();

      // free device extras
      if (extraUsed) extra.destroy();
    }

    // [Reference / Dereference]

    Data* ref()
    {
      refCount.inc();
      return this;
    }

    void deref()
    {
      if (refCount.deref()) delete this;
    }

  private:
    FOG_DISABLE_COPY(Data)
  };

  // [Members]

  FOG_DECLARE_D(Data)

  static Fog::Static<Data> sharedNull;

  // [Construction / Destruction]

  //! @brief Creates non-initialized pixel converter.
  Converter();

  //! @brief Creates equal converter to @a other.
  Converter(const Converter& other);

  // setup() will overwrite _d, so it will not point to NULL after constructed
  // (this is simple optimization, but probabbly not so readable:( )

  //! @brief Creates pixel converter from @a source to @a dest pixel formats.
  Converter(const Format& dest, const Format& source);

  //! @brief Creates pixel converter from @a source to @a dest pixel formats.
  //!
  //! @note @a source and @a dest are pixel format IDs, see @c Fog::Converter::IdEnum.
  Converter(uint32_t dest, uint32_t source);

  //! @brief Destroys pixel converter.
  ~Converter();

  // [Implicit Sharing]

  //! @copydoc Doxygen::Implicit::refCount().
  FOG_INLINE sysuint_t refCount() const { return _d->refCount.get(); }
  //! @copydoc Doxygen::Implicit::isDetached().
  FOG_INLINE bool isDetached() const { return refCount() == 1; }
  //! @copydoc Doxygen::Implicit::detach().
  FOG_INLINE void detach() { if (!isDetached()) _detach(); }
  //! @copydoc Doxygen::Implicit::tryDetach().
  FOG_INLINE bool tryDetach() { return (!isDetached()) ? _tryDetach() : true; }
  //! @copydoc Doxygen::Implicit::_detach().
  void _detach();
  //! @copydoc Doxygen::Implicit::_tryDetach().
  bool _tryDetach();
  //! @copydoc Doxygen::Implicit::free().
  void free();

  void prepare();
  void clear();

  // [Flags]

  //! @copydoc Doxygen::Implicit::isNull().
  FOG_INLINE bool isNull() const { return _d == sharedNull.instancep(); }

  // [Dest / Source]
  
  //! @brief Returns destination pixel format.
  FOG_INLINE const Format& destFormat() const { return _d->dest; }
  //! @brief Returns source pixel format.
  FOG_INLINE const Format& sourceFormat() const { return _d->source; }

  // [Extra]

  //! @brief Returns extra data associated with pixel format.
  //!
  //! This function is probabbly too internal to use it in anything else
  //! than Fog library. But extra data can be sometimes useful.
  //!
  //! @sa Fog::Converter::extraUsed()

  FOG_INLINE const Extra* extra() const { return _d->extra.instancep(); }
  //! @brief Returns if pixel converter contains extra data.
  //!
  //! @sa Fog::Converter::extra()

  FOG_INLINE bool extraUsed() const { return (bool)_d->extraUsed; }

  // [Setup]

  //! @brief Creates pixel converter from @a source to @a dest pixel formats.
  bool setup(const Format& dest, const Format& source);

  //! @brief Creates pixel converter from @a source to @a dest pixel formats.
  //!
  //! @note @a source and @a dest are pixel format IDs, see @c Fog::Converter::IdEnum.
  bool setup(uint32_t dest, uint32_t source);

  // [Function]

  //! @brief Returns pixel converter function (low level stuff).
  FOG_INLINE SpanFunc spanFunc() const { return _d->spanFunc; }

  // [Convert]

  //! @brief Call convert function with given parameters @a params.
  //!
  //! Call to @c convert() is reentrant, so when pixel converter is
  //! initialized, it can be used from more threads.

  FOG_INLINE void convert(const Params& params) const
  {
    _d->spanFunc(_d, &params);
  }

  //! @overload.
  FOG_INLINE void convertSpan(
    uint8_t* destData, sysuint_t destX,
    const uint8_t* srcData, sysuint_t srcX,
    sysuint_t width) const
  {
    Params params;

    params.destData = destData;
    params.destX = destX;
    params.srcData = srcData;
    params.srcX = srcX;
    params.width = width;
    params.origin.clear();

    convert(params);
  }

  //! @overload.
  FOG_INLINE void convertSpan(
    uint8_t* destData, sysuint_t destX,
    const uint8_t* srcData, sysuint_t srcX,
    sysuint_t width,
    const Point& origin) const
  {
    Params params;

    params.destData = destData;
    params.destX = destX;
    params.srcData = srcData;
    params.srcX = srcX;
    params.width = width;
    params.origin = origin;

    convert(params);
  }

  FOG_INLINE void convertRect(
    uint8_t* destData, sysuint_t destX, sysint_t destStride,
    const uint8_t* srcData, sysuint_t srcX, sysint_t srcStride,
    sysuint_t width,
    sysuint_t height,
    const Point& origin) const
  {
    Params params;

    params.destData = destData;
    params.destX = destX;
    params.srcData = srcData;
    params.srcX = srcX;
    params.width = width;
    params.origin = origin;

    sysuint_t y;
    for (y = 0; y != height; y++,
      params.origin._y++,
      params.destData += destStride,
      params.srcData += srcStride)
    {
      convert(params);
    }
  }

  // [Operator Overload]

  //! @brief Creates equal converter to @a other.
  Converter& operator=(const Converter& other);
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_CONVERTER_H
