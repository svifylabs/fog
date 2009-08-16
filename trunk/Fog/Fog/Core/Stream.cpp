// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/FileSystem.h>
#include <Fog/Core/FileUtil.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/Stream.h>
#include <Fog/Core/String.h>
#include <Fog/Core/TextCodec.h>

#include <stdio.h>
#include <stdlib.h>

#include <new>

#if defined(FOG_OS_POSIX)
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#endif // FOG_OS_POSIX

#if !defined(FOG_OS_WINDOWS)
# define _fseeki64 fseeko
# define _ftelli64 ftello
#elif defined(__MINGW32__)
# warning "MinGW or Borland portability issue: large files support for FILE* stream is turned off"
# define _fseeki64 fseek
# define _ftelli64 ftell
#elif defined(FOG_CC_BORLAND)
# define _fseeki64 fseek
# define _ftelli64 ftell
#endif

namespace Fog {

// ============================================================================
// [Fog::StreamDevice]
// ============================================================================

StreamDevice::StreamDevice()
{
  refCount.init(1);
  flags = 0;
}

StreamDevice::~StreamDevice()
{
}

StreamDevice* StreamDevice::ref() const
{
  refCount.inc();
  return const_cast<StreamDevice*>(this);
}

void StreamDevice::deref()
{
  if (refCount.deref()) delete this;
}

// ============================================================================
// [Fog::StreamDeviceNull]
// ============================================================================

struct FOG_HIDDEN StreamDeviceNull : public StreamDevice
{
  StreamDeviceNull();

  virtual int64_t seek(int64_t offset, int whence);
  virtual int64_t tell() const;
  virtual sysuint_t read(void* buffer, sysuint_t size);
  virtual sysuint_t write(const void* buffer, sysuint_t size);
  virtual err_t truncate(int64_t offset);
  virtual void close();
};

StreamDeviceNull::StreamDeviceNull()
{
  flags = Stream::IsNull     |
          Stream::IsWritable ;
}

int64_t StreamDeviceNull::seek(int64_t offset, int whence)
{
  return -1;
}

int64_t StreamDeviceNull::tell() const
{
  return -1;
}

sysuint_t StreamDeviceNull::read(void* buffer, sysuint_t size)
{
  return 0;
}

sysuint_t StreamDeviceNull::write(const void* buffer, sysuint_t size)
{
  return 0;
}

err_t StreamDeviceNull::truncate(int64_t offset)
{
  return Error::TruncateNotSupported;
}

void StreamDeviceNull::close()
{
}

static Static<StreamDeviceNull> streamdevice_null;

// ============================================================================
// [Fog::StreamDeviceFILE]
// ============================================================================

struct FOG_HIDDEN StreamDeviceFILE : public StreamDevice
{
  StreamDeviceFILE(FILE* fp, uint32_t fflags);

  virtual int64_t seek(int64_t offset, int whence);
  virtual int64_t tell() const;
  virtual sysuint_t read(void* buffer, sysuint_t size);
  virtual sysuint_t write(const void* buffer, sysuint_t size);
  virtual err_t truncate(int64_t offset);
  virtual void close();

  FILE* fp;
};

StreamDeviceFILE::StreamDeviceFILE(FILE* fp, uint32_t fflags) :
  fp(fp)
{
  flags |= fflags;
}

int64_t StreamDeviceFILE::seek(int64_t offset, int whence)
{
  if (_fseeki64(fp, offset, whence) != 0)
    return -1;
  else
    return _ftelli64(fp);
}

int64_t StreamDeviceFILE::tell() const
{
  return _ftelli64(fp);
}

sysuint_t StreamDeviceFILE::read(void* buffer, sysuint_t size)
{
  return fread(buffer, 1, size, fp);
}

sysuint_t StreamDeviceFILE::write(const void* buffer, sysuint_t size)
{
  return fwrite(buffer, 1, size, fp);
}

err_t StreamDeviceFILE::truncate(int64_t offset)
{
  return Error::TruncateNotSupported;
}

void StreamDeviceFILE::close()
{
  if ((flags & Stream::CanClose) != 0) fclose(fp);
}

// ============================================================================
// [Fog::StreamDeviceHANDLE]
// ============================================================================

#if defined(FOG_OS_WINDOWS)
struct FOG_HIDDEN StreamDeviceHANDLE : public StreamDevice
{
  StreamDeviceHANDLE(HANDLE hFile, uint32_t fflags);
  ~StreamDeviceHANDLE();

