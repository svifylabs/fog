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
#include <Fog/Core/FileUtil.h>
#include <Fog/Core/String.h>
#include <Fog/Core/StringUtil.h>
#include <Fog/Core/TextCodec.h>
#include <Fog/Core/UserInfo.h>

#if defined(FOG_OS_WINDOWS)
// windows.h is already included in Fog/Build/Build.h
#include <io.h>
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
uint32_t FileSystem::testFile(const String32& fileName, uint32_t flags)
{
  if (flags == 0) return 0;

  TemporaryString16<TemporaryLength> fileNameW;
  err_t err = fileNameW.set(fileName);
  if (err) return 0;

  WIN32_FILE_ATTRIBUTE_DATA fi;

  if (GetFileAttributesExW(fileNameW.cStrW(), GetFileExInfoStandard, &fi))
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

    if (fi.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) result |= FileSystem::IsHidden;
    if (fi.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE) result |= FileSystem::IsArchive;
    if (fi.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED) result |= FileSystem::IsCompressed;
    if (fi.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE) result |= FileSystem::IsSparse;
    if (fi.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM) result |= FileSystem::IsSystem;

    // IsExecutable and CanExecute
    if ((flags & (FileSystem::IsExecutable | FileSystem::CanExecute)) != 0)
    {
      TemporaryString32<16> ext;
      FileUtil::extractExtension(ext, fileName);

      const Char32* extStr = ext.cData();
      sysuint_t extLength = ext.length();

      // Executable extension has usually 3 characters
      if (extLength == 3)
      {
        if (StringUtil::eq(extStr, (const Char8*)"exe", 3, CaseInsensitive) ||
            StringUtil::eq(extStr, (const Char8*)"com", 3, CaseInsensitive) ||
            StringUtil::eq(extStr, (const Char8*)"bat", 3, CaseInsensitive))
        {
          result |= FileSystem::IsExecutable |
                    FileSystem::CanExecute;
        }
      }
    }

    return result & flags;
  }
  else
    return 0;
}

