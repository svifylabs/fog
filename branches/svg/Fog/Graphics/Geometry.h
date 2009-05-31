// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_GEOMETRY_H
#define _FOG_GRAPHICS_GEOMETRY_H

// [Dependencies]
#include <Fog/Core/Memory.h>
#include <Fog/Core/TypeInfo.h>

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
  FOG_INLINE Point()
  {
  }
  
  FOG_INLINE Point(int x, int y) : _x(x), _y(y)
  {
  }

  FOG_INLINE Point(const Point& other)
  {
    if (sizeof(Point) == 8)
    {
      Fog::Memory::copy8B(static_cast<void*>(this), static_cast<const void*>(&other));
    }
    else
    {
      _x = other.x();
      _y = other.y();
    }
  }

  FOG_INLINE int x() const { return _x; }
  FOG_INLINE int y() const { return _y; }

  FOG_INLINE Point& set(const Point& other) 
  { 
    if (sizeof(Point) == 8)
    {
      Fog::Memory::copy8B(static_cast<void*>(this), static_cast<const void*>(&other));
    }
    else
    {
      _x = other.x();
      _y = other.y();
    }
    return *this;
  }

  FOG_INLINE Point& set(int x, int y)
  { 
    _x = x;
    _y = y;
    return *this;
  }

  FOG_INLINE Point& setX(int x) { _x = x; return *this; }
  FOG_INLINE Point& setY(int y) { _y = y; return *this; }

  FOG_INLINE Point& clear() 
  { 
    _x = 0; 
    _y = 0; 
    return *this;
  }

  FOG_INLINE Point& translate(int x, int y)
  {
    _x += x;
    _y += y;
    return *this;
  }

  FOG_INLINE Point& translate(const Point& other)
  {
    _x += other.x();
    _y += other.y();
    return *this;
  }

  FOG_INLINE Point& negate()
  {
    _x = -_x;
    _y = -_y;
    return *this;
  }

  FOG_INLINE Point& operator+=(const Point& other)
  {
    _x += other.x();
    _y += other.y();
    return *this;
  }

  FOG_INLINE Point& operator-=(const Point& other) 
  {
    _x -= other.x();
    _y -= other.y();
    return *this;
  }

  FOG_INLINE bool eq(int x, int y) const
  {
    return _x == x && _y == y;
  }
  
  FOG_INLINE bool eq(const Point& other) const
  {
    if (sizeof(Point) == 8)
    {
      return Fog::Memory::eq8B(static_cast<const void*>(this), static_cast<const void*>(&other));
    }
    else
    {
      return _x == other.x() && _y == other.y();
    }
  }

  FOG_INLINE Point translated(int x, int y) const
  {
    return Point(_x + x, _y + y);
  }

  FOG_INLINE Point translated(const Point& other) const
  {
    return Point(_x + other.x(), _y + other.y());
  }

  FOG_INLINE Point negated() const 
  {
    return Point(-_x, -_y);
  }

  int _x;
  int _y;
};

// ============================================================================
// [Fog::PointF]
// ============================================================================

struct PointF
{
  FOG_INLINE PointF()
  {
  }
  
  FOG_INLINE PointF(double x, double y) : _x(x), _y(y)
  {
  }

  FOG_INLINE PointF(const PointF& other) : _x(other.x()), _y(other.y())
  {
  }

  FOG_INLINE double x() const { return _x; }
  FOG_INLINE double y() const { return _y; }

  FOG_INLINE PointF& set(const PointF& other)
  {
    _x = other.x();
    _y = other.y();
    return *this;
  }

  FOG_INLINE PointF& set(double x, double y) 
  { 
    _x = x;
    _y = y;
    return *this;
  }

  FOG_INLINE PointF& setX(double x) { _x = x; return *this; }
  FOG_INLINE PointF& setY(double y) { _y = y; return *this; }

  FOG_INLINE PointF& clear()
  { 
    _x = 0.0;
    _y = 0.0;
    return *this;
  }

  FOG_INLINE PointF& translate(double x, double y)
  {
    _x += x;
    _y += y;
    return *this;
  }

  FOG_INLINE PointF& translate(const PointF& other)
  {
    _x += other.x();
    _y += other.y();
    return *this;
  }

  FOG_INLINE PointF& negate()
  {
    _x = -_x;
    _y = -_y;
    return *this;
  }

  FOG_INLINE PointF& operator+=(const PointF& other) 
  {
    _x += other.x();
    _y += other.y();
    return *this;
  }
  
  FOG_INLINE PointF& operator-=(const PointF& other) 
  { 
    _x -= other.x();
    _y -= other.y();
    return *this;
  }

  FOG_INLINE bool eq(double x, double y) const
  { 
    return _x == x && _y == y;
  }

