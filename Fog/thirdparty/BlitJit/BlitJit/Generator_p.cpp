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

#include "BlitJit.h"
#include "Constants_p.h"
#include "Generator_p.h"
#include "Module_p.h"
#include "Module_Blit_p.h"
#include "Module_Fill_p.h"
#include "Module_MemCpy_p.h"
#include "Module_MemSet_p.h"

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
// [BlitJit::PremultiplyModule_32_SSE2]
// ============================================================================

// TODO: Move to separate file

struct PremultiplyModule_32_SSE2 : public Module_Filter
{
  PremultiplyModule_32_SSE2(
    Generator* g,
    const PixelFormat* pf);
  virtual ~PremultiplyModule_32_SSE2();

  virtual void init();
  virtual void free();

  virtual void processPixelsPtr(
    const AsmJit::PtrRef* dst,
    const AsmJit::PtrRef* src,
    const AsmJit::PtrRef* msk,
    SysInt count,
    SysInt offset,
    UInt32 kind,
    UInt32 flags);

  UInt32 dstAlphaPos;
};

PremultiplyModule_32_SSE2::PremultiplyModule_32_SSE2(
  Generator* g,
  const PixelFormat* pf) : Module_Filter(g, pf)
{
  _maxPixelsPerLoop = 4;
  _complexity = Complex;

  dstAlphaPos = getARGB32AlphaPos(pf);
}

PremultiplyModule_32_SSE2::~PremultiplyModule_32_SSE2()
{
}

void PremultiplyModule_32_SSE2::init()
{
  g->usingXMMZero();
}

void PremultiplyModule_32_SSE2::free()
{
}

void PremultiplyModule_32_SSE2::processPixelsPtr(
  const AsmJit::PtrRef* dst,
  const AsmJit::PtrRef* src,
  const AsmJit::PtrRef* msk,
  SysInt count,
  SysInt offset,
  UInt32 kind,
  UInt32 flags)
{
  // TODO: Verify
  SysUInt i = count;
  
  do {
    SysInt dstDisp = dstPf->bytesPerPixel() * offset;

    if (i >= 4 && c->numFreeXmm() >= 4)
    {
      XMMRef dst0(c->newVariable(VARIABLE_TYPE_XMM));
      XMMRef dst1(c->newVariable(VARIABLE_TYPE_XMM));

      c->movq(dst0.x(), ptr(dst->c(), dstDisp));
      c->movq(dst1.x(), ptr(dst->c(), dstDisp + 8));

      c->punpcklbw(dst0.r(), g->xmmZero().r());
      c->punpcklbw(dst1.r(), g->xmmZero().r());

      g->_Premultiply_4(
        dst0, dstAlphaPos, 
        dst1, dstAlphaPos);

      c->packuswb(dst0.r(), dst0.r());

      g->_StoreMovDQ(ptr(dst->c(), dstDisp), dst0.r(), false, (flags & DstAligned) != 0);

      offset += 4;
      i -= 4;
    }
    else if (i >= 2)
    {
      XMMRef dst0(c->newVariable(VARIABLE_TYPE_XMM));

      c->movq(dst0.x(), ptr(dst->c(), dstDisp));
      c->punpcklbw(dst0.r(), g->xmmZero().r());
      g->_Premultiply(dst0, dstAlphaPos, true);
      c->packuswb(dst0.r(), dst0.r());
      c->movq(ptr(dst->c(), dstDisp), dst0.r());

      offset += 2;
      i -= 2;
    }
    else if (i >= 1)
    {
      XMMRef dst0(c->newVariable(VARIABLE_TYPE_XMM));

      c->movd(dst0.x(), ptr(dst->c(), dstDisp));
      c->punpcklbw(dst0.r(), g->xmmZero().r());
      g->_Premultiply(dst0, dstAlphaPos, false);
      c->packuswb(dst0.r(), dst0.r());
      c->movd(ptr(dst->c(), dstDisp), dst0.r());

      offset++;
      i--;
    }
  } while (i > 0);
}

// ============================================================================
// [BlitJit::DemultiplyModule_32_SSE2]
// ============================================================================

// TODO: Move to separate file

struct DemultiplyModule_32_SSE2 : public Module_Filter
{
  DemultiplyModule_32_SSE2(
    Generator* g,
    const PixelFormat* pf);
  virtual ~DemultiplyModule_32_SSE2();

  virtual void init();
  virtual void free();

  virtual void processPixelsPtr(
    const AsmJit::PtrRef* dst,
    const AsmJit::PtrRef* src,
    const AsmJit::PtrRef* msk,
    SysInt count,
    SysInt offset,
    UInt32 kind,
    UInt32 flags);

  UInt32 dstAlphaPos;
};

DemultiplyModule_32_SSE2::DemultiplyModule_32_SSE2(
  Generator* g,
  const PixelFormat* pf) : Module_Filter(g, pf)
{
  _maxPixelsPerLoop = 1;
  _complexity = Complex;

  dstAlphaPos = getARGB32AlphaPos(pf);
}

DemultiplyModule_32_SSE2::~DemultiplyModule_32_SSE2()
{
}

void DemultiplyModule_32_SSE2::init()
{
  g->usingXMMZero();
}

void DemultiplyModule_32_SSE2::free()
{
}

void DemultiplyModule_32_SSE2::processPixelsPtr(
  const AsmJit::PtrRef* dst,
  const AsmJit::PtrRef* src,
  const AsmJit::PtrRef* msk,
  SysInt count,
  SysInt offset,
  UInt32 kind,
  UInt32 flags)
{
  // TODO: Verify
  UInt32 alphaMask = (0xFF << (dstAlphaPos * 8));
  SysUInt i = count;
  
  do {
    SysInt dstDisp = dstPf->bytesPerPixel() * offset;

    Label* skip = c->newLabel();

    XMMRef dst0(c->newVariable(VARIABLE_TYPE_XMM));
    XMMRef a0(c->newVariable(VARIABLE_TYPE_XMM));

    Int32Ref reg0(c->newVariable(VARIABLE_TYPE_INT32));
    Int32Ref reg1(c->newVariable(VARIABLE_TYPE_INT32));

    // Skip if alpha is 0x00 or 0xFF
    c->mov(reg0.x(), ptr(dst->c(), dstDisp));
    c->and_(reg0.r(), imm(alphaMask));
    c->jz(skip);
    c->cmp(reg0.r(), imm(alphaMask));
    c->je(skip);

    c->shr(reg0.r(), imm(dstAlphaPos * 8));
    c->mov(reg1.x(), BLITJIT_GETCONST(g, CxDemultiply[dstAlphaPos]));
    c->movd(dst0.x(), ptr(dst->c(), dstDisp));
    c->movq(a0.x(), ptr(reg1.r(), reg0.r(), TIMES_8));
    c->punpcklbw(dst0.x(), g->xmmZero().c());

    g->_PackedMultiply(dst0, a0, a0);
    c->packuswb(dst0.r(), dst0.r());
    c->movd(ptr(dst->c(), dstDisp), dst0.r());

    c->bind(skip);

    offset++;
    i--;
  } while (i > 0);
}

