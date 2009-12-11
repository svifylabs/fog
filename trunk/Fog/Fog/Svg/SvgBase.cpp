// [Fog/Svg Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Dependencies]
#include <Fog/Core/Char.h>
#include <Fog/Core/ManagedString.h>
#include <Fog/Core/String.h>
#include <Fog/Core/Strings.h>
#include <Fog/Svg/Constants.h>
#include <Fog/Svg/SvgBase.h>
#include <Fog/Svg/SvgUtil.h>

namespace Fog {

// ============================================================================
// [Fog::SvgStyleItem]
// ============================================================================

static const uint8_t svgStyleTypeToValueType[] =
{
  /* SVG_STYLE_NONE               -> */ SVG_VALUE_NONE,
  /* SVG_STYLE_CLIP_PATH          -> */ 0,
  /* SVG_STYLE_CLIP_RULE          -> */ 0,
  /* SVG_STYLE_ENABLE_BACKGROUND  -> */ 0,
  /* SVG_STYLE_FILL               -> */ SVG_VALUE_COLOR_OR_PATTERN,
  /* SVG_STYLE_FILL_OPACITY       -> */ SVG_VALUE_OPACITY,
  /* SVG_STYLE_FILL_RULE          -> */ SVG_VALUE_ENUM,
  /* SVG_STYLE_FILTER             -> */ 0,
  /* SVG_STYLE_FONT_FAMILY        -> */ SVG_VALUE_STRING,
  /* SVG_STYLE_FONT_SIZE          -> */ SVG_VALUE_COORD,
  /* SVG_STYLE_LETTER_SPACING     -> */ SVG_VALUE_COORD,
  /* SVG_STYLE_MASK               -> */ 0,
  /* SVG_STYLE_OPACITY            -> */ SVG_VALUE_OPACITY,
  /* SVG_STYLE_STOP_COLOR         -> */ SVG_VALUE_COLOR,
  /* SVG_STYLE_STOP_OPACITY       -> */ SVG_VALUE_OPACITY,
  /* SVG_STYLE_STROKE             -> */ SVG_VALUE_COLOR_OR_PATTERN,
  /* SVG_STYLE_STROKE_DASH_ARRAY  -> */ 0,
  /* SVG_STYLE_STROKE_DASH_OFFSET -> */ SVG_VALUE_COORD,
  /* SVG_STYLE_STROKE_LINE_CAP    -> */ SVG_VALUE_ENUM,
  /* SVG_STYLE_STROKE_LINE_JOIN   -> */ SVG_VALUE_ENUM,
  /* SVG_STYLE_STROKE_MITER_LIMIT -> */ SVG_VALUE_COORD,
  /* SVG_STYLE_STROKE_OPACITY     -> */ SVG_VALUE_OPACITY,
  /* SVG_STYLE_STROKE_WIDTH       -> */ SVG_VALUE_COORD
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
  for (uint32_t styleId = 1; styleId < SVG_STYLE_INVALID; styleId++)
  {
    const ManagedString& s = fog_strings->getString(STR_SVG_STYLE_NAMES + styleId);
    if (s == name)
    {
      FOG_ASSERT(styleId < FOG_ARRAY_SIZE(svgStyleTypeToValueType));

      _styleType = styleId;
      _valueType = svgStyleTypeToValueType[styleId];
      _name = s;
      _value.free();
      return ERR_OK;
    }
  }

  // Not supported style. We will create it, but it will be ignored by 
  // svg rendering engine.
  _name = name;
  _value.free();
  return ERR_OK;
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
    case SVG_VALUE_NONE:
    {
      break;
    }

    case SVG_VALUE_ENUM:
    {
      const SvgEnumToInt* choices;

      switch (_styleType)
      {
        case SVG_STYLE_FILL_RULE:
        {
          static const SvgEnumToInt choicesFillRule[] =
          {
            { "nonzero", FILL_NON_ZERO },
            { "evenodd", FILL_EVEN_ODD },
            { "", 0 }
          };
          choices = choicesFillRule;
          break;
        }

        case SVG_STYLE_STROKE_LINE_CAP:
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

        case SVG_STYLE_STROKE_LINE_JOIN:
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

    case SVG_VALUE_COLOR:
    {
      Argb color;
      if (SvgUtil::parseColor(value, &color) == SVG_PATTERN_COLOR)
      {
        _kind = SVG_PATTERN_COLOR;
        _isValid = true;
        _valueU32 = color;
      }
      else
      {
        _kind = SVG_PATTERN_COLOR;
        _isValid = false;
        _valueU32 = 0x00000000;
      }
      break;
    }

    case SVG_VALUE_COLOR_OR_PATTERN:
    {
      Argb color;
      switch (SvgUtil::parseColor(value, &color))
      {
        case SVG_PATTERN_NONE:
        {
          _kind = SVG_PATTERN_NONE;
          _isValid = true;
          _valueU32 = 0x00000000;
          break;
        }
        case SVG_PATTERN_COLOR:
        {
          _kind = SVG_PATTERN_COLOR;
          _isValid = true;
          _valueU32 = color;
          break;
        }
        case SVG_PATTERN_URI:
        {
          _kind = SVG_PATTERN_URI;
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

    case SVG_VALUE_COORD:
    {
      SvgCoord coord = SvgUtil::parseCoord(value);
      _valueD = coord.value;
      _kind = coord.unit;
      _isValid = coord.unit != SVG_UNIT_NONE;
      break;
    }

    case SVG_VALUE_OPACITY:
    {
      double d = 0;
      if (value.atod(&d) == ERR_OK)
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

    case SVG_VALUE_STRING:
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
