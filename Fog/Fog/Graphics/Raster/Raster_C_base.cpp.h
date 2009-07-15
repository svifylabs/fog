// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// For some IDEs to enable code-assist.
#include <Fog/Build/Build.h>
#if defined(FOG_IDE)
#include <Fog/Graphics/Raster/Raster_C.h>
#endif // FOG_IDE

namespace Fog {
namespace Raster {

// ============================================================================
// [Fog::Raster - Defines]
// ============================================================================

#define READ_8(ptr)  (((const uint8_t *)(ptr))[0])
#define READ_16(ptr) (((const uint16_t*)(ptr))[0])
#define READ_32(ptr) (((const uint32_t*)(ptr))[0])

static FOG_INLINE int double_to_int(double d) { return (int)d; }
static FOG_INLINE int64_t double_to_fixed48x16(double d) { return (int64_t)(d * 65536.0); }

// ============================================================================
// [Fog::Raster - Demultiply Reciprocal Table]
// ============================================================================

// Reciprocal table created by material provided by M Joonas Pihlaja, see
// <http://cgit.freedesktop.org/~joonas/unpremultiply/tree/README>
//
// Copyright (c) 2009  M Joonas Pihlaja
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.

// The reciprocal_table[i] entries are defined by
//
//   0    when i = 0
//  255 / i    when i > 0
//
// represented in fixed point format with RECIPROCAL_BITS of
// precision and errors rounded up.
//
// #define ceil_div(a,b) ((a) + (b)-1) / (b)
//
// for each:
//   (i) ? ceil_div(255 * (1 << 16), (i)) : 0) : 0

uint32_t const demultiply_reciprocal_table[256] =
{
  0x00000000, 0x00FF0000, 0x007F8000, 0x00550000, 0x003FC000, 0x00330000, 0x002A8000, 0x00246DB7,
  0x001FE000, 0x001C5556, 0x00198000, 0x00172E8C, 0x00154000, 0x00139D8A, 0x001236DC, 0x00110000,
  0x000FF000, 0x000F0000, 0x000E2AAB, 0x000D6BCB, 0x000CC000, 0x000C2493, 0x000B9746, 0x000B1643,
  0x000AA000, 0x000A3334, 0x0009CEC5, 0x000971C8, 0x00091B6E, 0x0008CB09, 0x00088000, 0x000839CF,
  0x0007F800, 0x0007BA2F, 0x00078000, 0x00074925, 0x00071556, 0x0006E454, 0x0006B5E6, 0x000689D9,
  0x00066000, 0x00063832, 0x0006124A, 0x0005EE24, 0x0005CBA3, 0x0005AAAB, 0x00058B22, 0x00056CF0,
  0x00055000, 0x0005343F, 0x0005199A, 0x00050000, 0x0004E763, 0x0004CFB3, 0x0004B8E4, 0x0004A2E9,
  0x00048DB7, 0x00047944, 0x00046585, 0x00045271, 0x00044000, 0x00042E2A, 0x00041CE8, 0x00040C31,
  0x0003FC00, 0x0003EC4F, 0x0003DD18, 0x0003CE55, 0x0003C000, 0x0003B217, 0x0003A493, 0x00039770,
  0x00038AAB, 0x00037E40, 0x0003722A, 0x00036667, 0x00035AF3, 0x00034FCB, 0x000344ED, 0x00033A55,
  0x00033000, 0x000325EE, 0x00031C19, 0x00031282, 0x00030925, 0x00030000, 0x0002F712, 0x0002EE59,
  0x0002E5D2, 0x0002DD7C, 0x0002D556, 0x0002CD5D, 0x0002C591, 0x0002BDF0, 0x0002B678, 0x0002AF29,
  0x0002A800, 0x0002A0FE, 0x00029A20, 0x00029365, 0x00028CCD, 0x00028657, 0x00028000, 0x000279CA,
  0x000273B2, 0x00026DB7, 0x000267DA, 0x00026218, 0x00025C72, 0x000256E7, 0x00025175, 0x00024C1C,
  0x000246DC, 0x000241B3, 0x00023CA2, 0x000237A7, 0x000232C3, 0x00022DF3, 0x00022939, 0x00022493,
  0x00022000, 0x00021B82, 0x00021715, 0x000212BC, 0x00020E74, 0x00020A3E, 0x00020619, 0x00020205,
  0x0001FE00, 0x0001FA0C, 0x0001F628, 0x0001F253, 0x0001EE8C, 0x0001EAD4, 0x0001E72B, 0x0001E38F,
  0x0001E000, 0x0001DC80, 0x0001D90C, 0x0001D5A4, 0x0001D24A, 0x0001CEFB, 0x0001CBB8, 0x0001C881,
  0x0001C556, 0x0001C235, 0x0001BF20, 0x0001BC15, 0x0001B915, 0x0001B61F, 0x0001B334, 0x0001B052,
  0x0001AD7A, 0x0001AAAB, 0x0001A7E6, 0x0001A52A, 0x0001A277, 0x00019FCC, 0x00019D2B, 0x00019A91,
  0x00019800, 0x00019578, 0x000192F7, 0x0001907E, 0x00018E0D, 0x00018BA3, 0x00018941, 0x000186E6,
  0x00018493, 0x00018246, 0x00018000, 0x00017DC2, 0x00017B89, 0x00017958, 0x0001772D, 0x00017508,
  0x000172E9, 0x000170D1, 0x00016EBE, 0x00016CB2, 0x00016AAB, 0x000168AA, 0x000166AF, 0x000164B9,
  0x000162C9, 0x000160DE, 0x00015EF8, 0x00015D18, 0x00015B3C, 0x00015966, 0x00015795, 0x000155C8,
  0x00015400, 0x0001523E, 0x0001507F, 0x00014EC5, 0x00014D10, 0x00014B5F, 0x000149B3, 0x0001480B,
  0x00014667, 0x000144C7, 0x0001432C, 0x00014194, 0x00014000, 0x00013E71, 0x00013CE5, 0x00013B5D,
  0x000139D9, 0x00013859, 0x000136DC, 0x00013563, 0x000133ED, 0x0001327B, 0x0001310C, 0x00012FA1,
  0x00012E39, 0x00012CD5, 0x00012B74, 0x00012A16, 0x000128BB, 0x00012763, 0x0001260E, 0x000124BD,
  0x0001236E, 0x00012223, 0x000120DA, 0x00011F94, 0x00011E51, 0x00011D11, 0x00011BD4, 0x00011A99,
  0x00011962, 0x0001182C, 0x000116FA, 0x000115CA, 0x0001149D, 0x00011372, 0x0001124A, 0x00011124,
  0x00011000, 0x00010EE0, 0x00010DC1, 0x00010CA5, 0x00010B8B, 0x00010A73, 0x0001095E, 0x0001084B,
  0x0001073A, 0x0001062C, 0x0001051F, 0x00010415, 0x0001030D, 0x00010207, 0x00010103, 0x00010000
};

// Example code how demultiply can be used.
// 
// static void FOG_FASTCALL demultiply_argb(uint32_t* dst, uint32_t const * src, sysuint_t w)
// {
//   for (sysuint_t i = w; i; i--, dst++, src++)
//   {
//     uint32_t rgba = src[0];
//     uint32_t a = (rgba >> 24);
//     uint32_t r = (rgba >> 16) & 255;
//     uint32_t g = (rgba >>  8) & 255;
//     uint32_t b = (rgba      ) & 255;
//     uint32_t recip = demultiply_reciprocal_table[a];
//
//     r = ((r * recip)      ) & 0x00FF0000;
//     g = ((g * recip) >>  8) & 0x0000FF00;
//     b = ((b * recip) >> 16) & 0x000000FF;
//
//     dst[0] = r | g | b | (a << 24);
//   }
// }

} // Raster namespace
} // Fog namespace
