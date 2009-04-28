// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_CONSTANTS_H
#define _FOG_CORE_CONSTANTS_H

// [Dependencies]
#include <Fog/Build/Build.h>

namespace Fog {

// [Fog::CreateSharable]

enum _CreateSharable { CreateSharable };
enum _LinkerInitialized { LinkerInitialized };

// [Fog::AllocPolicy]

enum AllocPolicy { AllocCantFail = 0, AllocCanFail = 1 };

// [Fog::[] constants]

static const sysuint_t DetectLength  = (sysuint_t)-1;
static const sysuint_t InvalidIndex  = (sysuint_t)-1;

// [Fog::Flags]

static const uint32_t  NoFlags = 0;

// how much to reserve stack for local strings in Core library
// or all additional libraries
static const sysuint_t TemporaryLength = 256;

// [Fog::CaseSensitivity]

enum CaseSensitivity
{
  CaseInsensitive = 0,
  CaseSensitive = 1,

#if defined(FOG_OS_WINDOWS)
  FileSystemCase = CaseInsensitive
#else
  FileSystemCase = CaseSensitive
#endif // FOG_OS_WINDOWS
};

// [Fog::SplitBehavior]

enum SplitBehavior
{
  KeepEmptyParts = 0,
  RemoveEmptyParts = 1
};

// [Fog::Justification]
enum Justify
{
  LeftJustify = 0x1,
  RightJustify = 0x2,
  CenterJustify = 0x3
};

// ---------------------------------------------------------------------------
// [Enumerations]
// ---------------------------------------------------------------------------

enum OrderMode
{
  OrderAscent = 0,
  OrderDescent = 1
};

enum DoubleForm
{
  //! @brief Decimal double form (sprintf %f).
  DF_Decimal = 0,
  //! @brief Exponent double form (sprintf %e).
  DF_Exponent = 1,
  //! @brief Significant digits (sprintf %g).
  DF_SignificantDigits = 2
};

} // Fog namespace

// [Guard]
#endif // _FOG_CORE_CONSTANTS_H
