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
#include <Fog/UI/Constants.h>

namespace Fog {

// ============================================================================
// [Fog::LayoutItem]
// ============================================================================

//! @brief Base for @c Fog::Widget or @c Wde_Layout classes.
//!
//! Layout item can be imagined as interface. It contains pure virtual methods
//! that's overriden by widgets or layouts.
struct FOG_API LayoutItem : public Object
{
  FOG_DECLARE_OBJECT(LayoutItem, Object)

  LayoutItem();
  virtual ~LayoutItem();

  FOG_INLINE Size sizeHint() const { return _sizeHint; }
  FOG_INLINE Size minimumSize() const { return _minimumSize; }
  FOG_INLINE Size maximumSize() const { return _maximumSize; }
  FOG_INLINE bool hasHeightForWidth() const { return _hasHeightForWidth; }
  FOG_INLINE bool dirtyLayout() const { return _dirtyLayout; }

  virtual void invalidateLayout() const = 0;
  virtual void setSizeHint(const Size& sizeHint) = 0;
  virtual void setMinimumSize(const Size& sizeHint) = 0;
  virtual void setMaximumSize(const Size& sizeHint) = 0;
  virtual int heightForWidth(int width) const = 0;

protected:
  Size _sizeHint;
  Size _minimumSize;
  Size _maximumSize;
  bool _hasHeightForWidth;
  bool _dirtyLayout;
  uint8_t _widthPolicy;
  uint8_t _heightPolicy;
};

} // Fog namespace

// [Guard]
#endif // GUI_LAYOUT_ITEM_H
