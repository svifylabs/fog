// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_REGION_H
#define _FOG_GRAPHICS_REGION_H

// [Dependencies]
#include <Fog/Core/Atomic.h>
#include <Fog/Core/Error.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/Static.h>
#include <Fog/Graphics/Geometry.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Fog::Region]
// ============================================================================

struct FOG_API Region
{
  // [Data]

  struct FOG_API Data
  {
    // [Ref / Deref]

    FOG_INLINE Data* refAlways() { refCount.inc(); return const_cast<Data*>(this); }
    FOG_INLINE void derefInline() { if (refCount.deref() && (flags & IsDynamic) != 0) Memory::free(this); }

    Data* ref() const;
    void deref();

    static Data* adopt(void* address, sysuint_t capacity);
    static Data* adopt(void* address, sysuint_t capacity, const Box& r);
    static Data* adopt(void* address, sysuint_t capacity, const Box* extents, const Box* rects, sysuint_t count);
    
    static Data* create(sysuint_t capacity);
    static Data* create(sysuint_t capacity, const Box* extents, const Box* rects, sysuint_t count);

    static Data* copy(const Data* other);

    static FOG_INLINE sysuint_t sizeFor(sysuint_t capacity)
    { return sizeof(Data) - sizeof(Box) + sizeof(Box) * capacity; }

    // [Flags]

    //! @brief String data flags.
    enum Flags
    {
      //! @brief Null 'd' object. 
      //!
      //! This is very likely object that's shared between all null objects. So
      //! normally only one data instance can has this flag set on.
      IsNull = (1U << 0),

      //! @brief String data are created on the heap. 
      //!
      //! Object is created by function like @c Fog::Memory::alloc() or by
      //! @c new operator. It this flag is not set, you can't delete object from
      //! the heap and object is probabbly only temporary (short life object).
      IsDynamic = (1U << 1),

      //! @brief String data are shareable.
      //!
      //! Object can be directly referenced by internal method @c ref(). 
      //! Sharable data are usually created on the heap and together 
      //! with this flag is set also @c IsDynamic, but it isn't prerequisite.
      IsSharable = (1U << 2),

      //! @brief String data are strong to weak assignments.
      //!
      //! This flag means:
      //!   "Don't assign other data to me, instead, copy them to me!".
      IsStrong = (1U << 3)
    };

    // [Members]

    //! @brief Reference count.
    mutable Atomic<sysuint_t> refCount;
    //! @brief Region flags.
    uint32_t flags;

    //! @brief Count of preallocated rectangles.
    sysuint_t capacity;
    //! @brief Count of used rectangles.
    sysuint_t length;
    //! @brief Region extents.
    Box extents;
    //! @brief Region rectangles, always YX sorted.
    Box rects[1];
  };

  static Fog::Static<Data> sharedNull;

  // [Construction / Destruction]

  Region();
  Region(const Region& other);
  explicit Region(const Box& rect);
  explicit Region(const Rect& rect);
  explicit Region(Data* d);
  ~Region();

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

  //! @copydoc Doxygen::Implicit::getFlags().
  FOG_INLINE uint32_t getFlags() const { return _d->flags; }
  //! @copydoc Doxygen::Implicit::isDynamic().
  FOG_INLINE bool isDynamic() const { return (_d->flags & Data::IsDynamic) != 0; }
  //! @copydoc Doxygen::Implicit::isSharable().
  FOG_INLINE bool isSharable() const { return (_d->flags & Data::IsSharable) != 0; }
  //! @copydoc Doxygen::Implicit::isNull().
  FOG_INLINE bool isNull() const { return (_d->flags & Data::IsNull) != 0; }
  //! @copydoc Doxygen::Implicit::isStrong().
  FOG_INLINE bool isStrong() const { return (_d->flags & Data::IsStrong) != 0; }

  err_t setSharable(bool val);
  err_t setStrong(bool val);

  // [Data]

  //! @brief Get const pointer to region data.
  FOG_INLINE const Box* cData() const  { return _d->rects; }

