// [Fog/UI Library - Public API]
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

#include <Fog/UI/UISystem/Base.h>

//! @addtogroup Fog_UI
//! @{

namespace Fog {

// ============================================================================
// [Fog::WinUISystem]
// ============================================================================

struct FOG_API WinUISystem : public BaseUISystem
{
  FOG_DECLARE_OBJECT(WinUISystem, BaseUISystem)

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  WinUISystem();
  virtual ~WinUISystem();

  // --------------------------------------------------------------------------
  // [Display]
  // --------------------------------------------------------------------------

  virtual void updateDisplayInfo();

  // --------------------------------------------------------------------------
  // [Update]
  // --------------------------------------------------------------------------

  virtual void doBlitWindow(UIWindow* window, const Box* rects, sysuint_t count);

  // --------------------------------------------------------------------------
  // [UIWindow]
  // --------------------------------------------------------------------------

  virtual UIWindow* createUIWindow(Widget* widget);
  virtual void destroyUIWindow(UIWindow* native);

  // --------------------------------------------------------------------------
  // [Windows Specific]
  // --------------------------------------------------------------------------

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

  // --------------------------------------------------------------------------
  // [Debug]
  // --------------------------------------------------------------------------

  const char* msgToStr(uint message);

  friend struct WinUIWindow;
};

// ============================================================================
// [Fog::WinUIWindow]
// ============================================================================

struct FOG_API WinUIWindow : public BaseUIWindow
{
  FOG_DECLARE_OBJECT(WinUIWindow, BaseUIWindow)

  typedef HWND Handle;

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  WinUIWindow(Widget* widget);
  virtual ~WinUIWindow();

  // --------------------------------------------------------------------------
  // [Window Manipulation]
  // --------------------------------------------------------------------------

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

  virtual err_t setTitle(const String& title);
  virtual err_t getTitle(String& title);

  virtual err_t setIcon(const Image& icon);
  virtual err_t getIcon(Image& icon);

  virtual err_t setSizeGranularity(const Point& pt);
  virtual err_t getSizeGranularity(Point& pt);

  virtual err_t worldToClient(Point* coords);
  virtual err_t clientToWorld(Point* coords);

  // --------------------------------------------------------------------------
  // [Windowing System]
  // --------------------------------------------------------------------------

  virtual void onMousePress(uint32_t button, bool repeated);
  virtual void onMouseRelease(uint32_t button);

  virtual LRESULT onWinMsg(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

  void getWindowRect(Rect* windowRect, Rect* clientRect);

  friend struct WinUISystem;
};

// ============================================================================
// [Fog::WinUIBackingStore]
// ============================================================================

//! @brief Provides cacheable buffer for widgets.
struct FOG_API WinUIBackingStore : public UIBackingStore
{
  WinUIBackingStore();
  virtual ~WinUIBackingStore();

  virtual bool resize(int width, int height, bool cache);
  virtual void destroy();
  virtual void updateRects(const Box* rects, sysuint_t count);

  void blitRects(HDC target, const Box* rects, sysuint_t count);

  FOG_INLINE HBITMAP getHBitmap() const { return _hBitmap; }
  FOG_INLINE HDC getHdc() const { return _hdc; }

  HBITMAP _hBitmap;
  HDC _hdc;

  // Type
  enum TYPE
  {
    TYPE_NONE = 0,
    TYPE_WIN_DIB = 1
  };
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_UI_UISYSTEM_WIN_H
