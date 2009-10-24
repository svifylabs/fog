// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_CONSTANTS_H
#define _FOG_GRAPHICS_CONSTANTS_H

// [Dependencies]
#include <Fog/Build/Build.h>

namespace Fog {

// ============================================================================
// [Fog::CompositeOp]
// ============================================================================

//! @brief Image compositing operator.
//!
//! @note Many values and formulas are from antigrain and from SVG specification
//! that can be found here: 
//! - http://www.w3.org/TR/2004/WD-SVG12-20041027/rendering.html
enum CompositeOp
{
  // --------------------------------------------------------------------------
  // [SRC]
  // --------------------------------------------------------------------------

  //! @brief The source is copied to the destination.
  //!
  //! The source pixel is copied to destination pixel. If destination pixel 
  //! format not supports alpha channel, the source alpha value is ignored. 
  //! If there is alpha channel mask the composition is done using LERP 
  //! operator.
  //!
  //! Formulas for PRGB(dst), PRGB(src) colorspaces (SRC):
  //!   Dca' = Sca.Da + Sca.(1 - Da)
  //!        = Sca
  //!   Da'  = Sa.Da + Sa.(1 - Da)
  //!        = Sa
  //!
  //!   Msk:
  //!
  //!   Dca' = Sca.m + Dca.(1 - m)
  //!   Da'  = Sa.m + Da.(1-m)
  //!
  //! Formulas for PRGB(dst), RGB(src) colorspaces (SRC):
  //!   Dca' = Sc
  //!   Da'  = 1
  //!
  //!   Msk:
  //!
  //!   Dca' = Sc.m + Dca.(1 - m)
  //!   Da'  = 1.m + Da.(1 - m)
  //!
  //! Formulas for ARGB(dst), PRGB(src) colorspaces (SRC):
  //!   Dc'  = Sca / Da'
  //!   Da'  = Sa
  //!
  //!   Msk:
  //!
  //!   Dc'  = (Sca.m + Dc.Da.(1 - m)) / Da'
  //!   Da'  = Sa.m + Da.(1 - m)
  //!
  //! Formulas for ARGB(dst), ARGB(src) colorspaces (SRC):
  //!   Dc'  = Sc
  //!   Da'  = Sa
  //!
  //!   Msk:
  //!
  //!   Dc'  = (Sc.Sa.m + Dc.Da.(1 - m)) / Da'
  //!   Da'  = Sa.m + Da.(1 - m)
  //!
  //! Formulas for ARGB(dst), RGB(src) colorspaces (SRC):
  //!   Dc'  = Sc
  //!   Da'  = 1
  //!
  //!   Msk:
  //!
  //!   Dc'  = (Sc.m + Dc.Da.(1 - m)) / Da'
  //!   Da'  = m + Da.(1 - m)
  //!
  //! Formulas for RGB(dst), PRGB(src) colorspaces (SRC):
  //!   Dc'  = Sca
  //!
  //!   Msk:
  //!
  //!   Dc'  = Sca.m + Dc.(1 - m)
  //!
  //! Formulas for RGB(dst), RGB(src) colorspaces (SRC):
  //!   Dc'  = Sc
  //!
  //!   Msk:
  //!
  //!   Dc'  = Sc.m + Dc.(1 - m)
  //!
  //! Formulas for A(dst), A(src) colorspaces (SRC):
  //!   Da'  = Sa
  //!
  //!   Msk:
  //!
  //!   Da'  = Sa.m + Da.(1 - m)
  CompositeSrc = 0,

  // --------------------------------------------------------------------------
  // [DST]
  // --------------------------------------------------------------------------

  //! @brief The destination is left untouched.
  //!
  //! Destination pixels remains unchanged.
  //!
  //! Formulas for PRGB(dst), PRGB(src) colorspaces (NOP):
  //!   Dca' = Dca
  //!   Da'  = Da
  //!
  //! Formulas for PRGB(dst), RGB(src) colorspaces (NOP):
  //!   Dca' = Dca
  //!   Da'  = Da
  //!
  //! Formulas for ARGB(dst), PRGB(src) colorspaces (NOP):
  //!   Dc'  = Dc
  //!   Da'  = Da
  //!
  //! Formulas for ARGB(dst), ARGB(src) colorspaces (NOP):
  //!   Dc'  = Dc
  //!   Da'  = Da
  //!
  //! Formulas for ARGB(dst), RGB(src) colorspaces (NOP):
  //!   Dc'  = Dc
  //!   Da'  = Da
  //!
  //! Formulas for RGB(dst), PRGB(src) colorspaces (NOP):
  //!   Dc'  = Dc
  //!
  //! Formulas for RGB(dst), RGB(src) colorspaces (NOP):
  //!   Dc'  = Dc
  //!
  //! Formulas for A(dst), A(src) colorspaces (NOP):
  //!   Da'  = Da
  CompositeDest,

  // --------------------------------------------------------------------------
  // [SRC_OVER]
  // --------------------------------------------------------------------------

  //! @brief The source is composited over the destination.
  //!
  //! Formulas for PRGB(dst), PRGB(src) colorspaces (SRC_OVER):
  //!   Dca' = Sca.Da + Sca.(1 - Da) + Dca.(1 - Sa)
  //!        = Sca + Dca.(1 - Sa)
  //!   Da'  = Sa.Da + Sa.(1 - Da) + Da.(1 - Sa)
  //!        = Sa + Da.(1 - Sa)
  //!        = Sa + Da - Sa.Da
  //!
  //!   Msk:
  //!
  //!   Dca' = (Sca + Dca.(1 - Sa)).m + Dca.(1 - m)
  //!        = Sca.m + Dca.(1 - Sa.m)
  //!   Da'  = (Sa + Da.(1 - Sa)).m + Da .(1 - m)
  //!        = Sa.m + Da.(1 - Sa.m)
  //!
  //! Formulas for PRGB(dst), RGB(src) colorspaces (SRC):
  //!   Dca' = Sc
  //!   Da'  = 1
  //!
  //!   Msk:
  //!
  //!   Dca' = Sc.m + Dca.(1 - m)
  //!   Da'  = 1.m + Da.(1 - m)
  //!
  //! Formulas for ARGB(dst), PRGB(src) colorspaces (SRC_OVER):
  //!   Dc'  = (Sca + Dc.Da.(1 - Sa)) / Da'
  //!   Da'  = Sa + Da.(1 - Sa)
  //!
  //!   Msk:
  //!
  //!   Dc'  = ((Sca + Dc.Da.(1 - Sa)).m + Dc.Da.(1 - m)) / Da'
  //!        = (Sca.m + Dc.Da.(1 - Sa.m)) / Da'
  //!   Da'  = (Sa + Da.(1 - Sa)).m + Da .(1 - m)
  //!        = Sa.m + Da.(1 - Sa.m)
  //!
  //! Formulas for ARGB(dst), ARGB(src) colorspaces (SRC_OVER):
  //!   Dc'  = (Sc.Sa + Dc.Da.(1 - Sa)) / Da'
  //!        = (Da.(Dc - Dc.Sa) + Sa.Sc) / Da'
  //!   Da'  = Sa + Da.(1 - Sa)
  //!
  //!   Msk:
  //!
  //!   Dc'  = ((Sc.Sa + Dc.Da.(1 - Sa)).m + Dc.Da.(1 - m)) / Da'
  //!        = (Sc.Sa.m + Dc.Da.(1 - Sa.m)) / Da'
  //!   Da'  = (Sa + Da.(1 - Sa)).m + Da .(1 - m)
  //!        = Sa.m + Da.(1 - Sa.m)
  //!
  //! Formulas for ARGB(dst), RGB(src) colorspaces (SRC):
  //!   Dc'  = Sc
  //!   Da'  = 1
  //!
  //!   Msk:
  //!
  //!   Dca' = (Sc.m + Dc.Da.(1 - m)) / Da'
  //!   Da'  = 1.m + Da.(1 - m)
  //!
  //! Formulas for RGB(dst), PRGB(src) colorspaces (SRC):
  //!   Dc'  = Sca
  //!
  //!   Msk:
  //!
  //!   Da'  = Sca.m + Dc.(1 - m)
  //!
  //! Formulas for RGB(dst), RGB(src) colorspaces (SRC):
  //!   Dc'  = Sc
  //!
  //!   Msk:
  //!
  //!   Da'  = Sc.m + Dc.(1 - m)
  //!
  //! Formulas for A(dst), A(src) colorspaces (SRC_OVER):
  //!   Da'  = Da + Sa.(1 - Da)
  //!
  //!   Msk:
  //!
  //!   Da'  = Da + Sa.m.(1 - Da)
  //!
  //! If the source is full opaque (Sa == 1.0 or there is no alpha), it 
  //! replaces the destination.
  CompositeSrcOver,

