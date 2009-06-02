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
#include <Fog/Core/TextCodec.h>
#include <Fog/Core/Vector.h>
#include <Fog/Xml/Entity.h>
#include <Fog/Xml/Error.h>
#include <Fog/Xml/Reader.h>

namespace Fog {

// ============================================================================
// [Fog::XmlReader]
// ============================================================================

XmlReader::XmlReader() :
  _status(StatusReady),
  _encodingParsed(false)
{
  // Default text codec for xml streams.
  _textCodec = TextCodec::fromCode(TextCodec::UTF8);
}

XmlReader::~XmlReader()
{
}

void XmlReader::setStream(Stream& stream)
{
  _stream = stream;
}

void XmlReader::resetStream()
{
  _stream = Stream();
}

} // Fog namespace

// STOPPED HERE!

#if 0
// [Core::XmlDocument]

// Special xml characters
static const uchar xmlChars[128] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x40,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x04, 0x02, 0x08, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x20, 0x00, 0x20, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

enum
{
  XML_SPACE    = (1 << 0), /* '\t''\r''\n'' ' */
  XML_EQUAL    = (1 << 1), /* = */
  XML_LT       = (1 << 2), /* < */
  XML_GT       = (1 << 3), /* > */
  XML_SLASH    = (1 << 4), /* / */
  XML_BRACKETS = (1 << 5), /* [] */
  XML_QUOT     = (1 << 6) /* '" */
};

struct CORE_HIDDEN XmlReader
{
  XmlDocument& _doc;
  uint32_t _readFlags;

  CORE_INLINE XmlReader(XmlDocument& doc, uint32_t readFlags = 0)
    : _doc(doc), _readFlags(readFlags)
  {
  }

  CORE_INLINE ~XmlReader()
  {
  }

  Value readUnicode(const Char* str, sysuint_t length);

  static bool checkForEncoding(const void* mem, sysuint_t length, TextCodec& textCodec);
  static uint strCSPN(const Char* buffer, const Char* end, uint reject_mask);
  static uint strSPN(const Char* buffer, const Char* end, uint accept_mask);
  static const Char* strCHR(const Char* buffer, const Char* end, Char uc);
  static void decode(String& dest, const Char* buffer, sysuint_t length);
  static void openTag(XmlNode** pCur, const Char* tag, sysuint_t tagLength, XmlAttribute* last);
  static void closeTag(XmlNode** pCur);
};

