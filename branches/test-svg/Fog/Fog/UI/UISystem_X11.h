// [Gui library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_UI_UISYSTEM_X11_H
#define _FOG_UI_UISYSTEM_X11_H

// [Dependencies]
#include <Fog/Build/Build.h>

#define FOG_UISYSTEM_X11_API FOG_API
//#if defined(Fog_EXPORTS)
//# define FOG_UISYSTEM_X11_API FOG_DLL_EXPORT
//#else
//# define FOG_UISYSTEM_X11_API FOG_DLL_IMPORT
//#endif // Fog_EXPORTS

#include <Fog/Core/Atomic.h>
#include <Fog/Core/Object.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/UI/Constants.h>
#include <Fog/UI/UISystem.h>
#include <Fog/UI/UISystem_Def.h>
#include <Fog/UI/Xlib.h>

namespace Fog {

// ============================================================================
// [Fog::UISystemX11]
// ============================================================================

struct FOG_UISYSTEM_X11_API UISystemX11 : public UISystemDefault
{
  FOG_DECLARE_OBJECT(UISystemX11, UISystemDefault)

  // [Construction / Destruction]

  UISystemX11();
  virtual ~UISystemX11();

  // [Display]

  virtual void updateDisplayInfo();

  // [Update]

  virtual void doBlitWindow(UIWindow* window, const Box* rects, sysuint_t count);

  // [UIWindow]

  virtual UIWindow* createUIWindow(Widget* widget);
  virtual void destroyUIWindow(UIWindow* native);

  // [X11 Atoms]

  enum Atom_Enum
  {
    Atom_WM_PROTOCOLS = 0,
    Atom_WM_DELETE_WINDOW,
    Atom_WM_TAKE_FOCUS,
    Atom_WM_CLASS,
    Atom_WM_NAME,
    Atom_WM_COMMAND,
    Atom_WM_ICON_NAME,
    Atom_WM_CLIENT_MACHINE,
    Atom_WM_CHANGE_STATE,
    Atom_MOTIF_WM_HINTS,
    Atom_WIN_LAYER,

    Atom_XdndSelection,
    Atom_XdndAware,
    Atom_XdndTypeList,
    Atom_XdndEnter,
    Atom_XdndPosition,
    Atom_XdndActionCopy,
    Atom_XdndActionMove,
    Atom_XdndActionPrivate,
    Atom_XdndActionAsk,
    Atom_XdndActionList,
    Atom_XdndActionLink,
    Atom_XdndActionDescription,
    Atom_XdndProxy,
    Atom_XdndStatus,
    Atom_XdndLeave,
    Atom_XdndDrop,
    Atom_XdndFinished,

    Atom_NET_SUPPORTED,
    Atom_NET_SUPPORTING_WM_CHECK,

    Atom_NET_NUMBER_OF_DESKTOPS,
    Atom_NET_DESKTOP_GEOMETRY,
    Atom_NET_DESKTOP_NAMES,
    Atom_NET_CURRENT_DESKTOP,
    Atom_NET_DESKTOP_VIEWPORT,
    Atom_NET_WORKAREA,
    Atom_NET_VIRTUAL_ROOTS,

    Atom_NET_CLIENT_LIST,
    Atom_NET_CLIENT_LIST_STACKING,
    Atom_NET_ACTIVE_WINDOW,
    Atom_NET_CLOSE_WINDOW,

    Atom_NET_WM_MOVERESIZE,
    Atom_NET_WM_NAME,
    Atom_NET_WM_VISIBLE_NAME,
    Atom_NET_WM_ICON_NAME,
    Atom_NET_WM_VISIBLE_ICON_NAME,
    Atom_NET_WM_DESKTOP,
    Atom_NET_WM_WINDOW_TYPE,
    Atom_NET_WM_STATE,
    Atom_NET_WM_ALLOWED_ACTIONS,
    Atom_NET_WM_STRUT,
    Atom_NET_WM_STRUT_PARTIAL,
    Atom_NET_WM_ICON_GEOMETRY,
    Atom_NET_WM_ICON,
    Atom_NET_WM_PID,
    Atom_NET_WM_USER_TIME,

