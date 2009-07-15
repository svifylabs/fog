// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// For some IDEs to enable code-assist.
#include <Fog/Build/Build.h>
#if defined(FOG_IDE)
#include <Fog/Graphics/Raster/Raster_C_base.cpp.h>
#include <Fog/Graphics/Raster/Raster_C_convert.cpp.h>
#endif // FOG_IDE

namespace Fog {
namespace Raster {

// ============================================================================
// [Fog::Raster - Operator - Base Macros and Generic Implementation]
// ============================================================================

#define BEGIN_OPERATOR_IMPL(OP_NAME, OP_INHERITS) \
  template<typename DstFmt, typename SrcFmt> \
  struct OP_NAME : public OP_INHERITS<OP_NAME<DstFmt, SrcFmt>, DstFmt, SrcFmt> \
  { \
    typedef OP_INHERITS<OP_NAME<DstFmt, SrcFmt>, DstFmt, SrcFmt> INHERITED; \
    using INHERITED::normalize_src_fetch; \
    using INHERITED::normalize_dst_fetch; \
    using INHERITED::normalize_dst_store;

#define END_OPERATOR_IMPL \
  }

template<typename Operator, typename DstFmt, typename SrcFmt>
struct Operator_Base
{
  static FOG_INLINE uint32_t normalize_src_fetch(uint32_t x)
  {
    if (!SrcFmt::HasAlpha && DstFmt::HasAlpha)
    {
      return x |= 0xFF000000;
    }
    else if (SrcFmt::HasAlpha && DstFmt::HasAlpha && !SrcFmt::IsPremultiplied)
    {
      return premultiply(x);
    }
    else
    {
      return x;
    }
  }

  static FOG_INLINE uint32_t normalize_dst_fetch(uint32_t x)
  {
    if (SrcFmt::HasAlpha && !DstFmt::HasAlpha)
    {
      return x |= 0xFF000000;
    }
    else if (SrcFmt::HasAlpha && DstFmt::HasAlpha && !DstFmt::IsPremultiplied)
    {
      return premultiply(x);
    }
    else
    {
      return x;
    }
  }

  static FOG_INLINE uint32_t normalize_dst_store(uint32_t x)
  {
    if (DstFmt::HasAlpha && !DstFmt::IsPremultiplied)
    {
      return demultiply(x);
    }
    else
    {
      return x;
    }
  }

  static FOG_INLINE void span_composite(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    for (sysint_t i = w; i; i--, dst += DstFmt::BytesPerPixel, src += SrcFmt::BytesPerPixel)
    {
      Operator::pixel(dst, src);
    }
  }

  static FOG_INLINE void span_composite_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    for (sysint_t i = w; i; i--, dst += DstFmt::BytesPerPixel, src += SrcFmt::BytesPerPixel, msk += 1)
    {
      Operator::pixel_a8(dst, src, READ_8(msk));
    }
  }
};

template<typename Operator, typename DstFmt, typename SrcFmt>
struct Operator_Complex : public Operator_Base<Operator, DstFmt, SrcFmt>
{
  using Operator_Base<Operator, DstFmt, SrcFmt>::normalize_src_fetch;
  using Operator_Base<Operator, DstFmt, SrcFmt>::normalize_dst_fetch;
  using Operator_Base<Operator, DstFmt, SrcFmt>::normalize_dst_store;

  static FOG_INLINE void pixel(uint8_t* dst, const uint8_t* src)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));

    byte1x2 src0lo, src0hi;
    byte1x2 dst0lo, dst0hi;

    byte2x2_unpack_0213(dst0lo, dst0hi, dst0);
    byte2x2_unpack_0213(src0lo, src0hi, src0);

    Operator::op(dst0lo, dst0hi, src0lo, src0hi);

    DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(dst0lo, dst0hi)));
  }

  static FOG_INLINE void pixel_a8(uint8_t* dst, const uint8_t* src, uint32_t msk)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));

    byte1x2 src0lo, src0hi;
    byte1x2 dst0lo, dst0hi;

    byte2x2_unpack_0213(dst0lo, dst0hi, dst0);
    byte2x2_unpack_0213(src0lo, src0hi, src0);
    byte2x2_mul_u(src0lo, src0hi, msk);

    Operator::op(dst0lo, dst0hi, src0lo, src0hi);

    DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(dst0lo, dst0hi)));
  }

  static FOG_INLINE void span_solid(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    uint32_t dst0;

    byte1x2 src0lo, src0hi;
    byte1x2 dst0lo, dst0hi;

    byte2x2_unpack_0213(src0lo, src0hi, src0);

    do {
      dst0 = normalize_dst_fetch(DstFmt::fetch(dst));

      byte2x2_unpack_0213(dst0lo, dst0hi, dst0);
      Operator::op(dst0lo, dst0hi, src0lo, src0hi);
      DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(dst0lo, dst0hi)));

      dst += DstFmt::BytesPerPixel;
    } while(--w);
  }

  static FOG_INLINE void span_solid_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    uint32_t dst0;

    byte1x2 src0lo, src0hi;
    byte1x2 dst0lo, dst0hi;

    byte2x2_unpack_0213(src0lo, src0hi, src0);

    do {
      dst0 = normalize_dst_fetch(DstFmt::fetch(dst));

      byte2x2_unpack_0213(dst0lo, dst0hi, dst0);
      Operator::op(dst0lo, dst0hi, src0lo, src0hi);
      DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(dst0lo, dst0hi)));

      dst += DstFmt::BytesPerPixel;
    } while(--w);
  }

  static FOG_INLINE void span_composite(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    do {
      uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
      uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));

      byte1x2 dst0lo, dst0hi;
      byte1x2 src0lo, src0hi;

      byte2x2_unpack_0213(dst0lo, dst0hi, dst0);
      byte2x2_unpack_0213(src0lo, src0hi, src0);
      Operator::op(dst0lo, dst0hi, src0lo, src0hi);
      DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(dst0lo, dst0hi)));

      dst += DstFmt::BytesPerPixel;
      src += DstFmt::BytesPerPixel;
    } while (--w);
  }

  static FOG_INLINE void span_composite_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    do {
      uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
      uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
      uint32_t msk0 = READ_8(msk);

      byte1x2 dst0lo, dst0hi;
      byte1x2 src0lo, src0hi;

      byte2x2_unpack_0213(dst0lo, dst0hi, dst0);
      byte2x2_unpack_0213(src0lo, src0hi, src0);
      if (msk0 != 0xFF) byte2x2_mul_u(src0lo, src0hi, msk0);
      Operator::op(dst0lo, dst0hi, src0lo, src0hi);
      DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(dst0lo, dst0hi)));

      dst += DstFmt::BytesPerPixel;
      src += DstFmt::BytesPerPixel;
      msk += 1;
    } while (--w);
  }
};

// ============================================================================
// [Fog::Raster - Operator_Src]
// ============================================================================

