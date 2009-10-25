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
#include <Fog/Core/Atomic.h>
#include <Fog/Core/Byte.h>
#include <Fog/Core/ByteArrayMatcher.h>
#include <Fog/Core/StringUtil.h>
#include <Fog/Core/Thread.h>

namespace Fog {

// ============================================================================
// [Fog::ByteArrayMatcher]
// ============================================================================

static void buildTable(ByteArrayMatcher::SkipTable* skipTable, const char* ps, sysuint_t plen, uint cs)
{
  FOG_ASSERT(plen > 1);
  FOG_ASSERT(plen < UINT_MAX);

  if (skipTable->status.cmpXchg(ByteArrayMatcher::SkipTable::Uninitialized, ByteArrayMatcher::SkipTable::Initializing))
  {
    // Init skip table.
    sysuint_t a = 32; // 256 / 8
    sysuint_t i = plen;
    uint* data = skipTable->data;

    while (a--)
    {
      data[0] = i; data[1] = i; data[2] = i; data[3] = i;
      data[4] = i; data[5] = i; data[6] = i; data[7] = i;
      data += 8;
    }

    data = skipTable->data;

    if (cs == CaseSensitive)
    {
      while (i--)
      {
        data[(uint8_t)*ps] = (uint)i; ps++;
      }
    }
    else
    {
      while (i--)
      {
        data[(uint8_t)Byte::toLower(*ps)] = (uint)i; ps++;
      }
    }
    skipTable->status.set(ByteArrayMatcher::SkipTable::Initialized);
    return;
  }

  // Wait...another thread creating the table...
  while (skipTable->status.get() != ByteArrayMatcher::SkipTable::Initialized) Thread::_yield();
}

ByteArrayMatcher::ByteArrayMatcher()
{
  _skipTable[CaseInsensitive].status.init(SkipTable::Uninitialized);
  _skipTable[CaseSensitive  ].status.init(SkipTable::Uninitialized);
}

ByteArrayMatcher::ByteArrayMatcher(const ByteArray& pattern)
{
  setPattern(pattern);
}

ByteArrayMatcher::ByteArrayMatcher(const ByteArrayMatcher& matcher)
{
  setPattern(matcher);
}

ByteArrayMatcher::~ByteArrayMatcher()
{
}

err_t ByteArrayMatcher::setPattern(const ByteArray& pattern)
{
  err_t err;
  if ( (err = _pattern.set(pattern)) ) return err;

  // Mark tables as uninitialized.
  _skipTable[CaseInsensitive].status.set(SkipTable::Uninitialized);
  _skipTable[CaseSensitive  ].status.set(SkipTable::Uninitialized);

  return Error::Ok;
}

err_t ByteArrayMatcher::setPattern(const ByteArrayMatcher& matcher)
{
  SkipTable* dstSkipTable = _skipTable;
  const SkipTable* srcSkipTable = matcher._skipTable;

  err_t err;
  if ( (err = _pattern.set(matcher._pattern)) ) return err;

  // Copy skip tables if they are initialized.
  for (sysuint_t i = 0; i != 2; i++)
  {
    if (srcSkipTable[i].status.get() == SkipTable::Initialized)
    {
      dstSkipTable[i].status.set(SkipTable::Initialized);
      memcpy(&dstSkipTable[i].data, &srcSkipTable[i].data, sizeof(uint) * 256);
    }
    else
    {
      dstSkipTable[i].status.set(SkipTable::Uninitialized);
    }
  }

  return Error::Ok;
}

Range ByteArrayMatcher::match(const char* str, sysuint_t length, uint cs, const Range& range) const
{
  FOG_ASSERT(length != DetectLength);
  FOG_ASSERT(range.index <= length);
  FOG_ASSERT(length - range.index >= range.length);

  sysuint_t patternLength = _pattern.getLength();

  // simple reject.
  if (patternLength == 0 || patternLength > length) return InvalidIndex;

  // We want 0 or 1.
  cs = !!cs;

  const uint8_t* strCur = reinterpret_cast<const uint8_t*>(str) + range.index;
  const uint8_t* patternStr = reinterpret_cast<const uint8_t*>(_pattern.cData());

  // Simple 'Char' search.
  if (patternLength == 1)
  {
    sysuint_t i = StringUtil::indexOf(reinterpret_cast<const char*>(strCur), range.length, (char)patternStr[0], cs);
    if (i != InvalidIndex) i += range.index;
    return i;
  }

  if (_skipTable[cs].status.get() != ByteArrayMatcher::SkipTable::Initialized)
  {
    buildTable(const_cast<SkipTable*>(&_skipTable[cs]), reinterpret_cast<const char*>(patternStr), patternLength, cs);
  }

  const uint* skipTable = _skipTable[cs].data;

  sysuint_t skip;
  sysuint_t remain = range.length;

  patternStr += patternLength - 1;
  strCur     += patternLength - 1;
  remain     -= patternLength - 1;

  if (cs == CaseSensitive)
  {
    for (;;)
    {
      // Get count of characters to skip from skip table.
      if ((skip = skipTable[*strCur]) == 0)
      {
        // Check if there is possible match.
        while (skip < patternLength)
        {
          if (strCur[-(sysint_t)skip] != patternStr[-(sysint_t)skip]) break;
          skip++;
        }

        // Match.
        if (skip >= patternLength)
          return Range((sysuint_t)(strCur - reinterpret_cast<const uint8_t*>(str)) - skip + 1, patternLength);

        if (skipTable[strCur[-(sysint_t)skip]] == patternLength)
          skip = patternLength - skip;
        else
          skip = 1;
      }

      if (remain < skip) break;
      strCur += skip;
      remain -= skip;
    }
  }
  else
  {
    for (;;)
    {
      // Get count of characters to skip from skip table.
      if ((skip = skipTable[Byte::toLower(*strCur)]) == 0)
      {
        // Check if there is possible match.
        while (skip < patternLength)
        {
          if (Byte::toLower(strCur[-(sysint_t)skip]) != Byte::toLower(patternStr[-(sysint_t)skip])) break;
          skip++;
        }

        // Match.
        if (skip >= patternLength)
          return Range((sysuint_t)(strCur - reinterpret_cast<const uint8_t*>(str)) - skip + 1, patternLength);

        if (skipTable[Byte::toLower(strCur[-(sysint_t)skip])] == patternLength)
          skip = patternLength - skip;
        else
          skip = 1;
      }

      if (remain < skip) break;
      strCur += skip;
      remain -= skip;
    }
  }

  return Range(InvalidIndex);
}

sysuint_t ByteArrayMatcher::getLength() const
{
  return _pattern.getLength();
}

} // Fog namespace
