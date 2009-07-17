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
#include <Fog/Core/Math.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/Std.h>
#include <Fog/Core/Vector.h>

namespace Fog {

// ===========================================================================
// [Debug]
// ===========================================================================

#if defined(FOG_DEBUG)
#define FOG_VECTOR_VERIFY_DATA(d) \
  FOG_ASSERT((d)->length <= (d)->capacity); \
  FOG_ASSERT((d)->startIndex + (d)->length <= (d)->capacity); \
  FOG_ASSERT((d)->startIndex + (d)->length == (d)->endIndex)
#else
#define FOG_VECTOR_VERIFY_DATA(d) \
  FOG_NOOP
#endif // FOG_DEBUG

// ===========================================================================
// [Fog::VectorAPI_Base - Primitive Data Type]
// ===========================================================================

static FOG_INLINE void VectorAPI_Base_deref_p(SequenceUntyped::Data* d)
{
  if (d->refCount.deref()) SequenceAPI_Base::_freeData((void*)d);
}

void VectorAPI_Base::_reserve_p(void* self, sysuint_t sizeOfT, sysuint_t to)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;

  if (d->capacity < to || d->refCount.get() > 1)
  {
    SequenceUntyped::Data* newd;
    sysuint_t length = d->length;

    newd = (SequenceUntyped::Data*)SequenceAPI_Base::_allocData(sizeOfT, to, AllocCantFail);
    newd->length = length;
    newd->endIndex = length;
    memcpy(newd->data, d->data, sizeOfT * length);

    VectorAPI_Base_deref_p(
      AtomicBase::ptr_setXchg(&((SequenceUntyped*)self)->_d, newd));
  }
}

void VectorAPI_Base::_grow_p(void* self, sysuint_t sizeOfT, sysuint_t by)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;
  sysuint_t to = d->length + by;

  if (d->capacity < to || d->refCount.get() > 1) 
  {
    _reserve_p(self, sizeOfT, Std::calcOptimalCapacity(sizeof(SequencePrivate::Data), sizeOfT, d->length, to));
  }
}

void VectorAPI_Base::_assign_p(void* self, sysuint_t sizeOfT, const void* data, sysuint_t dataLength)
{
  SequenceAPI_Base::_clear_p(self);
  _reserve_p(self, sizeOfT, dataLength);

  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;
  d->length = dataLength;
  d->endIndex = dataLength;
  FOG_VECTOR_VERIFY_DATA(d);

  memcpy(d->data, data, dataLength * sizeOfT);
}

void VectorAPI_Base::_append_p(void* self, sysuint_t sizeOfT, const void* data, sysuint_t dataLength)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;
  sysuint_t end = d->length;

  if (FOG_UNLIKELY(d->capacity - d->length < dataLength) || FOG_UNLIKELY(d->refCount.get() > 1))
  {
    _grow_p(self, sizeOfT, dataLength);
    d = ((SequenceUntyped*)self)->_d;
  }

  d->length += dataLength;
  d->endIndex += dataLength;
  FOG_VECTOR_VERIFY_DATA(d);

  memcpy(d->data + end * sizeOfT, data, dataLength * sizeOfT);
}

void VectorAPI_Base::_insert_p(void* self, sysuint_t sizeOfT, sysuint_t index, const void* data, sysuint_t dataLength)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;
  sysuint_t end = d->length;

  if (FOG_UNLIKELY(d->capacity - d->length < dataLength) || FOG_UNLIKELY(d->refCount.get() > 1))
  {
    _grow_p(self, sizeOfT, dataLength);
    d = ((SequenceUntyped*)self)->_d;
  }

  d->length += dataLength;
  d->endIndex += dataLength;
  FOG_VECTOR_VERIFY_DATA(d);

  if (index > end) index = end;

  memmove(
    d->data + (index + dataLength) * sizeOfT,
    d->data + index * sizeOfT,
    (end - index) * sizeOfT);

  memcpy(
    d->data + index * sizeOfT, 
    data, 
    dataLength * sizeOfT);
}

sysuint_t VectorAPI_Base::_removeAt_p(void* self, sysuint_t sizeOfT, sysuint_t index, sysuint_t range)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;

  sysuint_t length = d->length;
  if (FOG_UNLIKELY(index >= length)) return 0;

  range = Math::min(range, d->length - index);
  if (FOG_UNLIKELY(range == 0)) return 0;

  sysuint_t newLength = length - range;

  if (FOG_UNLIKELY(d->refCount.get() > 1))
  {
    SequenceUntyped::Data* newd;

    newd = (SequenceUntyped::Data*)SequenceAPI_Base::_allocData(sizeOfT, newLength, AllocCantFail);
    newd->length = newLength;
    newd->endIndex = newLength;
    FOG_VECTOR_VERIFY_DATA(newd);

    memcpy(newd->data, d->data, index * sizeOfT);
    memcpy(newd->data + index * sizeOfT, d->data + (index + range) * sizeOfT, (newLength - index) * sizeOfT);

    VectorAPI_Base_deref_p(
      AtomicBase::ptr_setXchg(&((SequenceUntyped*)self)->_d, newd));
  }
  else
  {
    d->length = newLength;
    d->endIndex = newLength;
    FOG_VECTOR_VERIFY_DATA(d);

    memmove(d->data + index * sizeOfT, d->data + (index + range) * sizeOfT, (newLength - index) * sizeOfT);
  }

  return range;
}

