// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_PATH_H
#define _FOG_GRAPHICS_PATH_H

// [Dependencies]
#include <Fog/Core/Atomic.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/Vector.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Geometry.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct AffineMatrix;

// ============================================================================
// [Fog::StrokeParams]
// ============================================================================

struct FOG_HIDDEN StrokeParams
{
  FOG_INLINE StrokeParams() :
    lineWidth(1.0),
    miterLimit(1.0),
    lineCap(LineCapButt),
    lineJoin(LineJoinMiter)
  {
  }

  FOG_INLINE StrokeParams(double lineWidth, double miterLimit, uint32_t lineCap, uint32_t lineJoin) :
    lineWidth(lineWidth),
    miterLimit(miterLimit),
    lineCap(lineCap),
    lineJoin(lineJoin)
  {
  }

  double lineWidth;
  double miterLimit;
  uint32_t lineCap;
  uint32_t lineJoin;
};

// ============================================================================
// [Fog::Path]
// ============================================================================

//! @brief Path defines graphics path that can be filled or stroked by painter.
struct FOG_API Path
{
  // [Cmd]

  enum CmdEnum
  {
    CmdStop     = 0,
    CmdMoveTo   = 1,
    CmdLineTo   = 2,
    CmdCurve3   = 3,
    CmdCurve4   = 4,
    CmdCurveN   = 5,
    CmdCatrom   = 6,
    CmdUBSpline = 7,
    CmdEndPoly  = 0xF,
    CmdMask     = 0xF
  };

  enum CFlag
  {
    CFlagNone  = 0,
    CFlagCCW   = 0x10,
    CFlagCW    = 0x20,
    CFlagClose = 0x40,
    CFlagMask  = 0xF0
  };

  struct FOG_HIDDEN Cmd
  {
    FOG_INLINE Cmd() {}
    FOG_INLINE Cmd(const Cmd& other) : _cmd(other._cmd) {}
    FOG_INLINE Cmd(uint32_t cmd) : _cmd(cmd) {}

    FOG_INLINE Cmd& operator=(const Cmd& other) { _cmd = other._cmd; return *this; }
    FOG_INLINE Cmd& operator=(uint32_t cmd) { _cmd = cmd; return *this; }

    FOG_INLINE uint32_t cmd() const { return _cmd; }
    FOG_INLINE operator uint32_t() { return _cmd; }

    FOG_INLINE bool isVertex() const { return _cmd >= CmdMoveTo && _cmd < CmdEndPoly; }
    FOG_INLINE bool isDrawing() const { return _cmd >= CmdLineTo && _cmd < CmdEndPoly; }

    FOG_INLINE bool isStop() const { return _cmd == CmdStop; }
    FOG_INLINE bool isMoveTo() const { return _cmd == CmdMoveTo; }
    FOG_INLINE bool isLineTo() const { return _cmd == CmdLineTo; }
    FOG_INLINE bool isCurve() const { return _cmd == CmdCurve3 || _cmd == CmdCurve4; }
    FOG_INLINE bool isCurve3() const { return _cmd == CmdCurve3; }
    FOG_INLINE bool isCurve4() const { return _cmd == CmdCurve4; }
    FOG_INLINE bool isEndPoly() const { return (_cmd & CmdMask) == CmdEndPoly; }
    FOG_INLINE bool isClose() const { return (_cmd & ~(CFlagCW | CFlagCCW)) == (CmdEndPoly | CFlagClose); }
    FOG_INLINE bool isNextPoly() const { return isStop() || isMoveTo() || isEndPoly(); }
    FOG_INLINE bool isCW() const { return (_cmd & CFlagCW) != 0; }
    FOG_INLINE bool isCCW() const { return (_cmd & CFlagCCW) != 0; }
    FOG_INLINE bool isOriented() const { return (_cmd & (CFlagCW | CFlagCCW)) != 0;  }
    FOG_INLINE bool isClosed() const { return (_cmd & CFlagClose) != 0; }

    uint32_t _cmd;
  };

  // [Vertex]

  struct FOG_HIDDEN Vertex
  {
    Cmd cmd;
    double x;
    double y;
  };

  // [Type]

  enum Type
  {
    UndefinedType = 0,
    LineType = 1,
    CurveType = 2
  };

  // [Data]

  struct FOG_API Data
  {
    // [Flags]

    enum Flags
    {
      IsNull = (1 << 0),
      IsDynamic = (1 << 1),
      IsSharable = (1 << 2),
      IsStrong = (1 << 3)
    };

    // [Implicit Sharing]

    Data* ref() const;
    void deref();

    FOG_INLINE Data* refAlways() const
    {
      refCount.inc();
      return const_cast<Data*>(this);
    }

    Data* copy() const;

    // [Alloc]

    static Data* alloc(sysuint_t capacity);
    static Data* realloc(Data* d, sysuint_t capacity);