  static err_t openFile(const String32& fileName, uint32_t openFlags, StreamDevice** dst);

  virtual int64_t seek(int64_t offset, int whence);
  virtual int64_t tell() const;
  virtual sysuint_t read(void* buffer, sysuint_t size);
  virtual sysuint_t write(const void* buffer, sysuint_t size);
  virtual err_t truncate(int64_t offset);
  virtual void close();

  HANDLE hFile;
};

StreamDeviceHANDLE::StreamDeviceHANDLE(HANDLE hFile, uint32_t fflags) :
  hFile(hFile)
{
  flags |= fflags | Stream::IsHFILE;
}

StreamDeviceHANDLE::~StreamDeviceHANDLE()
{
}

err_t StreamDeviceHANDLE::openFile(const String32& fileName, uint32_t openFlags, StreamDevice** dst)
{
  FOG_ASSERT((openFlags & (Stream::OpenRead | Stream::OpenWrite)) != 0);

  HANDLE hFile;

  // Convert path to local file system string
  err_t err;
  TemporaryString16<TemporaryLength> fileNameW;

  if ((err = fileNameW.set(fileName)) ||
      (err = fileNameW.slashesToWin()))
  {
    return err;
  }

  DWORD dwDesiredAccess = 0;
  DWORD dwCreationDisposition = 0;
  DWORD dwFlags = 0;

  uint32_t createPathFlags = 
    Stream::OpenCreate     | 
    Stream::OpenCreatePath | 
    Stream::OpenWrite      ;

  uint32_t fflags = Stream::IsOpen | Stream::IsSeekable;
  if (openFlags & Stream::OpenRead ) fflags |= Stream::IsReadable;
  if (openFlags & Stream::OpenWrite) fflags |= Stream::IsWritable;

  if (openFlags & Stream::OpenRead ) dwDesiredAccess |= GENERIC_READ;
  if (openFlags & Stream::OpenWrite) dwDesiredAccess |= GENERIC_WRITE;

  if ((openFlags & (Stream::OpenWrite | Stream::OpenTruncate)) ==
                   (Stream::OpenWrite | Stream::OpenTruncate))
  {
    dwCreationDisposition = TRUNCATE_EXISTING;
  }
  else if (openFlags & Stream::OpenCreate)
  {
    if (openFlags & Stream::OpenCreateOnly)
      // Only create file, fail if file exist
      dwCreationDisposition = CREATE_NEW;
    else
      // Create or open
      dwCreationDisposition = OPEN_ALWAYS;
  }
  else
  {
    dwCreationDisposition = OPEN_EXISTING;
  }

  hFile = CreateFileW(
    fileNameW.cStrW(),         // file name
    dwDesiredAccess,           // open mode (read / write)
    FILE_SHARE_READ | FILE_SHARE_WRITE, // share mode
    NULL,                      // security
    dwCreationDisposition,     // creation disposition
    dwFlags,                   // attributes (defaults)
    NULL);                     // no template

  if (hFile == INVALID_HANDLE_VALUE)
  {
    DWORD dwError = GetLastError();

    if ((openFlags & Stream::OpenCreate) &&
      dwCreationDisposition == TRUNCATE_EXISTING &&
      dwError == ERROR_FILE_NOT_FOUND)
    {
      hFile = CreateFileW(
        fileNameW.cStrW(),         // file name
        dwDesiredAccess,           // open mode (read / write)
        FILE_SHARE_READ | FILE_SHARE_WRITE, // share mode
        NULL,                      // security
        CREATE_NEW,                // creation disposition
        dwFlags,                   // attributes (defaults)
        NULL);                     // no template

      if (hFile == INVALID_HANDLE_VALUE)
      {
        return GetLastError();
      }
      else
      {
        *dst = new StreamDeviceHANDLE(hFile, fflags);
        return Error::Ok;
      }
    }
    else
    {
      return (err_t)dwError;
    }
  }
  else
  {
    // Sucess, but there is 'Stream::OpenAppend' possibility.
    if ((openFlags & (Stream::OpenWrite | Stream::OpenAppend)) ==
                     (Stream::OpenWrite | Stream::OpenAppend))
    {
      SetFilePointer(hFile, 0, 0, FILE_END);
    }

    *dst = new StreamDeviceHANDLE(hFile, fflags);
    return Error::Ok;
  }
}

int64_t StreamDeviceHANDLE::seek(int64_t offset, int whence)
{
  UInt64Union i;
  DWORD dwError;

  i.u64 = offset;
  i.u32low = SetFilePointer(hFile, i.u32low, (PLONG)&i.u32high, whence);

  if (i.u32low == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != ERROR_SUCCESS)
  {
    return -1;
  }
  else
  {
    return (int64_t)i.u64;
  }
}

int64_t StreamDeviceHANDLE::tell() const
{
  UInt64Union i;
  DWORD dwError;

  i.u32high = 0;
  i.u32low = SetFilePointer(hFile, 0, (PLONG)&i.u32high, FILE_CURRENT);

  if (i.u32low == INVALID_SET_FILE_POINTER && (dwError = GetLastError()) != ERROR_SUCCESS)
  {
    return -1;
  }
  else
  {
    return (int64_t)i.u64;
  }
}

sysuint_t StreamDeviceHANDLE::read(void* buffer, sysuint_t size)
{
  // TODO: 64 bit?
  DWORD readedBytes;

  if (ReadFile(hFile, buffer, size, &readedBytes, NULL) == 0)
    return (sysuint_t)-1;
  else
    return readedBytes;
}

sysuint_t StreamDeviceHANDLE::write(const void* buffer, sysuint_t size)
{
  DWORD writtenBytes;

  if (WriteFile(hFile, buffer, size, &writtenBytes, NULL) == 0)
    return (sysuint_t)-1;
  else
    return writtenBytes;
}

err_t StreamDeviceHANDLE::truncate(int64_t offset)
{
  if (seek(offset, Stream::SeekSet) != -1)
  {
    if (!SetEndOfFile(hFile)) return Error::TruncateFailed;
    return Error::Ok;
  }
  else
  {
    return Error::TruncateFailed;
  }
}

void StreamDeviceHANDLE::close()
{
  if ((flags & Stream::CanClose) != 0) CloseHandle(hFile);
}
#endif // FOG_OS_WINDOWS

// ============================================================================
// [Fog::StreamDeviceFd]
// ============================================================================

#if defined FOG_OS_POSIX
struct FOG_HIDDEN StreamDeviceFd : public StreamDevice
{
  StreamDeviceFd(int fd, uint32_t fflags);
  ~StreamDeviceFd();

