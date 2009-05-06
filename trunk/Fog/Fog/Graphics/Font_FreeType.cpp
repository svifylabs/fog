// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

#include <Fog/Build/Build.h>

#if defined(FOG_FONT_FREETYPE)

// [Dependencies]
#include <Fog/Core/AutoLock.h>
#include <Fog/Core/Hash.h>
#include <Fog/Core/Lock.h>
#include <Fog/Core/Library.h>
#include <Fog/Core/MapFile.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/OS.h>
#include <Fog/Core/FileSystem.h>
#include <Fog/Core/FileUtil.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/StringCache.h>
#include <Fog/Core/TextCodec.h>
#include <Fog/Core/UserInfo.h>
#include <Fog/Graphics/Font.h>
#include <Fog/Graphics/Font_FreeType.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/Rgba.h>

// These includes means
// --------------------
// #include <freetype/freetype.h>
// #include <freetype/ftoutln.h>
// #include <freetype/ttnameid.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H
#include FT_TRUETYPE_IDS_H

// [include fontconfig header]
#if defined(FOG_HAVE_FONTCONFIG)
#include <fontconfig/fontconfig.h>
#endif // FOG_HAVE_FONTCONFIG

#if !defined(FT_OPEN_MEMORY)
#define FT_OPEN_MEMORY ft_open_memory
#endif

namespace Fog {

static FontEngineFTPrivate* fepriv;

// [Macros]

// Routines for converting from fixed point
#define FT_FLOOR(X)     ((X & -64) / 64)
#define FT_CEIL(X)      (((X + 63) & -64) / 64)

// Returns @c true is mactrices 'a' and 'b' are equal
#define FT_Matrix_Equal(a,b) ( \
  (a)->xx == (b)->xx && \
  (a)->yy == (b)->yy && \
  (a)->xy == (b)->xy && \
  (a)->yx == (b)->yx )

// [Helpers]

struct FontEngineFTTranslator
{
  FontEngineFTTranslator()
  {
  }

  FontEngineFTTranslator(const String32 &family, const String32& file)
    : family(family), file(file)
  {
  }

  ~FontEngineFTTranslator()
  {
  }

  String32 family;
  String32 file;

  bool operator==(const FontEngineFTTranslator& other)
  { return family == other.family && file == other.file; }
};

}

FOG_DECLARE_TYPEINFO(Fog::FontEngineFTTranslator, Fog::PrimitiveType)

namespace Fog {

// ============================================================================
// [Fog::FtFile]
// ============================================================================

// FtFile represends one freetype file and it's face object. It's allowed to
// load and uload faces on-the-fly.
struct FtFile
{
  /*! @brief Reference count. */
  Atomic<sysuint_t> refCount;
  /*! @brief Use count. */
  Atomic<sysuint_t> used;
  /*! @brief Absolute file path. */
  String32 fileName;
  /*! @brief Family. */
  String32 family;
  /*! @brief Map file object. */
  MapFile mapFile;
  /*! @brief Freetype face. */
  FT_Face ftFace;
  /*! @brief Loading error. */
  FT_Error loadError;
  /*! @brief Unload after use. */
  uint32_t unloadAfterUse;
  /*! @brief Fixed width index. */
  uint32_t fixedWidthIndex;

  FtFile(const String32& fileName, const String32& family);
  ~FtFile();
  FtFile* ref();
  void deref();
  // use() and unuse() are public methods that's used by engine.
  bool use();
  void unuse();
  bool load();
  void unload();
  FOG_INLINE bool loaded() const;
  bool setupSize(uint32_t size);

  FontFace* createFace(uint32_t size, const FontAttributes& attributes);

private:
  FOG_DISABLE_COPY(FtFile)
};

// ============================================================================
// [Fog::FontEngineFTPrivate]
// ============================================================================

struct FontEngineFTPrivate
{
  bool fcInitialized;
  bool ftInitialized;

  FontEngineFTPrivate() :
    fcInitialized(false),
    ftInitialized(false)
  {
    Vector<String32> paths;

#if defined(FOG_HAVE_FONTCONFIG)
    uint32_t fcStatus = fcLoad();
#endif // FOG_HAVE_FONTCONFIG
    uint32_t ftStatus = ftLoad();

    // add basic font translator if font config is not used
#if defined(FOG_HAVE_FONTCONFIG)
    if (fcStatus != Error::Ok)
    {
#endif // FOG_HAVE_FONTCONFIG
      // Fontconfig load failed or is not configured to use it, so add basic
      // paths that's used in distributions
      paths = ftGetFontDirectories();

      // And initialize our freetype font translator that can translate
      // corefonts and bitstream fonts into their file names so we can
      // use them.
      ftInitTranslator();
#if defined(FOG_HAVE_FONTCONFIG)
    }
    else
    {
      // add font directories through font config API
      paths = fcGetFontDirectories();
    }
#endif // FOG_HAVE_FONTCONFIG

    Font::addFontPaths(paths);
  }

