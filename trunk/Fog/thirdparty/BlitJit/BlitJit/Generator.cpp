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

#include "Api.h"
#include "Generator.h"
#include "GeneratorPrivate.h"

#include <new>

// Label groups used in generator:
// - L_Align - Align loop labels
// - L_Main - Main loop labels
// - L_Tail - Tail loop labels
//
// Label subgroups used in generator:
// - ...Entry - Before previous label counter cleanup (see the code)
// - ...Prepare - Before loop conditional (after cleanup)
// - ...Loop - Before main loop instruction stream
// - ...End - End of loop or code block

using namespace AsmJit;

namespace BlitJit {

// ============================================================================
// [BlitJit::OutsideBlock]
// ============================================================================

struct OutsideBlock
{
  OutsideBlock(Compiler* c) : c(c)
  {
    _savedState = c->saveState();
    _savedCurrent = c->current();

    c->setCurrent(c->lastEmittable());
  }

  ~OutsideBlock()
  {
    c->setState(_savedState);
    c->setCurrent(_savedCurrent);
  }

  inline State* savedState() const { return _savedState; }

private:
  Compiler* c;
  State* _savedState;
  Emittable* _savedCurrent;
};

// ============================================================================
// [BlitJit::PixelFormat helpers]
// ============================================================================

static UInt32 getARGB32AlphaPos(const PixelFormat* pf)
{
  return pf->aShift() / 8;
}

// ============================================================================
// [BlitJit::MemSetModule_32]
// ============================================================================

struct BLITJIT_HIDDEN MemSetModule_32 : public FillModule
{
  MemSetModule_32(
    Generator* g,
    const PixelFormat* pf,
    const Operator* op);
  virtual ~MemSetModule_32();

  virtual void init(PtrRef& _src, const PixelFormat* pfSrc);
  virtual void free();

  virtual void processPixelsPtr(
    const AsmJit::PtrRef& dst,
    SysInt count,
    SysInt displacement,
    UInt32 kind,
    UInt32 flags);

  SysIntRef src;
  MMRef srcmm;
  XMMRef srcxmm;
};

MemSetModule_32::MemSetModule_32(
  Generator* g,
  const PixelFormat* pf,
  const Operator* op) :
    FillModule(g, pf, op)
{
  _prefetchDst = false;
  _prefetchSrc = false;

  src.use(c->newVariable(VARIABLE_TYPE_SYSINT));

  switch (g->optimization())
  {
    case Api::OptimizeX86:
      _maxPixelsPerLoop = 4;
      break;
    case Api::OptimizeMMX:
      srcmm.use(c->newVariable(VARIABLE_TYPE_MM));
      _maxPixelsPerLoop = 16;
      break;
    case Api::OptimizeSSE2:
      srcxmm.use(c->newVariable(VARIABLE_TYPE_XMM));
      _maxPixelsPerLoop = 32;
      break;
  }
}

MemSetModule_32::~MemSetModule_32()
{
}

void MemSetModule_32::init(PtrRef& _src, const PixelFormat* pfSrc)
{
  c->mov(src.x32(), dword_ptr(_src.c()));
  // TODO

  switch (g->optimization())
  {
    case Api::OptimizeMMX:
      c->movd(srcmm.x(), src.c32());
      c->pshufw(srcmm.r(), srcmm.r(), mm_shuffle(0, 1, 0, 1));
      break;
    case Api::OptimizeSSE2:
      c->movd(srcxmm.x(), src.c32());
      c->pshufd(srcxmm.r(), srcxmm.r(), mm_shuffle(0, 0, 0, 0));
      break;
  }
}

void MemSetModule_32::free()
{
}

void MemSetModule_32::processPixelsPtr(
  const AsmJit::PtrRef& dst,
  SysInt count,
  SysInt displacement,
  UInt32 kind,
  UInt32 flags)
{
  StateRef state(c->saveState());

  bool nt = g->nonThermalHint();

  switch (g->optimization())
  {
    case Api::OptimizeX86:
    {
      SysInt i = count;

      do {
#if defined(ASMJIT_X64)
        if (i >= 2)
        {
          g->_StoreMov(qword_ptr(dst.c(), displacement), src.c64(), nt);

          displacement += 8;
          i -= 2;
        }
        else
        {
#endif // ASMJIT_X64
          g->_StoreMov(dword_ptr(dst.c(), displacement), src.c32(), nt);

          displacement += 4;
          i--;
#if defined(ASMJIT_X64)
        }
#endif // ASMJIT_X64
      } while (i > 0);
      break;
    }

    case Api::OptimizeMMX:
    {
      SysInt i = count;

      do {
        if (i >= 2)
        {
          g->_StoreMovQ(qword_ptr(dst.c(), displacement), srcmm.c(), nt);

          displacement += 8;
          i -= 2;
        }
        else
        {
          g->_StoreMov(dword_ptr(dst.c(), displacement), src.c32(), nt);

          displacement += 4;
          i--;
        }
      } while (i > 0);
      break;
    }

    case Api::OptimizeSSE2:
    {
      SysInt i = count;
      bool aligned = (flags & DstAligned) != 0;

      do {
        if (i >= 4)
        {
          g->_StoreMovDQ(dqword_ptr(dst.c(), displacement), srcxmm.c(), nt, aligned);

          displacement += 16;
          i -= 4;
        }
        else
        {
          g->_StoreMov(dword_ptr(dst.c(), displacement), src.c32(), nt);

          displacement += 4;
          i--;
        }
      } while (i > 0);
      break;
    }
  }
}

// ============================================================================
// [BlitJit::MemCpyModule_32]
// ============================================================================

struct BLITJIT_HIDDEN MemCpyModule_32 : public CompositeModule
{
  MemCpyModule_32(
    Generator* g,
    const PixelFormat* pfDst,
    const PixelFormat* pfSrc,
    const Operator* op);
  virtual ~MemCpyModule_32();

  virtual void processPixelsPtr(
    const AsmJit::PtrRef& dst,
    const AsmJit::PtrRef& src,
    SysInt count,
    SysInt displacement,
    UInt32 kind,
    UInt32 flags);
};

MemCpyModule_32::MemCpyModule_32(
  Generator* g,
  const PixelFormat* pfDst,
  const PixelFormat* pfSrc,
  const Operator* op) :
  CompositeModule(g, pfDst, pfSrc, op)
{
  _prefetchDst = false;
  _prefetchSrc = true;

  switch (g->optimization())
  {
    case Api::OptimizeX86:
      _maxPixelsPerLoop = 4;
      break;
    case Api::OptimizeMMX:
      _maxPixelsPerLoop = 16;
      break;
    case Api::OptimizeSSE2:
      _maxPixelsPerLoop = 32;
      break;
  }
}

MemCpyModule_32::~MemCpyModule_32()
{
}

void MemCpyModule_32::processPixelsPtr(
  const AsmJit::PtrRef& dst,
  const AsmJit::PtrRef& src,
  SysInt count,
  SysInt displacement,
  UInt32 kind,
  UInt32 flags)
{
  StateRef state(c->saveState());

  bool nt = g->nonThermalHint();
  bool srcAligned = (flags & SrcAligned) != 0;
  bool dstAligned = (flags & DstAligned) != 0;

  switch (g->optimization())
  {
    case Api::OptimizeX86:
    {
      SysInt i = count;

      do {
#if defined(ASMJIT_X64)
        if (i >= 2)
        {
          Int64Ref t(c->newVariable(VARIABLE_TYPE_INT64));

          g->_LoadMov(t.x(), qword_ptr(src.c(), displacement));
          g->_StoreMov(qword_ptr(dst.c(), displacement), t.c(), nt);

          displacement += 8;
          i -= 2;
        }
        else
        {
#endif // ASMJIT_X64
          Int32Ref t(c->newVariable(VARIABLE_TYPE_INT32));

          g->_LoadMov(t.x(), dword_ptr(src.c(), displacement));
          g->_StoreMov(dword_ptr(dst.c(), displacement), t.c(), nt);

          displacement += 4;
          i--;
#if defined(ASMJIT_X64)
        }
#endif // ASMJIT_X64
      } while (i > 0);
      break;
    }

    case Api::OptimizeMMX:
    {
      SysInt i = count;

      do {
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

          g->_LoadMovQ(t0.x(), qword_ptr(src.c(), displacement));
          g->_LoadMovQ(t1.x(), qword_ptr(src.c(), displacement + 8));
          g->_LoadMovQ(t2.x(), qword_ptr(src.c(), displacement + 16));
          g->_LoadMovQ(t3.x(), qword_ptr(src.c(), displacement + 24));
          g->_LoadMovQ(t4.x(), qword_ptr(src.c(), displacement + 32));
          g->_LoadMovQ(t5.x(), qword_ptr(src.c(), displacement + 40));
          g->_LoadMovQ(t6.x(), qword_ptr(src.c(), displacement + 48));
          g->_LoadMovQ(t7.x(), qword_ptr(src.c(), displacement + 56));

          g->_StoreMovQ(qword_ptr(dst.c(), displacement), t0.c(), nt);
          g->_StoreMovQ(qword_ptr(dst.c(), displacement + 8), t1.c(), nt);
          g->_StoreMovQ(qword_ptr(dst.c(), displacement + 16), t2.c(), nt);
          g->_StoreMovQ(qword_ptr(dst.c(), displacement + 24), t3.c(), nt);
          g->_StoreMovQ(qword_ptr(dst.c(), displacement + 32), t4.c(), nt);
          g->_StoreMovQ(qword_ptr(dst.c(), displacement + 40), t5.c(), nt);
          g->_StoreMovQ(qword_ptr(dst.c(), displacement + 48), t6.c(), nt);
          g->_StoreMovQ(qword_ptr(dst.c(), displacement + 56), t7.c(), nt);

          displacement += 64;
          i -= 16;
        }
        else if (i >= 8 && c->numFreeMm() >= 4)
        {
          MMRef t0(c->newVariable(VARIABLE_TYPE_MM));
          MMRef t1(c->newVariable(VARIABLE_TYPE_MM));
          MMRef t2(c->newVariable(VARIABLE_TYPE_MM));
          MMRef t3(c->newVariable(VARIABLE_TYPE_MM));

          g->_LoadMovQ(t0.x(), qword_ptr(src.c(), displacement));
          g->_LoadMovQ(t1.x(), qword_ptr(src.c(), displacement + 8));
          g->_LoadMovQ(t2.x(), qword_ptr(src.c(), displacement + 16));
          g->_LoadMovQ(t3.x(), qword_ptr(src.c(), displacement + 24));

          g->_StoreMovQ(qword_ptr(dst.c(), displacement), t0.c(), nt);
          g->_StoreMovQ(qword_ptr(dst.c(), displacement + 8), t1.c(), nt);
          g->_StoreMovQ(qword_ptr(dst.c(), displacement + 16), t2.c(), nt);
          g->_StoreMovQ(qword_ptr(dst.c(), displacement + 24), t3.c(), nt);

          displacement += 32;
          i -= 8;
        }
        else if (i >= 4 && c->numFreeMm() >= 2)
        {
          MMRef t0(c->newVariable(VARIABLE_TYPE_MM));
          MMRef t1(c->newVariable(VARIABLE_TYPE_MM));

          g->_LoadMovQ(t0.x(), qword_ptr(src.c(), displacement));
          g->_LoadMovQ(t1.x(), qword_ptr(src.c(), displacement + 8));

          g->_StoreMovQ(qword_ptr(dst.c(), displacement), t0.c(), nt);
          g->_StoreMovQ(qword_ptr(dst.c(), displacement + 8), t1.c(), nt);

          displacement += 16;
          i -= 4;
        }
        else if (i >= 2)
        {
          MMRef t(c->newVariable(VARIABLE_TYPE_MM));

          g->_LoadMovQ(t.x(), qword_ptr(src.c(), displacement));
          g->_StoreMovQ(qword_ptr(dst.c(), displacement), t.c(), nt);

          displacement += 8;
          i -= 2;
        }
        else
        {
          Int32Ref t(c->newVariable(VARIABLE_TYPE_INT32));

          g->_LoadMov(t.x(), dword_ptr(src.c(), displacement));
          g->_StoreMov(dword_ptr(dst.c(), displacement), t.c(), nt);

          displacement += 4;
          i--;
        }
      } while (i > 0);
      break;
    }

    case Api::OptimizeSSE2:
    {
      SysInt i = count;

      do {
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

          g->_LoadMovDQ(t0.x(), dqword_ptr(src.c(), displacement +  0), srcAligned);
          g->_LoadMovDQ(t1.x(), dqword_ptr(src.c(), displacement + 16), srcAligned);
          g->_LoadMovDQ(t2.x(), dqword_ptr(src.c(), displacement + 32), srcAligned);
          g->_LoadMovDQ(t3.x(), dqword_ptr(src.c(), displacement + 48), srcAligned);
          g->_LoadMovDQ(t4.x(), dqword_ptr(src.c(), displacement + 64), srcAligned);
          g->_LoadMovDQ(t5.x(), dqword_ptr(src.c(), displacement + 80), srcAligned);
          g->_LoadMovDQ(t6.x(), dqword_ptr(src.c(), displacement + 96), srcAligned);
          g->_LoadMovDQ(t7.x(), dqword_ptr(src.c(), displacement + 112), srcAligned);

          g->_StoreMovDQ(dqword_ptr(dst.c(), displacement +  0), t0.c(), nt, dstAligned);
          g->_StoreMovDQ(dqword_ptr(dst.c(), displacement + 16), t1.c(), nt, dstAligned);
          g->_StoreMovDQ(dqword_ptr(dst.c(), displacement + 32), t2.c(), nt, dstAligned);
          g->_StoreMovDQ(dqword_ptr(dst.c(), displacement + 48), t3.c(), nt, dstAligned);
          g->_StoreMovDQ(dqword_ptr(dst.c(), displacement + 64), t4.c(), nt, dstAligned);
          g->_StoreMovDQ(dqword_ptr(dst.c(), displacement + 80), t5.c(), nt, dstAligned);
          g->_StoreMovDQ(dqword_ptr(dst.c(), displacement + 96), t6.c(), nt, dstAligned);
          g->_StoreMovDQ(dqword_ptr(dst.c(), displacement + 112), t7.c(), nt, dstAligned);

          displacement += 128;
          i -= 32;
        }
        else if (i >= 16 && c->numFreeXmm() >= 4)
        {
          XMMRef t0(c->newVariable(VARIABLE_TYPE_XMM));
          XMMRef t1(c->newVariable(VARIABLE_TYPE_XMM));
          XMMRef t2(c->newVariable(VARIABLE_TYPE_XMM));
          XMMRef t3(c->newVariable(VARIABLE_TYPE_XMM));

          g->_LoadMovDQ(t0.x(), dqword_ptr(src.c(), displacement +  0), srcAligned);
          g->_LoadMovDQ(t1.x(), dqword_ptr(src.c(), displacement + 16), srcAligned);
          g->_LoadMovDQ(t2.x(), dqword_ptr(src.c(), displacement + 32), srcAligned);
          g->_LoadMovDQ(t3.x(), dqword_ptr(src.c(), displacement + 48), srcAligned);

          g->_StoreMovDQ(dqword_ptr(dst.c(), displacement +  0), t0.c(), nt, dstAligned);
          g->_StoreMovDQ(dqword_ptr(dst.c(), displacement + 16), t1.c(), nt, dstAligned);
          g->_StoreMovDQ(dqword_ptr(dst.c(), displacement + 32), t2.c(), nt, dstAligned);
          g->_StoreMovDQ(dqword_ptr(dst.c(), displacement + 48), t3.c(), nt, dstAligned);

          displacement += 64;
          i -= 16;
        }
        else if (i >= 8 && c->numFreeXmm() >= 2)
        {
          XMMRef t0(c->newVariable(VARIABLE_TYPE_XMM));
          XMMRef t1(c->newVariable(VARIABLE_TYPE_XMM));

          g->_LoadMovDQ(t0.x(), dqword_ptr(src.c(), displacement +  0), srcAligned);
          g->_LoadMovDQ(t1.x(), dqword_ptr(src.c(), displacement + 16), srcAligned);

          g->_StoreMovDQ(dqword_ptr(dst.c(), displacement +  0), t0.c(), nt, dstAligned);
          g->_StoreMovDQ(dqword_ptr(dst.c(), displacement + 16), t1.c(), nt, dstAligned);

          displacement += 32;
          i -= 8;
        }
        else if (i >= 4)
        {
          XMMRef t0(c->newVariable(VARIABLE_TYPE_XMM));

          g->_LoadMovDQ(t0.x(), dqword_ptr(src.c(), displacement), srcAligned);
          g->_StoreMovDQ(dqword_ptr(dst.c(), displacement), t0.c(), nt, dstAligned);

          displacement += 16;
          i -= 4;
        }
        else
        {
          Int32Ref t(c->newVariable(VARIABLE_TYPE_INT32));

          g->_LoadMov(t.x(), dword_ptr(src.c(), displacement));
          g->_StoreMov(dword_ptr(dst.c(), displacement), t.c(), nt);

          displacement += 4;
          i--;
        }
      } while (i > 0);
      break;
    }
  }
}

