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
#include <Fog/Core/Sequence.h>
#include <Fog/Core/Error.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/Std.h>

static uint8_t Core_Sequence_sharedNull_storage[sizeof(Fog::SequencePrivate::UntypedData)];
FOG_CVAR_DECLARE void* Core_Sequence_sharedNull;

namespace Fog {

// ===========================================================================
// [Fog::SequenceAPI_Base - Data Functions]
// ===========================================================================

void* SequenceAPI_Base::_allocData(sysuint_t sizeOfT, sysuint_t capacity, uint allocPolicy)
{
  SequenceUntyped::Data* d = NULL;
  sysuint_t dsize = _getDataSize(sizeOfT, capacity);

  // Alloc
  if (dsize == 0 || !(d = (SequenceUntyped::Data*)Memory::alloc(dsize)))
  {
    if (allocPolicy == AllocCantFail)
    {
      fog_out_of_memory_fatal_format(
        "Fog::Sequence", "_allocData", 
        "Couldn't allocate %lu bytes of memory for continuous data", (ulong)dsize);
    }
    return NULL;
  }

  // Init 'd'
  d->refCount.init(1);
  d->flags = 0;
#if FOG_ARCH_BITS == 64
  d->_data_padding = 0;
#endif //FOG_ARCH_BITS == 64
  d->capacity = capacity;
  d->length = 0;
  d->startIndex = 0;
  d->endIndex = 0;

  return (void*)d;
}

void SequenceAPI_Base::_freeData(void* d)
{
  Memory::free(d);
}

sysuint_t SequenceAPI_Base::_getDataSize(sysuint_t sizeOfT, sysuint_t capacity)
{
  sysuint_t dSize = sizeof(SequencePrivate::Data);
  sysuint_t eSize = sizeOfT * capacity;
  sysuint_t _Size = dSize + eSize;

  // Overflow
  if ((eSize / sizeOfT) != capacity) return 0;
  if (_Size < eSize) return 0;

  return _Size;
}

// ===========================================================================
// [Fog::SequenceAPI_Base - Primitive Data Type]
// ===========================================================================

static inline void SequenceAPI_Base_deref_p(SequenceUntyped::Data* d)
{
  if (d->refCount.deref()) SequenceAPI_Base::_freeData((void*)d);
}

void SequenceAPI_Base::_detach_p(void* self, sysuint_t sizeOfT)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;

  if (d->refCount.get() > 1)
  {
    SequenceUntyped::Data* newd;
    sysuint_t length = d->length;

    newd = (SequenceUntyped::Data*)_allocData(sizeOfT, length, AllocCantFail);
    newd->length = length;
    newd->endIndex = length;
    memcpy(newd->data, d->data + (d->startIndex * sizeOfT), sizeOfT * length);

    SequenceAPI_Base_deref_p(
      AtomicBase::ptr_setXchg(&((SequenceUntyped*)self)->_d, newd));
  }
}

void SequenceAPI_Base::_squeeze_p(void* self, sysuint_t sizeOfT)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;

  if (d->capacity != d->length || d->refCount.get() > 1)
  {
    SequenceUntyped::Data* newd;
    sysuint_t length = d->length;

    newd = (SequenceUntyped::Data*)_allocData(sizeOfT, length, AllocCantFail);
    newd->length = length;
    newd->endIndex = length;
    memcpy(newd->data, d->data + (d->startIndex * sizeOfT), sizeOfT * length);

    SequenceAPI_Base_deref_p(
      AtomicBase::ptr_setXchg(&((SequenceUntyped*)self)->_d, newd));
  }
}

void SequenceAPI_Base::_clear_p(void* self)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;

  if (d->length > 0)
  {
    if (d->refCount.get() > 1)
    {
      _free_p(self);
    }
    else
    {
      d->length = 0;
      d->startIndex = 0;
      d->endIndex = 0;
    }
  }
}

void SequenceAPI_Base::_free_p(void* self)
{
  ((SequenceUntyped::Data*)Core_Sequence_sharedNull)->refCount.inc();

  SequenceAPI_Base_deref_p(
    AtomicBase::ptr_setXchg(
      &((SequenceUntyped*)self)->_d, 
      ((SequenceUntyped::Data*)Core_Sequence_sharedNull)));
}

