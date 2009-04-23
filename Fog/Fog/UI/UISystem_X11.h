// [Gui library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_UI_UISYSTEM_X11_H
#define _FOG_UI_UISYSTEM_X11_H

// [Dependencies]
#include <Fog/Core/Object.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/UI/Constants.h>
#include <Fog/UI/UISystem.h>

namespace Fog {

// ============================================================================
// [Fog::UISystemX11]
// ============================================================================

struct FOG_API UISystemX11 : public UISystem
{
  UISystemX11();
  virtual ~UISystemX11();

};

// ============================================================================
// [Fog::UINativeX11]
// ============================================================================

struct FOG_API UINativeX11 : public UINative
{
  FOG_DECLARE_OBJECT(UINativeX11, Object)

  typedef XID Handle;

  UINativeX11(Widget* widget);
  virtual ~UINativeX11();

  virtual err_t create(uint32_t flags);
  virtual err_t destroy();
  virtual err_t show();
  virtual err_t hide();
  virtual err_t move(const Point& pt);
  virtual err_t resize(const Size& size);
  virtual err_t reconfigure(const Rect& rect);

  virtual err_t enable();
  virtual err_t disable();

  virtual err_t takeFocus();

  virtual err_t setTitle(const String32& title);
  virtual err_t getTitle(String& title);

  virtual err_t setIcon(const Image& icon);
  virtual err_t getIcon(Image& icon);

  virtual err_t setSizeGranularity(const Point& pt);
  virtual err_t getSizeGranularity(Point& pt);

  virtual err_t worldToClient(Point* coords);
  virtual err_t clientToWorld(Point* coords);

  virtual void onX11Event(XEvent* xe);

protected:
  //! @brief Window title.
  String32 _title;

  //! @brief Window resize granularity.
  Point _sizeGranularity;

  //! @brief Window X Input Context (XIC).
  XIC _xic;
};

// ============================================================================
// [Fog::UIBackingStoreX11]
// ============================================================================

//! @brief Provides cacheable buffer for widgets.
struct FOG_API UIBackingStoreX11 : public UIBackingStore
{
  UIBackingStoreX11();
  virtual ~UIBackingStoreX11();

  virtual bool resize(uint width, uint height, bool cache);
  virtual void destroy();
  virtual void updateRects(const Fog::Box* rects, sysuint_t count);

  void blitRects(XID target, const Fog::Box* rects, sysuint_t count);

  FOG_INLINE Pixmap pixmap() const { return _pixmap; }
  FOG_INLINE XImage* ximage() const { return _ximage; }

  // X11 resources
  Pixmap _pixmap;
  XShmSegmentInfo _shmi;
  XImage* _ximage;

  // Type
  enum TypeEnum
  {
    TypeNone = 0,
    TypeXShmPixmap = 1,
    TypeXImage = 2,
    TypeXImageWithPixmap = TypeXShmPixmap | TypeXImage
  };
};

} // Fog namespace

// [Guard]
#endif // _FOG_UI_UISYSTEM_X11_H