// ============================================================================
// [BlitJit::FillModule_32_SSE2]
// ============================================================================

struct BLITJIT_HIDDEN FillModule_32_SSE2 : public FillModule
{
  FillModule_32_SSE2(
    Generator* g,
    const PixelFormat* pf,
    const Operator* op);
  virtual ~FillModule_32_SSE2();

  virtual void beginSwitch();

  virtual void init(PtrRef& _src, const PixelFormat* pfSrc);
  virtual void free();

  virtual void processPixelsPtr(
    const AsmJit::PtrRef& dst,
    SysInt count,
    SysInt displacement,
    UInt32 kind,
    UInt32 flags);

  void processPixelsRaw(
    XMMRef& dst0,
    bool two);

  void processPixelsRaw_4(
    XMMRef& dst0,
    XMMRef& dst1,
    UInt32 flags);

  void processPixelsUnpacked(
    XMMRef& dst0,
    bool two);

  void processPixelsUnpacked_4(
    XMMRef& dst0,
    XMMRef& dst1);

  UInt32 dstAlphaPos;

  SysIntRef src;
  XMMRef srcxmm;
  XMMRef alphaxmm;
};

FillModule_32_SSE2::FillModule_32_SSE2(
  Generator* g,
  const PixelFormat* pf,
  const Operator* op) :
    FillModule(g, pf, op)
{
  // Calculate correct pixel format positions
  dstAlphaPos = getARGB32AlphaPos(pf);

  // First look at NOPs
  if (op->id() == Operator::CompositeDest)
  {
    _isNop = true;
    return;
  }

  _maxPixelsPerLoop = 4;
  _complexity = Complex;

  setNumKinds(2);
}

FillModule_32_SSE2::~FillModule_32_SSE2()
{
}

void FillModule_32_SSE2::beginSwitch()
{
  // Expand pixel
  c->movd(srcxmm.x(), src.c32());
  c->pshufd(srcxmm.r(), srcxmm.r(), mm_shuffle(0, 0, 0, 0));

  // Check if alpha is 0xFF (255) 
  c->cmp(src.r32(), imm(0xFF000000));
  c->jae(getKindLabel(1));

  // Alpha is not 0xFF, premultiply and prepare for compositing
  c->punpcklbw(srcxmm.r(), g->xmmZero().r());
  c->punpcklqdq(srcxmm.r(), srcxmm.r());
  g->_Premultiply_1(srcxmm);
  g->_ExtractAlpha(alphaxmm.r(), srcxmm.r(), dstAlphaPos, true, true);
}

void FillModule_32_SSE2::init(PtrRef& _src, const PixelFormat* pfSrc)
{
  g->usingConstants();
  g->usingXMMZero();
  g->usingXMM0080();

  src.use(c->newVariable(VARIABLE_TYPE_SYSINT));
  srcxmm.use(c->newVariable(VARIABLE_TYPE_XMM));
  alphaxmm.use(c->newVariable(VARIABLE_TYPE_XMM));

  c->mov(src.x32(), ptr(_src.c()));
}

void FillModule_32_SSE2::free()
{
  src.unuse();
  srcxmm.unuse();
  alphaxmm.unuse();
}

void FillModule_32_SSE2::processPixelsPtr(
  const AsmJit::PtrRef& dst,
  SysInt count,
  SysInt displacement,
  UInt32 kind,
  UInt32 flags)
{
  SysInt i = count;

  // Default
  if (kind == 0)
  {
    XMMRef dst0(c->newVariable(VARIABLE_TYPE_XMM));

    do {
      if (i >= 4)
      {
        XMMRef dst1(c->newVariable(VARIABLE_TYPE_XMM));

        //c->movdqa(dst0.x(), ptr(dst.c(), displacement));
        //c->movq(dst1.x(), dst0.r());
        c->movq(dst0.x(), ptr(dst.c(), displacement));
        c->movq(dst1.x(), ptr(dst.c(), displacement + 8));
        processPixelsRaw_4(dst0, dst1, 0);
        c->movdqa(ptr(dst.c(), displacement), dst0.r());

        displacement += 16;
        i -= 4;
      }
      else if (i >= 2)
      {
        c->movq(dst0.x(), ptr(dst.c(), displacement));
        processPixelsRaw(dst0, true);
        c->movq(ptr(dst.c(), displacement), dst0.r());

        displacement += 8;
        i -= 2;
      }
      else if (i >= 1)
      {
        c->movd(dst0.x(), ptr(dst.c(), displacement));
        processPixelsRaw(dst0, false);
        c->movd(ptr(dst.c(), displacement), dst0.r());

        displacement += 4;
        i -= 1;
      }
    } while (i > 0);
  }
  // Optimized special case - Alpha == 0xFF
  else
  {
    do {
      if (i >= 4)
      {
        c->movdqa(ptr(dst.c(), displacement), srcxmm.r());

        displacement += 16;
        i -= 4;
      }
      else if (i >= 1)
      {
        c->mov(ptr(dst.c(), displacement), src.r());

        displacement += 4;
        i -= 1;
      }
    } while (i > 0);
  }
}

void FillModule_32_SSE2::processPixelsRaw(
  XMMRef& dst0,
  bool two)
{
  c->punpcklbw(dst0.r(), g->xmmZero().r());
  processPixelsUnpacked(dst0, two);
  c->packuswb(dst0.r(), dst0.r());
}

void FillModule_32_SSE2::processPixelsRaw_4(
  XMMRef& dst0,
  XMMRef& dst1,
  UInt32 flags)
{
  c->punpcklbw(dst0.r(), g->xmmZero().r());
  c->punpcklbw(dst1.r(), g->xmmZero().r());
  processPixelsUnpacked_4(dst0, dst1);
  c->packuswb(dst0.r(), dst1.r());
  //c->packuswb(dst0.r(), dst0.r());
  //c->packuswb(dst1.r(), dst1.r());
}

void FillModule_32_SSE2::processPixelsUnpacked(
  XMMRef& dst0,
  bool two)
{
  XMMRef t0(c->newVariable(VARIABLE_TYPE_XMM));

  g->_PackedMultiply(dst0.r(), alphaxmm.c(), t0.r());
  c->paddusb(dst0.r(), srcxmm.r());
}

void FillModule_32_SSE2::processPixelsUnpacked_4(
  XMMRef& dst0,
  XMMRef& dst1)
{
  XMMRef t0(c->newVariable(VARIABLE_TYPE_XMM));
  XMMRef t1(c->newVariable(VARIABLE_TYPE_XMM));

  g->_PackedMultiply_4(
    dst0.r(), alphaxmm.c(), t0.r(),
    dst1.r(), alphaxmm.c(), t1.r(),
    0);
  c->paddusb(dst0.r(), srcxmm.r());
  c->paddusb(dst1.r(), srcxmm.r());
}

// ============================================================================
// [BlitJit::CompositeModule_32_SSE2]
// ============================================================================

