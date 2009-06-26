// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_SEQUENCE_H
#define _FOG_CORE_SEQUENCE_H

// [Dependencies]
#include <Fog/Core/Assert.h>
#include <Fog/Core/Atomic.h>
#include <Fog/Build/Build.h>
#include <Fog/Core/Error.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/SequenceInfo.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/Std.h>
#include <Fog/Core/TypeInfo.h>

#include <new>

FOG_CVAR_EXTERN void* Core_Sequence_sharedNull;

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

template<typename T> struct Sequence;

// ============================================================================
// [Fog::SequenceAPI]
// ============================================================================

//! @addtogroup Fog_Core_Detail
//! @{

struct FOG_API SequenceAPI_Base
{
  // [Data Functions]
  static void* _allocData(sysuint_t sizeOfT, sysuint_t capacity, uint allocPolicy);
  static void _freeData(void* d);
  static sysuint_t _getDataSize(sysuint_t sizeOfT, sysuint_t capacity);

  // [Primitive Data Type]
  static void _detach_p(void* self, sysuint_t sizeOfT);
  static void _squeeze_p(void* self, sysuint_t sizeOfT);
  static void _clear_p(void* self);
  static void _free_p(void* self);

  static void _swap_p(void* self, sysuint_t sizeOfT, sysuint_t index1, sysuint_t index2);
  static void _sort_p(void* self, sysuint_t sizeOfT, TypeInfo_CompareFn compar);
  static void _reverse_p(void* self, sysuint_t sizeOfT);

  // [Class Data Type]
  static void _detach_c(void* self, const SequenceInfoVTable* vtable);
  static void _squeeze_c(void* self, const SequenceInfoVTable* vtable);
  static void _clear_c(void* self, const SequenceInfoVTable* vtable);
  static void _free_c(void* self, const SequenceInfoVTable* vtable);

  static void _swap_c(void* self, const SequenceInfoVTable* vtable, sysuint_t index1, sysuint_t index2);
  static void _sort_c(void* self, const SequenceInfoVTable* vtable, TypeInfo_CompareFn compar);
  static void _reverse_c(void* self, const SequenceInfoVTable* vtable);

  // [All Data Types]

  static sysuint_t _indexOf(void* self, sysuint_t sizeOfT, const void* e, TypeInfo_EqFn eq);
  static sysuint_t _lastIndexOf(void* self, sysuint_t sizeOfT, const void* e, TypeInfo_EqFn eq);

  static sysuint_t _indexOfAt(void* self, sysuint_t sizeOfT, sysuint_t index, sysuint_t range, const void* e, TypeInfo_EqFn eq);
  static sysuint_t _lastIndexOfAt(void* self, sysuint_t sizeOfT, sysuint_t index, sysuint_t range, const void* e, TypeInfo_EqFn eq);
};

// 0 = PrimitiveType
// 1 = MoveableType
// 2 = ClassType
template<typename T, uint __TypeInfo__>
struct SequenceAPI_Wrapper {};

template<typename T>
struct SequenceAPI_Wrapper<T, 0> : public SequenceAPI_Base
{
  typedef bool (*ElementEqFn)(const T* a, const T* b);
  typedef int (*ElementCompareFn)(const T* a, const T* b);

  // Sequence<T> functions
  static FOG_INLINE void detach(Sequence<T>* self)
  { _detach_p((void*)self, sizeof(T)); }

  static FOG_INLINE void squeeze(Sequence<T>* self)
  { _squeeze_p((void*)self, sizeof(T)); }

  static FOG_INLINE void clear(Sequence<T>* self)
  { _clear_p((void*)self); }

  static FOG_INLINE void free(Sequence<T>* self)
  { _free_p((void*)self); }

  static FOG_INLINE void swap(Sequence<T>* self, sysuint_t index1, sysuint_t index2)
  { _swap_p((void*)self, sizeof(T), index1, index2); }

  static FOG_INLINE void sort(Sequence<T>* self, ElementCompareFn compar)
  { _sort_p((void*)self, sizeof(T), (TypeInfo_CompareFn)compar); }

