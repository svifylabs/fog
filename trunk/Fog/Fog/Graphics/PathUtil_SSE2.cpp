// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Dependencies]
#include <Fog/Core/Intrin_SSE2.h>
#include <Fog/Core/Math.h>
#include <Fog/Graphics/Matrix.h>
#include <Fog/Graphics/PathUtil.h>

namespace Fog {
namespace PathUtil {

// ============================================================================
// [Fog::PathUtil::Transformations]
// ============================================================================

static void FOG_FASTCALL transformVertex_sse2(PathVertex* data, sysuint_t length, const Matrix* matrix)
{
  __m128d m_sx_sy;
  __m128d m_shx_shy;
  __m128d m_tx_ty;

  m_sx_sy   = _mm_set_pd(matrix->sy, matrix->sx);
  m_shx_shy = _mm_set_pd(matrix->shy, matrix->shx);
  m_tx_ty   = _mm_loadu_pd(&matrix->m[4]);

  for (sysuint_t i = length >> 1; i; i--, data += 2)
  {
    uint msk = ((uint)data[0].cmd.isVertex()) | ((uint)data[1].cmd.isVertex() << 1);

    switch (msk)
    {
      case 0x0:
        break;

      case 0x1:
      {
        __m128d m_x_y_0;
        __m128d m_y_x_0;

        m_x_y_0 = _mm_loadu_pd(&data[0].x);
        m_y_x_0 = _mm_shuffle_pd(m_x_y_0, m_x_y_0, _MM_SHUFFLE2(0, 1));

        m_x_y_0 = _mm_mul_pd(m_x_y_0, m_sx_sy);
        m_y_x_0 = _mm_mul_pd(m_y_x_0, m_shx_shy);
        m_x_y_0 = _mm_add_pd(m_x_y_0, m_tx_ty);
        m_x_y_0 = _mm_add_pd(m_x_y_0, m_y_x_0);

        _mm_storeu_pd(&data[0].x, m_x_y_0);
        break;
      }

      case 0x2:
      {
        __m128d m_x_y_1;
        __m128d m_y_x_1;

        m_x_y_1 = _mm_loadu_pd(&data[1].x);
        m_y_x_1 = _mm_shuffle_pd(m_x_y_1, m_x_y_1, _MM_SHUFFLE2(0, 1));

        m_x_y_1 = _mm_mul_pd(m_x_y_1, m_sx_sy);
        m_y_x_1 = _mm_mul_pd(m_y_x_1, m_shx_shy);
        m_x_y_1 = _mm_add_pd(m_x_y_1, m_tx_ty);
        m_x_y_1 = _mm_add_pd(m_x_y_1, m_y_x_1);

        _mm_storeu_pd(&data[1].x, m_x_y_1);
        break;
      }

      case 0x3:
      {
        __m128d m_x_y_0;
        __m128d m_x_y_1;
        __m128d m_y_x_0;
        __m128d m_y_x_1;

        m_x_y_0 = _mm_loadu_pd(&data[0].x);
        m_y_x_0 = _mm_shuffle_pd(m_x_y_0, m_x_y_0, _MM_SHUFFLE2(0, 1));
        m_x_y_1 = _mm_loadu_pd(&data[1].x);
        m_y_x_1 = _mm_shuffle_pd(m_x_y_1, m_x_y_1, _MM_SHUFFLE2(0, 1));

        m_x_y_0 = _mm_mul_pd(m_x_y_0, m_sx_sy);
        m_x_y_1 = _mm_mul_pd(m_x_y_1, m_sx_sy);
        m_y_x_0 = _mm_mul_pd(m_y_x_0, m_shx_shy);
        m_y_x_1 = _mm_mul_pd(m_y_x_1, m_shx_shy);
        m_x_y_0 = _mm_add_pd(m_x_y_0, m_tx_ty);
        m_x_y_1 = _mm_add_pd(m_x_y_1, m_tx_ty);
        m_x_y_0 = _mm_add_pd(m_x_y_0, m_y_x_0);
        m_x_y_1 = _mm_add_pd(m_x_y_1, m_y_x_1);

        _mm_storeu_pd(&data[0].x, m_x_y_0);
        _mm_storeu_pd(&data[1].x, m_x_y_1);
        break;
      }
    }
  }

  if ((length & 1) && data[0].cmd.isVertex())
  {
    __m128d m_x_y_0 = _mm_loadu_pd(&data[0].x);
    __m128d m_y_x_0 = _mm_shuffle_pd(m_x_y_0, m_x_y_0, _MM_SHUFFLE2(0, 1));

    m_x_y_0 = _mm_mul_pd(m_x_y_0, m_sx_sy);
    m_y_x_0 = _mm_mul_pd(m_y_x_0, m_shx_shy);
    m_x_y_0 = _mm_add_pd(m_x_y_0, m_tx_ty);
    m_x_y_0 = _mm_add_pd(m_x_y_0, m_y_x_0);

    _mm_storeu_pd(&data[0].x, m_x_y_0);
  }
}

static void FOG_FASTCALL transformVertex2_sse2(PathVertex* dst, const PathVertex* src, sysuint_t length, const Matrix* matrix)
{
  __m128d m_sx_sy;
  __m128d m_shx_shy;
  __m128d m_tx_ty;

  m_sx_sy   = _mm_set_pd(matrix->sy, matrix->sx);
  m_shx_shy = _mm_set_pd(matrix->shy, matrix->shx);
  m_tx_ty   = _mm_loadu_pd(&matrix->m[4]);

  for (sysuint_t i = length >> 1; i; i--, dst += 2, src += 2)
  {
    PathCmd cmd0 = src[0].cmd;
    PathCmd cmd1 = src[1].cmd;

    __m128d m_x_y_0 = _mm_loadu_pd(&src[0].x);
    __m128d m_x_y_1 = _mm_loadu_pd(&src[1].x);

    uint msk = ((uint)cmd0.isVertex()) | ((uint)cmd1.isVertex() << 1);

    switch (msk)
    {
      case 0x0:
        break;

      case 0x1:
      {
        __m128d m_y_x_0;

        m_y_x_0 = _mm_shuffle_pd(m_x_y_0, m_x_y_0, _MM_SHUFFLE2(0, 1));

        m_x_y_0 = _mm_mul_pd(m_x_y_0, m_sx_sy);
        m_y_x_0 = _mm_mul_pd(m_y_x_0, m_shx_shy);
        m_x_y_0 = _mm_add_pd(m_x_y_0, m_tx_ty);
        m_x_y_0 = _mm_add_pd(m_x_y_0, m_y_x_0);
        break;
      }

      case 0x2:
      {
        __m128d m_y_x_1;

        m_y_x_1 = _mm_shuffle_pd(m_x_y_1, m_x_y_1, _MM_SHUFFLE2(0, 1));

        m_x_y_1 = _mm_mul_pd(m_x_y_1, m_sx_sy);
        m_y_x_1 = _mm_mul_pd(m_y_x_1, m_shx_shy);
        m_x_y_1 = _mm_add_pd(m_x_y_1, m_tx_ty);
        m_x_y_1 = _mm_add_pd(m_x_y_1, m_y_x_1);
        break;
      }

      case 0x3:
      {
        __m128d m_y_x_0;
        __m128d m_y_x_1;

        m_y_x_0 = _mm_shuffle_pd(m_x_y_0, m_x_y_0, _MM_SHUFFLE2(0, 1));
        m_y_x_1 = _mm_shuffle_pd(m_x_y_1, m_x_y_1, _MM_SHUFFLE2(0, 1));

        m_x_y_0 = _mm_mul_pd(m_x_y_0, m_sx_sy);
        m_x_y_1 = _mm_mul_pd(m_x_y_1, m_sx_sy);
        m_y_x_0 = _mm_mul_pd(m_y_x_0, m_shx_shy);
        m_y_x_1 = _mm_mul_pd(m_y_x_1, m_shx_shy);
        m_x_y_0 = _mm_add_pd(m_x_y_0, m_tx_ty);
        m_x_y_1 = _mm_add_pd(m_x_y_1, m_tx_ty);
        m_x_y_0 = _mm_add_pd(m_x_y_0, m_y_x_0);
        m_x_y_1 = _mm_add_pd(m_x_y_1, m_y_x_1);
        break;
      }
    }

    dst[0].cmd = cmd0;
    dst[1].cmd = cmd1;

    _mm_storeu_pd(&dst[0].x, m_x_y_0);
    _mm_storeu_pd(&dst[1].x, m_x_y_1);
  }

  if ((length & 1))
  {
    PathCmd cmd0 = src[0].cmd;
    __m128d m_x_y_0 = _mm_loadu_pd(&src[0].x);

    if (cmd0.isVertex())
    {
      __m128d m_y_x_0 = _mm_shuffle_pd(m_x_y_0, m_x_y_0, _MM_SHUFFLE2(0, 1));

      m_x_y_0 = _mm_mul_pd(m_x_y_0, m_sx_sy);
      m_y_x_0 = _mm_mul_pd(m_y_x_0, m_shx_shy);
      m_x_y_0 = _mm_add_pd(m_x_y_0, m_tx_ty);
      m_x_y_0 = _mm_add_pd(m_x_y_0, m_y_x_0);
    }

    dst[0].cmd = cmd0;
    _mm_storeu_pd(&dst[0].x, m_x_y_0);
  }
}

} // PathUtil namespace
} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE void fog_pathutil_init_sse2(void)
{
  using namespace Fog;

  PathUtil::fm.transformVertex = PathUtil::transformVertex_sse2;
  PathUtil::fm.transformVertex2 = PathUtil::transformVertex2_sse2;
}
