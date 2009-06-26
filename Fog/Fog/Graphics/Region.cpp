// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled headers]
#ifdef FOG_PRECOMP
#include FOG_PRECOMP
#endif

// [Dependencies]
#include <Fog/Core/Assert.h>
#include <Fog/Core/Constants.h>
#include <Fog/Core/Error.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/Misc.h>
#include <Fog/Core/Std.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Region.h>

/************************************************************************

Copyright (c) 1987  x Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
x CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the x Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the x Consortium.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted,
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

************************************************************************/

namespace Fog {

// [Fog::Region]

#define Fog_Region_Local 256

// [static functions]

static FOG_INLINE void _copyRects(Box* dest, const Box* src, sysuint_t count)
{
  sysuint_t i;
  for (i = count; i; i--, dest++, src++) *dest = *src;
}

static void _copyRectsExtents(Box* dest, const Box* src, sysuint_t count, Box* extents)
{
  sysuint_t i;

  int extentsX1 = src[0].x1();
  int extentsY1 = src[0].y1();
  int extentsX2 = src[count-1].x2();
  int extentsY2 = src[count-1].y2();

  for (i = count; i; i--, dest++, src++) 
  {
    if (extentsX1 > src->x1()) extentsX1 = src->x1();
    if (extentsX2 < src->x2()) extentsX2 = src->x2();
    *dest = *src;
  }

  extents->set(extentsX1, extentsY1, extentsX2, extentsY2);
}

/*
  -----------------------------------------------------------------------
  Utility procedure compress_:

  Replace r by the region r', where
    p in r' iff (Quantifer m <= dx) (p + m in r), and
    Quantifier is Exists if grow is true, For all if grow is false, and
    (x,y) + m = (x+m,y) if xdir is true; (x,y+m) if xdir is false.

  Thus, if xdir is true and grow is false, r is replaced by the region
  of all points p such that p and the next dx points on the same
  horizontal scan line are all in r.  We do this using by noting
  that p is the head of a run of length 2^i + k iff p is the head
  of a run of length 2^i and p+2^i is the head of a run of length
  k. Thus, the loop invariant: s contains the region corresponding
  to the runs of length shift. r contains the region corresponding
  to the runs of length 1 + dxo & (shift-1), where dxo is the original
  value of dx.  dx = dxo & ~(shift-1).  As parameters, s and t are
  scratch regions, so that we don't have to allocate them on every
  call.
  -----------------------------------------------------------------------
*/
static void _compress(
  Region& r, Region& s, Region& t,
  register uint dx,
  register bool xdir, register bool grow)
{
  register uint shift = 1;

  s.setDeep(r);

  while (dx)
  {
    if (dx & shift)
    {
      if (xdir)
        Region::translate(r, r, Point(-(int)shift, 0)); 
      else
        Region::translate(r, r, Point(0, -(int)shift));

      if (grow)
        r.unite(s);
      else
        r.intersect(s);

      dx -= shift;
      if (!dx) break;
    }

    t.setDeep(s);
    if (xdir)
      Region::translate(s, s, Point(-(int)shift, 0));
    else 
      Region::translate(s, s, Point(0, -(int)shift));

    if (grow)
      s.unite(t); 
    else
      s.intersect(t);

    shift <<= 1;
  }
}

/*
  -----------------------------------------------------------------------
  coalesce
    Attempt to merge the boxes in the current band with those in the
    previous one. Used only by miRegionOp.

  Results:
    dest pointer (can be smaller if coalesced)

  Side Effects:
    If coalescing takes place:
    - rectangles in the previous band will have their y2 fields
      altered.
    - Count may be decreased.
  -----------------------------------------------------------------------
*/

static Box* _coalesceHelper(Box* dest_ptr, Box** prev_start_, Box** cur_start_)
{
  Box* prev_start = *prev_start_;
  Box* cur_start  = *cur_start_;

  sysuint_t c1 = (sysuint_t)( cur_start - prev_start );
  sysuint_t c2 = (sysuint_t)( dest_ptr - cur_start );

  if (c1 == c2 && prev_start->y2() == cur_start->y1())
  {
    // need to scan their x coords, if prev_start and cur_start will has
    // same x coordinates, we can coalesce it
    sysuint_t i;
    for (i = 0; i != c1; i++)
    {
      if (prev_start[i].x1() != cur_start[i].x1() || prev_start[i].x2() != cur_start[i].x2())
      {
        // coalesce isn't possible
        *prev_start_ = cur_start;
        return dest_ptr;
      }
    }

    // coalesce
    int bot = cur_start->y2();
    for (i = 0; i != c1; i++) prev_start[i].setY2(bot);
    return cur_start;
  }

  *prev_start_ = cur_start;
  return dest_ptr;
}

// inline here produces better results
static FOG_INLINE Box* _coalesce(Box* dest_ptr, Box** prev_start_, Box** cur_start_)
{
  if (*prev_start_ != *cur_start_)
  {
    return _coalesceHelper(dest_ptr, prev_start_, cur_start_);
  }
  else
    return dest_ptr;
}

// Forward declarations for rectangle processor
static void _unitePrivate(Region* dest, const Box* src1, sysuint_t count1, const Box* src2, sysuint_t count2, bool memOverlap, const Box* new_extents);
static void _intersectPrivate(Region* dest, const Box* src1, sysuint_t count1, const Box* src2, sysuint_t count2, bool memOverlap);
static void _subtractPrivate(Region* dest, const Box* src1, sysuint_t count1, const Box* src2, sysuint_t count2, bool memOverlap);
static void _appendPrivate(Region* dest, const Box* src, sysuint_t count, const Box* new_extents);

// ::Data statics
static Region::Data* _reallocRegion(Region::Data* d, sysuint_t capacity);

// [Fog::Region]

// ---------------------------------------------------------------------------
// Fog::Region private routines (union, subtraction, intersection and append)
// ---------------------------------------------------------------------------

static void _unitePrivate(Region* dest, const Box* src1, sysuint_t count1, const Box* src2, sysuint_t count2, bool memOverlap, const Box* new_extents)
{
  Box* destBegin;                      // destination begin
  Box* destCur;                        // destination ptr

  const Box* src1End = src1 + count1;  // end of src1
  const Box* src2End = src2 + count2;  // end of src2

  const Box* src1BandEnd;              // end of current band in src1
  const Box* src2BandEnd;              // end of current band in src2

  int top;                             // Top of non-overlapping band
  int bot;                             // Bottom of non-overlapping band

  int ytop;                            // top of intersection
  int ybot;                            // bottom of intersection

  Box* prevBand;                       // pointer to start of previous band
  Box* curBand;                        // pointer to start of current band

  sysuint_t min_rects_needed = (count1 + count2) << 1;

  // trivial reject
  if (src1 == src1End || src2 == src2End) 
  {
    dest->clear();
    return;
  }

  // new region data in case that it needs it
  Region::Data* newd = 0;

  // local buffer that can be used instead of malloc in mosst calls
  // can be increased to higher values, but I think that 256 is ok
  Box static_buffer[256];

  if (memOverlap)
  {
    // need to allocate new block
    if (min_rects_needed < FOG_ARRAY_SIZE(static_buffer))
    {
      destCur = static_buffer;
    }
    else
    {
      newd = Region::Data::create(min_rects_needed, Fog::AllocCantFail);
      destCur = newd->rects;
    }
  }
  else
  {
    // can use dest
    dest->prepare(min_rects_needed);
    destCur = dest->_d->rects;
  }

  destBegin = destCur;

  /*
    Initialize ybot and ytop.
    In the upcoming loop, ybot and ytop serve different functions depending
    on whether the band being handled is an overlapping or non-overlapping
    band.
    In the case of a non-overlapping band (only one of the regions
    has points in the band), ybot is the bottom of the most recent
    intersection and thus clips the top of the rectangles in that band.
    ytop is the top of the next intersection between the two regions and
    serves to clip the bottom of the rectangles in the current band.
    For an overlapping band (where the two regions intersect), ytop clips
    the top of the rectangles of both regions and ybot clips the bottoms.
  */

  if (src1->y1() < src2->y1())
    ybot = src1->y1();
  else
    ybot = src2->y1();

  /*
    prevBand serves to mark the start of the previous band so rectangles
    can be coalesced into larger rectangles. qv. coalesce, above.
    In the beginning, there is no previous band, so prevBand == curBand
    (curBand is set later on, of course, but the first band will always
    start at index 0). prevBand and curBand must be indices because of
    the possible expansion, and resultant moving, of the new region's
    array of rectangles.
  */
  prevBand = destCur;

  do {
    curBand = destCur;
    /*
      This algorithm proceeds one source-band (as opposed to a
      destination band, which is determined by where the two regions
      intersect) at a time. src1BandEnd and src2BandEnd serve to mark the
      rectangle after the last one in the current band for their
      respective regions.
    */
    src1BandEnd = src1;
    src2BandEnd = src2;

    while ((++src1BandEnd != src1End) && (src1BandEnd->y1() == src1->y1())) ;
    while ((++src2BandEnd != src2End) && (src2BandEnd->y1() == src2->y1())) ;

    /*
      First handle the band that doesn't intersect, if any.

      Note that attention is restricted to one band in the
      non-intersecting region at once, so if a region has n
      bands between the current position and the next place it overlaps
      the other, this entire loop will be passed through n times.
    */
    if (src1->y1() < src2->y1())
    {
      top = fog_max(src1->y1(), ybot);
      bot = fog_min(src1->y2(), src2->y1());

      if (top != bot)
      {
        const Box* ovrlp = src1;
        while(ovrlp != src1BandEnd) { (*destCur++).set(ovrlp->x1(), top, ovrlp->x2(), bot); ovrlp++; }

        destCur = _coalesce(destCur, &prevBand, &curBand);
      }
    }
    else if (src2->y1() < src1->y1())
    {
      top = fog_max(src2->y1(), ybot);
      bot = fog_min(src2->y2(), src1->y1());

      if (top != bot)
      {
        const Box* ovrlp = src2;
        while(ovrlp != src2BandEnd) { (*destCur++).set(ovrlp->x1(), top, ovrlp->x2(), bot); ovrlp++; }

        destCur = _coalesce(destCur, &prevBand, &curBand);
      }
    }

    /*
      Now see if we've hit an intersecting band. The two bands only
      intersect if ybot > ytop
    */
    ytop = fog_max(src1->y1(), src2->y1());
    ybot = fog_min(src1->y2(), src2->y2());
    if (ybot > ytop)
    {
      const Box* i1 = src1;
      const Box* i2 = src2;

      // unite
      #define MERGE_RECT(__x1__, __y1__, __x2__, __y2__) \
          if (destCur != destBegin &&                                   \
             (destCur[-1].y1() == __y1__) &&                            \
             (destCur[-1].y2() == __y2__) &&                            \
             (destCur[-1].x2() >= __x1__))                              \
          {                                                             \
              if (destCur[-1].x2() < __x2__) destCur[-1]._x2 = __x2__;  \
          }                                                             \
          else {                                                        \
              (*destCur++).set(__x1__, __y1__, __x2__, __y2__);         \
          }

      while ((i1 != src1BandEnd) && (i2 != src2BandEnd))
      {
        if (i1->x1() < i2->x1())
        {
          MERGE_RECT(i1->x1(), ytop, i1->x2(), ybot); i1++;
        }
        else
        {
          MERGE_RECT(i2->x1(), ytop, i2->x2(), ybot); i2++;
        }
      }

      if (i1 != src1BandEnd)
      {
        do {
          MERGE_RECT(i1->x1(), ytop, i1->x2(), ybot); i1++;
        } while (i1 != src1BandEnd);
      }
      else
      {
        while (i2 != src2BandEnd)
        {
          MERGE_RECT(i2->x1(), ytop, i2->x2(), ybot); i2++;
        }
      }

      #undef MERGE_RECT

      destCur = _coalesce(destCur, &prevBand, &curBand);
    }

    /*
      If we've finished with a band (y2 == ybot) we skip forward
      in the region to the next band.
    */
    if (src1->y2() == ybot) src1 = src1BandEnd;
    if (src2->y2() == ybot) src2 = src2BandEnd;
  } while ((src1 != src1End) && (src2 != src2End));

  /*
    Deal with whichever region still has rectangles left.
  */
  if (src1 != src1End || src2 != src2End)
  {
    const Box* src;
    const Box* srcEnd;

    curBand = destCur;

    if (src1 != src1End)
    {
      src = src1; srcEnd = src1End;
    }
    else
    {
      src = src2; srcEnd = src2End;
    }

    int y0 = src->y1();
    int y1 = fog_max(src->y1(), ybot);

    // append first band and coalesce
    while (src != srcEnd && src->y1() == y0) { (*destCur++).set(src->x1(), y1, src->x2(), src->y2()); src++; }
    destCur = _coalesce(destCur, &prevBand, &curBand);

    // append remaining rectangles, coalesce isn't needed
    while (src != srcEnd) *destCur++ = *src++;

  }

  // finished, we have complete intersected region in destCur
  sysuint_t c = sysuint_t( destCur - destBegin );

  // its paranoid here, because unite_private is called only if there are
  // rectangles in regions
  if (c)
  {
    if (memOverlap)
    {
      if (destBegin == static_buffer)
      {
        dest->reserve(c);
        _copyRects(dest->_d->rects, destBegin, c);
      }
      else
      {
        dest->_d->DEREF_INLINE();
        dest->_d = newd;
      }
    }

    dest->_d->extents = *new_extents;
    dest->_d->count = c;
  }
  else
  {
    if (memOverlap && destBegin == static_buffer) Fog::Memory::free(newd);
    dest->clear();
  }
}

static void _intersectPrivate(Region* dest, const Box* src1, sysuint_t count1, const Box* src2, sysuint_t count2, bool memOverlap)
{
  Box* destBegin;                      // destination begin
  Box* destCur;                        // destination ptr

  const Box* src1End = src1 + count1;  // end of src1
  const Box* src2End = src2 + count2;  // end of src2

  const Box* src1BandEnd;              // end of current band in src1
  const Box* src2BandEnd;              // end of current band in src2

  int ytop;                            // top of intersection
  int ybot;                            // bottom of intersection

  int extentsX1 = INT_MAX;             // extents x1 coord (computed in loop)
  int extentsX2 = INT_MIN;             // extents x2 coord (computed in loop)

  Box* prevBand;                       // pointer to start of previous band
  Box* curBand;                        // pointer to start of current band

  // simplest case, if there are only 1 rect in each -> rects overlap
  if (count1 == 1 && count2 == 1)
  {
    dest->set(Box(fog_max(src1->x1(), src2->x1()), fog_max(src1->y1(), src2->y1()),
                  fog_min(src1->x2(), src2->x2()), fog_min(src1->y2(), src2->y2())));
    return;
  }

  sysuint_t min_rects_needed = (count1 + count2) << 1;

  /* Find first rectangle that can intersect */
  while (true)
  {
    if (src1 == src1End || src2 == src2End) { dest->clear(); return; }

    if (src1->y2() < src2->y1()) { src1++; continue; }
    if (src2->y2() < src1->y1()) { src2++; continue; }

    break;
  }

  // new region data in case that it needs it
  Region::Data* newd = 0;

  // local buffer that can be used instead of malloc in mosst calls
  // can be increased to higher values, but I think that 256 is ok
  Box static_buffer[256];

  if (memOverlap)
  {
    // need to allocate new block
    if (min_rects_needed < FOG_ARRAY_SIZE(static_buffer))
    {
      destCur = static_buffer;
    }
    else
    {
      newd = Region::Data::create(min_rects_needed, Fog::AllocCantFail);
      destCur = newd->rects;
    }
  }
  else
  {
    // can use dest
    dest->prepare(min_rects_needed);
    destCur = dest->_d->rects;
  }

  destBegin = destCur;

  /*
    prevBand serves to mark the start of the previous band so rectangles
    can be coalesced into larger rectangles. qv. miCoalesce, above.
    In the beginning, there is no previous band, so prevBand == curBand
    (curBand is set later on, of course, but the first band will always
    start at index 0). prevBand and curBand must be indices because of
    the possible expansion, and resultant moving, of the new region's
    array of rectangles.
  */
  prevBand = destCur;

  do {
    curBand = destCur;

    /*
      This algorithm proceeds one source-band (as opposed to a
      destination band, which is determined by where the two regions
      intersect) at a time. src1BandEnd and src2BandEnd serve to mark the
      rectangle after the last one in the current band for their
      respective regions.
    */
    src1BandEnd = src1;
    src2BandEnd = src2;

    while ((src1BandEnd != src1End) && (src1BandEnd->y1() == src1->y1())) src1BandEnd++;
    while ((src2BandEnd != src2End) && (src2BandEnd->y1() == src2->y1())) src2BandEnd++;

    /*
      See if we've hit an intersecting band. The two bands only
      intersect if ybot > ytop
    */
    ytop = fog_max(src1->y1(), src2->y1());
    ybot = fog_min(src1->y2(), src2->y2());
    if (ybot > ytop)
    {
      // intersect
      const Box* i1 = src1;
      const Box* i2 = src2;

      int x1;
      int x2;

      while ((i1 != src1BandEnd) && (i2 != src2BandEnd))
      {
        x1 = fog_max(i1->x1(), i2->x1());
        x2 = fog_min(i1->x2(), i2->x2());

        /*
          If there's any overlap between the two rectangles, add that
          overlap to the new region.
          There's no need to check for subsumption because the only way
          such a need could arise is if some region has two rectangles
          right next to each other. Since that should never happen...
        */
        if (x1 < x2)
        {
          // append rectangle
          destCur->set(x1, ytop, x2, ybot);
          destCur++;
        }

        /*
          Need to advance the pointers. Shift the one that extends
          to the right the least, since the other still has a chance to
          overlap with that region's next rectangle, if you see what I mean.
        */
        if (i1->x2() < i2->x2())
        {
          i1++;
        }
        else if (i2->x2() < i1->x2())
        {
          i2++;
        }
        else
        {
          i1++;
          i2++;
        }
      }

      if (curBand != destCur)
      {
        destCur = _coalesce(destCur, &prevBand, &curBand);

        // update x1 and x2 extents
        if (curBand[0].x1() < extentsX1) extentsX1 = curBand[0].x1();
        if (destCur[-1].x2() > extentsX2) extentsX2 = destCur[-1].x2();
      }
    }

    /*
      If we've finished with a band (y2 == ybot) we skip forward
      in the region to the next band.
    */
    if (src1->y2() == ybot) src1 = src1BandEnd;
    if (src2->y2() == ybot) src2 = src2BandEnd;
  } while ((src1 != src1End) && (src2 != src2End));

  // finished, we have complete intersected region in destCur
  sysuint_t c = sysuint_t( destCur - destBegin );

  if (c)
  {
    if (memOverlap) {
      if (destBegin == static_buffer)
      {
        dest->reserve(c);
        _copyRects(dest->_d->rects, destBegin, c);
      }
      else
      {
        dest->_d->deref();
        dest->_d = newd;
      }
    }

    dest->_d->count = c;
    dest->_d->extents.set(
      extentsX1, destBegin[0].y1(), 
      extentsX2, destCur[-1].y2());
  }
  else
  {
    if (memOverlap && destBegin == static_buffer) Fog::Memory::free(newd);
    dest->clear();
  }
}

static void _subtractPrivate(Region* dest, const Box* src1, sysuint_t count1, const Box* src2, sysuint_t count2, bool memOverlap)
{
  Box* destBegin;                      // destination begin
  Box* destCur;                        // destination ptr
  Box* destEnd;                        // destination end

  const Box* src1End = src1 + count1;  // end of src1
  const Box* src2End = src2 + count2;  // end of src2

  const Box* src1BandEnd;              // end of current band in src1
  const Box* src2BandEnd;              // end of current band in src2

  int top;                             // Top of non-overlapping band
  int bot;                             // Bottom of non-overlapping band

  int ytop;                            // top of intersection
  int ybot;                            // bottom of intersection

  int extentsX1 = INT_MAX;             // extents x1 coord (computed in loop)
  int extentsX2 = INT_MIN;             // extents x2 coord (computed in loop)

  Box* prevBand;                       // pointer to start of previous band
  Box* curBand;                        // pointer to start of current band

  sysuint_t min_rects_needed = (count1 + count2) << 1;

  Region::Data* d;

  if (memOverlap)
  {
    d = Region::Data::create(min_rects_needed, Fog::AllocCantFail);
  }
  else
  {
    dest->prepare(min_rects_needed);
    d = dest->_d;
  }

  destCur = d->rects;
  destEnd = destCur + d->capacity;
  destBegin = destCur;

  /*
    Initialize ybot and ytop.
    In the upcoming loop, ybot and ytop serve different functions depending
    on whether the band being handled is an overlapping or non-overlapping
    band.
    In the case of a non-overlapping band (only one of the regions
    has points in the band), ybot is the bottom of the most recent
    intersection and thus clips the top of the rectangles in that band.
    ytop is the top of the next intersection between the two regions and
    serves to clip the bottom of the rectangles in the current band.
    For an overlapping band (where the two regions intersect), ytop clips
    the top of the rectangles of both regions and ybot clips the bottoms.
  */

  if (src1->y1() < src2->y1())
    ybot = src1->y1();
  else
    ybot = src2->y1();

  /*
    prevBand serves to mark the start of the previous band so rectangles
    can be coalesced into larger rectangles. qv. coalesce, above.
    In the beginning, there is no previous band, so prevBand == curBand
    (curBand is set later on, of course, but the first band will always
    start at index 0). prevBand and curBand must be indices because of
    the possible expansion, and resultant moving, of the new region's
    array of rectangles.
  */
  prevBand = destCur;

  /*
    macro for merging rectangles, it's designed to simplify loop, because
    result of subtraction can be more than 'min_need_needed' we need to
    detect end of the destination buffer.
  */
  #define ADD_RECT(__x1__, __y1__, __x2__, __y2__) \
      if (FOG_UNLIKELY(destCur == destEnd)) {                        \
          sysuint_t count = d->capacity;                              \
          sysuint_t prevBandIndex = (sysuint_t)(prevBand - destBegin);\
          sysuint_t curBandIndex = (sysuint_t)(curBand - destBegin);  \
                                                                      \
          d = _reallocRegion(d, count << 1);                          \
          destBegin = d->rects;                                       \
          destCur = destBegin + count;                                \
          destEnd = destBegin + d->capacity;                          \
                                                                      \
          prevBand = destBegin + prevBandIndex;                       \
          curBand = destBegin + curBandIndex;                         \
      }                                                               \
      destCur->set(__x1__, __y1__, __x2__, __y2__);                   \
      destCur++;                                                      \
                                                                      \
      if (FOG_UNLIKELY(extentsX1 > __x1__)) extentsX1 = __x1__;      \
      if (FOG_UNLIKELY(extentsX2 < __x2__)) extentsX2 = __x2__

  do {
    curBand = destCur;
    /*
      This algorithm proceeds one source-band (as opposed to a
      destination band, which is determined by where the two regions
      intersect) at a time. src1BandEnd and src2BandEnd serve to mark the
      rectangle after the last one in the current band for their
      respective regions.
    */
    src1BandEnd = src1;
    src2BandEnd = src2;

    while ((src1BandEnd != src1End) && (src1BandEnd->y1() == src1->y1())) src1BandEnd++;
    while ((src2BandEnd != src2End) && (src2BandEnd->y1() == src2->y1())) src2BandEnd++;

    /*
      First handle the band that doesn't intersect, if any.

      Note that attention is restricted to one band in the
      non-intersecting region at once, so if a region has n
      bands between the current position and the next place it overlaps
      the other, this entire loop will be passed through n times.
    */
    if (src1->y1() < src2->y1())
    {
      // non overlap (src1) - merge it
      top = fog_max(src1->y1(), ybot);
      bot = fog_min(src1->y2(), src2->y1());

      if (top != bot)
      {
        const Box* ovrlp = src1;
        while (ovrlp != src1BandEnd) { ADD_RECT(ovrlp->x1(), top, ovrlp->x2(), bot); ovrlp++; }

        destCur = _coalesce(destCur, &prevBand, &curBand);
      }
    }

    /*
      Now see if we've hit an intersecting band. The two bands only
      intersect if ybot > ytop
    */
    ytop = fog_max(src1->y1(), src2->y1());
    ybot = fog_min(src1->y2(), src2->y2());
    if (ybot > ytop)
    {
      const Box* i1 = src1;
      const Box* i2 = src2;

      int x1 = i1->x1();

      while ((i1 != src1BandEnd) && (i2 != src2BandEnd))
      {
        if (i2->x2() <= x1)
        {
          /* Subtrahend missed the boat: go to next subtrahend. */
          i2++;
        }
        else if (i2->x1() <= x1)
        {
          /* Subtrahend preceeds minuend: nuke left edge of minuend. */
          x1 = i2->x2();
          if (x1 >= i1->x2())
          {
            /*
              Minuend completely covered: advance to next minuend and
              reset left fence to edge of new minuend.
            */
            if (++i1 != src1BandEnd) x1 = i1->x1();
          }
          else
          {
            /*
              Subtrahend now used up since it doesn't extend beyond
              minuend
            */
            i2++;
          }
        }
        else if (i2->x1() < i1->x2())
        {
          /*
            x1 part of subtrahend covers part of minuend: add uncovered
            part of minuend to region and skip to next subtrahend.
          */
          ADD_RECT(x1, ytop, i2->x1(), ybot);

          x1 = i2->x2();
          if (x1 >= i1->x2())
          {
            /* Minuend used up: advance to new... */
            if (++i1 != src1BandEnd) x1 = i1->x1();
          }
          else
          {
            /* Subtrahend used up */
            i2++;
          }
        }
        else
        {
          /* Minuend used up: add any remaining piece before advancing. */
          if (i1->x2() > x1) { ADD_RECT(x1, ytop, i1->x2(), ybot); }
          if (++i1 != src1BandEnd) x1 = i1->x1();
        }
      }

      /* Add remaining minuend rectangles to region. */
      while (i1 != src1BandEnd)
      {
        ADD_RECT(x1, ytop, i1->x2(), ybot);
        if (++i1 != src1BandEnd) x1 = i1->x1();
      }

      destCur = _coalesce(destCur, &prevBand, &curBand);
    }

    /*
      If we've finished with a band (y2 == ybot) we skip forward
      in the region to the next band.
    */
    if (src1->y2() == ybot) src1 = src1BandEnd;
    if (src2->y2() == ybot) src2 = src2BandEnd;
  } while ((src1 != src1End) && (src2 != src2End));

  /*
    Deal with whichever src1 still has rectangles left.
  */
  if (src1 != src1End)
  {
    const Box* src;
    const Box* srcEnd;

    curBand = destCur;
    src = src1; srcEnd = src1End;

    int y0 = src->y1();
    int y1 = fog_max(src->y1(), ybot);

    // append first band and coalesce
    while (src != srcEnd && src->y1() == y0)
    {
      ADD_RECT(src->x1(), y1, src->x2(), src->y2()); src++;
    }
    destCur = _coalesce(destCur, &prevBand, &curBand);

    // append remaining rectangles, coalesce isn't needed
    while (src != srcEnd) { ADD_RECT(src->x1(), src->y1(), src->x2(), src->y2()); src++; }
  }

  #undef ADD_RECT

  // finished, we have complete intersected region in destCur
  sysuint_t count = sysuint_t( destCur - destBegin );
  if (count)
  {
    if (memOverlap) dest->_d->deref();

    d->extents.set(extentsX1, destBegin[0].y1(), extentsX2, destCur[-1].y2());
    d->count = count;
    dest->_d = d;
  }
  else
  {
    if (memOverlap) Fog::Memory::free(d);
    dest->clear();
  }
}

static void _appendPrivate(Region* dest, const Box* src, sysuint_t count, const Box* new_extents)
{
  // its guaranted that we CAN append region and its also guaranted that
  // destination region has minimal 1 rectangle, so [-1] index from end
  // must be valid
  dest->reserve(dest->count() + count);

  Box* destBegin = dest->_d->rects;
  Box* destCur = destBegin + dest->_d->count;

  const Box* srcEnd = src + count;

  if (src->y1() == destCur[-1].y1())
  {
    // here is an interesting case that we will append an band that exist
    // in destination
    //
    // XXXXXXXXX
    // XXXXX<-Here

    // merge first? (we can increase an existing band)
    if (destCur[-1].x2() == src->x1()) { destCur[-1].setX2(src->x2()); src++; }

    // append all other bands
    while (src != srcEnd && destCur[-1].y1() == src->y1()) *destCur++ = *src++;

    // coalesce
    {
      Box* destBand1Begin;
      Box* destBand2Begin = destCur-1;

      int y = destBand2Begin->y1();

      while (destBand2Begin != destBegin && destBand2Begin[-1].y1() == y) destBand2Begin--;
      if (destBand2Begin != destBegin && destBand2Begin[-1].y2() == destBand2Begin[0].y1())
      {
        destBand1Begin = destBand2Begin - 1;
        while (destBand1Begin != destBegin && destBand1Begin[-1].y2() == y) destBand1Begin--;
        
        sysuint_t index1 = (sysuint_t)(destBand2Begin - destBand1Begin);
        sysuint_t index2 = (sysuint_t)(destCur - destBand2Begin);

        if (index1 == index2)
        {
          // it's chance for coalesce, need to compare bands
          sysuint_t i;
          for (i = 0; i != index1; i++)
          {
            if (destBand1Begin[i].x1() == destBand2Begin[i].x1() &&
                destBand1Begin[i].x2() == destBand2Begin[i].x2()) continue;
            goto __noCoalesce;
          }

          // coalesce now;
          y = destBand2Begin[0].y2();
          for (i = 0; i != index1; i++)
          {
            destBand1Begin[i].setY2(y);
          }
          destCur -= index1;
        }
      }
    }

__noCoalesce:
    if (src == srcEnd) goto __end;
  }

  if (src->y1() == destCur[-1].y2())
  {
    // coalesce, need to find previous band in dest
    Box* destBandBegin = destCur-1;
    while (destBandBegin != destBegin && destBandBegin->y1() == destCur[-1].y1()) destBandBegin--;

    const Box* srcBandBegin = src;
    const Box* srcBandEnd = src;
    while(srcBandEnd != srcEnd && srcBandEnd->y1() == src->y1()) srcBandEnd++;

    // now we have:
    // - source:
    //       srcBandBegin
    //       srcBandEnd
    // - dest:
    //       destBandBegin
    //       destBandEnd == destCur !
    if (srcBandEnd - srcBandBegin == destCur - destBandBegin)
    {
      // probabbility for coasesce...
      sysuint_t i, count = (sysuint_t)(srcBandEnd - srcBandBegin);
      for (i = 0; i != count; i++)
      {
        if (srcBandBegin[i].x1() != destBandBegin[i].x1() ||
            srcBandBegin[i].x2() != destBandBegin[i].x2())
        {
          goto __fast_append;
        }
      }

      // coalesce success
      int y2 = srcBandBegin->y2();
      for (i = 0; i != count; i++) destBandBegin[i].setY2(y2);
    }
  }

__fast_append:
  // fastest case, fast append, no coasesce...
  while (src != srcEnd) *destCur++ = *src++;

__end:
  dest->_d->count = (sysuint_t)(destCur - destBegin);
  dest->_d->extents = *new_extents;
}

// [Fog::Region]

Fog::Static<Region::Data> Region::sharedNull;

Region::Region() :
  _d(sharedNull.instancep()->REF_ALWAYS())
{
}

Region::Region(const Box& rect) :
  _d(Data::create(1, &rect, &rect, 1))
{
}

Region::Region(const Rect& rect)
{
  Box t(rect);
  _d = Data::create(1, &t, &t, 1);
}

Region::Region(const Region& other) :
  _d(other._d->REF_INLINE())
{
}

Region::Region(Data* d) :
  _d(d)
{
}

Region::~Region()
{
  _d->DEREF_INLINE();
}

void Region::_detach()
{
  Data* d = _d;

  if (d->refCount.get() > 1)
  {
    if (d->count > 0)
    {
      d = Data::create(d->count, &d->extents, d->rects, d->count);
    }
    else
    {
      d = Data::create(d->count, Fog::AllocCantFail);
      d->extents.clear();
    }
    Fog::AtomicBase::ptr_setXchg(&_d, d)->DEREF_INLINE();
  }
}

bool Region::_tryDetach()
{
  Data* d = _d;

  if (d->refCount.get() > 1)
  {
    sysuint_t count = d->count;

    d = Data::create(count, Fog::AllocCantFail);
    if (!d) return false;
    d->count = count;
    d->extents = _d->extents;
    _copyRects(d->rects, _d->rects, count);
    Fog::AtomicBase::ptr_setXchg(&_d, d)->DEREF_INLINE();
  }
  return true;
}

void Region::free()
{
  Fog::AtomicBase::ptr_setXchg(&_d,
    sharedNull.instancep()->REF_ALWAYS())->DEREF_INLINE();
}

void Region::setSharable(bool val)
{
  if (isSharable() != val)
  {
    detach();

    if (val)
      _d->flags |= Data::IsSharable;
    else
      _d->flags &= ~Data::IsSharable;
  }
}

void Region::setStrong(bool val)
{
  if (isSharable() != val)
  {
    detach();

    if (val)
      _d->flags |= Data::IsStrong;
    else
      _d->flags &= ~Data::IsStrong;
  }
}

void Region::reserve(sysuint_t to)
{
  Data* d = _d;

  if (d->refCount.get() > 1)
  {
__create:
    Fog::AtomicBase::ptr_setXchg(&_d, 
      Data::create(to, &d->extents, d->rects, d->count))->DEREF_INLINE();
  }
  else if (d->capacity < to)
  {
    if (!(d->flags & Data::IsDynamic)) goto __create;

    d = _d = (Data *)Fog::Memory::xrealloc(d, Data::sizeFor(to));
    d->capacity = to;
  }
}

void Region::prepare(sysuint_t to)
{
  Data* d = _d;

  if (d->refCount.get() > 1)
  {
__create:
    Fog::AtomicBase::ptr_setXchg(&_d, 
      Data::create(to, Fog::AllocCantFail))->DEREF_INLINE();
  }
  else if (d->capacity < to)
  {
    if (!(d->flags & Data::IsDynamic)) goto __create;

    d = _d = (Data *)Fog::Memory::xrealloc(d, Data::sizeFor(to));
    d->capacity = to;
    d->count = 0;
  }
  else
    d->count = 0;
}

void Region::squeeze()
{
  Data* d = _d;
  
  if (d->flags & Data::IsDynamic)
  {
    sysuint_t count = d->count;
    if (d->capacity == count) return;

    if (d->refCount.get() > 1)
    {
      d = Data::create(count, Fog::AllocCanFail);
      if (!d) return;
      d->count = count;
      d->extents = _d->extents;
      _copyRects(d->rects, _d->rects, count);
      Fog::AtomicBase::ptr_setXchg(&_d, d)->deref();
    }
    else
    {
      d = (Data *)Fog::Memory::realloc(d, Data::sizeFor(count));
      if (!d) return;
      _d = d;
      d->capacity = count;
    }
  }
}

uint32_t Region::type() const
{
  uint32_t t = _d->count;
  return t < 2 ? t : 2;
}

uint32_t Region::contains(const Point& pt) const
{
  Data* d = _d;

  sysuint_t i;
  sysuint_t c = d->count;

  if (!c) return Out;

  // Check if point position is in extents, if not -> Out
  if (!(d->extents.contains(pt))) return Out;
  if (c == 1) return In;

  // Binary search for matching position
  const Box* base = d->rects;
  const Box* r;

  int x = pt.x();
  int y = pt.y();

  for (i = c; i != 0; i >>= 1)
  {
    r = base + (i >> 1);

    // Try match
    if (y >= r->y1() && y < r->y2())
    {
      if (x >= r->x1())
      {
        if (x < r->x2())
        {
          // Match
          return In;
        }
        else
        {
          // Move right
          base = r + 1;
          i--;
        }
      }
      // else: Move left
    }
    else if (r->y2() <= y)
    {
      // Move right
      base = r + 1;
      i--;
    }
    // else: Move left
  }
  return Out;
}

uint Region::contains(const Rect& r) const
{
  return contains(Box(r));
}

uint Region::contains(const Box& r) const
{
  Data* d = _d;

  sysuint_t c = d->count;

  // this is (just) a useful optimization
  if (!c || !d->extents.overlaps(r)) return Out;

  const Box* cur = d->rects;
  const Box* end = cur + c;

  bool partIn = false;
  bool partOut = false;

  int x = r.x1();
  int y = r.y1();

  // can stop when both PartOut and PartIn are true, or we reach cur->y2
  for (; cur < end; cur++)
  {
    if (cur->y2() <= y)
      // getting up to speed or skipping remainder of band
      continue;

    if (cur->y1() > y)
    {
      // missed part of rectangle above
      partOut = true;
      if (partIn || (cur->y1() >= r.y2())) break;
      // x guaranteed to be == rect->x1
      y = cur->y1();
    }

    if (cur->x2() <= x)
      // not far enough over yet
      continue;

    if (cur->x1() > x)
    {
      // missed part of rectangle to left
      partOut = true;
      if (partIn) break;
    }

    if (cur->x1() < r.x2())
    {
      // definitely overlap
      partIn = true;
      if (partOut) break;
    }

    if (cur->x2() >= r.x2())
    {
      // finished with this band
      y = cur->y2();
      if (y >= r.y2()) break;
      // reset x out to left again
      x = r.x1();
    }
    else
    {
      // Because boxes in a band are maximal width, if the first box
      // to overlap the rectangle doesn't completely cover it in that
      // band, the rectangle must be partially out, since some of it
      // will be uncovered in that band. partIn will have been set true
      // by now...
      break;
    }
  }

  return (partIn) ? ((y < r.y2()) ? Part : In) : Out;
}

Region& Region::clear()
{
  Data* d = _d;
  if (d->count == 0) return *this;

  if (d->refCount.get() > 0)
  {
    Fog::AtomicBase::ptr_setXchg(&_d, sharedNull.instancep()->REF_ALWAYS())->DEREF_INLINE();
  }
  else
  {
    d->count = 0;
    d->extents.clear();
  }

  return *this;
}

Region& Region::set(const Region& r)
{
  Data* td = _d;
  Data* rd = r._d;
  if (td == rd) return *this;

  if ( (td->flags & Data::IsStrong) ||
      !(rd->flags & Data::IsSharable))
  {
    prepare(rd->count);

    td = _d;
    td->count = rd->count;
    td->extents = rd->extents;
    _copyRects(td->rects, rd->rects, rd->count);
  }
  else
  {
    Fog::AtomicBase::ptr_setXchg(&_d, rd->REF_ALWAYS())->DEREF_INLINE();
  }

  return *this;
}

Region& Region::set(const Rect& r)
{
  if (!r.isValid()) return clear();

  prepare(1);
  Data* d = _d;
  d->count = 1;
  Box b(r);
  d->extents = b;
  d->rects[0] = b;

  return *this;
}

Region& Region::set(const Box& r)
{
  if (!r.isValid()) return clear();

  prepare(1);
  Data* d = _d;
  d->count = 1;
  d->extents = r;
  d->rects[0] = r;

  return *this;
}

Region& Region::setDeep(const Region& r)
{
  Data* td = _d;
  Data* rd = r._d;
  if (td == rd) return *this;

  prepare(rd->count);
  td = _d;
  td->count = rd->count;
  td->extents = rd->extents;
  _copyRects(td->rects, rd->rects, rd->count);

  return *this;
}

Region& Region::set(const Rect* rects, sysuint_t count)
{
  // TODO: Not optimal
  clear();
  for (sysuint_t i = 0; i < count; i++) unite(rects[i]);
  return *this;
}

Region& Region::set(const Box* rects, sysuint_t count)
{
  // TODO: Not optimal
  clear();
  for (sysuint_t i = 0; i < count; i++) unite(rects[i]);
  return *this;
}

Region& Region::unite(const Region& r)
{
  Data* td = _d;
  Data* rd = r._d;

  // Union region is same or r is empty... -> nop
  if (td == rd || rd->count == 0) return *this;

  // We are empty or r completely subsumes us -> set r
  if (td->count == 0 || (rd->count == 1 && rd->extents.subsumes(td->extents)))
  {
    return set(r);
  }

  // We completely subsumes r
  if (td->count == 1 && rd->count == 1 && td->extents.subsumes(rd->extents)) return *this;

  // Last optimization can be append
  const Box* sdlast = td->rects + td->count - 1;
  const Box* rdfirst = rd->rects;

  Box ext(
    fog_min(td->extents.x1(), rd->extents.x1()),
    fog_min(td->extents.y1(), rd->extents.y1()),
    fog_max(td->extents.x2(), rd->extents.x2()),
    fog_max(td->extents.y2(), rd->extents.y2())
  );

  if ( sdlast->y2() <= rdfirst->y1() ||
      (sdlast->y1() == rdfirst->y1() &&
       sdlast->y2() == rdfirst->y2() &&
       sdlast->x2() <= rdfirst->x1() ))
    _appendPrivate(this, rd->rects, rd->count, &ext);
  else
    _unitePrivate(this, td->rects, td->count, rd->rects, rd->count, true, &ext);

  return *this;
}

Region& Region::unite(const Rect& r)
{
  return unite(Box(r));
}

Region& Region::unite(const Box& r)
{
  Data* td = _d;

  if (!r.isValid()) return *this;

  // We are empty or 'r' completely subsumes us
  if (td->count == 0 || r.subsumes(td->extents)) 
  {
    return set(r);
  }

  // We completely subsumes src
  if (td->count == 1 && td->extents.subsumes(r)) return *this;

  Box ext(
    fog_min(td->extents.x1(), r.x1()),
    fog_min(td->extents.y1(), r.y1()),
    fog_max(td->extents.x2(), r.x2()),
    fog_max(td->extents.y2(), r.y2()));

  // Last optimization can be append

  Box* sdlast = td->rects + td->count-1;
  if ( sdlast->y2() <= r.y1() ||
      (sdlast->y1() == r.y1() &&
       sdlast->y2() == r.y2() &&
       sdlast->x2() <= r.x1() ))
    _appendPrivate(this, &r, 1, &ext);
  else
    _unitePrivate(this, td->rects, td->count, &r, 1, true, &ext);

  return *this;
}

Region& Region::intersect(const Region& r)
{
  Data* td = _d;
  Data* rd = r._d;

  if (td == rd)
    return *this;
  else if (td->count == 0 || rd->count == 0 || !td->extents.overlaps(rd->extents)) 
    clear();
  else
    _intersectPrivate(this, td->rects, td->count, rd->rects, rd->count, true);

  return *this;
}

Region& Region::intersect(const Rect& r)
{
  return intersect(Box(r));
}

Region& Region::intersect(const Box& r)
{
  Data* td = _d;

  if (td->count == 0 || !r.isValid() || !td->extents.overlaps(r)) 
    clear();
  else
    _intersectPrivate(this, td->rects, td->count, &r, 1, true);
  return *this;
}

Region& Region::eor(const Region& r)
{
  eor(*this, *this, r);
  return *this;
}

Region& Region::eor(const Rect& r)
{
  TemporaryRegion<1> reg(r);
  return eor(reg);
}

Region& Region::eor(const Box& r)
{
  TemporaryRegion<1> reg(r);
  return eor(reg);
}

Region& Region::subtract(const Region& r)
{
  Data* td = _d;
  Data* rd = r._d;

  if (td == rd) 
    clear();
  else if (td->count == 0 || rd->count == 0 || !td->extents.overlaps(rd->extents)) 
    ;
  else
    _subtractPrivate(this, td->rects, td->count, rd->rects, rd->count, true);
  return *this;
}

Region& Region::subtract(const Rect& r)
{
  return subtract(Box(r));
}

Region& Region::subtract(const Box& r)
{
  Data* td = _d;

  if (td->count == 0 || !r.isValid() || !td->extents.overlaps(r))
    clear();
  else
    _subtractPrivate(this, td->rects, td->count, &r, 1, true);

  return *this;
}

Region& Region::op(const Region& r, uint _op)
{
  switch (_op)
  {
    case OpCopy:
      return set(r);
    case OpUnite:
      return unite(r);
    case OpIntersect:
      return intersect(r);
    case OpEor:
      return eor(r);
    case OpSubtract:
      return subtract(r);
    default:
      FOG_ASSERT_NOT_REACHED();
      return *this;
  }
}

Region& Region::op(const Rect& r, uint _op)
{
  return op(Box(r), _op);
}

Region& Region::op(const Box& r, uint _op)
{
  switch (_op)
  {
    case OpCopy:
      return set(r);
    case OpUnite:
      return unite(r);
    case OpIntersect:
      return intersect(r);
    case OpEor:
      return eor(r);
    case OpSubtract:
      return subtract(r);
    default:
      FOG_ASSERT_NOT_REACHED();
      return *this;
  }
}

Region& Region::translate(const Point& pt)
{
  translate(*this, *this, pt);
  return *this;
}

Region& Region::shrink(const Point& pt)
{
  shrink(*this, *this, pt);
  return *this;
}

Region& Region::frame(const Point& pt)
{
  frame(*this, *this, pt);
  return *this;
}

bool Region::eq(const Region& other) const
{
  Data* d1 = _d;
  Data* d2 = other._d;

  if (d1 == d2) return true;
  if (d1->count != d2->count) return false;

  sysuint_t i;
  sysuint_t count = d1->count;

  for (i = 0; i != count; i++)
  {
    if (d1->rects[i] != d2->rects[i]) return false;
  }

  return true;
}

void Region::set(Region& dest, const Region& src)
{
  dest = src;
}

void Region::unite(Region& dest, const Region& src1, const Region& src2)
{
  Data* destd = dest._d;
  Data* src1d = src1._d;
  Data* src2d = src2._d;

  // Trivial operations
  if (src1d->count == 0) { dest.set(src2); return; }
  if (src2d->count == 0) { dest.set(src1); return; }
  if (src1d == src2d   ) { dest.set(src1); return; }

  const Box* src1first = src1d->rects;
  const Box* src2first = src2d->rects;
  const Box* src1last = src1first + src1d->count - 1;
  const Box* src2last = src2first + src2d->count - 1;

  Box ext(fog_min(src1d->extents.x1(), src2d->extents.x1()),
          fog_min(src1d->extents.y1(), src2d->extents.y1()),
          fog_max(src1d->extents.x2(), src2d->extents.x2()),
          fog_max(src1d->extents.y2(), src2d->extents.y2()));

  if (src1last->y2() <= src2first->y1()) 
  {
    dest.setDeep(src1);
    _appendPrivate(&dest, src2first, src2d->count, &ext);
  }
  else if (src2last->y2() <= src1first->y1()) 
  {
    dest.setDeep(src2);
    _appendPrivate(&dest, src1first, src1d->count, &ext);
  }
  else
  {
    _unitePrivate(&dest, src1first, src1d->count, src2first, src2d->count, destd == src1d || destd == src2d, &ext);
  }
}

void Region::intersect(Region& dest, const Region& src1, const Region& src2)
{
  Data* destd = dest._d;
  Data* src1d = src1._d;
  Data* src2d = src2._d;

  // trivial rejects
  if (FOG_UNLIKELY(src1d == src2d))
    dest.set(src1);
  else if (FOG_UNLIKELY(src1d->count == 0 || src2d->count == 0 || !src1d->extents.overlaps(src2d->extents))) 
    dest.clear();
  else
    _intersectPrivate(&dest, src1d->rects, src1d->count, src2d->rects, src2d->count, destd == src1d || destd == src2d);
}

void Region::eor(Region& dest, const Region& src1, const Region& src2)
{
  TemporaryRegion<Fog_Region_Local> r1;
  TemporaryRegion<Fog_Region_Local> r2;

  subtract(r1, src1, src2);
  subtract(r2, src2, src1);
  unite(dest, r1, r2);
}

void Region::subtract(Region& dest, const Region& src1, const Region& src2)
{
  Data* destd = dest._d;
  Data* src1d = src1._d;
  Data* src2d = src2._d;

  // Trivial reject
  if (src1d == src2d || src1d->count == 0) 
    dest.clear();
  else if (src2d->count == 0 || !src1d->extents.overlaps(src2d->extents)) 
    dest.set(src1);
  else
    _subtractPrivate(&dest, src1d->rects, src1d->count, src2d->rects, src2d->count, destd == src1d || destd == src2d);
}

void Region::op(Region& dest, const Region& src1, const Region& src2, uint _op)
{
  switch (_op)
  {
    case OpCopy:
      return set(dest, src1);
    case OpUnite:
      return unite(dest, src1, src2);
    case OpIntersect:
      return intersect(dest, src1, src2);
    case OpEor:
      return eor(dest, src1, src2);
    case OpSubtract:
      return subtract(dest, src1, src2);
    default:
      FOG_ASSERT_NOT_REACHED();
  }
}

void Region::translate(Region& dest, const Region& src, const Point& pt)
{
  int x = pt.x();
  int y = pt.y();

  if (x == 0 && y == 0) { dest = src; return; }

  Data* dest_d = dest._d;
  Data* src_d = src._d;

  sysuint_t i;
  Box* dest_r;
  Box* src_r;

  if (src_d->count == 0) 
  {
    dest.clear();
  }
  else if (dest_d == src_d && dest_d->refCount.get() == 1)
  {
    dest_d->extents.translate(x, y);
    dest_r = dest_d->rects;
    for (i = dest_d->count; i; i--, dest_r++) dest_r->translate(x, y);
  }
  else
  {
    dest.prepare(src_d->count);
    dest_d = dest._d; 

    dest_d->extents.set(src_d->extents.x1() + x, src_d->extents.y1() + y,
                        src_d->extents.x2() + x, src_d->extents.y2() + y);
    dest_d->count = src_d->count;

    dest_r = dest_d->rects;
    src_r = src_d->rects;
    for (i = dest_d->count; i; i--, dest_r++) 
    {
      dest_r->set(
        src_r->x1() + x, src_r->y1() + y,
        src_r->x2() + x, src_r->y2() + y);
    }
  }
}

void Region::shrink(Region& dest, const Region& src, const Point& pt)
{
  int x = pt.x();
  int y = pt.y();

  if (x == 0 && y == 0) return;
  dest.setDeep(src);

  TemporaryRegion<Fog_Region_Local> s;
  TemporaryRegion<Fog_Region_Local> t;
  bool grow;

  if (x) { if ((grow = (x < 0))) { x = -x; } _compress(dest, s, t, uint(x) << 1, true , grow); }
  if (y) { if ((grow = (y < 0))) { y = -y; } _compress(dest, s, t, uint(y) << 1, false, grow); }

  dest.translate(x, y);
}

void Region::frame(Region& dest, const Region& src, const Point& pt)
{
  // In cases that dest == src, we need to backup src
  TemporaryRegion<Fog_Region_Local> r1;
  TemporaryRegion<Fog_Region_Local> r2;

  translate(r2, src, Point(-pt.x(),       0));
  translate(r1, src, Point( pt.x(),       0)); r2.intersect(r1);
  translate(r1, src, Point(      0, -pt.y())); r2.intersect(r1);
  translate(r1, src, Point(      0,  pt.y())); r2.intersect(r1);

  subtract(dest, src, r2);
}

#if defined(FOG_OS_WINDOWS)
static inline void BoxToRECT(RECT* dest, const Box* src)
{
  dest->left   = src->x1();
  dest->top    = src->y1();
  dest->right  = src->x2();
  dest->bottom = src->y2();
}

static inline void RECTToBox(Box* dest, const RECT* src)
{
  dest->_x1 = src->left;
  dest->_y1 = src->top;
  dest->_x2 = src->right;
  dest->_y2 = src->bottom;
}

HRGN Region::toHRGN() const
{
  Data* d = _d;
  sysuint_t i;
  sysuint_t count = d->count;

  Fog::MemoryBuffer<4096> mem;
  RGNDATAHEADER *hdr = (RGNDATAHEADER *)mem.alloc(sizeof(RGNDATAHEADER) + count * sizeof(RECT));
  if (!hdr) return (HRGN)NULLREGION;

  hdr->dwSize = sizeof(RGNDATAHEADER);
  hdr->iType = RDH_RECTANGLES;
  hdr->nCount = count;
  hdr->nRgnSize = count * sizeof(RECT);
  BoxToRECT(&hdr->rcBound, &d->extents);

  RECT* r = (RECT *)((uint8_t *)hdr + sizeof(RGNDATAHEADER));
  for (i = 0; i < count; i++, r++)
  {
    BoxToRECT(r, &d->rects[i]);
  }
  return ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT)*count), (RGNDATA *)hdr);
}

