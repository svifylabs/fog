// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_BITARRAY_H
#define _FOG_CORE_BITARRAY_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Core/Assert.h>
#include <Fog/Core/Atomic.h>
#include <Fog/Core/Constants.h>
#include <Fog/Core/Error.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/TypeInfo.h>

//! @addtogroup Fog_Core
//! @{

namespace Fog {

// ============================================================================
// [Fog::BitArray]
// ============================================================================

struct FOG_API BitArray
{
  struct FOG_API Data
  {
    // [Flags]

    //! @brief Basic RefData<T> flags that supports most classes.
    enum Flags
    {
      //! @brief Null 'd' object.
      //!
      //! This is very likely object that's shared between all null objects. So
      //! normally only one data instance can has this flag set on.
      IsNull = (1U << 0),

      //! @brief Object is created on the heap.
      //!
      //! Object is created by function like @c malloc() or Fog::Memory::alloc() or
      //! by @c new operator. It this flag is not set, you can't delete object from
      //! the heap and object is probabbly only temporary (short life).
      IsDynamic = (1U << 1),

      //! @brief Object is shareable.
      //!
      //! Object can be directly referenced by internal method @c ref(). Sharable
      //! objects are usually created on the heap and together with this flag is set
      //! also @c IsDynamic, but it couldn't be.
      IsSharable = (1U << 2),

      //! @brief Prevents destroying from assigning operations.
      //!
      //! This flag is usually only for @c Fog::String family classes and it
      //! it means
      //!   "Don't assign other object to me, instead, copy it to me directly!".
      IsStrong = (1U << 3)
    };

    // [Ref / Deref]

    FOG_INLINE Data* refAlways() const
    {
      refCount.inc();
      return const_cast<Data*>(this);
    }

    Data* ref() const;
    void deref();

    static Data* adopt(void* address, sysuint_t capacity);
    static Data* adopt(void* address, sysuint_t capacity, const uint8_t* data, sysuint_t bitoffset, sysuint_t count);
    static Data* adopt(void* address, sysuint_t capacity, uint32_t bit, sysuint_t count);

    static Data* create(sysuint_t capacity);
    static Data* create(sysuint_t capacity, const uint8_t* data, sysuint_t bitoffset, sysuint_t count);

    static Data* createFor(uint32_t bit, sysuint_t count);
    static Data* createFor(const uint8_t* data, sysuint_t bitoffset, sysuint_t count);
    static Data* createFor2(const uint8_t* data1, sysuint_t bitoffset1, sysuint_t count1, const uint8_t* data2, sysuint_t bitoffset2, sysuint_t count2);

    static Data* copy(const Data* other);

    // [Members]

    mutable Atomic<sysuint_t> refCount;
    uint32_t flags;

    sysuint_t capacity;
    sysuint_t length;
    uint8_t data[4];
  };

  static Static<Data> sharedNull;

  // [Construction / Destruction]

  BitArray();
  BitArray(const BitArray& other);
  explicit BitArray(Data* d);
  ~BitArray();

  // [Implicit Sharing]

  //! @copydoc Doxygen::Implicit::refCount().
  FOG_INLINE sysuint_t refCount() const { return _d->refCount.get(); }
  //! @copydoc Doxygen::Implicit::isDetached().
  FOG_INLINE bool isDetached() const { return refCount() == 1; }
  //! @copydoc Doxygen::Implicit::detach().
  FOG_INLINE err_t detach() { return !isDetached() ? _detach() : (err_t)Error::Ok; }
  //! @copydoc Doxygen::Implicit::_detach().
  err_t _detach();
  //! @copydoc Doxygen::Implicit::free().
  void free();

  // [Flags]

  //! @copydoc Doxygen::Implicit::flags().
  FOG_INLINE uint32_t flags() const { return _d->flags; }
  //! @copydoc Doxygen::Implicit::isDynamic().
  FOG_INLINE bool isDynamic() const { return (_d->flags & Data::IsDynamic) != 0; }
  //! @copydoc Doxygen::Implicit::isSharable().
  FOG_INLINE bool isSharable() const { return (_d->flags & Data::IsSharable) != 0; }
  //! @copydoc Doxygen::Implicit::isNull().
  FOG_INLINE bool isNull() const { return (_d->flags & Data::IsNull) != 0; }
  //! @copydoc Doxygen::Implicit::isStrong().
  FOG_INLINE bool isStrong() const { return (_d->flags & Data::IsStrong) != 0; }

  // [Data]

  //! @brief Returns const pointer to bit array data (bits).
  FOG_INLINE const uint8_t* cData() const
  {
    return _d->data;
  }

  //! @brief Returns mutable pointer to bit array data (bits).
  //! 
  //! To ensure that data can be overwritten, @c detach()
  //! is called internally.

