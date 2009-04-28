// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

#include <Fog/Graphics/Font.h>

#if defined(FOG_FONT_WINDOWS)

#include <Fog/Core/AutoLock.h>
#include <Fog/Core/Lock.h>
#include <Fog/Core/Vector.h>

#include <Fog/Graphics/Font_Win.h>

namespace Fog {

// tools

struct EnumFontStruct
{
  Vector<String32>* fonts;
  HDC hdc;
  WCHAR buffer[256];
};

// ============================================================================
// [Fog::FontEngineWin]
// ============================================================================

FontEngineWin::FontEngineWin() :
  FontEngine(StubAscii8("Windows"))
{
}

FontEngineWin::~FontEngineWin()
{
}

static int CALLBACK enumFontCb(CONST LOGFONTW* lplf, CONST TEXTMETRICW* lpntm, DWORD fontType, LPARAM lParam)
{
  EnumFontStruct* efs = (EnumFontStruct*)lParam;
  const WCHAR* lfFaceName = lplf->lfFaceName; 

  // Some rejects, I dislike '@'.
  if (lfFaceName[0] == L'@') return 1;
  
  // Windows will send us more fonts that we want, but usually
  // equal fonts are sent together, so we will simply copy this
  // font to buffer and compare it with previous. If this will
  // match - reject it now to save cpu cycles.
  if (wcscmp(efs->buffer, lfFaceName) == 0) return 1;
  // Font not match, so copy it to buffer.
  wcscpy_s(efs->buffer, 256, lfFaceName);

  // Many of the fonts are the same family, so temporary string
  // is here better
  TemporaryString32<TemporaryLength> name;

  name.set(StubUtf16(lfFaceName));

  if (!efs->fonts->contains(name)) efs->fonts->append(name);
  
  // We return 1 to continue listing...
  return 1;
}

Vector<String32> FontEngineWin::getFonts()
{
  Vector<String32> fonts;
  EnumFontStruct efs;
  efs.fonts = &fonts;
  efs.hdc = GetDC(NULL);
  efs.buffer[0] = 0;

  LOGFONT lf;
  ZeroMemory(&lf, sizeof(lf));
  lf.lfCharSet = DEFAULT_CHARSET;

  EnumFontFamiliesEx(efs.hdc, &lf, (FONTENUMPROC)enumFontCb, (LPARAM)&efs, 0);
  ReleaseDC(NULL, efs.hdc);

  return fonts;
}

FontFace* FontEngineWin::getDefaultFace()
{
  FontAttributes a;
  ZeroMemory(&a, sizeof(FontAttributes));
  return getFace(String32(StubAscii8("arial")), 12, a);
}

FontFace* FontEngineWin::getFace(
  const String32& family, uint32_t size,
  const FontAttributes& attributes)
{
  TemporaryString16<128> _tfamily;
  FontFaceWin* face = NULL;

  LOGFONTW logFont;
  TEXTMETRICW textMetrics;
  HFONT hOldFont;
  HFONT hFont;
  HDC hdc;

  // Get LOGFONT.
  ZeroMemory(&logFont, sizeof(LOGFONT));

  _tfamily.set(family);
  if (_tfamily.length() >= FOG_ARRAY_SIZE(logFont.lfFaceName)) goto fail;

  CopyMemory(logFont.lfFaceName, _tfamily.cStrW(),
    (_tfamily.length() + 1) * sizeof(WCHAR));
  logFont.lfHeight = -(int)size;
  logFont.lfWeight = (attributes.bold  ) ? FW_BOLD : FW_NORMAL;
  logFont.lfItalic = (attributes.italic) != 0;

  // Create HFONT.
  if ((hFont = CreateFontIndirectW(&logFont)) == NULL) goto fail;

  // Get text metrics (need to create temporary HDC).
  if ((hdc = CreateCompatibleDC(NULL)) == NULL) goto failFreeHFONT;
  if ((hOldFont = (HFONT)SelectObject(hdc, (HGDIOBJ)hFont)) == (HFONT)GDI_ERROR) goto failFreeHFONTandHDC;

  GetTextMetricsW(hdc, &textMetrics);
  SelectObject(hdc, (HGDIOBJ)hOldFont);
  DeleteDC(hdc);

  // Everything should be OK, create new font face.
  face = new FontFaceWin();
  face->family = family;
  face->family.squeeze();
  face->metrics.size = size;
  face->metrics.ascent = textMetrics.tmAscent;
  face->metrics.descent = textMetrics.tmDescent;
  face->metrics.averageWidth = textMetrics.tmAveCharWidth;
  face->metrics.maximumWidth = textMetrics.tmMaxCharWidth;
  face->metrics.height = textMetrics.tmHeight;
  face->attributes = attributes;
  face->hFont = hFont;

  // Fog_FontFaceCache_put(face);
  return face;

failFreeHFONTandHDC:
  DeleteDC(hdc);
failFreeHFONT:
  DeleteObject((HGDIOBJ)hFont);
fail:
  return NULL;
}

// ============================================================================
// [Fog::FontFaceWin]
// ============================================================================

FontFaceWin::FontFaceWin() :
  hFont(NULL),
  hdc(NULL),
  hOldFont(NULL)
{
}

FontFaceWin::~FontFaceWin()
{
  renderEnd();
  if (hFont) DeleteObject((HGDIOBJ)hFont);
}

void FontFaceWin::deref()
{
  if (refCount.deref()) delete this;
}

err_t FontFaceWin::getGlyphs(const Char32* str, sysuint_t length, GlyphSet& glyphSet)
{
  err_t err;
  if ( (err = glyphSet.begin(length)) ) return err;

  AutoLock locked(lock);

  Glyph::Data* glyphd;
  bool renderUsed = false;

  for (sysuint_t i = 0; i != length; i++)
  {
    uint32_t uc = str[i].ch();

    // First try cache.
    glyphd = glyphCache.get(uc);
    if (FOG_UNLIKELY(!glyphd))
    {
      if (!renderUsed) renderUsed = renderBegin();
      glyphd = renderGlyph(uc);
      if (glyphd) glyphCache.set(uc, glyphd);
    }

    if (FOG_LIKELY(glyphd)) glyphSet._add(glyphd->ref());
  }

  if (renderUsed) renderEnd();

  if ( (err = glyphSet.end()) ) return err;
  return Error::Ok;
}

err_t FontFaceWin::getTextWidth(const Char32* str, sysuint_t length, TextWidth* textWidth)
{
  GlyphSet glyphSet;
  err_t err = getGlyphs(str, length, glyphSet);

  if (err)
  {
    ZeroMemory(textWidth, sizeof(TextWidth));
    return err;
  }
  else
  {
    textWidth->advance = glyphSet.advance();
    textWidth->beginWidth = 0;
    textWidth->endWidth = 0;
    return Error::Ok;
  }
}

bool FontFaceWin::renderBegin()
{
  if (hdc == NULL)
  {
    if ((hdc = CreateCompatibleDC(NULL)) == NULL) return false;
    if ((hOldFont = (HFONT)SelectObject(hdc, (HGDIOBJ)hFont)) == NULL) 
    {
      DeleteDC(hdc);
      hdc = NULL; 
      return false;
    }
  }
  return true;
}

void FontFaceWin::renderEnd()
{
  if (hdc != NULL)
  {
    SelectObject(hdc, (HGDIOBJ)hOldFont);
    DeleteDC(hdc);

    hdc = NULL;
    hOldFont = NULL;
  }
}

Glyph::Data* FontFaceWin::renderGlyph(uint32_t uc)
{
  // renderBegin() must be called before
  FOG_ASSERT(hdc);

  Glyph::Data* glyphd = NULL;
  Image::Data* imaged = NULL;

  SetBkMode(hdc, TRANSPARENT);

  GLYPHMETRICS gm;

  // Identity matrix. It seems that this matrix must be passed to 
  // GetGlyphOutlineW function.
  MAT2 m2 = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};

