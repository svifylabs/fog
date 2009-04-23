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

struct FOG_API FontFaceFT : public FontFace
{
  // Lock
  mutable Fog::Lock lock;
  // Glyph cache:
  GlyphCache glyphCache;
  // FtFile
  FtFile* ftFile;

  FontFaceFT();
  virtual ~FontFaceFT();
  virtual void deref();

  virtual void getTextWidth(
    // in
    const Fog::Char32* str, sysuint_t length,
    // out
    TextWidth* textWidth);

  virtual void getGlyphs(
    // in
    const Fog::Char32* str, sysuint_t length,
    // out
    Glyph* target,
    TextWidth* textWidth);

private:
  Glyph::Data* renderGlyph(uint32_t uc);

  friend struct FontEngineWin;
};

struct FOG_API FontEngineFT : public FontEngine
{
  FontEngineFTPrivate* p;

  FontEngineFT();
  virtual ~FontEngineFT();

  virtual Fog::Vector<Fog::String32> getFonts();

  virtual FontFace* getDefaultFace();

  virtual FontFace* getFace(
    const Fog::String32& family, uint32_t size, 
    const FontAttributes& attributes);
};

} // Fog namespace

//! @}

#endif // FOG_FONT_FREETYPE

// [Guard]
#endif // _FOG_GRAPHICS_FONT_FREETYPE_H
