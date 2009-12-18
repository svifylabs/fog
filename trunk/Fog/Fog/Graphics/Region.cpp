// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Assert.h>
#include <Fog/Core/Constants.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/Math.h>
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

// ============================================================================
// [Fog::Region]
// ============================================================================

#define Fog_Region_Local 256

// ============================================================================
// [Fog::Region - Statics]
// ============================================================================

static FOG_INLINE void _copyRects(Box* dest, const Box* src, sysuint_t length)
{
  for (sysuint_t i = length; i; i--, dest++, src++) *dest = *src;
}

static void _copyRectsExtents(Box* dest, const Box* src, sysuint_t length, Box* extents)
{
  int extentsX1 = src[0].getX1();
  int extentsY1 = src[0].getY1();
  int extentsX2 = src[length-1].getX2();
  int extentsY2 = src[length-1].getY2();

  for (sysuint_t i = length; i; i--, dest++, src++) 
  {
    if (extentsX1 > src->getX1()) extentsX1 = src->getX1();
    if (extentsX2 < src->getX2()) extentsX2 = src->getX2();
    *dest = *src;
  }

  extents->set(extentsX1, extentsY1, extentsX2, extentsY2);
}

// -----------------------------------------------------------------------
// Utility procedure _compress:
//
// Replace r by the region r', where
//   p in r' iff (Quantifer m <= dx) (p + m in r), and
//   Quantifier is Exists if grow is true, For all if grow is false, and
//   (x,y) + m = (x+m,y) if xdir is true; (x,y+m) if xdir is false.
//
// Thus, if xdir is true and grow is false, r is replaced by the region
// of all points p such that p and the next dx points on the same
// horizontal scan line are all in r.  We do this using by noting
// that p is the head of a run of length 2^i + k iff p is the head
// of a run of length 2^i and p+2^i is the head of a run of length
// k. Thus, the loop invariant: s contains the region corresponding
// to the runs of length shift. r contains the region corresponding
// to the runs of length 1 + dxo & (shift-1), where dxo is the original
// value of dx.  dx = dxo & ~(shift-1).  As parameters, s and t are
// scratch regions, so that we don't have to allocate them on every
// call.
// -----------------------------------------------------------------------
static err_t _compress(Region& r, Region& s, Region& t, uint dx, bool xdir, register bool grow)
{
  err_t err;
  uint shift = 1;

  if ((err = s.setDeep(r))) return err;

  while (dx)
  {
    if (dx & shift)
    {
      if (xdir)
        err = Region::translate(r, r, Point(-(int)shift, 0)); 
      else
        err = Region::translate(r, r, Point(0, -(int)shift));
      if (err) return err;

      if (grow)
        err = r.unite(s);
      else
        err = r.intersect(s);
      if (err) return err;

      dx -= shift;
      if (!dx) break;
    }

    if ((err = t.setDeep(s))) return err;
    if (xdir)
      err = Region::translate(s, s, Point(-(int)shift, 0));
    else 
      err = Region::translate(s, s, Point(0, -(int)shift));
    if (err) return err;

    if (grow)
      err = s.unite(t); 
    else
      err = s.intersect(t);
    if (err) return err;

    shift <<= 1;
  }

  return ERR_OK;
}

// -----------------------------------------------------------------------
// coalesce
//   Attempt to merge the boxes in the current band with those in the
//   previous one. Used only by miRegionOp.
//
// Results:
//   dest pointer (can be smaller if coalesced)
//
// Side Effects:
//   If coalescing takes place:
//   - rectangles in the previous band will have their y2 fields
//     altered.
//   - Count may be decreased.
// -----------------------------------------------------------------------

static Box* _coalesceHelper(Box* dest_ptr, Box** prev_start_, Box** cur_start_)
{
  Box* prev_start = *prev_start_;
  Box* cur_start  = *cur_start_;

  sysuint_t c1 = (sysuint_t)( cur_start - prev_start );
  sysuint_t c2 = (sysuint_t)( dest_ptr - cur_start );

  if (c1 == c2 && prev_start->getY2() == cur_start->getY1())
  {
    // need to scan their x coords, if prev_start and cur_start will has
    // same x coordinates, we can coalesce it
    sysuint_t i;
    for (i = 0; i != c1; i++)
    {
      if (prev_start[i].getX1() != cur_start[i].getX1() || prev_start[i].getX2() != cur_start[i].getX2())
      {
        // coalesce isn't possible
        *prev_start_ = cur_start;
        return dest_ptr;
      }
    }

    // coalesce
    int bot = cur_start->getY2();
    for (i = 0; i != c1; i++) prev_start[i].setY2(bot);
    return cur_start;
  }

  *prev_start_ = cur_start;
  return dest_ptr;
}

// Inline here produces better results
static FOG_INLINE Box* _coalesce(Box* dest_ptr, Box** prev_start_, Box** cur_start_)
{
  if (*prev_start_ != *cur_start_)
  {
    return _coalesceHelper(dest_ptr, prev_start_, cur_start_);
  }
  else
    return dest_ptr;
}

// Forward declarations for rectangle processor.
static err_t _unitePrivate(Region* dest, const Box* src1, sysuint_t count1, const Box* src2, sysuint_t count2, bool memOverlap, const Box* new_extents);
static err_t _intersectPrivate(Region* dest, const Box* src1, sysuint_t count1, const Box* src2, sysuint_t count2, bool memOverlap);
static err_t _subtractPrivate(Region* dest, const Box* src1, sysuint_t count1, const Box* src2, sysuint_t count2, bool memOverlap);
static err_t _appendPrivate(Region* dest, const Box* src, sysuint_t length, const Box* new_extents);

// Fog::Region::Data statics
static Region::Data* _reallocRegion(Region::Data* d, sysuint_t capacity);

// ============================================================================
// [Fog::Region - Union, Subtraction, Intersection and Append Implementation]
// ============================================================================

