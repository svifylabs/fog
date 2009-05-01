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
#include "Module_Fill_p.h"

using namespace AsmJit;

namespace BlitJit {

// ============================================================================
// [BlitJit::Module_Fill_32_SSE2]
// ============================================================================

Module_Fill_32_SSE2::Module_Fill_32_SSE2(
  Generator* g,
  const PixelFormat* dstPf,
  const PixelFormat* srcPf,
  const PixelFormat* mskPf,
  const Operator* op) :
    Module_Fill(g, dstPf, srcPf, mskPf, op)
{
  // Calculate correct pixel format positions
  dstAlphaPos = getARGB32AlphaPos(dstPf);

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

Module_Fill_32_SSE2::~Module_Fill_32_SSE2()
{
}

void Module_Fill_32_SSE2::beginSwitch()
{
  // Expand pixel
  c->movd(srcxmm.x(), srcgp.c32());
  c->pshufd(srcxmm.r(), srcxmm.r(), mm_shuffle(0, 0, 0, 0));

  if (mskPf)
  {
    c->punpcklbw(srcxmm.r(), g->xmmZero().r());
    //c->punpcklqdq(srcxmm.r(), srcxmm.r());
    g->_Premultiply(srcxmm, dstAlphaPos, true);
  }

  // Check if alpha is 0xFF (255) 
  c->cmp(srcgp.r32(), imm(0xFF000000));
  c->jae(getKindLabel(1));

  // Alpha is not 0xFF, premultiply and prepare for compositing, but only if 
  // there is no mask. If there is mask, we must first multiply src with mask
  // and this kind of optimization is not applicable.
  if (!mskPf)
  {
    c->punpcklbw(srcxmm.r(), g->xmmZero().r());
    //c->punpcklqdq(srcxmm.r(), srcxmm.r());
    g->_Premultiply(srcxmm, dstAlphaPos, true);
    g->_ExtractAlpha(alphaxmm, srcxmm, dstAlphaPos, false, true);
  }
}

void Module_Fill_32_SSE2::init(PtrRef& _src)
{
  g->usingConstants();
  g->usingXMMZero();
  g->usingXMM0080();

  srcgp.use(c->newVariable(VARIABLE_TYPE_SYSINT));
  srcxmm.use(c->newVariable(VARIABLE_TYPE_XMM));

  // Only used as fastpath for CompositeOver operator.
  if (mskPf == NULL && op->id() == Operator::CompositeOver)
  {
    alphaxmm.use(c->newVariable(VARIABLE_TYPE_XMM));
  }

  c->mov(srcgp.x32(), ptr(_src.c()));
}

void Module_Fill_32_SSE2::free()
{
  srcgp.unuse();
  srcxmm.unuse();
  alphaxmm.unuse();
}

void Module_Fill_32_SSE2::processPixelsPtr(
  const AsmJit::PtrRef* dst,
  const AsmJit::PtrRef* src,
  const AsmJit::PtrRef* msk,
  SysInt count,
  SysInt offset,
  UInt32 kind,
  UInt32 flags)
{
  StateRef state(c->saveState());

  BLITJIT_ASSERT(dst != NULL);
  BLITJIT_ASSERT(src != NULL);

  SysInt i = count;
  bool dstAligned = (flags & DstAligned) != 0;

  // --------------------------------------------
  // [DST + SRC]
  // --------------------------------------------

  if (msk == NULL)
  {
    // Default
    if (kind == 0)
    {
      XMMRef dst0(c->newVariable(VARIABLE_TYPE_XMM));

      do {
        SysInt dstDisp = dstPf->bytesPerPixel() * offset;

        if (i >= 4)
        {
          XMMRef dst1(c->newVariable(VARIABLE_TYPE_XMM));

          c->movq(dst0.x(), ptr(dst->c(), dstDisp));
          c->movq(dst1.x(), ptr(dst->c(), dstDisp + 8));
          processPixelsRaw_4(dst0, dst1, 0);
          g->_StoreMovDQ(ptr(dst->c(), dstDisp), dst0.r(), false, dstAligned);

          offset += 4;
          i -= 4;
        }
        else if (i >= 2)
        {
          c->movq(dst0.x(), ptr(dst->c(), dstDisp));
          processPixelsRaw(dst0, true);
          c->movq(ptr(dst->c(), dstDisp), dst0.r());

          offset += 2;
          i -= 2;
        }
        else if (i >= 1)
        {
          c->movd(dst0.x(), ptr(dst->c(), dstDisp));
          processPixelsRaw(dst0, false);
          c->movd(ptr(dst->c(), dstDisp), dst0.r());

          offset++;
          i--;
        }
      } while (i > 0);
    }
    // Optimized special case - Alpha == 0xFF
    else
    {
      do {
        SysInt dstDisp = dstPf->bytesPerPixel() * offset;

        if (i >= 4)
        {
          g->_StoreMovDQ(ptr(dst->c(), dstDisp), srcxmm.r(), false, dstAligned);

          offset -= 4;
          i -= 4;
        }
        else if (i >= 1)
        {
          c->mov(ptr(dst->c(), dstDisp), srcgp.r32());

          offset++;
          i--;
        }
      } while (i > 0);
    }
  }

  // --------------------------------------------
  // [DST + SRC * MASK]
  // --------------------------------------------

  else
  {
    // Default
    if (kind == 0)
    {
      XMMRef dst0(c->newVariable(VARIABLE_TYPE_XMM));
      XMMRef msk0(c->newVariable(VARIABLE_TYPE_XMM));

      do {
        SysInt dstDisp = dstPf->bytesPerPixel() * offset;
        SysInt mskDisp = mskPf->bytesPerPixel() * offset;

        if (i >= 4)
        {
          XMMRef dst1(c->newVariable(VARIABLE_TYPE_XMM));
          Int32Ref mskVal(c->newVariable(VARIABLE_TYPE_INT32));

          Label* end = c->newLabel();

          c->mov(mskVal.x(), ptr(msk->r(), mskDisp));
          c->test(mskVal.x(), mskVal.x());
          c->jz(end);
          c->movd(msk0.x(), mskVal.c());

          c->movq(dst0.x(), ptr(dst->c(), dstDisp));
          c->movq(dst1.x(), ptr(dst->c(), dstDisp + 8));
          processPixelsRawMask_4(dst0, dst1, msk0, 0);
          g->_StoreMovDQ(ptr(dst->c(), dstDisp), dst0.r(), false, dstAligned);

          c->bind(end);

          offset += 4;
          i -= 4;
        }
        else if (i >= 2)
        {
          Int32Ref mskVal(c->newVariable(VARIABLE_TYPE_INT32));

          Label* end = c->newLabel();

          c->movzx(mskVal.x(), word_ptr(msk->r(), mskDisp));
          c->test(mskVal.r(), mskVal.r());
          c->jz(end);
          c->movd(msk0.x(), mskVal.c());

          c->movq(dst0.x(), ptr(dst->c(), dstDisp));
          processPixelsRawMask(dst0, msk0, true);
          c->movq(ptr(dst->c(), dstDisp), dst0.r());

          c->bind(end);

          offset += 2;
          i -= 2;
        }
        else if (i >= 1)
        {
          Int32Ref mskVal(c->newVariable(VARIABLE_TYPE_INT32));

          Label* end = c->newLabel();

          c->movzx(mskVal.x(), byte_ptr(msk->r(), mskDisp));
          c->test(mskVal.r(), mskVal.r());
          c->jz(end);
          c->movd(msk0.x(), mskVal.c());

          c->movd(dst0.x(), ptr(dst->c(), dstDisp));
          processPixelsRawMask(dst0, msk0, false);
          c->movd(ptr(dst->c(), dstDisp), dst0.r());

          c->bind(end);

          offset++;
          i--;
        }
      } while (i > 0);
    }
    // Optimized special case - Alpha == 0xFF
    else
    {
      XMMRef dst0(c->newVariable(VARIABLE_TYPE_XMM));
      XMMRef msk0(c->newVariable(VARIABLE_TYPE_XMM));

      do {
        SysInt dstDisp = dstPf->bytesPerPixel() * offset;
        SysInt mskDisp = mskPf->bytesPerPixel() * offset;

        if (i >= 4)
        {
          XMMRef dst1(c->newVariable(VARIABLE_TYPE_XMM));
          Int32Ref mskVal(c->newVariable(VARIABLE_TYPE_INT32));

          Label* end = c->newLabel();

          c->mov(mskVal.x(), ptr(msk->r(), mskDisp));
          c->test(mskVal.x(), mskVal.x());
          c->jz(end);

          c->movd(msk0.x(), mskVal.c());

          c->movq(dst0.x(), ptr(dst->c(), dstDisp));
          c->movq(dst1.x(), ptr(dst->c(), dstDisp + 8));
          processPixelsRawMask_4(dst0, dst1, msk0, 0);
          g->_StoreMovDQ(ptr(dst->c(), dstDisp), dst0.r(), false, dstAligned);

          c->bind(end);

          offset -= 4;
          i -= 4;
        }
        else if (i >= 1)
        {
          Int32Ref mskVal(c->newVariable(VARIABLE_TYPE_INT32));

          Label* end = c->newLabel();

          c->movzx(mskVal.x(), byte_ptr(msk->r(), mskDisp));
          c->test(mskVal.r(), mskVal.r());
          c->jz(end);
          c->movd(msk0.x(), mskVal.c());

          c->movd(dst0.x(), ptr(dst->c(), dstDisp));
          processPixelsRawMask(dst0, msk0, false);
          c->movd(ptr(dst->c(), dstDisp), dst0.r());

          c->bind(end);

          offset++;
          i--;
        }
      } while (i > 0);
    }

  }
}

void Module_Fill_32_SSE2::processPixelsRaw(
  XMMRef& dst0,
  bool two)
{
  c->punpcklbw(dst0.r(), g->xmmZero().r());

  processPixelsUnpacked(dst0, two);

  c->packuswb(dst0.r(), dst0.r());
}

void Module_Fill_32_SSE2::processPixelsRaw_4(
  XMMRef& dst0,
  XMMRef& dst1,
  UInt32 flags)
{
  c->punpcklbw(dst0.r(), g->xmmZero().r());
  c->punpcklbw(dst1.r(), g->xmmZero().r());

  processPixelsUnpacked_4(dst0, dst1);

  c->packuswb(dst0.r(), dst1.r());
}

void Module_Fill_32_SSE2::processPixelsRawMask(
  AsmJit::XMMRef& dst0,
  AsmJit::XMMRef& msk0,
  bool two)
{
  c->punpcklbw(dst0.r(), g->xmmZero().r());

  if (two)
  {
    c->punpcklbw(msk0.r(), g->xmmZero().r());
    c->punpcklwd(msk0.r(), msk0.r());
    c->punpckldq(msk0.r(), msk0.r());
  }
  else
  {
    c->pshuflw(msk0.r(), msk0.r(), imm(mm_shuffle(0, 0, 0, 0)));
  }

  { XMMRef v0(c->newVariable(VARIABLE_TYPE_XMM));
    g->_PackedMultiply(msk0, srcxmm, v0, false); }

  g->_CompositePixels(dst0, msk0, dstAlphaPos, op, two);
  c->packuswb(dst0.r(), dst0.r());
}

void Module_Fill_32_SSE2::processPixelsRawMask_4(
  AsmJit::XMMRef& dst0,
  AsmJit::XMMRef& dst1,
  AsmJit::XMMRef& msk0,
  UInt32 flags)
{
  XMMRef msk1(c->newVariable(VARIABLE_TYPE_XMM));

  c->punpcklbw(dst0.r(), g->xmmZero().r());
  c->punpcklbw(dst1.r(), g->xmmZero().r());

  c->punpcklbw(msk0.r(), g->xmmZero().r());                    // [  ][  ][  ][  ][ 3][ 2][ 1][ 0]
  c->pshufd(msk0.r(), msk0.r(), imm(mm_shuffle(1, 0, 1, 0)));  // [ 3][ 2][ 1][ 0][ 3][ 2][ 1][ 0]

  c->pshufhw(msk1.r(), msk0.r(), imm(mm_shuffle(3, 3, 3, 3)));
  c->pshuflw(msk1.r(), msk1.r(), imm(mm_shuffle(2, 2, 2, 2)));
  c->pshufhw(msk0.r(), msk0.r(), imm(mm_shuffle(1, 1, 1, 1)));
  c->pshuflw(msk0.r(), msk0.r(), imm(mm_shuffle(0, 0, 0, 0)));

  {
    XMMRef v0(c->newVariable(VARIABLE_TYPE_XMM));
    g->_PackedMultiply_4(
      msk0, srcxmm, v0,
      msk1, srcxmm, v0);
  }

  g->_CompositePixels_4(
    dst0, msk0, dstAlphaPos,
    dst1, msk1, dstAlphaPos,
    op);

  c->packuswb(dst0.r(), dst1.r());
}

void Module_Fill_32_SSE2::processPixelsUnpacked(
  XMMRef& dst0,
  bool two)
{
  if (op->id() == Operator::CompositeOver)
  {
    XMMRef t0(c->newVariable(VARIABLE_TYPE_XMM));

    g->_PackedMultiply(dst0, alphaxmm, t0);
    c->paddusb(dst0.r(), srcxmm.r());
  }
  else
  {
    g->_CompositePixels(dst0, srcxmm, dstAlphaPos, op, two);
  }
}

void Module_Fill_32_SSE2::processPixelsUnpacked_4(
  XMMRef& dst0,
  XMMRef& dst1)
{
  if (op->id() == Operator::CompositeOver)
  {
    XMMRef t0(c->newVariable(VARIABLE_TYPE_XMM));
    XMMRef t1(c->newVariable(VARIABLE_TYPE_XMM));

    g->_PackedMultiply_4(
      dst0, alphaxmm, t0,
      dst1, alphaxmm, t1,
      0);
    c->paddusb(dst0.r(), srcxmm.r());
    c->paddusb(dst1.r(), srcxmm.r());
  }
  else
  {
    g->_CompositePixels_4(
      dst0, srcxmm, dstAlphaPos,
      dst1, srcxmm, dstAlphaPos,
      op);
  }
}

} // BlitJit namespace
