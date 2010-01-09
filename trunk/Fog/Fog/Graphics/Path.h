// [Fog/Graphics Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

//----------------------------------------------------------------------------
// Anti-Grain Geometry - Version 2.4
// Copyright (C) 2002-2005 Maxim Shemanarev (http://www.antigrain.com)
//
// Permission to copy, use, modify, sell and distribute this software
// is granted provided this copyright notice appears in all copies.
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
//----------------------------------------------------------------------------
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://www.antigrain.com
//----------------------------------------------------------------------------

// [Guard]
#ifndef _FOG_GRAPHICS_PATH_H
#define _FOG_GRAPHICS_PATH_H

// [Dependencies]
#include <Fog/Core/Atomic.h>
#include <Fog/Core/List.h>
#include <Fog/Core/Static.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Geometry.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct Matrix;

// ============================================================================
// [Fog::PathCmd]
// ============================================================================

struct FOG_HIDDEN PathCmd
{
  FOG_INLINE PathCmd() {}
  FOG_INLINE PathCmd(const PathCmd& other) : _cmd(other._cmd) {}
  FOG_INLINE PathCmd(uint32_t cmd) : _cmd(cmd) {}

  FOG_INLINE PathCmd& operator=(const PathCmd& other) { _cmd = other._cmd; return *this; }
  FOG_INLINE PathCmd& operator=(uint32_t cmd) { _cmd = cmd; return *this; }

  FOG_INLINE uint32_t cmd() const { return _cmd; }
  FOG_INLINE operator uint32_t() const { return _cmd; }

  FOG_INLINE bool isVertex() const { return _cmd >= PATH_CMD_MOVE_TO && _cmd < PATH_CMD_END; }
  FOG_INLINE bool isDrawing() const { return _cmd >= PATH_CMD_LINE_TO && _cmd < PATH_CMD_END; }

  FOG_INLINE bool isStop() const { return _cmd == PATH_CMD_STOP; }
  FOG_INLINE bool isMoveTo() const { return _cmd == PATH_CMD_MOVE_TO; }
  FOG_INLINE bool isLineTo() const { return _cmd == PATH_CMD_LINE_TO; }
  FOG_INLINE bool isCurve() const { return _cmd == PATH_CMD_CURVE_3 || _cmd == PATH_CMD_CURVE_4; }
  FOG_INLINE bool isCurve3() const { return _cmd == PATH_CMD_CURVE_3; }
  FOG_INLINE bool isCurve4() const { return _cmd == PATH_CMD_CURVE_4; }
  FOG_INLINE bool isEndPoly() const { return (_cmd & PATH_CMD_MASK) == PATH_CMD_END; }
  FOG_INLINE bool isClose() const { return (_cmd & ~(PATH_CFLAG_CW | PATH_CFLAG_CCW)) == (PATH_CMD_END | PATH_CFLAG_CLOSE); }
  FOG_INLINE bool isNextPoly() const { return isStop() || isMoveTo() || isEndPoly(); }
  FOG_INLINE bool isCW() const { return (_cmd & PATH_CFLAG_CW) != 0; }
  FOG_INLINE bool isCCW() const { return (_cmd & PATH_CFLAG_CCW) != 0; }
  FOG_INLINE bool isOriented() const { return (_cmd & (PATH_CFLAG_CW | PATH_CFLAG_CCW)) != 0;  }
  FOG_INLINE bool isClosed() const { return (_cmd & PATH_CFLAG_CLOSE) != 0; }

  uint32_t _cmd;
};

// ============================================================================
// [Fog::PathVertex]
// ============================================================================

struct FOG_HIDDEN PathVertex
{
  PathCmd cmd;
  double x;
  double y;
};

// ============================================================================
// [Fog::Path]
// ============================================================================

//! @brief Path defines graphics path that can be filled or stroked by painter.
struct FOG_API Path
{
  // --------------------------------------------------------------------------
  // [Data]
  // --------------------------------------------------------------------------

