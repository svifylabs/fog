// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Assert.h>
#include <Fog/Core/Error.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/List.h>
#include <Fog/Core/Std.h>

namespace Fog {

// ===========================================================================
// [Debug]
// ===========================================================================

#if defined(DEBUG)
#define FOG_LIST_VERIFY_DATA(d) \
  FOG_ASSERT((d)->length <= (d)->capacity); \
  FOG_ASSERT((d)->startIndex + (d)->length <= (d)->capacity); \
  FOG_ASSERT((d)->startIndex + (d)->length == (d)->endIndex)
#else
#define FOG_LIST_VERIFY_DATA(d) \
  FOG_NOOP
#endif // DEBUG

// ===========================================================================
// [Fog::ListAPI_Base - Primitive Data Type]
// ===========================================================================

static inline void ListAPI_Base_deref_p(SequenceUntyped::Data* d)
{
  if (d->refCount.deref()) SequenceAPI_Base::_freeData((void*)d);
}

void ListAPI_Base::_reserve_p(void* self, sysuint_t sizeOfT, sysuint_t to)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;

  if (d->capacity < to || d->refCount.get() > 1)
  {
    SequenceUntyped::Data* newd;
    sysuint_t length = d->length;

    newd = (SequenceUntyped::Data*)SequenceAPI_Base::_allocData(sizeOfT, to, AllocCantFail);
    newd->length = length;
    newd->endIndex = length;
    memcpy(newd->data, d->data + (d->startIndex * sizeOfT), length * sizeOfT);

    ListAPI_Base_deref_p(
      AtomicBase::ptr_setXchg(&((SequenceUntyped*)self)->_d, newd));
  }
}

void ListAPI_Base::_reserve_p(void* self, sysuint_t sizeOfT, sysuint_t left, sysuint_t right)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;

  sysuint_t leftCapacity = d->startIndex;
  sysuint_t rightCapacity = d->capacity - d->endIndex;

  if (leftCapacity < left || rightCapacity < right || d->refCount.get() > 1) 
  {
    sysuint_t to = left + right;
    sysuint_t length = d->length;

    // Overflow
    FOG_ASSERT(to >= left);
    FOG_ASSERT(to + length >= to);

    to += length;

    SequenceUntyped::Data* newd;

    newd = (SequenceUntyped::Data*)SequenceAPI_Base::_allocData(sizeOfT, to, AllocCantFail);
    newd->length = length;
    newd->startIndex = left;
    newd->endIndex = left + length;
    memcpy(newd->data + (left * sizeOfT), d->data + (d->startIndex * sizeOfT), sizeOfT * length);

    ListAPI_Base_deref_p(
      AtomicBase::ptr_setXchg(&((SequenceUntyped*)self)->_d, newd));
  }
}

void ListAPI_Base::_grow_p(void* self, sysuint_t sizeOfT, sysuint_t left, sysuint_t right)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;

  sysuint_t leftCapacity = d->startIndex;
  sysuint_t rightCapacity = d->capacity - d->endIndex;

  if (leftCapacity < left || rightCapacity < right)
  {
    // Data will be reallocated and we want to create optimal startIndex 
    // and endIndex variables.
    //
    // There are some situations that Lists are used like Vectors only
    // for appending data. We can predict future by checking leftCapacity
    // and rightCapacity variables. leftCapacity is never larger than 0
    // if data hasn't been prepended, inserted, removed or replaced.

    sysuint_t length = d->length;

    sysuint_t leftThreshold = leftCapacity - (leftCapacity >> 1);
    sysuint_t rightThreshold = rightCapacity - (rightCapacity >> 1);

    sysuint_t oldLeft = left;
    sysuint_t oldRight = right;

    if (leftThreshold >= left) left = leftThreshold;
    if (rightThreshold >= right) right = rightThreshold;

    sysuint_t by = left + right;
    sysuint_t after = length + by;

    // Overflow
    if (by >= left || length >= after)
    {
      left = oldLeft;
      right = oldRight;
      by = left + right;
      after = length + by;

      FOG_ASSERT(by >= left);
      FOG_ASSERT(length >= after);
    }
    
    sysuint_t optimal = Std::calcOptimalCapacity(sizeof(SequencePrivate::Data), sizeOfT, d->capacity, after);
    sysuint_t distribute = optimal - length - left - right;
    
    if (oldLeft > oldRight)
      left += distribute;
    else
      right += distribute;

    _reserve_p(self, sizeOfT, left, right);
  }
  else if (d->refCount.get() > 1)
  {
    _reserve_p(self, sizeOfT, left, right);
  }
}

