// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

#include <Fog/Graphics/Font.h>

#if defined(FOG_FONT_WINDOWS)

#include <Fog/Core/AutoLock.h>
#include <Fog/Core/Lock.h>
#include <Fog/Core/Vector.h>

#include <Fog/Graphics/AffineMatrix.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/Graphics/Font_Win.h>
#include <Fog/Graphics/Path.h>

#ifndef GGO_UNHINTED
#define GGO_UNHINTED 0x0100
#endif // GGO_UNHINTED

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
  FontEngine(Ascii8("Windows"))
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

  name.set(Utf16(lfFaceName));

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
  return getFace(String32(Ascii8("arial")), 12, a);
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
// [Fog::FontFaceWin - Helpers]
// ============================================================================

// Identity matrix. It seems that this matrix must be always passed to 
// GetGlyphOutlineW function.
static const MAT2 mat2identity = {{0, 1}, {0, 0}, {0, 0}, {0, 1}};

static FOG_INLINE double fxToDouble(const FIXED& p)
{
  return (double)p.value + (double)p.fract * (1.0 / 65536.0);
}

static bool decompose_win32_glyph_outline(
  const uint8_t* gbuf,
  unsigned total_size,
  bool flip_y,
  const AffineMatrix& mtx,
  Path& path)
{
  const uint8_t* cur_glyph = gbuf;
  const uint8_t* end_glyph = gbuf + total_size;
  double x, y;

  if (cur_glyph == end_glyph) return true;

  do {
    const TTPOLYGONHEADER* th = (TTPOLYGONHEADER*)cur_glyph;
    
    const uint8_t* end_poly = cur_glyph + th->cb;
    const uint8_t* cur_poly = cur_glyph + sizeof(TTPOLYGONHEADER);

    x = fxToDouble(th->pfxStart.x);
    y = fxToDouble(th->pfxStart.y);
    if (flip_y) y = -y;
    mtx.transform(&x, &y);

    path.closePolygon();
    path.moveTo(PointF(x, y));

    while (cur_poly < end_poly)
    {
      const TTPOLYCURVE* pc = (const TTPOLYCURVE*)cur_poly;
      
      if (pc->wType == TT_PRIM_LINE)
      {
        int i;
        for (i = 0; i < pc->cpfx; i++)
        {
          x = fxToDouble(pc->apfx[i].x);
          y = fxToDouble(pc->apfx[i].y);
          if(flip_y) y = -y;
          mtx.transform(&x, &y);
          path.lineTo(PointF(x, y));
        }
      }
      
      if (pc->wType == TT_PRIM_QSPLINE)
      {
        int u;
        for (u = 0; u < pc->cpfx - 1; u++) // Walk through points in spline
        {
          POINTFX pnt_b = pc->apfx[u]; // B is always the current point
          POINTFX pnt_c = pc->apfx[u+1];
          
          if (u < pc->cpfx - 2) // If not on last spline, compute C
          {
            // midpoint (x,y)
            *(int*)&pnt_c.x = (*(int*)&pnt_b.x + *(int*)&pnt_c.x) / 2;
            *(int*)&pnt_c.y = (*(int*)&pnt_b.y + *(int*)&pnt_c.y) / 2;
          }
          
          double x2, y2;
          x  = fxToDouble(pnt_b.x);
          y  = fxToDouble(pnt_b.y);
          x2 = fxToDouble(pnt_c.x);
          y2 = fxToDouble(pnt_c.y);
          if (flip_y) { y = -y; y2 = -y2; }
          mtx.transform(&x,  &y);
          mtx.transform(&x2, &y2);
          path.curveTo(PointF(x, y), PointF(x2, y2));
        }
      }
      cur_poly += sizeof(WORD) * 2 + sizeof(POINTFX) * pc->cpfx;
    }
    cur_glyph += th->cb;
  } while (cur_glyph < end_glyph);
  path.closePolygon();

  return true;
}

// ============================================================================
// [Fog::FontFaceWin]
// ============================================================================

FontFaceWin::FontFaceWin() :
  hFont(NULL)
{
}

FontFaceWin::~FontFaceWin()
{
  if (hFont) DeleteObject((HGDIOBJ)hFont);
}

err_t FontFaceWin::getGlyphs(const Char32* str, sysuint_t length, GlyphSet& glyphSet)
{
  err_t err;
  if ( (err = glyphSet.begin(length)) ) return err;

  AutoLock locked(lock);

  Glyph::Data* glyphd;
  HDC hdc = NULL;

  for (sysuint_t i = 0; i != length; i++)
  {
    uint32_t uc = str[i].ch();

    // First try cache.
    glyphd = glyphCache.get(uc);
    if (FOG_UNLIKELY(!glyphd))
    {
      // Glyph is not in cache, initialize HDC (if not initialized) and try
      // to get glyph from HFONT.
      if (hdc == NULL)
      {
        if ((hdc = CreateCompatibleDC(NULL)) == NULL) continue;
        SelectObject(hdc, (HGDIOBJ)hFont);
      }

      if ((glyphd = renderGlyph(hdc, uc))) glyphCache.set(uc, glyphd);
    }

    if (FOG_LIKELY(glyphd)) glyphSet._add(glyphd->ref());
  }

  if (hdc) DeleteDC(hdc);

  if ( (err = glyphSet.end()) ) return err;
  return Error::Ok;
}

