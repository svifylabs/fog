// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

#include <Fog/Build/Build.h>

#if defined(FOG_UI_WINDOWS)

// [Dependencies]
#include <Fog/Core/Application.h>
#include <Fog/UI/Constants.h>
#include <Fog/UI/Error.h>
#include <Fog/UI/UISystem_Win.h>
#include <Fog/UI/Widget.h>

// ----------------------------------------------------------------------------

#ifndef MSH_MOUSEWHEEL
# define MSH_MOUSEWHEEL L"MSWHEEL_ROLLMSG"
#endif

#ifndef WHEEL_DELTA
# define WHEEL_DELTA 120
#endif

#ifndef WM_MOUSEWHEEL
# define WM_MOUSEWHEEL (WM_MOUSELAST+1)
#endif

#ifndef MOUSEZ_CLASSNAME
# define MOUSEZ_CLASSNAME L"MouseZ"
#endif

#ifndef MOUSEZ_TITLE
# define MOUSEZ_TITLE L"Magellan MSWHEEL"
#endif

#define MSH_WHEELMODULE_CLASS (MOUSEZ_CLASSNAME)
#define MSH_WHEELMODULE_TITLE (MOUSEZ_TITLE)

#ifndef MSH_WHEELSUPPORT
// MSH_WHEELSUPPORT returns TRUE if wheel support is active, otherwise FALSE
# define MSH_WHEELSUPPORT L"MSH_WHEELSUPPORT_MSG"
#endif

#ifndef MSH_SCROLL_LINES
# define MSH_SCROLL_LINES L"MSH_SCROLL_LINES_MSG"
#endif
                                
#ifndef WHEEL_PAGESCROLL  
// MSH_SCROLL_LINES returns number of lines to scroll
# define WHEEL_PAGESCROLL (UINT_MAX)
#endif 

#ifndef SPI_SETWHEELSCROLLLINES
# define SPI_SETWHEELSCROLLLINES 105
#endif

// Keymasks
#ifndef REPEATED_KEYMASK
# define REPEATED_KEYMASK (1 << 30)
#endif

#ifndef EXTENDED_KEYMASK
# define EXTENDED_KEYMASK (1 << 24)
#endif

// ----------------------------------------------------------------------------

FOG_IMPLEMENT_OBJECT(Fog::UISystemWin)
FOG_IMPLEMENT_OBJECT(Fog::UIWindowWin)

namespace Fog {

// ============================================================================
// [Helpers]
// ============================================================================

#define UI_SYSTEM() \
  reinterpret_cast<UISystemWin*>(Application::instance()->uiSystem())

static void hwndGetRect(HWND handle, Rect* out)
{
  RECT wr;
  RECT cr;

  GetWindowRect(handle, &wr);
  GetClientRect(handle, &cr);

  int cw = cr.right - cr.left;
  int ch = cr.bottom - cr.top;

  out->set(wr.left, wr.top, cw, ch);
}

static LRESULT CALLBACK hwndWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  UIWindowWin* window = 
    reinterpret_cast<UIWindowWin*>(
      UI_SYSTEM()->handleToNative((void*)hwnd));

  if (window)
    return window->onWinMsg(hwnd, message, wParam, lParam);
  else
    return DefWindowProc(hwnd, message, wParam, lParam);
}

// Returns count of lines to scroll when using mouse wheel, inspired in
// MSDN http://msdn2.microsoft.com/en-us/library/ms645602.aspx
static uint32_t getWheelLinesCount(void)
{
  UINT ucNumLines = 3;
  SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &ucNumLines, 0);
  return ucNumLines;
}

// ============================================================================
// [Fog::UISystemWin - Construction / Destruction]
// ============================================================================

