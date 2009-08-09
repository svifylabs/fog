// [Fog/Svg Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Dependencies]
#include <Fog/Core/StringUtil.h>
#include <Fog/Svg/SvgUtil.h>

namespace Fog {
namespace SvgUtil {

static uint8_t singleHexToColorValue(Char32 c0)
{
  uint8_t i;
  if (c0.isAsciiDigit())
    i = (uint8_t)(c0.ch() - '0');
  else
    i = (uint8_t)(c0.toAsciiLower().ch() - 'a' + 10);
  return (i << 4) | i;
}

static uint8_t doubleHexToColorValue(Char32 c0, Char32 c1)
{
  uint8_t i;
  uint8_t j;

  if (c0.isAsciiDigit())
    i = (uint8_t)(c0.ch() - '0');
  else
    i = (uint8_t)(c0.toAsciiLower().ch() - 'a' + 10);

  if (c1.isAsciiDigit())
    j = (uint8_t)(c1.ch() - '0');
  else
    j = (uint8_t)(c1.toAsciiLower().ch() - 'a' + 10);
  return (i << 4) | j;
}

Rgba parseColor(const String32& str)
{
  Rgba color(0xFF000000);

  const Char32* strCur = str.cData();
  const Char32* strEnd = strCur + str.length();

  while (strCur != strEnd && strCur->isSpace()) strCur++;
  if (strCur == strEnd) goto bail;

  if (*strCur == Char32('#'))
  {
    const Char32* begin = ++strCur;
    const Char32* end;
    if (strCur == strEnd) goto bail;

    while (strCur != strEnd && strCur->isXDigit()) strCur++;
    end = strCur;

    sysuint_t len = (sysuint_t)(end - begin);
    if (len == 3)
    {
      color.r = singleHexToColorValue(begin[0]);
      color.g = singleHexToColorValue(begin[1]);
      color.b = singleHexToColorValue(begin[2]);
    }
    else if (len == 6)
    {
      color.r = doubleHexToColorValue(begin[0], begin[1]);
      color.g = doubleHexToColorValue(begin[2], begin[3]);
      color.b = doubleHexToColorValue(begin[4], begin[5]);
    }
  }
  else if (*strCur == Char32('r'))
  {
    // SVG TODO:
  }

bail:
  return color;
}

SvgCoord parseCoord(const String32& str)
{
  double d = 0.0;
  uint32_t unit = SvgUnitNone;

  sysuint_t end;
  err_t err = str.atod(&d, NULL, &end);

  if (err == Error::Ok)
  {
    unit = SvgUnitPixel;

    if (end < str.length())
    {
      sysuint_t end2 = str.indexOf(Char32(' '), CaseSensitive, Range(end));
      TemporaryString32<16> ustr(Utf32(str.cData() + end, (end2 == InvalidIndex ? str.length() : end2) - end));

      if (ustr == Ascii8("cm"))
        unit = SvgUnitCm;
      else if (ustr == Ascii8("px"))
        unit = SvgUnitPixel;
      else if (ustr == Ascii8("%"))
        unit = SvgUnitPercent;
    }
  }

  return SvgCoord(d, unit);
}

Vector<SvgStyleItem> parseStyles(const String32& str)
{
  Vector<SvgStyleItem> items;

  const Char32* strCur = str.cData();
  const Char32* strEnd = strCur + str.length();

  for (;;)
  {
    if (strCur == strEnd) break;

    const Char32* styleNameBegin;
    const Char32* styleNameEnd;
    const Char32* styleValueBegin;
    const Char32* styleValueEnd;

    err_t err;

    // Skip spaces.
    while (strCur->isSpace())
    {
      if (++strCur == strEnd) goto bail;
    }

    // Parse style name.
    styleNameBegin = strCur;
    while (*strCur != Char32(':') && !strCur->isSpace())
    {
      if (++strCur == strEnd) goto bail;
    }
    styleNameEnd = strCur;

    if (strCur->isSpace())
    {
      while (*strCur != Char32(':'))
      {
        if (++strCur == strEnd) goto bail;
      }
    }

    // Skip ':'
    strCur++;

    // Skip spaces.
    while (strCur->isSpace())
    {
      if (++strCur == strEnd) goto bail;
    }

    // Parse style value.
    styleValueBegin = strCur;
    while (*strCur != Char32(';'))
    {
      if (++strCur == strEnd) break;
    }
    styleValueEnd = strCur;

    // Skip ';'
    if (strCur != strEnd) strCur++;

    // Done, create item
    SvgStyleItem item;
    String32 txt;

    err = txt.set(Utf32(styleNameBegin, sysuint_t(styleNameEnd - styleNameBegin)));
    if (err) continue;
    item.setName(txt);

    err = txt.set(Utf32(styleValueBegin, sysuint_t(styleValueEnd - styleValueBegin)));
    if (err) continue;
    item.setValue(txt);

    items.append(item);
  }

bail:
  return items;
}

String32 joinStyles(const Vector<SvgStyleItem>& items)
{
  String32 result;

  if (!items.isEmpty())
  {
    result.reserve(128);

    Vector<SvgStyleItem>::ConstIterator it(items);
    for (it.toStart(); it.isValid(); it.toNext())
    {
      const SvgStyleItem& item = it.value();
      result.append(item.name());
      result.append(Char32(':'));
      result.append(item.value());
      result.append(Char32(';'));
    }
  }

  return result;
}

Path parsePoints(const String32& str)
{
  Path path;

  const Char32* strCur = str.cData();
  const Char32* strEnd = strCur + str.length();

  return path;
}

Path parsePath(const String32& str)
{
  Path path;

  const Char32* strCur = str.cData();
  const Char32* strEnd = strCur + str.length();

  uint32_t command;
  uint32_t position;
  uint32_t count;
  double coords[10];

  for (;;)
  {
    // Finished?
    if (strCur == strEnd) goto bail;

    // Skip spaces.
    while (strCur->isSpace())
    {
      if (++strCur == strEnd) goto bail;
    }

    // Parse command.
    if (!strCur->isAsciiAlpha())
    {
      if (command == 0) goto bail;
    }
    else
    {
      command = strCur->ch();
      if (++strCur == strEnd) goto bail;
    }

    switch (command)
    {
      case 'A':
      case 'a': count = 7; break;
      case 'C':
      case 'c': count = 6; break;
      case 'L':
      case 'l': count = 2; break;
      case 'M':
      case 'm': count = 2; break;
      case 'Q':
      case 'q': count = 4; break;
      case 'S':
      case 's': count = 4; break;
      case 'T':
      case 't': count = 2; break;
      case 'Z':
      case 'z': count = 0; break;

      default:
        // Unknown command.
        goto bail;
    }

    // Parse coordinates.
    for (position = 0; position < count; position++)
    {
      // Skip spaces.
      while (strCur->isSpace())
      {
        if (++strCur == strEnd) goto bail;
      }

      // Parse number.
      sysuint_t numEnd;
      err_t err = StringUtil::atod(strCur, (sysuint_t)(strEnd - strCur), &coords[position], Char32('.'), &numEnd);
      if (err) goto bail;

      strCur += numEnd;
      if (strCur == strEnd) break;

      if (*strCur == Char32(','))
      {
        if (++strCur == strEnd) break;
      }
    }

    if (path.isEmpty()) path.reserve(64);

    switch (command)
    {
      case 'A':
        // SVG TODO:
        break;
      case 'a':
        // SVG TODO:
        break;

      case 'C':
        path.cubicTo(coords[0], coords[1], coords[2], coords[3], coords[4], coords[5]);
        break;
      case 'c':
        path.cubicToRel(coords[0], coords[1], coords[2], coords[3], coords[4], coords[5]);
        break;

      case 'L':
        path.lineTo(coords[0], coords[1]);
        break;
      case 'l':
        path.lineToRel(coords[0], coords[1]);
        break;

      case 'M':
        path.moveTo(coords[0], coords[1]);
        break;
      case 'm':
        path.moveToRel(coords[0], coords[1]);
        break;

      case 'Q':
        path.curveTo(coords[0], coords[1], coords[2], coords[3]);
        break;
      case 'q':
        path.curveToRel(coords[0], coords[1], coords[2], coords[3]);
        break;

      case 'S':
        path.cubicTo(coords[0], coords[1], coords[2], coords[3]);
        break;
      case 's':
        path.cubicToRel(coords[0], coords[1], coords[2], coords[3]);
        break;

      case 'T':
        path.curveTo(coords[0], coords[1]);
        break;
      case 't':
        path.curveToRel(coords[0], coords[1]);
        break;

      case 'Z': 
      case 'z':
        path.closePolygon();
        break;
    }
  }

bail:
  return path;
}

} // SvgUtil namespace
} // Fog namespace
