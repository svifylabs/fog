// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_UI_ERROR_H
#define _FOG_UI_ERROR_H

// [Dependencies]
#include <Fog/Build/Build.h>

// ============================================================================
// [Error]
// ============================================================================

namespace Error {

enum UIError 
{
  // [Errors Range]
  _UIErrorStart = 0x00012000,
  _UIErrorLast  = 0x00012FFF,

  FailedToCreateUISystem = _UIErrorStart,
  FailedToCreateUIWindow,
  FailedToTranslateCoordinates,
  UISystemNotExists,
  UISystemX11_CantLoadX11,
  UISystemX11_CantLoadX11Symbol,
  UISystemX11_CantLoadXext,
  UISystemX11_CantLoadXextSymbol,
  UISystemX11_CantLoadXrender,
  UISystemX11_CantLoadXrenderSymbol,
  UISystemX11_CantOpenDisplay,
  UISystemX11_CantCreatePipe,
  UISystemX11_CantCreateColormap,
  UISystemX11_TextListToTextPropertyFailed,
  UIWindowAlreadyExists
};

} // Error namespace

// [Guard]
#endif // _FOG_UI_ERROR_H
