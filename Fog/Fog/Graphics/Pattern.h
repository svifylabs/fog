// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_PATTERN_H
#define _FOG_GRAPHICS_PATTERN_H

// [Dependencies]
#include <Fog/Core/Static.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/Graphics/Gradient.h>
#include <Fog/Graphics/Image.h>
#include <Fog/Graphics/Rgba.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Fog::Pattern]
// ============================================================================

//! @brief Pattern can be used to define stroke or fill source when painting.
struct FOG_API Pattern
{
  // [Type]

  enum Type
  {
    TypeGradientMask = 0x10,

    TypeNull = 0x0,
    TypeSolid = 0x1,
    TypeTexture = 0x2,
    TypeLinearGradient = TypeGradientMask | 0x0,
    TypeRadialGradient = TypeGradientMask | 0x1,
    TypeConicalGradient = TypeGradientMask | 0x2
  };

  // [Spread]

  enum Spread
  {
    SpreadPad = 0,
    SpreadRepeat = 1,
    SpreadReflect = 2
  };

  // [Data]

  struct FOG_API Data
  {
    // [Construction / Destruction]
    Data();
    Data(const Data& other);
    ~Data();

    // [Ref / Deref]

    FOG_INLINE Data* ref() const
    {
      refCount.inc();
      return const_cast<Data*>(this);
    }

    FOG_INLINE void deref()
    {
      if (refCount.deref()) delete this;
    }

    Data* copy();
    void deleteResources();

    // [Members]

    mutable Atomic<sysuint_t> refCount;
    uint32_t type;
    uint32_t spread;
    PointF points[2];
    double gradientRadius;

    union Objects {
      Static<Rgba> rgba;
      Static<Image> texture;
      Static<GradientStops> gradientStops;
    } obj;
  };

  static Static<Data> sharedNull;

  // [Construction / Destruction]

  Pattern();
  Pattern(const Pattern& other);
  FOG_INLINE explicit Pattern(Data* d) : _d(d) {}
  ~Pattern();

  // [Implicit Sharing]

  FOG_INLINE sysuint_t refCount() const { return _d->refCount.get(); }
  FOG_INLINE sysuint_t isDetached() const { return _d->refCount.get() == 1; }

  FOG_INLINE err_t detach() { return (_d->refCount.get() > 1) ? _detach() : Error::Ok; }
  err_t _detach();

  void free();

  // [Type]

  FOG_INLINE uint32_t getType() const { return _d->type; }
  err_t setType(uint32_t type);

  FOG_INLINE bool isNull() const { return _d->type == TypeNull; }
  FOG_INLINE bool isSolid() const { return _d->type == TypeSolid; }
  FOG_INLINE bool isTexture() const { return _d->type == TypeTexture; }
  FOG_INLINE bool isGradient() const { return (_d->type & TypeGradientMask) == TypeGradientMask; }
  FOG_INLINE bool isLinearGradient() const { return _d->type == TypeLinearGradient; }
  FOG_INLINE bool isRadialGradient() const { return _d->type == TypeRadialGradient; }
  FOG_INLINE bool isConicalGradient() const { return _d->type == TypeConicalGradient; }

  // [Null]

  void setNull();

  // [Spread]

  FOG_INLINE uint32_t getSpread() const { return _d->spread; }
  err_t setSpread(uint32_t spread);

  // [Start Point / End Point]

  FOG_INLINE const PointF& getStartPoint() const { return _d->points[0]; }
  FOG_INLINE const PointF& getEndPoint() const { return _d->points[1]; }

  err_t setStartPoint(const Point& pt);
  err_t setStartPoint(const PointF& pt);

  err_t setEndPoint(const Point& pt);
  err_t setEndPoint(const PointF& pt);

  err_t setPoints(const Point& startPt, const Point& endPt);
  err_t setPoints(const PointF& startPt, const PointF& endPt);

  // [Solid]

  Rgba getColor() const;
  err_t setColor(const Rgba& rgba);

  // [Texture]

  Image getTexture() const;
  err_t setTexture(const Image& texture);

  // [Gradient]

  FOG_INLINE double gradientRadius() const { return _d->gradientRadius; }

  err_t setGradientRadius(double r);
  err_t setGradientStops(const GradientStops& stops);
  err_t resetGradientStops();
  err_t addGradientStop(const GradientStop& stop);

  // [Operator Overload]

  Pattern& operator=(const Pattern& other);
  Pattern& operator=(const Rgba& rgba);

  // [Members]

  FOG_DECLARE_D(Data)
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_PATTERN_H
