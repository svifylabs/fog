// [Fog/Svg Library - C++ API]
//
// [Licence]
// MIT, See COPYING.txt file in package
// Author: Anders Jansson, Copyright(c) 2009 <l8.jansson@gmail.com>

// [Purpose]
// This declares the class Symbol that is ment to be drawn on the screen.
//
// See the acompanying Symbol.cpp file for further explanations.


#ifndef SYMBOL_H_INCLUDED
#define SYMBOL_H_INCLUDED

typedef enum SymbolCommand_e {
	SC_endOfSymbol = 0,

	SC_fillColor, SC_noFill, SC_fillColorIndex,
	SC_lineColor, SC_noLine, SC_lineColorIndex,

	SC_lineWidth,
	SC_lineCap_Butt, SC_lineCap_Square, SC_lineCap_Round,
	SC_lineJoin_Miter, SC_lineJoin_Round, SC_lineJoin_Bevel,

	SC_fillEvenOdd_true, SC_fillEvenOdd_false,

	SC_moveTo, SC_moveRel,
	SC_lineTo, SC_lineRel,
	SC_horLineTo, SC_horLineRel,
	SC_verLineTo, SC_verLineRel,
	SC_arcTo_as, SC_arcTo_aS, SC_arcTo_As, SC_arcTo_AS,
	SC_arcRel_as, SC_arcRel_aS, SC_arcRel_As, SC_arcRel_AS,
	SC_quadricCurveTo, SC_quadricCurveRel,
        SC_quadricCurveTo2, SC_quadricCurveRel2,
	SC_cubicCurveTo, SC_cubicCurveRel,
        SC_cubicCurveTo4, SC_cubicCurveRel4,

	SC_addEllipseCW, SC_addEllipseCCW,

    SC_closePolygon,

    SC_drawPath_Fill, SC_drawPath_Stroke, SC_drawPath, SC_drawPath_FWLC,
    SC_drawPathNT_Fill, SC_drawPathNT_Stroke, SC_drawPathNT, SC_drawPathNT_FWLC,
    SC_polygon, SC_polyline,

	SC_line, SC_triangle, SC_rectangle, SC_roundedRect, SC_roundedRect2, SC_roundedRect4,
	SC_ellipse, SC_arc, SC_star, SC_curve, SC_curve4,

} SymbolCommand;




class Symbol {

protected:
public:
    string		   name;
    SymbolCommand* cmd;
    int            cmd_size;
    double*        data;
    int            data_size;
    double         x, y, w, h,	// bounding box
				   cx, cy;		// center of symbol
    agg::trans_affine tfm;

public:
    Symbol();
    Symbol(const Symbol &rhs);
    virtual Symbol& operator=(Symbol &s);
    virtual Symbol& copy();

    virtual ~Symbol();
    virtual void clear();

    virtual void draw(Graphics& g);
	virtual void debug();

    virtual void move(double x, double y); // relative move
    virtual void position(double x, double y); // absolute position
    virtual void size(double w, double h); // absolute size
    virtual void resize(double w, double h); // relative resize
    virtual void fitin(double x, double y, double w, double h);

	virtual long store(unsigned char* buff, long buffsize, long begin);

    virtual char* strname();
    virtual void strname(char *n);

private:
	virtual void internal_minmax();


friend class SymbolCanvas;

};




#endif // SYMBOL_H_INCLUDED
