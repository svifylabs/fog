// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Memory.h>
#include <Fog/Graphics/PainterUtil.h>

namespace Fog {

void PainterUtil::draw3dRect(Painter* p, const Rect& r, Rgba color1, Rgba color2, bool checked)
{
  if (!r.isValid()) return;

  int x1 = r.x1();
  int y1 = r.y1();
  int x2 = r.x2()-1;
  int y2 = r.y2()-1;

  p->setSource(checked ? color2 : color1);
  p->drawLine(Point(x1  , y1  ), Point(x2  , y1  ));
  p->drawLine(Point(x1  , y1+1), Point(x1  , y2-1));

  p->setSource(checked ? color1 : color2);
  p->drawLine(Point(x2  , y1+1), Point(x2  , y2-1));
  p->drawLine(Point(x1  , y2  ), Point(x2  , y2  ));
}

} // Fog namespace