  ~FontEngineFTPrivate()
  {
    if (ftInitialized) ftClose();
#if defined(FOG_HAVE_FONTCONFIG)
    if (fcInitialized) fcClose();
#endif // FOG_HAVE_FONTCONFIG
  }

  String32 resolveFontPath(const String32& family, uint32_t size, const FontAttributes& attributes)
  {
    String32 result;

#if defined(FOG_HAVE_FONTCONFIG)
    if (fcInitialized)
    {
      result = fcResolveFontPath(family, size, attributes);
      if (!result.isEmpty()) return result;
    }
#endif // FOG_HAVE_FONTCONFIG

    return result;
  }

  // --------------------------------------------------------------------------
  // [FontConfig Support]
  // --------------------------------------------------------------------------

#if defined(FOG_HAVE_FONTCONFIG)
  Library fcDll;
  enum { FcSymbolsCount = 25 };

  union
  {
    struct
    {
      FcConfig* (*pFcInitLoadConfig)(void);
      FcConfig* (*pFcInitLoadConfigAndFonts)(void);
      FcBool (*pFcInit)(void);
      void (*pFcFini)(void);
      int (*pFcGetVersion)(void);
      FcBool (*pFcInitReinitialize)(void);
      FcBool (*pFcInitBringUptoDate)(void);

      FcPattern* (*pFcPatternCreate)(void);
      void (*pFcPatternDestroy)(FcPattern *p);
      FcResult (*pFcPatternGet)(const FcPattern *p, const char *object, int id, FcValue *v);
      FcResult (*pFcPatternAddInteger)(const FcPattern *p, const char *object, int i);
      FcResult (*pFcPatternAddDouble)(const FcPattern *p, const char *object, double d);
      FcObjectSet* (*pFcObjectSetBuild)(const char *first, ...);
      void (*pFcObjectSetDestroy)(FcObjectSet *os);
      void (*pFcFontSetDestroy)(FcFontSet *s);
      FcPattern* (*pFcNameParse)(const FcChar8 *name);
      FcChar8* (*pFcNameUnparse)(FcPattern *pat);
      FcFontSet* (*pFcFontList)(FcConfig* config, FcPattern* p, FcObjectSet* os);
      FcFontSet* (*pFcFontSort)(FcConfig *config, FcPattern *p, FcBool trim, FcCharSet **csp, FcResult *result);
      FcPattern* (*pFcFontMatch)(FcConfig *config, FcPattern *p, FcResult *result);
      FcBool (*pFcConfigSubstitute)(FcConfig* config, FcPattern *p, FcMatchKind kind);
      void (*pFcDefaultSubstitute)(FcPattern *pattern);

      FcStrList* (*pFcConfigGetFontDirs)(FcConfig* config);
      FcChar8* (*pFcStrListNext)(FcStrList* list);
      void (*pFcStrListDone)(FcStrList* list);
    };

    void *fcAddr[FcSymbolsCount];
  };

  uint32_t fcLoad()
  {
    static const char symbols[] =
    {
      "FcInitLoadConfig\0"
      "FcInitLoadConfigAndFonts\0"
      "FcInit\0"
      "FcFini\0"
      "FcGetVersion\0"
      "FcInitReinitialize\0"
      "FcInitBringUptoDate\0"
      "FcPatternCreate\0"
      "FcPatternDestroy\0"
      "FcPatternGet\0"
      "FcPatternAddInteger\0"
      "FcPatternAddDouble\0"
      "FcObjectSetBuild\0"
      "FcObjectSetDestroy\0"
      "FcFontSetDestroy\0"
      "FcNameParse\0"
      "FcNameUnparse\0"
      "FcFontList\0"
      "FcFontSort\0"
      "FcFontMatch\0"
      "FcConfigSubstitute\0"
      "FcDefaultSubstitute\0"
      "FcConfigGetFontDirs\0"
      "FcStrListNext\0"
      "FcStrListDone\0"
    };

    // Load fontconfig library
    if (fcDll.open(StubAscii8("fontconfig"), Library::OpenSystemPrefix | Library::OpenSystemSuffix))
    {
      return Error::FontConfigLibraryNotFound;
    }

    // Load symbols
    if (fcDll.symbols(fcAddr, symbols, FOG_ARRAY_SIZE(symbols), FcSymbolsCount, (char**)NULL) != FcSymbolsCount)
    {
      fcDll.close();
      return Error::FontConfigSymbolNotFound;
    }

    // Initialize fontconfig library
    if (!pFcInit())
    {
      fcDll.close();
      return Error::FontConfigInitFailed;
    }

    fcInitialized = true;
    return Error::Ok;
  }