// ============================================================================
// [BlitJit::CreateModule]
// ============================================================================

static Module_Filter* createModule_Premultiply(
  Generator* g,
  const PixelFormat* pfDst)
{
  return new PremultiplyModule_32_SSE2(g, pfDst);
}

static Module_Filter* createModule_Demultiply(
  Generator* g,
  const PixelFormat* pfDst)
{
  return new DemultiplyModule_32_SSE2(g, pfDst);
}

static Module_Fill* createModule_Fill(
  Generator* g,
  const PixelFormat* dstPf,
  const PixelFormat* srcPf,
  const PixelFormat* mskPf,
  const Operator* op)
{
  BLITJIT_ASSERT(dstPf != NULL);
  BLITJIT_ASSERT(srcPf != NULL);

  if (op->id() == Operator::CompositeSrc && dstPf->id() == srcPf->id() && mskPf == NULL)
  {
    return new Module_MemSet32(g, dstPf, op);
  }
  else
  {
    return new Module_Fill_32_SSE2(g, dstPf, srcPf, mskPf, op);
  }
}

static Module_Blit* createModule_Blit(
  Generator* g,
  const PixelFormat* dstPf,
  const PixelFormat* srcPf,
  const PixelFormat* mskPf,
  const Operator* op)
{
  BLITJIT_ASSERT(dstPf != NULL);
  BLITJIT_ASSERT(srcPf != NULL);

  if (op->id() == Operator::CompositeSrc && mskPf == NULL)
  {
    return new Module_MemCpy32(g, dstPf, srcPf, op);
  }
  else
  {
    return new Module_Blit_32_SSE2(g, dstPf, srcPf, mskPf, op);
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
    return OptimizeSSE2;
  else if (features & CpuInfo::Feature_MMX)
    return OptimizeMMX;
  else
    return OptimizeX86;
#else
  // 64-bit mode: 64-bit processors are SSE2 capable by default.
  return OptimizeSSE2;
#endif
}

Generator::Generator(Compiler* _c) : GeneratorBase(_c)
{
  // Initialize cpu features we will use
  _features = cpuInfo()->features;

  // Detect maximum possible optimization
  _optimization = getOptimizationFromCpuFeatures(_features);

  // Configure calling convention
  _callingConvention = CALL_CONV_DEFAULT;

#if defined(BLITJIT_X86)
# if defined(BLITJIT_USE_FASTCALL)
#  if defined(__GNUC__)
  _callingConvention = CALL_CONV_GCCFASTCALL_3;
#  else
  _callingConvention = CALL_CONV_MSFASTCALL;
#  endif
# endif
# if defined(BLITJIT_USE_STDCALL)
  _callingConvention = CALL_CONV_STDCALL;
# endif
# if defined(BLITJIT_USE_CDECL)
  _callingConvention = CALL_CONV_CDECL;
# endif
#endif

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

void Generator::genPremultiply(const PixelFormat* dstPf)
{
  c->comment("BlitJit::Generator::genPremultiply() - %s", dstPf->name());

  f = c->newFunction(_callingConvention, BuildFunction2<void*, SysUInt>());
  f->setNaked(true);
  f->setAllocableEbp(true);

  PtrRef dst(c->argument(0));
  SysIntRef cnt(c->argument(1));

  Module_Filter* module = createModule_Premultiply(this, dstPf);

  Loop loop;
  loop.finalizePointers = false;

  module->init();
  _GenLoop(&dst, NULL, NULL, &cnt, module, 0, loop);
  module->free();

  delete module;
}

void Generator::genDemultiply(const PixelFormat* dstPf)
{
  c->comment("BlitJit::Generator::genDemultiply() - %s", dstPf->name());

  f = c->newFunction(_callingConvention, BuildFunction2<void*, SysUInt>());
  f->setNaked(true);
  f->setAllocableEbp(true);

  PtrRef dst(c->argument(0));
  SysIntRef cnt(c->argument(1));

  Module_Filter* module = createModule_Demultiply(this, dstPf);

  Loop loop;
  loop.finalizePointers = false;

  module->init();
  _GenLoop(&dst, NULL, NULL, &cnt, module, 0, loop);
  module->free();

  delete module;
}

// ============================================================================
// [BlitJit::Generator - Fill Span / Rect]
// ============================================================================

void Generator::genFillSpan(
  const PixelFormat* dstPf,
  const PixelFormat* srcPf,
  const Operator* op)
{
  c->comment("BlitJit::Generator::genFillSpan() - %s <- %s : %s",
    dstPf->name(), srcPf->name(), op->name());

  f = c->newFunction(_callingConvention, BuildFunction3<void*, const void*, SysUInt>());
  f->setNaked(true);
  f->setAllocableEbp(true);

  // Filter module
  Module_Fill* module = createModule_Fill(this, dstPf, srcPf, NULL, op);

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

    module->init(src);
    src.unuse();
    module->beginSwitch();

    for (UInt32 kind = 0; kind < module->numKinds(); kind++)
    {
      module->beginKind(kind);
      _GenLoop(&dst, &src, NULL, &cnt, module, kind, loop);
      module->endKind(kind);
    }

    module->endSwitch();
    module->free();
  }

  c->endFunction();

  // Cleanup
  delete module;
}

void Generator::genFillSpanWithMask(
  const PixelFormat* dstPf,
  const PixelFormat* srcPf,
  const PixelFormat* pfMask,
  const Operator* op)
{
  c->comment("BlitJit::Generator::genFillSpan() - %s <- %s * %s : %s",
    dstPf->name(), srcPf->name(), pfMask->name(), op->name());

  f = c->newFunction(_callingConvention, BuildFunction4<void*, const void*, const void*, SysUInt>());
  f->setNaked(true);
  f->setAllocableEbp(true);

  // Filter module
  Module_Fill* module = createModule_Fill(this, dstPf, srcPf, pfMask, op);

  if (!module->isNop())
  {
    // Destination and source
    PtrRef dst(c->argument(0));
    PtrRef src(c->argument(1));
    PtrRef msk(c->argument(2));
    SysIntRef cnt(c->argument(3));

    cnt.alloc();
    dst.alloc();
    src.alloc();
    msk.alloc();

    // Loop properties
    Loop loop;
    loop.finalizePointers = false;

    module->init(src);
    src.unuse();
    module->beginSwitch();

    for (UInt32 kind = 0; kind < module->numKinds(); kind++)
    {
      module->beginKind(kind);
      _GenLoop(&dst, &src, &msk, &cnt, module, kind, loop);
      module->endKind(kind);
    }

    module->endSwitch();
    module->free();
  }

  c->endFunction();

  // Cleanup
  delete module;
}

void Generator::genFillRect(
  const PixelFormat* dstPf,
  const PixelFormat* srcPf,
  const Operator* op)
{
  c->comment("BlitJit::Generator::genFillRect() - %s <- %s : %s",
    dstPf->name(), srcPf->name(), op->name());

  f = c->newFunction(_callingConvention, BuildFunction5<void*, const void*, SysInt, SysUInt, SysUInt>());
  f->setNaked(true);
  f->setAllocableEbp(true);

  // Filter module
  Module_Fill* module = createModule_Fill(this, dstPf, srcPf, NULL, op);

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

    module->init(src);
    src.unuse();
    module->beginSwitch();

    for (UInt32 kind = 0; kind < module->numKinds(); kind++)
    {
      module->beginKind(kind);

      Label* L_Loop = c->newLabel();
      c->bind(L_Loop);
      c->mov(cnt.r(), width);

      _GenLoop(&dst, &src, NULL, &cnt, module, kind, loop);

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

void Generator::genFillRectWithMask(
  const PixelFormat* dstPf,
  const PixelFormat* srcPf,
  const PixelFormat* pfMask,
  const Operator* op)
{
  // TODO
}

// ============================================================================
// [BlitJit::Generator - Blit Span / Rect]
// ============================================================================

void Generator::genBlitSpan(
  const PixelFormat* dstPf,
  const PixelFormat* srcPf,
  const Operator* op)
{
  c->comment("BlitJit::Generator::genBlitSpan() - %s <- %s : %s",
    dstPf->name(), srcPf->name(), op->name());

  f = c->newFunction(_callingConvention, BuildFunction3<void*, void*, SysUInt>());
  f->setNaked(true);
  f->setAllocableEbp(true);

  // Compositing module
  Module_Blit* module = createModule_Blit(this, srcPf, dstPf, NULL, op);

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
      _GenLoop(&dst, &src, NULL, &cnt, module, kind, loop);
      module->endKind(kind);
    }

    module->endSwitch();
    module->free();
  }

  c->endFunction();

  // Cleanup
  delete module;
}

void Generator::genBlitRect(
  const PixelFormat* dstPf,
  const PixelFormat* srcPf,
  const Operator* op)
{
  c->comment("BlitJit::Generator::genBlitRect() - %s <- %s : %s",
    dstPf->name(), srcPf->name(), op->name());

  f = c->newFunction(_callingConvention, BuildFunction6<void*, void*, SysInt, SysInt, SysUInt, SysUInt>());
  f->setNaked(true);
  f->setAllocableEbp(true);

  // Compositing module
  Module_Blit* module = createModule_Blit(this, srcPf, dstPf, NULL, op);

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
      _GenLoop(&dst, &src, NULL, &cnt, module, kind, loop);
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

void Generator::_GenLoop(
  PtrRef* dst,
  PtrRef* src,
  PtrRef* msk,
  SysIntRef* cnt,
  Module* module,
  UInt32 kind,
  const Loop& loop)
{
  // First calculate how the loop will be structured
  SysInt perLoop = module->maxPixelsPerLoop();
  SysInt dstSize = module->dstPf ? module->dstPf->bytesPerPixel() : 0;
  SysInt srcSize = module->srcPf ? module->srcPf->bytesPerPixel() : 0;
  SysInt mskSize = module->mskPf ? module->mskPf->bytesPerPixel() : 0;

  SysInt align = getAlignment(dstSize, perLoop);

  // Alloc variables if they are not allocated before.
  if (dst) dst->alloc();
  if (src) src->alloc();
  if (msk) msk->alloc();

  cnt->alloc();

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

  // We need one temporary variable for alignment
  if (align != 0) tmp.alloc();
  c->clearPrevented();

  // Skip alignment and main loop if cnt is too small (to be useful)
  if (perLoop > 1 && align)
  {
    c->cmp(cnt->r(), imm(perLoop));
    c->jb(L_TailLoop);
  }

  // Align 4 bytes dst to 8 bytes. We are using trick that if alignment
  // fail, we will continue into main loop, because this type of alignment
  // is usually used together with MMX or X64 modes (no SSE and unaligned
  // exceptions here)
  if (dstSize == 4 && align == 8)
  {
    c->mov(tmp.x(), dst->r());
    c->and_(tmp.r(), imm(7));
    c->jz(L_MainEntry);
    tmp.unuse();

    module->processPixelsPtr(dst, src, msk, 1, 0, kind, 0);
    if (dst) c->add(dst->r(), imm(dstSize));
    if (src) c->add(src->r(), imm(srcSize));
    if (msk) c->add(msk->r(), imm(mskSize));
    c->sub(cnt->r(), imm(1));
  }
  // Align 4 bytes dst to 16 bytes. This is hardest job, because we must
  // ensure that destination is aligned for instructions that needs it.
  // This is reason for last check and bail into tail loop if destination
  // is totally misaligned (this shouldn't happpen, but we must be sure).
  else if (dstSize == 4 && align == 16)
  {
    // Calculate how many pixels needs to be aligned
    c->xor_(tmp.x(), tmp.x());
    c->sub(tmp.r(), dst->r());
    c->and_(tmp.r(), imm(15));
    // Alignment not needed, jump to main loop entry
    c->jz(L_MainEntry);

    c->shr(tmp.r(), imm(2));
    c->sub(cnt->r(), tmp.r());

    Label* L_AlignLoop = c->newLabel();
    c->bind(L_AlignLoop);
    module->processPixelsPtr(dst, src, msk, 1, 0, kind, 0);
    if (dst) c->add(dst->r(), imm(dstSize));
    if (src) c->add(src->r(), imm(srcSize));
    if (msk) c->add(msk->r(), imm(mskSize));
    c->sub(tmp.r(), imm(1));
    c->jnz(L_AlignLoop);

    // Make sure that dst is aligned to 16 bytes
    c->mov(tmp.x(), dst->r());
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
    c->sub(cnt->r(), imm(perLoop));
    c->jc(L_TailEntry);

    c->align(_mainLoopAlignment);
    c->bind(L_MainLoop);

    if (src && _prefetch && module->prefetchSrc()) c->prefetch(ptr(src->r(), perLoop * srcSize), PREFETCH_T0);
    if (dst && _prefetch && module->prefetchDst()) c->prefetch(ptr(dst->r(), perLoop * dstSize), PREFETCH_T0);

    module->processPixelsPtr(dst, src, msk, perLoop, 0, kind, Module::DstAligned);
    if (dst) c->add(dst->r(), imm(perLoop * dstSize));
    if (src) c->add(src->r(), imm(perLoop * srcSize));
    if (msk) c->add(msk->r(), imm(perLoop * mskSize));
    c->sub(cnt->r(), imm(perLoop));
    c->jnc(L_MainLoop);
  }
  else
  {
    // One pixel at a time
    c->align(_mainLoopAlignment);
    c->bind(L_MainLoop);
    module->processPixelsPtr(dst, src, msk, 1, 0, kind, 0);
    if (dst) c->add(dst->r(), imm(dstSize));
    if (src) c->add(src->r(), imm(srcSize));
    if (msk) c->add(msk->r(), imm(mskSize));
    c->sub(cnt->r(), imm(1));
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
    c->sub(cnt->r(), imm(perLoop));
    c->jc(L_MisalignedTailEntry);

    c->align(_mainLoopAlignment);
    c->bind(L_MisalignedLoop);

    if (src && _prefetch && module->prefetchSrc()) c->prefetch(ptr(src->r(), perLoop * srcSize), PREFETCH_T0);
    if (dst && _prefetch && module->prefetchDst()) c->prefetch(ptr(dst->r(), perLoop * dstSize), PREFETCH_T0);

    module->processPixelsPtr(dst, src, msk, perLoop, 0, kind, 0);
    if (dst) c->add(dst->r(), imm(perLoop * dstSize));
    if (src) c->add(src->r(), imm(perLoop * srcSize));
    if (msk) c->add(msk->r(), imm(perLoop * mskSize));
    c->sub(cnt->r(), imm(perLoop));
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
      c->mov(tmp.x(), cnt->r());
      c->shr(tmp.r(), 2);
      c->jumpToTable(jumpTable, tmp.r());
      tmp.unuse();

      for (SysInt j = (perLoop / 4) - 1; j > 0; j--)
      {
        c->bind(jumpTable->addLabel(NULL, j));
        module->processPixelsPtr(dst, src, msk, 4, 0, kind, 0);
        if (dst) c->add(dst->r(), imm(dstSize * 4));
        if (src) c->add(src->r(), imm(srcSize * 4));
        if (msk) c->add(msk->r(), imm(mskSize * 4));
      }
      c->and_(cnt->r(), 3);
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
    c->add(cnt->r(), imm(2));
    c->jz(L_End);

    c->bind(L_TailLoop);
    module->processPixelsPtr(dst, src, msk, 1, 0, kind, 0);
    if (loop.finalizePointers)
    {
      if (dst) c->add(dst->r(), imm(dstSize));
      if (src) c->add(src->r(), imm(srcSize));
      if (msk) c->add(msk->r(), imm(mskSize));
    }
  }
  else if (module->complexity() == Module::Simple)
  {
    SysInt i = perLoop;

    // Jump table based loop for simple operators
    c->bind(L_TailEntry);
    c->add(cnt->r(), imm(perLoop));
    c->jz(L_End);

    c->bind(L_TailLoop);

    if (i > 8)
    {
      JumpTable* jumpTable = c->newJumpTable();
      jumpTable->addLabel(L_TailSkipLargeJumpTable, 0);

      tmp.use(c->newVariable(VARIABLE_TYPE_SYSINT));
      c->mov(tmp.x(), cnt->r());
      c->shr(tmp.r(), 2);
      c->jumpToTable(jumpTable, tmp.r());
      tmp.unuse();

      for (SysInt j = (i / 4) - 1; j > 0; j--, i -= 4)
      {
        c->bind(jumpTable->addLabel(NULL, j));
        module->processPixelsPtr(dst, src, msk, 4, 0, kind, Module::DstAligned);
        if (dst) c->add(dst->r(), imm(dstSize * 4));
        if (src) c->add(src->r(), imm(srcSize * 4));
        if (msk) c->add(msk->r(), imm(mskSize * 4));
      }
      c->and_(cnt->r(), 3);
      c->bind(L_TailSkipLargeJumpTable);
    }

    // Last few pixels will be processed through jump table
    {
      JumpTable* jumpTable = c->newJumpTable();
      jumpTable->addLabel(L_End, 0);

      // cnt is modified by jumpToTable().
      c->jumpToTable(jumpTable, cnt->r());

      for (i--; i > 0; i--)
      {
        c->bind(jumpTable->addLabel(NULL, i));
        module->processPixelsPtr(dst, src, msk, 1, 0, kind, 0);
        if (dst) c->add(dst->r(), imm(dstSize));
        if (src) c->add(src->r(), imm(srcSize));
        if (msk) c->add(msk->r(), imm(mskSize));
      }
    }
  }
  else
  {
    // Complex operators
    c->bind(L_TailEntry);
    c->add(cnt->r(), imm(perLoop));
    c->jz(L_End);

    c->bind(L_TailLoop);
    module->processPixelsPtr(dst, src, msk, 1, 0, kind, 0);
    if (dst) c->add(dst->r(), imm(dstSize));
    if (src) c->add(src->r(), imm(srcSize));
    if (msk) c->add(msk->r(), imm(mskSize));
    c->sub(cnt->r(), imm(1));
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
  return ptr_abs(Constants::instance, displacement);
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

void Generator::_CompositePixels(
  const XMMRef& dst0, const XMMRef& src0, int alphaPos0,
  const Operator* op,
  bool two)
{
  XMMRef t0(c->newVariable(VARIABLE_TYPE_XMM, 0));
  XMMRef t1(c->newVariable(VARIABLE_TYPE_XMM, 0));

  switch (op->id())
  {
    // Dca' = Sca
    // Da'  = Sa
    case Operator::CompositeSrc:
    {
      // copy operation (optimized in frontends and also by Generator itself)
      c->movdqa(dst0.r(), src0.r());
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
      _ExtractAlpha(t0, src0, alphaPos0, true, two);
      _PackedMultiply(dst0, t0, t0);
      c->paddusb(dst0.r(), src0.r());
      break;
    }

    // Dca' = Dca + Sca.(1 - Da)
    // Da'  = Da + Sa.(1 - Da)
    //      = Da + Sa - Da.Sa
    case Operator::CompositeOverReverse:
    {
      XMMRef t2(c->newVariable(VARIABLE_TYPE_XMM, 0));

      _ExtractAlpha(t0, dst0, alphaPos0, true, two);
      _PackedMultiply(t0, src0, t2);
      c->paddusb(dst0.r(), t0.r());
      break;
    }

    // Dca' = Sca.Da
    // Da'  = Sa.Da 
    case Operator::CompositeIn:
    {
      _ExtractAlpha(t0, dst0, alphaPos0, false, two);
      _PackedMultiply(t0, src0, dst0, true);
      break;
    }

    // Dca' = Dca.Sa
    // Da'  = Da.Sa
    case Operator::CompositeInReverse:
    {
      _ExtractAlpha(t0, src0, alphaPos0, false, two);
      _PackedMultiply(dst0, t0, t0);
      break;
    }

    // Dca' = Sca.(1 - Da)
    // Da'  = Sa.(1 - Da) 
    case Operator::CompositeOut:
    {
      _ExtractAlpha(t0, dst0, alphaPos0, true, two);
      _PackedMultiply(t0, src0, dst0, true);
      break;
    }

    // Dca' = Dca.(1 - Sa) 
    // Da'  = Da.(1 - Sa) 
    case Operator::CompositeOutReverse:
    {
      _ExtractAlpha(t0, src0, alphaPos0, true, two);
      _PackedMultiply(dst0, t0, t0);
      break;
    }

    // Dca' = Sca.Da + Dca.(1 - Sa)
    // Da'  = Sa.Da + Da.(1 - Sa)
    //      = Da.(Sa + 1 - Sa)
    //      = Da
    case Operator::CompositeAtop:
    {
      _ExtractAlpha(t0, src0, alphaPos0, true, two);
      _ExtractAlpha(t1, dst0, alphaPos0, false, two);
      _PackedMultiplyAdd(t1, src0, dst0, t0, dst0, true);
      break;
    }

    // Dca' = Dca.Sa + Sca.(1 - Da)
    // Da'  = Da.Sa + Sa.(1 - Da)
    //      = Sa.(Da + 1 - Da)
    //      = Sa 
    case Operator::CompositeAtopReverse:
    {
      _ExtractAlpha(t0, src0, alphaPos0, false, two);
      _ExtractAlpha(t1, dst0, alphaPos0, true, two);
      _PackedMultiplyAdd(t1, src0, dst0, t0, dst0, true);
      break;
    }

    // Dca' = Sca.(1 - Da) + Dca.(1 - Sa)
    // Da'  = Sa.(1 - Da) + Da.(1 - Sa)
    //      = Sa + Da - 2.Sa.Da 
    case Operator::CompositeXor:
    {
      _ExtractAlpha(t0, src0, alphaPos0, true, two);
      _ExtractAlpha(t1, dst0, alphaPos0, true, two);
      _PackedMultiplyAdd(
        t1, src0,
        dst0, t0,
        dst0, true);
      break;
    }

    // Dca' = 0
    // Da'  = 0
    case Operator::CompositeClear:
    {
      // clear operation (optimized in frontends and also by Generator itself)
      c->pxor(dst0.r(), dst0.r());
      break;
    }

    // Dca' = Sca + Dca
    // Da'  = Sa + Da 
    case Operator::CompositeAdd:
    {
      c->paddusb(dst0.r(), src0.r());
      break;
    }

    // Dca' = Dca - Sca
    // Da'  = 1 - (1 - Sa).(1 - Da)
    case Operator::CompositeSubtract:
    {
      _ExtractAlpha(t0, src0, alphaPos0, true, two);
      _ExtractAlpha(t1, dst0, alphaPos0, true, two);
      _PackedMultiply(t0, t1, t1);
      c->psubusb(dst0.r(), src0.r());
      c->por(dst0.r(), BLITJIT_GETCONST(this, Cx00FF00000000000000FF000000000000));
      c->pand(t0.r(), BLITJIT_GETCONST(this, Cx00FF00000000000000FF000000000000));
      c->psubusb(dst0.r(), t0.r());
      break;
    }

    // Dca' = Sca.Dca + Sca.(1 - Da) + Dca.(1 - Sa)
    // Da'  = Sa.Da + Sa.(1 - Da) + Da.(1 - Sa)
    //      = Sa + Da - Sa.Da 
    case Operator::CompositeMultiply:
    {
      XMMRef t2(c->newVariable(VARIABLE_TYPE_XMM, 0));

      c->movdqa(t2.r(), dst0.r());
      _PackedMultiply(t2, dst0, t0);

      _ExtractAlpha(t0, src0, alphaPos0, true, two); // t0 == 1-alpha from src0
      _ExtractAlpha(t1, dst0, alphaPos0, true, two); // t1 == 1-alpha from dst0

      _PackedMultiplyAdd(dst0, t0, t1, src0, t0);
      c->paddusw(dst0.r(), t2.r());
      break;
    }

    // Dca' = (Sca.Da + Dca.Sa - Sca.Dca) + Sca.(1 - Da) + Dca.(1 - Sa)
    //      = Sca + Dca - Sca.Dca
    // Da'  = Sa + Da - Sa.Da 
    case Operator::CompositeScreen:
    {
      c->movdqa(t0.r(), dst0.r());
      c->paddusw(dst0.r(), src0.r());
      _PackedMultiply(t0, src0, t1);
      c->psubusw(dst0.r(), t0.r());
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
      XMMRef t2(c->newVariable(VARIABLE_TYPE_XMM, 0));
      XMMRef t3(c->newVariable(VARIABLE_TYPE_XMM, 0));

      _ExtractAlpha_4(
        t0, src0, alphaPos0, false,
        t1, dst0, alphaPos0, false);
      _PackedMultiply_4(
        t0, dst0, t2,
        t1, src0, t3, true);

      if (op->id() == Operator::CompositeDarken)
        c->pminsw(t3.r(), t2.r());
      else
        c->pmaxsw(t3.r(), t2.r());

      _ExtractAlpha_4(
        t0, src0, alphaPos0, true,
        t1, dst0, alphaPos0, true);
      _PackedMultiplyAdd(
        dst0, t0,
        t1, src0, t2);
      c->paddusw(dst0.r(), t3.r());
      break;
    }

    // Dca' = Sca + Dca - 2.min(Sca.Da, Dca.Sa)
    // Da'  = Sa + Da - min(Sa.Da, Da.Sa)
    //      = Sa + Da - Sa.Da
    case Operator::CompositeDifference:
    {
      XMMRef t2(c->newVariable(VARIABLE_TYPE_XMM, 0));
      XMMRef t3(c->newVariable(VARIABLE_TYPE_XMM, 0));

      _ExtractAlpha_4(
        t0, src0, alphaPos0, false,
        t1, dst0, alphaPos0, false);
      _PackedMultiply_4(
        t0, dst0, t2,
        t1, src0, t3);
      c->pminsw(t0.r(), t1.r());
      c->paddusw(dst0.r(), src0.r());
      c->psubusw(dst0.r(), t0.r());
      c->pand(t0.r(), BLITJIT_GETCONST(this, Cx000000FF00FF00FF000000FF00FF00FF));
      c->psubusw(dst0.r(), t0.r());
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
      c->movdqa(t0.r(), src0.r());
      _PackedMultiply(t0, dst0, t1);
      c->paddusw(dst0.r(), src0.r());
      c->psubusw(dst0.r(), t0.r());
      c->pand(t0.r(), BLITJIT_GETCONST(this, Cx000000FF00FF00FF000000FF00FF00FF));
      c->psubusw(dst0.r(), t0.r());
      break;
    }

    // Dca' = (Da - Dca) * Sa + Dca.(1 - Sa)
    // Da'  = Sa + (Da - Da) * Sa + Da - Sa.Da
    //      = Sa + Da - Sa.Da
    case Operator::CompositeInvert:
    {
      XMMRef t2(c->newVariable(VARIABLE_TYPE_XMM, 0));
      XMMRef t3(c->newVariable(VARIABLE_TYPE_XMM, 0));

      _ExtractAlpha_4(
        t0, src0, alphaPos0, false,  // Sa
        t2, dst0, alphaPos0, false); // Da

      c->movdqa(t1.r(), t0.r());
      c->psubusb(t2.r(), dst0.r());
      c->pxor(t1.r(), BLITJIT_GETCONST(this, Cx00FF00FF00FF00FF00FF00FF00FF00FF));

      // t1 = 1 - Sa
      // t2 = Da - Dca

      _PackedMultiply_4(
        t2, t0, t3,     // t2   = Sa.(Da - Dca)
        dst0, t1, t1);  // dst0 = Dca.(1 - Sa)

      c->pand(t0.r(), BLITJIT_GETCONST(this, Cx00FF00000000000000FF000000000000));
      c->paddusb(dst0.r(), t2.r());
      c->paddusb(dst0.r(), t0.r());
      break;
    }

    // Dca' = (Da - Dca) * Sca + Dca.(1 - Sa)
    // Da'  = Sa + (Da - Da) * Sa + Da - Da.Sa
    //      = Sa + Da - Da.Sa
    case Operator::CompositeInvertRgb:
    {
      XMMRef t2(c->newVariable(VARIABLE_TYPE_XMM, 0));
      XMMRef t3(c->newVariable(VARIABLE_TYPE_XMM, 0));

      _ExtractAlpha_4(
        t0, src0, alphaPos0, false,  // Sa
        t2, dst0, alphaPos0, false); // Da

      c->movdqa(t1.r(), t0.r());
      c->psubw(t2.r(), dst0.r());
      c->pxor(t1.r(), BLITJIT_GETCONST(this, Cx00FF00FF00FF00FF00FF00FF00FF00FF));

      // t1 = 1 - Sa
      // t2 = Da - Dca

      _PackedMultiply_4(
        t2, src0, t3,   // t2   = Sca.(Da - Dca)
        dst0, t1, t1);  // dst0 = Dca.(1 - Sa)

      c->pand(t0.r(), BLITJIT_GETCONST(this, Cx00FF00000000000000FF000000000000));
      c->paddusb(dst0.r(), t2.r());
      c->paddusb(dst0.r(), t0.r());
      break;
    }
  }
}

void Generator::_CompositePixels_4(
  const XMMRef& dst0, const XMMRef& src0, int alphaPos0,
  const XMMRef& dst1, const XMMRef& src1, int alphaPos1,
  const Operator* op)
{
  XMMRef t0(c->newVariable(VARIABLE_TYPE_XMM, 0));
  XMMRef t1(c->newVariable(VARIABLE_TYPE_XMM, 0));

  switch (op->id())
  {
    case Operator::CompositeSrc:
      // copy operation (optimized in frontends and also by Generator itself)
      c->movdqa(dst0.r(), src0.r());
      c->movdqa(dst1.r(), src1.r());
      break;
    case Operator::CompositeDest:
      // no operation (optimized in frontends and also by Generator itself)
      break;
    case Operator::CompositeOver:
      _ExtractAlpha_4(
        t0, src0, alphaPos0, true,
        t1, src1, alphaPos1, true);
      _PackedMultiply_4(
        dst0, t0, t0,
        dst1, t1, t1);
      c->paddusb(dst0.r(), src0.r());
      c->paddusb(dst1.r(), src1.r());
      break;
    case Operator::CompositeOverReverse:
      // FIXME: Src modified
      _ExtractAlpha_4(
        t0, dst0, alphaPos0, true,
        t1, dst1, alphaPos1, true);
      _PackedMultiply_4(
        src0, t0, t0,
        src1, t1, t1);
      c->paddusb(dst0.r(), src0.r());
      c->paddusb(dst1.r(), src1.r());
      break;
    case Operator::CompositeIn:
      _ExtractAlpha_4(
        t0, dst0, alphaPos0, false,
        t1, dst1, alphaPos1, false);
      _PackedMultiply_4(
        t0, src0, dst0,
        t1, src1, dst1,
        true);
      break;
    case Operator::CompositeInReverse:
      _ExtractAlpha_4(
        t0, src0, alphaPos0, false,
        t1, src1, alphaPos1, false);
      _PackedMultiply_4(
        dst0, t0, t0,
        dst1, t1, t1);
      break;
    case Operator::CompositeOut:
      _ExtractAlpha_4(
        t0, dst0, alphaPos0, true,
        t1, dst1, alphaPos1, true);
      _PackedMultiply_4(
        t0, src0, dst0,
        t1, src1, dst1,
        true);
      break;
    case Operator::CompositeOutReverse:
      _ExtractAlpha_4(
        t0, src0, alphaPos0, true,
        t1, src1, alphaPos1, true);
      _PackedMultiply_4(
        dst0, t0, t0,
        dst1, t1, t1);
      break;
    case Operator::CompositeAtop:
      _ExtractAlpha_4(
        t0, src0, alphaPos0, true,
        t1, dst0, alphaPos1, false);
      _PackedMultiplyAdd(t1, src0, dst0, t0, dst0, true);

      _ExtractAlpha_4(
        t0, src1, alphaPos0, true,
        t1, dst1, alphaPos1, false);
      _PackedMultiplyAdd(t1, src1, dst1, t0, dst1, true);
      break;
    case Operator::CompositeAtopReverse:
      _ExtractAlpha_4(
        t0, src0, alphaPos0, false,
        t1, dst0, alphaPos1, true);
      _PackedMultiplyAdd(t1, src0, dst0, t0, dst0, true);

      _ExtractAlpha_4(
        t0, src1, alphaPos0, false,
        t1, dst1, alphaPos1, true);
      _PackedMultiplyAdd(t1, src1, dst1, t0, dst1, true);
      break;
    case Operator::CompositeXor:
      _ExtractAlpha_4(
        t0, src0, alphaPos0, true,
        t1, dst0, alphaPos1, true);
      _PackedMultiplyAdd(t1, src0, dst0, t0, dst0, true);

      _ExtractAlpha_4(
        t0, src1, alphaPos0, true,
        t1, dst1, alphaPos1, true);
      _PackedMultiplyAdd(t1, src1, dst1, t0, dst1, true);
      break;
    case Operator::CompositeClear:
      // clear operation (optimized in frontends and also by Generator itself)
      c->pxor(dst0.r(), dst0.r());
      c->pxor(dst1.r(), dst1.r());
      break;
    case Operator::CompositeAdd:
      c->paddusb(dst0.r(), src0.r());
      c->paddusb(dst1.r(), src1.r());
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

void Generator::_ExtractAlpha(
  const XMMRef& dst0, const XMMRef& src0, UInt8 alphaPos0, UInt8 negate0, bool two)
{
  c->pshuflw(dst0.r(), src0.r(), mm_shuffle(alphaPos0, alphaPos0, alphaPos0, alphaPos0));
  if (two) c->pshufhw(dst0.r(), dst0.r(), mm_shuffle(alphaPos0, alphaPos0, alphaPos0, alphaPos0));

  if (negate0) c->pxor(dst0.r(), BLITJIT_GETCONST(this, Cx00FF00FF00FF00FF00FF00FF00FF00FF));
}

void Generator::_ExtractAlpha_4(
  const XMMRef& dst0, const XMMRef& src0, UInt8 alphaPos0, UInt8 negate0,
  const XMMRef& dst1, const XMMRef& src1, UInt8 alphaPos1, UInt8 negate1)
{
  c->pshuflw(dst0.r(), src0.r(), mm_shuffle(alphaPos0, alphaPos0, alphaPos0, alphaPos0));
  c->pshuflw(dst1.r(), src1.r(), mm_shuffle(alphaPos1, alphaPos1, alphaPos1, alphaPos1));

  c->pshufhw(dst0.r(), dst0.r(), mm_shuffle(alphaPos0, alphaPos0, alphaPos0, alphaPos0));
  c->pshufhw(dst1.r(), dst1.r(), mm_shuffle(alphaPos1, alphaPos1, alphaPos1, alphaPos1));

  if (negate0) c->pxor(dst0.r(), BLITJIT_GETCONST(this, Cx00FF00FF00FF00FF00FF00FF00FF00FF));
  if (negate1) c->pxor(dst1.r(), BLITJIT_GETCONST(this, Cx00FF00FF00FF00FF00FF00FF00FF00FF));
}

void Generator::_PackedMultiply(
  const XMMRef& a0, const XMMRef& b0, const XMMRef& t0,
  bool moveToT0)
{
  c->pmullw(a0.r(), b0.r());            // a0 *= b0
  c->paddusw(a0.r(), xmm0080().c());    // a0 += 80
  c->movdqa(t0.r(), a0.r());            // t0  = a0
  c->psrlw(a0.r(), 8);                  // a0 /= 256

  if (!moveToT0)
  {
    c->paddusw(a0.r(), t0.r());         // a0 += t0
    c->psrlw(a0.r(), 8);                // a0 /= 256
  }
  else
  {
    c->paddusw(t0.r(), a0.r());         // t0 += a0
    c->psrlw(t0.r(), 8);                // t0 /= 256
  }
}

void Generator::_PackedMultiplyWithAddition(
  const XMMRef& a0, const XMMRef& b0, const XMMRef& t0)
{
  c->movdqa(t0.r(), a0.r());            // t0  = a0
  c->pmullw(a0.r(), b0.r());            // a0 *= b0
  c->paddusw(a0.r(), xmm0080().c());    // a0 += 80
  c->paddusw(b0.r(), t0.r());           // b0 += a0
  c->movdqa(t0.r(), a0.r());            // t0  = a0
  c->psrlw(a0.r(), 8);                  // a0 /= 256
  c->paddusw(a0.r(), t0.r());           // a0 += t0
  c->psrlw(a0.r(), 8);                  // a0 /= 256
  c->paddusw(a0.r(), b0.r());           // a0 += b0
}

void Generator::_PackedMultiply_4(
  const XMMRef& a0, const XMMRef& b0, const XMMRef& t0,
  const XMMRef& a1, const XMMRef& b1, const XMMRef& t1,
  bool moveToT0T1)
{
  // Standard case
  if (t0 != t1)
  {
    c->pmullw(a0.r(), b0.r());          // a0 *= b0
    c->pmullw(a1.r(), b1.r());          // a1 *= b1

    c->paddusw(a0.r(), xmm0080().c());  // a0 += 80
    c->paddusw(a1.r(), xmm0080().c());  // a1 += 80

    c->movdqa(t0.r(), a0.r());          // t0  = a0
    c->psrlw(a0.r(), 8);                // a0 /= 256
    c->movdqa(t1.r(), a1.r());          // t1  = a1
    c->psrlw(a1.r(), 8);                // a1 /= 256

    if (moveToT0T1)
    {
      c->paddusw(t0.r(), a0.r());       // t0 += a0
      c->psrlw(t0.r(), 8);              // t0 /= 256

      c->paddusw(t1.r(), a1.r());       // t1 += a1
      c->psrlw(t1.r(), 8);              // t1 /= 256
    }
    else
    {
      c->paddusw(a0.r(), t0.r());       // a0 += t0
      c->paddusw(a1.r(), t1.r());       // a1 += t1

      c->psrlw(a0.r(), 8);              // a0 /= 256
      c->psrlw(a1.r(), 8);              // a1 /= 256
    }
  }
  // Special case if t0 is t1 (can be used to save one regiter if you
  // haven't it)
  else
  {
    // Can't be moved to t0 and t1 if they are same!
    BLITJIT_ASSERT(moveToT0T1 == 0);

    c->pmullw(a0.r(), b0.r());          // a0 *= b0
    c->pmullw(a1.r(), b1.r());          // a1 *= b1
    c->paddusw(a0.r(), xmm0080().c());  // a0 += 80
    c->paddusw(a1.r(), xmm0080().c());  // a1 += 80

    c->movdqa(t0.r(), a0.r());          // t   = a0
    c->psrlw(a0.r(), 8);                // a0 /= 256
    c->paddusw(a0.r(), t0.r());         // a0 += t

    c->movdqa(t0.r(), a1.r());          // t   = a1
    c->psrlw(a1.r(), 8);                // a1 /= 256
    c->paddusw(a1.r(), t0.r());         // a1 += t

    c->psrlw(a0.r(), 8);                // a0 /= 256
    c->psrlw(a1.r(), 8);                // a1 /= 256
  }
}

void Generator::_PackedMultiplyAdd(
  const XMMRef& a0, const XMMRef& b0,
  const XMMRef& c0, const XMMRef& d0,
  const XMMRef& t0, bool moveToT0)
{
  c->pmullw(a0.r(), b0.r());            // a0 *= b0
  c->pmullw(c0.r(), d0.r());            // c0 *= d0
  c->paddusw(a0.r(), xmm0080().c());    // a0 += 80
  c->paddusw(a0.r(), c0.r());           // a0 += c0

  c->movdqa(t0.r(), a0.r());            // t0  = a0
  c->psrlw(a0.r(), 8);                  // a0 /= 256

  if (!moveToT0)
  {
    c->paddusw(a0.r(), t0.r());         // a0 += t0
    c->psrlw(a0.r(), 8);                // a0 /= 256
  }
  else
  {
    c->paddusw(t0.r(), a0.r());         // t0 += a0
    c->psrlw(t0.r(), 8);                // t0 /= 256
  }
}

void Generator::_PackedMultiplyAdd_4(
  const XMMRef& a0, const XMMRef& b0,
  const XMMRef& c0, const XMMRef& d0,
  const XMMRef& t0,
  const XMMRef& e0, const XMMRef& f0,
  const XMMRef& g0, const XMMRef& h0,
  const XMMRef& t1,
  bool moveToT0T1)
{
  c->pmullw(a0.r(), b0.r());            // a0 *= b0
  c->pmullw(e0.r(), f0.r());            // e0 *= f0

  c->paddusw(a0.r(), xmm0080().c());    // a0 += 80
  c->pmullw(c0.r(), d0.r());            // c0 *= d0

  c->paddusw(e0.r(), xmm0080().c());    // e0 += 80
  c->pmullw(g0.r(), h0.r());            // g0 *= h0

  c->paddusw(a0.r(), c0.r());           // a0 += c0
  c->paddusw(e0.r(), g0.r());           // e0 += g0

  c->movdqa(t0.r(), a0.r());            // t0  = a0
  c->movdqa(t1.r(), e0.r());            // t1  = e0

  c->psrlw(a0.r(), 8);                  // a0 /= 256
  c->psrlw(e0.r(), 8);                  // e0 /= 256

  if (!moveToT0T1)
  {
    c->paddusw(a0.r(), t0.r());         // a0 += t0
    c->paddusw(e0.r(), t1.r());         // e0 += t1
    c->psrlw(a0.r(), 8);                // a0 /= 256
    c->psrlw(e0.r(), 8);                // e0 /= 256
  }
  else
  {
    c->paddusw(t0.r(), a0.r());         // t0 += a0
    c->paddusw(t1.r(), e0.r());         // t1 += e0
    c->psrlw(t0.r(), 8);                // t0 /= 256
    c->psrlw(t1.r(), 8);                // t1 /= 256
  }
}

void Generator::_Premultiply(
  const XMMRef& pix0, int alphaPos0, bool two)
{
  XMMRef a0(c->newVariable(VARIABLE_TYPE_XMM));

  c->pshuflw(a0.r(), pix0.r(), mm_shuffle(alphaPos0, alphaPos0, alphaPos0, alphaPos0));
  c->pshufhw(a0.r(), a0.r()  , mm_shuffle(alphaPos0, alphaPos0, alphaPos0, alphaPos0));
  c->por(a0.r(), BLITJIT_GETCONST_WITH_DISPLACEMENT(this, Cx00000000000000FF00000000000000FF, alphaPos0 * 16));

  _PackedMultiply(pix0, a0, a0);
}

void Generator::_Premultiply_4(
  const XMMRef& pix0, int alphaPos0,
  const XMMRef& pix1, int alphaPos1)
{
  XMMRef a0(c->newVariable(VARIABLE_TYPE_XMM));
  XMMRef a1(c->newVariable(VARIABLE_TYPE_XMM));

  c->pshuflw(a0.r(), pix0.r(), mm_shuffle(alphaPos0, alphaPos0, alphaPos0, alphaPos0));
  c->pshuflw(a1.r(), pix1.r(), mm_shuffle(alphaPos1, alphaPos1, alphaPos1, alphaPos1));
  c->pshufhw(a0.r(), a0.r()  , mm_shuffle(alphaPos0, alphaPos0, alphaPos0, alphaPos0));
  c->pshufhw(a1.r(), a1.r()  , mm_shuffle(alphaPos1, alphaPos1, alphaPos1, alphaPos1));
  c->por(a0.r(), BLITJIT_GETCONST_WITH_DISPLACEMENT(this, Cx00000000000000FF00000000000000FF, alphaPos0 * 16));
  c->por(a1.r(), BLITJIT_GETCONST_WITH_DISPLACEMENT(this, Cx00000000000000FF00000000000000FF, alphaPos1 * 16));

  _PackedMultiply_4(
    pix0, a0, a0,
    pix1, a1, a1);
}

void Generator::_Demultiply(
  const XMMRef& pix0, Int32Ref& val0, int alphaPos0)
{
  // TODO: Verify
  Label* skip = c->newLabel();

  // Alpha mask.
  UInt32 alphaMask = (0xFF << (alphaPos0 * 8));

  // Variables.
  XMMRef a0(c->newVariable(VARIABLE_TYPE_XMM));
  Int32Ref base0(c->newVariable(VARIABLE_TYPE_INT32));

  // Skip if alpha is 0x00 or 0xFF.
  c->and_(val0.r(), imm(alphaMask));
  c->jz(skip);
  c->cmp(val0.r(), imm(alphaMask));
  c->je(skip);

  // Demultiply using multiplication - fast.
  c->shr(val0.r(), imm(alphaPos0 * 8));
  c->mov(base0.x(), BLITJIT_GETCONST(this, CxDemultiply[alphaPos0]));
  c->movq(a0.x(), ptr(base0.r(), val0.r(), TIMES_8));

  _PackedMultiply(pix0, a0, a0);

  // Here is label if pixel not needs to be demultiplied.
  c->bind(skip);
}

} // BlitJit namespace
