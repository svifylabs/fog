// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_FONT_WIN_H
#define _FOG_GRAPHICS_FONT_WIN_H

// [Dependencies]
#include <Fog/Build/Build.h>

#if defined(FOG_FONT_WINDOWS)

#include <Fog/Core/Lock.h>
#include <Fog/Graphics/Font.h>

#include <windows.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

struct FontEngineWin;

// ============================================================================
// [Fog::FontFaceWin]
// ============================================================================

struct FOG_API FontFaceWin : public FontFace
{
  // Lock
  mutable Lock lock;
  // Glyph cache:
  GlyphCache glyphCache;
  // Windows font handle
  HFONT hFont;

  FontFaceWin();
  virtual ~FontFaceWin();

  virtual err_t getGlyphs(const Char32* str, sysuint_t length, GlyphSet& glyphSet);
  virtual err_t getTextWidth(const Char32* str, sysuint_t length, TextWidth* textWidth);
  virtual err_t getPath(const Char32* str, sysuint_t length, Path& dst);

private:
  //! @brief Renders glyph and returns it.
  //! @note You must use renderBegin() and renderEnd().
  Glyph::Data* renderGlyph(HDC hdc, uint32_t uc);

  friend struct FontEngineWin;
};

// ============================================================================
// [Fog::FontEngineWin]
// ============================================================================

struct FOG_API FontEngineWin : public FontEngine
{
  FontEngineWin();
  virtual ~FontEngineWin();

  virtual Vector<String32> getFonts();

  virtual FontFace* getDefaultFace();

  virtual FontFace* getFace(
    const Fog::String32& family, uint32_t size, 
    const FontAttributes& attributes);
};

} // Fog namespace

//! @}

#endif // FOG_FONT_WINDOWS

// [Guard]
#endif // _FOG_GRAPHICS_FONT_WIN_H
