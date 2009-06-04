// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_UI_LAYOUT_ITEM_H
#define _FOG_UI_LAYOUT_ITEM_H

// [Dependencies]
#include <Fog/Core/Object.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/UI/Event.h>
#include <Fog/UI/Constants.h>

//! @addtogroup Fog_UI
//! @{

namespace Fog {

// ============================================================================
// [Fog::LayoutItem]
// ============================================================================

//! @brief Base for @c Fog::Widget or @c Fog::Layout classes.
//!
//! Layout item can be imagined as interface. It contains pure virtual methods
//! that's overriden by widgets or layouts.
struct FOG_API LayoutItem : public Object
{
  FOG_DECLARE_OBJECT(LayoutItem, Object)

  LayoutItem();
  virtual ~LayoutItem();

  virtual Size sizeHint() const;
  virtual Size minimumSize() const;
  virtual Size maximumSize() const;
  virtual bool hasHeightForWidth() const;

  virtual void invalidateLayout() const = 0;
  virtual void setSizeHint(const Size& sizeHint) = 0;
  virtual void setMinimumSize(const Size& minSize) = 0;
  virtual void setMaximumSize(const Size& maxSize) = 0;
  virtual int heightForWidth(int width) const = 0;

  FOG_INLINE bool isLayoutDirty() const { return _isLayoutDirty; }

  // [Events]

  virtual void onLayout(LayoutEvent* e);

  // [Event Map]

  fog_event_begin()
    fog_event(EvLayoutSet        , onLayout          , LayoutEvent    , Override)
    fog_event(EvLayoutRemove     , onLayout          , LayoutEvent    , Override)
  fog_event_end()

protected:
  Size _sizeHint;
  Size _minimumSize;
  Size _maximumSize;
  bool _hasHeightForWidth;
  bool _isLayoutDirty;
  uint8_t _widthPolicy;
  uint8_t _heightPolicy;
};

} // Fog namespace

//! @}

// [Guard]
#endif // GUI_LAYOUT_ITEM_H