static err_t _unitePrivate(Region* dest, const Box* src1, sysuint_t count1, const Box* src2, sysuint_t count2, bool memOverlap, const Box* new_extents)
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

  sysuint_t minRectsNeeded = (count1 + count2) << 1;

  // Trivial reject.
  if (src1 == src1End) return dest->set(*src2);
  if (src2 == src2End) return dest->set(*src1);

  // New region data in case that it needs it.
  Region::Data* newd = NULL;
  sysuint_t length;

  // Local buffer that can be used instead of malloc in mosst calls
  // can be increased to higher values, but I think that 256 is ok.
  Box staticBuffer[256];

  if (memOverlap)
  {
    // Need to allocate new block.
    if (minRectsNeeded < FOG_ARRAY_SIZE(staticBuffer))
    {
      destCur = staticBuffer;
    }
    else
    {
      newd = Region::Data::create(minRectsNeeded);
      if (!newd) return ERR_RT_OUT_OF_MEMORY;
      destCur = newd->rects;
    }
  }
  else
  {
    // Can use dest.
    err_t err;
    if ((err = dest->prepare(minRectsNeeded))) { dest->clear(); return err; }
    destCur = dest->_d->rects;
  }

  destBegin = destCur;
  
  // Initialize ybot and ytop.
  // In the upcoming loop, ybot and ytop serve different functions depending
  // on whether the band being handled is an overlapping or non-overlapping
  // band.
  // In the case of a non-overlapping band (only one of the regions
  // has points in the band), ybot is the bottom of the most recent
  // intersection and thus clips the top of the rectangles in that band.
  // ytop is the top of the next intersection between the two regions and
  // serves to clip the bottom of the rectangles in the current band.
  // For an overlapping band (where the two regions intersect), ytop clips
  // the top of the rectangles of both regions and ybot clips the bottoms.

  if (src1->getY1() < src2->getY1())
    ybot = src1->getY1();
  else
    ybot = src2->getY1();

  // prevBand serves to mark the start of the previous band so rectangles
  // can be coalesced into larger rectangles. qv. coalesce, above.
  // In the beginning, there is no previous band, so prevBand == curBand
  // (curBand is set later on, of course, but the first band will always
  // start at index 0). prevBand and curBand must be indices because of
  // the possible expansion, and resultant moving, of the new region's
  // array of rectangles.
  prevBand = destCur;

  do {
    curBand = destCur;
    // This algorithm proceeds one source-band (as opposed to a
    // destination band, which is determined by where the two regions
    // intersect) at a time. src1BandEnd and src2BandEnd serve to mark the
    // rectangle after the last one in the current band for their
    // respective regions.
    src1BandEnd = src1;
    src2BandEnd = src2;

    while ((++src1BandEnd != src1End) && (src1BandEnd->getY1() == src1->getY1())) ;
    while ((++src2BandEnd != src2End) && (src2BandEnd->getY1() == src2->getY1())) ;

    // First handle the band that doesn't intersect, if any.
    //
    // Note that attention is restricted to one band in the
    // non-intersecting region at once, so if a region has n
    // bands between the current position and the next place it overlaps
    // the other, this entire loop will be passed through n times.
    if (src1->getY1() < src2->getY1())
    {
      top = Math::max(src1->getY1(), ybot);
      bot = Math::min(src1->getY2(), src2->getY1());

      if (top != bot)
      {
        const Box* ovrlp = src1;
        while (ovrlp != src1BandEnd) { (*destCur++).set(ovrlp->getX1(), top, ovrlp->getX2(), bot); ovrlp++; }

        destCur = _coalesce(destCur, &prevBand, &curBand);
      }
    }
    else if (src2->getY1() < src1->getY1())
    {
      top = Math::max(src2->getY1(), ybot);
      bot = Math::min(src2->getY2(), src1->getY1());

      if (top != bot)
      {
        const Box* ovrlp = src2;
        while (ovrlp != src2BandEnd) { (*destCur++).set(ovrlp->getX1(), top, ovrlp->getX2(), bot); ovrlp++; }

        destCur = _coalesce(destCur, &prevBand, &curBand);
      }
    }

    // Now see if we've hit an intersecting band. The two bands only
    // intersect if ybot > ytop.
    ytop = Math::max(src1->getY1(), src2->getY1());
    ybot = Math::min(src1->getY2(), src2->getY2());
    if (ybot > ytop)
    {
      const Box* i1 = src1;
      const Box* i2 = src2;

      // Unite.
      #define MERGE_RECT(__x1__, __y1__, __x2__, __y2__) \
        if (destCur != destBegin &&                                 \
         (destCur[-1].getY1() == __y1__) &&                         \
         (destCur[-1].getY2() == __y2__) &&                         \
         (destCur[-1].getX2() >= __x1__))                           \
        {                                                           \
          if (destCur[-1].getX2() < __x2__) destCur[-1].x2 = __x2__;\
        }                                                           \
        else {                                                      \
          (*destCur++).set(__x1__, __y1__, __x2__, __y2__);         \
        }

      while ((i1 != src1BandEnd) && (i2 != src2BandEnd))
      {
        if (i1->getX1() < i2->getX1())
        {
          MERGE_RECT(i1->getX1(), ytop, i1->getX2(), ybot); i1++;
        }
        else
        {
          MERGE_RECT(i2->getX1(), ytop, i2->getX2(), ybot); i2++;
        }
      }

      if (i1 != src1BandEnd)
      {
        do {
          MERGE_RECT(i1->getX1(), ytop, i1->getX2(), ybot); i1++;
        } while (i1 != src1BandEnd);
      }
      else
      {
        while (i2 != src2BandEnd)
        {
          MERGE_RECT(i2->getX1(), ytop, i2->getX2(), ybot); i2++;
        }
      }

      #undef MERGE_RECT

      destCur = _coalesce(destCur, &prevBand, &curBand);
    }

    // If we've finished with a band (y2 == ybot) we skip forward
    // in the region to the next band.
    if (src1->getY2() == ybot) src1 = src1BandEnd;
    if (src2->getY2() == ybot) src2 = src2BandEnd;
  } while ((src1 != src1End) && (src2 != src2End));

  // Deal with whichever region still has rectangles left.
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

    int y0 = src->getY1();
    int y1 = Math::max(src->getY1(), ybot);

    // Append first band and coalesce.
    while (src != srcEnd && src->getY1() == y0) { (*destCur++).set(src->getX1(), y1, src->getX2(), src->getY2()); src++; }
    destCur = _coalesce(destCur, &prevBand, &curBand);

    // Append remaining rectangles, coalesce isn't needed.
    while (src != srcEnd) *destCur++ = *src++;
  }

  // Finished, we have complete intersected region in destCur.
  //
  // Its paranoid here, because unite_private is called only if there are
  // rectangles in regions.
  if ((length = (sysuint_t)(destCur - destBegin)) != 0)
  {
    if (memOverlap)
    {
      if (destBegin == staticBuffer)
      {
        err_t err;
        if ((err = dest->reserve(length))) { dest->clear(); return err; }
        _copyRects(dest->_d->rects, destBegin, length);
      }
      else
      {
        dest->_d->deref();
        dest->_d = newd;
      }
    }

    dest->_d->extents = *new_extents;
    dest->_d->length = length;
  }
  else
  {
    if (memOverlap && destBegin == staticBuffer) Memory::free(newd);
    dest->clear();
  }
  return ERR_OK;
}

