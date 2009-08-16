// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled Headers]
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
#include <Fog/Graphics/Error.h>
#include <Fog/Graphics/Font.h>
#include <Fog/Graphics/Font_FreeType.h>
#include <Fog/Graphics/Font_Win.h>
#include <Fog/Graphics/Glyph.h>
#include <Fog/Graphics/GlyphCache.h>
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
// [Fog::FontFace]
// ============================================================================

FontFace::FontFace()
{
  refCount.init(1);
  memset(&metrics, 0, sizeof(FontMetrics));
  memset(&attributes, 0, sizeof(FontAttributes));
}

FontFace::~FontFace()
{
}

FontFace* FontFace::ref()
{
  refCount.inc();
  return this;
}

void FontFace::deref()
{
  if (refCount.deref()) delete this;
}

// ============================================================================
// [Fog::FontCache]
// ============================================================================

FontCache::FontCache()
{
}

FontCache::~FontCache()
{
  deleteAll();
}

FontFace* FontCache::getFace(const String32& family, uint32_t size, const FontAttributes& attrs)
{
  Entry entry(family, size, attrs);
  AutoLock locked(_lock);

  FontFace* face = _cache.value(entry, NULL);
  if (face) face = face->ref();
  return face;
}

err_t FontCache::putFace(FontFace* face)
{
  Entry entry(face->family, face->metrics.size, face->attributes);
  AutoLock locked(_lock);

  return _cache.put(entry, face, false);
}

void FontCache::deleteAll()
{
  AutoLock locked(_lock);
  Hash<Entry, FontFace*>::MutableIterator it(_cache);

  for (it.toStart(); it.isValid(); it.remove()) it.value()->deref();
}

// ============================================================================
// [Fog::Font]
// ============================================================================

Font::Data* Font::sharedNull;
FontCache* Font::_cache;
FontEngine* Font::_engine;

static err_t _setFace(Font* self, FontFace* face)
{
  if (!face) return Error::FontInvalidFace;
  if (self->_d->face == face) return Error::Ok;

  err_t err = self->detach();
  if (err) return err;

  if (self->_d->face) self->_d->face->deref();
  self->_d->face = face;

  return Error::Ok;
}

Font::Font() :
  _d(sharedNull->ref())
{
}

Font::Font(const Font& other) :
  _d(other._d->ref())
{
}

Font::Font(Data* d) :
  _d(d)
{
}

Font::~Font()
{
  _d->deref();
}

err_t Font::_detach()
{
  if (_d->refCount.get() > 1)
  {
    Data* newd = Data::copy(_d);
    if (!newd) return Error::OutOfMemory;
    AtomicBase::ptr_setXchg(&_d, newd)->deref();
  }
  return Error::Ok;
}

void Font::free()
{
  AtomicBase::ptr_setXchg(&_d, sharedNull->ref())->deref();
}

err_t Font::setFamily(const String32& family)
{
  return _setFace(this, _engine->cachedFace(family, size(), attributes()));
}

err_t Font::setFamily(const String32& family, uint32_t size)
{
  return _setFace(this, _engine->cachedFace(family, size, attributes()));
}

err_t Font::setSize(uint32_t size)
{
  return _setFace(this, _engine->cachedFace(family(), size, attributes()));
}

err_t Font::setAttributes(const FontAttributes& a)
{
  return _setFace(this, _engine->cachedFace(family(), size(), a));
}

err_t Font::setBold(bool val)
{
  if (isBold() == val) return Error::Ok;

  FontAttributes a = attributes();
  a.bold = val;

  return _setFace(this, _engine->cachedFace(family(), size(), a));
}

err_t Font::setItalic(bool val)
{
  if (isItalic() == val) return Error::Ok;

  FontAttributes a = attributes();
  a.italic = val;

  return _setFace(this, _engine->cachedFace(family(), size(), a));
}

err_t Font::setStrike(bool val)
{
  if (isStrike() == val) return Error::Ok;

  FontAttributes a = attributes();
  a.strike = val;

  return _setFace(this, _engine->cachedFace(family(), size(), a));
}

err_t Font::setUnderline(bool val)
{
  if (isUnderline() == val) return Error::Ok;

  FontAttributes a = attributes();
  a.underline = val;

  return _setFace(this, _engine->cachedFace(family(), size(), a));
}

err_t Font::set(const Font& other)
{
  AtomicBase::ptr_setXchg(&_d, other._d->ref())->deref();
  return Error::Ok;
}

err_t Font::getTextWidth(const String32& str, TextWidth* textWidth) const
{
  return _d->face->getTextWidth(str.cData(), str.getLength(), textWidth);
}

err_t Font::getTextWidth(const Char32* str, sysuint_t length, TextWidth* textWidth) const
{
  return _d->face->getTextWidth(str, length, textWidth);
}