  static FOG_INLINE void reverse(Sequence<T>* self)
  { _reverse_p((void*)self, sizeof(T)); }

  static FOG_INLINE sysuint_t indexOf(const Sequence<T>* self, const T* e, ElementEqFn eq)
  { return _indexOf((void*)self, sizeof(T), e, (TypeInfo_EqFn)eq); }

  static FOG_INLINE sysuint_t lastIndexOf(const Sequence<T>* self, const T* e, ElementEqFn eq)
  { return _lastIndexOf((void*)self, sizeof(T), e, (TypeInfo_EqFn)eq); }

  static FOG_INLINE sysuint_t indexOfAt(const Sequence<T>* self, sysuint_t index, sysuint_t range, const T* e, ElementEqFn eq)
  { return _indexOfAt((void*)self, sizeof(T), index, range, e, (TypeInfo_EqFn)eq); }

  static FOG_INLINE sysuint_t lastIndexOfAt(const Sequence<T>* self, sysuint_t index, sysuint_t range, const T* e, ElementEqFn eq)
  { return _lastIndexOfAt((void*)self, sizeof(T), index, range, e, (TypeInfo_EqFn)eq); }
};

template<typename T>
struct SequenceAPI_Wrapper<T, 1> : public SequenceAPI_Base
{
  typedef bool (*ElementEqFn)(const T* a, const T* b);
  typedef int (*ElementCompareFn)(const T* a, const T* b);

  // Sequence<T> functions
  static FOG_INLINE void detach(Sequence<T>* self)
  { _detach_c((void*)self, SequenceInfo<T>::vtable()); }

  static FOG_INLINE void squeeze(Sequence<T>* self)
  { _squeeze_c((void*)self, SequenceInfo<T>::vtable()); }

  static FOG_INLINE void clear(Sequence<T>* self)
  { _clear_c((void*)self, SequenceInfo<T>::vtable()); }

  static FOG_INLINE void free(Sequence<T>* self)
  { _free_c((void*)self, SequenceInfo<T>::vtable()); }

  static FOG_INLINE void swap(Sequence<T>* self, sysuint_t index1, sysuint_t index2)
  { _swap_c((void*)self, SequenceInfo<T>::vtable(), index1, index2); }

  static FOG_INLINE void sort(Sequence<T>* self, ElementCompareFn compar)
  { _sort_c((void*)self, SequenceInfo<T>::vtable(), (TypeInfo_CompareFn)compar); }

  static FOG_INLINE void reverse(Sequence<T>* self)
  { _reverse_c((void*)self, SequenceInfo<T>::vtable()); }

  static FOG_INLINE sysuint_t indexOf(const Sequence<T>* self, const T* e, ElementEqFn eq)
  { return _indexOf((void*)self, sizeof(T), e, (TypeInfo_EqFn)eq); }

  static FOG_INLINE sysuint_t lastIndexOf(const Sequence<T>* self, const T* e, ElementEqFn eq)
  { return _lastIndexOf((void*)self, sizeof(T), e, (TypeInfo_EqFn)eq); }

  static FOG_INLINE sysuint_t indexOfAt(const Sequence<T>* self, sysuint_t index, sysuint_t range, const T* e, ElementEqFn eq)
  { return _indexOfAt((void*)self, sizeof(T), index, range, e, (TypeInfo_EqFn)eq); }

  static FOG_INLINE sysuint_t lastIndexOfAt(const Sequence<T>* self, sysuint_t index, sysuint_t range, const T* e, ElementEqFn eq)
  { return _lastIndexOfAt((void*)self, sizeof(T), index, range, e, (TypeInfo_EqFn)eq); }
};

template<typename T>
struct SequenceAPI_Wrapper<T, 2> : public SequenceAPI_Wrapper<T, 1> 
{
};

template<typename T>
struct SequenceAPI : public SequenceAPI_Wrapper<T, TypeInfo<T>::Type> {};

// ===========================================================================
// [Fog::SequencePrivate]
// ===========================================================================

