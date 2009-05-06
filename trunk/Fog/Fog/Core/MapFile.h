// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_MAPFILE_H
#define _FOG_CORE_MAPFILE_H

// [Dependencies]
#include <Fog/Core/Atomic.h>
#include <Fog/Core/FileSystem.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/String.h>
#include <Fog/Core/Value.h>

//! @addtogroup Fog_Core
//! @{

namespace Fog {

// ============================================================================
// [Fog::MapFile]
// ============================================================================

//! @brief Class designed to provide mapping files into shared memory in a few
//! lines of code. Mapping files into shared memory increases performance. It
//! avoids mutex locking and unlocking in read / white functions and gives us
//! higher performance if file was loaded into shared memory before. System
//! will simply make data readable for current process.
//!
//! @note Class is designed only for read-only access.
struct FOG_API MapFile
{
  // [Construction / Destruction]

  MapFile();
  ~MapFile();

  // [Map / Unmap]

  err_t map(const String32& fileName, bool loadOnFail = true);
  void unmap();

  FOG_INLINE bool isOpen() const
  { return _data != NULL; }

  // [State]

  enum State
  {
    None = 0,
    Mapped = 1,
    Loaded = 2
  };

  FOG_INLINE uint32_t state() const
  { return _state; }

  // [Data]

  FOG_INLINE const String32& fileName() const
  { return _fileName; }

  FOG_INLINE const void* data() const
  { return _data; }

  FOG_INLINE sysuint_t size() const
  { return _size; }

  // [Handles]

#if defined(FOG_OS_WINDOWS)
  FOG_INLINE HANDLE hFileMapping() const
  { return _hFileMapping; }

  FOG_INLINE HANDLE hFile() const
  { return _hFile; }
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
  FOG_INLINE int fd() const
  { return _fd; }
#endif // FOG_OS_POSIX

  // [Members]

private:
  String32 _fileName;

  void* _data;
  sysuint_t _size;

#if defined(FOG_OS_WINDOWS)
  HANDLE _hFileMapping;
  HANDLE _hFile;
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
  int _fd;
#endif // FOG_OS_POSIX

  uint32_t _state;

private:
  FOG_DISABLE_COPY(MapFile)
};

} // Fog namespace

//! @}

#endif // _FOG_CORE_MAPFILE_H