// Dca' = Sca
// Da'  = Sa
BEGIN_OPERATOR_IMPL(Operator_Src, Operator_Base)
  static FOG_INLINE void pixel(uint8_t* dst, const uint8_t* src)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    DstFmt::store(dst, normalize_dst_store(src0));
  }

  static FOG_INLINE void pixel_a8(uint8_t* dst, const uint8_t* src, uint32_t msk)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    if (DstFmt::HasAlpha && msk != 0xFF) src0 = bytemul(src0, msk);
    DstFmt::store(dst, normalize_dst_store(src0));
  }

  static FOG_INLINE void span_solid(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    for (sysint_t i = w; i; i--, dst += DstFmt::BytesPerPixel)
    {
      DstFmt::store(dst, normalize_dst_store(src0));
    }
  }

  static FOG_INLINE void span_solid_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    if (DstFmt::HasAlpha)
    {
      uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
      byte1x2 pix0;
      byte1x2 pix1;
      byte2x2_unpack_0213(pix0, pix1, src0);

      for (sysint_t i = w; i; i--, dst += DstFmt::BytesPerPixel, msk += 1)
      {
        uint32_t p = src0;
        uint32_t m = READ_8(msk);

        if (m != 0xFF)
        {
          byte1x2 p0 = pix0;
          byte1x2 p1 = pix1;
          byte2x2_mul_u(p0, p1, m);
          p = byte2x2_pack_0213(p0, p1);
        }

        DstFmt::store(dst, normalize_dst_store(p));
      }
    }
    else
    {
      span_solid(dst, src, w);
    }
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Dst]
// ============================================================================

// Dca' = Dca
// Da'  = Da
BEGIN_OPERATOR_IMPL(Operator_Dst, Operator_Base)
  static FOG_INLINE void pixel(uint8_t* dst, const uint8_t* src)
  {
  }

  static FOG_INLINE void pixel_a8(uint8_t* dst, const uint8_t* src, uint32_t msk)
  {
  }

  static FOG_INLINE void span_solid(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
  }

  static FOG_INLINE void span_solid_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Over]
// ============================================================================

// Dca' = Sca + Dca.(1 - Sa)
// Da'  = Sa + Da.(1 - Sa)
//      = Sa + Da - Sa.Da
BEGIN_OPERATOR_IMPL(Operator_Over, Operator_Base)
  static FOG_INLINE void pixel(uint8_t* dst, const uint8_t* src)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    uint32_t a0 = getAlpha(~src0);

    if (a0 != 0x00) src0 = bytemuladd(normalize_dst_fetch(DstFmt::fetch(dst)), a0, src0);

    DstFmt::store(dst, normalize_dst_store(src0));
  }

  static FOG_INLINE void pixel_a8(uint8_t* dst, const uint8_t* src, uint32_t msk)
  {
    if (msk == 0) return;

    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    byte1x2 src0lo, src0hi;

    byte2x2_unpack_0213(src0lo, src0hi, src0);
    byte2x2_mul_u(src0lo, src0hi, msk);

    uint32_t src0a = byte1x2_hi(src0hi);
    if (src0a == 0xFF)
    {
      DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(src0lo, src0hi)));
    }
    else
    {
      uint32_t dst0 = DstFmt::fetch(dst);
      byte1x2 dst0lo, dst0hi;

      byte2x2_unpack_0213(dst0lo, dst0hi, src0);
      byte2x2_muladd_u_byte2x2(dst0lo, dst0hi, 255 - src0a, src0lo, src0hi);
      DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(dst0lo, dst0hi)));
    }
  }

  static FOG_INLINE void span_solid(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    if (!SrcFmt::HasAlpha)
    {
      Operator_Src<DstFmt, SrcFmt>::span_solid(dst, src, w);
      return;
    }
    else
    {
      uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
      uint32_t dst0;
      uint32_t ia0 = (~src0) >> 24;

      if (ia0 == 0x00)
      {
        Operator_Src<DstFmt, SrcFmt>::span_solid(dst, src, w);
      }
      else
      {
        byte1x2 src0lo, src0hi;
        byte1x2 dst0lo, dst0hi;
        byte2x2_unpack_0213(src0lo, src0hi, src0);

        do {
          dst0 = normalize_dst_fetch(DstFmt::fetch(dst));

          byte2x2_unpack_0213(dst0lo, dst0hi, dst0);
          byte2x2_muladd_u_byte2x2(dst0lo, dst0hi, ia0, src0lo, src0hi);
          DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(dst0lo, dst0hi)));

          dst += DstFmt::BytesPerPixel;
        } while (--w);
      }
    }
  }

  static FOG_INLINE void span_solid_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    if (!SrcFmt::HasAlpha)
    {
      Operator_Src<DstFmt, SrcFmt>::span_solid_a8(dst, src, msk, w);
      return;
    }
    else
    {
      uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
      uint32_t dst0;
      uint32_t a0 = src0 >> 24;
      uint32_t msk0;

      byte1x2 src0lo, src0hi;
      byte1x2 dst0lo, dst0hi;
      byte1x2 pix0lo, pix0hi;

      byte2x2_unpack_0213(src0lo, src0hi, src0);

      if (a0 == 0xFF)
      {
        do {
          if ((msk0 = READ_8(msk)) != 0)
          {
            dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
            pix0lo = src0lo;
            pix0hi = src0hi;
            byte2x2_unpack_0213(dst0lo, dst0hi, dst0);
            byte2x2_mul_u(pix0lo, pix0hi, msk0);
            byte2x2_muladd_u_byte2x2(dst0lo, dst0hi, 255 - msk0, pix0lo, pix0hi);
            DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(dst0lo, dst0hi)));
          }

          dst += DstFmt::BytesPerPixel;
          msk += 1;
        } while (--w);
      }
      else
      {
        do {
          if ((msk0 = READ_8(msk)) != 0)
          {
            dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
            pix0lo = src0lo;
            pix0hi = src0hi;
            byte2x2_unpack_0213(dst0lo, dst0hi, dst0);
            byte2x2_mul_u(pix0lo, pix0hi, msk0);
            byte2x2_muladd_u_byte2x2(dst0lo, dst0hi, 255 - byte1x2_hi(pix0hi), dst0lo, dst0hi);
            DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(dst0lo, dst0hi)));
          }

          dst += DstFmt::BytesPerPixel;
          msk += 1;
        } while (--w);
      }
    }
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_OverReverse]
// ============================================================================

