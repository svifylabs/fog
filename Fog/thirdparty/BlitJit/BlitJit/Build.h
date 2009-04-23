// BlitJit - Just In Time Image Blitting Library for C++ Language.

// Copyright (c) 2008-2009, Petr Kobalicek <kobalicek.petr@gmail.com>
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

// [Guard]
#ifndef _BLITJIT_BUILD_H
#define _BLITJIT_BUILD_H

// [Include]
#include "Config.h"

// Here should be optional include files that's needed fo successfuly
// use macros defined here. Remember, BlitJit uses only BlitJit namespace
// and all macros are used within it. So for example crash handler is
// not called as BlitJit::crash(0) in BLITJIT_ILLEGAL() macro, but simply
// as crash(0).
#include <stdlib.h>

// depends to AsmJit
#include <AsmJit/Build.h>

// [BlitJit - OS]
#if !defined(BLITJIT_WINDOWS) && !defined(BLITJIT_POSIX)
# if defined(ASMJIT_WINDOWS)
#  define BLITJIT_WINDOWS
# endif // ASMJIT_WINDOWS
# if defined(ASMJIT_POSIX)
#  define BLITJIT_POSIX
# endif // ASMJIT_POSIX
#endif

// [BlitJit - Architecture]
// define it only if it's not defined. In some systems we can
// use -D command in compiler to bypass this autodetection.
#if !defined(BLITJIT_X86) && !defined(BLITJIT_X64)
# if defined(ASMJIT_X64)
#  define BLITJIT_X64
# else
#  define BLITJIT_X86
# endif
#endif

// [BlitJit - API]
#if !defined(BLITJIT_HIDDEN)
# define BLITJIT_HIDDEN
#endif

#if !defined(BLITJIT_API)
# define BLITJIT_API
#endif

// [BlitJit - Memory Management]
#if !defined(BLITJIT_MALLOC)
# define BLITJIT_MALLOC ::malloc
#endif // BLITJIT_MALLOC

#if !defined(BLITJIT_REALLOC)
# define BLITJIT_REALLOC ::realloc
#endif // BLITJIT_REALLOC

#if !defined(BLITJIT_FREE)
# define BLITJIT_FREE ::free
#endif // BLITJIT_FREE

// [BlitJit - Crash handler]
namespace BlitJit
{
  static void crash(int* ptr = 0) { *ptr = 0; }
}

#if !defined(BLITJIT_USE)
# define BLITJIT_USE(var) ((void)var)
#endif // BLITJIT_USE

#if !defined(BLITJIT_NOP)
# define BLITJIT_NOP() ((void)0)
#endif // BLITJIT_NOP

// [BlitJit - Types]
namespace BlitJit
{
  // Standard types
  using AsmJit::Int8;
  using AsmJit::UInt8;
  using AsmJit::Int16;
  using AsmJit::UInt16;
  using AsmJit::Int32;
  using AsmJit::UInt32;
  using AsmJit::Int64;
  using AsmJit::UInt64;

  using AsmJit::SysInt;
  using AsmJit::SysUInt;
}

#if defined(_MSC_VER)
# define BLITJIT_INT64_C(num) num##i64
# define BLITJIT_UINT64_C(num) num##ui64
#else
# define BLITJIT_INT64_C(num) num##LL
# define BLITJIT_UINT64_C(num) num##ULL
#endif

// [BlitJit - C++ Macros]
#define BLITJIT_ARRAY_SIZE(A) (sizeof(A) / sizeof(*A))

#define BLITJIT_DISABLE_COPY(__type__) \
private: \
  inline __type__(const __type__& other); \
  inline __type__& operator=(const __type__& other)

// [BlitJit - Debug]
#if defined(DEBUG) || 1
# if !defined(BLITJIT_CRASH)
#  define BLITJIT_CRASH() crash()
# endif
# if !defined(BLITJIT_ASSERT)
#  define BLITJIT_ASSERT(exp) do { if (!(exp)) BLITJIT_CRASH(); } while(0)
# endif
#else
# if !defined(BLITJIT_CRASH)
#  define BLITJIT_CRASH() do {} while(0)
# endif
# if !defined(BLITJIT_ASSERT)
#  define BLITJIT_ASSERT(exp) do {} while(0)
# endif
#endif // DEBUG

// [Guard]
#endif // _BLITJIT_BUILD_H
