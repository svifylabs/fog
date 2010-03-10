// [Fog-Gui Library - Public API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Graphics/Painter.h>
#include <Fog/Graphics/PaintUtil.h>
#include <Fog/Gui/Button.h>

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
  Painter* p = e->getPainter();
  Size s = getClientSize();

  Rect bounds(0, 0, s.w, s.h);
  PaintUtil::draw3dRect(p, bounds, 0xFFFFFFFF, 0xFF000000, isDown());

  bounds.shrink(1);

  Pattern pat;
  pat.setType(PATTERN_LINEAR_GRADIENT);
  pat.setSpread(SPREAD_PAD);

  if (!isDown())
  {
    pat.setStartPoint(Point(0, 0));
    pat.setEndPoint(Point(0, s.h));
  }
  else
  {
    pat.setStartPoint(Point(0, s.h));
    pat.setEndPoint(Point(0, 0));
  }

  pat.addStop(ArgbStop(0.00, 0xFFFFFFFF));
  pat.addStop(ArgbStop(0.48, 0xFF8CAAFF));
  pat.addStop(ArgbStop(0.52, 0xFF6E8CFF));
  pat.addStop(ArgbStop(1.00, 0xFF4664FF));
  p->setSource(pat);
  p->fillRect(bounds);

  if (isDown()) bounds.translate(1, 1);
  p->setSource(0xFF000000);
  p->drawText(bounds, _text, _font, TEXT_ALIGN_CENTER);
}

} // Fog namespace
