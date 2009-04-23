// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_USERINFO_H
#define _FOG_CORE_USERINFO_H

// [Dependencies]
#include <Fog/Core/String.h>

namespace Fog {

//! @addtogroup Fog_Core
//! @{

// [Fog::UserInfo]

//! @brief Provides informations about user.
struct FOG_API UserInfo
{
  enum Directories
  {
    Home,
    Desktop,
    Documents,
    Music,
    Pictures,
    Videos
  };

  static uint32_t uid();
  static uint32_t gid();

  static String32 directory(uint32_t dir);
  static bool directoryTo(uint32_t dir, String32& to);
};

//! @}

} // Fog namespace

// [Guard]
#endif // _FOG_CORE_USERINFO_H
