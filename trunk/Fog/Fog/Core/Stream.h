// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_STREAM_H
#define _FOG_CORE_STREAM_H

// [Dependencies]
#include <Fog/Core/ByteArray.h>
#include <Fog/Core/FileSystem.h>
#include <Fog/Core/String.h>

//! @addtogroup Fog_Core
//! @{

namespace Fog {

// ============================================================================
// [Fog::StreamDevice]
// ============================================================================

//! @brief Stream device is interface used inside the Fog::Stream.
//!
//! It contains abstract virtual methods that can be reimplemented to make a
//! new stream device that can be used in all functions that needs Fog::Stream.
struct FOG_API StreamDevice
{
  // [Construction / Destruction]

  StreamDevice();
  virtual ~StreamDevice();

  // [Implicit Sharing]

  virtual StreamDevice* ref() const;
  virtual void deref();

  FOG_INLINE StreamDevice* refAlways() const
  {
    refCount.inc();
    return const_cast<StreamDevice*>(this);
  }

  // [Abstract]

  virtual int64_t seek(int64_t offset, int whence) = 0;
  virtual int64_t tell() const = 0;
  virtual sysuint_t read(void* buffer, sysuint_t size) = 0;
  virtual sysuint_t write(const void* buffer, sysuint_t size) = 0;
  virtual err_t truncate(int64_t offset) = 0;
  virtual void close() = 0;

  virtual ByteArray getBuffer() const;

  // [Members]

  mutable Atomic<sysuint_t> refCount;
  uint32_t flags;
};

// ============================================================================
// [Fog::Stream]
// ============================================================================

//! @brief Stream is IO stream implementation used in by Fog library.
struct FOG_API Stream
{
  // [Data]

  static StreamDevice* sharedNull;

  // [Construction / Destruction]

  Stream();
  Stream(const Stream& stream);
  explicit Stream(StreamDevice* d);
  ~Stream();

  // [Implicit Sharing]

  FOG_INLINE sysuint_t refCount() const { return _d->refCount.get(); }

  // [Flags]

  enum Flag_Enum
  {
    IsNull     = (1 << 0),

    IsOpen     = (1 << 8),
    IsSeekable = (1 << 9),
    IsReadable = (1 << 10),
    IsWritable = (1 << 11),
    CanClose   = (1 << 12),

    IsFILE     = (1 << 16),
    IsHFILE    = (1 << 17),
    IsFD       = (1 << 18),
    IsMemory   = (1 << 19),
    IsGrowable = (1 << 20)
  };

  FOG_INLINE uint32_t flags() const { return _d->flags; }

  FOG_INLINE bool isNull()     const { return (_d->flags & IsNull    ) != 0; }
  FOG_INLINE bool isOpen()     const { return (_d->flags & IsOpen    ) != 0; }
  FOG_INLINE bool isSeekable() const { return (_d->flags & IsSeekable) != 0; }
  FOG_INLINE bool isReadable() const { return (_d->flags & IsReadable) != 0; }
  FOG_INLINE bool isWritable() const { return (_d->flags & IsWritable) != 0; }
  FOG_INLINE bool canClose()   const { return (_d->flags & CanClose  ) != 0; }

  FOG_INLINE bool isFILE()     const { return (_d->flags & IsFILE    ) != 0; }
  FOG_INLINE bool isHFILE()    const { return (_d->flags & IsHFILE   ) != 0; }
  FOG_INLINE bool isFD()       const { return (_d->flags & IsFD      ) != 0; }
  FOG_INLINE bool isMemory()   const { return (_d->flags & IsMemory  ) != 0; }
  FOG_INLINE bool isGrowable() const { return (_d->flags & IsGrowable) != 0; }

  // [Open]

  enum OpenEnum
  {
    OpenRead = (1 << 0),
    OpenWrite = (1 << 1),
    OpenReadWrite = OpenRead | OpenWrite,
    OpenTruncate = (1 << 2),
    OpenAppend = (1 << 3),
    OpenCreate = (1 << 4),
    OpenCreatePath = (1 << 5),
    OpenCreateOnly = (1 << 6)
  };

  err_t openFile(const String& fileName, uint32_t openFlags);
  err_t openFILE(FILE* fp, uint32_t openFlags, bool canClose);

#if defined(FOG_OS_WINDOWS)
  err_t openHANDLE(HANDLE hFile, uint32_t openFlags, bool canClose);
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
  err_t openFd(int fd, uint32_t openFlags, bool canClose);
#endif // FOG_OS_POSIX

  err_t openBuffer();
  err_t openBuffer(const ByteArray& buffer);
  err_t openBuffer(void* buffer, sysuint_t size, uint32_t openFlags);

  // [Seek]

  enum SeekEnum
  {
    SeekSet = 0,
    SeekCur = 1,
    SeekEnd = 2
  };

  int64_t seek(int64_t offset, int whence);
  int64_t tell() const;

  // [Read / Write]

  sysuint_t read(void* buffer, sysuint_t size);
  sysuint_t read(ByteArray& dst, sysuint_t size);
  sysuint_t readAll(ByteArray& dst, int64_t maxBytes = -1);

  sysuint_t write(const void* buffer, sysuint_t size);
  sysuint_t write(const ByteArray& data);

  // [Truncate]

  err_t truncate(int64_t offset);

  // [Close]

  void close();

  // [Stream Specific]

  //! @brief Get stream memory buffer. This method works only on memory streams.
  //!
  //! If stream was open by:
  //!   <core>openBuffer(void* buffer, sysuint_t size, uint32_t openFlags);</core>
  //! buffer will be created for it and data will be copied to this buffer.
  //!
  //! If stream was open by @c ByteArray instance, this method will return it.
  ByteArray getBuffer() const;

  // [Operator Overload]

  Stream& operator=(const Stream& other);

  // [Members]

  FOG_DECLARE_D(StreamDevice)
};

} // Fog namespace

//! @}

// ============================================================================
// [Fog::TypeInfo<>]
// ============================================================================

FOG_DECLARE_TYPEINFO(Fog::Stream, Fog::MoveableType)

// [Guard]
#endif // _FOG_CORE_STREAM_H
