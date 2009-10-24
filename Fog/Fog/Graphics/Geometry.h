// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_GEOMETRY_H
#define _FOG_GRAPHICS_GEOMETRY_H

// [Dependencies]
#include <Fog/Core/Math.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/TypeInfo.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct Rect;
struct Box;

// ============================================================================
// [Fog::Point]
// ============================================================================

struct FOG_HIDDEN Point
{
  // [Construction / Destruction]

  FOG_INLINE Point()
  {
  }
  
  FOG_INLINE Point(int px, int py) : x(px), y(py)
  {
  }

  FOG_INLINE Point(const Point& other)
  {
    if (sizeof(Point) == 8)
    {
      Memory::copy8B(static_cast<void*>(this), static_cast<const void*>(&other));
    }
    else
    {
      x = other.x;
      y = other.y;
    }
  }

  // [Methods]

  FOG_INLINE int getX() const { return x; }
  FOG_INLINE int getY() const { return y; }

  FOG_INLINE Point& set(const Point& other) 
  { 
    if (sizeof(Point) == 8)
    {
      Memory::copy8B(static_cast<void*>(this), static_cast<const void*>(&other));
    }
    else
    {
      x = other.x;
      y = other.y;
    }
    return *this;
  }

  FOG_INLINE Point& set(int px, int py)
  { 
    x = px;
    y = py;
    return *this;
  }

  FOG_INLINE Point& setX(int px) { x = px; return *this; }
  FOG_INLINE Point& setY(int py) { y = py; return *this; }

  FOG_INLINE Point& clear() 
  { 
    x = 0; 
    y = 0; 
    return *this;
  }

  FOG_INLINE Point& translate(int px, int py)
  {
    x += px;
    y += py;
    return *this;
  }

  FOG_INLINE Point& translate(const Point& other)
  {
    x += other.x;
    y += other.y;
    return *this;
  }

  FOG_INLINE Point& negate()
  {
    x = -x;
    y = -y;
    return *this;
  }

  FOG_INLINE Point& operator+=(const Point& other)
  {
    x += other.x;
    y += other.y;
    return *this;
  }

  FOG_INLINE Point& operator-=(const Point& other) 
  {
    x -= other.x;
    y -= other.y;
    return *this;
  }

  FOG_INLINE bool eq(int px, int py) const
  {
    return x == px && y == py;
  }
  
  FOG_INLINE bool eq(const Point& other) const
  {
    if (sizeof(Point) == 8)
      return Memory::eq8B(static_cast<const void*>(this), static_cast<const void*>(&other));
    else
      return (x == other.x) & (y == other.y);
  }

  FOG_INLINE Point translated(int px, int py) const
  {
    return Point(x + px, y + py);
  }

  FOG_INLINE Point translated(const Point& other) const
  {
    return Point(x + other.x, y + other.y);
  }

  FOG_INLINE Point negated() const 
  {
    return Point(-x, -y);
  }

  // [Members]

  int x;
  int y;
};

// ============================================================================
// [Fog::PointD]
// ============================================================================

struct PointD
{
  // [Construction / Destruction]

  FOG_INLINE PointD()
  {
  }
  
  FOG_INLINE PointD(double px, double py) : x(px), y(py)
  {
  }

  FOG_INLINE PointD(const PointD& other) : x(other.x), y(other.y)
  {
  }

  // [Methods]

  FOG_INLINE double getX() const { return x; }
  FOG_INLINE double getY() const { return y; }

  FOG_INLINE PointD& set(const PointD& other)
  {
    x = other.x;
    y = other.y;
    return *this;
  }

  FOG_INLINE PointD& set(double px, double py) 
  { 
    x = px;
    y = py;
    return *this;
  }

  FOG_INLINE PointD& setX(double px) { x = px; return *this; }
  FOG_INLINE PointD& setY(double py) { y = py; return *this; }

  FOG_INLINE PointD& clear()
  { 
    x = 0.0;
    y = 0.0;
    return *this;
  }

  FOG_INLINE PointD& translate(double px, double py)
  {
    x += px;
    y += py;
    return *this;
  }

  FOG_INLINE PointD& translate(const PointD& other)
  {
    x += other.x;
    y += other.y;
    return *this;
  }

  FOG_INLINE PointD& negate()
  {
    x = -x;
    y = -y;
    return *this;
  }

  FOG_INLINE PointD& operator+=(const PointD& other) 
  {
    x += other.x;
    y += other.y;
    return *this;
  }
  
  FOG_INLINE PointD& operator-=(const PointD& other) 
  { 
    x -= other.x;
    y -= other.y;
    return *this;
  }

  FOG_INLINE bool eq(double px, double py) const
  { 
    return (x == px) & (y == py);
  }

  FOG_INLINE bool eq(const PointD& other) const
  {
    return (x == other.x) & (y == other.y);
  }