void ListAPI_Base::_assign_p(void* self, sysuint_t sizeOfT, const void* data, sysuint_t dataLength)
{
  SequenceAPI_Base::_clear_p(self);
  _reserve_p(self, sizeOfT, dataLength);

  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;
  d->length = dataLength;
  d->endIndex = dataLength;
  FOG_LIST_VERIFY_DATA(d);

  memcpy(d->data, data, dataLength * sizeOfT);
}

void ListAPI_Base::_append_p(void* self, sysuint_t sizeOfT, const void* data, sysuint_t dataLength)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;

  if (FOG_UNLIKELY(d->capacity - d->endIndex < dataLength) || FOG_UNLIKELY(d->refCount.get() > 1))
  {
    _grow_p(self, sizeOfT, 0, dataLength);
    d = ((SequenceUntyped*)self)->_d;
  }

  sysuint_t endIndex = d->endIndex;

  d->length += dataLength;
  d->endIndex += dataLength;
  FOG_LIST_VERIFY_DATA(d);

  memcpy(d->data + (endIndex * sizeOfT), data, dataLength * sizeOfT);
}

void ListAPI_Base::_insert_p(void* self, sysuint_t sizeOfT, sysuint_t index, const void* data, sysuint_t dataLength)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;

  sysuint_t length = d->length;
  if (index >= length) { _append_p(self, sizeOfT, data, length); return; }

  sysuint_t half = length >> 1;

  if (index < half)
  {
    if (FOG_UNLIKELY(d->startIndex < dataLength) || FOG_UNLIKELY(d->refCount.get() > 1))
    {
      _grow_p(self, sizeOfT, dataLength, 0);
      d = ((SequenceUntyped*)self)->_d;
    }

    sysuint_t moveFrom = d->startIndex;
    sysuint_t moveTo = moveFrom - dataLength;

    d->startIndex -= dataLength;
    d->length += dataLength;

    memmove(d->data + (moveTo * sizeOfT), d->data + (moveFrom * sizeOfT), index * sizeOfT);
    memcpy(d->data + ((moveTo + index) * sizeOfT), data, dataLength * sizeOfT);
  }
  else
  {
    if (FOG_UNLIKELY(d->capacity - d->endIndex < dataLength) || FOG_UNLIKELY(d->refCount.get() > 1))
    {
      _grow_p(self, sizeOfT, 0, dataLength);
      d = ((SequenceUntyped*)self)->_d;
    }

    sysuint_t moveFrom = d->startIndex + index;
    sysuint_t moveTo = moveFrom + dataLength;

    d->endIndex += dataLength;
    d->length += dataLength;

    memmove(d->data + (moveTo * sizeOfT), d->data + (moveFrom * sizeOfT), index * sizeOfT);
    memcpy(d->data + (moveFrom * sizeOfT), data, dataLength * sizeOfT);
  }
}

sysuint_t ListAPI_Base::_removeAt_p(void* self, sysuint_t sizeOfT, sysuint_t index, sysuint_t range)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;

  sysuint_t length = d->length;
  if (FOG_UNLIKELY(index >= length)) return 0;

  range = fog_min(range, d->length - index);
  if (FOG_UNLIKELY(range == 0)) return 0;

  sysuint_t newLength = length - range;

  if (FOG_UNLIKELY(d->refCount.get() > 1))
  {
    SequenceUntyped::Data* newd;

    newd = (SequenceUntyped::Data*)SequenceAPI_Base::_allocData(sizeOfT, newLength, AllocCantFail);
    newd->length = newLength;
    newd->endIndex = newLength;
    FOG_LIST_VERIFY_DATA(newd);

    memcpy(newd->data, d->data + (d->startIndex * sizeOfT), index * sizeOfT);
    memcpy(newd->data + index * sizeOfT, d->data + (d->startIndex + index + range) * sizeOfT, (newLength - index) * sizeOfT);

    ListAPI_Base_deref_p(
      AtomicBase::ptr_setXchg(&((SequenceUntyped*)self)->_d, newd));
  }
  else
  {
    sysuint_t startIndex = d->startIndex;

    sysuint_t moveFrom = startIndex;
    sysuint_t moveTo = startIndex;
    sysuint_t moveBy;

    if (index >= (length >> 1))
    {
      moveFrom += index + range;
      moveTo += index;
      moveBy = length - (index + range);

      d->endIndex -= range;
    }
    else
    {
      moveTo += range;
      moveBy = length - (index + range);

      d->startIndex += range;
    }

    d->length = newLength;
    FOG_LIST_VERIFY_DATA(d);

    memmove(d->data + moveTo * sizeOfT, d->data + moveFrom * sizeOfT, moveBy * sizeOfT);
  }

  return range;
}

