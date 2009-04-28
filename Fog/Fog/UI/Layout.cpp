// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/UI/Layout.h>
#include <Fog/UI/LayoutItem.h>

FOG_IMPLEMENT_OBJECT(Fog::Layout)

namespace Fog {

// ============================================================================
// [Fog::Layout]
// ============================================================================

Layout::Layout() : 
  _parentItem(NULL)
{
}

Layout::~Layout() 
{
}

LayoutItem* Layout::parentItem() const
{
  return _parentItem;
}

Widget* Layout::parentWidget() const
{
  LayoutItem* item = _parentItem;

  do {
    if (item->isWidget()) return reinterpret_cast<Widget*>(item);
    item = ((Layout *)item)->_parentItem;
  } while (item);

  return NULL;
}

void Layout::reparent()
{
}

} // Fog namespace
