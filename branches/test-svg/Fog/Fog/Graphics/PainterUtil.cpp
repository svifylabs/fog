// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Graphics/PainterUtil.h>

namespace Fog {

void PainterUtil::draw3dRect(Painter* p, const Rect& r, Rgba color1, Rgba color2, bool checked)
{
  if (!r.isValid()) return;

  double x1 = (double)r.x1()+.5;
  double y1 = (double)r.y1()+.5;
  double x2 = (double)r.x2()-.5;
  double y2 = (double)r.y2()-.5;

  p->save();

  p->setLineWidth(1.0);
  p->setLineCap(LineCapSquare);

  p->setSource(checked ? color2 : color1);
  p->drawLine(PointF((double)x1   , (double)y1   ), PointF((double)x2   , (double)y1   ));
  p->drawLine(PointF((double)x1   , (double)y1+1.), PointF((double)x1   , (double)y2-1.));

  p->setSource(checked ? color1 : color2);
  p->drawLine(PointF((double)x2   , (double)y1+1.), PointF((double)x2   , (double)y2-1.));
  p->drawLine(PointF((double)x1   , (double)y2   ), PointF((double)x2   , (double)y2   ));

  p->restore();
}

} // Fog namespace
