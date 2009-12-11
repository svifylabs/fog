Fog-Framework - Version SVN
===========================

http://code.google.com/p/fog/

Introduction
============

Fog library is library for crossplatform development in C++ language. Main goal
of the Fog library is to provide very fast and accurate graphics output on all 
supported operating systems and architectures. Currently Fog is not only graphics 
library, it contains many other classes related to crossplatform console and UI 
development.

Library dependencies
====================

Windows:
- Fog/Core
  - Advapi32   (direct   - linked)
  - Winmm      (direct   - linked)
- Fog/Graphics
  - Gdi32      (direct   - linked)
  - GdiPlus    (optional - runtime loading)
- Fog/UI
  - Gdi32      (direct   - linked)
  - User32     (direct   - linked)

Linux:
- Fog/Core
  - pthreads   (direct   - linked)
  - libdl      (direct   - linked)
  - librt      (direct   - linked)
- Fog/Graphics
  - Fontconfig (optional - runtime loading)
  - Freetype2  (direct   - runtime loading)
- Fog/UI
  - X11        (optional - runtime loading)

Support:
- Fog/Graphics
  - libjpeg    (optional - runtime loading)
  - libpng     (optional - runtime loading)
