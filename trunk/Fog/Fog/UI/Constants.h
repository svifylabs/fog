// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_UI_CONSTANTS_H
#define _FOG_UI_CONSTANTS_H

// [Dependencies]
#include <Fog/Build/Build.h>

// TODO: Move to Fog/UI/Error.h
namespace Error {

enum UIError {
  FailedToCreateUISystem,
  FailedToCreateUIWindow,
  FailedToTranslateCoordinates,
  UISystemNotExists,
  UISystemX11_CantLoadX11,
  UISystemX11_CantLoadX11Symbol,
  UISystemX11_CantLoadXext,
  UISystemX11_CantLoadXextSymbol,
  UISystemX11_CantLoadXrender,
  UISystemX11_CantLoadXrenderSymbol,
  UISystemX11_CantOpenDisplay,
  UISystemX11_CantCreateColormap,
  UISystemX11_TextListToTextPropertyFailed,
  UIWindowAlreadyExists
};

} // Error namespace

namespace Fog {

//! @brief Keys.
enum KeyEnum
{
  KeyBackspace       = 8,
  KeyTab             = 9,
  KeyClear           = 12,
  KeyEnter           = 13,
  KeyPause           = 19,
  KeyEsc             = 27,
  KeySpace           = 32,
  KeyExclaim         = 33,
  KeyQuotedbl        = 34,
  KeyHash            = 35,
  KeyDollar          = 36,
  KeyPercent         = 37,
  KeyAmpersand       = 38,
  KeyQuote           = 39,
  KeyLeftparen       = 40,
  KeyRightparen      = 41,
  KeyAsterisk        = 42,
  KeyPlus            = 43,
  KeyComma           = 44,
  KeyMinus           = 45,
  KeyPeriod          = 46,
  KeySlash           = 47,
  Key0               = 48,
  Key1               = 49,
  Key2               = 50,
  Key3               = 51,
  Key4               = 52,
  Key5               = 53,
  Key6               = 54,
  Key7               = 55,
  Key8               = 56,
  Key9               = 57,
  KeyColon           = 58,
  KeySemicolon       = 59,
  KeyLess            = 60,
  KeyEquals          = 61,
  KeyGreater         = 62,
  KeyQuestion        = 63,
  KeyAt              = 64,
  KeyLeftbracket     = 91,
  KeyBackslash       = 92,
  KeyRightbracket    = 93,
  KeyCaret           = 94,
  KeyUnderscore      = 95,
  KeyBackquote       = 96,
  KeyA               = 97,
  KeyB               = 98,
  KeyC               = 99,
  KeyD               = 100,
  KeyE               = 101,
  KeyF               = 102,
  KeyG               = 103,
  KeyH               = 104,
  KeyI               = 105,
  KeyJ               = 106,
  KeyK               = 107,
  KeyL               = 108,
  KeyM               = 109,
  KeyN               = 110,
  KeyO               = 111,
  KeyP               = 112,
  KeyQ               = 113,
  KeyR               = 114,
  KeyS               = 115,
  KeyT               = 116,
  KeyU               = 117,
  KeyV               = 118,
  KeyW               = 119,
  KeyX               = 120,
  KeyY               = 121,
  KeyZ               = 122,
  KeyLeftBrace       = 123,
  KeyPipe            = 124,
  KeyRightBrace      = 125,
  KeyTilde           = 126,
  KeyDelete          = 127,
  KeyKP0             = 256,
  KeyKP1             = 257,
  KeyKP2             = 258,
  KeyKP3             = 259,
  KeyKP4             = 260,
  KeyKP5             = 261,
  KeyKP6             = 262,
  KeyKP7             = 263,
  KeyKP8             = 264,
  KeyKP9             = 265,
  KeyKPPeriod        = 266,
  KeyKPDivide        = 267,
  KeyKPMultiply      = 268,
  KeyKPMinus         = 269,
  KeyKPPlus          = 270,
  KeyKPEnter         = 271,
  KeyKPEquals        = 272,
  KeyUp              = 273,
  KeyDown            = 274,
  KeyRight           = 275,
  KeyLeft            = 276,
  KeyInsert          = 277,
  KeyHome            = 278,
  KeyEnd             = 279,
  KeyPageUp          = 280,
  KeyPageDown        = 281,
  KeyF1              = 282,
  KeyF2              = 283,
  KeyF3              = 284,
  KeyF4              = 285,
  KeyF5              = 286,
  KeyF6              = 287,
  KeyF7              = 288,
  KeyF8              = 289,
  KeyF9              = 290,
  KeyF10             = 291,
  KeyF11             = 292,
  KeyF12             = 293,
  KeyF13             = 294,
  KeyF14             = 295,
  KeyF15             = 296,
  KeyNumLock         = 300,
  KeyCapsLock        = 301,
  KeyScrollLock      = 302,
  KeyRightShift      = 303,
  KeyLeftShift       = 304,
  KeyRightCtrl       = 305,
  KeyLeftCtrl        = 306,
  KeyRightAlt        = 307,
  KeyLeftAlt         = 308,
  KeyRightMeta       = 309,
  KeyLeftMeta        = 310,
  KeyLeftSuper       = 311, //!< @brief Left "Windows" key.
  KeyRightSuper      = 312, //!< @brief Right "Windows" key.
  KeyMode            = 313, //!< @brief "Alt Gr" key.
  KeyHelp            = 315,
  KeyPrint           = 316,
  KeySys_Req         = 317,
  KeyBreak           = 318,
  KeyMenu            = 319,
  KeyPower           = 320, //!< @brief Macintosh power key.
  KeyEuro            = 321, //!< @brief Some european keyboards.
  KeyAlpha           = 322, //!< @brief Selects letters on a numeric keypad (for celphones and similar devices).