UISystemWin::UISystemWin()
{
  memset(&_winKeymap, 0, sizeof(_winKeymap));

  // Initialize key translation tables.
  Fog::Memory::zero(&_winKeymap, sizeof(WinKeymap));

  _winKeymap.vk[VK_BACK] = KeyBackspace;
  _winKeymap.vk[VK_TAB] = KeyTab;
  _winKeymap.vk[VK_CLEAR] = KeyClear;
  _winKeymap.vk[VK_RETURN] = KeyEnter;
  _winKeymap.vk[VK_PAUSE] = KeyPause;
  _winKeymap.vk[VK_ESCAPE] = KeyEsc;
  _winKeymap.vk[VK_SPACE] = KeySpace;
  _winKeymap.vk[0xDE] = KeyQuote;
  _winKeymap.vk[0xBC] = KeyComma;
  _winKeymap.vk[0xBD] = KeyMinus;
  _winKeymap.vk[0xBE] = KeyPeriod;
  _winKeymap.vk[0xBF] = KeySlash;
  _winKeymap.vk['0'] = Key0;
  _winKeymap.vk['1'] = Key1;
  _winKeymap.vk['2'] = Key2;
  _winKeymap.vk['3'] = Key3;
  _winKeymap.vk['4'] = Key4;
  _winKeymap.vk['5'] = Key5;
  _winKeymap.vk['6'] = Key6;
  _winKeymap.vk['7'] = Key7;
  _winKeymap.vk['8'] = Key8;
  _winKeymap.vk['9'] = Key9;
  _winKeymap.vk[0xBA] = KeySemicolon;
  _winKeymap.vk[0xBB] = KeyEquals;
  _winKeymap.vk[0xDB] = KeyLeftBrace;
  _winKeymap.vk[0xDC] = KeyBackslash;
  _winKeymap.vk[0xE2] = KeyLess;
  _winKeymap.vk[0xDD] = KeyRightBrace;
  _winKeymap.vk[0xC0] = KeyBackquote;
  _winKeymap.vk[0xDF] = KeyBackquote;
  _winKeymap.vk['A'] = KeyA;
  _winKeymap.vk['B'] = KeyB;
  _winKeymap.vk['C'] = KeyC;
  _winKeymap.vk['D'] = KeyD;
  _winKeymap.vk['E'] = KeyE;
  _winKeymap.vk['F'] = KeyF;
  _winKeymap.vk['G'] = KeyG;
  _winKeymap.vk['H'] = KeyH;
  _winKeymap.vk['I'] = KeyI;
  _winKeymap.vk['J'] = KeyJ;
  _winKeymap.vk['K'] = KeyK;
  _winKeymap.vk['L'] = KeyL;
  _winKeymap.vk['M'] = KeyM;
  _winKeymap.vk['N'] = KeyN;
  _winKeymap.vk['O'] = KeyO;
  _winKeymap.vk['P'] = KeyP;
  _winKeymap.vk['Q'] = KeyQ;
  _winKeymap.vk['R'] = KeyR;
  _winKeymap.vk['S'] = KeyS;
  _winKeymap.vk['T'] = KeyT;
  _winKeymap.vk['U'] = KeyU;
  _winKeymap.vk['V'] = KeyV;
  _winKeymap.vk['W'] = KeyW;
  _winKeymap.vk['X'] = KeyX;
  _winKeymap.vk['Y'] = KeyY;
  _winKeymap.vk['Z'] = KeyZ;
  _winKeymap.vk[VK_DELETE] = KeyDelete;

  _winKeymap.vk[VK_NUMPAD0] = KeyKP0;
  _winKeymap.vk[VK_NUMPAD1] = KeyKP1;
  _winKeymap.vk[VK_NUMPAD2] = KeyKP2;
  _winKeymap.vk[VK_NUMPAD3] = KeyKP3;
  _winKeymap.vk[VK_NUMPAD4] = KeyKP4;
  _winKeymap.vk[VK_NUMPAD5] = KeyKP5;
  _winKeymap.vk[VK_NUMPAD6] = KeyKP6;
  _winKeymap.vk[VK_NUMPAD7] = KeyKP7;
  _winKeymap.vk[VK_NUMPAD8] = KeyKP8;
  _winKeymap.vk[VK_NUMPAD9] = KeyKP9;
  _winKeymap.vk[VK_DECIMAL] = KeyKPPeriod;
  _winKeymap.vk[VK_DIVIDE] = KeyKPDivide;
  _winKeymap.vk[VK_MULTIPLY] = KeyKPMultiply;
  _winKeymap.vk[VK_SUBTRACT] = KeyKPMinus;
  _winKeymap.vk[VK_ADD] = KeyKPPlus;

  _winKeymap.vk[VK_UP] = KeyUp;
  _winKeymap.vk[VK_DOWN] = KeyDown;
  _winKeymap.vk[VK_RIGHT] = KeyRight;
  _winKeymap.vk[VK_LEFT] = KeyLeft;
  _winKeymap.vk[VK_INSERT] = KeyInsert;
  _winKeymap.vk[VK_HOME] = KeyHome;
  _winKeymap.vk[VK_END] = KeyEnd;
  _winKeymap.vk[VK_PRIOR] = KeyPageUp;
  _winKeymap.vk[VK_NEXT] = KeyPageDown;

  _winKeymap.vk[VK_F1] = KeyF1;
  _winKeymap.vk[VK_F2] = KeyF2;
  _winKeymap.vk[VK_F3] = KeyF3;
  _winKeymap.vk[VK_F4] = KeyF4;
  _winKeymap.vk[VK_F5] = KeyF5;
  _winKeymap.vk[VK_F6] = KeyF6;
  _winKeymap.vk[VK_F7] = KeyF7;
  _winKeymap.vk[VK_F8] = KeyF8;
  _winKeymap.vk[VK_F9] = KeyF9;
  _winKeymap.vk[VK_F10] = KeyF10;
  _winKeymap.vk[VK_F11] = KeyF11;
  _winKeymap.vk[VK_F12] = KeyF12;
  _winKeymap.vk[VK_F13] = KeyF13;
  _winKeymap.vk[VK_F14] = KeyF14;
  _winKeymap.vk[VK_F15] = KeyF15;

  _winKeymap.vk[VK_NUMLOCK] = KeyNumLock;
  _winKeymap.vk[VK_CAPITAL] = KeyCapsLock;
  _winKeymap.vk[VK_SCROLL] = KeyScrollLock;
  _winKeymap.vk[VK_RSHIFT] = KeyRightShift;
  _winKeymap.vk[VK_LSHIFT] = KeyLeftShift;
  _winKeymap.vk[VK_RCONTROL] = KeyRightCtrl;
  _winKeymap.vk[VK_LCONTROL] = KeyLeftCtrl;
  _winKeymap.vk[VK_RMENU] = KeyRightAlt;
  _winKeymap.vk[VK_LMENU] = KeyLeftAlt;
  _winKeymap.vk[VK_RWIN] = KeyRightSuper;
  _winKeymap.vk[VK_LWIN] = KeyLeftSuper;

  _winKeymap.vk[VK_HELP] = KeyHelp;
  _winKeymap.vk[42 /*VK_PRINT*/] = KeyPrint;
  _winKeymap.vk[VK_SNAPSHOT] = KeyPrint;
  _winKeymap.vk[VK_CANCEL] = KeyBreak;
  _winKeymap.vk[VK_APPS] = KeyMenu;

  uMSH_MOUSEWHEEL = RegisterWindowMessageW(MSH_MOUSEWHEEL);

  // Get Wheel lines count.
  _wheelLines = getWheelLinesCount();

  // Query timings.
  INT keyboardDelay;
  DWORD keyboardSpeed;
  DWORD dwTime;

  if (SystemParametersInfo(SPI_GETKEYBOARDDELAY, 0, &keyboardDelay, 0))
  {
    _repeatingDelay = TimeDelta::fromMilliseconds((int64_t)((0.25 * (double)(keyboardDelay + 1)) * 1000));
  }

  if (SystemParametersInfo(SPI_GETKEYBOARDSPEED, 0, &keyboardSpeed, 0))
  {
    _repeatingInterval = TimeDelta::fromMilliseconds((int64_t)((1 / (2.5 + (double)keyboardSpeed * 0.90)) * 1000));
  }

  if (SystemParametersInfo(SPI_SETDOUBLECLICKTIME, 0, &dwTime, 0))
  {
    _doubleClickInterval = TimeDelta::fromMilliseconds(dwTime);
  }

  HINSTANCE hInstance = (HINSTANCE)GetModuleHandleW(NULL);
  WNDCLASSEXW wc;

  // Init Window Class structure.
  wc.cbSize        = sizeof(WNDCLASSEXW);
  wc.style         = 0;
  wc.lpfnWndProc   = hwndWndProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = hInstance;
  wc.hIcon         = LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION);
  wc.hCursor       = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
  wc.hbrBackground = NULL;
  wc.lpszMenuName  = NULL;
  wc.lpszClassName = NULL;
  wc.hIconSm       = LoadIconW(NULL, (LPCWSTR)IDI_APPLICATION);

  // "Fog_Window" window class.
  wc.style         = CS_OWNDC;
  wc.lpszClassName = L"Fog_Window";
  if (!RegisterClassExW(&wc)) return;

  // "Fog_Popup" window class.
  wc.style         = CS_OWNDC;
  wc.lpszClassName = L"Fog_Popup";
  if (!RegisterClassExW(&wc)) return;

  updateDisplayInfo();
  _initialized = true;
}

