// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_DIRITERATOR_H
#define _FOG_CORE_DIRITERATOR_H

// [Dependencies]
#include <Fog/Core/ByteArray.h>
#include <Fog/Core/String.h>

#if defined(FOG_OS_WINDOWS)
#include <windows.h>
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
#include <sys/stat.h>
#endif // FOG_OS_POSIX

//! @addtogroup Fog_Core
//! @{

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct DirIterator;

// ============================================================================
// [Fog::DirIterator]
// ============================================================================

//! @brief DirIterator browsing class.
//!
//! This class can be used to browse a directory.
//!
//! @code
//! // Code example to successfully parse current directory
//!
//! // Open a directory stream with a given path, path will be normalized,
//! // so we can use "" and "." for current directory or for example ".." for
//! // parent of current directory
//! Fog::DirIterator dir(".");
//! Fog::DirIterator::Entry entry;
//!
//! // We can check if directory is successfully opened, bud @c read() return
//! // false if not, so this check isn't necessary here.
//! while (dir.read(entry))
//! {
//!   // We are readed directory entry, check it by entry.type() and
//!   // use its name by entry.name(). See @c Entry for details.
//! }
//!
//! // Close directory stream if opened
//! dir.close();
//! @endcode
struct FOG_API DirIterator
{
  //! @brief DirIterator entry.
  struct FOG_API Entry
  {
    Entry();
    Entry(const Entry& other);
    ~Entry();

    Entry& operator=(const Entry& other);

    FOG_INLINE const String& getName() const { return _name; }
    FOG_INLINE uint32_t getType() const { return _type; }
    FOG_INLINE uint64_t getSize() const { return _size; }

#if defined(FOG_OS_WINDOWS)
    FOG_INLINE const WIN32_FIND_DATAW& getWinFindData() const { return _winFindData; }
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
    FOG_INLINE const struct stat& getStatInfo() const { return _statInfo; }
#endif // FOG_OS_POSIX

    //! @brief  entry type.
    enum Type
    {
      Unknown = 0,
      File = (1 << 0),
      Directory = (1 << 1),
      CharacterDevice = (1 << 2),
      BlockDevice = (1 << 3),
      Fifo = (1 << 4),
      Link = (1 << 5),
      Socket = (1 << 6)
    };

  private:
    String _name;
    uint32_t _type;
    uint64_t _size;

#if defined(FOG_OS_WINDOWS)
    WIN32_FIND_DATAW _winFindData;
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
    struct stat _statInfo;
#endif // FOG_OS_POSIX

    friend struct DirIterator;
  };

  DirIterator();
  DirIterator(const String& path);
  ~DirIterator();

  err_t open(const String& path);
  void close();
  bool read(Entry& to);

  err_t rewind();
  int64_t tell();

  FOG_INLINE void setSkipDots(bool val) { _skipDots = val; }

  FOG_INLINE void* getHandle() const { return _handle; }

  FOG_INLINE const String& getPath() const { return _path; }

  FOG_INLINE bool isOpen() const  { return _handle != NULL; }

  FOG_INLINE bool skipDots() const { return _skipDots; }

private:
  void* _handle;
  String _path;

  // System specific

#if defined(FOG_OS_WINDOWS)
  WIN32_FIND_DATAW _winFindData;
  int64_t _position;
  bool _fileInEntry;
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
  ByteArray _pathCache;
  sysuint_t _pathCacheBaseLength;
#endif // FOG_OS_POSIX

  bool _skipDots;

  FOG_DISABLE_COPY(DirIterator)
};

} // Fog namespace

//! @}

// ============================================================================
// [Fog::TypeInfo<T>]
// ============================================================================

FOG_DECLARE_TYPEINFO(Fog::DirIterator::Entry, Fog::MoveableType)

// [Guard]
#endif // _FOG_CORE_DIRITERATOR_H