sysuint_t VectorAPI_Base::_replace_p(void* self, sysuint_t sizeOfT, sysuint_t index, sysuint_t range, const void* data, sysuint_t dataLength)
{
  if (FOG_UNLIKELY(dataLength == 0)) { return _removeAt_p(self, sizeOfT, index, range); }

  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;

  sysuint_t length = d->length;
  if (index > length) index = length;

  range = Math::min(range, d->length - index);

  sysuint_t newLength = length - range;
  FOG_ASSERT(newLength + dataLength >= newLength); // Overflow
  newLength += dataLength;

  if (d->refCount.get() > 1 || d->capacity < newLength)
  {
    SequenceUntyped::Data* newd;

    newd = (SequenceUntyped::Data*)SequenceAPI_Base::_allocData(sizeOfT, newLength, AllocCantFail);
    newd->length = newLength;
    newd->endIndex = newLength;
    FOG_VECTOR_VERIFY_DATA(newd);

    memcpy(newd->data, d->data, index * sizeOfT);
    memcpy(newd->data + index * sizeOfT, data, dataLength * sizeOfT);
    memcpy(newd->data + (index + dataLength) * sizeOfT, d->data + (index + range) * sizeOfT, (length - range - index) * sizeOfT);

    VectorAPI_Base_deref_p(
      AtomicBase::ptr_setXchg(&((SequenceUntyped*)self)->_d, newd));
  }
  else
  {
    d->length = newLength;
    d->endIndex = newLength;
    FOG_VECTOR_VERIFY_DATA(d);

    // TODO: Check if it's correct
    memmove(d->data + (index + dataLength) * sizeOfT, d->data + (index + range) * sizeOfT, (length - range - index) * sizeOfT);
    memcpy(d->data + index * sizeOfT, data, dataLength * sizeOfT);
  }

  return range;
}

// ===========================================================================
// [Fog::VectorAPI_Base - Class Data Type]
// ===========================================================================

static inline void VectorAPI_Base_deref_c(SequenceUntyped::Data* d, const SequenceInfoVTable* vtable)
{
  if (d->refCount.deref()) 
  {
    vtable->free(d->data, d->length);
    SequenceAPI_Base::_freeData((void*)d);
  }
}

void VectorAPI_Base::_reserve_c(void* self, const SequenceInfoVTable* vtable, sysuint_t to)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;

  if (d->capacity < to || d->refCount.get() > 1)
  {
    SequenceUntyped::Data* newd;
    sysuint_t length = d->length;

    newd = (SequenceUntyped::Data*)SequenceAPI_Base::_allocData(vtable->sizeOfT, to, AllocCantFail);
    newd->length = length;
    newd->endIndex = length;
    vtable->copy(newd->data, d->data, length);

    VectorAPI_Base_deref_c(
      AtomicBase::ptr_setXchg(&((SequenceUntyped*)self)->_d, newd),
      vtable);
  }
}

void VectorAPI_Base::_grow_c(void* self, const SequenceInfoVTable* vtable, sysuint_t by)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;
  sysuint_t to = d->length + by;

  if (d->capacity < to || d->refCount.get() > 1) 
  {
    _reserve_c(self, vtable, Std::calcOptimalCapacity(sizeof(SequencePrivate::Data), vtable->sizeOfT, d->length, to));
  }
}

void VectorAPI_Base::_assign_c(void* self, const SequenceInfoVTable* vtable, const void* data, sysuint_t dataLength)
{
  SequenceAPI_Base::_clear_c(self, vtable);
  _reserve_c(self, vtable, dataLength);

  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;
  d->length = dataLength;
  d->endIndex = dataLength;
  FOG_VECTOR_VERIFY_DATA(d);

  vtable->copy(d->data, data, dataLength);
}