  FOG_INLINE PointD translated(double px, double py)
  {
    return PointD(x + px, y + py);
  }

  FOG_INLINE PointD translated(const PointD& other)
  {
    return PointD(x + other.x, y + other.y);
  }

  FOG_INLINE PointD negated() const 
  {
    return PointD(-x, -y);
  }

  // [Members]

  double x;
  double y;
};

// ============================================================================
// [Fog::Size]
// ============================================================================

struct Size
{
  // [Construction / Destruction]

  FOG_INLINE Size()
  {
  }

  FOG_INLINE Size(int sw, int sh) : w(sw), h(sh)
  {
  }

  FOG_INLINE Size(const Size& other) : w(other.w), h(other.h)
  {
  }

  // [Methods]

  FOG_INLINE int getWidth() const { return w; }
  FOG_INLINE int getHeight() const { return h; }

  FOG_INLINE Size& set(const Size& other) 
  {
    w = other.w;
    h = other.h;
    return *this;
  }

  FOG_INLINE Size& set(int sw, int sh)
  {
    w = sw;
    h = sh;
    return *this;
  }

  FOG_INLINE Size& setWidth(int sw) { w = sw; return *this; }
  FOG_INLINE Size& setHeight(int sh) { h = sh; return *this; }

  FOG_INLINE Size& clear()
  {
    w = 0;
    h = 0;
    return *this;
  }

  FOG_INLINE bool isValid() const
  {
    return (w > 0) & (h > 0);
  }
  
  FOG_INLINE bool eq(const Size& other) const
  {
    return (w == other.w) & (h == other.h);
  }
  
  FOG_INLINE bool eq(int sw, int sh) const
  {
    return (w == sw) && (h == sh);
  }

  FOG_INLINE Size& adjust(int sw, int sh)
  {
    w += sw;
    h += sh;
    return *this;
  }

  FOG_INLINE Size adjusted(int sw, int sh) const
  {
    return Size(w + sw, h + sh);
  }

  // [Members]

  int w;
  int h;
};

// ============================================================================
// [Fog::SizeF]
// ============================================================================

struct SizeF
{
  // [Construction / Destruction]

  FOG_INLINE SizeF() 
  {
  }

  FOG_INLINE SizeF(double sw, double sh) : w(sw), h(sh)
  {
  }

  FOG_INLINE SizeF(const SizeF& other) : w(other.w), h(other.h)
  {
  }

  // [Methods]

  FOG_INLINE double getWidth() const { return w; }
  FOG_INLINE double getHeight() const { return h; }

  FOG_INLINE SizeF& set(const SizeF& other)
  {
    w = other.w;
    h = other.h;
    return *this;
  }

  FOG_INLINE SizeF& set(double sw, double sh)
  {
    w = sw;
    h = sh;
    return *this;
  }

  FOG_INLINE SizeF& setWidth(double sw) { w = sw; return *this; }
  FOG_INLINE SizeF& setHeight(double sh) { h = sh; return *this; }

  FOG_INLINE SizeF& clear()
  {
    w = 0.0;
    h = 0.0;
    return *this; 
  }

  FOG_INLINE bool isValid() const
  {
    return (w > 0.0) & (h > 0.0);
  }

  FOG_INLINE bool eq(const SizeF& other) const
  {
    return (w == other.w) & (h == other.h);
  }
  
  FOG_INLINE bool eq(double sw, double sh) const
  {
    return (w == sw) & (h == sh);
  }

  FOG_INLINE SizeF& adjust(double sw, double sh)
  {
    w += sw;
    h += sh;
    return *this;
  }

  FOG_INLINE SizeF adjusted(double sw, double sh) const
  {
    return SizeF(w + sw, h + sh);
  }

  // [Members]

  double w;
  double h;
};

// ============================================================================
// [Fog::Rect]
// ============================================================================

struct Rect
{
  // [Construction / Destruction]

  FOG_INLINE Rect()
  {
  }

  FOG_INLINE Rect(int rx, int ry, int rw, int rh) : 
    x(rx), y(ry), w(rw), h(rh)
  {
  }

  FOG_INLINE Rect(const Rect& other) : 
    x(other.x), y(other.y), w(other.w), h(other.h)
  {
  }

  // [Methods]

  // defined later
  FOG_INLINE explicit Rect(const Box& box);

  FOG_INLINE int getX() const { return x; }
  FOG_INLINE int getY() const { return y; }
  FOG_INLINE int getWidth() const { return w; }
  FOG_INLINE int getHeight() const { return h; }

  FOG_INLINE int getX1() const { return x; }
  FOG_INLINE int getY1() const { return y; }
  FOG_INLINE int getX2() const { return x + w; }
  FOG_INLINE int getY2() const { return y + h; }

