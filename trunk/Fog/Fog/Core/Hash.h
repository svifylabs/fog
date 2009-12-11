// [Fog/Core Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_HASH_H
#define _FOG_CORE_HASH_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Core/Assert.h>
#include <Fog/Core/Atomic.h>
#include <Fog/Core/Char.h>
#include <Fog/Core/Constants.h>
#include <Fog/Core/HashUtil.h>
#include <Fog/Core/List.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/SequenceInfo.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/Std.h>
#include <Fog/Core/TypeInfo.h>

namespace Fog {

//! @addtogroup Fog_Core
//! @{

// ============================================================================
// [Fog::Hash_Abstract]
// ============================================================================

//! @brief Abstract hash container.
struct FOG_API Hash_Abstract
{
  // [Node]

  struct FOG_HIDDEN Node
  {
    Node* next;
    uint32_t hashCode;
  };

  // [Data]

  struct FOG_HIDDEN Data
  {
    //! @brief Reference count.
    mutable Atomic<sysuint_t> refCount;

    //! @brief Count of buckets.
    sysuint_t capacity;
    //! @brief Count of nodes.
    sysuint_t length;

    //! @brief Count of buckets we will expand to if length exceeds _expandLength.
    sysuint_t expandCapacity;
    //! @brief Count of nodes to grow.
    sysuint_t expandLength;

    //! @brief Count of buckets we will shrink to if length gets _shinkLength.
    sysuint_t shrinkCapacity;
    //! @brief Count of nodes to shrink.
    sysuint_t shrinkLength;

    //! @brief Buckets.
    void* buckets[1];

    FOG_INLINE Data* ref() const
    {
      refCount.inc();
      return const_cast<Data*>(this);
    }
  };

  static Static<Data> sharedNull;

  FOG_INLINE sysuint_t getCapacity() const { return _d->capacity; }
  FOG_INLINE sysuint_t getLength() const { return _d->length; }
  FOG_INLINE bool isEmpty() const { return _d->length == 0; }

  FOG_INLINE bool isDetached() const { return _d->refCount.get() == 1; }
  FOG_INLINE sysuint_t refCount() const { return _d->refCount.get(); }

  // [Abstract Iterator]

  struct FOG_API _Iterator
  {
    FOG_INLINE _Iterator(Hash_Abstract& hash) :
      _hash(&hash),
      _node(NULL),
      _index(INVALID_INDEX)
    {}

    FOG_INLINE _Iterator(Hash_Abstract* hash) :
      _hash(hash),
      _node(NULL),
      _index(INVALID_INDEX)
    {}

    FOG_INLINE bool isValid() const
    { return _node != NULL; }

    void _toBegin();
    void _toNext();
    Node* _removeCurrent();

  protected:
    Hash_Abstract* _hash;
    Node* _node;
    sysuint_t _index;
  };

  // [Helpers]

  bool _rehash(sysuint_t bc);

  static Data* _allocData(sysuint_t capacity);
  static void _freeData(Data* d);

  static sysuint_t _calcExpandCapacity(sysuint_t capacity);
  static sysuint_t _calcShrinkCapacity(sysuint_t capacity);

  // [Members]

  FOG_DECLARE_D(Data)
};

// ============================================================================
// [Fog::Hash]
// ============================================================================

template<typename KeyT, typename ValueT>
struct Hash : public Hash_Abstract
{
  struct Node
  {
    FOG_INLINE Node(uint32_t hashCode, const KeyT& key) :
      next(NULL), hashCode(hashCode), key(key) {}

    FOG_INLINE Node(uint32_t hashCode, const KeyT& key, const ValueT& value) :
      next(NULL), hashCode(hashCode), key(key), value(value) {}

    FOG_INLINE ~Node() {};

    Node* next;
    uint32_t hashCode;
    KeyT key;
    ValueT value;
  };

  struct Data : public Hash_Abstract::Data
  {
    void deref()
    {
      if (refCount.deref())
      {
        clear();
        Hash_Abstract::_freeData(this);
      }
    }

    void clear()
    {
      sysuint_t i, len = capacity;

      for (i = 0; i < len; i++)
      {
        Node* node = (Node*)(buckets[i]);
        if (node)
        {
          do {
            Node* next = node->next;
            delete node;
            node = next;
          } while (node);
          buckets[i] = NULL;
        }
      }
    }
  };

  Hash()
  { _d = sharedNull->ref(); }

  Hash(const Hash<KeyT, ValueT>& other)
  { _d = other._d->ref(); }

  ~Hash()
  { ((Data*)_d)->deref(); }

  FOG_INLINE bool detach()
  { return (!isDetached()) ? _detach() : true; }

  bool _detach(Node* exclude = NULL);

  void clear();

  bool contains(const KeyT& key) const;

  err_t put(const KeyT& key, const ValueT& value, bool replace = true);
  bool remove(const KeyT& key);

