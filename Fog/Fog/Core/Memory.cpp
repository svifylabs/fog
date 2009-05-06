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
#include <Fog/Core/AutoLock.h>
#include <Fog/Core/Constants.h>
#include <Fog/Core/Error.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/Lock.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/Std.h>

#include <stdio.h>
#include <stdlib.h>

// ===========================================================================
// [Fog::Memory]
// ===========================================================================

FOG_CAPI_DECLARE void* fog_memory_alloc(sysuint_t size)
{
  if (FOG_LIKELY(size))
  {
    void* addr = malloc(size);

    // If out of memory returns true, try malloc again
    if (!addr && fog_out_of_memory()) addr = malloc(size);

#if defined(FOG_MEMDBG_ENABLED)
    if (addr) fog_memdbg_add(addr, size);
#endif // FOG_MEMDBG_ENABLED

    return addr;
  }
  else
    return NULL;
}

FOG_CAPI_DECLARE void* fog_memory_calloc(sysuint_t size)
{
  if (FOG_LIKELY(size))
  {
    void* addr = calloc(1, size);

    // If out of memory returns true, try calloc again
    if (!addr && fog_out_of_memory()) addr = calloc(1, size);

#if defined(FOG_MEMDBG_ENABLED)
    if (addr) fog_memdbg_add(addr, size);
#endif // FOG_MEMDBG_ENABLED

    return addr;
  }
  else
    return NULL;
}

FOG_CAPI_DECLARE void* fog_memory_realloc(void* addr, sysuint_t size)
{
  if (FOG_LIKELY(addr != NULL))
  {
    if (FOG_LIKELY(size))
    {
      void* r_addr = realloc(addr, size);
      // If out of memory returns true, try realloc again
      if (!r_addr && fog_out_of_memory()) r_addr = realloc(addr, size);

#if defined(FOG_MEMDBG_ENABLED)
      if (r_addr)
      {
        fog_memdbg_remove(addr);
        fog_memdbg_add(r_addr, size);
      }
#endif // FOG_MEMDBG_ENABLED

      return r_addr;
    }
    else
    {
      free(addr);
      return NULL;
    }
  }
  else
    return fog_memory_alloc(size);
}

FOG_CAPI_DECLARE void* fog_memory_reallocf(void* addr, sysuint_t size)
{
  void* r_addr = fog_memory_realloc(addr, size);
  // If out of memory returns true, try realloc again
  if (!r_addr && fog_out_of_memory()) r_addr = realloc(addr, size);

  if (FOG_UNLIKELY(r_addr == NULL))
  {
    fog_memory_free(addr);
  }

  return r_addr;
}

FOG_CAPI_DECLARE void* fog_memory_xalloc(sysuint_t size)
{
  if (FOG_LIKELY(size))
  {
    void* addr = malloc(size);
    // If out of memory returns true, try malloc again
    if (!addr && fog_out_of_memory()) addr = malloc(size);

    if (FOG_LIKELY(addr != NULL))
    {
#if defined(FOG_MEMDBG_ENABLED)
      fog_memdbg_add(addr, size);
#endif // FOG_MEMDBG_ENABLED

      return addr;
    }
    else
    {
      fog_out_of_memory_fatal_format("Fog::Memory", "xalloc", "Couldn't allocate memory block at size %lu", size);
    }
  }
  else
    return NULL;
}

FOG_CAPI_DECLARE void* fog_memory_xcalloc(sysuint_t size)
{
  if (FOG_LIKELY(size))
  {
    void* addr = calloc(1, size);
    // If out of memory returns true, try malloc again
    if (!addr && fog_out_of_memory()) addr = calloc(1, size);

    if (FOG_LIKELY(addr != NULL))
    {
#if defined(FOG_MEMDBG_ENABLED)
      fog_memdbg_add(addr, size);
#endif // FOG_MEMDBG_ENABLED

      return addr;
    }
    else
    {
      fog_out_of_memory_fatal_format("Fog::Memory", "xcalloc", "Couldn't allocate memory block at size %lu", size);
    }
  }
  else
    return NULL;
}