  FOG_INLINE uint8_t* mData()
  {
    return detach() == Error::Ok ? _d->data : NULL;
  }

  FOG_INLINE uint8_t* xData()
  {
    FOG_ASSERT_X(isDetached(), "Fog::BitArray::xData() - Non detached data.");
    return _d->data;
  }

  FOG_INLINE bool at(sysuint_t index) const
  {
    FOG_ASSERT_X(index < length(), "Fog::BitArray::at() - Index out of range.");
    return (bool)(_d->data[index >> 3] & (1 << (index & 7)));
  }

  //! @brief Returns count of allocated bits (capacity).
  FOG_INLINE sysuint_t capacity() const { return _d->capacity; }
  //! @brief Returns count of used bits (length).
  FOG_INLINE sysuint_t length() const { return _d->length; }
  //! @brief Returns true if bit array is empty.
  FOG_INLINE bool isEmpty() const { return length() == 0; }

  err_t reserve(sysuint_t to);
  err_t resize(sysuint_t to, uint32_t fill = 0);
  err_t grow(sysuint_t by);

  void squeeze();
  void clear();

  // [Single bit manipulation]

  err_t setBit(sysuint_t index);
  err_t setBit(sysuint_t index, uint32_t bit);
  err_t resetBit(sysuint_t index);
  err_t invertBit(sysuint_t index);

  // [Multiple bits manipulation]

  err_t fill(uint32_t bit);
  err_t fillAt(sysuint_t index, sysuint_t count, uint32_t bit);
  err_t invert();
  err_t invertAt(sysuint_t index, sysuint_t count);

  // [Set]

  err_t set(const BitArray& other);

  // [Append]

  err_t append(uint32_t bit);
  err_t append(uint32_t bit, sysuint_t count);
  err_t append(const BitArray& other);

  // [Prepend]

  err_t prepend(uint32_t bit);
  err_t prepend(uint32_t bit, sysuint_t count);
  err_t prepend(const BitArray& other);

  // [Insert]

  err_t insert(sysuint_t index, uint32_t bit);
  err_t insert(sysuint_t index, uint32_t bit, sysuint_t count);
  err_t insert(sysuint_t index, const BitArray& other);

  // [Overloaded Operators]

  FOG_INLINE BitArray& operator=(const BitArray& other) { set(other); return *this; }
  FOG_INLINE BitArray& operator+=(const BitArray& other) { append(other); return *this; }

  // [Comparing]

  FOG_INLINE bool eq(const BitArray& other) const { return eq(this, &other); }
  FOG_INLINE int compare(const BitArray& other) const { return compare(this, &other); }

  static bool eq(const BitArray* a, const BitArray* b);
  static int compare(const BitArray* a, const BitArray* b);

  // [Members]

  FOG_DECLARE_D(Data)
};

// [Fog::TemporaryBitArray<N>]

/*
template<sysuint_t N>
struct Fog::TemporaryBitArray : public Fog::BitArray
{
  // Keep 'Storage' name for this struct for Borland compiler
  struct Storage
  {
    Fog::BitArrayData _d;
    char _str[(N + 7) / 8];
  } _storage;
};
*/

} // Fog namespace

static FOG_INLINE const Fog::BitArray operator+(const Fog::BitArray& a, const Fog::BitArray& b) { Fog::BitArray t(a); t += b; return t; }
/*
static FOG_INLINE const Fog::BitArray operator|(const Fog::BitArray& a, const Fog::BitArray& b) { Fog::BitArray t(*this); t |= b; return t; }
static FOG_INLINE const Fog::BitArray operator&(const Fog::BitArray& a, const Fog::BitArray& b) { Fog::BitArray t(*this); t &= b; return t; }
static FOG_INLINE const Fog::BitArray operator^(const Fog::BitArray& a, const Fog::BitArray& b) { Fog::BitArray t(*this); t ^= b; return t; }
*/
static FOG_INLINE bool operator==(const Fog::BitArray& a, const Fog::BitArray& b) { return  a.eq(b); }
static FOG_INLINE bool operator!=(const Fog::BitArray& a, const Fog::BitArray& b) { return !a.eq(b); }
static FOG_INLINE bool operator<=(const Fog::BitArray& a, const Fog::BitArray& b) { return  a.compare(b) <= 0; }
static FOG_INLINE bool operator< (const Fog::BitArray& a, const Fog::BitArray& b) { return  a.compare(b) <  0; }
static FOG_INLINE bool operator>=(const Fog::BitArray& a, const Fog::BitArray& b) { return  a.compare(b) >= 0; }
static FOG_INLINE bool operator> (const Fog::BitArray& a, const Fog::BitArray& b) { return  a.compare(b) >  0; }

//! @}

// [Guard]
#endif // _FOG_CORE_BITARRAY_H