  // --------------------------------------------------------------------------
  // [DST_OVER]
  // --------------------------------------------------------------------------

  //! @brief The destination is composited over the source and the result 
  //! replaces the destination. 
  //!
  //! Formulas for PRGB(dst), PRGB(src) colorspaces:
  //!   Dca' = Dca.Sa + Sca.(1 - Da) + Dca.(1 - Sa)
  //!        = Dca + Sca.(1 - Da)
  //!   Da'  = Da.Sa + Sa.(1 - Da) + Da.(1 - Sa)
  //!        = Da + Sa.(1 - Da)
  //!        = Da + Sa - Da.Sa
  //!
  //!   Msk:
  //!
  //!   Dca' = (Dca + Sca.(1 - Da)).m + Dca.(1 - m)   => Dca + Sca.(1 - Da).m
  //!   Da'  = (Da  + Sa .(1 - Da)).m + Da .(1 - m)   => Da  + Sa .(1 - Da).m
  //!
  //! Formulas for PRGB(dst), RGB(src) colorspaces:
  //!   Dca' = Dca + Sc.(1 - Da)
  //!   Da'  = 1
  //!
  //!   Msk:
  //!
  //!   Dca' = Dca + Sc.m.(1 - Da)
  //!   Da'  = Da + Sa.m.(1 - Da)
  //!
  //! Formulas for ARGB(dst), PRGB(src) colorspaces:
  //!   Dc'  = (Dc.Da + Sca.(1 - Da)) / Da'
  //!   Da'  = Da + Sa.(1 - Da)
  //!
  //!   Msk:
  //!
  //!   Dc'  = (Dc.Da + Sca.m.(1 - Da)) / Da'
  //!   Da'  = Da + Sa.m.(1 - Da)
  //!
  //! Formulas for ARGB(dst), ARGB(src) colorspaces:
  //!   Dc'  = (Dc.Da + Sc.Sa.(1 - Da)) / Da'
  //!   Da'  = Da + Sa.(1 - Da)
  //!
  //!   Msk:
  //!
  //!   Dc'  = (Dc.Da + Sc.Sa.m.(1 - Da)) / Da'
  //!   Da'  = Da + Sa.m.(1 - Da)
  //!
  //! Formulas for ARGB(dst), RGB(src) colorspaces:
  //!   Dc'  = Dc.Da + Sc.(1 - Da) -> LERP(Dc, Sc, Da)
  //!   Da'  = 1
  //!
  //!   Msk:
  //!
  //!   Dca' = (Dc.Da + Sc.m.(1 - Da)) / Da'
  //!   Da'  = Da + m.(1-Da) 
  //!
  //! Formulas for RGB(dst), PRGB(src) colorspaces:
  //!   Dc'  = Dc
  //!
  //!   Msk:
  //!
  //!   Dc ' = Dc
  //!
  //! Formulas for RGB(dst), RGB(src) colorspaces:
  //!   Dc'  = Dc
  //!
  //!   Msk:
  //!
  //!   Dc ' = Dc
  //!
  //! Formulas for A(dst), A(src) colorspaces:
  //!   Da'  = Da + Sa.(1 - Da) (standard formula used many times)
  //!
  //!   Msk:
  //!
  //!   Da'  = Da + Sa.m.(1 - Da)
  //!
  //! If the destination is full opaque (Da == 1.0 or there is no aplha), there
  //! is no change.
  CompositeDestOver,

  // --------------------------------------------------------------------------
  // [SRC_IN]
  // --------------------------------------------------------------------------

  //! @brief The part of the source lying inside of the destination replaces
  //! the destination. 
  //!
  //! Formulas for PRGB(dst), PRGB(src) colorspaces (SRC_IN):
  //!   Dca' = Sca.Da
  //!   Da'  = Sa.Da
  //!
  //!   Msk:
  //!
  //!   Dca' = Sca.Da.m + Dca.(1 - m)
  //!   Da'  = Sa .Da.m + Da .(1 - m)
  //! 
  //! Formulas for PRGB(dst), RGB(src) colorspaces (?):
  //!   Dca' = Sc.Da
  //!   Da'  = Da
  //!
  //!   Msk:
  //!
  //!   Dca' = Sc.Da.m + Dca.(1 - m)
  //!   Da'  = 1 .Da.m + Da .(1 - m)
  //!        = Da
  //!
  //! Formulas for ARGB(dst), PRGB(src) colorspaces (SRC_IN):
  //!   Dc'  = Sca/Sa
  //!   Da'  = Sa.Da
  //!
  //!   Msk:
  //!
  //!   Dc'  = (Sca.Da.m + Dc.Da.(1 - m)) / Da'
  //!   Da'  = Sa.Da.m + Da .(1 - m)
  //!
  //! Formulas for ARGB(dst), ARGB(src) colorspaces (SRC_IN):
  //!   Dc'  = Sc
  //!   Da'  = Sa.Da
  //!
  //!   Msk:
  //!
  //!   Dc'  = (Sc.Sa.Da.m + Dc.Da.(1 - m)) / Da'
  //!   Da'  = Sa.Da.m + Da.(1 - m)
  //!
  //! Formulas for ARGB(dst), RGB(src) colorspaces (?):
  //!   Dc'  = Sc
  //!   Da'  = Da
  //!
  //!   Msk:
  //!
  //!   Dc'  = (Sc.m + Dc.(1 - m))
  //!   Da'  = Da.m + Da.(1 - m) => Da
  //!
  //! Formulas for RGB(dst), PRGB(src) colorspaces (SRC):
  //!   Dc'  = Sca
  //!
  //!   Msk:
  //!
  //!   Dc'  = (Sca.m + Dc.(1 - m))
  //!
  //! Formulas for RGB(dst), RGB(src) colorspaces (SRC):
  //!   Dc'  = Sc
  //!
  //!   Msk:
  //!
  //!   Dc'  = (Sc.m + Dc.(1 - m))
  //!
  //! Formulas for A(dst), A(src) colorspaces (MUL):
  //!   Da'  = Sa.Da
  //!
  //!   Msk:
  //!
  //!   Da'  = Sa.Da.m + Da.(1 - m)
  //!        = Da.(Sa.m + 1 - m)
  CompositeSrcIn,