struct BLITJIT_HIDDEN CompositeModule_32_SSE2 : public CompositeModule
{
  CompositeModule_32_SSE2(
    Generator* g,
    const PixelFormat* pfDst,
    const PixelFormat* pfSrc,
    const Operator* op);
  virtual ~CompositeModule_32_SSE2();

  virtual void init();
  virtual void free();

  virtual void processPixelsPtr(
    const AsmJit::PtrRef& dst,
    const AsmJit::PtrRef& src,
    SysInt count,
    SysInt displacement,
    UInt32 kind,
    UInt32 flags);

  enum RawFlags
  {
    Raw4UnpackFromDst0 = (1 << 0),
    Raw4UnpackFromDst2 = (1 << 1), // X64
    Raw4UnpackFromSrc0 = (1 << 2),
    Raw4UnpackFromSrc2 = (1 << 3), // X64
    Raw4PackToDst0 = (1 << 4),
    Raw4PackToDst2 = (1 << 5) // X64
  };

  void processPixelsRaw(
    const XMMRegister& dst0, const XMMRegister& src0,
    bool two);

  void processPixelsRaw_4(
    const XMMRegister& dst0, const XMMRegister& src0,
    const XMMRegister& dst1, const XMMRegister& src1,
    UInt32 flags);

  void processPixelsUnpacked(
    const XMMRegister& dst0, const XMMRegister& src0, bool two);

  void processPixelsUnpacked_4(
    const XMMRegister& dst0, const XMMRegister& src0,
    const XMMRegister& dst1, const XMMRegister& src1);

  UInt32 dstAlphaPos;
  UInt32 srcAlphaPos;
};

CompositeModule_32_SSE2::CompositeModule_32_SSE2(
  Generator* g,
  const PixelFormat* pfDst,
  const PixelFormat* pfSrc,
  const Operator* op) :
    CompositeModule(g, pfDst, pfSrc, op)
{
  // Calculate correct pixel format positions
  dstAlphaPos = getARGB32AlphaPos(pfDst);
  srcAlphaPos = getARGB32AlphaPos(pfSrc);

  // First look at NOPs
  if (op->id() == Operator::CompositeDest)
  {
    _isNop = true;
    return;
  }

  _maxPixelsPerLoop = 4;
  _complexity = Complex;

  switch (op->id())
  {
    case Operator::CompositeSubtract  : _maxPixelsPerLoop = 2; break;
    case Operator::CompositeMultiply  : _maxPixelsPerLoop = 2; break;
    case Operator::CompositeScreen    : _maxPixelsPerLoop = 2; break;
    case Operator::CompositeDarken    : _maxPixelsPerLoop = 2; break;
    case Operator::CompositeLighten   : _maxPixelsPerLoop = 2; break;
    case Operator::CompositeDifference: _maxPixelsPerLoop = 2; break;
    case Operator::CompositeExclusion : _maxPixelsPerLoop = 2; break;
    case Operator::CompositeInvert    : _maxPixelsPerLoop = 2; break;
    case Operator::CompositeInvertRgb : _maxPixelsPerLoop = 2; break;
  }
}

CompositeModule_32_SSE2::~CompositeModule_32_SSE2()
{
}

void CompositeModule_32_SSE2::init()
{
  g->usingConstants();
  g->usingXMMZero();
  g->usingXMM0080();
}

void CompositeModule_32_SSE2::free()
{
}

void CompositeModule_32_SSE2::processPixelsPtr(
  const AsmJit::PtrRef& dst,
  const AsmJit::PtrRef& src,
  SysInt count,
  SysInt displacement,
  UInt32 kind,
  UInt32 flags)
{
  StateRef state(c->saveState());

  // These are needed in all cases
  XMMRef dstpix0(c->newVariable(VARIABLE_TYPE_XMM, 5));
  XMMRef srcpix0(c->newVariable(VARIABLE_TYPE_XMM, 5));

  bool srcAligned = (flags & SrcAligned) != 0;
  bool dstAligned = (flags & DstAligned) != 0;

  switch (count)
  {
    case 1: // Process 1 pixel
    {
      c->movd(srcpix0.x(), ptr(src.r(), displacement));
      c->movd(dstpix0.x(), ptr(dst.r(), displacement));
      processPixelsRaw(dstpix0.r(), srcpix0.r(), false);
      c->movd(ptr(dst.r(), displacement), dstpix0.r());
      break;
    }
    case 2: // Process 2 pixels
    {
      c->movq(srcpix0.x(), ptr(src.r(), displacement));
      c->movq(dstpix0.x(), ptr(dst.r(), displacement));
      processPixelsRaw(dstpix0.r(), srcpix0.r(), true);
      c->movq(ptr(dst.r(), displacement), dstpix0.r());
      break;
    }
    case 4: // Process 4 pixels
    {
      // Need more variables to successfully parallelize instructions
      XMMRef dstpix1(c->newVariable(VARIABLE_TYPE_XMM, 5));
      XMMRef srcpix1(c->newVariable(VARIABLE_TYPE_XMM, 5));

      Label* L_LocalLoopExit = c->newLabel();

      g->_LoadMovDQ(srcpix0.x(), ptr(src.r(), displacement), srcAligned);

      // This can improve speed by skipping pixels of zero or full alpha
      if (op->id() == Operator::CompositeOver)
      {
        Label* L_1 = c->newLabel();

        c->pcmpeqb(dstpix0.r(), dstpix0.r());
        c->pxor(dstpix1.r(), dstpix1.r());

        c->pcmpeqb(dstpix0.r(), srcpix0.r());
        c->pcmpeqb(dstpix1.r(), srcpix0.r());

        {
          SysIntRef t(c->newVariable(VARIABLE_TYPE_SYSINT, 0));
          SysIntRef k(c->newVariable(VARIABLE_TYPE_SYSINT, 0));

          c->pmovmskb(k.r32(), dstpix0.r());
          c->pmovmskb(t.r32(), dstpix1.r());

          c->and_(k.r32(), 0x8888);
          c->cmp(t.r32(), 0xFFFF);
          c->jz(L_LocalLoopExit);

          c->cmp(k.r32(), 0x8888);
          c->jnz(L_1);
        }

        g->_StoreMovDQ(ptr(dst.r(), displacement), srcpix0.r(), false, dstAligned);
        c->jmp(L_LocalLoopExit);

        c->bind(L_1);
      }

      g->_LoadMovDQ(dstpix0.x(), ptr(dst.r(), displacement), dstAligned);

      // Source and destination is in srcpix0 and dstpix0, also we want to
      // pack destination to dstpix0.
      processPixelsRaw_4(dstpix0.r(), srcpix0.r(), dstpix1.r(), srcpix1.r(),
        CompositeModule_32_SSE2::Raw4UnpackFromSrc0 |
        CompositeModule_32_SSE2::Raw4UnpackFromDst0 |
        CompositeModule_32_SSE2::Raw4PackToDst0);

      g->_StoreMovDQ(ptr(dst.r(), displacement), dstpix0.r(), false, dstAligned);

      c->bind(L_LocalLoopExit);
      break;
    }
  }
}

void CompositeModule_32_SSE2::processPixelsRaw(
  const XMMRegister& dst0, const XMMRegister& src0,
  bool two)
{
  switch (op->id())
  {
    case Operator::CompositeSrc:
      // copy operation (optimized in frontends and also by Generator itself)
      c->movdqa(dst0, src0);
      return;
    case Operator::CompositeDest:
      // no operation (optimized in frontends and also by Generator itself)
      return;
    case Operator::CompositeClear:
      // clear operation (optimized in frontends and also by Generator itself)
      c->pxor(dst0, dst0);
      return;
    case Operator::CompositeAdd:
      // add operation (not needs to be unpacked and packed)
      c->paddusb(dst0, src0);
      return;
    default:
      // default operations - needs unpacking before and packing after
      break;
  }

  c->punpcklbw(src0, g->xmmZero().r());
  c->punpcklbw(dst0, g->xmmZero().r());

  processPixelsUnpacked(dst0, src0, two);

  c->packuswb(dst0, dst0);
}

void CompositeModule_32_SSE2::processPixelsRaw_4(
  const XMMRegister& dst0, const XMMRegister& src0,
  const XMMRegister& dst1, const XMMRegister& src1,
  UInt32 flags)
{
  if (flags & Raw4UnpackFromSrc0) c->movdqa(src1, src0);
  if (flags & Raw4UnpackFromDst0) c->movdqa(dst1, dst0);

  if (flags & Raw4UnpackFromSrc0)
  {
    c->punpcklbw(src0, g->xmmZero().r());
    c->punpckhbw(src1, g->xmmZero().r());
  }
  else
  {
    c->punpcklbw(src0, g->xmmZero().r());
    c->punpcklbw(src1, g->xmmZero().r());
  }

  if (flags & Raw4UnpackFromDst0)
  {
    c->punpcklbw(dst0, g->xmmZero().r());
    c->punpckhbw(dst1, g->xmmZero().r());
  }
  else
  {
    c->punpcklbw(dst0, g->xmmZero().r());
    c->punpcklbw(dst1, g->xmmZero().r());
  }

  processPixelsUnpacked_4(dst0, src0, dst1, src1);

  if (flags & Raw4PackToDst0)
  {
    c->packuswb(dst0, dst1);
  }
  else
  {
    c->packuswb(dst0, dst0);
    c->packuswb(dst1, dst1);
  }
}

