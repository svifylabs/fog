// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_FILESYSTEM_H
#define _FOG_CORE_FILESYSTEM_H

// [Dependencies]
#include <Fog/Core/Sequence.h>
#include <Fog/Core/String.h>

#if defined(FOG_OS_POSIX)
#include <sys/stat.h>
#endif // FOG_OS_POSIX

//! @addtogroup Fog_Core
//! @{

namespace Fog {
namespace FileSystem {

// ============================================================================
// [Fog::FileSystem]
// ============================================================================

enum Test
{
  // [General Attributes]

  //! @brief Test if file element exist (it's a file, directory or anything else).
  Exists = (1 << 0),
  //! @brief Test if file is regular file.
  IsFile = (1 << 1),
  //! @brief Test if file is directory.
  IsDirectory = (1 << 2),
  //! @brief Test if file is symlink.
  IsLink = (1 << 3),
  //! @brief Test if file is executable.
  IsExecutable = (1 << 4),
  //! @brief Test if file is hidden.
  IsHidden = (1 << 5),

  // [Windows Only Attributes]

  //! @brief File is archive, has Windows @c FILE_ATTRIBUTE_ARCHIVE attribute.
  IsArchive = (1 << 6),
  //! @brief File is archive, has Windows @c FILE_ATTRIBUTE_COMPRESSED attribute.
  IsCompressed = (1 << 7),
  //! @brief File is archive, has Windows @c FILE_ATTRIBUTE_SPARSE_FILE attribute.
  IsSparse = (1 << 8),
  //! @brief File is archive, has Windows @c FILE_ATTRIBUTE_SYSTEM attribute.
  IsSystem = (1 << 9),

  // [Permissions]

  //! @brief Test if file can be readed.
  CanRead = (1 << 5),
  //! @brief Test if file can be writed.
  CanWrite = (1 << 6),
  //! @brief Test if file can be executed.
  CanExecute = (1 << 7),

  // [All]

  //! @brief Test for all.
  TestAll =
    Exists       |
    IsFile       |
    IsDirectory  |
    IsLink       |
    IsExecutable |
    CanRead      |
    CanWrite     |
    CanExecute   |
    IsHidden     |
    IsArchive    |
    IsCompressed |
    IsSparse     |
    IsSystem
};

#if defined(FOG_OS_POSIX)
static int stat(const String& fileName, struct stat* s);
#endif // FOG_OS_POSIX

FOG_API uint32_t testFile(const String& fileName, uint32_t flags);
FOG_API bool findFile(const Sequence<String>& paths, const String& fileName, String& dest);

static FOG_INLINE bool exists(const String& fileName)
{ return testFile(fileName, Exists) == Exists; }

static FOG_INLINE bool isFile(const String& fileName)
{ return testFile(fileName, IsFile) == IsFile; }

static FOG_INLINE bool isDirectory(const String& fileName)
{ return testFile(fileName, IsDirectory) == IsDirectory;}

FOG_API err_t createDirectory(const String& dir, bool recursive = true);
FOG_API err_t deleteDirectory(const String& dir);

} // FileSystem namespace
} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_CORE_FILESYSTEM_H