  FOG_INLINE bool eq(const PointF& other) const
  {
    return _x == other.x() && _y == other.y();
  }

  FOG_INLINE PointF translated(double x, double y)
  {
    return PointF(_x + x, _y + y);
  }

  FOG_INLINE PointF translated(const PointF& other)
  {
    return PointF(_x + other.x(), _y + other.y());
  }

  FOG_INLINE PointF negated() const 
  {
    return PointF(-_x, -_y);
  }

  double _x;
  double _y;
};

// ============================================================================
// [Fog::Size]
// ============================================================================

struct Size
{
  FOG_INLINE Size()
  {
  }

  FOG_INLINE Size(int w, int h) : _w(w), _h(h)
  {
  }

  FOG_INLINE Size(const Size& other) : _w(other.width()), _h(other.height())
  {
  }

  FOG_INLINE int w() const { return _w; }
  FOG_INLINE int h() const { return _h; }

  FOG_INLINE int width() const { return _w; }
  FOG_INLINE int height() const { return _h; }

  FOG_INLINE Size& set(const Size& other) 
  {
    _w = other.width();
    _h = other.height();
    return *this;
  }

  FOG_INLINE Size& set(int w, int h)
  {
    _w = w;
    _h = h;
    return *this;
  }

  FOG_INLINE Size& setW(int w) { _w = w; return *this; }
  FOG_INLINE Size& setH(int h) { _h = h; return *this; }

  FOG_INLINE Size& setWidth(int w) { _w = w; return *this; }
  FOG_INLINE Size& setHeight(int h) { _h = h; return *this; }

  FOG_INLINE Size& clear()
  {
    _w = 0;
    _h = 0;
    return *this;
  }

  FOG_INLINE bool isValid() const
  {
    return _w > 0 && _h > 0;
  }
  
  FOG_INLINE bool eq(const Size& other) const
  {
    return _w == other.width() && _h == other.height();
  }
  
  FOG_INLINE bool eq(int w, int h) const
  {
    return _w == w && _h == h;
  }

  FOG_INLINE Size& adjust(int w, int h)
  {
    _w += w;
    _h += h;
    return *this;
  }

  FOG_INLINE Size adjusted(int w, int h) const
  {
    return Size(_w + w, _h + h);
  }

  int _w;
  int _h;
};

// ============================================================================
// [Fog::SizeF]
// ============================================================================

struct SizeF
{
  FOG_INLINE SizeF() 
  {
  }

  FOG_INLINE SizeF(double w, double h) : _w(w), _h(h)
  {
  }

  FOG_INLINE SizeF(const SizeF& other) : _w(other.w()), _h(other.h())
  {
  }

  FOG_INLINE double w() const { return _w; }
  FOG_INLINE double h() const { return _h; }

  FOG_INLINE double width() const { return _w; }
  FOG_INLINE double height() const { return _h; }

  FOG_INLINE SizeF& set(const SizeF& other) 
  {
    _w = other.width();
    _h = other.height();
    return *this;
  }

  FOG_INLINE SizeF& set(double w, double h)
  {
    _w = w;
    _h = h;
    return *this;
  }

  FOG_INLINE SizeF& setW(double w) { _w = w; return *this; }
  FOG_INLINE SizeF& setH(double h) { _h = h; return *this; }

  FOG_INLINE SizeF& setWidth(double w) { _w = w; return *this; }
  FOG_INLINE SizeF& setHeight(double h) { _h = h; return *this; }

  FOG_INLINE SizeF& clear()
  {
    _w = 0.0;
    _h = 0.0;
    return *this; 
  }

  FOG_INLINE bool isValid() const
  {
    return _w > 0.0 && _h > 0.0;
  }

  FOG_INLINE bool eq(const SizeF& other) const
  {
    return _w == other.width() && _h == other.height();
  }
  
  FOG_INLINE bool eq(double w, double h) const
  {
    return _w == w && _h == h;
  }

  FOG_INLINE SizeF& adjust(double w, double h)
  {
    _w += w;
    _h += h;
    return *this;
  }

  FOG_INLINE SizeF adjusted(double w, double h) const
  {
    return SizeF(_w + w, _h + h);
  }

  double _w;
  double _h;
};

// ============================================================================
// [Fog::Rect]
// ============================================================================

struct Rect
{
  FOG_INLINE Rect()
  {
  }

  FOG_INLINE Rect(int x, int y, int w, int h) : 
    _x(x), _y(y), _w(w), _h(h)
  {
  }

  FOG_INLINE Rect(const Rect& other) : 
    _x(other.x()), _y(other.y()), _w(other.width()), _h(other.height())
  {
  }

  // defined later
  FOG_INLINE explicit Rect(const Box& box);

