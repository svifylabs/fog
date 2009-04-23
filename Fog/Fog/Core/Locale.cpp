// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Assert.h>
#include <Fog/Core/Error.h>
#include <Fog/Core/Locale.h>
#include <Fog/Core/String.h>
#include <Fog/Core/StringUtil.h>
#include <Fog/Core/TextCodec.h>

#include <wchar.h>
#include <locale.h>

#if defined(FOG_HAVE_XLOCALE_H) && defined(FOG_HAVE_XLOCALE_H_LOCALECONV_L)
#include <xlocale.h>
#endif

namespace Fog {

// ============================================================================
// [Fog::Locale]
// ============================================================================

Static<Locale::Data> Locale::sharedNull;
Static<Locale::Data> Locale::sharedPosix;
Static<Locale::Data> Locale::sharedUser;

Locale::Data* Locale::sharedNullObj;
Locale::Data* Locale::sharedPosixObj;
Locale::Data* Locale::sharedUserObj;

static void _setDefaults(Locale::Data* d)
{
  d->decimalPoint = '.';
  d->thousandsGroup = '\0';
  d->zero = '0';
  d->plus = '+';
  d->minus = '-';
  d->space = ' ';
  d->exponential = 'e';
  d->firstThousandsGroup = 3;
  d->nextThousandsGroup = 3;
  d->reserved = 0;
}

static void _setLConv(Locale::Data* d, const struct lconv* conv)
{
  _setDefaults(d);

  if (conv)
  {
    wchar_t buf[4];

    if (mbstowcs(buf, conv->decimal_point, 3) == 1) d->decimalPoint   = (uint32_t)buf[0];
    if (mbstowcs(buf, conv->thousands_sep, 3) == 1) d->thousandsGroup = (uint32_t)buf[0];
    if (mbstowcs(buf, conv->positive_sign, 3) == 1) d->plus           = (uint32_t)buf[0];
    if (mbstowcs(buf, conv->negative_sign, 3) == 1) d->minus          = (uint32_t)buf[0];
  }
}

Locale::Locale() :
  _d(sharedNull.instancep()->REF_INLINE())
{
}

Locale::Locale(Data* d) :
  _d(d)
{
}

Locale::Locale(const Locale& other) :
  _d(other._d->REF_INLINE())
{
}

Locale::Locale(const String32& name) :
  _d(sharedNull.instancep()->REF_INLINE())
{
  set(name);
}

Locale::~Locale()
{
  _d->DEREF_INLINE();
}

void Locale::_detach()
{
  if (refCount() > 1)
  {
    Data* newd = new Data(*_d);
    AtomicBase::ptr_setXchg(&_d, newd)->DEREF_INLINE();
  }
}

void Locale::free()
{
  AtomicBase::ptr_setXchg(&_d, sharedNull.instancep()->REF_INLINE())->DEREF_INLINE();
}

// [Set]

bool Locale::set(const String32& name)
{
  char lcNameA[512];
  if (!StringUtil::utf32ToLatin((Char8*)lcNameA, (Char32*)name.cData(), name.length())) return false;

  TemporaryString8<128> savedLocale(Stub8(setlocale(LC_ALL, NULL)));

  if (setlocale(LC_ALL, lcNameA))
  {
    detach();
    _d->name = name;
    _setLConv(_d, localeconv());
  }
  else 
  {
    free();
  }

  setlocale(LC_ALL, savedLocale.cStr());
  return !isNull();
}

bool Locale::set(const Locale& other)
{
  AtomicBase::ptr_setXchg(&_d, other._d->REF_INLINE())->DEREF_INLINE();
  return true;
}

Locale& Locale::_setChar(sysuint_t index, uint32_t uc)
{
  FOG_ASSERT(index < Data::N);

  if (_d->data[index] != uc)
  {
    detach();
    _d->data[index] = uc;
  }

  return *this;
}

// [Fog::Locale::Data]

Locale::Data::Data()
{
  refCount.init(1);
}

Locale::Data::Data(const Data& other) :
  name(other.name)
{
  refCount.init(1);
  memcpy(data, other.data, sizeof(uint32_t) * N);
}

Locale::Data::~Data()
{
}

// [Ref]

Locale::Data* Locale::Data::ref()
{
  return REF_INLINE();
}

void Locale::Data::deref()
{
  DEREF_INLINE();
}

Locale::Data* Locale::Data::copy(Data* d, uint allocPolicy)
{
  Data* newd = new Data(*d);

  // TODO: allocPolicy

  return newd;
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_locale_init(void)
{
  Fog::Locale::sharedNull.init();
  Fog::Locale::sharedNull->name = Fog::StubAscii8("NULL");
  Fog::_setDefaults(Fog::Locale::sharedNull.instancep());

  Fog::Locale::sharedPosix.init();
  Fog::Locale::sharedPosix->name= Fog::StubAscii8("POSIX");
  Fog::_setDefaults(Fog::Locale::sharedPosix.instancep());

  Fog::Locale::sharedUser.init();
  Fog::_setLConv(Fog::Locale::sharedUser.instancep(), localeconv());

  new (&Fog::Locale::sharedNullObj ) Fog::Locale(Fog::Locale::sharedNull .instancep()->REF_INLINE());
  new (&Fog::Locale::sharedPosixObj) Fog::Locale(Fog::Locale::sharedPosix.instancep()->REF_INLINE());
  new (&Fog::Locale::sharedUserObj ) Fog::Locale(Fog::Locale::sharedUser .instancep()->REF_INLINE());

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_locale_shutdown(void)
{
  ((Fog::Locale *)&Fog::Locale::sharedNullObj )->~Locale();
  ((Fog::Locale *)&Fog::Locale::sharedPosixObj)->~Locale();
  ((Fog::Locale *)&Fog::Locale::sharedUserObj )->~Locale();

  Fog::Locale::sharedNull.destroy();
  Fog::Locale::sharedPosix.destroy();
  Fog::Locale::sharedUser.destroy();
}
