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
#ifndef _BLITJIT_MODULE_FILL_H
#define _BLITJIT_MODULE_FILL_H

// [Dependencies]
#include <AsmJit/Compiler.h>

#include "Module_p.h"

namespace BlitJit {

//! @addtogroup BlitJit_Private
//! @{

// ============================================================================
// [BlitJit::Module_Fill_32_SSE2]
// ============================================================================

struct BLITJIT_HIDDEN Module_Fill_32_SSE2 : public Module_Fill
{
  Module_Fill_32_SSE2(
    Generator* g,
    const PixelFormat* dstPf,
    const PixelFormat* srcPf,
    const PixelFormat* mskPf,
    const Operator* op);
  virtual ~Module_Fill_32_SSE2();

  virtual void beginSwitch();

  virtual void init(AsmJit::PtrRef& _src);
  virtual void free();

  virtual void processPixelsPtr(
    const AsmJit::PtrRef* dst,
    const AsmJit::PtrRef* src,
    const AsmJit::PtrRef* msk,
    SysInt count,
    SysInt offset,
    UInt32 kind,
    UInt32 flags);

  void processPixelsRaw(
    AsmJit::XMMRef& dst0,
    bool two);

  void processPixelsRaw_4(
    AsmJit::XMMRef& dst0,
    AsmJit::XMMRef& dst1,
    UInt32 flags);

  void processPixelsRawMask(
    AsmJit::XMMRef& dst0,
    AsmJit::XMMRef& msk0,
    bool two);

  void processPixelsRawMask_4(
    AsmJit::XMMRef& dst0,
    AsmJit::XMMRef& dst1,
    AsmJit::XMMRef& msk0,
    UInt32 flags);

  void processPixelsUnpacked(
    AsmJit::XMMRef& dst0,
    bool two);

  void processPixelsUnpacked_4(
    AsmJit::XMMRef& dst0,
    AsmJit::XMMRef& dst1);

  UInt32 dstAlphaPos;

  AsmJit::SysIntRef srcgp;
  AsmJit::XMMRef srcxmm;
  AsmJit::XMMRef alphaxmm;
};

//! @}

} // BlitJit namespace

// [Guard]
#endif // _BLITJIT_MODULE_FILL_H