  FOG_INLINE int x() const { return _x; }
  FOG_INLINE int y() const { return _y; }
  FOG_INLINE int w() const { return _w; }
  FOG_INLINE int h() const { return _h; }

  FOG_INLINE int x1() const { return _x; }
  FOG_INLINE int y1() const { return _y; }
  FOG_INLINE int x2() const { return _x+_w; }
  FOG_INLINE int y2() const { return _y+_h; }

  FOG_INLINE int left() const { return x1(); }
  FOG_INLINE int top() const { return y1(); }
  FOG_INLINE int right() const { return x2(); }
  FOG_INLINE int bottom() const { return y2(); }

  FOG_INLINE int width() const { return _w; }
  FOG_INLINE int height() const { return _h; }

  FOG_INLINE const Point& point() const { return *(const Point *)(const void*)(&this->_x); }
  FOG_INLINE const Size& size() const { return *(const Size *)(const void*)(&this->_w); }

  FOG_INLINE Rect& set(int x, int y, int w, int h)
  { 
    _x = x; 
    _y = y; 
    _w = w; 
    _h = h; 

    return *this;
  }

  FOG_INLINE Rect& set(const Rect &other)
  {
    if (sizeof(Rect) == 16)
    {
      Fog::Memory::copy16B(static_cast<void*>(this), static_cast<const void*>(&other));
    }
    else
    {
      _x = other.x();
      _y = other.y();
      _w = other.w();
      _h = other.h();
    }
    return *this;
  }

  FOG_INLINE Rect& set(const Box& box);

  FOG_INLINE Rect& setX(int x) { _x = x; return *this; }
  FOG_INLINE Rect& setY(int y) { _y = y; return *this; }
  FOG_INLINE Rect& setX1(int x1) { _x = x1; return *this; }
  FOG_INLINE Rect& setY1(int y1) { _y = y1; return *this; }
  FOG_INLINE Rect& setX2(int x2) { _w = _x - x2; return *this; }
  FOG_INLINE Rect& setY2(int y2) { _h = _y - y2; return *this; }
  FOG_INLINE Rect& translate(int x, int y) { _x += x; _y += y; return *this; }
  FOG_INLINE Rect& translate(const Point& t) { _x += t.x(); _y += t.y(); return *this; }

  FOG_INLINE Rect& clear() 
  {
    if (sizeof(Rect) == 16)
    {
      Fog::Memory::zero16B(static_cast<void*>(this));
    }
    else
    {
      _x = 0;
      _y = 0;
      _w = 0;
      _h = 0;
    }
    return *this;
  }

  FOG_INLINE Rect& operator+=(const Point& t) 
  { 
    _x += t.x();
    _y += t.y();
    return *this;
  }
  
  FOG_INLINE Rect& operator-=(const Point& t) 
  { 
    _x -= t.x();
    _y -= t.y();
    return *this;
  }

  /*!
    @brief Checks if two rectangles overlap.
    @return @c true if two rectangles overlap, @c false if two rectangles
    do not overlap. Remember, x2() and y2() coords aren't in the rectangle.
  */
  FOG_INLINE bool overlaps(const Rect& r) const
  {
    return (( ((y1()-r.y2()) ^ (y2()-r.y1())) &
              ((x1()-r.x2()) ^ (x2()-r.x1())) ) < 0);
  }

  /*! @brief Returns @c true if rectangle completely subsumes @a r. */
  FOG_INLINE bool subsumes(const Rect& r) const
  {
    return (r.x1() >= x1() && r.x2() <= x2() &&
            r.y1() >= y1() && r.y2() <= y2() );
  }

  /*!
    @brief Returns @c true if given point is in rectangle.
    @brief x Point x coordinate.
    @brief y Point y coordinate.
    @return @c true if point is in rectangle.
  */
  FOG_INLINE bool contains(int x, int y) const
  {
    return (x >= x1() && y >= y1() && x < x2() && y < y2());
  }

  /*!
    @brief Returns @c true if given point @a pt is in rectangle.
    @brief x Point x coordinate.
    @brief y Point y coordinate.
    @return @c true if point is in rectangle.
  */
  FOG_INLINE bool contains(const Point& pt) const
  {
    return (pt.x() >= x1() && pt.y() >= y1() && pt.x() < x2() && pt.y() < y2());
  }

  /*!
    @brief Returns @c true if rectangle is valid.

    Rectangle is only valid if @c x2 is greater than @c x1 and @c y2 is
    greater than @c y1.
  */
  FOG_INLINE bool isValid() const
  {
    return (FOG_LIKELY(_w > 0) && FOG_LIKELY(_h > 0));
  }

  FOG_INLINE bool eq(int x, int y, int w, int h) const
  {
    return _x == x && _y == y && _w == w && _h == h;
  }