  void fcClose()
  {
    if (fcInitialized)
    {
      pFcFini();
      fcDll.close();
      fcInitialized = false;
    }
  }

  // adds font directories into font paths using fontconfig functions.
  Vector<String32> fcGetFontDirectories()
  {
    FOG_ASSERT(fcInitialized);

    Vector<String32> result;
    FcStrList* list = pFcConfigGetFontDirs(NULL);
    char* localDirName;
    String32 uniDirName;

    while ((localDirName = (char*)pFcStrListNext(list)) != NULL)
    {
      uniDirName.set(StubLocal8(localDirName, DetectLength));
      if (!result.contains(uniDirName)) result.append(uniDirName);
    }

    pFcStrListDone(list);
    return result;
  }

  String32 fcResolveFontPath(const String32& family, uint32_t size, const FontAttributes& attributes)
  {
    FOG_ASSERT(fcInitialized);

    TemporaryString8<TemporaryLength> family8;
    family8.set(family, TextCodec::local8());

    FcPattern* p1;
    FcPattern* p2;
    FcValue filename;
    FcResult res;
    String32 result;

    p1 = pFcNameParse((FcChar8 *)family8.cStr());
    pFcPatternAddDouble(p1, FC_SIZE, (double)size);
    pFcConfigSubstitute(NULL, p1, FcMatchPattern);
    pFcDefaultSubstitute(p1);

    if ((p2 = pFcFontMatch(NULL, p1, &res)))
    {
      pFcPatternGet(p2, FC_FILE, 0, &filename);
      result.set((const char*)filename.u.s, DetectLength, TextCodec::local8());
      pFcPatternDestroy(p2);
    }

    pFcPatternDestroy(p1);

    return result;
  }

  Vector<String32> fcFontsList()
  {
    FOG_ASSERT(fcInitialized);

    Vector<String32> result;

    FcPattern* p;
    FcFontSet* set = NULL;
    FcObjectSet* os;
    FcValue val;

    sysint_t i;

    p = pFcPatternCreate();
    os = pFcObjectSetBuild(FC_FAMILY, FC_STYLE, NULL);

    if (p && os) set = pFcFontList(NULL, p, os);

    if (set)
    {
      String32 t;

      for (i = 0; i < set->nfont; i++)
      {
        pFcPatternGet(set->fonts[i], FC_FAMILY, 0, &val);
        t.set((const char*)val.u.s, DetectLength, TextCodec::local8());
        if (!result.contains(t)) result.append(t);
      }

      pFcFontSetDestroy(set);
    }

    if (p) pFcPatternDestroy(p);
    if (os) pFcObjectSetDestroy(os);

    return result;
  }
#endif // FOG_HAVE_FONTCONFIG

  // --------------------------------------------------------------------------
  // [FreeType Support]
  // --------------------------------------------------------------------------

  Library ftDll;
  enum { FtSymbolsCount = 12 };

  FT_Library ftLibrary;

