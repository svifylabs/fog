// [Fog/UI Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/UI/LayoutItem.h>

FOG_IMPLEMENT_OBJECT(Fog::LayoutItem)

namespace Fog {

LayoutItem::LayoutItem() :
  _sizeHint(0, 0),
  _minimumSize(0, 0),
  _maximumSize(65535, 65535),
  _dirtyLayout(true),
  _hasHeightForWidth(false)
{
}

LayoutItem::~LayoutItem() 
{
}

} // Fog namespace
