// [Fog/Svg Library - C++ API]
//
// [Licence]
// MIT, See COPYING.txt file in package
// Author: Anders Jansson, Copyright(c) 2009 <l8.jansson@gmail.com>

// [Purpose]
// This declares the class Svg - Scalable Vector Graphics that could be read and
// written to file. This is not directly rendered to screen but compiled into
// a drawable representation called Symbol.



class Svg {
public:

    static Svg& read(file);
    write(file);

    Symbol& compile();

};
