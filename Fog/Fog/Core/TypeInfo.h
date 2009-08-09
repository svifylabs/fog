// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_TYPEINFO_H
#define _FOG_CORE_TYPEINFO_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Core/Constants.h>

namespace Fog {

// ===========================================================================
// [Fog::TypeInfo - Types]
//
// Class types for Fog::TypeInfo<T> template.
// ===========================================================================

//! @brief Types for @c Fog::TypeInfo, use together with
//! @c FOG_DECLARE_TYPEINFO macro.
enum TypeInfoType
{
  //! @brief Simple data type like @c int, @c long, ...
  //!
  //! Simple data can be copyed from one memory location into another.
  PrimitiveType = 0,

  //! @brief Moveable data type line @c Fog::String, ...
  //!
  //! Moveable data type can't be copyed to another location, but
  //! can be moved in memory.
  MoveableType = 1,

  //! @brief Class data type. 
  //!
  //! Means that class cannot be moved to another location.
  ClassType = 2
};

//! @brief Additional flags for FOG_DECLARE_TYPEINFO. All flags are cleared in
//! in @c Fog::TypeInfo<T> template.
enum TypeInfoFlags
{
  TypeInfoIsPODType   = 0x00000100,
  TypeInfoIsFloatType = 0x00000200,
  TypeInfoHasCompare  = 0x00000400,
  TypeInfoHasEq       = 0x00000800,
  TypeInfoMask        = 0xFFFFFF00
};

// [Fog::TypeInfo_CompareFn and Fog::TypeInfo_EqFn]

typedef int (*TypeInfo_CompareFn)(const void* a, const void* b);
typedef bool (*TypeInfo_EqFn)(const void* a, const void* b);

// ===========================================================================
// [Fog::TypeInfo - TypeInfo<T>
//
// Type info
// ===========================================================================

//! @brief Template for partial @c Fog::TypeInfo specialization.
template<typename T>
struct TypeInfo
{
  // TypeInfo constants
  enum
  {
    // [Type and Size - Generic type is always ClassType]
    Type = Fog::ClassType,
    Flags = 0,
    Size = sizeof(T),

    // [Basic Informations]
    IsPrimitive = (Type == Fog::PrimitiveType),
    IsMoveable = (Type <= Fog::MoveableType),
    IsClass = (Type == Fog::ClassType),
    IsPointer = 0,

    // [Extended Informations]
    IsPOD = 0,
    IsFloat = 0,
    HasCompare = 0,
    HasEq = 0
  };

  typedef bool (*EqFn)(const T* a, const T* b);
  typedef int (*CompareFn)(const T* a, const T* b);
};

// specialization for pointers. Pointer is always PrimitiveType
template<typename T>
struct TypeInfo<T*>
{
  // TypeInfo constants
  enum
  {
    // [Type and Size - Pointer is always simple type]
    Type = Fog::PrimitiveType,
    Flags = Fog::TypeInfoIsPODType,
    Size = sizeof(T*),

    // [Type and Size - Generic type is always ClassType]
    IsPrimitive = (Type == Fog::PrimitiveType),
    IsMoveable = (Type <= Fog::MoveableType),
    IsClass = (Type == Fog::ClassType),
    IsPointer = 1,

    // [Extended Informations]
    IsPOD = 1, // POD is comparable by default
    IsFloat = 0,
    HasCompare = 0,
    HasEq = 0
  };

  typedef bool (*EqFn)(const T** a, const T** b);
  typedef int (*CompareFn)(const T** a, const T** b);
};

// inherited by all types declared as FOG_DECLARE_TYPEINFO()
template<typename T, uint __TypeInfo__>
struct TypeInfo_Wrapper
{
  // TypeInfo constants
  enum {
    // [Type and Size - Based on __TypeInfo__]
    Type = (__TypeInfo__ & ~Fog::TypeInfoMask),
    Flags = (__TypeInfo__ & Fog::TypeInfoMask),
    Size = sizeof(T),

