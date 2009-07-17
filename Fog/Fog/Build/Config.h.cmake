// [Fog Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_BUILD_CONFIG_H
#define _FOG_BUILD_CONFIG_H

// [Library Directories]

//! @brief Install prefix specified by configure parameters.
#define FOG_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}"

// [Debugging]

#cmakedefine FOG_DEBUG
#cmakedefine FOG_DEBUG_MEMORY

// [Byte Order]

//! @brief Target byteorder, can be: 
//!   - @c FOG_LITTLE_ENDIAN
//!   - @c FOG_BIG_ENDIAN. 
#define FOG_BYTE_ORDER ${FOG_BYTE_ORDER}

// [Size of Types]

#define FOG_SIZEOF_VOID ${FOG_SIZEOF_VOID}
#define FOG_SIZEOF_SHORT ${FOG_SIZEOF_SHORT}
#define FOG_SIZEOF_INT ${FOG_SIZEOF_INT}
#define FOG_SIZEOF_LONG ${FOG_SIZEOF_LONG}
#define FOG_SIZEOF_WCHAR_T ${FOG_SIZEOF_WCHAR_T}

// [Dynamic Loadable Library Support]

//! @brief Defined if we are using Windows dynamic library loader.
#cmakedefine FOG_LIBRARY_WINDOWS
//! @brief Defined if we are using Linux @c dlopen() dynamic library loader.
#cmakedefine FOG_LIBRARY_DL

// [Header files]

//! @brief Defined if we have fenv.h
#cmakedefine FOG_HAVE_FENV_H
//! @brief Defined if we have float.h
#cmakedefine FOG_HAVE_FLOAT_H
//! @brief Defined if we have limits.h
#cmakedefine FOG_HAVE_LIMITS_H
//! @brief Defined if we have stdint.h
#cmakedefine FOG_HAVE_STDINT_H
//! @brief Defined if we have unistd.h
#cmakedefine FOG_HAVE_UNISTD_H

//! @brief Defined if we have jpeglib.h
#cmakedefine FOG_HAVE_JPEGLIB_H
//! @brief Defined if we have png.h
#cmakedefine FOG_HAVE_PNG_H
//! @brief Have fontconfig header files.
#cmakedefine FOG_HAVE_FONTCONFIG

// [MMX/SSE support]

//! @brief Build Fog with MMX support
#cmakedefine FOG_BUILD_MMX
//! @brief Build Fog with MMXExt support
#cmakedefine FOG_BUILD_MMXEXT
//! @brief Build Fog with SSE support
#cmakedefine FOG_BUILD_SSE
//! @brief Build Fog with SSE2 support
#cmakedefine FOG_BUILD_SSE2

// [Graphics]

//! @brief Used if freetype library is located (can be used together with windows font subsystem).
#cmakedefine FOG_FONT_FREETYPE
//! @brief Used for windows font subsystem.
#cmakedefine FOG_FONT_WINDOWS

// [UI]

//! @brief Defined if we are using Windows GUI.
#cmakedefine FOG_UI_WINDOWS
//! @brief Defined if we are using X Window System.
#cmakedefine FOG_UI_X11

// [BuiltIn]
#cmakedefine FOG_BUILD_MODULE_X11_EXTERNAL
#cmakedefine FOG_BUILD_MODULE_X11_INTERNAL

#if defined(FOG_BUILD_MODULE_X11_EXTERNAL) && defined(FOG_BUILD_MODULE_X11_INTERNAL)
#error "Can't be defined both FOG_BUILD_MODULE_X11_EXTERNAL and FOG_BUILD_MODULE_X11_INTERNAL."
#endif

// [Guard]
#endif // _FOG_BUILD_CONFIG_H
