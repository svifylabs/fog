// [Fog-G2d]
//
// [License]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_G2D_FONT_GLYPH_H
#define _FOG_G2D_FONT_GLYPH_H

// [Dependencies]
#include <Fog/Core/Global/Static.h>
#include <Fog/G2d/Geometry/Path.h>
#include <Fog/G2d/Geometry/Point.h>
#include <Fog/G2d/Global/Constants.h>
#include <Fog/G2d/Imaging/Image.h>

namespace Fog {

//! @addtogroup Fog_G2d_Text
//! @{

// ============================================================================
// [Fog::GlyphData]
// ============================================================================

//! @brief Glyph data.
struct FOG_API GlyphData
{
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  GlyphData();
  explicit GlyphData(const GlyphData* other);
  ~GlyphData();

  // --------------------------------------------------------------------------
  // [Ref / Deref]
  // --------------------------------------------------------------------------

  FOG_INLINE GlyphData* ref() const
  {
    refCount.inc();
    return const_cast<GlyphData*>(this);
  }

  FOG_INLINE void deref()
  {
    if (refCount.deref()) fog_delete(this);
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  //! @brief Reference count.
  mutable Atomic<sysuint_t> refCount;

  //! @brief Glyph bitmap data, supported formats are only A8 for now.
  Image bitmap;
  //! @brief Bitmap offset.
  PointI offset;

  //! @brief Begin width.
  int beginWidth;
  //! @brief End width.
  int endWidth;
  //! @brief Glyph advance.
  int advance;

private:
  FOG_DISABLE_COPY(GlyphData)
};

// ============================================================================
// [Fog::Glyph]
// ============================================================================

//! @brief Glyph.
struct FOG_API Glyph
{
  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  FOG_INLINE Glyph() : _d(_dnull->ref()) {}
  FOG_INLINE Glyph(const Glyph& other) : _d(other._d->ref()) {}

  explicit FOG_INLINE Glyph(GlyphData* d) : _d(d) {}

  FOG_INLINE ~Glyph() { _d->deref(); }

  // --------------------------------------------------------------------------
  // [Implicit Sharing]
  // --------------------------------------------------------------------------

  //! @copydoc Doxygen::Implicit::getRefCount().
  FOG_INLINE sysuint_t getRefCount() const { return _d->refCount.get(); }
  //! @copydoc Doxygen::Implicit::isDetached().
  FOG_INLINE bool isDetached() const { return getRefCount() == 1; }
  //! @copydoc Doxygen::Implicit::detach().
  FOG_INLINE err_t detach() { return isDetached() ? (err_t)ERR_OK : _detach(); }

  //! @copydoc Doxygen::Implicit::_detach().
  err_t _detach();

  // --------------------------------------------------------------------------
  // [Flags]
  // --------------------------------------------------------------------------

  FOG_INLINE bool isNull() const { return _d == _dnull.instancep(); }

  // --------------------------------------------------------------------------
  // [Accessors]
  // --------------------------------------------------------------------------

  FOG_INLINE const GlyphData* getData() const { return _d; }

  FOG_INLINE const Image& getBitmap() const { return _d->bitmap; }
  FOG_INLINE const PointI& getOffset() const { return _d->offset; }

  FOG_INLINE int getBeginWidth() const { return _d->beginWidth; }
  FOG_INLINE int getEndWidth() const { return _d->endWidth; }
  FOG_INLINE int getAdvance() const { return _d->advance; }

  err_t setBitmap(const Image& bitmap);
  err_t setOffset(const PointI& offset);

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  Glyph& operator=(const Glyph& other);

  // --------------------------------------------------------------------------
  // [Statics]
  // --------------------------------------------------------------------------

  static Static<GlyphData> _dnull;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  FOG_DECLARE_D(GlyphData)
};

//! @}

} // Fog namespace

// ============================================================================
// [Fog::TypeInfo<>]
// ============================================================================

FOG_DECLARE_TYPEINFO(Fog::Glyph, Fog::TYPEINFO_MOVABLE)

// [Guard]
#endif // _FOG_G2D_FONT_GLYPH_H