// Dca' = Dca + Sca.(1 - Da)
// Da'  = Da + Sa.(1 - Da)
//      = Da + Sa - Da.Sa
BEGIN_OPERATOR_IMPL(Operator_OverReverse, Operator_Base)
  static FOG_INLINE void pixel(uint8_t* dst, const uint8_t* src)
  {
    uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    if (src0)
    {
      uint32_t dst0ia = 255 - getAlpha(dst0);
      DstFmt::store(dst, normalize_dst_store(bytemuladd(src0, dst0ia, dst0)));
    }
  }

  static FOG_INLINE void pixel_a8(uint8_t* dst, const uint8_t* src, uint32_t msk)
  {
    if (msk == 0x00) return;

    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));

    if (src0)
    {
      byte1x2 dst0lo, dst0hi;
      byte1x2 src0lo, src0hi;

      uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
      uint32_t dst0a = getAlpha(dst0);

      byte2x2_unpack_0213(dst0lo, dst0hi, dst0);
      byte2x2_unpack_0213(src0lo, src0hi, src0);
      if (msk != 0xFF) byte2x2_mul_u(src0lo, src0hi, msk);
      byte2x2_muladd_u_byte2x2(src0lo, src0hi, 255 - dst0a, dst0lo, dst0hi);
      DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(dst0lo, dst0hi)));
    }
  }

  static FOG_INLINE void span_solid(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    uint32_t dst0;
    uint32_t a0;

    if (!src0) return;

    byte1x2 src0lo, src0hi;
    byte1x2 dst0lo, dst0hi;
    byte1x2 pix0lo, pix0hi;

    byte2x2_unpack_0213(src0lo, src0hi, src0);

    do {
      dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
      a0 = dst0 >> 24;

      if (a0 != 0xFF)
      {
        byte2x2_unpack_0213(dst0lo, dst0hi, dst0);
        pix0lo = src0lo;
        pix0hi = src0hi;
        byte2x2_muladd_u_byte2x2(pix0lo, pix0hi, 255 - a0, dst0lo, dst0hi);
        DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(pix0lo, pix0hi)));
      }

      dst += DstFmt::BytesPerPixel;
    } while (--w);
  }

  static FOG_INLINE void span_solid_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    uint32_t dst0;
    uint32_t a0;
    uint32_t msk0;

    if (!src0) return;

    byte1x2 src0lo, src0hi;
    byte1x2 dst0lo, dst0hi;
    byte1x2 pix0lo, pix0hi;

    byte2x2_unpack_0213(src0lo, src0hi, src0);

    do {
      if ((msk0 = READ_8(msk)) != 0)
      {
        dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
        a0 = dst0 >> 24;

        if (a0 != 0xFF)
        {
          byte2x2_unpack_0213(dst0lo, dst0hi, dst0);
          pix0lo = src0lo;
          pix0hi = src0hi;
          byte2x2_mul_u(pix0lo, pix0hi, msk0);
          byte2x2_muladd_u_byte2x2(pix0lo, pix0hi, 255 - a0, dst0lo, dst0hi);
          DstFmt::store(dst, normalize_dst_store(byte2x2_pack_0213(pix0lo, pix0hi)));
        }
      }

      dst += DstFmt::BytesPerPixel;
      msk += 1;
    } while (--w);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_In]
// ============================================================================

// Dca' = Sca.Da
// Da'  = Sa.Da
BEGIN_OPERATOR_IMPL(Operator_In, Operator_Base)
  static FOG_INLINE void pixel(uint8_t* dst, const uint8_t* src)
  {
    uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
    if (dst0)
    {
      uint32_t a0 = getAlpha(dst0);
      uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
      DstFmt::store(dst, normalize_dst_store(bytemul(src0, a0)));
    }
  }

  static FOG_INLINE void pixel_a8(uint8_t* dst, const uint8_t* src, uint32_t msk)
  {
    uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
    if (dst0)
    {
      uint32_t a0 = getAlpha(dst0);
      uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
      if (msk != 0xFF) a0 = div255(a0 * msk);
      DstFmt::store(dst, normalize_dst_store(bytemul(src0, a0)));
    }
  }

  static FOG_INLINE void span_solid(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    sysint_t i = w;
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));

    do {
      uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
      if (dst0)
      {
        uint32_t a0 = getAlpha(dst0);
        DstFmt::store(dst, normalize_dst_store(bytemul(src0, a0)));
      }

      dst += DstFmt::BytesPerPixel;
    } while (--i);
  }

  static FOG_INLINE void span_solid_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    sysint_t i = w;
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));

    do {
      uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
      if (dst0)
      {
        uint32_t a0 = getAlpha(dst0);
        uint32_t m0 = READ_8(msk);
        uint32_t s0 = src0;
        if (m0 != 0xFF) s0 = bytemul(s0, m0);
        DstFmt::store(dst, normalize_dst_store(bytemul(s0, a0)));
      }

      dst += DstFmt::BytesPerPixel;
      msk += 1;
    } while (--i);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_InReverse]
// ============================================================================

// Dca' = Dca.Sa
// Da'  = Da.Sa
BEGIN_OPERATOR_IMPL(Operator_InReverse, Operator_Base)
  static FOG_INLINE void pixel(uint8_t* dst, const uint8_t* src)
  {
    if (SrcFmt::HasAlpha)
    {
      uint32_t a0 = SrcFmt::fetchAlpha(src);
      if (a0 != 0xFF)
      {
        uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
        DstFmt::store(dst, normalize_dst_store(bytemul(dst0, a0)));
      }
    }
  }

  static FOG_INLINE void pixel_a8(uint8_t* dst, const uint8_t* src, uint32_t msk)
  {
    if (SrcFmt::HasAlpha)
    {
      uint32_t a0 = div255(SrcFmt::fetchAlpha(src) * msk);
      if (a0 != 0xFF)
      {
        uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
        DstFmt::store(dst, normalize_dst_store(bytemul(dst0, a0)));
      }
    }
    else
    {
      if (msk != 0xFF)
      {
        uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
        DstFmt::store(dst, normalize_dst_store(bytemul(dst0, msk)));
      }
    }
  }

  static FOG_INLINE void span_solid(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    if (SrcFmt::HasAlpha)
    {
      sysint_t i = w;
      uint32_t a0 = SrcFmt::fetchAlpha(src);
      if (a0 == 0xFF) return;

      do {
        uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
        DstFmt::store(dst, normalize_dst_store(bytemul(dst0, a0)));

        dst += DstFmt::BytesPerPixel;
      } while (--i);
    }
  }

  static FOG_INLINE void span_solid_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    if (SrcFmt::HasAlpha)
    {
      sysint_t i = w;
      uint32_t a0 = SrcFmt::fetchAlpha(src);

      if (a0 == 0xFF)
      {
        do {
          uint32_t m0 = READ_8(msk);
          if (m0 != 0xFF)
          {
            uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
            DstFmt::store(dst, normalize_dst_store(bytemul(dst0, m0)));
          }

          dst += DstFmt::BytesPerPixel;
          msk += 1;
        } while (--i);
      }
      else
      {
        do {
          uint32_t m0 = div255(a0 * READ_8(msk));
          uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
          DstFmt::store(dst, normalize_dst_store(bytemul(dst0, m0)));

          dst += DstFmt::BytesPerPixel;
          msk += 1;
        } while (--i);
      }
    }
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Out]
// ============================================================================

// Dca' = Sca.(1 - Da)
// Da'  = Sa.(1 - Da)
BEGIN_OPERATOR_IMPL(Operator_Out, Operator_Base)
  static FOG_INLINE void pixel(uint8_t* dst, const uint8_t* src)
  {
    uint32_t a0 = 255 - DstFmt::fetchAlpha(dst);
    uint32_t src0 = 0;
    if (a0 != 0) src0 = bytemul(normalize_src_fetch(SrcFmt::fetch(src)), a0);
    DstFmt::store(dst, normalize_dst_store(src0));
  }

  static FOG_INLINE void pixel_a8(uint8_t* dst, const uint8_t* src, uint32_t msk)
  {
    uint32_t a0 = 255 - DstFmt::fetchAlpha(dst);
    uint32_t src0 = bytemul(SrcFmt::fetch(src), div255(a0 * msk));
    DstFmt::store(dst, normalize_dst_store(src0));
  }

  static FOG_INLINE void span_solid(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    sysint_t i = w;

    do {
      uint32_t a0 = 255 - DstFmt::fetchAlpha(dst);
      DstFmt::store(dst, normalize_dst_store(bytemul(src0, a0)));

      dst += DstFmt::BytesPerPixel;
    } while (--i);
  }

  static FOG_INLINE void span_solid_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    sysint_t i = w;

    do {
      uint32_t a0 = 255 - div255(DstFmt::fetchAlpha(dst) * READ_8(msk));
      DstFmt::store(dst, normalize_dst_store(bytemul(src0, a0)));

      dst += DstFmt::BytesPerPixel;
      msk += 1;
    } while (--i);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_OutReverse]