UISystemWin::~UISystemWin()
{
  // Is safe to unregister window classes when we ends ?
  // HINSTANCE hInstance = (HINSTANCE)GetModuleHandleW(NULL);
  // UnregisterClassW(L"Fog_Window", hInstance);
  // UnregisterClassW(L"Fog_Popup", hInstance);
}

// ============================================================================
// [Fog::UISystemWin - Display]
// ============================================================================

void UISystemWin::updateDisplayInfo()
{
  int dibSize;
  LPBITMAPINFOHEADER dibHdr;
  HDC hdc;
  HBITMAP hbm;

  // Screen size
  _displayInfo.width  = ::GetSystemMetrics(SM_CXSCREEN);
  _displayInfo.height = ::GetSystemMetrics(SM_CYSCREEN);
  _displayInfo.is16BitSwapped = false;

  // Allocate enough space for a DIB header plus palette 
  // (for 8-bit modes) or bitfields (for 16 and 32-bit modes)
  dibSize = sizeof(BITMAPINFOHEADER) + 256 * sizeof (RGBQUAD);

  dibHdr = (LPBITMAPINFOHEADER)Memory::calloc(dibSize);
  if (!dibHdr) return;

  dibHdr->biSize = sizeof(BITMAPINFOHEADER);

  // Get a device-dependent bitmap that's compatible with the screen.
  hdc = GetDC(NULL);
  hbm = CreateCompatibleBitmap(hdc, 1, 1);

  // Convert the DDB to a DIB.  We need to call GetDIBits twice:
  // the first call just fills in the BITMAPINFOHEADER; the 
  // second fills in the bitfields or palette.
  GetDIBits(hdc, hbm, 0, 1, NULL, (LPBITMAPINFO)dibHdr, DIB_RGB_COLORS);
  GetDIBits(hdc, hbm, 0, 1, NULL, (LPBITMAPINFO)dibHdr, DIB_RGB_COLORS);
  DeleteObject(hbm);
  ReleaseDC(NULL, hdc);

  _displayInfo.depth = dibHdr->biBitCount;

  _paletteInfo.palCount = 0;
  memset(_paletteInfo.palConv, 0, sizeof(_paletteInfo.palConv));

  switch (dibHdr->biBitCount)
  {
    case 8:
      _paletteInfo.palCount = 256;
      break;
    case 16:
      if (dibHdr->biCompression == BI_BITFIELDS)
      {
        _displayInfo.rMask = ((DWORD*)((uint8_t*)dibHdr + dibHdr->biSize))[0];
        _displayInfo.gMask = ((DWORD*)((uint8_t*)dibHdr + dibHdr->biSize))[1];
        _displayInfo.bMask = ((DWORD*)((uint8_t*)dibHdr + dibHdr->biSize))[2];
      }
      else
      {
        // I don't know if this can happen...
        _displayInfo.rMask = 0x0000F800;
        _displayInfo.gMask = 0x000007E0;
        _displayInfo.bMask = 0x0000001F;
      }
      break;
    case 24:
      _displayInfo.rMask = 0x00FF0000;
      _displayInfo.gMask = 0x0000FF00;
      _displayInfo.bMask = 0x000000FF;
      break;
    case 32:
      _displayInfo.rMask = 0x00FF0000;
      _displayInfo.gMask = 0x0000FF00;
      _displayInfo.bMask = 0x000000FF;
      break;
    default:
      _displayInfo.depth = 0;
      _displayInfo.rMask = 0x00000000;
      _displayInfo.gMask = 0x00000000;
      _displayInfo.bMask = 0x00000000;
      break;
  }

  Memory::free(dibHdr);
}

// ============================================================================
// [Fog::UISystemWin - Update]
// ============================================================================

void UISystemWin::doBlitWindow(UIWindow* window, const Box* rects, sysuint_t count)
{
  HDC hdc = GetDC((HWND)window->handle());
  reinterpret_cast<UIBackingStoreWin*>(window->_backingStore)->blitRects(hdc, rects, count);
  ReleaseDC((HWND)window->handle(), hdc);
}

// ============================================================================
// [Fog::UISystemWin - UIWindow]
// ============================================================================

UIWindow* UISystemWin::createUIWindow(Widget* widget)
{
  return new UIWindowWin(widget);
}

void UISystemWin::destroyUIWindow(UIWindow* native)
{
  delete native;
}

// ============================================================================
// [Fog::UISystemWin - Windows Specific]
// ============================================================================

uint32_t UISystemWin::winKeyToModifier(WPARAM* wParam, LPARAM lParam)
{
  uint32_t modifier = 0;

  switch (*wParam)
  {
    case VK_SHIFT:
    {
      if ((_keyboardStatus.modifiers & ModifierLeftShift) != 0 && !(GetKeyState(VK_LSHIFT) & 0x8000))
      {
        *wParam = VK_LSHIFT;
      } 
      else if ((_keyboardStatus.modifiers & ModifierRightShift) != 0 && !(GetKeyState(VK_RSHIFT) & 0x8000))
      {
        *wParam = VK_RSHIFT;
      } 
      else 
      {
        // Win9x
        int sc = HIWORD(lParam) & 0xFF;

        if (sc == 0x2A)
          *wParam = VK_LSHIFT;
        else if (sc == 0x36)
          *wParam = VK_RSHIFT;
        else
          *wParam = VK_LSHIFT;
      }
      break;
    }

    case VK_CONTROL:
      if (lParam & EXTENDED_KEYMASK)
        *wParam = VK_RCONTROL;
      else
        *wParam = VK_LCONTROL;
      break;

    case VK_MENU:
      if (lParam & EXTENDED_KEYMASK)
        *wParam = VK_RMENU;
      else
        *wParam = VK_LMENU;
      break;
  }

  switch (*wParam)
  {
    case VK_LSHIFT  : modifier = ModifierLeftShift; break;
    case VK_RSHIFT  : modifier = ModifierRightShift; break;
    case VK_LCONTROL: modifier = ModifierLeftCtrl; break;
    case VK_RCONTROL: modifier = ModifierRightCtrl; break;
    case VK_LMENU   : modifier = ModifierLeftAlt; break;
    case VK_RMENU   : modifier = ModifierRightAlt; break;

    case VK_CAPITAL : modifier = ModifierCaps; break;
    case VK_NUMLOCK : modifier = ModifierNum; break;
    case VK_INSERT  : modifier = ModifierMode; break;
  }

  return modifier;
}

