// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Static.h>
#include <Fog/Core/String.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Font.h>
#include <Fog/Graphics/FontEngine.h>
#include <Fog/Graphics/FontManager.h>

namespace Fog {

// ============================================================================
// [Fog::Font]
// ============================================================================

Font::Data* Font::sharedNull;

static err_t _setFace(Font* self, FontFace* face)
{
  if (!face) return ERR_FONT_INVALID_FACE;
  if (self->_d->face == face) return ERR_OK;

  err_t err = self->detach();
  if (err) return err;

  if (self->_d->face) self->_d->face->deref();
  self->_d->face = face;

  return ERR_OK;
}

Font::Font() :
  _d(sharedNull->ref())
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

err_t Font::_detach()
{
  if (_d->refCount.get() > 1)
  {
    Data* newd = Data::copy(_d);
    if (!newd) return ERR_RT_OUT_OF_MEMORY;
    atomicPtrXchg(&_d, newd)->deref();
  }
  return ERR_OK;
}

void Font::free()
{
  atomicPtrXchg(&_d, sharedNull->ref())->deref();
}

err_t Font::setFamily(const String& family)
{
  return _setFace(this, FontManager::getFace(family, getSize(), getCaps()));
}

err_t Font::setFamily(const String& family, uint32_t size)
{
  return _setFace(this, FontManager::getFace(family, size, getCaps()));
}

err_t Font::setSize(uint32_t size)
{
  return _setFace(this, FontManager::getFace(getFamily(), size, getCaps()));
}

err_t Font::setCaps(const FontCaps& a)
{
  return _setFace(this, FontManager::getFace(getFamily(), getSize(), a));
}

err_t Font::setBold(bool val)
{
  if (isBold() == val) return ERR_OK;

  FontCaps a = getCaps();
  a.bold = val;

  return _setFace(this, FontManager::getFace(getFamily(), getSize(), a));
}

err_t Font::setItalic(bool val)
{
  if (isItalic() == val) return ERR_OK;

  FontCaps a = getCaps();
  a.italic = val;

  return _setFace(this, FontManager::getFace(getFamily(), getSize(), a));
}

err_t Font::setStrike(bool val)
{
  if (isStrike() == val) return ERR_OK;

  FontCaps a = getCaps();
  a.strike = val;

  return _setFace(this, FontManager::getFace(getFamily(), getSize(), a));
}

err_t Font::setUnderline(bool val)
{
  if (isUnderline() == val) return ERR_OK;

  FontCaps a = getCaps();
  a.underline = val;

  return _setFace(this, FontManager::getFace(getFamily(), getSize(), a));
}

err_t Font::set(const Font& other)
{
  atomicPtrXchg(&_d, other._d->ref())->deref();
  return ERR_OK;
}

err_t Font::getGlyphSet(const String& str, GlyphSet& glyphSet) const
{
  return _d->face->getGlyphSet(str.getData(), str.getLength(), glyphSet);
}

err_t Font::getGlyphSet(const Char* str, sysuint_t length, GlyphSet& glyphSet) const
{
  return _d->face->getGlyphSet(str, length, glyphSet);
}

err_t Font::getOutline(const String& str, Path& dst) const
{
  return _d->face->getOutline(str.getData(), str.getLength(), dst);
}

err_t Font::getOutline(const Char* str, sysuint_t length, Path& dst) const
{
  return _d->face->getOutline(str, length, dst);
}

err_t Font::getTextExtents(const String& str, TextExtents& extents) const
{
  return _d->face->getTextExtents(str.getData(), str.getLength(), extents);
}

err_t Font::getTextExtents(const Char* str, sysuint_t length, TextExtents& extents) const
{
  return _d->face->getTextExtents(str, length, extents);
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

Font::Data* Font::Data::copy(const Data* d)
{
  Data* newd = new(std::nothrow) Data();
  if (!newd) return NULL;

  newd->face = d->face ? d->face->ref() : NULL;
  return newd;
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_EXTERN void fog_font_shutdown(void);

FOG_INIT_DECLARE err_t fog_font_init(void)
{
  using namespace Fog;

  // Font is initialized before font manager, so it will set correct default
  // face to this null (default) font.
  Font::sharedNull = new(std::nothrow) Font::Data();
  if (!Font::sharedNull) return ERR_RT_OUT_OF_MEMORY;

  return ERR_OK;
}

FOG_INIT_DECLARE void fog_font_shutdown(void)
{
  using namespace Fog;

  delete Font::sharedNull;
  Font::sharedNull = NULL;
}