// ============================================================================

// Dca' = Dca.(1 - Sa)
// Da'  = Da.(1 - Sa)
BEGIN_OPERATOR_IMPL(Operator_OutReverse, Operator_Base)
  static FOG_INLINE void pixel(uint8_t* dst, const uint8_t* src)
  {
    uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
    if (dst0)
    {
      uint32_t ia0 = 255 - SrcFmt::fetchAlpha(src);
      DstFmt::store(dst, normalize_dst_store(bytemul(dst0, ia0)));
    }
  }

  static FOG_INLINE void pixel_a8(uint8_t* dst, const uint8_t* src, uint32_t msk)
  {
    uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
    if (dst0)
    {
      uint32_t ia0 = div255((255 - SrcFmt::fetchAlpha(src)) * msk);
      DstFmt::store(dst, normalize_dst_store(bytemul(dst0, ia0)));
    }
  }

  static FOG_INLINE void span_solid(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    uint32_t ia0 = 255 - SrcFmt::fetchAlpha(src);
    sysint_t i = w;

    if (ia0 == 0xFF) return;

    do {
      uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
      if (dst0)
      {
        DstFmt::store(dst, normalize_dst_store(bytemul(dst0, ia0)));
      }

      dst += DstFmt::BytesPerPixel;
    } while (--i);
  }

  static FOG_INLINE void span_solid_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    uint32_t a0 = SrcFmt::fetchAlpha(src);
    sysint_t i = w;

    if (a0 == 0x00) return;

    if (a0 == 0xFF)
    {
      do {
        uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
        if (dst0)
        {
          DstFmt::store(dst, normalize_dst_store(bytemul(dst0, 255 - READ_8(msk))));
        }

        dst += DstFmt::BytesPerPixel;
        msk += 1;
      } while (--i);
    }
    else
    {
      do {
        uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
        if (dst0)
        {
          DstFmt::store(dst, normalize_dst_store(bytemul(dst0, 255 - div255(a0 * READ_8(msk)))));
        }

        dst += DstFmt::BytesPerPixel;
        msk += 1;
      } while (--i);
    }
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Atop]
// ============================================================================

// Dca' = Sca.Da + Dca.(1 - Sa)
// Da'  = Sa.Da + Da.(1 - Sa)
//      = Da.(Sa + 1 - Sa)
//      = Da
BEGIN_OPERATOR_IMPL(Operator_Atop, Operator_Base)
  static FOG_INLINE void pixel(uint8_t* dst, const uint8_t* src)
  {
    uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
    if (dst0)
    {
      uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
      uint32_t dsta0 = getAlpha(dst0);
      uint32_t srci0 = getAlpha(~src0);
      DstFmt::store(dst, normalize_dst_store(byteaddmul(src0, dsta0, dst0, srci0)));
    }
  }

  static FOG_INLINE void pixel_a8(uint8_t* dst, const uint8_t* src, uint32_t msk)
  {
    uint32_t dst0 = DstFmt::fetch(dst);
    if (dst0)
    {
      uint32_t src0 = SrcFmt::fetch(src);
      if (msk != 0xFF) src0 = bytemul(src0, msk);
      uint32_t dsta0 = getAlpha(dst0);
      uint32_t srci0 = getAlpha(~src0);
      DstFmt::store(dst, normalize_dst_store(byteaddmul(src0, dsta0, dst0, srci0)));
    }
  }

  static FOG_INLINE void span_solid(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    sysint_t i = w;
    uint32_t src0 = SrcFmt::fetch(src);
    uint32_t srci0 = getAlpha(~src0);

    do {
      uint32_t dst0 = DstFmt::fetch(dst);
      if (dst0)
      {
        uint32_t dsta0 = getAlpha(dst0);
        DstFmt::store(dst, normalize_dst_store(byteaddmul(src0, dsta0, dst0, srci0)));
      }

      dst += DstFmt::BytesPerPixel;
    } while(--i);
  }

  static FOG_INLINE void span_solid_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    sysint_t i = w;
    uint32_t src0 = SrcFmt::fetch(src);

    do {
      uint32_t dst0 = DstFmt::fetch(dst);
      if (dst0)
      {
        uint32_t srcm0 = bytemul(src0, READ_8(msk));
        uint32_t srci0 = getAlpha(~srcm0);
        uint32_t dsta0 = getAlpha(dst0);
        DstFmt::store(dst, normalize_dst_store(byteaddmul(srcm0, dsta0, dst0, srci0)));
      }

      dst += DstFmt::BytesPerPixel;
      msk += 1;
    } while(--i);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_AtopReverse]
// ============================================================================

// Dca' = Dca.Sa + Sca.(1 - Da)
// Da'  = Da.Sa + Sa.(1 - Da)
//      = Sa.(Da + 1 - Da)
//      = Sa
BEGIN_OPERATOR_IMPL(Operator_AtopReverse, Operator_Base)
  static FOG_INLINE void pixel(uint8_t* dst, const uint8_t* src)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
    uint32_t srca0 = getAlpha(src0);
    uint32_t dsti0 = getAlpha(~dst0);
    DstFmt::store(dst, normalize_dst_store(byteaddmul(src0, dsti0, dst0, srca0)));
  }

  static FOG_INLINE void pixel_a8(uint8_t* dst, const uint8_t* src, uint32_t msk)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
    if (msk != 0xFF) src0 = bytemul(src0, msk);
    uint32_t srca0 = getAlpha(src0);
    uint32_t dsti0 = getAlpha(~dst0);
    DstFmt::store(dst, normalize_dst_store(byteaddmul(src0, dsti0, dst0, srca0)));
  }

  static FOG_INLINE void span_solid(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    sysint_t i = w;

    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    uint32_t srca0 = getAlpha(src0);

    do {
      uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
      uint32_t dsti0 = getAlpha(~dst0);
      DstFmt::store(dst, normalize_dst_store(byteaddmul(src0, dsti0, dst0, srca0)));

      dst += DstFmt::BytesPerPixel;
    } while (--i);
  }

  static FOG_INLINE void span_solid_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    sysint_t i = w;

    do {
      uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
      uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
      uint32_t m0 = READ_8(msk);
      if (m0 != 0xFF) src0 = bytemul(src0, m0);
      uint32_t srca0 = getAlpha(src0);
      uint32_t dsti0 = getAlpha(~dst0);
      DstFmt::store(dst, normalize_dst_store(byteaddmul(src0, dsti0, dst0, srca0)));

      dst += DstFmt::BytesPerPixel;
      msk += 1;
    } while (--i);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Xor]