err_t Font::getGlyphs(const Char32* str, sysuint_t length, GlyphSet& glyphSet) const
{
  return _d->face->getGlyphs(str, length, glyphSet);
}

err_t Font::getPath(const Char32* str, sysuint_t length, Path& dst) const
{
  return _d->face->getPath(str, length, dst);
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

Font::Data::Data()
{
  refCount.init(1);
  face = NULL;
}

Font::Data::~Data()
{
  if (face) face->deref();
}

Font::Data* Font::Data::ref() const
{
  refCount.inc();
  return const_cast<Data*>(this);
}

void Font::Data::deref()
{
  if (refCount.deref()) delete this;
}

Font::Data* Font::Data::copy(Data* d)
{
  Data* newd = new(std::nothrow) Data();
  if (!newd) return NULL;

  newd->face = d->face ? d->face->ref() : NULL;
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

FontFace* FontEngine::cachedFace(
  const String32& family, uint32_t size, 
  const FontAttributes& attributes)
{
  FontFace* face;
  
  // Create to get font face from global cache.
  face = Font::_cache->getFace(family, size, attributes);
  if (face) return face->ref();

  // If needed face is not in cache, we will try to create it.
  face = createFace(family, size, attributes);
  if (!face) return NULL;

  // Now we created font face that will be put into cache.
  err_t err = Font::_cache->putFace(face);
  if (err == Error::Ok)
  {
    // Everything is OK, our face is now in cache and we need to increase reference
    // count if we want to use it - cachedFace() and getFace() returns font face
    // that can be used (so minimal reference count value is 1).
    return face->ref();
  }

  // This can happen if ther is concurrency. We created font face that was created
  // by another thread and put into cache. Everything we need is to delete our one
  // and use font face that is in cache already.
  //
  // Second case is that engine created face that is close to demanded one but not
  // equal.
  else if (err == Error::ObjectAlreadyExists)
  {
    // We are using values from font face, not family, size and attributes given to
    // this function.
    FontFace* other = Font::_cache->getFace(face->family, face->metrics.size, face->attributes);
    if (!other) return face;

    // Dereference our font face and use cached one.
    face->deref();
    return other->ref();
  }
  else
  {
    // Failed? Maybe memory allocation error? Just use this face.
    return face;
  }
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_EXTERN void fog_font_shutdown(void);

FOG_INIT_DECLARE err_t fog_font_init(void)
{
  using namespace Fog;

  // [Local]

  font_local.init();
  font_local.instance().listInitialized = false;

  err_t err = Error::Ok;

  // [Font Shared Null]

  Font::sharedNull = new Font::Data();

  // [Font Paths]

  // add $HOME and $HOME/fonts directories (standard in linux)
  // (can be for example symlink to real font path)
  String32 home = UserInfo::directory(UserInfo::Home);
  String32 homeFonts;

  FileUtil::joinPath(homeFonts, home, Ascii8("fonts"));

  font_local.instance().paths.append(home);
  font_local.instance().paths.append(homeFonts);

#if defined(FOG_OS_WINDOWS)
  // Add Windows standard font directory.
  String32 winFonts = OS::getWindowsDirectory();
  FileUtil::joinPath(winFonts, winFonts, Ascii8("fonts"));
  font_local.instance().paths.append(winFonts);
#endif // FOG_OS_WINDOWS

  // [Font Cache]

  Font::_cache = new(std::nothrow) FontCache();
  if (Font::_cache == NULL) { err = Error::OutOfMemory; goto fail; }

  // [Font Engine]

#if defined(FOG_FONT_WINDOWS)
  if (!Font::_engine)
  {
    Font::_engine = new(std::nothrow) FontEngineWin();
    if (Font::_engine == NULL) { err = Error::OutOfMemory; goto fail; }
  }
#endif // FOG_FONT_WINDOWS

#if defined(FOG_FONT_FREETYPE)
  if (!Font::_engine)
  {
    Font::_engine = new(std::nothrow) FontEngineFT();
    if (Font::_engine == NULL) { err = Error::OutOfMemory; goto fail; }
  }
#endif // FOG_FONT_FREETYPE

  Font::sharedNull->face = Font::_engine->getDefaultFace();

  if (Font::sharedNull->face == NULL)
  {
    err = Error::FontCantLoadDefaultFace;
    goto fail;
  }

  return err;

fail:
  fog_font_shutdown();
  return err;
}

FOG_INIT_DECLARE void fog_font_shutdown(void)
{
  using namespace Fog;

  // [Font Shared Null]
  delete Font::sharedNull;
  Font::sharedNull = NULL;

  // [Font Cache]
  if (Font::_cache)
  {
    delete Font::_cache;
    Font::_cache = NULL;
  }

  // [Font Engine]
  if (Font::_engine)
  {
    delete Font::_engine;
    Font::_engine = NULL;
  }

  // [Local]
  font_local.destroy();
}
