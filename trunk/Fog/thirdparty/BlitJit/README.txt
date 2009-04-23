BlitJit - Just In Time Image Blitting Library for C++ Language
==============================================================

http://code.google.com/p/blitjit/

BlitJit is high performance low level image blitting library for C++ language
designed to take power of jit compilation. It's designed for embedding in 
graphics libraries with higher level API, but it's possible to use it directly
without any other wrappers. BlitJit supports several pixel formats and 
compositing operations and can compile (just in time) functions for them. 
Currently BlitJit is crossplatform, but host processor must be x86 or x64 
compatible (64 bit mode is of course supported).

BlitJit library should be used to maximize speed of graphics operations. Each
higher level library needs low level pixel manipulation functions that can
affect overall library performance. Because BlitJit compiles everything in
runtime (just in time) it can compile functions that are best for current
processor.

For just in time compilation is used AsmJit library.

Examples
========

For examples look at http://code.google.com/p/blitjit/

To build standalone test program in /test directory you need to have AsmJit 
library in directory in one level up:

  ../AsmJit
  ../BlitJit

If you have AsmJit in directory one level up, the cmake project should be
created without problems and test application should run.

Contact
=======

Petr Kobalicek <kobalicek.petr@gmail.com>