// ============================================================================

// Dca' = Sca.(1 - Da) + Dca.(1 - Sa)
// Da'  = Sa.(1 - Da) + Da.(1 - Sa)
//      = Sa + Da - 2.Sa.Da
BEGIN_OPERATOR_IMPL(Operator_Xor, Operator_Base)
  static FOG_INLINE void pixel(uint8_t* dst, const uint8_t* src)
  {
    if (SrcFmt::HasAlpha)
    {
      uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
      uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
      uint32_t srci0 = getAlpha(~src0);
      uint32_t dsti0 = getAlpha(~dst0);
      DstFmt::store(dst, normalize_dst_store(byteaddmul(src0, dsti0, dst0, srci0)));
    }
    else
    {
      Operator_Over<DstFmt, SrcFmt>::pixel(dst, src);
    }
  }

  static FOG_INLINE void pixel_a8(uint8_t* dst, const uint8_t* src, uint32_t msk)
  {
    uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    if (msk != 0xFF) src0 = bytemul(src0, msk);
    uint32_t srci0 = getAlpha(~src0);
    uint32_t dsti0 = getAlpha(~dst0);
    DstFmt::store(dst, normalize_dst_store(byteaddmul(src0, dsti0, dst0, srci0)));
  }

  static FOG_INLINE void span_solid(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    if (SrcFmt::HasAlpha)
    {
      sysint_t i = w;

      uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
      uint32_t srci0 = getAlpha(~src0);

      do {
        uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
        uint32_t dsti0 = getAlpha(~dst0);
        DstFmt::store(dst, normalize_dst_store(byteaddmul(src0, dsti0, dst0, srci0)));
        dst += DstFmt::BytesPerPixel;
      } while (--i);
    }
    else
    {
      Operator_Over<DstFmt, SrcFmt>::span_solid(dst, src, w);
    }
  }

  static FOG_INLINE void span_solid_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    uint32_t src0 = normalize_src_fetch(SrcFmt::fetch(src));
    sysint_t i = w;

    do {
      uint32_t dst0 = normalize_dst_fetch(DstFmt::fetch(dst));
      uint32_t srcm0 = src0;
      uint32_t m0 = READ_8(msk);
      if (m0 != 0xFF) srcm0 = bytemul(srcm0, m0);
      uint32_t srci0 = getAlpha(~srcm0);
      uint32_t dsti0 = getAlpha(~dst0);
      DstFmt::store(dst, normalize_dst_store(byteaddmul(srcm0, dsti0, dst0, srci0)));
      dst += DstFmt::BytesPerPixel;
      msk += 1;
    } while (--i);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Clear]
// ============================================================================