  struct FOG_API Data
  {
    // [Flags]

    enum Flags
    {
      IsDynamic = (1 << 0),
      IsSharable = (1 << 1),
      IsStrong = (1 << 2)
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
    int32_t flat;
    sysuint_t capacity;
    sysuint_t length;
    PathVertex data[1];
  };

  static Static<Data> sharedNull;

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  Path();
  Path(const Path& other);
  FOG_INLINE explicit Path(Data* d) : _d(d) {}
  ~Path();

  // --------------------------------------------------------------------------
  // [Implicit Sharing]
  // --------------------------------------------------------------------------

  FOG_INLINE sysuint_t refCount() const { return _d->refCount.get(); }
  FOG_INLINE bool isDetached() const { return _d->refCount.get() == 1; }

  FOG_INLINE err_t detach() { return !isDetached() ? _detach() : ERR_OK; }

  // --------------------------------------------------------------------------
  // [Flags]
  // --------------------------------------------------------------------------

  FOG_INLINE uint32_t getFlags() const { return _d->flags; }

  FOG_INLINE bool isNull() const { return _d == sharedNull.instancep(); }
  FOG_INLINE bool isDynamic() const { return (_d->flags & Data::IsDynamic) != 0; }
  FOG_INLINE bool isSharable() const { return (_d->flags & Data::IsSharable) != 0; }
  FOG_INLINE bool isStrong() const { return (_d->flags & Data::IsStrong) != 0; }

  // --------------------------------------------------------------------------
  // [Data]
  // --------------------------------------------------------------------------

  //! @brief Get path capacity (count of allocated vertices).
  FOG_INLINE sysuint_t getCapacity() const { return _d->capacity; }
  //! @brief Get path length (count of vertices used).
  FOG_INLINE sysuint_t getLength() const { return _d->length; }

  FOG_INLINE bool isEmpty() const { return _d->length == 0; }

  FOG_INLINE const PathVertex* getData() const
  { return _d->data; }

  FOG_INLINE PathVertex* getMData()
  { return detach() == ERR_OK ? _d->data : NULL; }

  err_t reserve(sysuint_t capacity);
  void squeeze();

  PathVertex* _add(sysuint_t count);
  err_t _detach();

  err_t set(const Path& other);
  err_t setDeep(const Path& other);

  void clear();
  void free();

  // --------------------------------------------------------------------------
  // [Bounding Rect]
  // --------------------------------------------------------------------------

  RectD getBoundingRect() const;

  // --------------------------------------------------------------------------
  // [SubPath]
  // --------------------------------------------------------------------------

  //! @brief Get count of vertices in @a subPathId.
  //!
  //! @note Sub path id is simply index of vertex where path starts.
  sysuint_t getSubPathLength(sysuint_t subPathId) const;

  // --------------------------------------------------------------------------
  // [Start / End]
  // --------------------------------------------------------------------------

  err_t start(sysuint_t* index);

  err_t endPoly(uint32_t cmdflags = PATH_CFLAG_CLOSE);
  err_t closePolygon(uint32_t cmdflags = PATH_CFLAG_NONE);

  // --------------------------------------------------------------------------
  // [MoveTo]
  // --------------------------------------------------------------------------

  err_t moveTo(double x, double y);
  err_t moveRel(double dx, double dy);

  FOG_INLINE err_t moveTo(const PointD& pt) { return moveTo(pt.x, pt.y); }
  FOG_INLINE err_t moveRel(const PointD& pt) { return moveRel(pt.x, pt.y); }

  // --------------------------------------------------------------------------
  // [LineTo]
  // --------------------------------------------------------------------------

  err_t lineTo(double x, double y);
  err_t lineRel(double dx, double dy);

  FOG_INLINE err_t lineTo(const PointD& pt) { return lineTo(pt.x, pt.y); }
  FOG_INLINE err_t lineRel(const PointD& pt) { return lineRel(pt.x, pt.y); }

