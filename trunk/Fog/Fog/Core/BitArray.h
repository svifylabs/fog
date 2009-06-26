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
#include <Fog/Core/Memory.h>
#include <Fog/Core/RefData.h>
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
  struct FOG_API Data : public RefData<Data>
  {
    sysuint_t capacity;
    sysuint_t length;
    uint8_t data[4];

    Data* ref();
    void deref();

    FOG_INLINE void destroy() {}
    FOG_INLINE void free() { Fog::Memory::free(this); }

    static Data* adopt(void* address, sysuint_t capacity);
    static Data* adopt(void* address, sysuint_t capacity, const uint8_t* data, sysuint_t bitoffset, sysuint_t count);
    static Data* adopt(void* address, sysuint_t capacity, uint32_t bit, sysuint_t count);

    static Data* create(sysuint_t capacity, uint allocPolicy);
    static Data* create(sysuint_t capacity, const uint8_t* data, sysuint_t bitoffset, sysuint_t count);

    static Data* createFor(uint32_t bit, sysuint_t count);
    static Data* createFor(const uint8_t* data, sysuint_t bitoffset, sysuint_t count);
    static Data* createFor2(const uint8_t* data1, sysuint_t bitoffset1, sysuint_t count1, const uint8_t* data2, sysuint_t bitoffset2, sysuint_t count2);

    static Data* copy(const Data* other, uint allocPolicy);
  };

  static Static<Data> sharedNull;

  // [Construction / Destruction]

  BitArray();
  BitArray(Data* d);
  BitArray(const BitArray& other);
  ~BitArray();

  // [Implicit Sharing]

  //! @copydoc Doxygen::Implicit::refCount().
  FOG_INLINE sysuint_t refCount() const { return _d->refCount.get(); }
  //! @copydoc Doxygen::Implicit::isDetached().
  FOG_INLINE bool isDetached() const { return refCount() == 1; }
  //! @copydoc Doxygen::Implicit::detach().
  FOG_INLINE void detach() { if (!isDetached()) _detach(); }
  //! @copydoc Doxygen::Implicit::tryDetach().
  FOG_INLINE bool tryDetach() { return (!isDetached()) ? _tryDetach() : true; }
  //! @copydoc Doxygen::Implicit::_detach().
  void _detach();
  //! @copydoc Doxygen::Implicit::_tryDetach().
  bool _tryDetach();
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
    detach();
    return _d->data;
  }

  FOG_INLINE uint8_t* xData()
  {
    FOG_ASSERT_X(isDetached(), "Fog::BitArray::xData() - Non detached data.");
    return _d->data;
  }

  FOG_INLINE bool at(sysuint_t index) const
  {
    FOG_ASSERT_X(index < length(), "Fog::BitArray::at() - Index out of range.");
    return (_d->data[index >> 3] & (1 << (index & 7))) != 0;
  }

  //! @brief Returns count of allocated bits (capacity).
  FOG_INLINE sysuint_t capacity() const { return _d->capacity; }
  //! @brief Returns count of used bits (length).
  FOG_INLINE sysuint_t length() const { return _d->length; }
  //! @brief Returns true if bit array is empty.
  FOG_INLINE bool isEmpty() const { return length() == 0; }

  FOG_INLINE void reserve(sysuint_t to)
  { _reserve(to); }
  FOG_INLINE void resize(sysuint_t to, uint32_t fill = 0)
  { _resize(to); }
  FOG_INLINE void grow(sysuint_t by)
  { _grow(by); }

  FOG_INLINE bool tryReserve(sysuint_t to)
  { return _tryReserve(to) != 0; }
  FOG_INLINE bool tryResize(sysuint_t to, uint32_t fill = 0)
  { return _tryResize(to) != 0; }
  FOG_INLINE bool tryGrow(sysuint_t by)
  { return _tryGrow(by) != 0; }

  uint8_t* _reserve(sysuint_t to);
  uint8_t* _resize(sysuint_t to, uint32_t fill = 0);
  uint8_t* _grow(sysuint_t by);

  uint8_t* _tryReserve(sysuint_t to);
  uint8_t* _tryResize(sysuint_t to, uint32_t fill = 0);
  uint8_t* _tryGrow(sysuint_t by);

  void squeeze();

  void clear();

  // [Single bit manipulation]

  BitArray& setBit(sysuint_t index);
  BitArray& setBit(sysuint_t index, uint32_t bit);
  BitArray& resetBit(sysuint_t index);
  BitArray& invertBit(sysuint_t index);

  // [Multiple bits manipulation]

  BitArray& fill(uint32_t bit);
  BitArray& fillAt(sysuint_t index, sysuint_t count, uint32_t bit);
  BitArray& invert();
  BitArray& invertAt(sysuint_t index, sysuint_t count);

  // [Set]

  BitArray& set(const BitArray& other);

  // [Append]

  BitArray& append(uint32_t bit);
  BitArray& append(uint32_t bit, sysuint_t count);
  BitArray& append(const BitArray& other);

  // [Prepend]

  BitArray& prepend(uint32_t bit);
  BitArray& prepend(uint32_t bit, sysuint_t count);
  BitArray& prepend(const BitArray& other);

  // [Insert]

  BitArray& insert(sysuint_t index, uint32_t bit);
  BitArray& insert(sysuint_t index, uint32_t bit, sysuint_t count);
  BitArray& insert(sysuint_t index, const BitArray& other);

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
