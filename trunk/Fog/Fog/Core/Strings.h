// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_STRINGS_H
#define _FOG_CORE_STRINGS_H

// [Dependencies]
#include <Fog/Build/Build.h>

//! @addtogroup Fog_Core
//! @{

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct StringCache;

// ============================================================================
// [Fog::fog_strings]
// ============================================================================

//! @brief Cached strings IDs used in @c fog_strings array.
enum STR_ID
{
  // [Core - Object]

  STR_CORE_id = 0,
  STR_CORE_name,

  // [Core - Properties]

  // reserved...

  // [Graphics - Image File Types]

  STR_GRAPHICS_ANI,
  STR_GRAPHICS_APNG,
  STR_GRAPHICS_BMP,
  STR_GRAPHICS_FLI,
  STR_GRAPHICS_FLC,
  STR_GRAPHICS_GIF,
  STR_GRAPHICS_ICO,
  STR_GRAPHICS_JPEG,
  STR_GRAPHICS_LBM,
  STR_GRAPHICS_MNG,
  STR_GRAPHICS_PCX,
  STR_GRAPHICS_PNG,
  STR_GRAPHICS_PNM,
  STR_GRAPHICS_TGA,
  STR_GRAPHICS_TIFF,
  STR_GRAPHICS_XBM,
  STR_GRAPHICS_XPM,

  // [Graphics - Image File Extensions]

  STR_GRAPHICS_ani,
  STR_GRAPHICS_apng,
  STR_GRAPHICS_bmp,
  STR_GRAPHICS_fli,
  STR_GRAPHICS_flc,
  STR_GRAPHICS_gif,
  STR_GRAPHICS_ico,
  STR_GRAPHICS_jfi,
  STR_GRAPHICS_jfif,
  STR_GRAPHICS_jpg,
  STR_GRAPHICS_jpeg,
  STR_GRAPHICS_lbm,
  STR_GRAPHICS_mng,
  STR_GRAPHICS_pcx,
  STR_GRAPHICS_png,
  STR_GRAPHICS_pnm,
  STR_GRAPHICS_ras,
  STR_GRAPHICS_tga,
  STR_GRAPHICS_tif,
  STR_GRAPHICS_tiff,
  STR_GRAPHICS_xbm,
  STR_GRAPHICS_xpm,

  // [Graphics - ImageIO Properties]

  STR_GRAPHICS_width,
  STR_GRAPHICS_height,
  STR_GRAPHICS_depth,
  STR_GRAPHICS_planes,
  STR_GRAPHICS_actualFrame,
  STR_GRAPHICS_framesCount,
  STR_GRAPHICS_progress,
  STR_GRAPHICS_quality,

  // [Graphics - ImageFilter Properties]

  STR_GRAPHICS_filterType,
  STR_GRAPHICS_blurType,
  STR_GRAPHICS_borderMode,
  STR_GRAPHICS_borderColor,
  STR_GRAPHICS_horizontalRadius,
  STR_GRAPHICS_verticalRadius,
  STR_GRAPHICS_kernel,

  // [Count]

  STR_COUNT
};

extern FOG_API StringCache* fog_strings;

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_CORE_STRINGS_H