sysuint_t ListAPI_Base::_replace_p(void* self, sysuint_t sizeOfT, sysuint_t index, sysuint_t range, const void* data, sysuint_t dataLength)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;
  sysuint_t d_length = d->length;

  if (Std::checkRange(d_length, index, &range))
  {
    if (dataLength == range)
    {
      SequenceAPI_Base::_detach_p(self, sizeOfT);

      d = ((SequenceUntyped*)self)->_d;
      memcpy(d->data + (d->startIndex + index) * sizeOfT, data, dataLength * sizeOfT);
    }
    else if (dataLength > range)
    {
      sysuint_t diff = dataLength - range;

      _insert_p(self, sizeOfT, index, data, diff);
      SequenceAPI_Base::_detach_p(self, sizeOfT);

      d = ((SequenceUntyped*)self)->_d;
      memcpy(d->data + (d->startIndex + index + diff) * sizeOfT, (const uint8_t*)data + (diff * sizeOfT), (range - diff) * sizeOfT);
    }
    else
    {
      sysuint_t diff = range - dataLength;

      _removeAt_p(self, sizeOfT, index, diff);
      SequenceAPI_Base::_detach_p(self, sizeOfT);

      d = ((SequenceUntyped*)self)->_d;
      memcpy(d->data + (d->startIndex + index) * sizeOfT, data, dataLength * sizeOfT);
    }
  }

  return range;
}

// ===========================================================================
// [Fog::ListAPI_Base - Class Data Type]
// ===========================================================================

static inline void ListAPI_Base_deref_c(SequenceUntyped::Data* d, const SequenceInfoVTable* vtable)
{
  if (d->refCount.deref()) 
  {
    vtable->free(d->data + (d->startIndex * vtable->sizeOfT), d->length);
    SequenceAPI_Base::_freeData((void*)d);
  }
}

void ListAPI_Base::_reserve_c(void* self, const SequenceInfoVTable* vtable, sysuint_t to)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;

  if (d->capacity < to || d->refCount.get() > 1)
  {
    SequenceUntyped::Data* newd;
    sysuint_t sizeOfT = vtable->sizeOfT;
    sysuint_t length = d->length;

    newd = (SequenceUntyped::Data*)SequenceAPI_Base::_allocData(vtable->sizeOfT, to, AllocCantFail);
    newd->length = length;
    newd->endIndex = length;
    vtable->copy(newd->data, d->data + (d->startIndex * sizeOfT), length);

    ListAPI_Base_deref_c(
      AtomicBase::ptr_setXchg(&((SequenceUntyped*)self)->_d, newd),
      vtable);
  }
}

void ListAPI_Base::_reserve_c(void* self, const SequenceInfoVTable* vtable, sysuint_t left, sysuint_t right)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;

  sysuint_t leftCapacity = d->startIndex;
  sysuint_t rightCapacity = d->capacity - d->endIndex;

  if (leftCapacity < left || rightCapacity < right || d->refCount.get() > 1) 
  {
    sysuint_t sizeOfT = vtable->sizeOfT;
    sysuint_t to = left + right;
    sysuint_t length = d->length;

    // Overflow
    FOG_ASSERT(to >= left);
    FOG_ASSERT(to + length >= to);

    to += length;

    SequenceUntyped::Data* newd;

    newd = (SequenceUntyped::Data*)SequenceAPI_Base::_allocData(sizeOfT, to, AllocCantFail);
    newd->length = length;
    newd->startIndex = left;
    newd->endIndex = left + length;
    vtable->copy(newd->data + (left * sizeOfT), d->data + (d->startIndex * sizeOfT), length);

    ListAPI_Base_deref_c(
      AtomicBase::ptr_setXchg(&((SequenceUntyped*)self)->_d, newd),
      vtable);
  }
}