// Dca' = 0
// Da'  = 0
BEGIN_OPERATOR_IMPL(Operator_Clear, Operator_Base)
  static FOG_INLINE void pixel(uint8_t* dst, const uint8_t* src)
  {
    DstFmt::store(dst, 0);
  }

  static FOG_INLINE void pixel_a8(uint8_t* dst, const uint8_t* src, uint32_t msk)
  {
    DstFmt::store(dst, 0);
  }

  static FOG_INLINE void span_solid(uint8_t* dst, const uint8_t* src, sysint_t w)
  {
    sysint_t i = w;
    do {
      DstFmt::store(dst, 0);
    } while (--i);
  }

  static FOG_INLINE void span_solid_a8(uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
  {
    sysint_t i = w;
    do {
      DstFmt::store(dst, 0);
    } while (--i);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Add]
// ============================================================================

// Dca' = Sca + Dca
// Da'  = Sa + Da
BEGIN_OPERATOR_IMPL(Operator_Add, Operator_Complex)
  static FOG_INLINE void op(byte1x2& dst0lo, byte1x2& dst0hi, byte1x2 src0lo, byte1x2 src0hi)
  {
    byte2x2_adds_byte2x2(dst0lo, dst0hi, src0lo, src0hi);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Subtract]
// ============================================================================

// Dca' = Dca - Sca
// Da'  = 1 - (1 - Sa).(1 - Da)
BEGIN_OPERATOR_IMPL(Operator_Subtract, Operator_Complex)
  static FOG_INLINE void op(byte1x2& dst0lo, byte1x2& dst0hi, byte1x2 src0lo, byte1x2 src0hi)
  {
    uint32_t a0 = 255 - div255(byte1x2_hi(~dst0hi) * byte1x2_hi(~src0hi));

    byte2x2_subs_byte2x2(dst0lo, dst0hi, src0lo, src0hi);
    dst0hi &= 0x00FF0000;
    dst0hi |= a0 << 16;
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Multiply]
// ============================================================================

// Dca' = Sca.Dca + Sca.(1 - Da) + Dca.(1 - Sa)
// Da'  = Sa.Da + Sa.(1 - Da) + Da.(1 - Sa)
//      = Sa + Da - Sa.Da
BEGIN_OPERATOR_IMPL(Operator_Multiply, Operator_Complex)
  static FOG_INLINE void op(byte1x2& dst0lo, byte1x2& dst0hi, byte1x2 src0lo, byte1x2 src0hi)
  {
    byte1x2 res0lo = dst0lo;
    byte1x2 res0hi = dst0hi;
    byte2x2_mul_byte2x2(res0lo, res0hi, src0lo, src0hi);

    uint32_t srcia = 255 - byte1x2_hi(src0hi);
    uint32_t dstia = 255 - byte1x2_hi(dst0hi);

    byte2x2_mul_u(dst0lo, dst0hi, srcia);
    byte2x2_mul_u(src0lo, src0hi, dstia);

    byte2x2_adds_byte2x2(dst0lo, dst0hi, res0lo, res0hi);
    byte2x2_adds_byte2x2(dst0lo, dst0hi, src0lo, src0hi);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Screen]
// ============================================================================

// Dca' = (Sca.Da + Dca.Sa - Sca.Dca) + Sca.(1 - Da) + Dca.(1 - Sa)
//      = Sca + Dca - Sca.Dca
// Da'  = Sa + Da - Sa.Da
BEGIN_OPERATOR_IMPL(Operator_Screen, Operator_Complex)
  static FOG_INLINE void op(byte1x2& dst0lo, byte1x2& dst0hi, byte1x2 src0lo, byte1x2 src0hi)
  {
    byte1x2 res0lo = dst0lo, res0hi = dst0hi;
    byte2x2_mul_byte2x2(res0lo, res0hi, src0lo, src0hi);
    byte2x2_addsub_byte2x2(dst0lo, dst0hi, src0lo, src0hi, res0lo, res0hi);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Darken]
// ============================================================================

// Dca' = min(Sca.Da, Dca.Sa) + Sca.(1 - Da) + Dca.(1 - Sa)
// Da'  = min(Sa.Da, Da.Sa) + Sa.(1 - Da) + Da.(1 - Sa)
//      = Sa.Da + Sa - Sa.Da + Da - Sa.Da
//      = Sa + Da - Sa.Da
BEGIN_OPERATOR_IMPL(Operator_Darken, Operator_Complex)
  static FOG_INLINE void op(byte1x2& dst0lo, byte1x2& dst0hi, byte1x2 src0lo, byte1x2 src0hi)
  {
    byte1x2 res0lo = src0lo, res0hi = src0hi;
    byte1x2 res1lo = dst0lo, res1hi = dst0hi;

    byte2x2_mul_u(res0lo, res0hi, byte1x2_hi(dst0hi));
    byte2x2_mul_u(res1lo, res1hi, byte1x2_hi(src0hi));
    byte2x2_min(res0lo, res0hi, res1lo, res1hi);

    uint32_t dstia = 255 - byte1x2_hi(dst0hi);
    uint32_t srcia = 255 - byte1x2_hi(src0hi);

    byte2x2_mul_u(dst0lo, dst0hi, srcia);
    byte2x2_mul_u(src0lo, src0hi, dstia);

    byte2x2_adds_byte2x2(dst0lo, dst0hi, src0lo, src0hi);
    byte2x2_adds_byte2x2(dst0lo, dst0hi, res0lo, res0hi);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Lighten]
// ============================================================================

// Dca' = max(Sca.Da, Dca.Sa) + Sca.(1 - Da) + Dca.(1 - Sa)
// Da'  = max(Sa.Da, Da.Sa) + Sa.(1 - Da) + Da.(1 - Sa)
//      = Sa.Da + Sa - Sa.Da + Da - Sa.Da
//      = Sa + Da - Sa.Da
BEGIN_OPERATOR_IMPL(Operator_Lighten, Operator_Complex)
  static FOG_INLINE void op(byte1x2& dst0lo, byte1x2& dst0hi, byte1x2 src0lo, byte1x2 src0hi)
  {
    byte1x2 res0lo = src0lo, res0hi = src0hi;
    byte1x2 res1lo = dst0lo, res1hi = dst0hi;

    byte2x2_mul_u(res0lo, res0hi, byte1x2_hi(dst0hi));
    byte2x2_mul_u(res1lo, res1hi, byte1x2_hi(src0hi));
    byte2x2_max(res0lo, res0hi, res1lo, res1hi);

    uint32_t dstia = 255 - byte1x2_hi(dst0hi);
    uint32_t srcia = 255 - byte1x2_hi(src0hi);

    byte2x2_mul_u(dst0lo, dst0hi, srcia);
    byte2x2_mul_u(src0lo, src0hi, dstia);

    byte2x2_adds_byte2x2(dst0lo, dst0hi, src0lo, src0hi);
    byte2x2_adds_byte2x2(dst0lo, dst0hi, res0lo, res0hi);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Difference]
// ============================================================================

// Dca' = Sca + Dca - 2.min(Sca.Da, Dca.Sa)
// Da'  = Sa + Da - min(Sa.Da, Da.Sa)
//      = Sa + Da - Sa.Da
BEGIN_OPERATOR_IMPL(Operator_Difference, Operator_Complex)
  static FOG_INLINE void op(byte1x2& dst0lo, byte1x2& dst0hi, byte1x2 src0lo, byte1x2 src0hi)
  {
    byte1x2 res0lo = src0lo, res0hi = src0hi;
    byte1x2 res1lo = dst0lo, res1hi = dst0hi;

    byte2x2_mul_u(res0lo, res0hi, byte1x2_hi(dst0hi));
    byte2x2_mul_u(res1lo, res1hi, byte1x2_hi(src0hi));
    byte2x2_min(res0lo, res0hi, res1lo, res1hi);
    byte2x2_lshift_by_1_no_alpha(res0lo, res0hi);
    byte2x2_addsub_byte2x2(dst0lo, dst0hi, src0lo, src0hi, res0lo, res0hi);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Exclusion]
// ============================================================================

// Dca' = (Sca.Da + Dca.Sa - 2.Sca.Dca) + Sca.(1 - Da) + Dca.(1 - Sa)
// Dca' = Sca + Dca - 2.Sca.Dca
//
// Da'  = (Sa.Da + Da.Sa - 2.Sa.Da) + Sa.(1 - Da) + Da.(1 - Sa)
//      = Sa - Sa.Da + Da - Da.Sa = Sa + Da - 2.Sa.Da
// Substitute 2.Sa.Da with Sa.Da
//
// Da'  = Sa + Da - Sa.Da
BEGIN_OPERATOR_IMPL(Operator_Exclusion, Operator_Complex)
  static FOG_INLINE void op(byte1x2& dst0lo, byte1x2& dst0hi, byte1x2 src0lo, byte1x2 src0hi)
  {
    byte1x2 res0lo = dst0lo, res0hi = dst0hi;
    byte2x2_mul_byte2x2(res0lo, res0hi, src0lo, src0hi);
    byte2x2_lshift_by_1_no_alpha(res0lo, res0hi);
    byte2x2_addsub_byte2x2(dst0lo, dst0hi, src0lo, src0hi, res0lo, res0hi);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_Invert]
// ============================================================================

// Dca' = (Da - Dca) * Sa + Dca.(1 - Sa)
// Da'  = Sa + (Da - Da) * Sa + Da - Sa.Da
//      = Sa + Da - Sa.Da
//
// For calculation this formula is best:
// Dca' = (Da - Dca) * Sa + Dca.(1 - Sa)
// Da'  = (1 + Da - Da) * Sa + Da.(1 - Sa)
BEGIN_OPERATOR_IMPL(Operator_Invert, Operator_Complex)
  static FOG_INLINE void op(byte1x2& dst0lo, byte1x2& dst0hi, byte1x2 src0lo, byte1x2 src0hi)
  {
    byte1x2 dstalo;
    byte1x2 dstahi;

    byte2x2_expand_hi(dstalo, dstahi, dst0hi); dstahi += 0x00FF0000;
    byte2x2_sub_byte2x2(dstalo, dstahi, dst0lo, dst0hi);
    byte2x2_mul_u(dstalo, dstahi, byte1x2_hi(src0hi));
    byte2x2_mul_u(dst0lo, dst0hi, 255 - byte1x2_hi(src0hi));
    byte2x2_adds_byte2x2(dst0lo, dst0hi, dstalo, dstahi);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Operator_InvertRgb]
// ============================================================================

// Dca' = (Da - Dca) * Sca + Dca.(1 - Sa)
// Da'  = Sa + (Da - Da) * Sa + Da - Da.Sa
//      = Sa + Da - Sa.Da
//
// For calculation this formula is best:
// Dca' = (Da - Dca) * Sca + Dca.(1 - Sa)
// Da'  = (1 + Da - Da) * Sa + Da.(1 - Sa)
BEGIN_OPERATOR_IMPL(Operator_InvertRgb, Operator_Complex)
  static FOG_INLINE void op(byte1x2& dst0lo, byte1x2& dst0hi, byte1x2 src0lo, byte1x2 src0hi)
  {
    byte1x2 dstalo;
    byte1x2 dstahi;

    byte2x2_expand_hi(dstalo, dstahi, dst0hi); dstahi += 0x00FF0000;
    byte2x2_sub_byte2x2(dstalo, dstahi, dst0lo, dst0hi);
    byte2x2_mul_byte2x2(dstalo, dstahi, src0lo, src0hi);
    byte2x2_mul_u(dst0lo, dst0hi, 255 - byte1x2_hi(src0hi));
    byte2x2_adds_byte2x2(dst0lo, dst0hi, dstalo, dstahi);
  }
END_OPERATOR_IMPL;

// ============================================================================
// [Fog::Raster - Raster - NOP]
// ============================================================================

static void FOG_FASTCALL raster_pixel_nop(
  uint8_t* dst, uint32_t src)
{
}

static void FOG_FASTCALL raster_pixel_a8_nop(
  uint8_t* dst, uint32_t src, uint32_t msk)
{
}

static void FOG_FASTCALL raster_span_solid_nop(
  uint8_t* dst, uint32_t src, sysint_t w)
{
}

static void FOG_FASTCALL raster_span_solid_a8_nop(
  uint8_t* dst, uint32_t src, const uint8_t* msk, sysint_t w)
{
}

static void FOG_FASTCALL raster_span_composite_nop(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
}

static void FOG_FASTCALL raster_span_composite_a8_nop(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
}

static void FOG_FASTCALL raster_span_composite_indexed_nop(
  uint8_t* dst, const uint8_t* src, sysint_t w,
  const Rgba* pal)
{
}

static void FOG_FASTCALL raster_span_composite_indexed_a8_nop(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w,
  const Rgba* pal)
{
}

// ============================================================================
// [Fog::Raster - Raster - General]
// ============================================================================

template <typename Operator>
static void FOG_FASTCALL raster_pixel(
  uint8_t* dst, uint32_t src)
{
  Operator::pixel(dst, (const uint8_t*)&src);
}

template <typename Operator>
static void FOG_FASTCALL raster_pixel_a8(
  uint8_t* dst, uint32_t src, uint32_t msk)
{
  Operator::pixel_a8(dst, (const uint8_t*)&src, msk);
}

template <typename Operator>
static void FOG_FASTCALL raster_span_solid(
  uint8_t* dst, uint32_t src, sysint_t w)
{
  Operator::span_solid(dst, (const uint8_t*)&src, w);
}

template <typename Operator>
static void FOG_FASTCALL raster_span_solid_a8(
  uint8_t* dst, uint32_t src, const uint8_t* msk, sysint_t w)
{
  Operator::span_solid_a8(dst, (const uint8_t*)&src, msk, w);
}

template <typename Operator>
static void FOG_FASTCALL raster_span_composite(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  Operator::span_composite(dst, src, w);
}

template <typename Operator>
static void FOG_FASTCALL raster_span_composite_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  Operator::span_composite_a8(dst, src, msk, w);
}

