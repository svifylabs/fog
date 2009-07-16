// [Fog Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Error.h>
#include <Fog/Core/Math.h>

// ============================================================================
// [Fog::Math]
// ============================================================================

namespace Fog {
namespace Math {

static FunctionMap functionMapData;

const FunctionMap* functionMap;

} // Math namespace
} // Fog namespace

// ============================================================================
// [Fog::Math - C]
// ============================================================================

namespace Fog {
namespace MathImplementation {

static void FOG_FASTCALL vAddArrayF(float* dst, const float* src1, const float* src2, sysuint_t size)
{
  for (sysuint_t i = 0; i < size; i++) { dst[i] = src1[i] + src2[i]; }
}

static void FOG_FASTCALL vSubArrayF(float* dst, const float* src1, const float* src2, sysuint_t size)
{
  for (sysuint_t i = 0; i < size; i++) { dst[i] = src1[i] - src2[i]; }
}

static void FOG_FASTCALL vMulArrayF(float* dst, const float* src1, const float* src2, sysuint_t size)
{
  for (sysuint_t i = 0; i < size; i++) { dst[i] = src1[i] * src2[i]; }
}

static void FOG_FASTCALL vDivArrayF(float* dst, const float* src1, const float* src2, sysuint_t size)
{
  for (sysuint_t i = 0; i < size; i++) { dst[i] = src1[i] / src2[i]; }
}

static void FOG_FASTCALL vAddScalarF(float* dst, const float* src1, float src2, sysuint_t size)
{
  for (sysuint_t i = 0; i < size; i++) { dst[i] = src1[i] + src2; }
}

static void FOG_FASTCALL vSubScalarF(float* dst, const float* src1, float src2, sysuint_t size)
{
  for (sysuint_t i = 0; i < size; i++) { dst[i] = src1[i] - src2; }
}

static void FOG_FASTCALL vMulScalarF(float* dst, const float* src1, float src2, sysuint_t size)
{
  for (sysuint_t i = 0; i < size; i++) { dst[i] = src1[i] * src2; }
}

static void FOG_FASTCALL vDivScalarF(float* dst, const float* src1, float src2, sysuint_t size)
{
  for (sysuint_t i = 0; i < size; i++) { dst[i] = src1[i] / src2; }
}

} // MathImplementation namespace
} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_math_init(void)
{
  Fog::Math::functionMap = &Fog::Math::functionMapData;
  Fog::Math::FunctionMap* m = &Fog::Math::functionMapData;

  m->vAddArrayF = Fog::MathImplementation::vAddArrayF;
  m->vSubArrayF = Fog::MathImplementation::vSubArrayF;
  m->vMulArrayF = Fog::MathImplementation::vMulArrayF;
  m->vDivArrayF = Fog::MathImplementation::vDivArrayF;

  m->vAddScalarF = Fog::MathImplementation::vAddScalarF;
  m->vSubScalarF = Fog::MathImplementation::vSubScalarF;
  m->vMulScalarF = Fog::MathImplementation::vMulScalarF;
  m->vDivScalarF = Fog::MathImplementation::vDivScalarF;

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_math_shutdown(void)
{
}
