// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_FILEUTIL_H
#define _FOG_CORE_FILEUTIL_H

// [Dependencies]
#include <Fog/Core/String.h>

namespace Fog {
namespace FileUtil {

// ============================================================================
// [Fog::FileUtil]
// ============================================================================

FOG_API err_t extractFile(String32& dst, const String32& path);
FOG_API err_t extractDirectory(String32& dst, const String32& path);
FOG_API err_t extractExtension(String32& dst, const String32& path);
FOG_API err_t normalizePath(String32& dst, const String32& path);
FOG_API err_t toAbsolutePath(String32& dst, const String32& base, const String32& path);
FOG_API err_t joinPath(String32& dst, const String32& base, const String32& part);

FOG_API bool isPathContainsFile(const String32& path, const String32& file, uint cs = CaseSensitive);
FOG_API bool isPathContainsDirectory(const String32& path, const String32& directory, uint cs = CaseSensitive);
FOG_API bool isPathContainsExtension(const String32& path, const String32& extension, uint cs = CaseSensitive);
FOG_API bool isNormalizedPath(const String32& path);
FOG_API bool isAbsolutePath(const String32& path);

FOG_API bool testLocalName(const String32& path);

#if defined(FOG_OS_WINDOWS)
static const Char8  directorySeparator8  = Char8('\\');
static const Char16 directorySeparator16 = Char16('\\');
static const Char32 directorySeparator32 = Char32('\\');

static const Char8  pathSeparator8  = Char8(';');
static const Char16 pathSeparator16 = Char16(';');
static const Char32 pathSeparator32 = Char32(';');
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
static const Char8  directorySeparator8  = Char8('/');
static const Char16 directorySeparator16 = Char16('/');
static const Char32 directorySeparator32 = Char32('/');

static const Char8  pathSeparator8  = Char8(':');
static const Char16 pathSeparator16 = Char16(':');
static const Char32 pathSeparator32 = Char32(':');
#endif // FOG_OS_POSIX

} // FileUtil namespace
} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_CORE_FILEUTIL_H
