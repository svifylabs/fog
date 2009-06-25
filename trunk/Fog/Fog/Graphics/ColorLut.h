// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_COLORLUT_H
#define _FOG_GRAPHICS_COLORLUT_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Core/Assert.h>
#include <Fog/Core/Atomic.h>
#include <Fog/Core/Error.h>
#include <Fog/Core/Static.h>
#include <Fog/Graphics/Constants.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct Rgba;
struct Rgba64;

// ============================================================================
// [Fog::ColorLut]
// ============================================================================

//! @brief Color lut.
//!
//! Color lut is class that stores information about color channel
//! transformations that can be applied to an image data. The main idea is to
//! group many transformations to this class and lets graphics backend decide
//! if it can support particular transform directly or it will use color lut
//! instead.
struct FOG_API ColorLut
{
  // [Data]

  struct FOG_API Data
  {
    // [Ref / Deref]

    Data* ref() const;
    void deref();

    FOG_INLINE Data* refAlways() const { refCount.inc(); return const_cast<Data*>(this); }

    static Data* alloc();
    static Data* copy(const Data* other);

    // [Members]

    mutable Atomic<sysuint_t> refCount;
    uint32_t type;
    uint32_t flags;

    uint8_t rTable[256];
    uint8_t gTable[256];
    uint8_t bTable[256];
    uint8_t aTable[256];
  };

  static Static<Data> sharedNull;
  static const uint8_t* linearLut;

  // [Construction / Destruction]

  ColorLut();
  ColorLut(const ColorLut& other);
  ~ColorLut();

  // [Implicit Sharing]

  //! @copydoc Doxygen::Implicit::refCount().
  FOG_INLINE sysuint_t refCount() const { return _d->refCount.get(); }
  //! @copydoc Doxygen::Implicit::isDetached().
  FOG_INLINE bool isDetached() const { return refCount() == 1; }
  //! @copydoc Doxygen::Implicit::detach().
  FOG_INLINE err_t detach() { return !isDetached() ? _detach() : (err_t)Error::Ok; }
  //! @copydoc Doxygen::Implicit::_detach().
  err_t _detach();
  //! @copydoc Doxygen::Implicit::free().
  void free();

  // [Type]

  //! @brief Type of color lut.
  enum Type
  {
    NopType = 0,
    CustomType = 0xFF
  };

  //! @brief Get type of color lut.
  FOG_INLINE int type() const { return _d->type; }

  FOG_INLINE bool isNop() const { return _d->type == NopType; }

  // [Flags]

  // [Lut Tables]

  FOG_INLINE const uint8_t* cRedTable  () const { return _d->rTable; }
  FOG_INLINE const uint8_t* cGreenTable() const { return _d->gTable; }
  FOG_INLINE const uint8_t* cBlueTable () const { return _d->bTable; }
  FOG_INLINE const uint8_t* cAlphaTable() const { return _d->aTable; }

  FOG_INLINE uint8_t* mRedTable  () { return detach() == Error::Ok ? _d->rTable : NULL; }
  FOG_INLINE uint8_t* mGreenTable() { return detach() == Error::Ok ? _d->gTable : NULL; }
  FOG_INLINE uint8_t* mBlueTable () { return detach() == Error::Ok ? _d->bTable : NULL; }
  FOG_INLINE uint8_t* mAlphaTable() { return detach() == Error::Ok ? _d->aTable : NULL; }

  FOG_INLINE uint8_t* xRedTable  () { FOG_ASSERT_X(isDetached(), "Fog::ColorLut::xRedTable() - Not detached data."  ); return _d->rTable; }
  FOG_INLINE uint8_t* xGreenTable() { FOG_ASSERT_X(isDetached(), "Fog::ColorLut::xGreenTable() - Not detached data."); return _d->gTable; }
  FOG_INLINE uint8_t* xBlueTable () { FOG_ASSERT_X(isDetached(), "Fog::ColorLut::xBlueTable() - Not detached data." ); return _d->bTable; }
  FOG_INLINE uint8_t* xAlphaTable() { FOG_ASSERT_X(isDetached(), "Fog::ColorLut::xAlphaTable() - Not detached data."); return _d->aTable; }

  // [Brightness / Contrast]

  // [Operator Overload]

  //! @brief Assignment operator.
  ColorLut& operator=(const ColorLut& other);

  // [Members]

  FOG_DECLARE_D(Data)
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_COLORLUT_H
