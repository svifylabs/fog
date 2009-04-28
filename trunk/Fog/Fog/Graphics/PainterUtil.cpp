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
  p->setSource(checked ? color2 : color1);
  p->drawLine(Point(r.x1()  , r.y1()  ), Point(r.x2()-1, r.y1()  ));
  p->drawLine(Point(r.x1()  , r.y1()+1), Point(r.x1()  , r.y2()-1));

  p->setSource(checked ? color1 : color2);
  p->drawLine(Point(r.x2()-1, r.y1()+1), Point(r.x2()-1, r.y2()-1));
  p->drawLine(Point(r.x1()  , r.y2()-1), Point(r.x2()-1, r.y2()-1));
}

} // Fog namespace
