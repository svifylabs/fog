// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_LIBRARY_H
#define _FOG_CORE_LIBRARY_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Core/Constants.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/String.h>
#include <Fog/Core/TypeInfo.h>

//! @addtogroup Fog_Core
//! @{

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

template<typename T> struct Vector;

// ============================================================================
// [Fog::Library]
// ============================================================================

struct FOG_API Library
{
  // [Data]

  struct FOG_API Data
  {
    // [Ref / Deref]

    Data* ref() const;
    void deref();

    Data* refAlways() const
    {
      refCount.inc();
      return const_cast<Data*>(this);
    }

    static Data* alloc();

    // [Members]

    mutable Atomic<sysuint_t> refCount;
    void* handle;
  };

  static Static<Data> sharedNull;

  // [Open Mode]

  //! @brief Library open flags used in @c open().
  enum OpenMode
  {
    //! @brief Open library with system prefix (default @c true).
    //!
    //! System prefix is mainly for systems like unix, where library prefix
    //! is usually 'lib', but plugins are usually linked without this prefix,
    //! but default is to use system prefix.
    OpenSystemPrefix = (1 << 0),

    //! @brief Open libray with system suffix (default @c true).
    //!
    //! System suffix = dot + library extension, for example in MS Windows
    //! this suffix is equal to '.dll', on unix like machines this suffix
    //! is '.so'. If you need to specify your own suffix, don't set this
    //! flag, because if you not use open flags argument. This flag is
    //! default.
    OpenSystemSuffix = (1 << 1),

    //! @brief Default open flags for Library constructors and @c open() methods.
    //!
    //! Default is to use @c OpenSystemPrefix with @c OpenSystemSuffix
    OpenDefault = OpenSystemPrefix | OpenSystemSuffix
  };

  // [Construction / Destruction]

  Library();
  Library(const Library& other);
  Library(const String32& fileName, uint32_t openFlags = OpenDefault);
  ~Library();

  // [Implicit Sharing]

  //! @copydoc Doxygen::Implicit::refCount().
  FOG_INLINE sysuint_t refCount() const { return _d->refCount.get(); }

  // [Flags]

  //! @copydoc Doxygen::Implicit::isNull().
  FOG_INLINE bool isNull() const { return _d->handle == NULL; }

  FOG_INLINE void* getHandle() const { return _d->handle; }
  FOG_INLINE bool isOpen() const { return _d->handle != NULL; }

  err_t open(const String32& fileName, uint32_t openFlags = OpenDefault);
  err_t openPlugin(const String32& category, const String32& fileName);
  void close();

  // Two functions for symbol name, because symbols are encoded in
  // 8 bit ASCII null terminated strings and we can avoid to use
  // text codecs from Fog::String32 <=> ASCII string. Only reason is
  // performance, nothing more.

  //! @brief Returns a symbol @a symbolName from opened library. 
  //!
  //! Return value can be @c NULL if symbol is not found or library is
  //! not open.
  void* getSymbol(const char* symbolName);
  //! @overload
  void* getSymbol(const String32& symbolName);

  //! @brief Loads more symbols at the time.
  //!
  //! @param target Target array where to store pointers to functions.
  //! @param symbols Null terminated string that contains all symbols
  //! separated by @c NULL terminator, so valid string declaration for
  //! two symbols is <code>"FirstSymbol\000SecondSymbol"</code>.
  //! @param symbolsLength Length of @c symbols string. use sizeof() if
  //! your string is static ( or @c FOG_ARRAY_SIZE() )
  //! @param symbolsCount Count of symbols to load.
  //! @param fail here is stored first symbol that's not loaded.
  //!
  //! @return Count of loaded symbols, if return value is smaller than
  //! @a count parameter, @a fail parameter will be position to begin of
  //! symbol name that wasn't loaded.
  sysuint_t getSymbols(void** target, const char* symbols, sysuint_t symbolsLength, sysuint_t symbolsCount, char** fail);

  // [Operator Overload]

  Library& operator=(const Library& other);

  // [Statics]

  static const Char8* systemPrefix;
  static const Char8* systemSuffix;
  static const Char8* systemExtension;

  // [Paths]

  enum AddPathMode
  {
    PathAppend = 0,
    PathPrepend
  };

  static Vector<String32> paths();
  static bool addPath(const String32& path, int mode = PathAppend);
  static bool removePath(const String32& path);
  static bool hasPath(const String32& path);

  // [Members]

  FOG_DECLARE_D(Data)
};

// ============================================================================
// [FOG_LIBRARY_DECLARE Macros - For Implementing Plugins]
// ============================================================================

//! @def FOG_LIBRARY_DECLARE(library)
//! @brief Creates basic defines for dynamic loadable module.
//!
//! This macro should be placed only once per loadable object. Use it with
//! @c FOG_LIBRARY_ENTRY and @c FOG_LIBRARY_LEAVE macros together.
#if defined(FOG_LIBRARY_WINDOWS)
#define FOG_LIBRARY_DECLARE(library) \
\
static void library##Entry(void); \
static void library##Leave(void); \
\
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) \
{ \
  switch(fdwReason) { \
    case DLL_PROCESS_ATTACH: \
      library##Entry(); \
      break; \
    case DLL_PROCESS_DETACH: \
      library##Leave(); \
      break; \
  } \
  \
  return TRUE; \
}

#define FOG_LIBRARY_ENTRY(library) static void library##Entry(void)
#define FOG_LIBRARY_LEAVE(library) static void library##Leave(void)
#else
#define FOG_LIBRARY_DECLARE(library) \
\
static void library##Entry(void); \
static void library##Leave(void); \
\
struct FOG_HIDDEN library##Interface \
{ \
  library##Interface::library##Interface() { library##Entry(); } \
  library##Interface::~library##Interface() { library##Leave(); } \
}; \
static library##Interface library##_interface;

#define FOG_LIBRARY_ENTRY(library) static void library##Entry(void)
#define FOG_LIBRARY_LEAVE(library) static void library##Leave(void)
#endif

} // Fog namespace

//! @}

// ============================================================================
// [Fog::TypeInfo<>]
// ============================================================================

FOG_DECLARE_TYPEINFO(Fog::Library, Fog::MoveableType)

// [Guard]
#endif // _FOG_CORE_LIBRARY_H
