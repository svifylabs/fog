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
//
// The author gratefully acknowleges the support of David Turner, 
// Robert Wilhelm, and Werner Lemberg - the authors of the FreeType 
// libray - in producing this work. See http://www.freetype.org for details.
//
//----------------------------------------------------------------------------
// Contact: mcseem@antigrain.com
//          mcseemagg@yahoo.com
//          http://www.antigrain.com
//----------------------------------------------------------------------------
//
// Adaptation for 32-bit screen coordinates has been sponsored by 
// Liberty Technology Systems, Inc., visit http://lib-sys.com
//
// Liberty Technology Systems, Inc. is the provider of
// PostScript and PDF technology for software developers.
// 
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// This is custom rasterizer that can be used in multithreaded environment
// from multiple threads. Method sweep_scanline() from antigrain 
// agg::rasterizer_scanline_aa<> template was replaced to method that accepts
// y coordinate and it's tagged as const. After you serialize your content use
// new sweep_scanline() method with you own Y coordinate.
//
// To use this rasterizer you must first set gamma table that will be used. In
// multithreaded environment recomputing gamma table in each thread command
// is not good, so the gamma table is here just const pointer to your real
// table that is shared across many rasterizer instances. Use setGamma()
// function to set gamma table.
//
// Note, gamma table is BYTE type not int as in original rasterizer.
//
// Contribution by Petr Kobalicek <kobalicek.petr@gmail.com>,
// This contribution follows antigrain licence (Public Domain).
//
// Permission to copy, use, modify, sell and distribute this software
// is granted provided this copyright notice appears in all copies.
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//----------------------------------------------------------------------------

#ifndef AGG_RASTERIZER_SCANLINE_AA_CUSTOM_INCLUDED
#define AGG_RASTERIZER_SCANLINE_AA_CUSTOM_INCLUDED

#include <Fog/Graphics/Path.h>

#include "agg_rasterizer_cells_aa.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_rasterizer_sl_clip.h"
#include "agg_gamma_functions.h"

namespace agg
{
    // ========================================================================
    // [rasterizer_scanline_aa_custom]
    // ========================================================================