  err_t lineTo(const double* x, const double* y, sysuint_t count);
  err_t lineTo(const PointD* pts, sysuint_t count);

  err_t hlineTo(double x);
  err_t hlineRel(double dx);

  err_t vlineTo(double y);
  err_t vlineRel(double dy);

  // --------------------------------------------------------------------------
  // [ArcTo]
  // --------------------------------------------------------------------------

  err_t _arcTo(double cx, double cy, double rx, double ry, double start, double sweep, uint initialCommand, bool closePath);
  err_t _svgArcTo(
    double rx, double ry,
    double angle,
    bool largeArcFlag,
    bool sweepFlag,
    double x2, double y2,
    uint initialCommand, bool closePath);

  err_t arcTo(double cx, double cy, double rx, double ry, double start, double sweep);
  err_t arcRel(double cx, double cy, double rx, double ry, double start, double sweep);

  FOG_INLINE err_t arcTo(const PointD& cp, const PointD& r, double start, double sweep)
  { return arcTo(cp.x, cp.y, r.x, r.y, start, sweep); }

  FOG_INLINE err_t arcRel(const PointD& cp, const PointD& r, double start, double sweep)
  { return arcRel(cp.x, cp.y, r.x, r.y, start, sweep); }

  err_t svgArcTo(
    double rx, double ry,
    double angle,
    bool largeArcFlag,
    bool sweepFlag,
    double x2, double y2);

  err_t svgArcRel(
    double rx, double ry,
    double angle,
    bool largeArcFlag,
    bool sweepFlag,
    double x2, double y2);

  // --------------------------------------------------------------------------
  // [CurveTo]
  // --------------------------------------------------------------------------

  err_t curveTo(double cx, double cy, double tx, double ty);
  err_t curveRel(double cx, double cy, double tx, double ty);

  err_t curveTo(double tx, double ty);
  err_t curveRel(double tx, double ty);

  FOG_INLINE err_t curveTo(const PointD& cp, const PointD& to)
  { return curveTo(cp.x, cp.y, to.x, to.y); }

  FOG_INLINE err_t curveRel(const PointD& cp, const PointD& to)
  { return curveRel(cp.x, cp.y, to.x, to.y); }

  FOG_INLINE err_t curveTo(const PointD& to)
  { return curveTo(to.x, to.y); }

  FOG_INLINE err_t curveRel(const PointD& to)
  { return curveRel(to.x, to.y); }

  // --------------------------------------------------------------------------
  // [CubicTo]
  // --------------------------------------------------------------------------

  err_t cubicTo(double cx1, double cy1, double cx2, double cy2, double tx, double ty);
  err_t cubicRel(double cx1, double cy1, double cx2, double cy2, double tx, double ty);

  err_t cubicTo(double cx2, double cy2, double tx, double ty);
  err_t cubicRel(double cx2, double cy2, double tx, double ty);

  FOG_INLINE err_t cubicTo(const PointD& cp1, const PointD& cp2, const PointD& to)
  { return cubicTo(cp1.x, cp1.y, cp2.x, cp2.y, to.x, to.y); }

  FOG_INLINE err_t cubicRel(const PointD& cp1, const PointD& cp2, const PointD& to)
  { return cubicRel(cp1.x, cp1.y, cp2.x, cp2.y, to.x, to.y); }

  FOG_INLINE err_t cubicTo(const PointD& cp2, const PointD& to)
  { return cubicTo(cp2.x, cp2.y, to.x, to.y); }

  FOG_INLINE err_t cubicRel(const PointD& cp2, const PointD& to)
  { return cubicRel(cp2.x, cp2.y, to.x, to.y); }

  // --------------------------------------------------------------------------
  // [FlipX / FlipY]
  // --------------------------------------------------------------------------

  err_t flipX(double x1, double x2);
  err_t flipY(double y1, double y2);