Value XmlReader::readUnicode(const Char* str, sysuint_t length)
{
  #define __XML_ERROR(n) do { errorCode = n; goto error; } while(0)

  /* ===== Local variables ===== */
  Value result;
  uint32_t errorCode;

  XmlNode* doc = _doc._root; // new xml tree
  XmlNode* cur;              // current node
  XmlAttribute *attributes;  // attributes
  bool skipTagText = true;   // skip tag text...?

  const Char* strCur;        // parsing buffer
  const Char* strEnd;        // end of buffer

  /* ===== Check if encoded length is zero (no document) ===== */
  if (length == DetectLength) length = String::Raw::len(str);
  if (length == 0) __XML_ERROR(EXmlNoDocument);

  /* ===== Initialize parser ===== */
  strCur = str;
  strEnd = strCur + length;

  /* ===== Initialize root tag ===== */
  cur = doc;

  /* ===== Find first tag ===== */
  for (;;)
  {
    if (strCur == strEnd) __XML_ERROR(EXmlMissingRootTag);
    else if (strCur->isSpace()) strCur++;
    else if (*strCur == '<') break;
    else __XML_ERROR(EXmlMissingRootTag);
  }

  /* --------------------------------------------------------------------- */
  /* ===== Loop (*strCur is now '<' --- beginning of first tag) ===== */
  for (;;)
  {
    /* next character */
    strCur++;

    /* ----- New tag...? ----- */
    if (strCur->isAlpha() || *strCur == '_' || *strCur == ':' || *strCur > 127)
    {
      if (!cur) __XML_ERROR(EXmlUnmatchedClosingTag);

      /* === Clean attributes === */
      attributes = NULL;

      /* === Parse tag name === */
      const Char* tag = strCur;
      uint tagLength = strCSPN(strCur, strEnd, XML_SPACE | XML_SLASH | XML_GT);

      strCur += tagLength;

      /* === New tag attributes === */
      while (strCur != strEnd && *strCur != '/' && *strCur != '>')
      {
        // Skip spaces
        while (strCur->isSpace()) strCur++;

        // Temp variable where is stored beginning of attribute name or text
        const Char* begin = strCur;

        strCur += strCSPN(strCur, strEnd, XML_SPACE | XML_EQUAL | XML_SLASH | XML_GT);
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

          // q can be (") or (')
          Char q = *(strCur += strSPN(strCur, strEnd, XML_SPACE | XML_EQUAL));

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

              __XML_ERROR(EXmlMissingAttribute);
            }

            strCur++;
          }
        }
      }

      /* === Self closing tag...? === */
      if (*strCur == '/')
      {
        /* Assign node */
        openTag(&cur, tag, tagLength, attributes);
        closeTag(&cur);

        /* Skip spaces...Allow <SelfClosingNode/ > */
        for (;;)
        {
          /* strEnd of input will cause error */
          if (++strCur == strEnd) __XML_ERROR(EXmlMissingTag);
          /* Skip spaces */
          if (strCur->isSpace()) continue;
          /* 'Only '>' is valid */
          else if (*strCur == '>')
            break;
          else
            __XML_ERROR(EXmlMissingTag);
        }
      }

      /* === Open tag...? === */
      else if (*strCur == '>')
      {
        openTag(&cur, tag, tagLength, attributes);
      }

      /* === Error, Missing > === */
      else
      {
        /* Assign last node, but this node can be damaged. */
        openTag(&cur, tag, tagLength, attributes);
        closeTag(&cur);

        __XML_ERROR(EXmlMissingTag);
      }

      skipTagText = false;
    }


    /* ----- Close tag ----- */
    else if (*strCur == '/')
    {
      if (!cur) __XML_ERROR(EXmlUnmatchedClosingTag);

      closeTag(&cur);
      while (strCur != strEnd && *strCur != '>') strCur++;
      if (strCur == strEnd) __XML_ERROR(EXmlMissingTag);
    }


    /* ----- Comment ----- */
    else if (strCur + 3 <= strEnd && String::Raw::eq(strCur, "!--", 3))
    {
      strCur += 3;
      for (;;)
      {
        if (strCur + 3 >= strEnd) __XML_ERROR(EXmlMissingComment);
        else if (String::Raw::eq(strCur, "-->", 3)) { strCur += 3; break; }
        else strCur++;
      }
    }


    /* ----- CDATA ----- */
    else if (strCur + 8 <= strEnd && String::Raw::eq(strCur, "![CDATA[", 8))
    {
#if 0
      //FIXME:
      if ((strCur = WUniStrStr(strCur, U_("]]>")) )) {
        //WXmlCharContent(root, D + 8, (S += 2) - D - 10, 0);
      }
      else
        __XML_ERROR(EXmlXmlCDataMissing);
#endif
    }


    /* ----- !DOCTYPE ----- */
    else if (strCur + 8 <= strEnd && String::Raw::eq(strCur, "!DOCTYPE", 8))
    {
      uint i;
      for (i = 0; strCur != strEnd && ((!i && *strCur != '>') || (i && (*strCur != ']' ||
                   strCur[strSPN(strCur + 1, strEnd, XML_SPACE) + 1] != '>')));
                   i = (*strCur == '[') ? 1 : i)
      {
        strCur += strCSPN(strCur + 1, strEnd, XML_BRACKETS | XML_GT) + 1;
      }

      /* Unclosed <!DOCTYPE */
      if (strCur == strEnd) __XML_ERROR(EXmlMissingDocType);
    }


    /* ----- <?...?> processing instructions ----- */
    else if (*strCur == '?')
    {
      do {
        strCur = strCHR(strCur, strEnd, Char('?'));
      } while (strCur && ++strCur != strEnd && *strCur != '>');

      /* Unclosed <? */
      if (!strCur)
      {
        __XML_ERROR(EXmlMissingPreprocessor);
      }
      else
      {
        //FIXME:continue;
        //FIXME:To tady nebylo continue
        //FIXME:Co ted ?
        //FIXME:;//XmlProcInst(doc, D + 1, strCur - D - 2);
      }
    }


    /* ----- Syntax Error ----- */
    else
    {
      __XML_ERROR(EXmlSyntaxError);
    }


    /* ----- check for end ----- */
    if (strCur == strEnd) break;


    /* ----- tag character content ----- */
    {
      const Char* begin = ++strCur;
      while (strCur != strEnd && *strCur != '<') strCur++;

      if (strCur == strEnd) break;
      if (skipTagText) continue;

      /* Remove white spaces, but don't remove &..; spaces */
      while (begin->isSpace()) begin++;

      const Char* tr = strCur-1;
      while (tr > begin && tr->isSpace()) tr--;
      tr++;

      /* Decode Xml text (&..;) to text */
      if (tr - begin) decode(cur->_text, begin, sysuint_t( tr - begin ));
      skipTagText = true;
    }
  }
  /* --------------------------------------------------------------------- */

  if (cur)
  {
    if (cur->tag().isEmpty())
      __XML_ERROR(EXmlMissingRootTag);
    else
      __XML_ERROR(EXmlMissingTag);
  }

  return result;