void CompositeModule_32_SSE2::processPixelsUnpacked(
  const XMMRegister& dst0, const XMMRegister& src0, bool two)
{
  XMMRef _t0(c->newVariable(VARIABLE_TYPE_XMM, 0));
  XMMRef _t1(c->newVariable(VARIABLE_TYPE_XMM, 0));

  XMMRegister t0 = _t0.r();
  XMMRegister t1 = _t1.r();

  switch (op->id())
  {
    // Dca' = Sca
    // Da'  = Sa
    case Operator::CompositeSrc:
    {
      // copy operation (optimized in frontends and also by Generator itself)
      c->movdqa(dst0, src0);
      break;
    }

    // Dca' = Dca
    // Da'  = Da
    case Operator::CompositeDest:
    {
      // no operation (optimized in frontends and also by Generator itself)
      break;
    }

    // Dca' = Sca + Dca.(1 - Sa)
    // Da'  = Sa + Da.(1 - Sa)
    //      = Sa + Da - Sa.Da
    case Operator::CompositeOver:
    {
      g->_ExtractAlpha(t0, src0, dstAlphaPos, true, two);
      g->_PackedMultiply(dst0, t0, t0);
      c->paddusb(dst0, src0);
      break;
    }

    // Dca' = Dca + Sca.(1 - Da)
    // Da'  = Da + Sa.(1 - Da)
    //      = Da + Sa - Da.Sa
    case Operator::CompositeOverReverse:
    {
      g->_ExtractAlpha(t0, dst0, dstAlphaPos, true, two);
      g->_PackedMultiply(src0, t0, t0);
      c->paddusb(dst0, src0);
      break;
    }

    // Dca' = Sca.Da
    // Da'  = Sa.Da 
    case Operator::CompositeIn:
    {
      g->_ExtractAlpha(t0, dst0, dstAlphaPos, false, two);
      g->_PackedMultiply(src0, t0, dst0, true);
      break;
    }

    // Dca' = Dca.Sa
    // Da'  = Da.Sa
    case Operator::CompositeInReverse:
    {
      g->_ExtractAlpha(t0, src0, dstAlphaPos, false, two);
      g->_PackedMultiply(dst0, t0, t0);
      break;
    }

    // Dca' = Sca.(1 - Da)
    // Da'  = Sa.(1 - Da) 
    case Operator::CompositeOut:
    {
      g->_ExtractAlpha(t0, dst0, dstAlphaPos, true, two);
      g->_PackedMultiply(src0, t0, dst0, true);
      break;
    }

    // Dca' = Dca.(1 - Sa) 
    // Da'  = Da.(1 - Sa) 
    case Operator::CompositeOutReverse:
    {
      g->_ExtractAlpha(t0, src0, dstAlphaPos, true, two);
      g->_PackedMultiply(dst0, t0, t0);
      break;
    }

    // Dca' = Sca.Da + Dca.(1 - Sa)
    // Da'  = Sa.Da + Da.(1 - Sa)
    //      = Da.(Sa + 1 - Sa)
    //      = Da
    case Operator::CompositeAtop:
    {
      g->_ExtractAlpha(t0, src0, dstAlphaPos, true, two);
      g->_ExtractAlpha(t1, dst0, dstAlphaPos, false, two);
      g->_PackedMultiplyAdd(src0, t1, dst0, t0, dst0, true);
      break;
    }

    // Dca' = Dca.Sa + Sca.(1 - Da)
    // Da'  = Da.Sa + Sa.(1 - Da)
    //      = Sa.(Da + 1 - Da)
    //      = Sa 
    case Operator::CompositeAtopReverse:
    {
      g->_ExtractAlpha(t0, src0, dstAlphaPos, false, two);
      g->_ExtractAlpha(t1, dst0, dstAlphaPos, true, two);
      g->_PackedMultiplyAdd(src0, t1, dst0, t0, dst0, true);
      break;
    }

    // Dca' = Sca.(1 - Da) + Dca.(1 - Sa)
    // Da'  = Sa.(1 - Da) + Da.(1 - Sa)
    //      = Sa + Da - 2.Sa.Da 
    case Operator::CompositeXor:
    {
      g->_ExtractAlpha(t0, src0, dstAlphaPos, true, two);
      g->_ExtractAlpha(t1, dst0, dstAlphaPos, true, two);
      g->_PackedMultiplyAdd(
        src0, t1, 
        dst0, t0, 
        dst0, true);
      break;
    }

    // Dca' = 0
    // Da'  = 0
    case Operator::CompositeClear:
    {
      // clear operation (optimized in frontends and also by Generator itself)
      c->pxor(dst0, dst0);
      break;
    }

    // Dca' = Sca + Dca
    // Da'  = Sa + Da 
    case Operator::CompositeAdd:
    {
      c->paddusb(dst0, src0);
      break;
    }

    // Dca' = Dca - Sca
    // Da'  = 1 - (1 - Sa).(1 - Da)
    case Operator::CompositeSubtract:
    {
      g->_ExtractAlpha(t0, src0, dstAlphaPos, true, two);
      g->_ExtractAlpha(t1, dst0, dstAlphaPos, true, two);
      g->_PackedMultiply(t0, t1, t1);
      c->psubusb(dst0, src0);
      c->por(dst0, BLITJIT_GETCONST(g, Cx00FF00000000000000FF000000000000));
      c->pand(t0, BLITJIT_GETCONST(g, Cx00FF00000000000000FF000000000000));
      c->psubusb(dst0, t0);
      break;
    }

    // Dca' = Sca.Dca + Sca.(1 - Da) + Dca.(1 - Sa)
    // Da'  = Sa.Da + Sa.(1 - Da) + Da.(1 - Sa)
    //      = Sa + Da - Sa.Da 
    case Operator::CompositeMultiply:
    {
      XMMRef _t2(c->newVariable(VARIABLE_TYPE_XMM, 0));
      XMMRegister t2 = _t2.r();

      c->movdqa(t2, dst0);
      g->_PackedMultiply(t2, dst0, t0);

      g->_ExtractAlpha(t0, src0, dstAlphaPos, true, two); // t0 == 1-alpha from src0
      g->_ExtractAlpha(t1, dst0, dstAlphaPos, true, two); // t1 == 1-alpha from dst0

      g->_PackedMultiplyAdd(dst0, t0, src0, t1, t0);
      c->paddusw(dst0, t2);
      break;
    }

    // Dca' = (Sca.Da + Dca.Sa - Sca.Dca) + Sca.(1 - Da) + Dca.(1 - Sa)
    //      = Sca + Dca - Sca.Dca
    // Da'  = Sa + Da - Sa.Da 
    case Operator::CompositeScreen:
    {
      c->movdqa(t0, dst0);
      c->paddusw(dst0, src0);
      g->_PackedMultiply(t0, src0, t1);
      c->psubusw(dst0, t0);
      break;
    }

    // Dca' = min(Sca.Da, Dca.Sa) + Sca.(1 - Da) + Dca.(1 - Sa)
    // Da'  = min(Sa.Da, Da.Sa) + Sa.(1 - Da) + Da.(1 - Sa)
    //      = Sa.Da + Sa - Sa.Da + Da - Sa.Da
    //      = Sa + Da - Sa.Da 
    case Operator::CompositeDarken:
    // ... go through ...

    // Dca' = max(Sca.Da, Dca.Sa) + Sca.(1 - Da) + Dca.(1 - Sa)
    // Da'  = max(Sa.Da, Da.Sa) + Sa.(1 - Da) + Da.(1 - Sa)
    //      = Sa.Da + Sa - Sa.Da + Da - Sa.Da
    //      = Sa + Da - Sa.Da 
    case Operator::CompositeLighten:
    {
      XMMRef _t2(c->newVariable(VARIABLE_TYPE_XMM, 0));
      XMMRef _t3(c->newVariable(VARIABLE_TYPE_XMM, 0));

      XMMRegister t2 = _t2.r();
      XMMRegister t3 = _t3.r();

      g->_ExtractAlpha_4(
        t0, src0, dstAlphaPos, false,
        t1, dst0, dstAlphaPos, false);
      g->_PackedMultiply_4(
        t0, dst0, t2,
        t1, src0, t3, true);

      if (op->id() == Operator::CompositeDarken)
        c->pminsw(t3, t2);
      else
        c->pmaxsw(t3, t2);

      g->_ExtractAlpha_4(
        t0, src0, dstAlphaPos, true,
        t1, dst0, dstAlphaPos, true);
      g->_PackedMultiplyAdd(
        dst0, t0,
        src0, t1, t2);
      c->paddusw(dst0, t3);
      break;
    }

    // Dca' = Sca + Dca - 2.min(Sca.Da, Dca.Sa)
    // Da'  = Sa + Da - min(Sa.Da, Da.Sa)
    //      = Sa + Da - Sa.Da
    case Operator::CompositeDifference:
    {
      XMMRef _t2(c->newVariable(VARIABLE_TYPE_XMM, 0));
      XMMRef _t3(c->newVariable(VARIABLE_TYPE_XMM, 0));

      XMMRegister t2 = _t2.r();
      XMMRegister t3 = _t3.r();

      g->_ExtractAlpha_4(
        t0, src0, dstAlphaPos, false,
        t1, dst0, dstAlphaPos, false);
      g->_PackedMultiply_4(
        t0, dst0, t2,
        t1, src0, t3);
      c->pminsw(t0, t1);
      c->paddusw(dst0, src0);
      c->psubusw(dst0, t0);
      c->pand(t0, BLITJIT_GETCONST(g, Cx000000FF00FF00FF000000FF00FF00FF));
      c->psubusw(dst0, t0);
      break;
    }

    // Dca' = (Sca.Da + Dca.Sa - 2.Sca.Dca) + Sca.(1 - Da) + Dca.(1 - Sa)
    // Dca' = Sca + Dca - 2.Sca.Dca
    //      
    // Da'  = (Sa.Da + Da.Sa - 2.Sa.Da) + Sa.(1 - Da) + Da.(1 - Sa)
    //      = Sa - Sa.Da + Da - Da.Sa = Sa + Da - 2.Sa.Da
    // Substitute 2.Sa.Da with Sa.Da
    //
    // Da'  = Sa + Da - Sa.Da 
    case Operator::CompositeExclusion:
    {
      c->movdqa(t0, src0);
      g->_PackedMultiply(t0, dst0, t1);
      c->paddusw(dst0, src0);
      c->psubusw(dst0, t0);
      c->pand(t0, BLITJIT_GETCONST(g, Cx000000FF00FF00FF000000FF00FF00FF));
      c->psubusw(dst0, t0);
      break;
    }

    // Dca' = (Da - Dca) * Sa + Dca.(1 - Sa)
    // Da'  = Sa + (Da - Da) * Sa + Da - Sa.Da
    //      = Sa + Da - Sa.Da
    case Operator::CompositeInvert:
    {
      XMMRef _t2(c->newVariable(VARIABLE_TYPE_XMM, 0));
      XMMRef _t3(c->newVariable(VARIABLE_TYPE_XMM, 0));

      XMMRegister t2 = _t2.r();
      XMMRegister t3 = _t3.r();

      g->_ExtractAlpha_4(
        t0, src0, dstAlphaPos, false,  // Sa
        t2, dst0, dstAlphaPos, false); // Da

      c->movdqa(t1, t0);
      c->psubusb(t2, dst0);
      c->pxor(t1, BLITJIT_GETCONST(g, Cx00FF00FF00FF00FF00FF00FF00FF00FF));

      // t1 = 1 - Sa
      // t2 = Da - Dca

      g->_PackedMultiply_4(
        t2, t0, t3,     // t2   = Sa.(Da - Dca)
        dst0, t1, t1);  // dst0 = Dca.(1 - Sa)

      c->pand(t0, BLITJIT_GETCONST(g, Cx00FF00000000000000FF000000000000));
      c->paddusb(dst0, t2);
      c->paddusb(dst0, t0);
      break;
    }

    // Dca' = (Da - Dca) * Sca + Dca.(1 - Sa)
    // Da'  = Sa + (Da - Da) * Sa + Da - Da.Sa
    //      = Sa + Da - Da.Sa
    case Operator::CompositeInvertRgb:
    {
      XMMRef _t2(c->newVariable(VARIABLE_TYPE_XMM, 0));
      XMMRef _t3(c->newVariable(VARIABLE_TYPE_XMM, 0));

      XMMRegister t2 = _t2.r();
      XMMRegister t3 = _t3.r();

      g->_ExtractAlpha_4(
        t0, src0, dstAlphaPos, false,  // Sa
        t2, dst0, dstAlphaPos, false); // Da

      c->movdqa(t1, t0);
      c->psubw(t2, dst0);
      c->pxor(t1, BLITJIT_GETCONST(g, Cx00FF00FF00FF00FF00FF00FF00FF00FF));

      // t1 = 1 - Sa
      // t2 = Da - Dca

      g->_PackedMultiply_4(
        t2, src0, t3,   // t2   = Sca.(Da - Dca)
        dst0, t1, t1);  // dst0 = Dca.(1 - Sa)

      c->pand(t0, BLITJIT_GETCONST(g, Cx00FF00000000000000FF000000000000));
      c->paddusb(dst0, t2);
      c->paddusb(dst0, t0);
      break;
    }
  }
}

