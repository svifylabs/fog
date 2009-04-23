// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_REGION_H
#define _FOG_GRAPHICS_REGION_H

// [Dependencies]
#include <Fog/Core/Atomic.h>
#include <Fog/Core/Flags.h>
#include <Fog/Core/RefData.h>
#include <Fog/Core/Static.h>
#include <Fog/Graphics/Geometry.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// [Fog::Region]

struct FOG_API Region
{
  // [Data]

  struct FOG_API Data : public Fog::RefData<Data>
  {
    //! @brief Count of preallocated rectangles.
    sysuint_t capacity;
    //! @brief Count of used rectangles.
    sysuint_t count;
    //! @brief Region extents.
    Box extents;
    //! @brief Region rectangles, always YX sorted.
    Box rects[1];

    Data* ref();
    void deref();

    FOG_INLINE void destroy() {}
    FOG_INLINE void free() { Fog::Memory::free(this); }

    static Data* adopt(void* address, sysuint_t capacity);
    static Data* adopt(void* address, sysuint_t capacity, const Box& r);
    static Data* adopt(void* address, sysuint_t capacity, const Box* extents, const Box* rects, sysuint_t count);
    
    static Data* create(sysuint_t capacity, uint allocPolicy);
    static Data* create(sysuint_t capacity, const Box* extents, const Box* rects, sysuint_t count);

    static Data* copy(const Data* other, uint allocPolicy);

    static FOG_INLINE sysuint_t sizeFor(sysuint_t capacity)
    { return sizeof(Data) - sizeof(Box) + sizeof(Box) * capacity; }
  };

  static Fog::Static<Data> sharedNull;

  // [Construction / Destruction]

  Region();
  explicit Region(const Box& rect);
  explicit Region(const Rect& rect);
  Region(const Region& other);

  Region(Data* d);
  ~Region();

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
  FOG_INLINE bool isDynamic() const { return _d->flags.anyOf(Data::IsDynamic); }
  //! @copydoc Doxygen::Implicit::isSharable().
  FOG_INLINE bool isSharable() const { return _d->flags.anyOf(Data::IsSharable); }
  //! @copydoc Doxygen::Implicit::isNull().
  FOG_INLINE bool isNull() const { return _d->flags.anyOf(Data::IsNull); }
  //! @copydoc Doxygen::Implicit::isStrong().
  FOG_INLINE bool isStrong() const { return _d->flags.anyOf(Data::IsStrong); }

  void setSharable(bool val);
  void setWeakAssignPrevented(bool val);

  // [Data]

  //! @brief Returns const pointer to region data.
  FOG_INLINE const Box* cData() const 
  { return _d->rects; }

  //! @brief Returns read / write pointer to region data.
  FOG_INLINE Box* mData() 
  {
    detach();
    return _d->rects; 
  }

  //! @brief Returns read / write pointer to region data.
  FOG_INLINE Box* xData() 
  {
    FOG_ASSERT(isDetached());
    return _d->rects; 
  }

  // [Container]

  //! @brief Returns capacity of region in rectangles.
  FOG_INLINE sysuint_t capacity() const 
  { return _d->capacity; }

  //! @brief Returns count of rectangles in region.
  FOG_INLINE sysuint_t count() const 
  { return _d->count; }

  //! @brief Returns @c true if region is empty.
  FOG_INLINE bool isEmpty() const 
  { return _d->count == 0; }

  //! @brief Returns region extents.
  FOG_INLINE const Box& extents() const 
  { return _d->extents; }

  void reserve(sysuint_t to);
  void prepare(sysuint_t to);
  void squeeze();

  uint32_t type() const;

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

  FOG_INLINE uint32_t contains(int x, int y) const
  { return contains(Point(x, y)); }

  uint contains(const Rect& r) const;
  uint contains(const Box& r) const;

  //! @brief Removes all rectagnels from region.
  Region& clear();

  Region& set(const Region& r);
  Region& set(const Rect& r);
  Region& set(const Box& r);
  
  //! @brief Creates a deep copy instead of reference.
  //!
  //! This function is used internally to optimize computing where we know
  //! that we need to copy region and do some ops with them (Because if we
  //! create a reference, it will free existing allocated block of memory
  //! and it's expensive to alloc it back.
  //!
  //! @param r Region to copy from;
  //! @return Reference to itself.
  Region& setDeep(const Region& r);

  Region& set(const Rect* rects, sysuint_t count);
  Region& set(const Box* rects, sysuint_t count);

  Region& unite(const Region& r);
  Region& unite(const Rect& r);
  Region& unite(const Box& r);

  Region& intersect(const Region& r);
  Region& intersect(const Rect& r);
  Region& intersect(const Box& r);

  Region& eor(const Region& r);
  Region& eor(const Rect& r);
  Region& eor(const Box& r);

  Region& subtract(const Region& r);
  Region& subtract(const Rect& r);
  Region& subtract(const Box& r);

  Region& op(const Region& r, uint _op);
  Region& op(const Rect& r, uint _op);
  Region& op(const Box& r, uint _op);

  Region& translate(const Point& pt);

  FOG_INLINE Region& translate(int x, int y)
  { return translate(Point(x, y)); }

  Region& shrink(const Point& pt);

