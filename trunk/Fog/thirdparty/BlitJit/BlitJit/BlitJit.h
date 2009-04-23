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
#ifndef _BLITJIT_H
#define _BLITJIT_H

// [Documentation]

//! @mainpage
//!

//! @defgroup BlitJit_Api API declared to be public.
//!
//! Contains small subset of API declared to be public and easy to use.

//! @defgroup BlitJit_Config Configuration.
//!
//! Contains macros that can be redefined to fit to any project.

//! @defgroup BlitJit_Generator Generator.
//!
//! Generator is heart of BlitJit library.

//! @addtogroup BlitJit_Config
//! @{

//! @def BLITJIT_API
//! @brief Attribute that's added to classes that can be exported if BlitJit
//! is compiled as a dll library.

//! @def BLITJIT_MALLOC
//! @brief Function to call to allocate dynamic memory.

//! @def BLITJIT_REALLOC
//! @brief Function to call to reallocate dynamic memory.

//! @def BLITJIT_FREE
//! @brief Function to call to free dynamic memory.

//! @def BLITJIT_ILLEGAL
//! @brief Code that is execute if an one or more operands are invalid.

//! @def BLITJIT_ASSERT
//! @brief Assertion macro. Default implementation calls @c BLITJIT_ILLEGAL
//! if assert fails.

//! @}

//! @namespace BlitJit
//! @brief Main BlitJit library namespace.

// [Includes]

#include "Build.h"
#include "Api.h"
#include "CodeManager.h"
#include "Generator.h"

// [Guard]
#endif // _BLITJIT_H
