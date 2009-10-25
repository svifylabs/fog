// [Fog/Svg Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Dependencies]
#include <Fog/Core/Char.h>
#include <Fog/Core/ManagedString.h>
#include <Fog/Core/String.h>
#include <Fog/Core/Strings.h>
#include <Fog/Svg/SvgBase.h>
#include <Fog/Svg/SvgUtil.h>

namespace Fog {

// ============================================================================
// [Fog::SvgStyleItem]
// ============================================================================

static const uint8_t svgStyleTypeToValueType[] =
{
  /* SvgStyleNone             -> */ SvgValueNone,
  /* SvgStyleClipPath         -> */ 0,
  /* SvgStyleClipRule         -> */ 0,
  /* SvgStyleEnableBackground -> */ 0,
  /* SvgStyleFill             -> */ SvgValueColorOrPattern,
  /* SvgStyleFillOpacity      -> */ SvgValueOpacity,
  /* SvgStyleFillRule         -> */ SvgValueEnum,
  /* SvgStyleFilter           -> */ 0,
  /* SvgStyleFontFamily       -> */ SvgValueString,
  /* SvgStyleFontSize         -> */ SvgValueCoord,
  /* SvgStyleLetterSpacing    -> */ SvgValueCoord,
  /* SvgStyleMask             -> */ 0,
  /* SvgStyleOpacity          -> */ SvgValueOpacity,
  /* SvgStyleStopColor        -> */ SvgValueColor,
  /* SvgStyleStopOpacity      -> */ SvgValueOpacity,
  /* SvgStyleStroke           -> */ SvgValueColorOrPattern,
  /* SvgStyleStrokeDashArray  -> */ 0,
  /* SvgStyleStrokeDashOffset -> */ SvgValueCoord,
  /* SvgStyleStrokeLineCap    -> */ SvgValueEnum,
  /* SvgStyleStrokeLineJoin   -> */ SvgValueEnum,
  /* SvgStyleStrokeMiterLimit -> */ SvgValueCoord,
  /* SvgStyleStrokeOpacity    -> */ SvgValueOpacity,
  /* SvgStyleStrokeWidth      -> */ SvgValueCoord
};

SvgStyleItem::SvgStyleItem() :
  _data0(FOG_UINT64_C(0)),
  _data1(FOG_UINT64_C(0))
{
}

SvgStyleItem::SvgStyleItem(const SvgStyleItem& other) : 
  _name(other._name),
  _value(other._value),
  _data0(other._data0),
  _data1(other._data1)
{
}

SvgStyleItem::~SvgStyleItem()
{
}

err_t SvgStyleItem::setName(const ManagedString& name)
{
  _data0 = FOG_UINT64_C(0);
  _data1 = FOG_UINT64_C(0);

  // Match the name in string array (we get also StyleType ID).
  for (uint32_t styleId = 1; styleId < SvgStyleCount; styleId++)
  {
    const ManagedString& s = fog_strings->getString(STR_SVG_STYLE_NAMES + styleId);
    if (s == name)
    {
      FOG_ASSERT(styleId < FOG_ARRAY_SIZE(svgStyleTypeToValueType));

      _styleType = styleId;
      _valueType = svgStyleTypeToValueType[styleId];
      _name = s;
      _value.free();
      return Error::Ok;
    }
  }

  // Not supported style. We will create it, but it will be ignored by 
  // svg rendering engine.
  _name = name;
  _value.free();
  return Error::Ok;
}

err_t SvgStyleItem::setName(const String& name)
{
  return setName(ManagedString(name));
}

struct SvgEnumToInt 
{
  char name[12];
  uint32_t value;
};

err_t SvgStyleItem::setValue(const String& value)
{
  err_t err = _value.set(value);
  if (err) return err;

  switch (_valueType)
  {
    case SvgValueNone:
    {
      break;
    }

    case SvgValueEnum:
    {
      const SvgEnumToInt* choices;

      switch (_styleType)
      {
        case SvgStyleFillRule:
        {
          static const SvgEnumToInt choicesFillRule[] =
          {
            { "nonzero", FillNonZero }, 
            { "evenodd", FillEvenOdd },
            { "", 0 }
          };
          choices = choicesFillRule;
          break;
        }

        case SvgStyleStrokeLineCap:
        {
          static const SvgEnumToInt choicesStrokeLineCap[] =
          {
            { "butt", }, 
            { "round", }, 
            { "square", },
            { "", 0 }
          };
          choices = choicesStrokeLineCap;
          break;
        }

        case SvgStyleStrokeLineJoin:
        {
          static const SvgEnumToInt choicesStrokeLineJoin[] =
          {
            { "miter", },
            { "round", },
            { "bevel", },
            { "", 0 },
          };
          choices = choicesStrokeLineJoin;
          break;
        }
        default:
          FOG_ASSERT_NOT_REACHED();
      }

      _isValid = false;

      while (choices->name[0] != '\0')
      {
        if (value.eq(Ascii8(choices->name)))
        {
          _valueU32 = choices->value;
          _isValid = true;
          break;
        }
        choices++;
      }

      break;
    }

    case SvgValueColor:
    {
      Rgba color;
      if (SvgUtil::parseColor(value, &color) == SvgStatusOk)
      {
        _kind = SvgPatternColor;
        _isValid = true;
        _valueU32 = color;
      }
      else
      {
        _kind = SvgPatternColor;
        _isValid = false;
        _valueU32 = 0x00000000;
      }
      break;
    }

    case SvgValueColorOrPattern:
    {
      Rgba color;
      switch (SvgUtil::parseColor(value, &color))
      {
        case SvgStatusOk:
        {
          _kind = SvgPatternColor;
          _isValid = true;
          _valueU32 = color;
          break;
        }
        case SvgStatusColorIsNone:
        {
          _kind = SvgPatternNone;
          _isValid = true;
          _valueU32 = 0x00000000;
          break;
        }
        case SvgStatusColorIsUri:
        {
          _kind = SvgPatternUri;
          _isValid = true;
          break;
        }
        default:
        {
          _isValid = false;
          _valueU32 = 0x00000000;
          break;
        }
      }
      break;
    }

    case SvgValueCoord:
    {
      SvgCoord coord = SvgUtil::parseCoord(value);
      _valueD = coord.value;
      _kind = coord.unit;
      _isValid = coord.unit != SvgUnitNotDefined;
      break;
    }

    case SvgValueOpacity:
    {
      double d = 0;
      if (value.atod(&d) == Error::Ok)
      {
        if (d < 0.0) d = 0.0;
        if (d > 1.0) d = 1.0;
        _isValid = true;
      }
      else
      {
        _isValid = false;
      }
      _valueD = d;
      break;
    }

    case SvgValueString:
    {
      _isValid = true;
      break;
    }

    default:
      FOG_ASSERT_NOT_REACHED();
  }

  return err;
}

SvgStyleItem& SvgStyleItem::operator=(const SvgStyleItem& other)
{
  _name = other._name;
  _value = other._value;
  _data0 = other._data0;
  _data1 = other._data1;

  return *this;
}

} // Fog namespace
