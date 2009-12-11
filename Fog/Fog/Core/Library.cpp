// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Assert.h>
#include <Fog/Core/AutoLock.h>
#include <Fog/Core/ByteArray.h>
#include <Fog/Core/Constants.h>
#include <Fog/Core/FileSystem.h>
#include <Fog/Core/Library.h>
#include <Fog/Core/List.h>
#include <Fog/Core/Lock.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/String.h>
#include <Fog/Core/TextCodec.h>

#if defined(FOG_LIBRARY_WINDOWS)
#include <windows.h>
#endif // FOG_LIBRARY_WINDOWS

#if defined(FOG_LIBRARY_DL)
#include <dlfcn.h>
#include <errno.h>
#ifndef RTLD_NOW
#define RTLD_NOW 0
#endif
#endif // FOG_LIBRARY_DL

namespace Fog {

// ============================================================================
// [Fog::Library - Local]
// ============================================================================

struct Library_Local
{
  Lock lock;
  List<String> paths;
};

static Static<Library_Local> library_local;

// ============================================================================
// [Fog::Library - Platform support]
// ============================================================================

#if defined(FOG_LIBRARY_WINDOWS)
static err_t platformOpenLibrary(const String& fileName, void** handle)
{
  TemporaryString<TEMP_LENGTH> fileNameW;
  err_t err;

  if ((err = fileNameW.set(fileName)) ||
      (err = fileNameW.slashesToWin()))
  {
    return err;
  }

  HMODULE hLibrary = ::LoadLibraryW(reinterpret_cast<const wchar_t*>(fileNameW.getData()));
  if (!hLibrary) return GetLastError();

  *handle = (void*)hLibrary;
  return ERR_OK;
}

static void platformCloseLibrary(void* handle)
{
  ::FreeLibrary((HMODULE)handle);
}

static void* platformLoadSymbol(void* handle, const char* symbol)
{
  if (handle == NULL) return NULL;
  return (void*)GetProcAddress((HINSTANCE)handle, symbol);
}
#endif // FOG_LIBRARY_WINDOWS

#if defined(FOG_LIBRARY_DL)
static err_t platformOpenLibrary(const String& fileName, void** handle)
{
  err_t err;
  TemporaryByteArray<TEMP_LENGTH> fileName8;
  if ((err = TextCodec::local8().appendFromUnicode(fileName8, fileName))) return err;

  void* h = (void*)::dlopen(fileName8.getData(), RTLD_NOW);
  if (h == NULL) return ERR_LIB_LOAD_FAILED;

  *handle = h;
  return ERR_OK;
}

static void platformCloseLibrary(void* handle)
{
  ::dlclose(handle);
}

static void* platformLoadSymbol(void* handle, const char* symbol)
{
  if (handle == NULL) return NULL;
  return (void *)::dlsym(handle, symbol);
}
#endif // FOG_LIBRARY_DL

// ============================================================================
// [Fog::Library]
// ============================================================================

const char* Library::systemPrefix;
const char* Library::systemSuffix;
const char* Library::systemExtension;

Library::Library() :
  _d(sharedNull->ref())
{
}

Library::Library(const Library& other) :
  _d(other._d->ref())
{
}

Library::Library(const String& fileName, uint32_t openFlags) :
  _d(sharedNull->ref())
{
  open(fileName, openFlags);
}

Library::~Library()
{
  _d->deref();
}

err_t Library::open(const String& _fileName, uint32_t openFlags)
{
  err_t err;
  TemporaryString<TEMP_LENGTH> fileName;
  
  if ((err = fileName.set(_fileName))) return err;

  if ((openFlags & LIBRARY_OPEN_SYSTEM_PREFIX) != 0)
  {
    if ((err = fileName.insert(fileName.lastIndexOf(Char('/')) + 1, Ascii8(systemPrefix)))) return err;
  }
  if ((openFlags & LIBRARY_OPEN_SYSTEM_SUFFIX) != 0)
  {
    if ((err = fileName.append(Ascii8(systemSuffix)))) return err;
  }

  void* handle;
  if ((err = platformOpenLibrary(fileName, &handle))) return err;

  Data* newd = Data::alloc();
  if (!newd)
  {
    platformCloseLibrary(handle);
    return ERR_RT_OUT_OF_MEMORY;
  }
  newd->handle = handle;

  AtomicBase::ptr_setXchg(&_d, newd)->deref();
  return ERR_OK;
}

err_t Library::openPlugin(const String& category, const String& fileName)
{
  err_t err;
  TemporaryString<TEMP_LENGTH> relative;
  TemporaryString<TEMP_LENGTH> absolute;

  if ( (err = relative.append(Ascii8(systemPrefix))) ) return err;
  if (!category.isEmpty())
  {
    if ( (err = relative.append(category)) ) return err;
    if ( (err = relative.append(Char('_'))) ) return err;
  }
  if ( (err = relative.append(fileName)) ) return err;
  if ( (err = relative.append(Ascii8(systemSuffix))) ) return err;

  if (FileSystem::findFile(paths(), relative, absolute))
    return open(absolute, 0);
  else
    return ERR_IO_FILE_NOT_EXISTS;
}

void Library::close()
{
  AtomicBase::ptr_setXchg(&_d, sharedNull->ref())->deref();
}

void* Library::getSymbol(const char* symbolName)
{
  return platformLoadSymbol(_d->handle, symbolName);
}

void* Library::getSymbol(const String& symbolName)
{
  TemporaryByteArray<TEMP_LENGTH> symb8;
  TextCodec::utf8().fromUnicode(symb8, symbolName);
  return platformLoadSymbol(_d->handle, symb8.getData());
}

sysuint_t Library::getSymbols(void** target, const char* symbols, sysuint_t symbolsLength, sysuint_t symbolsCount, char** fail)
{
  sysuint_t i;

  void* address;
  void* handle = _d->handle;
  const char* current;
  const char* symbolsCur = symbols;
  const char* symbolsEnd = symbols + symbolsLength;

  if (fail) *fail = NULL;

  for (i = 0; i != symbolsCount; i++)
  {
    current = symbolsCur;
    for (;;)
    {
      if (symbolsCur == symbolsEnd) goto end;
      if (*symbolsCur++ == 0) break;
    }

    if ((address = platformLoadSymbol(handle, current)) == NULL)
    {
      if (fail) *fail = (char*)current;
      goto end;
    }

    target[i] = address;
  }

end:
  return i;
}

// [Operator Overload]

Library& Library::operator=(const Library& other)
{
  AtomicBase::ptr_setXchg(&_d, other._d->ref())->deref();
  return *this;
}

// [Paths]

List<String> Library::paths()
{
  AutoLock locked(library_local.instance().lock);
  return library_local.instance().paths;
}

bool Library::addPath(const String& path, int pathMode)
{
  AutoLock locked(library_local.instance().lock);
  if (path.isEmpty()) return false;
  if (library_local.instance().paths.indexOf(path) != INVALID_INDEX) return false;

  if (pathMode == PATH_APPEND)
    library_local.instance().paths.append(path);
  else
    library_local.instance().paths.prepend(path);
  return true;
}

bool Library::removePath(const String& path)
{
  AutoLock locked(library_local.instance().lock);
  return library_local.instance().paths.remove(path) != 0;
}

bool Library::hasPath(const String& path)
{
  AutoLock locked(library_local.instance().lock);
  return library_local.instance().paths.indexOf(path) != INVALID_INDEX;
}

// ============================================================================
// [Fog::Library::Data]
// ============================================================================

Static<Library::Data> Library::sharedNull;

void Library::Data::deref()
{
  if (refCount.deref())
  {
    if (handle) platformCloseLibrary(handle);
    Memory::free(this);
  }
}

Library::Data* Library::Data::alloc()
{
  Data* d = (Data*)Memory::alloc(sizeof(Data));
  if (!d) return NULL;

  d->refCount.init(1);
  d->handle = NULL;

  return d;
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_library_init(void)
{
  using namespace Fog;

  Library::sharedNull.init();
  Library::Data* d = Library::sharedNull.instancep();
  d->refCount.init(1);
  d->handle = NULL;

  library_local.init();

#if defined(FOG_OS_WINDOWS)
  static const char libraryPrefix[] = "";
  static const char librarySuffix[] = ".dll";
#endif // FOG_OS_WINDOWS
#if defined(FOG_OS_POSIX)
# if defined(FOG_OS_HPUX)
  static const char libraryPrefix[] = "lib";
  static const char librarySuffix[] = ".sl";
# elif defined(FOG_OS_MAC)
  static const char libraryPrefix[] = "lib";
  static const char librarySuffix[] = ".bundle";
# else
  static const char libraryPrefix[] = "lib";
  static const char librarySuffix[] = ".so";
# endif
#endif // FOG_OS_POSIX

  Library::systemPrefix = libraryPrefix;
  Library::systemSuffix = librarySuffix;
  Library::systemExtension = librarySuffix + 1;

#if defined(FOG_OS_POSIX)
#if FOG_ARCH_BITS == 32
  library_local.instance().paths.append(Ascii8("/usr/lib"));
  library_local.instance().paths.append(Ascii8("/usr/local/lib"));
#else
  library_local.instance().paths.append(Ascii8("/usr/lib64"));
  library_local.instance().paths.append(Ascii8("/usr/local/lib64"));
#endif // FOG_ARCH_BITS
#endif // FOG_OS_POSIX

  return ERR_OK;
}

FOG_INIT_DECLARE void fog_library_shutdown(void)
{
  using namespace Fog;

  library_local.destroy();

  Library::sharedNull.instancep()->refCount.dec();
  Library::sharedNull.destroy();
}
