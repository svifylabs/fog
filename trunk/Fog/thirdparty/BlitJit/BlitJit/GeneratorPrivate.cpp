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

namespace BlitJit {

// ============================================================================
// [BlitJit::Module]
// ============================================================================

Module::Module(Generator* g) :
  g(g),
  c(g->c),
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
// [BlitJit::FilterModule]
// ============================================================================

FilterModule::FilterModule(
  Generator* g,
  const PixelFormat* pf,
  const Operator* op) :
    Module(g), pf(pf), op(op)
{
}

FilterModule::~FilterModule()
{
}

void FilterModule::init()
{
}

void FilterModule::free()
{
}

// ============================================================================
// [BlitJit::FillModule]
// ============================================================================

FillModule::FillModule(
  Generator* g,
  const PixelFormat* pf,
  const Operator* op) :
    Module(g), pf(pf), op(op)
{
}

FillModule::~FillModule()
{
}

void FillModule::init(AsmJit::PtrRef& _src, const PixelFormat* pfSrc)
{
  BLITJIT_USE(_src);
  BLITJIT_USE(pfSrc);
}

void FillModule::free()
{
}

// ============================================================================
// [BlitJit::CompositeModule]
// ============================================================================

CompositeModule::CompositeModule(
  Generator* g,
  const PixelFormat* pfDst,
  const PixelFormat* pfSrc,
  const Operator* op) :
    Module(g), pfDst(pfDst), pfSrc(pfSrc), op(op)
{
}

CompositeModule::~CompositeModule()
{
}

void CompositeModule::init()
{
}

void CompositeModule::free()
{
}

} // BlitJit namespace
