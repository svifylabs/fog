// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_LOCALE_H
#define _FOG_CORE_LOCALE_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Core/Constants.h>
#include <Fog/Core/Atomic.h>
#include <Fog/Core/Char.h>
#include <Fog/Core/Flags.h>
#include <Fog/Core/RefData.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/String.h>
#include <Fog/Core/TypeInfo.h>

//! @addtogroup Fog_Core
//! @{

// [Fog::}
namespace Fog {

// ============================================================================
// [Fog::Locale]
// ============================================================================

//! @brief Locale.
//!
//! Locales are useful when translating numbers or floats from / to strings.
//! In some cases there are needed that numbers and strings are converted to 
//! user native locale, this happen in GUI input components, but in some 
//! situations there is needed to use exactly one locale. For example 
//! configuration files or platform indenpendent documents should use POSIX
//! locale only.
//!
//! To get user locale use @c Locale::user() method and to get built-in posix
//! locale use @c Locale::posix().
struct FOG_API Locale
{
  // [Data]

  //! @brief %Locale data.
  struct FOG_API Data : public RefDataSimple<Data>
  {
    enum { N = 10 };

    // [Locale]

    String32 name;

    // [Members]
    union
    {
      uint32_t data[N];

      struct
      {
        uint32_t decimalPoint;
        uint32_t thousandsGroup;
        uint32_t zero;
        uint32_t plus;
        uint32_t minus;
        uint32_t space;
        uint32_t exponential;
        uint32_t firstThousandsGroup;
        uint32_t nextThousandsGroup;
        uint32_t reserved;
      };
    };

    // [Construction / Destruction]

    Data();
    Data(const Data& other);
    ~Data();

    // [Ref]

    Data* ref();
    void deref();

    FOG_INLINE Data* REF_INLINE()
    { return REF_ALWAYS(); }
    
    FOG_INLINE void free()
    { delete this; }

    static Data* copy(Data* d, uint allocPolicy);
  };

  // [Members]

  FOG_DECLARE_D(Data)

  static Static<Data> sharedNull;
  static Static<Data> sharedPosix;
  static Static<Data> sharedUser;

  static Data* sharedNullObj;
  static Data* sharedPosixObj;
  static Data* sharedUserObj;

  // [Construction / Destruction]

  Locale();
  Locale(Data* d);
  Locale(const Locale& other);
  explicit Locale(const String32& name);
  ~Locale();

  // [Implicit Sharing]

  //! @copydoc Doxygen::Implicit::refCount().
  FOG_INLINE sysuint_t refCount() const { return _d->refCount.get(); }
  //! @copydoc Doxygen::Implicit::isDetached().
  FOG_INLINE bool isDetached() const { return refCount() == 1; }
  //! @copydoc Doxygen::Implicit::detach().
  FOG_INLINE void detach() { if (!isDetached()) _detach(); }
  //! @copydoc Doxygen::Implicit::_detach().
  void _detach();
  //! @copydoc Doxygen::Implicit::free().
  void free();
  
  // [Flags]

  FOG_INLINE bool isNull()
  { return _d == sharedNull.instancep(); }

  // [Set]

  bool set(const String32& name);
  bool set(const Locale& other);

  // [Data Getters]

  FOG_INLINE Char32 decimalPoint() const
  { return Char32(_d->decimalPoint); }
  
  FOG_INLINE Char32 thousandsGroup() const
  { return Char32(_d->thousandsGroup); }
  
  FOG_INLINE Char32 zero() const
  { return Char32(_d->zero); }
  
  FOG_INLINE Char32 plus() const
  { return Char32(_d->plus); }
  
  FOG_INLINE Char32 minus() const
  { return Char32(_d->minus); }
  
  FOG_INLINE Char32 space() const
  { return Char32(_d->space); }

  FOG_INLINE Char32 exponential() const
  { return Char32(_d->exponential); }
  
  FOG_INLINE uint32_t firstThousandsGroup() const
  { return _d->firstThousandsGroup; }
  
  FOG_INLINE uint32_t nextThousandsGroup() const
  { return _d->nextThousandsGroup; }

  // [Data Setters]

  FOG_INLINE Locale& setDecimalPoint(Char32 c)
  { return _setChar(0, c.ch()); }
  
  FOG_INLINE Locale& setThousandsGroup(Char32 c)
  { return _setChar(1, c.ch()); }
  
  FOG_INLINE Locale& setZero(Char32 c)
  { return _setChar(2, c.ch()); }
  
  FOG_INLINE Locale& setPlus(Char32 c)
  { return _setChar(3, c.ch()); }
  
  FOG_INLINE Locale& setMinus(Char32 c)
  { return _setChar(4, c.ch()); }
  
  FOG_INLINE Locale& setSpace(Char32 c)
  { return _setChar(5, c.ch()); }

  FOG_INLINE Locale& setExponential(Char32 c)
  { return _setChar(6, c.ch()); }
  
  FOG_INLINE Locale& setFirstThousandsGroup(uint32_t i)
  { return _setChar(7, i); }
  
  FOG_INLINE Locale& setNextThousandsGroup(uint32_t i)
  { return _setChar(8, i); }

  // [Operator Overload]

  FOG_INLINE Locale& operator=(const Locale& other)
  { set(other); return *this; }

  // [Statics]

  static FOG_INLINE const Locale& null()
  { return *((Fog::Locale*)&sharedNullObj); }

  static FOG_INLINE const Locale& posix()
  { return *((Fog::Locale*)&sharedPosixObj); }
  
  static FOG_INLINE const Locale& user()
  { return *((Fog::Locale*)&sharedUserObj); }

private:
  Locale& _setChar(sysuint_t index, uint32_t uc);
};

} // Fog namespace

//! @}

// ============================================================================
// [Fog::TypeInfo<>]
// ============================================================================

FOG_DECLARE_TYPEINFO(Fog::Locale, Fog::MoveableType)

// [Guard]
#endif // _FOG_CORE_LOCALE_H
