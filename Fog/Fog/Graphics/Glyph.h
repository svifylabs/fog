// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_GLYPH_H
#define _FOG_GRAPHICS_GLYPH_H

// [Dependencies]
#include <Fog/Core/RefData.h>
#include <Fog/Core/Static.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/Graphics/Image.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Fog::Glyph]
// ============================================================================

struct FOG_API Glyph
{
  // [Data]

  struct FOG_API Data : public RefDataSimple<Data>
  {
    Data();
    ~Data();

    //! @brief Glyph image, supported formats are only A8 and XRGB32 for now.
    Image image;
    //! @brief X offset for rendering glyph image.
    int offsetX;
    //! @brief Y offset for rendering glyph image.
    int offsetY;
    //! @brief Begin width.
    int beginWidth;
    //! @brief End width.
    int endWidth;
    //! @brief Glyph advance.
    int advance;

    FOG_INLINE Data* ref() { return REF_ALWAYS(); }
    FOG_INLINE void deref() { DEREF_INLINE(); }
    FOG_INLINE void free() { delete this; }

  private:
    FOG_DISABLE_COPY(Data)
  };

  // [Members]

  FOG_DECLARE_D(Data)

  static Static<Data> sharedNull;

  // [Construction / Destruction]

  FOG_INLINE Glyph() : _d(sharedNull.instancep()->ref()) 
  {}

  FOG_INLINE Glyph(const Glyph& other) : _d(other._d->ref()) 
  {}

  FOG_INLINE Glyph(Data* d) : _d(d) 
  {}

  FOG_INLINE ~Glyph()
  { _d->deref(); }

  // [Implicit Sharing]

  FOG_INLINE sysuint_t refCount() const { return _d->refCount.get(); }
  FOG_INLINE bool isNull() const { return _d == sharedNull.instancep(); }

  // [Getters]

  FOG_INLINE const Image& image() const { return _d->image; }
  FOG_INLINE int offsetX() const { return _d->offsetX; }
  FOG_INLINE int offsetY() const { return _d->offsetY; }
  FOG_INLINE int beginWidth() const { return _d->beginWidth; }
  FOG_INLINE int endWidth() const { return _d->endWidth; }
  FOG_INLINE int advance() const { return _d->advance; }

  // [Operator Overload]

  FOG_INLINE Glyph& operator=(const Glyph& other)
  { AtomicBase::ptr_setXchg(&_d, other._d->ref())->deref(); return *this; }
};

} // Fog namespace

//! @}

// ============================================================================
// [Fog::TypeInfo<>]
// ============================================================================

FOG_DECLARE_TYPEINFO(Fog::Glyph, Fog::MoveableType)

// [Guard]
#endif // _FOG_GRAPHICS_GLYPH_H
