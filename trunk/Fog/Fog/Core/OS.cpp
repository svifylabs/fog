// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Core/ByteArray.h>
#include <Fog/Core/Error.h>
#include <Fog/Core/OS.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/String.h>
#include <Fog/Core/TextCodec.h>

#if defined(FOG_OS_POSIX)
#include <errno.h>
#include <string.h>
#include <sys/statvfs.h>
#include <sys/utsname.h>
#include <unistd.h>
#endif // FOG_OS_POSIX

#if defined(FOG_OS_MACOSX)
#include <mach/mach_host.h>
#include <mach/mach_init.h>
#endif // FOG_OS_MACOSX

// [Fog::System - Windows stuff]
#if defined(FOG_OS_WINDOWS) && !defined(PATH_MAX)
#define PATH_MAX 1024
#endif // FOG_OS_WINDOWS && !defined(PATH_MAX)

namespace Fog {

// ============================================================================
// [Fog::OS]
// ============================================================================

struct OS_Local
{
#if defined(FOG_OS_WINDOWS)
  uint32_t windowsVersion;
  String windowsName;
  String windowsVersionString;
  String windowsDirectory;
#endif // FOG_OS_WINDOWS

  OS_Local()
  {
    // [Windows Specific]
#if defined(FOG_OS_WINDOWS)

    // [Get Windows Version]

    windowsName.set(Ascii8("Windows"));
    windowsVersion = 0;

    OSVERSIONINFOW osVersion;
    ZeroMemory(&osVersion, sizeof(OSVERSIONINFOW));
    osVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
    GetVersionExW(&osVersion);

    switch (osVersion.dwPlatformId)
    {
      case VER_PLATFORM_WIN32_NT:
        windowsName.set(Ascii8("WinNT"));
        windowsVersion |= OS::Win_NT;

        // Windows NT 3.0
        if (osVersion.dwMajorVersion == 3)
        {
          windowsVersion |= OS::Win_NT_3;
        }
        // Windows NT 4.0
        else if (osVersion.dwMajorVersion == 4)
        {
          windowsVersion |= OS::Win_NT_4;
        }
        // Windows 2000, XP or 2003
        else if (osVersion.dwMajorVersion == 5)
        {
          if (osVersion.dwMinorVersion == 0)
          {
            windowsName.set(Ascii8("Win2000"));
            windowsVersion |= OS::Win_2000;
          }
          else if (osVersion.dwMinorVersion == 1)
          {
            windowsName.set(Ascii8("WinXP"));
            windowsVersion |= OS::Win_XP;
          }
          else if (osVersion.dwMinorVersion == 2)
          {
            windowsName.set(Ascii8("Win2003"));
            windowsVersion |= OS::Win_2003;
          }
        }
        // Windows Vista
        else if (osVersion.dwMajorVersion == 6)
        {
          windowsName.set(Ascii8("WinVista"));
          windowsVersion |= OS::Win_Vista;
        }
        break;
    }

    windowsVersionString.format("%u.%u",
      (uint32_t)osVersion.dwMajorVersion,
      (uint32_t)osVersion.dwMinorVersion);

    // [Get Windows Directory]

    WCHAR winDir[PATH_MAX];
    UINT result = GetWindowsDirectoryW(winDir, PATH_MAX);

    if (!result || result > PATH_MAX)
    {
      windowsDirectory.set(Ascii8("C:/WINDOWS"));
    }
    else
    {
      windowsDirectory.set(reinterpret_cast<const Char*>(winDir));
      windowsDirectory.slashesToPosix();
    }
#endif // FOG_OS_WINDOWS
  }