  //! @brief Get read / write pointer to region data.
  FOG_INLINE Box* mData() { return detach() == Error::Ok ? _d->rects : (Box*)NULL; }

  //! @brief Get read / write pointer to region data.
  FOG_INLINE Box* xData() 
  {
    FOG_ASSERT_X(isDetached(), "Fog::Region::xData() - Not detached");
    return _d->rects; 
  }

  // [Container]

  //! @brief Returns capacity of region in rectangles.
  FOG_INLINE sysuint_t getCapacity() const  { return _d->capacity; }

  //! @brief Returns count of rectangles in region.
  FOG_INLINE sysuint_t getLength() const  { return _d->length; }

  //! @brief Returns @c true if region is empty.
  FOG_INLINE bool isEmpty() const  { return _d->length == 0; }

  //! @brief Returns region extents.
  FOG_INLINE const Box& extents() const  { return _d->extents; }

  err_t reserve(sysuint_t to);
  err_t prepare(sysuint_t to);
  void squeeze();

  uint32_t getType() const;

  // [Contains]

  //! @brief Region testing.
  enum Contains
  {
    //! @brief Object isn't in region (point, rectangle or another region).
    Out = 0,
    //! @brief Object is in region (point, rectangle or another region).
    In = 1,
    //! @brief Object is partially in region (point, rectangle or another region).
    Part = 2
  };

  //! @brief Tests if a given point is in region.
  //! @return @c In if region contains a given point, otherwise @c Out.
  uint32_t contains(const Point& pt) const;

  FOG_INLINE uint32_t contains(int x, int y) const { return contains(Point(x, y)); }

  uint contains(const Rect& r) const;
  uint contains(const Box& r) const;

  //! @brief Removes all rectagnels from region.
  void clear();

  err_t set(const Region& r);
  err_t set(const Rect& r);
  err_t set(const Box& r);
  
  //! @brief Creates a deep copy instead of reference.
  //!
  //! This function is used internally to optimize computing where we know
  //! that we need to copy region and do some ops with them (Because if we
  //! create a reference, it will free existing allocated block of memory
  //! and it's expensive to alloc it back.
  //!
  //! @param r Region to copy from;
  //! @return Reference to itself.
  err_t setDeep(const Region& r);

  err_t set(const Rect* rects, sysuint_t count);
  err_t set(const Box* rects, sysuint_t count);

  err_t unite(const Region& r);
  err_t unite(const Rect& r);
  err_t unite(const Box& r);

  err_t intersect(const Region& r);
  err_t intersect(const Rect& r);
  err_t intersect(const Box& r);

  err_t eor(const Region& r);
  err_t eor(const Rect& r);
  err_t eor(const Box& r);

  err_t subtract(const Region& r);
  err_t subtract(const Rect& r);
  err_t subtract(const Box& r);

  err_t op(const Region& r, uint _op);
  err_t op(const Rect& r, uint _op);
  err_t op(const Box& r, uint _op);

  err_t translate(const Point& pt);

  FOG_INLINE err_t translate(int x, int y) { return translate(Point(x, y)); }

  err_t shrink(const Point& pt);

  FOG_INLINE err_t shrink(int x, int y) { return shrink(Point(x, y)); }

  err_t frame(const Point& pt);

  FOG_INLINE err_t frame(int x, int y) { return frame(Point(x, y)); }

  err_t round(const Rect& r, uint xradius, uint yradius, bool fill = true);

  err_t polygon(const Point* pts, sysuint_t count, uint fillRule);
  err_t polyPolygon(const Point* src, const sysuint_t* count, sysuint_t polygons, uint fillRule);

  bool eq(const Region& other) const;

  static err_t set(Region& dest, const Region& src);
  static err_t unite(Region& dest, const Region& src1, const Region& src2);
  static err_t intersect(Region& dest, const Region& src1, const Region& src2);
  static err_t eor(Region& dest, const Region& src1, const Region& src2);
  static err_t subtract(Region& dest, const Region& src1, const Region& src2);
  static err_t op(Region& dest, const Region& src1, const Region& src2, uint _op);
  static err_t translate(Region& dest, const Region& src, const Point& pt);
  static err_t shrink(Region& dest, const Region& src, const Point& pt);
  static err_t frame(Region& dest, const Region& src, const Point& pt);

