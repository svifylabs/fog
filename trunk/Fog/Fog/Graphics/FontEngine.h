// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_FONTENGINE_H
#define _FOG_GRAPHICS_FONTENGINE_H

// [Dependencies]
#include <Fog/Core/Hash.h>
#include <Fog/Core/Lock.h>
#include <Fog/Core/Misc.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/String.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/Graphics/Glyph.h>
#include <Fog/Graphics/GlyphCache.h>
#include <Fog/Graphics/GlyphSet.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

union FontCaps;
struct FontEngine;
struct FontFace;
struct FontMetrics;
struct GlyphSet;
struct Path;
struct TextExtents;

// ============================================================================
// [Fog::FontMetrics]
// ============================================================================

struct FontMetrics
{
  uint32_t size;
  uint32_t ascent;
  uint32_t descent;
  uint32_t averageWidth;
  uint32_t maximumWidth;
  uint32_t height;
};

// ============================================================================
// [Fog::FontCaps]
// ============================================================================

union FontCaps
{
  struct
  {
    uint8_t bold;
    uint8_t italic;
    uint8_t strike;
    uint8_t underline;
  };
  uint32_t value;
};

// ============================================================================
// [Fog::FontFace]
// ============================================================================

//! @brief Font face.
struct FOG_API FontFace
{
  // [Construction / Destruction]

  FontFace();
  virtual ~FontFace();

  // [Ref / Deref]

  virtual FontFace* ref();
  virtual void deref();

  // [Abstract]

  virtual err_t getGlyphSet(const Char* str, sysuint_t length, GlyphSet& glyphSet) = 0;
  virtual err_t getOutline(const Char* str, sysuint_t length, Path& dst) = 0;
  virtual err_t getTextExtents(const Char* str, sysuint_t length, TextExtents& extents) = 0;

  // [Members]

  //! @brief Reference count.
  mutable Atomic<sysuint_t> refCount;

  //! @brief Font family (can be normalized).
  String family;
  //! @brief Font face caps.
  FontCaps caps;
  //! @brief Font face metrics.
  FontMetrics metrics;
  //! @brief Whether face is scalable (font is vector based).
  uint32_t scalable;

private:
  FOG_DISABLE_COPY(FontFace)
};

// ============================================================================
// [Fog::FontEngine]
// ============================================================================

//! @brief Font engine.
struct FOG_API FontEngine
{
  // [Construction / Destruction]

  FontEngine(const String& name);
  virtual ~FontEngine();

  // [Abstract]

  virtual List<String> getFontList() = 0;

  virtual FontFace* createDefaultFace() = 0;

  virtual FontFace* createFace(
    const String& family, uint32_t size,
    const FontCaps& caps) = 0;

  virtual List<String> getDefaultFontDirectories();

  FOG_INLINE const String& getName() const { return _name; }

  // [Members]

protected:
  String _name;

private:
  FOG_DISABLE_COPY(FontEngine)
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_FONTENGINE_H
