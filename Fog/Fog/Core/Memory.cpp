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

// [on bsd systems we can't include <malloc.h>]
#if defined(FOG_HAVE_MALLOC_H)
#include <malloc.h>
#endif

// [Fog::Memory]

FOG_CAPI_DECLARE void* Core_Memory_alloc(sysuint_t size)
{
  if (FOG_LIKELY(size))
  {
    void* addr = malloc(size);

    // If out of memory returns true, try malloc again
    if (!addr && fog_out_of_memory()) addr = malloc(size);

    #if defined(WDE_DEBUG_MEMORY)
    if (addr) Core_MemoryDebugger_add(addr, size);
    #endif

    return addr;
  }
  else
    return NULL;
}

FOG_CAPI_DECLARE void* Core_Memory_calloc(sysuint_t size)
{
  if (FOG_LIKELY(size))
  {
    void* addr = calloc(1, size);

    // If out of memory returns true, try calloc again
    if (!addr && fog_out_of_memory()) addr = calloc(1, size);

    #if defined(WDE_DEBUG_MEMORY)
    if (addr) Core_MemoryDebugger_add(addr, size);
    #endif

    return addr;
  }
  else
    return NULL;
}

FOG_CAPI_DECLARE void* Core_Memory_realloc(void* addr, sysuint_t size)
{
  if (FOG_LIKELY(addr != NULL))
  {
    if (FOG_LIKELY(size))
    {
      void* r_addr = realloc(addr, size);
      // If out of memory returns true, try realloc again
      if (!r_addr && fog_out_of_memory()) r_addr = realloc(addr, size);

      #if defined(WDE_DEBUG_MEMORY)
      if (r_addr)
      {
        Core_MemoryDebugger_remove(addr);
        Core_MemoryDebugger_add(r_addr, size);
      }
      #endif

      return r_addr;
    }
    else
    {
      free(addr);
      return NULL;
    }
  }
  else
    return Core_Memory_alloc(size);
}

FOG_CAPI_DECLARE void* Core_Memory_reallocf(void* addr, sysuint_t size)
{
  void* r_addr = Core_Memory_realloc(addr, size);
  // If out of memory returns true, try realloc again
  if (!r_addr && fog_out_of_memory()) r_addr = realloc(addr, size);

  if (FOG_UNLIKELY(r_addr == NULL))
  {
    Core_Memory_free(addr);
  }

  return r_addr;
}

