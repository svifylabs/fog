// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_MISC_H
#define _FOG_CORE_MISC_H

// [Dependencies]
#include <Fog/Core/Assert.h>
#include <Fog/Build/Build.h>
#include <Fog/Core/Memory.h>

namespace Fog {

/*!
  @brief This template is for fast routines that needs to alloc memory on
  the stack. Maximum size of memory allocated on the stack is @a N.

  If N is zero, it will always use heap allocation.

  This class is used internaly in Core and all upper libraries to simplify 
  some code, because there are situations where we need to alloc memory by 
  alloca(), but if the amount of requested memory is too big, it's better 
  to use memory on heap.

  @c alloc() member can be called only once. Memory allocated on the heap is
  freed by destructor or explicit @c free() call.

  If you use @c free() to free allocated bytes, you can use @c alloc() again.

  This template simulates the @c alloca() behavior.
*/
template<sysuint_t N = 0>
struct MemoryBuffer
{
private:
  void *_mem;
  uint8_t _storage[N];

public:
  FOG_INLINE MemoryBuffer() : _mem(0)
  {
  }

  FOG_INLINE ~MemoryBuffer()
  {
    _free();
  }

  FOG_INLINE void* alloc(sysuint_t size)
  {
    FOG_ASSERT(_mem == NULL);

    if (N == 0)
      return (_mem = Memory::alloc(size));
    else if (size > N)
      return (_mem = Memory::alloc(size));
    else
      return (_mem = (void*)_storage);
  }

  FOG_INLINE void free()
  {
    _free();
    _mem = NULL;
  }

  FOG_INLINE void* mem() const
  {
    return _mem;
  }

private:
  FOG_INLINE void _free()
  {
    if (N == 0)
    {
      if (_mem != NULL) Memory::free(_mem);
    }
    else
    {
      if (_mem != NULL && _mem != (void*)_storage) Memory::free(_mem);
    }
  }

  FOG_DISABLE_COPY(MemoryBuffer)
};

/*!
  @brief Fast and secure stack implementation for critical routines.

  This stack class allocates some memory on the <b>stack</b> and if
  it's needed it allocs blocks of memory on the heap.
*/
template<ulong N>
struct LocalStack
{
  struct Node
  {
    /*! @brief Pointer to current position in buffer for this node. */
    uint8_t* cur;
    /*! @brief Pointer to next node. */
    Node* next;
    /*! @brief Pointer to previous node. */
    Node* prev;
    /*! @brief Remaining bytes in bufer. */
    sysuint_t remain;
    /*! @brief Node data. */
    uint8_t buffer[N];
  };

  Node* _current;
  Node _first;

  FOG_INLINE LocalStack()
  {
    _current = &_first;
    _first.prev = NULL;
    _first.next = NULL;
    _first.cur = _first.buffer;
    _first.remain = N;
  }

  FOG_INLINE ~LocalStack()
  {
    Node* node = _first.next;
    while (node)
    {
      Node* next = node->next;
      Fog::Memory::free(node);
      node = next;
    }
  }

  template<class T>
  FOG_INLINE LocalStack& push(T& data)
  {
    if (FOG_UNLIKELY(_current->remain < sizeof(T)))
    {
      if (_current->next == NULL)
      {
        Node* node = (Node*)Fog::Memory::xalloc(sizeof(Node) - N + (1024*32));
        node->cur = node->buffer;
        node->prev = _current;
        node->next = NULL;
        node->remain = 1024*32;
        _current->next = node;
        _current = node;
      }
      else
      {
        _current = _current->next;
      }
    }

    *(T *)(_current->cur) = data;
    _current->cur += sizeof(T);
    _current->remain -= sizeof(T);

    return *this;
  }

  template<class T>
  FOG_INLINE LocalStack& pop(T& data)
  {
    if (FOG_UNLIKELY(_current->cur == _current->buffer))
    {
      _current = _current->prev;
    }

    _current->cur -= sizeof(T);
    _current->remain += sizeof(T);
    data = *(T *)(_current->cur);

    return *this;
  }

  FOG_INLINE bool isEmpty() const
  {
    return (_current == &_first && _first.cur == _first.buffer);
  }

private:
  FOG_DISABLE_COPY(LocalStack)
};

} // Fog namespace

// [Guard]
#endif // _FOG_CORE_MISC_H