uint32_t UISystemWin::winKeyToUnicode(UINT vKey, UINT scanCode)
{
  switch (vKey)
  {
    case VK_LEFT:
    case VK_RIGHT:
    case VK_UP:
    case VK_DOWN:
    case VK_TAB:
    case VK_ESCAPE:
    case VK_BACK:
    case VK_RETURN:
      return 0;
    default:
    {
      BYTE keystate[256];
      WCHAR unicode[2];

      GetKeyboardState(keystate);
      if (ToUnicode(vKey, scanCode, keystate, unicode, FOG_ARRAY_SIZE(unicode), 0) == 1 && unicode[0] > 31)
        return unicode[0];
      else
        return 0;
    }
  }
}

uint32_t UISystemWin::winKeyToFogKey(WPARAM vk, UINT scancode)
{
  uint32_t key;

  if ((vk == VK_RETURN) && (scancode & 0x100)) 
    key = KeyKPEnter;
  else 
    key = _winKeymap.vk[vk];

  return key;
}

// ============================================================================
// [Fog::UISystemWin - Debug]
// ============================================================================

const char* UISystemWin::msgToStr(uint message)
{
  struct MsgTable
  {
    uint message;
    const char *str;
  };

  static const MsgTable msgTable[] = {
    {  WM_NULL, "WM_NULL"  },
    {  WM_CREATE, "WM_CREATE"  },
    {  WM_DESTROY, "WM_DESTROY"  },
    {  WM_MOVE, "WM_MOVE"  },
    {  WM_SIZE, "WM_SIZE"  },
    {  WM_ACTIVATE, "WM_ACTIVATE"  },
    {  WM_SETFOCUS, "WM_SETFOCUS"  },
    {  WM_KILLFOCUS, "WM_KILLFOCUS"  },
    {  WM_ENABLE, "WM_ENABLE"  },
    {  WM_SETREDRAW, "WM_SETREDRAW"  },
    {  WM_SETTEXT, "WM_SETTEXT"  },
    {  WM_GETTEXT, "WM_GETTEXT"  },
    {  WM_GETTEXTLENGTH, "WM_GETTEXTLENGTH"  },
    {  WM_PAINT, "WM_PAINT"  },
    {  WM_CLOSE, "WM_CLOSE"  },
    {  WM_QUERYENDSESSION, "WM_QUERYENDSESSION"  },
    {  WM_QUIT, "WM_QUIT"  },
    {  WM_QUERYOPEN, "WM_QUERYOPEN"  },
    {  WM_ERASEBKGND, "WM_ERASEBKGND"  },
    {  WM_SYSCOLORCHANGE, "WM_SYSCOLORCHANGE"  },
    {  WM_ENDSESSION, "WM_ENDSESSION"  },
    {  WM_SHOWWINDOW, "WM_SHOWWINDOW"  },
    {  WM_WININICHANGE, "WM_WININICHANGE"  },
    {  WM_DEVMODECHANGE, "WM_DEVMODECHANGE"  },
    {  WM_ACTIVATEAPP, "WM_ACTIVATEAPP"  },
    {  WM_FONTCHANGE, "WM_FONTCHANGE"  },
    {  WM_TIMECHANGE, "WM_TIMECHANGE"  },
    {  WM_CANCELMODE, "WM_CANCELMODE"  },
    {  WM_SETCURSOR, "WM_SETCURSOR"  },
    {  WM_MOUSEACTIVATE, "WM_MOUSEACTIVATE"  },
    {  WM_CHILDACTIVATE, "WM_CHILDACTIVATE"  },
    {  WM_QUEUESYNC, "WM_QUEUESYNC"  },
    {  WM_GETMINMAXINFO, "WM_GETMINMAXINFO"  },
    {  WM_PAINTICON, "WM_PAINTICON"  },
    {  WM_ICONERASEBKGND, "WM_ICONERASEBKGND"  },
    {  WM_NEXTDLGCTL, "WM_NEXTDLGCTL"  },
    {  WM_SPOOLERSTATUS, "WM_SPOOLERSTATUS"  },
    {  WM_DRAWITEM, "WM_DRAWITEM"  },
    {  WM_MEASUREITEM, "WM_MEASUREITEM"  },
    {  WM_DELETEITEM, "WM_DELETEITEM"  },
    {  WM_VKEYTOITEM, "WM_VKEYTOITEM"  },
    {  WM_CHARTOITEM, "WM_CHARTOITEM"  },
    {  WM_SETFONT, "WM_SETFONT"  },
    {  WM_GETFONT, "WM_GETFONT"  },
    {  WM_SETHOTKEY, "WM_SETHOTKEY"  },
    {  WM_GETHOTKEY, "WM_GETHOTKEY"  },
    {  WM_QUERYDRAGICON, "WM_QUERYDRAGICON"  },
    {  WM_COMPAREITEM, "WM_COMPAREITEM"  },
    {  WM_COMPACTING, "WM_COMPACTING"  },
    {  WM_WINDOWPOSCHANGING, "WM_WINDOWPOSCHANGING"  },
    {  WM_WINDOWPOSCHANGED, "WM_WINDOWPOSCHANGED"  },
    {  WM_POWER, "WM_POWER"  },
    {  WM_COPYDATA, "WM_COPYDATA"  },
    {  WM_CANCELJOURNAL, "WM_CANCELJOURNAL"  },
    {  WM_NCCREATE, "WM_NCCREATE"  },
    {  WM_NCDESTROY, "WM_NCDESTROY"  },
    {  WM_NCCALCSIZE, "WM_NCCALCSIZE"  },
    {  WM_NCHITTEST, "WM_NCHITTEST"  },
    {  WM_NCPAINT, "WM_NCPAINT"  },
#if defined(WM_SYNCPAINT)
    {  WM_SYNCPAINT, "WM_SYNCPAINT"  },
#endif
    {  WM_NCACTIVATE, "WM_NCACTIVATE"  },
    {  WM_GETDLGCODE, "WM_GETDLGCODE"  },
    {  WM_NCMOUSEMOVE, "WM_NCMOUSEMOVE"  },
    {  WM_NCLBUTTONDOWN, "WM_NCLBUTTONDOWN"  },
    {  WM_NCLBUTTONUP, "WM_NCLBUTTONUP"  },
    {  WM_NCLBUTTONDBLCLK, "WM_NCLBUTTONDBLCLK"  },
    {  WM_NCRBUTTONDOWN, "WM_NCRBUTTONDOWN"  },
    {  WM_NCRBUTTONUP, "WM_NCRBUTTONUP"  },
    {  WM_NCRBUTTONDBLCLK, "WM_NCRBUTTONDBLCLK"  },
    {  WM_NCMBUTTONDOWN, "WM_NCMBUTTONDOWN"  },
    {  WM_NCMBUTTONUP, "WM_NCMBUTTONUP"  },
    {  WM_NCMBUTTONDBLCLK, "WM_NCMBUTTONDBLCLK"  },
    {  WM_KEYFIRST, "WM_KEYFIRST"  },
    {  WM_KEYDOWN, "WM_KEYDOWN"  },
    {  WM_KEYUP, "WM_KEYUP"  },
    {  WM_CHAR, "WM_CHAR"  },
    {  WM_DEADCHAR, "WM_DEADCHAR"  },
    {  WM_SYSKEYDOWN, "WM_SYSKEYDOWN"  },
    {  WM_SYSKEYUP, "WM_SYSKEYUP"  },
    {  WM_SYSCHAR, "WM_SYSCHAR"  },
    {  WM_SYSDEADCHAR, "WM_SYSDEADCHAR"  },
    {  WM_KEYLAST, "WM_KEYLAST"  },
    {  WM_INITDIALOG, "WM_INITDIALOG"  },
    {  WM_COMMAND, "WM_COMMAND"  },
    {  WM_SYSCOMMAND, "WM_SYSCOMMAND"  },
    {  WM_TIMER, "WM_TIMER"  },
    {  WM_HSCROLL, "WM_HSCROLL"  },
    {  WM_VSCROLL, "WM_VSCROLL"  },
    {  WM_INITMENU, "WM_INITMENU"  },
    {  WM_INITMENUPOPUP, "WM_INITMENUPOPUP"  },
    {  WM_MENUSELECT, "WM_MENUSELECT"  },
    {  WM_MENUCHAR, "WM_MENUCHAR"  },
    {  WM_ENTERIDLE, "WM_ENTERIDLE"  },
    {  WM_CTLCOLORMSGBOX, "WM_CTLCOLORMSGBOX"  },
    {  WM_CTLCOLOREDIT, "WM_CTLCOLOREDIT"  },
    {  WM_CTLCOLORLISTBOX, "WM_CTLCOLORLISTBOX"  },
    {  WM_CTLCOLORBTN, "WM_CTLCOLORBTN"  },
    {  WM_CTLCOLORDLG, "WM_CTLCOLORDLG"  },
    {  WM_CTLCOLORSCROLLBAR, "WM_CTLCOLORSCROLLBAR"  },
    {  WM_CTLCOLORSTATIC, "WM_CTLCOLORSTATIC"  },
    {  WM_MOUSEFIRST, "WM_MOUSEFIRST"  },
    {  WM_MOUSEMOVE, "WM_MOUSEMOVE"  },
    {  WM_LBUTTONDOWN, "WM_LBUTTONDOWN"  },
    {  WM_LBUTTONUP, "WM_LBUTTONUP"  },
    {  WM_LBUTTONDBLCLK, "WM_LBUTTONDBLCLK"  },
    {  WM_RBUTTONDOWN, "WM_RBUTTONDOWN"  },
    {  WM_RBUTTONUP, "WM_RBUTTONUP"  },
    {  WM_RBUTTONDBLCLK, "WM_RBUTTONDBLCLK"  },
    {  WM_MBUTTONDOWN, "WM_MBUTTONDOWN"  },
    {  WM_MBUTTONUP, "WM_MBUTTONUP"  },
    {  WM_MBUTTONDBLCLK, "WM_MBUTTONDBLCLK"  },
    {  WM_MOUSELAST, "WM_MOUSELAST"  },
    {  WM_PARENTNOTIFY, "WM_PARENTNOTIFY"  },
    {  WM_ENTERMENULOOP, "WM_ENTERMENULOOP"  },
    {  WM_EXITMENULOOP, "WM_EXITMENULOOP"  },
    {  WM_MDICREATE, "WM_MDICREATE"  },
    {  WM_MDIDESTROY, "WM_MDIDESTROY"  },
    {  WM_MDIACTIVATE, "WM_MDIACTIVATE"  },
    {  WM_MDIRESTORE, "WM_MDIRESTORE"  },
    {  WM_MDINEXT, "WM_MDINEXT"  },
    {  WM_MDIMAXIMIZE, "WM_MDIMAXIMIZE"  },
    {  WM_MDITILE, "WM_MDITILE"  },
    {  WM_MDICASCADE, "WM_MDICASCADE"  },
    {  WM_MDIICONARRANGE, "WM_MDIICONARRANGE"  },
    {  WM_MDIGETACTIVE, "WM_MDIGETACTIVE"  },
    {  WM_MDISETMENU, "WM_MDISETMENU"  },
    {  WM_DROPFILES, "WM_DROPFILES"  },
    {  WM_MDIREFRESHMENU, "WM_MDIREFRESHMENU"  },
    {  WM_CUT, "WM_CUT"  },
    {  WM_COPY, "WM_COPY"  },
    {  WM_PASTE, "WM_PASTE"  },
    {  WM_CLEAR, "WM_CLEAR"  },
    {  WM_UNDO, "WM_UNDO"  },
    {  WM_RENDERFORMAT, "WM_RENDERFORMAT"  },
    {  WM_RENDERALLFORMATS, "WM_RENDERALLFORMATS"  },
    {  WM_DESTROYCLIPBOARD, "WM_DESTROYCLIPBOARD"  },
    {  WM_DRAWCLIPBOARD, "WM_DRAWCLIPBOARD"  },
    {  WM_PAINTCLIPBOARD, "WM_PAINTCLIPBOARD"  },
    {  WM_VSCROLLCLIPBOARD, "WM_VSCROLLCLIPBOARD"  },
    {  WM_SIZECLIPBOARD, "WM_SIZECLIPBOARD"  },
    {  WM_ASKCBFORMATNAME, "WM_ASKCBFORMATNAME"  },
    {  WM_CHANGECBCHAIN, "WM_CHANGECBCHAIN"  },
    {  WM_HSCROLLCLIPBOARD, "WM_HSCROLLCLIPBOARD"  },
    {  WM_QUERYNEWPALETTE, "WM_QUERYNEWPALETTE"  },
    {  WM_PALETTEISCHANGING, "WM_PALETTEISCHANGING"  },
    {  WM_PALETTECHANGED, "WM_PALETTECHANGED"  },
    {  WM_HOTKEY, "WM_HOTKEY"  },
    {  WM_PENWINFIRST, "WM_PENWINFIRST"  },
    {  WM_PENWINLAST, "WM_PENWINLAST"  }
#if defined(WM_SIZING)
    ,{  WM_SIZING, "WM_SIZING"  }
#endif
#if defined(WM_MOVING)
    ,{  WM_MOVING, "WM_MOVING"  }
#endif
#if defined(WM_ENTERSIZEMOVE)
    ,{  WM_ENTERSIZEMOVE, "WM_ENTERSIZEMOVE"  }
#endif
#if defined(WM_EXITSIZEMOVE)
    ,{  WM_EXITSIZEMOVE, "WM_EXITSIZEMOVE"  }
#endif
#if defined(WM_ASYNCIO)
    ,{  WM_ASYNCIO, "WM_ASYNCIO"  }
#endif
  };

  for (sysuint_t i = 0; i != FOG_ARRAY_SIZE(msgTable); i++)
  {
    if (msgTable[i].message == message) return msgTable[i].str;
  }

  return "Unknown";
}