  FOG_INLINE int getLeft() const { return x; }
  FOG_INLINE int getTop() const { return y; }
  FOG_INLINE int getRight() const { return x + w; }
  FOG_INLINE int getBottom() const { return y + h; }

  FOG_INLINE const Point& getPosition() const { return *(const Point *)(const void*)(&x); }
  FOG_INLINE const Size& getSize() const { return *(const Size *)(const void*)(&w); }

  FOG_INLINE Rect& set(int rx, int ry, int rw, int rh)
  { 
    x = rx; 
    y = ry; 
    w = rw; 
    h = rh; 

    return *this;
  }

  FOG_INLINE Rect& set(const Rect &other)
  {
    if (sizeof(Rect) == 16)
    {
      Memory::copy16B(static_cast<void*>(this), static_cast<const void*>(&other));
    }
    else
    {
      x = other.x;
      y = other.y;
      w = other.w;
      h = other.h;
    }
    return *this;
  }

  FOG_INLINE Rect& set(const Box& box);

  FOG_INLINE Rect& setX(int rx) { x = rx; return *this; }
  FOG_INLINE Rect& setY(int ry) { y = ry; return *this; }
  FOG_INLINE Rect& setWidth(int rw) { w = rw; return *this; }
  FOG_INLINE Rect& setHeight(int rh) { h = rh; return *this; }

  FOG_INLINE Rect& setX1(int x1) { x = x1; return *this; }
  FOG_INLINE Rect& setY1(int y1) { y = y1; return *this; }
  FOG_INLINE Rect& setX2(int x2) { w = x2 - x; return *this; }
  FOG_INLINE Rect& setY2(int y2) { h = y2 - y; return *this; }

  FOG_INLINE Rect& setLeft(int x1) { x = x1; return *this; }
  FOG_INLINE Rect& setTop(int y1) { y = y1; return *this; }
  FOG_INLINE Rect& setRight(int x2) { w = x2 - x; return *this; }
  FOG_INLINE Rect& setBottom(int y2) { h = y2 - y; return *this; }

  FOG_INLINE Rect& translate(int rx, int ry) { x += rx; y += ry; return *this; }
  FOG_INLINE Rect& translate(const Point& p) { x += p.x; y += p.y; return *this; }

  FOG_INLINE Rect& clear() 
  {
    if (sizeof(Rect) == 16)
    {
      Memory::zero16B(static_cast<void*>(this));
    }
    else
    {
      x = 0;
      y = 0;
      w = 0;
      h = 0;
    }
    return *this;
  }

  FOG_INLINE Rect& operator+=(const Point& p) 
  { 
    x += p.x;
    y += p.y;
    return *this;
  }
  
  FOG_INLINE Rect& operator-=(const Point& p) 
  { 
    x -= p.x;
    y -= p.y;
    return *this;
  }

  //! @brief Checks if two rectangles overlap.
  //! @return @c true if two rectangles overlap, @c false if two rectangles
  //! do not overlap. Remember, x2() and y2() coords aren't in the rectangle.
  FOG_INLINE bool overlaps(const Rect& r) const
  {
    return (( ((getY1()-r.getY2()) ^ (getY2()-r.getY1())) &
              ((getX1()-r.getX2()) ^ (getX2()-r.getX1())) ) < 0);
  }

  //! @brief Returns @c true if rectangle completely subsumes @a r.
  FOG_INLINE bool subsumes(const Rect& r) const
  {
    return ((r.getX1() >= getX1()) & (r.getX2() <= getX2()) &
            (r.getY1() >= getY1()) & (r.getY2() <= getY2()) );
  }

  //! @brief Returns @c true if given point is in rectangle.
  //! @brief x Point x coordinate.
  //! @brief y Point y coordinate.
  //! @return @c true if point is in rectangle.
  FOG_INLINE bool contains(int px, int py) const
  {
    return ((px >= getX1()) & (py >= getY1()) & 
            (px <  getX2()) & (py <  getY2()) );
  }

  //! @brief Returns @c true if given point @a pt is in rectangle.
  //! @brief x Point x coordinate.
  //! @brief y Point y coordinate.
  //! @return @c true if point is in rectangle.
  FOG_INLINE bool contains(const Point& pt) const
  {
    return ((pt.x >= getX1()) & (pt.y >= getY1()) & 
            (pt.x <  getX2()) & (pt.y <  getY2()) );
  }

  //! @brief Returns @c true if rectangle is valid.
  //!
  //! Rectangle is only valid if @c x2 is greater than @c x1 and @c y2 is
  //! greater than @c y1.
  FOG_INLINE bool isValid() const
  {
    return (w > 0) & (h > 0);
  }

  FOG_INLINE bool eq(int px, int py, int pw, int ph) const
  {
    return (x == px) & (y == py) & (w == pw) & (h == ph);
  }