struct FOG_HIDDEN SequencePrivate
{
  struct Data
  {
    //! @brief Flags.
    enum
    {
      //! @brief Null 'd' object. 
      //!
      //! This is very likely object that's shared between all null objects. So
      //! normally only one data instance can has this flag set on.
      IsNull = (1U << 0)
    };

    //! @brief Reference count.
    Atomic<sysuint_t> refCount;

    //! @brief Flags
    uint32_t flags;

#if FOG_ARCH_BITS == 64
    uint32_t _data_padding;
#endif // FOG_ARCH_BITS == 64

    //! @brief Capacity.
    sysuint_t capacity;
    //! @brief Elements in list.
    sysuint_t length;

    //! @brief Start index in data.
    //!
    //! In Fog::Vector<T>::Data is startIndex always equal to 0.
    //!
    //! In Fog::List<T>::Data is startIndex variable.
    sysuint_t startIndex;
    //! @brief Compatibility with ListBase::UntypedData::endIndex
    //!
    //! In Fog::Vector<T>::Data is endIndex always equal to 'length'.
    //!
    //! In Fog::List<T>::Data is endIndex variable, but never lower than 
    //! 'startIndex'. Also endIndex - startIndex is equal to length and
    //! endIndex is never larger than capacity.
    sysuint_t endIndex;
  };

  struct UntypedData : public Data
  {
    //! @brief Raw untyped pointer to data inlined to this structure.
    uint8_t data[sizeof(void*)];

    //! @brief Used to reference data if we know that flag 
    //! IsSharable is set.
    UntypedData* REF()
    {
      refCount.inc();
      return this;
    }
  };

  template<typename T>
  struct TypedData : public Data
  {
    //! @brief Raw typed pointer to data inlined to this structure.
    T data[1];

    //! @brief Used to increase reference count of data
    TypedData<T>* REF()
    {
      refCount.inc();
      return this;
    }

    void DEREF()
    {
      if (refCount.deref())
      {
        if (!TypeInfo<T>::IsPrimitive) SequenceInfo<T>::free(data + startIndex, length);
        Memory::free(this);
      }
    }
  };

  template<typename T>
  static FOG_INLINE TypedData<T>* sharedNullT()
  { return reinterpret_cast<TypedData<T>*>(Core_Sequence_sharedNull); }
};

// ===========================================================================
// [Fog::SequenceUntyped]
// ===========================================================================

struct FOG_HIDDEN SequenceUntyped
{
  // [Function Prototypes]

  typedef bool (*EqFn)(const void* a, const void* b);
  typedef int (*CompareFn)(const void* a, const void* b);

  // [Types]

  typedef SequencePrivate::UntypedData Data;

  // [Members]

  FOG_DECLARE_D(Data)

  static FOG_INLINE Data* sharedNull()
  { return reinterpret_cast<Data*>(Core_Sequence_sharedNull); }

  // [Implicit Sharing]

  //! @copydoc Doxygen::Implicit::refCount().
  FOG_INLINE sysuint_t refCount() const { return _d->refCount.get(); }
  //! @copydoc Doxygen::Implicit::isDetached().
  FOG_INLINE bool isDetached() const { return _d->refCount.get() == 1; }

  // [Flags]

  //! @brief Returns object flags.
  FOG_INLINE uint32_t flags() const { return _d->flags; }
  //! @copydoc Doxygen::Implicit::isNull().
  FOG_INLINE bool isNull() const { return (_d->flags & Data::IsNull) != 0; }

  // [Container]

  FOG_INLINE sysuint_t length() const { return _d->length; }
  FOG_INLINE bool isEmpty() const { return _d->length == 0; }
};

//! @}

//! @addtogroup Fog_Core
//! @{

// ===========================================================================
// [Fog::Sequence<T>]
// ===========================================================================

template<typename T>
struct Sequence
{
  // [Function Prototypes]

  typedef bool (*ElementEqFn)(const T* a, const T* b);
  typedef int (*ElementCompareFn)(const T* a, const T* b);

  // [Typedefs]

  typedef SequencePrivate::TypedData<T> Data;