// ============================================================================
// [Fog::UIWindowWin]
// ============================================================================

UIWindowWin::UIWindowWin(Widget* widget) : 
  UIWindowDefault(widget)
{
  _backingStore = new UIBackingStoreWin();
}

UIWindowWin::~UIWindowWin()
{
  destroy();
  delete _backingStore;
}

err_t UIWindowWin::create(uint32_t flags)
{
  if (_handle) return Error::UIWindowAlreadyExists;

  // Create the Window (HWND is returned)
  DWORD dwStyle;
  DWORD dwStyleEx;
  int x;
  int y;
  WCHAR* wndClass;

  if (flags & CreatePopup)
  {
    dwStyle = WS_POPUP;
    dwStyleEx = WS_EX_TOOLWINDOW;
    x = -1;
    y = -1;
    wndClass = L"Fog_Popup";
  }
  else
  {
    dwStyle = WS_OVERLAPPEDWINDOW;
    dwStyleEx = WS_EX_CLIENTEDGE;
    x = CW_USEDEFAULT;
    y = CW_USEDEFAULT;
    wndClass = L"Fog_Window";
  }

  _handle = (void*)CreateWindowExW(
    dwStyleEx, wndClass, L"",
    dwStyle, x, y,
    1, 1,
    NULL, NULL, (HINSTANCE)GetModuleHandleW(NULL), NULL);

  if (_handle == NULL)
  {
    fog_stderr_msg("Fog::UIWindowWin", "create", "CreateWindowExW() failed.");
    goto fail;
  }

  // Create HWND <-> UIWindow* connection.
  UI_SYSTEM()->mapHandle(_handle, this);

  // Windows are enabled by default.
  _enabled = true;
  // Windows are not visible by default.
  _visible = false;
  // Need to blit window content.
  _blit = true;
  // Default focus is no focus, Windows will inform us if this gets changed.
  _focus = false;

  // Get correct window and client rectangle.
  getWindowRect(&_windowRect, &_clientRect);

  return Error::Ok;

fail:
  return Error::FailedToCreateUIWindow;
}

