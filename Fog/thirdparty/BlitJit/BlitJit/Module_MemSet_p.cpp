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

// [Dependencies]
#include <AsmJit/Compiler.h>
#include <AsmJit/CpuInfo.h>

#include "Generator_p.h"
#include "Module_MemSet_p.h"

using namespace AsmJit;

namespace BlitJit {

// ============================================================================
// [BlitJit::Module_MemSet32]
// ============================================================================

Module_MemSet32::Module_MemSet32(
  Generator* g,
  const PixelFormat* pf,
  const Operator* op) :
    Module_Fill(g, pf, NULL, NULL,op)
{
  _prefetchDst = false;
  _prefetchSrc = false;

  srcgp.use(c->newVariable(VARIABLE_TYPE_SYSINT));

  switch (g->optimization())
  {
    case OptimizeX86:
      _maxPixelsPerLoop = 4;
      break;
    case OptimizeMMX:
      srcmm.use(c->newVariable(VARIABLE_TYPE_MM));
      _maxPixelsPerLoop = 16;
      break;
    case OptimizeSSE2:
      srcxmm.use(c->newVariable(VARIABLE_TYPE_XMM));
      _maxPixelsPerLoop = 32;
      break;
  }
}

Module_MemSet32::~Module_MemSet32()
{
}

void Module_MemSet32::init(PtrRef& _src)
{
  c->mov(srcgp.x32(), ptr(_src.c()));

  switch (g->optimization())
  {
#if defined(ASMJIT_X64)
    case OptimizeX86:
    {
      SysIntRef tmp(c->newVariable(VARIABLE_TYPE_SYSINT));
      c->mov(tmp.x64(), srcgp.r64());
      c->shl(tmp.r64(), 32);
      c->or_(srcgp.r64(), tmp.r64());
      break;
    }
#endif
    case OptimizeMMX:
      c->movd(srcmm.x(), srcgp.c32());
      c->pshufw(srcmm.r(), srcmm.r(), mm_shuffle(0, 1, 0, 1));
      break;
    case OptimizeSSE2:
      c->movd(srcxmm.x(), srcgp.c32());
      c->pshufd(srcxmm.r(), srcxmm.r(), mm_shuffle(0, 0, 0, 0));
      break;
  }
}

void Module_MemSet32::free()
{
}

void Module_MemSet32::processPixelsPtr(
  const AsmJit::PtrRef* dst,
  const AsmJit::PtrRef* src,
  const AsmJit::PtrRef* msk,
  SysInt count,
  SysInt offset,
  UInt32 kind,
  UInt32 flags)
{
  StateRef state(c->saveState());

  bool nt = g->nonThermalHint();

  switch (g->optimization())
  {
    case OptimizeX86:
    {
      SysInt i = count;

      do {
        SysInt dstDisp = dstPf->bytesPerPixel() * offset;

#if defined(ASMJIT_X64)
        if (i >= 2)
        {
          g->_StoreMov(qword_ptr(dst->c(), dstDisp), srcgp.c64(), nt);

          offset += 2;
          i -= 2;
        }
        else
        {
#endif // ASMJIT_X64
          g->_StoreMov(dword_ptr(dst->c(), dstDisp), srcgp.c32(), nt);

          offset++;
          i--;
#if defined(ASMJIT_X64)
        }
#endif // ASMJIT_X64
      } while (i > 0);
      break;
    }

    case OptimizeMMX:
    {
      SysInt i = count;

      do {
        SysInt dstDisp = dstPf->bytesPerPixel() * offset;

        if (i >= 2)
        {
          g->_StoreMovQ(qword_ptr(dst->c(), dstDisp), srcmm.c(), nt);

          offset += 2;
          i -= 2;
        }
        else
        {
          g->_StoreMov(dword_ptr(dst->c(), dstDisp), srcgp.c32(), nt);

          offset++;
          i--;
        }
      } while (i > 0);
      break;
    }

    case OptimizeSSE2:
    {
      SysInt i = count;
      bool aligned = (flags & DstAligned) != 0;

      do {
        SysInt dstDisp = dstPf->bytesPerPixel() * offset;

        if (i >= 4)
        {
          g->_StoreMovDQ(dqword_ptr(dst->c(), dstDisp), srcxmm.c(), nt, aligned);

          offset += 4;
          i -= 4;
        }
        else
        {
          g->_StoreMov(dword_ptr(dst->c(), dstDisp), srcgp.c32(), nt);

          offset++;
          i--;
        }
      } while (i > 0);
      break;
    }
  }
}

} // BlitJit namespace
