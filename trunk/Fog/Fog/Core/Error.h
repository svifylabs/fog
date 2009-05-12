// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_ERROR_H
#define _FOG_CORE_ERROR_H

// [Dependencies]
#include <Fog/Build/Build.h>

// ============================================================================
// [Error]
// ============================================================================

namespace Error {

enum NoError
{
  Ok = 0
};

//! @brief Core library error codes.
enum CoreError
{
  // [Errors Range]
  _CoreErrorStart = 0x00010000,
  _CoreErrorLast  = 0x00010FFF,

  // [Runtime Errors]

  OutOfMemory = _CoreErrorStart,

  NotImplemented,

  InvalidFunction,
  InvalidProperty,
  InvalidArgument,
  InvalidHandle,

  InvalidInput,

  Overflow,

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

  // [TextCodec Errors]

  //! @brief Invalid text codec (Null) used to encode / decode string
  InvalidCodec,
  //! @brief Invalid UTF-8 sequence.
  InvalidUtf8,
  //! @brief Invalid UTF-16 sequence (surrogate pair)
  InvalidUtf16,
  //! @brief Invalid Unicode character.
  InvalidUnicode,
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

// [Guard]
#endif // _FOG_CORE_ERROR_H