  FOG_INLINE bool eq(const Rect& other) const
  {
    if (sizeof(Rect) == 16)
      return Memory::eq16B(static_cast<const void*>(this), static_cast<const void*>(&other));
    else
      return (x == other.x) & (y == other.y) & (w == other.w) & (h == other.h);
  }

  //! @brief Shrinks rectangle by @c n coordinates.
  FOG_INLINE Rect& shrink(int n)
  {
    x += n;
    y += n;
    n <<= 1;
    w -= n;
    h -= n;
    
    return *this;
  }

  /*! @brief Expands rectangle by @c n coordinates. */
  FOG_INLINE Rect& expand(int n)
  {
    x -= n;
    y -= n;
    n <<= 1;
    w += n;
    h += n;

    return *this;
  }

  FOG_INLINE Rect& adjust(int px, int py)
  {
    x += px;
    y += py;
    w -= px;
    h -= py;

    w -= px;
    h -= py;

    return *this;
  }

  FOG_INLINE Rect& adjust(int px1, int py1, int px2, int py2)
  {
    x += px1;
    y += py1;
    w -= px1;
    h -= py1;

    w += px2;
    h += py2;

    return *this;
  }

  FOG_INLINE Rect adjusted(int px, int py) const 
  {
    return Rect(x + px, y + py, w - px - px, h - py - py);
  }

  FOG_INLINE Rect adjusted(int px1, int py1, int px2, int py2) const 
  {
    return Rect(x + px1, y + py1, w - px1 - px2, h - py1 - py2);
  }

  static FOG_INLINE bool intersect(Rect& dest, const Rect& src1, const Rect& src2)
  {
    int xx = Math::max(src1.getX1(), src2.getX1());
    int yy = Math::max(src1.getY1(), src2.getY1());

    dest.set(
      xx,
      yy,
      Math::min(src1.getX2(), src2.getX2()) - xx,
      Math::min(src1.getY2(), src2.getY2()) - yy);
    return dest.isValid();
  }

  // [Members]

  int x;
  int y;
  int w;
  int h;
};

// ============================================================================
// [Fog::RectD]
// ============================================================================

struct RectD
{
  // [Construction / Destruction]

  FOG_INLINE RectD()
  {
  }

  FOG_INLINE RectD(double rx, double ry, double rw, double rh)
    : x(rx), y(ry), w(rw), h(rh)
  {
  }

  FOG_INLINE RectD(const PointD& pt1, const PointD& pt2)
    : x(pt1.x), y(pt1.y), w(pt2.x - pt1.x), h(pt2.y - pt1.y)
  {
  }

  FOG_INLINE RectD(const RectD& other)
    : x(other.x), y(other.y), w(other.w), h(other.h)
  {
  }

  // [Methods]

  FOG_INLINE double getX() const { return x; }
  FOG_INLINE double getY() const { return y; }
  FOG_INLINE double getWidth() const { return w; }
  FOG_INLINE double getHeight() const { return h; }

  FOG_INLINE double getX1() const { return x; }
  FOG_INLINE double getY1() const { return y; }
  FOG_INLINE double getX2() const { return x + w; }
  FOG_INLINE double getY2() const { return y + h; }

  FOG_INLINE double getLeft() const { return x; }
  FOG_INLINE double getTop() const { return y; }
  FOG_INLINE double getRight() const { return x + w; }
  FOG_INLINE double getBottom() const { return y + h; }

  FOG_INLINE const PointD& getPosition() const { return *(const PointD *)(const void*)(&x); }
  FOG_INLINE const SizeF& getSize() const { return *(const SizeF *)(const void*)(&w); }

  FOG_INLINE RectD& set(double rx, double ry, double rw, double rh)
  { 
    x = rx;
    y = ry;
    w = rw;
    h = rh;
    return *this;
  }

  FOG_INLINE RectD& set(const RectD &other)
  {
    x = other.x;
    y = other.y;
    w = other.w;
    h = other.h;
    return *this;
  }

  FOG_INLINE RectD& setX(double x) { x = x; return *this; }
  FOG_INLINE RectD& setY(double y) { y = y; return *this; }
  FOG_INLINE RectD& setWidth(double w) { w = w; return *this; }
  FOG_INLINE RectD& setHeight(double h) { h = h; return *this; }

  FOG_INLINE RectD& setX1(double x1) { x = x1; return *this; }
  FOG_INLINE RectD& setY1(double y1) { y = y1; return *this; }
  FOG_INLINE RectD& setX2(double x2) { w = x2 - x; return *this; }
  FOG_INLINE RectD& setY2(double y2) { h = y2 - y; return *this; }

  FOG_INLINE RectD& setLeft(double x1) { x = x1; return *this; }
  FOG_INLINE RectD& setTop(double y1) { y = y1; return *this; }
  FOG_INLINE RectD& setRight(double x2) { w = x2 - x; return *this; }
  FOG_INLINE RectD& setBottom(double y2) { h = y2 - y; return *this; }

