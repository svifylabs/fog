// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_UI_UISYSTEM_WIN_H
#define _FOG_UI_UISYSTEM_WIN_H

// [Dependencies]
#include <Fog/Core/Object.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/UI/Constants.h>
#include <Fog/UI/UISystem.h>
#include <Fog/UI/UISystem_Def.h>

//! @addtogroup Fog_UI_Win
//! @{

namespace Fog {

// ============================================================================
// [Fog::UISystemWin]
// ============================================================================

struct FOG_API UISystemWin : public UISystemDefault
{
  FOG_DECLARE_OBJECT(UISystemWin, UISystemDefault)

  // [Construction / Destruction]

  UISystemWin();
  virtual ~UISystemWin();

  // [Display]

  virtual void updateDisplayInfo();

  // [Update]

  virtual void doBlitWindow(UIWindow* window, const Box* rects, sysuint_t count);

  // [UIWindow]

  virtual UIWindow* createUIWindow(Widget* widget);
  virtual void destroyUIWindow(UIWindow* native);

  // [Windows Specific]

  //! @brief Mouse wheel message ID.
  UINT uMSH_MOUSEWHEEL;

  //! @brief Virtual keys map.
  struct WinKeymap
  {
    uint16_t vk[256];
  } _winKeymap;

  uint32_t winKeyToModifier(WPARAM* wParam, LPARAM lParam);
  uint32_t winKeyToUnicode(UINT vKey, UINT scanCode);
  uint32_t winKeyToFogKey(WPARAM vk, UINT scancode);

  // [Debug]

  const char* msgToStr(uint message);

  friend struct UIWindowWin;
};

// ============================================================================
// [Fog::UIWindowWin]
// ============================================================================

struct FOG_API UIWindowWin : public UIWindowDefault
{
  FOG_DECLARE_OBJECT(UIWindowWin, UIWindowDefault)

  typedef HWND Handle;

  // [Construction / Destruction]

  UIWindowWin(Widget* widget);
  virtual ~UIWindowWin();

  // [Window Manipulation]

  virtual err_t create(uint32_t flags);
  virtual err_t destroy();

  virtual err_t enable();
  virtual err_t disable();

  virtual err_t show();
  virtual err_t hide();

  virtual err_t move(const Point& pt);
  virtual err_t resize(const Size& size);
  virtual err_t reconfigure(const Rect& rect);

  virtual err_t takeFocus();

  virtual err_t setTitle(const String32& title);
  virtual err_t getTitle(String32& title);

  virtual err_t setIcon(const Image& icon);
  virtual err_t getIcon(Image& icon);

  virtual err_t setSizeGranularity(const Point& pt);
  virtual err_t getSizeGranularity(Point& pt);

  virtual err_t worldToClient(Point* coords);
  virtual err_t clientToWorld(Point* coords);

  // [Windowing System]

  virtual void onMousePress(uint32_t button, bool repeated);
  virtual void onMouseRelease(uint32_t button);

  virtual LRESULT onWinMsg(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  void getWindowRect(Rect* windowRect, Rect* clientRect);

  friend struct UISystemWin;
};

// ============================================================================
// [Fog::UIBackingStoreWin]
// ============================================================================

//! @brief Provides cacheable buffer for widgets.
struct FOG_API UIBackingStoreWin : public UIBackingStore
{
  UIBackingStoreWin();
  virtual ~UIBackingStoreWin();

  virtual bool resize(int width, int height, bool cache);
  virtual void destroy();
  virtual void updateRects(const Box* rects, sysuint_t count);

  void blitRects(HDC target, const Box* rects, sysuint_t count);

  FOG_INLINE bool expired(TimeTicks now) const
  {
    return (_width != _widthOrig || _height != _heightOrig) && (now >= _expires);
  }

  FOG_INLINE HBITMAP hBitmap() const { return _hBitmap; }
  FOG_INLINE HDC hdc() const { return _hdc; }

  HBITMAP _hBitmap;
  HDC _hdc;

  // Type
  enum TypeEnum
  {
    TypeNone = 0,
    TypeDIB = 1
  };
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_UI_UISYSTEM_WIN_H
