// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_IMAGEIO_H
#define _FOG_GRAPHICS_IMAGEIO_H

// [Dependencies]
#include <Fog/Core/Properties.h>
#include <Fog/Core/Stream.h>
#include <Fog/Core/String.h>
#include <Fog/Core/Value.h>
#include <Fog/Core/Vector.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/Palette.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {
namespace ImageIO {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct BaseDevice;
struct DecoderDevice;
struct EncoderDevice;
struct Provider;

// ============================================================================
// [Functions]
// ============================================================================

FOG_API bool addProvider(Provider* provider);
FOG_API bool removeProvider(Provider* provider);
FOG_API bool hasProvider(Provider* provider);

FOG_API Vector<Provider*> getProviders();

FOG_API Provider* getProviderByName(const String32& name);
FOG_API Provider* getProviderByExtension(const String32& extension);
FOG_API Provider* getProviderByMemory(void* mem, sysuint_t len);

// ============================================================================
// [Fog::ImageIO::Provider]
// ============================================================================

//! @brief Image file encoder/decoder provider singleton.
//!
//! Providers are used to create image decoder and encoder devices and
//! to check if image data can be readed by decoder.
//!
//! Befire image decoder is created, often is checked relevance by
//! mime type data (first image file bytes)
struct FOG_API Provider
{
  // [Construction / Destruction]

  //! @brief Image file format Constructor.
  Provider();
  //! @brief Image file format descructor.
  virtual ~Provider();

  // [Features]

  //! @brief File format features.
  struct Features
  {
    // [Devices]

    //! @brief Provider provides decoder.
    uint32_t decoder : 1;
    //! @brief Provider provides encoder.
    uint32_t encoder : 1;

    // [Pixel format]

    //! @brief Image file format supports 1 bit mono images.
    uint32_t mono : 1;
    //! @brief Image file format supports 1 bit palette based images.
    uint32_t pal1 : 1;
    //! @brief Image file format supports 4 bit palette based images.
    uint32_t pal4 : 1;
    //! @brief Image file format supports 8 bit palette based images.
    uint32_t pal8 : 1;
    //! @brief Image file format supports 15 bit RGB images.
    uint32_t rgb15 : 1;
    //! @brief Image file format supports 16 bit RGB images.
    uint32_t rgb16 : 1;
    //! @brief Image file format supports 24 bit RGB images.
    uint32_t rgb24 : 1;
    //! @brief Image file format supports 48 bit RGB images.
    uint32_t rgb48 : 1;
    //! @brief Image file format supports 96 bit floating point RGB images.
    uint32_t rgbf : 1;
    //! @brief Image file format supports 32 bit ARGB images.
    uint32_t argb32 : 1;
    //! @brief Image file format supports 64 bit ARGB images.
    uint32_t argb64 : 1;
    //! @brief Image file format supports 128 bit floating point ARGB images.
    uint32_t argbf : 1;

    // [Size]

    //! @brief @c true if size of image or animation is limited or predefined (for example FLI).
    uint32_t limitedSize : 1;

    // [Animations]

    //! @brief Image file format supports animations.
    uint32_t animations : 1;

    // [Opacity]

    //! @brief Image file format supports opacity in rgb images.
    uint32_t rgbAlpha : 1;

    //! @brief Image file format supports opacity in palette based images.
    uint32_t palAlpha : 1;

    //! @brief Image file format supports key alpha (index in palette)
    uint32_t keyAlpha : 1;

    // [Compression / Quality]

    //! @brief Image file format uses RLE compression for 1 bit depth.
    uint32_t rle1 : 1;
    //! @brief Image file format uses RLE compression for 4 bit depth.
    uint32_t rle4 : 1;
    //! @brief Image file format uses RLE compression for 8 bit depth.
    uint32_t rle8 : 1;
    //! @brief Image file format uses RLE compression for 15/16 bit depth.
    uint32_t rle16 : 1;
    //! @brief Image file format uses RLE compression for 24 bit depth.
    uint32_t rle24 : 1;
    //! @brief Image file format uses RLE compression for 32 bit depth.
    uint32_t rle32 : 1;