  FOG_INLINE RectD& clear() 
  {
    x = 0.0;
    y = 0.0;
    w = 0.0;
    h = 0.0;

    return *this;
  }

  FOG_INLINE RectD& translate(double px, double py)
  {
    x += px;
    y += py;
    return *this;
  }
  
  FOG_INLINE RectD& translate(const PointD& pt)
  {
    x += pt.x;
    y += pt.y;
    return *this;
  }

  FOG_INLINE RectD& adjust(double px, double py)
  {
    x += px;
    y += py;

    w -= px;
    h -= py;
    w -= px;
    h -= py;

    return *this;
  }

  FOG_INLINE RectD& operator+=(const PointD& other) 
  { 
    x += other.x;
    y += other.y;
    return *this;
  }
  
  FOG_INLINE RectD& operator-=(const PointD& other) 
  { 
    x -= other.x;
    y -= other.y;
    return *this;
  }

  //! @brief Checks if two rectangles overlap.
  //! @return @c true if two rectangles overlap, @c false if two rectangles
  //! do not overlap.
  FOG_INLINE bool overlaps(const RectD& r) const
  {
    return (Math::max(getX1(), r.getX1()) < Math::min(getX1(), r.getX2()) &&
            Math::max(getY1(), r.getY1()) < Math::min(getY2(), r.getY2()) );
  }

  //! @brief Returns @c true if rectangle completely subsumes @a r.
  FOG_INLINE bool subsumes(const RectD& r) const
  {
    return ((r.getX1() >= getX1()) & (r.getX2() <= getX2()) &
            (r.getY1() >= getY1()) & (r.getY2() <= getY2()) );
  }

  //! @brief Returns @c true if given point is in rectangle.
  //! @brief x Point x coordinate.
  //! @brief y Point y coordinate.
  //! @return @c true if point is in rectangle.
  FOG_INLINE bool contains(double px, double py) const
  {
    double ix = px - x;
    double iy = py - y;

    return ((ix >= 0) & (ix < w) & (iy > 0) & (iy < h));
  }

  //! @brief Returns @c true if given point @a pt is in rectangle.
  //! @brief pt Point coordinates.
  //! @return @c true if point is in rectangle.
  FOG_INLINE bool contains(const PointD& other) const
  {
    double ix = other.x - x;
    double iy = other.y - y;

    return ((ix >= 0) & (ix < w) & (iy >= 0) & (iy < h));
  }

  //! @brief Returns @c true if rectangle is valid.
  //!
  //! Rectangle is only valid if it's width and height is equal or larger
  //! than zero (it means that width and height is not negative).
  FOG_INLINE bool isValid() const
  {
    return ((w > 0.0) & (h > 0.0));
  }

  FOG_INLINE bool eq(double px, double py, double pw, double ph) const
  {
    return (x == px) & (y == py) & (w == pw) & (h == ph);
  }

  FOG_INLINE bool eq(const RectD& other) const
  {
    return (x == other.x) & (y == other.y) & (w == other.w) & (h == other.h);
  }

  FOG_INLINE RectD translated(double px, double py) const
  {
    return RectD(x + px, y + py, w, h);
  }

  FOG_INLINE RectD adjusted(double px, double py) const
  {
    return RectD(x + px, y + py, w - px - px, h - py - py);
  }

  static FOG_INLINE bool intersect(RectD& dest, const RectD& src1, const RectD& src2)
  {
    double src1x1 = src1.getX1();
    double src1y1 = src1.getY1();
    double src1x2 = src1.getX2();
    double src1y2 = src1.getY2();
    double src2x1 = src2.getX1();
    double src2y1 = src2.getY1();
    double src2x2 = src2.getX2();
    double src2y2 = src2.getY2();

    dest.x = Math::max(src1x1, src2x1);
    dest.y = Math::max(src1y1, src2y1);
    dest.w = Math::min(src1x2, src2x2) - dest.x;
    dest.h = Math::min(src1y2, src2y2) - dest.y;

    return dest.isValid();
  }

  static FOG_INLINE void unite(RectD& dest, const RectD& src1, const RectD& src2)
  {
    double src1x1 = src1.getX1();
    double src1y1 = src1.getY1();
    double src1x2 = src1.getX2();
    double src1y2 = src1.getY2();
    double src2x1 = src2.getX1();
    double src2y1 = src2.getY1();
    double src2x2 = src2.getX2();
    double src2y2 = src2.getY2();

    dest.x = Math::min(src1x1, src2x1);
    dest.y = Math::min(src1y1, src2y1);
    dest.w = Math::max(src1x2, src2x2) - dest.x;
    dest.h = Math::max(src1y2, src2y2) - dest.y;
  }

  // [Members]

  double x;
  double y;
  double w;
  double h;
};

// ============================================================================
// [Fog::Box]
// ============================================================================

