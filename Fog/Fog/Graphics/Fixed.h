// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_FIXED_H
#define _FOG_GRAPHICS_FIXED_H

#include <Fog/Build/Build.h>

// This is library for fixed point math

// Cleanup mess from other libraries!
#if defined(min)
#undef min
#endif // min
#if defined(max)
#undef max
#endif // max

namespace Fog {

// ============================================================================
// [Fog::Fixed16x16]
// ============================================================================

struct Fixed16x16
{
  // [Important constants]

  enum { Bits = 16 };
  enum { One = 1 << Bits };

  // [Constructors / From methods]

  FOG_INLINE Fixed16x16() 
  {}

  FOG_INLINE Fixed16x16(const Fixed16x16& other) :
    _fixed(other._fixed)
  {}

  static FOG_INLINE Fixed16x16 fromInt(int val)
  { return Fixed16x16( val << Bits ); }

  static FOG_INLINE Fixed16x16 fromFixed(int val)
  { return Fixed16x16( val ); }

  static FOG_INLINE Fixed16x16 fromFixed(const Fixed16x16& val)
  { return Fixed16x16( val._fixed ); }

  static FOG_INLINE Fixed16x16 fromFloat(float val)
  { return Fixed16x16( (int)(val * (float)One + (val < 0 ? -0.5 : 0.5)) ); }

  static FOG_INLINE Fixed16x16 fromDouble(double val)
  { return Fixed16x16( (int)(val * (double)One + (val < 0 ? -0.5 : 0.5)) ); }

  // [To methods]
  FOG_INLINE int toInt() const
  { return _fixed >> Bits; }

  FOG_INLINE int toIntRound() const
  { return (_fixed + (One >> 1)) >> Bits; }

  FOG_INLINE float toFloat() const
  { return (float)_fixed / (float)One; }

  FOG_INLINE double toDouble() const
  { return (double)_fixed / (double)One; }

  // [Value]

  FOG_INLINE int32_t fixed() const
  { return _fixed; }

  FOG_INLINE int32_t fraction() const
  { return _fixed & (One - 1); }

  // [Basic operations]

  FOG_INLINE Fixed16x16& add(int a)
  { _fixed += (a << Bits); return *this; }

  FOG_INLINE Fixed16x16& add(Fixed16x16 a)
  { _fixed += a._fixed; return *this; }

  FOG_INLINE Fixed16x16& sub(int a)
  { _fixed -= (a << Bits); return *this; }

  FOG_INLINE Fixed16x16& sub(Fixed16x16 a)
  { _fixed -= a._fixed; return *this; }

  FOG_INLINE Fixed16x16& mul(int a)
  { _fixed *= a; return *this; }

  FOG_INLINE Fixed16x16& mul(Fixed16x16 a)
  { _fixed = (int32_t)( ((int64_t)_fixed * a._fixed ) >> 16 ); return *this; }

  FOG_INLINE Fixed16x16& mul32(Fixed16x16 a)
  { _fixed = (int32_t)( (_fixed * a._fixed) >> 16 ); return *this; }

  FOG_INLINE Fixed16x16& div(int a)
  { _fixed /= a; return *this; }

  // [Operator Overload]

  FOG_INLINE Fixed16x16& operator+=(int a)
  { return add(a); }

  FOG_INLINE Fixed16x16& operator+=(Fixed16x16 a)
  { return add(a); }

  FOG_INLINE Fixed16x16& operator-=(int a)
  { return sub(a); }

  FOG_INLINE Fixed16x16& operator-=(Fixed16x16 a)
  { return sub(a); }

  FOG_INLINE Fixed16x16& operator*=(int a)
  { return mul(a); }

  FOG_INLINE Fixed16x16& operator*=(Fixed16x16 a)
  { return mul(a); }

  FOG_INLINE Fixed16x16& operator/=(int a)
  { return div(a); }

  FOG_INLINE Fixed16x16 operator+(int a)
  { return Fixed16x16( _fixed + (a << Bits) ); }

  FOG_INLINE Fixed16x16 operator+(Fixed16x16 a)
  { return Fixed16x16( _fixed + a._fixed ); }

  FOG_INLINE Fixed16x16 operator-(int a)
  { return Fixed16x16( _fixed - (a << Bits) ); }

  FOG_INLINE Fixed16x16 operator-(Fixed16x16 a)
  { return Fixed16x16( _fixed - a._fixed ); }

  FOG_INLINE Fixed16x16 operator*(int a)
  { return Fixed16x16( _fixed * a ); }

  FOG_INLINE Fixed16x16 operator*(Fixed16x16 a)
  { return Fixed16x16( (int32_t)( ((int64_t)_fixed * a._fixed ) >> 16 ) ); }

  FOG_INLINE Fixed16x16 operator/(int a)
  { return Fixed16x16( _fixed / a ); }

  FOG_INLINE bool operator==(const Fixed16x16& other) { return _fixed == other._fixed; }
  FOG_INLINE bool operator!=(const Fixed16x16& other) { return _fixed != other._fixed; }
  FOG_INLINE bool operator< (const Fixed16x16& other) { return _fixed <  other._fixed; }
  FOG_INLINE bool operator<=(const Fixed16x16& other) { return _fixed <= other._fixed; }
  FOG_INLINE bool operator> (const Fixed16x16& other) { return _fixed >  other._fixed; }
  FOG_INLINE bool operator>=(const Fixed16x16& other) { return _fixed >= other._fixed; }

  // [Math functions]

  static FOG_INLINE Fixed16x16 min(Fixed16x16 a, Fixed16x16 b)
  { return Fixed16x16( a < b ? a : b ); }

  static FOG_INLINE Fixed16x16 max(Fixed16x16 a, Fixed16x16 b)
  { return Fixed16x16( a > b ? a : b ); }

  FOG_INLINE Fixed16x16 abs() const
  { return Fixed16x16( _fixed < 0 ? -_fixed : _fixed ); }

  FOG_INLINE Fixed16x16 inv() const
  { return Fixed16x16( -_fixed ); }

  // Some information about this algorithm can be found here:
  // http://www.worldserver.com/turk/computergraphics/FixedSqrt.pdf
  FOG_INLINE Fixed16x16 sqrt() const
  {
    // Clear root
    uint32_t root = 0;
    // Clear high part of partial remainder
    uint32_t remHi = 0;
    // Get argument into low part of partial remainder
    uint32_t remLo = _fixed;
    // Loop counter
    uint32_t count = (15 + (16 >> 1));

    uint32_t test;

    do {
      remHi = (remHi << 2) | (remLo >> 30);
      remLo <<= 2; // get 2 bits of arg
      root <<= 1; // Get ready for the next bit in the root
      test = (root << 1) + 1; // Test radical
      if (remHi >= test)
      {
        remHi -= test;
        root++;
      }
    } while (count-- != 0);

    return Fixed16x16((int32_t)root);
  }

private:
  FOG_INLINE Fixed16x16(int fixed) :
    _fixed(fixed)
  {}

  int32_t _fixed;
};

} // Fog namespace

#endif // _FOG_GRAPHICS_FIXED_H
