// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_ASSERT_H
#define _FOG_CORE_ASSERT_H

// [Dependencies]
#include <Fog/Build/Build.h>

#if !defined(_FOG_STD_H)
FOG_CAPI_EXTERN void FOG_NO_RETURN fog_assertion(const char* file, int line, const char* msg);
#endif

// ============================================================================
// [FOG_ASSERT]
// ============================================================================

#if defined(FOG_DEBUG)
#define FOG_ASSERT(expression) \
  if (!(expression)) fog_assertion(__FILE__, __LINE__, #expression)
#define FOG_ASSERT_X(expression, msg) \
  if (!(expression)) fog_assertion(__FILE__, __LINE__, msg)
#define FOG_ASSERT_NOT_REACHED() \
  fog_assertion(__FILE__, __LINE__, NULL)
#else // !FOG_DEBUG
#define FOG_ASSERT(expression) FOG_NOOP
#define FOG_ASSERT_X(expression, msg) FOG_NOOP
#define FOG_ASSERT_NOT_REACHED() FOG_NOOP
#endif // FOG_DEBUG

// [Guard]
#endif // _FOG_CORE_ASSERT_H