  // --------------------------------------------------------------------------
  // [DST_IN]
  // --------------------------------------------------------------------------

  //! @brief The part of the destination lying inside of the source replaces
  //! the destination. 
  //!
  //! Formulas for PRGB(dst), PRGB(src) colorspaces (SRC IN):
  //!   Dca' = Dca.Sa
  //!   Da'  = Da.Sa
  //!
  //!   Msk:
  //!
  //!   Dca' = Dca.Sa.m + Dca.(1 - m)
  //!   Da'  = Da.Sa.m + Da.(1 - m)
  //!
  //! Formulas for PRGB(dst), RGB(src) colorspaces (NOP):
  //!   Dca' = Dca
  //!   Da'  = Da
  //!
  //! Formulas for ARGB(dst), PRGB(src) colorspaces (SRC IN):
  //!   Dc'  = Dc
  //!   Da'  = Da.Sa
  //!
  //!   Msk:
  //!
  //!   Dc'  = (Dc.Sa.m + Dc.(1 - m)) / Da'
  //!   Da'  = Sa.m + (1 - m)
  //!
  //! Formulas for ARGB(dst), ARGB(src) colorspaces (SRC IN):
  //!   Dc'  = Dc
  //!   Da'  = Da.Sa
  //!
  //!   Msk:
  //!
  //!   Dc'  = (Dc.Sa.m + Dc.(1 - m)) / Da'
  //!   Da'  = Sa.m + (1 - m)
  //!
  //! Formulas for ARGB(dst), RGB(src) colorspaces (NOP):
  //!   Dc'  = Dc
  //!   Da'  = Da
  //!
  //! Formulas for RGB(dst), PRGB(src) colorspaces (CLEAR):
  //!   Dc'  = Dc.Sa
  //!
  //!   Msk:
  //!
  //!   Dc'  = Dc.Sa.m + Dc.(m - 1)
  //!
  //! Formulas for RGB(dst), RGB(src) colorspaces (NOP):
  //!   Dc'  = Dc
  //!
  //! Formulas for A(dst), A(src) colorspaces (MUL):
  //!   Da'  = Sa.Da
  //!
  //!   Msk:
  //!
  //!   Da'  = Sa.Da.m + Da.(1 - m)
  //!        = Da.(Sa.m + 1 - m)
  CompositeDestIn,

  // --------------------------------------------------------------------------
  // [SRC_OUT]
  // --------------------------------------------------------------------------

  //! @brief The part of the source lying outside of the destination replaces
  //! the destination. 
  //!
  //! Formulas for PRGB(dst), PRGB(src) colorspaces (SRC_OUT):
  //!   Dca' = Sca.(1 - Da)
  //!   Da'  = Sa.(1 - Da)
  //!
  //!   Msk:
  //!
  //!   Dca' = Sca.(1 - Da).m + Dca.(1 - m)
  //!   Da'  = Sa.(1 - Da).m + Da.(1 - m)
  //!
  //! Formulas for PRGB(dst), RGB(src) colorspaces (?):
  //!   Dca' = Sc.(1 - Da)
  //!   Da'  = (1 - Da)
  //!
  //!   Msk:
  //!
  //!   Dca' = Sc.(1 - Da).m + Dca.(1 - m)
  //!   Da'  = (1 - Da).m + Da.(m - 1)
  //!
  //! Formulas for ARGB(dst), PRGB(src) colorspaces (SRC_OUT):
  //!   Dc'  = (Sca/Sa).(1 - Da)
  //!   Da'  = Sa.(1 - Da)
  //!
  //!   Msk:
  //!
  //!   Dca' = (Sca.(1 - Da).m + Dc.Da.(1 - m)) / Da'
  //!   Da'  = Sa.(1 - Da).m + Da.(1 - m)
  //!
  //! Formulas for ARGB(dst), ARGB(src) colorspaces (SRC_OUT):
  //!   Dc'  = Sc
  //!   Da'  = Sa.(1 - Da)
  //!
  //!   Msk:
  //!
  //!   Dca' = (Sc.Sa.(1 - Da).m + Dc.Da.(1 - m)) / Da'
  //!   Da'  = Sa.(1 - Da).m + Da.(1 - m)
  //!
  //! Formulas for ARGB(dst), RGB(src) colorspaces (?):
  //!   Dc'  = Sc
  //!   Da'  = (1 - Da)
  //!
  //!   Msk:
  //!
  //!   Dca' = (Sc.(1 - Da).m + Dc.Da.(1 - m)) / Da'
  //!   Da'  = 1.(1 - Da).m + Da.(1 - m)
  //!
  //! Formulas for RGB(dst), PRGB(src) colorspaces (CLEAR):
  //!   Dc'  = 0
  //!
  //!   Msk:
  //!
  //!   Dc'  = Dca.(1 - m)
  //!
  //! Formulas for RGB(dst), RGB(src) colorspaces (CLEAR):
  //!   Dc'  = 0
  //!
  //!   Msk:
  //!
  //!   Dc'  = Dc.(1 - m)
  //!
  //! Formulas for A(dst), A(src) colorspaces (SRC_OUT):
  //!   Da'  = Sa.(1 - Da)
  //!
  //!   Msk:
  //!
  //!   Da'  = Sa.(1 - Da).m + Da.(1 - m)
  CompositeSrcOut,

  // --------------------------------------------------------------------------
  // [DST_OUT]
  // --------------------------------------------------------------------------

  //! @brief The part of the destination lying outside of the source replaces
  //! the destination. 
  //!
  //! Formulas for PRGB(dst), PRGB(src) colorspaces (SRC_OUT):
  //!   Dca' = Dca.(1 - Sa)
  //!   Da'  = Da.(1 - Sa)
  //!
  //!   Msk:
  //!
  //!   Dca' = Dca.(1 - Sa).m + Dca.(1 - m)  
  //!        = Dca - Sa.m
  //!   Da'  = Da.(1 - Sa).m + Da.(1 - m)
  //!        = Da - Sa.m
  //!
  //! Formulas for PRGB(dst), RGB(src) colorspaces (CLEAR):
  //!   Dca' = 0
  //!   Da'  = 0
  //!
  //!   Msk:
  //!
  //!   Dca' = Dca.(m - 1)
  //!   Da'  = Da.(m - 1)
  //!
  //! Formulas for ARGB(dst), PRGB(src) colorspaces (SRC_OUT):
  //!   Dc'  = Dc
  //!   Da'  = Da.(1 - Sa)
  //!
  //!   Msk:
  //!
  //!   Dc'  = Dc
  //!   Da'  = Da - Sa.m
  //!
  //! Formulas for ARGB(dst), ARGB(src) colorspaces (SRC_OUT):
  //!   Dc'  = Dc
  //!   Da'  = Da.(1 - Sa)
  //!
  //!   Msk:
  //!
  //!   Dc'  = Dc
  //!   Da'  = Da - Sa.m
  //!
  //! Formulas for ARGB(dst), RGB(src) colorspaces (CLEAR):
  //!   Dc'  = 0
  //!   Da'  = 0
  //!
  //!   Msk:
  //!
  //!   Dca' = Dc.(m - 1)
  //!   Da'  = Da.(m - 1)
  //!
  //! Formulas for RGB(dst), PRGB(src) colorspaces (CLEAR):
  //!   Dc'  = 0
  //!
  //!   Msk:
  //!
  //!   Dc'  = Dc.(m - 1)
  //!   Da'  = Da.(m - 1)
  //!
  //! Formulas for RGB(dst), RGB(src) colorspaces (CLEAR):
  //!   Dc'  = 0
  //!
  //!   Msk:
  //!
  //!   Dc'  = Dc.(m - 1)
  //!   Da'  = Da.(m - 1)
  //!
  //! Formulas for A(dst), A(src) colorspaces (SRC_OUT):
  //!   Da'  = Da.(1 - Sa)
  CompositeDestOut,

