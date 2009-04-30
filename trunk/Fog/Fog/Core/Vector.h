// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_VECTOR_H
#define _FOG_CORE_VECTOR_H

// [Dependencies]
#include <Fog/Core/Assert.h>
#include <Fog/Core/Atomic.h>
#include <Fog/Build/Build.h>
#include <Fog/Core/Error.h>
#include <Fog/Core/Flags.h>
#include <Fog/Core/Sequence.h>
#include <Fog/Core/SequenceInfo.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/Std.h>
#include <Fog/Core/TypeInfo.h>

#include <new>

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

template<typename T> struct List;
template<typename T> struct Vector;

// ============================================================================
// [Fog::VectorAPI]
// ============================================================================

struct FOG_API VectorAPI_Base
{
  // [Primitive Data Type]
  static void _reserve_p(void* self, sysuint_t sizeOfT, sysuint_t to);
  static void _grow_p(void* self, sysuint_t sizeOfT, sysuint_t by);

  static void _assign_p(void* self, sysuint_t sizeOfT, const void* data, sysuint_t length);
  static void _append_p(void* self, sysuint_t sizeOfT, const void* data, sysuint_t length);
  static void _insert_p(void* self, sysuint_t sizeOfT, sysuint_t index, const void* data, sysuint_t length);
  static sysuint_t _removeAt_p(void* self, sysuint_t sizeOfT, sysuint_t index, sysuint_t range);
  static sysuint_t _replace_p(void* self, sysuint_t sizeOfT, sysuint_t index, sysuint_t range, const void* data, sysuint_t length);

  // [Class Data Type]
  static void _reserve_c(void* self, const SequenceInfoVTable* vtable, sysuint_t to);
  static void _grow_c(void* self, const SequenceInfoVTable* vtable, sysuint_t by);

  static void _assign_c(void* self, const SequenceInfoVTable* vtable, const void* data, sysuint_t length);
  static void _append_c(void* self, const SequenceInfoVTable* vtable, const void* data, sysuint_t length);
  static void _insert_c(void* self, const SequenceInfoVTable* vtable, sysuint_t index, const void* data, sysuint_t length);
  static sysuint_t _removeAt_c(void* self, const SequenceInfoVTable* vtable, sysuint_t index, sysuint_t range);
  static sysuint_t _replace_c(void* self, const SequenceInfoVTable* vtable, sysuint_t index, sysuint_t range, const void* data, sysuint_t length);
};

// 0 = PrimitiveType
// 1 = MoveableType
// 2 = ClassType
template<typename T, uint __TypeInfo__>
struct VectorAPI_Wrapper {};

template<typename T>
struct VectorAPI_Wrapper<T, 0> : public SequenceAPI<T>, public VectorAPI_Base
{
  // Vector<T> functions
  static FOG_INLINE void reserve(Vector<T>* self, sysuint_t to)
  { _reserve_p((void*)self, sizeof(T), to); }

  static FOG_INLINE void grow(Vector<T>* self, sysuint_t by)
  { _grow_p((void*)self, sizeof(T), by); }

  static FOG_INLINE void assign(Vector<T>* self, const T* data, sysuint_t length)
  { _assign_p((void*)self, sizeof(T), (const void*)data, length); }

  static FOG_INLINE void append(Vector<T>* self, const T* data, sysuint_t length)
  { _append_p((void*)self, sizeof(T), (const void*)data, length); }

  static FOG_INLINE void insert(Vector<T>* self, sysuint_t index, const T* data, sysuint_t length)
  { _insert_p((void*)self, sizeof(T), index, (const void*)data, length); }

  static FOG_INLINE sysuint_t removeAt(Vector<T>* self, sysuint_t index, sysuint_t range)
  { return _removeAt_p((void*)self, sizeof(T), index, range); }

  static FOG_INLINE sysuint_t replace(Vector<T>* self, sysuint_t index, sysuint_t range, const T* data, sysuint_t length)
  { return _replace_p((void*)self, sizeof(T), index, range, (const void*)data, length); }
};

template<typename T>
struct VectorAPI_Wrapper<T, 1> : public SequenceAPI<T>, public VectorAPI_Base
{
  // Vector<T> functions
  static FOG_INLINE void reserve(Vector<T>* self, sysuint_t to)
  { _reserve_c((void*)self, SequenceInfo<T>::vtable(), to); }

