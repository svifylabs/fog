// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.4
// Copyright (C) 2002-2005 Maxim Shemanarev (http://www.antigrain.com)
//
// Permission to copy, use, modify, sell and distribute this software 
// is granted provided this copyright notice appears in all copies. 
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
//----------------------------------------------------------------------------
//
// The author gratefully acknowleges the support of David Turner, 
// Robert Wilhelm, and Werner Lemberg - the authors of the FreeType 
// libray - in producing this work. See http://www.freetype.org for details.
//
//----------------------------------------------------------------------------
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://www.antigrain.com
//----------------------------------------------------------------------------
//
// Adaptation for 32-bit screen coordinates has been sponsored by 
// Liberty Technology Systems, Inc., visit http://lib-sys.com
//
// Liberty Technology Systems, Inc. is the provider of
// PostScript and PDF technology for software developers.
//
//----------------------------------------------------------------------------

// [Precompiled Headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/Error.h>
#include <Fog/Core/Math.h>
#include <Fog/Core/Memory.h>
#include <Fog/Graphics/Error.h>
#include <Fog/Graphics/Scanline.h>

namespace Fog {

const Scanline32::Span _zeroSpan;

Scanline32::Scanline32() :
  _xEnd(CoordInit),
  _y(CoordInit),
  _coversCapacity(0),
  _spansCapacity(0),
  _spansCount(0),
  _coversData(NULL),
  _coversCur(NULL),
  _spansData(NULL),
  _spanCur(const_cast<Span*>(&_zeroSpan) + 1),
  _spanEnd(const_cast<Span*>(&_zeroSpan) + 1)
{
}

Scanline32::~Scanline32()
{
  if (_coversData) Memory::free(_coversData);
  if (_spansData) Memory::free(_spansData - 1);
}

err_t Scanline32::init(int x1, int x2)
{
  FOG_ASSERT(x1 <= x2);

  uint32_t cap;

  _xEnd = CoordInit;
  _y = CoordInit;

  cap = Math::max(x2 - x1 + 3, 512);
  if (cap > _coversCapacity)
  {
    if (_coversData) Memory::free(_coversData);
    _coversData = (uint8_t*)Memory::alloc(cap * sizeof(uint8_t));
    if (!_coversData) goto error;
    _coversCapacity = cap;
  }

  cap = 32;
  if (cap > _spansCapacity)
  {
    if (_spansData) Memory::free(_spansData - 1);
    _spansData = (Span*)Memory::alloc((cap + 1) * sizeof(Span));
    if (!_spansData) goto error;

    _spansData[0].x = CoordInit;
    _spansData[0].len = 0;
    _spansData[0].covers = NULL;
    _spansData++;

    _spansCapacity = cap;
  }

  _coversCur = _coversData;
  _spanCur = _spansData;
  _spanEnd = _spansData + _spansCapacity;

  _spansCount = 0;
  return Error::Ok;

error:
  if (_coversData) { Memory::free(_coversData); _coversData = NULL; }
  if (_spansData) { Memory::free(_spansData - 1); _spansData = NULL; }

  _coversCapacity = 0;
  _spansCapacity = 0;
  _spansCount = 0;

  _coversCur = NULL;
  _spanCur = const_cast<Span*>(&_zeroSpan) + 1;
  _spanEnd = const_cast<Span*>(&_zeroSpan) + 1;

  return Error::OutOfMemory;
}

void Scanline32::reset()
{
  _xEnd = CoordInit;
  _y = CoordInit;

  _coversCur = _coversData;
  _spanCur = _spansData;
  _spanEnd = _spansData + _spansCapacity;

  _spansCount = 0;
}

bool Scanline32::grow()
{
  // This can't be called if something went wrong!
  FOG_ASSERT(_spansCapacity > 0);

  uint32_t cap = _spansCapacity < 65536 ? (_spansCapacity << 1) : _spansCapacity + 65536;

  Span *newData = (Span*)Memory::realloc(_spansData - 1, (cap + 1) * sizeof(Span));
  if (!newData) return false;

  _spansData = newData + 1;
  _spanCur = _spansData + _spansCapacity;
  _spanEnd = _spansData + cap;
  _spansCapacity = cap;
  return true;
}

} // Fog namespace