  // Overloaded operators

  FOG_INLINE Region& operator=(const Region& r) { set(r); return *this; }
  FOG_INLINE Region& operator=(const Rect& r) { set(r); return *this; }
  FOG_INLINE Region& operator=(const Box& r) { set(r); return *this; }

  FOG_INLINE Region& operator+=(const Region& r) { unite(r); return *this; }
  FOG_INLINE Region& operator+=(const Rect& r) { unite(r); return *this; }
  FOG_INLINE Region& operator+=(const Box& r) { unite(r); return *this; }

  FOG_INLINE Region& operator|=(const Region& r) { unite(r); return *this; }
  FOG_INLINE Region& operator|=(const Rect& r) { unite(r); return *this; }
  FOG_INLINE Region& operator|=(const Box& r) { unite(r); return *this; }

  FOG_INLINE Region& operator&=(const Region& r) { intersect(r); return *this; }
  FOG_INLINE Region& operator&=(const Rect& r) { intersect(r); return *this; }
  FOG_INLINE Region& operator&=(const Box& r) { intersect(r); return *this; }

  FOG_INLINE Region& operator^=(const Region& r) { eor(r); return *this; }
  FOG_INLINE Region& operator^=(const Rect& r) { eor(r); return *this; }
  FOG_INLINE Region& operator^=(const Box& r) { eor(r); return *this; }

  FOG_INLINE Region& operator-=(const Region& r) { subtract(r); return *this; }
  FOG_INLINE Region& operator-=(const Rect& r) { subtract(r); return *this; }
  FOG_INLINE Region& operator-=(const Box& r) { subtract(r); return *this; }

#if defined(FOG_OS_WINDOWS)
  HRGN toHRGN() const;
  err_t fromHRGN(HRGN hrgn);
#endif // FOG_OS_WINDOWS

  //! @brief Region type.
  enum Type
  {
    //! @brief Region is empty
    TypeEmpty = 0,
    //! @brief Region has only one rectangle (rectangular).
    TypeSimple = 1,
    //! @brief Region has more YX sorted rectangles.
    TypeComplex = 2
  };

  //! @brief Region ops.
  enum Op
  {
    //! @brief Copy.*/
    OpCopy = 0,
    //! @brief Union (OR)
    OpUnite = 1,
    //! @brief Intersection (AND).
    OpIntersect = 2,
    //! @brief eXclusive or (XOR).
    OpEor = 3,
    //! @brief eXclusive or (XOR).
    OpXor = 3,
    //! @brief Subtraction (Difference).
    OpSubtract = 4
  };

  // [Members]

  FOG_DECLARE_D(Data)
};

// ============================================================================
// [Fog::TemporaryRegion<N>]
// ============================================================================

template<sysuint_t N>
struct TemporaryRegion : public Region
{
  struct Storage
  {
    Data d;
    // There will be 1 rect more, because some compilers gives me warnings
    // about this construct when <N == 1>
    Box rects[N];
  } _storage;

  FOG_INLINE TemporaryRegion() :
    Region(Data::adopt((void*)&_storage, N))
  {
  }

  FOG_INLINE TemporaryRegion(const TemporaryRegion<N>& other) :
    Region(Data::adopt((void*)&_storage, N, &other._d->extents, other._d->rects, other._d->length))
  {
  }
  
  FOG_INLINE TemporaryRegion(const Region& other) :
    Region(Data::adopt((void*)&_storage, N, &other._d->extents, other._d->rects, other._d->length))
  {
  }

  FOG_INLINE explicit TemporaryRegion(const Box& r) :
    Region(Data::adopt((void*)&_storage, N, r))
  {
  }

  FOG_INLINE explicit TemporaryRegion(const Rect& r) :
    Region(Data::adopt((void*)&_storage, N, Box(r)))
  {
  }