    //! @brief Image file format supports progressive rendering.
    uint32_t progressive : 1;

    //! @brief Image file format supports compression type adjustment.
    uint32_t compressionTypeAdjust : 1;

    //! @brief Image file format supports quality adjustment.
    uint32_t qualityAdjust : 1;
  };

  // [Members access]

  //! @brief Returns image file format name.
  FOG_INLINE const String32& name() const { return _name; }
  //! @brief Returns image file format extensions.
  FOG_INLINE const Vector<String32>& extensions() const { return _extensions; }
  //! @brief Returns image file format features.
  FOG_INLINE const Features& features() const { return _features; }

  // [Virtuals]

  //! @brief Check mime type for this format and return it's relevance. 
  //!
  //! @note Relevance is number between 0 to 100 in percents. Image
  //! loader will use relevance for decoders.
  virtual uint32_t check(const void* mem, sysuint_t length) = 0;
  //! @overload
  FOG_INLINE uint32_t check(const String8& mem) { return check(mem.cData(), mem.getLength()); }

  virtual EncoderDevice* createEncoder();
  virtual DecoderDevice* createDecoder();

  // [Members]

protected:
  //! @brief Image file format name ("BMP", "JPEG", "PNG", ...) .
  String32 _name;
  //! @brief Image file format id.
  uint32_t _id;
  //! @brief Image file format extensions ("bmp", "jpg", "jpeg", ...).
  Vector<String32> _extensions;

  //! @brief Image file format features.
  Features _features;

private:
  FOG_DISABLE_COPY(Provider)
};

// ============================================================================
// [Fog::ImageIO::BaseDevice]
// ============================================================================

struct FOG_API BaseDevice : public PropertiesContainer
{
  typedef PropertiesContainer base;

  // [Construction / Descruction]

  BaseDevice();
  virtual ~BaseDevice();

  // [Properties]

  FOG_DECLARE_PROPERTIES_CONTAINER()

  enum PropertyId
  {
    PropertyWidth,
    PropertyHeight,
    PropertyDepth,
    PropertyPlanes,
    PropertyActualFrame,
    PropertyFramesCount,
    PropertyProgress,

    PropertyLast
  };

  virtual int propertyInfo(int id) const;
  virtual err_t getProperty(int id, Value& value) const;
  virtual err_t setProperty(int id, const Value& value);

  // [Device Type]

  enum DeviceType
  {
    //! @brief None, null codec or non initialized.
    None = 0x0,
    //! @brief Image IO Encoder.
    EncoderType = 0x1,
    //! @brief Image IO Decoder.
    DecoderType = 0x2,
    //! @brief Proxy for another image processing library.
    ProxyType = 0x4
  };

  // [Members access]

  FOG_INLINE Provider* provider() const { return _provider; }
  FOG_INLINE uint32_t deviceType() const { return _deviceType; }
  FOG_INLINE uint32_t flags() const { return _flags; }
  FOG_INLINE bool isNone() const { return deviceType() == None; }
  FOG_INLINE bool isEncoder() const { return (deviceType() & EncoderType) != 0; }
  FOG_INLINE bool isDecoder() const { return (deviceType() & DecoderType) != 0; }
  FOG_INLINE bool isProxy() const { return (deviceType() & ProxyType) != 0; }

  FOG_INLINE uint64_t attachedOffset() const { return _attachedOffset; }
  FOG_INLINE Stream& stream() { return _stream; }
  FOG_INLINE const Stream& stream() const { return _stream; }

  FOG_INLINE uint32_t width() const { return _width; }
  FOG_INLINE uint32_t height() const { return _height; }
  FOG_INLINE uint32_t depth() const { return _depth; }
  FOG_INLINE uint32_t planes() const { return _planes; }

