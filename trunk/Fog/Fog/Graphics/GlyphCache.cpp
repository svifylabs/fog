// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Graphics/GlyphCache.h>

namespace Fog {

// ============================================================================
// [Fog::GlyphCache]
// ============================================================================

GlyphCache::GlyphCache() :
  _rows(0),
  _count(0)
{
}

GlyphCache::~GlyphCache()
{
  free();
}

bool GlyphCache::set(uint32_t uc, Entity data)
{
  // First reject too high character
  if (uc > 65535) return false;

  uint32_t ucRow = uc >> 8;
  uint32_t ucCol = uc & 0xFF;

  // Realloc ROWS array if needed.
  if (FOG_UNLIKELY(ucRow >= _count))
  {
    _rows = (Entity**)Memory::xrealloc(_rows, (ucRow + 1) * sizeof(Entity));
    Memory::zero(_rows + _count, (ucRow - _count + 1) * sizeof(Entity));
    _count = ucRow + 1;
  }

  // Alloc COL array if needed.
  if (FOG_UNLIKELY(_rows[ucRow] == NULL))
  {
    _rows[ucRow] = (Entity*)Memory::xcalloc(256 * sizeof(Entity));
  }

  _rows[ucRow][ucCol] = data;
  return true;
}

void GlyphCache::free()
{
  sysuint_t i, j;
  sysuint_t count = _count;
  Entity** rows = _rows;
  Entity* row;

  for (i = 0; i != count; i++, rows++)
  {
    if ((row = *rows) != NULL)
    {
      for (j = 0; j != 256; j++)
      {
        if (row[j]) row[j]->deref();
      }
      Memory::free(row);
    }
  }
  Memory::free(_rows);

  _rows = 0;
  _count = 0;
}

} // Fog namespace