  // --------------------------------------------------------------------------
  // [Translate]
  // --------------------------------------------------------------------------

  err_t translate(double dx, double dy);
  err_t translate(double dx, double dy, sysuint_t pathId);

  // --------------------------------------------------------------------------
  // [Scale]
  // --------------------------------------------------------------------------

  err_t scale(double sx, double sy, bool keepStartPos = false);

  // --------------------------------------------------------------------------
  // [ApplyMatrix]
  // --------------------------------------------------------------------------

  //! @brief Apply affine matrix transformations to each vertex in path.
  err_t applyMatrix(const Matrix& matrix);
  //! @brief Apply affine matrix transformations to vertex that starts with
  //! @a index and their length is given in @a length.
  err_t applyMatrix(const Matrix& matrix, sysuint_t index, sysuint_t length);

  // --------------------------------------------------------------------------
  // [Complex]
  // --------------------------------------------------------------------------

  //! @brief Add closed rectangle into path.
  err_t addRect(const RectD& r);

  //! @brief Add rectangles.
  err_t addRects(const RectD* r, sysuint_t count);

  //! @brief Add round.
  err_t addRound(const RectD& r, const PointD& radius);

  //! @brief Add Closed ellipse into path.
  err_t addEllipse(const RectD& r);
  //! @overload
  err_t addEllipse(const PointD& cp, const PointD& r);

  //! @brief Add arc into path.
  err_t addArc(const RectD& r, double start, double sweep);
  //! @overload
  err_t addArc(const PointD& cp, const PointD& r, double start, double sweep);

  //! @brief Add chord into path.
  err_t addChord(const RectD& r, double start, double sweep);
  //! @overload
  err_t addChord(const PointD& cp, const PointD& r, double start, double sweep);

  //! @brief Add pie into path.
  err_t addPie(const RectD& r, double start, double sweep);
  //! @overload
  err_t addPie(const PointD& cp, const PointD& r, double start, double sweep);

  //! @brief Add path.
  err_t addPath(const Path& path);
  //! @brief Add translated path.
  err_t addPath(const Path& path, const PointD& pt);
  //! @brief Add transformed path.
  err_t addPath(const Path& path, const Matrix& matrix);

  // --------------------------------------------------------------------------
  // [Inlines]
  // --------------------------------------------------------------------------

  //! @brief Translate each vertex in path by @a pt.
  FOG_INLINE err_t translate(const PointD& pt) { return translate(pt.x, pt.y); }

  //! @brief Translate each vertex in subpath @a pathId by @a pt.
  FOG_INLINE err_t translate(const PointD& pt, sysuint_t pathId) { return translate(pt.x, pt.y, pathId); }

  //! @brief Scale each vertex in path by @a pt.
  FOG_INLINE err_t scale(const PointD& pt, bool keepStartPos = false) { return scale(pt.x, pt.y, keepStartPos); }

  // --------------------------------------------------------------------------
  // [Flatten]
  // --------------------------------------------------------------------------

  //! @brief Get whether path is flat.
  //!
  //! Path is flat only if it contains only lines (no curves). To make path flat
  //! use @c flatten() or @c flattenTo() methods.
  bool isFlat() const;

  //! @brief Invalidate type of path.
  //!
  //! This method must be called after path was manually changed by getMData() or
  //! similar methods to invalidate type of path.
  FOG_INLINE void resetFlat() const { _d->flat = -1; }

  err_t flatten();
  err_t flatten(const Matrix* matrix, double approximationScale = 1.0);

  err_t flattenTo(Path& dst, const Matrix* matrix, double approximationScale = 1.0) const;
  err_t flattenSubPathTo(Path& dst, sysuint_t subPathId, const Matrix* matrix, double approximationScale = 1.0) const;

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  FOG_INLINE Path& operator=(const Path& other)
  {
    set(other);
    return *this;
  }

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  FOG_DECLARE_D(Data)
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_PATH_H
