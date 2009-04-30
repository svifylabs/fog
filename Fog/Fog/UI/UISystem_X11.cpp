
#include "Fog/Core/TextCodec.h"
#include "UISystem_Def.h"

// [Fog/UI Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

#include <Fog/Build/Build.h>

#if defined(FOG_UI_X11)

// [Dependencies]
#include <Fog/Core/Application.h>
#include <Fog/UI/Constants.h>
#include <Fog/UI/UISystem_X11.h>
#include <Fog/UI/Widget.h>

#include <new>

// [Shared memory and IPC]
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>

FOG_IMPLEMENT_OBJECT(Fog::UISystemX11)
FOG_IMPLEMENT_OBJECT(Fog::UIWindowX11)

// Enabled by default.
#define FOG_UIBACKINGSTORE_FORCE_PIXMAP

namespace Fog {

// ============================================================================
// [Helpers]
// ============================================================================

#define UI_SYSTEM() \
  reinterpret_cast<UISystemX11*>(Application::instance()->uiSystem())

// ============================================================================
// [Fog::UISystemX11 - Error]
// ============================================================================

static int X11_IOErrorHandler(Display* d)
{
  fog_fail("Fog::UISystemX11::IOErrorHandler() - Fatal error");

  // be quite
  return 0;
}

static int X11_ErrorHandler(Display* d, XErrorEvent* e)
{
  char buffer[256];
  UI_SYSTEM()->pXGetErrorText(d, e->error_code, buffer, 256);

  fog_stderr_msg("Fog::UIsystemX11", "ErrorHandler", "%s", buffer);
  return 0;
}

// ============================================================================
// [Fog::UISystemX11 - Functions in libX11, libXext, libXrender]
// ============================================================================

// Function names in libX
static const char X11_xlibFunctionNames[] =
{
  "XSupportsLocale\0"
  "XSetLocaleModifiers\0"
  "XOpenDisplay\0"
  "XCloseDisplay\0"
  "XSetErrorHandler\0"
  "XSetIOErrorHandler\0"
  "XGetErrorText\0"
  "XInternAtom\0"
  "XInternAtoms\0"
  "XRootWindow\0"
  "XDisplayWidth\0"
  "XDisplayHeight\0"
  "XDefaultScreen\0"
  "XDefaultVisual\0"
  "XDefaultColormap\0"
  "XDefaultDepth\0"
  "XPending\0"
  "XNextEvent\0"
  "XCheckTypedEvent\0"
  "XCheckTypedWindowEvent\0"
  "XCheckWindowEvent\0"
  "XOpenIM\0"
  "XCloseIM\0"
  "XCreateIC\0"
  "XDestroyIC\0"
  "XRefreshKeyboardMapping\0"
  "XCreateWindow\0"
  "XDestroyWindow\0"
  "XMoveWindow\0"
  "XResizeWindow\0"
  "XMoveResizeWindow\0"
  "XMapWindow\0"
  "XUnmapWindow\0"
  "XRaiseWindow\0"
  "XLowerWindow\0"
  "XReparentWindow\0"
  "XChangeWindowAttributes\0"
  "XGetWindowAttributes\0"
  "XSetInputFocus\0"
  "XGetInputFocus\0"
  "XSelectInput\0"
  "XSetNormalHints\0"
  "XSetWMProtocols\0"
  "XGetWMProtocols\0"
  "XTranslateCoordinates\0"
  "XwcTextListToTextProperty\0"
  "XFlush\0"
  "XSync\0"
  "XCreateGC\0"
  "XFreeGC\0"
  "XKeycodeToKeysym\0"
  "XLookupString\0"
  "XwcLookupString\0"
  "XGrabPointer\0"
  "XUngrabPointer\0"
  "XGetPointerMapping\0"
  "XCreatePixmap\0"
  "XFreePixmap\0"
  "XCopyArea\0"
  "XFree\0"
  "XCreateImage\0"
  "XDestroyImage\0"
  "XPutImage\0"
  "XCreateColormap\0"
  "XFreeColormap\0"
  "XAllocColor\0"
  "XFreeColors\0"
  "XMatchVisualInfo\0"
  "XSetWMName\0"
  "XSendEvent\0"
  "XChangeProperty\0"
  "XSetSelectionOwner\0"
  "XGetAtomName\0"
  "XConvertSelection\0"
  "XGetWindowProperty\0"
  "XDeleteProperty\0"
};

// Function names in libXext
static const char X11_xextFunctionNames[] =
{
  "XShmQueryVersion\0"
  "XShmAttach\0"
  "XShmDetach\0"
  "XShmCreateImage\0"
  "XShmCreatePixmap\0"
};

//Function names in libXrender
static const char X11_xrenderFunctionNames[] =
{
  "XRenderQueryVersion\0"
  "XRenderQueryFormats\0"
  "XRenderQuerySubpixelOrder\0"
  "XRenderSetSubpixelOrder\0"
  "XRenderFindVisualFormat\0"
  "XRenderFindFormat\0"
  "XRenderFindStandardFormat\0"
  "XRenderCreatePicture\0"
  "XRenderChangePicture\0"
  "XRenderSetPictureTransform\0"
  "XRenderFreePicture\0"
  "XRenderComposite\0"
  "XRenderCreateCursor\0"
};

// ============================================================================
// [Fog::UISystemX11 - Atoms]
// ============================================================================

// XAtom names, order specified in Wde/Gui/Application.h
static const char *X11_atomNames[UISystemX11::Atom_Count] =
{
  // WM events
  "WM_PROTOCOLS",                 /* 0 */
  "WM_DELETE_WINDOW",             /* 1 */
  "WM_TAKE_FOCUS",                /* 2 */
  "WM_CLASS",                     /* 3 */
  "WM_NAME",                      /* 4 */
  "WM_COMMAND",                   /* 5 */
  "WM_ICON_NAME",                 /* 6 */
  "WM_CLIENT_MACHINE",            /* 7 */
  "WM_CHANGE_STATE",              /* 8 */

  // motif WM hints
  "_MOTIF_WM_HINTS",              /* 9 */

  "_WIN_LAYER",                   /* 10 */

  // Drag and drop atoms
  "XdndSelection",                /* 11 */
  "XdndAware",                    /* 12 */
  "XdndTypeList",                 /* 13 */
  "XdndEnter",                    /* 14 */
  "XdndPosition",                 /* 15 */
  "XdndActionCopy",               /* 16 */
  "XdndActionMove",               /* 17 */
  "XdndActionPrivate",            /* 18 */
  "XdndActionAsk",                /* 19 */
  "XdndActionList",               /* 20 */
  "XdndActionLink",               /* 21 */
  "XdndActionDescription",        /* 22 */
  "XdndProxy",                    /* 23 */
  "XdndStatus",                   /* 24 */
  "XdndLeave",                    /* 25 */
  "XdndDrop",                     /* 26 */
  "XdndFinished",                 /* 27 */

  // standartized NET atoms
  "_NET_SUPPORTED",               /* 28 */
  "_NET_SUPPORTING_WM_CHECK",     /* 29 */

  "_NET_NUMBER_OF_DESKTOPS",      /* 30 */
  "_NET_DESKTOP_GEOMETRY",        /* 31 */
  "_NET_DESKTOP_NAMES",           /* 32 */
  "_NET_CURRENT_DESKTOP",         /* 33 */
  "_NET_DESKTOP_VIEWPORT",        /* 34 */
  "_NET_WORKAREA",                /* 35 */
  "_NET_VIRTUAL_ROOTS",           /* 36 */

  "_NET_CLIENT_LIST",             /* 37 */
  "_NET_CLIENT_LIST_STACKING",    /* 38 */
  "_NET_ACTIVE_WINDOW",           /* 39 */
  "_NET_CLOSE_WINDOW",            /* 40 */

  "_NET_WM_MOVERESIZE",           /* 41 */
  "_NET_WM_NAME",                 /* 42 */
  "_NET_WM_VISIBLE_NAME",         /* 43 */
  "_NET_WM_ICON_NAME",            /* 44 */
  "_NET_WM_VISIBLE_ICON_NAME",    /* 45 */
  "_NET_WM_DESKTOP",              /* 46 */
  "_NET_WM_WINDOW_TYPE",          /* 47 */
  "_NET_WM_STATE",                /* 48 */
  "_NET_WM_ALLOWED_ACTIONS",      /* 49 */
  "_NET_WM_STRUT",                /* 50 */
  "_NET_WM_STRUT_PARTIAL",        /* 51 */
  "_NET_WM_ICON_GEOMETRY",        /* 52 */
  "_NET_WM_ICON",                 /* 53 */
  "_NET_WM_PID",                  /* 54 */
  "_NET_WM_USER_TIME",            /* 55 */

  "_NET_WM_WINDOW_TYPE_DESKTOP",  /* 56 */
  "_NET_WM_WINDOW_TYPE_DOCK",     /* 57 */
  "_NET_WM_WINDOW_TYPE_TOOLBAR",  /* 58 */
  "_NET_WM_WINDOW_TYPE_MENU",     /* 59 */
  "_NET_WM_WINDOW_TYPE_UTILITY",  /* 60 */
  "_NET_WM_WINDOW_TYPE_SPLASH",   /* 61 */
  "_NET_WM_WINDOW_TYPE_DIALOG",   /* 62 */
  "_NET_WM_WINDOW_TYPE_NORMAL",   /* 63 */

  "_NET_WM_STATE_MODAL",          /* 64 */
  "_NET_WM_STATE_STICKY",         /* 65 */
  "_NET_WM_STATE_MAXIMIZED_VERT", /* 66 */
  "_NET_WM_STATE_MAXIMIZED_HORZ", /* 67 */
  "_NET_WM_STATE_SHADED",         /* 68 */
  "_NET_WM_STATE_SKIP_TASKBAR",   /* 69 */
  "_NET_WM_STATE_SKIP_PAGER",     /* 70 */
  "_NET_WM_STATE_HIDDEN",         /* 71 */
  "_NET_WM_STATE_FULLSCREEN",     /* 72 */
  "_NET_WM_STATE_ABOVE",          /* 73 */
  "_NET_WM_STATE_BELOW",          /* 74 */
  "_NET_WM_WINDOW_OPACITY",       /* 75 */

  // Clipboard
  "CLIPBOARD",                    /* 76 */
  "TARGETS",                      /* 77 */
  "COMPOUND_TEXT",                /* 78 */
  "UTF8_STRING",                  /* 79 */
  "FILE_NAME",                    /* 80 */
  "STRING",                       /* 81 */
  "TEXT",                         /* 82 */
  "INCR"
};

// ============================================================================
// [Fog::UISystemX11 - Construction / Destruction]
// ============================================================================

UISystemX11::UISystemX11()
{
  err_t err;

  // Clear all members
  Memory::zero(&_atoms, sizeof(_atoms));
  Memory::zero(&_xKeymap, sizeof(_xKeymap));

  _display = NULL;
  _fd = -1;
  _screen = -1;
  _visual = NULL;
  _colormap = 0;
  _root = 0;
  _gc = 0;
  _xShm = false;
  _xPrivateColormap = false;
  _xim = false;
  _xButtons = 0;

  // Open X11, Xext and Xrender libraries.
  if ( (err = loadLibraries()) )
  {
    return;
  }

  // X locale support.
  if (!pXSupportsLocale())
  {
    fog_stderr_msg("Fog::UISystemX11", "UISystemX11", "X does not support locale");
  }
  else {
    char* localeModifiers;
    if ( !(localeModifiers = pXSetLocaleModifiers("")) )
    {
      fog_stderr_msg("Fog::UISystemX11", "UISystemX11", "Can't set X locale modifiers");
    }
  }

  if ((_display = pXOpenDisplay("")) == NULL)
  {
    err = Error::UISystemX11_CantOpenDisplay;
    return;
  }

  // Set error handlers.
  pXSetIOErrorHandler(&X11_IOErrorHandler);
  pXSetErrorHandler(&X11_ErrorHandler);

  // Setup X11 variables.
  _fd               = ConnectionNumber (display());
  _screen           = pXDefaultScreen  (display());
  _visual           = pXDefaultVisual  (display(), screen());
  _colormap         = pXDefaultColormap(display(), screen());
  _root             = pXRootWindow     (display(), screen());
  _gc               = pXCreateGC       (display(), root(), 0, NULL);
  _xPrivateColormap = false;

  // Now we can setup display info. We must do it here, because we must setup
  // colormap if depth is too low (8 bit or less)
  updateDisplayInfo();

  // Intern atoms.
  pXInternAtoms(display(), (char **)X11_atomNames, Atom_Count, False, _atoms);

  // Alloc colormap for 4, 8 bit depth
  if (_displayInfo.depth <= 8 && !createColormap())
  {
    err = Error::UISystemX11_CantCreateColormap;
    goto fail;
  }

  // Open XIM.
  _xim = pXOpenIM(display(), NULL, NULL, NULL);

  // Are SHM pixmaps supported?
  {
    int major, minor;
    XBool pixmaps;

    pXShmQueryVersion(display(), &major, &minor, &pixmaps);
    _xShm = pixmaps;
  }

  initKeyboard();
  initMouse();

  // Finally add the event loop type into application. Event loop will be
  // instantiated by application after UISystem was properly constructed.
  Application::addEventLoopType<Fog::EventLoopX11>(Fog::StubAscii8("UI::X11"));

  _initialized = true;
  return;

fail:
  pXCloseDisplay(display());
}

UISystemX11::~UISystemX11()
{
  // We don't want that event loop is available after UISystemX11 was destroyed.
  Application::removeEventLoopType(Fog::StubAscii8("UI::X11"));

  // Close display and free X resources.
  if (display())
  {
    if (xim()) pXCloseIM(xim());
    pXFreeGC(display(), gc());
    if (xPrivateColormap()) pXFreeColormap(display(), colormap());

    pXCloseDisplay(display());
  }
}

// ============================================================================
// [Fog::UISystemX11 - Display]
// ============================================================================

void UISystemX11::updateDisplayInfo()
{
  _displayInfo.depth = pXDefaultDepth(display(), screen());
  _displayInfo.rMask = visual()->red_mask;
  _displayInfo.gMask = visual()->green_mask;
  _displayInfo.bMask = visual()->blue_mask;
  _displayInfo.is16BitSwapped = false;
}

// ============================================================================
// [Fog::UISystemX11 - Update]
// ============================================================================

void UISystemX11::doBlitWindow(UIWindow* window, const Box* rects, sysuint_t count)
{
  reinterpret_cast<UIBackingStoreX11*>(window->_backingStore)->blitRects(
    (XID)window->handle(), rects, count);
}

// ============================================================================
// [Fog::UISystemX11 - X11 Keyboard]
// ============================================================================

void UISystemX11::initKeyboard()
{
  // Initialize key translation tables.
  Memory::zero(&_xKeymap, sizeof(_xKeymap));

  // 0xFE
  _xKeymap.odd[32] = KeyTab; // TAB

   _xKeymap.odd[XK_dead_grave            & 0xFF] = KeyCompose;
   _xKeymap.odd[XK_dead_acute            & 0xFF] = KeyCompose;
   _xKeymap.odd[XK_dead_tilde            & 0xFF] = KeyCompose;
   _xKeymap.odd[XK_dead_macron           & 0xFF] = KeyCompose;
   _xKeymap.odd[XK_dead_breve            & 0xFF] = KeyCompose;
   _xKeymap.odd[XK_dead_abovedot         & 0xFF] = KeyCompose;
   _xKeymap.odd[XK_dead_diaeresis        & 0xFF] = KeyCompose;
   _xKeymap.odd[XK_dead_abovering        & 0xFF] = KeyCompose;
   _xKeymap.odd[XK_dead_doubleacute      & 0xFF] = KeyCompose;
   _xKeymap.odd[XK_dead_caron            & 0xFF] = KeyCompose;
   _xKeymap.odd[XK_dead_cedilla          & 0xFF] = KeyCompose;
   _xKeymap.odd[XK_dead_ogonek           & 0xFF] = KeyCompose;
   _xKeymap.odd[XK_dead_iota             & 0xFF] = KeyCompose;
   _xKeymap.odd[XK_dead_voiced_sound     & 0xFF] = KeyCompose;
   _xKeymap.odd[XK_dead_semivoiced_sound & 0xFF] = KeyCompose;
   _xKeymap.odd[XK_dead_belowdot         & 0xFF] = KeyCompose;
#ifdef XK_dead_hook
   _xKeymap.odd[XK_dead_hook             & 0xFF] = KeyCompose;
#endif
#ifdef XK_dead_horn
   _xKeymap.odd[XK_dead_horn             & 0xFF] = KeyCompose;
#endif

#ifdef XK_dead_circumflex
  _xKeymap.odd[XK_dead_circumflex       & 0xFF] = KeyCaret;
#endif

#ifdef XK_ISO_Level3_Shift
  // _xKeymap.odd[XK_ISO_Level3_Shift   & 0xFF] = KeyMove;
#endif

  // 0xFF
  _xKeymap.misc[XK_BackSpace            & 0xFF] = KeyBackspace;
  _xKeymap.misc[XK_Tab                  & 0xFF] = KeyTab;
  _xKeymap.misc[XK_Clear                & 0xFF] = KeyClear;
  _xKeymap.misc[XK_Return               & 0xFF] = KeyEnter;
  _xKeymap.misc[XK_Pause                & 0xFF] = KeyPause;
  _xKeymap.misc[XK_Escape               & 0xFF] = KeyEsc;
  _xKeymap.misc[XK_Delete               & 0xFF] = KeyDelete;

  _xKeymap.misc[XK_KP_0                 & 0xFF] = KeyKP0;
  _xKeymap.misc[XK_KP_1                 & 0xFF] = KeyKP1;
  _xKeymap.misc[XK_KP_2                 & 0xFF] = KeyKP2;
  _xKeymap.misc[XK_KP_3                 & 0xFF] = KeyKP3;
  _xKeymap.misc[XK_KP_4                 & 0xFF] = KeyKP4;
  _xKeymap.misc[XK_KP_5                 & 0xFF] = KeyKP5;
  _xKeymap.misc[XK_KP_6                 & 0xFF] = KeyKP6;
  _xKeymap.misc[XK_KP_7                 & 0xFF] = KeyKP7;
  _xKeymap.misc[XK_KP_8                 & 0xFF] = KeyKP8;
  _xKeymap.misc[XK_KP_9                 & 0xFF] = KeyKP9;
  _xKeymap.misc[XK_KP_Insert            & 0xFF] = KeyKP0;
  _xKeymap.misc[XK_KP_End               & 0xFF] = KeyKP1;
  _xKeymap.misc[XK_KP_Down              & 0xFF] = KeyKP2;
  _xKeymap.misc[XK_KP_Page_Down         & 0xFF] = KeyKP3;
  _xKeymap.misc[XK_KP_Left              & 0xFF] = KeyKP4;
  _xKeymap.misc[XK_KP_Begin             & 0xFF] = KeyKP5;
  _xKeymap.misc[XK_KP_Right             & 0xFF] = KeyKP6;
  _xKeymap.misc[XK_KP_Home              & 0xFF] = KeyKP7;
  _xKeymap.misc[XK_KP_Up                & 0xFF] = KeyKP8;
  _xKeymap.misc[XK_KP_Page_Up           & 0xFF] = KeyKP9;
  _xKeymap.misc[XK_KP_Delete            & 0xFF] = KeyKPPeriod;
  _xKeymap.misc[XK_KP_Decimal           & 0xFF] = KeyKPPeriod;
  _xKeymap.misc[XK_KP_Divide            & 0xFF] = KeyKPDivide;
  _xKeymap.misc[XK_KP_Multiply          & 0xFF] = KeyKPMultiply;
  _xKeymap.misc[XK_KP_Subtract          & 0xFF] = KeyKPMinus;
  _xKeymap.misc[XK_KP_Add               & 0xFF] = KeyKPPlus;
  _xKeymap.misc[XK_KP_Enter             & 0xFF] = KeyKPEnter;
  _xKeymap.misc[XK_KP_Equal             & 0xFF] = KeyKPEquals;

  _xKeymap.misc[XK_Up                   & 0xFF] = KeyUp;
  _xKeymap.misc[XK_Down                 & 0xFF] = KeyDown;
  _xKeymap.misc[XK_Right                & 0xFF] = KeyRight;
  _xKeymap.misc[XK_Left                 & 0xFF] = KeyLeft;
  _xKeymap.misc[XK_Insert               & 0xFF] = KeyInsert;
  _xKeymap.misc[XK_Home                 & 0xFF] = KeyHome;
  _xKeymap.misc[XK_End                  & 0xFF] = KeyEnd;
  _xKeymap.misc[XK_Page_Up              & 0xFF] = KeyPageUp;
  _xKeymap.misc[XK_Page_Down            & 0xFF] = KeyPageDown;

  _xKeymap.misc[XK_F1                   & 0xFF] = KeyF1;
  _xKeymap.misc[XK_F2                   & 0xFF] = KeyF2;
  _xKeymap.misc[XK_F3                   & 0xFF] = KeyF3;
  _xKeymap.misc[XK_F4                   & 0xFF] = KeyF4;
  _xKeymap.misc[XK_F5                   & 0xFF] = KeyF5;
  _xKeymap.misc[XK_F6                   & 0xFF] = KeyF6;
  _xKeymap.misc[XK_F7                   & 0xFF] = KeyF7;
  _xKeymap.misc[XK_F8                   & 0xFF] = KeyF8;
  _xKeymap.misc[XK_F9                   & 0xFF] = KeyF9;
  _xKeymap.misc[XK_F10                  & 0xFF] = KeyF10;
  _xKeymap.misc[XK_F11                  & 0xFF] = KeyF11;
  _xKeymap.misc[XK_F12                  & 0xFF] = KeyF12;
  _xKeymap.misc[XK_F13                  & 0xFF] = KeyF13;
  _xKeymap.misc[XK_F14                  & 0xFF] = KeyF14;
  _xKeymap.misc[XK_F15                  & 0xFF] = KeyF15;

  _xKeymap.misc[XK_Num_Lock             & 0xFF] = KeyNumLock;
  _xKeymap.misc[XK_Caps_Lock            & 0xFF] = KeyCapsLock;
  _xKeymap.misc[XK_Scroll_Lock          & 0xFF] = KeyScrollLock;
  _xKeymap.misc[XK_Shift_L              & 0xFF] = KeyLeftShift;
  _xKeymap.misc[XK_Shift_R              & 0xFF] = KeyRightShift;
  _xKeymap.misc[XK_Control_L            & 0xFF] = KeyLeftCtrl;
  _xKeymap.misc[XK_Control_R            & 0xFF] = KeyRightCtrl;
  _xKeymap.misc[XK_Alt_L                & 0xFF] = KeyLeftAlt;
  _xKeymap.misc[XK_Alt_R                & 0xFF] = KeyRightAlt;
  _xKeymap.misc[XK_Meta_L               & 0xFF] = KeyLeftMeta;
  _xKeymap.misc[XK_Meta_R               & 0xFF] = KeyRightMeta;
  _xKeymap.misc[XK_Super_L              & 0xFF] = KeyLeftSuper;
  _xKeymap.misc[XK_Super_R              & 0xFF] = KeyRightSuper;
  _xKeymap.misc[XK_Mode_switch          & 0xFF] = KeyMode;
  _xKeymap.misc[XK_Multi_key            & 0xFF] = KeyCompose;

  _xKeymap.misc[XK_Help                 & 0xFF] = KeyHelp;
  _xKeymap.misc[XK_Print                & 0xFF] = KeyPrint;
  _xKeymap.misc[XK_Sys_Req              & 0xFF] = KeySys_Req;
  _xKeymap.misc[XK_Break                & 0xFF] = KeyBreak;
  _xKeymap.misc[XK_Menu                 & 0xFF] = KeyMenu;
  _xKeymap.misc[XK_Hyper_R              & 0xFF] = KeyMenu;
}

uint32_t UISystemX11::translateXSym(KeySym xsym) const
{
  uint key = 0;

  if (xsym == 0) return 0;

  switch (xsym >> 8)
  {
    case 0x1005FF:
      key = 0;
#ifdef SunXK_F36
      if (xsym == SunXK_F36) key = KeyF11;
#endif
#ifdef SunXK_F37
      if (xsym == SunXK_F37) key = KeyF12;
#endif
      break;

    case 0x00: /* Latin 1 */
    case 0x01: /* Latin 2 */
    case 0x02: /* Latin 3 */
    case 0x03: /* Latin 4 */
    case 0x04: /* Katakana */
    case 0x05: /* Arabic */
    case 0x06: /* Cyrillic */
    case 0x07: /* Greek */
    case 0x08: /* Technical */
    case 0x0A: /* Publishing */
    case 0x0C: /* Hebrew */
    case 0x0D: /* Thai */
      key = Char8((uint8_t)(xsym & 0xFF)).toLower().ch();
      break;
    case 0xFE:
      key = _xKeymap.odd[xsym & 0xFF];
      break;
    case 0xFF:
      key = _xKeymap.misc[xsym & 0xFF];
      break;
    default: /* Unknown key */
      break;
  }

  return key;
}

// ============================================================================
// [Fog::UISystemX11 - X11 Mouse]
// ============================================================================

void UISystemX11::initMouse()
{
  // Get count of mouse buttons.
  uint8_t pm[5];
  _xButtons = pXGetPointerMapping(display(), pm, 5);
}

uint32_t UISystemX11::translateButton(uint x11Button) const
{
  switch (x11Button)
  {
    case 1:
      return ButtonLeft;
    case 2:
      if (_xButtons < 3)
        return ButtonRight;
      else
        return ButtonMiddle;
    case 3:
      return ButtonRight;
    case 4:
      return WheelUp;
    case 5:
      return WheelDown;
    default:
      return 0;
  }
}


// ============================================================================
// [Fog::UISystemX11 - X11 ColorMap]
// ============================================================================

bool UISystemX11::createColormap()
{
  // Synchronize supported parameters with 'PixelConvert'
  struct __PseudoParams
  {
    uint32_t redColors, greenColors, blueColors;
    uint32_t redMask, greenMask, blueMask;
  };

  __PseudoParams p[] =
  {
    { 4, 8, 4, 0x60, 0x1C, 0x03 },
    { 4, 4, 4, 0x30, 0x0C, 0x03 },
    { 2, 2, 2, 0x04, 0x02, 0x01 }
  };

  uint i, v;

  // First try to match some visuals, because some X modes can't
  // alloc to alloc requester numbers of colors...
  XVisualInfo vi;
  XVisual* visuals[4];
  XVisual** visualPtr = visuals;
  uint visualsCount;

  // Default or created colormap, see loop
  XColormap colormapToUse;

  // Default visual is the most wanted...
  *visualPtr++ = visual();

  // Match some interesting visuals
  if (pXMatchVisualInfo(display(), screen(), _displayInfo.depth, DirectColor, &vi))
  {
    *visualPtr++ = vi.visual;
  }

  if (pXMatchVisualInfo(display(), screen(), _displayInfo.depth, PseudoColor, &vi))
  {
    *visualPtr++ = vi.visual;
  }

  visualsCount = (uint)(visualPtr - visuals);

  for (v = 0; v != visualsCount; v++)
  {
    if (v == 0)
    {
      colormapToUse = colormap();
    }
    else
    {
      colormapToUse = pXCreateColormap(display(), root(), visuals[v], AllocNone);
      if (!colormapToUse) return false;
    }

    for (i = 0; i != FOG_ARRAY_SIZE(p); i++)
    {
      _paletteInfo.palCount = allocRGB(
        colormapToUse,
        p[i].redColors,
        p[i].greenColors,
        p[i].blueColors,
        _paletteInfo.palConv);

      if (_paletteInfo.palCount > 0)
      {
        _displayInfo.rMask = p[i].redMask;
        _displayInfo.gMask = p[i].greenMask;
        _displayInfo.bMask = p[i].blueMask;

        if (v != 0)
        {
          _visual = visuals[v];
          _colormap = colormapToUse;
          _xPrivateColormap = true;
        }
        return true;
      }

    }

    if (v != 0)
    {
      pXFreeColormap(display(), colormapToUse);
    }
  }

  return false;
}

uint UISystemX11::allocRGB(XColormap colormap, int nr, int ng, int nb, uint8_t* palconv)
{
  int r, g, b, i;
  int sig_mask = 0;

  for (i = 0; i < visual()->bits_per_rgb; i++)
  {
    sig_mask |= (0x1 << i);
  }
  sig_mask <<= (16 - visual()->bits_per_rgb);

  i = 0;
  for (r = 0; r < nr; r++)
  {
    for (g = 0; g < ng; g++)
    {
      for (b = 0; b < nb; b++)
      {
        XColor xcl;
        XColor xcl_in;
        int val;
        XStatus ret;

        val = (int)((((double)r) / ((nr) - 1)) * 65535.0);
        xcl.red = (unsigned short)(val);
        val = (int)((((double)g) / ((ng) - 1)) * 65535.0);
        xcl.green = (unsigned short)(val);
        val = (int)((((double)b) / ((nb) - 1)) * 65535.0);
        xcl.blue = (unsigned short)(val);
        xcl_in = xcl;

        ret = pXAllocColor(display(), colormap, &xcl);

        if ((ret == 0) ||
          ((xcl_in.red & sig_mask) != (xcl.red & sig_mask)) ||
          ((xcl_in.green & sig_mask) != (xcl.green & sig_mask)) ||
          ((xcl_in.blue & sig_mask) != (xcl.blue & sig_mask)))
        {
          freeRGB(colormap, palconv, i);
          return 0;
        }

        palconv[i++] = xcl.pixel;
      }
    }
  }

  return i;
}

void UISystemX11::freeRGB(XColormap colormap, const uint8_t* palconv, uint count)
{
  if (count == 0) return;

  ulong pixels[count];
  for (uint i = 0; i < count; i++) pixels[i] = (ulong)palconv[i];

  pXFreeColors(display(), colormap, pixels, count, 0);
}

// ============================================================================
// [Fog::UISystemX11 - UIWindow]
// ============================================================================

UIWindow* UISystemX11::createUIWindow(Widget* widget)
{
  return new UIWindowX11(widget);
}

void UISystemX11::destroyUIWindow(UIWindow* native)
{
  delete native;
}

// ============================================================================
// [Fog::UISystemX11 - X11 API]
// ============================================================================

err_t UISystemX11::loadLibraries()
{
  // Load X11
  if (_xlib.open(StubAscii8("X11")) != Error::Ok)
    return Error::UISystemX11_CantLoadX11;

  if (_xlib.symbols(xlibFunctions,
    X11_xlibFunctionNames, FOG_ARRAY_SIZE(X11_xlibFunctionNames),
    XlibFunctionsCount,
    (char**)NULL) != XlibFunctionsCount)
  {
    return Error::UISystemX11_CantLoadX11Symbol;
  }

  // Load Xext
  if (_xext.open(StubAscii8("Xext")) != Error::Ok)
    return Error::UISystemX11_CantLoadXext;

  if (_xext.symbols(xextFunctions,
    X11_xextFunctionNames, FOG_ARRAY_SIZE(X11_xextFunctionNames),
    XextFunctionsCount,
    (char**)NULL) != XextFunctionsCount)
  {
    return Error::UISystemX11_CantLoadXextSymbol;
  }

  // Load Xrender
  if (_xrender.open(StubAscii8("Xrender")) != Error::Ok)
    return Error::UISystemX11_CantLoadXrender;

  if (_xrender.symbols(xrenderFunctions,
    X11_xrenderFunctionNames, FOG_ARRAY_SIZE(X11_xrenderFunctionNames),
    XrenderFunctionsCount,
    (char**)NULL) != XrenderFunctionsCount)
  {
    return Error::UISystemX11_CantLoadXrenderSymbol;
  }

  return Error::Ok;
}

// ============================================================================
// [Fog::UIWindowX11]
// ============================================================================

UIWindowX11::UIWindowX11(Widget* widget) :
  UIWindowDefault(widget),
  _xic(0),
  _inputOnly(false),
  _mapRequest(false),
  _xflags(0)
{
  _backingStore = new UIBackingStoreX11();
}

UIWindowX11::~UIWindowX11()
{
  destroy();
  delete _backingStore;
}

err_t UIWindowX11::create(uint32_t createFlags)
{
  if (_handle) return Error::UIWindowAlreadyExists;

  UISystemX11* uiSystem = UI_SYSTEM();

  Display* display = uiSystem->display();
  Atom* atoms = uiSystem->atoms();

  int x = _widget->x1();
  int y = _widget->y1();
  int w = _widget->width();
  int h = _widget->height();

  if (w == 0) w++;
  if (h == 0) h++;

  XSetWindowAttributes attr;

  if ((createFlags & UIWindow::X11OnlyPropertyChangeMask) == 0)
  {
    ulong attr_mask;

    attr.backing_store = NotUseful;
    attr.override_redirect = (createFlags & UIWindow::X11OverrideRedirect) != 0;
    attr.colormap = uiSystem->colormap();
    attr.border_pixel = 0;
    attr.background_pixel = XNone;
    attr.save_under = (createFlags & UIWindow::CreatePopup) != 0;
    attr.event_mask =
      StructureNotifyMask      | ButtonPressMask    |
      ButtonReleaseMask        | PointerMotionMask  |
      EnterWindowMask          | LeaveWindowMask    |
      KeyPressMask             | KeyReleaseMask     |
      ButtonMotionMask         | ExposureMask       |
      FocusChangeMask          | PropertyChangeMask |
      VisibilityChangeMask;
    attr_mask =
      CWOverrideRedirect       |
      CWSaveUnder              |
      CWBackingStore           |
      CWColormap               |
      CWBorderPixel            |
      CWEventMask;

    _handle = (void*)uiSystem->pXCreateWindow(display,
      uiSystem->root(),
      x, y, w, h,
      0, uiSystem->_displayInfo.depth,
      InputOutput,
      uiSystem->visual(),
      attr_mask,
      &attr);
    _inputOnly = false;
  }
  else
  {
    _handle = (void*)uiSystem->pXCreateWindow(display,
      uiSystem->root(),
      x, y, w, h,
      0, CopyFromParent, InputOnly, CopyFromParent, 0, &attr);

    uiSystem->pXSelectInput(display,
      (XID)_handle,
      PropertyChangeMask);
    _inputOnly = true;
  }

  // Create XID <-> UIWindow* connection.
  uiSystem->mapHandle(_handle, this);

  // Atom - _NET_WM_PID
  long pid = getpid();
  uiSystem->pXChangeProperty(display, (XID)handle(), atoms[UISystemX11::Atom_NET_WM_PID], XA_CARDINAL, 32, PropModeReplace, (uchar*)&pid, 1);

  // Atom - WM_DELETE_WINDOW
  uiSystem->pXSetWMProtocols(display, (XID)handle(), &atoms[UISystemX11::Atom_WM_DELETE_WINDOW], 1);

  // Get correct window position
  {
    XWindowAttributes wa;
    uiSystem->pXGetWindowAttributes(display, (XID)handle(), &wa);

    _windowRect.set(wa.x, wa.y, (int)wa.width, (int)wa.height);
  }

  // create X input context
  if (uiSystem->xim())
  {
    _xic = uiSystem->pXCreateIC(uiSystem->xim(),
      XNInputStyle, (XIMPreeditNothing | XIMStatusNothing),
      XNClientWindow, (XID)handle(),
      XNFocusWindow, (XID)handle(),
      NULL);
  }

  // Windows are enabled by default.
  _enabled = true;
  // Windows are not visible by default.
  _visible = false;
  // Need to blit window content.
  _blit = true;
  // Default focus is no focus, X11 will inform us if this gets changed.
  _focus = false;

  // Clear flags, we will wait for events to set them.
  _mapRequest = false;
  _xflags = 0;

  return Error::Ok;
}

err_t UIWindowX11::destroy()
{
  if (!_handle) return Error::InvalidHandle;

  UISystemX11* uiSystem = UI_SYSTEM();

  uiSystem->pXDestroyIC(_xic);
  uiSystem->pXDestroyWindow(uiSystem->display(), (XID)handle());

  // Destroy XID <-> UIWindow* connection.
  uiSystem->unmapHandle(_handle);

  // Clear all variables.
  _handle = NULL;
  _focus = false;
  _blit = false;
  _mapRequest = false;

  // Flags not needed to save.
  _xflags = 0;

  return Error::Ok;
}

err_t UIWindowX11::enable()
{
  if (!_handle) return Error::InvalidHandle;

  // TODO:
  return Error::Ok;
}

err_t UIWindowX11::disable()
{
  if (!_handle) return Error::InvalidHandle;

  // TODO:
  return Error::Ok;
}

err_t UIWindowX11::show()
{
  if (!_handle) return Error::InvalidHandle;

  UISystemX11* uiSystem = UI_SYSTEM();
  uiSystem->pXMapWindow(uiSystem->display(), (XID)handle());

  return Error::Ok;
}

err_t UIWindowX11::hide()
{
  if (!_handle) return Error::InvalidHandle;

  UISystemX11* uiSystem = UI_SYSTEM();
  uiSystem->pXUnmapWindow(uiSystem->display(), (XID)handle());

  return Error::Ok;
}

err_t UIWindowX11::move(const Point& pt)
{
  if (!_handle) return Error::InvalidHandle;

  if (_windowRect.x1() != pt.x() || _windowRect.y1() != pt.y())
  {
    UISystemX11* uiSystem = UI_SYSTEM();

    // some window managers can change widget move request
    if ((_xflags & XFlag_Configured) == 0)
      setMoveableHints();

    uiSystem->pXMoveWindow(uiSystem->display(), (XID)handle(), pt.x(), pt.y());
  }

  return Error::Ok;
}

err_t UIWindowX11::resize(const Size& size)
{
  if (!_handle) return Error::InvalidHandle;
  if (size.width() <= 0 || size.height() <= 0) return Error::InvalidArgument;

  if (_windowRect.size() != size)
  {
    UISystemX11* uiSystem = UI_SYSTEM();
    uiSystem->pXResizeWindow(uiSystem->display(), (XID)handle(),
      (uint)size.width(), (uint)size.height());
  }

  return Error::Ok;
}

err_t UIWindowX11::reconfigure(const Rect& rect)
{
  if (!_handle) return Error::InvalidHandle;
  if (!rect.isValid()) return Error::InvalidArgument;

  if (_windowRect != rect)
  {
    // some window managers can change widget move request
    if ((_xflags & XFlag_Configured) == 0)
      setMoveableHints();

    UISystemX11* uiSystem = UI_SYSTEM();
    uiSystem->pXMoveResizeWindow(uiSystem->display(), (XID)handle(),
      rect.x1(),
      rect.y1(),
      rect.width(),
      rect.height());
  }

  return Error::Ok;
}

err_t UIWindowX11::takeFocus()
{
  if (!_handle) return Error::InvalidHandle;

  UISystemX11* uiSystem = UI_SYSTEM();
  uiSystem->pXSetInputFocus(uiSystem->display(), (XID)handle(), XNone, 0);

  return Error::Ok;
}

err_t UIWindowX11::setTitle(const String32& title)
{
  if (!_handle) return Error::InvalidHandle;

  err_t err = Error::Ok;

  XTextProperty windowProperty;

#if FOG_SIZEOF_WCHAR_T == 2
  TemporaryString16<TemporaryLength> titleW;
  if ((err = titleW.set(title))) return err;
  const wchar_t *titleWChar = (const wchar_t *)titleW.cStr();
#else
  const wchar_t *titleWChar = (const wchar_t *)title.cStr();
#endif
  UISystemX11* uiSystem = UI_SYSTEM();
  int result = uiSystem->pXwcTextListToTextProperty(uiSystem->display(),
    (wchar_t **)&titleWChar, 1, XTextStyle, &windowProperty);

  if (result == XSuccess)
  {
    uiSystem->pXSetWMName(uiSystem->display(), (XID)handle(), &windowProperty);
    uiSystem->pXFree(windowProperty.value);
    uiSystem->pXSync(uiSystem->display(), False);

    _title = title;
  }
  else
  {
    err = Error::UISystemX11_TextListToTextPropertyFailed;
  }

  return err;
}

err_t UIWindowX11::getTitle(String32& title)
{
  if (!_handle) return Error::InvalidHandle;

  title = _title;
  return Error::Ok;
}

err_t UIWindowX11::setIcon(const Image& icon)
{
  if (!_handle) return Error::InvalidHandle;

}

err_t UIWindowX11::getIcon(Image& icon)
{
  if (!_handle) return Error::InvalidHandle;

}

err_t UIWindowX11::setSizeGranularity(const Point& pt)
{
  if (!_handle) return Error::InvalidHandle;

  XSizeHints hints;
  Memory::zero(&hints, sizeof(XSizeHints));
  hints.flags = PResizeInc;
  hints.width_inc = pt.x();
  hints.height_inc = pt.y();

  _sizeGranularity = pt;

  UISystemX11* uiSystem = UI_SYSTEM();
  uiSystem->pXSetNormalHints(uiSystem->display(), (XID)handle(), &hints);

  return Error::Ok;
}

err_t UIWindowX11::getSizeGranularity(Point& pt)
{
  if (!_handle) return Error::InvalidHandle;

  pt = _sizeGranularity;
  return Error::Ok;
}

err_t UIWindowX11::worldToClient(Point* coords)
{
  if (!_handle) return Error::InvalidHandle;

  XWindow childRet;
  UISystemX11* uiSystem = UI_SYSTEM();
  bool ok = uiSystem->pXTranslateCoordinates(uiSystem->display(),
    uiSystem->root(), (XID)handle(),
    coords->x(), coords->y(),
    &coords->_x, &coords->_y,
    &childRet);

  return (ok) ? (err_t)Error::Ok : (err_t)Error::FailedToTranslateCoordinates;
}

err_t UIWindowX11::clientToWorld(Point* coords)
{
  if (!_handle) return Error::InvalidHandle;

  XWindow childRet;
  UISystemX11* uiSystem = UI_SYSTEM();
  bool ok = uiSystem->pXTranslateCoordinates(uiSystem->display(),
    (XID)handle(), uiSystem->root(),
    coords->x(), coords->y(),
    &coords->_x, &coords->_y,
    &childRet);

  return (ok) ? (err_t)Error::Ok : (err_t)Error::FailedToTranslateCoordinates;
}

void UIWindowX11::onX11Event(XEvent* xe)
{
  UISystemX11* uiSystem = UI_SYSTEM();

  switch (xe->xany.type)
  {
    case XCreateNotify:
      fog_debug("Fog::UIWindowX11::onX11Event() - Create notify");
      break;
    case XDestroyNotify:
      fog_debug("Fog::UIWindowX11::onX11Event() - Destroy notify");
      break;

    case XMapNotify:
      _mapRequest = false;
      onVisibility(true);
      break;

    case XMapRequest:
      _mapRequest = true;
      onVisibility(false);
      break;

    case XUnmapNotify:
      _mapRequest = false;
      onVisibility(false);
      break;

    case XConfigureRequest:
      fog_debug("Fog::UIWindowX11::onX11Event() - Configure request: %d %d", xe->xconfigurerequest.x, xe->xconfigurerequest.y);
      break;

    case XConfigureNotify:
    {
      _xflags |= XFlag_Configured;

      // Don't process old configure events.
      while (uiSystem->pXCheckTypedWindowEvent(uiSystem->display(), xe->xany.window, XConfigureNotify, xe)) ;

      Rect windowRect(
        xe->xconfigure.x,
        xe->xconfigure.y,
        (int)xe->xconfigure.width,
        (int)xe->xconfigure.height);
      Rect clientRect(
        0,
        0,
        windowRect.width(),
        windowRect.height());

      onConfigure(windowRect, clientRect);
      break;
    }

    case XFocusIn:
      onFocus(true);
      break;
    
    case XFocusOut:
      onFocus(false);
      break;

    case XKeyPress:
    {
      TemporaryString32<8> unicode;

      KeySym xsym = 0;
      uint32_t key;

      if (_xic)
      {
        // Use XIC (X input context) to translate key
        wchar_t buf[32];
        XStatus status;
        int len = uiSystem->pXwcLookupString(_xic,
          (XKeyPressedEvent *)xe,
          buf,
          FOG_ARRAY_SIZE(buf) - 1,
          &xsym,
          &status);

        switch (status)
        {
          case XBufferOverflow:
            fog_debug("Fog::UIWindowX11::onX11Event() - 'KeyPress', Buffer too small (XIC)");
            // I don't know if this is possible when we have
            // buffer for 31 characters, if this error occurs,
            // we will skip this event.
            return;
          case XLookupChars:
          case XLookupKeySym:
          case XLookupBoth:
            unicode.set(StubW(buf, len));
            break;
          default:
            goto __keyPressNoXIC;
        }
      }
      else
      {
__keyPressNoXIC:
        // XIC not supported...?
        char buf[15*6 + 1];
        int len = uiSystem->pXLookupString(&xe->xkey, buf, FOG_ARRAY_SIZE(buf) - 1, &xsym, 0);
        unicode.set(StubLocal8(buf, len));
      }

      key = uiSystem->translateXSym(xsym);
      onKeyPress(key, uiSystem->keyToModifier(key), xe->xkey.keycode,
        Char32(unicode.length() == 1 ? unicode.at(0).ch() : 0));
      break;
    }

    case XKeyRelease:
    {
      KeySym xsym = uiSystem->pXKeycodeToKeysym(uiSystem->display(), xe->xkey.keycode, 0);
      uint32_t key = uiSystem->translateXSym(xsym);

      onKeyRelease(key, uiSystem->keyToModifier(key), xe->xkey.keycode, Char32(0));
      break;
    }

    case XEnterNotify:
      if (uiSystem->_systemMouseStatus.uiWindow == this &&
          uiSystem->_systemMouseStatus.buttons)
      {
        onMouseMove(xe->xmotion.x, xe->xmotion.y);
      }
      else
      {
        onMouseHover(xe->xmotion.x, xe->xmotion.y);
      }
      break;
    case XLeaveNotify:
      if (uiSystem->_systemMouseStatus.uiWindow == this &&
          uiSystem->_systemMouseStatus.buttons)
      {
        onMouseMove(xe->xmotion.x, xe->xmotion.y);
      }
      else
      {
        onMouseLeave(xe->xmotion.x, xe->xmotion.y);
      }
      break;
    case XMotionNotify:
      onMouseMove(xe->xmotion.x, xe->xmotion.y);
      break;

    case XButtonPress:
    {
#if 0
      uiSystem->pXGrabPointer(
        uiSystem->display(),                    /* Display              */
        (XID)handle(),                          /* Window               */
        true,                                   /* Owner events         */
        ButtonPressMask  | ButtonReleaseMask |  /* Event mask           */
        ButtonMotionMask | PointerMotionMask ,
        GrabModeAsync,                          /* Pointer mode         */
        GrabModeAsync,                          /* Keyboard mode        */
        XNone,                                  /* Confine to (window)  */
        0,                                      /* Cursor               */
        CurrentTime);                           /* Time                 */
#endif

      uint button = uiSystem->translateButton(xe->xbutton.button);
      switch (button)
      {
        case ButtonLeft:
        case ButtonMiddle:
        case ButtonRight:
          onMousePress(button, false);
          break;
        case WheelUp:
        case WheelDown:
          onMouseWheel(button);
          break;
      }
      break;
    }

    case XButtonRelease:
    {
      uint button = uiSystem->translateButton(xe->xbutton.button);
      switch (button)
      {
        case ButtonLeft:
        case ButtonMiddle:
        case ButtonRight:
          onMouseRelease(button);
          break;
      }
      break;
    }

    case XExpose:
    {
      if (!_dirty)
      {
        do {
          Box box(
            xe->xexpose.x,
            xe->xexpose.y,
            xe->xexpose.x + xe->xexpose.width,
            xe->xexpose.y + xe->xexpose.height);
          reinterpret_cast<UIBackingStoreX11*>(_backingStore)->blitRects(
            (XID)handle(), &box, 1);
        } while (xe->xexpose.count > 0 && uiSystem->pXCheckTypedWindowEvent(uiSystem->display(), xe->xany.window, XExpose, xe));

        uiSystem->pXFlush(uiSystem->display());
      }
      else
      {
        // Eat all events, because we will repaint later (in update process).
        while (uiSystem->pXCheckTypedWindowEvent(uiSystem->display(), xe->xany.window, XExpose, xe)) ;
        _blit = true;
      }
      break;
    }

    case XClientMessage:
    {
      if (xe->xclient.message_type == uiSystem->atom(UISystemX11::Atom_WM_PROTOCOLS))
      {
        if ((Atom)xe->xclient.data.l[0] == uiSystem->atom(UISystemX11::Atom_WM_DELETE_WINDOW))
        {
          CloseEvent e;
          _widget->sendEvent(&e);
        }
      }
      break;
    }

    default:
      break;
  }
}

// tell window manager that we want's to move our window to our position
// (it will be not discarded if we will specify PPosition)
void UIWindowX11::setMoveableHints()
{
  XSizeHints hints;
  Memory::zero(&hints, sizeof(XSizeHints));
  hints.flags = PPosition;

  UISystemX11* uiSystem = UI_SYSTEM();
  uiSystem->pXSetNormalHints(uiSystem->display(), (XID)handle(), &hints);
}
// ============================================================================
// [Fog::UIBackingStoreX11]
// ============================================================================

UIBackingStoreX11::UIBackingStoreX11()
{
  _pixmap = 0;
  Memory::zero(&_shmi, sizeof(_shmi));
  _ximage = NULL;
}

UIBackingStoreX11::~UIBackingStoreX11()
{
  destroy();
}

bool UIBackingStoreX11::resize(uint width, uint height, bool cache)
{
  UISystemX11* uiSystem = UI_SYSTEM();

  uint32_t targetWidth = width;
  uint32_t targetHeight = height;
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

      case TypeXShmPixmap:
        uiSystem->pXShmDetach(uiSystem->display(), &_shmi);
        uiSystem->pXSync(uiSystem->display(), False);

        shmdt(_shmi.shmaddr);
        shmctl(_shmi.shmid, IPC_RMID, NULL);

        uiSystem->pXFreePixmap(uiSystem->display(), _pixmap);

        if (_pixelsSecondary) Memory::free(_pixelsSecondary);
        break;

      case TypeXImage:
      case TypeXImageWithPixmap:
        // We want to free image data ourselves
        Memory::free(_pixelsPrimary);
        _ximage->data = NULL;

        uiSystem->pXDestroyImage(_ximage);
#if defined(FOG_UIBACKINGSTORE_FORCE_PIXMAP)
        if (_type == TypeXImageWithPixmap)
        {
          uiSystem->pXFreePixmap(uiSystem->display(), _pixmap);
        }
#endif
        if (_pixelsSecondary) Memory::free(_pixelsSecondary);
        break;
    }
  }

  // Create image buffer
  if (createImage)
  {
    // correct target BPP, some X server settings can be amazing:-)
    uint targetBPP = uiSystem->_displayInfo.depth;
    if (targetBPP > 4 && targetBPP < 8) targetBPP = 8;
    else if (targetBPP == 15) targetBPP = 16;
    else if (targetBPP == 24) targetBPP = 32;

    targetStride = Image::calcStride(targetWidth, targetBPP);
    targetSize = targetStride * targetHeight;

    // TypeXShmPixmap
    if (uiSystem->xShm())
    {
      if ((_shmi.shmid = shmget(IPC_PRIVATE, targetSize, IPC_CREAT | IPC_EXCL | 0666)) < 0)
      {
        fog_stderr_msg("Fog::UIBackingStoreX11", "resize", "shmget() failed: %s", strerror(errno));
        goto __tryImage;
      }

      if ((_shmi.shmaddr = (char *)shmat(_shmi.shmid, NULL, 0)) == NULL)
      {
        fog_stderr_msg("Fog::UIBackingStoreX11", "resize", "shmat() failed: %s", strerror(errno));
        shmctl(_shmi.shmid, IPC_RMID, NULL);
        goto __tryImage;
      }

      _shmi.readOnly = False;

      // Get the X server to attach this segment to a pixmap
      uiSystem->pXShmAttach(uiSystem->display(), &_shmi);
      uiSystem->pXSync(uiSystem->display(), False);

      _pixmap = uiSystem->pXShmCreatePixmap(
        uiSystem->display(),
        uiSystem->root(),
        _shmi.shmaddr,
        &_shmi,
        targetWidth, targetHeight,
        uiSystem->_displayInfo.depth);

      _type = TypeXShmPixmap;
      _pixelsPrimary = (uint8_t*)_shmi.shmaddr;
    }

    // TypeXImage
    else
    {
__tryImage:
      // try to alloc image data
      _pixelsPrimary = (uint8_t*)Memory::alloc(targetSize);
      if (!_pixelsPrimary)
      {
        fog_stderr_msg("Fog::UIBackingStoreX11", "resize", "Memory allocation error %s", strerror(errno));
        goto fail;
      }

      // try to create XImage
      _ximage = uiSystem->pXCreateImage(
        uiSystem->display(),
        uiSystem->visual(),
        uiSystem->_displayInfo.depth,
        ZPixmap, 0, (char *)_pixelsPrimary,
        targetWidth, targetHeight,
        32, targetStride);

      if (!_ximage)
      {
        fog_stderr_msg("Fog::UIBackingStoreX11", "resize", "XCreateImage() failed");
        Memory::free(_pixelsPrimary);
        goto fail;
      }
#if defined(FOG_UIBACKINGSTORE_FORCE_PIXMAP)
      // this step should be optional, but can increase performance
      // on remote machines (local machines should use XSHM extension)
      _pixmap = uiSystem->pXCreatePixmap(
        uiSystem->display(),
        uiSystem->root(),
        targetWidth, targetHeight,
        uiSystem->_displayInfo.depth);

      if (_pixmap)
        _type = TypeXImageWithPixmap;
      else
#endif
        _type = TypeXImage;
    }

    if (_type != TypeNone)
    {
      _created = TimeTicks::now();
      _expire = _created + TimeDelta::fromSeconds(15);

      _format.set(ImageFormat::XRGB32);

      _stridePrimary = targetStride;
      _widthOrig = targetWidth;
      _heightOrig = targetHeight;

      // Now image is created and we must check if we have correct
      // depth and pixel format, if not, we must create secondary
      // buffer that will be used for conversion
      sysint_t secondaryStride = (sysint_t)targetWidth << 2;

      if (targetStride != secondaryStride)
      {
        Converter::Format fTarget;
        Converter::Format fSource;

        fTarget.depth = targetBPP;
        fTarget.rMask = uiSystem->_displayInfo.rMask;
        fTarget.gMask = uiSystem->_displayInfo.gMask;
        fTarget.bMask = uiSystem->_displayInfo.bMask;
        fTarget.aMask = 0x00000000;
        fTarget.flags = 0;
        if (uiSystem->_displayInfo.is16BitSwapped) fTarget.flags |= Converter::ByteSwap;

        fSource.depth = 32;
        fSource.rMask = 0x00FF0000;
        fSource.gMask = 0x0000FF00;
        fSource.bMask = 0x000000FF;
        fSource.aMask = 0x00000000;
        fSource.flags = 0;

        // Alloc extra buffer.
        _pixelsSecondary = (uint8_t*)Memory::alloc(secondaryStride * targetHeight);
        if (!_pixelsSecondary)
        {
          fog_stderr_msg("Fog::UIBackingStoreX11", "resize", "Can't create secondary buffer for converting");
        }

        _strideSecondary = secondaryStride;

        _usingConverter = true;
        _converter.setup(fTarget, fSource);

        _pixels = _pixelsSecondary;
        _width = width;
        _height = height;
        _stride = _strideSecondary;
      }
      else
      {
        // Extra buffer not needed.
        _pixelsSecondary = NULL;
        _strideSecondary = 0;

        _usingConverter = false;

        _pixels = _pixelsPrimary;
        _width = width;
        _height = height;
        _stride = _stridePrimary;
      }
      return true;
    }
  }

