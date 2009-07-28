// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

#include <Fog/Core/Algorithms.h>
#include <Fog/Core/Error.h>
#include <Fog/Core/Hash.h>
#include <Fog/Core/Locale.h>
#include <Fog/Core/Sequence.h>
#include <Fog/Core/String.h>
#include <Fog/Core/StringUtil.h>
#include <Fog/Core/TextCodec.h>
#include <Fog/Core/Thread.h>
#include <Fog/Core/Vector.h>

#include <stdarg.h>

#define __G_GENERATE

#define __G_SIZE 1
#include <Fog/Core/String_gen.cpp>
#undef __G_SIZE

#define __G_SIZE 2
#include <Fog/Core/String_gen.cpp>
#undef __G_SIZE

#define __G_SIZE 4
#include <Fog/Core/String_gen.cpp>
#undef __G_SIZE

#undef __G_GENERATE

FOG_INIT_DECLARE err_t fog_string_init(void)
{
  using namespace Fog;

  String8::Data* d8 = String8::sharedNull.instancep();
  d8->refCount.init(1);
  d8->flags |= String8::Data::IsNull | String8::Data::IsSharable;
  d8->hashCode = 0;
  d8->capacity = 0;
  d8->length = 0;
  memset(d8->data, 0, sizeof(d8->data));

  String16::Data* d16 = String16::sharedNull.instancep();
  d16->refCount.init(1);
  d16->flags |= String16::Data::IsNull | String16::Data::IsSharable;
  d16->hashCode = 0;
  d16->capacity = 0;
  d16->length = 0;
  memset(d16->data, 0, sizeof(d16->data));

  String32::Data* d32 = String32::sharedNull.instancep();
  d32->refCount.init(1);
  d32->flags |= String32::Data::IsNull | String32::Data::IsSharable;
  d32->hashCode = 0;
  d32->capacity = 0;
  d32->length = 0;
  memset(d32->data, 0, sizeof(d32->data));

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_string_shutdown(void)
{
}
