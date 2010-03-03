// [Fog/Gui Library - Public API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Gui/Layout/LayoutItem.h>

FOG_IMPLEMENT_OBJECT(Fog::LayoutItem)

namespace Fog {

// ============================================================================
// [Fog::LayoutItem]
// ============================================================================

LayoutItem::LayoutItem()
{
}

LayoutItem::~LayoutItem() 
{
}

Size LayoutItem::getSizeHint() const
{
  return getLayoutHint().getSizeHint();
}

Size LayoutItem::getMinimumSize() const
{
  return getLayoutHint().getMinimumSize();
}

Size LayoutItem::getMaximumSize() const
{
  return getLayoutHint().getMaximumSize();
}

void LayoutItem::setSizeHint(const Size& sizeHint)
{
  LayoutHint hint = getLayoutHint();
  hint.setSizeHint(sizeHint);

  setLayoutHint(hint);
}

void LayoutItem::setMinimumSize(const Size& minimumSize)
{
  LayoutHint hint = getLayoutHint();
  hint.setMinimumSize(minimumSize);

  setLayoutHint(hint);
}

void LayoutItem::setMaximumSize(const Size& maximumSize)
{
  LayoutHint hint = getLayoutHint();
  hint.setMaximumSize(maximumSize);

  setLayoutHint(hint);
}

void LayoutItem::onLayout(LayoutEvent* e)
{
}

} // Fog namespace
