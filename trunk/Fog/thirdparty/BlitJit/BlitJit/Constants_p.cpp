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
#include "Constants_p.h"

namespace BlitJit {

// ============================================================================
// [BlitJit::Constants]
// ============================================================================

Constants* Constants::instance = NULL;

void Constants::init()
{
  static UInt8 constantsStorage[sizeof(Constants) + 16];
  if (instance) return;

  // Align to 16 bytes (default SSE alignment)
  Constants* c = (Constants*)(void*)(((SysUInt)constantsStorage + 15) & ~(SysUInt)15);

  c->Cx00800080008000800080008000800080.set_uw(0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080, 0x0080);
  c->Cx00FF00FF00FF00FF00FF00FF00FF00FF.set_uw(0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF, 0x00FF);

  c->Cx000000FF00FF00FF000000FF00FF00FF.set_uw(0x00FF, 0x00FF, 0x00FF, 0x0000, 0x00FF, 0x00FF, 0x00FF, 0x0000);
  c->Cx00FF000000FF00FF00FF000000FF00FF.set_uw(0x00FF, 0x00FF, 0x0000, 0x00FF, 0x00FF, 0x00FF, 0x0000, 0x00FF);
  c->Cx00FF00FF000000FF00FF00FF000000FF.set_uw(0x00FF, 0x0000, 0x00FF, 0x00FF, 0x00FF, 0x0000, 0x00FF, 0x00FF);
  c->Cx00FF00FF00FF000000FF00FF00FF0000.set_uw(0x0000, 0x00FF, 0x00FF, 0x00FF, 0x0000, 0x00FF, 0x00FF, 0x00FF);

  c->Cx00FF00000000000000FF000000000000.set_uw(0x0000, 0x0000, 0x0000, 0x00FF, 0x0000, 0x0000, 0x0000, 0x00FF);
  c->Cx000000FF00000000000000FF00000000.set_uw(0x0000, 0x0000, 0x00FF, 0x0000, 0x0000, 0x0000, 0x00FF, 0x0000);
  c->Cx0000000000FF00000000000000FF0000.set_uw(0x0000, 0x00FF, 0x0000, 0x0000, 0x0000, 0x00FF, 0x0000, 0x0000);
  c->Cx00000000000000FF00000000000000FF.set_uw(0x00FF, 0x0000, 0x0000, 0x0000, 0x00FF, 0x0000, 0x0000, 0x0000);

  SysInt i;

  for (i = 0; i < 256; i++)
  {
    UInt16 a = 0xFF;
    UInt16 x = i > 0 ? (int)((256.0 * 255.0) / (float)i + 0.5) : 0;
    c->CxDemultiply[0][i].set_uw(a, i, i, i);
    c->CxDemultiply[1][i].set_uw(i, a, i, i);
    c->CxDemultiply[2][i].set_uw(i, i, a, i);
    c->CxDemultiply[3][i].set_uw(i, i, i, a);
  }

  instance = c;
}

} // BlitJit namespace
