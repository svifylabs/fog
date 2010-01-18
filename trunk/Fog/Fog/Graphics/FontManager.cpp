// [Fog/Graphics Library - Public API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/AutoLock.h>
#include <Fog/Core/Constants.h>
#include <Fog/Core/FileSystem.h>
#include <Fog/Core/FileUtil.h>
#include <Fog/Core/Lock.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/OS.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/String.h>
#include <Fog/Core/UserInfo.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Font.h>
#include <Fog/Graphics/FontEngine.h>
#include <Fog/Graphics/FontManager.h>

#if defined(FOG_FONT_WINDOWS)
#include <Fog/Graphics/FontEngine/Gdi.h>
#endif // FOG_FONT_WINDOWS

#if defined(FOG_FONT_FREETYPE)
#include <Fog/Graphics/FontEngine/FreeType.h>
#endif // FOG_FONT_FREETYPE

#include <Fog/Graphics/FontEngine/Null_p.h>

namespace Fog {

// ============================================================================
// [Fog::FontFaceCacheEntry]
// ============================================================================

struct FOG_HIDDEN FontFaceCacheEntry
{
  // [Construction / Destruction]

  FOG_INLINE FontFaceCacheEntry()
  {
    size = 0;
    caps.value = 0;
  }

  FOG_INLINE FontFaceCacheEntry(const FontFaceCacheEntry& other) :
    family(other.family)
  {
    size = other.size;
    caps.value = other.caps.value;
  }

  FOG_INLINE FontFaceCacheEntry(const String& _family, uint32_t _size, const FontCaps& _caps) :
    family(_family)
  {
    size = _size;
    caps.value = _caps.value;
  }

  FOG_INLINE ~FontFaceCacheEntry() {}

  // [Methods]

  FOG_INLINE uint32_t getHashCode() const
  {
    return family.getHashCode() ^ (size * 133) ^ caps.value;
  }

  // [Operator Overload]

  FOG_INLINE FontFaceCacheEntry& operator=(const FontFaceCacheEntry& other)
  {
    family = other.family;
    size = other.size;
    caps.value = other.caps.value;
  }

  FOG_INLINE bool operator==(const FontFaceCacheEntry& other) const
  {
    return family == other.family && size == other.size && caps.value == other.caps.value;
  }

  FOG_INLINE bool operator!=(const FontFaceCacheEntry& other) const
  {
    return family != other.family || size != other.size || caps.value != other.caps.value;
  }

  // [Members]

  //! @brief Font family name.
  String family;
  //! @brief Font size.
  uint32_t size;
  //! @brief Font caps.
  FontCaps caps;
};

// ============================================================================
// [Fog::FontManager_Local]
// ============================================================================

struct FontManager_Local
{
  FontManager_Local();
  ~FontManager_Local();

  FontFace* getFaceFromCache(const String& family, uint32_t size, const FontCaps& caps);
  err_t putFaceToCache(FontFace* face);

  void resetFaceCache();

  // [Members]

  Lock lock;

  List<String> paths;
  List<String> list;
  bool listInitialized;