void VectorAPI_Base::_append_c(void* self, const SequenceInfoVTable* vtable, const void* data, sysuint_t dataLength)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;
  sysuint_t sizeOfT = vtable->sizeOfT;

  sysuint_t end = d->length;

  if (FOG_UNLIKELY(d->capacity - d->length < dataLength) || FOG_UNLIKELY(d->refCount.get() > 1))
  {
    _grow_c(self, vtable, dataLength);
    d = ((SequenceUntyped*)self)->_d;
  }

  d->length += dataLength;
  d->endIndex += dataLength;
  FOG_VECTOR_VERIFY_DATA(d);

  vtable->copy(d->data + end * sizeOfT, data, dataLength);
}

void VectorAPI_Base::_insert_c(void* self, const SequenceInfoVTable* vtable, sysuint_t index, const void* data, sysuint_t dataLength)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;
  sysuint_t sizeOfT = vtable->sizeOfT;

  sysuint_t end = d->length;

  if (FOG_UNLIKELY(d->capacity - d->length < dataLength) || FOG_UNLIKELY(d->refCount.get() > 1))
  {
    _grow_c(self, vtable, dataLength);
    d = ((SequenceUntyped*)self)->_d;
  }

  d->length += dataLength;
  d->endIndex += dataLength;
  FOG_VECTOR_VERIFY_DATA(d);

  if (index > end) index = end;

  vtable->move(
    d->data + (index + dataLength) * sizeOfT,
    d->data + index * sizeOfT,
    (end - index));

  vtable->copy(
    d->data + index * sizeOfT,
    data,
    dataLength);
}

sysuint_t VectorAPI_Base::_removeAt_c(void* self, const SequenceInfoVTable* vtable, sysuint_t index, sysuint_t range)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;
  sysuint_t sizeOfT = vtable->sizeOfT;

  sysuint_t length = d->length;
  if (FOG_UNLIKELY(index >= length)) return 0;

  range = Math::min(range, d->length - index);
  if (FOG_UNLIKELY(range == 0)) return 0;

  sysuint_t newLength = length - range;

  if (FOG_UNLIKELY(d->refCount.get() > 1))
  {
    SequenceUntyped::Data* newd;

    newd = (SequenceUntyped::Data*)SequenceAPI_Base::_allocData(sizeOfT, newLength, AllocCantFail);
    newd->length = newLength;
    newd->endIndex = newLength;
    FOG_VECTOR_VERIFY_DATA(newd);

    vtable->copy(newd->data, d->data, index);
    vtable->copy(newd->data + index * sizeOfT, d->data + (index + range) * sizeOfT, (newLength - index));

    VectorAPI_Base_deref_c(
      AtomicBase::ptr_setXchg(&((SequenceUntyped*)self)->_d, newd),
      vtable);
  }
  else
  {
    d->length = newLength;
    d->endIndex = newLength;
    FOG_VECTOR_VERIFY_DATA(d);

    vtable->free(d->data + index * sizeOfT, range);
    vtable->move(d->data + index * sizeOfT, d->data + (index + range) * sizeOfT, newLength - index);
  }

  return range;
}

sysuint_t VectorAPI_Base::_replace_c(void* self, const SequenceInfoVTable* vtable, sysuint_t index, sysuint_t range, const void* data, sysuint_t dataLength)
{
  if (FOG_UNLIKELY(dataLength == 0)) { return _removeAt_c(self, vtable, index, range); }

  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;
  sysuint_t sizeOfT = vtable->sizeOfT;

  sysuint_t length = d->length;
  if (index > length) index = length;

  range = Math::min(range, d->length - index);

  sysuint_t newLength = length - range;
  FOG_ASSERT(newLength + dataLength >= newLength); // Overflow
  newLength += dataLength;

  if (d->refCount.get() > 1 || d->capacity < newLength)
  {
    SequenceUntyped::Data* newd;

    newd = (SequenceUntyped::Data*)SequenceAPI_Base::_allocData(sizeOfT, newLength, AllocCantFail);
    newd->length = newLength;
    newd->endIndex = newLength;
    FOG_VECTOR_VERIFY_DATA(newd);

    vtable->copy(newd->data, d->data, index);
    vtable->copy(newd->data + index * sizeOfT, data, dataLength);
    vtable->copy(newd->data + (index + dataLength) * sizeOfT, d->data + (index + range) * sizeOfT, length - range - index);

    VectorAPI_Base_deref_c(
      AtomicBase::ptr_setXchg(&((SequenceUntyped*)self)->_d, newd),
      vtable);
  }
  else
  {
    d->length = newLength;
    d->endIndex = newLength;
    FOG_VECTOR_VERIFY_DATA(d);

    // TODO: Check if it's correct
    vtable->free(d->data + index * sizeOfT, length);
    vtable->move(d->data + (index + dataLength) * sizeOfT, d->data + (index + range) * sizeOfT, length - range - index);
    vtable->copy(d->data + index * sizeOfT, data, dataLength);
  }

  return range;
}

} // Fog namespace