bool Region::fromHRGN(HRGN hrgn)
{
  clear();

  if (hrgn == NULL) return false;
  if (hrgn == (HRGN)NULLREGION) return true;

  DWORD size = GetRegionData(hrgn, 0, NULL);
  if (size == 0) return false;

  Fog::MemoryBuffer<4096> mem;
  RGNDATAHEADER *hdr = (RGNDATAHEADER *)mem.alloc(size);
  if (hdr) return false;

  if (!GetRegionData(hrgn, size, (RGNDATA*)hdr)) return false;

  sysuint_t i;
  sysuint_t count = hdr->nCount;
  RECT* r = (RECT *)((uint8_t *)hdr + sizeof(RGNDATAHEADER));

  // TODO: Rects should be already YX sorted, but I'm not sure
  for (i = 0; i != count; i++, r++)
  {
    unite(Fog::Box(r->left, r->top, r->right, r->bottom));
  }

  return true;
}
#endif // FOG_OS_WINDOWS

// [Fog::Region::Data]

Region::Data* Region::Data::ref()
{
  return REF_INLINE();
}

void Region::Data::deref()
{
  DEREF_INLINE();
}

static Region::Data* _reallocRegion(Region::Data* d, sysuint_t capacity)
{
  if (d->flags & Region::Data::IsDynamic)
  {
    d = (Region::Data*)Fog::Memory::xrealloc(d, Region::Data::sizeFor(capacity));
    d->capacity = capacity;
    return d;
  }
  else
  {
    return Region::Data::create(capacity, &d->extents, d->rects, d->count);
  }
}

