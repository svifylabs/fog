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

        if ( (err = tempTagName.set(StubUtf32(markTagStart, (sysuint_t)(markTagEnd - markTagStart)))) ) goto end;
        if ( (err = openElement(tempTagName)) ) goto end;

        state = StateTagInside;
        depth++;
        element = ElementTag;

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
          depth--;

          // Do not call closeElement() for <?xml ?> declaration.
          if (element == ElementTag)
          {
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
          if ( (err = tempTagName.set(StubUtf32(markTagStart, (sysuint_t)(markTagEnd - markTagStart)))) ) goto end;
          if ( (err = closeElement(tempTagName)) ) goto end;

          state = StateReady;
          mark = ++strCur;
          depth--;
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
          goto begin;
        }
        else
        {
          err = Error::XmlReaderSyntaxError;
          goto end;
        }

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

err_t XmlDomReader::addDOCTYPE(const String32& doctype)
{
  if (_current != _document) return Error::XmlDomNotAllowed;
  return _document->setDOCTYPE(doctype);
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









// STOPPED HERE!
#if 0
#define __XML_CHAR_ERROR(n) return n
  //XmlNode* doc = _doc._root; // new xml tree
  //XmlNode* cur;              // current node
  //XmlAttribute *attributes;  // attributes

  // ===== Find first tag =====
  for (;;)
  {
    if (strCur == strEnd) __XML_CHAR_ERROR(Error::XmlReaderMissingRootTag);
    else if (strCur->isSpace()) strCur++;
    else if (*strCur == '<') break;
    else __XML_CHAR_ERROR(Error::XmlReaderMissingRootTag);
  }

  // ---------------------------------------------------------------------
  // ===== Loop (*strCur is now '<' --- beginning of first tag) =====
  for (;;)
  {
    // next character
    strCur++;

    // ----- New tag...? -----
    if (strCur->isAlpha() || *strCur == '_' || *strCur == ':' || *strCur > 127)
    {
      if (!cur) __XML_CHAR_ERROR(Error::XmlReaderUnmatchedClosingTag);

      // === Clean attributes ===
      attributes = NULL;

      // === Parse tag name ===
      const Char32* tag = strCur;
      uint tagLength = xmlStrCSPN(strCur, strEnd, XML_CHAR_SPACE | XML_CHAR_SLASH | XML_CHAR_GT);

      strCur += tagLength;

      // === New tag attributes ===
      while (strCur != strEnd && *strCur != '/' && *strCur != '>')
      {
        // Skip spaces
        while (strCur->isSpace()) strCur++;

        // Temp variable where is stored beginning of attribute name or text
        const Char32* begin = strCur;

        strCur += xmlStrCSPN(strCur, strEnd, XML_CHAR_SPACE | XML_CHAR_EQUAL | XML_CHAR_SLASH | XML_CHAR_GT);
        if (*strCur == '=' || strCur->isSpace())
        {
          // Create a new attribute
          {
            XmlAttribute *a = new XmlAttribute();

            a->_prev = attributes;
            a->_next = NULL;
            a->setName( String(begin, (sysuint_t)(strCur - begin)) );

            if (attributes) attributes->_next = a;
            attributes = a;
          }

          // q can be " or '
          Char32 q = *(strCur += xmlStrSPN(strCur, strEnd, XML_CHAR_SPACE | XML_CHAR_EQUAL));

          // Get attribute value
          if (q == '\"' || q == '\'')
          {
            // Save begin of attribute text
            begin = ++strCur;
            while (strCur != strEnd && *strCur != q) strCur++;

            // Copy attribute text to attribute object Text must be decoded
            decode(attributes->_value, begin, (sysuint_t)(strCur - begin));

            // Assign last node, but this node can be damaged.
            if (strCur == strEnd)
            {
              openTag(&cur, tag, tagLength, attributes);
              closeTag(&cur);

              __XML_CHAR_ERROR(Error::XmlReaderMissingAttribute);
            }

            strCur++;
          }
        }
      }

      // === Self closing tag...? ===
      if (*strCur == '/')
      {
        // Assign node
        openTag(&cur, tag, tagLength, attributes);
        closeTag(&cur);

        // Skip spaces...Allow <SelfClosingNode/ >
        for (;;)
        {
          // strEnd of input will cause error
          if (++strCur == strEnd) __XML_CHAR_ERROR(Error::XmlReaderMissingTag);
          // Skip spaces
          if (strCur->isSpace()) continue;
          // 'Only '>' is valid
          else if (*strCur == '>')
            break;
          else
            __XML_CHAR_ERROR(Error::XmlReaderMissingTag);
        }
      }

      // === Open tag...? ===
      else if (*strCur == '>')
      {
        openTag(&cur, tag, tagLength, attributes);
      }

      // === Error, Missing > ===
      else
      {
        // Assign last node, but this node can be damaged.
        openTag(&cur, tag, tagLength, attributes);
        closeTag(&cur);

        __XML_CHAR_ERROR(Error::XmlReaderMissingTag);
      }

      skipTagText = false;
    }


    // ----- Close tag -----
    else if (*strCur == '/')
    {
      if (!cur) __XML_CHAR_ERROR(Error::XmlReaderUnmatchedClosingTag);

      closeTag(&cur);
      while (strCur != strEnd && *strCur != '>') strCur++;
      if (strCur == strEnd) __XML_CHAR_ERROR(Error::XmlReaderMissingTag);
    }


    // ----- Comment -----
    else if (strCur + 3 <= strEnd && String::Raw::eq(strCur, "!--", 3))
    {
      const Char32* dataStart = strCur += 3;

      for (;;)
      {
        if (strCur + 3 >= strEnd) __XML_CHAR_ERROR(Error::XmlReaderUnclosedComment);
        else if (StringUtil::eq(strCur, "-->", 3)) { strCur += 3; break; }
        else strCur++;
      }

      const Char32* dataEnd = strCur - 3;
      addComment(String32(StubUtf32(dataStart, (sysuint_t)(dataEnd - dataStart))));
    }


    // ----- CDATA -----
    else if (strCur + 8 <= strEnd && String::Raw::eq(strCur, "![CDATA[", 8))
    {
#if 0
      //FIXME:
      if ((strCur = WUniStrStr(strCur, U_("]]>")) )) {
        //WXmlCharContent(root, D + 8, (S += 2) - D - 10, 0);
      }
      else
        __XML_CHAR_ERROR(Error::XmlReaderUnclosedCDATA);
#endif
    }


    // ----- !DOCTYPE -----
    else if (strCur + 8 <= strEnd && String::Raw::eq(strCur, "!DOCTYPE", 8))
    {
      uint i;
      for (i = 0; strCur != strEnd && ((!i && *strCur != '>') || (i && (*strCur != ']' ||
                   strCur[xmlStrSPN(strCur + 1, strEnd, XML_CHAR_SPACE) + 1] != '>')));
                   i = (*strCur == '[') ? 1 : i)
      {
        strCur += xmlStrCSPN(strCur + 1, strEnd, XML_CHAR_BRACKETS | XML_CHAR_GT) + 1;
      }

      // Unclosed <!DOCTYPE
      if (strCur == strEnd) __XML_CHAR_ERROR(Error::XmlReaderUnclosedDOCTYPE);
    }


    // ----- <?...?> processing instructions -----
    else if (*strCur == '?')
    {
      Char32* dataStart = ++strCur;

      do {
        strCur = xmlStrCHR(strCur, strEnd, Char32('?'));
      } while (strCur && ++strCur != strEnd && *strCur != '>');

      // Unclosed <?
      if (!strCur)
      {
        __XML_CHAR_ERROR(Error::XmlReaderUnclosedPI);
      }
      else
      {
        const Char32* dataEnd = strCur - 3;
        addProcessingInstruction(String32(StubUtf32(dataStart, (sysuint_t)(dataEnd - dataStart))));
      }
    }


    // ----- Syntax Error -----
    else
    {
      __XML_CHAR_ERROR(EXmlSyntaxError);
    }


    // ----- check for end -----
    if (strCur == strEnd) break;


    // ----- tag character content -----
    {
      const Char32* begin = ++strCur;
      while (strCur != strEnd && *strCur != '<') strCur++;

      if (strCur == strEnd) break;
      if (skipTagText) continue;

      // Remove white spaces, but don't remove &..; spaces
      while (begin->isSpace()) begin++;

      const Char32* tr = strCur-1;
      while (tr > begin && tr->isSpace()) tr--;
      tr++;

      // Decode Xml text (&..;) to text
      if (tr - begin) decode(cur->_text, begin, sysuint_t( tr - begin ));
      skipTagText = true;
    }
  }
  // ---------------------------------------------------------------------

  if (cur)
  {
    if (cur->tag().isEmpty())
      __XML_CHAR_ERROR(Error::XmlReaderMissingRootTag);
    else
      __XML_CHAR_ERROR(Error::XmlReaderMissingTag);
  }

  return err;
#undef __XML_CHAR_ERROR
#endif

#if 0
// Decode xml &entities; into normal unicode text
void XmlReader::decode(String& dest, const Char32* buffer, sysuint_t length)
{
  // This will probabbly never happen
  if (length == DetectLength) length = String::Raw::len(buffer);

  const Char32* end = buffer + length;
  Char32* p = dest._reserve(length);

__begin:
  while (buffer != end)
  {
    if (*buffer == '&')
    {
      const Char32* begin = ++buffer;

      while (buffer != end)
      {
        if (*buffer == ';')
        {
          Char32 uc = XmlEntity::decode(begin, sysuint_t( buffer - begin ));
          buffer++;
          if (uc.ch())
          {
            *p++ = uc;
            goto __begin;
          }
          else
            goto __copy;
        }
        buffer++;
      }

      // end of input, parse everything
      core_stderr_msg("XmlReader", "decode", "End of input in entity");
__copy:
      {
        sysuint_t i = sysuint_t( buffer - (--begin) );
        String::Raw::copy(p, begin, i);
        p += i;
      }
      if (buffer == end)
        break;
      else
        goto __begin;
    }
    *p++ = *buffer++;
  }

  dest.xFinalize(p);
}

struct CORE_HIDDEN XmlWriter
{
  XmlDocument& _doc;   // link to xml document
  Stream& _stream;     // link to stream
  ByteArray _target;   // target encoded buffer

  TextCodec _textCodec;
  TextCodec::State _textState;

  XmlWriter(XmlDocument& doc, Stream& stream) :
    _doc(doc),
    _stream(stream),
    _textCodec(doc.textCodec())
  {
    // reserve memory
    _target.reserve(1024);

    // setup encoder
    _textState.map = xml_escape_mask;
    _textState.replace = xml_escape_fn;
  }

  void run();
  void writeNodesR(const XmlNode* node, uint depth);
  void writeAttributes(const XmlNode* node);

  inline void writeRaw(const String& s)
  { _textCodec.appendFromUtf32(_target, s); }

  inline void writeStr(const String& s)
  { _textCodec.appendFromUtf32(_target, s, &_textState); }

  inline void writeChar(Char32 uc)
  { _textCodec.appendFromUtf32(_target, &uc, sizeof(Char32)); }

  inline void writeSpaces(sysuint_t count)
  {
    sysuint_t i;
    for (i = 0; i != count; i++) writeChar(Char32(' '));
  }

  inline void flush() { _stream.write(_target); _target.clear(); }
};

void XmlWriter::run()
{
  String str;
  str.format("<?xml version=\"1.0\" encoding=\"%s\" ?>\n", _textCodec.mime());

  writeRaw(str);
  writeNodesR(_doc.constRoot(), 0);

  flush();
}

void XmlWriter::writeNodesR(const XmlNode* node, uint depth)
{
  const XmlNode* current = node;

  while (current)
  {
    writeSpaces(depth);

    // Make all unnamed tags as 'Unnamed'
    String tag;
    if (current->tag().isEmpty())
      tag = "Unnamed";
    else
      tag = current->tag();

    if (current->children())
    {
      // Not self closing tag

      // Write <Tag [Attributes]>
      writeChar(Char32('<'));
      writeStr(tag);
      writeAttributes(current);
      writeChar(Char32('>'));

      // Write Text\n
      writeStr(current->text());
      writeChar(Char32('\n'));

      // Write children
      writeNodesR(current->children(), depth+1);

      // Write </Tag>\n
      writeSpaces(depth);
      writeChar(Char32('<'));
      writeChar(Char32('/'));
      writeStr(tag);
      writeChar(Char32('>'));
      writeChar(Char32('\n'));
    }
    else
    {
      // Self closing tag
      if (current->text().isEmpty())
      {
        // Write <Tag [Attributes] />\n
        writeChar(Char32('<'));
        writeStr(tag);
        writeAttributes(current);
        writeChar(Char32(' '));
        writeChar(Char32('/'));
        writeChar(Char32('>'));
        writeChar(Char32('\n'));
      }
      else
      {
        // Write <Tag [Attributes]>Text</Tag>\n
        writeChar(Char32('<'));
        writeStr(tag);
        writeAttributes(current);
        writeChar(Char32('>'));
        writeStr(current->text());
        writeChar(Char32('<'));
        writeChar(Char32('/'));
        writeStr(tag);
        writeChar(Char32('>'));
        writeChar(Char32('\n'));
      }
    }

    current = current->next();
    if (_target.length() > 512) flush();
  }
}

void XmlWriter::writeAttributes(const XmlNode* node)
{
  static const Char32 q = Char32('\"');

  const XmlAttribute* current = node->attributes();

  if (!current) return;

  while (current)
  {
    // Write Attribute="Text"
    writeChar(Char32(' '));
    writeStr(current->name());
    writeChar(Char32('='));
    writeChar(q);
    writeStr(current->value());
    writeChar(q);

    current = current->next();
  }
}

Value XmlDocument::writeFile(const String& fileName)
{
  Stream stream;
  Value result;

  result = stream.openFile(fileName,
    Stream::Open_Write |
    Stream::Open_Truncate |
    Stream::Open_Create |
    Stream::Open_CreatePath);

  if (result.ok())
    return writeStream(stream);
  else
    return result;
}

Value XmlDocument::writeStream(Core::Stream& stream)
{
  XmlWriter w(*this, stream);
  w.run();

  return Core::Value();
  //return stream.error();
}

// [Core::]
}
#endif
