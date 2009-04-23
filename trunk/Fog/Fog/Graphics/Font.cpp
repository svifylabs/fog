// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/AutoLock.h>
#include <Fog/Core/FileSystem.h>
#include <Fog/Core/FileUtil.h>
#include <Fog/Core/Hash.h>
#include <Fog/Core/Lock.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/OS.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/String.h>
#include <Fog/Core/UserInfo.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Font.h>
#include <Fog/Graphics/Font_FreeType.h>
#include <Fog/Graphics/Font_Win.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/Rgba.h>

namespace Fog {

// ============================================================================
// [Fog::Font_Local]
// ============================================================================

struct Font_Local
{
  Lock lock;
  Vector<String32> paths;
  Vector<String32> list;
  bool listInitialized;
};

static Static<Font_Local> font_local;

// ============================================================================
// [Fog::Glyph]
// ============================================================================

Fog::Static<Glyph::Data> Glyph::sharedNull;

// ============================================================================
// [Fog::Glyph::Data]
// ============================================================================

Glyph::Data::Data() :
  offsetX(0),
  offsetY(0),
  beginWidth(0),
  endWidth(0),
  advance(0)
{
  refCount.init(1);
}

Glyph::Data::~Data()
{
}

// ============================================================================
// [Fog::GlyphCache]
// ============================================================================

GlyphCache::GlyphCache() :
  _rows(0),
  _count(0)
{
}

GlyphCache::~GlyphCache()
{
  free();
}

bool GlyphCache::set(uint32_t uc, Entity data)
{
  // First reject too high character
  if (uc > 65535) return false;

  uint32_t ucRow = uc >> 8;
  uint32_t ucCol = uc & 0xFF;

  // realloc ROWS array if needed
  if (FOG_UNLIKELY(ucRow >= _count))
  {
    _rows = (Entity**)Fog::Memory::xrealloc(_rows, (ucRow + 1) * sizeof(Entity));
    Fog::Memory::zero(_rows + _count, (ucRow - _count + 1) * sizeof(Entity));
    _count = ucRow + 1;
  }

  // alloc COL array if needed
  if (FOG_UNLIKELY(_rows[ucRow] == NULL))
  {
    _rows[ucRow] = (Entity*)Fog::Memory::xcalloc(256 * sizeof(Entity));
  }

  _rows[ucRow][ucCol] = data->ref();
  return true;
}

void GlyphCache::free()
{
  sysuint_t i, j;
  sysuint_t count = _count;
  Entity** rows = _rows;
  Entity* row;

  for (i = 0; i != count; i++, rows++)
  {
    if ((row = *rows) != NULL)
    {
      for (j = 0; j != 256; j++)
      {
        if (row[j]) row[j]->deref();
      }
      Fog::Memory::free(row);
    }
  }
  Fog::Memory::free(_rows);

  _rows = 0;
  _count = 0;
}

// ============================================================================
// [Fog::FontFace]
// ============================================================================

FontFace::FontFace()
{
  memset(&metrics, 0, sizeof(FontMetrics));
}

FontFace::~FontFace()
{
}

FontFace* FontFace::ref()
{
  refCount.inc();
  return this;
}

// ============================================================================
// [Fog::Font]
// ============================================================================

Font::Data* Font::sharedNull;
FontEngine* Font::_engine;

static bool _setFace(Font* self, FontFace* face)
{
  if (!face) return false;
  if (self->_d->face == face) return true;

  self->detach();
  self->_d->face->deref();
  self->_d->face = face;

  return true;
}

Font::Font() :
  _d(sharedNull->ref())
{
}

Font::Font(Data* d) :
  _d(d)
{
}

Font::Font(const Font& other) :
  _d(other._d->ref())
{
}

Font::~Font()
{
  _d->deref();
}

void Font::_detach()
{
  AtomicBase::ptr_setXchg(&_d, Data::copy(_d))->deref();
}

void Font::free()
{
  AtomicBase::ptr_setXchg(&_d, sharedNull->ref())->deref();
}

bool Font::setFamily(const String32& family)
{
  return _setFace(this, _engine->getFace(family, size(), attributes()));
}

bool Font::setFamily(const String32& family, uint32_t size)
{
  return _setFace(this, _engine->getFace(family, size, attributes()));
}

bool Font::setSize(uint32_t size)
{
  return _setFace(this, _engine->getFace(family(), size, attributes()));
}

bool Font::setAttributes(const FontAttributes& a)
{
  return _setFace(this, _engine->getFace(family(), size(), a));
}

bool Font::setBold(bool val)
{
  if (isBold() == val) return true;

  FontAttributes a = attributes();
  a.bold = val;

  return _setFace(this, _engine->getFace(family(), size(), a));
}

bool Font::setItalic(bool val)
{
  if (isItalic() == val) return true;

  FontAttributes a = attributes();
  a.italic = val;

  return _setFace(this, _engine->getFace(family(), size(), a));
}

bool Font::setStrike(bool val)
{
  if (isStrike() == val) return true;

  FontAttributes a = attributes();
  a.strike = val;

  return _setFace(this, _engine->getFace(family(), size(), a));
}

bool Font::setUnderline(bool val)
{
  if (isUnderline() == val) return true;

  FontAttributes a = attributes();
  a.underline = val;

  return _setFace(this, _engine->getFace(family(), size(), a));
}

const Font& Font::set(const Font& other)
{
  AtomicBase::ptr_setXchg(&_d, other._d->ref())->deref();
  return *this;
}

void Font::getTextWidth(
  const String32& text, 
  TextWidth* textWidth) const
{
  _d->face->getTextWidth(text.cData(), text.length(), textWidth);
}

void Font::getTextWidth(
  const Char32* text, sysuint_t length, 
  TextWidth* textWidth) const
{
  _d->face->getTextWidth(text, length, textWidth);
}

void Font::getGlyphs(
  const Char32* str, sysuint_t length,
  Glyph* glyphs, TextWidth* textWidth) const
{
  _d->face->getGlyphs(str, length, glyphs, textWidth);
}

bool Font::addFontPath(const String32& path)
{
  AutoLock locked(font_local.instance().lock);

  if (!font_local.instance().paths.contains(path) &&
      FileSystem::isDirectory(path))
  {
    font_local.instance().paths.append(path);
    return true;
  }
  else
    return false;
}

void Font::addFontPaths(const Sequence<String32>& paths)
{
  AutoLock locked(font_local.instance().lock);

  Sequence<String32>::ConstIterator it(paths);
  for (; it.isValid(); it.toNext())
  {
    String32 path = it.value();
    if (!font_local.instance().paths.contains(path) &&
        FileSystem::isDirectory(path))
    {
      font_local.instance().paths.append(path);
    }
  }
}

bool Font::removeFontPath(const String32& path)
{
  AutoLock locked(font_local.instance().lock);
  return font_local.instance().paths.remove(path) != 0;
}

bool Font::hasFontPath(const String32& path)
{
  AutoLock locked(font_local.instance().lock);
  return font_local.instance().paths.contains(path);
}

bool Font::findFontFile(const String32& fileName, String32& dest)
{
  AutoLock locked(font_local.instance().lock);
  return FileSystem::findFile(font_local.instance().paths, fileName, dest);
}

Vector<String32> Font::fontPaths()
{
  AutoLock locked(font_local.instance().lock);
  return font_local.instance().paths;
}

Vector<String32> Font::fontList()
{
  AutoLock locked(font_local.instance().lock);
  if (!font_local.instance().listInitialized)
  {
    font_local.instance().list.append(_engine->getFonts());
    font_local.instance().listInitialized = true;
  }
  return font_local.instance().list;
}

// ============================================================================
// [Font::Data]
// ============================================================================

Font::Data::Data() :
  face(0)
{
  refCount.init(1);
}

Font::Data::~Data()
{
  if (face) face->deref();
}

Font::Data* Font::Data::copy(Data* d)
{
  Data* newd = new Data();
  newd->face = d->face ? d->face->ref() : 0;
  return newd;
}

// ============================================================================
// [Fog::FontEngine]
// ============================================================================

FontEngine::FontEngine(const Fog::String32& name) :
  _name(name)
{
}

FontEngine::~FontEngine()
{
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_EXTERN void fog_font_shutdown(void);

FOG_INIT_DECLARE err_t fog_font_init(void)
{
  // [Local]

  Fog::font_local.init();
  Fog::font_local.instance().listInitialized = false;

  uint initResult = Error::Ok;

  // [Glyph Shared Null]

  Fog::Glyph::sharedNull.init();

  // [Font Shared Null]

  Fog::Font::sharedNull = new Fog::Font::Data();

  // [Font Paths]

  // add $HOME and $HOME/fonts directories (standard in linux)
  // (can be for example symlink to real font path)
  Fog::String32 home = Fog::UserInfo::directory(Fog::UserInfo::Home);
  Fog::String32 homeFonts;

  Fog::FileUtil::joinPath(homeFonts, home, Fog::StubAscii8("fonts"));

  Fog::font_local.instance().paths.append(home);
  Fog::font_local.instance().paths.append(homeFonts);

#if defined(FOG_OS_WINDOWS)
  // Add Windows standard font directory.
  Fog::String32 winFonts = Fog::OS::windowsDirectory();
  Fog::FileUtil::joinPath(winFonts, winFonts, Fog::StubAscii8("fonts"));
  Fog::font_local.instance().paths.append(winFonts);
#endif // FOG_OS_WINDOWS

  // [Font Face Cache]

  // [Font Engine]

  Fog::Font::_engine = NULL;

#if defined(FOG_FONT_WINDOWS)
  if (!Fog::Font::_engine) Fog::Font::_engine = new Fog::FontEngineWin();
#endif // FOG_FONT_WINDOWS

#if defined(FOG_FONT_FREETYPE)
  if (!Fog::Font::_engine) Fog::Font::_engine = new Fog::FontEngineFT();
#endif // FOG_FONT_FREETYPE

  Fog::Font::sharedNull->face = Fog::Font::_engine->getDefaultFace();

  if (Fog::Font::sharedNull->face == NULL)
  {
    initResult = Fog::EFontCantLoadDefaultFace;
    goto __fail;
  }

  return initResult;

__fail:
  fog_font_shutdown();
  return initResult;
}

FOG_INIT_DECLARE void fog_font_shutdown(void)
{
  // [Font Shared Null]
  delete Fog::Font::sharedNull;
  Fog::Font::sharedNull = NULL;

  // [Glyph Shared Null]

  Fog::Glyph::sharedNull.instancep()->refCount.dec();
  Fog::Glyph::sharedNull.destroy();

  // [Font Face Cache]

  //Fog_Font_cleanup();
  //Fog_FontFace_cache->~Fog_FontFaceCache();

  // [Font Engine]

  if (Fog::Font::_engine)
  {
    delete Fog::Font::_engine;
    Fog::Font::_engine = NULL;
  }

  // [Local]

  Fog::font_local.destroy();
}






































































































#if 0









// mutex
// static Fog::Static<Fog::Lock> Fog_Font_mutex;

// Fog_Font_paths
// static uint8_t Fog_Font_paths_storage[sizeof(Fog::StringList)];
// static Fog::StringList* Fog_Font_paths;

// Fog_Font_list
// static uint8_t Fog_Font_list_storage[sizeof(Fog::StringList)];
// static Fog::StringList* Fog_Font_list;
// static bool Fog_Font_listInitialized = 0;

// ---------------------------------------------------------------------------
// Fog_FontFaceCache
// ---------------------------------------------------------------------------

struct Fog_FontFaceCacheEntry
{
  /*! @brief Font family name. */
  Fog::String32 _family;
  /*! @brief Font size. */
  uint32_t _size;
  /*! @brief Font attributes. Only BOLD and ITALIC!. */
  uint32_t _attributes;

  FOG_INLINE Fog_FontFaceCacheEntry() {}
  FOG_INLINE Fog_FontFaceCacheEntry(const Fog::String32& family, uint32_t size, uint32_t attributes)
    : _family(family), _size(size), _attributes(attributes & Fog::Font::Attribute_FontFaceMask) {}
  FOG_INLINE Fog_FontFaceCacheEntry(const Fog_FontFaceCacheEntry& other)
    : _family(other.family()), _size(other.size()), _attributes(other.attributes()) {}
  FOG_INLINE ~Fog_FontFaceCacheEntry() {}

  FOG_INLINE const Fog::String32& family() const { return _family; }
  FOG_INLINE uint32_t size() const { return _size; }
  FOG_INLINE uint32_t attributes() const { return _attributes; }
};

FOG_DECLARE_TYPEINFO(Fog_FontFaceCacheEntry, Fog::MoveableType)

static FOG_INLINE bool operator==(const Fog_FontFaceCacheEntry &a, const Fog_FontFaceCacheEntry &b)
{
  return a.family() == b.family() && a.size() == b.size() && a.attributes() == b.attributes();
}

static FOG_INLINE uint32_t Wde_hash(const Fog_FontFaceCacheEntry& entry)
{
  return Wde_hash(entry.family()) ^ (entry.size() * 133) ^ entry.attributes();
}

typedef Fog::Hash<Fog_FontFaceCacheEntry, Fog_FontFace*> Fog_FontFaceCache;

// font face cache storage
static uint8_t Fog_FontFace_cache_storage[sizeof(Fog_FontFaceCache)];
static Fog_FontFaceCache *Fog_FontFace_cache;

// returns a font face from cache
static Fog_FontFace* Fog_FontFaceCache_get(const Fog::String32& family, uint32_t size, uint32_t attributes)
{
  Fog::AutoLock locked(local.instance().cs);

  Fog_FontFaceCache::Node* node;
  Fog_FontFace* result = NULL;
  
  if ((node = Fog_FontFace_cache->get(Fog_FontFaceCacheEntry(family, size, attributes))) != NULL)
  {
    result = node->value();
    result->refCount.inc();
  }

  return result;
}

// puts a font face into cache
static void Fog_FontFaceCache_put(Fog_FontFace* face)
{
  Fog::AutoLock locked(local.instance().cs);

  Fog_FontFace_cache->put(Fog_FontFaceCacheEntry(face->family, face->metrics.size, face->attributes), face);
}

// cleans all font faces in cache that have reference count 0
static void Fog_FontFaceCache_cleanup(void)
{
  Fog::AutoLock locked(local.instance().cs);

  Fog_FontFaceCache::MutableIterator iterator(*Fog_FontFace_cache);
  for (iterator.begin(); iterator.exist();)
  {
    Fog_FontFace* face = iterator.value();
    if (face->refCount.get() == 0)
    {
      delete face;
      iterator.removeAndNext();
    }
    else
      iterator.next();
  }
}















// ---------------------------------------------------------------------------
// Fog::Font
// ---------------------------------------------------------------------------

// variaous text width functions
FOG_CAPI_DECLARE void Fog_Font_getTextWidth(const Fog::Font* self, const Fog::Char* text, sysuint_t count, Fog::TextWidth* textWidth)
{
  if (count)
  {
    Fog_Glyphs glyphs;
    glyphs.getGlyphs(*self, text, count);

    textWidth->beginWidth = glyphs.glyphs()[0].beginWidth();
    textWidth->advance = glyphs.advance();
    textWidth->endWidth = glyphs.glyphs()[count-1].endWidth();
  }
  else
  {
    textWidth->beginWidth = 0;
    textWidth->advance = 0;
    textWidth->endWidth = 0;
  }
}

FOG_CAPI_DECLARE void Fog_Font_getFontGlyphsWidth(const Fog::Font* self, const Fog::Glyph* glyphs, sysuint_t count, Fog::TextWidth* textWidth)
{
  if (count)
  {
    sysuint_t i;
    int advance = 0;

    for (i = 0; i != count; i++)
    {
      advance += glyphs[i].advance();
    }

    textWidth->beginWidth = glyphs[0].beginWidth();
    textWidth->advance = advance;
    textWidth->endWidth = glyphs[count-1].endWidth();
  }
  else
  {
    textWidth->beginWidth = 0;
    textWidth->advance = 0;
    textWidth->endWidth = 0;
  }
}






// ---------------------------------------------------------------------------
// font cleanup
// ---------------------------------------------------------------------------

FOG_CAPI_DECLARE void Fog_Font_cleanup(void)
{
  Fog_FontFaceCache_cleanup();
}
#endif