err_t UIWindowWin::destroy()
{
  if (!_handle) return Error::InvalidHandle;

  // Destroy HWND <-> UIWindow* connection.
  UI_SYSTEM()->unmapHandle(_handle);

  // Destroy HWND.
  DestroyWindow((HWND)_handle);

  // Clear all variables.
  _handle = NULL;
  _focus = false;
  _blit = false;

  return Error::Ok;
}

err_t UIWindowWin::enable()
{
  if (!_handle) return Error::InvalidHandle;

  if (!_enabled) EnableWindow((HWND)_handle, TRUE);
  return Error::Ok;
}

err_t UIWindowWin::disable()
{
  if (!_handle) return Error::InvalidHandle;

  if (_enabled) EnableWindow((HWND)_handle, FALSE);
  return Error::Ok;
}

err_t UIWindowWin::show()
{
  if (!_handle) return Error::InvalidHandle;

  if (!_visible) ShowWindow((HWND)_handle, SW_SHOWNA);
  return Error::Ok;
}

err_t UIWindowWin::hide()
{
  if (!_handle) return Error::InvalidHandle;

  if (_visible) ShowWindow((HWND)_handle, SW_HIDE);
  return Error::Ok;
}

err_t UIWindowWin::move(const Point& pt)
{
  if (!_handle) return Error::InvalidHandle;

  if (_windowRect.x1() != pt.x() || _windowRect.y1() != pt.y())
  {
    MoveWindow((HWND)_handle,
      pt.x(),
      pt.y(), 
      _windowRect.width(),
      _windowRect.height(),
      FALSE);
  }

  return Error::Ok;
}

err_t UIWindowWin::resize(const Size& size)
{
  if (!_handle) return Error::InvalidHandle;
  if (size.width() <= 0 || size.height() <= 0) return Error::InvalidArgument;

  if (_windowRect.size() != size)
  {
    MoveWindow((HWND)_handle,
      _widget->x1(),
      _widget->y1(),
      size.width(),
      size.height(),
      TRUE);
  }

  return Error::Ok;
}

err_t UIWindowWin::reconfigure(const Rect& rect)
{
  if (!_handle) return Error::InvalidHandle;
  if (!rect.isValid()) return Error::InvalidArgument;

  if (_windowRect != rect)
  {
    MoveWindow((HWND)_handle, 
      rect.x1(),
      rect.y1(),
      rect.width(),
      rect.height(),
      TRUE);
  }

  return Error::Ok;
}

