// [Fog/Gui Library - Public API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GUI_CONSTANTS_H
#define _FOG_GUI_CONSTANTS_H

// [Dependencies]
#include <Fog/Build/Build.h>

//! @addtogroup Fog_Gui
//! @{

namespace Fog {

// ============================================================================
// [Fog::WIDGET_STATE]
// ============================================================================

//! @brief Widget state.
enum WIDGET_STATE
{
  //! @brief Widget is disabled.
  WIDGET_DISABLED = 0,
  //! @brief Widget is enabled, but parent is disabled and this makes control disabled.
  WIDGET_DISABLED_BY_PARENT = 1,
  //! @brief Widget is enabled and it parent too.
  WIDGET_ENABLED = 2
};

// ============================================================================
// [Fog::WIDGET_VISIBILITY]
// ============================================================================

//! @brief Widget visibility
enum WIDGET_VISIBILITY
{
  //! @brief Widget is hidden.
  WIDGET_HIDDEN = 0,
  //! @brief Widget is hidden by parent that's not visible.
  WIDGET_HIDDEN_BY_PARENT = 1,
  //! @brief Widget is visible.
  WIDGET_VISIBLE = 2,
};

// ============================================================================
// [Fog::FOCUSABLE]
// ============================================================================

enum FOCUSABLE
{
  FOCUSABLE_NEXT = 0,
  FOCUSABLE_PREVIOUS = 1
};

// ============================================================================
// [Fog::FOCUS_MASK]
// ============================================================================

//! @brief Focus masks used by @c FOCUS_POLICY
enum FOCUS_MASK
{
  FOCUS_MASK_TAB = (1 << 0),
  FOCUS_MASK_CLICK = (1 << 1),
  FOCUS_MASK_WHEEL = (1 << 2)
};

// ============================================================================
// [Fog::FOCUS_POLICY]
// ============================================================================

//! @brief Focus policy
enum FOCUS_POLICY
{
  FOCUS_NONE = 0,
  FOCUS_TAB = FOCUS_MASK_TAB,
  FOCUS_CLICK = FOCUS_MASK_CLICK,
  FOCUS_STRONG = FOCUS_TAB | FOCUS_CLICK,
  FOCUS_WHEEL = FOCUS_STRONG | FOCUS_MASK_WHEEL
};

// ============================================================================
// [Fog::LAYOUT_POLICY]
// ============================================================================

enum LAYOUT_POLICY
{
  LAYOUT_EXPANDING_WIDTH = 0x01,
  LAYOUT_EXPANDING_HEIGHT = 0x10
};

// ============================================================================
// [Fog::KEY_CODE]
// ============================================================================

//! @brief Key codes.
enum KEY_CODE
{
  KEY_BACKSPACE            = 8,
  KEY_TAB                  = 9,
  KEY_CLEAR                = 12,
  KEY_ENTER                = 13,
  KEY_PAUSE                = 19,
  KEY_ESC                  = 27,
  KEY_SPACE                = 32,
  KEY_EXCLAIM              = 33,
  KEY_DOUBLEQUOTE          = 34,
  KEY_HASH                 = 35,
  KEY_DOLLAR               = 36,
  KEY_PERCENT              = 37,
  KEY_AMPERSAND            = 38,
  KEY_QUOTE                = 39,
  KEY_LEFT_PAREN           = 40,
  KEY_RIGHT_PAREN          = 41,
  KEY_ASTERISK             = 42,
  KEY_PLUS                 = 43,
  KEY_COMMA                = 44,
  KEY_MINUS                = 45,
  KEY_PERIOD               = 46,
  KEY_SLASH                = 47,
  KEY_0                    = 48,
  KEY_1                    = 49,
  KEY_2                    = 50,
  KEY_3                    = 51,
  KEY_4                    = 52,
  KEY_5                    = 53,
  KEY_6                    = 54,
  KEY_7                    = 55,
  KEY_8                    = 56,
  KEY_9                    = 57,
  KEY_COLON                = 58,
  KEY_SEMICOLON            = 59,
  KEY_LESS                 = 60,
  KEY_EQUALS               = 61,
  KEY_GREATER              = 62,
  KEY_QUESTION             = 63,
  KEY_AT                   = 64,
  KEY_LEFT_BRACKET         = 91,
  KEY_BACKSLASH            = 92,
  KEY_RIGHT_BRACKET        = 93,
  KEY_CARET                = 94,
  KEY_UNDERSCORE           = 95,
  KEY_BACKQUOTE            = 96,
  KEY_A                    = 97,
  KEY_B                    = 98,
  KEY_C                    = 99,
  KEY_D                    = 100,
  KEY_E                    = 101,
  KEY_F                    = 102,
  KEY_G                    = 103,
  KEY_H                    = 104,
  KEY_I                    = 105,
  KEY_J                    = 106,
  KEY_K                    = 107,
  KEY_L                    = 108,
  KEY_M                    = 109,
  KEY_N                    = 110,
  KEY_O                    = 111,
  KEY_P                    = 112,
  KEY_Q                    = 113,
  KEY_R                    = 114,
  KEY_S                    = 115,
  KEY_T                    = 116,
  KEY_U                    = 117,
  KEY_V                    = 118,
  KEY_W                    = 119,
  KEY_X                    = 120,
  KEY_Y                    = 121,
  KEY_Z                    = 122,
  KEY_LEFT_BRACE           = 123,
  KEY_PIPE                 = 124,
  KEY_RIGHT_BRACE          = 125,
  KEY_TILDE                = 126,
  KEY_DELETE               = 127,
  KEY_KP_0                 = 256,
  KEY_KP_1                 = 257,
  KEY_KP_2                 = 258,
  KEY_KP_3                 = 259,
  KEY_KP_4                 = 260,
  KEY_KP_5                 = 261,
  KEY_KP_6                 = 262,
  KEY_KP_7                 = 263,
  KEY_KP_8                 = 264,
  KEY_KP_9                 = 265,
  KEY_KP_PERIOD            = 266,
  KEY_KP_DIVIDE            = 267,
  KEY_KP_MULTIPLY          = 268,
  KEY_KP_MINUS             = 269,
  KEY_KP_PLUS              = 270,
  KEY_KP_ENTER             = 271,
  KEY_KP_EQUALS            = 272,
  KEY_UP                   = 273,
  KEY_DOWN                 = 274,
  KEY_RIGHT                = 275,
  KEY_LEFT                 = 276,
  KEY_INSERT               = 277,
  KEY_HOME                 = 278,
  KEY_END                  = 279,
  KEY_PAGE_UP              = 280,
  KEY_PAGE_DOWN            = 281,
  KEY_F1                   = 282,
  KEY_F2                   = 283,
  KEY_F3                   = 284,
  KEY_F4                   = 285,
  KEY_F5                   = 286,
  KEY_F6                   = 287,
  KEY_F7                   = 288,
  KEY_F8                   = 289,
  KEY_F9                   = 290,
  KEY_F10                  = 291,
  KEY_F11                  = 292,
  KEY_F12                  = 293,
  KEY_F13                  = 294,
  KEY_F14                  = 295,
  KEY_F15                  = 296,
  KEY_NUM_LOCK             = 300,
  KEY_CAPS_LOCK            = 301,
  KEY_SCROLL_LOCK          = 302,
  KEY_RIGHT_SHIFT          = 303,
  KEY_LEFT_SHIFT           = 304,
  KEY_RIGHT_CTRL           = 305,
  KEY_LEFT_CTRL            = 306,
  KEY_RIGHT_ALT            = 307,
  KEY_LEFT_ALT             = 308,
  KEY_RIGHT_META           = 309,
  KEY_LEFT_META            = 310,
  KEY_LEFT_SUPER           = 311, //!< @brief Left "Windows" key.
  KEY_RIGHT_SUPER          = 312, //!< @brief Right "Windows" key.
  KEY_MODE                 = 313, //!< @brief "Alt Gr" key.
  KEY_HELP                 = 315,
  KEY_PRINT                = 316,
  KEY_SYS_REQ              = 317,
  KEY_BREAK                = 318,
  KEY_MENU                 = 319,
  KEY_POWER                = 320, //!< @brief Macintosh power key.
  KEY_EURO                 = 321, //!< @brief Some european keyboards.
  KEY_ALPHA                = 322, //!< @brief Selects letters on a numeric keypad (for celphones and similar devices).

