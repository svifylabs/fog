// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_UI_THEME_H
#define _FOG_UI_THEME_H

// [Dependencies]
#include <Fog/Core/Object.h>

//! @addtogroup Fog_UI
//! @{

namespace Fog {

// ============================================================================
// [Fog::Theme]
// ============================================================================

struct FOG_API Theme : public Object
{
  FOG_DECLARE_OBJECT(Theme, Object)

  // [Construction / Destruction]

  Theme();
  virtual ~Theme();

  // [Properties]

  // [Rendering]

  // [Statics]

  static Theme* _instance;

  static FOG_INLINE Theme* instance() { return _instance; }
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_UI_THEME_H
