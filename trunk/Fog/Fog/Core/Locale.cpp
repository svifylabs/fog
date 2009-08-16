// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif // _MSC_VER

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
  _d(sharedNull->refAlways())
{
}

Locale::Locale(const Locale& other) :
  _d(other._d->ref())
{
}

Locale::Locale(Data* d) :
  _d(d)
{
}

Locale::Locale(const String32& name) :
  _d(sharedNull->refAlways())
{
  set(name);
}

Locale::~Locale()
{
  _d->deref();
}

err_t Locale::_detach()
{
  if (_d->refCount.get() == 1) return Error::Ok;

  Data* newd = new Data(*_d);
  if (newd) return Error::OutOfMemory;

  AtomicBase::ptr_setXchg(&_d, newd)->deref();
  return Error::Ok;
}

void Locale::free()
{
  AtomicBase::ptr_setXchg(&_d, sharedNull->ref())->deref();
}

// [Set]

bool Locale::set(const String32& name)
{
  char lcNameA[512];
  if (!StringUtil::utf32ToLatin((Char8*)lcNameA, (Char32*)name.cData(), name.getLength())) return false;

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
  AtomicBase::ptr_setXchg(&_d, other._d->ref())->deref();
  return true;
}

err_t Locale::_setChar(sysuint_t index, uint32_t uc)
{
  if (index >= Data::N) return Error::InvalidArgument;
  if (_d->data[index] == uc) return Error::Ok;

  err_t err;
  if ((err = detach())) return err;

  _d->data[index] = uc;
  return Error::Ok;
}

// ============================================================================
// [Fog::Locale::Data]
// ============================================================================

Locale::Data::Data()
{
  refCount.init(1);
  memset(data, 0, sizeof(uint32_t) * N);
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

Locale::Data* Locale::Data::ref() const
{
  return refAlways();
}

void Locale::Data::deref()
{
  if (refCount.deref()) delete this;
}

Locale::Data* Locale::Data::copy(Data* d)
{
  return new(std::nothrow) Data(*d);
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_locale_init(void)
{
  using namespace Fog;

  Locale::sharedNull.init();
  Locale::sharedNull->name = Ascii8("NULL");
  _setDefaults(Locale::sharedNull.instancep());

  Locale::sharedPosix.init();
  Locale::sharedPosix->name= Ascii8("POSIX");
  _setDefaults(Locale::sharedPosix.instancep());

  Locale::sharedUser.init();
  _setLConv(Locale::sharedUser.instancep(), localeconv());

  new (&Locale::sharedNullObj ) Locale(Locale::sharedNull ->refAlways());
  new (&Locale::sharedPosixObj) Locale(Locale::sharedPosix->refAlways());
  new (&Locale::sharedUserObj ) Locale(Locale::sharedUser ->refAlways());

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_locale_shutdown(void)
{
  using namespace Fog;

  ((Locale *)&Locale::sharedNullObj )->~Locale();
  ((Locale *)&Locale::sharedPosixObj)->~Locale();
  ((Locale *)&Locale::sharedUserObj )->~Locale();

  Locale::sharedNull.destroy();
  Locale::sharedPosix.destroy();
  Locale::sharedUser.destroy();
}