  // --------------------------------------------------------------------------
  // [SRC_ATOP]
  // --------------------------------------------------------------------------

  //! @brief The part of the source lying inside of the destination is
  //! composited onto the destination. 
  //!
  //! Formulas for PRGB(dst), PRGB(src) colorspaces (ATOP):
  //!   Dca' = Sca.Da + Dca.(1 - Sa)
  //!   Da'  = Sa.Da + Da.(1 - Sa) 
  //!        = Sa.Da + Da - Da.Sa
  //!        = Da
  //!
  //!   Msk:
  //!
  //!   Dca' = (Sca.Da + Dca.(1 - Sa)).m + Dca.(1 - m)
  //!        = Sca.Da.m + Dca.(1 - Sa).m + Dca.(1 - m)
  //!        = Sca.Da.m + Dca.(1 - Sa.m)
  //!   Da'  = Sa.Da.m + Da.(1 - Sa.m)
  //!        = Da
  //!
  //! Formulas for PRGB(dst), RGB(src) colorspaces (?):
  //!   Dca' = Sc.Da
  //!   Da'  = Da
  //!
  //!   Msk:
  //!
  //!   Dca' = Sc.Da.m + Dca.(1 - m)
  //!   Da'  = 1.Da.m + Da.(1 - m)
  //!        = Da
  //!
  //! Formulas for ARGB(dst), PRGB(src) colorspaces (ATOP):
  //!   Dc'  = Sca + Dc.(1 - Sa)
  //!   Da'  = Da
  //!
  //!   Msk:
  //!
  //!   Dc'  = ((Sca.Da + Dc.Da.(1 - Sa)).m + Dc.Da.(1 - m)) / Da'
  //!        = (Sca.Da.m + Dc.Da.(1 - Sa).m + Dc.Da.(1 - m)) / Da'
  //!        = Sca.m + Dc.(1 - Sa.m)
  //!   Da'  = Sa.Da.m + Da.(1 - Sa.m)
  //!        = Da
  //!
  //! Formulas for ARGB(dst), ARGB(src) colorspaces (ATOP):
  //!   Dc'  = Sc.Sa + Dc.(1 - Sa)
  //!   Da'  = Da
  //!
  //!   Msk:
  //!
  //!   Dc'  = ((Sc.Sa.Da + Dc.Da.(1 - Sa)).m + Dc.Da.(1 - m)) / Da'
  //!        = (Sc.Sa.Da.m + Dc.Da.(1 - Sa).m + Dc.Da.(1 - m)) / Da'
  //!        = Sc.Sa.m + Dc.(1 - Sa.m)
  //!   Da'  = Sa.Da.m + Da.(1 - Sa.m)
  //!        = Da
  //!
  //! Formulas for ARGB(dst), RGB(src) colorspaces (ATOP):
  //!   Dc'  = Sc
  //!   Da'  = Da
  //!
  //!   Msk:
  //!
  //!   Dc'  = Sc.m + Dc.(1 - m)
  //!   Da'  = Da.m + Da.(1 - m)
  //!        = Da
  //!
  //! Formulas for RGB(dst), PRGB(src) colorspaces (?):
  //!   Dc'  = Sca + Dc.(1 - Sa)
  //!
  //!   Msk:
  //!
  //!   Dc'  = (Sca + Dc.(1 - Sa)).m + Dc.(1 - m)
  //!
  //! Formulas for RGB(dst), RGB(src) colorspaces (SRC):
  //!   Dc'  = Sc
  //!
  //!   Msk:
  //!
  //!   Dc'  = Sc.m + Dc.(1 - m)
  //!
  //! Formulas for A(dst), A(src) colorspaces (NOP):
  //!   Da'  = Da
  CompositeSrcAtop,

  // --------------------------------------------------------------------------
  // [DST_ATOP]
  // --------------------------------------------------------------------------

  //! @brief The part of the destination lying inside of the source is 
  //! composited over the source and replaces the destination. 
  //!
  //! Formulas for PRGB(dst), PRGB(src) colorspaces:
  //!   Dca' = Dca.Sa + Sca.(1 - Da)
  //!   Da'  = Da.Sa + Sa.(1 - Da)
  //!        = Sa.(Da + 1 - Da)
  //!        = Sa
  //!
  //!   Msk:
  //!
  //!   Dca' = (Dca.Sa + Sca.(1 - Da)).m + Dca.(1 - m)
  //!   Da'  = (Da.Sa + Sa.(1 - Da)).m + Da.(1 - m)
  //!
  //! Formulas for PRGB(dst), RGB(src) colorspaces:
  //!   Dca' = Dca + Sc.(1 - Da)
  //!   Da'  = 1
  //!
  //!   Msk:
  //!
  //!   Dca' = (Dca + Sc.(1 - Da)).m + Dca.(1 - m)
  //!        = Dca + Sc.m.(1 - Da)
  //!   Da'  = Da + 1.m.(1 - Da)
  //!
  //! Formulas for ARGB(dst), PRGB(src) colorspaces:
  //!   Dc'  = Dc.Da + (Sca/Sa).(1 - Da)
  //!   Da'  = Sa
  //!
  //!   Msk:
  //!
  //!   Dc'  = ((Dc.Da.Sa + Sca.(1 - Da)).m + Dc.Da.(1 - m)) / Da'
  //!        = (Dc.Da.Sa.m + Sca.m - Sca.Da.m + Dc.Da - Dc.Da.m) / Da'
  //!        = (Dc.Da.(Sa.m + 1 - m) + Sca.m.(1 - Da)) / Da'
  //!   Da'  = Da.(Sa.m + 1 - m) + Sa.m.(1 - Da)
  //!        = Sa.m + Da - Da.m
  //!        = Sa.m + Da.(1 - m)
  //!
  //! Formulas for ARGB(dst), ARGB(src) colorspaces:
  //!   Dc'  = Dc.Da + Sc.(1 - Da)
  //!   Da'  = Sa
  //!
  //! Formulas for ARGB(dst), RGB(src) colorspaces:
  //!   Dc'  = Dc.Da + Sc.(1 - Da)
  //!   Da'  = 1
  //!
  //! Formulas for RGB(dst), PRGB(src) colorspaces:
  //!   Dc'  = Dc.Sa
  //!
  //! Formulas for RGB(dst), RGB(src) colorspaces:
  //!   Dc'  = Dc
  //!
  //! Formulas for A(dst), A(src) colorspaces:
  //!   Da'  = Sa
  CompositeDestAtop,

  // --------------------------------------------------------------------------
  // [XOR]
  // --------------------------------------------------------------------------

