// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Graphics/Painter.h>
#include <Fog/Graphics/PainterUtil.h>
#include <Fog/UI/Button.h>

FOG_IMPLEMENT_OBJECT(Fog::Button)

namespace Fog {

// ============================================================================
// [Fog::Button]
// ============================================================================

Button::Button() : 
  ButtonBase()
{
}

Button::~Button()
{
}

void Button::onPaint(PaintEvent* e)
{
  Painter* p = e->painter();

  Rect bounds(0, 0, width(), height());
  PainterUtil::draw3dRect(p, bounds, 0xFFFFFFFF, 0xFF000000, isDown());

  bounds.shrink(1);

  Pattern pat;
  pat.setType(Pattern::LinearGradient);
  pat.setSpread(Pattern::PadSpread);

  if (!isDown())
  {
    pat.setStartPoint(Point(0, 0));
    pat.setEndPoint(Point(0, height()));
  }
  else
  {
    pat.setStartPoint(Point(0, height()));
    pat.setEndPoint(Point(0, 0));
  }

  pat.addGradientStop(GradientStop(0.0, 255, 255, 255));
  pat.addGradientStop(GradientStop(0.48, 140, 170, 255));
  pat.addGradientStop(GradientStop(0.52, 110, 140, 255));
  pat.addGradientStop(GradientStop(1.0, 70, 100, 255));
  p->setSource(pat);
  p->fillRect(bounds);

  if (isDown()) bounds.translate(1, 1);
  p->setSource(0xFF000000);
  p->drawText(bounds, _text, _font, TextAlignCenter);
}

} // Fog namespace
