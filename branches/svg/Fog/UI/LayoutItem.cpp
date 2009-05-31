// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/UI/LayoutItem.h>

FOG_IMPLEMENT_OBJECT(Fog::LayoutItem)

namespace Fog {

LayoutItem::LayoutItem() :
  _sizeHint(-1, -1),
  _minimumSize(-1, -1),
  _maximumSize(-1, -1),
  _hasHeightForWidth(false),
  _isLayoutDirty(true)
{
}

LayoutItem::~LayoutItem() 
{
}

Size LayoutItem::sizeHint() const
{
  return _sizeHint;
}

Size LayoutItem::minimumSize() const
{
  return _minimumSize;
}

Size LayoutItem::maximumSize() const
{
  return _maximumSize;
}

bool LayoutItem::hasHeightForWidth() const
{
  return _hasHeightForWidth;
}

void LayoutItem::onLayout(LayoutEvent* e)
{
}

} // Fog namespace