void CompositeModule_32_SSE2::processPixelsUnpacked_4(
  const XMMRegister& dst0, const XMMRegister& src0,
  const XMMRegister& dst1, const XMMRegister& src1)
{
  XMMRef _t0(c->newVariable(VARIABLE_TYPE_XMM, 0));
  XMMRef _t1(c->newVariable(VARIABLE_TYPE_XMM, 0));

  XMMRegister t0 = _t0.r();
  XMMRegister t1 = _t1.r();

  switch (op->id())
  {
    case Operator::CompositeSrc:
      // copy operation (optimized in frontends and also by Generator itself)
      c->movdqa(dst0, src0);
      c->movdqa(dst1, src1);
      break;
    case Operator::CompositeDest:
      // no operation (optimized in frontends and also by Generator itself)
      break;
    case Operator::CompositeOver:
      g->_ExtractAlpha_4(
        t0, src0, dstAlphaPos, true,
        t1, src1, dstAlphaPos, true);
      g->_PackedMultiply_4(
        dst0, t0, t0,
        dst1, t1, t1);
      c->paddusb(dst0, src0);
      c->paddusb(dst1, src1);
      break;
    case Operator::CompositeOverReverse:
      g->_ExtractAlpha_4(
        t0, dst0, dstAlphaPos, true,
        t1, dst1, dstAlphaPos, true);
      g->_PackedMultiply_4(
        src0, t0, t0,
        src1, t1, t1);
      c->paddusb(dst0, src0);
      c->paddusb(dst1, src1);
      break;
    case Operator::CompositeIn:
      g->_ExtractAlpha_4(
        t0, dst0, dstAlphaPos, false,
        t1, dst1, dstAlphaPos, false);
      g->_PackedMultiply_4(
        src0, t0, dst0,
        src1, t1, dst1,
        true);
      break;
    case Operator::CompositeInReverse:
      g->_ExtractAlpha_4(
        t0, src0, dstAlphaPos, false,
        t1, src1, dstAlphaPos, false);
      g->_PackedMultiply_4(
        dst0, t0, t0,
        dst1, t1, t1);
      break;
    case Operator::CompositeOut:
      g->_ExtractAlpha_4(
        t0, dst0, dstAlphaPos, true,
        t1, dst1, dstAlphaPos, true);
      g->_PackedMultiply_4(
        src0, t0, dst0,
        src1, t1, dst1,
        true);
      break;
    case Operator::CompositeOutReverse:
      g->_ExtractAlpha_4(
        t0, src0, dstAlphaPos, true,
        t1, src1, dstAlphaPos, true);
      g->_PackedMultiply_4(
        dst0, t0, t0,
        dst1, t1, t1);
      break;
    case Operator::CompositeAtop:
      g->_ExtractAlpha_4(
        t0, src0, dstAlphaPos, true,
        t1, dst0, dstAlphaPos, false);
      g->_PackedMultiplyAdd(src0, t1, dst0, t0, dst0, true);

      g->_ExtractAlpha_4(
        t0, src1, dstAlphaPos, true,
        t1, dst1, dstAlphaPos, false);
      g->_PackedMultiplyAdd(src1, t1, dst1, t0, dst1, true);
      break;
    case Operator::CompositeAtopReverse:
      g->_ExtractAlpha_4(
        t0, src0, dstAlphaPos, false,
        t1, dst0, dstAlphaPos, true);
      g->_PackedMultiplyAdd(src0, t1, dst0, t0, dst0, true);

      g->_ExtractAlpha_4(
        t0, src1, dstAlphaPos, false,
        t1, dst1, dstAlphaPos, true);
      g->_PackedMultiplyAdd(src1, t1, dst1, t0, dst1, true);
      break;
    case Operator::CompositeXor:
      g->_ExtractAlpha_4(
        t0, src0, dstAlphaPos, true,
        t1, dst0, dstAlphaPos, true);
      g->_PackedMultiplyAdd(src0, t1, dst0, t0, dst0, true);

      g->_ExtractAlpha_4(
        t0, src1, dstAlphaPos, true,
        t1, dst1, dstAlphaPos, true);
      g->_PackedMultiplyAdd(src1, t1, dst1, t0, dst1, true);
      break;
    case Operator::CompositeClear:
      // clear operation (optimized in frontends and also by Generator itself)
      c->pxor(dst0, dst0);
      c->pxor(dst1, dst1);
      break;
    case Operator::CompositeAdd:
      c->paddusb(dst0, src0);
      c->paddusb(dst1, src1);
      break;
    case Operator::CompositeSubtract:
    case Operator::CompositeMultiply:
    case Operator::CompositeScreen:
    case Operator::CompositeDarken:
    case Operator::CompositeLighten:
    case Operator::CompositeDifference:
    case Operator::CompositeExclusion:
    case Operator::CompositeInvert:
    case Operator::CompositeInvertRgb:
    {
      BLITJIT_ASSERT(0);
      break;
    }
  }
}

// ============================================================================
// [BlitJit::CreateModule Wrappers]
// ============================================================================

static FillModule* createFillModule(
  Generator* g,
  const PixelFormat* pf,
  const Operator* op)
{
  if (op->id() == Operator::CompositeSrc)
  {
    return new MemSetModule_32(g, pf, op);
  }
  else
  {
    return new FillModule_32_SSE2(g, pf, op);
  }
}

static CompositeModule* createCompositeModule(
  Generator* g,
  const PixelFormat* pfDst,
  const PixelFormat* pfSrc,
  const Operator* op)
{
  if (op->id() == Operator::CompositeSrc)
  {
    return new MemCpyModule_32(g, pfDst, pfSrc, op);
  }
  else
  {
    return new CompositeModule_32_SSE2(g, pfDst, pfSrc, op);
  }
}

// ============================================================================
// [BlitJit::GeneratorBase]
// ============================================================================

GeneratorBase::GeneratorBase(AsmJit::Compiler* _c) :
  c(_c),
  f(NULL)
{
  // First of all, initialize BlitJit library if it's not initialized before.
  Api::init();

  // Create own compiler instance if no one provided.
  if (c == NULL) c = new(_compiler) Compiler();
}

GeneratorBase::~GeneratorBase()
{
  // Delete compiler if we created it.
  if ((void*)c == (void*)_compiler) c->~Compiler();
}

// ============================================================================
// [BlitJit::Generator - Construction / Destruction]
// ============================================================================

static UInt32 getOptimizationFromCpuFeatures(UInt32 features)
{
  // Optimize is enumeration we are using internally to select cpu specific
  // optimizations.
#if defined(ASMJIT_X86)
  // 32-bit mode: Detect features for 32 bit processors
  if (features & CpuInfo::Feature_SSE2)
    return Api::OptimizeSSE2;
  else if (features & CpuInfo::Feature_MMX)
    return Api::OptimizeMMX;
  else
    return Api::OptimizeX86;
#else
  // 64-bit mode: 64-bit processors are SSE2 capable by default.
  return Api::OptimizeSSE2;
#endif
}

Generator::Generator(Compiler* _c) : GeneratorBase(_c)
{
  // Initialize cpu features we will use
  _features = cpuInfo()->features;

  // Detect maximum possible optimization
  _optimization = getOptimizationFromCpuFeatures(_features);

  // Turn ON prefetching by default
  _prefetch = true;
  _prefetch = false;

  // Turn OFF non-thermal hints by default
  _nonThermalHint = false;

  _mainLoopAlignment = 16;

  // Body flags are clean by default
  _body = 0;
}

Generator::~Generator()
{
}

// ============================================================================
// [BlitJit::Generator - Getters / Setters]
// ============================================================================

void Generator::setFeatures(UInt32 features)
{
  _features = features;
  _optimization = getOptimizationFromCpuFeatures(features);
}

void Generator::setOptimization(UInt32 optimization)
{
  _optimization = optimization;
}

void Generator::setPrefetch(bool prefetch)
{
  _prefetch = prefetch;
}

void Generator::setNonThermalHint(bool nonThermalHint)
{
  _nonThermalHint = nonThermalHint;
}

// ============================================================================
// [BlitJit::Generator - Premultiply / Demultiply]
// ============================================================================

//! @brief Generate pixel premultiply function.
void Generator::genPremultiply(const PixelFormat& pfDst)
{
  c->comment("BlitJit::Generator::genPremultiply() - %s", pfDst.name());

  f = c->newFunction(CConv, BuildFunction2<void*, SysUInt>());
  f->setNaked(true);
  f->setAllocableEbp(true);

  PtrRef dst(c->argument(0));
  SysIntRef cnt(c->argument(1));

  // TODO
}

//! @brief Generate pixel demultiply function.
void Generator::genDemultiply(const PixelFormat& pfDst)
{
  c->comment("BlitJit::Generator::genDemultiply() - %s", pfDst.name());

  f = c->newFunction(CConv, BuildFunction2<void*, SysUInt>());
  f->setNaked(true);
  f->setAllocableEbp(true);

  PtrRef dst(c->argument(0));
  SysIntRef cnt(c->argument(1));

  // TODO
}

// ============================================================================
// [BlitJit::Generator - Fill Span / Rect]
// ============================================================================

void Generator::genFillSpan(const PixelFormat& pfDst, const PixelFormat& pfSrc, const Operator& op)
{
  c->comment("BlitJit::Generator::genFillSpan() - %s <- %s : %s", pfDst.name(), pfSrc.name(), op.name());

  f = c->newFunction(CConv, BuildFunction3<void*, const void*, SysUInt>());
  f->setNaked(true);
  f->setAllocableEbp(true);

  // Filter module
  FillModule* module = createFillModule(this, &pfDst, &op);

  if (!module->isNop())
  {
    // Destination and source
    PtrRef dst(c->argument(0));
    PtrRef src(c->argument(1));
    SysIntRef cnt(c->argument(2));

    cnt.alloc();
    dst.alloc();
    src.alloc();

    // Loop properties
    Loop loop;
    loop.finalizePointers = false;

    module->init(src, &pfSrc);
    src.unuse();
    module->beginSwitch();

    for (UInt32 kind = 0; kind < module->numKinds(); kind++)
    {
      module->beginKind(kind);
      _GenFillLoop(dst, cnt, module, kind, loop);
      module->endKind(kind);
    }

    module->endSwitch();
    module->free();
  }

  c->endFunction();

  // Cleanup
  delete module;
}

void Generator::genFillRect(const PixelFormat& pfDst, const PixelFormat& pfSrc, const Operator& op)
{
  c->comment("BlitJit::Generator::genFillRect() - %s <- %s : %s", pfDst.name(), pfSrc.name(), op.name());

  f = c->newFunction(CConv, BuildFunction5<void*, const void*, SysInt, SysUInt, SysUInt>());
  f->setNaked(true);
  f->setAllocableEbp(true);

  // Filter module
  FillModule* module = createFillModule(this, &pfDst, &op);

  if (!module->isNop())
  {
    // Destination and source
    PtrRef dst(c->argument(0));
    PtrRef src(c->argument(1));
    SysIntRef dstStride(c->argument(2));
    SysIntRef width(c->argument(3));
    SysIntRef height(c->argument(4));

    SysIntRef cnt(c->newVariable(VARIABLE_TYPE_SYSINT));

    // Adjust dstStride
    {
      SysIntRef t(c->newVariable(VARIABLE_TYPE_SYSINT));

      c->mov(t.r(), width);
      c->shl(t.r(), 2);

      c->sub(dstStride, t.r());
    }

    cnt.alloc();
    dst.alloc();
    src.alloc();

    // Loop properties
    Loop loop;
    loop.finalizePointers = true;

    module->init(src, &pfSrc);
    src.unuse();
    module->beginSwitch();

    for (UInt32 kind = 0; kind < module->numKinds(); kind++)
    {
      module->beginKind(kind);

      Label* L_Loop = c->newLabel();
      c->bind(L_Loop);
      c->mov(cnt.r(), width);

      _GenFillLoop(dst, cnt, module, kind, loop);

      c->add(dst.r(), dstStride);
      c->sub(height, imm(1));
      c->jnz(L_Loop);

      module->endKind(kind);
    }

    module->endSwitch();
    module->free();
  }

  c->endFunction();

  // Cleanup
  delete module;
}

// ============================================================================
// [BlitJit::Generator - Blit Span / Rect]
// ============================================================================

void Generator::genBlitSpan(const PixelFormat& pfDst, const PixelFormat& pfSrc, const Operator& op)
{
  c->comment("BlitJit::Generator::genBlitSpan() - %s <- %s : %s", pfDst.name(), pfSrc.name(), op.name());

  f = c->newFunction(CConv, BuildFunction3<void*, void*, SysUInt>());
  f->setNaked(true);
  f->setAllocableEbp(true);

  // Compositing module
  CompositeModule* module = createCompositeModule(this, &pfSrc, &pfDst, &op);

  if (!module->isNop())
  {
    // Destination and source
    PtrRef dst(c->argument(0));
    PtrRef src(c->argument(1));
    SysIntRef cnt(c->argument(2));

    cnt.alloc();
    dst.alloc();
    src.alloc();

    // Loop properties
    Loop loop;
    loop.finalizePointers = false;

    module->init();
    module->beginSwitch();

    for (UInt32 kind = 0; kind < module->numKinds(); kind++)
    {
      module->beginKind(kind);
      _GenCompositeLoop(dst, src, cnt, module, kind, loop);
      module->endKind(kind);
    }

    module->endSwitch();
    module->free();
  }

  c->endFunction();

  // Cleanup
  delete module;
}