Region::Data* Region::Data::adopt(void* address, sysuint_t capacity)
{
  Data* d = (Data*)address;
  
  d->refCount.init(1);
  d->flags = IsStrong;
  d->capacity = capacity;
  d->count = 0;
  d->extents.clear();

  return d;
}

Region::Data* Region::Data::adopt(void* address, sysuint_t capacity, const Box& r)
{
  if (!r.isValid() || capacity == 0) return adopt(address, capacity);

  Data* d = (Data*)address;
  
  d->refCount.init(1);
  d->flags = IsStrong;
  d->capacity = capacity;
  if (r.isValid())
  {
    d->count = 1;
    d->extents = r;
    d->rects[0] = r;
  }
  else
  {
    d->count = 0;
    d->extents.clear();
    d->rects[0].clear();
  }

  return d;
}

Region::Data* Region::Data::adopt(void* address, sysuint_t capacity, const Box* extents, const Box* rects, sysuint_t count)
{
  if (capacity < count) create(count, extents, rects, count);

  Data* d = (Data*)address;
  
  d->refCount.init(1);
  d->flags = IsStrong;
  d->capacity = capacity;
  d->count = count;
  
  if (extents)
  {
    d->extents = *extents;
    _copyRects(d->rects, rects, count);
  }
  else
  {
    _copyRectsExtents(d->rects, rects, count, &d->extents);
  }

  return d;
}