static err_t _intersectPrivate(Region* dest, const Box* src1, sysuint_t count1, const Box* src2, sysuint_t count2, bool memOverlap)
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
    return dest->set(Box(Math::max(src1->getX1(), src2->getX1()), Math::max(src1->getY1(), src2->getY1()),
                         Math::min(src1->getX2(), src2->getX2()), Math::min(src1->getY2(), src2->getY2())));
  }

  sysuint_t minRectsNeeded = (count1 + count2) << 1;

  // Find first rectangle that can intersect.
  for (;;)
  {
    if (src1 == src1End || src2 == src2End) { dest->clear(); return ERR_OK; }

    if (src1->getY2() < src2->getY1()) { src1++; continue; }
    if (src2->getY2() < src1->getY1()) { src2++; continue; }

    break;
  }

  // New region data in case that it needs it.
  Region::Data* newd = NULL;
  sysuint_t length;

  // Local buffer that can be used instead of malloc in mosst calls
  // can be increased to higher values, but I think that 256 is ok.
  Box staticBuffer[256];

  if (memOverlap)
  {
    // Need to allocate new block.
    if (minRectsNeeded < FOG_ARRAY_SIZE(staticBuffer))
    {
      destCur = staticBuffer;
    }
    else
    {
      newd = Region::Data::create(minRectsNeeded);
      if (!newd) { dest->clear(); return ERR_RT_OUT_OF_MEMORY; }
      destCur = newd->rects;
    }
  }
  else
  {
    // Can use dest.
    err_t err;
    if ((err = dest->prepare(minRectsNeeded))) { dest->clear(); return err; }
    destCur = dest->_d->rects;
  }

  destBegin = destCur;

  // prevBand serves to mark the start of the previous band so rectangles
  // can be coalesced into larger rectangles. qv. miCoalesce, above.
  // In the beginning, there is no previous band, so prevBand == curBand
  // (curBand is set later on, of course, but the first band will always
  // start at index 0). prevBand and curBand must be indices because of
  // the possible expansion, and resultant moving, of the new region's
  // array of rectangles.
  prevBand = destCur;

  do {
    curBand = destCur;

    // This algorithm proceeds one source-band (as opposed to a destination
    // band, which is determined by where the two regions intersect) at a time.
    // src1BandEnd and src2BandEnd serve to mark the rectangle after the last 
    // one in the current band for their respective regions.
    src1BandEnd = src1;
    src2BandEnd = src2;

    while ((src1BandEnd != src1End) && (src1BandEnd->getY1() == src1->getY1())) src1BandEnd++;
    while ((src2BandEnd != src2End) && (src2BandEnd->getY1() == src2->getY1())) src2BandEnd++;

    // See if we've hit an intersecting band. The two bands only
    // intersect if ybot > ytop.
    ytop = Math::max(src1->getY1(), src2->getY1());
    ybot = Math::min(src1->getY2(), src2->getY2());
    if (ybot > ytop)
    {
      // Intersect.
      const Box* i1 = src1;
      const Box* i2 = src2;

      int x1;
      int x2;

      while ((i1 != src1BandEnd) && (i2 != src2BandEnd))
      {
        x1 = Math::max(i1->getX1(), i2->getX1());
        x2 = Math::min(i1->getX2(), i2->getX2());

        // If there's any overlap between the two rectangles, add that
        // overlap to the new region.
        // There's no need to check for subsumption because the only way
        // such a need could arise is if some region has two rectangles
        // right next to each other. Since that should never happen...
        if (x1 < x2)
        {
          // Append rectangle.
          destCur->set(x1, ytop, x2, ybot);
          destCur++;
        }

        // Need to advance the pointers. Shift the one that extends
        // to the right the least, since the other still has a chance to
        // overlap with that region's next rectangle, if you see what I mean.
        if (i1->getX2() < i2->getX2())
        {
          i1++;
        }
        else if (i2->getX2() < i1->getX2())
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

        // Update x1 and x2 extents.
        if (curBand[0].getX1() < extentsX1) extentsX1 = curBand[0].getX1();
        if (destCur[-1].getX2() > extentsX2) extentsX2 = destCur[-1].getX2();
      }
    }

    // If we've finished with a band (y2 == ybot) we skip forward
    // in the region to the next band.
    if (src1->getY2() == ybot) src1 = src1BandEnd;
    if (src2->getY2() == ybot) src2 = src2BandEnd;
  } while ((src1 != src1End) && (src2 != src2End));

  // Finished, we have complete intersected region in destCur.
  if ((length = sysuint_t(destCur - destBegin)) != 0)
  {
    if (memOverlap)
    {
      if (destBegin == staticBuffer)
      {
        err_t err;
        if ((err = dest->reserve(length))) { dest->clear(); return err; }
        _copyRects(dest->_d->rects, destBegin, length);
      }
      else
      {
        dest->_d->deref();
        dest->_d = newd;
      }
    }

    dest->_d->length = length;
    dest->_d->extents.set(extentsX1, destBegin[0].getY1(), extentsX2, destCur[-1].getY2());
  }
  else
  {
    if (memOverlap && destBegin == staticBuffer) Memory::free(newd);
    dest->clear();
  }
  return ERR_OK;
}