  Hash<FontFaceCacheEntry, FontFace*> faceCache;
};

static Static<FontManager_Local> fontmanager_local;

FontManager_Local::FontManager_Local()
{
}

FontManager_Local::~FontManager_Local()
{
  resetFaceCache();
}

FontFace* FontManager_Local::getFaceFromCache(const String& family, uint32_t size, const FontCaps& caps)
{
  AutoLock locked(lock);

  FontFace* face = faceCache.value(FontFaceCacheEntry(family, size, caps), NULL);
  if (face) face = face->ref();
  return face;
}

err_t FontManager_Local::putFaceToCache(FontFace* face)
{
  AutoLock locked(lock);

  err_t err = faceCache.put(FontFaceCacheEntry(face->family, face->metrics.size, face->caps), face, false);
  if (err == ERR_OK) face->ref();
  return err;
}

void FontManager_Local::resetFaceCache()
{
  AutoLock locked(lock);
  Hash<FontFaceCacheEntry, FontFace*>::MutableIterator it(faceCache);

  for (it.toStart(); it.isValid(); it.remove()) it.value()->deref();
}

// ============================================================================
// [Fog::FontManager]
// ============================================================================

FontEngine* FontManager::_engine;

bool FontManager::addFontPath(const String& path)
{
  AutoLock locked(fontmanager_local->lock);

  if (!fontmanager_local->paths.contains(path) && FileSystem::isDirectory(path))
  {
    fontmanager_local->paths.append(path);
    return true;
  }
  else
    return false;
}

void FontManager::addFontPaths(const List<String>& paths)
{
  AutoLock locked(fontmanager_local->lock);

  List<String>::ConstIterator it(paths);
  for (; it.isValid(); it.toNext())
  {
    String path = it.value();
    if (!fontmanager_local->paths.contains(path) && FileSystem::isDirectory(path))
    {
      fontmanager_local->paths.append(path);
    }
  }
}

bool FontManager::removeFontPath(const String& path)
{
  AutoLock locked(fontmanager_local->lock);
  return fontmanager_local->paths.remove(path) != 0;
}

bool FontManager::hasFontPath(const String& path)
{
  AutoLock locked(fontmanager_local->lock);
  return fontmanager_local->paths.contains(path);
}

bool FontManager::findFontFile(const String& fileName, String& dest)
{
  AutoLock locked(fontmanager_local->lock);
  return FileSystem::findFile(fontmanager_local->paths, fileName, dest);
}

List<String> FontManager::getPathList()
{
  AutoLock locked(fontmanager_local->lock);
  return fontmanager_local->paths;
}

List<String> FontManager::getFontList()
{
  AutoLock locked(fontmanager_local->lock);
  if (!fontmanager_local->listInitialized)
  {
    fontmanager_local->list.append(_engine->getFontList());
    fontmanager_local->listInitialized = true;
  }
  return fontmanager_local->list;
}

FontFace* FontManager::getFace(const String& family, uint32_t size, const FontCaps& caps)
{
  FontFace* face;

  // Create to get font face from cache.
  face = fontmanager_local->getFaceFromCache(family, size, caps);
  if (face) return face;

  // If demanded face is not in cache, we will try to create it.
  face = _engine->createFace(family, size, caps);
  if (!face) return NULL;

  err_t err = fontmanager_local->putFaceToCache(face);
  if (err == ERR_OK)
  {
    // Everything is OK. We increased reference count before adding face to cache
    // and we can't do it again!
    return face;
  }

  // This can happen if there is concurrency. We created font face that was
  // created by another thread and put into the cache. Everything we need is to
  // delete our one and use font face that is in cache already.
  //
  // Second case is that engine created face that is close to demanded one but not
  // equal.
  if (err == ERR_RT_OBJECT_ALREADY_EXISTS)
  {
    FontFace* other = fontmanager_local->getFaceFromCache(face->family, face->metrics.size, face->caps);
    if (other) { face->deref(); face = other; }
  }

  return face;
}

err_t FontManager::putFace(FontFace* face)
{
  return fontmanager_local->putFaceToCache(face);
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_EXTERN void fog_fontmanager_shutdown(void);

FOG_INIT_DECLARE err_t fog_fontmanager_init(void)
{
  using namespace Fog;

  err_t err = ERR_OK;

  fontmanager_local.init();
  fontmanager_local.instance().listInitialized = false;

  // Font Engine.
#if defined(FOG_FONT_WINDOWS)
  if (!FontManager::_engine)
  {
    FontManager::_engine = new(std::nothrow) WinFontEngine();
    if (FontManager::_engine == NULL) { err = ERR_RT_OUT_OF_MEMORY; goto fail; }
  }
#endif // FOG_FONT_WINDOWS

#if defined(FOG_FONT_FREETYPE)
  if (!FontManager::_engine)
  {
    FontManager::_engine = new(std::nothrow) FTFontEngine();
    if (FontManager::_engine == NULL) { err = ERR_RT_OUT_OF_MEMORY; goto fail; }
  }
#endif // FOG_FONT_FREETYPE

  // Add engine specific font directories.
  FontManager::addFontPaths(FontManager::_engine->getDefaultFontDirectories());

  // Set engine specific default font face to null (default) font.
  {
    Font::Data* defd = Font::sharedNull;
    defd->face = FontManager::_engine->createDefaultFace();

    if (defd->face == NULL)
    {
      fog_debug("Fog::FontManager::init() - Can't load default font face, setting to Fog::NullFontFace.");
      defd->face = new(std::nothrow) NullFontFace();
    }
  }

  return ERR_OK;

fail:
  fog_fontmanager_shutdown();
  return err;
}

FOG_INIT_DECLARE void fog_fontmanager_shutdown(void)
{
  using namespace Fog;

  // Dereference null (default) face, because we are ready to destroy font
  // engine and this is last place where FontFace instance from this engine can
  // be.
  if (Font::sharedNull->face)
  {
    Font::sharedNull->face->deref();
    Font::sharedNull->face = NULL;
  }

  fontmanager_local->resetFaceCache();

  // Destroy font engine.
  if (FontManager::_engine)
  {
    delete FontManager::_engine;
    FontManager::_engine = NULL;
  }

  // Destroy font manager.
  fontmanager_local.destroy();
}
