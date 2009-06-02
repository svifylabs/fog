// [Fog/Xml Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/Assert.h>
#include <Fog/Core/Char.h>
#include <Fog/Core/Hash.h>
#include <Fog/Core/MapFile.h>
#include <Fog/Core/Stream.h>
#include <Fog/Core/String.h>
#include <Fog/Core/StringUtil.h>
#include <Fog/Core/TextCodec.h>
#include <Fog/Core/Vector.h>
#include <Fog/Xml/Dom.h>
#include <Fog/Xml/Entity.h>
#include <Fog/Xml/Error.h>
#include <Fog/Xml/Reader.h>

#include "Error.h"

namespace Fog {

// ============================================================================
// [Fog::XmlReader - Helpers]
// ============================================================================

enum
{
  XML_CHAR_SPACE    = 0x01, // '\t''\r''\n'' '
  XML_CHAR_EQUAL    = 0x02, // =
  XML_CHAR_LT       = 0x04, // <
  XML_CHAR_GT       = 0x08, // >
  XML_CHAR_SLASH    = 0x10, // /
  XML_CHAR_BRACKETS = 0x20, // []
  XML_CHAR_QUOT     = 0x40, // '"
  XML_CHAR_AND      = 0x80  // &
};

// Special xml characters.
static const uint8_t xmlChars[128] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0
  0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, // 8
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 16
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 24
  0x01, 0x00, 0x40, 0x00, 0x00, 0x00, 0x80, 0x40, // 32
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, // 40
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 48
  0x00, 0x00, 0x00, 0x00, 0x04, 0x02, 0x08, 0x00, // 56
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 64
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 72
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 80
  0x00, 0x00, 0x00, 0x20, 0x00, 0x20, 0x00, 0x00, // 88
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 96
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 104
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 112
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // 120
};

// Optimized strcspn for xml reading with mask instead of string
static uint xmlStrCSPN(const Char32* buffer, const Char32* end, uint rejectMask)
{
  const Char32* start = buffer;

  while (buffer < end)
  {
    if (buffer->ch() < 128 && (xmlChars[buffer->ch()] & rejectMask)) break;
    buffer++;
  }

  return (sysuint_t)(buffer - start);
}

// Optimized strspn for xml reading with mask instead of string
static uint xmlStrSPN(const Char32* buffer, const Char32* end, uint acceptMask)
{
  const Char32* start = buffer;

  while (buffer < end)
  {
    if (*buffer > 127 || !(xmlChars[buffer->ch()] & acceptMask)) break;
    buffer++;
  }

  return (sysuint_t)(buffer - start);
}

static const Char32* xmlStrCHR(const Char32* buffer, const Char32* end, Char32 uc)
{
  while (buffer < end && *buffer != uc) buffer++;
  return buffer;
}

static bool xmlIsWhiteSpace(const Char32* buffer, const Char32* end)
{
  while (buffer < end)
  {
    if (!buffer->isSpace()) return false;
    buffer++;
  }
  return true;
}

// ============================================================================
// [Fog::XmlReader]
// ============================================================================

XmlReader::XmlReader()
{
}

XmlReader::~XmlReader()
{
}

err_t XmlReader::parseFile(const String32& fileName)
{
  Stream stream;
  err_t err = stream.openFile(fileName, Stream::OpenRead);
  if (err) return err;

  return parseStream(stream);
}

err_t XmlReader::parseStream(Stream& stream)
{
  String8 buffer;
  stream.readAll(buffer);
  return parseMemory(reinterpret_cast<const void*>(buffer.cData()), buffer.length());
}

err_t XmlReader::parseMemory(const void* mem, sysuint_t size)
{
  TextCodec textCodec = _detectEncoding(mem, size);
  if (textCodec.isNull()) textCodec.setCode(TextCodec::UTF8);

  String32 buffer;
  err_t err = textCodec.toUtf32(buffer, Stub8((const char*)mem, size));
  if (err) return err;

  return parseString(buffer.cData(), buffer.length());
}