Region::Data* Region::Data::create(sysuint_t capacity, uint allocPolicy)
{
  if (FOG_UNLIKELY(capacity == 0)) return Region::sharedNull.instancep()->REF_ALWAYS();

  sysuint_t dsize = sizeFor(capacity);
  Data* d = (Data*)Fog::Memory::alloc(dsize);

  if (FOG_LIKELY(d != 0))
  {
    d->refCount.init(1);
    d->flags = IsDynamic | IsSharable;
    d->capacity = capacity;
    d->count = 0;
  }
  else if (allocPolicy == Fog::AllocCantFail)
  {
    fog_out_of_memory_fatal_format(
      "Fog::Region::Data", "create",
      "Couldn't allocate %lu bytes of memory for region data", (ulong)dsize);
  }

  return d;
}

Region::Data* Region::Data::create(sysuint_t capacity, const Box* extents, const Box* rects, sysuint_t count)
{
  if (FOG_UNLIKELY(capacity < count)) capacity = count;

  Data* d = create(capacity, Fog::AllocCantFail);

  if (FOG_LIKELY(count))
  {
    d->count = count;
    if (extents)
    {
      d->extents = *extents;
      _copyRects(d->rects, rects, count);
    }
    else
    {
      _copyRectsExtents(d->rects, rects, count, &d->extents);
    }
  }

  return d;
}

Region::Data* Region::Data::copy(const Data* other, uint allocPolicy)
{
  if (!other->count) return Region::sharedNull.instancep()->REF_ALWAYS();

  Data* d = create(other->count, allocPolicy);

  if (d)
  {
    d->count = other->count;
    d->extents = other->extents;
    _copyRects(d->rects, other->rects, other->count);
  }

  return d;
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_region_init(void)
{
  Fog::Region::sharedNull.init();
  Fog::Region::Data* d = Fog::Region::sharedNull.instancep();

  d->refCount.init(1);
  d->flags = Fog::Region::Data::IsSharable | Fog::Region::Data::IsNull;
  d->extents.clear();
  d->rects[0].clear();

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_region_shutdown(void)
{
  Fog::Region::sharedNull.instancep()->refCount.dec();
  Fog::Region::sharedNull.destroy();
}
