// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

#include <Fog/Core/Char.h>
#include <Fog/Core/Error.h>
#include <Fog/Core/Math.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/Std.h>
#include <Fog/Core/StringUtil.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#if defined(FOG_HAVE_FLOAT_H)
#include <float.h>
#endif // FOG_HAVE_FLOAT_H

#if defined(FOG_HAVE_FENV_H)
#include <fenv.h>
#endif // FOG_HAVE_FENV_H

namespace Fog {
namespace StringUtil {

// ============================================================================
// [Fog::StringUtil::Unicode Tools]
// ============================================================================

sysuint_t utf8Validate(const Char8* src, sysuint_t len)
{
  // TODO
  return 0;
}

sysuint_t utf16Validate(const Char16* src, sysuint_t len)
{
  const Char16* srcCur = src;
  const Char16* srcEnd = srcCur + len;
  uint16_t uc;

  for (;;)
  {
    if (srcCur == srcEnd) return InvalidIndex;
    uc = *srcCur++;

    if (Char16::isLeadSurrogate(uc))
    {
      if (srcCur == srcEnd) break;
      uc = *srcCur++;
      if (!Char16::isTrailSurrogate(uc)) break;
    }
    else
    {
      if (uc >= 0xFFFE) break;
    }
  }

  return (sysuint_t)((--srcCur) - src);
}

sysuint_t utf32Validate(Char32* src, sysuint_t len)
{
  const Char32* srcCur = src;
  const Char32* srcEnd = srcCur + len;
  uint32_t uc;

  for (;;)
  {
    if (FOG_UNLIKELY(srcCur == srcEnd)) return InvalidIndex;
    uc = *srcCur++;

    if (!Char32::isValid(uc)) break;
  }

  return (sysuint_t)(--srcCur - src);
}

sysuint_t utf8ToUtf16(Char16* dest, const Char8* src, sysuint_t srcLen)
{
  // TODO
  return 0;
}

sysuint_t utf8ToUtf16Len(const Char8* src, sysuint_t len)
{
  // TODO
  return 0;
}

sysuint_t utf8ToUtf32(Char32* dest, const Char8* src, sysuint_t srcLen)
{
  // TODO
  return 0;
}

sysuint_t utf8ToUtf32Len(const Char8* src, sysuint_t len)
{
  // TODO
  return 0;
}

sysuint_t utf16ToUtf8(Char8* dest, const Char16* src, sysuint_t srcLen)
{
  const Char16* srcCur = src;
  const Char16* srcEnd = srcCur + srcLen;
  Char8* destCur = dest;
  uint16_t uc;

  for (;;)
  {
    if (FOG_UNLIKELY(srcCur == srcEnd)) break;
    uc = *srcCur++;

    if (FOG_LIKELY(uc < 0x80))
    {
      *destCur++ = (Char8)uc;
    }
    else if (FOG_LIKELY(uc < 0x800))
    {
      destCur[1] = (Char8)(0x80 | (uc & 0x3F)); uc = (uc >> 6) | 0xC0;
      destCur[0] = (Char8)(uc);
      destCur += 2;
    }
    else
    {
      if (Char16::isLeadSurrogate(uc))
      {
        uint32_t uc32 = uc;

        if (srcCur == srcEnd) return InvalidIndex;
        uc = *srcCur++;

        if (!Char16::isTrailSurrogate(uc)) return InvalidIndex;

        uc32 = 0x10000 + ((uc32 - LeadSurrogateMin) << 10) + ((uint32_t)uc - (LeadSurrogateMax + 1));

        destCur[3] = (Char8)(0x80 | (uc32 & 0x3F)); uc32 = (uc32 >> 6) | 0x10000;
        destCur[2] = (Char8)(0x80 | (uc32 & 0x3F)); uc32 = (uc32 >> 6) | 0x800;
        destCur[1] = (Char8)(0x80 | (uc32 & 0x3F)); uc32 = (uc32 >> 6) | 0xC0;
        destCur[0] = (Char8)(uc32);
        destCur += 4;
      }
      else
      {
        if (uc >= 0xFFFE) return InvalidIndex;

        destCur[2] = (Char8)(0x80 | (uc & 0x3F)); uc = (uc >> 6) | 0x800;
        destCur[1] = (Char8)(0x80 | (uc & 0x3F)); uc = (uc >> 6) | 0xC0;
        destCur[0] = (Char8)(uc);
        destCur += 3;
      }
    }
  }

  return (sysuint_t)(destCur - dest);
}

sysuint_t utf16ToUtf8Len(const Char16* src, sysuint_t len)
{
  const Char16* srcCur = src;
  const Char16* srcEnd = srcCur + len;
  uint16_t uc;
  sysuint_t result = len;

  for (;;)
  {
    if (FOG_UNLIKELY(srcCur == srcEnd)) break;
    uc = *srcCur++;

    if (FOG_LIKELY(uc < 0x80))
    {
      continue;
    }
    else if (FOG_LIKELY(uc < 0x800))
    {
      result++;
    }
    else
    {
      if (Char16::isLeadSurrogate(uc))
      {
        if (srcCur == srcEnd) return InvalidIndex;
        uc = *srcCur++;
        if (!Char16::isTrailSurrogate(uc)) return InvalidIndex;
        result += 3;
      }
      else
      {
        if (uc >= 0xFFFE) return InvalidIndex;
        result += 2;
      }
    }
  }

  return result;
}

sysuint_t utf16ToUtf32(Char32* dest, const Char16* src, sysuint_t srcLen)
{
  const Char16* srcCur = src;
  const Char16* srcEnd = srcCur + srcLen;
  Char32* destCur = dest;
  uint16_t uc;

  for (;;)
  {
    if (srcCur == srcEnd) break;

    uc = *srcCur++;
    if (Char16::isLeadSurrogate(uc))
    {
      uint32_t uc32 = uc;

      if (srcCur == srcEnd) return InvalidIndex;
      uc = *srcCur++;

      if (!Char16::isTrailSurrogate(uc)) return InvalidIndex;

      uc32 = 0x10000 + ((uc32 - LeadSurrogateMin) << 10) + ((uint32_t)uc - (LeadSurrogateMax + 1));
      *destCur++ = Char32(uc32);
    }
    else
    {
      if (uc >= 0xFFFE) return InvalidIndex;
      *destCur++ = Char32(uc);
    }
  }

  return (sysuint_t)(destCur - dest);
}

sysuint_t utf16ToUtf32Len(const Char16* src, sysuint_t len)
{
  const Char16* srcCur = src;
  const Char16* srcEnd = srcCur + len;
  uint16_t uc;
  sysuint_t result = len;

  for (;;)
  {
    if (srcCur == srcEnd) break;

    uc = *srcCur++;
    if (Char16::isLeadSurrogate(uc))
    {
      if (srcCur == srcEnd) return InvalidIndex;
      uc = *srcCur++;

      if (!Char16::isTrailSurrogate(uc)) return InvalidIndex;
      result--;
    }
    else
    {
      if (uc >= 0xFFFE) return InvalidIndex;
    }
  }

  return result;
}

sysuint_t utf32ToUtf8(Char8* dest, const Char32* src, sysuint_t srcLen)
{
  const Char32* srcCur = src;
  const Char32* srcEnd = srcCur + srcLen;
  Char8* destCur = dest;
  uint32_t uc;

  for (;;)
  {
    if (FOG_UNLIKELY(srcCur == srcEnd)) break;
    uc = *srcCur++;

    if (FOG_LIKELY(uc < 0x80))
    {
      *destCur++ = Char8(uc);
    }
    else if (FOG_LIKELY(uc < 0x800))
    {
      destCur[1] = Char8(0x80 | (uc & 0x3F)); uc = (uc >> 6) | 0xC0;
      destCur[0] = Char8(uc);
      destCur += 2;
    }
    else if (FOG_LIKELY(uc < 0x10000)) 
    {
      if (uc >= 0xFFFE || Char16::isLeadSurrogate(uc)) return InvalidIndex;
      destCur[2] = Char8(0x80 | (uc & 0x3F)); uc = (uc >> 6) | 0x800;
      destCur[1] = Char8(0x80 | (uc & 0x3F)); uc = (uc >> 6) | 0xC0;
      destCur[0] = Char8(uc);
      destCur += 3;
    }
    else
    {
      if (uc > MaxCodePoint) return InvalidIndex;
      destCur[3] = Char8(0x80 | (uc & 0x3F)); uc = (uc >> 6) | 0x10000;
      destCur[2] = Char8(0x80 | (uc & 0x3F)); uc = (uc >> 6) | 0x800;
      destCur[1] = Char8(0x80 | (uc & 0x3F)); uc = (uc >> 6) | 0xC0;
      destCur[0] = Char8(uc);
      destCur += 4;
    }
  }

  return (sysuint_t)(destCur - dest);
}

sysuint_t utf32ToUtf8Len(const Char32* src, sysuint_t len)
{
  const Char32* srcCur = src;
  const Char32* srcEnd = srcCur + len;
  uint32_t uc;
  sysuint_t result = len;

  for (;;)
  {
    if (FOG_UNLIKELY(srcCur == srcEnd)) break;
    uc = *srcCur++;

    if (FOG_LIKELY(uc < 0x80))
    {
      continue;
    }
    else if (FOG_LIKELY(uc < 0x800))
    {
      result += 1;
    }
    else if (FOG_LIKELY(uc < 0x10000)) 
    {
      if (uc >= 0xFFFE || Char16::isLeadSurrogate((uint16_t)uc)) return InvalidIndex;
      result += 2;
    }
    else
    {
      if (uc > MaxCodePoint) return InvalidIndex;
      result += 3;
    }
  }

  return result;
}

sysuint_t utf32ToUtf16(Char16* dest, const Char32* src, sysuint_t srcLen)
{
  const Char32* srcCur = src;
  const Char32* srcEnd = src + srcLen;
  Char16* destCur = dest;
  uint32_t uc;

  for (;;)
  {
    if (FOG_UNLIKELY(srcCur == srcEnd)) break;
    uc = *srcCur++;

    // decompose
    if (uc > 0xFFFF)
    {
      if (uc > MaxCodePoint) return InvalidIndex;
      uc -= 0x10000;
      destCur[0] = Char16((uc >> 10) + 0xD800);
      destCur[1] = Char16((uc & 0x03FF) + 0xDC00);
      destCur += 2;
    }
    else if (Char16::isLeadSurrogate((uint16_t)uc) || uc >= 0xFFFE)
      return InvalidIndex;
    else
      *destCur++ = (Char16)(uc);
  }

  return (sysuint_t)(destCur - dest);
}

sysuint_t utf32ToUtf16Len(const Char32* src, sysuint_t len)
{
  const Char32* srcCur = src;
  const Char32* srcEnd = srcCur + len;
  uint32_t uc;
  sysuint_t result = len;

  for (;;)
  {
    if (FOG_UNLIKELY(srcCur == srcEnd)) break;
    uc = *srcCur++;

    if (uc > 0xFFFF)
    {
      if (uc > MaxCodePoint) return InvalidIndex;
      result++;
    }
    else if (Char16::isLeadSurrogate((uint16_t)uc) || uc >= 0xFFFE)
    {
      return InvalidIndex;
    }
  }

  return result;
}

// Miscellany

bool utf16ToLatin(Char8* dest, const Char16* src, sysuint_t length)
{
  bool ok = true;
  sysuint_t i;
  uint16_t uc;

  for (i = length; i; i--, src++, dest++)
  {
    if ((uc = src->ch()) > 255) { uc = '?'; ok = false; }
    *dest = (uint8_t)uc;
  }

  return ok;
}

bool utf32ToLatin(Char8* dest, const Char32* src, sysuint_t length)
{
  bool ok = true;
  sysuint_t i;
  uint32_t uc;

  for (i = length; i; i--, src++, dest++)
  {
    if ((uc = src->ch()) > 255) { uc = '?'; ok = false; }
    *dest = (uint8_t)uc;
  }

  return ok;
}

// ============================================================================
// [Fog::StringUtil::asciiMap]
// ============================================================================

// Map acsii character into its numerical equivalent.
static const uint8_t asciiMap[256] =
{
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
  0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
  0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
  0x21, 0x22, 0x23, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
  0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
  0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
  0x21, 0x22, 0x23, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

// ============================================================================
// [Fog::StringUtil::boolMap]
// ============================================================================

// Boolean translating table from 'char*' or 'Fog::Char*' into 'bool'
// Should be 12 bytes
#include <Fog/Core/Pack.h>
struct BoolMap
{
  char str[10];
  uint8_t length;
  uint8_t result;
};

static const BoolMap boolMap[] =
{
  { { 't', 'r', 'u', 'e', 0  , 0  , 0  , 0  ,  0 ,  0 }, 4, true  },
  { { 'f', 'a', 'l', 's', 'e', 0  , 0  , 0  ,  0 ,  0 }, 5, false },
  
  { { 'y', 'e', 's', 0  , 0  , 0  , 0  , 0  ,  0 ,  0 }, 3, true  },
  { { 'n', 'o', 0  , 0  , 0  , 0  , 0  , 0  ,  0 ,  0 }, 2, false },
  
  { { 'o', 'n', 0  , 0  , 0  , 0  , 0  , 0  ,  0 ,  0 }, 2, true  },
  { { 'o', 'f', 'f', 0  , 0  , 0  , 0  , 0  ,  0 ,  0 }, 3, false },
  
  { { 'e', 'n', 'a', 'b', 'l', 'e', 0  , 0  ,  0 ,  0 }, 6, true  },
  { { 'd', 'i', 's', 'a', 'b', 'l', 'e', 0  ,  0 ,  0 }, 7, false },
  
  { { 'e', 'n', 'a', 'b', 'l', 'e', 'd', 0  ,  0 ,  0 }, 7, true  },
  { { 'd', 'i', 's', 'a', 'b', 'l', 'e', 'd',  0 ,  0 }, 8, false },
  
  { { '1', 0  , 0  , 0  , 0  , 0  , 0  , 0  ,  0 ,  0 }, 1, true  },
  { { '0', 0  , 0  , 0  , 0  , 0  , 0  , 0  ,  0 ,  0 }, 1, false }
};
#include <Fog/Core/Unpack.h>

// ============================================================================
// [Fog::StringUtil::Raw]
// ============================================================================

void copy(Char16* dst, const Char8* src, sysuint_t length)
{
  sysuint_t i;
  for (i = 0; i < length; i++) dst[i] = src[i];
}

void copy(Char32* dst, const Char8* src, sysuint_t length)
{
  sysuint_t i;
  for (i = 0; i < length; i++) dst[i] = src[i];
}

bool eq(const Char16* a, const Char8* b, sysuint_t length, uint cs)
{
  sysuint_t i;
  if (cs == CaseSensitive)
  {
    for (i = 0; i < length; i++)
    {
      if (a[i] != b[i]) return false;
    }
  }
  else
  {
    for (i = 0; i < length; i++)
    {
      if (a[i].toLower() != b[i].toLower()) return false;
    }
  }
  return true;
}

bool eq(const Char32* a, const Char8* b, sysuint_t length, uint cs)
{
  sysuint_t i;
  if (cs == CaseSensitive)
  {
    for (i = 0; i < length; i++)
    {
      if (a[i] != b[i]) return false;
    }
  }
  else
  {
    for (i = 0; i < length; i++)
    {
      if (a[i].toLower() != b[i].toLower()) return false;
    }
  }
  return true;
}

} // StringUtil namespace
} // Fog namespace

// ============================================================================
// [Fog::StringUtil::dtoa]
// ============================================================================

#define __G_GENERATE
#include <Fog/Core/StringUtil_dtoa.cpp>
#undef __G_GENERATE

// ============================================================================
// [Fog::StringUtil::itoa]
// ============================================================================

#define __G_GENERATE
#include <Fog/Core/StringUtil_itoa.cpp>
#undef __G_GENERATE

// ============================================================================
// [Fog::StringUtil::Generator]
// ============================================================================

#define __G_GENERATE

#define __G_SIZE 1
#include <Fog/Core/StringUtil_gen.cpp>
#undef __G_SIZE

#define __G_SIZE 2
#include <Fog/Core/StringUtil_gen.cpp>
#undef __G_SIZE

#define __G_SIZE 4
#include <Fog/Core/StringUtil_gen.cpp>
#undef __G_SIZE

#undef __G_GENERATE
