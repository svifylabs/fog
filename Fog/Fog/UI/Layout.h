// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_UI_LAYOUT_H
#define _FOG_UI_LAYOUT_H

// [Dependencies]
#include <Fog/Core/Event.h>
#include <Fog/Core/Object.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/UI/Constants.h>
#include <Fog/UI/LayoutItem.h>

//! @addtogroup Fog_UI
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

  // [Layout Policy]

  enum Policy
  {
    ExpandingWidth = 0x01,
    ExpandingHeight = 0x10
  };

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

  fog_event_begin()
    fog_event(EvLayoutItemAdd    , onLayout          , LayoutEvent    , Override)
    fog_event(EvLayoutItemRemove , onLayout          , LayoutEvent    , Override)
  fog_event_end()

protected:
  LayoutItem* _parentItem;

  bool _isLayoutDirty;

  friend struct Widget;
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_UI_LAYOUT_H
