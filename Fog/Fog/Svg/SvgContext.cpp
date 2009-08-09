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
  _strokeStyle.enabled = false;
  _strokeStyle.opacity = 1.0f;
  _strokeStyle.pattern.setColor(0xFF000000);

  _fillStyle.enabled = true;
  _fillStyle.opacity = 1.0f;
  _fillStyle.pattern.setColor(0xFF000000);
}

SvgContext::~SvgContext()
{
}

void SvgContext::drawEllipse(const PointF& cp, const PointF& r)
{
  if (_fillStyle.enabled)
  {
    _painter->setSource(_fillStyle.pattern);
    _painter->fillEllipse(cp, r);
  }
  if (_strokeStyle.enabled)
  {
    _painter->setSource(_strokeStyle.pattern);
    _painter->drawEllipse(cp, r);
  }
}

void SvgContext::drawLine(const PointF& p1, const PointF& p2)
{
  if (_strokeStyle.enabled)
  {
    _painter->setSource(_strokeStyle.pattern);
    _painter->drawLine(p1, p2);
  }
}

void SvgContext::drawRect(const RectF& rect)
{
  if (_fillStyle.enabled)
  {
    _painter->setSource(_fillStyle.pattern);
    _painter->fillRect(rect);
  }
  if (_strokeStyle.enabled)
  {
    _painter->setSource(_strokeStyle.pattern);
    _painter->drawRect(rect);
  }
}

void SvgContext::drawRound(const RectF& rect, const PointF& r)
{
  if (_fillStyle.enabled)
  {
    _painter->setSource(_fillStyle.pattern);
    _painter->fillRound(rect, r);
  }
  if (_strokeStyle.enabled)
  {
    _painter->setSource(_strokeStyle.pattern);
    _painter->drawRound(rect, r);
  }
}

void SvgContext::drawPath(const Path& path)
{
  if (_fillStyle.enabled)
  {
    _painter->setSource(_fillStyle.pattern);
    _painter->fillPath(path);
  }
  if (_strokeStyle.enabled)
  {
    _painter->setSource(_strokeStyle.pattern);
    _painter->drawPath(path);
  }
}

} // Fog namespace
