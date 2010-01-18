// [Fog/Graphics Library - Public API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Graphics/Glyph.h>

namespace Fog {

// ============================================================================
// [Fog::Glyph]
// ============================================================================

Static<Glyph::Data> Glyph::sharedNull;

// ============================================================================
// [Fog::Glyph::Data]
// ============================================================================

Glyph::Data::Data() :
  bitmapX(0),
  bitmapY(0),
  beginWidth(0),
  endWidth(0),
  advance(0)
{
  refCount.init(1);
}

Glyph::Data::~Data()
{
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_glyph_init(void)
{
  using namespace Fog;

  Glyph::sharedNull.init();
  return ERR_OK;
}

FOG_INIT_DECLARE void fog_glyph_shutdown(void)
{
  using namespace Fog;

  Glyph::sharedNull.instancep()->refCount.dec();
  Glyph::sharedNull.destroy();
}
