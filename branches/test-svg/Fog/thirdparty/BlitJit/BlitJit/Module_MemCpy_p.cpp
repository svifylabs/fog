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
#include "Module_MemCpy_p.h"

using namespace AsmJit;

namespace BlitJit {

// ============================================================================
// [BlitJit::Module_MemCpy32]
// ============================================================================

Module_MemCpy32::Module_MemCpy32(
  Generator* g,
  const PixelFormat* dstPf,
  const PixelFormat* srcPf,
  const Operator* op) :
  Module_Blit(g, dstPf, srcPf, NULL, op)
{
  _prefetchDst = false;
  _prefetchSrc = true;

  switch (g->optimization())
  {
    case OptimizeX86:
      _maxPixelsPerLoop = 4;
      break;
    case OptimizeMMX:
      _maxPixelsPerLoop = 16;
      break;
    case OptimizeSSE2:
      _maxPixelsPerLoop = 32;
      break;
  }
}

Module_MemCpy32::~Module_MemCpy32()
{
}

void Module_MemCpy32::processPixelsPtr(
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
  bool srcAligned = (flags & SrcAligned) != 0;
  bool dstAligned = (flags & DstAligned) != 0;

  switch (g->optimization())
  {
    case OptimizeX86:
    {
      SysInt i = count;

      do {
        SysInt dstDisp = dstPf->bytesPerPixel() * offset;
        SysInt srcDisp = srcPf->bytesPerPixel() * offset;

#if defined(ASMJIT_X64)
        if (i >= 2)
        {
          Int64Ref t(c->newVariable(VARIABLE_TYPE_INT64));

          g->_LoadMov(t.x(), qword_ptr(src->c(), dstDisp));
          g->_StoreMov(qword_ptr(dst->c(), srcDisp), t.c(), nt);

          offset += 2;
          i -= 2;
        }
        else
        {
#endif // ASMJIT_X64
          Int32Ref t(c->newVariable(VARIABLE_TYPE_INT32));

          g->_LoadMov(t.x(), dword_ptr(src->c(), dstDisp));
          g->_StoreMov(dword_ptr(dst->c(), srcDisp), t.c(), nt);

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
        SysInt srcDisp = srcPf->bytesPerPixel() * offset;

        if (i >= 16 && c->numFreeMm() >= 8)
        {
          MMRef t0(c->newVariable(VARIABLE_TYPE_MM));
          MMRef t1(c->newVariable(VARIABLE_TYPE_MM));
          MMRef t2(c->newVariable(VARIABLE_TYPE_MM));
          MMRef t3(c->newVariable(VARIABLE_TYPE_MM));
          MMRef t4(c->newVariable(VARIABLE_TYPE_MM));
          MMRef t5(c->newVariable(VARIABLE_TYPE_MM));
          MMRef t6(c->newVariable(VARIABLE_TYPE_MM));
          MMRef t7(c->newVariable(VARIABLE_TYPE_MM));

          g->_LoadMovQ(t0.x(), qword_ptr(src->c(), dstDisp));
          g->_LoadMovQ(t1.x(), qword_ptr(src->c(), dstDisp + 8));
          g->_LoadMovQ(t2.x(), qword_ptr(src->c(), dstDisp + 16));
          g->_LoadMovQ(t3.x(), qword_ptr(src->c(), dstDisp + 24));
          g->_LoadMovQ(t4.x(), qword_ptr(src->c(), dstDisp + 32));
          g->_LoadMovQ(t5.x(), qword_ptr(src->c(), dstDisp + 40));
          g->_LoadMovQ(t6.x(), qword_ptr(src->c(), dstDisp + 48));
          g->_LoadMovQ(t7.x(), qword_ptr(src->c(), dstDisp + 56));

          g->_StoreMovQ(qword_ptr(dst->c(), srcDisp), t0.c(), nt);
          g->_StoreMovQ(qword_ptr(dst->c(), srcDisp + 8), t1.c(), nt);
          g->_StoreMovQ(qword_ptr(dst->c(), srcDisp + 16), t2.c(), nt);
          g->_StoreMovQ(qword_ptr(dst->c(), srcDisp + 24), t3.c(), nt);
          g->_StoreMovQ(qword_ptr(dst->c(), srcDisp + 32), t4.c(), nt);
          g->_StoreMovQ(qword_ptr(dst->c(), srcDisp + 40), t5.c(), nt);
          g->_StoreMovQ(qword_ptr(dst->c(), srcDisp + 48), t6.c(), nt);
          g->_StoreMovQ(qword_ptr(dst->c(), srcDisp + 56), t7.c(), nt);

          offset += 16;
          i -= 16;
        }
        else if (i >= 8 && c->numFreeMm() >= 4)
        {
          MMRef t0(c->newVariable(VARIABLE_TYPE_MM));
          MMRef t1(c->newVariable(VARIABLE_TYPE_MM));
          MMRef t2(c->newVariable(VARIABLE_TYPE_MM));
          MMRef t3(c->newVariable(VARIABLE_TYPE_MM));

          g->_LoadMovQ(t0.x(), qword_ptr(src->c(), srcDisp));
          g->_LoadMovQ(t1.x(), qword_ptr(src->c(), srcDisp + 8));
          g->_LoadMovQ(t2.x(), qword_ptr(src->c(), srcDisp + 16));
          g->_LoadMovQ(t3.x(), qword_ptr(src->c(), srcDisp + 24));

          g->_StoreMovQ(qword_ptr(dst->c(), dstDisp), t0.c(), nt);
          g->_StoreMovQ(qword_ptr(dst->c(), dstDisp + 8), t1.c(), nt);
          g->_StoreMovQ(qword_ptr(dst->c(), dstDisp + 16), t2.c(), nt);
          g->_StoreMovQ(qword_ptr(dst->c(), dstDisp + 24), t3.c(), nt);

          offset += 8;
          i -= 8;
        }
        else if (i >= 4 && c->numFreeMm() >= 2)
        {
          MMRef t0(c->newVariable(VARIABLE_TYPE_MM));
          MMRef t1(c->newVariable(VARIABLE_TYPE_MM));

          g->_LoadMovQ(t0.x(), qword_ptr(src->c(), srcDisp));
          g->_LoadMovQ(t1.x(), qword_ptr(src->c(), srcDisp + 8));

          g->_StoreMovQ(qword_ptr(dst->c(), dstDisp), t0.c(), nt);
          g->_StoreMovQ(qword_ptr(dst->c(), dstDisp + 8), t1.c(), nt);

          offset += 4;
          i -= 4;
        }
        else if (i >= 2)
        {
          MMRef t(c->newVariable(VARIABLE_TYPE_MM));

          g->_LoadMovQ(t.x(), qword_ptr(src->c(), srcDisp));
          g->_StoreMovQ(qword_ptr(dst->c(), dstDisp), t.c(), nt);

          offset += 2;
          i -= 2;
        }
        else
        {
          Int32Ref t(c->newVariable(VARIABLE_TYPE_INT32));

          g->_LoadMov(t.x(), dword_ptr(src->c(), srcDisp));
          g->_StoreMov(dword_ptr(dst->c(), dstDisp), t.c(), nt);

          offset++;
          i--;
        }
      } while (i > 0);
      break;
    }

    case OptimizeSSE2:
    {
      SysInt i = count;

      do {
        SysInt dstDisp = dstPf->bytesPerPixel() * offset;
        SysInt srcDisp = srcPf->bytesPerPixel() * offset;

        if (i >= 32 && c->numFreeXmm() >= 8)
        {
          XMMRef t0(c->newVariable(VARIABLE_TYPE_XMM));
          XMMRef t1(c->newVariable(VARIABLE_TYPE_XMM));
          XMMRef t2(c->newVariable(VARIABLE_TYPE_XMM));
          XMMRef t3(c->newVariable(VARIABLE_TYPE_XMM));
          XMMRef t4(c->newVariable(VARIABLE_TYPE_XMM));
          XMMRef t5(c->newVariable(VARIABLE_TYPE_XMM));
          XMMRef t6(c->newVariable(VARIABLE_TYPE_XMM));
          XMMRef t7(c->newVariable(VARIABLE_TYPE_XMM));

          g->_LoadMovDQ(t0.x(), dqword_ptr(src->c(), srcDisp +  0), srcAligned);
          g->_LoadMovDQ(t1.x(), dqword_ptr(src->c(), srcDisp + 16), srcAligned);
          g->_LoadMovDQ(t2.x(), dqword_ptr(src->c(), srcDisp + 32), srcAligned);
          g->_LoadMovDQ(t3.x(), dqword_ptr(src->c(), srcDisp + 48), srcAligned);
          g->_LoadMovDQ(t4.x(), dqword_ptr(src->c(), srcDisp + 64), srcAligned);
          g->_LoadMovDQ(t5.x(), dqword_ptr(src->c(), srcDisp + 80), srcAligned);
          g->_LoadMovDQ(t6.x(), dqword_ptr(src->c(), srcDisp + 96), srcAligned);
          g->_LoadMovDQ(t7.x(), dqword_ptr(src->c(), srcDisp + 112), srcAligned);

          g->_StoreMovDQ(dqword_ptr(dst->c(), dstDisp +  0), t0.c(), nt, dstAligned);
          g->_StoreMovDQ(dqword_ptr(dst->c(), dstDisp + 16), t1.c(), nt, dstAligned);
          g->_StoreMovDQ(dqword_ptr(dst->c(), dstDisp + 32), t2.c(), nt, dstAligned);
          g->_StoreMovDQ(dqword_ptr(dst->c(), dstDisp + 48), t3.c(), nt, dstAligned);
          g->_StoreMovDQ(dqword_ptr(dst->c(), dstDisp + 64), t4.c(), nt, dstAligned);
          g->_StoreMovDQ(dqword_ptr(dst->c(), dstDisp + 80), t5.c(), nt, dstAligned);
          g->_StoreMovDQ(dqword_ptr(dst->c(), dstDisp + 96), t6.c(), nt, dstAligned);
          g->_StoreMovDQ(dqword_ptr(dst->c(), dstDisp + 112), t7.c(), nt, dstAligned);

          offset += 32;
          i -= 32;
        }
        else if (i >= 16 && c->numFreeXmm() >= 4)
        {
          XMMRef t0(c->newVariable(VARIABLE_TYPE_XMM));
          XMMRef t1(c->newVariable(VARIABLE_TYPE_XMM));
          XMMRef t2(c->newVariable(VARIABLE_TYPE_XMM));
          XMMRef t3(c->newVariable(VARIABLE_TYPE_XMM));

          g->_LoadMovDQ(t0.x(), dqword_ptr(src->c(), srcDisp +  0), srcAligned);
          g->_LoadMovDQ(t1.x(), dqword_ptr(src->c(), srcDisp + 16), srcAligned);
          g->_LoadMovDQ(t2.x(), dqword_ptr(src->c(), srcDisp + 32), srcAligned);
          g->_LoadMovDQ(t3.x(), dqword_ptr(src->c(), srcDisp + 48), srcAligned);

          g->_StoreMovDQ(dqword_ptr(dst->c(), dstDisp +  0), t0.c(), nt, dstAligned);
          g->_StoreMovDQ(dqword_ptr(dst->c(), dstDisp + 16), t1.c(), nt, dstAligned);
          g->_StoreMovDQ(dqword_ptr(dst->c(), dstDisp + 32), t2.c(), nt, dstAligned);
          g->_StoreMovDQ(dqword_ptr(dst->c(), dstDisp + 48), t3.c(), nt, dstAligned);

          offset += 16;
          i -= 16;
        }
        else if (i >= 8 && c->numFreeXmm() >= 2)
        {
          XMMRef t0(c->newVariable(VARIABLE_TYPE_XMM));
          XMMRef t1(c->newVariable(VARIABLE_TYPE_XMM));

          g->_LoadMovDQ(t0.x(), dqword_ptr(src->c(), srcDisp +  0), srcAligned);
          g->_LoadMovDQ(t1.x(), dqword_ptr(src->c(), srcDisp + 16), srcAligned);

          g->_StoreMovDQ(dqword_ptr(dst->c(), dstDisp +  0), t0.c(), nt, dstAligned);
          g->_StoreMovDQ(dqword_ptr(dst->c(), dstDisp + 16), t1.c(), nt, dstAligned);

          offset += 8;
          i -= 8;
        }
        else if (i >= 4)
        {
          XMMRef t0(c->newVariable(VARIABLE_TYPE_XMM));

          g->_LoadMovDQ(t0.x(), dqword_ptr(src->c(), srcDisp), srcAligned);
          g->_StoreMovDQ(dqword_ptr(dst->c(), dstDisp), t0.c(), nt, dstAligned);

          offset += 4;
          i -= 4;
        }
        else
        {
          Int32Ref t(c->newVariable(VARIABLE_TYPE_INT32));

          g->_LoadMov(t.x(), dword_ptr(src->c(), srcDisp));
          g->_StoreMov(dword_ptr(dst->c(), dstDisp), t.c(), nt);

          offset++;
          i--;
        }
      } while (i > 0);
      break;
    }
  }
}

} // BlitJit namespace
