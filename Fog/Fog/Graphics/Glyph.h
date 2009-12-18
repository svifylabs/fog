// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_GLYPH_H
#define _FOG_GRAPHICS_GLYPH_H

// [Dependencies]
#include <Fog/Core/Static.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/Path.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Fog::Glyph]
// ============================================================================

struct FOG_API Glyph
{
  // [Data]

  struct FOG_API Data
  {
    // [Construction / Destruction]

    Data();
    ~Data();

    // [Ref / Deref]

    FOG_INLINE Data* ref() const
    {
      refCount.inc();
      return const_cast<Data*>(this);
    }

    FOG_INLINE void deref()
    {
      if (refCount.deref()) delete this;
    }

    // [Members]

    //! @brief Reference count.
    mutable Atomic<sysuint_t> refCount;

    //! @brief Glyph bitmap data, supported formats are only A8 for now.
    Image bitmap;
    //! @brief Bitmap X offset.
    int bitmapX;
    //! @brief Bitmap Y offset.
    int bitmapY;

    //! @brief Begin width.
    int beginWidth;
    //! @brief End width.
    int endWidth;
    //! @brief Glyph advance.
    int advance;

  private:
    FOG_DISABLE_COPY(Data)
  };

  // [Flags]

  static Static<Data> sharedNull;

  // [Construction / Destruction]

  FOG_INLINE Glyph() : _d(sharedNull.instancep()->ref())  {}
  FOG_INLINE Glyph(const Glyph& other) : _d(other._d->ref())  {}

  FOG_INLINE explicit Glyph(Data* d) : _d(d)  {}

  FOG_INLINE ~Glyph() { _d->deref(); }

  // [Implicit Sharing]

  FOG_INLINE sysuint_t refCount() const { return _d->refCount.get(); }
  FOG_INLINE bool isNull() const { return _d == sharedNull.instancep(); }

  // [Getters]

  FOG_INLINE const Image& getBitmap() const { return _d->bitmap; }
  FOG_INLINE int getBitmapX() const { return _d->bitmapX; }
  FOG_INLINE int getBitmapY() const { return _d->bitmapY; }

  FOG_INLINE int getBeginWidth() const { return _d->beginWidth; }
  FOG_INLINE int getEndWidth() const { return _d->endWidth; }
  FOG_INLINE int getAdvance() const { return _d->advance; }

  // [Operator Overload]

  FOG_INLINE Glyph& operator=(const Glyph& other)
  { atomicPtrXchg(&_d, other._d->ref())->deref(); return *this; }

  // [Members]

  FOG_DECLARE_D(Data)
};

} // Fog namespace

//! @}

// ============================================================================
// [Fog::TypeInfo<>]
// ============================================================================

FOG_DECLARE_TYPEINFO(Fog::Glyph, Fog::TYPEINFO_MOVABLE)

// [Guard]
#endif // _FOG_GRAPHICS_GLYPH_H