FOG_CAPI_DECLARE void* fog_memory_xrealloc(void* addr, sysuint_t size)
{
  if (FOG_LIKELY(addr != NULL))
  {
#if defined(FOG_MEMDBG_ENABLED)
    fog_memdbg_remove(addr);
#endif // FOG_MEMDBG_ENABLED

    if (FOG_LIKELY(size))
    {
      void* r_addr = realloc(addr, size);
      // If out of memory returns true, try realloc again
      if (!r_addr && fog_out_of_memory()) r_addr = realloc(addr, size);

      if (FOG_LIKELY(r_addr != NULL))
      {
#if defined(FOG_MEMDBG_ENABLED)
        fog_memdbg_add(r_addr, size);
#endif // FOG_MEMDBG_ENABLED

        return r_addr;
      }
      else
      {
        fog_out_of_memory_fatal_format("Fog::Memory", "xrealloc", "Couldn't allocate memory block at size %lu", size);
      }
    }
    else
    {
      free(addr);
      return NULL;
    }
  }
  else
    return fog_memory_xalloc(size);
}

FOG_CAPI_DECLARE void fog_memory_free(void *addr)
{
  if (FOG_LIKELY(addr != NULL))
  {
#if defined(FOG_MEMDBG_ENABLED)
    fog_memdbg_remove(addr);
#endif // FOG_MEMDBG_ENABLED
    free(addr);
  }
}

FOG_CAPI_DECLARE void* fog_memory_dup(void *addr, sysuint_t size)
{
  if (FOG_LIKELY(addr != NULL) && FOG_LIKELY(size))
  {
    void* dupAddr = fog_memory_alloc(size);

    if (FOG_LIKELY(dupAddr != NULL))
    {
      fog_memory_copy(dupAddr, addr, size);
    }

    return dupAddr;
  }
  else
    return NULL;
}

FOG_CAPI_DECLARE void* fog_memory_xdup(void *addr, sysuint_t size)
{
  void *dupAddr = fog_memory_dup(addr, size);

  if (FOG_LIKELY(dupAddr != NULL))
  {
    return dupAddr;
  }
  else
  {
    fog_out_of_memory_fatal_format("Fog::Memory", "xdup", "Couldn't allocate memory block at size %lu", size);
  }
}

static void fog_memory_copy_C(void* dst, const void* src, sysuint_t size)
{
  memcpy(dst, src, size);
}

static void fog_memory_move_C(void* dst, const void* src, sysuint_t size)
{
  memmove(dst, src, size);
}

static void fog_memory_set_C(void* dst, uint c, sysuint_t size)
{
  memset(dst, (int)c, size);
}

static void fog_memory_zero_C(void* dst, sysuint_t size)
{
  memset(dst, 0, size);
}

FOG_CVAR_DECLARE void (*fog_memory_copy)(void*, const void*, sysuint_t);
FOG_CVAR_DECLARE void (*fog_memory_move)(void*, const void*, sysuint_t);
FOG_CVAR_DECLARE void (*fog_memory_set)(void*, uint, sysuint_t);
FOG_CVAR_DECLARE void (*fog_memory_zero)(void*, sysuint_t);

FOG_CVAR_DECLARE void (*fog_memory_copyNT)(void*, const void*, sysuint_t);
FOG_CVAR_DECLARE void (*fog_memory_moveNT)(void*, const void*, sysuint_t);
FOG_CVAR_DECLARE void (*fog_memory_setNT)(void*, uint, sysuint_t);
FOG_CVAR_DECLARE void (*fog_memory_zeroNT)(void*, sysuint_t);

// [Fog::Memory::xchg]