  FOG_INLINE bool eq(const Rect& other) const
  {
    if (sizeof(Rect) == 16)
      return Fog::Memory::eq16B(static_cast<const void*>(this), static_cast<const void*>(&other));
    else
      return _x == other.x() && _y == other.y() && _w == other.w() && _h == other.h();
  }

  /*! @brief Shrinks rectangle by @c n coordinates. */
  FOG_INLINE Rect& shrink(int n)
  {
    _x += n;
    _y += n;
    n <<= 1;
    _w -= n;
    _h -= n;
    
    return *this;
  }

  /*! @brief Expands rectangle by @c n coordinates. */
  FOG_INLINE Rect& expand(int n)
  {
    _x -= n;
    _y -= n;
    n <<= 1;
    _w += n;
    _h += n;

    return *this;
  }

  FOG_INLINE Rect& adjust(int ax, int ay)
  {
    _x += ax;
    _y += ay;
    _w -= ax;
    _h -= ay;

    _w -= ax;
    _h -= ay;

    return *this;
  }

  FOG_INLINE Rect& adjust(int ax1, int ay1, int ax2, int ay2)
  {
    _x += ax1;
    _y += ay1;
    _w -= ax1;
    _h -= ay1;

    _w += ax2;
    _h += ay2;

    return *this;
  }

  FOG_INLINE Rect adjusted(int ax, int ay) const 
  {
    return Rect(_x + ax, _y + ay, _w - ax - ax, _h - ay - ay);
  }

  FOG_INLINE Rect adjusted(int ax1, int ay1, int ax2, int ay2) const 
  {
    return Rect(_x + ax1, _y + ay1, _w - ax1 - ax2, _h - ay1 - ay2);
  }

  static FOG_INLINE bool intersect(Rect& dest, const Rect& src1, const Rect& src2)
  {
    int x = fog_max(src1.x1(), src2.x1());
    int y = fog_max(src1.y1(), src2.y1());

    dest.set(
      x,
      y,
      fog_min(src1.x2(), src2.x2()) - x,
      fog_min(src1.y2(), src2.y2()) - y);
    return dest.isValid();
  }

  int _x;
  int _y;
  int _w;
  int _h;
};

// ============================================================================
// [Fog::RectF]
// ============================================================================

struct RectF
{
  FOG_INLINE RectF()
  {
  }

  FOG_INLINE RectF(double x, double y, double w, double h)
    : _x(x), _y(y), _w(w), _h(h)
  {
  }

  FOG_INLINE RectF(const PointF& pt1, const PointF& pt2)
  {
    setPoints(pt1, pt2);
  }

  FOG_INLINE RectF(const RectF& other)
    : _x(other.x()), _y(other.y()), _w(other.w()), _h(other.h())
  {
  }

  FOG_INLINE double x() const { return _x; }
  FOG_INLINE double y() const { return _y; }
  FOG_INLINE double w() const { return _w; }
  FOG_INLINE double h() const { return _h; }

  FOG_INLINE double x1() const { return _x; }
  FOG_INLINE double y1() const { return _y; }
  FOG_INLINE double x2() const { return _x + _w - 1.0; }
  FOG_INLINE double y2() const { return _y + _h - 1.0; }

  FOG_INLINE double width() const { return _w; }
  FOG_INLINE double height() const { return _h; }

  FOG_INLINE double left() const { return x1(); }
  FOG_INLINE double top() const { return y1(); }
  FOG_INLINE double right() const { return x2(); }
  FOG_INLINE double bottom() const { return y2(); }

  FOG_INLINE const PointF& point() const { return *(const PointF *)(const void*)(&this->_x); }
  FOG_INLINE const SizeF& size() const { return *(const SizeF *)(const void*)(&this->_w); }

  FOG_INLINE RectF& set(double x, double y, double w, double h) 
  { 
    _x = x; 
    _y = y; 
    _w = w; 
    _h = h; 
    return *this;
  }

  FOG_INLINE RectF& set(const RectF &other) 
  { 
    _x = other.x(); 
    _y = other.y(); 
    _w = other.w();
    _h = other.h();
    return *this;
  }

  FOG_INLINE RectF& setX(double x) { _x = x; return *this; }
  FOG_INLINE RectF& setY(double y) { _y = y; return *this; }
  FOG_INLINE RectF& setW(double w) { _w = w; return *this; }
  FOG_INLINE RectF& setH(double h) { _h = h; return *this; }

  FOG_INLINE RectF& setPoints(double x1, double y1, double x2, double y2)
  {
    if (x1 <= x2) 
      { _x = x1; _w = x2 - x1 + 1.0; }
    else
      { _x = x2; _w = x1 - x2 + 1.0; }

    if (y1 <= y2) 
      { _y = y1; _h = y2 - y1 + 1.0; }
    else
      { _y = y2; _h = y1 - y2 + 1.0; }

    return *this;
  }

