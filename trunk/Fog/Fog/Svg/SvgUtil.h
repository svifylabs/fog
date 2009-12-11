// [Fog/Svg Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_SVG_SVGUTIL_H
#define _FOG_SVG_SVGUTIL_H

// [Dependencies]
#include <Fog/Core/Char.h>
#include <Fog/Core/List.h>
#include <Fog/Core/String.h>
#include <Fog/Graphics/Argb.h>
#include <Fog/Graphics/Matrix.h>
#include <Fog/Graphics/Path.h>
#include <Fog/Svg/SvgBase.h>

namespace Fog {

//! @addtogroup Fog_Svg
//! @{

// ============================================================================
// [Svg::Util]
// ============================================================================

namespace SvgUtil {

//! @brief Parse SVG color and save it to @a dst. This method can also check
//! whether a given string is 'none' or URI to pattern definition.
//!
//! @return One of @c SVG_PATTERN_TYPE enumeration value is returned.
FOG_API int parseColor(const String& str, Argb* dst);

//! @brief Parse SVG matrix and save it to @a dst.
FOG_API err_t parseMatrix(const String& str, Matrix* dst);

//! @brief Parse SVG coorinate and return it.
FOG_API SvgCoord parseCoord(const String& str);

FOG_API List<SvgStyleItem> parseStyles(const String& str);
FOG_API String joinStyles(const List<SvgStyleItem>& items);

FOG_API Path parsePoints(const String& str);
FOG_API Path parsePath(const String& str);

} // SvgUtil namespace

//! @}

} // Fog namespace

// [Guard]
#endif // _FOG_SVG_SVGUTIL_H
