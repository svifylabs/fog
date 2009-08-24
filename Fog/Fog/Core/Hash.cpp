// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled Headers]
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
