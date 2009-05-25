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
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Geometry.h>

//! @addtogroup Fog_Graphics_CAPI
//! @{

namespace Fog {

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

  // [Data]

  FOG_INLINE sysuint_t capacity() const { return _d->capacity; }
  FOG_INLINE sysuint_t length() const { return _d->length; }
  FOG_INLINE bool isEmpty() const { return _d->length == 0; }

  FOG_INLINE const Vertex* cData() const
  { return _d->data; }

  FOG_INLINE Vertex* mData()
  {
    return detach() == Error::Ok ? _d->data : NULL;
  }

  err_t reserve(sysuint_t capacity);

  Vertex* _add(sysuint_t count);
  err_t _detach();

  err_t set(const Path& other);
  err_t setDeep(const Path& other);

  void clear();
  void free();

  err_t start(sysuint_t* index);

  err_t moveTo(double x, double y);
  err_t moveToRel(double dx, double dy);

  err_t lineTo(double x, double y);
  err_t lineToRel(double dx, double dy);

  err_t hlineTo(double x);
  err_t hlineToRel(double dx);

  err_t vlineTo(double y);
  err_t vlineToRel(double dy);

  err_t arcTo(
    double rx, double ry,
    double angle,
    bool largeArcFlag,
    bool sweepFlag,
    double x, double y);

  err_t arcToRel(
    double rx, double ry,
    double angle,
    bool largeArcFlag,
    bool sweepFlag,
    double dx, double dy);

  err_t curve3To(
    double x_ctrl, double y_ctrl, 
    double x_to,   double y_to);

  err_t curve3ToRel(
    double dx_ctrl, double dy_ctrl, 
    double dx_to,   double dy_to);

  err_t curve3To(
    double x_to,    double y_to);

  err_t curve3ToRel(
    double dx_to,   double dy_to);

  err_t curve4To(
    double x_ctrl1, double y_ctrl1,
    double x_ctrl2, double y_ctrl2,
    double x_to,    double y_to);

  err_t curve4ToRel(
    double dx_ctrl1, double dy_ctrl1, 
    double dx_ctrl2, double dy_ctrl2, 
    double dx_to,    double dy_to);

  err_t curve4To(
    double x_ctrl2, double y_ctrl2, 
    double x_to,    double y_to);

  err_t curve4ToRel(
    double dx_ctrl2, double dy_ctrl2, 
    double dx_to,    double dy_to);

  err_t endPoly(uint32_t cmdflags = CFlagClose);
  err_t closePolygon(uint32_t cmdflags = CFlagNone);

  err_t flipX(double x1, double x2);
  err_t flipY(double y1, double y2);

  err_t translate(double dx, double dy);
  err_t translate(double dx, double dy, sysuint_t pathId);

  err_t scale(double sx, double sy, bool keepStartPos = false);

  // [Complex]

  err_t addRect(const RectF& r);
  err_t addLineTo(const PointF* pts, sysuint_t count);
  err_t addEllipse(const PointF& cp, const PointF& r);
  err_t addArc(const PointF& cp, const PointF& r, double start, double sweep);

  // [Inlines]

  FOG_INLINE err_t moveTo(const PointF& pt)
  { return moveTo(pt.x(), pt.y()); }
  
  FOG_INLINE err_t moveToRel(const PointF& pt)
  { return moveToRel(pt.x(), pt.y()); }

  FOG_INLINE err_t lineTo(const PointF& pt)
  { return lineTo(pt.x(), pt.y()); }
  
  FOG_INLINE err_t lineToRel(const PointF& pt)
  { return lineToRel(pt.x(), pt.y()); }

  FOG_INLINE err_t arcTo(
    const PointF& r,
    double angle,
    bool largeArcFlag,
    bool sweepFlag,
    const PointF& pt) 
  { return arcTo(r.x(), r.y(), angle, largeArcFlag, sweepFlag, pt.x(), pt.y()); }

  FOG_INLINE err_t arcToRel(
    const PointF& r,
    double angle,
    bool largeArcFlag,
    bool sweepFlag,
    const PointF& pt)
  { return arcToRel(r.x(), r.y(), angle, largeArcFlag, sweepFlag, pt.x(), pt.y()); }

  FOG_INLINE err_t curve3To(
    const PointF& ctrl,
    const PointF& to)
  { return curve3To(ctrl.x(), ctrl.y(), to.x(), to.y()); }

  FOG_INLINE err_t curve3ToRel(
    const PointF& ctrl,
    const PointF& to)
  { return curve3ToRel(ctrl.x(), ctrl.y(), to.x(), to.y()); }

  FOG_INLINE err_t curve3To(
    const PointF& to)
  { return curve3To(to.x(), to.y()); }

  FOG_INLINE err_t curve3ToRel(
    const PointF& to)
  { return curve3ToRel(to.x(), to.y()); }

  FOG_INLINE err_t curve4To(
    const PointF& ctrl1,
    const PointF& ctrl2,
    const PointF& to)
  { return curve4To(ctrl1.x(), ctrl1.y(), ctrl2.x(), ctrl2.y(), to.x(), to.y()); }

  FOG_INLINE err_t curve4ToRel(
    const PointF& ctrl1,
    const PointF& ctrl2,
    const PointF& to)
  { return curve4ToRel(ctrl1.x(), ctrl1.y(), ctrl2.x(), ctrl2.y(), to.x(), to.y()); }

  FOG_INLINE err_t curve4To(
    const PointF& ctrl2,
    const PointF& to)
  { return curve4To(ctrl2.x(), ctrl2.y(), to.x(), to.y()); }

  FOG_INLINE err_t curve4ToRel(
    const PointF& ctrl2,
    const PointF& to)
  { return curve4ToRel(ctrl2.x(), ctrl2.y(), to.x(), to.y()); }

  FOG_INLINE err_t translate(const Point& pt)
  { return translate(pt.x(), pt.y()); }

  FOG_INLINE err_t translate(const Point& pt, sysuint_t pathId)
  { return translate(pt.x(), pt.y(), pathId); }

  // [Operator Overload]

  Path& operator=(const Path& other);

  // [Members]

  FOG_DECLARE_D(Data)
};

} // Fog namespace

// @}

// [Guard]
#endif // _FOG_GRAPHICS_PATH_H
