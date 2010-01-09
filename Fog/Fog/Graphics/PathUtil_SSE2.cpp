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

static void FOG_FASTCALL translateVertex_sse2(PathVertex* data, sysuint_t length, const PointD* pt)
{
  sysuint_t i;

  __m128d m_tx_ty = _mm_loadu_pd(&pt->x);

  for (i = length; i; i--, data++)
  {
    if (data->cmd.isVertex())
    {
      __m128d m_x_y_0 = _mm_loadu_pd(&data[0].x);
      m_x_y_0 = _mm_add_pd(m_x_y_0, m_tx_ty);
      _mm_storeu_pd(&data[0].x, m_x_y_0);
    }
  }
}

static void FOG_FASTCALL translateVertex2_sse2(PathVertex* FOG_RESTRICT dst, const PathVertex* FOG_RESTRICT src, sysuint_t length, const PointD* pt)
{
  sysuint_t i;

  __m128d m_tx_ty = _mm_loadu_pd(&pt->x);

  for (i = length >> 2; i; i--, dst += 4, src += 4)
  {
    PathCmd cmd_0 = src[0].cmd;
    PathCmd cmd_1 = src[1].cmd;
    PathCmd cmd_2 = src[2].cmd;
    PathCmd cmd_3 = src[3].cmd;

    __m128d m_x_y_0 = _mm_loadu_pd(&src[0].x);
    __m128d m_x_y_1 = _mm_loadu_pd(&src[1].x);
    __m128d m_x_y_2 = _mm_loadu_pd(&src[2].x);
    __m128d m_x_y_3 = _mm_loadu_pd(&src[3].x);

    dst[0].cmd = cmd_0;
    dst[1].cmd = cmd_1;
    dst[2].cmd = cmd_2;
    dst[3].cmd = cmd_3;

    if (FOG_LIKELY(cmd_0.isVertex())) m_x_y_0 = _mm_add_pd(m_x_y_0, m_tx_ty);
    if (FOG_LIKELY(cmd_1.isVertex())) m_x_y_1 = _mm_add_pd(m_x_y_1, m_tx_ty);
    if (FOG_LIKELY(cmd_2.isVertex())) m_x_y_2 = _mm_add_pd(m_x_y_2, m_tx_ty);
    if (FOG_LIKELY(cmd_3.isVertex())) m_x_y_3 = _mm_add_pd(m_x_y_3, m_tx_ty);

    _mm_storeu_pd(&dst[0].x, m_x_y_0);
    _mm_storeu_pd(&dst[1].x, m_x_y_1);
    _mm_storeu_pd(&dst[2].x, m_x_y_2);
    _mm_storeu_pd(&dst[3].x, m_x_y_3);
  }

  for (i = length & 3; i; i--, dst++, src++)
  {
    PathCmd cmd_0 = src[0].cmd;
    __m128d m_x_y_0 = _mm_loadu_pd(&src[0].x);

    dst[0].cmd = cmd_0;
    if (cmd_0.isVertex()) m_x_y_0 = _mm_add_pd(m_x_y_0, m_tx_ty);
    _mm_storeu_pd(&dst[0].x, m_x_y_0);
  }
}

