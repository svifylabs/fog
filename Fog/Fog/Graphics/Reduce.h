#ifndef _FOG_GRAPHICS_REDUCE_H
#define _FOG_GRAPHICS_REDUCE_H

#include <Fog/Core/Hash.h>
#include <Fog/Graphics/Image.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Fog::Reduce]
// ============================================================================

//! @brief Class used to get information about colors in image.
//!
//! It's designed for tasks where it's needed or optional to reduce color
//! depth, so this class can calculate count of colors and generate palette 
//! for you.
//!
//! If count of colors are larger than 256, operation is stopped and count
//! of colors are set to zero. Zero means "not able to reduce depth".
struct FOG_API Reduce
{
  // [Construction / Destruction]

  Reduce();
  ~Reduce();

  // [Entity]

  //! @brief Rgba entity with usage information.
  struct Entity
  {
    uint32_t key;
    uint64_t usage;
  };

  // [Methods]

  FOG_INLINE const Hash<uint32_t, uint8_t>& indexes() const
  { return _indexes; }

  FOG_INLINE const Entity* entities() const
  { return _entities; }

  FOG_INLINE uint32_t count() const
  { return _count; }

  void clear();
  bool analyze(const Image& image, bool discardAlpha = false);

  uint32_t traslate(uint32_t key) const;

  Palette toPalette();

private:
  //! @brief Color to index hash.
  Hash<uint32_t, uint8_t> _indexes;

  //! @brief Entities.
  Entity _entities[256];

  //! @brief Count of colors / entities in image or zero.
  uint32_t _count;

  FOG_DISABLE_COPY(Reduce)
};

} // Fog namespace

//! @}

#endif // _FOG_GRAPHICS_REDUCE_H