void ListAPI_Base::_grow_c(void* self, const SequenceInfoVTable* vtable, sysuint_t left, sysuint_t right)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;

  sysuint_t leftCapacity = d->startIndex;
  sysuint_t rightCapacity = d->capacity - d->endIndex;

  if (leftCapacity < left || rightCapacity < right)
  {
    // Data will be reallocated and we want to create optimal startIndex 
    // and endIndex variables.
    //
    // There are some situations that Lists are used like Vectors only
    // for appending data. We can predict future by checking leftCapacity
    // and rightCapacity variables. leftCapacity is never larger than 0
    // if data hasn't been prepended, inserted, removed or replaced.

    sysuint_t sizeOfT = vtable->sizeOfT;
    sysuint_t length = d->length;

    sysuint_t leftThreshold = leftCapacity - (leftCapacity >> 1);
    sysuint_t rightThreshold = rightCapacity - (rightCapacity >> 1);

    sysuint_t oldLeft = left;
    sysuint_t oldRight = right;

    if (leftThreshold >= left) left = leftThreshold;
    if (rightThreshold >= right) right = rightThreshold;

    sysuint_t by = left + right;
    sysuint_t after = length + by;

    // Overflow
    if (by >= left || length >= after)
    {
      left = oldLeft;
      right = oldRight;
      by = left + right;
      after = length + by;

      FOG_ASSERT(by >= left);
      FOG_ASSERT(length >= after);
    }
    
    sysuint_t optimal = Std::calcOptimalCapacity(sizeof(SequencePrivate::Data), sizeOfT, d->capacity, after);
    sysuint_t distribute = optimal - length - left - right;
    
    if (oldLeft > oldRight)
      left += distribute;
    else
      right += distribute;

    _reserve_c(self, vtable, left, right);
  }
  else if (d->refCount.get() > 1)
  {
    _reserve_c(self, vtable, left, right);
  }
}

void ListAPI_Base::_assign_c(void* self, const SequenceInfoVTable* vtable, const void* data, sysuint_t dataLength)
{
  SequenceAPI_Base::_clear_c(self, vtable);
  _reserve_c(self, vtable, 0, dataLength);

  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;
  d->length = dataLength;
  d->endIndex = dataLength;
  FOG_LIST_VERIFY_DATA(d);

  vtable->copy(d->data, data, dataLength);
}

void ListAPI_Base::_append_c(void* self, const SequenceInfoVTable* vtable, const void* data, sysuint_t dataLength)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;
  sysuint_t sizeOfT = vtable->sizeOfT;

  if (FOG_UNLIKELY(d->capacity - d->length < dataLength) || FOG_UNLIKELY(d->refCount.get() > 1))
  {
    _grow_c(self, vtable, 0, dataLength);
    d = ((SequenceUntyped*)self)->_d;
  }

  sysuint_t endIndex = d->endIndex;

  d->length += dataLength;
  d->endIndex += dataLength;
  FOG_LIST_VERIFY_DATA(d);

  vtable->copy(d->data + (endIndex * sizeOfT), data, dataLength);
}

void ListAPI_Base::_insert_c(void* self, const SequenceInfoVTable* vtable, sysuint_t index, const void* data, sysuint_t dataLength)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;
  sysuint_t sizeOfT = vtable->sizeOfT;

  sysuint_t length = d->length;
  if (index >= length) { _append_p(self, sizeOfT, data, length); return; }

  sysuint_t half = length >> 1;

  if (index < half)
  {
    if (FOG_UNLIKELY(d->startIndex < dataLength) || FOG_UNLIKELY(d->refCount.get() > 1))
    {
      _grow_c(self, vtable, dataLength, 0);
      d = ((SequenceUntyped*)self)->_d;
    }

    sysuint_t moveFrom = d->startIndex;
    sysuint_t moveTo = moveFrom - dataLength;

    d->startIndex -= dataLength;
    d->length += dataLength;

    vtable->move(d->data + (moveTo * sizeOfT), d->data + (moveFrom * sizeOfT), index);
    vtable->copy(d->data + ((moveTo + index) * sizeOfT), data, dataLength);
  }
  else
  {
    if (FOG_UNLIKELY(d->capacity - d->endIndex < dataLength) || FOG_UNLIKELY(d->refCount.get() > 1))
    {
      _grow_c(self, vtable, 0, dataLength);
      d = ((SequenceUntyped*)self)->_d;
    }

    sysuint_t moveFrom = d->startIndex + index;
    sysuint_t moveTo = moveFrom + dataLength;

    d->endIndex += dataLength;
    d->length += dataLength;

    vtable->move(d->data + (moveTo * sizeOfT), d->data + (moveFrom * sizeOfT), index);
    vtable->copy(d->data + (moveFrom * sizeOfT), data, dataLength);
  }
}