  //! @brief The part of the source that lies outside of the destination is 
  //! combined with the part of the destination that lies outside of the source.
  //!
  //! Formulas for PRGB(dst), PRGB(src) colorspaces (XOR):
  //!   Dca' = Sca.(1 - Da) + Dca.(1 - Sa)
  //!   Da'  = Sa.(1 - Da) + Da.(1 - Sa)
  //!        = Sa + Da - 2.Sa.Da
  //!
  //!   Msk:
  //!
  //!   Dca' = (Sca.(1 - Da) + Dca.(1 - Sa)).m + Dca.(1 - m)
  //!   Da'  = (Sa .(1 - Da) + Da .(1 - Sa)).m + Da .(1 - m)
  //!
  //! Formulas for PRGB(dst), RGB(src) colorspaces:
  //!   Dca' = Sc.(1 - Da)
  //!   Da'  = (1 - Da)
  //!
  //!   Msk:
  //!
  //!   Dca' = Sc.(1 - Da).m + Dca.(1 - m)
  //!   Da'  = (1 - Da).m + Da.(m - 1)
  //!
  //! Formulas for ARGB(dst), PRGB(src) colorspaces:
  //!   Dc'  = (Sca.(1 - Da) + Dc.Da.(1 - Sa))/Da'
  //!   Da'  = Sa.(1 - Da) + Da.(1 - Sa)
  //!
  //! Formulas for ARGB(dst), ARGB(src) colorspaces:
  //!   Dc'  = (Sc.Sa.(1 - Da) + Dc.Da.(1 - Sa))/Da'
  //!   Da'  = Sa.(1 - Da) + Da.(1 - Sa)
  //!
  //! Formulas for ARGB(dst), RGB(src) colorspaces:
  //!   Dc'  = Sc
  //!   Da'  = (1 - Da)
  //!
  //! Formulas for RGB(dst), PRGB(src) colorspaces:
  //!   Dc'  = Dc.(1 - Sa)
  //!
  //! Formulas for RGB(dst), RGB(src) colorspaces:
  //!   Dc'  = 0
  //!
  //! Formulas for A(dst), A(src) colorspaces:
  //!   Da'  = Sa.(1 - Da) + Da.(1 - Sa)
  CompositeXor,

  // --------------------------------------------------------------------------
  // [CLEAR]
  // --------------------------------------------------------------------------

  //! @brief Clear the destination not using the source.
  //!
  //! Formulas for PRGB(dst), PRGB(src) colorspaces (CLEAR):
  //!   Dca' = 0
  //!   Da'  = 0
  //!
  //!   Msk:
  //!
  //!   Dca' = Dca.(1 - m)
  //!   Da'  = Da .(1 - m)
  //!
  //! Formulas for PRGB(dst), RGB(src) colorspaces (CLEAR):
  //!   Dca' = 0
  //!   Da'  = 0
  //!
  //!   Msk:
  //!
  //!   Dca' = Dca.(1 - m)
  //!   Da'  = Da .(1 - m)
  //!
  //! Formulas for ARGB(dst), PRGB(src) colorspaces (CLEAR):
  //!   Dc'  = 0
  //!   Da'  = 0
  //!
  //!   Msk:
  //!
  //!   Dc'  = Dc
  //!   Da'  = Da .(1 - m)
  //!
  //! Formulas for ARGB(dst), ARGB(src) colorspaces (CLEAR):
  //!   Dc'  = 0
  //!   Da'  = 0
  //!
  //!   Msk:
  //!
  //!   Dc'  = Dc
  //!   Da'  = Da .(1 - m)
  //!
  //! Formulas for ARGB(dst), RGB(src) colorspaces (CLEAR):
  //!   Dc'  = 0
  //!   Da'  = 0
  //!
  //!   Msk:
  //!
  //!   Dc'  = Dc
  //!   Da'  = Da .(1 - m)
  //!
  //! Formulas for RGB(dst), PRGB(src) colorspaces (CLEAR):
  //!   Dc'  = 0
  //!
  //!   Msk:
  //!
  //!   Dc'  = Dc.(1 - m)
  //!
  //! Formulas for RGB(dst), RGB(src) colorspaces (CLEAR):
  //!   Dc'  = 0
  //!
  //!   Msk:
  //!
  //!   Dc'  = Dc.(1 - m)
  //!
  //! Formulas for A(dst), A(src) colorspaces (CLEAR):
  //!   Da'  = 0
  //!
  //!   Msk:
  //!
  //!   Da'  = Da.(1 - m)
  CompositeClear,

  //! @brief The source is added to the destination and replaces the destination.
  //!
  //! Formulas for PRGB(dst), PRGB(src) colorspaces:
  //!   Dca' = Sca.Da + Dca.Sa + Sca.(1 - Da) + Dca.(1 - Sa)
  //!        = Sca + Dca
  //!   Da'  = Sa.Da + Da.Sa + Sa.(1 - Da) + Da.(1 - Sa)
  //!        = Sa + Da
  //!
  //!   Msk:
  //!
  //!   Dca' = Sca.m + Dca.m + Dca.(1 - m)
  //!        = Sca.m + Dca
  //!   Da'  = Sa.m + Da.m + Da.(1 - m)
  //!        = Sa.m + Da
  //!
  //! Formulas for PRGB(dst), RGB(src) colorspaces:
  //!   Dca' = Sc + Dca
  //!   Da'  = 1
  //!
  //! Formulas for ARGB(dst), PRGB(src) colorspaces:
  //!   Dc'  = (Sca + Dc.Da) / Da'
  //!   Da'  = Sa + Da
  //!
  //! Formulas for ARGB(dst), ARGB(src) colorspaces:
  //!   Dc'  = (Sc.Sa + Dc.Da) / Da'
  //!   Da'  = Sa + Da
  //!
  //! Formulas for ARGB(dst), RGB(src) colorspaces:
  //!   Dc'  = Sc + Dc.Da
  //!   Da'  = 1
  //!
  //! Formulas for RGB(dst), PRGB(src) colorspaces:
  //!   Dc'  = Sca + Dc
  //!
  //! Formulas for RGB(dst), RGB(src) colorspaces:
  //!   Dc'  = Sc + Dc
  //!
  //! Formulas for A(dst), A(src) colorspaces:
  //!   Da'  = Sa + Da
  CompositeAdd,

  //! @brief The source is subtracted from the destination and replaces 
  //! the destination.
  //!
  //! Formulas for PRGB(dst), PRGB(src) colorspaces:
  //!   Dca' = Dca - Sca
  //!   Da'  = 1 - (1 - Sa).(1 - Da)
  //!        = Sa + Da - Sa.Da
  //!
  //!   Msk:
  //!
  //!   Dca' = Dca.m - Sca.m + Dca.(1 - m)
  //!        = Dca - Sca.m
  //!   Da'  = Da + Sa.m - Sa.m.Da
  //!        = Da + Sa.m - (1 - Da)
  //!
  //! Formulas for PRGB(dst), RGB(src) colorspaces:
  //!   Dca' = Dca - Sa
  //!   Da'  = 1
  //!
  //! Formulas for ARGB(dst), PRGB(src) colorspaces:
  //!   Dc'  = (Dc.Da - Sca) / Da'
  //!   Da'  = Sa + Da.(1 - Sa)
  //!
  //! Formulas for ARGB(dst), ARGB(src) colorspaces:
  //!   Dc'  = (Dc.Da - Sc.Sa) / Da'
  //!   Da'  = Sa + Da.(1 - Sa)
  //!
  //! Formulas for ARGB(dst), RGB(src) colorspaces:
  //!   Dc'  = Dc.Da - Sc
  //!   Da'  = 1 
  //!
  //! Formulas for RGB(dst), PRGB(src) colorspaces:
  //!   Dc'  = Dc - Sca
  //!
  //! Formulas for RGB(dst), RGB(src) colorspaces:
  //!   Dc'  = Dc - Sc
  //!
  //! Formulas for A(dst), A(src) colorspaces:
  //!   Da'  = Da + Sa.(1 - Da) (standard formula used many times)
  //!
  //!   Msk:
  //!
  //!   Da'  = Da + Sa.m.(1 - Da)
  CompositeSubtract,