FOG_CAPI_DECLARE void fog_memory_xchg(uint8_t* addr1, uint8_t* addr2, sysuint_t count)
{
  sysuint_t i;

#if FOG_ARCH_BITS == 32
  for (i = count >> 2; i; i--, addr1 += 4, addr2 += 4) Fog::Memory::xchg4B((void*)addr1, (void*)addr2);
  for (i = count & 3; i; i--, addr1 += 1, addr2 += 1) Fog::Memory::xchg1B((void*)addr1, (void*)addr2);
#else
  for (i = count >> 3; i; i--, addr1 += 8, addr2 += 8) Fog::Memory::xchg8B((void*)addr1, (void*)addr2);
  for (i = count & 7; i; i--, addr1 += 1, addr2 += 1) Fog::Memory::xchg1B((void*)addr1, (void*)addr2);
#endif // FOG_ARCH_BITS
}

// ===========================================================================
// [Fog::MemDbg]
// ===========================================================================

#if defined(FOG_MEMDBG_ENABLED)
struct MemDbgNode
{
  void* addr;
  sysuint_t size;
  MemDbgNode* next;
};

static Fog::Static<Fog::Lock> fog_memdbg_lock;

static uint fog_memdbg_state;
static sysuint_t fog_memdbg_blocks;
static uint64_t fog_memdbg_heapallocsize;

// This is small non-growing hash table, it's more better than single linked lists
static MemDbgNode** fog_memdbg_nodes;

static MemDbgNode* fog_memdbg_find(void* addr);
static void fog_memdbg_dump(void* addr, sysuint_t size);

#define fog_memdbg_TableSize 47431

static FOG_INLINE uint32_t fog_memdbg_hash(void* addr)
{
  // Pointers are usually aligned to 4 or 8 bytes (depending to arch)
#if FOG_ARCH_BITS == 32
  return ((uint32_t)addr >> 2) % fog_memdbg_TableSize;
#else
  return (((uint32_t)(sysuint_t)addr ^ (uint32_t)((sysuint_t)addr >> 32)) >> 3) % fog_memdbg_TableSize;
#endif
}

FOG_CAPI_DECLARE void fog_memdbg_init(void)
{
  fog_memdbg_lock.init();

  fog_memdbg_state = 1;
  fog_memdbg_blocks = 0;
  fog_memdbg_heapallocsize = 0ULL;
  fog_memdbg_nodes = (MemDbgNode**)calloc(1, sizeof(void*) * fog_memdbg_TableSize);
  if (fog_memdbg_nodes == NULL)
  {
    fog_out_of_memory_fatal_format("Fog::MemDbg", "init", "Couldn't allocate memory for memory debugger");
  }
}

FOG_CAPI_DECLARE void fog_memdbg_shutdown(void)
{
  // only show debug information if application wasn't fail.
  if (fog_failed == 0) fog_memdbg_leaks();

  free((void*)fog_memdbg_nodes);
  fog_memdbg_state = 0;
  fog_memdbg_heapallocsize = 0ULL;
  fog_memdbg_lock.destroy();
}

FOG_CAPI_DECLARE void fog_memdbg_add(void* addr, sysuint_t size)
{
  MemDbgNode* node = (MemDbgNode*)malloc(sizeof(MemDbgNode));

  if (FOG_LIKELY(node != NULL))
  {
    fog_memdbg_lock.instance().lock();

    node->addr = addr;
    node->size = size;

    node->next = fog_memdbg_nodes[fog_memdbg_hash(addr)];
    fog_memdbg_nodes[fog_memdbg_hash(addr)] = node;
    fog_memdbg_blocks++;
    fog_memdbg_heapallocsize += size;

    fog_memdbg_lock.instance().unlock();
  }
  else
  {
    fog_out_of_memory_fatal_format("Fog::MemDbg", "add", "Couldn't allocate memory for debugger hash node");
  }
}

FOG_CAPI_DECLARE void fog_memdbg_remove(void* addr)
{
  fog_memdbg_lock.instance().lock();

  MemDbgNode* node = fog_memdbg_nodes[fog_memdbg_hash(addr)];
  MemDbgNode* prev = 0;

  while (node)
  {
    if (FOG_UNLIKELY(node->addr == addr))
    {
      if (prev)
        prev->next = node->next;
      else
        fog_memdbg_nodes[fog_memdbg_hash(addr)] = node->next;

      fog_memdbg_blocks--;
      fog_memdbg_heapallocsize -= node->size;

      free(node);
      fog_memdbg_lock.instance().unlock();
      return;
    }

    prev = node;
    node = node->next;
  }

  fog_memdbg_lock.instance().unlock();

  fog_stderr_msg("Fog::MemDbg", "remove", "Memory address not found");

  // Cause segfault rather than exit
  *(uint*)NULL = 0;

  exit(1);
}