void Generator::genBlitRect(const PixelFormat& pfDst, const PixelFormat& pfSrc, const Operator& op)
{
  c->comment("BlitJit::Generator::genBlitRect() - %s <- %s : %s", pfDst.name(), pfSrc.name(), op.name());

  f = c->newFunction(CConv, BuildFunction6<void*, void*, SysInt, SysInt, SysUInt, SysUInt>());
  f->setNaked(true);
  f->setAllocableEbp(true);

  // Compositing module
  CompositeModule* module = createCompositeModule(this, &pfSrc, &pfDst, &op);

  if (!module->isNop())
  {
    // Destination and source
    PtrRef dst(c->argument(0));
    PtrRef src(c->argument(1));
    SysIntRef dstStride(c->argument(2));
    SysIntRef srcStride(c->argument(3));
    SysIntRef width(c->argument(4));
    SysIntRef height(c->argument(5));

    SysIntRef cnt(c->newVariable(VARIABLE_TYPE_SYSINT));

    // Adjust dstStride and srcStride
    {
      SysIntRef t(c->newVariable(VARIABLE_TYPE_SYSINT));

      c->mov(t.r(), width);
      c->shl(t.r(), 2);

      c->sub(dstStride, t.r());
      c->sub(srcStride, t.r());
    }

    cnt.alloc();
    dst.alloc();
    src.alloc();

    // Loop properties
    Loop loop;
    loop.finalizePointers = true;

    module->init();
    module->beginSwitch();

    for (UInt32 kind = 0; kind < module->numKinds(); kind++)
    {
      Label* L_Loop = c->newLabel();
      c->bind(L_Loop);
      c->mov(cnt.r(), width);

      module->beginKind(kind);
      _GenCompositeLoop(dst, src, cnt, module, kind, loop);
      module->endKind(kind);

      c->add(dst.r(), dstStride);
      c->add(src.r(), srcStride);
      c->sub(height, imm(1));
      c->jnz(L_Loop);
    }

    module->endSwitch();
    module->free();
  }

  c->endFunction();

  // Cleanup
  delete module;
}

// ==========================================================================
// [BlitJit::Generator - Loops]
// ==========================================================================

// Calculate correct alignment. We need to use dstSize and pixels perLoop.
// If perLoop is too short (1, maybe two pixels) alignment is in most cases
// no necessary (for two pixels it's simple condition).
static SysInt getAlignment(SysInt dstSize, SysInt perLoop)
{
  SysInt align = 0;
  if (perLoop <= 1) return align;

  switch (dstSize)
  {
    case 1:
      if      (perLoop >=16) align =16;
      else if (perLoop >= 8) align = 8;
      else if (perLoop >= 4) align = 4;
      else if (perLoop >= 2) align = 2;
      break;
    case 2:
      if      (perLoop >= 8) align =16;
      else if (perLoop >= 4) align = 8;
      else if (perLoop >= 2) align = 4;
      break;
    case 3:
      align = 4;
      break;
    case 4:
      align = (perLoop >= 4) ? 16 : 8;
      break;
    case 8:
      align = 16;
      break;
  }

  return align;
}

void Generator::_GenFillLoop(
  PtrRef& dst,
  SysIntRef& cnt,
  FillModule* module,
  UInt32 kind,
  const Loop& loop)
{
  // First calculate how the loop will be structured
  SysInt perLoop = module->maxPixelsPerLoop();
  SysInt dstSize = module->pf->depth() / 8;

  SysInt align = getAlignment(dstSize, perLoop);

  // Save state
  StateRef state(c->saveState());

  // Helpers
  SysIntRef tmp(c->newVariable(VARIABLE_TYPE_SYSINT));

  // Labels
  Label* L_MainEntry  = (perLoop > 1) ? c->newLabel() : NULL;
  Label* L_MainLoop   = c->newLabel();
  Label* L_Misaligned = NULL;
  // Only if more pixels at a time are used
  Label* L_TailEntry  = (perLoop > 1) ? c->newLabel() : NULL;
  Label* L_TailPre    = (perLoop > 1) ? c->newLabel() : NULL;
  Label* L_TailLoop   = (perLoop > 1) ? c->newLabel() : NULL;
  Label* L_TailSkipLargeJumpTable = c->newLabel();
  // End
  Label* L_End        = c->newLabel();

  // Alloc variables if they are not allocated before
  dst.alloc();
  cnt.alloc();
  // We need one temporary variable for alignment
  if (align != 0) tmp.alloc();
  c->clearPrevented();

  // Skip alignment and main loop if cnt is too small (to be useful)
  if (perLoop > 1 && align)
  {
    c->cmp(cnt.r(), imm(perLoop));
    c->jb(L_TailLoop);
  }

  // Align 4 bytes dst to 8 bytes. We are using trick that if alignment
  // fail, we will continue into main loop, because this type of alignment
  // is usually used together with MMX or X64 modes (no SSE and unaligned
  // exceptions here)
  if (dstSize == 4 && align == 8)
  {
    c->mov(tmp.x(), dst.r());
    c->and_(tmp.r(), imm(7));
    c->jz(L_MainEntry);
    tmp.unuse();

    module->processPixelsPtr(dst, 1, 0, kind, 0);
    c->add(dst.r(), imm(dstSize));
    c->sub(cnt.r(), imm(1));
  }
  // Align 4 bytes dst to 16 bytes. This is hardest job, because we must
  // ensure that destination is aligned for instructions that needs it.
  // This is reason for last check and bail into tail loop if destination
  // is totally misaligned (this shouldn't happpen, but we must be sure).
  else if (dstSize == 4 && align == 16)
  {
    // Calculate how many pixels needs to be aligned
    c->xor_(tmp.x(), tmp.x());
    c->sub(tmp.r(), dst.r());
    c->and_(tmp.r(), imm(15));
    // Alignment not needed, jump to main loop entry
    c->jz(L_MainEntry);

    c->shr(tmp.r(), imm(2));
    c->sub(cnt.r(), tmp.r());

    Label* L_AlignLoop = c->newLabel();
    c->bind(L_AlignLoop);
    module->processPixelsPtr(dst, 1, 0, kind, 0);
    c->add(dst.r(), imm(dstSize));
    c->sub(tmp.r(), imm(1));
    c->jnz(L_AlignLoop);

    // Make sure that dst is aligned to 16 bytes
    c->mov(tmp.x(), dst.r());
    c->and_(tmp.r(), imm(15));

    if (module->complexity() == Module::Simple)
    {
      L_Misaligned = c->newLabel();
      c->jnz(L_Misaligned);
    }
    else
    {
      c->jnz(L_TailLoop);
    }
    tmp.unuse();
  }
  // TODO: Not implemented
  else if (align)
  {
    BLITJIT_ASSERT(0);
  }

  // Main Loop
  if (perLoop > 1)
  {
    // More pixels at a time
    c->bind(L_MainEntry);
    c->sub(cnt.r(), imm(perLoop));
    c->jc(L_TailEntry);

    c->align(_mainLoopAlignment);
    c->bind(L_MainLoop);

    if (_prefetch && module->prefetchDst()) c->prefetch(ptr(dst.r(), perLoop * dstSize), PREFETCH_T0);

    module->processPixelsPtr(dst, perLoop, 0, kind, Module::DstAligned);
    c->add(dst.r(), imm(perLoop * dstSize));
    c->sub(cnt.r(), imm(perLoop));
    c->jnc(L_MainLoop);
  }
  else
  {
    // One pixel at a time
    c->align(_mainLoopAlignment);
    c->bind(L_MainLoop);
    module->processPixelsPtr(dst, 1, 0, kind, 0);
    c->add(dst.r(), imm(dstSize));
    c->sub(cnt.r(), imm(1));
    c->jnz(L_MainLoop);
  }

  if (L_Misaligned)
  {
    bool embedMisalignedTail = (perLoop > 8 && module->complexity() == Module::Simple);

    OutsideBlock block(c);
    Label* L_MisalignedLoop = c->newLabel();
    Label* L_MisalignedTailEntry = embedMisalignedTail
      ? c->newLabel() 
      : L_TailEntry;

    // More pixels at a time
    c->bind(L_Misaligned);
    c->sub(cnt.r(), imm(perLoop));
    c->jc(L_MisalignedTailEntry);

    c->align(_mainLoopAlignment);
    c->bind(L_MisalignedLoop);

    if (_prefetch && module->prefetchDst()) c->prefetch(ptr(dst.r(), perLoop * dstSize), PREFETCH_T0);

    module->processPixelsPtr(dst, perLoop, 0, kind, 0);
    c->add(dst.r(), imm(perLoop * dstSize));
    c->sub(cnt.r(), imm(perLoop));
    c->jnc(L_MisalignedLoop);

    if (!embedMisalignedTail)
    {
      c->jmp(L_TailEntry);
    }
    else
    {
      // Misaligned tail loop
      JumpTable* jumpTable = c->newJumpTable();
      jumpTable->addLabel(L_TailSkipLargeJumpTable, 0);

      tmp.use(c->newVariable(VARIABLE_TYPE_SYSINT));
      c->mov(tmp.x(), cnt.r());
      c->shr(tmp.r(), 2);
      c->jumpToTable(jumpTable, tmp.r());
      tmp.unuse();

      for (SysInt j = (perLoop / 4) - 1; j > 0; j--)
      {
        c->bind(jumpTable->addLabel(NULL, j));
        module->processPixelsPtr(dst, 4, 0, kind, 0);
        c->add(dst.r(), imm(dstSize * 4));
      }
      c->and_(cnt.r(), 3);
      c->jmp(L_TailSkipLargeJumpTable);
    }
  }

  // Tail (only if more pixels at a time loop is used)
  if (perLoop == 1)
  {
    // NOP
  }
  else if (perLoop == 2)
  {
    // Two pixels at a time, it's only needed to process remaining pixel
    c->bind(L_TailEntry);
    c->add(cnt.r(), imm(2));
    c->jz(L_End);

    c->bind(L_TailLoop);
    module->processPixelsPtr(dst, 1, 0, kind, 0);
    if (loop.finalizePointers)
    {
      c->add(dst.r(), imm(dstSize));
    }
  }
  else if (module->complexity() == Module::Simple)
  {
    SysInt i = perLoop;

    // Jump table based loop for simple operators
    c->bind(L_TailEntry);
    c->add(cnt.r(), imm(perLoop));
    c->jz(L_End);

    c->bind(L_TailLoop);

    if (i > 8)
    {
      JumpTable* jumpTable = c->newJumpTable();
      Label* L_Skip = c->newLabel();
      jumpTable->addLabel(L_Skip, 0);

      tmp.use(c->newVariable(VARIABLE_TYPE_SYSINT));
      c->mov(tmp.x(), cnt.r());
      c->shr(tmp.r(), 2);
      c->jumpToTable(jumpTable, tmp.r());
      tmp.unuse();

      for (SysInt j = (i / 4) - 1; j > 0; j--, i -= 4)
      {
        c->bind(jumpTable->addLabel(NULL, j));
        module->processPixelsPtr(dst, 4, 0, kind, Module::DstAligned);
        c->add(dst.r(), imm(dstSize * 4));
      }
      c->and_(cnt.r(), 3);
      c->bind(L_TailSkipLargeJumpTable);
    }

    // Last few pixels will be processed through jump table
    {
      JumpTable* jumpTable = c->newJumpTable();
      jumpTable->addLabel(L_End, 0);

      // cnt is modified by jumpToTable().
      c->jumpToTable(jumpTable, cnt.r());

      for (i--; i > 0; i--)
      {
        c->bind(jumpTable->addLabel(NULL, i));
        module->processPixelsPtr(dst, 1, 0, kind, 0);
        c->add(dst.r(), imm(dstSize));
      }
    }
  }
  else
  {
    // Complex operators
    c->bind(L_TailEntry);
    c->add(cnt.r(), imm(perLoop));
    c->jz(L_End);

    c->bind(L_TailLoop);
    module->processPixelsPtr(dst, 1, 0, kind, 0);
    c->add(dst.r(), imm(dstSize));
    c->sub(cnt.r(), imm(1));
    c->jnz(L_TailLoop);
  }

  // End
  c->bind(L_End);
}