  KEY_COMPOSE              = 400,

  KEY_COUNT                = 401,
  KEY_MASK                 = 0x1FFFFFFF,

  // Modifiers mapped to key
  KEY_SHIFT                = (1 << 29),
  KEY_CTRL                 = (1 << 30),
  KEY_ALT                  = (1 << 31)
};

// ============================================================================
// [Fog::MODIFIER_CODE]
// ============================================================================

//! @brief Key modifier codes.
enum MODIFIER_CODE
{
  MODIFIER_LEFT_SHIFT      = 0x0001, //!< @brief Left shift.
  MODIFIER_RIGHT_SHIFT     = 0x0002, //!< @brief Right shift.
  MODIFIER_SHIFT           = 0x0003, //!< @brief Left or right shift, use if (mod & MODIFIER_SHIFT).

  MODIFIER_LEFT_CTRL       = 0x0004, //!< @brief Left control.
  MODIFIER_RIGHT_CTRL      = 0x0008, //!< @brief Right control.
  MODIFIER_CTRL            = 0x000C, //!< @brief Left or right control.

  MODIFIER_LEFT_ALT        = 0x0010, //!< @brief Left alt.
  MODIFIER_RIGHT_ALT       = 0x0020, //!< @brief Right alt.
  MODIFIER_ALT             = 0x0030, //!< @brief Left or right alt.

