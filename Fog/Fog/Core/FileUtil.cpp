// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Application.h>
#include <Fog/Core/Error.h>
#include <Fog/Core/FileSystem.h>
#include <Fog/Core/FileUtil.h>
#include <Fog/Core/String.h>
#include <Fog/Core/StringUtil.h>
#include <Fog/Core/TextCodec.h>
#include <Fog/Core/UserInfo.h>

namespace Fog {

// ============================================================================
// [Fog::FileUtil]
// ============================================================================

// Copy src to dst and convert slashes to posix form
static Char32* copyAndNorm(Char32* dst, const Char32* src, sysuint_t length)
{
  for (sysuint_t i = length; i; i--)
  {
#if defined(FOG_OS_WINDOWS)
    Char32 ch = *src++;
    if (FOG_UNLIKELY(ch == Char32('\\')))
      *dst++ = '/';
    else
      *dst++ = ch;
#else
    *dst++ = *src++;
#endif
  }

  return dst;
}

static bool isDirSeparator(Char32 ch)
{
  return ch == Char32('/') || ch == Char32('\\');
}

err_t FileUtil::extractFile(String32& dst, const String32& path)
{
  sysuint_t index = path.lastIndexOf(Char32('/')) + 1U;

  if (index)
  {
    String32 path_(path);
    return dst.set(Utf32(path_.cData() + index, path_.getLength() - index));
  }
  else
  {
    return dst.set(path);
  }
}

err_t FileUtil::extractDirectory(String32& dst, const String32& _path)
{
  sysuint_t index = _path.lastIndexOf(Char32('/'));

  // in some cases (for example path /root), index can be 0. So check for this
  // case
  if (index != InvalidIndex)
  {
    String32 path(_path);
    return dst.set(Utf32(path.cData(), index != 0 ? index : 1));
  }
  else
  {
    dst.clear();
    return Error::Ok;
  }
}

err_t FileUtil::extractExtension(String32& dst, const String32& _path)
{
  String32 path(_path);
  dst.clear();

  sysuint_t pathLength = path.getLength();
  if (!pathLength) { return Error::InvalidArgument; }

  // Speed is important, so RAW manipulation is needed
  const Char32* pathBegin = path.cData();
  const Char32* pathEnd = pathBegin + pathLength;
  const Char32* pathCur = pathEnd;

  do {
    if (FOG_UNLIKELY(*--pathCur == Char32('.')))
    {
      if (++pathCur != pathEnd)
      {
        return dst.set(Utf32(pathCur, sysuint_t(pathEnd - pathCur)));
      }
      break;
    }
    else if (*pathCur == Char32('/'))
    {
      break;
    }
  } while (pathCur != pathBegin);

  return Error::Ok;
}

err_t FileUtil::normalizePath(String32& dst, const String32& _path)
{
  if (isNormalizedPath(_path)) return dst.set(_path);

  String32 path(_path);
  // we need to convert:
  // - all "//" sequences to "/"
  // - all "/./" to "/" or "BEGIN./" to ""
  // - all "/../" eats previous directory

  sysuint_t pathLength = path.getLength();
  bool prevSlash = false;

  err_t err;
  if ( (err = dst.resize(pathLength)) ) return err;

  Char32* dstBeg = dst.xData();
  Char32* dstCur = dstBeg;

  const Char32* pathCur = path.cData();
  const Char32* pathEnd = pathCur + pathLength;

  Char32 c;

  // Handle Windows absolute path "X:\"
#if defined(FOG_OS_WINDOWS)
  if (pathLength > 2 &&
    pathCur[0].isAsciiAlpha() &&
    pathCur[1] == Char32(':') &&
    isDirSeparator(pathCur[2]))
  {
    dstCur[0] = pathCur[0];
    dstCur[1] = pathCur[1];

    // We will make path like absolute, next slash will be handled
    // in loop and corrected
    dstBeg += 2; // Increase begin pointer too, see code why...
    dstCur += 2;
    pathCur += 2;
  }
#endif

  while (pathCur < pathEnd)
  {
    c = *pathCur++;

    // skip "./" from result
    if (isDirSeparator(c) && (sysuint_t)(dstCur - dstBeg) == 1 && dstCur[-1] == Char32('.'))
    { 
      prevSlash = true; 
      dstCur--; 
      continue; 
    }

    if (prevSlash)
    {
      // Catch "//" -> "/" sequence
      if (isDirSeparator(c)) 
        // We were already written "/", so continue is ok
        continue;

      // Catch "/."
      if (c == Char32('.'))
      {
        register sysuint_t remain = (sysuint_t)(pathEnd - pathCur);

        // Catch "/./" -> "/"
        if (remain > 0 && isDirSeparator(pathCur[0])) 
        {
          // We were already written "/", so increase by one and continue
          pathCur++; continue;
        }

        // Catch "/../" -> Eat previous if is
        if (remain > 1 && pathCur[0] == Char32('.') && isDirSeparator(pathCur[1]))
        {
          // We know that dstCur[-1] contains '/', so we need to
          // skip this separator and get back last directory. Only what
          // we need to check if "../" possibility. In this case we can't
          // go back and we will append "../" to the result.

          sysuint_t resultLength = (sysuint_t)(dstCur - dstBeg);

          // Bail if result is too small to hold any directory
          if (resultLength <= 1) goto __inc;

          // check for "../"
          if (resultLength > 2 && 
            (dstCur - 3 == dstBeg || isDirSeparator(dstCur[-4])) &&
            dstCur[-3] == Char32('.') &&
            dstCur[-2] == Char32('.')) goto __inc;

          // Now we can continue
          dstCur -= 2;
          pathCur += 2;
          
          for (;;) 
          {
            c = *dstCur;
            if (isDirSeparator(c)) goto __inc;
            if (dstCur == dstBeg) 
            {
              prevSlash = false; 
              break; 
            }
            dstCur--;
          }
          continue;
        }
      }
    }

__inc:
    prevSlash = isDirSeparator(c);
#if defined(FOG_OS_WINDOWS)
    if (c == Char32('\\')) c = Char32('/');
#endif
    *dstCur++ = c;
  }

  dst.xFinalize(dstCur);
  return Error::Ok;
}

err_t FileUtil::toAbsolutePath(String32& dst, const String32& base, const String32& path)
{
  if (!FileUtil::isAbsolutePath(path))
  {
    err_t err;
    if (base.isEmpty())
    {
      TemporaryString32<TemporaryLength> working;
      if ( (err = Application::getWorkingDirectory(working)) ) return err;
      if ( (err = FileUtil::joinPath(dst, working, path)) ) return err;
    }
    else
    {
      if ( (err = FileUtil::joinPath(dst, base, path)) ) return err;
    }
    return FileUtil::normalizePath(dst, dst);
  }
  else
    return FileUtil::normalizePath(dst, path);
}

static err_t _joinPath(String32& dst, const String32& base, const String32& part)
{
  const Char32* d_str = base.cData();
  const Char32* f_str = part.cData();
  sysuint_t d_length = base.getLength();
  sysuint_t f_length = part.getLength();

  if (d_length && isDirSeparator(d_str[d_length-1])) d_length--;

  // This is not so readable but fast
  err_t err;
  if ( (err = dst.resize(d_length + f_length + 1)) ) return err;

  Char32* dstCur = dst.xData();
  dstCur = copyAndNorm(dstCur, d_str, d_length); *dstCur++ = '/';
  dstCur = copyAndNorm(dstCur, f_str, f_length);
  dst.xFinalize();

  return Error::Ok;
}

err_t FileUtil::joinPath(String32& dst, const String32& base, const String32& part)
{
  // if base or path is shared with dst, we need to do copy
  if (dst._d == base._d || dst._d == part._d)
    return _joinPath(dst, String32(base), String32(part));
  else
    return _joinPath(dst, base, part);
}

bool FileUtil::isPathContainsFile(const String32& path, const String32& file, uint cs)
{
  sysuint_t index = path.lastIndexOf(Char32('/'));
  sysuint_t length = path.getLength() - index;

  return (length == file.getLength() &&
    StringUtil::eq(path.cData() + index, file.cData(), length, cs));
}

bool FileUtil::isPathContainsDirectory(const String32& path, const String32& directory, uint cs)
{
  const Char32* d_str = directory.cData();
  sysuint_t d_length = directory.getLength();

  if (d_length == 0) return false;
  if (isDirSeparator(d_str[d_length-1])) d_length--;

  return (path.getLength() > d_length &&
    isDirSeparator(path.at(d_length)) &&
    path.startsWith(Utf32(d_str, d_length), cs));
}

bool FileUtil::isPathContainsExtension(const String32& path, const String32& extension, uint cs)
{
  sysuint_t pathLength = path.getLength();
  if (!pathLength) return false;

  // Speed is important, so RAW manipulation is needed
  const Char32* pathBegin = path.cData();
  const Char32* pathEnd = pathBegin + pathLength;
  const Char32* pathCur = pathEnd;

  do {
    if (FOG_UNLIKELY(*--pathCur == Char32('.')))
    {
      pathCur++;
      return ((sysuint_t)(pathEnd - pathCur) == extension.getLength() &&
        StringUtil::eq(pathCur, extension.cData(), extension.getLength(), cs));
    }
    else if (FOG_UNLIKELY(isDirSeparator(*pathCur)))
    {
      return false;
    }
  } while (pathCur != pathBegin);
  return false;
}

bool FileUtil::isNormalizedPath(const String32& path)
{
  if (!FileUtil::isAbsolutePath(path)) return false;

#if defined(FOG_OS_WINDOWS)
  const Char32* pathBegin = path.cData();
  const Char32* pathCur = pathBegin;
  const Char32* pathEnd = pathBegin + path.getLength();

  // normalize X:\ form
  if (pathCur[2] == Char32('\\')) return false;

  pathCur += 3;
  while (pathCur < pathEnd)
  {
    // All "\" will be replaced to "/"
    if (pathCur[0] == Char32('\\')) return false;

    if (pathCur[0] == Char32('/'))
    {
      // test for "//"
      if (pathCur[-1] == Char32('/')) return false;
      if (pathCur[-1] == Char32('.'))
      {
        // NOTE: Here is no problem with reading out of
        // buffer range, because buffer often starts with
        // "\" if we are in this loop.

        // test for "/./"
        if (pathCur[-2] == Char32('/')) return false;
        // test for "/../"
        if (pathCur[-2] == Char32('.') && pathCur[-3] == Char32('/')) return false;
      }
    }
    pathCur++;
  }
  return true;
#else
  const Char32* pathBegin = path.cData();
  const Char32* pathCur = pathBegin;
  const Char32* pathEnd = pathBegin + path.getLength();

  if (*pathCur++ == Char32('/'))
  {
    while (pathCur != pathEnd)
    {

      if (pathCur[0] == Char32('/'))
      {
        // test for "//"
        if (pathCur[-1] == Char32('/')) return false;
        if (pathCur[-1] == Char32('.'))
        {
          // NOTE: Here is no problem reading out of buffer
          // range, because buffer often starts with "/" if
          // we are in this loop.

          // test for "/./"
          if (pathCur[-2] == Char32('/')) return false;
          // test for "/../"
          if (pathCur[-2] == Char32('.') && pathCur[-3] == Char32('/')) return false;
        }
      }
      pathCur++;
    }
    return true;
  }
  else
    return false;
#endif
}

bool FileUtil::isAbsolutePath(const String32& path)
{
#if defined(FOG_OS_WINDOWS)
  // We can accept that "[A-Za-z]:/" as absolute path
  if (path.getLength() > 2)
  {
    const Char32* pathStr = path.cData();
    return (pathStr[0].isAsciiAlpha() &&
      pathStr[1] == Char32(':') &&
      isDirSeparator(pathStr[2]));
  }
  else
    return false;
#else
  return (path.getLength() != 0 && path.at(0) == Char32('/'));
#endif
}

bool FileUtil::testLocalName(const String32& path)
{
#if defined(FOG_OS_WINDOWS)
  return true;
#else
  if (TextCodec::local8().isUnicode()) return true;

  TemporaryString8<TemporaryLength> path8;
  return (path8.set(path, TextCodec::local8()) == Error::Ok);
#endif
}

} // Fog namespace