  ZeroMemory(&gm, sizeof(gm));

  uint32_t dataSize = GetGlyphOutlineW(hdc, uc, GGO_GRAY8_BITMAP, &gm, 0, NULL, &m2);

  if (dataSize == GDI_ERROR) return NULL;

  glyphd = new Glyph::Data();

  // Whitespace?
  if (dataSize == 0) gm.gmBlackBoxX = gm.gmBlackBoxY = 0;

  glyphd->offsetX = gm.gmptGlyphOrigin.x;
  glyphd->offsetY = metrics.height - gm.gmptGlyphOrigin.y;

  glyphd->beginWidth = 0;
  glyphd->endWidth = 0;

  glyphd->advance = gm.gmCellIncX;
  // glyphd->advanceY = -gm.gmCellIncY;

  // Whitespace? We are done
  if (dataSize == 0) return glyphd;

  // Alloc image for glyph
  if (!glyphd->image.create(gm.gmBlackBoxX, gm.gmBlackBoxY, ImageFormat::a8()))
  {
    delete glyphd;
    return NULL;
  }
  imaged = glyphd->image._d;

  // Fog library should align scanlines to 32 bits like Windows does.
  FOG_ASSERT((imaged->stride & 0x3) == 0);
  // This should be also equal
  FOG_ASSERT(dataSize == imaged->stride * imaged->height);

  dataSize = GetGlyphOutlineW(hdc, uc, GGO_GRAY8_BITMAP, &gm, dataSize, imaged->data, &m2);
  FOG_ASSERT(dataSize != GDI_ERROR);

  uint32_t x, y;

  for (y = 0; y != gm.gmBlackBoxY; y++)
  {
    uint8_t* p = imaged->first + y * imaged->stride;

    for (x = 0; x < gm.gmBlackBoxX; x++)
    {
      *p++ = (*p > 63) ? 0xFF : *p << 2;
    }
  }

  // Windows uses bottom-to-top images while we are using top-to-bottom
  // glyphd->image.mirror(Image::MirrorVertical);

  return glyphd;
}

} // Fog namespace

#endif // FOG_FONT_WINDOWS
