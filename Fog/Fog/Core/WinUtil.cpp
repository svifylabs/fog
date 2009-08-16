// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Dependencies]
#include <Fog/Build/Build.h>

#if defined(FOG_OS_WINDOWS)

#include <Fog/Core/String.h>
#include <Fog/Core/WinUtil.h>

namespace Fog {
namespace WinUtil {

err_t getModuleFileName(HMODULE hModule, String32& dst)
{
  TemporaryString16<1024> t;

  for (;;)
  {
    DWORD capacity = (DWORD)t.getCapacity();
    DWORD result = GetModuleFileNameW(hModule, (WCHAR*)t.mData(), capacity + 1);

    if (result == 0) return ::GetLastError();

    if (result <= capacity)
    {
      t.resize(result);
      return dst.set(t);
    }
    else
    {
      t.reserve(t.getCapacity() << 1);
    }
  }
}

} // WinUtil namespace
} // Fog namespace

#endif // FOG_OS_WINDOWS