  static err_t openFile(const String32& fileName, uint32_t openFlags, StreamDevice** dst);

  virtual int64_t seek(int64_t offset, int whence);
  virtual int64_t tell() const;
  virtual sysuint_t read(void* buffer, sysuint_t size);
  virtual sysuint_t write(const void* buffer, sysuint_t size);
  virtual err_t truncate(int64_t offset);
  virtual void close();

  int fd;
};

StreamDeviceFd::StreamDeviceFd(int fd, uint32_t fflags) :
  fd(fd)
{
  flags |= Stream::IsFD | Stream::IsOpen | fflags;
}

StreamDeviceFd::~StreamDeviceFd()
{
  close();
}

err_t StreamDeviceFd::openFile(const String32& fileName, uint32_t openFlags, StreamDevice** dst)
{
  FOG_ASSERT((openFlags & (Stream::OpenRead | Stream::OpenWrite)) != 0);

  int fd;
  int fdFlags = 0; // be quite

  uint32_t fflags = Stream::IsOpen | Stream::IsSeekable;
  if (openFlags & Stream::OpenRead ) fflags |= Stream::IsReadable;
  if (openFlags & Stream::OpenWrite) fflags |= Stream::IsWritable;

  // Convert path to local file system string
  err_t err;
  TemporaryString8<TemporaryLength> fileName8;

  if ((err = fileName8.set(fileName, TextCodec::local8()))) return err;

  // Read / Write
  if ((openFlags & Stream::OpenReadWrite) == Stream::OpenReadWrite)
    fdFlags = O_RDWR;
  else if ((openFlags & Stream::OpenRead) != 0)
    fdFlags = O_RDONLY;
  else if ((openFlags & Stream::OpenWrite) != 0)
    fdFlags = O_WRONLY;

  // Truncate / Append
  if (openFlags & Stream::OpenWrite)
  {
    if (openFlags & Stream::OpenTruncate) fdFlags |= O_TRUNC;
    if (openFlags & Stream::OpenAppend  ) fdFlags |= O_APPEND;
  }

  // Only create file, fail if file exist
  if (openFlags & (Stream::OpenCreate | Stream::OpenCreateOnly) ==
                  (Stream::OpenCreate | Stream::OpenCreateOnly))
  {
    fdFlags |= O_EXCL;
  }

  // Open file
  fd = ::open64(fileName8.cStr(), fdFlags | O_LARGEFILE);

  // Try to create file if open failed (or create it if OpenCreate flag was set)
  if (fd < 0 && (errno == ENOENT) && (openFlags & Stream::OpenCreate) != 0)
  {
    fd = ::open64(fileName8.cStr(), fdFlags | O_CREAT | O_LARGEFILE, 0644);
  }

  if (fd < 0)
  {
    // Error (Invalid file descriptor)
    return errno;
  }
  else
  {
    // Success
    *dst = new StreamDeviceFd(fd, fflags);
    return Error::Ok;
  }
}

int64_t StreamDeviceFd::seek(int64_t offset, int whence)
{
  int64_t result = ::lseek64(fd, offset, whence);

  if (result < 0)
    return -1;
  else
    return result;
}

int64_t StreamDeviceFd::tell() const
{
  int64_t result = ::lseek64(fd, 0, SEEK_SET);

  if (result < 0)
    return -1;
  else
    return result;
}

sysuint_t StreamDeviceFd::read(void* buffer, sysuint_t size)
{
  sysint_t n = ::read(fd, buffer, size);

  if (n < 0)
    return (sysuint_t)-1;
  else
    return (sysuint_t)n;
}

sysuint_t StreamDeviceFd::write(const void* buffer, sysuint_t size)
{
  sysint_t n = ::write(fd, buffer, size);

  if (n < 0)
    return (sysuint_t)-1;
  else
    return (sysuint_t)n;
}

err_t StreamDeviceFd::truncate(int64_t offset)
{
  int result = ::ftruncate64(fd, offset);
  return result != 0 ? (err_t)Error::TruncateFailed : (err_t)Error::Ok;
}

void StreamDeviceFd::close()
{
  if ((flags & Stream::CanClose) != 0) ::close(fd);
}
#endif // FOR_OS_POSIX

// ============================================================================
// [Fog::StreamDeviceMemory]
// ============================================================================

struct FOG_HIDDEN StreamDeviceMemory : public StreamDevice
{
  StreamDeviceMemory(const void* memory, sysuint_t size, uint32_t fflags);

