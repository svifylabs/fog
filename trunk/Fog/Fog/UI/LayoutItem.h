// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_UI_LAYOUTITEM_H
#define _FOG_UI_LAYOUTITEM_H

// [Dependencies]
#include <Fog/Core/Constants.h>
#include <Fog/Core/Object.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/UI/Constants.h>
#include <Fog/UI/Event.h>

//! @addtogroup Fog_UI
//! @{

namespace Fog {

// ============================================================================
// [Fog::Forward Declarations]
// ============================================================================

struct Layout;
struct Widget;

// ============================================================================
// [Fog::LayoutItem]
// ============================================================================

#if 0
//! @brief Base for @c Widget and @c Layout classes.
struct FOG_API LayoutItem : public Object
{
  FOG_DECLARE_OBJECT(LayoutItem, Object)

  LayoutItem();
  virtual ~LayoutItem();

  // UI TODO: Make all of them pure virtual:
  virtual Size getSizeHint() const;
  virtual void setSizeHint(const Size& sizeHint) = 0;

  virtual Size getMinimumSize() const;
  virtual void setMinimumSize(const Size& minSize) = 0;

  virtual Size getMaximumSize() const;
  virtual void setMaximumSize(const Size& maxSize) = 0;

  virtual bool hasHeightForWidth() const;
  virtual int getHeightForWidth(int width) const = 0;

  virtual void invalidateLayout() const = 0;

  FOG_INLINE bool isLayoutDirty() const { return _isLayoutDirty; }

  // [Event Handlers]

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
#endif

//! @brief Base for @c Widget and @c Layout classes.
struct FOG_API LayoutItem : public Object
{
  // [Construction / Destruction]

  FOG_DECLARE_OBJECT(LayoutItem, Object)

  LayoutItem();
  virtual ~LayoutItem();

  // [Layout Hints]

  virtual Size getSizeHint() const = 0;
  virtual void setSizeHint(const Size& sizeHint) = 0;

  virtual Size getMinimumSize() const = 0;
  virtual void setMinimumSize(const Size& minSize) = 0;

  virtual Size getMaximumSize() const = 0;
  virtual void setMaximumSize(const Size& maxSize) = 0;

  // [Layout Policy]

  enum Policy
  {
    ExpandingWidth = 0x01,
    ExpandingHeight = 0x10
  };

  virtual uint32_t getLayoutPolicy() const = 0;
  virtual void setLayoutPolicy(uint32_t policy) = 0;

  // [Layout Height For Width]

  virtual bool hasHeightForWidth() const = 0;
  virtual int getHeightForWidth(int width) const = 0;

  // [Layout State]

  virtual bool isLayoutDirty() const = 0;
  virtual void invalidateLayout() const = 0;

  // [Event Handlers]

  virtual void onLayout(LayoutEvent* e);

  // [Event Map]

  fog_event_begin()
    fog_event(EvLayoutSet        , onLayout          , LayoutEvent    , Override)
    fog_event(EvLayoutRemove     , onLayout          , LayoutEvent    , Override)
  fog_event_end()
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_UI_LAYOUTITEM_H