struct Box
{
  // [Construction / Destruction]

  FOG_INLINE Box() 
  {
  }

  FOG_INLINE Box(int px1, int py1, int px2, int py2) :
    x1(px1), y1(py1), x2(px2), y2(py2)
  {
  }

  FOG_INLINE Box(const Box& other) :
    x1(other.x1), y1(other.y1), x2(other.x2), y2(other.y2)
  {
  }

  // [Methods]
  
  // defined later
  FOG_INLINE Box(const Rect& other);

  FOG_INLINE int getX() const { return x1; }
  FOG_INLINE int getY() const { return y1; }
  FOG_INLINE int getWidth() const { return x2 - x1; }
  FOG_INLINE int getHeight() const { return y2 - y1; }

  FOG_INLINE int getX1() const { return x1; }
  FOG_INLINE int getY1() const { return y1; }
  FOG_INLINE int getX2() const { return x2; }
  FOG_INLINE int getY2() const { return y2; }

  FOG_INLINE Point position() const { return Point(x1, y1); }
  FOG_INLINE Size size() const { return Size(x2 - x1, y2 - y1); }

  FOG_INLINE Box& set(const Box &other) 
  { 
    if (sizeof(Box) == 16)
    {
      Memory::copy16B(static_cast<void*>(this), static_cast<const void*>(&other));
    }
    else
    {
      x1 = other.x1;
      y1 = other.y1;
      x2 = other.x2;
      y2 = other.y2;
    }
    return *this;
  }
  
  FOG_INLINE Box& set(int px1, int py1, int px2, int py2) 
  { 
    x1 = px1; 
    y1 = py1; 
    x2 = px2; 
    y2 = py2;

    return *this;
  }

  FOG_INLINE Box& setX(int x) { x1 = x; return *this; }
  FOG_INLINE Box& setY(int y) { y1 = y; return *this; }
  FOG_INLINE Box& setWidth(int w) { x2 = x1 + w; return *this; }
  FOG_INLINE Box& setHeight(int h) { y2 = y1 + h; return *this; }

  FOG_INLINE Box& setX1(int px1) { x1 = px1; return *this; }
  FOG_INLINE Box& setY1(int py1) { y1 = py1; return *this; }
  FOG_INLINE Box& setX2(int px2) { x2 = px2; return *this; }
  FOG_INLINE Box& setY2(int py2) { y2 = py2; return *this; }

  FOG_INLINE Box& translate(int px, int py)
  { 
    x1 += px;
    y1 += py;
    x2 += px;
    y2 += py;
    return *this;
  }

  FOG_INLINE Box& translate(const Point& pt)
  { 
    x1 += pt.x;
    y1 += pt.y;
    x2 += pt.x;
    y2 += pt.y;
    return *this;
  }

  FOG_INLINE Box& clear() 
  {
    if (sizeof(Box) == 16)
    {
      Memory::zero16B(static_cast<void*>(this));
    }
    else
    {
      x1 = 0; 
      y1 = 0; 
      x2 = 0; 
      y2 = 0; 
    }
    return *this;
  }

  FOG_INLINE Box& operator+=(const Point& pt)
  {
    x1 += pt.x;
    y1 += pt.y;
    x2 += pt.x;
    y2 += pt.y;
    return *this;
  }
  
  FOG_INLINE Box& operator-=(const Point& pt)
  {
    x1 -= pt.x;
    y1 -= pt.y;
    x2 -= pt.x;
    y2 -= pt.y;
    return *this;
  }

  //! @brief Checks if two rectangles overlap.
  //! @return @c true if two rectangles overlap, @c false if two rectangles
  //! do not overlap. Remember, x2() and y2() coords aren't in the rectangle.
  FOG_INLINE bool overlaps(const Box& r) const
  {
    return (( ((y1 - r.y2) ^ (y2-r.y1)) &
              ((x1 - r.x2) ^ (x2-r.x1)) ) < 0);
  }

  /*! @brief Returns @c true if rectangle completely subsumes @a r. */
  FOG_INLINE bool subsumes(const Box& r) const
  {
    return ((r.x1 >= x1) & (r.x2 <= x2) &
            (r.y1 >= y1) & (r.y2 <= y2) );
  }

  //! @brief Returns @c true if given point is in rectangle.
  //! @brief x Point x coordinate.
  //! @brief y Point y coordinate.
  //! @return @c true if point is in rectangle.
  FOG_INLINE bool contains(int px, int py) const
  {
    return ((px >= x1) & (py >= y1) & (px < x2) & (py < y2));
  }

  //! @brief Returns @c true if given point @a pt is in rectangle.
  //! @brief x Point x coordinate.
  //! @brief y Point y coordinate.
  //! @return @c true if point is in rectangle.
  FOG_INLINE bool contains(const Point& pt) const
  {
    return ((pt.x >= x1) & (pt.y >= y1) & (pt.x < x2) & (pt.y < y2));
  }

