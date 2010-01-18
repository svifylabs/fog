// [Fog/UI Library - Public API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_UI_UISYSTEM_X11HEADERS_H
#define _FOG_UI_UISYSTEM_X11HEADERS_H

// [Dependencies]
#include <Fog/Build/Build.h>

// [Guard]
#if defined(FOG_UI_X11)

#ifdef _XLIB_H
#error "You must include Fog/UI/UISystem/X11Headers.h before xlib.h"
#endif

// redefine some critical things

#define Window XWindow
#define Region XRegion
#define Complex XComplex
#define Visual XVisual
#define Colormap XColormap
#define Picture XPicture
#define Time XTime

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#define XK_MISCELLANY
#define XK_LATIN1
#define XK_XKB_KEYS
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
#undef Time

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
#endif // index

#else
#warning "Fog::X11 - Including Fog/UI/UISystem/X11Headers on non-x11 plaform"
#endif // FOG_UI_X11

// [Guard]
#endif // _FOG_UI_UISYSTEM_X11HEADERS_H