error:
  return result.setError(ErrorDomain_Core, errorCode);
  #undef __XML_ERROR
}

bool XmlReader::checkForEncoding(const void* mem, sysuint_t length, TextCodec& textCodec)
{
  // first check for BOM
  if (!(textCodec = TextCodec::fromBom(mem, length)).isNull()) return true;
  if (length < 15) return false;

  const char* ptr = (const char *)mem;
  const char* end = ptr + length;

  while (ptr != end)
  {
    // TODO: Detect UTF16LE, UTF16BE, UTF32LE, UTF32BE

    if  (ptr[0] == '<' && ptr < end - 5 &&
         ptr[1] == '?' &&
        (ptr[2] == 'x' || ptr[2] == 'X') &&
        (ptr[3] == 'm' || ptr[2] == 'M') &&
        (ptr[4] == 'l' || ptr[2] == 'L'))
    {
      // Xml header, we are in <?xml
      ptr += 5;

      while(ptr + 9 < end)
      {
        if (*ptr == '>') goto end;
        if (CType::isAsciiSpace(*ptr) && ByteArray::Raw::ieq(ptr + 1, "encoding", 8))
        {
          /* we are in <?xml ..... encoding */
          const char* begin;
          char q;
          ptr += 9;

          /* Find '=' */
          while (ptr != end && *ptr != '=') ptr++;
          if (ptr == end) goto end;

          ptr++;

          /* we are in <?xml ..... encoding = */
          while (ptr != end && CType::isAsciiSpace(*ptr)) ptr++;
          if (ptr == end) goto end;

          q = *ptr++;
          begin = ptr;

          while (ptr != end && *ptr != q) ptr++;
          if (ptr == end) goto end;

          /* Try encoding and return */
          textCodec = TextCodec::fromMime(String(begin, (sysuint_t)(ptr - begin)));
          goto end;
        }
        ptr++;
      }
    }
    else if (ptr[0] == '<')
      // xml header not found, default encoding is UTF-8
      break;
    ptr++;
  }

end:
  return !textCodec.isNull();
}

// Optimized strcspn for xml reading with mask instead of string
uint XmlReader::strCSPN(const Char* buffer, const Char* end, uint reject_mask)
{
  const Char* start = buffer;

  while (buffer < end)
  {
    if (*buffer < 128 && (xmlChars[buffer->uc()] & reject_mask)) break;
    buffer++;
  }

  return sysuint_t( buffer - start );
}

// Optimized strspn for xml reading with mask instead of string
uint XmlReader::strSPN(const Char* buffer, const Char* end, uint accept_mask)
{
  const Char* start = buffer;

  while (buffer < end)
  {
    if (*buffer > 127 || !(xmlChars[buffer->uc()] & accept_mask)) break;
    buffer++;
  }

  return sysuint_t( buffer - start );
}

const Char* XmlReader::strCHR(const Char* buffer, const Char* end, Char uc)
{
  while (buffer < end && *buffer != uc) buffer++;
  return buffer;
}

