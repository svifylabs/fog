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

// [Fog::ExitCode]

//! @brief Exit codes that can be used instead @c EXIT_SUCCESS and @c EXIT_FAILURE.
enum ExitCode
{
  //! @brief Success exit code (replaces @c EXIT_SUCCESS).
  ExitSuccess = 0,
  //! @brief Failure exit code (replaces @c EXIT_FAILURE).
  ExitFailure = 1
};

//! @brief Core event IDs.
enum EvCore
{
  // [0 is reserved]

  // [Object events]

  EvCreate = 1,
  EvDestroy,
  EvDelete,
  EvTimer,
  EvPropertyChanged = 10,

  //! @brief First user event.
  EvUser = 1024,

  //! @brief Last event that can be used by all libraries.
  //!
  //! This number is first retrieved by Fog::Event::uid() and incremented
  //! each time that function is called.
  EvUID = 10000000
};

//! @brief Core error domain namespace.
//!
//! Core library uses special class for error handling named @c Value. This class 
//! is abstraction between number types and strings not designed only for errors.
//! To check if value contains an error, it must contains an error domain other
//! than 0 (@c ErrorDomain_Ok).
enum ErrorDomain
{
  //! @brief This domain means that there is not an error (Success).
  ErrorDomain_Ok = 0,

  // Platform or C library specific error domains

  //! @brief C library error (can be used only if FOG_OS_POSIX is defined).
  ErrorDomain_LibC = 1,
  //! @brief Windows error (can be used only if FOG_OS_WINDOWS macro is defined).
  ErrorDomain_Windows = 2,

  ErrorDomain_Core = 4,

  //! @brief User errors begins from this number and can continue to 65535.
  ErrorDomain_User = 65535
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
