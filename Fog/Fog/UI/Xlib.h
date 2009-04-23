// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_UI_XLIB_H
#define _FOG_UI_XLIB_H

// [Dependencies]
#include <Fog/Core/Library.h>

#if defined(FOG_UI_X11)

#ifdef _XLIB_H
#error "You must include Gui/Xlib.h before xlib"
#endif

// redefine some critical things

#define Window XWindow
#define Region XRegion
#define Complex XComplex
#define Visual XVisual
#define Colormap XColormap
#define Picture XPicture

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#define XK_MISCELLANY
#define XK_LATIN1
#define XK_XKB_KEYS // for slackware
#include <X11/keysymdef.h>

#include <X11/Xpoll.h>

// We need XShm extension to speedup our blits to top level windows.
#include <X11/extensions/XShm.h>
// We need XRender extension?
#include <X11/extensions/Xrender.h>
// We need shape extension to enable non-rectangular windows.
#include <X11/extensions/shape.h>

#undef Window
#undef Region
#undef Complex
#undef Visual
#undef Colormap
#undef Picture

#ifdef Unsorted
const int XUnsorted = Unsorted;
#undef Unsorted
#endif

#ifdef None
const XID XNone = None;
#undef None
#endif

#ifdef Bool
typedef Bool XBool;
#undef Bool
#endif

#ifdef ConfigureRequest
const int XConfigureRequest = ConfigureRequest;
#undef ConfigureRequest
#endif

#ifdef CreateNotify
const int XCreateNotify = CreateNotify;
#undef CreateNotify
#endif

#ifdef DestroyNotify
const int XDestroyNotify = DestroyNotify;
#undef DestroyNotify
#endif

#ifdef MapNotify
const int XMapNotify = MapNotify;
#undef MapNotify
#endif

#ifdef UnmapNotify
const int XUnmapNotify = UnmapNotify;
#undef UnmapNotify
#endif

#ifdef MapRequest
const int XMapRequest = MapRequest;
#undef MapRequest
#endif

#ifdef KeymapNotify
const int XKeymapNotify = KeymapNotify;
#undef KeymapNotify
#endif

#ifdef MappingNotify
const int XMappingNotify = MappingNotify;
#undef MappingNotify
#endif

#ifdef KeyPress
const int XKeyPress = KeyPress;
#undef KeyPress
#endif

#ifdef KeyRelease
const int XKeyRelease = KeyRelease;
#undef KeyRelease
#endif

#ifdef EnterNotify
const int XEnterNotify = EnterNotify;
#undef EnterNotify
#endif

#ifdef MotionNotify
const int XMotionNotify = MotionNotify;
#undef MotionNotify
#endif

#ifdef LeaveNotify
const int XLeaveNotify = LeaveNotify;
#undef LeaveNotify
#endif

#ifdef ButtonPress
const int XButtonPress = ButtonPress;
#undef ButtonPress
#endif

#ifdef ButtonRelease
const int XButtonRelease = ButtonRelease;
#undef ButtonRelease
#endif

#ifdef Expose
const int XExpose = Expose;
#undef Expose
#endif

#ifdef VisibilityNotify
const int XVisibilityNotify = VisibilityNotify;
#undef VisibilityNotify
#endif

#ifdef ConfigureNotify
const int XConfigureNotify = ConfigureNotify;
#undef ConfigureNotify
#endif

#ifdef ClientMessage
const int XClientMessage = ClientMessage;
#undef ClientMessage
#endif

#ifdef Above
const int XAbove = Above;
#undef Above
#endif

#ifdef Below
const int XBelow = Below;
#undef Below
#endif

#ifdef FocusIn
const int XFocusIn = FocusIn;
#undef FocusIn
#endif

#ifdef FocusOut
const int XFocusOut = FocusOut;
#undef FocusOut
#endif

#ifdef Always
const int XAlways = Always;
#undef Always
#endif

#ifdef Success
const int XSuccess = Success;
#undef Success
#endif

#ifdef GrayScale
const int XGrayScale = GrayScale;
#undef GrayScale
#endif

#ifdef Status
typedef Status XStatus;
#undef Status
#endif

#ifdef CursorShape
const int XCursorShape = CursorShape;
#undef CursorShape
#endif

// index is used for methods...
#if defined(index)
#undef index
#endif