  FOG_INLINE RectF& setPoints(const PointF& pt1, const PointF& pt2)
  {
    return setPoints(pt1.x(), pt1.y(), pt2.x(), pt2.y());
  }

  FOG_INLINE RectF& clear() 
  {
    _x = 0.0;
    _y = 0.0;
    _w = 0.0;
    _h = 0.0;

    return *this;
  }

  FOG_INLINE RectF& translate(double tx, double ty)
  {
    _x += tx;
    _y += ty;
    return *this;
  }
  
  FOG_INLINE RectF& translate(const PointF& pt)
  {
    _x += pt.x();
    _y += pt.y();
    return *this;
  }

  FOG_INLINE RectF& adjust(double ax, double ay)
  {
    _x += ax;
    _y += ay;

    _w -= ax;
    _h -= ay;
    _w -= ax;
    _h -= ay;

    return *this;
  }

  FOG_INLINE RectF& operator+=(const PointF& other) 
  { 
    _x += other.x();
    _y += other.y();
    return *this;
  }
  
  FOG_INLINE RectF& operator-=(const PointF& other) 
  { 
    _x -= other.x();
    _y -= other.y();
    return *this;
  }

  /*!
    @brief Checks if two rectangles overlap.
    @return @c true if two rectangles overlap, @c false if two rectangles
    do not overlap.
  */
  FOG_INLINE bool overlaps(const RectF& r) const
  {
    return (fog_max(x1(), r.x1()) < fog_min(x1(), r.x2()) &&
            fog_max(y1(), r.y1()) < fog_min(y2(), r.y2()) );
  }

  /*! @brief Returns @c true if rectangle completely subsumes @a r. */
  FOG_INLINE bool subsumes(const RectF& r) const
  {
    return (r.x1() >= x1() && r.x2() <= x2() &&
            r.y1() >= y1() && r.y2() <= y2() );
  }

  /*!
    @brief Returns @c true if given point is in rectangle.
    @brief x Point x coordinate.
    @brief y Point y coordinate.
    @return @c true if point is in rectangle.
  */
  FOG_INLINE bool contains(double ox, double oy) const
  {
    double ix = ox - _x;
    double iy = oy - _y;

    return (ix >= 0 && ix < _w && iy >= 0 && iy < _h);
  }

  /*!
    @brief Returns @c true if given point @a pt is in rectangle.
    @brief pt Point coordinates.
    @return @c true if point is in rectangle.
  */
  FOG_INLINE bool contains(const PointF& other) const
  {
    double ix = other.x() - _x;
    double iy = other.y() - _y;

    return (ix >= 0 && ix < _w && iy >= 0 && iy < _h);
  }

  /*!
    @brief Returns @c true if rectangle is valid.

    Rectangle is only valid if it's width and height is equal or larger
    than zero (it means that width and height is not negative).
  */
  FOG_INLINE bool isValid() const
  {
    return (_w > 0.0 && _h > 0.0);
  }

  FOG_INLINE bool eq(double ox, double oy, double ow, double oh) const
  {
    return _x == ox && _y == oy && _w == ow && _h == oh;
  }

  FOG_INLINE bool eq(const RectF& other) const
  {
    return _x == other.x() && _y == other.y() && _w == other.w() && _h == other.h();
  }

  FOG_INLINE RectF translated(double tx, double ty) const
  {
    return RectF(_x + tx, _y + ty, _w, _h);
  }

  FOG_INLINE RectF adjusted(double ax, double ay) const
  {
    return RectF(_x + ax, _y + ay, _w - ax - ax, _h - ay - ay);
  }

  static FOG_INLINE bool intersect(RectF& dest, const RectF& src1, const RectF& src2)
  {
    double src1x1 = src1.x1();
    double src1y1 = src1.y1();
    double src1x2 = src1.x2();
    double src1y2 = src1.y2();
    double src2x1 = src2.x1();
    double src2y1 = src2.y1();
    double src2x2 = src2.x2();
    double src2y2 = src2.y2();

    dest._x = fog_max(src1x1, src2x1);
    dest._y = fog_max(src1y1, src2y1);
    dest._w = fog_min(src1x2, src2x2) - dest._x;
    dest._h = fog_min(src1y2, src2y2) - dest._y;

    return dest.isValid();
  }