sysuint_t ListAPI_Base::_removeAt_c(void* self, const SequenceInfoVTable* vtable, sysuint_t index, sysuint_t range)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;
  sysuint_t sizeOfT = vtable->sizeOfT;

  sysuint_t length = d->length;
  if (FOG_UNLIKELY(index >= length)) return 0;

  range = fog_min(range, d->length - index);
  if (FOG_UNLIKELY(range == 0)) return 0;

  sysuint_t newLength = length - range;

  if (FOG_UNLIKELY(d->refCount.get() > 1))
  {
    SequenceUntyped::Data* newd;

    newd = (SequenceUntyped::Data*)SequenceAPI_Base::_allocData(sizeOfT, newLength, AllocCantFail);
    newd->length = newLength;
    newd->endIndex = newLength;
    FOG_LIST_VERIFY_DATA(newd);

    vtable->copy(newd->data, d->data + (d->startIndex * sizeOfT), index);
    vtable->copy(newd->data + index * sizeOfT, d->data + (d->startIndex + index + range) * sizeOfT, newLength - index);

    ListAPI_Base_deref_p(
      AtomicBase::ptr_setXchg(&((SequenceUntyped*)self)->_d, newd));
  }
  else
  {
    sysuint_t startIndex = d->startIndex;

    sysuint_t moveFrom = startIndex;
    sysuint_t moveTo = startIndex;
    sysuint_t moveBy;

    if (index >= (length >> 1))
    {
      moveFrom += index + range;
      moveTo += index;
      moveBy = length - (index + range);

      d->endIndex -= range;
    }
    else
    {
      moveTo += range;
      moveBy = length - (index + range);

      d->startIndex += range;
    }

    d->length = newLength;
    FOG_LIST_VERIFY_DATA(d);

    vtable->move(d->data + moveTo * sizeOfT, d->data + moveFrom * sizeOfT, moveBy);
  }

  return range;
}

sysuint_t ListAPI_Base::_replace_c(void* self, const SequenceInfoVTable* vtable, sysuint_t index, sysuint_t range, const void* data, sysuint_t dataLength)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;
  sysuint_t sizeOfT = vtable->sizeOfT;
  sysuint_t d_length = d->length;

  if (Std::checkRange(d_length, index, &range))
  {
    if (dataLength == range)
    {
      SequenceAPI_Base::_detach_c(self, vtable);

      d = ((SequenceUntyped*)self)->_d;
      uint8_t* addr = d->data + (d->startIndex + index) * sizeOfT;
      vtable->free(addr, dataLength);
      vtable->copy(addr, data, dataLength);
    }
    else if (dataLength > range)
    {
      sysuint_t diff = dataLength - range;

      _insert_c(self, vtable, index, data, diff);
      SequenceAPI_Base::_detach_c(self, vtable);

      d = ((SequenceUntyped*)self)->_d;
      uint8_t* addr = d->data + (d->startIndex + index + diff) * sizeOfT;
      vtable->free(addr, range - diff);
      vtable->copy(addr, (const uint8_t*)data + (diff * sizeOfT), range - diff);
    }
    else
    {
      sysuint_t diff = range - dataLength;

      _removeAt_c(self, vtable, index, diff);
      SequenceAPI_Base::_detach_c(self, vtable);

      d = ((SequenceUntyped*)self)->_d;
      uint8_t* addr = d->data + (d->startIndex + index) * sizeOfT;
      vtable->free(addr, dataLength);
      vtable->copy(addr, data, dataLength);
    }
  }

  return range;
}

} // Fog namespace
