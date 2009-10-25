// [Fog/Svg Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_SVG_SVGBASE_H
#define _FOG_SVG_SVGBASE_H

// [Dependencies]
#include <Fog/Core/Char.h>
#include <Fog/Core/ManagedString.h>
#include <Fog/Core/String.h>
#include <Fog/Core/Vector.h>
#include <Fog/Graphics/Rgba.h>
#include <Fog/Graphics/Path.h>

namespace Fog {

//! @addtogroup Fog_Svg
//! @{

// ============================================================================
// [Fog::SvgStatus]
// ============================================================================

//! @brief Svg status that can
typedef uint SvgStatus;

enum SvgStatusCode
{
  SvgStatusOk = 0,

  SvgStatusColorIsNone = 1,
  SvgStatusColorIsInvalid = 2,
  SvgStatusColorIsUri = 3
};

// ============================================================================
// [Fog::SvgCoord]
// ============================================================================

struct FOG_HIDDEN SvgCoord
{
  FOG_INLINE SvgCoord() {}
  FOG_INLINE SvgCoord(double value, uint32_t unit) : value(value), unit(unit) {}

  double value;
  uint32_t unit;
};

// ============================================================================
// [Fog::SvgUnitType]
// ============================================================================

enum SvgUnitType
{
  SvgUnitNotDefined = 0,
  SvgUnitCm,
  SvgUnitEm,
  SvgUnitEx,
  SvgUnitIn,
  SvgUnitMm,
  SvgUnitPc,
  SvgUnitPt,
  SvgUnitPx,
  SvgUnitPercent,

  SvgUnitCount
};

// ============================================================================
// [Fog::SvgPaintType]
// ============================================================================

enum SvgPaintType
{
  SvgPaintFill,
  SvgPaintStroke
};

// ============================================================================
// [Fog::SvgStyleType]
// ============================================================================

enum SvgStyleType
{
  // When adding value here, it's important to add value to Fog/Core/Strings.h
  // and Fog/Core/Strings.cpp. There are static strings for style names.
  SvgStyleNone = 0,

  SvgStyleClipPath,
  SvgStyleClipRule,
  SvgStyleEnableBackground,
  SvgStyleFill,
  SvgStyleFillOpacity,
  SvgStyleFillRule,
  SvgStyleFilter,
  SvgStyleFontFamily,
  SvgStyleFontSize,
  SvgStyleLetterSpacing,
  SvgStyleMask,
  SvgStyleOpacity,
  SvgStyleStopColor,
  SvgStyleStopOpacity,
  SvgStyleStroke,
  SvgStyleStrokeDashArray,
  SvgStyleStrokeDashOffset,
  SvgStyleStrokeLineCap,
  SvgStyleStrokeLineJoin,
  SvgStyleStrokeMiterLimit,
  SvgStyleStrokeOpacity,
  SvgStyleStrokeWidth,

  SvgStyleCount
};

// ============================================================================
// [Fog::SvgValueType]
// ============================================================================

//! @brief Svg value types.
enum SvgValueType
{
  //! @brief Value is nothing special (not recognized style).
  SvgValueNone = 0,

  //! @brief Value is enumeration. To get it use @c getValue() or @c getValueAsUInt32().
  SvgValueEnum,
  //! @brief Value is color. To get it use @c getValue() or @c getValueAsUInt32().
  SvgValueColor,
  //! @brief Value is color or url (for gradients).
  SvgValueColorOrPattern,
  //! @brief Value is string. To get it use @c getValue() or @c getValueAsDouble()
  //! and @c unitType().
  SvgValueCoord,
  //! @brief Value is opacity. To get it use @c getValue() or @c getValueAsDouble().
  SvgValueOpacity,
  //! @brief Value is string. To get it use @c getValue().
  SvgValueString,

  SvgValueCount
};

// ============================================================================
// [Fog::SvgPatternType]
// ============================================================================

enum SvgPatternType
{
  SvgPatternNone = 0,
  SvgPatternColor = 1,
  SvgPatternUri = 2
};

// ============================================================================
// [Fog::SvgGradientUnits]
// ============================================================================

enum SvgGradientUnits
{
  SvgObjectBoundingBox,
  SvgUserSpaceOnUse
};

// ============================================================================
// [Fog::SvgStyleItem]
// ============================================================================

struct FOG_API SvgStyleItem
{
  SvgStyleItem();
  SvgStyleItem(const SvgStyleItem& other);
  ~SvgStyleItem();

  // [Name / Value]

  FOG_INLINE const String& getName() const { return _name.getString(); }
  err_t setName(const ManagedString& name);
  err_t setName(const String& name);

  FOG_INLINE const String& getValue() const { return _value; }
  err_t setValue(const String& value);

  // [Type Control]

  FOG_INLINE uint32_t getStyleType() const { return _styleType; }
  FOG_INLINE uint32_t getValueType() const { return _valueType; }

  // [Is Valid]

  FOG_INLINE uint32_t isValid() const { return _isValid; }

  // [SvgValueEnum]

  FOG_INLINE uint32_t getEnum() const { return _valueU32; }

  // [SvgValueColor]
  // [SvgValueColorOrPattern]

  FOG_INLINE uint32_t getPatternType() const { return _kind; }
  FOG_INLINE uint32_t getColor() const { return _valueU32; }
  
  // [SvgValueCoord]

  FOG_INLINE double getDouble() const { return _valueD; }
  FOG_INLINE uint32_t getUnit() const { return _kind; }

  FOG_INLINE SvgCoord getCoord() const { return SvgCoord(_valueD, _kind); }

  // [Operator Overload]

  SvgStyleItem& operator=(const SvgStyleItem& other);

protected:
  ManagedString _name;
  String _value;

  union {
    struct {
      //! @brief Style type ID, see @c SvgStyleType.
      uint8_t _styleType;

      //! @brief Value type ID, see @c SvgValueType.
      uint8_t _valueType;

      //! @brief Value that has different meaning for different value types.
      //!
      //! Meaning for known value types:
      //! - @c SvgValueCoord: Contains coordinate units.
      //! - @c SvgValueColorOrPattern. Contains pattern type (see @c SvgPatternType).
      uint8_t _kind;

      //! @brief Whether this value is valid.
      uint8_t _isValid;

      // ...
    };
    uint64_t _data0;
  };

  union {
    uint32_t _valueU32;
    double _valueD;

    uint64_t _data1;
  };
};

//! @}

} // Fog namespace

FOG_DECLARE_TYPEINFO(Fog::SvgCoord, Fog::PrimitiveType)
FOG_DECLARE_TYPEINFO(Fog::SvgStyleItem, Fog::MoveableType)

// [Guard]
#endif // _FOG_SVG_SVGBASE_H