  //! @brief The source is multiplied by the destination and replaces 
  //! the destination
  //!
  //! The resultant color is always at least as dark as either of the two 
  //! constituent colors. Multiplying any color with black produces black.
  //! Multiplying any color with white leaves the original color unchanged.
  //!
  //! Formulas for PRGB(dst), PRGB(src) colorspaces:
  //!   Dca' = Sca.Dca + Sca.(1 - Da) + Dca.(1 - Sa)
  //!        = Sca.(Dca + 1 - Da) + Dca.(1 - Sa)
  //!        = Dca.(Sca + 1 - Sa) + Sca.(1 - Da)
  //!   Da'  = Sa.Da + Sa.(1 - Da) + Da.(1 - Sa)
  //!        = Sa.(Da + 1 - Da) + Da.(1 - Sa)
  //!        = Da.(Sa + 1 - Sa) + Sa.(1 - Da)
  //!        = Sa + Da - Sa.Da
  //!
  //! Formulas for PRGB(dst), RGB(src) colorspaces:
  //!   Dca' = Sc.Dca + Sc.(1 - Da)
  //!   Da'  = 1
  //!
  //! Formulas for ARGB(dst), PRGB(src) colorspaces:
  //!   Dc'  = (Sca.Dc.Da + Sca.(1 - Da) + Dc.Da.(1 - Sa)) / Da'
  //!   Da'  = Sa.Da + Sa.(1 - Da) + Da.(1 - Sa)
  //!        = Sa + Da - Sa.Da
  //!
  //! Formulas for ARGB(dst), ARGB(src) colorspaces:
  //!   Dc'  = (Sc.Sa.Dc.Da + Sc.Sa.(1 - Da) + Dc.Da.(1 - Sa)) / Da'
  //!   Da'  = Sa.Da + Sa.(1 - Da) + Da.(1 - Sa)
  //!        = Sa + Da - Sa.Da
  //!
  //! Formulas for ARGB(dst), RGB(src) colorspaces:
  //!   Dc'  = Sc.Dc.Da + Sc.(1 - Da)
  //!        = Sc.(Dc.Da + (1 - Da))
  //!        = Sc + Sc.Da(1 - Dc)
  //!   Da'  = 1
  //!
  //! Formulas for RGB(dst), PRGB(src) colorspaces:
  //!   Dc'  = Sca.Dc + Dc.(1 - Sa)
  //!
  //! Formulas for RGB(dst), RGB(src) colorspaces:
  //!   Dc'  = Sc.Dc
  //!
  //! Formulas for A(dst), A(src) colorspaces:
  //!   Da'  = Da + Sa.(1 - Da) (standard formula used many times)
  //!
  //!   Msk:
  //!
  //!   Da'  = Da + Sa.m.(1 - Da)
  CompositeMultiply,

  //! @brief The source and destination are complemented and then multiplied 
  //! and then replace the destination
  //!
  //! Formulas for PRGB(dst), PRGB(src) colorspaces:
  //!   Dca' = (Sca.Da + Dca.Sa - Sca.Dca) + Sca.(1 - Da) + Dca.(1 - Sa)
  //!        = Sca + Dca - Sca.Dca
  //!        = Dca + Sca.(1 - Dca)
  //!        = Sca + Dca.(1 - Sca)
  //!   Da'  = Sa + Da - Sa.Da
  //!        = Da + Sa.(1 - Da) = Sa + Da.(1 - Sa)
  //!
  //! Formulas for PRGB(dst), RGB(src) colorspaces:
  //!   Dca' = Sc + Dca - Sc.Dca
  //!        = Dca + Sc.(1 - Dca)
  //!        = Sc + Dca.(1 - Sc)
  //!   Da'  = 1
  //!
  //! Formulas for ARGB(dst), PRGB(src) colorspaces:
  //!   Dc'  = (Dc.Da + Sca.(1 - Dc.Da)) / Da'
  //!   Da'  = Da + Sa.(1 - Da)
  //!
  //! Formulas for ARGB(dst), ARGB(src) colorspaces:
  //!   Dc'  = (Dc.Da + Sc.Sa.(1 - Dc.Da)) / Da'
  //!   Da'  = Da + Sa.(1 - Da)
  //!
  //! Formulas for ARGB(dst), RGB(src) colorspaces:
  //!   Dc'  = Sc + Dc.Da - Sc.Dc.Da
  //!        = Sc + Dc.Da.(1 - Sc)
  //!   Da'  = 1
  //!
  //! Formulas for RGB(dst), PRGB(src) colorspaces:
  //!   Dc'  = Sca + Dc - Sca.Dc
  //!        = Sca + Dc.(1 - Sca)
  //!        = Dc + Sca.(1 - Dc)
  //!
  //! Formulas for RGB(dst), RGB(src) colorspaces:
  //!   Dc'  = Sc + Dc - Sc.Dc
  //!        = Sc + Dc.(1 - Sc)
  //!
  //! Formulas for A(dst), A(src) colorspaces:
  //!   Da'  = Da + Sa.(1 - Da) (standard formula used many times)
  //!
  //!   Msk:
  //!
  //!   Da'  = Da + Sa.m.(1 - Da)
  CompositeScreen,

  //! @brief Selects the darker of the destination and source colors. The 
  //! destination is replaced with the source when the source is darker,
  //! otherwise it is left unchanged.
  //!
  //! Formulas for PRGB(dst), PRGB(src) colorspaces:
  //!   Dca' = min(Sca.Da, Dca.Sa) + Sca.(1 - Da) + Dca.(1 - Sa)
  //!   Da'  = min(Sa.Da, Da.Sa) + Sa.(1 - Da) + Da.(1 - Sa)
  //!        = Sa.Da + Sa - Sa.Da + Da - Sa.Da
  //!        = Sa + Da - Sa.Da
  //!
  //!   OR: if (Sca.Da < Dca.Sa) Src-Over() else Dst-Over()
  //!
  //! Formulas for PRGB(dst), RGB(src) colorspaces:
  //!   Dca' = min(Sc.Da, Dca) + Sc.(1 - Da)
  //!   Da'  = 1
  //!
  //! Formulas for ARGB(dst), PRGB(src) colorspaces:
  //!   Dc'  = min(Sca.Da, Dc.Da.Sa) + Sca.(1 - Da) + Dc.Da.(1 - Sa)
  //!   Da'  = min(Sa.Da, Da.Sa) + Sa.(1 - Da) + Da.(1 - Sa)
  //!
  //! Formulas for ARGB(dst), ARGB(src) colorspaces:
  //!   Dc'  = Sa.Da.min(Sc, Dc) + Sc.Sa.(1 - Da) + Dc.Da.(1 - Sa)
  //!   Da'  = Sa.Da + Sa.(1 - Da) + Da.(1 - Sa)
  //!
  //! Formulas for ARGB(dst), RGB(src) colorspaces:
  //!   Dc'  = min(Sc.Da, Dc.Da) + Sc.(1 - Da)
  //!   Da'  = 1
  //!
  //! Formulas for RGB(dst), PRGB(src) colorspaces:
  //!   Dc'  = min(Sca, Dc.Sa) + Dc.(1 - Sa)
  //!
  //! Formulas for RGB(dst), RGB(src) colorspaces:
  //!   Dc'  = min(Sc, Dc)
  //!
  //! Formulas for A(dst), A(src) colorspaces:
  //!   Da'  = Da + Sa.(1 - Da) (standard formula used many times)
  //!
  //!   Msk:
  //!
  //!   Da'  = Da + Sa.m.(1 - Da)
  CompositeDarken,

