// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/ByteArray.h>
#include <Fog/Core/Error.h>
#include <Fog/Core/FileSystem.h>
#include <Fog/Core/FileUtil.h>
#include <Fog/Core/String.h>
#include <Fog/Core/StringUtil.h>
#include <Fog/Core/TextCodec.h>
#include <Fog/Core/UserInfo.h>

#if defined(FOG_OS_WINDOWS)
// windows.h is already included in Fog/Build/Build.h
#include <io.h>
#ifndef IO_REPARSE_TAG_SYMLINK
#define IO_REPARSE_TAG_SYMLINK 0xA000000C
#endif // IO_REPARSE_TAG_SYMLINK
#else
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#if defined(FOG_HAVE_UNISTD_H)
#include <unistd.h>
#endif
#endif

namespace Fog {

// ============================================================================
// [Fog::FileSystem]
// ============================================================================

#if defined(FOG_OS_WINDOWS)
uint32_t FileSystem::testFile(const String& fileName, uint32_t flags)
{
  if (flags == 0) return 0;

  TemporaryString<TemporaryLength> fileNameW;
  err_t err;
  if ((err = fileNameW.set(fileName)) || (err = fileNameW.slashesToWin())) return 0;

  WIN32_FILE_ATTRIBUTE_DATA fi;

  if (GetFileAttributesExW(reinterpret_cast<const char wchar_t*>(fileNameW.cData()), GetFileExInfoStandard, &fi))
  {
    uint result = Exists;
    if (fi.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      result |= FileSystem::IsDirectory;
    else
      result |= FileSystem::IsFile;

    if (fi.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
      result |= FileSystem::CanRead;
    else
      result |= FileSystem::CanRead | FileSystem::CanWrite;

    if ((flags & FileSystem::IsLink) && (fi.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
    {
      WIN32_FIND_DATAW fd;
      HANDLE r;
      
      r = FindFirstFileW(reinterpret_cast<const wchar_t*>(fileNameW.cData()), &fd);
      if (r != INVALID_HANDLE_VALUE)
      {
        if (fd.dwReserved0 & IO_REPARSE_TAG_SYMLINK)
        {
          result |= FileSystem::IsLink;
        }
        FindClose(r);
      }
    }

    if (fi.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) result |= FileSystem::IsHidden;
    if (fi.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) result |= FileSystem::IsArchive;
    if (fi.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) result |= FileSystem::IsCompressed;
    if (fi.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE) result |= FileSystem::IsSparse;
    if (fi.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) result |= FileSystem::IsSystem;

    // IsExecutable and CanExecute
    if ((flags & (FileSystem::IsExecutable | FileSystem::CanExecute)) != 0)
    {
      TemporaryString<16> ext;
      FileUtil::extractExtension(ext, fileName);

      const Char* extStr = ext.cData();
      sysuint_t extLength = ext.getLength();

      // Executable extension has usually 3 characters
      if (extLength == 3)
      {
        if (StringUtil::eq(extStr, "exe", 3, CaseInsensitive) ||
            StringUtil::eq(extStr, "com", 3, CaseInsensitive) ||
            StringUtil::eq(extStr, "bat", 3, CaseInsensitive))
        {
          result |= FileSystem::IsExecutable | FileSystem::CanExecute;
        }
      }
    }

    return result & flags;
  }
  else
    return 0;
}

bool FileSystem::findFile(const Sequence<String>& paths, const String& fileName, String& dst)
{
  Sequence<String> _paths(paths);
  Sequence<String>::ConstIterator it(_paths);

  WIN32_FILE_ATTRIBUTE_DATA fi;
  TemporaryString<TemporaryLength> path;

  for (it.toStart(); it.isValid(); it.toNext())
  {
    // Set path.
    path.setDeep(it.value());

    // Append directory separator if needed.
    if (!it.value().endsWith(Ascii8("\\", 1)) || !it.value().endsWith(Ascii8("/", 1)))
    {
      path.append(Char('\\'));
    }

    // Append file. If something fail here we try to find file in next path.
    if (path.append(fileName) != Error::Ok) continue;
    if (path.slashesToWin() != Error::Ok) continue;

    // Test.
    if (GetFileAttributesExW(reinterpret_cast<const wchar_t*>(pathW.cData()), GetFileExInfoStandard, &fi) &&
      !(fi.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
      return FileUtil::joinPath(dst, it.value(), fileName) == Error::Ok;
    }
  }

  return false;
}

static uint createDirectoryHelper(const Char* path, sysuint_t len)
{
  err_t err;

  if (len == 3 && path[0].isAsciiAlpha() && path[1] == Char(':') && path[2] == Char('/'))
  {
    // TODO: Maybe we should return failure if disk is not mounted.
    return Error::DirectoryAlreadyExists;
  }

  TemporaryString<TemporaryLength> pathW;
  if ((err = pathW.set(path, len)) || (err = pathW.slashesToWin())) return err;

  if (!CreateDirectoryW(reinterpret_cast<const wchar_t*>(pathW.cData()), NULL))
  {
    // TODO: Move this to some helper function that will translate windows error
    // codes into Fog error codes.
    DWORD lastError = GetLastError();
    if (lastError != ERROR_ALREADY_EXISTS)
      return lastError;
    else
      return Error::DirectoryAlreadyExists;
  }

  return Error::Ok;
}

err_t FileSystem::createDirectory(const String& dir, bool recursive)
{
  if (dir.isEmpty()) return Error::InvalidArgument;
  if (!recursive) return createDirectoryHelper(dir.cData(), dir.getLength());

  err_t err;
  TemporaryString<TemporaryLength> dirAbs;

  if ( (err = FileUtil::toAbsolutePath(dirAbs, String(), dir)) ) return err;

  // FileSystem::toAbsolutePath() always normalize dir to '/', we can imagine
  // that dirAbs is absolute dir, so we need to find first two occurences
  // of '/'. Second occurece can be end of string.
  sysuint_t i = dirAbs.indexOf(Char('/'));
  sysuint_t length = dirAbs.getLength();

  if (i == InvalidIndex) return Error::InvalidArgument;
  if (dirAbs.at(length-1) == Char('/')) length--;

  do {
    i++;
    i = dirAbs.indexOf(Char('/'), CaseSensitive, Range(i, length - i));

    err = createDirectoryHelper(dirAbs.cData(), (i == InvalidIndex) ? length : i);
    if (err != Error::Ok && err != Error::DirectoryAlreadyExists) return err;
  } while (i != InvalidIndex);

  return Error::Ok;
}

err_t FileSystem::deleteDirectory(const String& path)
{
  err_t err;
  TemporaryString<TemporaryLength> pathW;

  if ((err = pathW.set(path)) ||
      (err = pathW.slashesToWin()))
  {
    return err;
  }

  if (RemoveDirectoryW(reinterpret_cast<const wchar_t*>(pathW.cData())))
    return Error::Ok;
  else
    return GetLastError();
}
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
int FileSystem::stat(const String& fileName, struct stat* s)
{
  TemporaryByteArray<TemporaryLength> t;
  TextCodec::local8().appendFromUnicode(t, fileName);
  return ::stat(t.cData(), s);
}

static uint32_t test_stat(struct stat *s, uint32_t flags)
{
  uint32_t result = FileSystem::Exists;

  if (S_ISREG(s->st_mode)) result |= FileSystem::IsFile;
  if (S_ISDIR(s->st_mode)) result |= FileSystem::IsDirectory;
  if (S_ISLNK(s->st_mode)) result |= FileSystem::IsLink;

  if ((s->st_mode & S_IXUSR) ||
      (s->st_mode & S_IXGRP) ||
      (s->st_mode & S_IXOTH) )
  {
    result |= FileSystem::IsExecutable;
  }

  if ((flags & (FileSystem::CanRead   |
                FileSystem::CanWrite  |
                FileSystem::CanExecute)) != 0)
  {
    uid_t uid = UserInfo::uid();
    gid_t gid = UserInfo::gid();

    if (s->st_uid == uid && (s->st_mode & S_IRWXU)) {
      if (s->st_mode & S_IRUSR) result |= FileSystem::CanRead;
      if (s->st_mode & S_IWUSR) result |= FileSystem::CanWrite;
      if (s->st_mode & S_IXUSR) result |= FileSystem::CanExecute;
    }
    else if (s->st_gid == gid && (s->st_mode & S_IRWXG)) {
      if (s->st_mode & S_IRGRP) result |= FileSystem::CanRead;
      if (s->st_mode & S_IWGRP) result |= FileSystem::CanWrite;
      if (s->st_mode & S_IXGRP) result |= FileSystem::CanExecute;
    }
    else if (s->st_mode & S_IRWXO) {
      if (s->st_mode & S_IROTH) result |= FileSystem::CanRead;
      if (s->st_mode & S_IWOTH) result |= FileSystem::CanWrite;
      if (s->st_mode & S_IXOTH) result |= FileSystem::CanExecute;
    }
    // TODO: How to handle this...?
    else
    {
      result |= FileSystem::CanRead;
      result |= FileSystem::CanWrite;
      result |= FileSystem::CanExecute;
    }
  }

  // Return and clear up flags that wasn't specified to return.
  return result & flags;
}

uint32_t FileSystem::testFile(const String& fileName, uint32_t flags)
{
  if (flags == 0) return 0;
  struct stat s;
  return (stat(fileName, &s) == 0) ? test_stat(&s, flags) : 0;
}

bool FileSystem::findFile(const Sequence<String>& paths, const String& fileName, String& dst)
{
  Sequence<String> _paths(paths);
  Sequence<String>::ConstIterator it(_paths);

  struct stat s;

  TemporaryByteArray<TemporaryLength> path8;
  TemporaryByteArray<TemporaryLength> fileName8;

  // Encode fileName here to avoid encoding in loop.
  TextCodec::local8().appendFromUnicode(fileName8, fileName);

  for (it.toStart(); it.isValid(); it.toNext())
  {
    // Append path.
    TextCodec::local8().fromUnicode(path8, it.value());

    // Append directory separator if needed
    if (path8.getLength() && !path8.endsWith(Str8("/", 1))) path8.append('/');

    // Append file
    path8.append(fileName8);

    // Test
    if (::stat(path8.cData(), &s) == 0 && S_ISREG(s.st_mode))
    {
      return FileUtil::joinPath(dst, it.value(), fileName) == Error::Ok;
    }
  }
  return false;
}

static err_t createDirectoryHelper(const Char* path, sysuint_t len)
{
  if (len == 1 && path[0] == '/') return Error::DirectoryAlreadyExists;

  TemporaryByteArray<TemporaryLength> path8;
  err_t err;

  if ((err = TextCodec::local8().appendFromUnicode(path8, path, len))) return err;
  if (::mkdir(path8.cData(), S_IRWXU | S_IXGRP | S_IXOTH) == 0) return Error::Ok;

  if (errno == EEXIST)
    return Error::DirectoryAlreadyExists;
  else
    return errno;
}

err_t FileSystem::createDirectory(const String& dir, bool recursive)
{
  if (dir.isEmpty()) return Error::InvalidArgument;
  if (!recursive) return createDirectoryHelper(dir.cData(), dir.getLength());

  err_t err;
  TemporaryString<TemporaryLength> dirAbs;
  if ( (err = FileUtil::toAbsolutePath(dirAbs, String(), dir)) ) return err;

  // FileSystem::toAbsolutePath() always normalize dir to '/', we can imagine
  // that dirAbs is absolute dir, so we need to find first two occurences
  // of '/'. Second occurece can be end of string.
  if (dirAbs.getLength() == 1 && dirAbs.at(0) == '/') return Error::DirectoryAlreadyExists;

  sysuint_t i = dirAbs.indexOf(Char('/'));
  sysuint_t length = dirAbs.getLength();

  if (i == InvalidIndex) return Error::InvalidArgument;
  if (dirAbs.at(length-1) == Char('/')) length--;

  do {
    i++;
    i = dirAbs.indexOf(Char('/'), CaseSensitive, Range(i, length - i));

    err = createDirectoryHelper(dirAbs.cData(), (i == InvalidIndex) ? length : i);
    if (err != Error::Ok && err != Error::DirectoryAlreadyExists) return err;
  } while (i != InvalidIndex);

  return Error::Ok;
}

err_t FileSystem::deleteDirectory(const String& dir)
{
  err_t err;
  TemporaryByteArray<TemporaryLength> dir8;

  if ((err = TextCodec::local8().appendFromUnicode(dir8, dir))) return err;

  if (::rmdir(dir8.cData()) == 0)
    return Error::Ok;
  else
    return errno;
}
#endif // FOG_OS_POSIX

} // Fog namespace
