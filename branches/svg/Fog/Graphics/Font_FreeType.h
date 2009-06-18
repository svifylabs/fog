// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_FONT_FREETYPE_H
#define _FOG_GRAPHICS_FONT_FREETYPE_H

// [Dependencies]
#include <Fog/Graphics/Font.h>

#if defined(FOG_FONT_FREETYPE)

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

struct FontEngineFT;
struct FontEngineFTPrivate;
struct FtFile;

// ============================================================================
// [Fog::FontFaceFT]
// ============================================================================

struct FOG_API FontFaceFT : public FontFace
{
  // Lock
  mutable Lock lock;
  // Glyph cache:
  GlyphCache glyphCache;
  // FtFile
  FtFile* ftFile;

  FontFaceFT();
  virtual ~FontFaceFT();
  virtual void deref();

  virtual err_t getGlyphs(const Char32* str, sysuint_t length, GlyphSet& glyphSet);
  virtual err_t getTextWidth(const Char32* str, sysuint_t length, TextWidth* textWidth);
  virtual err_t getPath(const Char32* str, sysuint_t length, Path& dst);

private:
  Glyph::Data* renderGlyph(uint32_t uc);

  friend struct FontEngineWin;
};

// ============================================================================
// [Fog::FontEngineFT]
// ============================================================================

struct FOG_API FontEngineFT : public FontEngine
{
  FontEngineFTPrivate* p;

  FontEngineFT();
  virtual ~FontEngineFT();

  virtual Vector<String32> getFonts();

  virtual FontFace* getDefaultFace();

  virtual FontFace* getFace(
    const String32& family, uint32_t size, 
    const FontAttributes& attributes);
};

} // Fog namespace

//! @}

#endif // FOG_FONT_FREETYPE

// [Guard]
#endif // _FOG_GRAPHICS_FONT_FREETYPE_H