fail:
  _clear();
  _pixmap = 0;
  _ximage = NULL;
  return false;
}

void UIBackingStoreX11::destroy()
{
  resize(0, 0, false);
}

void UIBackingStoreX11::updateRects(const Box* rects, sysuint_t count)
{
  UISystemX11* uiSystem = UI_SYSTEM();

  // If there is secondary buffer, we need to convert it to primary
  // one that has same depth and pixel format as X display.
  if (_pixelsSecondary && _usingConverter)
  {
    // TODO: Palconv & Converter
    // _converter.palConv = uiSystem->_paletteInfo.palConv;

    for (sysuint_t i = 0; i != count; i++)
    {
      int x1 = rects[i].x1();
      int y1 = rects[i].y1();
      int x2 = rects[i].x2();
      int y2 = rects[i].y2();

      // Apply clip. In rare cases rectangles can contain bigger
      // coordinates that buffers are (reason can be resizing).
      if (x1 < 0) x1 = 0;
      if (y1 < 0) y1 = 0;
      if (x2 > (int)width()) x2 = (int)width();
      if (y2 > (int)height()) y2 = (int)height();

      if (x1 >= x2 || y1 >= y2) continue;

      uint w = uint(x2 - x1);
      uint h = uint(y2 - y1);

      _converter.convertRect(
        _pixelsPrimary + (y1 * _stridePrimary), _stridePrimary, x1,
        _pixelsSecondary + (y1 * _strideSecondary), _strideSecondary, x1,
        w, h, Point(x1, y1));
    }
  }

  // Possible secondary step is to put XImage to Pixmap here, because
  // it can increase performance on remote machines. Idea is to do
  // put XImage here instead in blitRects() method.
  if (_type == TypeXImageWithPixmap)
  {
    for (sysuint_t i = 0; i != count; i++)
    {
      int x = rects[i].x1();
      int y = rects[i].y1();
      uint w = uint(rects[i].width());
      uint h = uint(rects[i].height());

      uiSystem->pXPutImage(
        uiSystem->display(),
        _pixmap,
        uiSystem->gc(),
        _ximage,
        x, y,
        x, y,
        w, h);
    }
  }
}

