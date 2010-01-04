// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Graphics/TextOnPath.h>
#include <Fog/Graphics/Matrix.h>

namespace Fog {

// ============================================================================
// [Fog::PathTransform]
// ============================================================================

PathTransform::PathTransform()
{
}

PathTransform::~PathTransform()
{
}

// ============================================================================
// [Fog::TextOnPath]
// ============================================================================

TextOnPath::TextOnPath() :
  _calcLength(0.0),
  _baseLength(0.0)
{
}

TextOnPath::~TextOnPath()
{
}

err_t TextOnPath::transform(Path& dst, const Path& src) const
{
  return ERR_RT_NOT_IMPLEMENTED;
}

err_t TextOnPath::setPath(const Path& path, const Matrix* matrix)
{
  // Use larger scaling factor to increase path on path precision.
  static const double scaleFactor = 3.0;

  err_t err = path.flattenTo(_path, matrix, (matrix ? matrix->getAverageScaling() : 1.0) * scaleFactor);
  if (err) return err;

  _dist.resize(_path.getLength());

  return ERR_OK;
}

err_t TextOnPath::setBaseLength(double baseLength)
{
  _baseLength = baseLength;
}

} // Fog namespace