  FOG_INLINE void free()
  {
    if ((void*)_d != (void*)&_storage)
    {
      _d->deref();
      _d = Data::adopt((void*)&_storage, N);
    }
  }

  // These overloads are needed to succesfull use this template (or implicit conversion
  // will break template and new region will be allocated)
  FOG_INLINE TemporaryRegion& operator=(const TemporaryRegion<N>& r)
  {
    return reinterpret_cast<TemporaryRegion&>(set(r));
  }

  FOG_INLINE TemporaryRegion& operator=(const Region& r)
  {
    return reinterpret_cast<TemporaryRegion&>(set(r));
  }

  FOG_INLINE TemporaryRegion& operator=(const Box& r)
  {
    return reinterpret_cast<TemporaryRegion&>(set(r));
  }
};

} // Fog namespace

FOG_INLINE const Fog::Region operator+(const Fog::Region& src1, const Fog::Region& src2) { Fog::Region r; Fog::Region::unite(r, src1, src2); return r; }
FOG_INLINE const Fog::Region operator|(const Fog::Region& src1, const Fog::Region& src2) { Fog::Region r; Fog::Region::unite(r, src1, src2); return r; }
FOG_INLINE const Fog::Region operator&(const Fog::Region& src1, const Fog::Region& src2) { Fog::Region r; Fog::Region::intersect(r, src1, src2); return r; }
FOG_INLINE const Fog::Region operator^(const Fog::Region& src1, const Fog::Region& src2) { Fog::Region r; Fog::Region::eor(r, src1, src2); return r; }
FOG_INLINE const Fog::Region operator-(const Fog::Region& src1, const Fog::Region& src2) { Fog::Region r; Fog::Region::subtract(r, src1, src2); return r; }

FOG_INLINE const Fog::Region operator+(const Fog::Region& src1, const Fog::Rect& src2) { Fog::Region r(src1); r.unite(src2); return r; }
FOG_INLINE const Fog::Region operator|(const Fog::Region& src1, const Fog::Rect& src2) { Fog::Region r(src1); r.unite(src2); return r; }
FOG_INLINE const Fog::Region operator&(const Fog::Region& src1, const Fog::Rect& src2) { Fog::Region r(src1); r.intersect(src2); return r; }
FOG_INLINE const Fog::Region operator^(const Fog::Region& src1, const Fog::Rect& src2) { Fog::Region r(src1); r.eor(src2); return r; }
FOG_INLINE const Fog::Region operator-(const Fog::Region& src1, const Fog::Rect& src2) { Fog::Region r(src1); r.subtract(src2); return r; }

FOG_INLINE const Fog::Region operator+(const Fog::Region& src1, const Fog::Box& src2) { Fog::Region r(src1); r.unite(src2); return r; }
FOG_INLINE const Fog::Region operator|(const Fog::Region& src1, const Fog::Box& src2) { Fog::Region r(src1); r.unite(src2); return r; }
FOG_INLINE const Fog::Region operator&(const Fog::Region& src1, const Fog::Box& src2) { Fog::Region r(src1); r.intersect(src2); return r; }
FOG_INLINE const Fog::Region operator^(const Fog::Region& src1, const Fog::Box& src2) { Fog::Region r(src1); r.eor(src2); return r; }
FOG_INLINE const Fog::Region operator-(const Fog::Region& src1, const Fog::Box& src2) { Fog::Region r(src1); r.subtract(src2); return r; }

FOG_INLINE bool operator==(const Fog::Region& src1, const Fog::Region& src2) { return  src1.eq(src2); }
FOG_INLINE bool operator!=(const Fog::Region& src1, const Fog::Region& src2) { return !src1.eq(src2); }

//! @}

// ============================================================================
// [Fog::TypeInfo<>]
// ============================================================================

FOG_DECLARE_TYPEINFO(Fog::Region, Fog::MoveableType)

// [Guard]
#endif // _FOG_GRAPHICS_REGION_H