namespace Fog {

struct FOG_API Xlib
{
  enum XAtoms
  {
    XAtom_WM_PROTOCOLS = 0,
    XAtom_WM_DELETE_WINDOW,
    XAtom_WM_TAKE_FOCUS,
    XAtom_WM_CLASS,
    XAtom_WM_NAME,
    XAtom_WM_COMMAND,
    XAtom_WM_ICON_NAME,
    XAtom_WM_CLIENT_MACHINE,
    XAtom_WM_CHANGE_STATE,
    XAtom_MOTIF_WM_HINTS,
    XAtom_WIN_LAYER,

    XAtom_XdndSelection,
    XAtom_XdndAware,
    XAtom_XdndTypeList,
    XAtom_XdndEnter,
    XAtom_XdndPosition,
    XAtom_XdndActionCopy,
    XAtom_XdndActionMove,
    XAtom_XdndActionPrivate,
    XAtom_XdndActionAsk,
    XAtom_XdndActionList,
    XAtom_XdndActionLink,
    XAtom_XdndActionDescription,
    XAtom_XdndProxy,
    XAtom_XdndStatus,
    XAtom_XdndLeave,
    XAtom_XdndDrop,
    XAtom_XdndFinished,

    XAtom_NET_SUPPORTED,
    XAtom_NET_SUPPORTING_WM_CHECK,

    XAtom_NET_NUMBER_OF_DESKTOPS,
    XAtom_NET_DESKTOP_GEOMETRY,
    XAtom_NET_DESKTOP_NAMES,
    XAtom_NET_CURRENT_DESKTOP,
    XAtom_NET_DESKTOP_VIEWPORT,
    XAtom_NET_WORKAREA,
    XAtom_NET_VIRTUAL_ROOTS,

    XAtom_NET_CLIENT_LIST,
    XAtom_NET_CLIENT_LIST_STACKING,
    XAtom_NET_ACTIVE_WINDOW,
    XAtom_NET_CLOSE_WINDOW,

    XAtom_NET_WM_MOVERESIZE,
    XAtom_NET_WM_NAME,
    XAtom_NET_WM_VISIBLE_NAME,
    XAtom_NET_WM_ICON_NAME,
    XAtom_NET_WM_VISIBLE_ICON_NAME,
    XAtom_NET_WM_DESKTOP,
    XAtom_NET_WM_WINDOW_TYPE,
    XAtom_NET_WM_STATE,
    XAtom_NET_WM_ALLOWED_ACTIONS,
    XAtom_NET_WM_STRUT,
    XAtom_NET_WM_STRUT_PARTIAL,
    XAtom_NET_WM_ICON_GEOMETRY,
    XAtom_NET_WM_ICON,
    XAtom_NET_WM_PID,
    XAtom_NET_WM_USER_TIME,

    XAtom_NET_WM_WINDOW_TYPE_DESKTOP,
    XAtom_NET_WM_WINDOW_TYPE_DOCK,
    XAtom_NET_WM_WINDOW_TYPE_TOOLBAR,
    XAtom_NET_WM_WINDOW_TYPE_MENU,
    XAtom_NET_WM_WINDOW_TYPE_UTILITY,
    XAtom_NET_WM_WINDOW_TYPE_SPLASH,
    XAtom_NET_WM_WINDOW_TYPE_DIALOG,
    XAtom_NET_WM_WINDOW_TYPE_NORMAL,

    XAtom_NET_WM_STATE_MODAL,
    XAtom_NET_WM_STATE_STICKY,
    XAtom_NET_WM_STATE_MAXIMIZED_VERT,
    XAtom_NET_WM_STATE_MAXIMIZED_HORZ,
    XAtom_NET_WM_STATE_SHADED,
    XAtom_NET_WM_STATE_SKIP_TASKBAR,
    XAtom_NET_WM_STATE_SKIP_PAGER,
    XAtom_NET_WM_STATE_HIDDEN,
    XAtom_NET_WM_STATE_FULLSCREEN,
    XAtom_NET_WM_STATE_ABOVE,
    XAtom_NET_WM_STATE_BELOW,
    XAtom_NET_WM_WINDOW_OPACITY,

    XAtom_CLIPBOARD,
    XAtom_TARGETS,
    XAtom_COMPOUND_TEXT,
    XAtom_UTF8_STRING,
    XAtom_FILE_NAME,
    XAtom_STRING,
    XAtom_TEXT,
    XAtom_INCR,

    XAtom_Count
  };

  enum { 
    XlibFunctionsCount = 76,
    XextFunctionsCount = 5,
    XrenderFunctionsCount = 13 
  };

  union
  {
    struct
    {
      XBool (*pXSupportsLocale)(void);
      char* (*pXSetLocaleModifiers)(const char* /* modifierList */);

