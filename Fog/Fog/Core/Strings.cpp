// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Error.h>
#include <Fog/Core/StringCache.h>
#include <Fog/Core/Strings.h>

namespace Fog {

// ============================================================================
// [Fog::fog_strings]
// ============================================================================

static const char fog_strings_data[] =
{
  // [Core - Object]

  "id\0"
  "name\0"

  // [Core - Properties]

  // reserved...

  // [Graphics - Image File Types]

  "ANI\0"
  "APNG\0"
  "BMP\0"
  "FLI\0"
  "FLC\0"
  "GIF\0"
  "ICO\0"
  "JPEG\0"
  "LBM\0"
  "MNG\0"
  "PCX\0"
  "PNG\0"
  "PNM\0"
  "TGA\0"
  "TIFF\0"
  "XBM\0"
  "XPM\0"

  // [Graphics - Image File Extensions]

  "ani\0"
  "apng\0"
  "bmp\0"
  "fli\0"
  "flc\0"
  "gif\0"
  "ico\0"
  "jfi\0"
  "jfif\0"
  "jpg\0"
  "jpeg\0"
  "lbm\0"
  "mng\0"
  "pcx\0"
  "png\0"
  "pnm\0"
  "ras\0"
  "tga\0"
  "tif\0"
  "tiff\0"
  "xbm\0"
  "xpm\0"

  // [Graphics - ImageIO Properties]

  "width\0"
  "height\0"
  "depth\0"
  "planes\0"
  "actualFrame\0"
  "framesCount\0"
  "progress\0"
  "quality\0"

  // [Graphics - ImageFilter Properties]

  "filterType\0"
  "blurType\0"
  "borderMode\0"
  "borderColor\0"
  "horizontalRadius\0"
  "verticalRadius\0"
  "kernel\0"

  // [Xml]

  "unnamed\0"
  "id\0"
  "style\0"

  "#text\0"
  "#cdata\0"
  "#pi\0"
  "#comment\0"
  "#document\0"

  // [Svg]

  "circle\0"
  "ellipse\0"
  "g\0"
  "line\0"
  "polygon\0"
  "polyline\0"
  "rect\0"
  "path\0"

  "transform\0"

  "none\0"
  "clip-path\0"
  "clip-rule\0"
  "enable-background\0"
  "fill\0"
  "fill-opacity\0"
  "fill-rule\0"
  "filter\0"
  "font-family\0"
  "font-size\0"
  "letter-spacing\0"
  "mask\0"
  "opacity\0"
  "stop-color\0"
  "stop-opacity\0"
  "stroke\0"
  "stroke-dasharray\0"
  "stroke-linecap\0"
  "stroke-linejoin\0"
  "stroke-miterlimit\0"
  "stroke-opacity\0"
  "stroke-width\0"
};

FOG_CVAR_DECLARE Fog::StringCache* fog_strings;

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_strings_init(void)
{
  using namespace Fog;

  fog_strings = StringCache::create(
    fog_strings_data,
    FOG_ARRAY_SIZE(fog_strings_data),
    STR_COUNT,
    Ascii8("Fog")
  );

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_strings_shutdown(void)
{
  using namespace Fog;

  StringCache::destroy(fog_strings);
}
