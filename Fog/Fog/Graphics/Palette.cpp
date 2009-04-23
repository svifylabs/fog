// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/Assert.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/Static.h>
#include <Fog/Graphics/Palette.h>

namespace Fog {

// [Fog::Palette]

Fog::Static<Palette::Data> Palette::sharedNull;
Fog::Static<Palette::Data> Palette::sharedGrey;

Palette::Palette() : _d(sharedNull.instancep()->REF_INLINE()) {}
Palette::Palette(const Palette& other) : _d(other._d->REF_INLINE()) {}
Palette::Palette(Data* d) : _d(d) {}

Palette::~Palette()
{
  _d->DEREF_INLINE();
}

void Palette::_detach()
{
  Data* newd = Data::copy(_d, Fog::AllocCantFail);
  Fog::AtomicBase::ptr_setXchg(&_d, newd)->deref();
}

bool Palette::_tryDetach()
{
  Data* newd = Data::copy(_d, Fog::AllocCanFail);

  if (newd)
  {
    Fog::AtomicBase::ptr_setXchg(&_d, newd)->deref();
  }

  return newd != 0;
}

void Palette::free()
{
  _d->DEREF_INLINE();
  _d = sharedNull.instancep()->REF_INLINE();
}

Palette& Palette::set(const Fog::Palette& other)
{
  if (isStrong() || !other.isSharable())
    set(0, 256, other.cData());
  else
    Fog::AtomicBase::ptr_setXchg(&_d, other._d->ref())->deref();

  return *this;
}

Palette& Palette::set(sysuint_t index, sysuint_t count, const Rgba* rgba)
{
  if (index > 255 || count == 0) return *this;
  if (256 - index > count) count = 256 - index;

  detach();
  Fog::Memory::copy(_d->data + index, rgba, sizeof(Rgba) * count);

  return *this;
}

Palette& Palette::greyscale()
{
  if (isStrong())
  {
    return set(0, 256, sharedGrey.instancep()->data);
  }
  else
  {
    Fog::AtomicBase::ptr_setXchg(&_d, sharedGrey.instancep()->REF_ALWAYS())->deref();
    return *this;
  }
}

// static
bool Palette::isGreyOnly(const Rgba* data, sysuint_t count)
{
  sysuint_t i;

  for (i = count; i; i--, data++)
  {
    if (data->r != data->g || data->g != data->b) break;
  }

  return i == 0;
}

// [Fog::Palette::Data]
Palette::Data* Palette::Data::ref()
{
  return REF_INLINE();
}

void Palette::Data::deref()
{
  DEREF_INLINE();
}

Palette::Data* Palette::Data::create(uint allocPolicy)
{
  Data* d = (Data*)Fog::Memory::alloc(sizeof(Data));

  if (d)
  {
    d->refCount.init(1);
    d->flags.set(IsDynamic | IsSharable);
  }
  else if (allocPolicy == Fog::AllocCantFail)
  {
    fog_out_of_memory_fatal_format(
      "Fog::Palette::Data", "create", 
      "Couldn't allocate %lu bytes of memory for palette data", (ulong)sizeof(Data));
  }

  return d;
}

Palette::Data* Palette::Data::copy(const Data* other, uint allocPolicy)
{
  Data* d = create(allocPolicy);

  if (d)
  {
    Fog::Memory::copy(d->data, other->data, sizeof(Rgba) * 256);
  }

  return d;
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_palette_init(void)
{
  Fog::Palette::Data* d;

  Fog::Palette::sharedNull.init();
  Fog::Palette::sharedGrey.init();

  // Setup null palette;
  d = Fog::Palette::sharedNull.instancep();
  d->refCount.init(1);
  d->flags.init(
    Fog::Palette::Data::IsSharable | 
    Fog::Palette::Data::IsNull);
  Fog::Memory::zero(d->data, 256 * sizeof(Fog::Rgba));

  // Setup grey palette;
  d = Fog::Palette::sharedGrey.instancep();
  d->refCount.init(1);
  d->flags.init(
    Fog::Palette::Data::IsSharable);

  sysuint_t i;
  for (i = 0; i != 256; i++)
  {
    d->data[i] = Fog::Rgba(
      (uint8_t)i, 
      (uint8_t)i, 
      (uint8_t)i, 
      (uint8_t)0xFF);
  }

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_palette_shutdown(void)
{
  Fog::Palette::sharedGrey.instancep()->refCount.dec();
  Fog::Palette::sharedGrey.destroy();
  Fog::Palette::sharedNull.instancep()->refCount.dec();
  Fog::Palette::sharedNull.destroy();
}