    // [Basic Informations]
    IsPrimitive = (Type == Fog::PrimitiveType),
    IsMoveable = (Type <= Fog::MoveableType),
    IsClass = (Type == Fog::ClassType),
    IsPointer = 0,

    // [Extended Informations]
    IsPOD = (__TypeInfo__ & Fog::TypeInfoIsPODType) != 0,
    IsFloat = (__TypeInfo__ & Fog::TypeInfoIsFloatType) != 0,
    HasCompare = (__TypeInfo__ & Fog::TypeInfoHasCompare) != 0,
    HasEq = (__TypeInfo__ & Fog::TypeInfoHasEq) != 0
  };

  typedef bool (*EqFn)(const T* a, const T* b);
  typedef int (*CompareFn)(const T* a, const T* b);
};

// ===========================================================================
// [Fog::TypeInfo - Compare
// ===========================================================================

template<typename T>
struct TypeInfo_Compare_Integral
{
  static int compare(const T* a, const T* b)
  {
    // Ouh, optimized comparision function for POD types.
    //
    // TODO: Needs more testing how it can be used for 8, 16, 32 and 64 bit
    // types (signed or unsigned).
    return (int)(*a - *b);
  }
};

template<typename T>
struct TypeInfo_Compare_Default
{
  static int compare(const T* a, const T* b)
  {
    // This is default compare method. For POD types it's quite inefficient,
    // because comparision must be do twice. For POD types is ideal schema:
    //
    //   return *a - *b;
    //
    // that is implemented in specialized templates.
    //
    // Also this implementation needs overloaded operator < and == in classes
    // that are ready for comparisions. I think that this method is compatible
    // with STL and other libraries as well.
    if (*a < *b) 
      return -1;
    else if (*a == *b)
      return 0;
    else
      return 1;
  }
};

// TypeInfo_Compare_Wrapper_POD
template<typename T, uint __POD_Flags__>
struct TypeInfo_Compare_Wrapper_POD : 
  // Defaults to TypeInfo_Compare_Default, the default implementation
  public TypeInfo_Compare_Default<T> {};

template<typename T>
struct TypeInfo_Compare_Wrapper_POD<T, Fog::TypeInfoIsPODType /* and not Fog::TypeInfoIsFloatType */> : 
  // Defaults to TypeInfo_Compare_Integral, implementation for integral types
  public TypeInfo_Compare_Integral<T> {};

// TypeInfo_Compare_Wrapper
template<typename T, uint __TypeInfo__>
struct TypeInfo_Compare_Wrapper : 
  public TypeInfo_Compare_Default<T> {};

template<typename T>
struct TypeInfo_Compare_Wrapper<T, 0> : 
  public TypeInfo_Compare_Wrapper_POD<T, TypeInfo<T>::Flags & (Fog::TypeInfoIsPODType | Fog::TypeInfoIsFloatType)> {};

// TypeInfo_Compare_Wrapper_HasCompare
template<typename T, uint __HasCompare__>
struct TypeInfo_Compare_Wrapper_HasCompare : 
  public TypeInfo_Compare_Wrapper<T, TypeInfo<T>::Type> {};

template<typename T>
struct TypeInfo_Compare_Wrapper_HasCompare<T, 1> : 
  // simply redirect to original type expecting this function:
  //   static int T::compare(const T* a, const T* b);
  public T {};

// TypeInfo_Compare
template<typename T>
struct TypeInfo_Compare : 
  public TypeInfo_Compare_Wrapper_HasCompare<T, TypeInfo<T>::HasCompare> {};

// ===========================================================================
// [Fog::TypeInfo - Eq
// ===========================================================================

template<typename T>
struct TypeInfo_Eq_Default
{
  static bool eq(const T* a, const T* b)
  {
    return *a == *b;
  }
};

// TypeInfo_Eq_Wrapper
template<typename T, uint __TypeInfo__>
struct TypeInfo_Eq_Wrapper : 
  public TypeInfo_Eq_Default<T> {};

// TypeInfo_Eq_Wrapper_HasEq
template<typename T, uint __HasEq__>
struct TypeInfo_Eq_Wrapper_HasEq : 
  public TypeInfo_Eq_Wrapper<T, TypeInfo<T>::Type> {};

template<typename T>
struct TypeInfo_Eq_Wrapper_HasEq<T, 1> : 
  // simply redirect to original type expecting this function:
  //   static bool T::eq(const T* a, const T* b);
  public T {};

// TypeInfo_Eq
template<typename T>
struct TypeInfo_Eq : 
  public TypeInfo_Eq_Wrapper_HasEq<T, TypeInfo<T>::HasEq> {};

// ===========================================================================
// [Fog::TypeInfo - Macros]
// ===========================================================================

//! @brief Use this macro to declare @c Fog::TypeInfo. 
//!
//! @c Fog::TypeInfo is template to resolve type at compile time. It's
//! used in template specializations
#define FOG_DECLARE_TYPEINFO(__symbol__, __typeinfo__) \
namespace Fog { \
template <> \
struct TypeInfo <__symbol__> : public TypeInfo_Wrapper< __symbol__, __typeinfo__ > {}; \
}

#define FOG_DECLARE_TYPEINFO_TEMPLATE1(__symbol__, T1, A1, __typeinfo__) \
namespace Fog { \
template <T1 A1> \
struct TypeInfo < __symbol__<A1> > : public TypeInfo_Wrapper< __symbol__<A1>, __typeinfo__ > {}; \
}

#define FOG_DECLARE_TYPEINFO_TEMPLATE2(__symbol__, T1, A1, T2, A2, __typeinfo__) \
namespace Fog { \
template <T1 A1, T2 A2> \
struct TypeInfo < __symbol__<A1, A2> > : public TypeInfo_Wrapper< __symbol__<A1, A2>, __typeinfo__ > {}; \
}

#define FOG_DECLARE_TYPEINFO_TEMPLATE3(__symbol__, T1, A1, T2, A2, T3, A3, __typeinfo__) \
namespace Fog { \
template <T1 A1, T2 A2, T3 A3> \
struct TypeInfo < __symbol__<A1, A2, A3> > : public TypeInfo_Wrapper< __symbol__<A1, A2, A3>, __typeinfo__ > {}; \
}

#define FOG_DECLARE_TYPEINFO_TEMPLATE4(__symbol__, T1, A1, T2, A2, T3, A3, T4, A4, __typeinfo__) \
namespace Fog { \
template <T1 A1, T2 A2, T3 A3, T4 A4> \
struct TypeInfo < __symbol__<A1, A2, A3, A4> > : public TypeInfo_Wrapper< __symbol__<A1, A2, A3, A4>, __typeinfo__ > {}; \
}

#define FOG_DECLARE_TYPEINFO_TEMPLATE5(__symbol__, T1, A1, T2, A2, T3, A3, T4, A4, T5, A5, __typeinfo__) \
namespace Fog { \
template <T1 A1, T2 A2, T3 A3, T4 A4, T5 A5> \
struct TypeInfo < __symbol__<A1, A2, A3, A4, A5> > : public TypeInfo_Wrapper< __symbol__<A1, A2, A3, A4, A5>, __typeinfo__ > {}; \
}

#define FOG_DECLARE_TYPEINFO_TEMPLATE6(__symbol__, T1, A1, T2, A2, T3, A3, T4, A4, T5, A5, T6, A6, __typeinfo__) \
namespace Fog { \
template <T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6> \
struct TypeInfo < __symbol__<A1, A2, A3, A4, A5, A6> > : public TypeInfo_Wrapper< __symbol__<A1, A2, A3, A4, A5, A6>, __typeinfo__ > {}; \
}

#define FOG_DECLARE_TYPEINFO_TEMPLATE7(__symbol__, T1, A1, T2, A2, T3, A3, T4, A4, T5, A5, T6, A6, T7, A7, __typeinfo__) \
namespace Fog { \
template <T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7> \
struct TypeInfo < __symbol__<A1, A2, A3, A4, A5, A6, A7> > : public TypeInfo_Wrapper< __symbol__<A1, A2, A3, A4, A5, A6, A7>, __typeinfo__ > {}; \
}

#define FOG_DECLARE_TYPEINFO_TEMPLATE8(__symbol__, T1, A1, T2, A2, T3, A3, T4, A4, T5, A5, T6, A6, T7, A7, T8, A8, __typeinfo__) \
namespace Fog { \
template <T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8> \
struct TypeInfo < __symbol__<A1, A2, A3, A4, A5, A6, A7, A8> > : public TypeInfo_Wrapper< __symbol__<A1, A2, A3, A4, A5, A6, A7, A8>, __typeinfo__ > {}; \
}

#define FOG_DECLARE_TYPEINFO_TEMPLATE9(__symbol__, T1, A1, T2, A2, T3, A3, T4, A4, T5, A5, T6, A6, T7, A7, T8, A8, T9, A9, __typeinfo__) \
namespace Fog { \
template <T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9> \
struct TypeInfo < __symbol__<A1, A2, A3, A4, A5, A6, A7, A8, A9> > : public TypeInfo_Wrapper< __symbol__<A1, A2, A3, A4, A5, A6, A7, A8, A9>, __typeinfo__ > {}; \
}

#define FOG_DECLARE_TYPEINFO_TEMPLATE10(__symbol__, T1, A1, T2, A2, T3, A3, T4, A4, T5, A5, T6, A6, T7, A7, T8, A8, T9, A9, T10, A10, __typeinfo__) \
namespace Fog { \
template <T1 A1, T2 A2, T3 A3, T4 A4, T5 A5, T6 A6, T7 A7, T8 A8, T9 A9, T10 A10> \
struct TypeInfo < __symbol__<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10> > : public TypeInfo_Wrapper< __symbol__<A1, A2, A3, A4, A5, A6, A7, A8, A9, A10>, __typeinfo__ > {}; \
}

} // Fog namespace

