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

err_t getModuleFileName(HMODULE hModule, String& dst)
{
  dst.prepare(256);
  for (;;)
  {
    DWORD capacity = (DWORD)t.getCapacity();
    DWORD result = GetModuleFileNameW(hModule, reinterpret_cast<wchar_t*>(dst.xData()), capacity + 1);

    if (result == 0) return ::GetLastError();

    if (result <= capacity)
    {
      dst.resize(result);
      return Error::Ok;
    }
    else
    {
      dst.reserve(dst.getCapacity() << 1);
    }
  }
}

} // WinUtil namespace
} // Fog namespace

#endif // FOG_OS_WINDOWS
