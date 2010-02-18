// [Fog/Gui Library - Public API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Gui/Layout.h>
#include <Fog/Gui/LayoutItem.h>

FOG_IMPLEMENT_OBJECT(Fog::Layout)

namespace Fog {

// ============================================================================
// [Fog::Layout]
// ============================================================================

Layout::Layout() : 
  _parentItem(NULL),
  _isLayoutDirty(true)
{
  _flags |= OBJ_IS_LAYOUT;
}

Layout::~Layout() 
{
}

Size Layout::getSizeHint() const
{
  return Size(-1, -1);
}

void Layout::setSizeHint(const Size& sizeHint)
{
}

Size Layout::getMinimumSize() const
{
  return Size(-1, -1);
}

void Layout::setMinimumSize(const Size& minSize)
{
}

Size Layout::getMaximumSize() const
{
  return Size(-1, -1);
}

void Layout::setMaximumSize(const Size& maxSize)
{
}

uint32_t Layout::getLayoutPolicy() const
{
  return 0;
}

void Layout::setLayoutPolicy(uint32_t policy)
{
}

bool Layout::hasHeightForWidth() const
{
  return false;
}

int Layout::getHeightForWidth(int width) const
{
  return -1;
}

bool Layout::isLayoutDirty() const
{
  return false;
}

void Layout::invalidateLayout() const
{
}

LayoutItem* Layout::getParentItem() const
{
  return _parentItem;
}

Widget* Layout::getParentWidget() const
{
  LayoutItem* item = _parentItem;

  do {
    if (item->isWidget()) return reinterpret_cast<Widget*>(item);
    item = ((Layout *)item)->_parentItem;
  } while (item);

  return NULL;
}

} // Fog namespace
