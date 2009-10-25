// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_OS_H
#define _FOG_CORE_OS_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Core/String.h>

//! @addtogroup Fog_Core
//! @{

namespace Fog {

// ============================================================================
// [Fog::OS]
// ============================================================================

//! @brief Contains useful functions for getting system information.
namespace OS {

//! @brief Returns the name of the host operating system.
FOG_API String getName();

//! @brief Returns the version of the host operating system.
FOG_API String getVersion();

//! @brief Return the number of bytes of physical memory on the current machine.
FOG_API uint64_t getAmountOfPhysicalMemory();

//! @brief Return the number of megabytes of physical memory on the current machine.
FOG_API uint32_t getAmountOfPhysicalMemoryMB();

#if defined(FOG_OS_WINDOWS)
enum Win_Version
{
  Win_Unknown    = 0,

  //! @brief Set if system is WinNT based (can't be together with @c Win_98).
  Win_NT         = 0x0001,
  //! @brief Set if system is WinNT 4.x.
  Win_NT_3       = 0x0002,
  //! @brief Set if system is WinNT 4.x.
  Win_NT_4       = 0x0004,
  //! @brief Set if system is Windows 2000.
  Win_2000       = 0x0008,
  //! @brief Set if system is Windows XP.
  Win_XP         = 0x0010,
  //! @brief Set if system is Windows 2003.
  Win_2003       = 0x0020,
  //! @brief Set if system is Windows Vista.
  Win_Vista      = 0x0040
};

FOG_API uint32_t getWindowsVersion();
FOG_API String getWindowsDirectory();
#endif // FOG_OS_WINDOWS

FOG_API err_t getEnv(const String& name, String& value);
FOG_API err_t setEnv(const String& name, const String& value);

} // OS namespace
} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_CORE_OS_H