  static FOG_INLINE void unite(RectF& dest, const RectF& src1, const RectF& src2)
  {
    double src1x1 = src1.x1();
    double src1y1 = src1.y1();
    double src1x2 = src1.x2();
    double src1y2 = src1.y2();
    double src2x1 = src2.x1();
    double src2y1 = src2.y1();
    double src2x2 = src2.x2();
    double src2y2 = src2.y2();

    dest._x = fog_min(src1x1, src2x1);
    dest._y = fog_min(src1y1, src2y1);
    dest._w = fog_max(src1x2, src2x2) - dest._x;
    dest._h = fog_max(src1y2, src2y2) - dest._y;
  }

  double _x;
  double _y;
  double _w;
  double _h;
};

// ============================================================================
// [Fog::Box]
// ============================================================================

struct Box
{
  FOG_INLINE Box() 
  {
  }

  FOG_INLINE Box(int x1, int y1, int x2, int y2) :
    _x1(x1), _y1(y1), _x2(x2), _y2(y2)
  {
  }

  FOG_INLINE Box(const Box& other) :
    _x1(other.x1()), _y1(other.y1()), _x2(other.x2()), _y2(other.y2())
  {
  }
  
  // defined later
  FOG_INLINE Box(const Rect& other);

  FOG_INLINE int x() const { return _x1; }
  FOG_INLINE int y() const { return _y1; }
  FOG_INLINE int w() const { return _x2 - _x1; }
  FOG_INLINE int h() const { return _y2 - _y1; }

  FOG_INLINE int width() const { return _x2 - _x1; }
  FOG_INLINE int height() const { return _y2 - _y1; }

  FOG_INLINE int x1() const { return _x1; }
  FOG_INLINE int y1() const { return _y1; }
  FOG_INLINE int x2() const { return _x2; }
  FOG_INLINE int y2() const { return _y2; }

  FOG_INLINE Point point() const { return Point(x(), y()); }
  FOG_INLINE Size size() const { return Size(w(), h()); }

  FOG_INLINE Box& set(const Box &other) 
  { 
    if (sizeof(Box) == 16)
    {
      Fog::Memory::copy16B(static_cast<void*>(this), static_cast<const void*>(&other));
    }
    else
    {
      _x1 = other.x1();
      _y1 = other.y1();
      _x2 = other.x2();
      _y2 = other.y2();
    }
    return *this;
  }
  
  FOG_INLINE Box& set(int ox1, int oy1, int ox2, int oy2) 
  { 
    _x1 = ox1; 
    _y1 = oy1; 
    _x2 = ox2; 
    _y2 = oy2;

    return *this;
  }

  FOG_INLINE Box& setX1(int ox1) { _x1 = ox1; return *this; }
  FOG_INLINE Box& setY1(int oy1) { _y1 = oy1; return *this; }
  FOG_INLINE Box& setX2(int ox2) { _x2 = ox2; return *this; }
  FOG_INLINE Box& setY2(int oy2) { _y2 = oy2; return *this; }

  FOG_INLINE Box& translate(int tx, int ty)
  { 
    _x1 += tx;
    _y1 += ty;
    _x2 += tx;
    _y2 += ty;
    return *this;
  }

  FOG_INLINE Box& translate(const Point& t)
  { 
    _x1 += t.x();
    _y1 += t.y();
    _x2 += t.x();
    _y2 += t.y();
    return *this;
  }

  FOG_INLINE Box& clear() 
  {
    if (sizeof(Box) == 16)
    {
      Fog::Memory::zero16B(static_cast<void*>(this));
    }
    else
    {
      _x1 = 0; 
      _y1 = 0; 
      _x2 = 0; 
      _y2 = 0; 
    }
    return *this;
  }

  FOG_INLINE Box& operator+=(const Point& t) 
  {
    _x1 += t.x();
    _y1 += t.y();
    _x2 += t.x();
    _y2 += t.y();
    return *this;
  }
  
  FOG_INLINE Box& operator-=(const Point& t) 
  {
    _x1 -= t.x();
    _y1 -= t.y();
    _x2 -= t.x();
    _y2 -= t.y();
    return *this;
  }

  /*!
    @brief Checks if two rectangles overlap.
    @return @c true if two rectangles overlap, @c false if two rectangles
    do not overlap. Remember, x2() and y2() coords aren't in the rectangle.
  */
  FOG_INLINE bool overlaps(const Box& r) const
  {
    return (( ((_y1-r.y2()) ^ (_y2-r.y1())) &
              ((_x1-r.x2()) ^ (_x2-r.x1())) ) < 0);
  }

  /*! @brief Returns @c true if rectangle completely subsumes @a r. */
  FOG_INLINE bool subsumes(const Box& r) const
  {
    return (r.x1() >= _x1 && r.x2() <= _x2 &&
            r.y1() >= _y1 && r.y2() <= _y2 );
  }

  /*!
    @brief Returns @c true if given point is in rectangle.
    @brief x Point x coordinate.
    @brief y Point y coordinate.
    @return @c true if point is in rectangle.
  */
  FOG_INLINE bool contains(int x, int y) const
  {
    return (x >= _x1 && y >= _y1 && x < _x2 && y < _y2);
  }

