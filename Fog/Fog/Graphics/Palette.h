// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_PALETTE_H
#define _FOG_GRAPHICS_PALETTE_H

// [Dependencies]
#include <Fog/Core/Atomic.h>
#include <Fog/Core/Flags.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/RefData.h>
#include <Fog/Core/Static.h>
#include <Fog/Graphics/Rgba.h>

namespace Fog {

// ============================================================================
// [Fog::Palette]
// ============================================================================

struct FOG_API Palette
{
  // [Data]

  struct FOG_API Data : Fog::RefData<Data>
  {
    Rgba data[256];

    Data* ref();
    void deref();

    FOG_INLINE void destroy() {}
    FOG_INLINE void free() { Fog::Memory::free(this); }

    static Data* create(uint allocPolicy);
    static Data* copy(const Data* other, uint allocPolicy);
  };

  static Fog::Static<Data> sharedNull;
  static Fog::Static<Data> sharedGrey;

  // [Members]

  FOG_DECLARE_D(Data);

  // [Construction / Destruction]

  Palette();
  Palette(const Palette& other);
  Palette(Data* d);
  ~Palette();

  // [Implicit Sharing]

  /*! @copydoc Doxygen::Implicit::refCount(). */
  FOG_INLINE sysuint_t refCount() const { return _d->refCount.get(); }
  /*! @copydoc Doxygen::Implicit::isDetached(). */
  FOG_INLINE bool isDetached() const { return refCount() == 1; }
  /*! @copydoc Doxygen::Implicit::detach(). */
  FOG_INLINE void detach() { if (!isDetached()) _detach(); }
  /*! @copydoc Doxygen::Implicit::tryDetach(). */
  FOG_INLINE bool tryDetach() { return (!isDetached()) ? _tryDetach() : true; }
  /*! @copydoc Doxygen::Implicit::_detach(). */
  void _detach();
  /*! @copydoc Doxygen::Implicit::_tryDetach(). */
  bool _tryDetach();
  /*! @copydoc Doxygen::Implicit::free(). */
  void free();

  // [Flags]

  /*! @copydoc Doxygen::Implicit::flags(). */
  FOG_INLINE uint32_t flags() const { return _d->flags; }
  /*! @copydoc Doxygen::Implicit::isDynamic(). */
  FOG_INLINE bool isDynamic() const { return _d->flags.anyOf(Data::IsDynamic); }
  /*! @copydoc Doxygen::Implicit::isSharable(). */
  FOG_INLINE bool isSharable() const { return _d->flags.anyOf(Data::IsSharable); }
  /*! @copydoc Doxygen::Implicit::isNull(). */
  FOG_INLINE bool isNull() const { return _d->flags.anyOf(Data::IsNull); }
  /*! @copydoc Doxygen::Implicit::isStrong(). */
  FOG_INLINE bool isStrong() const { return _d->flags.anyOf(Data::IsStrong); }

  // [Data]

  /*! @brief Returns const pointer to palette data. */
  FOG_INLINE const Fog::Rgba* cData() const
  {
    return _d->data;
  }

  /*! @brief Returns mutable pointer to palette data. */
  FOG_INLINE Fog::Rgba* mData()
  {
    detach();
    return _d->data;
  }

  /*! @brief Returns mutable pointer to palette data and not calls detach(). */
  FOG_INLINE Fog::Rgba* xData()
  {
    FOG_ASSERT_X(isDetached(), "Fog::Palette::xData() - Non detached data.");
    return _d->data;
  }

  FOG_INLINE const Fog::Rgba& cAt(sysuint_t index) const
  { 
    FOG_ASSERT_X(index < 256, "Fog::Palette::cAt() - Index out of range.");
    return _d->data[index];
  }

  FOG_INLINE Fog::Rgba& mAt(sysuint_t index)
  { 
    FOG_ASSERT_X(index < 256, "Fog::Palette::mAt() - Index out of range.");
    detach();
    return _d->data[index];
  }

  FOG_INLINE Fog::Rgba& xAt(sysuint_t index)
  { 
    FOG_ASSERT_X(index < 256, "Fog::Palette::xAt() - Index out of range.");
    FOG_ASSERT_X(isDetached(), "Fog::Palette::xAt() - Non detached data.");
    return _d->data[index];
  }

  // [Set]

  Palette& set(const Fog::Palette& other);
  Palette& set(sysuint_t index, sysuint_t count, const Rgba* rgba);

  Palette& greyscale();

  // [Overloaded Operators]

  FOG_INLINE const Fog::Palette& operator=(const Fog::Palette& other)
  {
    return set(other);
  }

  // [Statics]

  static bool isGreyOnly(const Rgba* data, sysuint_t count);
};

} // Fog namespace

// [Guard]
#endif // _FOG_GRAPHICS_PALETTE_H
