// [Fog-Graphics Library - Public API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_FONTMANAGER_H
#define _FOG_GRAPHICS_FONTMANAGER_H

// [Dependencies]
#include <Fog/Core/List.h>
#include <Fog/Core/String.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Geometry.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct FontFaceCache;
struct FontEngine;

// ============================================================================
// [Fog::FontManager]
// ============================================================================

//! @brief Static class that can be used to manage font paths, aliases and font
//! resolving.
struct FOG_API FontManager
{
  // [Font path management functions]

  static bool addFontPath(const String& path);
  static void addFontPaths(const List<String>& paths);
  static bool removeFontPath(const String& path);
  static bool hasFontPath(const String& path);
  static bool findFontFile(const String& fileName, String& dest);
  static List<String> getPathList();

  // [Font list management]

  static List<String> getFontList();

  // [Face management]

  static FontFace* getFace(const String& family, uint32_t size, const FontCaps& caps);
  static err_t putFace(FontFace* face);

  // [Engine]

  static FontEngine* _engine;
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_FONTMANAGER_H
