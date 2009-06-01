// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Dependencies]
#include <Fog/Core/Algorithms.h>

namespace Fog {

// ============================================================================
// [Fog::QSortBase]
// ============================================================================

void* QSortBase::_med3(void* a, void* b, void* c, TypeInfo_CompareFn compar)
{
  int cmp_ab = compar(a, b);
  int cmp_bc = compar(b, c);
  int cmp_ac = compar(a, c);

  return cmp_ab < 0 ? (cmp_bc < 0 ? b : (cmp_ac < 0 ? c : a))
                    : (cmp_bc > 0 ? b : (cmp_ac < 0 ? a : c));
}

} // Fog namespace
