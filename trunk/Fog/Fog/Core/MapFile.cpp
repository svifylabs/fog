// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Error.h>
#include <Fog/Core/FileSystem.h>
#include <Fog/Core/MapFile.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/String.h>

#if defined(FOG_OS_POSIX)
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace Fog {

// ============================================================================
// [Fog::MapFile]
// ============================================================================

MapFile::MapFile() :
  _data(NULL),
  _size(0),
#if defined(FOG_OS_WINDOWS)
  _hFileMapping(NULL),
  _hFile(NULL),
#endif // FOG_OS_WINDOWS
#if defined(FOG_OS_POSIX)
  _fd(-1),
#endif // FOG_OS_POSIX
  _state(None)
{
}

MapFile::~MapFile()
{
  unmap();
}

#if defined(FOG_OS_WINDOWS)
err_t MapFile::map(const String32& fileName, bool loadOnFail)
{
  unmap();

  err_t err;

  TemporaryString16<TemporaryLength> fileNameW;
  if ((err = fileNameW.set(fileName)) ||
      (err = fileNameW.slashesToPosix()))
  {
    return err;
  }

  HANDLE hFile;
  HANDLE hFileMapping;
  void* data;
  sysuint_t size;

  DWORD szLow = 0;
  DWORD szHigh = 0;

  // Try to open file.
  if ((hFile = CreateFileW(
    fileNameW.cStrW(), FILE_READ_DATA, FILE_SHARE_READ,
    NULL, OPEN_EXISTING, 0, NULL)) == INVALID_HANDLE_VALUE)
  {
    return GetLastError();
  }

  // Get size of file (if size is too large -> fail)
  szLow = GetFileSize(hFile, &szHigh);
#if FOG_ARCH_BITS == 32
  if (szHigh) 
  {
    CloseHandle(hFile);
    return Error::FileSizeTooBig;
  }
  size = szLow;
#else
  size = (sysuint_t)( ((uint64_t)szHigh << FOG_UINT64_C(32)) | (uint64_t)szLow );
#endif

  if (size == 0)
  {
    CloseHandle(hFile);
    return Error::EmptyFile;
  }

  // Create FileMapping object.
  hFileMapping = CreateFileMappingW(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
  if (hFileMapping != INVALID_HANDLE_VALUE)
  {
    // Map it (if this will cause error, we try to load file manually).
    if ((data = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0)) != NULL)
    {
      _fileName = fileName;
      _data = data;
      _size = size;
      _hFileMapping = hFileMapping;
      _hFile = hFile;
      _state = Mapped;
      return Error::Ok;
    }

    err = GetLastError();
    CloseHandle(hFileMapping);
  }
  else
  {
    err = GetLastError();
  }

  if (!loadOnFail)
  {
    CloseHandle(hFile);
    return err;
  }

  data = Memory::alloc(size);
  if (!data)
  {
    CloseHandle(hFile);
    return Error::OutOfMemory;
  }

  static const sysuint_t chunkSize = 1024*1024*16;

  uint8_t* dataCur = (uint8_t*)data;
  sysuint_t i;
  DWORD bytesToRead;
  DWORD bytesRead;

  bytesToRead = chunkSize;
  for (i = size / chunkSize + 1; i; i--, dataCur += bytesToRead)
  {
    if (i == 1)
    {
      bytesToRead = (size % chunkSize);
      if (bytesToRead == 0) break;
    }

    if (!ReadFile(hFile, (LPVOID)dataCur, bytesToRead, &bytesRead, NULL) || bytesRead != bytesToRead)
    {
      err = GetLastError();
      Memory::free(data);
      CloseHandle(hFile);
      return err;
    }
  }

  CloseHandle(hFile);

  _fileName = fileName;
  _data = data;
  _size = size;
  _hFileMapping = NULL;
  _hFile = NULL;
  _state = Loaded;
  return Error::Ok;
}

void MapFile::unmap()
{
  if (!_hFileMapping && !_data) return;

  if (_hFileMapping)
  {
    UnmapViewOfFile(_data);
    CloseHandle(_hFileMapping);
    CloseHandle(_hFile);
  }
  else
  {
    Memory::free(_data);
  }

  _fileName.clear();
  _data = NULL;
  _size = 0UL;
  _hFileMapping = NULL;
  _hFile = NULL;
  _state = None;
}
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
err_t MapFile::map(const String32& fileName, bool loadOnFail)
{
  unmap();

  err_t err;

  TemporaryString8<TemporaryLength> fileName8;
  if ( (err = fileName8.set(fileName, TextCodec::local8())) ) return err;

  int fd = open(fileName8.cStr(), O_RDONLY);
  if (fd < 0) return errno;

  struct stat s;
  if (fstat(fd, &s) == 0
#if FOG_ARCH_BITS == 32
    && s.st_size < (int64_t)ULONG_MAX
#endif // FOG_ARCH_BITS == 32
    )
  {
    uint mmap_flags = PROT_READ;
    void *mmap_data = mmap(NULL, s.st_size, mmap_flags, MAP_SHARED, fd, 0);

    // Success
    if (mmap_data != (void *)-1)
    {
      _fileName = fileName;
      _data = mmap_data;
      _size = (sysuint_t)s.st_size;
      _fd = fd;
      _state = Mapped;
      return Error::Ok;
    }

    void* data = Memory::alloc((sysuint_t)s.st_size);
    if (!data)
    {
      close(fd);
      return Error::OutOfMemory;
    }

    const sysuint_t chunkSize = 1024*1024*16;

    uint8_t* dataCur = (uint8_t*)data;
    sysuint_t i;
    sysuint_t bytesToRead;
    sysuint_t bytesRead;

    bytesToRead = chunkSize;
    for (i = (sysuint_t)s.st_size / chunkSize + 1; i; i--, dataCur += bytesToRead)
    {
      if (i == 1)
      {
        bytesToRead = ((sysuint_t)s.st_size % chunkSize);
        if (bytesToRead == 0) break;
      }

      bytesRead = read(fd, dataCur, bytesToRead);
      if (bytesRead != bytesToRead)
      {
        err = errno;
        Memory::free(data);
        close(fd);
        return err;
      }
    }

    close(fd);

    _fileName = fileName;
    _data = data;
    _size = (sysuint_t)s.st_size;
    _fd = -1;
    _state = Loaded;
    return Error::Ok;
  }
  else
  {
    err = errno;
    close(fd);
    return err;
  }
}

void MapFile::unmap()
{
  if (_fd == -1 && !_data) return;

  if (_fd != -1)
  {
    munmap((char *)_data, _size);
    close(_fd);
  }
  else
  {
    Memory::free(_data);
  }

  _fileName.clear();
  _data = NULL;
  _size = 0UL;
  _fd = -1;
  _state = None;
}
#endif // FOG_OS_POSIX

} // Fog namespace
