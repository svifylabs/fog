// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_FONT_H
#define _FOG_GRAPHICS_FONT_H

// [Dependencies]
#include <Fog/Core/Hash.h>
#include <Fog/Core/Lock.h>
#include <Fog/Core/Misc.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/String.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/FontEngine.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/Graphics/Glyph.h>
#include <Fog/Graphics/GlyphCache.h>
#include <Fog/Graphics/GlyphSet.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct FontEngine;
struct Path;

// ============================================================================
// [Fog::Font]
// ============================================================================

struct FOG_API Font
{
  // [Data]

  struct FOG_API Data
  {
    // [Construction / Destruction]

    Data();
    ~Data();

    // [Ref / Deref]

    Data* ref() const;
    void deref();

    static Data* copy(const Data* d);

    // [Members]

    mutable Atomic<sysuint_t> refCount;
    FontFace* face;

  private:
    FOG_DISABLE_COPY(Data)
  };

  static Data* sharedNull;

  // [Construction and destruction]

  Font();
  Font(const Font& other);
  FOG_INLINE explicit Font(Data* d) : _d(d) {}
  ~Font();

  // [Implicit sharing and basic flags]

  //! @copydoc Doxygen::Implicit::refCount().
  FOG_INLINE sysuint_t refCount() const { return _d->refCount.get(); }
  //! @copydoc Doxygen::Implicit::isDetached().
  FOG_INLINE bool isDetached() const { return refCount() == 1; }
  //! @copydoc Doxygen::Implicit::detach().
  FOG_INLINE err_t detach() { return !isDetached() ? _detach() : (err_t)ERR_OK; }
  //! @copydoc Doxygen::Implicit::_detach().
  err_t _detach();
  //! @copydoc Doxygen::Implicit::free().
  void free();

  // [Font family and metrics]

  FOG_INLINE const String& getFamily() const { return _d->face->family; }
  FOG_INLINE const FontMetrics& getMetrics() const { return _d->face->metrics; }
  FOG_INLINE uint32_t getSize() const { return _d->face->metrics.size; }
  FOG_INLINE uint32_t getAscent() const { return _d->face->metrics.ascent; }
  FOG_INLINE uint32_t getDescent() const { return _d->face->metrics.descent; }
  FOG_INLINE uint32_t getAverageWidth() const { return _d->face->metrics.averageWidth; }
  FOG_INLINE uint32_t getMaximumWidth() const { return _d->face->metrics.maximumWidth; }
  FOG_INLINE uint32_t getHeight() const { return _d->face->metrics.height; }

  FOG_INLINE const FontCaps& getCaps() const { return _d->face->caps; }
  FOG_INLINE bool isBold() const { return _d->face->caps.bold != 0; }
  FOG_INLINE bool isItalic() const { return _d->face->caps.italic != 0; }
  FOG_INLINE bool isStrike() const { return _d->face->caps.strike != 0; }
  FOG_INLINE bool isUnderline() const { return _d->face->caps.underline != 0; }

  FOG_INLINE uint32_t isScalable() const { return _d->face->scalable; }

  err_t setFamily(const String& family);
  err_t setFamily(const String& family, uint32_t size);
  err_t setFamily(const String& family, uint32_t size, const FontCaps& caps);

  err_t setSize(uint32_t size);
  err_t setCaps(const FontCaps& a);
  err_t setBold(bool val);
  err_t setItalic(bool val);
  err_t setStrike(bool val);
  err_t setUnderline(bool val);

  // [Set]

  err_t set(const Font& other);

  // [Face Methods]

  err_t getGlyphSet(const String& str, GlyphSet& glyphSet) const;
  err_t getGlyphSet(const Char* str, sysuint_t length, GlyphSet& glyphSet) const;

  err_t getOutline(const String& str, Path& dst) const;
  err_t getOutline(const Char* str, sysuint_t length, Path& dst) const;

  err_t getTextExtents(const String& str, TextExtents& extents) const;
  err_t getTextExtents(const Char* str, sysuint_t length, TextExtents& extents) const;

  // [Overloaded Operators]

  FOG_INLINE const Font& operator=(const Font& other) { set(other); return *this; }

  // [Members]

  FOG_DECLARE_D(Data)
};

} // Fog namespace

//! @}

// ============================================================================
// [Fog::TypeInfo<>]
// ============================================================================

FOG_DECLARE_TYPEINFO(Fog::Font, Fog::TYPEINFO_MOVABLE)

// [Guard]
#endif // _FOG_GRAPHICS_FONT_H