FOG_CAPI_DECLARE void fog_memdbg_leaks(void)
{
  fog_memdbg_lock.instance().lock();

  if (fog_memdbg_blocks > 0)
  {
    fog_stderr_msg("Fog::MemDbg", "leaks", "Detected %lu memory leak(s), size:%llu bytes.", fog_memdbg_blocks, fog_memdbg_heapallocsize);

    sysuint_t i;
    MemDbgNode* node;

    for (i = 0; i != fog_memdbg_TableSize; i++)
    {
      for (node = fog_memdbg_nodes[i]; node; node = node->next)
      {
        bool doDump = true;

        fprintf(fog_stderr, "At %p of %lu bytes", node->addr, (sysuint_t)node->size);

        // Fog uses implicit sharing that usually only needs to alloc
        // memory of pointer size in case that implicit shared object
        // was created by new operator (this is normal situation, because
        // objects allocated on the stack will not be here)
        if (node->size == sizeof(void*))
        {
          void *link = *(void **)node->addr;

          // Find if its a pointer.
          if (fog_memdbg_find(link))
          {
            fprintf(fog_stderr, " -> %p (pointer found)", link );
            doDump = false;
          }
          else
          {
            fprintf(fog_stderr, " -> %p (pointer not found)", link );
          }
        }

        fprintf(fog_stderr, "\n");
        if (doDump) fog_memdbg_dump(node->addr, (node->size < 512) ? node->size : 512);
      }
    }
    fflush(fog_stderr);
  }

  fog_memdbg_lock.instance().unlock();
}

static MemDbgNode* fog_memdbg_find(void* addr)
{
  /* don't lock mutex, this method is only called from MemoryDebugger::leaks() */
  MemDbgNode* node = fog_memdbg_nodes[fog_memdbg_hash(addr)];
  while (node)
  {
    if (node->addr == addr) return node;
    node = node->next;
  }

  return NULL;
}

static void fog_memdbg_dump(void* addr, sysuint_t size)
{
  const uchar* addr_c = (const uchar*)addr;
  sysuint_t a = 0, i;

  while (a < size)
  {
    sysuint_t width = 24;
    if (a + width > size) width = size - a;

    for (i = 0; i != width; i++)
    {
      int c = addr_c[i];
      fprintf(fog_stderr, "%02X", c);
    }
    while (i++ < 24) fprintf(fog_stderr, "  ");

    for (i = 0; i != width; i++) {
      int c = addr_c[i];
      if (c >= ' ' && c < 128)
        fprintf(fog_stderr, "%c", c);
      else
        fprintf(fog_stderr, ".");
    }
    fprintf(fog_stderr, "\n");

    a += width;
    addr_c += width;
  }
}
#endif // FOG_MEMDBG_ENABLED

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_memory_init(void)
{
  fog_memory_copy   = fog_memory_copy_C;
  fog_memory_move   = fog_memory_move_C;
  fog_memory_set    = fog_memory_set_C;
  fog_memory_zero   = fog_memory_zero_C;

  fog_memory_copyNT = fog_memory_copy_C;
  fog_memory_moveNT = fog_memory_move_C;
  fog_memory_setNT  = fog_memory_set_C;
  fog_memory_zeroNT = fog_memory_zero_C;

#if defined(FOG_MEMDBG_ENABLED)
  fog_memdbg_state = 0;
  fog_memdbg_init();
#endif // FOG_MEMDBG_ENABLED

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_memory_shutdown(void)
{
#if defined(FOG_MEMDBG_ENABLED)
  fog_memdbg_shutdown();
#endif // FOG_MEMDBG_ENABLED
}