    Atom_NET_WM_WINDOW_TYPE_DESKTOP,
    Atom_NET_WM_WINDOW_TYPE_DOCK,
    Atom_NET_WM_WINDOW_TYPE_TOOLBAR,
    Atom_NET_WM_WINDOW_TYPE_MENU,
    Atom_NET_WM_WINDOW_TYPE_UTILITY,
    Atom_NET_WM_WINDOW_TYPE_SPLASH,
    Atom_NET_WM_WINDOW_TYPE_DIALOG,
    Atom_NET_WM_WINDOW_TYPE_NORMAL,

    Atom_NET_WM_STATE_MODAL,
    Atom_NET_WM_STATE_STICKY,
    Atom_NET_WM_STATE_MAXIMIZED_VERT,
    Atom_NET_WM_STATE_MAXIMIZED_HORZ,
    Atom_NET_WM_STATE_SHADED,
    Atom_NET_WM_STATE_SKIP_TASKBAR,
    Atom_NET_WM_STATE_SKIP_PAGER,
    Atom_NET_WM_STATE_HIDDEN,
    Atom_NET_WM_STATE_FULLSCREEN,
    Atom_NET_WM_STATE_ABOVE,
    Atom_NET_WM_STATE_BELOW,
    Atom_NET_WM_WINDOW_OPACITY,

    Atom_CLIPBOARD,
    Atom_TARGETS,
    Atom_COMPOUND_TEXT,
    Atom_UTF8_STRING,
    Atom_FILE_NAME,
    Atom_STRING,
    Atom_TEXT,
    Atom_INCR,

    Atom_Count
  };

  Atom _atoms[Atom_Count];

  FOG_INLINE Atom* atoms(void) const
  {
    return (Atom*)_atoms;
  }

  FOG_INLINE Atom atom(int index) const
  {
    FOG_ASSERT(index < Atom_Count);
    return _atoms[index];
  }

  // [X11 Driver]

  Display* _display;
  int _fd;
  int _screen;
  XVisual* _visual;
  XColormap _colormap;
  XID _root;
  GC _gc;

  //! @brief Pipe to wake up event loop.
  int _wakeUpPipe[2];

  uint32_t _xShm : 1;
  uint32_t _xPrivateColormap : 1;

  XIM _xim;

  FOG_INLINE Display* display(void) const { return _display; }
  FOG_INLINE int fd(void) const { return _fd; }
  FOG_INLINE int screen(void) const { return _screen; }
  FOG_INLINE XVisual* visual(void) const { return _visual; }
  FOG_INLINE XColormap colormap(void) const { return _colormap; }
  FOG_INLINE XID root(void) const { return _root; }
  FOG_INLINE GC gc(void) const { return _gc; }

  FOG_INLINE uint32_t xShm(void) const { return _xShm; }
  FOG_INLINE uint32_t xPrivateColormap(void) const { return _xPrivateColormap; }
  FOG_INLINE uint32_t xButtons(void) const { return _xButtons; }

  FOG_INLINE XIM xim(void) const { return _xim; }

  // [X11 Keyboard / Unicode]

  struct X11Keymap
  {
    uint16_t odd[256];
    uint16_t misc[256];
  };

  X11Keymap _xKeymap;

  void initKeyboard();
  uint32_t translateXSym(KeySym xsym) const;

  // [X11 Mouse]

  uint32_t _xButtons;

  void initMouse();
  uint32_t translateButton(uint x11Button) const;

  // [X11 ColorMap]

  bool createColormap();
  uint allocRGB(XColormap colormap, int nr, int ng, int nb, uint8_t* palconv);
  void freeRGB(XColormap colormap, const uint8_t* palconv, uint count);

  // [X11 Libraries]

  //! @brief Xlib library object (dynamically opened libX11 library).
  Library _xlib;
  //! @brief Xext library object (dynamically opened libXext library).
  Library _xext;
  //! @brief Xrender library object (dynamically opened libXrender library).
  Library _xrender;

  FOG_INLINE Library& xlib(void) { return _xlib; }
  FOG_INLINE Library& xext(void) { return _xext; }
  FOG_INLINE Library& xrender(void) { return _xrender; }

  // [X11 API]

  enum { XlibFunctionsCount = 76 };
  enum { XextFunctionsCount = 5 };
  enum { XrenderFunctionsCount = 13 };

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