  //! @brief Returns @c true if rectangle is valid.
  //!
  //! Rectangle is only valid if @c x2 is greater than @c x1 and @c y2 is
  //! greater than @c y1.
  FOG_INLINE bool isValid() const
  {
    return (x2 > x1) & (y2 > y1);
  }

  FOG_INLINE bool eq(int px1, int py1, int px2, int py2) const
  {
    return (x1 == px1) & (y1 == py1) & (x2 == px2) & (y2 == py2);
  }

  FOG_INLINE bool eq(const Box& other) const
  {
    if (sizeof(Box) == 16)
      return Memory::eq16B(static_cast<const void*>(this), static_cast<const void*>(&other));
    else
      return (x1 == other.x1) & (y1 == other.y1) & (x2 == other.x2) & (y2 == other.y2);
  }

  //! @brief Shrinks rectangle by @c n coordinates.
  FOG_INLINE Box& shrink(int n)
  {
    x1 += n;
    y1 += n;
    x2 -= n;
    y2 -= n;
    
    return *this;
  }

  //! @brief Expands rectangle by @c n coordinates.
  FOG_INLINE Box& expand(int n)
  {
    x1 -= n;
    y1 -= n;
    x2 += n;
    y2 += n;

    return *this;
  }

  static FOG_INLINE bool intersect(Box& dest, const Box& src1, const Box& src2)
  {
#if defined(FOG_HARDCODE_SSE2)
    static const uint32_t FOG_ALIGN(16) c0[4] = { 0x00000000, 0x00000000, 0xFFFFFFFF, 0xFFFFFFFF };

    __m128i xmm0 = _mm_loadu_si128((__m128i *)(&src1));
    __m128i xmm1 = _mm_loadu_si128((__m128i *)(&src2));
    __m128i xmm2 = _mm_xor_si128(_mm_cmpgt_epi32(xmm0, xmm1), _mm_load_si128((__m128i *)(void*)(&c0[0])));
    xmm0 = _mm_and_si128(xmm0, xmm2);
    xmm2 = _mm_andnot_si128(xmm2, xmm1);
    _mm_storeu_si128((__m128i *)(&dest), _mm_or_si128(xmm0, xmm2));
#else
    dest.set(Math::max(src1.x1, src2.x1),
             Math::max(src1.y1, src2.y1),
             Math::min(src1.x2, src2.x2),
             Math::min(src1.y2, src2.y2));
#endif // FOG_HARDCODE_SSE2
    return dest.isValid();
  }

  // [Members]

  int x1;
  int y1;
  int x2;
  int y2;
};

} // Fog namespace

FOG_INLINE Fog::Rect::Rect(const Fog::Box& box) :
  x(box.x1), y(box.y1), w(box.x2 - box.x1), h(box.y2 - box.y1)
{
}

FOG_INLINE Fog::Box::Box(const Fog::Rect& rect) :
  x1(rect.x), y1(rect.y), x2(rect.x + rect.w), y2(rect.y + rect.h)
{
}

// ============================================================================
// [Operator Overload]
// ============================================================================

FOG_INLINE bool operator==(const Fog::Point& a, const Fog::Point& b) { return  a.eq(b); }
FOG_INLINE bool operator!=(const Fog::Point& a, const Fog::Point& b) { return !a.eq(b); }
FOG_INLINE bool operator< (const Fog::Point& a, const Fog::Point& b) { return (a.y < b.y) | ((a.y <= b.y) & (a.x <  b.x)); }
FOG_INLINE bool operator> (const Fog::Point& a, const Fog::Point& b) { return (a.y > b.y) | ((a.y <= b.y) & (a.x >  b.x)); }
FOG_INLINE bool operator<=(const Fog::Point& a, const Fog::Point& b) { return (a.y < b.y) | ((a.y == b.y) & (a.x <= b.x)); }
FOG_INLINE bool operator>=(const Fog::Point& a, const Fog::Point& b) { return (a.y > b.y) | ((a.y == b.y) & (a.x >= b.x)); }

FOG_INLINE Fog::Point operator+(const Fog::Point& a, const Fog::Point& b) { return Fog::Point(a.x + b.x, a.y + b.y); }
FOG_INLINE Fog::Point operator-(const Fog::Point& a, const Fog::Point& b) { return Fog::Point(a.x - b.x, a.y - b.y); }

