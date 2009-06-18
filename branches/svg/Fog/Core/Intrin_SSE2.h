// [Fog/Core Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// [Guard]
#ifndef _FOG_CORE_INTRIN_SSE2_H
#define _FOG_CORE_INTRIN_SSE2_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Core/Intrin_SSE.h>
#include <emmintrin.h>

//! @addtogroup Fog_Core
//! @{

namespace Fog {

//! @brief SSE2 data type that can be used to store / load data to / from
//! xmm register.
union FOG_ALIGN(16) sse2_t
{
  __m128d m128d;
  __m128i m128i;
  uint64_t uq[2];
  int64_t  sq[2];
  uint32_t ud[4];
  int32_t  sd[4];
  uint16_t uw[8];
  int16_t  sw[8];
  uint8_t  ub[16];
  int8_t   sb[16];
  float    f[4];
  double   d[2];
};

}

//! @}

// [Guard]
#endif // _FOG_CORE_INTRIN_SSE2_H