// Decode xml &entities; into normal unicode text
void XmlReader::decode(String& dest, const Char* buffer, sysuint_t length)
{
  // This will probabbly never happen
  if (length == DetectLength) length = String::Raw::len(buffer);

  const Char* end = buffer + length;
  Char* p = dest._reserve(length);

__begin:
  while (buffer != end)
  {
    if (*buffer == '&')
    {
      const Char* begin = ++buffer;

      while (buffer != end)
      {
        if (*buffer == ';')
        {
          Char uc = XmlEntity::decode(begin, sysuint_t( buffer - begin ));
          buffer++;
          if (uc.uc())
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

// Called when parser finds begin of tag.
void XmlReader::openTag(XmlNode** pCur, const Char* tag, sysuint_t tagLength, XmlAttribute* last)
{
  XmlNode* cur = *pCur;

  if (!cur->tag().isEmpty())
  {
    XmlNode* node = new XmlNode();
    node->_parent = cur;

    // not root tag
    if (cur->children())
    {
      // already have sub tags
      node->_prev = cur->_last;
      cur->_last->_next = node;
      cur->_last = node;
    }
    else
    {
      // first sub tag
      cur->_children = node;
      cur->_last = node;
    }
    *pCur = cur = node;
  }

  // initialize new tag
  cur->setTag(String(tag, tagLength));

  // assign attributes
  if (last)
  {
    while (last->prev()) last = last->prev();
  }

  cur->_attributes = last;
}

void XmlReader::closeTag(XmlNode** pCur)
{
  *pCur = (*pCur)->parent();
}

XmlDocument::XmlDocument()
{
  _root = new XmlNode();
}

XmlDocument::~XmlDocument()
{
  delete _root;
}

Value XmlDocument::readFile(const String& fileName, uint32_t readFlags)
{
  MapFile f;
  Value result = f.map(fileName);

  if (result.ok())
    return readMemory(f.data(), f.size(), readFlags);
  else
    return result;
}

Value XmlDocument::readStream(Stream& stream, uint32_t readFlags)
{
  ByteArray ba;

  if (!stream.readAll(ba))
    return Value().setError(ErrorDomain_Core, EOutOfMemory);
  else
    return readMemory((const void*)ba.cData(), ba.length(), readFlags);
}

Value XmlDocument::readMemory(const void* mem, ulong size, uint32_t readFlags)
{
  String buffer;

  /* ===== Setup xml encoding ===== */
  if (!XmlReader::checkForEncoding(mem, size, _textCodec))
    _textCodec = TextCodec::fromCode(TextCodec::UTF8);

  _textCodec.appendToUtf32(buffer, mem, size);
  return readUnicode(buffer.cData(), buffer.length(), readFlags);
}

Value XmlDocument::readUnicode(const Char* str, sysuint_t length, uint32_t readFlags)
{
  XmlReader xmlReader(*this, readFlags);
  return xmlReader.readUnicode(str, length);
}

// Writing process
static const uint8_t xml_escape_mask[] =
{
  // mask table for these characters: "%&<>
  0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x50,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static uint8_t* xml_escape_fn(uint8_t* buffer, sysuint_t length, Char uc)
{
  // destLength is at least 255, so we can skip checks,
  // but we must zero terminate output
  buffer[XmlEntity::encode((char*)buffer, uc)] = 0;
  return buffer;
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

  inline void writeChar(Char uc)
  { _textCodec.appendFromUtf32(_target, &uc, sizeof(Char)); }

  inline void writeSpaces(sysuint_t count)
  {
    sysuint_t i;
    for (i = 0; i != count; i++) writeChar(Char(' '));
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
      writeChar(Char('<'));
      writeStr(tag);
      writeAttributes(current);
      writeChar(Char('>'));

      // Write Text\n
      writeStr(current->text());
      writeChar(Char('\n'));

      // Write children
      writeNodesR(current->children(), depth+1);

      // Write </Tag>\n
      writeSpaces(depth);
      writeChar(Char('<'));
      writeChar(Char('/'));
      writeStr(tag);
      writeChar(Char('>'));
      writeChar(Char('\n'));
    }
    else
    {
      // Self closing tag
      if (current->text().isEmpty())
      {
        // Write <Tag [Attributes] />\n
        writeChar(Char('<'));
        writeStr(tag);
        writeAttributes(current);
        writeChar(Char(' '));
        writeChar(Char('/'));
        writeChar(Char('>'));
        writeChar(Char('\n'));
      }
      else
      {
        // Write <Tag [Attributes]>Text</Tag>\n
        writeChar(Char('<'));
        writeStr(tag);
        writeAttributes(current);
        writeChar(Char('>'));
        writeStr(current->text());
        writeChar(Char('<'));
        writeChar(Char('/'));
        writeStr(tag);
        writeChar(Char('>'));
        writeChar(Char('\n'));
      }
    }

    current = current->next();
    if (_target.length() > 512) flush();
  }
}

void XmlWriter::writeAttributes(const XmlNode* node)
{
  static const Char q = Char('\"');

  const XmlAttribute* current = node->attributes();

  if (!current) return;

  while (current)
  {
    // Write Attribute="Text"
    writeChar(Char(' '));
    writeStr(current->name());
    writeChar(Char('='));
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