err_t XmlReader::parseString(const Char32* s, sysuint_t len)
{
  // Check if encoded length is zero (no document).
  if (len == DetectLength) len = StringUtil::len(s);
  if (len == 0) return Error::XmlReaderNoDocument;

  const Char32* strCur = s;           // Parsing buffer.
  const Char32* strEnd = s + len;     // End of buffer.

  const Char32* mark = s;             // Mark to start position of currently parsed item.

  const Char32* markTagStart  = NULL; // Mark to start position of currently parsed tag name.
  const Char32* markTagEnd    = NULL; // Mark to end position of currently parsed tag name.

  const Char32* markAttrStart = NULL; // Mark to start of attribute.
  const Char32* markAttrEnd   = NULL; // Mark to end of attribute.

  const Char32* markDataStart = NULL; // Mark to start of data (CDATA, Comment, attribute text, ...).
  const Char32* markDataEnd   = NULL; // Mark to end of data (CDATA, Comment, attribute text, ...).

  Char32 ch;                          // Current character.
  Char32 attr;                        // Attribute marker (' or ").

  err_t err = Error::Ok;              // Current error code.
  int state = StateReady;             // Current state.
  int element = ElementTag;           // Element type.
  int depth = 0;                      // Current depth.
  bool skipTagText = true;            // skip tag text...?

  // Temporary reusable strings.
  String32 tempTagName;
  String32 tempAttrName;
  String32 tempAttrValue;
  String32 tempText;
  String32 tempData;

  Vector<String32> doctype;

  for (;;)
  {
begin:
    if (strCur == strEnd) break;
    ch = *strCur;

cont:
    switch (state)
    {
      case StateReady:
        // If xml char has special meaning, we will process it, otherwise go away.
        if (ch == Char32('<'))
        {
          // If there is text, we will call addText().
          if (mark != strCur)
          {
            bool isWhiteSpace = xmlIsWhiteSpace(mark, strCur);
            if ( (err = tempText.set(StubUtf32(mark, (sysuint_t)(strCur - mark))) ) ) goto end;
            if ( (err = addText(tempText, isWhiteSpace)) ) goto end;
          }

          state = StateTagBegin;
          mark = strCur;
        }
        break;

      case StateTagBegin:
        // Match start tag name (this is probably the most common)
        if (ch.isAlpha() || ch == Char32('_') || ch == Char32(':'))
        {
          state = StateTagName;
          markTagStart = strCur;
          break;
        }

        // Match closing tag slash.
        if (ch.ch() == Char32('/'))
        {
          state = StateTagClose;
          break;
        }

        if (ch.ch() == Char32('?'))
        {
          state = StateTagQuestionMark;
          break;
        }

        if (ch.ch() == Char32('!'))
        {
          state = StateTagExclamationMark;
          break;
        }

        if (ch.isSpace()) break;

        // Syntax Error
        err = Error::XmlReaderSyntaxError;
        goto end;

      case StateTagName:
        if (ch.isAlnum() || ch == Char32('_') || ch == Char32(':') || ch == Char32('-') || ch == Char32('.'))
          break;

        markTagEnd = strCur;

        state = StateTagInside;
        depth++;
        element = ElementTag;

        if ( (err = tempTagName.set(StubUtf32(markTagStart, (sysuint_t)(markTagEnd - markTagStart)))) ) goto end;
        if ( (err = openElement(tempTagName)) ) goto end;

        // ... go through ...

      case StateTagInside:
        if (ch.isSpace()) break;

        // Check for start of xml attribute.
        if (ch.isAlpha() || ch == Char32('_'))
        {
          markAttrStart = strCur;
          state = StateTagInsideAttrName;
          break;
        }

        // Check for end tag sequence.
        switch (element)
        {
          case ElementTag:
            if (ch == Char32('/'))
            {
              element = ElementSelfClosingTag;
              state = StateTagEnd;
              strCur++;
              goto begin;
            }
            if (ch == Char32('>')) goto tagEnd;
            break;
          case ElementXML:
            if (ch == Char32('?'))
            {
              state = StateTagEnd;
              strCur++;
              goto begin;
            }
            break;
        }

        err = Error::XmlReaderSyntaxError;
        goto end;

      case StateTagInsideAttrName:
        if (ch.isAlnum() || ch == Char32('_') || ch == Char32('-') || ch == Char32('.')) break;

        markAttrEnd = strCur;

        // Now we expect =
        while (ch.isSpace())
        {
          if (++strCur == strEnd) goto endOfInput;
          ch = *strCur;
        }

        if (ch != Char32('=')) { err = Error::XmlReaderSyntaxError; goto end; }

        if (++strCur == strEnd) goto endOfInput;
        ch = *strCur;

        // Now we expect ' or "
        while (ch.isSpace())
        {
          if (++strCur == strEnd) goto endOfInput;
          ch = *strCur;
        }

        if (ch == Char32('\'') || ch == Char32('\"'))
        {
          attr = ch;
          state = StateTagInsideAttrValue;
          markDataStart = ++strCur;
          goto begin;
        }
        else
        {
          err = Error::XmlReaderSyntaxError;
          goto end;
        }

      case StateTagInsideAttrValue:
        if (ch != attr) break;

        markDataEnd = strCur;
        strCur++;
        state = StateTagInside;

        if ( (err = tempAttrName.set(StubUtf32(markAttrStart, (sysuint_t)(markAttrEnd - markAttrStart)))) ) goto end;
        if ( (err = tempAttrValue.set(StubUtf32(markDataStart, (sysuint_t)(markDataEnd - markDataStart)))) ) goto end;
        if ( (err = addAttribute(tempAttrName, tempAttrValue))) goto end;

        goto begin;

      case StateTagEnd:
        if (ch.isSpace()) break;

        if (ch == Char32('>'))
        {
tagEnd:
          state = StateReady;
          mark = ++strCur;

          // Call closeElemet() here only for self-closing ones.
          if (element == ElementSelfClosingTag)
          {
            depth--;
            if ( (err = tempTagName.set(StubUtf32(markTagStart, (sysuint_t)(markTagEnd - markTagStart)))) ) goto end;
            if ( (err = closeElement(tempTagName)) ) goto end;
          }

          goto begin;
        }

        break;

      case StateTagClose:
        // Only possible sequence here is [StartTagSequence].
        if (ch.isAlpha() || ch == Char32('_') || ch == Char32(':'))
        {
          state = StateTagCloseName;
          markTagStart = strCur;
          break;
        }

        err = Error::XmlReaderSyntaxError;
        goto end;

      case StateTagCloseName:
        if (ch.isAlnum() || ch == Char32('_') || ch == Char32(':') || ch == Char32('-') || ch == Char32('.'))
          break;

        state = StateTagCloseEnd;
        markTagEnd = strCur;

        // ...go through ...

      case StateTagCloseEnd:
        // This is we are waiting for.
        if (ch == Char32('>'))
        {
          state = StateReady;
          mark = ++strCur;
          depth--;

          if ( (err = tempTagName.set(StubUtf32(markTagStart, (sysuint_t)(markTagEnd - markTagStart)))) ) goto end;
          if ( (err = closeElement(tempTagName)) ) goto end;

          goto begin;
        }

        if (ch.isSpace()) break;

        // Syntax Error.
        err = Error::XmlReaderSyntaxError;
        goto end;

      case StateTagQuestionMark:
        if ((sysuint_t)(strEnd - strCur) > 3 && StringUtil::eq(strCur, (const Char8*)"xml", 3, CaseInsensitive) && strCur[3].isSpace())
        {
          element = ElementXML;
          state = StateTagInside;
          strCur += 4;
          goto begin;
        }
        else
        {
          markDataStart = strCur;
          state = StatePI;
        }
        break;

      case StateTagExclamationMark:
        if ((sysuint_t)(strEnd - strCur) > 7 && StringUtil::eq(strCur, (const Char8*)"DOCTYPE", 7, CaseInsensitive) && strCur[7].isSpace())
        {
          element = ElementDOCTYPE;
          state = StateDOCTYPE;
          strCur += 8;
          doctype.clear();
          goto begin;
        }
        else
        {
          err = Error::XmlReaderSyntaxError;
          goto end;
        }

      case StateDOCTYPE:
        if (ch.isSpace()) break;

        if (doctype.length() < 2)
        {
          if (ch.isAlpha() || ch == Char32('_'))
          {
            state = StateDOCTYPEText;
            markDataStart = ++strCur;
            goto begin;
          }
        }
        else
        {
          if (ch == Char32('\"'))
          {
            if (doctype.length() < 4)
            {
              state = StateDOCTYPEAttr;
              markDataStart = ++strCur;
              goto begin;
            }
            else
            {
              err = Error::XmlReaderSyntaxError;
              goto end;
            }
          }
          if (ch == Char32('>'))
          {
            if ((err = addDOCTYPE(doctype))) return err;
            state = StateReady;
            mark = ++strCur;
            goto begin;
          }
        }

        err = Error::XmlReaderSyntaxError;
        break;

      case StateDOCTYPEText:
        if (ch.isAlnum() || ch == Char32('_') || ch == Char32(':') || ch == Char32('-') || ch == Char32('.')) break;
        markDataEnd = strCur;
        doctype.append(String32(StubUtf32(markDataStart, (sysuint_t)(markDataEnd - markDataStart))));

        state = StateDOCTYPE;
        goto cont;

      case StateDOCTYPEAttr:
        if (ch != Char32('\"')) break;

        markDataEnd = strCur;
        doctype.append(String32(StubUtf32(markDataStart, (sysuint_t)(markDataEnd - markDataStart))));

        state = StateDOCTYPE;
        break;

      case StatePI:
      {
        const Char32* q = strEnd-1;

        while (strCur < q &&
               strCur[0].ch() != Char32('?') &&
               strCur[1].ch() != Char32('>')) strCur++;

        if (strCur == q)
        {
          err = Error::XmlReaderSyntaxError;
          goto end;
        }
        else
        {
          markDataEnd = strCur;
          strCur += 2;

          state = StateReady;
          mark = strCur;

          if ( (err = tempData.set(StubUtf32(markDataStart, (sysuint_t)(markDataEnd - markDataStart)))) ) goto end;
          if ( (err = addProcessingInstruction(tempData)) ) goto end;

          goto begin;
        }
      }

      case StateComment:
      {
        const Char32* q = strEnd-2;

        while (strCur < q &&
               strCur[0].ch() != Char32('-') &&
               strCur[1].ch() != Char32('-') &&
               strCur[2].ch() != Char32('>')) strCur++;

        if (strCur == q)
        {
          err = Error::XmlReaderSyntaxError;
          goto end;
        }
        else
        {
          markDataEnd = strCur;
          strCur += 3;

          state = StateReady;
          mark = strCur;

          if ( (err = tempData.set(StubUtf32(markDataStart, (sysuint_t)(markDataEnd - markDataStart)))) ) goto end;
          if ( (err = addComment(tempData)) ) goto end;

          goto begin;
        }
      }

      case StateCDATA:
      {
        const Char32* q = strEnd-2;

        while (strCur < q &&
               strCur[0].ch() != Char32(']') &&
               strCur[1].ch() != Char32(']') &&
               strCur[2].ch() != Char32('>')) strCur++;

        if (strCur == q)
        {
          err = Error::XmlReaderSyntaxError;
          goto end;
        }
        else
        {
          markDataEnd = strCur;
          strCur += 3;

          state = StateReady;
          mark = strCur;

          if ( (err = tempData.set(StubUtf32(markDataStart, (sysuint_t)(markDataEnd - markDataStart)))) ) goto end;
          if ( (err = addCDATA(tempData)) ) goto end;

          goto begin;
        }
      }

      default:
        err = Error::XmlReaderUnknown;
        goto end;
    }

    strCur++;
  }

endOfInput:
  if (depth > 0 || state != StateReady)
  {
    err = Error::XmlReaderSyntaxError;
  }

end:
  return err;
}