  ValueT* get(const KeyT& key) const;
  ValueT* mod(const KeyT& key);

  Node* _getNode(const KeyT& key) const;

  ValueT value(const KeyT& key) const;
  ValueT value(const KeyT& key, const ValueT& defaultValue) const;

  List<KeyT> keys() const;
  List<KeyT> keys(const ValueT& value) const;

  Hash<KeyT, ValueT>& operator=(const Hash<KeyT, ValueT>& other)
  { ((Data*)AtomicBase::ptr_setXchg(&_d, other._d->ref()))->deref(); return *this; }

  FOG_INLINE ValueT* operator[](const KeyT& key)
  { return mod(key); }

  struct ConstIterator : public Hash_Abstract::_Iterator
  {
    FOG_INLINE ConstIterator(const Hash<KeyT, ValueT>& hash) :
      _Iterator((Hash_Abstract *)&hash)
    {}

    FOG_INLINE ConstIterator(const Hash<KeyT, ValueT>* hash) :
      _Iterator((Hash_Abstract *)hash)
    {}

    FOG_INLINE ConstIterator& toStart()
    { _toBegin(); return *this; }

    FOG_INLINE ConstIterator& toNext()
    { _toNext(); return *this; }

    FOG_INLINE const KeyT& key() const
    { return ((Node*)_node)->key; }

    FOG_INLINE const ValueT& value() const
    { return ((Node*)_node)->value; }
  };

  struct MutableIterator : public Hash_Abstract::_Iterator
  {
    FOG_INLINE MutableIterator(Hash<KeyT, ValueT>& hash) :
      _Iterator((Hash_Abstract *)&hash)
    { ((Hash*)_hash)->detach(); }

    FOG_INLINE MutableIterator(Hash<KeyT, ValueT>* hash) :
      _Iterator((Hash_Abstract *)hash)
    { ((Hash*)_hash)->detach(); }

    FOG_INLINE MutableIterator& toStart()
    { _toBegin(); return *this; }

    FOG_INLINE MutableIterator& toNext()
    { _toNext(); return *this; }

    FOG_INLINE const KeyT& key() const
    { return ((Node*)_node)->key; }

    FOG_INLINE const ValueT& value() const
    { return ((Node*)_node)->value; }

    FOG_INLINE ValueT& value()
    { return ((Node*)_node)->value; }