void UIBackingStoreX11::blitRects(XID target, const Box* rects, sysuint_t count)
{
  UISystemX11* uiSystem = UI_SYSTEM();
  sysuint_t i;

  switch (type())
  {
    case TypeNone:
      break;

    // These ones uses pixmap as X resource
    case TypeXShmPixmap:
    case TypeXImageWithPixmap:
      for (i = 0; i != count; i++)
      {
        int x = rects[i].x1();
        int y = rects[i].y1();
        int w = rects[i].width();
        int h = rects[i].height();

        uiSystem->pXCopyArea(
          uiSystem->display(),
          _pixmap,
          target,
          uiSystem->gc(),
          x, y,
          w, h,
          x, y);
      }
      break;

    case TypeXImage:
      for (i = 0; i != count; i++)
      {
        int x = rects[i].x1();
        int y = rects[i].y1();
        int w = rects[i].width();
        int h = rects[i].height();

        uiSystem->pXPutImage(
          uiSystem->display(),
          target,
          uiSystem->gc(),
          _ximage,
          x, y,
          x, y,
          w, h);
      }
      break;
  }
}

// ============================================================================
// [Fog::EventPumpX11]
// ============================================================================

EventPumpX11::EventPumpX11() :
  EventPump(StubAscii8("UI::X11")),
  _state(NULL)
{
}