// ===========================================================================
// [Fog::TypeInfo - Built-In]
// ===========================================================================

FOG_DECLARE_TYPEINFO(int8_t, Fog::PrimitiveType | Fog::TypeInfoIsPODType)
FOG_DECLARE_TYPEINFO(uint8_t, Fog::PrimitiveType | Fog::TypeInfoIsPODType)
FOG_DECLARE_TYPEINFO(int16_t, Fog::PrimitiveType | Fog::TypeInfoIsPODType)
FOG_DECLARE_TYPEINFO(uint16_t, Fog::PrimitiveType | Fog::TypeInfoIsPODType)
FOG_DECLARE_TYPEINFO(int32_t, Fog::PrimitiveType | Fog::TypeInfoIsPODType)
FOG_DECLARE_TYPEINFO(uint32_t, Fog::PrimitiveType | Fog::TypeInfoIsPODType)
FOG_DECLARE_TYPEINFO(int64_t, Fog::PrimitiveType | Fog::TypeInfoIsPODType)
FOG_DECLARE_TYPEINFO(uint64_t, Fog::PrimitiveType | Fog::TypeInfoIsPODType)

FOG_DECLARE_TYPEINFO(float, Fog::PrimitiveType | Fog::TypeInfoIsPODType | Fog::TypeInfoIsFloatType)
FOG_DECLARE_TYPEINFO(double, Fog::PrimitiveType | Fog::TypeInfoIsPODType | Fog::TypeInfoIsFloatType)

#if !defined(FOG_CC_BORLAND)
// char is same as int8_t or uint8_t for borland compiler
FOG_DECLARE_TYPEINFO(char, Fog::PrimitiveType | Fog::TypeInfoIsPODType)
#endif // FOG_CC_BORLAND

// TODO: long and ulong checking
#if !(defined(FOG_CC_GNU) && FOG_ARCH_BITS == 64)
FOG_DECLARE_TYPEINFO(long, Fog::PrimitiveType | Fog::TypeInfoIsPODType)
FOG_DECLARE_TYPEINFO(ulong, Fog::PrimitiveType | Fog::TypeInfoIsPODType)
#endif // long / ulong

// TODO: wchar_t

// [Guard]
#endif // _FOG_CORE_TYPEINFO_H