void SequenceAPI_Base::_swap_p(void* self, sysuint_t sizeOfT, sysuint_t index1, sysuint_t index2)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;

  sysuint_t length = d->length;

  FOG_ASSERT_X(index1 > length, "Fog::Sequence<T>::swap() - index1 out of range");
  FOG_ASSERT_X(index2 > length, "Fog::Sequence<T>::swap() - index2 out of range");

  if (d->refCount.get() > 1)
  {
    _detach_p(self, sizeOfT);
    d = ((SequenceUntyped*)self)->_d;
  }

  sysuint_t startIndex = d->startIndex;
  Memory::xchg(d->data + (startIndex + index1) * sizeOfT, d->data + (startIndex + index2) * sizeOfT, sizeOfT);
}

void SequenceAPI_Base::_sort_p(void* self, sysuint_t sizeOfT, TypeInfo_CompareFn compar)
{
  _detach_p(self, sizeOfT);

  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;
  fog_qsort(d->data + (d->startIndex) * sizeOfT, d->length, sizeOfT, (fog_qsort_callback)compar);
}

void SequenceAPI_Base::_reverse_p(void* self, sysuint_t sizeOfT)
{
  // TODO
}

// ===========================================================================
// [Fog::SequenceAPI_Base - Class Data Type]
// ===========================================================================

static inline void SequenceAPI_Base_deref_c(SequenceUntyped::Data* d, const SequenceInfoVTable* vtable)
{
  if (d->refCount.deref()) 
  {
    vtable->free(d->data + d->startIndex * (vtable->sizeOfT), d->length);
    SequenceAPI_Base::_freeData((void*)d);
  }
}

void SequenceAPI_Base::_detach_c(void* self, const SequenceInfoVTable* vtable)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;

  if (d->refCount.get() > 1)
  {
    SequenceUntyped::Data* newd;
    sysuint_t length = d->length;

    newd = (SequenceUntyped::Data*)_allocData(vtable->sizeOfT, length, AllocCantFail);
    newd->length = length;
    newd->endIndex = length;
    vtable->copy(newd->data, d->data + (d->startIndex * vtable->sizeOfT), length);

    SequenceAPI_Base_deref_c(
      AtomicBase::ptr_setXchg(&((SequenceUntyped*)self)->_d, newd),
      vtable);
  }
}

void SequenceAPI_Base::_squeeze_c(void* self, const SequenceInfoVTable* vtable)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;

  if (d->capacity != d->length || d->refCount.get() > 1)
  {
    SequenceUntyped::Data* newd;
    sysuint_t length = d->length;

    newd = (SequenceUntyped::Data*)_allocData(vtable->sizeOfT, length, AllocCantFail);
    newd->length = length;
    newd->endIndex = length;
    vtable->copy(newd->data, d->data + (d->startIndex * vtable->sizeOfT), length);

    SequenceAPI_Base_deref_c(
      AtomicBase::ptr_setXchg(&((SequenceUntyped*)self)->_d, newd),
      vtable);
  }
}

void SequenceAPI_Base::_clear_c(void* self, const SequenceInfoVTable* vtable)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;

  if (d->length > 0)
  {
    if (d->refCount.get() > 1)
    {
      _free_c(self, vtable);
    }
    else
    {
      vtable->free(d->data + (d->startIndex * vtable->sizeOfT), d->length);
      d->length = 0;
      d->startIndex = 0;
      d->endIndex = 0;
    }
  }
}

void SequenceAPI_Base::_free_c(void* self, const SequenceInfoVTable* vtable)
{
  ((SequenceUntyped::Data*)Core_Sequence_sharedNull)->refCount.inc();

  SequenceAPI_Base_deref_c(
    AtomicBase::ptr_setXchg(
      &((SequenceUntyped*)self)->_d, 
      (SequenceUntyped::Data*)Core_Sequence_sharedNull),
    vtable);
}