  // [Members]

  FOG_DECLARE_D(Data)

  static FOG_INLINE Data* sharedNull()
  { return SequencePrivate::sharedNullT<T>(); }

  // [Construction / Destruction]

  FOG_INLINE Sequence() : _d(sharedNull()->REF()) {}
  FOG_INLINE Sequence(const Sequence& other) : _d(other._d->REF()) {}
  FOG_INLINE Sequence(Data* d) : _d(d) {}
  FOG_INLINE ~Sequence() { _d->DEREF(); }

  // [Implicit Sharing]

  //! @copydoc Doxygen::Implicit::refCount().
  FOG_INLINE sysuint_t refCount() const { return _d->refCount.get(); }
  //! @copydoc Doxygen::Implicit::isDetached().
  FOG_INLINE bool isDetached() const { return _d->refCount.get() == 1; }

  FOG_INLINE void detach()
  { if (!isDetached()) SequenceAPI<T>::detach(this); }

  // [Flags]

  //! @brief Returns object flags.
  FOG_INLINE uint32_t flags() const { return _d->flags; }
  //! @copydoc Doxygen::Implicit::isNull().
  FOG_INLINE bool isNull() const { return _d->flags.anyOf(Data::IsNull); }

  // [Container]

  FOG_INLINE sysuint_t length() const { return _d->length; }
  FOG_INLINE bool isEmpty() const { return _d->length == 0; }

  FOG_INLINE void squeeze()
  { SequenceAPI<T>::squeeze(this); }

  // [Data]

  FOG_INLINE const T* cData() const
  {
    return _d->data + _d->startIndex;
  }

  FOG_INLINE T* mData()
  {
    detach();
    return _d->data + _d->startIndex;
  }

  FOG_INLINE T* xData()
  {
    FOG_ASSERT_X(isDetached(), "Fog::Sequence<T>::xData() - Non detached data.");
    return _d->data + _d->startIndex;
  }

  FOG_INLINE const T& cAt(sysuint_t index) const
  {
    FOG_ASSERT_X(index < length(), "Fog::Sequence<T>::cAt() - Index out of range.");
    return _d->data[_d->startIndex + index];
  }

  FOG_INLINE T& mAt(sysuint_t index)
  {
    FOG_ASSERT_X(index < length(), "Fog::Sequence<T>::mAt() - Index out of range.");
    detach();
    return _d->data[_d->startIndex + index];
  }

  FOG_INLINE T& xAt(sysuint_t index)
  {
    FOG_ASSERT_X(index < length(), "Fog::Sequence<T>::xAt() - Index out of range.");
    FOG_ASSERT_X(isDetached(), "Fog::Sequence<T>::xAt() - Non detached data.");
    return _d->data[_d->startIndex + index];
  }

  // [Manipulation]

  FOG_INLINE void clear()
  { SequenceAPI<T>::clear(this); }

  FOG_INLINE void free()
  { SequenceAPI<T>::free(this); }

  FOG_INLINE sysuint_t indexOf(const T& element) const
  { return SequenceAPI<T>::indexOf(this, &element, TypeInfo_Eq<T>::eq); }

  FOG_INLINE sysuint_t lastIndexOf(const T& element) const
  { return SequenceAPI<T>::lastIndexOf(this, &element, TypeInfo_Eq<T>::eq); }

  FOG_INLINE sysuint_t indexOfAt(sysuint_t index, sysuint_t range, const T& element) const
  { return SequenceAPI<T>::indexOfAt(this, index, range, &element, TypeInfo_Eq<T>::eq); }

  FOG_INLINE sysuint_t lastIndexOfAt(sysuint_t index, sysuint_t range, const T& element) const
  { return SequenceAPI<T>::lastIndexOfAt(this, index, range, &element, TypeInfo_Eq<T>::eq); }

  FOG_INLINE bool contains(const T& element) const
  { return indexOf(element) != InvalidIndex; }

  FOG_INLINE bool containsAt(sysuint_t index, sysuint_t range, const T& element) const
  { return indexOf(index, range, element) != InvalidIndex; }

