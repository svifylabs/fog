// [Fog/Svg Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Dependencies]
#include <Fog/Core/Math.h>
#include <Fog/Core/StringUtil.h>
#include <Fog/Graphics/Matrix.h>
#include <Fog/Svg/SvgUtil.h>

namespace Fog {
namespace SvgUtil {

// ============================================================================
// [Fog::SvgUtil]
// ============================================================================

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

FOG_API SvgStatus parseColor(const String32& str, Rgba* dst)
{
  SvgStatus status = SvgStatusColorIsInvalid;
  Rgba color(0xFF000000);

  const Char32* strCur = str.cData();
  const Char32* strEnd = strCur + str.getLength();

  while (strCur != strEnd && strCur->isSpace()) strCur++;
  if (strCur == strEnd) goto bail;

  // HTML color '#RGB' or '#RRGGBB'.
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

      status = SvgStatusOk;
    }
    else if (len == 6)
    {
      color.r = doubleHexToColorValue(begin[0], begin[1]);
      color.g = doubleHexToColorValue(begin[2], begin[3]);
      color.b = doubleHexToColorValue(begin[4], begin[5]);

      status = SvgStatusOk;
    }
  }
  // CSS color 'rgb(r, g, b)'.
  else if (*strCur == Char32('r'))
  {
    if (strCur + 2 < strEnd && StringUtil::eq(strCur, (const Char8*)"rgb", 3))
    {
      strCur += 3;

      // Skip spaces.
      for (;;) {
        if (strCur == strEnd) goto bail;
        else if (strCur->isSpace()) strCur++;
        else break;
      }

      // Parse '('.
      if (*strCur != Char32('(')) goto bail;
      if (++strCur == strEnd) goto bail;

      // Parse 0-255, 0-255, 0-255.
      uint8_t c[3];
      for (sysuint_t i = 0; i < 3; i++)
      {
        // Parse 8-bit number
        sysuint_t end;
        if (StringUtil::atou8(strCur, (sysuint_t)(strEnd - strCur), &c[i], 10, &end) != Error::Ok) goto bail;
        strCur += end;
        
        // Skip spaces and parse ','.
        if (i < 2)
        {
          for (;;) {
            if (strCur == strEnd) goto bail;

            if (strCur->isSpace())
            {
              strCur++;
            }
            else if (*strCur == Char32(','))
            {
              if (++strCur == strEnd) goto bail;
              break;
            }
            else
            {
              goto bail;
            }
          }
        }
      }

      // Skip spaces and parse ')'
      for (;;) {
        if (strCur == strEnd) goto bail;

        if (strCur->isSpace())
        {
          strCur++;
        }
        else if (*strCur == Char32(')'))
        {
          strCur++;
          break;
        }
        else
        {
          goto bail;
        }
      }

      color.r = c[0];
      color.g = c[1];
      color.b = c[2];

      status = SvgStatusOk;
    }
  }
  else if (*strCur == Char32('u'))
  {
    if (strCur + 2 < strEnd && StringUtil::eq(strCur, (const Char8*)"url", 3))
    {
      status = SvgStatusColorIsUri;
    }
  }
  else if (*strCur == Char32('n'))
  {
    if (strCur + 3 < strEnd && StringUtil::eq(strCur, (const Char8*)"none", 4))
    {
      status = SvgStatusColorIsNone;
    }
  }

bail:
  *dst = color;
  return status;
}

static const char svgUnitNames[] = "cmemexinmmpcptpx";

SvgCoord parseCoord(const String32& str)
{
  double d = 0.0;
  uint32_t unit = SvgUnitNotDefined;

  sysuint_t end;
  err_t err = str.atod(&d, NULL, &end);

  if (err == Error::Ok)
  {
    unit = SvgUnitPx;

    if (end < str.getLength())
    {
      sysuint_t end2 = str.indexOf(Char32(' '), CaseSensitive, Range(end));
      Utf32 spec(str.cData() + end, (end2 == InvalidIndex ? str.getLength() : end2) - end);

      if (spec.getLength() == 1)
      {
        if (spec.getStr()[0] == Char32('%'))
        {
          unit = SvgUnitPercent;
        }
      }
      else if (spec.getLength() == 2)
      {
        const char* units = svgUnitNames;
        for (sysuint_t u = 1; u < SvgUnitPercent; u++, units += 2)
        {
          if (spec.getStr()[0] == Char8(units[0]) && spec.getStr()[1] == Char8(units[1]))
          {
            unit = u;
            break;
          }
        }
      }
    }
  }

  if (unit == SvgUnitPercent) d *= 0.01;
  return SvgCoord(d, unit);
}

