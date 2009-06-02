// [Fog/Xml Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_XML_READER_H
#define _FOG_XML_READER_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Core/String.h>
#include <Fog/Core/Stream.h>
#include <Fog/Core/TextCodec.h>

namespace Fog {

//! @addtogroup Xml
//! @{

// ============================================================================
// [Fog::XmlReader]
// ============================================================================

//! @brief Xml reader.
struct FOG_API XmlReader
{
  // [Construction / Destruction]

  XmlReader();
  virtual ~XmlReader();

  // [Status]

  enum Status
  {
    StatusReady = 0,
    StatusError
  };

  // [Stream]

  FOG_INLINE Stream& stream() { return _stream; }

  void setStream(Stream& stream);
  void resetStream();

  // [Text Codec]

  FOG_INLINE const TextCodec& textCodec() const
  { return _textCodec; }

protected:
  Stream _stream;
  String8 _buffer;
  TextCodec _textCodec;
  TextCodec::State _textCodecState;
  int _status;
  bool _encodingParsed;
};

//! @}

} // Fog namespace

// [Guard]
#endif // _FOG_XML_READER_H