  /*!
    @brief Returns @c true if given point @a pt is in rectangle.
    @brief x Point x coordinate.
    @brief y Point y coordinate.
    @return @c true if point is in rectangle.
  */
  FOG_INLINE bool contains(const Point& pt) const
  {
    return (pt.x() >= _x1 && pt.y() >= _y1 && pt.x() < _x2 && pt.y() < _y2);
  }

  /*!
    @brief Returns @c true if rectangle is valid.

    Rectangle is only valid if @c x2 is greater than @c x1 and @c y2 is
    greater than @c y1.
  */
  FOG_INLINE bool isValid() const
  {
    return (FOG_LIKELY(_x2 > _x1) && FOG_LIKELY(_y2 > _y1));
  }

  FOG_INLINE bool eq(int ox1, int oy1, int ox2, int oy2) const
  {
    return _x1 == ox2 && _y1 == oy1 && _x2 == ox2 && _y2 == oy2;
  }

  FOG_INLINE bool eq(const Box& other) const
  {
    if (sizeof(Box) == 16)
      return Fog::Memory::eq16B(static_cast<const void*>(this), static_cast<const void*>(&other));
    else
      return _x1 == other.x2() && _y1 == other.y1() && _x2 == other.x2() && _y2 == other.y2();
  }

  /*! @brief Shrinks rectangle by @c n coordinates. */
  FOG_INLINE Box& shrink(int n)
  {
    _x1 += n;
    _y1 += n;
    _x2 -= n;
    _y2 -= n;
    
    return *this;
  }

  /*! @brief Expands rectangle by @c n coordinates. */
  FOG_INLINE Box& expand(int n)
  {
    _x1 -= n;
    _y1 -= n;
    _x2 += n;
    _y2 += n;

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
    dest.set(
      fog_max(src1.x1(), src2.x1()),
      fog_max(src1.y1(), src2.y1()),
      fog_min(src1.x2(), src2.x2()),
      fog_min(src1.y2(), src2.y2()));
#endif // FOG_HARDCODE_SSE2
    return dest.isValid();
  }

  int _x1;
  int _y1;
  int _x2;
  int _y2;
};

} // Fog namespace

FOG_INLINE Fog::Rect::Rect(const Fog::Box& box) :
  _x(box.x()), _y(box.y()), _w(box.width()), _h(box.height())
{
}

FOG_INLINE Fog::Box::Box(const Fog::Rect& rect) :
  _x1(rect.x1()), _y1(rect.y1()), _x2(rect.x2()), _y2(rect.y2())
{
}

// ============================================================================
// [Operator Overload]
// ============================================================================

FOG_INLINE bool operator==(const Fog::Point& a, const Fog::Point& b) { return  a.eq(b); }
FOG_INLINE bool operator!=(const Fog::Point& a, const Fog::Point& b) { return !a.eq(b); }
FOG_INLINE bool operator< (const Fog::Point& a, const Fog::Point& b) { return a.y() < b.y() || (a.y() <= b.y() && a.x() <  b.x()); }
FOG_INLINE bool operator> (const Fog::Point& a, const Fog::Point& b) { return a.y() > b.y() || (a.y() <= b.y() && a.x() >  b.x()); }
FOG_INLINE bool operator<=(const Fog::Point& a, const Fog::Point& b) { return a.y() < b.y() || (a.y() == b.y() && a.x() <= b.x()); }
FOG_INLINE bool operator>=(const Fog::Point& a, const Fog::Point& b) { return a.y() > b.y() || (a.y() == b.y() && a.x() >= b.x()); }

FOG_INLINE Fog::Point operator+(const Fog::Point& a, const Fog::Point& b) { return Fog::Point( a.x() + b.x(), a.y() + b.y() ); }
FOG_INLINE Fog::Point operator-(const Fog::Point& a, const Fog::Point& b) { return Fog::Point( a.x() - b.x(), a.y() - b.y() ); }

FOG_INLINE bool operator==(const Fog::PointF& a, const Fog::PointF& b) { return  a.eq(b); }
FOG_INLINE bool operator!=(const Fog::PointF& a, const Fog::PointF& b) { return !a.eq(b); }
FOG_INLINE bool operator< (const Fog::PointF& a, const Fog::PointF& b) { return a.y() <  b.y() || (a.y() == b.y() && a.x() < b.x()); }
FOG_INLINE bool operator> (const Fog::PointF& a, const Fog::PointF& b) { return a.y() >  b.y() || (a.y() == b.y() && a.x() > b.x()); }
FOG_INLINE bool operator<=(const Fog::PointF& a, const Fog::PointF& b) { return a.y() <= b.y() || (a.y() == b.y() && a.x() <= b.x()); }
FOG_INLINE bool operator>=(const Fog::PointF& a, const Fog::PointF& b) { return a.y() >= b.y() || (a.y() == b.y() && a.x() >= b.x()); }

