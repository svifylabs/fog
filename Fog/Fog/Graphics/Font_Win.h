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
  virtual void deref();

  virtual err_t getGlyphs(const Char32* str, sysuint_t length, GlyphSet& glyphSet);
  virtual err_t getTextWidth(const Char32* str, sysuint_t length, TextWidth* textWidth);

private:
  // To call these functions or access members you must LOCK the lock
  HDC hdc;
  HFONT hOldFont;

  //! @brief Initialize rendering resources for rendering glyphs.
  bool renderBegin();
  //! @brief Free rendering resources.
  void renderEnd();
  //! @brief Renders glyph and returns it.
  //! @note You must use renderBegin() and renderEnd().
  Glyph::Data* renderGlyph(uint32_t uc);

  friend struct FontEngineWin;
};

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
