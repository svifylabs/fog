// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

#if defined(_MSVC)
#pragma once
#endif // _MSVC

// [Guard]
#ifndef _FOG_GRAPHICS_H
#define _FOG_GRAPHICS_H

// [There is only documentation]
#if defined(_DOXYGEN)
//! @defgroup Fog_Graphics Fog/Graphics library API.
//!
//! Set of Fog/Graphics library classes and their members.

//! @defgroup Fog_Graphics_ImageIO Fog/Graphics ImageIO module API.
//! @ingroup Fog_Graphics
//!
//! This module contains classes related to specific ImageIO plugins.

//! @defgroup Fog_Graphics_Raster Fog/Graphics Raster module API.
//! @ingroup Fog_Graphics
//!
//! Raster module contains very low level functions to perform pixel operations.
//! The raster module is very optimized and functions defined in it all used
//! across many Fog/Graphics library classes. Using these classes in application
//! is not recommended, but on the other side, they will probably stay forever.

//! @namespace Fog::RasterUtil
//! @brief Low level raster based functions namespace.
//!
//! RasterUtil namespace contains lowest level pixel manipulation library that is
//! used in Fog library.
//!
//! @section Pixel format conversion and definitions.
//!
//! Fog library contains few pixel formats that can be usually combined together.
//! When usung compositing function and destination not supports alpha channel,
//! the result is premultiplied and alpha is set to 0xFF. This is like blitting
//! the result into black destination using source over operator.

#endif // _DOXYGEN

// [Fog/Graphics Include Files]
#include <Fog/Build/Build.h>

#include <Fog/Graphics/Argb.h>
#include <Fog/Graphics/ArgbAnalyzer.h>
#include <Fog/Graphics/ArgbUtil.h>
#include <Fog/Graphics/ByteUtil.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/ColorLut.h>
#include <Fog/Graphics/ColorMatrix.h>
#include <Fog/Graphics/DitherMatrix.h>
#include <Fog/Graphics/Fixed.h>
#include <Fog/Graphics/Font.h>
#include <Fog/Graphics/FontEngine.h>
#include <Fog/Graphics/FontManager.h>
#include <Fog/Graphics/Glyph.h>
#include <Fog/Graphics/GlyphCache.h>
#include <Fog/Graphics/GlyphSet.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/ImageFilter.h>
#include <Fog/Graphics/ImageIO.h>
#include <Fog/Graphics/Matrix.h>
#include <Fog/Graphics/Painter.h>
#include <Fog/Graphics/PainterEngine.h>
#include <Fog/Graphics/PainterUtil.h>
#include <Fog/Graphics/Palette.h>
#include <Fog/Graphics/Path.h>
#include <Fog/Graphics/PathUtil.h>
#include <Fog/Graphics/Pattern.h>
#include <Fog/Graphics/RasterUtil.h>
#include <Fog/Graphics/Rasterizer.h>
#include <Fog/Graphics/Reduce.h>
#include <Fog/Graphics/Region.h>
#include <Fog/Graphics/RegionBuilder.h>
#include <Fog/Graphics/Scanline.h>
#include <Fog/Graphics/Stroker.h>
#include <Fog/Graphics/TextLayout.h>

// [Guard]
#endif // _FOG_GRAPHICS_H
