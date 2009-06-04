// [Fog/Xml Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_XML_ERROR_H
#define _FOG_XML_ERROR_H

// [Dependencies]
#include <Fog/Build/Build.h>

namespace Error {

//! @addtogroup Xml
//! @{

enum XmlError
{
  // [Errors Range]
  _XmlErrorStart = 0x00011000,
  _XmlErrorLast  = 0x000110FF,

  // [XmlDom Errors]
  XmlDomNotAllowed = _XmlErrorStart,
  XmlDomAttributeNotFound,
  XmlDomInvalidAttribute,
  XmlDomCyclic,
  XmlDomInvalidChild,
  XmlDomDocumentHasAlreadyRoot,

  // [XmlReader Errors]
  XmlReaderUnknown,
  XmlReaderNoDocument,
  XmlReaderMissingRootTag,
  XmlReaderMissingTag,
  XmlReaderMissingAttribute,
  XmlReaderUnmatchedClosingTag,
  XmlReaderUnclosedCDATA,
  XmlReaderUnclosedPI,
  XmlReaderUnclosedComment,
  XmlReaderUnclosedDOCTYPE,
  XmlReaderSyntaxError,

  // [XmlDomReader Errors]
  XmlDomInvalidClosingTag,
};

//! @}

}

// [Guard]
#endif // _FOG_XML_ERROR_H