      Display* (*pXOpenDisplay)(const char*);
      int (*pXCloseDisplay)(Display*);

      XErrorHandler (*pXSetErrorHandler)(XErrorHandler /* func */);
      XIOErrorHandler (*pXSetIOErrorHandler)(XIOErrorHandler /* func */);
      int (*pXGetErrorText)(Display*, int /* code */, char* /* bufferReturn */, int /* bufferLength */);

      Atom (*pXInternAtom)(Display*, const char* /* atomName */, XBool /* onlyIfExists */);
      XStatus (*pXInternAtoms)(Display*, char** /* names */, int /* count */, XBool /* onlyIfExists */, Atom* /* atomsReturn */);

      Wde_ID (*pXRootWindow)(Display*, int /* screenNumber */);
      int (*pXDisplayWidth)(Display*, int /* screenNumber */);
      int (*pXDisplayHeight)(Display*, int /* screenNumber */);
      int (*pXDefaultScreen)(Display*);
      XVisual* (*pXDefaultVisual)(Display*, int /* screenNumber */);
      XColormap (*pXDefaultColormap)(Display*, int /* screenNumber */);
      int (*pXDefaultDepth)(Display*, int /* screenNumber */);

      int (*pXPending)(Display*);
      int (*pXNextEvent)(Display*, XEvent* /* xEvent */);
      XBool (*pXCheckTypedEvent)(Display*, int /* eventType */, XEvent* /* eventReturn */);
      XBool (*pXCheckTypedWindowEvent)(Display*, Wde_ID /* window */, int /* eventType */, XEvent* /* eventReturn */);
      XBool (*pXCheckWindowEvent)(Display*, Wde_ID /* window */, long /* eventMask */, XEvent* /* eventReturn */);

      XIM (*pXOpenIM)(Display*, void* /* rdb */, char* /* resName */, char* /* resClass */);
      XStatus (*pXCloseIM)(XIM /* im */);

      XIC (*pXCreateIC)(XIM /* im */, ...);
      void (*pXDestroyIC)(XIC /* ic */);
      int (*pXRefreshKeyboardMapping)(XMappingEvent* /* eventMap */);

      Wde_ID (*pXCreateWindow)(Display*, Wde_ID /* parent */, int /* x */, int /* y */, uint /* width */, uint /* height */, uint /* boderWidth */, int /* depth */, uint /* class */, XVisual* /* visual */, ulong /* valueMask */, XSetWindowAttributes* /* attributes */);
      int (*pXDestroyWindow)(Display*, Wde_ID /* window */);
      int (*pXMoveWindow)(Display*, Wde_ID /* window */, int /* x */, int /* y */);
      int (*pXResizeWindow)(Display*, Wde_ID /* window */, uint /* w */, uint /* h */);
      int (*pXMoveResizeWindow)(Display*, Wde_ID /* window */, int /* x */, int /* y */, uint /* w */, uint /* h */);
      int (*pXMapWindow)(Display*, Wde_ID /* window */);
      int (*pXUnmapWindow)(Display*, Wde_ID /* window */);
      int (*pXRaiseWindow)(Display*, Wde_ID /* window */);
      int (*pXLowerWindow)(Display*, Wde_ID /* window */);
      int (*pXReparentWindow)(Display*, Wde_ID /* window */, Wde_ID /* parent */, int /* x */, int /* y */);
      int (*pXChangeWindowAttributes)(Display*, Wde_ID /* window */, ulong /* valueMask */, XSetWindowAttributes* /* attributes */);
      XStatus (*pXGetWindowAttributes)(Display*, Wde_ID /* window */, XWindowAttributes* /* attributesReturns */);

      int (*pXSetInputFocus)(Display*, Wde_ID /* window */, int /* revert_to */, Time /* time */);
      int (*pXGetInputFocus)(Display*, Wde_ID* /* window_return */, int* /* revert_to_return */);

      int (*pXSelectInput)(Display* , Wde_ID /* window */, long /* event mask */);
      int (*pXSetNormalHints)(Display*, Wde_ID /* window */, XSizeHints* /* hints */);

      int (*pXSetWMProtocols)(Display*, Wde_ID /* window */, Atom* /* protocols */, int /* count */);
      int (*pXGetWMProtocols)(Display*, Wde_ID /* window */, Atom** /* protocolsReturn */, int* /* countReturn */);