  KeyCompose         = 400,

  KeyCount           = 401,
  KeyMask            = 0x1FFFFFFF,

  // Modifiers mapped to key
  KeyShift           = (1 << 29),
  KeyCtrl            = (1 << 30),
  KeyAlt             = (1 << 31)
};

//! @brief Key modifiers.
enum ModifierEnum
{
  ModifierLeftShift    = 0x0001, //!< @brief Left shift.
  ModifierRightShift   = 0x0002, //!< @brief Right shift.
  ModifierShift        = 0x0003, //!< @brief Left or right shift, use if (mod & W_MODIFIED_SHIFT).

  ModifierLeftCtrl     = 0x0004, //!< @brief Left control.
  ModifierRightCtrl    = 0x0008, //!< @brief Right control.
  ModifierCtrl         = 0x000C, //!< @brief Left or right control.

  ModifierLeftAlt      = 0x0010, //!< @brief Left alt.
  ModifierRightAlt     = 0x0020, //!< @brief Right alt.
  ModifierAlt          = 0x0030, //!< @brief Left or right alt.

  ModifierLeftMeta     = 0x0040, //!< @brief Left meta.
  ModifierRightMeta    = 0x0080, //!< @brief Right meta.
  ModifierMeta         = 0x00C0, //!< @brief Left or right meta.

  ModifierMask         = 0x00FF, //!< @brief Left, Shift, Alt and Meta masks.

  ModifierNum          = 0x1000, //!< @brief Num lock.
  ModifierCaps         = 0x2000, //!< @brief Caps lock.
  ModifierMode         = 0x4000  //!< @brief Mode.
};

//! @brief Pointer buttons.
enum ButtonEnum
{
  ButtonLeft           = 0x0001, //!< @brief Left button.
  ButtonMiddle         = 0x0002, //!< @brief Middle button.
  ButtonRight          = 0x0004, //!< @brief Right button.
  ButtonInvalid        = 0xFFFF  //!< @brief Invalib button (used internally).
};

enum WheelEnum
{
  WheelUp              = 0x0008, //!< @brief Wheel up button.
  WheelDown            = 0x0010  //!< @brief Wheel down button.
};

enum OrientationEnum
{
  OrientationHorizontal= 0,      //!< @brief Wheel down button.
  OrientationVertical  = 1       //!< @brief Wheel up button.
};

enum CaretTypeEnum
{
  CaretTypeNormal      = 0,      //!< @brief Normal type of caret
  CaretTypeOverwrite   = 1,      //!< @brief Overwrite type of caret.
};

//! @brief Text alignment
enum TextAlignEnum
{
  TextAlignLeft        = 0x1,
  TextAlignRight       = 0x2,
  TextAlignHCenter     = 0x3,
  TextAlignTop         = 0x10,
  TextAlignBottom      = 0x20,
  TextAlignVCenter     = 0x30,
  TextAlignCenter      = TextAlignVCenter | TextAlignHCenter
};

enum FocusReasonEnum
{
  FocusReasonNone = 0,
  FocusReasonMouse,
  FocusReasonWheel,
  FocusReasonTab,
  FocusReasonBackTab
};

//! @brief Wde_Gui init failcodes.
enum InitResult_Gui
{
  // 1 - 31 reserved for Wde_Core, but Wde_Code should always success at
  // initialization

  // X failure

  InitResult_CantLoadXLib = 32,
  InitResult_CantLoadXLibSymbol = 33,
  InitResult_CantLoadXextLib = 34,
  InitResult_CantLoadXextLibSymbol = 35,
  InitResult_CantLoadXrenderLib = 36,
  InitResult_CantLoadXrenderLibSymbol = 37,
  InitResult_CantOpenXDisplay = 38,
  InitResult_CantCreateColormap = 39,
  InitResult_CantCreateAccureColormap = 40,

  // Win failure

  InitResult_CantRegisterClass = 41,
  InitResult_CantCreateMsgWindow = 42,

  // Font failure

  InitResult_CantLoadFreetype = 43,
  InitResult_CantLoadFreetypeSymbol = 44,
  InitResult_CantInitializeFreetype = 45,
  InitResult_CantLoadDefaultFontFace = 46
};

static FOG_INLINE bool isClearMod   (uint32_t mod) { return (mod & ModifierMask  ) == 0; }
static FOG_INLINE bool isAltMod     (uint32_t mod) { return (mod & ModifierAlt   ) != 0; }
static FOG_INLINE bool isAltOnly    (uint32_t mod) { return (mod & ModifierMask  ) == (mod & ModifierAlt); }
static FOG_INLINE bool isShiftMod   (uint32_t mod) { return (mod & ModifierShift ) != 0; }
static FOG_INLINE bool isShiftOnly  (uint32_t mod) { return (mod & ModifierMask  ) == (mod & ModifierShift); }
static FOG_INLINE bool isCtrlMod    (uint32_t mod) { return (mod & ModifierCtrl  ) != 0; }
static FOG_INLINE bool isCtrlOnly   (uint32_t mod) { return (mod & ModifierMask  ) == (mod & ModifierCtrl); }
static FOG_INLINE bool isCapsLockMod(uint32_t mod) { return (mod & ModifierCaps  ) != 0; }
static FOG_INLINE bool isNumLockMod (uint32_t mod) { return (mod & ModifierNum   ) != 0; }
static FOG_INLINE bool isModeMod    (uint32_t mod) { return (mod & ModifierMode  ) != 0; }

} // Fog namespace


// [Guard]
#endif // _FOG_UI_CONSTANTS_H