  FOG_INLINE void swap(sysuint_t index1, sysuint_t index2)
  { SequenceAPI<T>::swap(this, index1, index2); }

  FOG_INLINE void sort()
  { return SequenceAPI<T>::sort(this, TypeInfo_Compare<T>::compare); }

  FOG_INLINE void sort(ElementCompareFn compar)
  { return SequenceAPI<T>::sort(this, compar); }

  FOG_INLINE void reverse()
  { SequenceAPI<T>::reverse(this); }

  // [Iterators]

  //! @brief Read only iterator for Sequence<T> class.
  struct ConstIterator
  {
    const Sequence<T>* _owner;
    const T* _beg;
    const T* _cur;
    const T* _end;
    bool _isValid;

    //! @brief Creates the new list iterator.
    FOG_INLINE ConstIterator(const Sequence<T>& owner) { _owner = &owner; init(); }
    //! @brief Creates the new list iterator.
    FOG_INLINE ConstIterator(const Sequence<T>* owner) { _owner = owner; init(); }
    //! @brief Destroys the list iterator.
    FOG_INLINE ~ConstIterator() {}

    //! @brief Go to the start of list.
    FOG_INLINE ConstIterator& toStart() { _cur = _beg; _isValid = (_cur <= _end); return *this; }
    //! @brief Go to the end of list.
    FOG_INLINE ConstIterator& toEnd() { _cur = _end; _isValid = (_cur >= _beg); return *this; }
    //! @brief Go to the specified index.
    FOG_INLINE ConstIterator& toIndex(sysuint_t index) { _cur = _beg + index; _isValid = (index < _owner->_d->length); return *this; }

    //! @brief Go to the next element.
    FOG_INLINE ConstIterator& toNext() { _cur++; _isValid = (_cur <= _end); return *this; }
    //! @brief Go to the next element.
    FOG_INLINE ConstIterator& toPrevious() { _cur--; _isValid = (_cur >= _beg); return *this; }

    //! @brief Returns @c true if the iterator is at the start of list.
    FOG_INLINE bool atStart() { return _cur == _beg; }
    //! @brief Returns @c true if the iterator is at the end of list.
    FOG_INLINE bool atEnd() { return _cur == _end; }
    //! @brief Returns @c true if the iterator is at the specified index @c index.
    FOG_INLINE bool atIndex(sysuint_t index) { return _cur == _beg + index; }

    //! @brief Returns @c true if iterator position is valid in both sides.
    FOG_INLINE bool isValid() const { return _isValid; }
   
    //! @brief Returns current index.
    FOG_INLINE sysuint_t index() const { return (sysuint_t)(_cur - _beg); }
   
    //! @brief Returns current element.
    FOG_INLINE const T& value() const { return *_cur; }

    // [Operator Overload]

    //! @brief Standard operator that is like @c value().
    FOG_INLINE const T& operator*() const { return *_cur; }

    FOG_INLINE ConstIterator& operator++() { _cur++; _isValid = (_cur <= _end); return *this; }
    FOG_INLINE ConstIterator& operator--() { _cur--; _isValid = (_cur >= _beg); return *this; }
    FOG_INLINE ConstIterator& operator+=(int n) { _cur += n; _isValid = (_cur <= _end); return *this; }
    FOG_INLINE ConstIterator& operator-=(int n) { _cur -= n; _isValid = (_cur >= _beg); return *this; }

    ConstIterator& init(sysuint_t i = 0)
    {
      Data* d = _owner->_d;

      _beg = d->data + d->startIndex;
      _cur = _beg + i;
      _end = _beg + d->length - 1;

      _isValid = (_cur >= _beg && _cur <= _end);

      return *this;
    }
  };
};

//! @}

} // Fog namespace

// ============================================================================
// [Fog::TypeInfo<>]
// ============================================================================

FOG_DECLARE_TYPEINFO_TEMPLATE1(Fog::Sequence,
  typename, T,
  Fog::MoveableType)

// [Guard]
#endif // _FOG_CORE_SEQUENCE_H