      int (*pXTranslateCoordinates)(Display*, XWindow /* src_w */, XWindow /* dest_w */, int /* src_x */, int /* src_y */, int* /* dest_x_return */, int* /* dest_y_return */, XWindow* /* child_return */);
      int (*pXwcTextListToTextProperty)(Display*, wchar_t** /* list */, int /* count */, XICCEncodingStyle /* style */, XTextProperty* /* textPropertyReturn */);

      int (*pXFlush)(Display*);
      int (*pXSync)(Display*, XBool /* discard */);

      GC (*pXCreateGC)(Display*, Wde_ID /* drawable */, ulong /* valueMask */, XGCValues* /* values */);
      int (*pXFreeGC)(Display*, GC /* gc */);

      KeySym (*pXKeycodeToKeysym)(Display*, uint /* keyCode */, int /* index */);

      int (*pXLookupString)(XKeyEvent* /* event */, char* /* bufferReturn */, int /* bufferLength */, KeySym* /* keySymReturn */, XComposeStatus* /* statusInOut */);
      int (*pXwcLookupString)(XIC /* ic */, XKeyPressedEvent* /* event */, wchar_t* /* bufferReturn*/, int /* wcharsBuffer */, KeySym* /* keysymReturn */, XStatus* /* statusReturn */);

      int (*pXGrabPointer)(Display*, Wde_ID /* grabWindow */, XBool /* ownerEvents */, uint /* eventMask */, int /* pointerMode */, int /* keyboardMode */, Wde_ID /* confineTo */, Cursor /* cursor */, Time /* time */);
      int (*pXUngrabPointer)(Display*, Time /* time */);

      int (*pXGetPointerMapping)(Display*, uchar* /* map_return*/, int /* namp*/);

      Wde_ID (*pXCreatePixmap)(Display*, Wde_ID /* drawable */, uint /* width */, uint /* height */, uint /* depth */);
      int (*pXFreePixmap)(Display*, Wde_ID /* pixmap */);
      int (*pXCopyArea)(Display*, Drawable /* src*/, Wde_ID /* dest*/, GC /* gc */, int /* srcX */, int /* srcY */, uint /* width */, uint /* height */, int /* destX */, int /* destY */);
      int (*pXFree)(void* /* data */);

      XImage* (*pXCreateImage)(Display*, XVisual* /* visual */, uint /* depth */, int /* format */, int /* offset */, char* /* data */, uint /* width */, uint /* height */, int /* bitmapPad */, int /* bytesPerLine */);
      int (*pXDestroyImage)(XImage* /* image */);
      int (*pXPutImage)(Display*, Wde_ID /* drawable */, GC /* gc */, XImage* /* xImage */, int /* srcX */, int /* srcY */, int /* destX */, int /* destY */, uint /* width */, uint /* height */);

      XColormap (*pXCreateColormap)(Display*, Wde_ID /* window */, XVisual* /* visual */, int /* alloc */);
      int (*pXFreeColormap)(Display*, XColormap /* colormap */);

      XStatus (*pXAllocColor)(Display*, XColormap, XColor* /* screen_in_out */);
      int (*pXFreeColors)(Display*, XColormap, ulong* /* pixels */, int /* npixels */, ulong /* planes*/);

      XStatus (*pXMatchVisualInfo)(Display*, int /* screen */, int /* depth */, int /* class */, XVisualInfo* /* vinfo_return */);

      // present in Xutil.h
      void (*pXSetWMName)(Display*, Wde_ID /* window */, XTextProperty* /* textProperty */);
      
      XStatus (*pXSendEvent)(Display*, Wde_ID /* window */, XBool, long, XEvent*);
      void (*pXChangeProperty)(Display*, Wde_ID /*window */, Atom, Atom, int, int, unsigned char*, int);
      void (*pXSetSelectionOwner)(Display*, Atom, XWindow, Time);
      char* (*pXGetAtomName)(Display*, Atom);
      void (*pXConvertSelection)(Display*, Atom, Atom, Atom, XWindow, Time);
      int (*pXGetWindowProperty)(Display*, XWindow, Atom, long, long, XBool, Atom, Atom*, int*, unsigned long*, unsigned long*, unsigned char**);
      void (*pXDeleteProperty)(Display*, XWindow, Atom);
    };

    // Function pointers for loader
    void* xlibAddr[XlibFunctionsCount];
  };