void Generator::_GenCompositeLoop(
  PtrRef& dst,
  PtrRef& src,
  SysIntRef& cnt,
  CompositeModule* module,
  UInt32 kind,
  const Loop& loop)
{
  // First calculate how the loop will be structured
  SysInt perLoop = module->maxPixelsPerLoop();
  SysInt dstSize = module->pfDst->depth() / 8;
  SysInt srcSize = module->pfSrc->depth() / 8;

  SysInt align = getAlignment(dstSize, perLoop);

  // Save state
  StateRef state(c->saveState());

  // Helpers
  SysIntRef tmp(c->newVariable(VARIABLE_TYPE_SYSINT));

  // Labels
  Label* L_MainEntry  = (perLoop > 1) ? c->newLabel() : NULL;
  Label* L_MainLoop   = c->newLabel();
  Label* L_Misaligned = NULL;
  // Only if more pixels at a time are used
  Label* L_TailEntry  = (perLoop > 1) ? c->newLabel() : NULL;
  Label* L_TailPre    = (perLoop > 1) ? c->newLabel() : NULL;
  Label* L_TailLoop   = (perLoop > 1) ? c->newLabel() : NULL;
  Label* L_TailSkipLargeJumpTable = c->newLabel();
  // End
  Label* L_End        = c->newLabel();

  // Alloc variables if they are not allocated before
  dst.alloc();
  src.alloc();
  cnt.alloc();
  // We need one temporary variable for alignment
  if (align != 0) tmp.alloc();
  c->clearPrevented();

  // Skip alignment and main loop if cnt is too small (to be useful)
  if (perLoop > 1 && align)
  {
    c->cmp(cnt.r(), imm(perLoop));
    c->jb(L_TailLoop);
  }

  // Align 4 bytes dst to 8 bytes. We are using trick that if alignment
  // fail, we will continue into main loop, because this type of alignment
  // is usually used together with MMX or X64 modes (no SSE and unaligned
  // exceptions here)
  if (dstSize == 4 && align == 8)
  {
    c->mov(tmp.x(), dst.r());
    c->and_(tmp.r(), imm(7));
    c->jz(L_MainEntry);
    tmp.unuse();

    module->processPixelsPtr(dst, src, 1, 0, kind, 0);
    c->add(dst.r(), imm(dstSize));
    c->add(src.r(), imm(srcSize));
    c->sub(cnt.r(), imm(1));
  }
  // Align 4 bytes dst to 16 bytes. This is hardest job, because we must
  // ensure that destination is aligned for instructions that needs it.
  // This is reason for last check and bail into tail loop if destination
  // is totally misaligned (this shouldn't happpen, but we must be sure).
  else if (dstSize == 4 && align == 16)
  {
    // Calculate how many pixels needs to be aligned
    c->xor_(tmp.x(), tmp.x());
    c->sub(tmp.r(), dst.r());
    c->and_(tmp.r(), imm(15));
    // Alignment not needed, jump to main loop entry
    c->jz(L_MainEntry);

    c->shr(tmp.r(), imm(2));
    c->sub(cnt.r(), tmp.r());

    Label* L_AlignLoop = c->newLabel();
    c->bind(L_AlignLoop);
    module->processPixelsPtr(dst, src, 1, 0, kind, 0);
    c->add(dst.r(), imm(dstSize));
    c->add(src.r(), imm(srcSize));
    c->sub(tmp.r(), imm(1));
    c->jnz(L_AlignLoop);

    // Make sure that dst is aligned to 16 bytes
    c->mov(tmp.x(), dst.r());
    c->and_(tmp.r(), imm(15));

    if (module->complexity() == Module::Simple)
    {
      L_Misaligned = c->newLabel();
      c->jnz(L_Misaligned);
    }
    else
    {
      c->jnz(L_TailLoop);
    }
    tmp.unuse();
  }
  // TODO: Not implemented
  else if (align)
  {
    BLITJIT_ASSERT(0);
  }

  // Main Loop
  if (perLoop > 1)
  {
    // More pixels at a time
    c->bind(L_MainEntry);
    c->sub(cnt.r(), imm(perLoop));
    c->jc(L_TailEntry);

    c->align(_mainLoopAlignment);
    c->bind(L_MainLoop);

    if (_prefetch && module->prefetchSrc()) c->prefetch(ptr(src.r(), perLoop * srcSize), PREFETCH_T0);
    if (_prefetch && module->prefetchDst()) c->prefetch(ptr(dst.r(), perLoop * dstSize), PREFETCH_T0);

    module->processPixelsPtr(dst, src, perLoop, 0, kind, Module::DstAligned);
    c->add(dst.r(), imm(perLoop * dstSize));
    c->add(src.r(), imm(perLoop * srcSize));
    c->sub(cnt.r(), imm(perLoop));
    c->jnc(L_MainLoop);
  }
  else
  {
    // One pixel at a time
    c->align(_mainLoopAlignment);
    c->bind(L_MainLoop);
    module->processPixelsPtr(dst, src, 1, 0, kind, 0);
    c->add(dst.r(), imm(dstSize));
    c->add(src.r(), imm(srcSize));
    c->sub(cnt.r(), imm(1));
    c->jnz(L_MainLoop);
  }

  if (L_Misaligned)
  {
    bool embedMisalignedTail = (perLoop > 8 && module->complexity() == Module::Simple);

    OutsideBlock block(c);
    Label* L_MisalignedLoop = c->newLabel();
    Label* L_MisalignedTailEntry = embedMisalignedTail
      ? c->newLabel() 
      : L_TailEntry;

    // More pixels at a time
    c->bind(L_Misaligned);
    c->sub(cnt.r(), imm(perLoop));
    c->jc(L_MisalignedTailEntry);

    c->align(_mainLoopAlignment);
    c->bind(L_MisalignedLoop);

    if (_prefetch && module->prefetchSrc()) c->prefetch(ptr(src.r(), perLoop * srcSize), PREFETCH_T0);
    if (_prefetch && module->prefetchDst()) c->prefetch(ptr(dst.r(), perLoop * dstSize), PREFETCH_T0);

    module->processPixelsPtr(dst, src, perLoop, 0, kind, 0);
    c->add(dst.r(), imm(perLoop * dstSize));
    c->add(src.r(), imm(perLoop * srcSize));
    c->sub(cnt.r(), imm(perLoop));
    c->jnc(L_MisalignedLoop);

    if (!embedMisalignedTail)
    {
      c->jmp(L_TailEntry);
    }
    else
    {
      // Misaligned tail loop
      JumpTable* jumpTable = c->newJumpTable();
      jumpTable->addLabel(L_TailSkipLargeJumpTable, 0);

      tmp.use(c->newVariable(VARIABLE_TYPE_SYSINT));
      c->mov(tmp.x(), cnt.r());
      c->shr(tmp.r(), 2);
      c->jumpToTable(jumpTable, tmp.r());
      tmp.unuse();

      for (SysInt j = (perLoop / 4) - 1; j > 0; j--)
      {
        c->bind(jumpTable->addLabel(NULL, j));
        module->processPixelsPtr(dst, src, 4, 0, kind, 0);
        c->add(dst.r(), imm(dstSize * 4));
        c->add(src.r(), imm(srcSize * 4));
      }
      c->and_(cnt.r(), 3);
      c->jmp(L_TailSkipLargeJumpTable);
    }
  }

  // Tail (only if more pixels at a time loop is used)
  if (perLoop == 1)
  {
    // NOP
  }
  else if (perLoop == 2)
  {
    // Two pixels at a time, it's only needed to process remaining pixel
    c->bind(L_TailEntry);
    c->add(cnt.r(), imm(2));
    c->jz(L_End);

    c->bind(L_TailLoop);
    module->processPixelsPtr(dst, src, 1, 0, kind, 0);
    if (loop.finalizePointers)
    {
      c->add(dst.r(), imm(dstSize));
      c->add(src.r(), imm(srcSize));
    }
  }
  else if (module->complexity() == Module::Simple)
  {
    SysInt i = perLoop;

    // Jump table based loop for simple operators
    c->bind(L_TailEntry);
    c->add(cnt.r(), imm(perLoop));
    c->jz(L_End);

    c->bind(L_TailLoop);

    if (i > 8)
    {
      JumpTable* jumpTable = c->newJumpTable();
      jumpTable->addLabel(L_TailSkipLargeJumpTable, 0);

      tmp.use(c->newVariable(VARIABLE_TYPE_SYSINT));
      c->mov(tmp.x(), cnt.r());
      c->shr(tmp.r(), 2);
      c->jumpToTable(jumpTable, tmp.r());
      tmp.unuse();

      for (SysInt j = (i / 4) - 1; j > 0; j--, i -= 4)
      {
        c->bind(jumpTable->addLabel(NULL, j));
        module->processPixelsPtr(dst, src, 4, 0, kind, Module::DstAligned);
        c->add(dst.r(), imm(dstSize * 4));
        c->add(src.r(), imm(srcSize * 4));
      }
      c->and_(cnt.r(), 3);
      c->bind(L_TailSkipLargeJumpTable);
    }

    // Last few pixels will be processed through jump table
    {
      JumpTable* jumpTable = c->newJumpTable();
      jumpTable->addLabel(L_End, 0);

      // cnt is modified by jumpToTable().
      c->jumpToTable(jumpTable, cnt.r());

      for (i--; i > 0; i--)
      {
        c->bind(jumpTable->addLabel(NULL, i));
        module->processPixelsPtr(dst, src, 1, 0, kind, 0);
        c->add(dst.r(), imm(dstSize));
        c->add(src.r(), imm(srcSize));
      }
    }
  }
  else
  {
    // Complex operators
    c->bind(L_TailEntry);
    c->add(cnt.r(), imm(perLoop));
    c->jz(L_End);

    c->bind(L_TailLoop);
    module->processPixelsPtr(dst, src, 1, 0, kind, 0);
    c->add(dst.r(), imm(dstSize));
    c->add(src.r(), imm(srcSize));
    c->sub(cnt.r(), imm(1));
    c->jnz(L_TailLoop);
  }

  // End
  c->bind(L_End);

  BLITJIT_ASSERT(!L_TailSkipLargeJumpTable->isLinked());
}

// ============================================================================
// [BlitJit::Generator - Mov Helpers]
// ============================================================================

void Generator::_LoadMov(const Register& dst, const Mem& src)
{
  c->mov(dst, src);
}

void Generator::_LoadMovQ(const MMRegister& dst, const Mem& src)
{
  c->movq(dst, src);
}

void Generator::_LoadMovDQ(const XMMRegister& dst, const Mem& src, bool aligned)
{
  if (aligned)
    c->movdqa(dst, src);
  else
    c->movdqu(dst, src);
}

void Generator::_StoreMov(const Mem& dst, const Register& src, bool nt)
{
  if (nt && (_features & AsmJit::CpuInfo::Feature_SSE2) != 0)
  {
    c->movnti(dst, src);
    f->setSfence(true);
  }
  else
  {
    c->mov(dst, src);
  }
}

void Generator::_StoreMovQ(const Mem& dst, const MMRegister& src, bool nt)
{
  if (nt && (_features & (
      AsmJit::CpuInfo::Feature_SSE |
      AsmJit::CpuInfo::Feature_3dNow)) != 0)
  {
    c->movntq(dst, src);
    f->setSfence(true);
  }
  else
  {
    c->movq(dst, src);
  }
}

void Generator::_StoreMovDQ(const Mem& dst, const XMMRegister& src, bool nt, bool aligned)
{
  if (nt && aligned)
  {
    c->movntdq(dst, src);
    f->setSfence(true);
  }
  else if (aligned)
  {
    c->movdqa(dst, src);
  }
  else
  {
    c->movdqu(dst, src);
  }
}

// ==========================================================================
// [BlitJit::Generator - Constants Management]
// ==========================================================================

void Generator::usingConstants()
{
  // Don't initialize it more times
  if (_body & BodyUsingConstants) return;

#if defined(BLITJIT_X86)
  // 32-bit mode: Nop
#else
  // 64-bit mode: Allocate register and set custom alloc/spill functions.
  // TODO:
  _rConstantsAddress.use(c->newVariable(VARIABLE_TYPE_PTR, 0));
  c->mov(_rConstantsAddress.x(), imm((SysInt)Api::constants));
#endif

  // Initialized, this will prevent us to do initialization more times
  _body |= BodyUsingConstants;
}