      XID (*pXRootWindow)(Display*, int /* screenNumber */);
      int (*pXDisplayWidth)(Display*, int /* screenNumber */);
      int (*pXDisplayHeight)(Display*, int /* screenNumber */);
      int (*pXDefaultScreen)(Display*);
      XVisual* (*pXDefaultVisual)(Display*, int /* screenNumber */);
      XColormap (*pXDefaultColormap)(Display*, int /* screenNumber */);
      int (*pXDefaultDepth)(Display*, int /* screenNumber */);

      int (*pXPending)(Display*);
      int (*pXNextEvent)(Display*, XEvent* /* xEvent */);
      XBool (*pXCheckTypedEvent)(Display*, int /* eventType */, XEvent* /* eventReturn */);
      XBool (*pXCheckTypedWindowEvent)(Display*, XID /* window */, int /* eventType */, XEvent* /* eventReturn */);
      XBool (*pXCheckWindowEvent)(Display*, XID /* window */, long /* eventMask */, XEvent* /* eventReturn */);

      XIM (*pXOpenIM)(Display*, void* /* rdb */, char* /* resName */, char* /* resClass */);
      XStatus (*pXCloseIM)(XIM /* im */);

      XIC (*pXCreateIC)(XIM /* im */, ...);
      void (*pXDestroyIC)(XIC /* ic */);
      int (*pXRefreshKeyboardMapping)(XMappingEvent* /* eventMap */);

      XID (*pXCreateWindow)(Display*, XID /* parent */, int /* x */, int /* y */, uint /* width */, uint /* height */, uint /* boderWidth */, int /* depth */, uint /* class */, XVisual* /* visual */, ulong /* valueMask */, XSetWindowAttributes* /* attributes */);
      int (*pXDestroyWindow)(Display*, XID /* window */);
      int (*pXMoveWindow)(Display*, XID /* window */, int /* x */, int /* y */);
      int (*pXResizeWindow)(Display*, XID /* window */, uint /* w */, uint /* h */);
      int (*pXMoveResizeWindow)(Display*, XID /* window */, int /* x */, int /* y */, uint /* w */, uint /* h */);
      int (*pXMapWindow)(Display*, XID /* window */);
      int (*pXUnmapWindow)(Display*, XID /* window */);
      int (*pXRaiseWindow)(Display*, XID /* window */);
      int (*pXLowerWindow)(Display*, XID /* window */);
      int (*pXReparentWindow)(Display*, XID /* window */, XID /* parent */, int /* x */, int /* y */);
      int (*pXChangeWindowAttributes)(Display*, XID /* window */, ulong /* valueMask */, XSetWindowAttributes* /* attributes */);
      XStatus (*pXGetWindowAttributes)(Display*, XID /* window */, XWindowAttributes* /* attributesReturns */);

      int (*pXSetInputFocus)(Display*, XID /* window */, int /* revert_to */, XTime /* time */);
      int (*pXGetInputFocus)(Display*, XID* /* window_return */, int* /* revert_to_return */);

      int (*pXSelectInput)(Display* , XID /* window */, long /* event mask */);
      int (*pXSetNormalHints)(Display*, XID /* window */, XSizeHints* /* hints */);

      int (*pXSetWMProtocols)(Display*, XID /* window */, Atom* /* protocols */, int /* count */);
      int (*pXGetWMProtocols)(Display*, XID /* window */, Atom** /* protocolsReturn */, int* /* countReturn */);

      int (*pXTranslateCoordinates)(Display*, XWindow /* src_w */, XWindow /* dest_w */, int /* src_x */, int /* src_y */, int* /* dest_x_return */, int* /* dest_y_return */, XWindow* /* child_return */);
      int (*pXwcTextListToTextProperty)(Display*, wchar_t** /* list */, int /* count */, XICCEncodingStyle /* style */, XTextProperty* /* textPropertyReturn */);

      int (*pXFlush)(Display*);
      int (*pXSync)(Display*, XBool /* discard */);

      GC (*pXCreateGC)(Display*, XID /* drawable */, ulong /* valueMask */, XGCValues* /* values */);
      int (*pXFreeGC)(Display*, GC /* gc */);

      KeySym (*pXKeycodeToKeysym)(Display*, uint /* keyCode */, int /* index */);