  union
  {
    struct
    {
      XBool (*pXShmQueryVersion)(Display*, int* /* majorVersion */, int* /* minorVersion */, XBool* /* sharedPixmaps */);
      XStatus (*pXShmAttach)(Display*, XShmSegmentInfo* /* shmInfo */);
      XStatus (*pXShmDetach)(Display*, XShmSegmentInfo* /* shmInfo */);
      XImage* (*pXShmCreateImage)(Display*, XVisual* /* visual */, uint /* depth */, int /* format */, char* /* data */, XShmSegmentInfo* /* shmInfo */, uint /* width */, uint /* height */);
      Pixmap (*pXShmCreatePixmap)(Display*, Wde_ID /* drawable */, char* /* data */, XShmSegmentInfo* /* shmInfo */, uint /* width */, uint /* height */, uint /* depth */);
    };

    // Function pointers for loader
    void* xextAddr[XextFunctionsCount];
  };

  union
  {
    struct
    {
      XStatus (*pXRenderQueryVersion)(Display*, int* /* major */, int * /* minor */);
      XStatus (*pXRenderQueryFormats)(Display*);
      int (*pXRenderQuerySubpixelOrder)(Display*, int /* screen */);
      XBool (*pXRenderSetSubpixelOrder)(Display*, int /* screen */, int /* subpixel */);
      XRenderPictFormat* (*pXRenderFindVisualFormat)(Display*, _Xconst XVisual* visual);
      XRenderPictFormat* (*pXRenderFindFormat)(Display*, unsigned long /* mask */, _Xconst XRenderPictFormat* /* templ */, int /* count */);
      XRenderPictFormat* (*pXRenderFindStandardFormat)(Display*, int /* format */);
      XPicture (*pXRenderCreatePicture)(Display*, Wde_ID /* drawable */, _Xconst XRenderPictFormat* /* format */, unsigned long /* valuemask */, _Xconst XRenderPictureAttributes* /* attributes */);
      void (*pXRenderChangePicture)(Display*, XPicture /* picture */, unsigned long /* valuemask */, _Xconst XRenderPictureAttributes* /* attributes */);
      void (*pXRenderSetPictureTransform)(Display*, XPicture /* picture */, XTransform* /* transform */);
      void (*pXRenderFreePicture)(Display*, XPicture /* picture*/);
      void (*pXRenderComposite)(Display*, int /* op */, XPicture /* src */, XPicture /* mask */, XPicture /* dest */, int /* srcX */, int /* srcY */, int /* maskX */, int /* maskY */, int /* destX */, int /* destY */, unsigned int /* width */, unsigned int /* height */);
      Cursor (*pXRenderCreateCursor)(Display*, XPicture /* source */, unsigned int /* x */, unsigned int /* y */);
    };
    // Function pointers for loader
    void* xrenderAddr[XrenderFunctionsCount];
  };

  FOG_INLINE Display* display() const 
  { return _display; }
  
  FOG_INLINE int fd() const 
  { return _fd; }
  
  FOG_INLINE int screen() const 
  { return _screen; }
  
  FOG_INLINE XVisual* visual() const
  { return _visual; }
  
  FOG_INLINE XColormap colormap() const
  { return _colormap; }
  
  FOG_INLINE XWindow root() const
  { return _root; }
  
  FOG_INLINE GC gc() const
  { return _gc; }

  FOG_INLINE uint32_t xShm() const
  { return _xShm; }

  FOG_INLINE uint32_t xPrivateColormap() const
  { return _xPrivateColormap; }
  
  FOG_INLINE uint32_t xButtons() const
  { return _xButtons; }

  FOG_INLINE XIM xim() const
  { return _xim; }

  FOG_INLINE Atom* atoms() const
  { return (Atom*)_atoms; }

  FOG_INLINE Atom atom(sysuint_t index) const
  {
    FOG_ASSERT(index < XAtom_Count);
    return _atoms[index];
  }

  FOG_INLINE Wde_Library& xlib(void) { return _xlib; }
  FOG_INLINE Wde_Library& xext(void) { return _xext; }
  FOG_INLINE Wde_Library& xrender(void) { return _xrender; }

private:
  /*! @brief Xlib library object (dynamically opened libX11 library). */
  Fog::Library _xlib;
  /*! @brief Xext library object (dynamically opened libXext library). */
  Fog::Library _xext;
  /*! @brief Xrender library object (dynamically opened libXrender library). */
  Fog::Library _xrender;

  Display* _display;
  int _fd;
  int _screen;
  XVisual* _visual;
  XColormap _colormap;
  XWindow _root;
  GC _gc;

  uint32_t _xShm : 1;
  uint32_t _xPrivateColormap : 1;
  uint32_t _xButtons;

  XIM _xim;

  Atom _atoms[XAtom_Count];

};

} // Fog namespace

#endif // FOG_UI_X11

// [Guard]
#endif // _FOG_UI_XLIB_H
