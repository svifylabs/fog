// [Fog/Gui Library - Public API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GUI_LAYOUT_H
#define _FOG_GUI_LAYOUT_H

// [Dependencies]
#include <Fog/Core/Event.h>
#include <Fog/Core/Object.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/Gui/Constants.h>
#include <Fog/Gui/LayoutItem.h>

//! @addtogroup Fog_Gui
//! @{

namespace Fog {

// ============================================================================
// [Fog::Layout]
// ============================================================================

//! @brief Base class for all layouts.
struct FOG_API Layout : public LayoutItem
{
  FOG_DECLARE_OBJECT(Layout, LayoutItem)

  // [Construction / Destruction]

  Layout();
  virtual ~Layout();

  // [Layout Hints]

  virtual Size getSizeHint() const;
  virtual void setSizeHint(const Size& sizeHint);

  virtual Size getMinimumSize() const;
  virtual void setMinimumSize(const Size& minSize);

  virtual Size getMaximumSize() const;
  virtual void setMaximumSize(const Size& maxSize);

  virtual uint32_t getLayoutPolicy() const;
  virtual void setLayoutPolicy(uint32_t policy);

  // [Layout Height For Width]

  virtual bool hasHeightForWidth() const;
  virtual int getHeightForWidth(int width) const;

  // [Layout State]

  virtual bool isLayoutDirty() const;
  virtual void invalidateLayout() const;

  // [Parents]

  LayoutItem* getParentItem() const;
  Widget* getParentWidget() const;

  // [Reparent]

  virtual void reparentChildren() = 0;

  // [Event Map]

  FOG_EVENT_BEGIN()
    FOG_EVENT_DEF(EV_LAYOUT_ITEM_ADD    , onLayout          , LayoutEvent    , OVERRIDE)
    FOG_EVENT_DEF(EV_LAYOUT_ITEM_REMOVE , onLayout          , LayoutEvent    , OVERRIDE)
  FOG_EVENT_END()

protected:
  LayoutItem* _parentItem;

  bool _isLayoutDirty;

  friend struct Widget;
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GUI_LAYOUT_H