FOG_INLINE Fog::PointF operator+(const Fog::PointF& a, const Fog::PointF& b) { return Fog::PointF(a.x() + b.x(), a.y() + b.y()); }
FOG_INLINE Fog::PointF operator-(const Fog::PointF& a, const Fog::PointF& b) { return Fog::PointF(a.x() - b.x(), a.y() - b.y()); }

FOG_INLINE bool operator==(const Fog::Size& a, const Fog::Size& b) { return  a.eq(b); }
FOG_INLINE bool operator!=(const Fog::Size& a, const Fog::Size& b) { return !a.eq(b); }

FOG_INLINE Fog::Size operator+(const Fog::Size& a, const Fog::Size& b) { return Fog::Size(a.w() + b.w(), a.h() + b.h() ); }
FOG_INLINE Fog::Size operator-(const Fog::Size& a, const Fog::Size& b) { return Fog::Size(a.w() - b.w(), a.h() - b.h() ); }

FOG_INLINE bool operator==(const Fog::SizeF& a, const Fog::SizeF& b) { return  a.eq(b); }
FOG_INLINE bool operator!=(const Fog::SizeF& a, const Fog::SizeF& b) { return !a.eq(b); }

FOG_INLINE Fog::SizeF operator+(const Fog::SizeF& a, const Fog::SizeF& b) { return Fog::SizeF(a.w() + b.w(), a.h() + b.h() ); }
FOG_INLINE Fog::SizeF operator-(const Fog::SizeF& a, const Fog::SizeF& b) { return Fog::SizeF(a.w() - b.w(), a.h() - b.h() ); }

FOG_INLINE bool operator==(const Fog::Rect& a, const Fog::Rect& b) { return  a.eq(b); }
FOG_INLINE bool operator!=(const Fog::Rect& a, const Fog::Rect& b) { return !a.eq(b); }

FOG_INLINE Fog::Rect operator+(const Fog::Rect& a, const Fog::Point& b) { return Fog::Rect(a.x() + b.x(), a.y() + b.y(), a.w(), a.h()); }
FOG_INLINE Fog::Rect operator-(const Fog::Rect& a, const Fog::Point& b) { return Fog::Rect(a.x() - b.x(), a.y() - b.y(), a.w(), a.h()); }

FOG_INLINE bool operator==(const Fog::RectF& a, const Fog::RectF& b) { return  a.eq(b); }
FOG_INLINE bool operator!=(const Fog::RectF& a, const Fog::RectF& b) { return !a.eq(b); }

FOG_INLINE Fog::RectF operator+(const Fog::RectF& a, const Fog::PointF& b) { return Fog::RectF(a.x() + b.x(), a.y() + b.y(), a.w(), a.h()); }
FOG_INLINE Fog::RectF operator-(const Fog::RectF& a, const Fog::PointF& b) { return Fog::RectF(a.x() - b.x(), a.y() - b.y(), a.w(), a.h()); }

FOG_INLINE bool operator==(const Fog::Box& a, const Fog::Box& b) { return  a.eq(b); }
FOG_INLINE bool operator!=(const Fog::Box& a, const Fog::Box& b) { return !a.eq(b); }

FOG_INLINE Fog::Box operator+(const Fog::Box& a, const Fog::Point& b) { return Fog::Box(a.x1() + b.x(), a.y1() + b.y(), a.x2() + b.x(), a.y2() + b.y()); }
FOG_INLINE Fog::Box operator-(const Fog::Box& a, const Fog::Point& b) { return Fog::Box(a.x1() - b.x(), a.y1() - b.y(), a.x2() - b.x(), a.y2() - b.y()); }


// [Fog::TypeInfo<>]
FOG_DECLARE_TYPEINFO(Fog::Point , Fog::PrimitiveType)
FOG_DECLARE_TYPEINFO(Fog::PointF, Fog::PrimitiveType)
FOG_DECLARE_TYPEINFO(Fog::Size  , Fog::PrimitiveType)
FOG_DECLARE_TYPEINFO(Fog::SizeF , Fog::PrimitiveType)
FOG_DECLARE_TYPEINFO(Fog::Rect  , Fog::PrimitiveType)
FOG_DECLARE_TYPEINFO(Fog::RectF , Fog::PrimitiveType)
FOG_DECLARE_TYPEINFO(Fog::Box   , Fog::PrimitiveType)

// [Guard]
#endif // _FOG_GRAPHICS_GEOMETRY_H