  MODIFIER_LEFT_META       = 0x0040, //!< @brief Left meta.
  MODIFIER_RIGHT_META      = 0x0080, //!< @brief Right meta.
  MODIFIER_META            = 0x00C0, //!< @brief Left or right meta.

  MODIFIER_MASK            = 0x00FF, //!< @brief Left, Shift, Alt and Meta masks.

  MODIFIER_NUM             = 0x1000, //!< @brief Num lock.
  MODIFIER_CAPS            = 0x2000, //!< @brief Caps lock.
  MODIFIER_MODE            = 0x4000  //!< @brief Mode.
};

static FOG_INLINE bool isClearMod   (uint32_t mod) { return (mod & MODIFIER_MASK  ) == 0; }
static FOG_INLINE bool isAltMod     (uint32_t mod) { return (mod & MODIFIER_ALT   ) != 0; }
static FOG_INLINE bool isAltOnly    (uint32_t mod) { return (mod & MODIFIER_MASK  ) == (mod & MODIFIER_ALT); }
static FOG_INLINE bool isShiftMod   (uint32_t mod) { return (mod & MODIFIER_SHIFT ) != 0; }
static FOG_INLINE bool isShiftOnly  (uint32_t mod) { return (mod & MODIFIER_MASK  ) == (mod & MODIFIER_SHIFT); }
static FOG_INLINE bool isCtrlMod    (uint32_t mod) { return (mod & MODIFIER_CTRL  ) != 0; }
static FOG_INLINE bool isCtrlOnly   (uint32_t mod) { return (mod & MODIFIER_MASK  ) == (mod & MODIFIER_CTRL); }
static FOG_INLINE bool isCapsLockMod(uint32_t mod) { return (mod & MODIFIER_CAPS  ) != 0; }
static FOG_INLINE bool isNumLockMod (uint32_t mod) { return (mod & MODIFIER_NUM   ) != 0; }
static FOG_INLINE bool isModeMod    (uint32_t mod) { return (mod & MODIFIER_MODE  ) != 0; }

// ============================================================================
// [Fog::BUTTON_CODE]
// ============================================================================

//! @brief Mouse button codes.
enum BUTTON_CODE
{
  BUTTON_LEFT              = 0x0001, //!< @brief Left button.
  BUTTON_MIDDLE            = 0x0002, //!< @brief Middle button.
  BUTTON_RIGHT             = 0x0004, //!< @brief Right button.
  BUTTON_INVALID           = 0xFFFF  //!< @brief Invalid button (used internally).
};

// ============================================================================
// [Fog::WHEEL_CODE]
// ============================================================================

enum WHEEL_CODE
{
  WHEEL_UP                 = 0x0008, //!< @brief Wheel up button.
  WHEEL_DOWN               = 0x0010  //!< @brief Wheel down button.
};

// ============================================================================
// [Fog::ORIENTATION]
// ============================================================================

enum ORIENTATION
{
  ORIENTATION_HORIZONTAL   = 0,      //!< @brief Wheel down button.
  ORIENTATION_VERTICAL     = 1       //!< @brief Wheel up button.
};

// ============================================================================
// [Fog::CARET_TYPE]
// ============================================================================

enum CARET_TYPE
{
  CARET_NORMAL             = 0,      //!< @brief Normal type of caret
  CARET_OVERWRITE          = 1,      //!< @brief Overwrite type of caret.
};

// ============================================================================
// [Fog::FOCUS_REASON]
// ============================================================================

enum FOCUS_REASON
{
  FOCUS_REASON_NONE = 0,
  FOCUS_REASON_MOUSE,
  FOCUS_REASON_WHEEL,
  FOCUS_REASON_TAB,
  FOCUS_REASON_BACK_TAB
};

// ============================================================================
// [Fog::CheckedState]
// ============================================================================

enum CHECKED_STATE
{
  CHECKED_OFF = 0,
  CHECKED_ON = 1,
  CHECKED_ALTERNATE = 2
};

// ============================================================================
// [Fog::Event IDs]
// ============================================================================

enum EV_GUI_ENUM
{
  // [ChildEvent]