      int (*pXLookupString)(XKeyEvent* /* event */, char* /* bufferReturn */, int /* bufferLength */, KeySym* /* keySymReturn */, XComposeStatus* /* statusInOut */);
      int (*pXwcLookupString)(XIC /* ic */, XKeyPressedEvent* /* event */, wchar_t* /* bufferReturn*/, int /* wcharsBuffer */, KeySym* /* keysymReturn */, XStatus* /* statusReturn */);

      int (*pXGrabPointer)(Display*, XID /* grabWindow */, XBool /* ownerEvents */, uint /* eventMask */, int /* pointerMode */, int /* keyboardMode */, XID /* confineTo */, Cursor /* cursor */, Time /* time */);
      int (*pXUngrabPointer)(Display*, XTime /* time */);

      int (*pXGetPointerMapping)(Display*, uchar* /* map_return*/, int /* namp*/);

      XID (*pXCreatePixmap)(Display*, XID /* drawable */, uint /* width */, uint /* height */, uint /* depth */);
      int (*pXFreePixmap)(Display*, XID /* pixmap */);
      int (*pXCopyArea)(Display*, Drawable /* src*/, XID /* dest*/, GC /* gc */, int /* srcX */, int /* srcY */, uint /* width */, uint /* height */, int /* destX */, int /* destY */);
      int (*pXFree)(void* /* data */);

      XImage* (*pXCreateImage)(Display*, XVisual* /* visual */, uint /* depth */, int /* format */, int /* offset */, char* /* data */, uint /* width */, uint /* height */, int /* bitmapPad */, int /* bytesPerLine */);
      int (*pXDestroyImage)(XImage* /* image */);
      int (*pXPutImage)(Display*, XID /* drawable */, GC /* gc */, XImage* /* xImage */, int /* srcX */, int /* srcY */, int /* destX */, int /* destY */, uint /* width */, uint /* height */);

      XColormap (*pXCreateColormap)(Display*, XID /* window */, XVisual* /* visual */, int /* alloc */);
      int (*pXFreeColormap)(Display*, XColormap /* colormap */);

      XStatus (*pXAllocColor)(Display*, XColormap, XColor* /* screen_in_out */);
      int (*pXFreeColors)(Display*, XColormap, ulong* /* pixels */, int /* npixels */, ulong /* planes*/);

      XStatus (*pXMatchVisualInfo)(Display*, int /* screen */, int /* depth */, int /* class */, XVisualInfo* /* vinfo_return */);

      // present in Xutil.h
      void (*pXSetWMName)(Display*, XID /* window */, XTextProperty* /* textProperty */);

      XStatus (*pXSendEvent)(Display*, XID /* window */, XBool, long, XEvent*);
      void (*pXChangeProperty)(Display*, XID /*window */, Atom, Atom, int, int, unsigned char*, int);
      void (*pXSetSelectionOwner)(Display*, Atom, XWindow, XTime);
      char* (*pXGetAtomName)(Display*, Atom);
      void (*pXConvertSelection)(Display*, Atom, Atom, Atom, XWindow, XTime);
      int (*pXGetWindowProperty)(Display*, XWindow, Atom, long, long, XBool, Atom, Atom*, int*, unsigned long*, unsigned long*, unsigned char**);
      void (*pXDeleteProperty)(Display*, XWindow, Atom);
    };

