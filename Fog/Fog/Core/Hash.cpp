// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Assert.h>
#include <Fog/Core/Hash.h>
#include <Fog/Core/Error.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/Std.h>
#include <Fog/Core/String.h>
#include <Fog/Core/StringUtil.h>

namespace Fog {

// ===========================================================================
// [Fog::hashData]
// ===========================================================================

// From OSSP val - Value Access <http://www.ossp.org/pkg/lib/val/>
//
// BJDDJ Hash Function (Bob Jenkins, Dr. Dobbs Journal):
// This is a very complex but also very good hash function, as proposed
// in the March'97 issue of Dr. Dobbs Journal (DDJ) by Bob Jenkins (see
// http://burtleburtle.net/bob/hash/doobs.html for online version). He
// showed that this hash function has both very good distribution and
// performance and our own hash function comparison confirmed this. The
// only difference to the original function of B.J. here is that our
// version doesn't provide the `level' (= previous hash) argument for
// consistency reasons with the other hash functions (i.e. same function
// signature). It can be definetely recommended as a good general
// purpose hash function.
uint32_t hashData(const void* data, sysuint_t size)
{
  const uint8_t* k = (const uint8_t*)data;
  uint32_t a, b, c;
  sysuint_t len;

  // some abbreviations
#define ub4 uint32_t
#define mix(a,b,c) { \
      a -= b; a -= c; a ^= (c>>13); \
      b -= c; b -= a; b ^= (a<< 8); \
      c -= a; c -= b; c ^= (b>>13); \
      a -= b; a -= c; a ^= (c>>12); \
      b -= c; b -= a; b ^= (a<<16); \
      c -= a; c -= b; c ^= (b>> 5); \
      a -= b; a -= c; a ^= (c>> 3); \
      b -= c; b -= a; b ^= (a<<10); \
      c -= a; c -= b; c ^= (b>>15); \
  }

  // setup the internal state
  len = size;
  // the golden ratio; an arbitrary value
  a = b = 0x9E3779B9;
  c = 0;

  // handle most of the key
  while (len >= 12)
  {
    a += (k[0] + ((ub4)k[1]<<8) + ((ub4)k[ 2]<<16) + ((ub4)k[ 3]<<24));
    b += (k[4] + ((ub4)k[5]<<8) + ((ub4)k[ 6]<<16) + ((ub4)k[ 7]<<24));
    c += (k[8] + ((ub4)k[9]<<8) + ((ub4)k[10]<<16) + ((ub4)k[11]<<24));
    mix(a, b, c);
    k += 12;
    len -= 12;
  }

  // handle the last 11 bytes
  c += (uint32_t)size;
  switch(len)
  {
    // all the case statements fall through
    case 11: c+=((ub4)k[10]<<24);
    case 10: c+=((ub4)k[ 9]<<16);
    case 9 : c+=((ub4)k[ 8]<< 8);
    // the first byte of c is reserved for the length
    case 8 : b+=((ub4)k[ 7]<<24);
    case 7 : b+=((ub4)k[ 6]<<16);
    case 6 : b+=((ub4)k[ 5]<< 8);
    case 5 : b+=k[4];
    case 4 : a+=((ub4)k[ 3]<<24);
    case 3 : a+=((ub4)k[ 2]<<16);
    case 2 : a+=((ub4)k[ 1]<< 8);
    case 1 : a+=k[0];
    // case 0: nothing left to add
  }
  mix(a, b, c);

#undef ub4
#undef mix

  // report the result
  return c;
}

// ===========================================================================
// [Fog::hashString]
// ===========================================================================

uint32_t hashString(const Char8* key, sysuint_t length)
{
  if (length == DetectLength) length = StringUtil::len(key);
  return hashData((const void*)key, length);
}

uint32_t hashString(const Char16* key, sysuint_t length)
{
  if (length == DetectLength) length = StringUtil::len(key);
  return hashData((const void*)key, length << 1);
}

uint32_t hashString(const Char32* key, sysuint_t length)
{
  if (length == DetectLength) length = StringUtil::len(key);
  return hashData((const void*)key, length << 2);
}

// ============================================================================
// [Fog::Hash_Abstract]
// ============================================================================

Static<Hash_Abstract::Data> Hash_Abstract::sharedNull;

