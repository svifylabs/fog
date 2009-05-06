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
#include <Fog/Core/Static.h>
#include <Fog/Graphics/Rgba.h>

namespace Fog {

// ============================================================================
// [Fog::Palette]
// ============================================================================

struct FOG_API Palette
{
  // [Data]

  struct FOG_API Data
  {
    // [Ref / Deref]

    Data* ref() const;
    void deref();

    FOG_INLINE Data* refAlways() const
    {
      refCount.inc();
      return const_cast<Data*>(this);
    }

    // [Create]

    static Data* create();
    static Data* copy(const Data* other);

    // [Members]

    mutable Atomic<sysuint_t> refCount;

    Rgba data[256];
  };

  static Static<Data> sharedNull;
  static Static<Data> sharedGrey;

  // [Members]

  FOG_DECLARE_D(Data);

  // [Construction / Destruction]

  Palette();
  Palette(const Palette& other);
  explicit Palette(Data* d);
  ~Palette();

  // [Implicit Sharing]

  //! @copydoc Doxygen::Implicit::refCount().
  FOG_INLINE sysuint_t refCount() const { return _d->refCount.get(); }
  //! @copydoc Doxygen::Implicit::isDetached().
  FOG_INLINE bool isDetached() const { return refCount() == 1; }
  //! @copydoc Doxygen::Implicit::detach().
  FOG_INLINE err_t detach() { return (!isDetached()) ? _detach() : Error::Ok; }
  //! @copydoc Doxygen::Implicit::_detach().
  err_t _detach();
  //! @copydoc Doxygen::Implicit::free().
  void free();

  // [Data]

  //! @brief Returns const pointer to palette data.
  FOG_INLINE const Rgba* cData() const
  {
    return _d->data;
  }

  //! @brief Returns mutable pointer to palette data.
  FOG_INLINE Rgba* mData()
  {
    detach();
    return _d->data;
  }

  //! @brief Returns mutable pointer to palette data and not calls detach().
  FOG_INLINE Rgba* xData()
  {
    FOG_ASSERT_X(isDetached(), "Fog::Palette::xData() - Non detached data.");
    return _d->data;
  }

  FOG_INLINE const Rgba& cAt(sysuint_t index) const
  { 
    FOG_ASSERT_X(index < 256, "Fog::Palette::cAt() - Index out of range.");
    return _d->data[index];
  }

  FOG_INLINE Rgba& mAt(sysuint_t index)
  { 
    FOG_ASSERT_X(index < 256, "Fog::Palette::mAt() - Index out of range.");
    detach();
    return _d->data[index];
  }

  FOG_INLINE Rgba& xAt(sysuint_t index)
  { 
    FOG_ASSERT_X(index < 256, "Fog::Palette::xAt() - Index out of range.");
    FOG_ASSERT_X(isDetached(), "Fog::Palette::xAt() - Non detached data.");
    return _d->data[index];
  }

  // [Operations]

  void clear();

  err_t set(const Palette& other);
  err_t setDeep(const Palette& other);
  err_t set(sysuint_t index, sysuint_t count, const Rgba* rgba);

  uint8_t findColor(uint8_t r, uint8_t g, uint8_t b) const;

  static Palette greyscale();
  static Palette colorCube(int r, int g, int b);

  // [Overloaded Operators]

  FOG_INLINE const Palette& operator=(const Palette& other)
  {
    set(other);
    return *this;
  }

  // [Statics]

  static bool isGreyOnly(const Rgba* data, sysuint_t count);
};

} // Fog namespace

// [Guard]
#endif // _FOG_GRAPHICS_PALETTE_H
