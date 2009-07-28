// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/AutoLock.h>
#include <Fog/Core/Assert.h>
#include <Fog/Core/Hash.h>
#include <Fog/Core/Lock.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/String.h>
#include <Fog/Core/StringCache.h>
#include <Fog/Core/StringUtil.h>

#include <Fog/Core/TextCodec.h>

// [Fog::StringCache - local variables]

namespace Fog {

// ============================================================================
// [Fog::StringCache_Local]
// ============================================================================

struct StringCache_Local
{
  Lock lock;
  Hash<String32, StringCache*> hash;
};

static Static<StringCache_Local> stringcache_local;

// ============================================================================
// [Fog::StringCache]
// ============================================================================

StringCache::StringCache(
  const char* strings,
  sysuint_t length,
  sysuint_t count)
{
  const Char8* pBeg = reinterpret_cast<const Char8*>(strings);
  const Char8* pCur = reinterpret_cast<const Char8*>(strings);
  String32* pData = reinterpret_cast<String32*>(_data);
  uint8_t* pMemory = reinterpret_cast<uint8_t*>(&pData[count]);
  sysuint_t counter = 0;

  for (;;)
  {
    if (!*pCur)
    {
      String32::Data* d = (String32::Data*)pMemory;
      sysuint_t len = (sysuint_t)(pCur - pBeg);

      d->refCount.init(1);
      d->flags = String32::Data::IsSharable;
      d->length = len;
      d->capacity = len;
      StringUtil::copy(d->data, pBeg, len);

      pMemory += String32::Data::sizeFor(len);
      new ((void*)pData) String32(d);

      pData++;
      counter++;

      pBeg = ++pCur;
      if (!*pBeg) break;
    }
    else
      pCur++;
  }
  FOG_ASSERT(counter == count);

  _count = counter;
}

StringCache::~StringCache()
{
#if defined(FOG_DEBUG)
  String32* pData = reinterpret_cast<String32*>(_data);
  sysuint_t i;

  for (i = count(); i; i--, pData++)
  {
    FOG_ASSERT_X(pData->_d->refCount.get() == 1, "Fog::~StringCache() - String reference is not one, it's leaked!");

    if (pData->_d->refCount.get() > 1)
    {
      String8 b;
      TextCodec::utf8().fromUtf32(b, Stub8((Char8*)pData->_d->data, pData->_d->length * 4));
      FOG_ASSERT_X(0, b.cStr());
    }
  }
#endif // FOG_DEBUG
}

StringCache* StringCache::create(
  const char* strings,
  sysuint_t length,
  sysuint_t count,
  const String32& name)
{
  StringCache* self;
  sysuint_t alloc;

  if (!name.isEmpty() && (self = getCacheByName(name))) return self;

  // Get count of 'strings'
  if (count == DetectLength) count = StringUtil::countOf(
    strings,
    // NOTE, sizeof() always gives size with null terminator, so for example
    // char[] a = "abc" will give us 4 ! This is reason why we are decrementing
    // it here
    length-1,
    '\0', CaseSensitive);

  alloc =
    // Fog::StringCache structure
    sizeof(StringCache) +
    // Fog::String32 structure
    sizeof(String32) * count +
    // Fog::String32::Data structure
    sizeof(String32::Data) * count +
    // Fog::String32::Data contains null terminator, so we will decrement it.
    sizeof(Char32) * (length - count);

  if ((self = (StringCache*)Memory::alloc(alloc)) == NULL)
  {
    return NULL;
  }

  new (self) StringCache(strings, length, count);

  if (!name.isEmpty())
  {
    AutoLock locked(stringcache_local.instance().lock);

    self->_name = name;
    self->_name.squeeze();

    stringcache_local.instance().hash.put(self->_name, self);
  }

  return self;
}

void StringCache::destroy(StringCache* cache)
{
  FOG_ASSERT(cache);

  if (!cache->name().isEmpty())
  {
    AutoLock locked(stringcache_local.instance().lock);

    if (!stringcache_local.instance().hash.remove(cache->name()))
    {
      // ou, this is not possible. Each named cache must be in hash
      FOG_ASSERT(0);
    }
  }

  cache->~StringCache();
  Memory::free(cache);
}

StringCache* StringCache::getCacheByName(const String32& name)
{
  if (name.isEmpty()) return NULL;

  AutoLock locked(stringcache_local.instance().lock);
  return stringcache_local->hash.value(name, NULL);
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_stringcache_init(void)
{
  using namespace Fog;

  stringcache_local.init();
  return Error::Ok;
}

FOG_INIT_DECLARE void fog_stringcache_shutdown(void)
{
  using namespace Fog;

  // Free cache here, because all loadable modules and libraries can define cache,
  // but it's guaranted that they will be valid until here.
  stringcache_local.destroy();
}