Mem Generator::getConstantsOperand(SysInt displacement)
{
#if defined(BLITJIT_X86)
  // 32-bit mode: Absolute address
  return ptr_abs(Api::constants, displacement);
#else
  // 64-bit mode: Register + Displacement
  return ptr(_rConstantsAddress.c(), displacement);
#endif
}

// ==========================================================================
// [BlitJit::Generator - MMX/SSE Zero Registers]
// ==========================================================================

// custom alloc / spills
static void customAlloc_zero(Variable* v)
{
  Compiler* c = v->compiler();

  switch (v->type())
  {
    case VARIABLE_TYPE_MM:
    {
      MMRegister r = mk_mm(v->registerCode());
      c->pxor(r, r);
      break;
    }
    case VARIABLE_TYPE_XMM:
    {
      XMMRegister r = mk_xmm(v->registerCode());
      c->pxor(r, r);
      break;
    }
  }
}

static void customAlloc_const(Variable* v)
{
  Compiler* c = v->compiler();

  Generator* g = (Generator*)v->dataPtr();
  SysInt displacement = v->dataInt();

  switch (v->type())
  {
    case VARIABLE_TYPE_MM:
    {
      MMRegister r = mk_mm(v->registerCode());
      c->movq(r, g->getConstantsOperand(displacement));
      break;
    }
    case VARIABLE_TYPE_XMM:
    {
      XMMRegister r = mk_xmm(v->registerCode());
      c->movdqa(r, g->getConstantsOperand(displacement));
      break;
    }
  }
}

static void customSpill_none(Variable* v)
{
  BLITJIT_USE(v);
}

void Generator::usingMMZero()
{
  // Don't initialize it more times
  if (_body & BodyUsingMMZero) return;

  // MM register with custom alloc / spill
  _mmZero.use(c->newVariable(VARIABLE_TYPE_MM, 20));
  _mmZero.setAllocFn(customAlloc_zero);
  _mmZero.setSpillFn(customSpill_none);
  _mmZero.alloc();

  // Initialized, this will prevent us to do initialization more times
  _body |= BodyUsingMMZero;
}

void Generator::usingXMMZero()
{
  // Don't initialize it more times
  if (_body & BodyUsingXMMZero) return;

  // XMM register with custom alloc / spill
  _xmmZero.use(c->newVariable(VARIABLE_TYPE_XMM, 20));
  _xmmZero.setAllocFn(customAlloc_zero);
  _xmmZero.setSpillFn(customSpill_none);
  _xmmZero.alloc();

  // Initialized, this will prevent us to do initialization more times
  _body |= BodyUsingXMMZero;
}

void Generator::usingXMM0080()
{
  // Don't initialize it more times
  if (_body & BodyUsingXMM0080) return;

  usingConstants();

  // XMM register with custom alloc / spill
  _xmm0080.use(c->newVariable(VARIABLE_TYPE_XMM, 20));
  _xmm0080.setAllocFn(customAlloc_const);
  _xmm0080.setSpillFn(customSpill_none);
  _xmm0080.setDataPtr((void*)this);
  _xmm0080.setDataInt(BLITJIT_DISPCONST(Cx00FF00FF00FF00FF00FF00FF00FF00FF));
  _xmm0080.alloc();

  // Initialized, this will prevent us to do initialization more times
  _body |= BodyUsingXMM0080;
}

// ==========================================================================
// [BlitJit::Generator - Generator Helpers]
// ==========================================================================

void Generator::_ExtractAlpha(
  const XMMRegister& dst0, const XMMRegister& src0, UInt8 alphaPos0, UInt8 negate0, bool two)
{
  c->pshuflw(dst0, src0, mm_shuffle(alphaPos0, alphaPos0, alphaPos0, alphaPos0));
  if (two) c->pshufhw(dst0, dst0, mm_shuffle(alphaPos0, alphaPos0, alphaPos0, alphaPos0));

  if (negate0) c->pxor(dst0, BLITJIT_GETCONST(this, Cx00FF00FF00FF00FF00FF00FF00FF00FF));
}

void Generator::_ExtractAlpha_4(
  const XMMRegister& dst0, const XMMRegister& src0, UInt8 alphaPos0, UInt8 negate0,
  const XMMRegister& dst1, const XMMRegister& src1, UInt8 alphaPos1, UInt8 negate1)
{
  c->pshuflw(dst0, src0, mm_shuffle(alphaPos0, alphaPos0, alphaPos0, alphaPos0));
  c->pshuflw(dst1, src1, mm_shuffle(alphaPos1, alphaPos1, alphaPos1, alphaPos1));

  c->pshufhw(dst0, dst0, mm_shuffle(alphaPos0, alphaPos0, alphaPos0, alphaPos0));
  c->pshufhw(dst1, dst1, mm_shuffle(alphaPos1, alphaPos1, alphaPos1, alphaPos1));

  if (negate0) c->pxor(dst0, BLITJIT_GETCONST(this, Cx00FF00FF00FF00FF00FF00FF00FF00FF));
  if (negate1) c->pxor(dst1, BLITJIT_GETCONST(this, Cx00FF00FF00FF00FF00FF00FF00FF00FF));
}

void Generator::_PackedMultiply(
  const XMMRegister& a0, const XMMRegister& b0, const XMMRegister& t0,
  bool moveToT0)
{
  XMMRegister r = xmm0080().c();

  c->pmullw(a0, b0);          // a0 *= b0
  c->paddusw(a0, r);          // a0 += 80
  c->movdqa(t0, a0);          // t0  = a0
  c->psrlw(a0, 8);            // a0 /= 256
  if (!moveToT0)
  {
    c->paddusw(a0, t0);       // a0 += t0
    c->psrlw(a0, 8);          // a0 /= 256
  }
  else
  {
    c->paddusw(t0, a0);       // t0 += a0
    c->psrlw(t0, 8);          // t0 /= 256
  }
}

void Generator::_PackedMultiplyWithAddition(
  const XMMRegister& a0, const XMMRegister& b0, const XMMRegister& t0)
{
  XMMRegister r = xmm0080().c();

  c->movdqa(t0, a0);
  c->pmullw(a0, b0);          // a0 *= b0
  c->paddusw(a0, r);          // a0 += 80
  c->paddusw(b0, t0);
  c->movdqa(t0, a0);          // t0  = a0
  c->psrlw(a0, 8);            // a0 /= 256
  c->paddusw(a0, t0);         // a0 += t0
  c->psrlw(a0, 8);            // a0 /= 256
  c->paddusw(a0, b0);
}

void Generator::_PackedMultiply_4(
  const XMMRegister& a0, const XMMRegister& b0, const XMMRegister& t0,
  const XMMRegister& a1, const XMMRegister& b1, const XMMRegister& t1,
  bool moveToT0T1)
{
  XMMRegister r = xmm0080().c();

  // Standard case
  if (t0 != t1)
  {
    c->pmullw(a0, b0);          // a0 *= b0
    c->pmullw(a1, b1);          // a1 *= b1

    c->paddusw(a0, r);          // a0 += 80
    c->paddusw(a1, r);          // a1 += 80

    c->movdqa(t0, a0);          // t0  = a0
    c->psrlw(a0, 8);            // a0 /= 256
    c->movdqa(t1, a1);          // t1  = a1
    c->psrlw(a1, 8);            // a1 /= 256

    if (moveToT0T1)
    {
      c->paddusw(t0, a0);       // t0 += a0
      c->psrlw(t0, 8);          // t0 /= 256

      c->paddusw(t1, a1);       // t1 += a1
      c->psrlw(t1, 8);          // t1 /= 256
    }
    else
    {
      c->paddusw(a0, t0);       // a0 += t0
      c->paddusw(a1, t1);       // a1 += t1

      c->psrlw(a0, 8);          // a0 /= 256
      c->psrlw(a1, 8);          // a1 /= 256
    }
  }
  // Special case if t0 is t1 (can be used to save one regiter if you
  // haven't it)
  else
  {
    const XMMRegister& t = t0;

    // Can't be moved to t0 and t1 if they are same!
    BLITJIT_ASSERT(moveToT0T1 == 0);

    c->pmullw(a0, b0);          // a0 *= b0
    c->pmullw(a1, b1);          // a1 *= b1
    c->paddusw(a0, r);          // a0 += 80
    c->paddusw(a1, r);          // a1 += 80

    c->movdqa(t, a0);           // t   = a0
    c->psrlw(a0, 8);            // a0 /= 256
    c->paddusw(a0, t);          // a0 += t

    c->movdqa(t, a1);           // t   = a1
    c->psrlw(a1, 8);            // a1 /= 256
    c->paddusw(a1, t);          // a1 += t

    c->psrlw(a0, 8);            // a0 /= 256
    c->psrlw(a1, 8);            // a1 /= 256
  }
}

void Generator::_PackedMultiplyAdd(
  const XMMRegister& a0, const XMMRegister& b0,
  const XMMRegister& c0, const XMMRegister& d0,
  const XMMRegister& t0, bool moveToT0)
{
  XMMRegister r = xmm0080().c();

  c->pmullw(a0, b0);          // a0 *= b0
  c->pmullw(c0, d0);          // c0 *= d0
  c->paddusw(a0, r);          // a0 += 80
  c->paddusw(a0, c0);         // a0 += c0

  c->movdqa(t0, a0);          // t0  = a0
  c->psrlw(a0, 8);            // a0 /= 256

  if (!moveToT0)
  {
    c->paddusw(a0, t0);       // a0 += t0
    c->psrlw(a0, 8);          // a0 /= 256
  }
  else
  {
    c->paddusw(t0, a0);       // t0 += a0
    c->psrlw(t0, 8);          // t0 /= 256
  }
}

void Generator::_PackedMultiplyAdd_4(
  const XMMRegister& a0, const XMMRegister& b0,
  const XMMRegister& c0, const XMMRegister& d0,
  const XMMRegister& t0,
  const XMMRegister& e0, const XMMRegister& f0,
  const XMMRegister& g0, const XMMRegister& h0,
  const XMMRegister& t1,
  bool moveToT0T1)
{
  XMMRegister r = xmm0080().c();

  c->pmullw(a0, b0);          // a0 *= b0
  c->pmullw(e0, f0);          // e0 *= f0

  c->paddusw(a0, r);          // a0 += 80
  c->pmullw(c0, d0);          // c0 *= d0

  c->paddusw(e0, r);          // e0 += 80
  c->pmullw(g0, h0);          // g0 *= h0

  c->paddusw(a0, c0);         // a0 += c0
  c->paddusw(e0, g0);         // e0 += g0

  c->movdqa(t0, a0);          // t0  = a0
  c->movdqa(t1, e0);          // t1  = e0

  c->psrlw(a0, 8);            // a0 /= 256
  c->psrlw(e0, 8);            // e0 /= 256

  if (!moveToT0T1)
  {
    c->paddusw(a0, t0);       // a0 += t0
    c->paddusw(e0, t1);       // e0 += t1
    c->psrlw(a0, 8);          // a0 /= 256
    c->psrlw(e0, 8);          // e0 /= 256
  }
  else
  {
    c->paddusw(t0, a0);       // t0 += a0
    c->paddusw(t1, e0);       // t1 += e0
    c->psrlw(t0, 8);          // t0 /= 256
    c->psrlw(t1, 8);          // t1 /= 256
  }
}

void Generator::_Premultiply_1(
  const XMMRef& pix0)
{
  int alphaPos0 = 3;
  XMMRef a0(c->newVariable(VARIABLE_TYPE_XMM));

  c->pshuflw(a0.r(), pix0.r(), mm_shuffle(alphaPos0, alphaPos0, alphaPos0, alphaPos0));
  c->pshufhw(a0.r(), a0.r()  , mm_shuffle(alphaPos0, alphaPos0, alphaPos0, alphaPos0));
  c->por(a0.r(), BLITJIT_GETCONST(this, Cx00FF00000000000000FF000000000000));

  _PackedMultiply(pix0.r(), a0.r(), a0.r());
}

} // BlitJit namespace