static err_t _subtractPrivate(Region* dest, const Box* src1, sysuint_t count1, const Box* src2, sysuint_t count2, bool memOverlap)
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

  sysuint_t minRectsNeeded = (count1 + count2) << 1;

  Region::Data* newd;
  sysuint_t length;

  if (memOverlap)
  {
    newd = Region::Data::create(minRectsNeeded);
    if (!newd) { dest->clear(); return ERR_RT_OUT_OF_MEMORY; }
  }
  else
  {
    err_t err = dest->prepare(minRectsNeeded);
    if (err) { dest->clear(); return ERR_RT_OUT_OF_MEMORY; }
    newd = dest->_d;
  }

  destCur = newd->rects;
  destEnd = destCur + newd->capacity;
  destBegin = destCur;

  // Initialize ybot and ytop.
  // In the upcoming loop, ybot and ytop serve different functions depending
  // on whether the band being handled is an overlapping or non-overlapping
  // band.
  // In the case of a non-overlapping band (only one of the regions
  // has points in the band), ybot is the bottom of the most recent
  // intersection and thus clips the top of the rectangles in that band.
  // ytop is the top of the next intersection between the two regions and
  // serves to clip the bottom of the rectangles in the current band.
  // For an overlapping band (where the two regions intersect), ytop clips
  // the top of the rectangles of both regions and ybot clips the bottoms.

  if (src1->getY1() < src2->getY1())
    ybot = src1->getY1();
  else
    ybot = src2->getY1();

  // prevBand serves to mark the start of the previous band so rectangles
  // can be coalesced into larger rectangles. qv. coalesce, above.
  // In the beginning, there is no previous band, so prevBand == curBand
  // (curBand is set later on, of course, but the first band will always
  // start at index 0). prevBand and curBand must be indices because of
  // the possible expansion, and resultant moving, of the new region's
  // array of rectangles.
  prevBand = destCur;

  // macro for merging rectangles, it's designed to simplify loop, because
  // result of subtraction can be more than 'minRectsNeeded' we need to
  // detect end of the destination buffer.
  #define ADD_RECT(__x1__, __y1__, __x2__, __y2__) \
    if (FOG_UNLIKELY(destCur == destEnd)) {                       \
      sysuint_t length = newd->capacity;                          \
      sysuint_t prevBandIndex = (sysuint_t)(prevBand - destBegin);\
      sysuint_t curBandIndex = (sysuint_t)(curBand - destBegin);  \
                                                                  \
      Region::Data* _d = _reallocRegion(newd, length << 1);       \
      if (!_d) goto outOfMemory;                                  \
                                                                  \
      newd = _d;                                                  \
      destBegin = newd->rects;                                    \
      destCur = destBegin + length;                               \
      destEnd = destBegin + newd->capacity;                       \
                                                                  \
      prevBand = destBegin + prevBandIndex;                       \
      curBand = destBegin + curBandIndex;                         \
    }                                                             \
    destCur->set(__x1__, __y1__, __x2__, __y2__);                 \
    destCur++;                                                    \
                                                                  \
    if (FOG_UNLIKELY(extentsX1 > __x1__)) extentsX1 = __x1__;     \
    if (FOG_UNLIKELY(extentsX2 < __x2__)) extentsX2 = __x2__

  do {
    curBand = destCur;
    // This algorithm proceeds one source-band (as opposed to a
    // destination band, which is determined by where the two regions
    // intersect) at a time. src1BandEnd and src2BandEnd serve to mark the
    // rectangle after the last one in the current band for their
    // respective regions.
    src1BandEnd = src1;
    src2BandEnd = src2;

    while ((src1BandEnd != src1End) && (src1BandEnd->getY1() == src1->getY1())) src1BandEnd++;
    while ((src2BandEnd != src2End) && (src2BandEnd->getY1() == src2->getY1())) src2BandEnd++;

    // First handle the band that doesn't intersect, if any.
    //
    // Note that attention is restricted to one band in the
    // non-intersecting region at once, so if a region has n
    // bands between the current position and the next place it overlaps
    // the other, this entire loop will be passed through n times.
    if (src1->getY1() < src2->getY1())
    {
      // non overlap (src1) - merge it
      top = Math::max(src1->getY1(), ybot);
      bot = Math::min(src1->getY2(), src2->getY1());

      if (top != bot)
      {
        const Box* ovrlp = src1;
        while (ovrlp != src1BandEnd) { ADD_RECT(ovrlp->getX1(), top, ovrlp->getX2(), bot); ovrlp++; }

        destCur = _coalesce(destCur, &prevBand, &curBand);
      }
    }

    // Now see if we've hit an intersecting band. The two bands only
    // intersect if ybot > ytop
    ytop = Math::max(src1->getY1(), src2->getY1());
    ybot = Math::min(src1->getY2(), src2->getY2());
    if (ybot > ytop)
    {
      const Box* i1 = src1;
      const Box* i2 = src2;

      int x1 = i1->getX1();

      while ((i1 != src1BandEnd) && (i2 != src2BandEnd))
      {
        if (i2->getX2() <= x1)
        {
          // Subtrahend missed the boat: go to next subtrahend.
          i2++;
        }
        else if (i2->getX1() <= x1)
        {
          // Subtrahend preceeds minuend: nuke left edge of minuend.
          x1 = i2->getX2();
          if (x1 >= i1->getX2())
          {
            // Minuend completely covered: advance to next minuend and
            // reset left fence to edge of new minuend.
            if (++i1 != src1BandEnd) x1 = i1->getX1();
          }
          else
          {
            // Subtrahend now used up since it doesn't extend beyond
            // minuend.
            i2++;
          }
        }
        else if (i2->getX1() < i1->getX2())
        {
          // x1 part of subtrahend covers part of minuend: add uncovered
          // part of minuend to region and skip to next subtrahend.
          ADD_RECT(x1, ytop, i2->getX1(), ybot);

          x1 = i2->getX2();
          if (x1 >= i1->getX2())
          {
            // Minuend used up: advance to new...
            if (++i1 != src1BandEnd) x1 = i1->getX1();
          }
          else
          {
            // Subtrahend used up.
            i2++;
          }
        }
        else
        {
          // Minuend used up: add any remaining piece before advancing.
          if (i1->getX2() > x1) { ADD_RECT(x1, ytop, i1->getX2(), ybot); }
          if (++i1 != src1BandEnd) x1 = i1->getX1();
        }
      }

      // Add remaining minuend rectangles to region.
      while (i1 != src1BandEnd)
      {
        ADD_RECT(x1, ytop, i1->getX2(), ybot);
        if (++i1 != src1BandEnd) x1 = i1->getX1();
      }

      destCur = _coalesce(destCur, &prevBand, &curBand);
    }

    // If we've finished with a band (y2 == ybot) we skip forward
    // in the region to the next band.
    if (src1->getY2() == ybot) src1 = src1BandEnd;
    if (src2->getY2() == ybot) src2 = src2BandEnd;
  } while ((src1 != src1End) && (src2 != src2End));

  // Deal with whichever src1 still has rectangles left.
  if (src1 != src1End)
  {
    const Box* src;
    const Box* srcEnd;

    curBand = destCur;
    src = src1; srcEnd = src1End;

    int y0 = src->getY1();
    int y1 = Math::max(src->getY1(), ybot);

    // Append first band and coalesce.
    while (src != srcEnd && src->getY1() == y0)
    {
      ADD_RECT(src->getX1(), y1, src->getX2(), src->getY2()); src++;
    }
    destCur = _coalesce(destCur, &prevBand, &curBand);

    // Append remaining rectangles, coalesce isn't needed.
    while (src != srcEnd) { ADD_RECT(src->getX1(), src->getY1(), src->getX2(), src->getY2()); src++; }
  }

  #undef ADD_RECT

  // Finished, we have complete intersected region in destCur.
  if ((length = sysuint_t(destCur - destBegin)) != 0)
  {
    if (memOverlap) dest->_d->deref();

    newd->extents.set(extentsX1, destBegin[0].getY1(), extentsX2, destCur[-1].getY2());
    newd->length = length;
    dest->_d = newd;
  }
  else
  {
    if (memOverlap) Memory::free(newd);
    dest->clear();
  }
  return ERR_OK;

outOfMemory:
  if (memOverlap) Memory::free(newd);
  dest->clear();
  return ERR_RT_OUT_OF_MEMORY;
}

