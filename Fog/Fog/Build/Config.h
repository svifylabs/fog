// [Fog Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_BUILD_CONFIG_H
#define _FOG_BUILD_CONFIG_H

// [Library Directories]

//! @brief Install prefix specified by configure parameters.
#define FOG_INSTALL_PREFIX "C:/Program Files/Fog"

// [Debugging]

/* #undef DEBUG */
/* #undef DEBUG_MEMORY */

// [Byte Order]

//! @brief Target byteorder, can be: 
//!   - @c FOG_LITTLE_ENDIAN
//!   - @c FOG_BIG_ENDIAN. 
#define FOG_BYTE_ORDER 1

// [Size of Types]

#define FOG_SIZEOF_VOID 4
#define FOG_SIZEOF_SHORT 2
#define FOG_SIZEOF_INT 4
#define FOG_SIZEOF_LONG 4
#define FOG_SIZEOF_WCHAR_T 2

// [Dynamic Loadable Library Support]

//! @brief Defined if we are using Windows dynamic library loader.
#define FOG_LIBRARY_WINDOWS
//! @brief Defined if we are using Linux @c dlopen() dynamic library loader.
/* #undef FOG_LIBRARY_DL */

// [Header files]

//! @brief Defined if we have fenv.h
/* #undef FOG_HAVE_FENV_H */
//! @brief Defined if we have float.h
#define FOG_HAVE_FLOAT_H
//! @brief Defined if we have limits.h
#define FOG_HAVE_LIMITS_H
//! @brief Defined if we have stdint.h
/* #undef FOG_HAVE_STDINT_H */
//! @brief Defined if we have unistd.h
/* #undef FOG_HAVE_UNISTD_H */

//! @brief Defined if we have jpeglib.h
/* #undef FOG_HAVE_JPEGLIB_H */
//! @brief Defined if we have png.h
/* #undef FOG_HAVE_PNG_H */
//! @brief Have fontconfig header files.
/* #undef FOG_HAVE_FONTCONFIG */

// [MMX/SSE support]

//! @brief Build Fog with MMX support
/* #undef FOG_BUILD_MMX */
//! @brief Build Fog with MMXExt support
/* #undef FOG_BUILD_MMXEXT */
//! @brief Build Fog with SSE support
/* #undef FOG_BUILD_SSE */
//! @brief Build Fog with SSE2 support
/* #undef FOG_BUILD_SSE2 */

// [Graphics]

//! @brief Used if freetype library is located (can be used together with windows font subsystem).
/* #undef FOG_FONT_FREETYPE */
//! @brief Used for windows font subsystem.
#define FOG_FONT_WINDOWS

// [UI]

//! @brief Defined if we are using Windows GUI.
#define FOG_UI_WINDOWS
//! @brief Defined if we are using X Window System.
/* #undef FOG_UI_X11 */

// [Guard]
#endif // _FOG_BUILD_CONFIG_H
