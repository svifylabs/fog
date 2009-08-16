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

  struct FOG_API Data
  {
    enum { N = 10 };

    // [Construction / Destruction]

    Data();
    Data(const Data& other);
    ~Data();

    // [Ref / Deref]

    Data* ref() const;
    void deref();

    FOG_INLINE Data* refAlways() const
    {
      refCount.inc();
      return const_cast<Data*>(this);
    }

    static Data* copy(Data* d);

    // [Members]

    mutable Atomic<sysuint_t> refCount;

    String32 name;

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
  };


  static Static<Data> sharedNull;
  static Static<Data> sharedPosix;
  static Static<Data> sharedUser;

  static Data* sharedNullObj;
  static Data* sharedPosixObj;
  static Data* sharedUserObj;

  // [Construction / Destruction]

  Locale();
  Locale(const Locale& other);
  explicit Locale(Data* d);
  explicit Locale(const String32& name);
  ~Locale();

  // [Implicit Sharing]

  //! @copydoc Doxygen::Implicit::refCount().
  FOG_INLINE sysuint_t refCount() const { return _d->refCount.get(); }
  //! @copydoc Doxygen::Implicit::isDetached().
  FOG_INLINE bool isDetached() const { return _d->refCount.get() == 1; }
  //! @copydoc Doxygen::Implicit::detach().
  FOG_INLINE err_t detach() { return (!isDetached()) ? _detach() : Error::Ok; }
  //! @copydoc Doxygen::Implicit::_detach().
  err_t _detach();
  //! @copydoc Doxygen::Implicit::free().
  void free();
  
  // [Flags]

  FOG_INLINE bool isNull()
  { return _d == sharedNull.instancep(); }

  // [Set]

  bool set(const String32& name);
  bool set(const Locale& other);

  // [Data Getters]

  FOG_INLINE Char32 getDecimalPoint() const
  { return Char32(_d->decimalPoint); }
  
  FOG_INLINE Char32 getThousandsGroup() const
  { return Char32(_d->thousandsGroup); }
  
  FOG_INLINE Char32 getZero() const
  { return Char32(_d->zero); }
  
  FOG_INLINE Char32 getPlus() const
  { return Char32(_d->plus); }
  
  FOG_INLINE Char32 getMinus() const
  { return Char32(_d->minus); }
  
  FOG_INLINE Char32 getSpace() const
  { return Char32(_d->space); }

  FOG_INLINE Char32 getExponential() const
  { return Char32(_d->exponential); }
  
  FOG_INLINE uint32_t getFirstThousandsGroup() const
  { return _d->firstThousandsGroup; }
  
  FOG_INLINE uint32_t getNextThousandsGroup() const
  { return _d->nextThousandsGroup; }

  // [Data Setters]

  FOG_INLINE err_t setDecimalPoint(Char32 c)
  { return _setChar(0, c.ch()); }
  
  FOG_INLINE err_t setThousandsGroup(Char32 c)
  { return _setChar(1, c.ch()); }
  
  FOG_INLINE err_t setZero(Char32 c)
  { return _setChar(2, c.ch()); }
  
  FOG_INLINE err_t setPlus(Char32 c)
  { return _setChar(3, c.ch()); }
  
  FOG_INLINE err_t setMinus(Char32 c)
  { return _setChar(4, c.ch()); }
  
  FOG_INLINE err_t setSpace(Char32 c)
  { return _setChar(5, c.ch()); }

  FOG_INLINE err_t setExponential(Char32 c)
  { return _setChar(6, c.ch()); }
  
  FOG_INLINE err_t setFirstThousandsGroup(uint32_t i)
  { return _setChar(7, i); }
  
  FOG_INLINE err_t setNextThousandsGroup(uint32_t i)
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
  err_t _setChar(sysuint_t index, uint32_t uc);

public:
  // [Members]

  FOG_DECLARE_D(Data)
};

} // Fog namespace

//! @}

// ============================================================================
// [Fog::TypeInfo<>]
// ============================================================================

FOG_DECLARE_TYPEINFO(Fog::Locale, Fog::MoveableType)

// [Guard]
#endif // _FOG_CORE_LOCALE_H
