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

// This file is designed to be changeable. Platform specific changes
// should be applied to this file and this guarantes and never versions
// of BlitJit library will never overwrite generated config files.
//
// So modify this will by your build system or hand.

// [Guard]
#ifndef _BLITJIT_CONFIG_H
#define _BLITJIT_CONFIG_H

// [BlitJit - OS]
// #define BLITJIT_WINDOWS
// #define BLITJIT_POSIX

// [BlitJit - Architecture]
// #define BLITJIT_X86
// #define BLITJIT_X64

// [BlitJit - API]
// #define BLITJIT_API

// [BlitJit - Memory Management]
// #define BLITJIT_MALLOC ::malloc
// #define BLITJIT_REALLOC ::realloc
// #define BLITJIT_FREE ::free

// [BlitJit - Debug]
// #define BLITJIT_CRASH() crash()
// #define BLITJIT_ASSERT(exp) do { if (!(exp)) BLITJIT_CRASH(); } while(0)

// [Guard]
#endif // _BLITJIT_CONFIG_H