  //! @brief Selects the lighter of the destination and source colors. The
  //! destination is replaced with the source when the source is lighter, 
  //! otherwise it is left unchanged.
  //!
  //! Formulas for PRGB(dst), PRGB(src) colorspaces:
  //!   Dca' = max(Sca.Da, Dca.Sa) + Sca.(1 - Da) + Dca.(1 - Sa)
  //!   Da'  = max(Sa.Da, Da.Sa) + Sa.(1 - Da) + Da.(1 - Sa)
  //!        = Sa.Da + Sa - Sa.Da + Da - Sa.Da
  //!        = Sa + Da - Sa.Da
  //!
  //!   OR: if (Sca.Da > Dca.Sa) Src-Over() else Dst-Over()
  //!
  //! Formulas for PRGB(dst), RGB(src) colorspaces:
  //!   Dca' = max(Sc.Da, Dca) + Sc.(1 - Da)
  //!   Da'  = 1
  //!
  //! Formulas for ARGB(dst), PRGB(src) colorspaces:
  //!   Dc'  =
  //!   Da'  =
  //!
  //! Formulas for ARGB(dst), ARGB(src) colorspaces:
  //!   Dc'  =
  //!   Da'  =
  //!
  //! Formulas for ARGB(dst), RGB(src) colorspaces:
  //!   Dc'  = max(Sc.Da, Dc.Da) + Sc.(1 - Da)
  //!   Da'  = 1
  //!
  //! Formulas for RGB(dst), PRGB(src) colorspaces:
  //!   Dc'  = max(Sca, Dc.Sa) + Dc.(1 - Sa)
  //!
  //! Formulas for RGB(dst), RGB(src) colorspaces:
  //!   Dc'  = max(Sc, Dc)
  //!
  //! Formulas for A(dst), A(src) colorspaces:
  //!   Da'  = Da + Sa.(1 - Da) (standard formula used many times)
  //!
  //!   Msk:
  //!
  //!   Da'  = Da + Sa.m.(1 - Da)
  CompositeLighten,

  //! @brief Subtracts the darker of the two constituent colors from the 
  //! lighter. Painting with white inverts the destination color. Painting
  //! with black produces no change.
  //!
  //! Formulas for PRGB(dst), PRGB(src) colorspaces:
  //!   Dca' = abs(Dca.Sa - Sca.Da) + Sca.(1 - Da) + Dca.(1 - Sa)
  //!   Dca' = Sca + Dca - 2.min(Sca.Da, Dca.Sa)
  //!   Da'  = Sa + Da - min(Sa.Da, Da.Sa)
  //!        = Sa + Da - Sa.Da
  //!
  //! Formulas for PRGB(dst), RGB(src) colorspaces:
  //!   Dca' = abs(Dca - Sc.Da) + Sc.(1 - Da)
  //!   Dca' = Sc + Dca - 2.min(Sc.Da, Dca)
  //!   Da'  = 1
  //!
  //! Formulas for ARGB(dst), PRGB(src) colorspaces:
  //!   Dc'  =
  //!   Da'  =
  //!
  //! Formulas for ARGB(dst), ARGB(src) colorspaces:
  //!   Dc'  =
  //!   Da'  =
  //!
  //! Formulas for ARGB(dst), RGB(src) colorspaces:
  //!   Dc'  = abs(Dc.Da - Sc.Da) + Sc.(1 - Da)
  //!        = Da.abs(Dc - Sc) + Sc.(1 - Da)
  //!   Dc'  = Sc + Dc.Da - 2.min(Sc.Da, Dc.Da)
  //!        = Sc + Dc.Da - 2.min(Sc, Dc)*Da
  //!        = Sc + Da.(Dc - 2.min(Sc, Dc))
  //!   Da'  = 1
  //!
  //! Formulas for RGB(dst), PRGB(src) colorspaces:
  //!   Dc'  = abs(Dc.Sa - Sca) + Dc.(1 - Sa)
  //!   Dc'  = Sca + Dc - 2.min(Sca, Dc.Sa)
  //!
  //! Formulas for RGB(dst), RGB(src) colorspaces:
  //!   Dc'  = abs(Dc - Sc)
  //!
  //! Formulas for A(dst), A(src) colorspaces:
  //!   Da'  = Da + Sa.(1 - Da) (standard formula used many times)
  //!
  //!   Msk:
  //!
  //!   Da'  = Da + Sa.m.(1 - Da)
  CompositeDifference,

  //! @brief Produces an effect similar to that of 'difference', but appears
  //! as lower contrast. Painting with white inverts the destination color.
  //! Painting with black produces no change.
  //!
  //! Formulas for PRGB(dst), PRGB(src) colorspaces:
  //!   Dca' = (Sca.Da + Dca.Sa - 2.Sca.Dca) + Sca.(1 - Da) + Dca.(1 - Sa)
  //!          Sca + Dca - 2.Sca.Dca
  //!   Da'  = (Sa.Da + Da.Sa - 2.Sa.Da) + Sa.(1 - Da) + Da.(1 - Sa)
  //!        = Sa - Sa.Da + Da - Da.Sa = Sa + Da - 2.Sa.Da
  //!          [Substitute 2.Sa.Da with Sa.Da]
  //!        = Sa + Da - Sa.Da
  //!
  //! Formulas for PRGB(dst), RGB(src) colorspaces:
  //!   Dca' = Sc + Dca - 2.Sc.Dca
  //!        = Dca - Sc.(1 - 2.Dca)
  //!   Da'  = 1
  //!
  //! Formulas for ARGB(dst), PRGB(src) colorspaces:
  //!   Dc'  =
  //!   Da'  =
  //!
  //! Formulas for ARGB(dst), ARGB(src) colorspaces:
  //!   Dc'  =
  //!   Da'  =
  //!
  //! Formulas for ARGB(dst), RGB(src) colorspaces:
  //!   Dc'  = Sc + Dc.Da - 2.Sc.Dc.Da
  //!        = Dc.Da - Sc.(1 - 2.Dc.Da)
  //!   Da'  = 1
  //!
  //! Formulas for RGB(dst), PRGB(src) colorspaces:
  //!   Dc'  = Sca + Dc - 2.Sca.Dc 
  //!        = Dc + Sca.(1 - 2.Dc)
  //!
  //! Formulas for RGB(dst), RGB(src) colorspaces:
  //!   Dc'  = Sc + Dc - 2.Sc.Dc
  //!        = Dc + Sc.(1 - 2.Dc)
  //!
  //! Formulas for A(dst), A(src) colorspaces:
  //!   Da'  = Da + Sa.(1 - Da) (standard formula used many times)
  //!
  //!   Msk:
  //!
  //!   Da'  = Da + Sa.m.(1 - Da)
  CompositeExclusion,