  union
  {
    struct
    {
      FOG_CDECL FT_Error (*pFT_Init_FreeType)(FT_Library*);
      FOG_CDECL FT_Error (*pFT_Done_FreeType)(FT_Library);
      FOG_CDECL FT_Long  (*pFT_MulFix)(FT_Long /* a */, FT_Long /* b */);
      FOG_CDECL FT_Long  (*pFT_DivFix)(FT_Long /* a */, FT_Long /* b */);
      FOG_CDECL FT_Error (*pFT_New_Face)(FT_Library, const char* /* fileName */, FT_Long /* face_index */, FT_Face* /* face */);
      FOG_CDECL FT_Error (*pFT_Open_Face)(FT_Library, const FT_Open_Args* /* openArgs */, FT_Long /* face_index */, FT_Face* /* face */);
      FOG_CDECL FT_Error (*pFT_Done_Face)(FT_Face /* face */);
      FOG_CDECL FT_Error (*pFT_Set_Char_Size)(FT_Face /* face */, FT_F26Dot6 /* char_width */, FT_F26Dot6 /* char_height */, FT_UInt /* horz_resolution */, FT_UInt /* vert_resolution */);
      FOG_CDECL FT_Error (*pFT_Set_Pixel_Sizes)(FT_Face /* face */, FT_UInt /* pixel_width */, FT_UInt /* pixel_height */);
      FOG_CDECL FT_UInt  (*pFT_Get_Char_Index)(FT_Face /* face */, FT_ULong /* charcode */);
      FOG_CDECL FT_Error (*pFT_Load_Glyph)(FT_Face /* face */, FT_UInt /* glyph_index */, FT_Int32 /* load_flags */);
      FOG_CDECL FT_Error (*pFT_Render_Glyph)(FT_GlyphSlot /* slot */, FT_Render_Mode /* glyph_index */);
    };
    void *ftAddr[FtSymbolsCount];
  };

  // Hash table that contains mapping path into FtFile* object.
  Hash<String32, FtFile*> ftFileCache;
  Vector<FontEngineFTTranslator> ftTranslator;

  uint32_t ftLoad()
  {
    static char symbols[] =
      "FT_Init_FreeType\0"
      "FT_Done_FreeType\0"
      "FT_MulFix\0"
      "FT_DivFix\0"
      "FT_New_Face\0"
      "FT_Open_Face\0"
      "FT_Done_Face\0"
      "FT_Set_Char_Size\0"
      "FT_Set_Pixel_Sizes\0"
      "FT_Get_Char_Index\0"
      "FT_Load_Glyph\0"
      "FT_Render_Glyph\0";

    FT_Error error;

    // Load freetype library
    if (!ftDll.open(StubAscii8("freetype"), Library::OpenSystemPrefix | Library::OpenSystemSuffix) == Error::Ok)
    {
      return Error::FreeTypeLibraryNotFound;
    }

    // Load symbols
    if (ftDll.symbols(ftAddr, symbols, FOG_ARRAY_SIZE(symbols), FtSymbolsCount, (char**)NULL) != FtSymbolsCount)
    {
      ftDll.close();
      return Error::FreeTypeSymbolNotFound;
    }

    // Initialize Freetype library
    if ((error = pFT_Init_FreeType(&ftLibrary)) != 0)
    {
      ftDll.close();
      return Error::FreeTypeInitFailed;
    }

    ftInitialized = true;
    return Error::Ok;
  }

  void ftClose()
  {
    Hash<String32, FtFile*>::MutableIterator it(ftFileCache);
    for (it.toBegin(); it.isValid(); it.remove())
    {
      FtFile* ftFile = it.value();
      if (ftFile->refCount.get())
      {
        fog_stderr_msg("Fog::FontEngineFTPrivate", "ftClose", "FtFile not dereferenced (refCount %ld)", (long)ftFile->refCount.get());
      }

      delete ftFile;
    }

    if (ftInitialized)
    {
      pFT_Done_FreeType(ftLibrary);
    }
  }

  void ftInitTranslator()
  {
    // static font translation table
    static const char translatorDef[] =
      // SYNTAX: "Font Family\0" "fontfile.ttf\0"

      // add basic translators (corefonts gentoo package)
      "Andale Mono\0" "andalemo.ttf\0"
      "Arial\0" "arial.ttf\0"
      "Arial Black\0" "ariblk.ttf\0"
      "Comic Sans\0" "comic.ttf\0"
      "Comic Sans MS\0" "comic.ttf\0"
      "Courier\0" "cour.ttf\0"
      "Courier New\0" "cour.ttf\0"
      "Georgia\0" "georgia.ttf\0"
      "Impact\0" "impact.ttf\0"
      "Times\0" "times.ttf\0"
      "Times New Roman\0" "times.ttf\0"
      "Trebuchet\0" "trebuc.ttf\0"
      "Trebuchet MS\0" "trebuc.ttf\0"
      "Verdana\0" "verdana.ttf\0"

      // Bitstream Vera Sans
      "Bitstream Vera Sans\0" "Vera.ttf\0"
      "Bitstream Vera Sans Mono\0" "VeraMono.ttf\0"
      "Bitstream Vera Serif\0" "VeraSe.ttf\0"

      "Tahoma\0" "tahoma.ttf\0";

    StringCache* cache = StringCache::create(
      translatorDef,
      sizeof(translatorDef),
      DetectLength,
      String32(StubAscii8("fog_freetype_translators")));

    sysuint_t i, count = cache->count();

    for (i = 0; i < count; i += 2)
    {
      ftTranslator.append(FontEngineFTTranslator(cache->get(i), cache->get(i+1)));
    }
  }

