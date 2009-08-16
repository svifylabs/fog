// [Fog/Svg Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Dependencies]
#include <Fog/Svg/SvgContext.h>
#include <Fog/Svg/SvgDom.h>

namespace Fog {

// ============================================================================
// [Fog::SvgContext]
// ============================================================================

SvgContext::SvgContext(Painter* painter) :
  _painter(painter)
{
  _lineStyle.type = SvgPatternNone;
  _fillStyle.type = SvgPatternColor;
  _fillMode = FillNonZero;

  setDpi(90.0);
}

SvgContext::~SvgContext()
{
}

void SvgContext::setDpi(double dpi)
{
  _dpi = dpi;

  _translateCoordData[SvgUnitNotDefined] = 1.0;
  _translateCoordData[SvgUnitPercent] = 1.0;

  // SVG TODO: Em and Ex units.
  _translateCoordData[SvgUnitCm] = dpi * 0.3937007777777778;
  _translateCoordData[SvgUnitEm] = 1.0;
  _translateCoordData[SvgUnitEx] = 1.0;
  _translateCoordData[SvgUnitIn] = dpi;
  _translateCoordData[SvgUnitMm] = dpi * 0.0393700777777778;
  _translateCoordData[SvgUnitPc] = dpi * 0.1666666666666667;
  _translateCoordData[SvgUnitPt] = dpi * 0.0138888888888889;
  _translateCoordData[SvgUnitPx] = 1.0;
}

void SvgContext::drawEllipse(const PointF& cp, const PointF& r)
{
  if (_fillStyle.type != SvgPatternNone)
  {
    setupFillStyle();
    _painter->fillEllipse(cp, r);
  }

  if (_lineStyle.type != SvgPatternNone)
  {
    setupStrokeStyle();
    _painter->drawEllipse(cp, r);
  }
}

void SvgContext::drawLine(const PointF& p1, const PointF& p2)
{
  if (_lineStyle.type != SvgPatternNone)
  {
    setupStrokeStyle();
    _painter->drawLine(p1, p2);
  }
}

void SvgContext::drawRect(const RectF& rect)
{
  if (_fillStyle.type != SvgPatternNone)
  {
    setupFillStyle();
    _painter->fillRect(rect);
  }

  if (_lineStyle.type != SvgPatternNone)
  {
    setupStrokeStyle();
    _painter->drawRect(rect);
  }
}

void SvgContext::drawRound(const RectF& rect, const PointF& r)
{
  if (_fillStyle.type != SvgPatternNone)
  {
    setupFillStyle();
    _painter->fillRound(rect, r);
  }

  if (_lineStyle.type != SvgPatternNone)
  {
    setupStrokeStyle();
    _painter->drawRound(rect, r);
  }
}

void SvgContext::drawPath(const Path& path)
{
  if (_fillStyle.type != SvgPatternNone)
  {
    setupFillStyle();
    _painter->fillPath(path);
  }

  if (_lineStyle.type != SvgPatternNone)
  {
    setupStrokeStyle();
    _painter->drawPath(path);
  }
}

} // Fog namespace