  //! @brief Invert.
  //!
  //! Formulas for PRGB(dst), PRGB(src) colorspaces:
  //!   Dca' = (Da - Dca) * Sa + Dca.(1 - Sa)
  //!   Da'  = Sa + (Da - Da) * Sa + Da - Sa.Da
  //!        = Sa + Da - Sa.Da
  //!
  //! For calculation this formula is best:
  //!   Dca' = (Da - Dca) * Sa + Dca.(1 - Sa)
  //!   Da'  = (1 + Da - Da) * Sa + Da.(1 - Sa)
  //!
  //! Formulas for PRGB(dst), RGB(src) colorspaces:
  //!   Dca' = (Da - Dca)
  //!   Da'  = 1
  //!
  //! Formulas for ARGB(dst), PRGB(src) colorspaces:
  //!   Dc'  =
  //!   Da'  =
  //!
  //! Formulas for ARGB(dst), ARGB(src) colorspaces:
  //!   Dc'  =
  //!   Da'  =
  //!
  //! Formulas for ARGB(dst), RGB(src) colorspaces:
  //!   Dc'  = Da.(1 - Dc)
  //!   Da'  = 1
  //!
  //! Formulas for RGB(dst), PRGB(src) colorspaces:
  //!   Dc'  = (1 - Dc) * Sa + Dc.(1 - Sa)
  //!
  //! Formulas for RGB(dst), RGB(src) colorspaces:
  //!   Dc'  = 1 - Dc
  //!
  //! Formulas for A(dst), A(src) colorspaces:
  //!   Da'  = Da + Sa.(1 - Da) (standard formula used many times)
  //!
  //!   Msk:
  //!
  //!   Da'  = Da + Sa.m.(1 - Da)
  CompositeInvert,

  //! @brief Invert RGB.
  //!
  //! Formulas for PRGB(dst), PRGB(src) colorspaces:
  //!   Dca' = (Da - Dca) * Sca + Dca.(1 - Sa)
  //!   Da'  = Sa + (Da - Da) * Sa + Da - Da.Sa
  //!        = Sa + Da - Sa.Da
  //!
  //! For calculation this formula is best:
  //!   Dca' = (Da - Dca) * Sca + Dca.(1 - Sa)
  //!   Da'  = (1 + Da - Da) * Sa + Da.(1 - Sa)
  //!
  //! Formulas for PRGB(dst), RGB(src) colorspaces:
  //!   Dca' = (Da - Dca) * Sc
  //!   Da'  = 1
  //!
  //! Formulas for ARGB(dst), PRGB(src) colorspaces:
  //!   Dc'  =
  //!   Da'  =
  //!
  //! Formulas for ARGB(dst), ARGB(src) colorspaces:
  //!   Dc'  =
  //!   Da'  =
  //!
  //! Formulas for ARGB(dst), RGB(src) colorspaces:
  //!   Dc'  = (Da.(1 - Dc)) * Sc
  //!   Da'  = 1
  //!
  //! Formulas for RGB(dst), PRGB(src) colorspaces:
  //!   Dc'  = (1 - Dc) * Sca + Dc.(1 - Sa)
  //!
  //! Formulas for RGB(dst), RGB(src) colorspaces:
  //!   Dc'  = (1 - Dc) * Sc
  //!
  //! Formulas for A(dst), A(src) colorspaces:
  //!   Da'  = Da + Sa.(1 - Da) (standard formula used many times)
  //!
  //!   Msk:
  //!
  //!   Da'  = Da + Sa.m.(1 - Da)
  CompositeInvertRgb,

  //! @brief Count of compositing operators (this is not a valid operator).
  CompositeCount
};

//! @brief Results from some color analyze functions.
//!
//! Usualy used to optimize image processing, because algorithm
//! for full opaque image is always better that generic algorithm
//! for image with alpha channel.
enum
{
  //! @brief Alpha values are not constant.
  VariableAlpha = -1,
  //! @brief All alpha values are transparent (0).
  TransparentAlpha = 0,
  //! @brief All alpha values are opaque (255).
  OpaqueAlpha = 255
};

// ============================================================================
// [Fog::ImageFile]
// ============================================================================

//! @brief Image file IDs.
enum ImageFile
{
  ImageFileNone = 0,

  ImageFileANI,
  ImageFileAPNG,
  ImageFileBMP,
  ImageFileFLI,
  ImageFileFLC,
  ImageFileGIF,
  ImageFileICO,
  ImageFileJPEG,
  ImageFileLBM,
  ImageFileMNG,
  ImageFilePCX,
  ImageFilePNG,
  ImageFilePNM,
  ImageFileTGA,
  ImageFileTIFF,
  ImageFileXBM,
  ImageFileXPM,

  ImageFileOther = 65536
};

// ============================================================================
// [Fog::Channel]
// ============================================================================

//! @brief Rgba color channels.
enum Channel
{
  ChannelRed = 0x1,
  ChannelGreen = 0x2,
  ChannelBlue = 0x4,
  ChannelAlpha = 0x8,
  ChannelRGB = ChannelRed | ChannelGreen | ChannelBlue,
  ChannelRGBA = ChannelRGB | ChannelAlpha,

  ChannelCount = ChannelRGBA + 1
};

// ============================================================================
// [Fog::MatrixOrder]
// ============================================================================

//! @brief Matrix multiply ordering.
enum MatrixOrder
{
  MatrixOrderPrepend = 0,
  MatrixOrderAppend = 1
};

// ============================================================================
// [Fog::FillMode]
// ============================================================================

//! @brief Fill mode.
enum FillMode
{
  FillNonZero = 0,
  FillEvenOdd = 1,

  FillModeInvalid = 2
};

// ============================================================================
// [Fog::LineCap]
// ============================================================================

//! @brief Line cap.
enum LineCap
{
  LineCapButt = 0,
  LineCapSquare = 1,
  LineCapRound = 2,

  LineCapInvalid = 3 
};

// ============================================================================
// [Fog::LineJoin]
// ============================================================================

//! @brief Line join.
enum LineJoin
{
  LineJoinMiter = 0,
  LineJoinMiterRevert = 1,
  LineJoinRound = 2,
  LineJoinBevel = 3,
  LineJoinMiterRound = 4,

  LineJoinInvalid = 5
};

// ============================================================================
// [Fog::InnerJoin]
// ============================================================================

//! @brief Inner join.
enum InnerJoin
{
  InnerJoinBevel = 0,
  InnerJoinMiter = 1,
  InnerJoinJag = 2,
  InnerJoinRound = 3,

  InnerJoinInvalid = 4
};

// ============================================================================
// [Fog::TextAlignment]
// ============================================================================

//! @brief Text alignment
enum TextAlign
{
  TextAlignLeft        = 0x01,
  TextAlignRight       = 0x02,
  TextAlignHCenter     = 0x03,
  TextAlignHMask       = 0x03,

  TextAlignTop         = 0x10,
  TextAlignBottom      = 0x20,
  TextAlignVCenter     = 0x30,
  TextAlignVMask       = 0x30,

  TextAlignCenter      = TextAlignVCenter | TextAlignHCenter
};

} // Fog namespace

// [Guard]
#endif // _FOG_GRAPHICS_CONSTANTS_H