  Vector<String32> ftGetFontDirectories()
  {
    Vector<String32> list;

    // Gentoo default font paths
    list.append(StubAscii8("/usr/share/fonts"));
    list.append(StubAscii8("/usr/share/fonts/TTF"));
    list.append(StubAscii8("/usr/share/fonts/corefonts"));
    list.append(StubAscii8("/usr/share/fonts/local"));
    list.append(StubAscii8("/usr/share/fonts/ttf-bitstream-vera"));
    list.append(StubAscii8("/usr/local/share/fonts"));
    list.append(StubAscii8("/usr/local/share/fonts/TTF"));
    list.append(StubAscii8("/usr/local/share/fonts/corefonts"));
    list.append(StubAscii8("/usr/local/share/fonts/local"));
    list.append(StubAscii8("/usr/local/share/fonts/ttf-bitstream-vera"));

    // Ubuntu default truetype font paths:
    list.append(StubAscii8("/usr/share/fonts/truetype/msttcorefonts"));

    // Please add more paths

    return list;
  }

  String32 ftResolveFontPath(const String32& family, uint32_t size, const FontAttributes& attributes)
  {
    Vector<FontEngineFTTranslator>::ConstIterator it(ftTranslator);
    String32 result;
    Vector<String32> fontPaths = Font::fontPaths();

    for (; it.isValid(); it.toNext())
    {
      if (it.value().family.eq(family, CaseInsensitive) &&
          FileSystem::findFile(fontPaths, it.value().file, result))
      {
        break;
      }
    }

    return result;
  }