err_t UIWindowWin::takeFocus()
{
  if (!_handle) return Error::InvalidHandle;

  SetFocus((HWND)_handle);
  return Error::Ok;
}

err_t UIWindowWin::setTitle(const String32& title)
{
  if (!_handle) return Error::InvalidHandle;

  err_t err;
  TemporaryString16<TemporaryLength> titleW;

  if ( (err = titleW.set(title)) ) return err;

  SetWindowTextW((HWND)_handle, titleW.cStrW());
  _title = title;

  return Error::Ok;
}

err_t UIWindowWin::getTitle(String32& title)
{
  if (!_handle) return Error::InvalidHandle;

  title = _title;
  return Error::Ok;
}

err_t UIWindowWin::setIcon(const Image& icon)
{
  if (!_handle) return Error::InvalidHandle;

}

err_t UIWindowWin::getIcon(Image& icon)
{
  if (!_handle) return Error::InvalidHandle;

}

err_t UIWindowWin::setSizeGranularity(const Point& pt)
{
  if (!_handle) return Error::InvalidHandle;

  _sizeGranularity = pt;
  return Error::Ok;
}

err_t UIWindowWin::getSizeGranularity(Point& pt)
{
  if (!_handle) return Error::InvalidHandle;

  pt = _sizeGranularity;
  return Error::Ok;
}

err_t UIWindowWin::worldToClient(Point* coords)
{
  if (!_handle) return Error::InvalidHandle;

  return ScreenToClient((HWND)_handle, (POINT *)coords)
    ? (err_t)Error::Ok
    : (err_t)Error::FailedToTranslateCoordinates;
}

err_t UIWindowWin::clientToWorld(Point* coords)
{
  if (!_handle) return Error::InvalidHandle;

  return ClientToScreen((HWND)_handle, (POINT *)coords) 
    ? (err_t)Error::Ok
    : (err_t)Error::FailedToTranslateCoordinates;
}

void UIWindowWin::onMousePress(uint32_t button, bool repeated)
{
  UISystemDefault* uiSystem = UI_SYSTEM();
  if (uiSystem->_systemMouseStatus.uiWindow != this) return;

  if (uiSystem->_systemMouseStatus.buttons == 0 && !repeated)
  {
    SetCapture((HWND)_handle);
  }

  base::onMousePress(button, repeated);
}

void UIWindowWin::onMouseRelease(uint32_t button)
{
  UISystemDefault* uiSystem = UI_SYSTEM();
  if (uiSystem->_systemMouseStatus.uiWindow != this) return;

  if ((uiSystem->_systemMouseStatus.buttons & ~button) == 0)
  {
    ReleaseCapture();
  }

  base::onMouseRelease(button);
}

LRESULT UIWindowWin::onWinMsg(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  UISystemWin* uiSystem = UI_SYSTEM();

  switch (message)
  {
    case WM_SHOWWINDOW:
      onVisibility(wParam ? true : false);
      return 0;

    case WM_WINDOWPOSCHANGED:
    {
      Rect wr;
      Rect cr;
      getWindowRect(&wr, &cr);

      WINDOWPOS *pos = (WINDOWPOS*)lParam;
      wr.setX(pos->x);
      wr.setY(pos->y);

      onConfigure(wr, cr);
      return 0;
    }

    // case WM_GETMINMAXINFO:
    // {
    //   return 0;
    // }

    case WM_SIZE:
    {
      Rect wr;
      Rect cr;
      getWindowRect(&wr, &cr);

      onConfigure(wr, cr);
      return 0;
    }

    case WM_SETFOCUS:
      onFocus(true);
      return 0;

    case WM_KILLFOCUS:
      onFocus(false);
      return 0;

    case WM_MOUSEHOVER:
    case WM_MOUSEMOVE:
    {
      // Mouse tracking for WM_MOUSELEAVE message.
      TRACKMOUSEEVENT tme;
      tme.cbSize    = sizeof(TRACKMOUSEEVENT);
      tme.dwFlags   = TME_LEAVE;
      tme.hwndTrack = hwnd;
      TrackMouseEvent(&tme);

      int x = (int)(short)LOWORD(lParam);
      int y = (int)(short)HIWORD(lParam);

      // WM_MOUSEHOVER is not sent sometimes.
      if (uiSystem->_systemMouseStatus.uiWindow != this || 
         !uiSystem->_systemMouseStatus.hover)
      {
        onMouseHover(x, y);
      }
      else
      {
        onMouseMove(x, y);
      }

      return 0;
    }

    case WM_MOUSELEAVE:
    {
      if (uiSystem->_systemMouseStatus.uiWindow == this && 
         !uiSystem->_systemMouseStatus.buttons)
      {
        onMouseLeave(-1, -1);
      }
      return 0;
    }

    case WM_LBUTTONDOWN: onMousePress(ButtonLeft, false); return 0;
    case WM_RBUTTONDOWN: onMousePress(ButtonRight, false); return 0;
    case WM_MBUTTONDOWN: onMousePress(ButtonMiddle, false); return 0;

    case WM_LBUTTONUP: onMouseRelease(ButtonLeft); return 0;
    case WM_RBUTTONUP: onMouseRelease(ButtonRight); return 0;
    case WM_MBUTTONUP: onMouseRelease(ButtonMiddle); return 0;

    case WM_MOUSEWHEEL:
      // Typical for synaptics.
      if (uiSystem->_systemMouseStatus.uiWindow != this || 
         !uiSystem->_systemMouseStatus.hover)
      {
        POINT pt;
        DWORD cursorPos = GetMessagePos();
        pt.x = (int)(short)LOWORD(cursorPos);
        pt.y = (int)(short)HIWORD(cursorPos);
        ScreenToClient(hwnd, &pt);
        onMouseHover(pt.x, pt.y);
      }

      if ((int16_t)HIWORD(wParam) < 0)
      {
wheelDown:
        onMouseWheel(WheelDown);
      }
      else
      {
wheelUp:
        onMouseWheel(WheelUp);
      }
      return 0;

    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
    {
      uint32_t modifier = uiSystem->winKeyToModifier(&wParam, lParam);
      bool used = onKeyPress(
        uiSystem->winKeyToFogKey(wParam, HIWORD(lParam)),
        modifier,
        (uint32_t)wParam,
        Char32(uiSystem->winKeyToUnicode(wParam, HIWORD(lParam)))
      );

      if (used) return 0;
      goto defWindowProc;
    }
    case WM_SYSKEYUP:
    case WM_KEYUP:
    {
      uint32_t modifier = uiSystem->winKeyToModifier(&wParam, lParam);
      bool used = onKeyRelease(
        uiSystem->winKeyToFogKey(wParam, HIWORD(lParam)),
        modifier,
        (uint32_t)wParam,
        Char32(uiSystem->winKeyToUnicode(wParam, HIWORD(lParam)))
      );

      if (used) return 0;
      goto defWindowProc;
    }

    case WM_ERASEBKGND:
      return 1;

    case WM_PAINT:
    {
      RECT rect;
      PAINTSTRUCT ps;
      HDC hdc;

      GetClientRect(hwnd, &rect);
      hdc = BeginPaint(hwnd, &ps);

      if (!_dirty)
      {
        BitBlt(
          hdc, 0, 0, rect.right-rect.left, rect.bottom-rect.top, 
          reinterpret_cast<UIBackingStoreWin*>(_backingStore)->hdc(), 0, 0, 
          SRCCOPY);
      }
      else
      {
        _blit = true;
      }

      EndPaint(hwnd, &ps);
      return 0;
    }

    case WM_CLOSE:
    {
      CloseEvent e;
      _widget->sendEvent(&e);
      return 0;
    }

    case WM_DESTROY:
      return 0;

    default:
      if (message == UI_SYSTEM()->uMSH_MOUSEWHEEL)
      {
        if ((int)wParam < 0) 
          goto wheelDown;
        else 
          goto wheelUp;
      }

defWindowProc:
      return DefWindowProc(hwnd, message, wParam, lParam);
  }
}