    // [Members]

    mutable Fog::Atomic<sysuint_t> refCount;
    uint32_t flags;
    uint32_t type;
    sysuint_t capacity;
    sysuint_t length;
    Vertex data[1];
  };

  static Static<Data> sharedNull;

  // [Construction / Destruction]

  Path();
  Path(Data* d);
  Path(const Path& other);

  ~Path();

  // [Implicit Sharing]

  FOG_INLINE sysuint_t refCount() const { return _d->refCount.get(); }
  FOG_INLINE bool isDetached() const { return _d->refCount.get() == 1; }

  FOG_INLINE err_t detach()
  { return !isDetached() ? _detach() : Error::Ok; }

  // [Flags]

  FOG_INLINE uint32_t flags() const
  { return _d->flags; }

  FOG_INLINE bool isNull() const
  { return (_d->flags & Data::IsNull) != 0; }

  FOG_INLINE bool isDynamic() const
  { return (_d->flags & Data::IsDynamic) != 0; }

  FOG_INLINE bool isSharable() const
  { return (_d->flags & Data::IsSharable) != 0; }

  FOG_INLINE bool isStrong() const
  { return (_d->flags & Data::IsStrong) != 0; }

  // [Type]

  //! @brief Get type of path.
  //!
  //! Type of path is probably only used by @c Painter to determine which
  //! operations are needed to successfuly rasterize it. This method can
  //! only return @c LineType or CurveType values. @c LineType means that path
  //! contains only lines and no curves and @c CurveType means that path
  //! contains lines and curves.
  uint32_t type() const;

  //! @brief Invalidate type of path.
  //!
  //! This method must be called after path was manually changed by mData() or
  //! similar methods to invalidate type of path.
  FOG_INLINE void invalidateType() const { _d->type = UndefinedType; }

  // [Data]

  //! @brief Get path capacity (count of allocated vertices).
  FOG_INLINE sysuint_t capacity() const { return _d->capacity; }
  //! @brief Get path length (count of vertices used).
  FOG_INLINE sysuint_t length() const { return _d->length; }

  FOG_INLINE bool isEmpty() const { return _d->length == 0; }

  FOG_INLINE const Vertex* cData() const
  { return _d->data; }

  FOG_INLINE Vertex* mData()
  { return detach() == Error::Ok ? _d->data : NULL; }

  err_t reserve(sysuint_t capacity);

  Vertex* _add(sysuint_t count);
  err_t _detach();

  err_t set(const Path& other);
  err_t setDeep(const Path& other);

  void clear();
  void free();

  // [Start / End]

  err_t start(sysuint_t* index);

  err_t endPoly(uint32_t cmdflags = CFlagClose);
  err_t closePolygon(uint32_t cmdflags = CFlagNone);

  // [MoveTo]

  err_t moveTo(double x, double y);
  err_t moveToRel(double dx, double dy);

  FOG_INLINE err_t moveTo(const PointF& pt) { return moveTo(pt.x(), pt.y()); }
  FOG_INLINE err_t moveToRel(const PointF& pt) { return moveToRel(pt.x(), pt.y()); }

  // [LineTo]

  err_t lineTo(double x, double y);
  err_t lineToRel(double dx, double dy);

  FOG_INLINE err_t lineTo(const PointF& pt) { return lineTo(pt.x(), pt.y()); }
  FOG_INLINE err_t lineToRel(const PointF& pt) { return lineToRel(pt.x(), pt.y()); }

  err_t lineTo(const double* x, const double* y, sysuint_t count);
  err_t lineTo(const PointF* pts, sysuint_t count);

  err_t hlineTo(double x);
  err_t hlineToRel(double dx);

  err_t vlineTo(double y);
  err_t vlineToRel(double dy);

  // [ArcTo]

  err_t _arcTo(double cx, double cy, double rx, double ry, double start, double sweep, uint initialCommand, bool closePath);

  err_t arcTo(double cx, double cy, double rx, double ry, double start, double sweep);
  err_t arcToRel(double cx, double cy, double rx, double ry, double start, double sweep);

  FOG_INLINE err_t arcTo(const PointF& cp, const PointF& r, double start, double sweep)
  { return arcTo(cp.x(), cp.y(), r.x(), r.y(), start, sweep); }

  FOG_INLINE err_t arcToRel(const PointF& cp, const PointF& r, double start, double sweep)
  { return arcToRel(cp.x(), cp.y(), r.x(), r.y(), start, sweep); }

  // [BezierTo]

  err_t curveTo(double cx, double cy, double tx, double ty);
  err_t curveToRel(double cx, double cy, double tx, double ty);

  err_t curveTo(double tx, double ty);
  err_t curveToRel(double tx, double ty);

  FOG_INLINE err_t curveTo(const PointF& cp, const PointF& to)
  { return curveTo(cp.x(), cp.y(), to.x(), to.y()); }