  EV_CHILD_ADD = 100,
  EV_CHILD_REMOVE,

  // [LayoutEvent]

  EV_LAYOUT_SET,
  EV_LAYOUT_REMOVE,

  EV_LAYOUT_ITEM_ADD,
  EV_LAYOUT_ITEM_REMOVE,

  // [StateEvent]

  EV_ENABLE,
  EV_DISABLE,
  EV_DISABLE_BY_PARENT,

  // [VisibilityEvent]

  EV_SHOW,
  EV_HIDE,
  EV_HIDE_BY_PARENT,

  // [ConfigureEvent]

  EV_CONFIGURE,

  // [OriginEvent]

  EV_ORIGIN,

  // [FocusEvent]

  EV_FOCUS_IN,
  EV_FOCUS_OUT,

  // [KeyEvent]

  EV_KEY_PRESS,
  EV_KEY_RELEASE,

  // [MouseEvent]

  EV_MOUSE_IN,
  EV_MOUSE_OUT,
  EV_MOUSE_MOVE,
  EV_MOUSE_PRESS,
  EV_MOUSE_RELEASE,
  EV_CLICK,
  EV_DOUBLE_CLICK,
  EV_WHEEL,

  // [SelectionEvent]

  EV_CLEAR_SELECTION,
  EV_SELECTION_REQUIRED,

  // [PaintEvent]

  EV_PAINT,

  // [CloseEvent]

  EV_CLOSE,

  // [CheckEvent]

  EV_CHECK,
  EV_UNCHECK,

  // [ThemeEvent]

  EV_THEME
};

// ============================================================================
// [Fog::ERR_GUI]
// ============================================================================

//! @brief Error codes used in Fog/Gui.
enum ERR_GUI_ENUM
{
  // Errors Range.
  ERR_GUI_START = 0x00011200,
  ERR_GUI_LAST  = 0x000112FF,

  // UI Errors.
  ERR_GUI_NOT_INITIALIZED = ERR_GUI_START,
  ERR_GUI_INTERNAL_ERROR,

  ERR_GUI_CANT_CREATE_UIENGINE,
  ERR_GUI_CANT_CREATE_UIWINDOW,

  ERR_GUI_LIBX11_NOT_LOADED,
  ERR_GUI_LIBEXT_NOT_LOADED,
  ERR_GUI_LIBXRENDER_NOT_LOADED,

  ERR_GUI_CANT_OPEN_DISPLAY,
  ERR_GUI_CANT_CREATE_PIPE,
  ERR_GUI_CANT_CREATE_COLORMAP,
  ERR_GUI_CANT_TRANSLETE_COORDINATES,

  ERR_GUI_WINDOW_ALREADY_EXISTS
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GUI_CONSTANTS_H