EventPumpX11::~EventPumpX11()
{
}

void EventPumpX11::run(EventPump::Delegate* delegate)
{
  RunState s;

  s.delegate = delegate;
  s.shouldQuit = false;
  s.runDepth = _state ? _state->runDepth + 1 : 1;

  RunState* previousState = _state;
  _state = &s;

  doRunLoop();

  _state = previousState;
}

void EventPumpX11::quit()
{
  FOG_ASSERT(_state);
  _state->shouldQuit = true;
}

void EventPumpX11::scheduleWork()
{
  // TODO: Wake up
}

void EventPumpX11::scheduleDelayedWork(const Time& delayedWorkTime)
{
  // We know that we can't be blocked on Wait right now since this method can
  // only be called on the same thread as Run, so we only need to update our
  // record of how long to sleep when we do sleep.
  _delayedWorkTime = delayedWorkTime;
}

void EventPumpX11::doRunLoop()
{
  // Inspired in EventPumpWin and ported to X11.
  for (;;)
  {
    bool moreWorkIsPlausible = processNextXEvent();
    if (_state->shouldQuit) break;

    moreWorkIsPlausible |= _state->delegate->doWork();
    if (_state->shouldQuit) break;

    moreWorkIsPlausible |= _state->delegate->doDelayedWork(&_delayedWorkTime);
    if (_state->shouldQuit) break;

    if (moreWorkIsPlausible) continue;

    // If quit is received in nestedLoop or through runAllPending(), we will
    // quit here, because we don't want to do XSync().
    moreWorkIsPlausible = _state->delegate->doIdleWork();
    if (_state->shouldQuit) break;

    // Call XSync, this is round-trip operation and can generate events.
    moreWorkIsPlausible = xsync();
    if (moreWorkIsPlausible) continue;

    waitForWork();
  }
}

void EventPumpX11::waitForWork()
{
  UISystemX11* uiSystem = UI_SYSTEM();
}

bool EventPumpX11::xsync()
{
  UISystemX11* uiSystem = UI_SYSTEM();
  uiSystem->pXSync(uiSystem->display(), false);
  return uiSystem->pXPending(uiSystem->display());
}

bool EventPumpX11::processNextXEvent()
{
  UISystemX11* uiSystem = UI_SYSTEM();
  if (!uiSystem->pXPending(uiSystem->display())) return false;

  XEvent xe;
  uiSystem->pXNextEvent(uiSystem->display(), &xe);

  UIWindowX11* uiWindow = reinterpret_cast<UIWindowX11*>(uiSystem->handleToNative((void*)xe.xany.window));
  if (uiWindow) uiWindow->onX11Event(&xe);

  return true;
}

// ============================================================================
// [Fog::EventLoopX11]
// ============================================================================

EventLoopX11::EventLoopX11() :
  EventLoop(new EventPumpX11())
{
}

} // Fog namespace

// ============================================================================
// [CAPI]
// ============================================================================

extern "C" FOG_DLL_EXPORT void* createUISystem()
{
  return new Fog::UISystemX11();
}

#endif /// FOG_UI_X11