    FOG_INLINE void remove()
    {
      delete (Node*)_removeCurrent();
    }
  };
};

template<typename KeyT, typename ValueT>
bool Hash<KeyT, ValueT>::_detach(Node* exclude)
{
  Data* newd = (Data*)_allocData(_d->capacity);
  if (!newd) return false;

  sysuint_t i, len = _d->capacity;
  sysuint_t bc = newd->capacity;

  for (i = 0; i < len; i++)
  {
    Node* node = (Node*)(_d->buckets[i]);
    while (node)
    {
      if (FOG_LIKELY(node != exclude))
      {
        uint32_t hashMod = node->hashCode % bc;
        Node* n = new(std::nothrow) Node(node->hashCode, node->key, node->value);
        if (FOG_UNLIKELY(!n)) goto alloc_fail;

        n->next = (Node*)newd->buckets[hashMod];
        newd->buckets[hashMod] = n;
      }
      node = node->next;
    }
  }

  newd->length = _d->length;
  if (exclude) newd->length--;

  ((Data*)AtomicBase::ptr_setXchg(&_d, (Hash_Abstract::Data*)newd))->deref();
  return true;

alloc_fail:
  newd->clear();
  _freeData(newd);
  return false;
}

template<typename KeyT, typename ValueT>
void Hash<KeyT, ValueT>::clear()
{
  ((Data*)AtomicBase::ptr_setXchg(&_d, sharedNull->ref()))->deref();
}

template<typename KeyT, typename ValueT>
bool Hash<KeyT, ValueT>::contains(const KeyT& key) const
{
  if (!_d->length) return false;

  uint32_t hashCode = HashUtil::getHashCode(key);
  uint32_t hashMod = hashCode % getCapacity();

  Node* node = (Node*)(_d->buckets[hashMod]);

  while (node)
  {
    if (node->hashCode == hashCode && node->key == key) return true;
    node = node->next;
  }
  return false;
}

template<typename KeyT, typename ValueT>
err_t Hash<KeyT, ValueT>::put(const KeyT& key, const ValueT& value, bool replace)
{
  if (!_d->length && !_rehash(32)) return ERR_RT_OUT_OF_MEMORY;

  detach();

  uint32_t hashCode = HashUtil::getHashCode(key);
  uint32_t hashMod = hashCode % getCapacity();

  Node* node = (Node*)(_d->buckets[hashMod]);
  Node* prev = NULL;

  while (node)
  {
    if (node->hashCode == hashCode && node->key == key) break;
    prev = node;
    node = node->next;
  }

  if (node)
  {
    if (!replace) return ERR_RT_OBJECT_ALREADY_EXISTS;
    node->value = value;
  }
  else
  {
    node = new(std::nothrow) Node(hashCode, key, value);
    if (!node) return ERR_RT_OUT_OF_MEMORY;

    if (prev)
      prev->next = node;
    else
      _d->buckets[hashMod] = node;
    if (++_d->length >= _d->expandLength) _rehash(_d->expandCapacity);
  }
  return ERR_OK;
}

template<typename KeyT, typename ValueT>
bool Hash<KeyT, ValueT>::remove(const KeyT& key)
{
  if (!_d->length) return false;

  uint32_t hashCode = HashUtil::getHashCode(key);
  uint32_t hashMod = hashCode % getCapacity();

  Node* node = (Node*)(_d->buckets[hashMod]);
  Node* prev = NULL;

  while (node)
  {
    if (node->hashCode == hashCode && node->key == key)
    {
      if (FOG_UNLIKELY(!isDetached())) return _detach(node);

      if (prev)
        prev->next = node->next;
      else
        _d->buckets[hashMod] = node->next;

      delete node;
      if (--_d->length <= _d->shrinkLength) _rehash(_d->shrinkCapacity);
      return true;
    }
    prev = node;
    node = node->next;
  }

  return false;
}

template<typename KeyT, typename ValueT>
ValueT* Hash<KeyT, ValueT>::get(const KeyT& key) const
{
  if (!_d->length) return NULL;

  uint32_t hashCode = HashUtil::getHashCode(key);
  uint32_t hashMod = hashCode % getCapacity();

  Node* node = (Node*)(_d->buckets[hashMod]);

  while (node)
  {
    if (node->hashCode == hashCode && node->key == key)
    {
      return &node->value;
    }
    node = node->next;
  }
  return NULL;
}

template<typename KeyT, typename ValueT>
ValueT* Hash<KeyT, ValueT>::mod(const KeyT& key)
{
  if (!_d->length && !_rehash(32)) return NULL;

  detach();

  uint32_t hashCode = HashUtil::getHashCode(key);
  uint32_t hashMod = hashCode % getCapacity();

  Node* node = (Node*)(_d->buckets[hashMod]);
  Node* prev = NULL;

  while (node)
  {
    if (node->hashCode == hashCode && node->key == key) break;
    prev = node;
    node = node->next;
  }

  if (!node)
  {
    node = new(std::nothrow) Node(hashCode, key);
    if (!node) return NULL;

    if (prev)
      prev->next = node;
    else
      _d->buckets[hashMod] = node;
    if (++_d->length >= _d->expandLength) _rehash(_d->expandCapacity);
  }

  return &node->value;
}

template<typename KeyT, typename ValueT>
typename Hash<KeyT, ValueT>::Node* Hash<KeyT, ValueT>::_getNode(const KeyT& key) const
{
  if (!_d->length) return NULL;

  uint32_t hashCode = HashUtil::getHashCode(key);
  uint32_t hashMod = hashCode % getCapacity();

  Node* node = (Node*)(_d->buckets[hashMod]);

  while (node)
  {
    if (node->hashCode == hashCode && node->key == key) return node;
    node = node->next;
  }
  return NULL;
}

template<typename KeyT, typename ValueT>
ValueT Hash<KeyT, ValueT>::value(const KeyT& key) const
{
  const ValueT* vptr = get(key);
  return vptr ? ValueT(*vptr) : ValueT();
}

template<typename KeyT, typename ValueT>
ValueT Hash<KeyT, ValueT>::value(const KeyT& key, const ValueT& defaultValue) const
{
  const ValueT* vptr = get(key);
  return ValueT(vptr ? *vptr : defaultValue);
}

template<typename KeyT, typename ValueT>
List<KeyT> Hash<KeyT, ValueT>::keys() const
{
  List<KeyT> result;
  sysuint_t i, len = _d->capacity;

  result.reserve(len);

  for (i = 0; i < len; i++)
  {
    Node* node = (Node*)(_d->buckets[i]);
    while (node)
    {
      result.append(node->key);
      node = node->next;
    }
  }

  return result;
}

template<typename KeyT, typename ValueT>
List<KeyT> Hash<KeyT, ValueT>::keys(const ValueT& value) const
{
  List<KeyT> result;
  sysuint_t i, len = _d->capacity;
  for (i = 0; i < len; i++)
  {
    Node* node = (Node*)(_d->buckets[i]);
    while (node)
    {
      if (node->value == value)
        result.append(node->key);
      node = node->next;
    }
  }

  return result;
}

//! @}

} // Fog namespace

// ============================================================================
// [Fog::TypeInfo<>]
// ============================================================================

FOG_DECLARE_TYPEINFO_TEMPLATE2(Fog::Hash,
  typename, KeyT,
  typename, ValueT,
  Fog::TYPE_INFO_MOVABLE)

// [Guard]
#endif // _FOG_CORE_HASH_H
