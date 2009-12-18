// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_GLYPHCACHE_H
#define _FOG_GRAPHICS_GLYPHCACHE_H

// [Dependencies]
#include <Fog/Graphics/Glyph.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Fog::GlyphCache]
// ============================================================================

//! @brief Glyph cache.
//!
//! Glyphs are stored in sparse Nx256 array. First pointer points to second
//! array where are stored real glyphs. This cache is per face per attribute
//! and access is very very fast.
//!
//! Address for glyph 'ch' is row[ch >> 8][ch & 255]
struct FOG_API GlyphCache
{
  typedef Glyph::Data* Entity;

  GlyphCache();
  ~GlyphCache();

  bool set(uint32_t uc, Entity data);

  FOG_INLINE Entity get(uint32_t uc) const
  {
    Entity* row;
    uint32_t ucRow = uc >> 8;
    return ucRow < _count && (row = _rows[ucRow]) ? row[uc & 0xFF] : 0;
  }

  void free();

private:
  //! @brief Pointers to glyphs
  Entity** _rows;
  //! @brief Count of first rows pointer. Initial value is 0
  sysuint_t _count;

private:
  FOG_DISABLE_COPY(GlyphCache)
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_GLYPHCACHE_H