  FOG_INLINE Region& shrink(int x, int y)
  { return shrink(Point(x, y)); }

  Region& frame(const Point& pt);

  FOG_INLINE Region& frame(int x, int y)
  { return frame(Point(x, y)); }

  Region& round(const Rect& r, uint xradius, uint yradius, bool fill = true);

  Region& polygon(const Point* pts, sysuint_t count, uint fillRule);
  Region& polyPolygon(const Point* src, const sysuint_t* count, sysuint_t polygons, uint fillRule);

  bool eq(const Region& other) const;

  static void set(Region& dest, const Region& src);
  static void unite(Region& dest, const Region& src1, const Region& src2);
  static void intersect(Region& dest, const Region& src1, const Region& src2);
  static void eor(Region& dest, const Region& src1, const Region& src2);
  static void subtract(Region& dest, const Region& src1, const Region& src2);
  static void op(Region& dest, const Region& src1, const Region& src2, uint _op);
  static void translate(Region& dest, const Region& src, const Point& pt);
  static void shrink(Region& dest, const Region& src, const Point& pt);
  static void frame(Region& dest, const Region& src, const Point& pt);

  // Overloaded operators

  FOG_INLINE Region& operator=(const Region& r) { return set(r); }
  FOG_INLINE Region& operator=(const Rect& r) { return set(r); }
  FOG_INLINE Region& operator=(const Box& r) { return set(r); }

  FOG_INLINE Region& operator+=(const Region& r) { return unite(r); }
  FOG_INLINE Region& operator+=(const Rect& r) { return unite(r); }
  FOG_INLINE Region& operator+=(const Box& r) { return unite(r); }

  FOG_INLINE Region& operator|=(const Region& r) { return unite(r); }
  FOG_INLINE Region& operator|=(const Rect& r) { return unite(r); }
  FOG_INLINE Region& operator|=(const Box& r) { return unite(r); }

  FOG_INLINE Region& operator&=(const Region& r) { return intersect(r); }
  FOG_INLINE Region& operator&=(const Rect& r) { return intersect(r); }
  FOG_INLINE Region& operator&=(const Box& r) { return intersect(r); }

  FOG_INLINE Region& operator^=(const Region& r) { return eor(r); }
  FOG_INLINE Region& operator^=(const Rect& r) { return eor(r); }
  FOG_INLINE Region& operator^=(const Box& r) { return eor(r); }

  FOG_INLINE Region& operator-=(const Region& r) { return subtract(r); }
  FOG_INLINE Region& operator-=(const Rect& r) { return subtract(r); }
  FOG_INLINE Region& operator-=(const Box& r) { return subtract(r); }

#if defined(FOG_OS_WINDOWS)
  HRGN toHRGN() const;
  bool fromHRGN(HRGN hrgn);
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

// [Fog::TemporaryRegion<N>]

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
    Region(Data::adopt((void*)&_storage, N, &other._d->extents, other._d->rects, other._d->count))
  {
  }
  
  FOG_INLINE TemporaryRegion(const Region& other) :
    Region(Data::adopt((void*)&_storage, N, &other._d->extents, other._d->rects, other._d->count))
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

FOG_INLINE const Fog::Region operator+(const Fog::Region& src1, const Fog::Rect& src2) { return Fog::Region(src1).unite(src2); }
FOG_INLINE const Fog::Region operator|(const Fog::Region& src1, const Fog::Rect& src2) { return Fog::Region(src1).unite(src2); }
FOG_INLINE const Fog::Region operator&(const Fog::Region& src1, const Fog::Rect& src2) { return Fog::Region(src1).intersect(src2); }
FOG_INLINE const Fog::Region operator^(const Fog::Region& src1, const Fog::Rect& src2) { return Fog::Region(src1).eor(src2); }
FOG_INLINE const Fog::Region operator-(const Fog::Region& src1, const Fog::Rect& src2) { return Fog::Region(src1).subtract(src2); }

FOG_INLINE const Fog::Region operator+(const Fog::Region& src1, const Fog::Box& src2) { return Fog::Region(src1).unite(src2); }
FOG_INLINE const Fog::Region operator|(const Fog::Region& src1, const Fog::Box& src2) { return Fog::Region(src1).unite(src2); }
FOG_INLINE const Fog::Region operator&(const Fog::Region& src1, const Fog::Box& src2) { return Fog::Region(src1).intersect(src2); }
FOG_INLINE const Fog::Region operator^(const Fog::Region& src1, const Fog::Box& src2) { return Fog::Region(src1).eor(src2); }
FOG_INLINE const Fog::Region operator-(const Fog::Region& src1, const Fog::Box& src2) { return Fog::Region(src1).subtract(src2); }

FOG_INLINE bool operator==(const Fog::Region& src1, const Fog::Region& src2) { return  src1.eq(src2); }
FOG_INLINE bool operator!=(const Fog::Region& src1, const Fog::Region& src2) { return !src1.eq(src2); }

//! @}

// ============================================================================
// [Fog::TypeInfo<>]
// ============================================================================

FOG_DECLARE_TYPEINFO(Fog::Region, Fog::MoveableType)

// [Guard]
#endif // _FOG_GRAPHICS_REGION_H