  ~OS_Local()
  {
  }
};

static Static<OS_Local> os_local;

/*! @brief Returns the name of the host operating system. */
String OS::getName()
{
  String result;

#if defined(FOG_OS_WINDOWS)
  result = os_local->windowsName;
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
  utsname info;
  if (uname(&info) >= 0) result.set(Ascii8(info.sysname));
#endif // FOG_OS_POSIX

  return result;
}

/*! @brief Returns the version of the host operating system. */
String OS::getVersion()
{
  String result;

#if defined(FOG_OS_WINDOWS)
  result = os_local->windowsVersionString;
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
  utsname info;
  if (uname(&info) >= 0) TextCodec::local8().appendToUnicode(result, info.release);
#endif // FOG_OS_POSIX

  return result;
}

/*! @brief Return the number of bytes of physical memory on the current machine. */
uint64_t OS::getAmountOfPhysicalMemory()
{
#if defined(FOG_OS_WINDOWS)
  MEMORYSTATUSEX meminfo;

  meminfo.dwLength = sizeof(meminfo);
  if (!GlobalMemoryStatusEx(&meminfo))
  {
    FOG_ASSERT(0);
    return 0;
  }

  return static_cast<uint64_t>(meminfo.ullTotalPhys);
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
// _SC_PHYS_PAGES is not part of POSIX and not available on OS X
#if defined(FOG_OS_MACOSX)
  struct host_basic_info hostinfo;
  mach_msg_type_number_t count = HOST_BASIC_INFO_COUNT;
  int result = host_info(mach_host_self(),
                         HOST_BASIC_INFO,
                         reinterpret_cast<host_info_t>(&hostinfo),
                         &count);

  FOG_ASSERT(count == HOST_BASIC_INFO_COUNT);

  if (result != KERN_SUCCESS)
  {
    FOG_ASSERT(0);
    return 0;
  }
  
  return static_cast<uint64_t>(hostinfo.max_mem);
#else
  long pages = sysconf(_SC_PHYS_PAGES);
  long page_size = sysconf(_SC_PAGE_SIZE);

  if (pages == -1 || page_size == -1)
  {
    FOG_ASSERT(0);
    return 0;
  }

  return ((uint64_t)pages * (uint64_t)page_size);
#endif
#endif // FOG_OS_POSIX
}

/*! @brief Return the number of megabytes of physical memory on the current machine. */
uint32_t OS::getAmountOfPhysicalMemoryMB()
{
  return static_cast<uint32_t>(getAmountOfPhysicalMemory() / 1048576);
}

#if defined (FOG_OS_WINDOWS)
uint32_t OS::getWindowsVersion()
{
  return os_local->windowsVersion;
}

String OS::getWindowsDirectory()
{
  return os_local->windowsDirectory;
}
#endif // FOG_OS_WINDOWS

err_t OS::getEnv(const String& name, String& value)
{
#if defined(FOG_OS_WINDOWS)
  DWORD sz;

  err_t err;
  if ((err = value.prepare(256))) return err;

  for (;;)
  {
    sz = GetEnvironmentVariableW(reinterpret_cast<const wchar_t*>(name.cData()),
      reinterpret_cast<wchar_t*>(value.xData()), value.getCapacity());

    if (sz == 0)
    {
      return Error::GetEnvFailure;
    }
    else if (sz > value.getCapacity())
    {
      if ((err = value.prepare(sz)) == Error::Ok) continue;
      return err;
    }
    else if (sz)
    {
      return value.resize(sz);
    }
  }
#endif // FOG_OS_WINDOWS

#if defined(FOG_OS_POSIX)
  TemporaryByteArray<TemporaryLength> name8;

  err_t err;
  if ((err = TextCodec::local8().appendFromUnicode(name8, name))) return err;

  const char* e = getenv(name8.cData());
  if (e) 
  {
    return TextCodec::local8().toUnicode(value, e);
  }
  else
  {
    value.clear();
    return Error::GetEnvFailure;
  }
#endif
}

err_t OS::setEnv(const String& name, const String& value)
{
#if defined(FOG_OS_WINDOWS)
  if (SetEnvironmentVariableW(reinterpret_cast<const wchar_t*>(name.cData()), 
    reinterpret_cast<const wchar_t*>(value.cData())))
  {
    return Error::Ok;
  }
  else
  {
    return Error::SetEnvFailure;
  }
#endif

#if defined(FOG_OS_POSIX)
  TemporaryByteArray<TemporaryLength> name8;
  TemporaryByteArray<TemporaryLength> value8;

  err_t err;
  int result;

  if ((err = TextCodec::local8().appendFromUnicode(name8, name))) return err;
  if ((err = TextCodec::local8().appendFromUnicode(value8, value))) return err;

  if (value8.isEmpty())
    result = unsetenv(name8.cData());
  else
    result = setenv(name8.cData(), value8.cData(), 1);

  if (result != 0) err = Error::SetEnvFailure;
  return err;
#endif
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_os_init(void)
{
  using namespace Fog;

  os_local.init();
  return Error::Ok;
}

FOG_INIT_DECLARE void fog_os_shutdown(void)
{
  using namespace Fog;

  os_local.destroy();
}
