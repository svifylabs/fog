// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_ERROR_H
#define _FOG_CORE_ERROR_H

// [Dependencies]
#include <Fog/Build/Build.h>

//! @addtogroup Fog_Core
//! @{

// ============================================================================
// [Error]
// ============================================================================

namespace Error {

//! @brief Single enumeration to define @c Ok value.
enum NoError
{
  //! @brief Function proceed without errors (success).
  Ok = 0
};

//! @brief Error codes used in Fog/Core.
enum CoreError
{
  // [Errors Range]
  _CoreErrorStart = 0x00010000,
  _CoreErrorLast  = 0x00010FFF,

  // [Runtime Errors]

  OutOfMemory = _CoreErrorStart,

  NotImplemented,

  InvalidFunction,
  InvalidArgument,
  InvalidHandle,
  InvalidInput,

  Overflow,

  // [Properties]

  InvalidPropertyName,
  InvalidPropertyValue,
  PropertyIsReadOnly,

  // [IO Errors]

  IOInvalid,
  IOTooBig,
  IOEmpty,
  IOReadOnly,
  IOInvalidOffset,
  IONotAFile,
  IONotADirectory,
  IONotFound,
  FileSizeTooBig,
  EmptyFile,

  CantSeek,

  TruncateNotSupported,
  TruncateFailed,

  FileNotExists,
  DirectoryAlreadyExists,

  // [Collections]

  ObjectNotExists,
  ObjectAlreadyExists,

  // [Library Errors]

  LibraryOpenFailed,
  LibrarySymbolNotFound,

  // [TextCodec Errors (Also StringUtil errors)]

  //! @brief Invalid text codec (Null) used to encode / decode string
  InvalidCodec,
  //! @brief Invalid UTF-8 sequence.
  InvalidUtf8Sequence,
  //! @brief Invalid UTF-16 sequence (surrogate pair error).
  InvalidUtf16Sequence,
  //! @brief Invalid Unicode character.
  InvalidUnicodeCharacter,
  //! @brief Incomplete input buffer.
  InputTruncated,
  //! @brief Some characters loss during conversion (replaced by '?' or a given replacer).
  //!
  //! This can only happen when converting unicode to non-unicode encoding.
  CharacterLoss,

  // [OS]

  GetEnvFailure,
  SetEnvFailure
};

} // Error namespace

//! @}

// [Guard]
#endif // _FOG_CORE_ERROR_H