FOG_INLINE bool operator==(const Fog::PointD& a, const Fog::PointD& b) { return  a.eq(b); }
FOG_INLINE bool operator!=(const Fog::PointD& a, const Fog::PointD& b) { return !a.eq(b); }
FOG_INLINE bool operator< (const Fog::PointD& a, const Fog::PointD& b) { return (a.y <  b.y) | ((a.y == b.y) & (a.x < b.x)); }
FOG_INLINE bool operator> (const Fog::PointD& a, const Fog::PointD& b) { return (a.y >  b.y) | ((a.y == b.y) & (a.x > b.x)); }
FOG_INLINE bool operator<=(const Fog::PointD& a, const Fog::PointD& b) { return (a.y <= b.y) | ((a.y == b.y) & (a.x <= b.x)); }
FOG_INLINE bool operator>=(const Fog::PointD& a, const Fog::PointD& b) { return (a.y >= b.y) | ((a.y == b.y) & (a.x >= b.x)); }

FOG_INLINE Fog::PointD operator+(const Fog::PointD& a, const Fog::PointD& b) { return Fog::PointD(a.x + b.x, a.y + b.y); }
FOG_INLINE Fog::PointD operator-(const Fog::PointD& a, const Fog::PointD& b) { return Fog::PointD(a.x - b.x, a.y - b.y); }

FOG_INLINE bool operator==(const Fog::Size& a, const Fog::Size& b) { return  a.eq(b); }
FOG_INLINE bool operator!=(const Fog::Size& a, const Fog::Size& b) { return !a.eq(b); }

FOG_INLINE Fog::Size operator+(const Fog::Size& a, const Fog::Size& b) { return Fog::Size(a.w + b.w, a.h + b.h); }
FOG_INLINE Fog::Size operator-(const Fog::Size& a, const Fog::Size& b) { return Fog::Size(a.w - b.w, a.h - b.h); }

FOG_INLINE bool operator==(const Fog::SizeF& a, const Fog::SizeF& b) { return  a.eq(b); }
FOG_INLINE bool operator!=(const Fog::SizeF& a, const Fog::SizeF& b) { return !a.eq(b); }

FOG_INLINE Fog::SizeF operator+(const Fog::SizeF& a, const Fog::SizeF& b) { return Fog::SizeF(a.w + b.w, a.h + b.h); }
FOG_INLINE Fog::SizeF operator-(const Fog::SizeF& a, const Fog::SizeF& b) { return Fog::SizeF(a.w - b.w, a.h - b.h); }

FOG_INLINE bool operator==(const Fog::Rect& a, const Fog::Rect& b) { return  a.eq(b); }
FOG_INLINE bool operator!=(const Fog::Rect& a, const Fog::Rect& b) { return !a.eq(b); }

FOG_INLINE Fog::Rect operator+(const Fog::Rect& a, const Fog::Point& b) { return Fog::Rect(a.x + b.x, a.y + b.y, a.w, a.h); }
FOG_INLINE Fog::Rect operator-(const Fog::Rect& a, const Fog::Point& b) { return Fog::Rect(a.x - b.x, a.y - b.y, a.w, a.h); }

FOG_INLINE bool operator==(const Fog::RectD& a, const Fog::RectD& b) { return  a.eq(b); }
FOG_INLINE bool operator!=(const Fog::RectD& a, const Fog::RectD& b) { return !a.eq(b); }

FOG_INLINE Fog::RectD operator+(const Fog::RectD& a, const Fog::PointD& b) { return Fog::RectD(a.x + b.x, a.y + b.y, a.w, a.h); }
FOG_INLINE Fog::RectD operator-(const Fog::RectD& a, const Fog::PointD& b) { return Fog::RectD(a.x - b.x, a.y - b.y, a.w, a.h); }

FOG_INLINE bool operator==(const Fog::Box& a, const Fog::Box& b) { return  a.eq(b); }
FOG_INLINE bool operator!=(const Fog::Box& a, const Fog::Box& b) { return !a.eq(b); }

FOG_INLINE Fog::Box operator+(const Fog::Box& a, const Fog::Point& b) { return Fog::Box(a.x1 + b.x, a.y1 + b.y, a.x2 + b.x, a.y2 + b.y); }
FOG_INLINE Fog::Box operator-(const Fog::Box& a, const Fog::Point& b) { return Fog::Box(a.x1 - b.x, a.y1 - b.y, a.x2 - b.x, a.y2 - b.y); }

//! @}

// [Fog::TypeInfo<>]
FOG_DECLARE_TYPEINFO(Fog::Point , Fog::PrimitiveType)
FOG_DECLARE_TYPEINFO(Fog::PointD, Fog::PrimitiveType)
FOG_DECLARE_TYPEINFO(Fog::Size  , Fog::PrimitiveType)
FOG_DECLARE_TYPEINFO(Fog::SizeF , Fog::PrimitiveType)
FOG_DECLARE_TYPEINFO(Fog::Rect  , Fog::PrimitiveType)
FOG_DECLARE_TYPEINFO(Fog::RectD , Fog::PrimitiveType)
FOG_DECLARE_TYPEINFO(Fog::Box   , Fog::PrimitiveType)

// [Guard]
#endif // _FOG_GRAPHICS_GEOMETRY_H