  static FOG_INLINE void grow(Vector<T>* self, sysuint_t by)
  { _grow_c((void*)self, SequenceInfo<T>::vtable(), by); }

  static FOG_INLINE void assign(Vector<T>* self, const T* data, sysuint_t length)
  { _assign_c((void*)self, SequenceInfo<T>::vtable(), (const void*)data, length); }

  static FOG_INLINE void append(Vector<T>* self, const T* data, sysuint_t length)
  { _append_c((void*)self, SequenceInfo<T>::vtable(), (const void*)data, length); }

  static FOG_INLINE void insert(Vector<T>* self, sysuint_t index, const T* data, sysuint_t length)
  { _insert_c((void*)self, SequenceInfo<T>::vtable(), index, (const void*)data, length); }

  static FOG_INLINE sysuint_t removeAt(Vector<T>* self, sysuint_t index, sysuint_t range = DetectLength)
  { return _removeAt_c((void*)self, SequenceInfo<T>::vtable(), index, range); }

  static FOG_INLINE sysuint_t replace(Vector<T>* self, sysuint_t index, sysuint_t range, const T* data, sysuint_t length)
  { return _replace_c((void*)self, SequenceInfo<T>::vtable(), index, range, (const void*)data, length); }
};

template<typename T>
struct VectorAPI_Wrapper<T, 2> : public VectorAPI_Wrapper<T, 1> 
{
};

template<typename T>
struct VectorAPI : public VectorAPI_Wrapper<T, TypeInfo<T>::Type> {};

// ============================================================================
// [Fog::Vector<T>]
// ============================================================================

//! @brief Vector is container template similar to std::vector.
//!
//! This implementation is based on implicit sharing and copy-on-write. 
//! Another great feature is that code generated by compiler should be
//! smaller than code needed for std::vector, because Fog::Vector uses
//! some tricks to do that (exported symbols for basic types in library 
//! and intelligent template design for Primitive, Moveable and Class
//! types).
template<typename T>
struct Vector : public Sequence<T>
{
  using Sequence<T>::_d;

  // Next line not works for GCC
  // using Sequence<T>::Data;
  typedef SequencePrivate::TypedData<T> Data;

  // [Construction / Destruction]

  FOG_INLINE Vector() : Sequence<T>() {}
  FOG_INLINE Vector(const Vector& other) : Sequence<T>(other._d->REF()) {}
  FOG_INLINE Vector(Data* d) : Sequence<T>(d) {}
  FOG_INLINE ~Vector() {} // 'd' is dereferenced in ~Sequence<T>

  // [Data]

  FOG_INLINE const T* cData() const
  {
    return _d->data;
  }

  FOG_INLINE T* mData()
  {
    Sequence<T>::detach();
    return _d->data;
  }

  FOG_INLINE T* xData()
  {
    FOG_ASSERT_X(Sequence<T>::isDetached(), "Fog::Vector<T>::xData() - Non detached data.");
    return _d->data;
  }

  FOG_INLINE const T& cAt(sysuint_t index) const
  {
    FOG_ASSERT_X(index < Sequence<T>::length(), "Fog::Vector<T>::cAt() - Index out of range.");
    return _d->data[index];
  }

  FOG_INLINE T& mAt(sysuint_t index)
  {
    FOG_ASSERT_X(index < Sequence<T>::length(), "Fog::Vector<T>::mAt() - Index out of range.");
    Sequence<T>::detach();
    return _d->data[index];
  }

  FOG_INLINE T& xAt(sysuint_t index)
  {
    FOG_ASSERT_X(index < Sequence<T>::length(), "Fog::Vector<T>::xAt() - Index out of range.");
    FOG_ASSERT_X(Sequence<T>::isDetached(), "Fog::Vector<T>::xAt() - Non detached data.");
    return _d->data[index];
  }

  // [Container]

  FOG_INLINE sysuint_t capacity() const
  { return _d->capacity; }

  FOG_INLINE void reserve(sysuint_t to)
  { VectorAPI<T>::reserve(this, to); }