static err_t _appendPrivate(Region* dest, const Box* src, sysuint_t length, const Box* new_extents)
{
  err_t err;

  // Its guaranted that we CAN append region and its also guaranted that
  // destination region has minimal 1 rectangle, so [-1] index from end
  // must be valid.
  if ((err = dest->reserve(dest->getLength() + length))) return err;

  Box* destBegin = dest->_d->rects;
  Box* destCur = destBegin + dest->_d->length;

  const Box* srcEnd = src + length;

  if (src->getY1() == destCur[-1].getY1())
  {
    // Here is an interesting case that we will append an band that exist
    // in destination
    //
    // XXXXXXXXX
    // XXXXX<-Here

    // Merge first? (we can increase an existing band).
    if (destCur[-1].getX2() == src->getX1()) { destCur[-1].setX2(src->getX2()); src++; }

    // Append all other bands.
    while (src != srcEnd && destCur[-1].getY1() == src->getY1()) *destCur++ = *src++;

    // Coalesce.
    {
      Box* destBand1Begin;
      Box* destBand2Begin = destCur-1;

      int y = destBand2Begin->getY1();

      while (destBand2Begin != destBegin && destBand2Begin[-1].getY1() == y) destBand2Begin--;
      if (destBand2Begin != destBegin && destBand2Begin[-1].getY2() == destBand2Begin[0].getY1())
      {
        destBand1Begin = destBand2Begin - 1;
        while (destBand1Begin != destBegin && destBand1Begin[-1].getY2() == y) destBand1Begin--;
        
        sysuint_t index1 = (sysuint_t)(destBand2Begin - destBand1Begin);
        sysuint_t index2 = (sysuint_t)(destCur - destBand2Begin);

        if (index1 == index2)
        {
          // It's chance for coalesce, need to compare bands.
          sysuint_t i;
          for (i = 0; i != index1; i++)
          {
            if (destBand1Begin[i].getX1() == destBand2Begin[i].getX1() &&
                destBand1Begin[i].getX2() == destBand2Begin[i].getX2()) continue;
            goto __noCoalesce;
          }

          // Coalesce now.
          y = destBand2Begin[0].getY2();
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

  if (src->getY1() == destCur[-1].getY2())
  {
    // Coalesce, need to find previous band in dest.
    Box* destBandBegin = destCur-1;
    while (destBandBegin != destBegin && destBandBegin->getY1() == destCur[-1].getY1()) destBandBegin--;

    const Box* srcBandBegin = src;
    const Box* srcBandEnd = src;
    while (srcBandEnd != srcEnd && srcBandEnd->getY1() == src->getY1()) srcBandEnd++;

    // Now we have:
    // - source:
    //       srcBandBegin
    //       srcBandEnd
    // - dest:
    //       destBandBegin
    //       destBandEnd == destCur !
    if (srcBandEnd - srcBandBegin == destCur - destBandBegin)
    {
      // Probability for coasesce...
      sysuint_t i, len = (sysuint_t)(srcBandEnd - srcBandBegin);
      for (i = 0; i != len; i++)
      {
        if (srcBandBegin[i].getX1() != destBandBegin[i].getX1() ||
            srcBandBegin[i].getX2() != destBandBegin[i].getX2())
        {
          goto __fastAppend;
        }
      }

      // Coalesce success.
      int y2 = srcBandBegin->getY2();
      for (i = 0; i != len; i++) destBandBegin[i].setY2(y2);
    }
  }

__fastAppend:
  // Fastest case, fast append, no coasesce...
  while (src != srcEnd) *destCur++ = *src++;

__end:
  dest->_d->length = (sysuint_t)(destCur - destBegin);
  dest->_d->extents = *new_extents;

  return ERR_OK;
}

// ============================================================================
// [Fog::Region]
// ============================================================================

Static<Region::Data> Region::sharedNull;

Region::Region() :
  _d(sharedNull.instancep()->refAlways())
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
  _d(other._d->ref())
{
}

Region::~Region()
{
  _d->derefInline();
}

err_t Region::_detach()
{
  Data* d = _d;

  if (d->refCount.get() > 1)
  {
    if (d->length > 0)
    {
      d = Data::create(d->length, &d->extents, d->rects, d->length);
      if (!d) return ERR_RT_OUT_OF_MEMORY;
    }
    else
    {
      d = Data::create(d->length);
      if (!d) return ERR_RT_OUT_OF_MEMORY;
      d->extents.clear();
    }
    atomicPtrXchg(&_d, d)->deref();
  }

  return ERR_OK;
}

void Region::free()
{
  atomicPtrXchg(&_d, sharedNull.instancep()->refAlways())->derefInline();
}

err_t Region::setSharable(bool val)
{
  if (isSharable() != val)
  {
    err_t err = detach();
    if (err) return err;

    if (val)
      _d->flags |= Data::IsSharable;
    else
      _d->flags &= ~Data::IsSharable;
  }

  return ERR_OK;
}

err_t Region::setStrong(bool val)
{
  if (isSharable() != val)
  {
    err_t err = detach();
    if (err) return err;

    if (val)
      _d->flags |= Data::IsStrong;
    else
      _d->flags &= ~Data::IsStrong;
  }

  return ERR_OK;
}

err_t Region::reserve(sysuint_t to)
{
  Data* d = _d;

  if (d->refCount.get() > 1)
  {
__create:
    Data* newd = Data::create(to, &d->extents, d->rects, d->length);
    if (!newd) return ERR_RT_OUT_OF_MEMORY;
    atomicPtrXchg(&_d, newd)->derefInline();
  }
  else if (d->capacity < to)
  {
    if (!(d->flags & Data::IsDynamic)) goto __create;

    Data* newd = (Data *)Memory::realloc(d, Data::sizeFor(to));
    if (!newd) return ERR_RT_OUT_OF_MEMORY;
    newd->capacity = to;
    _d = newd;
  }

  return ERR_OK;
}

err_t Region::prepare(sysuint_t to)
{
  Data* d = _d;

  if (d->refCount.get() > 1)
  {
__create:
    Data* newd = Data::create(to);
    if (!newd) return ERR_RT_OUT_OF_MEMORY;

    atomicPtrXchg(&_d, newd)->derefInline();
  }
  else if (d->capacity < to)
  {
    if (!(d->flags & Data::IsDynamic)) goto __create;

    Data* newd = (Data *)Memory::realloc(d, Data::sizeFor(to));
    if (!newd) return ERR_RT_OUT_OF_MEMORY;

    newd->capacity = to;
    newd->length = 0;
    _d = newd;
  }
  else
  {
    d->length = 0;
  }

  return ERR_OK;
}

void Region::squeeze()
{
  Data* d = _d;
  
  if (d->flags & Data::IsDynamic)
  {
    sysuint_t length = d->length;
    if (d->capacity == length) return;

    if (d->refCount.get() > 1)
    {
      d = Data::create(length);
      if (!d) return;

      d->length = length;
      d->extents = _d->extents;
      _copyRects(d->rects, _d->rects, length);
      atomicPtrXchg(&_d, d)->deref();
    }
    else
    {
      d = (Data *)Memory::realloc(d, Data::sizeFor(length));
      if (!d) return;

      _d = d;
      d->capacity = length;
    }
  }
}

uint32_t Region::getType() const
{
  uint32_t t = _d->length;
  return t < 2 ? t : 2;
}

int Region::hitTest(const Point& pt) const
{
  Data* d = _d;

  sysuint_t i;
  sysuint_t length = d->length;

  if (!length) return REGION_HITTEST_OUT;

  // Check if point position is in extents, if not -> Out
  if (!(d->extents.contains(pt))) return REGION_HITTEST_OUT;
  if (length == 1) return REGION_HITTEST_IN;

  // Binary search for matching position
  const Box* base = d->rects;
  const Box* r;

  int x = pt.getX();
  int y = pt.getY();

  for (i = length; i != 0; i >>= 1)
  {
    r = base + (i >> 1);

    // Try match
    if (y >= r->getY1() && y < r->getY2())
    {
      if (x >= r->getX1())
      {
        if (x < r->getX2())
        {
          // Match
          return REGION_HITTEST_IN;
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
    else if (r->getY2() <= y)
    {
      // Move right
      base = r + 1;
      i--;
    }
    // else: Move left
  }
  return REGION_HITTEST_OUT;
}

int Region::hitTest(const Rect& r) const
{
  return hitTest(Box(r));
}

int Region::hitTest(const Box& r) const
{
  Data* d = _d;
  sysuint_t length = d->length;

  // This is (just) a useful optimization.
  if (!length || !d->extents.overlaps(r)) return REGION_HITTEST_OUT;

  const Box* cur = d->rects;
  const Box* end = cur + length;

  bool partIn = false;
  bool partOut = false;

  int x = r.getX1();
  int y = r.getY1();

  // Can stop when both PartOut and PartIn are true, or we reach cur->y2.
  for (; cur < end; cur++)
  {
    // Getting up to speed or skipping remainder of band.
    if (cur->getY2() <= y) continue;

    if (cur->getY1() > y)
    {
      // Missed part of rectangle above.
      partOut = true;
      if (partIn || (cur->getY1() >= r.getY2())) break;
      // X guaranteed to be == rect->x1.
      y = cur->getY1();
    }

    // Not far enough over yet.
    if (cur->getX2() <= x) continue;

    if (cur->getX1() > x)
    {
      // Missed part of rectangle to left.
      partOut = true;
      if (partIn) break;
    }

    if (cur->getX1() < r.getX2())
    {
      // Definitely overlap.
      partIn = true;
      if (partOut) break;
    }

    if (cur->getX2() >= r.getX2())
    {
      // Finished with this band.
      y = cur->getY2();
      if (y >= r.getY2()) break;
      // Reset x out to left again.
      x = r.getX1();
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

  return (partIn)
    ? ((y < r.getY2()) ? REGION_HITTEST_PART : REGION_HITTEST_IN)
    : REGION_HITTEST_OUT;
}

void Region::clear()
{
  Data* d = _d;
  if (d->length == 0) return;

  if (d->refCount.get() > 0)
  {
    atomicPtrXchg(&_d, sharedNull.instancep()->refAlways())->derefInline();
  }
  else
  {
    d->length = 0;
    d->extents.clear();
  }
}

err_t Region::set(const Region& r)
{
  Data* td = _d;
  Data* rd = r._d;
  if (td == rd) return ERR_OK;

  if ((td->flags & Data::IsStrong) || !(rd->flags & Data::IsSharable))
  {
    err_t err = prepare(rd->length);
    if (err) return err;

    td = _d;
    td->length = rd->length;
    td->extents = rd->extents;
    _copyRects(td->rects, rd->rects, rd->length);
  }
  else
  {
    atomicPtrXchg(&_d, rd->refAlways())->derefInline();
  }

  return ERR_OK;
}

err_t Region::set(const Rect& r)
{
  if (!r.isValid()) { clear(); return ERR_OK; }

  err_t err = prepare(1);
  if (err) return err;

  Box b(r);
  Data* newd = _d;

  newd->length = 1;
  newd->extents = b;
  newd->rects[0] = b;

  return ERR_OK;
}

err_t Region::set(const Box& r)
{
  if (!r.isValid()) { clear(); return ERR_OK; }

  prepare(1);
  Data* d = _d;
  d->length = 1;
  d->extents = r;
  d->rects[0] = r;

  return ERR_OK;
}

err_t Region::setDeep(const Region& r)
{
  Data* td = _d;
  Data* rd = r._d;
  if (td == rd) return ERR_OK;

  err_t err = prepare(rd->length);
  if (err) return err;

  td = _d;
  td->length = rd->length;
  td->extents = rd->extents;
  _copyRects(td->rects, rd->rects, rd->length);

  return ERR_OK;
}

err_t Region::set(const Rect* rects, sysuint_t length)
{
  // TODO: Not optimal.
  clear();
  for (sysuint_t i = 0; i < length; i++) unite(rects[i]);
  return ERR_OK;
}

err_t Region::set(const Box* rects, sysuint_t length)
{
  // TODO: Not optimal.
  clear();
  for (sysuint_t i = 0; i < length; i++) unite(rects[i]);
  return ERR_OK;
}

err_t Region::unite(const Region& r)
{
  Data* td = _d;
  Data* rd = r._d;

  // Union region is same or r is empty... -> nop
  if (td == rd || rd->length == 0) return ERR_OK;

  // We are empty or r completely subsumes us -> set r
  if (td->length == 0 || (rd->length == 1 && rd->extents.subsumes(td->extents))) return set(r);

  // We completely subsumes r
  if (td->length == 1 && rd->length == 1 && td->extents.subsumes(rd->extents)) return ERR_OK;

  // Last optimization can be append
  const Box* sdlast = td->rects + td->length - 1;
  const Box* rdfirst = rd->rects;

  Box ext(Math::min(td->extents.getX1(), rd->extents.getX1()),
          Math::min(td->extents.getY1(), rd->extents.getY1()),
          Math::max(td->extents.getX2(), rd->extents.getX2()),
          Math::max(td->extents.getY2(), rd->extents.getY2()));

  err_t err;
  if (sdlast->getY2() <= rdfirst->getY1() ||
     (sdlast->getY1() == rdfirst->getY1() &&
      sdlast->getY2() == rdfirst->getY2() &&
      sdlast->getX2() <= rdfirst->getX1() ))
  {
    err = _appendPrivate(this, rd->rects, rd->length, &ext);
  }
  else
  {
    err = _unitePrivate(this, td->rects, td->length, rd->rects, rd->length, true, &ext);
  }

  return err;
}

err_t Region::unite(const Rect& r)
{
  return unite(Box(r));
}

err_t Region::unite(const Box& r)
{
  Data* td = _d;

  if (!r.isValid()) return ERR_OK;

  // We are empty or 'r' completely subsumes us.
  if (td->length == 0 || r.subsumes(td->extents)) return set(r);

  // We completely subsumes src.
  if (td->length == 1 && td->extents.subsumes(r)) return ERR_OK;

  Box ext(Math::min(td->extents.getX1(), r.getX1()),
          Math::min(td->extents.getY1(), r.getY1()),
          Math::max(td->extents.getX2(), r.getX2()),
          Math::max(td->extents.getY2(), r.getY2()));

  // Last optimization can be append.
  Box* sdlast = td->rects + td->length-1;
  err_t err;

  if ( sdlast->getY2() <= r.getY1() ||
      (sdlast->getY1() == r.getY1() &&
       sdlast->getY2() == r.getY2() &&
       sdlast->getX2() <= r.getX1() ))
  {
    err = _appendPrivate(this, &r, 1, &ext);
  }
  else
  {
    err = _unitePrivate(this, td->rects, td->length, &r, 1, true, &ext);
  }

  return err;
}

err_t Region::intersect(const Region& r)
{
  Data* td = _d;
  Data* rd = r._d;
  err_t err = ERR_OK;

  if (td == rd)
    ;
  else if (td->length == 0 || rd->length == 0 || !td->extents.overlaps(rd->extents)) 
    clear();
  else
    err = _intersectPrivate(this, td->rects, td->length, rd->rects, rd->length, true);

  return err;
}

err_t Region::intersect(const Rect& r)
{
  return intersect(Box(r));
}

err_t Region::intersect(const Box& r)
{
  Data* td = _d;
  err_t err = ERR_OK;

  if (td->length == 0 || !r.isValid() || !td->extents.overlaps(r)) 
    clear();
  else
    err = _intersectPrivate(this, td->rects, td->length, &r, 1, true);

  return err;
}

err_t Region::eor(const Region& r)
{
  return eor(*this, *this, r);
}

err_t Region::eor(const Rect& r)
{
  TemporaryRegion<1> reg(r);
  return eor(reg);
}

err_t Region::eor(const Box& r)
{
  TemporaryRegion<1> reg(r);
  return eor(reg);
}

err_t Region::subtract(const Region& r)
{
  Data* td = _d;
  Data* rd = r._d;
  err_t err = ERR_OK;

  if (td == rd) 
    clear();
  else if (td->length == 0 || rd->length == 0 || !td->extents.overlaps(rd->extents)) 
    ;
  else
    err = _subtractPrivate(this, td->rects, td->length, rd->rects, rd->length, true);

  return err;
}

err_t Region::subtract(const Rect& r)
{
  return subtract(Box(r));
}

err_t Region::subtract(const Box& r)
{
  Data* td = _d;
  err_t err = ERR_OK;

  if (td->length == 0 || !r.isValid() || !td->extents.overlaps(r))
    clear();
  else
    err = _subtractPrivate(this, td->rects, td->length, &r, 1, true);

  return err;
}

err_t Region::op(const Region& r, int _op)
{
  switch (_op)
  {
    case REGION_OP_COPY     : return set(r);
    case REGION_OP_UNITE    : return unite(r);
    case REGION_OP_INTERSECT: return intersect(r);
    case REGION_OP_XOR      : return eor(r);
    case REGION_OP_SUBTRACT : return subtract(r);

    default:
      FOG_ASSERT_NOT_REACHED();
      return ERR_RT_INVALID_ARGUMENT;
  }
}

err_t Region::op(const Rect& r, int _op)
{
  return op(Box(r), _op);
}

err_t Region::op(const Box& r, int _op)
{
  switch (_op)
  {
    case REGION_OP_COPY     : return set(r);
    case REGION_OP_UNITE    : return unite(r);
    case REGION_OP_INTERSECT: return intersect(r);
    case REGION_OP_XOR      : return eor(r);
    case REGION_OP_SUBTRACT : return subtract(r);

    default:
      FOG_ASSERT_NOT_REACHED();
      return ERR_RT_INVALID_ARGUMENT;
  }
}

err_t Region::translate(const Point& pt)
{
  return translate(*this, *this, pt);
}

err_t Region::shrink(const Point& pt)
{
  return shrink(*this, *this, pt);
}

err_t Region::frame(const Point& pt)
{
  return frame(*this, *this, pt);
}

bool Region::eq(const Region& other) const
{
  Data* d1 = _d;
  Data* d2 = other._d;

  if (d1 == d2) return true;
  if (d1->length != d2->length) return false;

  sysuint_t i;
  sysuint_t length = d1->length;

  for (i = 0; i != length; i++)
  {
    if (d1->rects[i] != d2->rects[i]) return false;
  }

  return true;
}

err_t Region::set(Region& dest, const Region& src)
{
  return dest.set(src);
}

err_t Region::unite(Region& dest, const Region& src1, const Region& src2)
{
  Data* destd = dest._d;
  Data* src1d = src1._d;
  Data* src2d = src2._d;

  // Trivial operations
  if (src1d->length == 0) { return dest.set(src2); }
  if (src2d->length == 0) { return dest.set(src1); }
  if (src1d == src2d    ) { return dest.set(src1); }

  const Box* src1first = src1d->rects;
  const Box* src2first = src2d->rects;
  const Box* src1last = src1first + src1d->length - 1;
  const Box* src2last = src2first + src2d->length - 1;

  Box ext(Math::min(src1d->extents.getX1(), src2d->extents.getX1()),
          Math::min(src1d->extents.getY1(), src2d->extents.getY1()),
          Math::max(src1d->extents.getX2(), src2d->extents.getX2()),
          Math::max(src1d->extents.getY2(), src2d->extents.getY2()));

  err_t err;
  if (src1last->getY2() <= src2first->getY1()) 
  {
    err = dest.setDeep(src1);
    if (!err) err = _appendPrivate(&dest, src2first, src2d->length, &ext);
  }
  else if (src2last->getY2() <= src1first->getY1()) 
  {
    err = dest.setDeep(src2);
    if (!err) err = _appendPrivate(&dest, src1first, src1d->length, &ext);
  }
  else
  {
    err = _unitePrivate(&dest, src1first, src1d->length, src2first, src2d->length, destd == src1d || destd == src2d, &ext);
  }

  return err;
}

err_t Region::intersect(Region& dest, const Region& src1, const Region& src2)
{
  Data* destd = dest._d;
  Data* src1d = src1._d;
  Data* src2d = src2._d;
  err_t err = ERR_OK;

  // Trivial rejects.
  if (FOG_UNLIKELY(src1d == src2d))
    err = dest.set(src1);
  else if (FOG_UNLIKELY(src1d->length == 0 || src2d->length == 0 || !src1d->extents.overlaps(src2d->extents))) 
    dest.clear();
  else
    err = _intersectPrivate(&dest, src1d->rects, src1d->length, src2d->rects, src2d->length, destd == src1d || destd == src2d);

  return err;
}

err_t Region::eor(Region& dest, const Region& src1, const Region& src2)
{
  TemporaryRegion<Fog_Region_Local> r1;
  TemporaryRegion<Fog_Region_Local> r2;
  err_t err;

  if ((err = subtract(r1, src1, src2))) return err;
  if ((err = subtract(r2, src2, src1))) return err;
  if ((err = unite(dest, r1, r2))) return err;

  return ERR_OK;
}

err_t Region::subtract(Region& dest, const Region& src1, const Region& src2)
{
  Data* destd = dest._d;
  Data* src1d = src1._d;
  Data* src2d = src2._d;
  err_t err = ERR_OK;

  // Trivial rejects.
  if (src1d == src2d || src1d->length == 0) 
    dest.clear();
  else if (src2d->length == 0 || !src1d->extents.overlaps(src2d->extents)) 
    err = dest.set(src1);
  else
    err = _subtractPrivate(&dest, src1d->rects, src1d->length, src2d->rects, src2d->length, destd == src1d || destd == src2d);

  return ERR_OK;
}

err_t Region::op(Region& dest, const Region& src1, const Region& src2, int _op)
{
  switch (_op)
  {
    case REGION_OP_COPY     : return set(dest, src1);
    case REGION_OP_UNITE    : return unite(dest, src1, src2);
    case REGION_OP_INTERSECT: return intersect(dest, src1, src2);
    case REGION_OP_XOR      : return eor(dest, src1, src2);
    case REGION_OP_SUBTRACT : return subtract(dest, src1, src2);
    default:
      FOG_ASSERT_NOT_REACHED();
      return ERR_RT_INVALID_ARGUMENT;
  }
}

err_t Region::translate(Region& dest, const Region& src, const Point& pt)
{
  int x = pt.getX();
  int y = pt.getY();

  if (x == 0 && y == 0) { return dest.set(src); }

  Data* dest_d = dest._d;
  Data* src_d = src._d;

  sysuint_t i;
  Box* dest_r;
  Box* src_r;

  if (src_d->length == 0) 
  {
    dest.clear();
  }
  else if (dest_d == src_d && dest_d->refCount.get() == 1)
  {
    dest_d->extents.translate(x, y);
    dest_r = dest_d->rects;
    for (i = dest_d->length; i; i--, dest_r++) dest_r->translate(x, y);
  }
  else
  {
    err_t err = dest.prepare(src_d->length);
    if (err) return err;

    dest_d = dest._d; 

    dest_d->extents.set(src_d->extents.getX1() + x, src_d->extents.getY1() + y,
                        src_d->extents.getX2() + x, src_d->extents.getY2() + y);
    dest_d->length = src_d->length;

    dest_r = dest_d->rects;
    src_r = src_d->rects;
    for (i = dest_d->length; i; i--, dest_r++) 
    {
      dest_r->set(
        src_r->getX1() + x, src_r->getY1() + y,
        src_r->getX2() + x, src_r->getY2() + y);
    }
  }
  return ERR_OK;
}

err_t Region::shrink(Region& dest, const Region& src, const Point& pt)
{
  int x = pt.getX();
  int y = pt.getY();
  if (x == 0 && y == 0) return dest.set(src);

  err_t err = dest.setDeep(src);
  if (err) return err;

  TemporaryRegion<Fog_Region_Local> s;
  TemporaryRegion<Fog_Region_Local> t;
  bool grow;

  if (x) { if ((grow = (x < 0))) { x = -x; } err = _compress(dest, s, t, uint(x) << 1, true , grow); if (err) return err; }
  if (y) { if ((grow = (y < 0))) { y = -y; } err = _compress(dest, s, t, uint(y) << 1, false, grow); if (err) return err; }

  return dest.translate(x, y);
}

err_t Region::frame(Region& dest, const Region& src, const Point& pt)
{
  // In cases that dest == src, we need to backup src
  TemporaryRegion<Fog_Region_Local> r1;
  TemporaryRegion<Fog_Region_Local> r2;
  err_t err;

  if ((err = translate(r2, src, Point(-pt.getX(), 0))) ||
      (err = translate(r1, src, Point( pt.getX(), 0))) ||
      (err = r2.intersect(r1)) ||
      (err = translate(r1, src, Point(0, -pt.getY()))) ||
      (err = r2.intersect(r1)) ||
      (err = translate(r1, src, Point(0,  pt.getY()))) ||
      (err = r2.intersect(r1)) ||
      (err = subtract(dest, src, r2))) return err;

  return ERR_OK;
}

#if defined(FOG_OS_WINDOWS)
static FOG_INLINE void BoxToRECT(RECT* dest, const Box* src)
{
  dest->left   = src->getX1();
  dest->top    = src->getY1();
  dest->right  = src->getX2();
  dest->bottom = src->getY2();
}

static FOG_INLINE void RECTToBox(Box* dest, const RECT* src)
{
  dest->x1 = src->left;
  dest->y1 = src->top;
  dest->x2 = src->right;
  dest->y2 = src->bottom;
}

HRGN Region::toHRGN() const
{
  Data* d = _d;
  sysuint_t i;
  sysuint_t length = d->length;

  LocalBuffer<4096> mem;
  RGNDATAHEADER *hdr = (RGNDATAHEADER *)mem.alloc(sizeof(RGNDATAHEADER) + length * sizeof(RECT));
  if (!hdr) return (HRGN)NULLREGION;

  hdr->dwSize = sizeof(RGNDATAHEADER);
  hdr->iType = RDH_RECTANGLES;
  hdr->nCount = length;
  hdr->nRgnSize = length * sizeof(RECT);
  BoxToRECT(&hdr->rcBound, &d->extents);

  RECT* r = (RECT *)((uint8_t *)hdr + sizeof(RGNDATAHEADER));
  for (i = 0; i < length; i++, r++)
  {
    BoxToRECT(r, &d->rects[i]);
  }
  return ExtCreateRegion(NULL, sizeof(RGNDATAHEADER) + (sizeof(RECT)*length), (RGNDATA *)hdr);
}

err_t Region::fromHRGN(HRGN hrgn)
{
  clear();

  if (hrgn == NULL) return ERR_RT_INVALID_ARGUMENT;
  if (hrgn == (HRGN)NULLREGION) return ERR_OK;

  DWORD size = GetRegionData(hrgn, 0, NULL);
  if (size == 0) return false;

  LocalBuffer<1024> mem;
  RGNDATAHEADER *hdr = (RGNDATAHEADER *)mem.alloc(size);
  if (hdr) return ERR_RT_OUT_OF_MEMORY;

  if (!GetRegionData(hrgn, size, (RGNDATA*)hdr)) return GetLastError();

  sysuint_t i;
  sysuint_t length = hdr->nCount;
  RECT* r = (RECT*)((uint8_t*)hdr + sizeof(RGNDATAHEADER));

  // TODO: Rects should be already YX sorted, but I'm not sure
  for (i = 0; i != length; i++, r++)
  {
    unite(Box(r->left, r->top, r->right, r->bottom));
  }

  return ERR_OK;
}
#endif // FOG_OS_WINDOWS

// ============================================================================
// [Fog::Region::Data]
// ============================================================================

Region::Data* Region::Data::ref() const
{
  if (flags & IsSharable)
  {
    refCount.inc();
    return const_cast<Data*>(this);
  }
  else
  {
    return Data::copy(this);
  }
}

void Region::Data::deref()
{
  if (refCount.deref() && (flags & IsDynamic) != 0) Memory::free(this);
}

static Region::Data* _reallocRegion(Region::Data* d, sysuint_t capacity)
{
  if ((d->flags & Region::Data::IsDynamic) != 0)
  {
    d = (Region::Data*)Memory::realloc(d, Region::Data::sizeFor(capacity));
    if (!d) return NULL;

    d->capacity = capacity;
    return d;
  }
  else
  {
    return Region::Data::create(capacity, &d->extents, d->rects, d->length);
  }
}

Region::Data* Region::Data::adopt(void* address, sysuint_t capacity)
{
  Data* d = (Data*)address;
  
  d->refCount.init(1);
  d->flags = IsStrong;
  d->capacity = capacity;
  d->length = 0;
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
    d->length = 1;
    d->extents = r;
    d->rects[0] = r;
  }
  else
  {
    d->length = 0;
    d->extents.clear();
    d->rects[0].clear();
  }

  return d;
}

Region::Data* Region::Data::adopt(void* address, sysuint_t capacity, const Box* extents, const Box* rects, sysuint_t length)
{
  if (capacity < length) create(length, extents, rects, length);

  Data* d = (Data*)address;
  
  d->refCount.init(1);
  d->flags = IsStrong;
  d->capacity = capacity;
  d->length = length;
  
  if (extents)
  {
    d->extents = *extents;
    _copyRects(d->rects, rects, length);
  }
  else
  {
    _copyRectsExtents(d->rects, rects, length, &d->extents);
  }

  return d;
}

Region::Data* Region::Data::create(sysuint_t capacity)
{
  if (FOG_UNLIKELY(capacity == 0)) return Region::sharedNull.instancep()->refAlways();

  sysuint_t dsize = sizeFor(capacity);
  Data* d = (Data*)Memory::alloc(dsize);
  if (!d) return NULL;

  d->refCount.init(1);
  d->flags = IsDynamic | IsSharable;
  d->capacity = capacity;
  d->length = 0;

  return d;
}

Region::Data* Region::Data::create(sysuint_t capacity, const Box* extents, const Box* rects, sysuint_t length)
{
  if (FOG_UNLIKELY(capacity < length)) capacity = length;

  Data* d = create(capacity);
  if (!d) return NULL;

  if (FOG_LIKELY(length))
  {
    d->length = length;
    if (extents)
    {
      d->extents = *extents;
      _copyRects(d->rects, rects, length);
    }
    else
    {
      _copyRectsExtents(d->rects, rects, length, &d->extents);
    }
  }

  return d;
}

Region::Data* Region::Data::copy(const Data* other)
{
  if (!other->length) return Region::sharedNull.instancep()->refAlways();

  Data* d = create(other->length);
  if (!d) return NULL;

  d->length = other->length;
  d->extents = other->extents;
  _copyRects(d->rects, other->rects, other->length);

  return d;
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_region_init(void)
{
  using namespace Fog;

  Region::sharedNull.init();
  Region::Data* d = Region::sharedNull.instancep();

  d->refCount.init(1);
  d->flags = Region::Data::IsSharable;
  d->extents.clear();
  d->rects[0].clear();

  return ERR_OK;
}

FOG_INIT_DECLARE void fog_region_shutdown(void)
{
  using namespace Fog;

  Region::sharedNull.instancep()->refCount.dec();
  Region::sharedNull.destroy();
}
