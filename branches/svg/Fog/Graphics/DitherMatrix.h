// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_DITHERMATRIX_H
#define _FOG_GRAPHICS_DITHERMATRIX_H

// [Dependencies]
#include <Fog/Build/Build.h>

namespace Fog {

// ============================================================================
// [Fog::DitherMatrix]
// ============================================================================

//! @brief Functions and tables that can be used to dithering
//! implementation
//!
//! You probabbly don't need to use this class, because it's mainly
//! for internals in Fog library. It's used by pixel converters with
//! dithering enabled.
//!
//! @note Dithering can be configured to use small dithering tables
//! instead of large ones that can save 16kb of library size. Default
//! are large dithering tables that produces better images.
struct FOG_API DitherMatrix
{
  // [Constants]

  enum {
#ifdef FOG_SMALL_DITHER
    /*! @brief Size of dithering table. */
    Size = 4,        // Always 2^N
    Bits = 4,        // Count of bits used per element
    Div = 16,
    Mask = Size - 1
#else
    /*! @brief Size of dithering table. */
    Size = 128,      // Always 2^N
    Bits = 6,        // Count of bits used per element
    Div = 64,
    Mask = Size - 1
#endif // FOG_SMALL_DITHER
  };

  // [Table]

  // To find correct position in dithering table, use this formula:
  // table[X & Mask][Y & Mask]
  static const uint8_t matrix[Size][Size];

  // [Helpers]
  //
  // Helpers that helps us to use this dithering table for
  // more image types and bit depths

  /*! @brief Shifting argument helper. */
  template<typename T>
  static FOG_INLINE T shf_arg(const T& t)
  {
    return Bits - (8 - t);
  }

  /*! @brief Shifting dither helper. */
  template<typename T>
  static FOG_INLINE T shf_dither(T value, int shift)
  {
    if (shift >= 0)
      return value >> (uint)shift;
    else
      return value << (uint)(-shift);
  }
};

} // Fog namespace

// [Guard]
#endif // _FOG_GRAPHICS_DITHERMATRIX_H