static void FOG_FASTCALL transformVertex_sse2(PathVertex* data, sysuint_t length, const Matrix* matrix)
{
  sysuint_t i;

  __m128d m_sx_sy = _mm_set_pd(matrix->sy, matrix->sx);
  __m128d m_shx_shy = _mm_set_pd(matrix->shy, matrix->shx);
  __m128d m_tx_ty = _mm_loadu_pd(&matrix->m[4]);

  for (i = length >> 2; i; i--, data += 4)
  {
    PathCmd cmd_0 = data[0].cmd;
    PathCmd cmd_1 = data[1].cmd;
    PathCmd cmd_2 = data[2].cmd;
    PathCmd cmd_3 = data[3].cmd;

    __m128d m_x_y_0 = _mm_loadu_pd(&data[0].x);
    __m128d m_x_y_1 = _mm_loadu_pd(&data[1].x);
    __m128d m_x_y_2 = _mm_loadu_pd(&data[2].x);
    __m128d m_x_y_3 = _mm_loadu_pd(&data[3].x);

    __m128d m_y_x_0 = _mm_shuffle_pd(m_x_y_0, m_x_y_0, _MM_SHUFFLE2(0, 1));
    __m128d m_y_x_1 = _mm_shuffle_pd(m_x_y_1, m_x_y_1, _MM_SHUFFLE2(0, 1));

    m_x_y_0 = _mm_mul_pd(m_x_y_0, m_sx_sy);
    m_x_y_1 = _mm_mul_pd(m_x_y_1, m_sx_sy);
    m_y_x_0 = _mm_mul_pd(m_y_x_0, m_shx_shy);
    m_y_x_1 = _mm_mul_pd(m_y_x_1, m_shx_shy);
    m_x_y_0 = _mm_add_pd(m_x_y_0, m_tx_ty);
    m_x_y_1 = _mm_add_pd(m_x_y_1, m_tx_ty);
    m_x_y_0 = _mm_add_pd(m_x_y_0, m_y_x_0);
    m_x_y_1 = _mm_add_pd(m_x_y_1, m_y_x_1);

    __m128d m_y_x_2 = _mm_shuffle_pd(m_x_y_2, m_x_y_2, _MM_SHUFFLE2(0, 1));
    __m128d m_y_x_3 = _mm_shuffle_pd(m_x_y_3, m_x_y_3, _MM_SHUFFLE2(0, 1));

    m_x_y_2 = _mm_mul_pd(m_x_y_2, m_sx_sy);
    m_x_y_3 = _mm_mul_pd(m_x_y_3, m_sx_sy);
    m_y_x_2 = _mm_mul_pd(m_y_x_2, m_shx_shy);
    m_y_x_3 = _mm_mul_pd(m_y_x_3, m_shx_shy);
    m_x_y_2 = _mm_add_pd(m_x_y_2, m_tx_ty);
    m_x_y_3 = _mm_add_pd(m_x_y_3, m_tx_ty);
    m_x_y_2 = _mm_add_pd(m_x_y_2, m_y_x_2);
    m_x_y_3 = _mm_add_pd(m_x_y_3, m_y_x_3);

    if (FOG_LIKELY(cmd_0.isVertex())) _mm_storeu_pd(&data[0].x, m_x_y_0);
    if (FOG_LIKELY(cmd_1.isVertex())) _mm_storeu_pd(&data[1].x, m_x_y_1);
    if (FOG_LIKELY(cmd_2.isVertex())) _mm_storeu_pd(&data[2].x, m_x_y_2);
    if (FOG_LIKELY(cmd_3.isVertex())) _mm_storeu_pd(&data[3].x, m_x_y_3);
  }

  for (i = length & 3; i; i--, data++)
  {
    PathCmd cmd_0 = data[0].cmd;
    if (cmd_0.isVertex())
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
}

static void FOG_FASTCALL transformVertex2_sse2(PathVertex* dst, const PathVertex* src, sysuint_t length, const Matrix* matrix)
{
  sysuint_t i;

  __m128d m_sx_sy = _mm_set_pd(matrix->sy, matrix->sx);
  __m128d m_shx_shy = _mm_set_pd(matrix->shy, matrix->shx);
  __m128d m_tx_ty = _mm_loadu_pd(&matrix->m[4]);

  for (i = length >> 2; i; i--, dst += 4, src += 4)
  {
    PathCmd cmd_0 = src[0].cmd;
    PathCmd cmd_1 = src[1].cmd;
    PathCmd cmd_2 = src[2].cmd;
    PathCmd cmd_3 = src[3].cmd;

    __m128d m_x_y_0 = _mm_loadu_pd(&src[0].x);
    __m128d m_x_y_1 = _mm_loadu_pd(&src[1].x);
    __m128d m_x_y_2 = _mm_loadu_pd(&src[2].x);
    __m128d m_x_y_3 = _mm_loadu_pd(&src[3].x);

    __m128d m_y_x_0 = _mm_shuffle_pd(m_x_y_0, m_x_y_0, _MM_SHUFFLE2(0, 1));
    __m128d m_y_x_1 = _mm_shuffle_pd(m_x_y_1, m_x_y_1, _MM_SHUFFLE2(0, 1));

    m_x_y_0 = _mm_mul_pd(m_x_y_0, m_sx_sy);
    m_x_y_1 = _mm_mul_pd(m_x_y_1, m_sx_sy);
    m_y_x_0 = _mm_mul_pd(m_y_x_0, m_shx_shy);
    m_y_x_1 = _mm_mul_pd(m_y_x_1, m_shx_shy);
    m_x_y_0 = _mm_add_pd(m_x_y_0, m_tx_ty);
    m_x_y_1 = _mm_add_pd(m_x_y_1, m_tx_ty);
    m_x_y_0 = _mm_add_pd(m_x_y_0, m_y_x_0);
    m_x_y_1 = _mm_add_pd(m_x_y_1, m_y_x_1);

    __m128d m_y_x_2 = _mm_shuffle_pd(m_x_y_2, m_x_y_2, _MM_SHUFFLE2(0, 1));
    __m128d m_y_x_3 = _mm_shuffle_pd(m_x_y_3, m_x_y_3, _MM_SHUFFLE2(0, 1));

    m_x_y_2 = _mm_mul_pd(m_x_y_2, m_sx_sy);
    m_x_y_3 = _mm_mul_pd(m_x_y_3, m_sx_sy);
    m_y_x_2 = _mm_mul_pd(m_y_x_2, m_shx_shy);
    m_y_x_3 = _mm_mul_pd(m_y_x_3, m_shx_shy);
    m_x_y_2 = _mm_add_pd(m_x_y_2, m_tx_ty);
    m_x_y_3 = _mm_add_pd(m_x_y_3, m_tx_ty);
    m_x_y_2 = _mm_add_pd(m_x_y_2, m_y_x_2);
    m_x_y_3 = _mm_add_pd(m_x_y_3, m_y_x_3);

    dst[0].cmd = cmd_0;
    dst[1].cmd = cmd_1;
    dst[2].cmd = cmd_2;
    dst[3].cmd = cmd_3;

    if (FOG_UNLIKELY(!cmd_0.isVertex())) m_x_y_0 = _mm_loadu_pd(&src[0].x);
    if (FOG_UNLIKELY(!cmd_1.isVertex())) m_x_y_1 = _mm_loadu_pd(&src[1].x);
    if (FOG_UNLIKELY(!cmd_2.isVertex())) m_x_y_2 = _mm_loadu_pd(&src[2].x);
    if (FOG_UNLIKELY(!cmd_3.isVertex())) m_x_y_3 = _mm_loadu_pd(&src[3].x);

    _mm_storeu_pd(&dst[0].x, m_x_y_0);
    _mm_storeu_pd(&dst[1].x, m_x_y_1);
    _mm_storeu_pd(&dst[2].x, m_x_y_2);
    _mm_storeu_pd(&dst[3].x, m_x_y_3);
  }

  for (i = length & 3; i; i--, dst++, src++)
  {
    PathCmd cmd_0 = src[0].cmd;
    __m128d m_x_y_0 = _mm_loadu_pd(&src[0].x);
    __m128d m_y_x_0 = _mm_shuffle_pd(m_x_y_0, m_x_y_0, _MM_SHUFFLE2(0, 1));

    m_x_y_0 = _mm_mul_pd(m_x_y_0, m_sx_sy);
    m_y_x_0 = _mm_mul_pd(m_y_x_0, m_shx_shy);
    m_x_y_0 = _mm_add_pd(m_x_y_0, m_tx_ty);
    m_x_y_0 = _mm_add_pd(m_x_y_0, m_y_x_0);

    dst[0].cmd = cmd_0;
    if (FOG_UNLIKELY(!cmd_0.isVertex())) m_x_y_0 = _mm_loadu_pd(&src[0].x);

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

  PathUtil::fm.translateVertex = PathUtil::translateVertex_sse2;
  PathUtil::fm.translateVertex2 = PathUtil::translateVertex2_sse2;

  PathUtil::fm.transformVertex = PathUtil::transformVertex_sse2;
  PathUtil::fm.transformVertex2 = PathUtil::transformVertex2_sse2;
}