SvgStatus parseMatrix(const String32& str, Matrix* dst)
{
  SvgStatus status = SvgStatusOk;

  const Char32* strCur = str.cData();
  const Char32* strEnd = strCur + str.getLength();
  const Char32* functionName;
  sysuint_t functionLen;

  double d[6];
  sysuint_t d_count;

  Matrix w;

start:
  // Skip spaces.
  for (;;) {
    if (strCur == strEnd) goto bail;
    else if (strCur->isSpace()) strCur++;
    else break;
  }

  // Parse function name.
  functionName = strCur;
  for (;;) {
    if (strCur == strEnd) goto bail;
    else if (strCur->isAsciiAlpha()) strCur++;
    else break;
  }
  functionLen = (sysuint_t)(strCur - functionName);

  // Parse '('.
  if (strCur[0] != Char32('(')) goto bail;
  strCur++;

  // Parse arguments.
  d_count = 0;
  for (;;) {
    if (strCur == strEnd) goto bail;

    // Parse number.
    sysuint_t end;
    if (StringUtil::atod(strCur, (sysuint_t)(strEnd - strCur), &d[d_count++], Char32('.'), &end) != Error::Ok) goto bail;    

    strCur += end;

    // Skip ',' and move to position of the next digit.
    bool commaParsed = false;
    for (;;) {
      if (strCur == strEnd) goto bail;

      if (strCur->isSpace())
      {
        strCur++;
      }
      else if (strCur[0] == Char32(','))
      {
        strCur++;
        if (commaParsed) goto bail;
        commaParsed = true;
      }
      else if (strCur[0] == Char32(')'))
      {
        strCur++;
        if (commaParsed) goto bail;
        else goto done;
      }
      else if (strCur[0].isAsciiDigit())
      {
        break;
      }
      else goto bail;
    }

    if (d_count == 6) goto bail;
  }
done:

  // matrix() function.
  if (functionLen == 6 && StringUtil::eq(functionName, (const Char8*)"matrix", 6))
  {
    if (d_count != 6) goto bail;
    w.multiply(*reinterpret_cast<Matrix *>(d));
  }
  // translate() function.
  else if (functionLen == 9 && StringUtil::eq(functionName, (const Char8*)"translate", 9))
  {
    if (d_count != 1 && d_count != 2) goto bail;

    // If ty is not provided, it's assumed to be zero.
    if (d_count == 1) d[1] = 0.0;
    w.translate(d[0], d[1]);
  }
  // scale() function.
  else if (functionLen == 5 && StringUtil::eq(functionName, (const Char8*)"scale", 5))
  {
    if (d_count != 1 && d_count != 2) goto bail;

    // If sy is not provided, it's assumed to be equal to sx.
    if (d_count == 1) d[1] = d[0];
    w.scale(d[0], d[1]);
  }
  // rotate() function.
  else if (functionLen == 6 && StringUtil::eq(functionName, (const Char8*)"rotate", 6))
  {
    if (d_count != 1 && d_count != 3) goto bail;

    if (d_count == 3) w.translate(d[1], d[2]);
    w.rotate(Math::deg2rad(d[0]));
    if (d_count == 3) w.translate(-d[1], -d[2]);
  }
  // skewX() function.
  else if (functionLen == 5 && StringUtil::eq(functionName, (const Char8*)"skewX", 5))
  {
    if (d_count != 1) goto bail;

    w.skew(Math::deg2rad(d[0]), 0.0);
  }
  // skewY() function.
  else if (functionLen == 5 && StringUtil::eq(functionName, (const Char8*)"skewY", 5))
  {
    if (d_count != 1) goto bail;

    w.skew(0.0, Math::deg2rad(d[0]));
  }
  else
  {
    goto bail;
  }

  // Skip spaces.
  for (;;) {
    if (strCur == strEnd) break;
    else if (strCur->isSpace()) strCur++;
    else goto start;
  }

bail:
  *dst = w;
  return status;
}

Vector<SvgStyleItem> parseStyles(const String32& str)
{
  Vector<SvgStyleItem> items;

  const Char32* strCur = str.cData();
  const Char32* strEnd = strCur + str.getLength();

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
    if (++strCur == strEnd) goto bail;


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

    // Remove trailing spaces.
    //
    // We can't cause buffer underflow, because we already parsed ':' that's
    // not space.
    while (styleValueEnd[-1].isSpace()) styleValueEnd--;

    // Skip ';'
    if (strCur != strEnd) strCur++;

    // Done, create item
    SvgStyleItem item;

    {
      ManagedString32 styleName;

      err = styleName.set(Utf32(styleNameBegin, sysuint_t(styleNameEnd - styleNameBegin)));
      if (err) continue;
      item.setName(styleName);
    }

    {
      String32 styleValue;
      err = styleValue.set(Utf32(styleValueBegin, sysuint_t(styleValueEnd - styleValueBegin)));
      if (err) continue;
      item.setValue(styleValue);
    }

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
      result.append(item.getName());
      result.append(Char32(':'));
      result.append(item.getValue());
      result.append(Char32(';'));
    }
  }

  return result;
}

Path parsePoints(const String32& str)
{
  Path path;

  const Char32* strCur = str.cData();
  const Char32* strEnd = strCur + str.getLength();

  return path;
}

Path parsePath(const String32& str)
{
  Path path;

  const Char32* strCur = str.cData();
  const Char32* strEnd = strCur + str.getLength();

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
