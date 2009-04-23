// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_MATH_H
#define _FOG_CORE_MATH_H

// [Dependencies]
#include <Fog/Build/Build.h>

#include <math.h>

// ============================================================================
// [Fog::Math]
// ============================================================================

// Some constants are not defined by Microsoft compiler

// Constants with 'l' suffix are gcc extensions for long double
// types

// e
#if !defined(M_E)
#define M_E            2.7182818284590452354
#define M_El           2.7182818284590452353602874713526625L
#endif

// log_2 e
#if !defined(M_LOG2E)
#define M_LOG2E        1.4426950408889634074
#define M_LOG2El       1.4426950408889634073599246810018921L
#endif

// log_10 e
#if !defined(M_LOG10E)
#define M_LOG10E       0.43429448190325182765
#define M_LOG10El      0.4342944819032518276511289189166051L
#endif

// log_e 2
#if !defined(M_LN2)
#define M_LN2          0.69314718055994530942
#define M_LN2l         0.6931471805599453094172321214581766L
#endif

// log_e 10
#if !defined(M_LN10)
#define M_LN10         2.30258509299404568402
#define M_LN10l        2.3025850929940456840179914546843642L
#endif

// pi
#if !defined(M_PI)
#define M_PI           3.14159265358979323846
#define M_PIl          3.1415926535897932384626433832795029L
#endif

// pi/2
#if !defined(M_PI_2)
#define M_PI_2         1.57079632679489661923
#define M_PI_2l        1.5707963267948966192313216916397514L
#endif

// pi/4
#if !defined(M_PI_4)
#define M_PI_4         0.78539816339744830962
#define M_PI_4l        0.7853981633974483096156608458198757L
#endif

// 1/pi
#if !defined(M_1_PI)
#define M_1_PI         0.31830988618379067154
#define M_1_PIl        0.3183098861837906715377675267450287L
#endif

// 2/pi
#if !defined(M_2_PI)
#define M_2_PI         0.63661977236758134308
#define M_2_PIl        0.6366197723675813430755350534900574L
#endif

// 2/sqrt(pi)
#if !defined(M_2_SQRTPI)
#define M_2_SQRTPI     1.12837916709551257390
#define M_2_SQRTPIl    1.1283791670955125738961589031215452L
#endif

// sqrt(2)
#if !defined(M_SQRT2)
#define M_SQRT2        1.41421356237309504880
#define M_SQRT2l       1.4142135623730950488016887242096981L
#endif

// 1/sqrt(2)
#if !defined(M_SQRT1_2)
#define M_SQRT1_2      0.70710678118654752440
#define M_SQRT1_2l     0.7071067811865475244008443621048490L
#endif

// [NaN and Inf]

// Not defined NAN ?
#if !defined(NAN)

#if defined(FOG_CC_GNU)
#define NAN __builtin_nan("")
#elif defined(FOG_ARCH_X86) || defined(FOG_ARCH_X86_64)
// For MSVC! Is there any other compiler that will not declare NAN ?
static FOG_HIDDEN const uint32_t __Wde_NAN__[2] = { 0xFFFFFFFF,0x7FFFFFFF };
#define NAN ( *(const double *)__Wde_NAN__)
#else
#error "Fog::Math - Haven't NAN"
#endif

#endif

// Not defined INFINITY ?
#if !defined(INFINITY)

#if defined(FOG_CC_GNU)
#define INFINITY __builtin_inf()
#elif defined(FOG_ARCH_X86) || defined(FOG_ARCH_X86_64)
// For MSVC! Is there any other compiler that will not declare INFINITY ?
static FOG_HIDDEN const uint32_t __Core_INF__[2] = { 0x00000000,0x7FF00000 };
#define INFINITY ( *(const double *)__Core_INF__)
#else
#error "Fog::Math - Haven't INFINITY"
#endif

#endif

// [control87]

#if defined(FOG_OS_WINDOWS) && defined(FOG_CC_MSVC)
#define FOG_CONTROL87_BEGIN() uint _control87_old = _control87(0x9001F, FOG_ARCH_32_64(_MCW_DN|_MCW_EM|_MCW_RC, 0xFFFF))
#define FOG_CONTROL87_END() _control87(_control87_old, FOG_ARCH_32_64(_MCW_DN|_MCW_EM|_MCW_RC, 0xFFFF))
#elif defined(FOG_OS_WINDOWS)
#define FOG_CONTROL87_BEGIN() _control87(0, 0)
#define FOG_CONTROL87_END() _clear87()
#elif defined(FOG_HAVE_FENV_H)
#define FOG_CONTROL87_BEGIN() fenv_t _control87_env; feholdexcept(&_control87_env)
#define FOG_CONTROL87_END() fesetenv(&_control87_env)
#else
#define FOG_CONTROL87_BEGIN do {} while(0)
#define FOG_CONTROL87_END do {} while(0)
#endif

// [floating point compare functions]

template<typename T>
static bool FOG_INLINE fog_feq(T a, T b, T epsilon = 0.000001)
{
  return (fabs(a - b) <= epsilon);
}

// [Guard]
#endif // _FOG_CORE_MATH_H