    // Polygon rasterizer that is used to render filled polygons with 
    // high-quality Anti-Aliasing. Internally, by default, the class uses 
    // integer coordinates in format 24.8, i.e. 24 bits for integer part 
    // and 8 bits for fractional - see poly_subpixel_shift. This class can be 
    // used in the following  way:
    //
    // 1. filling_rule(filling_rule_e ft) - optional.
    //
    // 2. gamma() - optional.
    //
    // 3. reset()
    //
    // 4. move_to(x, y) / line_to(x, y) - make the polygon. One can create 
    //    more than one contour, but each contour must consist of at least 3
    //    vertices, i.e. move_to(x1, y1); line_to(x2, y2); line_to(x3, y3);
    //    is the absolute minimum of vertices that define a triangle.
    //    The algorithm does not check either the number of vertices nor
    //    coincidence of their coordinates, but in the worst case it just 
    //    won't draw anything.
    //    The orger of the vertices (clockwise or counterclockwise) 
    //    is important when using the non-zero filling rule (fill_non_zero).
    //    In this case the vertex order of all the contours must be the same
    //    if you want your intersecting polygons to be without "holes".
    //    You actually can use different vertices order. If the contours do not 
    //    intersect each other the order is not important anyway. If they do, 
    //    contours with the same vertex order will be rendered without "holes" 
    //    while the intersecting contours with different orders will have "holes".
    //
    // filling_rule() and gamma() can be called anytime before "sweeping".
    //------------------------------------------------------------------------
    template<class Clip=rasterizer_sl_clip_int> class rasterizer_scanline_aa_custom
    {
        enum status
        {
            status_initial,
            status_move_to,
            status_line_to,
            status_closed
        };

    public:
        typedef Clip                      clip_type;
        typedef typename Clip::conv_type  conv_type;
        typedef typename Clip::coord_type coord_type;

        enum aa_scale_e
        {
            // NOTE: This must be 8.
            aa_shift  = 8,             // 8
            aa_scale  = 1 << aa_shift, // 256
            aa_mask   = aa_scale - 1,  // 255
            aa_scale2 = aa_scale * 2,  // 512
            aa_mask2  = aa_scale2 - 1  // 511
        };

        //--------------------------------------------------------------------
        AGG_INLINE rasterizer_scanline_aa_custom() :
            m_outline(),
            m_clipper(),
            m_gamma(NULL),
            m_filling_rule(fill_non_zero),
            m_auto_close(true),
            m_start_x(0),
            m_start_y(0),
            m_status(status_initial)
        {
        }

        //------------------------------------------------------------------------
        AGG_INLINE void reset() 
        { 
            m_outline.reset(); 
            m_status = status_initial;
        }

        //------------------------------------------------------------------------
        AGG_INLINE void reset_clipping()
        {
            reset();
            m_clipper.reset_clipping();
        }

        //------------------------------------------------------------------------
        AGG_INLINE void clip_box(double x1, double y1, double x2, double y2)
        {
            reset();
            m_clipper.clip_box(conv_type::upscale(x1), conv_type::upscale(y1), 
                               conv_type::upscale(x2), conv_type::upscale(y2));
        }

        AGG_INLINE void filling_rule(filling_rule_e filling_rule)
        { 
            m_filling_rule = filling_rule; 
        }

        void auto_close(bool flag) { m_auto_close = flag; }

        AGG_INLINE void gamma(const unsigned char* gamma)
        {
          m_gamma = gamma;
        }

        //--------------------------------------------------------------------
        AGG_INLINE unsigned apply_gamma(unsigned cover) const
        {
            return m_gamma[cover]; 
        }

        //--------------------------------------------------------------------
        AGG_INLINE void close_polygon()
        {
            if(m_status == status_line_to)
            {
                m_clipper.line_to(m_outline, m_start_x, m_start_y);
                m_status = status_closed;
            }
        }

        //------------------------------------------------------------------------
        AGG_INLINE void move_to(int x, int y)
        {
            if(m_outline.sorted()) reset();
            if(m_auto_close) close_polygon();
            m_clipper.move_to(m_start_x = conv_type::downscale(x), 
                              m_start_y = conv_type::downscale(y));
            m_status = status_move_to;
        }

        //------------------------------------------------------------------------
        AGG_INLINE void line_to(int x, int y)
        {
            m_clipper.line_to(m_outline, 
                              conv_type::downscale(x), 
                              conv_type::downscale(y));
            m_status = status_line_to;
        }

        //------------------------------------------------------------------------
        AGG_INLINE void move_to_d(double x, double y) 
        { 
            if(m_outline.sorted()) reset();
            if(m_auto_close) close_polygon();
            m_clipper.move_to(m_start_x = conv_type::upscale(x), m_start_y = conv_type::upscale(y)); 
            m_status = status_move_to;
        }

        //------------------------------------------------------------------------
        AGG_INLINE void line_to_d(double x, double y) 
        { 
            m_clipper.line_to(m_outline, conv_type::upscale(x), conv_type::upscale(y)); 
            m_status = status_line_to;
        }

        //------------------------------------------------------------------------
        AGG_INLINE void add_path(const Fog::Path::Vertex* src, sysuint_t count)
        {
          if (m_outline.sorted()) reset();

          for (sysuint_t i = count; i; i--, src++)
          {
            uint cmd = src->cmd.cmd();
            if (cmd == Fog::Path::CmdMoveTo)
            {
              move_to_d(src->x, src->y);
            }
            else if (cmd == Fog::Path::CmdLineTo)
            {
              line_to_d(src->x, src->y);
            }
            else if (is_close(cmd))
            {
              close_polygon();
            }
          }
        }

        //------------------------------------------------------------------------
        AGG_INLINE void edge(int x1, int y1, int x2, int y2)
        {
            if(m_outline.sorted()) reset();
            m_clipper.move_to(conv_type::downscale(x1), conv_type::downscale(y1));
            m_clipper.line_to(m_outline, conv_type::downscale(x2), conv_type::downscale(y2));
            m_status = status_move_to;
        }
        
        //------------------------------------------------------------------------
        AGG_INLINE void edge_d(double x1, double y1, double x2, double y2)
        {
            if(m_outline.sorted()) reset();
            m_clipper.move_to(conv_type::upscale(x1), conv_type::upscale(y1)); 
            m_clipper.line_to(m_outline, conv_type::upscale(x2), conv_type::upscale(y2)); 
            m_status = status_move_to;
        }
        
        //--------------------------------------------------------------------
        AGG_INLINE int min_x() const { return m_outline.min_x(); }
        AGG_INLINE int min_y() const { return m_outline.min_y(); }
        AGG_INLINE int max_x() const { return m_outline.max_x(); }
        AGG_INLINE int max_y() const { return m_outline.max_y(); }

        //------------------------------------------------------------------------
        AGG_INLINE void sort()
        {
            if(m_auto_close) close_polygon();
            m_outline.sort_cells();
        }

        //--------------------------------------------------------------------
        AGG_INLINE unsigned calculate_alpha_non_zero(int area) const
        {
            int cover = area >> (poly_subpixel_shift*2 + 1 - aa_shift);

            if (cover < 0) cover = -cover;
            if (cover > aa_mask) cover = aa_mask;
            return m_gamma[cover];
        }

        AGG_INLINE unsigned calculate_alpha_even_odd(int area) const
        {
            int cover = area >> (poly_subpixel_shift*2 + 1 - aa_shift);

            if (cover < 0) cover = -cover;
            cover &= aa_mask2;
            if (cover > aa_scale) cover = aa_scale2 - cover;
            if (cover > aa_mask) cover = aa_mask;
            return m_gamma[cover];
        }

        //--------------------------------------------------------------------
        template<class Scanline>
        AGG_INLINE unsigned sweep_scanline(Scanline& sl, int y) const
        {
          if (m_filling_rule == fill_non_zero)
            return sweep_scanline_non_zero(sl, y);
          else
            return sweep_scanline_even_odd(sl, y);
        }

        template<class Scanline>
        unsigned sweep_scanline_non_zero(Scanline& sl, int y) const
        {
            if (y > m_outline.max_y()) return 0;

            unsigned num_cells = m_outline.scanline_num_cells(y);
            if (!num_cells) return 0;

            sl.reset_spans();
            const cell_aa* const* cells = m_outline.scanline_cells(y);
            int cover = 0;

            do {
                const cell_aa* cur_cell = *cells;
                int x    = cur_cell->x;
                int area = cur_cell->area;
                unsigned alpha;

                cover += cur_cell->cover;

                //accumulate all cells with the same X
                while(--num_cells)
                {
                    cur_cell = *++cells;
                    if(cur_cell->x != x) break;
                    area  += cur_cell->area;
                    cover += cur_cell->cover;
                }

                if(area)
                {
                    alpha = calculate_alpha_non_zero((cover << (poly_subpixel_shift + 1)) - area);
                    if (alpha) sl.add_cell(x, alpha);
                    x++;
                }

                if(num_cells && cur_cell->x > x)
                {
                    alpha = calculate_alpha_non_zero(cover << (poly_subpixel_shift + 1));
                    if (alpha) sl.add_span(x, cur_cell->x - x, alpha);
                }
            } while (num_cells);
    
            unsigned ns = sl.num_spans();
            if (ns) sl.finalize(y);
            return ns;
        }

        template<class Scanline>
        unsigned sweep_scanline_even_odd(Scanline& sl, int y) const
        {
            if (y > m_outline.max_y()) return 0;

            unsigned num_cells = m_outline.scanline_num_cells(y);
            if (!num_cells) return 0;

            sl.reset_spans();
            const cell_aa* const* cells = m_outline.scanline_cells(y);
            int cover = 0;

            do {
                const cell_aa* cur_cell = *cells;
                int x    = cur_cell->x;
                int area = cur_cell->area;
                unsigned alpha;

                cover += cur_cell->cover;

                // accumulate all cells with the same X
                while(--num_cells)
                {
                    cur_cell = *++cells;
                    if(cur_cell->x != x) break;
                    area  += cur_cell->area;
                    cover += cur_cell->cover;
                }

                if(area)
                {
                    alpha = calculate_alpha_even_odd((cover << (poly_subpixel_shift + 1)) - area);
                    if (alpha) sl.add_cell(x, alpha);
                    x++;
                }

                if(num_cells && cur_cell->x > x)
                {
                    alpha = calculate_alpha_even_odd(cover << (poly_subpixel_shift + 1));
                    if (alpha) sl.add_span(x, cur_cell->x - x, alpha);
                }
            } while (num_cells);

            unsigned ns = sl.num_spans();
            if (ns) sl.finalize(y);
            return ns;
        }

        AGG_INLINE bool has_cells() const
        {
          return m_outline.total_cells() > 0;
        }

        //--------------------------------------------------------------------
        bool hit_test(int tx, int ty) const;

    private:
        rasterizer_cells_aa<cell_aa> m_outline;
        clip_type m_clipper;
        const unsigned char* m_gamma;
        filling_rule_e m_filling_rule;
        bool m_auto_close;
        coord_type m_start_x;
        coord_type m_start_y;
        unsigned m_status;

        //--------------------------------------------------------------------
        // Disable copying
        rasterizer_scanline_aa_custom(const rasterizer_scanline_aa_custom<Clip>&);
        const rasterizer_scanline_aa_custom<Clip>& 
        operator = (const rasterizer_scanline_aa_custom<Clip>&);
    };

    //------------------------------------------------------------------------
    template<class Clip> 
    bool rasterizer_scanline_aa_custom<Clip>::hit_test(int tx, int ty) const
    {
        scanline_hit_test sl(tx);
        sweep_scanline(sl, ty);
        return sl.hit();
    }
}

#endif