  // freetype error handler
  void ftErrorHandler(FT_Error e)
  {
    fog_debug("Freetype error %u", e);
  }
};

// ============================================================================
// [Fog::FontEngineFT]
// ============================================================================

FontEngineFT::FontEngineFT() :
  FontEngine(StubAscii8("FreeType"))
{
  p = fepriv = new FontEngineFTPrivate();
}

FontEngineFT::~FontEngineFT()
{
  delete p;
}

Vector<String32> FontEngineFT::getFonts()
{
}

FontFace* FontEngineFT::getDefaultFace()
{
  FontAttributes a;
  memset(&a, 0, sizeof(FontAttributes));
  return getFace(String32(StubAscii8("arial")), 12, a);
}

FontFace* FontEngineFT::getFace(
  const String32& family, uint32_t size,
  const FontAttributes& attributes)
{
  String32 fileName = p->resolveFontPath(family, size, attributes);

  if (!fileName.isEmpty())
  {
    // first look if this file is in cache.
    FtFile* ftFile = p->ftFileCache.value(fileName, NULL);

    if (!ftFile)
    {
      ftFile = new(std::nothrow) FtFile(fileName, family);

      // Ensure that ftFile is OK.
      if (!ftFile) return NULL;

      if (ftFile->loadError)
      {
        delete ftFile;
        return NULL;
      }

      // Put to cache, reference count is now 1 thats correct.
      fepriv->ftFileCache.put(fileName, ftFile->ref());
    }

    // If face was created, reference count is increased too.
    return ftFile->createFace(size, attributes);
  }

  return NULL;
}

// ============================================================================
// [Fog::FontFaceFT]
// ============================================================================

FontFaceFT::FontFaceFT() :
  ftFile(NULL)
{
}

FontFaceFT::~FontFaceFT()
{
  if (ftFile) ftFile->deref();
}

void FontFaceFT::deref()
{
  if (refCount.deref()) delete this;
}

err_t FontFaceFT::getGlyphs(const Char32* str, sysuint_t length, GlyphSet& glyphSet)
{
  AutoLock locked(lock);

  Glyph::Data* glyphd;
  TextWidth tw;
  memset(&tw, 0, sizeof(TextWidth));

  sysuint_t i;

  for (i = 0; i != length; i++)
  {
    uint32_t uc = str[i].ch();

    // First try cache
    glyphd = glyphCache.get(uc);
    if (FOG_UNLIKELY(!glyphd))
    {
      if ((glyphd = renderGlyph(uc))) glyphCache.set(uc, glyphd);
    }

    if (FOG_LIKELY(glyphd)) glyphSet._add(glyphd->ref());
  }
}

Glyph::Data* FontFaceFT::renderGlyph(uint32_t uc)
{
  Glyph::Data* glyphd = NULL;

  // if glyph is not in cache, it's needed to render it
  FT_Face ftFace;
  FT_Error ftError;

  FT_GlyphSlot ftGlyphSlot;
  FT_Glyph_Metrics *ftMetrics;
  FT_Outline *ftOutline;
  FT_Bitmap *ftBitmap;

  uint y;
  uint index;

  ftFile->use();

  ftFace = ftFile->ftFace;
  // setup font size (mainly for scalable fonts)
  ftFile->setupSize(metrics.size);

  // get glyph index
  if ((index = fepriv->pFT_Get_Char_Index(ftFace, uc)) == 0) goto fail;
  // try to load the glyph
  if ((ftError = fepriv->pFT_Load_Glyph(ftFace, index, FT_LOAD_DEFAULT)) != 0) goto fail;

  // get glyph shortcuts
  ftGlyphSlot = ftFace->glyph;
  ftMetrics = &ftGlyphSlot->metrics;
  ftOutline = &ftGlyphSlot->outline;

  {
    int offsetX;
    int offsetY;
    int advance;
    int width = 0;

    // calculate offsets for X and Y coordinates.
    // freetype returns only bitmap/pixmap that's needed to
    // sucessfully render glyph with anti-aliasing or without,
    // space is never larger, so we must use these values and
    // we are using them in our rendering process too.
    offsetX = FT_FLOOR(ftMetrics->horiBearingX);
    offsetY = metrics.ascent - FT_FLOOR(ftMetrics->horiBearingY) - 1;
    advance = FT_CEIL(ftMetrics->horiAdvance);

    /* Adjust for bold and italic text */
    //if (Param->Weight >= 5) Width += int(m->GlyphOverhang);

    // we can italize normal font if italic variant is not present.
    /*
    if ((ftFace->style_flags & FT_STYLE_FLAG_ITALIC) == 0 &&
      (attributes & Font::Attribute_Italic) != 0)
    {
      FT_Matrix shear;

      Width += (int)ceil(m->GlyphItalics);

      shear.xx = 1 << 16;
      shear.xy = (int)(m->GlyphItalics * ( 1 << 16 ) ) / m->Height;
      shear.yx = 0;
      shear.yy = 1 << 16;

      FT_Outline_Transform(ftOutline, &shear);
    }
    */

    if ((ftError = fepriv->pFT_Render_Glyph(ftGlyphSlot, FT_RENDER_MODE_NORMAL)) != 0)
    {
      goto fail;
    }

    ftBitmap = &ftGlyphSlot->bitmap;
    width += ftBitmap->width;

    glyphd = new(std::nothrow) Glyph::Data();
    if (glyphd->image.create(width, ftGlyphSlot->bitmap.rows, Image::FormatA8) != Error::Ok)
    {
      goto fail;
    }

    glyphd->offsetX = offsetX;
    glyphd->offsetY = offsetY;
    glyphd->beginWidth = 0;
    glyphd->endWidth = 0;
    glyphd->advance = advance;
  }

  {
    // copy FT_Bitmap to our image and clean bytes over width
    sysuint_t p, pCount = glyphd->image.stride() - ftBitmap->width;

    uint8_t *dstPtr = glyphd->image._d->first;
    const uint8_t *srcPtr = ftBitmap->buffer;

    for (y = 0; y != (uint)ftBitmap->rows; y++)
    {
      if (ftBitmap->pixel_mode == FT_PIXEL_MODE_MONO)
      {
        // 1 bit mono fonts are always rendered as 1 bit images, so
        // it's needed to convert these manually.
        uint x;
        uint pix8;

        uint8_t* dstCur = dstPtr;
        const uint8_t* srcCur = srcPtr;

        for (x = 0; x != (uint)ftBitmap->width; x++)
        {
          if ((x & 7) == 0) pix8 = *srcCur++;
          *dstCur++ = (pix8 & 0x80) ? 0xFF : 0x00;
          pix8 <<= 1;
        }
      }
      else
      {
        memcpy(dstPtr, srcPtr, ftBitmap->width);
      }

      dstPtr += ftBitmap->width;
      srcPtr += ftBitmap->pitch;

      p = pCount;
      while (p--) *dstPtr++ = 0;
    }
  }

  /* Handle the bold style ? */
  /*
  if (Param->Weight > WPP_Text_Weight_Normal && g.Width())
  {
    // For conversion between floating point is better signed number
    int Offset;
    uint8_t *Data;

    // The pixmap is a little hard, we have to add and clamp
    for (Y = 0; Y != g.Height(); Y++)
    {
      Data = (uint8_t *)g.Data()->Pixels + Y * g.Stride();
      for (Offset = 1; Offset <= m->GlyphOverhang; Offset++)
      {
        for (X = g.Width() - 1; X != 0; X--)
        {
          uint Pixel = uint(Data[X]) + uint(Data[X-1]);
          if (Pixel > 0xFF) Pixel = 0xFF;
          Data[X] = uint8_t(Pixel);
        }
      }
    }
  }
  */

#if 0
  // debug
  {
    fprintf(stderr, "GLYPH: '%lc' (width=%u, height=%u, stride=%ld, advance=%d, offx=%d, offy=%d)\n",
      ch,
      imaged->_width,
      imaged->_height,
      imaged->_stride,
      glyphd->_advance,
      glyphd->_offsetX,
      glyphd->_offsetY);

    for (y = 0; y != imaged->_height; y++) {
      for (x = 0; x != imaged->_width; x++) {
        fprintf(stderr, "%.2X ", imaged->_base[y * imaged->_stride + x]);
      }
      fprintf(stderr, "\n");
    }
    fprintf(stderr, "\n");
  }
#endif

fail:
  ftFile->unuse();
  return glyphd;
}

err_t FontFaceFT::getTextWidth(const Char32* str, sysuint_t length, TextWidth* textWidth)
{
  TemporaryGlyphSet<128> glyphSet;
  err_t err = getGlyphs(str, length, glyphSet);

  if (err)
  {
    memset(textWidth, 0, sizeof(TextWidth));
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

// ============================================================================
// [Fog::FtFile]
// ============================================================================

FtFile::FtFile(const String32& fileName, const String32& family) :
  fileName(fileName),
  family(family),
  ftFace(NULL),
  loadError(0),
  unloadAfterUse(false),
  fixedWidthIndex(0)
{
  //fog_debug("FtFile::new()");
  refCount.init(0);
  used.init(0);
  load();
}

FtFile::~FtFile()
{
  //fog_debug("FtFile::delete()");
  FOG_ASSERT((refCount.get() == 0 && used.get() == 0));
  unload();
}

FtFile* FtFile::ref()
{
  //fog_debug("FtFile::ref() - RefCount=%lld", (long long)refCount.get());
  refCount.inc();;
  return this;
}

void FtFile::deref()
{
  //fog_debug("FtFile::deref() - RefCount=%lld", (long long)refCount.get());
  refCount.dec();
}

// use() and unuse() are public methods that's used by engine.
bool FtFile::use()
{
  //fog_debug("FtFile::use()");
  used.inc();
  if (used.get() == 1 && !loaded() && !load())
  {
    used.dec();
    return false;
  }
  else
    return true;
}

void FtFile::unuse()
{
  //fog_debug("FtFile::unuse()");
  if (used.deref() && unloadAfterUse) unload();
}

bool FtFile::load()
{
  if (ftFace) return true;

  // Try to mmap file, if this fail, use FT_New_Face function that will
  // to open it manually.
  if (mapFile.map(fileName, false) == Error::Ok)
  {
    FT_Open_Args ftArgs;

    memset(&ftArgs, 0, sizeof(FT_Open_Args));
    ftArgs.memory_base = (const FT_Byte*)mapFile.data();
    ftArgs.memory_size = mapFile.size();
    ftArgs.flags = FT_OPEN_MEMORY;

    loadError = fepriv->pFT_Open_Face(fepriv->ftLibrary, &ftArgs, 0, &ftFace);
  }
  else
  {
    TemporaryString8<TemporaryLength> fileName8;
    fileName8.set(fileName, TextCodec::local8());

    loadError = fepriv->pFT_New_Face(fepriv->ftLibrary, fileName8.cStr(), 0, &ftFace);
  }

  return loadError == 0;
}

void FtFile::unload()
{
  FOG_ASSERT(used.get() == 0);

  if (ftFace)
  {
    fepriv->pFT_Done_Face(ftFace);
    ftFace = NULL;
  }
}

FOG_INLINE bool FtFile::loaded() const
{
  return ftFace != NULL;
}

bool FtFile::setupSize(uint32_t size)
{
  FT_Error error = 0;
  FOG_ASSERT(loaded());

  // Make sure that font face is scalable
  if (FT_IS_SCALABLE(ftFace))
  {
    // Set the character size and use default DPI (72)
    error = fepriv->pFT_Set_Char_Size(ftFace, 0, size * 64, 0, 0);
  }
  else
  {
    uint i;
    uint fs = ftFace->num_fixed_sizes;

    fixedWidthIndex = 0;

    for (i = 0; i != fs; i++)
    {
      uint32_t s = (ftFace->available_sizes[i].size + 31) >> 6;

      if (s == size)
      {
        error = fepriv->pFT_Set_Pixel_Sizes(
          ftFace,
          ftFace->available_sizes[i].width,
          ftFace->available_sizes[i].height);
        fixedWidthIndex = i;
        goto done;
      }
    }

    return false;
  }
done:
  return error == 0;
}

FontFace* FtFile::createFace(uint32_t size, const FontAttributes& attributes)
{
  FontFaceFT* face = NULL;

  use();
  if (setupSize(size))
  {
    // freetype not returned an error, so create new face and fill it
    face = new FontFaceFT();
    face->ftFile = this;
    face->family = family;
    face->metrics.size = size;
    face->attributes = attributes;

    if (FT_IS_SCALABLE(ftFace))
    {
      // Get the scalable font metrics for this font
      FT_Fixed scale = ftFace->size->metrics.y_scale;

      face->metrics.height = FT_CEIL(fepriv->pFT_MulFix(ftFace->bbox.yMax - ftFace->bbox.yMin, scale)) - 1;
      face->metrics.ascent = (FT_CEIL(fepriv->pFT_MulFix(ftFace->bbox.yMax, scale)));
      face->metrics.descent = face->metrics.height - face->metrics.ascent;

      /*
      face->metrics.lineSkip = FT_CEIL(fepriv->pFT_MulFix(ftFace->height, scale));
      face->metrics.underlineOffset = FT_FLOOR(fepriv->pFT_MulFix(ftFace->underline_position, scale));
      face->metrics.underlineHeight = FT_FLOOR(fepriv->pFT_MulFix(ftFace->underline_thickness, scale));
      if (face->metrics.underlineHeight < 1) face->metrics.underlineHeight = 1;

      face->metrics.glyphOverhang = ftFace->size->metrics.y_ppem / 10;
      face->metrics.glyphItalics = 0.207f * float(face->metrics.height);
      */
    }
    else
    {
      // This is bitmapped font. I don't know why freetype2 library
      // handles differently different font files, but this workaround
      // should be smart.

      face->metrics.height = ftFace->available_sizes[fixedWidthIndex].height;
      face->metrics.ascent = FT_FLOOR(ftFace->available_sizes[fixedWidthIndex].y_ppem);
      if (face->metrics.ascent == face->metrics.height)
        face->metrics.ascent = FT_FLOOR(ftFace->size->metrics.ascender);
      face->metrics.descent = face->metrics.height - face->metrics.ascent;

      /*
      face->metrics.lineSkip = FT_CEIL(face->metrics.ascent);
      face->metrics.underlineOffset = FT_FLOOR(ftFace->underline_position);
      face->metrics.underlineHeight = FT_FLOOR(ftFace->underline_thickness);
      face->metrics.underlineHeight < 1) face->metrics.underlineHeight = 1;

      face->metrics.glyphOverhang = ftFace->size->metrics.y_ppem / 10;
      face->metrics.glyphItalics = 0.207f * (float)face->metrics.height;
      */
    }

    //Fog_FontFaceCache_put(face);
  }

  unuse();
  return face;
}

// ---------------------------------------------------------------------------
// freetype font list
// ---------------------------------------------------------------------------
/*
static void Fog_Font_ftAppendFontsToList(Vector<String32>* list)
{
  if (Fog_FontConfig)
  {
    list->append(Fog_FontConfig->fontsList());
  }
  else
  {
    // fontconfig is not present are we should to locate fonts manually.
    // this is not preferred case and this is only small workaround.

    // Make common fonts list, but check if they are available
    Hash<String32, String32>::ConstIterator iterator(Fog_Ft->ftTranslator);
    String32 path;

    for (iterator.begin(); iterator.exist(); iterator.next())
    {
      if (Font::findFontFile(iterator.value(), path))
      {
        list->append(iterator.key());
      }
    }
  }
}
*/

} // Fog namespace

#endif // FOG_FONT_FREETYPE
