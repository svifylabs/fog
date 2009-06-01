// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// [Guard]
#ifndef _FOG_CORE_SCOPEDHANDLE_H
#define _FOG_CORE_SCOPEDHANDLE_H

// [Dependencies]
#include <Fog/Build/Build.h>

// [Windows Specific]
#if defined(FOG_OS_WINDOWS)

#include <windows.h>

namespace Fog {

// Used so we always remember to close the handle. Example:
//   Fog::ScopedHANDLE hfile(CreateFile(...));
//   if (!hfile.get())
//     ...process error
//   ReadFile(hfile.get(), ...);
//
// To sqirrel the handle away somewhere else:
//   secret_handle_ = hfile.take();
//
// To explicitly close the handle:
//   CloseHandle(hfile.take());
struct ScopedHANDLE
{
public:
  FOG_INLINE ScopedHANDLE() : _handle(NULL) {}
  FOG_INLINE explicit ScopedHANDLE(HANDLE h) : _handle(h) { normalize(); }
  FOG_INLINE ~ScopedHANDLE() { close(); }

  // Use this instead of comparing to INVALID_HANDLE_VALUE to pick up our NULL
  // usage for errors.
  FOG_INLINE bool isValid() const { return _handle != NULL; }

  FOG_INLINE HANDLE get() { return _handle; }
  FOG_INLINE HANDLE take() { HANDLE r = _handle; _handle = 0; return r; }
  FOG_INLINE void free() { close(); _handle = 0; }
  FOG_INLINE void set(HANDLE h) { close(); _handle = h; normalize(); }
  FOG_INLINE operator HANDLE() { return _handle; }

private:
  FOG_INLINE void close()
  {
    if (_handle)
    {
      if (!::CloseHandle(_handle))
      {
        FOG_ASSERT_NOT_REACHED();
      }
    }
  }

  FOG_INLINE void normalize()
  {
    // Windows is inconsistent about invalid handles, so we always use NULL
    if (_handle == INVALID_HANDLE_VALUE) _handle = NULL;
  }

  HANDLE _handle;
  FOG_DISABLE_COPY(ScopedHANDLE)
};

// Like ScopedHANDLE, but for HANDLEs returned from FindFile().
struct ScopedFindFileHANDLE
{
public:
  FOG_INLINE ScopedFindFileHANDLE() : _handle(NULL) {}
  FOG_INLINE explicit ScopedFindFileHANDLE(HANDLE handle) : _handle(handle) { normalize(); }
  FOG_INLINE ~ScopedFindFileHANDLE() { close(); }

  // Use this instead of comparing to INVALID_HANDLE_VALUE to pick up our NULL
  // usage for errors.
  FOG_INLINE bool isValid() const { return _handle != NULL; }

  FOG_INLINE HANDLE get() { return _handle; }
  FOG_INLINE HANDLE take() { HANDLE r = _handle; _handle = 0; return r; }
  FOG_INLINE void free() { close(); _handle = 0; }
  FOG_INLINE void set(HANDLE h) { close(); _handle = h; normalize(); }
  FOG_INLINE operator HANDLE() { return _handle; }

private:
  FOG_INLINE void close()
  {
    if (_handle) ::FindClose(_handle);
  }

  FOG_INLINE void normalize()
  {
    // Windows is inconsistent about invalid handles, so we always use NULL
    if (_handle == INVALID_HANDLE_VALUE) _handle = NULL;
  }

  HANDLE _handle;

  FOG_DISABLE_COPY(ScopedFindFileHANDLE)
};

// Like ScopedHANDLE but for HDC.  Only use this on HDCs returned from
// CreateCompatibleDC.  For an HDC returned by GetDC, use ReleaseDC instead.
struct ScopedHDC
{
public:
  FOG_INLINE ScopedHDC() : _hdc(NULL) {}
  FOG_INLINE explicit ScopedHDC(HDC h) : _hdc(h) {}
  FOG_INLINE ~ScopedHDC() { close(); }

  FOG_INLINE HDC get() { return _hdc; }
  FOG_INLINE HDC take() { HDC r = _hdc; _hdc = 0; return r; }
  FOG_INLINE void free() { close(); _hdc = 0; }
  FOG_INLINE void set(HDC h) { close(); _hdc = h; }
  FOG_INLINE operator HDC() { return _hdc; }

private:
  FOG_INLINE void close()
  {
    if (_hdc) ::DeleteDC(_hdc);
  }

  HDC _hdc;

  FOG_DISABLE_COPY(ScopedHDC)
};

// Like ScopedHANDLE but for HBITMAP.
struct ScopedHBITMAP
{
public:
  FOG_INLINE ScopedHBITMAP() : _hbitmap(NULL) {}
  FOG_INLINE explicit ScopedHBITMAP(HBITMAP h) : _hbitmap(h) {}
  FOG_INLINE ~ScopedHBITMAP() { close(); }

  FOG_INLINE HBITMAP get() { return _hbitmap; }
  FOG_INLINE HBITMAP take() { HBITMAP r = _hbitmap; _hbitmap = 0; return r; }
  FOG_INLINE void free() { close(); _hbitmap = 0; }
  FOG_INLINE void set(HBITMAP h) { close(); _hbitmap = h; }
  FOG_INLINE operator HBITMAP() { return _hbitmap; }

private:
  FOG_INLINE void close()
  {
    if (_hbitmap) ::DeleteObject(_hbitmap);
  }

  HBITMAP _hbitmap;

  FOG_DISABLE_COPY(ScopedHBITMAP)
};

// Like ScopedHANDLE but for HRGN.
struct ScopedHRGN
{
public:
  FOG_INLINE ScopedHRGN() : _hrgn(NULL) {}
  FOG_INLINE explicit ScopedHRGN(HRGN h) : _hrgn(h) {}
  FOG_INLINE ~ScopedHRGN() { close(); }

  FOG_INLINE HRGN get() { return _hrgn; }
  FOG_INLINE HRGN take() { HRGN r = _hrgn; _hrgn = 0; return r; }
  FOG_INLINE void free() { close(); _hrgn = 0; }
  FOG_INLINE void set(HRGN h) { close(); _hrgn = h; }
  FOG_INLINE operator HRGN() { return _hrgn; }

  FOG_INLINE ScopedHRGN& operator=(HRGN hrgn)
  {
    if (_hrgn && hrgn != _hrgn) DeleteObject(_hrgn);
    _hrgn = hrgn;
    return *this;
  }

private:
  FOG_INLINE void close()
  {
    if (_hrgn) ::DeleteObject(_hrgn);
  }

  HRGN _hrgn;

  FOG_DISABLE_COPY(ScopedHRGN)
};

// Like ScopedHANDLE except for HGLOBAL.
template<typename T>
struct ScopedHGLOBAL
{
public:
  FOG_INLINE explicit ScopedHGLOBAL(HGLOBAL glob) : _glob(glob)
  {
    _data = static_cast<T*>(::GlobalLock(_glob));
  }
  FOG_INLINE ~ScopedHGLOBAL()
  {
    ::GlobalUnlock(_glob);
  }

  FOG_INLINE T* get() { return _data; }
  FOG_INLINE sysuint_t size() const { return ::GlobalSize(_glob); }

  FOG_INLINE T* operator->() const 
  {
    FOG_ASSERT(_data != 0);
    return _data;
  }

private:
  HGLOBAL _glob;
  T* _data;

  FOG_DISABLE_COPY(ScopedHGLOBAL)
};

} // Fog namespace

// [Windows Specific]
#endif // FOG_OS_WINDOWS

// [Guard]
#endif // _FOG_CORE_SCOPEDHANDLE_H