Hash_Abstract::Data* Hash_Abstract::_allocData(sysuint_t capacity)
{
  sysuint_t dsize = 
    sizeof(Data) - sizeof(void*) + capacity * sizeof(void**);

  Data* d = (Data*)Memory::calloc(dsize);
  if (!d) return NULL;

  d->refCount.init(1);
  d->capacity = capacity;

  d->expandCapacity = _calcExpandCapacity(capacity);
  d->expandLength = (sysuint_t)((sysint_t)d->capacity * 0.92);

  d->shrinkCapacity = _calcShrinkCapacity(capacity);
  d->shrinkLength = (sysuint_t)((sysint_t)d->shrinkCapacity * 0.70);

  return d;
}

void Hash_Abstract::_freeData(Data* d)
{
  Memory::free(d);
}

sysuint_t Hash_Abstract::_calcExpandCapacity(sysuint_t capacity)
{
  static const sysuint_t threshold = 1024*1024*4;

  if (capacity < threshold)
    return capacity << 1;
  else
    return capacity + threshold;
}

sysuint_t Hash_Abstract::_calcShrinkCapacity(sysuint_t capacity)
{
  static const sysuint_t threshold = 1024*1024*4;

  if (capacity < threshold)
    return capacity >> 1;
  else
    return capacity - threshold;
}

bool Hash_Abstract::_rehash(sysuint_t bc)
{
  Data* newd = _allocData(bc);
  if (!newd) return false;

  sysuint_t i, len = _d->capacity;
  for (i = 0; i < len; i++)
  {
    Node* node = (Node*)(_d->buckets[i]);
    while (node)
    {
      uint32_t hashMod = node->hashCode % bc;
      Node* next = node->next;

      node->next = (Node*)newd->buckets[hashMod];
      newd->buckets[hashMod] = node;

      node = next;
    }
  }

  newd->length = _d->length;
  Data* old = AtomicBase::ptr_setXchg(&_d, newd);

  old->refCount.dec();
  if (old != sharedNull.instancep()) _freeData(old);
  
  return true;
}

void Hash_Abstract::_Iterator::_toBegin()
{
  if (FOG_UNLIKELY(_hash->isEmpty()))
  {
    _node = NULL;
    _index = InvalidIndex;
    return;
  }

  sysuint_t i, len = _hash->_d->capacity;
  Hash_Abstract::Node* node;

  for (i = 0; i < len; i++)
  {
    node = (Hash_Abstract::Node*)(_hash->_d->buckets[i]);
    if (node) break;
  }

  // We checked isEmpty(), node can't be NULL
  FOG_ASSERT(node != NULL);

  _node = node;
  _index = i;
}

void Hash_Abstract::_Iterator::_toNext()
{
  sysuint_t i, len = _hash->_d->capacity;
  Hash_Abstract::Node* node = _node;

  // Bail out if there is problem
  if (FOG_UNLIKELY(node == NULL)) return;

  // If node exists index must point to correct bucket
  FOG_ASSERT(_index < len);

  // Chains
  if (FOG_UNLIKELY(node->next))
  {
    _node = node->next;
    return;
  }

  for (i = _index + 1; i < len; i++)
  {
    node = (Hash_Abstract::Node*)(_hash->_d->buckets[i]);
    if (node) { _node = node; _index = i; return; }
  }

  _node = NULL;
  _index = InvalidIndex;
}

Hash_Abstract::Node* Hash_Abstract::_Iterator::_removeCurrent()
{
  Node* node = _node;
  sysuint_t i = _index;

  if (node == NULL) return NULL;

  _toNext();

  Node* n = (Node*)_hash->_d->buckets[i];
  Node* prev = NULL;

  // Find node to remove. We need 'prev' pointer to remove it from chains.
  while (n != node)
  { 
    prev = node;
    node = node->next;
    // Can't go out of range, becuase we know node bucked index.
    FOG_ASSERT(node != NULL);
  }

  if (prev)
    prev->next = node->next;
  else
    _hash->_d->buckets[i] = node->next;

  _hash->_d->length--;
  return n;
}

} // Fog namespace

// ===========================================================================
// [Library Initializers]
// ===========================================================================

FOG_INIT_DECLARE err_t fog_hash_init(void)
{
  using namespace Fog;

  Hash_Abstract::Data* d = Hash_Abstract::sharedNull.instancep();
  d->refCount.init(1);
  d->capacity = 1;
  d->length = 0;
  d->expandCapacity = 0;
  d->expandLength = 128;
  d->shrinkCapacity = 0;
  d->shrinkLength = 0;
  d->buckets[0] = NULL;

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_hash_shutdown(void)
{
  using namespace Fog;

  Hash_Abstract::Data* d = Hash_Abstract::sharedNull.instancep();
  d->refCount.dec();
}