FOG_CAPI_DECLARE void* Core_Memory_xalloc(sysuint_t size)
{
  if (FOG_LIKELY(size))
  {
    void* addr = malloc(size);
    // If out of memory returns true, try malloc again
    if (!addr && fog_out_of_memory()) addr = malloc(size);

    if (FOG_LIKELY(addr != NULL))
    {
#if defined(WDE_DEBUG_MEMORY)
      Core_MemoryDebugger_add(addr, size);
#endif

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

FOG_CAPI_DECLARE void* Core_Memory_xcalloc(sysuint_t size)
{
  if (FOG_LIKELY(size))
  {
    void* addr = calloc(1, size);
    // If out of memory returns true, try malloc again
    if (!addr && fog_out_of_memory()) addr = calloc(1, size);

    if (FOG_LIKELY(addr != NULL))
    {
      #if defined(WDE_DEBUG_MEMORY)
      Core_MemoryDebugger_add(addr, size);
      #endif

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

FOG_CAPI_DECLARE void* Core_Memory_xrealloc(void* addr, sysuint_t size)
{
  if (FOG_LIKELY(addr != NULL))
  {
    #if defined(WDE_DEBUG_MEMORY)
    Core_MemoryDebugger_remove(addr);
    #endif

    if (FOG_LIKELY(size))
    {
      void* r_addr = realloc(addr, size);
      // If out of memory returns true, try realloc again
      if (!r_addr && fog_out_of_memory()) r_addr = realloc(addr, size);

      if (FOG_LIKELY(r_addr != NULL))
      {
        #if defined(WDE_DEBUG_MEMORY)
        Core_MemoryDebugger_add(r_addr, size);
        #endif

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
    return Core_Memory_xalloc(size);
}

FOG_CAPI_DECLARE void Core_Memory_free(void *addr)
{
  if (FOG_LIKELY(addr != NULL))
  {
    #if defined(WDE_DEBUG_MEMORY)
    Core_MemoryDebugger_remove(addr);
    #endif
    free(addr);
  }
}

FOG_CAPI_DECLARE void* Core_Memory_dup(void *addr, sysuint_t size)
{
  if (FOG_LIKELY(addr != NULL) && FOG_LIKELY(size))
  {
    void* dupAddr = Core_Memory_alloc(size);

    if (FOG_LIKELY(dupAddr != NULL))
    {
      Core_Memory_copy(dupAddr, addr, size);
    }

    return dupAddr;
  }
  else
    return NULL;
}

FOG_CAPI_DECLARE void* Core_Memory_xdup(void *addr, sysuint_t size)
{
  void *dupAddr = Core_Memory_dup(addr, size);

  if (FOG_LIKELY(dupAddr != NULL))
  {
    return dupAddr;
  }
  else
  {
    fog_out_of_memory_fatal_format("Fog::Memory", "xdup", "Couldn't allocate memory block at size %lu", size);
  }
}

static void Core_Memory_copy_C(void* dest, const void* src, sysuint_t size)
{
  register ulong i;
  register uchar* destCur = (uchar*)dest;
  register const uchar* srcCur = (const uchar*)src;

  if (size > 16)
  {
    // align to native CPU word size boundary
    while (((ulong)destCur & (sizeof(ulong)-1)) != 0 && size)
    {
      ((uchar *)destCur)[0] = ((uchar *)srcCur)[0];

      destCur++;
      srcCur++;
      size--;
    }

    // 4 or 8 bytes at once
    for (i = size / sizeof(ulong); i; i--, destCur += sizeof(ulong), srcCur += sizeof(ulong))
    {
      ((ulong *)destCur)[0] = ((ulong *)srcCur)[0];
    }

    // tail
    for (i = (size & (sizeof(ulong)-1)); i; i--, destCur++, srcCur++)
    {
      ((uchar *)destCur)[0] = ((uchar *)srcCur)[0];
    }
  }
  else
  {
    // we not need to do something special, it's too small block
    for (i = size; i; i--, destCur++, srcCur++)
    {
      ((uchar *)destCur)[0] = ((uchar *)srcCur)[0];
    }
  }
}

static void Core_Memory_move_C(void* dest, const void* src, sysuint_t size)
{
  register ulong i;
  register uchar* destCur = (uchar*)dest;
  register const uchar* srcCur = (const uchar*)src;

  if (destCur < srcCur)
  {
    if (size > 16)
    {
      // align to native CPU word size boundary
      while (((ulong)destCur & (sizeof(ulong)-1)) != 0 && size)
      {
        ((uchar *)destCur)[0] = ((uchar *)srcCur)[0];

        destCur++;
        size--;
      }

      // 4 or 8 bytes at once
      for (i = size / sizeof(ulong); i; i--, destCur += sizeof(ulong), srcCur += sizeof(ulong))
      {
        ((ulong *)destCur)[0] = ((ulong *)srcCur)[0];
      }

      // tail
      for (i = (size & (sizeof(ulong)-1)); i; i--, destCur++, srcCur++)
      {
        ((uchar *)destCur)[0] = ((uchar *)srcCur)[0];
      }
    }
    else
    {
      // we not need to do something special, it's too small block
      for (i = size; i; i--, destCur++, srcCur++)
      {
        ((uchar *)destCur)[0] = ((uchar *)srcCur)[0];
      }
    }
  }
  else
  {
    destCur += size - 1;
    srcCur += size - 1;

    for (i = size; i; i--, destCur--, srcCur--)
    {
      ((uchar *)destCur)[0] = ((uchar *)srcCur)[0];
    }
  }
}

static void Core_Memory_set_C(void* dest, uint c, sysuint_t size)
{
  register ulong i;
  register uchar* destCur = (uchar*)dest;

  if (size > 16)
  {
    ulong pattern;

    pattern = (ulong)c & 0xFFU;
    pattern |= pattern << 8U;
    pattern |= pattern << 16U;

    #if FOG_ARCH_BITS == 64
    pattern |= pattern << 32U;
    #endif

    // align to native CPU word size boundary
    while (((ulong)destCur & (sizeof(ulong)-1)) != 0 && size)
    {
      ((uchar *)destCur)[0] = (uchar)c;

      destCur++;
      size--;
    }

    // 4 or 8 bytes at once
    for (i = size / sizeof(ulong); i; i--, destCur += sizeof(ulong))
    {
      ((ulong *)destCur)[0] = pattern;
    }

    // tail
    for (i = size & (sizeof(ulong)-1); i; i--, destCur++)
    {
      ((uchar *)destCur)[0] = c;
    }
  }
  else
  {
    for (i = size; i; i--, destCur++)
    {
      ((uchar *)destCur)[0] = (uchar)c;
    }
  }
}

static void Core_Memory_zero_C(void* dest, sysuint_t size)
{
  register ulong i;
  register uchar* destCur = (uchar*)dest;

  if (size > 16)
  {
    // align to native CPU word size boundary
    while (((ulong)destCur & (sizeof(ulong)-1)) != 0 && size)
    {
      ((uchar *)destCur)[0] = 0U;

      destCur++;
      size--;
    }

    // 4 or 8 bytes at once
    for (i = size / sizeof(ulong); i; i--, destCur += sizeof(ulong))
    {
      ((ulong *)destCur)[0] = 0UL;
    }

    // tail
    for (i = size & (sizeof(ulong)-1); i; i--, destCur++)
    {
      ((uchar *)destCur)[0] = 0U;
    }
  }
  else
  {
    for (i = size; i; i--, destCur++)
    {
      ((uchar *)destCur)[0] = 0U;
    }
  }
}

FOG_CVAR_DECLARE void (*Core_Memory_copy)(void*, const void*, sysuint_t);
FOG_CVAR_DECLARE void (*Core_Memory_move)(void*, const void*, sysuint_t);
FOG_CVAR_DECLARE void (*Core_Memory_set)(void*, uint, sysuint_t);
FOG_CVAR_DECLARE void (*Core_Memory_zero)(void*, sysuint_t);

FOG_CVAR_DECLARE void (*Core_Memory_copyNT)(void*, const void*, sysuint_t);
FOG_CVAR_DECLARE void (*Core_Memory_moveNT)(void*, const void*, sysuint_t);
FOG_CVAR_DECLARE void (*Core_Memory_setNT)(void*, uint, sysuint_t);
FOG_CVAR_DECLARE void (*Core_Memory_zeroNT)(void*, sysuint_t);

// [Fog::Memory::xchg]

FOG_CAPI_DECLARE void Core_Memory_xchg(uint8_t* addr1, uint8_t* addr2, sysuint_t count)
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

// [Fog::MemoryDebugger]

#if defined(WDE_DEBUG_MEMORY)
struct Core_MemoryDebuggerNode
{
  void* addr;
  sysuint_t size;
  Core_MemoryDebuggerNode* next;
};

static Fog::Static<Fog::Lock> Core_MemoryDebugger_csec;

static uint Core_MemoryDebugger_state;
static sysuint_t Core_MemoryDebugger_blocks;
static uint64_t Core_MemoryDebugger_heapallocsize;

// This is small non-growing hash table, it's more better than single linked lists
static Core_MemoryDebuggerNode** Core_MemoryDebugger_nodes;

static Core_MemoryDebuggerNode* Core_MemoryDebugger_find(void* addr);
static void Core_MemoryDebugger_dump(void* addr, ulong size);

#define Core_MemoryDebugger_TableSize 47431

static FOG_INLINE uint32_t Core_MemoryDebugger_hash(void* addr)
{
  // Pointers are usually aligned to 4 or 8 bytes (depending to arch)
#if FOG_ARCH_BITS == 32
  return ((uint32_t)addr >> 2) % Core_MemoryDebugger_TableSize;
#else
  return (((uint32_t)(ulong)addr ^ (uint32_t)((ulong)addr >> 32)) >> 3) % Core_MemoryDebugger_TableSize;
#endif
}
#endif

FOG_CAPI_DECLARE void Core_MemoryDebugger_init(void)
{
#if defined(WDE_DEBUG_MEMORY)
  Core_MemoryDebugger_csec.init();

  Core_MemoryDebugger_state = 1;
  Core_MemoryDebugger_blocks = 0;
  Core_MemoryDebugger_heapallocsize = 0ULL;
  Core_MemoryDebugger_nodes = (Core_MemoryDebuggerNode**)calloc(1, sizeof(void*) * Core_MemoryDebugger_TableSize);
  if (Core_MemoryDebugger_nodes == NULL)
  {
    fog_out_of_memory_fatal_format("Fog::MemoryDebugger", "init", "Couldn't allocate memory for memory debugger");
  }
#endif
}

FOG_CAPI_DECLARE void Core_MemoryDebugger_shutdown(void)
{
#if defined(WDE_DEBUG_MEMORY)
  // only show debug information if application wasn't fail.
  if (fog_failed == 0) Core_MemoryDebugger_leaks();

  free((void*)Core_MemoryDebugger_nodes);
  Core_MemoryDebugger_state = 0;
  Core_MemoryDebugger_heapallocsize = 0ULL;
  Core_MemoryDebugger_csec.destroy();
#endif
}

FOG_CAPI_DECLARE void Core_MemoryDebugger_add(void* addr, sysuint_t size)
{
#if defined(WDE_DEBUG_MEMORY)
  Core_MemoryDebuggerNode* node = (Core_MemoryDebuggerNode*)malloc(sizeof(Core_MemoryDebuggerNode));

  if (FOG_LIKELY(node != NULL))
  {
    Core_MemoryDebugger_csec.instance().lock();

    node->addr = addr;
    node->size = size;

    node->next = Core_MemoryDebugger_nodes[Core_MemoryDebugger_hash(addr)];
    Core_MemoryDebugger_nodes[Core_MemoryDebugger_hash(addr)] = node;
    Core_MemoryDebugger_blocks++;
    Core_MemoryDebugger_heapallocsize += size;

    Core_MemoryDebugger_csec.instance().unlock();
  }
  else
  {
    fog_out_of_memory_fatal_format("Fog::MemoryDebugger", "add", "Couldn't allocate memory for debugger hash node");
  }
#else
  FOG_UNUSED(addr);
  FOG_UNUSED(size);
#endif
}

FOG_CAPI_DECLARE void Core_MemoryDebugger_remove(void* addr)
{
#if defined(WDE_DEBUG_MEMORY)
  Core_MemoryDebugger_csec.instance().lock();

  Core_MemoryDebuggerNode* node = Core_MemoryDebugger_nodes[Core_MemoryDebugger_hash(addr)];
  Core_MemoryDebuggerNode* prev = 0;

  while (node)
  {
    if (FOG_UNLIKELY(node->addr == addr))
    {
      if (prev)
        prev->next = node->next;
      else
        Core_MemoryDebugger_nodes[Core_MemoryDebugger_hash(addr)] = node->next;

      Core_MemoryDebugger_blocks--;
      Core_MemoryDebugger_heapallocsize -= node->size;

      free(node);
      Core_MemoryDebugger_csec.instance().unlock();
      return;
    }

    prev = node;
    node = node->next;
  }

  Core_MemoryDebugger_csec.instance().unlock();

  fog_stderr_msg("Fog::MemoryDebugger", "remove", "Memory address not found");

  // Cause segfault rather than exit
  *(uint*)NULL = 0;

  exit(Fog::ExitFailure);
#else
  FOG_UNUSED(addr);
#endif
}

FOG_CAPI_DECLARE void Core_MemoryDebugger_leaks(void)
{
#if defined(WDE_DEBUG_MEMORY)
  Core_MemoryDebugger_csec.instance().lock();

  if (Core_MemoryDebugger_blocks > 0)
  {
    fog_stderr_msg("Fog::MemoryDebugger", "leaks", "Detected %lu memory leak(s), size:%llu bytes.", Core_MemoryDebugger_blocks, Core_MemoryDebugger_heapallocsize);

    sysuint_t i;
    Core_MemoryDebuggerNode* node;

    for (i = 0; i != Core_MemoryDebugger_TableSize; i++)
    {
      for (node = Core_MemoryDebugger_nodes[i]; node; node = node->next)
      {
        bool doDump = true;

        fprintf(fog_stderr, "At %p of %lu bytes", node->addr, (ulong)node->size);

        // WDE uses implicit sharing that usually only needs to alloc
        // memory of pointer size in case that implicit shared object
        // was created by new operator (this is normal situation, because
        // objects allocated on the stack will not be here)
        if (node->size == sizeof(void*))
        {
          void *link = *(void **)node->addr;

          // Find if its a pointer.
          if (Core_MemoryDebugger_find(link))
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
        if (doDump) Core_MemoryDebugger_dump(node->addr, (node->size < 512) ? node->size : 512);
      }
    }
    fflush(fog_stderr);
  }

  Core_MemoryDebugger_csec.instance().unlock();
#endif
}

#if defined(WDE_DEBUG_MEMORY)
static Core_MemoryDebuggerNode* Core_MemoryDebugger_find(void* addr)
{
  /* don't lock mutex, this method is only called from MemoryDebugger::leaks() */
  Core_MemoryDebuggerNode* node = Core_MemoryDebugger_nodes[Core_MemoryDebugger_hash(addr)];
  while (node)
  {
    if (node->addr == addr) return node;
    node = node->next;
  }

  return NULL;
}

static void Core_MemoryDebugger_dump(void* addr, ulong size)
{
  const uchar* addr_c = (const uchar*)addr;
  ulong a = 0, i;

  while (a < size)
  {
    ulong width = 24;
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
#endif

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_memory_init(void)
{
  Core_Memory_copy   = Core_Memory_copy_C;
  Core_Memory_move   = Core_Memory_move_C;
  Core_Memory_set    = Core_Memory_set_C;
  Core_Memory_zero   = Core_Memory_zero_C;

  Core_Memory_copyNT = Core_Memory_copy_C;
  Core_Memory_moveNT = Core_Memory_move_C;
  Core_Memory_setNT  = Core_Memory_set_C;
  Core_Memory_zeroNT = Core_Memory_zero_C;

#if defined(WDE_DEBUG_MEMORY)
  Core_MemoryDebugger_state = 0;
  Core_MemoryDebugger_init();
#endif

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_memory_shutdown(void)
{
#if defined(WDE_DEBUG_MEMORY)
  Core_MemoryDebugger_shutdown();
#endif
}