  FOG_INLINE void grow(sysuint_t by)
  { VectorAPI<T>::grow(this, by); }

  // [Manipulation]

  FOG_INLINE void assign(const T& element)
  { VectorAPI<T>::assign(element); }

  FOG_INLINE void assign(const Vector<T>& other)
  { AtomicBase::ptr_setXchg(&_d, other._d->REF())->DEREF(); }

  FOG_INLINE void set(sysuint_t index, const T& element)
  { mAt(index) = element; }

  FOG_INLINE void append(const T& element)
  { VectorAPI<T>::append(this, &element, 1); }

  FOG_INLINE void append(const T* data, sysuint_t length)
  { VectorAPI<T>::append(this, data, length); }

  FOG_INLINE void append(const Vector<T>& other)
  { Vector<T> t(other); append(t.cData(), t.length()); }

  FOG_INLINE void prepend(const T& element)
  { VectorAPI<T>::insert(this, 0, &element, 1); }

  FOG_INLINE void prepend(const T* data, sysuint_t length)
  { VectorAPI<T>::insert(this, 0, data, length); }

  FOG_INLINE void prepend(const Vector<T>& other)
  { Vector<T> t(other); prepend(t.cData(), t.length()); }

  FOG_INLINE void insert(sysuint_t index, const T& element)
  { VectorAPI<T>::insert(this, index, &element, 1); }

  FOG_INLINE void insert(sysuint_t index, const T* data, sysuint_t length)
  { VectorAPI<T>::insert(this, index, data, length); }

  FOG_INLINE void insert(sysuint_t index, const Vector<T>& other)
  { Vector<T> t(other); insert(index, t.cData(), t.length()); }

  FOG_INLINE sysuint_t remove(const T& element)
  { return removeAt(indexOf(element)); } 

  FOG_INLINE sysuint_t removeAt(sysuint_t index, sysuint_t range = 1)
  { return VectorAPI<T>::removeAt(this, index, range); }

  FOG_INLINE bool removeFirst()
  { return removeAt(0, 1) == 1; }

  FOG_INLINE bool removeLast()
  { return removeAt(Sequence<T>::length()-1, 1) == 1; }

  FOG_INLINE sysuint_t replace(sysuint_t index, sysuint_t range, const T* data, sysuint_t length)
  { return VectorAPI<T>::replace(this, index, range, data, length); }

  FOG_INLINE sysuint_t replace(sysuint_t index, sysuint_t range, const Vector<T>& other)
  { Vector<T> t(other); return replace(index, range, t.cData(), t.length()); }

  T takeAt(sysuint_t index)
  {
    FOG_ASSERT_X(index < Sequence<T>::length(), "Fog::Vector<T>::takeAt() - Index out of range.");
    T result = cData()[index];
    removeAt(index);
    return result;
  }

  T takeFirst() { return takeAt(0); }
  T takeLast() { return takeAt(Sequence<T>::length()-1); }
  T top() const { return cAt(Sequence<T>::length()-1); }

  void pop() { removeAt(Sequence<T>::length()-1, 1); }

  // [Operator Overload]

  FOG_INLINE const T& operator[](sysuint_t index) const
  { return cAt(index); }

  FOG_INLINE T& operator[](sysuint_t index)
  { return mAt(index); }

  FOG_INLINE Vector<T>& operator=(const T& element)
  { assign(element); return *this; }

  FOG_INLINE Vector<T>& operator=(const Vector<T>& other)
  { assign(other); return *this; }

  FOG_INLINE Vector<T>& operator+=(const T& element)
  { append(element); return *this; }

  FOG_INLINE Vector<T>& operator+=(const Vector<T>& other)
  { append(other); return *this; }

  // [Iterators]

  //! @brief Read only iterator.
  struct ConstIterator
  {
    const Vector<T>* _owner;
    const T* _beg;
    const T* _cur;
    const T* _end;
    bool _isValid;

    //! @brief Creates the new list iterator.
    FOG_INLINE ConstIterator(const Vector<T>& owner) { _owner = &owner; init(); }
    //! @brief Creates the new list iterator.
    FOG_INLINE ConstIterator(const Vector<T>* owner) { _owner = owner; init(); }
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
     