void UIWindowWin::getWindowRect(Rect* windowRect, Rect* clientRect)
{
  RECT wr;
  RECT cr;

  ::GetWindowRect((HWND)_handle, &wr);
  ::GetClientRect((HWND)_handle, &cr);

  windowRect->set(wr.left, wr.top, wr.right - wr.left, wr.bottom - wr.top);
  clientRect->set(cr.left, cr.top, cr.right - cr.left, cr.bottom - cr.top);
}

// ============================================================================
// [Fog::UIBackingStoreWin]
// ============================================================================

UIBackingStoreWin::UIBackingStoreWin()
{
}

UIBackingStoreWin::~UIBackingStoreWin()
{
  destroy();
}

bool UIBackingStoreWin::resize(int width, int height, bool cache)
{
  int targetWidth = width;
  int targetHeight = height;
  sysint_t targetSize;
  sysint_t targetStride;

  bool destroyImage = false;
  bool createImage = false;

  if (width == 0 || height == 0)
  {
    destroyImage = true;
  }
  else
  {
    if (cache)
    {
      if (width <= _widthOrig && height <= _heightOrig)
      {
        // Cached, here can be debug counter to create
        // statistics about usability of that
        _width = width;
        _height = height;
        return true;
      }

      // Don't create smaller buffer that previous!
      targetWidth  = width;
      targetHeight = height;

      if (targetWidth  < _width)  targetWidth  = _width;
      if (targetHeight < _height) targetHeight = _height;

      targetWidth  = (targetWidth  + 127) & ~127;
      targetHeight = (targetHeight + 127) & ~127;
    }

    destroyImage = true;
    createImage = (targetWidth > 0 && targetHeight > 0);
  }

  // Destroy image buffer
  if (destroyImage)
  {
    switch (_type)
    {
      case TypeNone:
        break;

      case TypeDIB:
        DeleteDC(_hdc);
        DeleteObject(_hBitmap);
        break;
    }
  }

  // Create image buffer.
  if (createImage)
  {
    int targetBPP = 32;

    // Define bitmap attributes.
    BITMAPINFO bmi;

    _hdc = CreateCompatibleDC(NULL);
    if (_hdc == NULL)
    {
      fog_stderr_msg("Fog::UIBackingStoreWin", "resize", "CreateCompatibleDC failed, WinError: %u\n", GetLastError());
      goto fail;
    }

    Fog::Memory::zero(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize        = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth       = (int)targetWidth;
    bmi.bmiHeader.biHeight      = -((int)targetHeight);
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = targetBPP;
    bmi.bmiHeader.biCompression = BI_RGB;

    _pixelsPrimary = NULL;
    _hBitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&_pixelsPrimary, NULL, 0);
    if (_hBitmap == NULL) 
    {
      fog_stderr_msg("Fog::UIBackingStoreWin", "resize", "CreateDIBSection failed, request size %dx%d, WinError: %u\n", targetWidth, targetHeight, GetLastError());
      DeleteDC(_hdc);
      goto fail;
    }

    // Select bitmap to DC.
    SelectObject(_hdc, _hBitmap);

    // We need stride, windows should use 32 bit alignment, but be safe.
    DIBSECTION info;
    GetObjectW(_hBitmap, sizeof(DIBSECTION), &info);
    targetStride = info.dsBm.bmWidthBytes;
    targetSize = targetStride * targetHeight;
    _type = TypeDIB;

    if (_type != TypeNone)
    {
      _created = TimeTicks::now();
      _expires = _created + TimeDelta::fromSeconds(15);

      _format = Image::FormatRGB32;

      _stridePrimary = targetStride;
      _widthOrig = targetWidth;
      _heightOrig = targetHeight;

      // Extra buffer not needed.
      _pixelsSecondary = NULL;
      _strideSecondary = 0;

      _convertFunc = NULL;
      _convertDepth = 0;

      _pixels = _pixelsPrimary;
      _width = width;
      _height = height;
      _stride = _stridePrimary;
      return true;
    }
  }

fail:
  _clear();
  return false;
}

void UIBackingStoreWin::destroy()
{
  resize(0, 0, false);
}

void UIBackingStoreWin::updateRects(const Box* rects, sysuint_t count)
{
}

void UIBackingStoreWin::blitRects(HDC target, const Box* rects, sysuint_t count)
{
  sysuint_t i;

  switch (type())
  {
    case TypeNone:
      break;

    case TypeDIB:
      for (i = 0; i != count; i++)
      {
        int x = rects[i].x1();
        int y = rects[i].y1();
        int w = rects[i].width();
        int h = rects[i].height();
        
        BitBlt(target, x, y, w, h, _hdc, x, y, SRCCOPY);
      }
      break;
  }
}

} // Fog namespace

#endif // FOG_OS_WINDOWS
