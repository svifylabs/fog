// [Fog/Svg Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_SVG_SVGBASE_H
#define _FOG_SVG_SVGBASE_H

// [Dependencies]
#include <Fog/Core/Char.h>
#include <Fog/Core/List.h>
#include <Fog/Core/ManagedString.h>
#include <Fog/Core/String.h>
#include <Fog/Graphics/Argb.h>
#include <Fog/Graphics/Path.h>

namespace Fog {

//! @addtogroup Fog_Svg
//! @{

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

  // [SVG_VALUE_ENUM]

  FOG_INLINE uint32_t getEnum() const { return _valueU32; }

  // [SVG_VALUE_COLOR]
  // [SVG_VALUE_COLOR_OR_PATTERN]

  FOG_INLINE uint32_t getPatternType() const { return _kind; }
  FOG_INLINE uint32_t getColor() const { return _valueU32; }

  // [SVG_VALUE_COORD]

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
      //! @brief Style type ID, see @c SVG_STYLE_TYPE.
      uint8_t _styleType;

      //! @brief Value type ID, see @c SVG_VALUE_TYPE.
      uint8_t _valueType;

      //! @brief Value that has different meaning for different value types.
      //!
      //! Meaning for known value types:
      //! - @c SVG_VALUE_COORD: Contains coordinate units.
      //! - @c SVG_VALUE_COLOR_OR_PATTERN. Contains pattern type (see @c SVG_PATTERN_TYPE).
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

FOG_DECLARE_TYPEINFO(Fog::SvgCoord, Fog::TYPE_INFO_PRIMITIVE)
FOG_DECLARE_TYPEINFO(Fog::SvgStyleItem, Fog::TYPE_INFO_MOVABLE)

// [Guard]
#endif // _FOG_SVG_SVGBASE_H