      _beg = d->data;
      _cur = _beg + i;
      _end = _beg + d->length - 1;
     
      _isValid = (_cur >= _beg && _cur <= _end);
     
      return *this;
    }
  };
 
  //! @brief Read / Write iterator.
  struct MutableIterator
  {
    Vector<T>* _owner;
    T* _beg;
    T* _cur;
    T* _end;
    bool _isValid;
   
    //! @brief Creates the new list iterator.
    FOG_INLINE MutableIterator(Vector<T>& owner) { _owner = &owner; _owner->detach(); init(); }
    //! @brief Creates the new list iterator.
    FOG_INLINE MutableIterator(Vector<T>* owner) { _owner = owner; _owner->detach(); init(); }
    //! @brief Destroys the list iterator.
    FOG_INLINE ~MutableIterator() {}
   
    //! @brief Go to the start of list.
    FOG_INLINE MutableIterator& toStart() { _cur = _beg; _isValid = (_cur <= _end); return *this; }
    //! @brief Go to the end of list.
    FOG_INLINE MutableIterator& toEnd() { _cur = _end; _isValid = (_cur >= _beg); return *this; }
    //! @brief Go to the specified index.
    FOG_INLINE MutableIterator& toIndex(sysuint_t index) { _cur = _beg + index; _isValid = (index < _owner->_d->length); return *this; }

    //! @brief Go to the next element.
    FOG_INLINE MutableIterator& toNext() { _cur++; _isValid = (_cur <= _end); return *this; }
    //! @brief Go to the next element.
    FOG_INLINE MutableIterator& toPrevious() { _cur--; _isValid = (_cur <= _end); return *this; }

    //! @brief Returns @c true if the iterator is at the start of list.
    FOG_INLINE bool atStart() { return _cur == _beg; }
    //! @brief Returns @c true if the iterator is at the end of list.
    FOG_INLINE bool atEnd() { return _cur == _end; }
    //! @brief Returns @c true if the iterator is at the specified index @c index.
    FOG_INLINE bool atIndex(sysuint_t index) { return _cur == (_beg + index); }

    //! @brief Returns @c true if iterator position is valid in both sides.
    FOG_INLINE bool isValid() const { return _isValid; }
   
    //! @brief Returns current index.
    FOG_INLINE sysuint_t index() const { return (sysuint_t)(_cur - _beg); }
   
    //! @brief Returns current element.
    FOG_INLINE T& value() const { return *_cur; }
   
    // [Operator Overload]
   
    //! @brief Standard operator that is like @c value().
    FOG_INLINE const T& operator*() const { return *_cur; }
   
    FOG_INLINE MutableIterator& operator++() { _cur++; _isValid = (_cur <= _end); return *this; }
    FOG_INLINE MutableIterator& operator--() { _cur--; _isValid = (_cur <= _beg); return *this; }
    FOG_INLINE MutableIterator& operator+=(int n) { _cur += n; _isValid = (_cur <= _end); return *this; }
    FOG_INLINE MutableIterator& operator-=(int n) { _cur -= n; _isValid = (_cur >= _beg); return *this; }
   
    // [Manipulators]

    MutableIterator& remove()
    {
      FOG_ASSERT_X(isValid(), "Fog::Vector<T>::MutableIterator::remove() - Iterator out of range");

      sysuint_t i = index();
      _owner->remove(i); 
      return init(i);
    }
   
    T take()
    {
      FOG_ASSERT_X(isValid(), "Fog::Vector<T>::MutableIterator::take() - Iterator out of range");

      sysuint_t i = index();
      T element = _owner->takeAt(i);
      init(i);

      return element;
    }
   
    MutableIterator& init(sysuint_t i = 0)
    {
      Data* d = _owner->_d;

      _beg = d->data;
      _cur = _beg + i;
      _end = _beg + d->length - 1;

      _isValid = (_cur >= _beg && _cur <= _end);

      return *this;
    }
  };
};

} // Fog namespace

// ============================================================================
// [Fog::TypeInfo<>]
// ============================================================================

FOG_DECLARE_TYPEINFO_TEMPLATE1(Fog::Vector, typename, T, Fog::MoveableType)

// [Guard]
#endif // _FOG_CORE_VECTOR_H