  FOG_INLINE err_t curveToRel(const PointF& cp, const PointF& to)
  { return curveToRel(cp.x(), cp.y(), to.x(), to.y()); }

  FOG_INLINE err_t curveTo(const PointF& to)
  { return curveTo(to.x(), to.y()); }

  FOG_INLINE err_t curveToRel(const PointF& to)
  { return curveToRel(to.x(), to.y()); }

  // [CubicTo]

  err_t cubicTo(double cx1, double cy1, double cx2, double cy2, double tx, double ty);
  err_t cubicToRel(double cx1, double cy1, double cx2, double cy2, double tx, double ty);

  err_t cubicTo(double cx2, double cy2, double tx, double ty);
  err_t cubicToRel(double cx2, double cy2, double tx, double ty);

  FOG_INLINE err_t cubicTo(const PointF& cp1, const PointF& cp2, const PointF& to)
  { return cubicTo(cp1.x(), cp1.y(), cp2.x(), cp2.y(), to.x(), to.y()); }

  FOG_INLINE err_t cubicToRel(const PointF& cp1, const PointF& cp2, const PointF& to)
  { return cubicToRel(cp1.x(), cp1.y(), cp2.x(), cp2.y(), to.x(), to.y()); }

  FOG_INLINE err_t cubicTo(const PointF& cp2, const PointF& to)
  { return cubicTo(cp2.x(), cp2.y(), to.x(), to.y()); }

  FOG_INLINE err_t cubicToRel(const PointF& cp2, const PointF& to)
  { return cubicToRel(cp2.x(), cp2.y(), to.x(), to.y()); }

  // [FlipX / FlipY]

  err_t flipX(double x1, double x2);
  err_t flipY(double y1, double y2);

  // [Translate]

  err_t translate(double dx, double dy);
  err_t translate(double dx, double dy, sysuint_t pathId);

  // [Scale]

  err_t scale(double sx, double sy, bool keepStartPos = false);

  // [applyMatrix]

  //! @brief Apply affine matrix transformations to each vertex in path.
  err_t applyMatrix(const AffineMatrix& matrix);

  // [Complex]

  //! @brief Add closed rectangle into path.
  err_t addRect(const RectF& r);

  err_t addRects(const RectF* r, sysuint_t count);

  //! @brief Add Closed ellipse into path.
  err_t addEllipse(const RectF& r);
  //! @overload
  err_t addEllipse(const PointF& cp, const PointF& r);

  //! @brief Add arc into path.
  err_t addArc(const RectF& r, double start, double sweep);
  //! @overload
  err_t addArc(const PointF& cp, const PointF& r, double start, double sweep);

  //! @brief Add chord into path.
  err_t addChord(const RectF& r, double start, double sweep);
  //! @overload
  err_t addChord(const PointF& cp, const PointF& r, double start, double sweep);

  //! @brief Add pie into path.
  err_t addPie(const RectF& r, double start, double sweep);
  //! @overload
  err_t addPie(const PointF& cp, const PointF& r, double start, double sweep);

  //! @brief Add other path into path.
  err_t addPath(const Path& path);

  // [Inlines]

  //! @brief Translate each vertex in path by @a pt.
  FOG_INLINE err_t translate(const PointF& pt)
  { return translate(pt.x(), pt.y()); }

  //! @brief Translate each vertex in subpath @a pathId by @a pt.
  FOG_INLINE err_t translate(const PointF& pt, sysuint_t pathId)
  { return translate(pt.x(), pt.y(), pathId); }

  //! @brief Scale each vertex in path by @a pt.
  FOG_INLINE err_t scale(const PointF& pt, bool keepStartPos = false)
  { return scale(pt.x(), pt.y(), keepStartPos); }

  // [Flatten]

  err_t flatten();
  err_t flatten(const AffineMatrix* matrix, double approximationScale = 1.0);

  //! @brief Similar method to @c flatten(), but with specified destination path.
  err_t flattenTo(Path& dst, const AffineMatrix* matrix, double approximationScale = 1.0) const;

  // [Dash]

  err_t dash(const Vector<double>& dashes, double startOffset, double approximationScale = 1.0);

  //! @brief Similar method to @c dash(), but with specified destination path.
  err_t dashTo(Path& dst, const Vector<double>& dashes, double startOffset, double approximationScale = 1.0);

  // [Stroke]

  //! @brief Stroke the path.
  //!
  //! @note Stroking path will also flatten it.
  err_t stroke(const StrokeParams& strokeParams, double approximationScale = 1.0);

  //! @brief Similar method to @c stroke(), but with specified destination path.
  err_t strokeTo(Path& dst, const StrokeParams& strokeParams, double approximationScale = 1.0) const;

  // [Operator Overload]

  Path& operator=(const Path& other);

  // [Members]

  FOG_DECLARE_D(Data)
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_PATH_H