bool FileSystem::findFile(const Sequence<String32>& paths, const String32& fileName, String32& dst)
{
  Sequence<String32> _paths(paths);
  Sequence<String32>::ConstIterator it(_paths);

  WIN32_FILE_ATTRIBUTE_DATA fi;
  TemporaryString16<TemporaryLength> pathW;
  TemporaryString16<TemporaryLength> fileNameW;

  // Encode fileName here to avoid encoding in loop
  err_t err;
  if ((err = fileNameW.set(fileName)) ||
      (err = fileNameW.slashesToWin()))
  {
    return err;
  }

  for (it.toStart(); it.isValid(); it.toNext())
  {
    // Set path
    pathW.set(it.value());

    // Append directory separator if needed
    if (!it.value().endsWith(StubAscii8("\\", 1)) || 
        !it.value().endsWith(StubAscii8("/", 1)))
    {
      pathW.append(StubAscii8("\\", 1));
    }

    // Append file
    pathW.append(fileNameW);

    // Test
    if (GetFileAttributesExW(pathW.cStrW(), GetFileExInfoStandard, &fi) &&
      !(fi.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
    {
      return FileUtil::joinPath(dst, it.value(), fileName) == Error::Ok;
    }
  }

  return false;
}

err_t FileSystem::getWorkingDirectory(String32& dst)
{
  err_t err;
  TemporaryString16<TemporaryLength> dirW;

  for (;;)
  {
    DWORD size = GetCurrentDirectoryW(dirW.capacity()+1, dirW.mStrW());
    if (size >= dirW.capacity())
    {
      if ( (err = dirW.reserve(size)) ) return err;
      continue;
    }
    else
    {
      if ((err = dst.set(dirW)) ) return err;
      return dst.slashesToPosix();
    }
  }
}

err_t FileSystem::setWorkingDirectory(const String32& dir)
{
  err_t err;
  TemporaryString16<TemporaryLength> dirW;

  if ((err = dirW.set(dir)) ||
      (err = dirW.slashesToWin()))
  {
    return err;
  }

  if (SetCurrentDirectoryW(dirW.cStrW()) == 0)
    return Error::Ok;
  else
    return GetLastError();
}

static uint createDirectoryHelper(const Char32* path, sysuint_t len)
{
  err_t err;

  if (len == 3 &&
    path[0].isAsciiAlpha() &&
    path[1] == Char32(':') &&
    path[2] == Char32('/'))
  {
    return 2;
  }

  TemporaryString16<TemporaryLength> pathW;
  if ((err = pathW.set(StubUtf32(path, len))) ||
      (err = pathW.slashesToWin()))
  {
    return err;
  }

  if (!CreateDirectoryW(pathW.cStrW(), NULL))
  {
    DWORD lastError = GetLastError();
    if (lastError != ERROR_ALREADY_EXISTS)
      return 0;
    else
      return 2;
  }

  return 1;
}

err_t FileSystem::createDirectory(const String32& dir, bool recursive)
{
  if (dir.isEmpty()) return Error::InvalidArgument;
  if (!recursive) return createDirectoryHelper(dir.cData(), dir.length());

  err_t err;
  TemporaryString32<TemporaryLength> dirAbs;

  if ( (err = FileUtil::toAbsolutePath(dirAbs, String32(), dir)) ) return err;

  // FileSystem::toAbsolutePath() always normalize dir to '/', we can imagine
  // that dirAbs is absolute dir, so we need to find first two occurences
  // of '/'. Second occurece can be end of string.
  sysuint_t i = dirAbs.indexOf(Char32('/'));
  sysuint_t length = dirAbs.length();

  if (i == InvalidIndex) return Error::InvalidArgument;
  if (dirAbs.at(length-1) == Char32('/')) length--;

  do {
    i++;
    i = dirAbs.indexOf(Char32('/'), CaseSensitive, Range(i, length - i));

    err = createDirectoryHelper(dirAbs.cData(), (i == InvalidIndex) ? length : i);
    if (err != Error::Ok && err != Error::DirectoryAlreadyExists) return err;
  } while (i != InvalidIndex);

  return Error::Ok;
}

err_t FileSystem::deleteDirectory(const String32& path)
{
  err_t err;
  TemporaryString16<TemporaryLength> pathW;

  if ((err = pathW.set(path)) ||
      (err = pathW.slashesToWin()))
  {
    return err;
  }

  if (RemoveDirectoryW(pathW.cStrW()))
    return Error::Ok;
  else
    return GetLastError();
}
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
int FileSystem::stat(const String32& fileName, struct stat* s)
{
  TemporaryString8<TemporaryLength> t;
  t.set(fileName, TextCodec::local8());
  return ::stat(t.cStr(), s);
}

static uint32_t test_stat(struct stat *s, uint32_t flags)
{
  uint result = 0;

  if (flags & File::Exists) result |= File::Exists;

  if ((flags & File::IsFile)      && S_ISREG(s->st_mode)) result |= File::IsFile;
  if ((flags & File::IsDirectory) && S_ISDIR(s->st_mode)) result |= File::IsDirectory;
  if ((flags & File::IsLink)      && S_ISLNK(s->st_mode)) result |= File::IsLink;

  if ((flags  & File::IsExecutable) &&
     ((s->st_mode & S_IXUSR) ||
    (s->st_mode & S_IXGRP) ||
    (s->st_mode & S_IXOTH) ))
  {
    result |= File::IsExecutable;
  }

  if ((flags & (File::CanRead | File::CanWrite | File::CanExecute)) != 0)
  {
    uid_t uid = UserInfo::uid();
    gid_t gid = UserInfo::gid();

    if (s->st_uid == uid && (s->st_mode & S_IRWXU)) {
      if ((flags & File::CanRead)    && (s->st_mode & S_IRUSR)) result |= File::CanRead;
      if ((flags & File::CanWrite)   && (s->st_mode & S_IWUSR)) result |= File::CanWrite;
      if ((flags & File::CanExecute) && (s->st_mode & S_IXUSR)) result |= File::CanExecute;
    }
    else if (s->st_gid == gid && (s->st_mode & S_IRWXG)) {
      if ((flags & File::CanRead)    && (s->st_mode & S_IRGRP)) result |= File::CanRead;
      if ((flags & File::CanWrite)   && (s->st_mode & S_IWGRP)) result |= File::CanWrite;
      if ((flags & File::CanExecute) && (s->st_mode & S_IXGRP)) result |= File::CanExecute;
    }
    else if (s->st_mode & S_IRWXO) {
      if ((flags & File::CanRead)    && (s->st_mode & S_IROTH)) result |= File::CanRead;
      if ((flags & File::CanWrite)   && (s->st_mode & S_IWOTH)) result |= File::CanWrite;
      if ((flags & File::CanExecute) && (s->st_mode & S_IXOTH)) result |= File::CanExecute;
    }
    // TODO: How to handle this...?
    else {
      if (flags & File::CanRead)    result |= File::CanRead;
      if (flags & File::CanWrite)   result |= File::CanWrite;
      if (flags & File::CanExecute) result |= File::CanExecute;
    }
  }

  return result & flags;
}

uint32_t FileSystem::testFile(const String32& fileName, uint32_t flags)
{
  if (flags == 0) return 0;
  struct stat s;
  return (stat(fileName, &s) == 0) ? test_stat(&s, flags) : 0;
}

bool FileSystem::findFile(const Sequence<String32>& paths, const String32& fileName, String32& dst)
{
  Sequence<String32> _paths(paths);
  Sequence<String32>::ConstIterator it(_paths);

  struct stat s;

  TemporaryString8<TemporaryLength> path8;
  TemporaryString8<TemporaryLength> fileName8;

  // Encode fileName here to avoid encoding in loop
  fileName8.set(fileName, TextCodec::local8());

  for (it.toStart(); it.isValid(); it.toNext())
  {
    // Append path
    path8.setLocal(it.value());

    // Append directory separator if needed
    if (path8.length() && !path8.endsWith("/", 1))
    {
      path8.append('/');
    }

    // Append file
    path8.append(fileName8);

    // Test
    if (::stat(path8.cStr(), &s) == 0 && S_ISREG(s.st_mode))
    {
      return FileSystem::joinPath(it.value(), fileName, dst) == Error::Ok;
    }
  }
  return false;
}

err_t FileSystem::getWorkingDirectory(String32& dst)
{
  TemporaryString8<TemporaryLength> dir8;

  dst.clear();
  for (;;)
  {
    char* ptr = ::getcwd(dir8.mStr(), dir8.capacity()+1);
    if (ptr)
    {
      dst.set(StubLocal(ptr));
      return Error::Ok;
    }
    if (errno != ERANGE) return errno;

    // Alloc more...
    if ((err = t.reserve(t.capacity() + 4096))) return err;
  }
}


err_t FileSystem::setWorkingDirectory(const String32& dir)
{
  err_t err;
  TemporaryString8<TemporaryLength> dir8;

  if ( (err = dir8.set(dir, TextCodec::local8())) ) return err;

  if (::chdir(dir8.cStr()) == 0)
    return Error::Ok;
  else
    return errno;
}

static err_t createDirectoryHelper(const Char32* path, sysuint_t len)
{
  if (len == 1 && path[0] == '/') return Error::DirectoryAlreadyExists;

  err_t err;
  TemporaryString8<TemporaryLength> path8;

  if ( (err = path8.set(StubUtf32(path, len), TextCodec::local8())) ) return err;

  if (mkdir(t.cStr(), S_IRWXU | S_IXGRP | S_IXOTH) == 0) return Error::Ok;

  if (errno == EEXIST) 
    return Error::DirectoryAlreadyExists;
  else
    return errno;
}

err_t FileSystem::createDirectory(const String32& dir, bool recursive)
{
  if (dir.isEmpty()) return Error::InvalidArgument;
  if (!recursive) return createDirectoryHelper(dir.cData(), dir.length());

  err_t err;
  TemporaryString32<TemporaryLength> dirAbs;
  if ( (err = FileSystem::toAbsolutePath(dirAbs, String32(), dir)) ) return err;

  // FileSystem::toAbsolutePath() always normalize dir to '/', we can imagine
  // that dirAbs is absolute dir, so we need to find first two occurences
  // of '/'. Second occurece can be end of string.
  if (dirAbs.length() == 1 && dirAbs.at(0) == '/') return Error::DirectoryAlreadyExists;

  sysuint_t i = dirAbs.indexOf(Char32('/'));
  sysuint_t length = dirAbs.length();

  if (i == InvalidIndex) return Error::InvalidArgument;
  if (dirAbs.at(length-1) == Char32('/')) length--;

  do {
    i++;
    i = dirAbs.indexOf(Char32('/'), Range(i, length - i));

    err = createDirectoryHelper(dirAbs.cData(), (i == InvalidIndex) ? length : i);
    if (err != Error::Ok && err != Error::DirectoryAlreadyExists) return err;
  } while (i != InvalidIndex);

  return Error::Ok;
}

err_t FileSystem::deleteDirectory(const String32& dir)
{
  err_t err;
  TemporaryString8<TemporaryLength> dir8;

  if ( (err = dir8.set(dir, TextCodec::local8())) ) return err;

  if (rmdir(dir8.cStr()) == 0)
    return Error::Ok;
  else
    return errno;
}
#endif // FOG_OS_POSIX

} // Fog namespace
