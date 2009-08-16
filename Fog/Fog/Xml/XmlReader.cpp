// [Fog/Xml Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled Headers]
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
#include <Fog/Xml/Error.h>
#include <Fog/Xml/XmlDom.h>
#include <Fog/Xml/XmlEntity.h>
#include <Fog/Xml/XmlReader.h>

namespace Fog {

// ============================================================================
// [Fog::XmlReader - Helpers]
// ============================================================================

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
  return parseMemory(reinterpret_cast<const void*>(buffer.cData()), buffer.getLength());
}

err_t XmlReader::parseMemory(const void* mem, sysuint_t size)
{
  TextCodec textCodec = _detectEncoding(mem, size);
  if (textCodec.isNull()) textCodec.setCode(TextCodec::UTF8);

  String32 buffer;
  err_t err = textCodec.toUtf32(buffer, Stub8((const char*)mem, size));
  if (err) return err;

  return parseString(buffer.cData(), buffer.getLength());
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

            err = onAddText(Utf32(mark, (sysuint_t)(strCur - mark)), isWhiteSpace);
            if (err) goto end;
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

        err = onAddElement(Utf32(markTagStart, (sysuint_t)(markTagEnd - markTagStart)));
        if (err) goto end;

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
        if (ch.isAlnum()  || ch == Char32('_') || ch == Char32(':') || ch == Char32('-') || ch == Char32('.')) break;

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

        err = onAddAttribute(
          Utf32(markAttrStart, (sysuint_t)(markAttrEnd - markAttrStart)),
          Utf32(markDataStart, (sysuint_t)(markDataEnd - markDataStart)));
        if (err) goto end;

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
            err = onCloseElement(Utf32(markTagStart, (sysuint_t)(markTagEnd - markTagStart)));
            if (err) goto end;
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

          err = onCloseElement(Utf32(markTagStart, (sysuint_t)(markTagEnd - markTagStart)));
          if (err) goto end;

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
        if ((sysuint_t)(strEnd - strCur) > 1 && StringUtil::eq(strCur, (const Char8*)"--", 2, CaseSensitive))
        {
          state = StateComment;
          strCur += 3;
          goto begin;
        }
        else if ((sysuint_t)(strEnd - strCur) > 7 && StringUtil::eq(strCur, (const Char8*)"DOCTYPE", 7, CaseInsensitive) && strCur[7].isSpace())
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

        if (doctype.getLength() < 2)
        {
          if (ch.isAlpha() || ch == Char32('_'))
          {
            state = StateDOCTYPEText;
            markDataStart = ++strCur;
            goto begin;
          }

          // End of DOCTYPE
          if (ch == Char32('>')) goto doctypeEnd;
        }
        else
        {
          if (ch == Char32('\"'))
          {
            if (doctype.getLength() < 4)
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
doctypeEnd:
            if ((err = onAddDOCTYPE(doctype))) return err;
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
        doctype.append(String32(Utf32(markDataStart, (sysuint_t)(markDataEnd - markDataStart))));

        state = StateDOCTYPE;
        goto cont;

      case StateDOCTYPEAttr:
        if (ch != Char32('\"')) break;

        markDataEnd = strCur;
        doctype.append(String32(Utf32(markDataStart, (sysuint_t)(markDataEnd - markDataStart))));

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

          err = onAddPI(Utf32(markDataStart, (sysuint_t)(markDataEnd - markDataStart)));
          if (err) goto end;

          goto begin;
        }
      }

      case StateComment:
      {
        const Char32* q = strEnd-2;

        while (strCur < q && (
               strCur[0].ch() != Char32('-') ||
               strCur[1].ch() != Char32('-') ||
               strCur[2].ch() != Char32('>'))) strCur++;

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

          err = onAddComment(Utf32(markDataStart, (sysuint_t)(markDataEnd - markDataStart)));
          if (err) goto end;

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

          err = onAddCDATA(Utf32(markDataStart, (sysuint_t)(markDataEnd - markDataStart)));
          if (err) goto end;

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
          textCodec = TextCodec::fromMime(Ascii8(begin, (sysuint_t)(ptr - begin)));
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

err_t XmlDomReader::onAddElement(const Utf32& tagName)
{
  XmlElement* e = _document->createElement(ManagedString32(tagName));
  if (!e) return Error::OutOfMemory;

  err_t err = _current->appendChild(e);
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

err_t XmlDomReader::onCloseElement(const Utf32& tagName)
{
  if (_current != _document)
  {
    _current = _current->getParent();
    return Error::Ok;
  }
  else
  {
    return Error::XmlDomInvalidClosingTag;
  }
}

err_t XmlDomReader::onAddAttribute(const Utf32& name, const Utf32& value)
{
  return _current->setAttribute(ManagedString32(name), String32(value));
}

err_t XmlDomReader::onAddText(const Utf32& data, bool isWhiteSpace)
{
  if (_current == _document)
  {
    if (isWhiteSpace)
      return Error::Ok;
    else
      return Error::XmlReaderSyntaxError;
  }

  XmlElement* e = new(std::nothrow) XmlText(String32(data));
  if (!e) return Error::OutOfMemory;

  err_t err = _current->appendChild(e);
  if (err) delete e;
  return err;
}

err_t XmlDomReader::onAddCDATA(const Utf32& data)
{
  if (_current == _document) return Error::XmlDomDocumentInvalidChild;

  XmlElement* e = new(std::nothrow) XmlCDATA(String32(data));
  if (!e) return Error::OutOfMemory;

  err_t err = _current->appendChild(e);
  if (err) delete e;
  return err;
}

err_t XmlDomReader::onAddDOCTYPE(const Vector<String32>& doctype)
{
  if (_current != _document) return Error::XmlDomDocumentInvalidChild;

  // XML TODO:
  // return _document->setDOCTYPE(doctype);

  return Error::Ok;
}

err_t XmlDomReader::onAddPI(const Utf32& data)
{
  XmlElement* e = new(std::nothrow) XmlPI(String32(data));
  if (!e) return Error::OutOfMemory;

  err_t err = _current->appendChild(e);
  if (err) delete e;
  return err;
}

err_t XmlDomReader::onAddComment(const Utf32& data)
{
  XmlElement* e = new(std::nothrow) XmlComment(String32(data));
  if (!e) return Error::OutOfMemory;

  err_t err = _current->appendChild(e);
  if (err) delete e;
  return err;
}

} // Fog namespace