// ============================================================================
// [Fog::Raster - Raster - Agb32 - SrcOver]
// ============================================================================
#if 0
static void FOG_FASTCALL raster_argb32_pixel_srcover(
  uint8_t* dst, uint32_t src)
{
  uint32_t a = src >> 24;

  if (a != 0xFF)
    src = blend_srcover_nonpremultiplied(((uint32_t*)dst)[0], src, a);

  ((uint32_t*)dst)[0] = src;
}

static void FOG_FASTCALL raster_argb32_pixel_a8_srcover(
  uint8_t* dst, uint32_t src, uint32_t msk)
{
  uint32_t a = div255((src >> 24) * msk);

  if (a != 0xFF)
    src = blend_srcover_nonpremultiplied(((uint32_t*)dst)[0], src, a);

  ((uint32_t*)dst)[0] = src;
}

static void FOG_FASTCALL raster_argb32_span_solid_srcover(
  uint8_t* dst, uint32_t src, sysint_t w)
{
  sysint_t i = w;
  uint32_t a = src >> 24;

  if (a != 0xFF)
  {
    src = bytemul(src, a);
    a = 255 - a;

    do {
      ((uint32_t*)dst)[0] = bytemul(((uint32_t*)dst)[0], a) + src;
      dst += 4;
    } while (--w);
  }
  else
  {
    do {
      ((uint32_t*)dst)[0] = src;
      dst += 4;
    } while (--w);
  }
}

static void FOG_FASTCALL raster_argb32_span_solid_a8_srcover(
  uint8_t* dst, uint32_t src, const uint8_t* msk, sysint_t w)
{
  sysint_t i = w;
  uint32_t a = src >> 24;
  uint32_t m;

  if (a != 0xFF)
  {
    src = bytemul(src, a);
    a = 255 - a;

    do {
      if ((m = READ_8(msk)) == 0xFF)
      {
        ((uint32_t*)dst)[0] = bytemul(((uint32_t*)dst)[0], a) + src;
      }
      else if (m)
      {
        uint32_t src0 = bytemul(src0, m);
      }
      dst += 4;
      msk += 1;
    } while (--w);
  }
  else
  {
    do {
      if ((m = READ_8(msk)) == 0xFF)
      {
        ((uint32_t*)dst)[0] = src;
      }
      else if (m)
      {
        ((uint32_t*)dst)[0] = bytemul(((uint32_t*)dst)[0], 255 - m) + bytemul(src, m);
      }
      dst += 4;
      msk += 1;
    } while (--w);
  }
}
#endif

// ============================================================================
// [Fog::Raster - Raster - Rgb32]
// ============================================================================

static void FOG_FASTCALL raster_rgb32_pixel(
  uint8_t* dst, uint32_t src)
{
  uint32_t a = src >> 24;

  if (a != 0xFF)
  {
    src = blend_over_srcpremultiplied(((uint32_t*)dst)[0], src, a);
  }

  ((uint32_t*)dst)[0] = src;
}

static void FOG_FASTCALL raster_rgb32_pixel_a8(
  uint8_t* dst, uint32_t src, uint32_t msk)
{
  uint32_t a = src >> 24;

  if (a != 0xFF || msk != 0xFF)
  {
    src = bytemul_reset_alpha(src, msk);
    src = blend_over_srcpremultiplied(((uint32_t*)dst)[0], src, src >> 24);
  }

  ((uint32_t*)dst)[0] = src;
}

static void FOG_FASTCALL raster_rgb32_span_solid(
  uint8_t* dst, uint32_t src, sysint_t w)
{
  sysint_t i = w;
  uint32_t a = src >> 24;

  if (a != 0xFF)
  {
    src |= 0xFF000000;
    a = 255 - a;

    do {
      ((uint32_t*)dst)[0] = bytemul_reset_alpha(((uint32_t*)dst)[0], a) + src;
      dst += 4;
    } while (--w);
  }
  else
  {
    // This is C optimized version of memfill for 32-bit/64-bit architectures.
    // On most x86 modern systems it will be always replaced by MMX / SSE2
    // version, so it's mainly for other architectures.
    sysuint_t src0 = unpackU32ToSysUInt(src);

#if FOG_ARCH_BITS == 64
    // Align.
    if ((sysuint_t)dst & 0x7)
    {
      ((uint32_t*)dst)[0] = src;
      dst += 4;
      i--;
    }
#endif

    while (i >= 8)
    {
      set32(dst, src0);
      dst += 32;
      i -= 8;
    }

    switch (i)
    {
      case 7: ((uint32_t*)dst)[0] = src; dst += 4;
      case 6: ((uint32_t*)dst)[0] = src; dst += 4;
      case 5: ((uint32_t*)dst)[0] = src; dst += 4;
      case 4: ((uint32_t*)dst)[0] = src; dst += 4;
      case 3: ((uint32_t*)dst)[0] = src; dst += 4;
      case 2: ((uint32_t*)dst)[0] = src; dst += 4;
      case 1: ((uint32_t*)dst)[0] = src; dst += 4;
    }
  }
}

