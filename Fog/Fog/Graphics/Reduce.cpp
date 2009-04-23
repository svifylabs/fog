// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/Hash.h>
#include <Fog/Core/Memory.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/ImageFormat.h>
#include <Fog/Graphics/Reduce.h>

#include <stdlib.h>

namespace Fog {

// ============================================================================
// [Fog::Reduce]
// ============================================================================

static int qsort_color_ascent(const void* _a, const void* _b)
{
  const Reduce::Entity* a = (const Reduce::Entity *)_a;
  const Reduce::Entity* b = (const Reduce::Entity *)_b;

  if (a->usage < b->usage)
    return -1;
  else if (a->usage > b->usage)
    return 1;
  else
    return 0;
}

/*
static int qsort_color_descent(const void* _a, const void* _b)
{
  const Reduce::Entity* a = (const Reduce::Entity *)_a;
  const Reduce::Entity* b = (const Reduce::Entity *)_b;

  if (a->usage < b->usage)
    return 1;
  else if (a->usage > b->usage)
    return -1;
  else
    return 0;
}
*/

Reduce::Reduce()
{
  clear();
}

Reduce::~Reduce()
{
}

void Reduce::clear()
{
  _indexes.clear();
  Memory::zero(_entities, sizeof(_entities));
  _count = 0;
}

bool Reduce::analyze(const Image& image)
{
  // first clear values
  clear();

  uint x;
  uint y;
  uint w = (uint)image.width();
  uint h = (uint)image.height();
  uint i;
  const uint8_t* p;

  Entity* e = _entities;

  if (image.format().isIndexed())
  {
    // It's guaranted maximum numbers of colors (256). So to calculate count
    // of colors is not hard. Entities are used directly.
    for (i = 0; i < 256; i++) e[i].key = i;

    for (y = 0; y < h; y++)
    {
      for (x = w, p = (const uint8_t*)image.cScanline(y); x; x--, p++)
      {
        e[p[0]].usage++;
      }
    }
  }
  else if (image.format().depth() == 32)
  {
    Hash<uint32_t, uint64_t> hash;

    for (y = 0; y < h; y++)
    {
      for (x = 0, p = (const uint8_t*)image.cScanline(y); x < w; x++, p += 4)
      {
        uint32_t c = *(const uint32_t *)p;

        // Increase count of 'c' if hash already contains it
        if (hash.contains(c))
          (*hash[c])++;
        // Create new node if there are not 256 colors already
        else if (hash.length() < 256)
          hash.put(c, 1);
        // Finished, no color reduction possible
        else
          return false;
      }
    }

    // If we are here, color reduction is possible, setup tables ...
    Hash<uint32_t, uint64_t>::ConstIterator iterator(hash);
    for (i = 0; iterator.isValid(); iterator.toNext(), i++)
    {
      e[i].key   = iterator.key();
      e[i].usage = iterator.value();
    }

    // Count items in hash table means count of colors used 
    _count = hash.length();
  }
  else
  {
    return false;
  }

  // sort, optimizes for PCX and all formats thats are dependent to 
  // color indexes (most used colors go first and unused last)
  qsort(e, count(), sizeof(Entity), qsort_color_ascent);

  // fix black and white images, it's usual that black color
  // is at index 0 
  if (count() == 2)
  {
    if (_entities[1].key == 0xFF000000)
    {
      Memory::xchg(&e[0], &e[1], sizeof(Entity));
    }
  }

  // create fast index table
  for (i = 0; i < count(); i++) _indexes.put(e[i].key, i);

  return true;
}

uint32_t Reduce::traslate(uint32_t key) const
{
  return _indexes.value(key);
}

} // Fog namespace
