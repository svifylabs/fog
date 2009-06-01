// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _FOG_CORE_OBSERVERLIST_H
#define _FOG_CORE_OBSERVERLIST_H

#include <Fog/Build/Build.h>
#include <Fog/Core/Vector.h>

namespace Fog {

// OVERVIEW:
//
//   A container for a list of observers.  Unlike a normal STL vector or list,
//   this container can be modified during iteration without invalidating the
//   iterator.  So, it safely handles the case of an observer removing itself
//   or other observers from the list while observers are being notified.
//
// TYPICAL USAGE:
//
//   class MyWidget
//   {
//   public:
//     ...
//
//     class Observer
//     {
//     public:
//       virtual void onFoo(MyWidget* w) = 0;
//       virtual void onBar(MyWidget* w, int x, int y) = 0;
//     };
//
//     void addObserver(Observer* obs) {
//       observer__list.AddObserver(obs);
//     }
//
//     void removeObserver(Observer* obs) {
//       observer__list.RemoveObserver(obs);
//     }
//
//     void notifyFoo() {
//       FOR_EACH_OBSERVER(Observer, observer__list, OnFoo(this));
//     }
//
//     void notifyBar(int x, int y) {
//       FOR_EACH_OBSERVER(Observer, observer__list, OnBar(this, x, y));
//     }
//
//   private:
//     ObserverList<Observer> observer__list;
//   };

struct FOG_API ObserverListBase
{
public:
  ObserverListBase();
  ~ObserverListBase();

protected:
  void _addObserver(void* observer);
  bool _removeObserver(void* observer);
  bool _hasObserver(void* observer);

  void _compact() const;

  FOG_INLINE sysuint_t _count() const
  { return _observers.length(); }

  FOG_INLINE void* _at(sysuint_t index) const
  { return _observers.cAt(index); }

  // These are marked mutable to facilitate having NotifyAll be const.
  mutable Fog::Vector<void*> _observers;
  mutable sysuint_t _notifyDepth;

private:
  FOG_DISABLE_COPY(ObserverListBase)
};

template <class ObserverType, bool CheckEmpty = false>
struct ObserverList : public ObserverListBase
{
public:
  FOG_INLINE ObserverList() : _notifyDepth(0) {}

  FOG_INLINE ~ObserverList()
  {
    // When check_empty is true, assert that the list is empty on destruction.
    if (CheckEmpty)
    {
      _compact();
      FOG_ASSERT(count() == 0U);
    }
  }

  // Add an observer to the list.
  FOG_INLINE void addObserver(ObserverType* observer)
  { ObserverListBase::_addObserver(static_cast<void*>(observer)); }

  // Remove an observer from the list.
  FOG_INLINE bool removeObserver(ObserverType* observer)
  { return ObserverListBase::_removeObserver(static_cast<void*>(observer)); }

  FOG_INLINE sysuint_t count() const
  { return ObserverListBase::_count(); }

  FOG_INLINE ObserverType* at(sysuint_t index) const
  { return ObserverListBase::_at(index); }

  FOG_INLINE Vector<ObserverType*>& observers() const
  { return *(Vector<ObserverType*>*)(&_observers); }

  // An iterator class that can be used to access the list of observers.  See
  // also the FOREACH_OBSERVER macro defined below.
  struct Iterator
  {
  public:
    FOG_INLINE Iterator(const ObserverList<ObserverType>& list) : _list(list), _index(0)
    { ++_list._notifyDepth; }

    FOG_INLINE ~Iterator()
    { if (--_list._notifyDepth == 0) _list._compact(); }

    FOG_INLINE ObserverType* next()
    {
      Vector<ObserverType*>& observers = _list.observers();
      sysuint_t length = observers.length();

      // Advance if the current element is null
      while (_index < length && !observers.cAt(_index)) _index++;
      return (ObserverType*)(_index < length ? observers.cAt(_index++) : NULL);
    }

  private:
    const ObserverList<ObserverType>& _list;
    sysuint_t _index;
  };

private:
  // These are marked mutable to facilitate having NotifyAll be const.
  mutable Vector<ObserverType*> _observers;
  mutable int _notifyDepth;

  friend struct ObserverList<ObserverType, CheckEmpty>::Iterator;

  FOG_DISABLE_COPY(ObserverList)
};

#define FOR_EACH_OBSERVER(_ObserverType, _olist, _func)       \
  FOG_BEGIN_MACRO                                             \
    ObserverList<_ObserverType>::Iterator it(_olist);         \
    _ObserverType* obs;                                       \
    while ((obs = it.next()) != NULL)                         \
      obs->_func;                                             \
  FOG_END_MACRO

} // Fog namespace

#endif  // _FOG_OBSERVER_LIST_H
