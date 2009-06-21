// [Generator]
#if defined(__G_GENERATE)

namespace Fog {
namespace StringUtil {

// all possible digits in lowercase and uppercase format
static const char itoa_digits[36*2] =
{
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
};

void itoa(int64_t n, int base, bool uppercase, NTOAOut* out)
{
  if (n < 0)
  {
    utoa((uint64_t)(-n), base, uppercase, out);
    out->negative = true;
  }
  else
  {
    utoa((uint64_t)(n), base, uppercase, out);
  }
}

void utoa(uint64_t n, int base, bool uppercase, NTOAOut* out)
{
  uint8_t* resultEnd = out->buffer + FOG_ARRAY_SIZE(out->buffer);
  uint8_t* resultCur = resultEnd;

  const uint8_t* digits = (const uint8_t*)itoa_digits;
  if (uppercase) digits += 36;

#if FOG_ARCH_BITS == 32
  uint32_t n32;
#endif // FOG_ARCH_BITS
  uint64_t n64;

#if FOG_ARCH_BITS == 32
  if (n <= UINT32_MAX)
  {
    n32 = (uint32_t)n;

    // decimal base (the most used)
    if (base == 10 || base < 2 || base > 36)
    {
__conv10_32bit:
      do {
        *--resultCur = '0' + (uint8_t)(n32 % 10);
        n32 /= 10;
      } while (n32 != 0);
    }
    // octal base
    else if (base == 8)
    {
__conv8_32bit:
      do {
        *--resultCur = '0' + (uint8_t)(n32 & 7);
        n32 >>= 3;
      } while (n32 != 0);
    }
    // hexadecimal base
    else if (base == 16)
    {
__conv16_32bit:
      do {
        *--resultCur = digits[n32 & 15];
        n32 >>= 4;
      } while (n32 != 0);
    }
    // variant base
    else
    {
__convBaseN_32bit:
      do {
        *--resultCur = digits[n32 % base];
        n32 /= base;
      } while (n32 != 0);
    }
  }
  else
  {
#endif // FOG_ARCH_BITS == 32
    n64 = n;

    // decimal base (the most used)
    if (base == 10 || base < 2 || base > 36)
    {
      do {
        *--resultCur = '0' + (uchar)(n64 % FOG_UINT64_C(10));
        n64 /= 10;
      } while
#if FOG_ARCH_BITS == 32
      (n64 > UINT32_MAX);
      if (n64 != 0) { n32 = (uint32_t)n64; goto __conv10_32bit; }
#else
      (n64 != FOG_UINT64_C(0));
#endif // FOG_ARCH_BITS == 32
    }
    // octal base
    else if (base == 8)
    {
      do {
        *--resultCur = '0' + (uchar)(n64 & FOG_UINT64_C(7));
        n64 >>= 3;
      } while
#if FOG_ARCH_BITS == 32
      (n64 > UINT32_MAX);
      if (n64 != 0) { n32 = (uint32_t)n64; goto __conv8_32bit; }
#else
      (n64 != FOG_UINT64_C(0));
#endif // FOG_ARCH_BITS == 32
    }
    // hexadecimal base
    else if (base == 16)
    {
      do {
        *--resultCur = digits[(ulong)(n64 & FOG_UINT64_C(15))];
        n64 >>= 4;
      } while
#if FOG_ARCH_BITS == 32
      (n64 > INT32_MAX);
      if (n64 != 0) { n32 = (uint32_t)n64; goto __conv16_32bit; }
#else
      (n64 != FOG_UINT64_C(0));
#endif // FOG_ARCH_BITS == 32
    }
    // variant base
    else
    {
      do {
        *--resultCur = digits[(ulong)(n64 & FOG_UINT64_C(15))];
        n64 >>= 4;
      } while
#if FOG_ARCH_BITS == 32
      (n64 > INT32_MAX);
      if (n64 != 0) { n32 = (uint32_t)n64; goto __convBaseN_32bit; }
#else
      (n64 != FOG_UINT64_C(0));
#endif // FOG_ARCH_BITS == 32
    }
#if FOG_ARCH_BITS == 32
  }
#endif // FOG_ARCH_BITS == 32

  // write result and it's length back to args
  out->result = resultCur;
  out->length = (sysuint_t)(resultEnd - resultCur);
  out->negative = false;
}

} // StringUtil namespace
} // Fog namespace

// [Generator]
#endif // _G_GENERATE