TextCodec XmlReader::_detectEncoding(const void* mem, sysuint_t size)
{
  // first check for BOM
  TextCodec textCodec = TextCodec::fromBom(mem, size);
  if (!textCodec.isNull()) return textCodec;

  const Char8* ptr = (const Char8 *)mem;
  const Char8* end = ptr + size;

  if (size < 15) goto end;

  while (ptr != end)
  {
    // TODO: Detect UTF16LE, UTF16BE, UTF32LE, UTF32BE

    if  (ptr[0] == Char8('<') && ptr < end - 5 &&
         ptr[1] == Char8('?') &&
        (ptr[2] == Char8('x') || ptr[2] == Char8('X')) &&
        (ptr[3] == Char8('m') || ptr[2] == Char8('M')) &&
        (ptr[4] == Char8('l') || ptr[2] == Char8('L')))
    {
      // Xml header, we are in "<?xml".
      ptr += 5;

      while(ptr + 9 < end)
      {
        if (*ptr == Char8('>')) goto end;
        if (ptr->isAsciiSpace() && StringUtil::eq(ptr + 1, (const Char8*)"encoding", 8, CaseInsensitive))
        {
          // We are in "<?xml ..... encoding".
          const Char8* begin;
          Char8 q;
          ptr += 9;

          // Find '='.
          while (ptr != end && *ptr != Char8('=')) ptr++;
          if (ptr == end) goto end;

          ptr++;

          // We are in "<?xml ..... encoding = "
          while (ptr != end && ptr->isAsciiSpace()) ptr++;
          if (ptr == end) goto end;

          q = *ptr++;
          begin = ptr;

          while (ptr != end && *ptr != q) ptr++;
          if (ptr == end) goto end;

          // Try encoding and return
          textCodec = TextCodec::fromMime(StubAscii8(begin, (sysuint_t)(ptr - begin)));
          goto end;
        }
        ptr++;
      }
    }
    else if (ptr[0] == Char8('<'))
    {
      // xml header not found, default encoding is UTF-8
      break;
    }

    ptr++;
  }

end:
  return textCodec;
}