err_t FontFaceWin::getTextWidth(const Char32* str, sysuint_t length, TextWidth* textWidth)
{
  TemporaryGlyphSet<128> glyphSet;
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

err_t FontFaceWin::getPath(const Char32* str, sysuint_t length, Path& dst)
{
  AutoLock locked(lock);

  err_t err = Error::Ok;
  AffineMatrix matrix;

  GLYPHMETRICS gm;
  ZeroMemory(&gm, sizeof(gm));

  HDC hdc = CreateCompatibleDC(NULL);
  if (hdc == NULL) return GetLastError();
  SelectObject(hdc, (HGDIOBJ)hFont);

  MemoryBuffer<1024> glyphBuffer;
  DWORD glyphBufferSize = 1024;
  uint8_t *glyphData = reinterpret_cast<uint8_t*>(glyphBuffer.alloc(1024));

  for (sysuint_t i = 0; i < length; i++)
  {
    uint32_t uc = str[i].ch();
    uint32_t dataSize;

repeat:
    dataSize = GetGlyphOutlineW(hdc, uc, GGO_NATIVE, &gm, glyphBufferSize, glyphData, &mat2identity);
    if (dataSize == GDI_ERROR) continue;

    if (dataSize > glyphBufferSize)
    {
      if (dataSize < 4000) dataSize = 4000;
      glyphBuffer.free();
      glyphData = reinterpret_cast<uint8_t*>(glyphBuffer.alloc(dataSize));
      if (!glyphData) continue;
      glyphBufferSize = dataSize;
      goto repeat;
    }

    decompose_win32_glyph_outline(glyphData, dataSize, true, matrix, dst);

    matrix.translate(gm.gmCellIncX, gm.gmCellIncY);
  }

end:
  DeleteDC(hdc);
  return err;
}

Glyph::Data* FontFaceWin::renderGlyph(HDC hdc, uint32_t uc)
{
  // renderBegin() must be called before
  FOG_ASSERT(hdc);

  Glyph::Data* glyphd = NULL;
  Image::Data* bitmapd = NULL;

  GLYPHMETRICS gm;
  ZeroMemory(&gm, sizeof(gm));

  uint32_t dataSize = GetGlyphOutlineW(hdc, uc, GGO_GRAY8_BITMAP, &gm, 0, NULL, &mat2identity);

  // GetGlyphOutline() fails when being called
  // for GGO_GRAY8_BITMAP and white space.
  if (dataSize == GDI_ERROR)
  {
    dataSize = GetGlyphOutlineW(hdc, uc, GGO_METRICS, &gm, 0, NULL, &mat2identity);
    if (dataSize == GDI_ERROR) return NULL;
    dataSize = 0;
  }

  glyphd = new(std::nothrow) Glyph::Data();
  if (glyphd == NULL) return NULL;

  // Whitespace?
  if (dataSize == 0) gm.gmBlackBoxX = gm.gmBlackBoxY = 0;

  glyphd->bitmapX = gm.gmptGlyphOrigin.x;
  glyphd->bitmapY = metrics.ascent - gm.gmptGlyphOrigin.y;

  glyphd->beginWidth = 0;
  glyphd->endWidth = 0;

  glyphd->advance = gm.gmCellIncX;
  // glyphd->advanceY = -gm.gmCellIncY;

  // Whitespace? We are done
  if (dataSize == 0) return glyphd;

  // Alloc image for glyph
  if (glyphd->bitmap.create(gm.gmBlackBoxX, gm.gmBlackBoxY, Image::FormatA8) != Error::Ok)
  {
    delete glyphd;
    return NULL;
  }
  bitmapd = glyphd->bitmap._d;

  // Fog library should align scanlines to 32 bits like Windows does.
  FOG_ASSERT((bitmapd->stride & 0x3) == 0);
  // This should be also equal.
  FOG_ASSERT(dataSize == bitmapd->stride * bitmapd->height);

  dataSize = GetGlyphOutlineW(hdc, uc, GGO_GRAY8_BITMAP, &gm, dataSize, bitmapd->data, &mat2identity);
  FOG_ASSERT(dataSize != GDI_ERROR);

  uint32_t x, y;

  for (y = 0; y != gm.gmBlackBoxY; y++)
  {
    uint8_t* p = bitmapd->first + y * bitmapd->stride;
    for (x = 0; x < gm.gmBlackBoxX; x++)
    {
      *p++ = (*p > 63) ? 0xFF : *p << 2;
    }
  }

  return glyphd;
}

} // Fog namespace

#endif // FOG_FONT_WINDOWS