static void FOG_FASTCALL raster_rgb32_span_solid_a8(
  uint8_t* dst, uint32_t src, const uint8_t* msk, sysint_t w)
{
  sysint_t i = w;
  uint32_t a = src >> 24;
  uint32_t m;

  if (a != 0xFF)
  {
    uint32_t FFsrc = src | 0xFF000000;
    uint32_t ia = 255 - a;

    do {
      if ((m = READ_8(msk)) == 0xFF)
      {
        ((uint32_t*)dst)[0] = bytemul_reset_alpha(((uint32_t*)dst)[0], ia) + FFsrc;
      }
      else if (m)
      {
        uint32_t srcm = bytemul(src, m);
        ((uint32_t*)dst)[0] = blend_over_srcpremultiplied(((uint32_t*)dst)[0], srcm, srcm >> 24);
      }
      dst += 4;
      msk += 1;
    } while (--w);
  }
  else
  {
    do {
      if ((m = READ_8(msk)) == 0xFF)
      {
        ((uint32_t*)dst)[0] = src;
      }
      else if (m)
      {
        ((uint32_t*)dst)[0] = blend_over_nonpremultiplied(((uint32_t*)dst)[0], src, m);
      }
      dst += 4;
      msk += 1;
    } while (--w);
  }
}

static void FOG_FASTCALL raster_rgb32_span_composite_argb32(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  do {
    uint32_t src0 = ((uint32_t*)src)[0];
    uint32_t a0 = src0 >> 24;

    if (a0 == 0xFF)
    {
      ((uint32_t*)dst)[0] = src0;
    }
    else if (a0)
    {
      ((uint32_t*)dst)[0] = bytemul(((uint32_t*)dst)[0], 255 - a0) + bytemul(src0, a0);
    }
    dst += 4;
    src += 4;
  } while (--w);
}

static void FOG_FASTCALL raster_rgb32_span_composite_prgb32(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
  do {
    uint32_t src0 = ((uint32_t*)src)[0];
    uint32_t a0 = src0 >> 24;

    if (a0 == 0xFF)
    {
      ((uint32_t*)dst)[0] = src0;
    }
    else if (a0)
    {
      ((uint32_t*)dst)[0] = bytemul(((uint32_t*)dst)[0], 255 - a0) + src0;
    }
    dst += 4;
    src += 4;
  } while (--w);
}

static void FOG_FASTCALL raster_rgb32_span_composite_indexed(
  uint8_t* dst, const uint8_t* src, sysint_t w, const Rgba* pal)
{
  do {
    uint32_t src0 = pal[src[0]];
    uint32_t a0 = src0 >> 24;
    if (a0)
    {
      if (a0 != 0xFF) src0 = bytemul(((uint32_t*)dst)[0], 255 - a0) + bytemul(src0, a0);
      ((uint32_t*)dst)[0] = src0;
    }

    dst += 4;
    src += 1;
  } while (--w);
}

static void FOG_FASTCALL raster_rgb32_span_composite_argb32_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  do {
    uint32_t m;
    if ((m = READ_8(msk)))
    {
      uint32_t src0 = ((uint32_t*)src)[0];
      uint32_t a0 = src0 >> 24;
      if (m != 0xFF) a0 = div255(a0 * m);
      ((uint32_t*)dst)[0] = bytemul(((uint32_t*)dst)[0], 255 - a0) + bytemul(src0, a0);
    }

    dst += 4;
    src += 4;
    msk += 1;
  } while (--w);
}

static void FOG_FASTCALL raster_rgb32_span_composite_prgb32_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  do {
    uint32_t m;
    if ((m = READ_8(msk)))
    {
      uint32_t src0 = ((uint32_t*)src)[0];
      if (m != 0xFF) src0 = bytemul(src0, m);
      ((uint32_t*)dst)[0] = blend_over_srcpremultiplied(((uint32_t*)dst)[0], src0, src0 >> 24);
    }

    dst += 4;
    src += 4;
    msk += 1;
  } while (--w);
}

static void FOG_FASTCALL raster_rgb32_span_composite_rgb32_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  do {
    uint32_t m;
    if ((m = READ_8(msk)))
    {
      uint32_t src0 = ((uint32_t*)src)[0];
      if (m != 0xFF) src0 = bytemul(((uint32_t*)dst)[0], 255 - m) + bytemul(src0, m);
      ((uint32_t*)dst)[0] = src0;
    }

    dst += 4;
    src += 4;
    msk += 1;
  } while (--w);
}

static void FOG_FASTCALL raster_rgb32_span_composite_rgb24_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
  do {
    uint32_t m;
    if ((m = READ_8(msk)))
    {
      uint32_t src0 = PixFmt_RGB24::fetch(src);
      if (m != 0xFF) src0 = bytemul(((uint32_t*)dst)[0], 255 - m) + bytemul(src0, m);
      ((uint32_t*)dst)[0] = src0;
    }

    dst += 4;
    src += 3;
    msk += 1;
  } while (--w);
}

static void FOG_FASTCALL raster_rgb32_span_composite_indexed_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w, const Rgba* pal)
{
  do {
    uint32_t m;
    if ((m = READ_8(msk)))
    {
      uint32_t src0 = pal[src[0]];
      uint32_t a0 = src0 >> 24;
      if (m != 0xFF) a0 = div255(a0 * m);
      ((uint32_t*)dst)[0] = bytemul(((uint32_t*)dst)[0], 255 - a0) + bytemul(src0, a0);
    }

    dst += 4;
    src += 1;
    msk += 1;
  } while (--w);
}

// ============================================================================
// [Fog::Raster - Raster - Rgb24]
// ============================================================================

static void FOG_FASTCALL raster_rgb24_pixel(
  uint8_t* dst, uint32_t src)
{
}

static void FOG_FASTCALL raster_rgb24_pixel_a8(
  uint8_t* dst, uint32_t src, uint32_t msk)
{
}

static void FOG_FASTCALL raster_rgb24_span_solid(
  uint8_t* dst, uint32_t src, sysint_t w)
{
}

static void FOG_FASTCALL raster_rgb24_span_solid_a8(
  uint8_t* dst, uint32_t src, const uint8_t* msk, sysint_t w)
{
}

static void FOG_FASTCALL raster_rgb24_span_composite_argb32(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
}

static void FOG_FASTCALL raster_rgb24_span_composite_prgb32(
  uint8_t* dst, const uint8_t* src, sysint_t w)
{
}

static void FOG_FASTCALL raster_rgb24_span_composite_argb32_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
}

static void FOG_FASTCALL raster_rgb24_span_composite_prgb32_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
}

static void FOG_FASTCALL raster_rgb24_span_composite_rgb32_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
}

static void FOG_FASTCALL raster_rgb24_span_composite_rgb24_a8(
  uint8_t* dst, const uint8_t* src, const uint8_t* msk, sysint_t w)
{
}

} // Raster namespace
} // Fog namespace