  virtual int64_t seek(int64_t offset, int whence);
  virtual int64_t tell() const;
  virtual sysuint_t read(void* buffer, sysuint_t size);
  virtual sysuint_t write(const void* buffer, sysuint_t size);
  virtual err_t truncate(int64_t offset);
  virtual void close();

  uint8_t* data;
  sysuint_t size;

  uint8_t* cur;
  uint8_t* end;
};

StreamDeviceMemory::StreamDeviceMemory(
  const void* memory, sysuint_t size, uint32_t fflags) :
    data((uint8_t*)memory), size(size)
{
  cur = data;
  end = data + size;

  flags |= 
    Stream::IsOpen     |
    Stream::IsSeekable |
    Stream::IsFixed    |
    Stream::IsMemory   |
    fflags;
}

int64_t StreamDeviceMemory::seek(int64_t offset, int whence)
{
  int64_t i;

  switch (whence)
  {
    case Stream::SeekSet:
      i = offset;
      break;
    case Stream::SeekCur:
      i = (int64_t)(cur - data) + offset;
      break;
    case Stream::SeekEnd:
      i = (int64_t)(end - data) + offset;
      break;
  }

  if (i < 0)
  {
    cur = data;
    return -1;
  }
  else if (i > (int64_t)size)
  {
    cur = end;
    return -1;
  }
  else
  {
    cur = data + (sysuint_t)i;
    return i;
  }
}

int64_t StreamDeviceMemory::tell() const
{
  return (int64_t)(cur - data);
}

sysuint_t StreamDeviceMemory::read(void* buffer, sysuint_t size)
{
  sysuint_t remain = (sysuint_t)(end - cur);
  if (size > remain) size = remain;

  Memory::copy(buffer, cur, size);
  cur += size;

  return size;
}

sysuint_t StreamDeviceMemory::write(const void* buffer, sysuint_t size)
{
  sysuint_t remain = (sysuint_t)(end - cur);
  if (size > remain) size = remain;

  Memory::copy(cur, buffer, size);
  cur += size;

  return size;
}

err_t StreamDeviceMemory::truncate(int64_t offset)
{
  return Error::TruncateNotSupported;
}

void StreamDeviceMemory::close()
{
}

// ============================================================================
// [Fog::Stream]
// ============================================================================

StreamDevice* Stream::sharedNull;

Stream::Stream() :
  _d(sharedNull->refAlways())
{
}

Stream::Stream(const Stream& other) :
  _d(other._d->ref())
{
  if (!_d) _d = sharedNull->refAlways();
}

Stream::Stream(StreamDevice* d) :
  _d(d)
{
}

Stream::~Stream()
{
  _d->deref();
}

err_t Stream::openFile(const String32& fileName, uint32_t openFlags)
{
  static uint32_t createPathFlags = OpenCreate | OpenCreatePath | OpenWrite;

  // If we are opening new file, we will close previous. This means that if 
  // open fail, we end at neutral state (null stream).
  close();

  err_t err;

  // Validate openFlags.
  if ((openFlags & OpenReadWrite) == 0) return Error::InvalidArgument;

  // Create path if asked for.
  if ((openFlags & createPathFlags) == createPathFlags)
  {
    TemporaryString32<TemporaryLength> dirName;
    if ((err = FileUtil::extractDirectory(dirName, fileName))) return err;

    if (!dirName.isEmpty() && dirName != Ascii8("."))
    {
      if ((err = FileSystem::createDirectory(dirName, true))) return err;
    }
  }

  // Try to open file using OS specific calls.
  StreamDevice* newd;

#if defined(FOG_OS_WINDOWS)
  err = StreamDeviceHANDLE::openFile(fileName, openFlags, &newd);
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
  err = StreamDeviceFd::openFile(fileName, openFlags, &newd);
#endif // FOG_OS_POSIX

  if (err) return err;

  AtomicBase::ptr_setXchg(&_d, newd)->deref();
  return Error::Ok;
}

err_t Stream::openFILE(FILE* fp, uint32_t openFlags, bool canClose)
{
  close();
  if (fp == NULL) return Error::InvalidArgument;

  uint32_t fflags = IsSeekable;
  if (openFlags & OpenRead ) fflags |= IsReadable;
  if (openFlags & OpenWrite) fflags |= IsWritable;
  if (canClose) fflags |= CanClose;

  StreamDevice* newd = new(std::nothrow) StreamDeviceFILE(fp, fflags);
  if (!newd) return Error::OutOfMemory;

  AtomicBase::ptr_setXchg(&_d, newd)->deref();
  return Error::Ok;
}

#if defined(FOG_OS_WINDOWS)
err_t Stream::openHANDLE(HANDLE hFile, uint32_t openFlags, bool canClose)
{
  close();

  if (hFile == NULL || hFile == INVALID_HANDLE_VALUE)
  {
    return Error::InvalidArgument;
  }

  uint32_t fflags = IsSeekable;
  if (openFlags & OpenRead ) fflags |= IsReadable;
  if (openFlags & OpenWrite) fflags |= IsWritable;
  if (canClose) fflags |= CanClose;

  StreamDevice* newd = new(std::nothrow) StreamDeviceHANDLE(hFile, fflags);
  if (!newd) return Error::OutOfMemory;

  AtomicBase::ptr_setXchg(&_d, newd)->deref();
  return Error::Ok;
}
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
err_t Stream::openFd(int fd, uint32_t openFlags, bool canClose)
{
  close();
  if (fd < 0) return Error::InvalidArgument;

  uint32_t fflags = IsSeekable;
  if (openFlags & OpenRead ) fflags |= IsReadable;
  if (openFlags & OpenWrite) fflags |= IsWritable;
  if (canClose) fflags |= CanClose;

  StreamDevice* newd = new(std::nothrow) StreamDeviceFd(fd, fflags);
  if (!newd) return Error::OutOfMemory;

  AtomicBase::ptr_setXchg(&_d, newd)->deref();
  return Error::Ok;
}
#endif // FOG_OS_POSIX

err_t Stream::openMemory(void* memory, sysuint_t size, uint32_t openFlags)
{
  close();
  if (memory == NULL) return Error::InvalidArgument;

  uint32_t fflags = 0;

  if (openFlags & OpenRead ) fflags |= IsReadable;
  if (openFlags & OpenWrite) fflags |= IsWritable;

  StreamDevice* newd = new(std::nothrow) StreamDeviceMemory(memory, size, fflags);
  if (!newd) return Error::OutOfMemory;

  AtomicBase::ptr_setXchg(&_d, newd)->deref();
  return Error::Ok;
}

int64_t Stream::seek(int64_t offset, int whence)
{
  return _d->seek(offset, whence);
}

int64_t Stream::tell() const
{
  return _d->tell();
}

sysuint_t Stream::read(void* buffer, sysuint_t size)
{
  return _d->read(buffer, size);
}

sysuint_t Stream::read(String8& dst, sysuint_t size)
{
  err_t err = dst.reserve(size);
  if (err) return 0;

  sysuint_t n = _d->read((void*)dst.mStr(), size);
  dst.resize(n);
  return n;
}

sysuint_t Stream::readAll(String8& dst, int64_t maxBytes)
{
  dst.clear();

  int64_t curPosition = tell();
  int64_t endPosition = seek(0, SeekEnd);

  if (curPosition == 0 && endPosition == 0)
  {
    // This happen for example in /proc/ or in virtual files. We will try to
    // read everything we can.
    uint64_t remain = (maxBytes < 0) ? UINT64_MAX : (uint64_t)maxBytes;

    for (;;)
    {
      sysuint_t count = 4096;
      sysuint_t done;

      if ((uint64_t)count > remain) count = (sysuint_t)remain;

      sysuint_t len = dst.getLength();
      err_t err = dst.reserve(len + (sysuint_t)count);
      if (err) break;

      done = read(dst.mData(), count);
      dst.resize(len + done);

      if (done != count) break;
    }

    return dst.getLength();
  }
  else
  {
    if (curPosition == -1 || endPosition == -1) return 0;
    int64_t remain = endPosition - curPosition;

    if (maxBytes > 0 && remain > maxBytes)
      remain = maxBytes;

    if (seek(curPosition, SeekSet) == -1) return 0;

#if FOG_ARCH_BITS > 32
    if (remain > (int64_t)(SYSINT_MAX)) return 0;
#endif // FOG_ARCH_BITS > 32
    if (dst.reserve((sysuint_t)remain) != Error::Ok) return 0;

    sysuint_t n = read(dst.mData(), (sysuint_t)remain);
    dst.resize(n);
    return n;
  }
}

sysuint_t Stream::write(const void* buffer, sysuint_t size)
{
  return _d->write(buffer, size);
}

sysuint_t Stream::write(const String8& data)
{
  return _d->write((const void*)data.cData(), data.getLength());
}

err_t Stream::truncate(int64_t offset)
{
  return _d->truncate(offset);
}

void Stream::close()
{
  AtomicBase::ptr_setXchg(&_d, sharedNull->refAlways())->deref();
}

Stream& Stream::operator=(const Stream& other)
{
  AtomicBase::ptr_setXchg(&_d, other._d->ref())->deref();
  return *this;
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_stream_init(void)
{
  Fog::streamdevice_null.init();
  Fog::Stream::sharedNull = Fog::streamdevice_null.instancep();

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_stream_shutdown(void)
{
  Fog::streamdevice_null.destroy();
}
