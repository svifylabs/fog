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
#include "Generator_p.h"
#include "Module_p.h"

namespace BlitJit {

// ============================================================================
// [BlitJit::Module]
// ============================================================================

Module::Module(Generator* g, 
  const PixelFormat* dstPf,
  const PixelFormat* srcPf,
  const PixelFormat* mskPf,
  const Operator* op) :
    g(g),
    c(g->c),
    dstPf(dstPf),
    srcPf(srcPf),
    mskPf(mskPf),
    op(op),
    _maxPixelsPerLoop(1),
    _complexity(Simple),
    _isNop(false),
    _prefetchDst(true),
    _prefetchSrc(true),
    _oldKindPos(NULL)
{
  setNumKinds(1);
  _bail = c->newLabel();
}

Module::~Module()
{
}

void Module::beginSwitch()
{
}

void Module::endSwitch()
{
  c->bind(_bail);
}

void Module::beginKind(UInt32 kind)
{
  if (kind > 0 )
  {
    _oldKindPos = c->setCurrent(c->lastEmittable());
  }
  c->bind(getKindLabel(kind));
}

void Module::endKind(UInt32 kind)
{
  if (kind > 0)
  {
    c->jmp(_bail);
    c->setCurrent(_oldKindPos);
    _oldKindPos = NULL;
  }
}

void Module::setNumKinds(UInt32 kinds)
{
  while (_labels.length() < kinds)
  {
    _labels.append(c->newLabel());
  }
}

UInt32 Module::numKinds() const
{
  return _labels.length();
}

AsmJit::Label* Module::getKindLabel(UInt32 kind) const
{
  return (kind < _labels.length()) ? _labels[kind] : NULL;
}

// ============================================================================
// [BlitJit::Module_Filter]
// ============================================================================

Module_Filter::Module_Filter(
  Generator* g,
  const PixelFormat* dstPf) :
    Module(g, dstPf, NULL, NULL, NULL)
{
}

Module_Filter::~Module_Filter()
{
}

void Module_Filter::init()
{
}

void Module_Filter::free()
{
}

// ============================================================================
// [BlitJit::Module_Fill]
// ============================================================================

Module_Fill::Module_Fill(
  Generator* g,
  const PixelFormat* dstPf,
  const PixelFormat* srcPf,
  const PixelFormat* mskPf,
  const Operator* op) :
    Module(g, dstPf, srcPf, mskPf, op)
{
}

Module_Fill::~Module_Fill()
{
}

void Module_Fill::init(AsmJit::PtrRef& _src)
{
}

void Module_Fill::free()
{
}

// ============================================================================
// [BlitJit::Module_Blit]
// ============================================================================

Module_Blit::Module_Blit(
  Generator* g,
  const PixelFormat* dstPf,
  const PixelFormat* srcPf,
  const PixelFormat* mskPf,
  const Operator* op) :
    Module(g, dstPf, srcPf, mskPf, op)
{
}

Module_Blit::~Module_Blit()
{
}

void Module_Blit::init()
{
}

void Module_Blit::free()
{
}

} // BlitJit namespace
