// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Graphics/Constants.h>

namespace Fog {

#define DST_C OPERATOR_CHAR_DST_C_USED
#define DST_A OPERATOR_CHAR_DST_A_USED
#define SRC_C OPERATOR_CHAR_SRC_C_USED
#define SRC_A OPERATOR_CHAR_SRC_A_USED
#define NOP   OPERATOR_CHAR_NOP
#define BOUND OPERATOR_CHAR_BOUND

uint32_t OperatorCharacteristics[OPERATOR_COUNT] =
{
  0     | 0     | SRC_C | SRC_A | 0,      // OPERATOR_SRC
  DST_C | DST_A | 0     | 0     | NOP,    // OPERATOR_DST
  DST_C | DST_A | SRC_C | SRC_A | BOUND,  // OPERATOR_SRC_OVER
  DST_C | DST_A | SRC_C | SRC_A | BOUND,  // OPERATOR_DST_OVER
  0     | DST_A | SRC_C | SRC_A | 0,      // OPERATOR_SRC_IN
  DST_C | DST_A | 0     | SRC_A | 0,      // OPERATOR_DST_IN
  0     | DST_A | SRC_C | SRC_A | 0,      // OPERATOR_SRC_OUT
  DST_C | DST_A | 0     | SRC_A | BOUND,  // OPERATOR_DST_OUT
  DST_C | DST_A | SRC_C | SRC_A | BOUND,  // OPERATOR_SRC_ATOP
  DST_C | DST_A | SRC_C | SRC_A | 0,      // OPERATOR_DST_ATOP
  DST_C | DST_A | SRC_C | SRC_A | BOUND,  // OPERATOR_XOR
  0     | 0     | 0     | 0     | 0,      // OPERATOR_CLEAR
  DST_C | DST_A | SRC_C | SRC_A | BOUND,  // OPERATOR_ADD
  DST_C | DST_A | SRC_C | SRC_A | BOUND,  // OPERATOR_SUBTRACT
#if 0
  DST_C | DST_A | SRC_C | SRC_A | 0,      // OPERATOR_MULTIPLY
#endif
  DST_C | DST_A | SRC_C | SRC_A | BOUND,  // OPERATOR_SCREEN
  DST_C | DST_A | SRC_C | SRC_A | BOUND,  // OPERATOR_DARKEN
  DST_C | DST_A | SRC_C | SRC_A | BOUND,  // OPERATOR_LIGHTEN
  DST_C | DST_A | SRC_C | SRC_A | BOUND,  // OPERATOR_DIFFERENCE
  DST_C | DST_A | SRC_C | SRC_A | BOUND,  // OPERATOR_EXCLUSION
  DST_C | DST_A | 0     | SRC_A | BOUND,  // OPERATOR_INVERT
  DST_C | DST_A | SRC_C | SRC_A | BOUND   // OPERATOR_INVERT_RGB
};

#undef DST_C
#undef DST_A
#undef SRC_C
#undef SRC_A
#undef NOP
#undef BOUND

} // Fog namespace