void SequenceAPI_Base::_swap_c(void* self, const SequenceInfoVTable* vtable, sysuint_t index1, sysuint_t index2)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;
  sysuint_t sizeOfT = vtable->sizeOfT;

  sysuint_t length = d->length;

  FOG_ASSERT_X(index1 > length, "Fog::Sequence<T>::swap() - index1 out of range");
  FOG_ASSERT_X(index2 > length, "Fog::Sequence<T>::swap() - index2 out of range");

  if (d->refCount.get() > 1)
  {
    _detach_c(self, vtable);
    d = ((SequenceUntyped*)self)->_d;
  }

  sysuint_t startIndex = d->startIndex;
  Memory::xchg(d->data + (startIndex + index1) * sizeOfT, d->data + (startIndex + index2) * sizeOfT, sizeOfT);
}

void SequenceAPI_Base::_sort_c(void* self, const SequenceInfoVTable* vtable, TypeInfo_CompareFn compar)
{
  _detach_c(self, vtable);

  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;
  fog_qsort(d->data + (d->startIndex * vtable->sizeOfT), d->length, vtable->sizeOfT, (fog_qsort_callback)compar);
}

void SequenceAPI_Base::_reverse_c(void* self, const SequenceInfoVTable* vtable)
{
  // TODO
}

// ===========================================================================
// [Fog::SequenceAPI_Base - Both]
// ===========================================================================

static inline bool rangeCheck(sysuint_t length, sysuint_t index, sysuint_t *range)
{
  if (index >= length) 
  {
    return false;
  }
  else
  {
    register sysuint_t max = length - index;

    if (*range > max) *range = max;
    return true;
  }
}

sysuint_t SequenceAPI_Base::_indexOf(void* self, sysuint_t sizeOfT, const void* e, TypeInfo_EqFn eq)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;

  sysuint_t i, length = d->length;
  const uint8_t* dCur = d->data + (d->startIndex) * sizeOfT;

  for (i = 0; i != length; dCur += sizeOfT, i++) if (eq((const void*)dCur, e)) return i;
  return InvalidIndex;
}

sysuint_t SequenceAPI_Base::_lastIndexOf(void* self, sysuint_t sizeOfT, const void* e, TypeInfo_EqFn eq)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;

  sysuint_t i, length = d->length;
  const uint8_t* dCur = d->data + (d->startIndex + length - 1) * sizeOfT;

  for (i = (length-1); i != InvalidIndex; dCur -= sizeOfT, i--) if (eq((const void*)dCur, e)) return i;
  return InvalidIndex;
}

sysuint_t SequenceAPI_Base::_indexOfAt(void* self, sysuint_t sizeOfT, sysuint_t index, sysuint_t range, const void* e, TypeInfo_EqFn eq)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;

  sysuint_t i, length = d->length;
  if (!rangeCheck(length, index, &range)) return InvalidIndex;
  const uint8_t* dCur = d->data + (d->startIndex + index * sizeOfT);

  for (i = 0; i != range; dCur += sizeOfT, i++) if (eq((const void*)dCur, e)) return index + i;
  return InvalidIndex;
}

sysuint_t SequenceAPI_Base::_lastIndexOfAt(void* self, sysuint_t sizeOfT, sysuint_t index, sysuint_t range, const void* e, TypeInfo_EqFn eq)
{
  SequenceUntyped::Data* d = ((SequenceUntyped*)self)->_d;

  sysuint_t i, length = d->length;
  if (!rangeCheck(length, index, &range)) return InvalidIndex;
  const uint8_t* dCur = d->data + ((d->startIndex + index + range - 1) * sizeOfT);

  for (i = range-1; i != InvalidIndex; dCur -= sizeOfT, i--) if (eq((const void*)dCur, e)) return index + i;
  return InvalidIndex;
}

} // Fog namespace

// ===========================================================================
// [Library Initializers]
// ===========================================================================

typedef Fog::SequencePrivate::UntypedData UntypedData;

FOG_INIT_DECLARE err_t fog_sequence_init(void)
{
  UntypedData* d = (UntypedData*)Core_Sequence_sharedNull_storage;
  Core_Sequence_sharedNull = (void*)d;

  memset(d, 0, sizeof(UntypedData));
  d->refCount.init(1);
  d->flags.init(Fog::SequenceUntyped::Data::IsNull);

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_sequence_shutdown(void)
{
  UntypedData* d = (UntypedData*)Core_Sequence_sharedNull;
  d->refCount.dec();
}
