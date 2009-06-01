Fog Library
===========

http://code.google.com/p/fog/

Introduction
============

Fog library is library for crossplatform development in C++ language. Main goal
of Fog is to provide very fast and accurate graphics output on all supported
operating systems and architectures. Currently Fog is not only graphics library,
it contains classes related to crossplatform GUI development all integrated into
one single library.

Library dependencies
====================

Windows:
- [Fog/UI] Win32 API only, no external dependencies

Linux:
- [Fog/Graphics] Freetype2 (direct - runtime loading)
- [Fog/Graphics] Fontconfig (optional - runtime loading)
- [Fog/UI] X11 (optional - runtime loading)

Support:
- [Fog/Graphics] libjpeg (optional - runtime loading)
- [Fog/Graphics] libpng (optional - runtime loading)
