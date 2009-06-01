// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_UI_THEME_FOG_H
#define _FOG_UI_THEME_FOG_H

// [Dependencies]
#include <Fog/UI/Theme.h>

namespace Fog {

// ============================================================================
// [Fog::Button]
// ============================================================================

struct FOG_API Theme_Fog : public Theme
{
  FOG_DECLARE_OBJECT(Theme_Fog, Theme)

  // [Construction / Destruction]

  Theme_Fog();
  virtual ~Theme_Fog();

  // [Properties]

  // [Rendering]
};

} // Fog namespace

// [Guard]
#endif // _FOG_UI_THEME_FOG_H