    // Function pointers for loader
    void* xlibFunctions[XlibFunctionsCount];
  };

  union
  {
    struct
    {
      XBool (*pXShmQueryVersion)(Display*, int* /* majorVersion */, int* /* minorVersion */, XBool* /* sharedPixmaps */);
      XStatus (*pXShmAttach)(Display*, XShmSegmentInfo* /* shmInfo */);
      XStatus (*pXShmDetach)(Display*, XShmSegmentInfo* /* shmInfo */);
      XImage* (*pXShmCreateImage)(Display*, XVisual* /* visual */, uint /* depth */, int /* format */, char* /* data */, XShmSegmentInfo* /* shmInfo */, uint /* width */, uint /* height */);
      Pixmap (*pXShmCreatePixmap)(Display*, XID /* drawable */, char* /* data */, XShmSegmentInfo* /* shmInfo */, uint /* width */, uint /* height */, uint /* depth */);
    };

    // Function pointers for loader
    void* xextFunctions[XextFunctionsCount];
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
      XPicture (*pXRenderCreatePicture)(Display*, XID /* drawable */, _Xconst XRenderPictFormat* /* format */, unsigned long /* valuemask */, _Xconst XRenderPictureAttributes* /* attributes */);
      void (*pXRenderChangePicture)(Display*, XPicture /* picture */, unsigned long /* valuemask */, _Xconst XRenderPictureAttributes* /* attributes */);
      void (*pXRenderSetPictureTransform)(Display*, XPicture /* picture */, XTransform* /* transform */);
      void (*pXRenderFreePicture)(Display*, XPicture /* picture*/);
      void (*pXRenderComposite)(Display*, int /* op */, XPicture /* src */, XPicture /* mask */, XPicture /* dest */, int /* srcX */, int /* srcY */, int /* maskX */, int /* maskY */, int /* destX */, int /* destY */, unsigned int /* width */, unsigned int /* height */);
      Cursor (*pXRenderCreateCursor)(Display*, XPicture /* source */, unsigned int /* x */, unsigned int /* y */);
    };

    // Function pointers for loader
    void* xrenderFunctions[XrenderFunctionsCount];
  };

  err_t loadLibraries();
};

// ============================================================================
// [Fog::UIWindowX11]
// ============================================================================

struct FOG_UISYSTEM_X11_API UIWindowX11 : public UIWindowDefault
{
  FOG_DECLARE_OBJECT(UIWindowX11, UIWindowDefault)

  typedef XID Handle;

  // [Construction / Destruction]

  UIWindowX11(Widget* widget);
  virtual ~UIWindowX11();

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

  virtual void onX11Event(XEvent* xe);

  // tell window manager that we want's to move our window to our position
  // (it will be not discarded if we will specify PPosition)
  void setMoveableHints();

  //! @brief X Input Context (XIC).
  XIC _xic;

  //! @brief Whether the x11 window is input only (default false)
  bool _inputOnly;
  //! @brief Whether the x11 window received MapRequest and currently is hidden.
  //! (It's requested to be mapped, but parent is hidden).
  bool _mapRequest;

  //! @brief X11 flags.
  uint _xflags;

  enum XFlag {
    XFlag_Configured
  };
};

// ============================================================================
// [Fog::UIBackingStoreX11]
// ============================================================================

//! @brief Provides cacheable buffer for widgets.
struct FOG_UISYSTEM_X11_API UIBackingStoreX11 : public UIBackingStore
{
  UIBackingStoreX11();
  virtual ~UIBackingStoreX11();

  virtual bool resize(int width, int height, bool cache);
  virtual void destroy();
  virtual void updateRects(const Box* rects, sysuint_t count);

  void blitRects(XID target, const Box* rects, sysuint_t count);

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

// ============================================================================
// [Fog::EventPumpX11]
// ============================================================================

//! @brief Default event pump that not uses operating system (or UI system)
//! dependent event loop.
struct FOG_UISYSTEM_X11_API EventPumpX11 : public EventPump
{
public:
  EventPumpX11();
  virtual ~EventPumpX11();

  // EventPump methods:
  virtual void run(Delegate* delegate);
  virtual void quit();
  virtual void scheduleWork();
  virtual void scheduleDelayedWork(const Time& delayedWorkTime);

  void doRunLoop();
  void waitForWork();
  bool xsync();
  bool processNextXEvent();

protected:
  struct RunState
  {
    Delegate* delegate;

    //! @brief Used to flag that the current run() invocation should return
    //! ASAP.
    bool shouldQuit;

    //! @brief Used to count how many run() invocations are on the stack.
    int runDepth;
  };

  void sendWakeUp();

private:
  Atomic<int> _wakeUpSent;

  //! @brief The time at which we should call @c scheduleDelayedWork().
  Time _delayedWorkTime;

  //! @brief State for the current invocation of run().
  RunState* _state;
};

// ============================================================================
// [Fog::EventLoopX11]
// ============================================================================

struct FOG_UISYSTEM_X11_API EventLoopX11 : public EventLoop
{
  EventLoopX11();
};

} // Fog namespace

// [Guard]
#endif // _FOG_UI_UISYSTEM_X11_H
