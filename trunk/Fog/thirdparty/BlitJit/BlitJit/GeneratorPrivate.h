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
#ifndef _BLITJIT_GENERATORPRIVATE_H
#define _BLITJIT_GENERATORPRIVATE_H

// [Dependencies]
#include <AsmJit/Compiler.h>

#include "Generator.h"

namespace BlitJit {

//! @addtogroup BlitJit_Generator
//! @{

// ============================================================================
// [BlitJit::Macros]
// ============================================================================

#define BLITJIT_DISPCONST(__cname__) \
  (SysInt)( (UInt8 *)&Api::constants->__cname__ - (UInt8 *)Api::constants )

#define BLITJIT_GETCONST(__generator__, __name__) \
  __generator__->getConstantsOperand(BLITJIT_DISPCONST(__name__))

// ============================================================================
// [BlitJit::BaseModule]
// ============================================================================

struct BLITJIT_API Module
{
  Module(Generator* g);
  virtual ~Module();

  virtual void beginSwitch();
  virtual void endSwitch();

  virtual void beginKind(UInt32 kind);
  virtual void endKind(UInt32 kind);

  void setNumKinds(UInt32 kinds);
  UInt32 numKinds() const;
  AsmJit::Label* getKindLabel(UInt32 kind) const;

  inline UInt32 maxPixelsPerLoop() const
  { return _maxPixelsPerLoop; }

  inline UInt32 complexity() const
  { return _complexity; }

  inline UInt32 isNop() const
  { return _isNop; }

  inline UInt32 prefetchDst() const
  { return _prefetchDst; }

  inline UInt32 prefetchSrc() const
  { return _prefetchSrc; }

  //! @brief Type of complexity.
  enum Complexity
  {
    //! @brief Algorithm complexity is simple. This is always standard rasterop
    //! operator and can be used by some really simple compositing operators.
    Simple = 0,

    //! @brief Algorithm complexity is complex. This is used by nearly all
    //! compositing operators.
    Complex = 1
  };

  enum Flags
  {
    DstAligned = (1 << 0),
    SrcAligned = (1 << 1)
  };

  //! @brief Generator instance.
  Generator* g;

  //! @brief Compiler instance.
  AsmJit::Compiler* c;

  //! @brief Maximum pixels in loop that should be used by loop generator.
  //!
  //! Default: @c 1.
  UInt32 _maxPixelsPerLoop;

  //! @brief Operation complexity used by loop generator.
  //!
  //! Default: @c Simple.
  UInt32 _complexity;

  //! @brief True if current combination means no operation (nop).
  //!
  //! Default: @c false.
  bool _isNop;

  //! @brief True if dst should be prefetched if prefetch is enabled.
  bool _prefetchDst;

  //! @brief True if src should be prefetched if prefetch is enabled.
  bool _prefetchSrc;

  //! @brief Labels for kinds.
  AsmJit::PodVector<AsmJit::Label*> _labels;

  //! @brief Bailout (used if module has more kinds).
  AsmJit::Label* _bail;

  //! @brief Saved emittable position (used for kind no. 1, 2, ...)
  AsmJit::Emittable* _oldKindPos;
};

// ============================================================================
// [BlitJit::FilterModule]
// ============================================================================

struct BLITJIT_API FilterModule : public Module
{
  FilterModule(
    Generator* g,
    const PixelFormat* pf,
    const Operator* op);
  virtual ~FilterModule();

  virtual void init();
  virtual void free();

  virtual void processPixelsPtr(
    const AsmJit::PtrRef& dst,
    SysInt count,
    SysInt displacement,
    UInt32 kind,
    UInt32 flags) = 0;

  const PixelFormat* pf;
  const Operator* op;
};

// ============================================================================
// [BlitJit::FillModule]
// ============================================================================

struct BLITJIT_API FillModule : public Module
{
  FillModule(
    Generator* g,
    const PixelFormat* pf,
    const Operator* op);
  virtual ~FillModule();

  virtual void init(AsmJit::PtrRef& _src, const PixelFormat* pfSrc);
  virtual void free();

  virtual void processPixelsPtr(
    const AsmJit::PtrRef& dst,
    SysInt count,
    SysInt displacement,
    UInt32 kind,
    UInt32 flags) = 0;

  const PixelFormat* pf;
  const Operator* op;
};

// ============================================================================
// [BlitJit::CompositeModule]
// ============================================================================

struct BLITJIT_API CompositeModule : public Module
{
  CompositeModule(
    Generator* g,
    const PixelFormat* pfDst,
    const PixelFormat* pfSrc,
    const Operator* op);
  virtual ~CompositeModule();

  virtual void init();
  virtual void free();

  virtual void processPixelsPtr(
    const AsmJit::PtrRef& dst,
    const AsmJit::PtrRef& src,
    SysInt count,
    SysInt displacement,
    UInt32 kind,
    UInt32 flags) = 0;

  const PixelFormat* pfDst;
  const PixelFormat* pfSrc;
  const Operator* op;
};

//! @}

} // BlitJit namespace

// [Guard]
#endif // _BLITJIT_GENERATORPRIVATE_H