  FOG_INLINE uint32_t actualFrame() const { return _actualFrame; }
  FOG_INLINE uint32_t framesCount() const { return _framesCount; }

  FOG_INLINE int format() const { return _format; }
  FOG_INLINE const Palette& palette() const { return _palette; }
  FOG_INLINE const String8& comment() const { return _comment; }

  // [Progress]

  FOG_INLINE float progress() const { return _progress; }

  //! @brief Update progress to @a value.
  void updateProgress(float value);
  //! @overload
  void updateProgress(uint32_t y, uint32_t height);

  // [Dimensions checking]

  bool areDimensionsZero() const;
  bool areDimensionsTooLarge() const;

  // [Stream]

  virtual void attachStream(Stream& stream);
  virtual void detachStream();

  // [Protected virtuals]
protected:
  virtual void reset();

  // [Members]

  //! @brief Device provider.
  Provider* _provider;
  //! @brief Device type.
  uint32_t _deviceType;
  //! @brief Flags.
  uint32_t _flags;

  //! @brief Attached stream offset.
  uint64_t _attachedOffset;
  //! @brief Attached stream.
  Stream _stream;

  //! @brief Image or animation width.
  uint32_t _width;
  //! @brief Image or animation height.
  uint32_t _height;
  //! @brief Image or animation depth (bits per pixel).
  uint32_t _depth;
  //! @brief Image or animation planes.
  uint32_t _planes;

  //! @brief Actual frame.
  uint32_t _actualFrame;
  //! @brief Count of frames.
  uint32_t _framesCount;

  //! @brief Image format of target image.
  int _format;
  //! @brief Palette if reading / writing 8 bit or less images.
  //!
  //! This is image palette that can be contained in loaded image,
  //! but it's not needed if image is directly loaded to different
  //! image format and decoder supports this.
  Palette _palette;
  //! @brief Comment
  String8 _comment;

  //! @brief Progress, 0 to 100 [percent]
  float _progress;
};

// ============================================================================
// [Fog::ImageIO::DecoderDevice]
// ============================================================================

//! @brief Image IO decoder device.
struct FOG_API DecoderDevice : public BaseDevice
{
  // [Construction / Descruction]

  DecoderDevice();
  virtual ~DecoderDevice();

  // [Members access]

  FOG_INLINE bool headerDone() const { return _headerDone; }
  FOG_INLINE bool readerDone() const { return _readerDone; }
  FOG_INLINE uint32_t headerResult() const { return _headerResult; }
  FOG_INLINE uint32_t readerResult() const { return _readerResult; }

  // [Virtuals]

  virtual uint32_t readHeader() = 0;
  virtual uint32_t readImage(Image& image) = 0;

  // [Protected virtuals]
protected:
  virtual void reset();

  // [Members]

  //! @brief @c true if header was read.
  uint32_t _headerDone : 1;
  //! @brief @c true if image was read.
  uint32_t _readerDone : 1;
  //! @brief Header decoder result code (returned by @c readHeader()).
  uint32_t _headerResult;
  //! @brief Image decoder result code (returned by @c readImage()).
  uint32_t _readerResult;
};

// ============================================================================
// [Fog::ImageIO::EncoderDevice]
// ============================================================================

//! @brief Image IO encoder device.
struct FOG_API EncoderDevice : public BaseDevice
{
  // [Construction / Descruction]

  EncoderDevice();
  virtual ~EncoderDevice();

  // [Members access]

  FOG_INLINE bool headerDone() const { return _headerDone; }
  FOG_INLINE bool writerDone() const { return _writerDone; }

  FOG_INLINE void setComment(const String8& comment) { _comment = comment; }

  // [Virtuals]

  virtual void detachStream();
  virtual uint32_t writeImage(const Image& image) = 0;

protected:
  virtual void reset();
  virtual void finalize();

  // [Members]

  uint32_t _headerDone : 1;
  uint32_t _writerDone : 1;
};

} // ImageIO namespace
} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_IMAGEIO_H
