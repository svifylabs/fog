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
#include <AsmJit/Assembler.h>
#include <AsmJit/Compiler.h>
#include <AsmJit/Logger.h>

#include "BlitJit.h"
#include "Constants_p.h"
#include "Generator_p.h"

namespace BlitJit {

// ============================================================================
// [BlitJit::Api - Initialization / Deinitialization]
// ============================================================================

void Api::init()
{
  if (Constants::instance == NULL) Constants::init();
}

// ============================================================================
// [BlitJit::Api - Pixel Formats]
// ============================================================================

const PixelFormat Api::pixelFormats[PixelFormat::Count] = 
{
  // Name   | Id                 | D | RGBA Size     | RGBA Shift      | premul | float |
  { "ARGB32", PixelFormat::ARGB32, 32,  8,  8,  8,  8, 16,  8 ,  0 , 24, false  , false },
  { "PRGB32", PixelFormat::PRGB32, 32,  8,  8,  8,  8, 16,  8 ,  0 , 24, true   , false },
  { "XRGB32", PixelFormat::XRGB32, 32,  8,  8,  8,  0, 16,  8 ,  0 ,  0, false  , false },

  { "RGB24" , PixelFormat::RGB24 , 24,  8,  8,  8,  0, 16,  8 ,  0 ,  0, false  , false },
  { "BGR24" , PixelFormat::BGR24 , 24,  8,  8,  8,  0,  0,  8 , 16 ,  0, false  , false },

  { "A8"    , PixelFormat::A8    ,  8,  0,  0,  0,  8,  0,  0 ,  0 ,  0, false  , false }
};

// ============================================================================
// [BlitJit::Api - Operators]
// ============================================================================

const Operator Api::operators[Operator::Count] = 
{
  // Name                 | Id                             | S, D Pixel  | S, D Alpha
  { "CompositeSrc"        , Operator::CompositeSrc        , true , true , false, false  },
  { "CompositeDest"       , Operator::CompositeDest       , false, false, true , true   },
  { "CompositeOver"       , Operator::CompositeOver       , true , true , true , true   },
  { "CompositeOverReverse", Operator::CompositeOverReverse, true , true , true , true   },
  { "CompositeIn"         , Operator::CompositeIn         , true , true , true , true   },
  { "CompositeInReverse"  , Operator::CompositeInReverse  , true , true , true , true   },
  { "CompositeOut"        , Operator::CompositeOut        , true , true , true , true   },
  { "CompositeOutReverse" , Operator::CompositeOutReverse , true , true , true , true   },
  { "CompositeAtop"       , Operator::CompositeAtop       , true , true , true , true   },
  { "CompositeAtopReverse", Operator::CompositeAtopReverse, true , true , true , true   },
  { "CompositeXor"        , Operator::CompositeXor        , true , true , true , true   },
  { "CompositeClear"      , Operator::CompositeClear      , false, false, false, false  },
  { "CompositeAdd"        , Operator::CompositeAdd        , true , true , true , true   },
  { "CompositeSubtract"   , Operator::CompositeSubtract   , true , true , true , true   },
  { "CompositeMultiply"   , Operator::CompositeMultiply   , true , true , true , true   },
  { "CompositeScreen"     , Operator::CompositeScreen     , true , true , true , true   },
  { "CompositeDarken"     , Operator::CompositeDarken     , true , true , true , true   },
  { "CompositeLighten"    , Operator::CompositeLighten    , true , true , true , true   },
  { "CompositeDifference" , Operator::CompositeDifference , true , true , true , true   },
  { "CompositeExclusion"  , Operator::CompositeExclusion  , true , true , true , true   },
  { "CompositeInvert"     , Operator::CompositeInvert     , true , true , true , true   },
  { "CompositeInvertRgb"  , Operator::CompositeInvertRgb  , true , true , true , true   }
};

// ============================================================================
// [BlitJit::Api - Generator]
// ============================================================================

void configureCompiler(AsmJit::Compiler* c)
{
  static AsmJit::FileLogger logger(stderr);
  c->setLogger(&logger);
}

PremultiplyFn Api::genPremultiply(
  const PixelFormat* dstPf)
{
  Generator gen;
  configureCompiler(gen.c);

  gen.genPremultiply(dstPf);
  return AsmJit::function_cast<PremultiplyFn>(gen.c->make());
}

DemultiplyFn Api::genDemultiply(
  const PixelFormat* dstPf)
{
  Generator gen;
  configureCompiler(gen.c);

  gen.genDemultiply(dstPf);
  return AsmJit::function_cast<DemultiplyFn>(gen.c->make());
}

FillSpanFn Api::genFillSpan(
  const PixelFormat* dstPf,
  const PixelFormat* srcPf, 
  const Operator* op)
{
  Generator gen;
  configureCompiler(gen.c);

  gen.genFillSpan(dstPf, srcPf, op);
  return AsmJit::function_cast<FillSpanFn>(gen.c->make());
}

FillSpanWithMaskFn Api::genFillSpanWithMask(
  const PixelFormat* dstPf,
  const PixelFormat* srcPf,
  const PixelFormat* mskPf,
  const Operator* op)
{
  Generator gen;
  configureCompiler(gen.c);

  gen.genFillSpanWithMask(dstPf, srcPf, mskPf, op);
  return AsmJit::function_cast<FillSpanWithMaskFn>(gen.c->make());
}

FillRectFn Api::genFillRect(
  const PixelFormat* dstPf,
  const PixelFormat* srcPf,
  const Operator* op)
{
  Generator gen;
  configureCompiler(gen.c);

  gen.genFillRect(dstPf, srcPf, op);
  return AsmJit::function_cast<FillRectFn>(gen.c->make());
}

FillRectWithMaskFn Api::genFillRectWithMask(
  const PixelFormat* dstPf,
  const PixelFormat* srcPf,
  const PixelFormat* mskPf,
  const Operator* op)
{
  Generator gen;
  configureCompiler(gen.c);

  gen.genFillRectWithMask(dstPf, srcPf, mskPf, op);
  return AsmJit::function_cast<FillRectWithMaskFn>(gen.c->make());
}

BlitSpanFn Api::genBlitSpan(
  const PixelFormat* dstPf,
  const PixelFormat* srcPf,
  const Operator* op)
{
  Generator gen;
  configureCompiler(gen.c);

  gen.genBlitSpan(dstPf, srcPf, op);
  return AsmJit::function_cast<BlitSpanFn>(gen.c->make());
}

BlitRectFn Api::genBlitRect(
  const PixelFormat* dstPf,
  const PixelFormat* srcPf,
  const Operator* op)
{
  Generator gen;
  configureCompiler(gen.c);

  gen.genBlitRect(dstPf, srcPf, op);
  return AsmJit::function_cast<BlitRectFn>(gen.c->make());
}

} // BlitJit namespace