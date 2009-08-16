// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

#include <Fog/Core/ObserverList.h>

// [Fog::ObserverListBase]

Fog::ObserverListBase::ObserverListBase()
  : _notifyDepth(0)
{
}

Fog::ObserverListBase::~ObserverListBase()
{
}

void Fog::ObserverListBase::_addObserver(void* observer)
{
  // Observers can only be added once!"
  FOG_ASSERT(_observers.indexOf(observer) == Fog::InvalidIndex);

  _observers.append(observer);
}

bool Fog::ObserverListBase::_removeObserver(void* observer)
{
  sysuint_t i = _observers.indexOf(observer);

  if (i != Fog::InvalidIndex)
  {
    if (_notifyDepth)
      _observers.set(i, NULL);
    else
      _observers.removeAt(i);
    return true;
  }
  else
    return false;
}

bool Fog::ObserverListBase::_hasObserver(void* observer)
{
  return _observers.contains(observer);
}

void Fog::ObserverListBase::_compact() const
{
  sysuint_t i = 0;

  while (i < _observers.getLength())
  {
    if (_observers.cAt(i) == 0)
      _observers.removeAt(i);
    else
      i++;
  }
}
