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

// ============================================================================
// [Fog::Palette]
// ============================================================================

Static<Palette::Data> Palette::sharedNull;
Static<Palette::Data> Palette::sharedGrey;

Palette::Palette() : _d(sharedNull->refAlways()) {}
Palette::Palette(const Palette& other) : _d(other._d->refAlways()) {}
Palette::Palette(Data* d) : _d(d) {}

Palette::~Palette()
{
  _d->deref();
}

err_t Palette::_detach()
{
  if (isDetached()) return Error::Ok;

  Data* newd = Data::copy(_d);
  if (!newd) return Error::OutOfMemory;

  AtomicBase::ptr_setXchg(&_d, newd)->deref();
  return Error::Ok;
}

void Palette::free()
{
  AtomicBase::ptr_setXchg(&_d, sharedNull->refAlways())->deref();
}

void Palette::clear()
{
  if (isDetached())
    Memory::zero(_d->data, 256 * sizeof(Rgba));
  else
    AtomicBase::ptr_setXchg(&_d, sharedNull->refAlways())->deref();
}

err_t Palette::set(const Palette& other)
{
  AtomicBase::ptr_setXchg(&_d, other._d->ref())->deref();
  return Error::Ok;
}

err_t Palette::setDeep(const Palette& other)
{
  return set(0, 256, other.cData());
}

err_t Palette::set(sysuint_t index, sysuint_t count, const Rgba* rgba)
{
  if (index > 255 || count == 0) return Error::InvalidArgument;
  if (256 - index > count) count = 256 - index;

  err_t err = detach();
  if (err) return err;

  Memory::copy(_d->data + index, rgba, sizeof(Rgba) * count);
  return Error::Ok;
}

uint8_t Palette::findColor(uint8_t r, uint8_t g, uint8_t b) const
{
  uint8_t best = 0;

  int i;
  int smallest = INT_MAX;

  const Rgba* data = _d->data;

  for (i = 0; i != 256; i++, data++)
  {
    int rd = (int)data->r - (int)r;
    int gd = (int)data->g - (int)g;
    int bd = (int)data->b - (int)b;

    int dist = (rd * rd) + (gd * gd) + (bd * bd);
    if (dist < smallest)
    {
      if (dist == 0) return i;
      best = i;
      smallest = dist;
    }
  }
  return best;
}

Palette Palette::greyscale()
{
  return Palette(sharedGrey->refAlways());
}

Palette Palette::colorCube(int nr, int ng, int nb)
{
  Palette pal;

  if (nr <= 0 || ng <= 0 || nb <= 0) return pal;

  Rgba* data = pal.mData();
  if (data == NULL) return pal;

  int i = 0, r, g, b;

  for (r = 0; r < nr; r++)
  {
    for (g = 0; g < ng; g++)
    {
      for (b = 0; b < nb; b++)
      {
        int palr = 0;
        int palg = 0;
        int palb = 0;

        if (nr > 1) palr = (r * 255) / (nr - 1);
        if (ng > 1) palg = (g * 255) / (ng - 1);
        if (nb > 1) palb = (b * 255) / (nb - 1);

        data[i] = Rgba(palr, palg, palb);
        if (++i == 256) goto end;
      }
    }
  }

end:
  return pal;
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

// ============================================================================
// [Fog::Palette::Data]
// ============================================================================

Palette::Data* Palette::Data::ref() const
{
  return refAlways();
}

void Palette::Data::deref()
{
  if (refCount.deref()) Memory::free(this);
}

Palette::Data* Palette::Data::create()
{
  Data* d = (Data*)Memory::alloc(sizeof(Data));

  if (d)
  {
    d->refCount.init(1);
  }

  return d;
}

Palette::Data* Palette::Data::copy(const Data* other)
{
  Data* d = create();

  if (d)
  {
    Memory::copy(d->data, other->data, sizeof(Rgba) * 256);
  }

  return d;
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_palette_init(void)
{
  using namespace Fog;

  Palette::Data* d;

  Palette::sharedNull;
  Palette::sharedGrey;

  // Setup null palette;
  d = Palette::sharedNull.instancep();
  d->refCount.init(1);
  Memory::zero(d->data, 256 * sizeof(Rgba));

  // Setup grey palette;
  d = Palette::sharedGrey.instancep();
  d->refCount.init(1);

  for (uint32_t i = 256, c0 = 0xFF000000; i; i--, c0 += 0x00010101)
  {
    d->data[i] = c0;
  }

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_palette_shutdown(void)
{
  using namespace Fog;

  Palette::sharedGrey.instancep()->refCount.dec();
  Palette::sharedNull.instancep()->refCount.dec();
}