// ============================================================================
// [Fog::XmlDomReader]
// ============================================================================

XmlDomReader::XmlDomReader(XmlDocument* document) :
  _document(document),
  _current(document)
{
}

XmlDomReader::~XmlDomReader()
{
}

err_t XmlDomReader::openElement(const String32& tagName)
{
  XmlElement* e = new(std::nothrow) XmlElement(tagName);
  if (!e) return Error::OutOfMemory;

  err_t err;

  if (_current == _document)
  {
    if (_document->documentRoot() == NULL)
      err = _document->setDocumentRoot(e);
    else
      err = Error::XmlDomDocumentHasAlreadyRoot;
  }
  else
  {
    err = _current->appendChild(e);
  }

  if (err)
  {
    delete e;
    return err;
  }
  else
  {
    _current = e;
    return Error::Ok;
  }
}

err_t XmlDomReader::closeElement(const String32& tagName)
{
  if (_current != _document)
  {
    _current = _current->parent();
    return Error::Ok;
  }
  else
  {
    return Error::XmlDomInvalidClosingTag;
  }
}

err_t XmlDomReader::addAttribute(const String32& name, const String32& data)
{
  return _current->setAttribute(name, data);
}

err_t XmlDomReader::addText(const String32& data, bool isWhiteSpace)
{
  if (_current == _document)
  {
    if (isWhiteSpace)
      return Error::Ok;
    else
      return Error::XmlReaderSyntaxError;
  }

  XmlElement* e = new XmlText(data);
  if (!e) return Error::OutOfMemory;

  err_t err;
  if ((err = _current->appendChild(e))) delete e;
  return err;
}

err_t XmlDomReader::addCDATA(const String32& data)
{
  if (_current == _document) return Error::XmlDomNotAllowed;

  XmlElement* e = new XmlCDATA(data);
  if (!e) return Error::OutOfMemory;

  err_t err;
  if ((err = _current->appendChild(e))) delete e;
  return err;
}

err_t XmlDomReader::addDOCTYPE(const Vector<String32>& doctype)
{
  if (_current != _document) return Error::XmlDomNotAllowed;

  // TODO:
  // return _document->setDOCTYPE(doctype);

  return Error::Ok;
}

err_t XmlDomReader::addProcessingInstruction(const String32& data)
{
  if (_current == _document) return Error::XmlDomNotAllowed;

  XmlElement* e = new XmlProcessingInstruction(data);
  if (!e) return Error::OutOfMemory;

  err_t err;
  if ((err = _current->appendChild(e))) delete e;
  return err;
}

err_t XmlDomReader::addComment(const String32& data)
{
  if (_current == _document) return Error::XmlDomNotAllowed;

  XmlElement* e = new XmlComment(data);
  if (!e) return Error::OutOfMemory;

  err_t err;
  if ((err = _current->appendChild(e))) delete e;
  return err;
}

} // Fog namespace
