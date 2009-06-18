// [Fog/Svg Library - C++ API]
//
// [Licence]
// MIT, See COPYING.txt file in package
// Author: Anders Jansson, Copyright(c) 2007,2009 <l8.jansson@gmail.com>


/*******************************************************************************

								class Symbol

********************************************************************************

ABSTRACT:

	A Symbol is a collection of lines and polygons that make up an symbol. This
	could be moved around and drawn on the screen. It is reated and packed from
	a SymbolCanvas for fast rendering and small size.

********************************************************************************

INTERNAL DATA DOCUMENTATION:

    name: string
		This is the name of the symbol. Refered in Symbolmanager to know what
		symbol should be found.

    cmd: SymbolCommand*
		This is an pod array of SymbolCommands that directs how to draw it. The
		structure is allocated with malloc and should be freed on destruction.

    cmd_size: int
		Tells how many commands is there in the cmd array.

    data: double*
		This is a pod array of double used by the symbol commands to draw with.
		ie. "draw line" needs two doubles to know where to draw the line.
		NOTE: arguments are stored backwards for easy calling.

    data_size: int
		Tells how many doubles there are in the data array.

    x, y, w, h:	double(s)
		This is the bounding box of the original symbol. This bounding box will
		not change during the execution of move, size or other functions in the
		symbol (except with minmax function).

	cx, cy: double
		This is the center of the symbol. Used for rotation or resizes.

    tfm: agg::trans_affine
		This describes the transformations done on the symbol. If it has moved
		or been rescaled this will deviate from the Unit Matrix. (Not really a
		matrix but see the agg_trans_affine.h for details).
		It is used when the symbol is drawn on the screen so it preserves the
		position and size of the symbol.


********************************************************************************

IDEAS:

1	We could make the symbol have a function to transform the underlying points
	in the data array. But this will be fairly comlicated.

2	Symbol creator from SymbolCanvas.

3	splitting on SymbolCanvas and symbol might be bad. (later: is really bad)

*******************************************************************************/

#include "Symbol.h"

/// creation of an empty Symbol
Symbol::Symbol() {
    cmd = 0;  cmd_size = 0;
    data = 0; data_size = 0;
    tfm.reset();
}

/// copy constructor
Symbol::Symbol(
	const Symbol &rhs	// the "right hand side" symbol to be copied
) {
    cmd_size = rhs.cmd_size;
    long num_bytes = sizeof(SymbolCommand)*cmd_size;
    cmd = (SymbolCommand*)malloc(num_bytes);
    memcpy(cmd,rhs.cmd, num_bytes);

    data_size = rhs.data_size;
    num_bytes = sizeof(double)*data_size;
    data = (double*)malloc(num_bytes);
    memcpy(data,rhs.data, num_bytes);

    x = rhs.x; y = rhs.y; w = rhs.w; h = rhs.h;
    cx = rhs.cx; cy = rhs.cy;

	name = rhs.name;
	tfm = rhs.tfm;
}

/// assignment operator
Symbol& Symbol::operator=(
	Symbol &rhs		// the "right hand side" symbol to be copied
) {
	clear();

    cmd_size = rhs.cmd_size;
    long num_bytes = sizeof(SymbolCommand)*cmd_size;
    cmd = (SymbolCommand*)malloc(num_bytes);
    memcpy(cmd,rhs.cmd, num_bytes);

    data_size = rhs.data_size;
    num_bytes = sizeof(double)*data_size;
    data = (double*)malloc(num_bytes);
    memcpy(data,rhs.data, num_bytes);

    x = rhs.x; y = rhs.y; w = rhs.w; h = rhs.h;
    cx = rhs.cx; cy = rhs.cy;
    tfm = rhs.tfm;

	name = rhs.name;

    return *this;
}

/// return a copy of the symbol
Symbol& Symbol::copy()
{
    Symbol& ret = *new Symbol(*this);
    return ret;
}

/// destructor of the symbol
Symbol::~Symbol() {
	clear();
}

/// free the data allocated by the symbiol
void Symbol::clear() {
    if (cmd!=0)
        free(cmd);
    if (data!=0)
        free(data);
	name = "";
}

/// return the name of the symbol
char* Symbol::strname()
{
    return strdup(name.c_str());
}

/// set the name of the symbol
void Symbol::strname(
	char *n		// the new name of the symbol
) {
    name = n;
}

/// show the symbol in the logfile
/// TODO: finish me
void Symbol::debug()
{
	logger.debug("DUMP Symbol @%08x",(void*)this);
	for (int i=0; i<data_size; i++) {
		logger.debug("    Data [%2i]: %.0f",i,data[i]);
	}
}

/// Relative move the symbol (ie nudge the symbol around)
void Symbol::move(
	double x, double y	// cumulative new position (x+x', y+y')
) {
    tfm.translate(x, y);
}

/// Set the position of the symbol. The old position has nothing to do with
/// the old position of the symbol.
void Symbol::position(
	double x, double y	// absolute new position (x, y)
) {
    tfm.translate(x-this->x, y-this->y);
}

// TODO (ajn#1#): Untested code
/// This will set the symbol to a new size (will operate from the center of
/// the symbol). It will preserve the aspect ratio of the symbol.
void Symbol::size(
	double w, double h	// new absolute width and heigth of the symbol
) {
    double scalex = w/this->w;
    double scaley = h/this->h;
    if (scalex<scaley) {
		tfm.reset();
        tfm.translate(-this->cx, -this->cy);
        tfm.scale(scalex);
        tfm.translate(this->cx, this->cy);
    } else {
		tfm.reset();
        tfm.translate(-this->cx, -this->cy);
        tfm.scale(scaley);
        tfm.translate(this->cx, this->cy);
    }
}

// TODO (ajn#1#): implement if needed
/// This will change the size of the symbol. w and h is in percent.
/// Should preserve aspect ratio.
void Symbol::resize(
	double w, double h	// percentage difference of the symbol
) {

}


/// move and stretch the symbol to fit box (x,y,x+w,y+h) and preserve aspect
/// will operate on tfm that is used when drawing the symbol.
void Symbol::fitin(
        double x, double y,     // new top left position of symbol
        double w, double h      // new width and height of symbol
    )
{
    double scalex = w/this->w;
    double scaley = h/this->h;
    if (scalex<scaley) {
		tfm.reset();
        tfm.translate(-this->x, -this->y);
        tfm.scale(scalex);
        tfm.translate(x, y + (h-this->h*scalex)/2);
    } else {
		tfm.reset();
        tfm.translate(-this->x, -this->y);
        tfm.scale(scaley);
        tfm.translate(x + (w-this->w*scaley)/2, y);
    }
}









// destructively set transformation so you have to store it before drawing a symbol.
void Symbol::draw(Graphics& g) {

    SymbolCommand* sym = cmd;
    double*        ptr = data;
    int pts;
    bool goon=true;

    g.affine(tfm);

    while(goon) {
        switch(*sym++) {
        case SC_endOfSymbol:       goon=false; break;
        case SC_noFill:            g.noFill(); break;
        case SC_fillColor:         g.fillColor(*(Graphics::Color*)ptr++); break;
        case SC_fillColorIndex:    g.fillColorIndex(*(unsigned*)ptr++); break;

        case SC_lineColor:         g.lineColor(*(Graphics::Color*)ptr++); break;
        case SC_lineColorIndex:    g.lineColorIndex(*(unsigned*)ptr++); break;
        case SC_noLine:            g.noLine(); break;

        case SC_lineWidth:         g.lineWidth(*ptr++); break;
        case SC_lineCap_Butt:      g.lineCap(Graphics::CapButt); break;
        case SC_lineCap_Square:    g.lineCap(Graphics::CapSquare); break;
        case SC_lineCap_Round:     g.lineCap(Graphics::CapRound); break;
        case SC_lineJoin_Miter:    g.lineJoin(Graphics::JoinMiter); break;
        case SC_lineJoin_Round:    g.lineJoin(Graphics::JoinRound); break;
        case SC_lineJoin_Bevel:    g.lineJoin(Graphics::JoinBevel); break;
        case SC_fillEvenOdd_true:  g.fillEvenOdd(true); break;
        case SC_fillEvenOdd_false: g.fillEvenOdd(false); break;

        case SC_moveTo:            g.moveTo(*ptr++,*ptr++); break;
        case SC_moveRel:           g.moveRel(*ptr++,*ptr++); break;
        case SC_lineTo:            g.lineTo(*ptr++,*ptr++); break;
        case SC_lineRel:           g.lineRel(*ptr++,*ptr++); break;
        case SC_horLineTo:         g.horLineTo(*ptr++); break;
        case SC_horLineRel:        g.horLineRel(*ptr++); break;
        case SC_verLineTo:         g.verLineTo(*ptr++); break;
        case SC_verLineRel:        g.verLineRel(*ptr++); break;

        case SC_arcTo_as:          g.arcTo(*ptr++, *ptr++, *ptr++, false, false, *ptr++, *ptr++); break;
        case SC_arcTo_aS:          g.arcTo(*ptr++, *ptr++, *ptr++, false, true, *ptr++, *ptr++); break;
        case SC_arcTo_As:          g.arcTo(*ptr++, *ptr++, *ptr++, true, false, *ptr++, *ptr++); break;
        case SC_arcTo_AS:          g.arcTo(*ptr++, *ptr++, *ptr++, true, true, *ptr++, *ptr++); break;
        case SC_arcRel_as:         g.arcRel(*ptr++, *ptr++, *ptr++, false, false, *ptr++, *ptr++); break;
        case SC_arcRel_aS:         g.arcRel(*ptr++, *ptr++, *ptr++, false, true, *ptr++, *ptr++); break;
        case SC_arcRel_As:         g.arcRel(*ptr++, *ptr++, *ptr++, true, false, *ptr++, *ptr++); break;
        case SC_arcRel_AS:         g.arcRel(*ptr++, *ptr++, *ptr++, true, true, *ptr++, *ptr++); break;

        case SC_quadricCurveTo:    g.quadricCurveTo(*ptr++, *ptr++, *ptr++, *ptr++); break;
        case SC_quadricCurveRel:   g.quadricCurveRel(*ptr++, *ptr++, *ptr++, *ptr++); break;
        case SC_quadricCurveTo2:   g.quadricCurveTo(*ptr++, *ptr++); break;
        case SC_quadricCurveRel2:  g.quadricCurveRel(*ptr++, *ptr++); break;

        case SC_cubicCurveTo:      g.cubicCurveTo(*ptr++, *ptr++, *ptr++, *ptr++, *ptr++, *ptr++);  break;
        case SC_cubicCurveRel:     g.cubicCurveRel(*ptr++, *ptr++, *ptr++, *ptr++, *ptr++, *ptr++);  break;
        case SC_cubicCurveTo4:     g.cubicCurveTo(*ptr++, *ptr++, *ptr++, *ptr++);  break;
        case SC_cubicCurveRel4:    g.cubicCurveRel(*ptr++, *ptr++, *ptr++, *ptr++);  break;

        case SC_addEllipseCW:      g.addEllipse(*ptr++, *ptr++, *ptr++, *ptr++, Graphics::CW); break;
        case SC_addEllipseCCW:     g.addEllipse(*ptr++, *ptr++, *ptr++, *ptr++, Graphics::CCW); break;

        case SC_polygon:           pts = (int)*ptr++; g.polygon(ptr,pts); ptr+=pts; break;
        case SC_polyline:          pts = (int)*ptr++; g.polyline(ptr,pts); ptr+=pts; break;

        case SC_closePolygon:      g.closePolygon(); break;
        case SC_drawPath_Fill:     g.drawPath(Graphics::FillOnly); g.resetPath(); break;
        case SC_drawPath_Stroke:   g.drawPath(Graphics::StrokeOnly); g.resetPath(); break;
        case SC_drawPath:          g.drawPath(Graphics::FillAndStroke); g.resetPath(); break;
        case SC_drawPath_FWLC:     g.drawPath(Graphics::FillWithLineColor); g.resetPath(); break;
        case SC_drawPathNT_Fill:   g.drawPathNoTransform(Graphics::FillOnly); g.resetPath(); break;
        case SC_drawPathNT_Stroke: g.drawPathNoTransform(Graphics::StrokeOnly); g.resetPath(); break;
        case SC_drawPathNT:        g.drawPathNoTransform(Graphics::FillAndStroke); g.resetPath(); break;
        case SC_drawPathNT_FWLC:   g.drawPathNoTransform(Graphics::FillWithLineColor); g.resetPath(); break;

        case SC_line:              g.line(*ptr++, *ptr++, *ptr++, *ptr++); break;
        case SC_triangle:          g.triangle(*ptr++, *ptr++, *ptr++, *ptr++, *ptr++, *ptr++); break;
        case SC_rectangle:         g.rectangle(*ptr++, *ptr++, *ptr++, *ptr++); break;

        case SC_roundedRect:       g.roundedRect(*ptr++, *ptr++, *ptr++, *ptr++, *ptr++); break;
        case SC_roundedRect2:      g.roundedRect(*ptr++, *ptr++, *ptr++, *ptr++, *ptr++, *ptr++); break;
        case SC_roundedRect4:      g.roundedRect(*ptr++, *ptr++, *ptr++, *ptr++, *ptr++, *ptr++, *ptr++, *ptr++); break;

        case SC_ellipse:           g.ellipse(*ptr++, *ptr++, *ptr++, *ptr++); break;
        case SC_arc:               g.arc(*ptr++, *ptr++, *ptr++, *ptr++, *ptr++, *ptr++); break;
        case SC_star:              g.star(*ptr++, *ptr++, *ptr++, *ptr++, *ptr++, (int)*ptr++); break;
        case SC_curve:             g.curve(*ptr++, *ptr++, *ptr++, *ptr++, *ptr++, *ptr++); break;
        case SC_curve4:            g.curve(*ptr++, *ptr++, *ptr++, *ptr++, *ptr++, *ptr++, *ptr++, *ptr++); break;
        }
	}
}





// store
//
// writes a symbol to file to be read with SymbolCanvas::load
//
long Symbol::store(
		unsigned char* buff,	// buffer to store this symbol to
		long buffsize,		    // how large is the buffer
		long begin			    // where do we start to write in the buffer
	)
{
	char   			*bptr = (char*)&buff[begin];
	int             ret;
	long            at = begin;
	bool 		    goon = true;
    SymbolCommand   *sym = cmd;
    double*         ptr = data;
	Graphics::Color *c;
	int 		    i, pts;

	ret = sprintf(bptr, "symbol %s {\n", name.c_str());
	if (ret<10) {
		return 0; // failed to print symbol headder to output buffer
	}
	bptr += ret; at += ret;

    while(at<buffsize && goon) {
        switch(*sym++) {
        case SC_endOfSymbol:       ret = sprintf(bptr,"}\n"); goon=false; break;
        case SC_fillColor:         c = (Graphics::Color*)ptr++; ret = sprintf(bptr, "  fC %d %d %d %d\n", c->r, c->g, c->b, c->a); break;
        case SC_noFill:            ret = sprintf(bptr, "  nF\n"); break;
        case SC_lineColorIndex:    pts = (int)*ptr++; ret = sprintf(bptr, "  lI %d\n", pts); break;

        case SC_lineColor:         c = (Graphics::Color*)ptr++; ret = sprintf(bptr, "  lC %d %d %d %d\n", c->r, c->g, c->b, c->a); break;
        case SC_noLine:            ret = sprintf(bptr, "  nL\n"); break;
        case SC_fillColorIndex:    pts = (int)*ptr++; ret = sprintf(bptr, "  fI %d\n", pts); break;

        case SC_lineWidth:         ret = sprintf(bptr, "  lW %f\n", *ptr++); break;
        case SC_lineCap_Butt:      ret = sprintf(bptr, "  lB\n"); break;
        case SC_lineCap_Square:    ret = sprintf(bptr, "  lS\n"); break;
        case SC_lineCap_Round:     ret = sprintf(bptr, "  lR\n"); break;
        case SC_lineJoin_Miter:    ret = sprintf(bptr, "  jM\n"); break;
        case SC_lineJoin_Round:    ret = sprintf(bptr, "  jR\n"); break;
        case SC_lineJoin_Bevel:    ret = sprintf(bptr, "  jB\n"); break;
        case SC_fillEvenOdd_true:  ret = sprintf(bptr, "  ft\n"); break;
        case SC_fillEvenOdd_false: ret = sprintf(bptr, "  ff\n"); break;

        case SC_moveTo:            ret = sprintf(bptr, "  M %f %f\n", *ptr++, *ptr++); break;
        case SC_moveRel:           ret = sprintf(bptr, "  m %f %f\n", *ptr++, *ptr++); break;
        case SC_lineTo:            ret = sprintf(bptr, "  L %f %f\n", *ptr++, *ptr++); break;
        case SC_lineRel:           ret = sprintf(bptr, "  l %f %f\n", *ptr++, *ptr++); break;
        case SC_horLineTo:         ret = sprintf(bptr, "  H %f\n", *ptr++); break;
        case SC_horLineRel:        ret = sprintf(bptr, "  h %f\n", *ptr++); break;
        case SC_verLineTo:         ret = sprintf(bptr, "  V %f\n", *ptr++); break;
        case SC_verLineRel:        ret = sprintf(bptr, "  v %f\n", *ptr++); break;

        case SC_arcTo_as:          ret = sprintf(bptr, "  A %f %f %f 0 0 %f %f\n", *ptr++, *ptr++, *ptr++, *ptr++, *ptr++); break;
        case SC_arcTo_aS:		   ret = sprintf(bptr, "  A %f %f %f 0 1 %f %f\n", *ptr++, *ptr++, *ptr++, *ptr++, *ptr++); break;
		case SC_arcTo_As:		   ret = sprintf(bptr, "  A %f %f %f 1 0 %f %f\n", *ptr++, *ptr++, *ptr++, *ptr++, *ptr++); break;
        case SC_arcTo_AS:          ret = sprintf(bptr, "  A %f %f %f 1 1 %f %f\n", *ptr++, *ptr++, *ptr++, *ptr++, *ptr++); break;

        case SC_arcRel_as:         ret = sprintf(bptr, "  a %f %f %f 0 0 %f %f\n", *ptr++, *ptr++, *ptr++, *ptr++, *ptr++); break;
        case SC_arcRel_aS:		   ret = sprintf(bptr, "  a %f %f %f 0 1 %f %f\n", *ptr++, *ptr++, *ptr++, *ptr++, *ptr++); break;
		case SC_arcRel_As:		   ret = sprintf(bptr, "  a %f %f %f 1 0 %f %f\n", *ptr++, *ptr++, *ptr++, *ptr++, *ptr++); break;
        case SC_arcRel_AS:         ret = sprintf(bptr, "  a %f %f %f 1 1 %f %f\n", *ptr++, *ptr++, *ptr++, *ptr++, *ptr++); break;

        case SC_quadricCurveTo:    ret = sprintf(bptr, "  Q %f %f %f %f\n", *ptr++, *ptr++, *ptr++, *ptr++); break;
        case SC_quadricCurveRel:   ret = sprintf(bptr, "  q %f %f %f %f\n", *ptr++, *ptr++, *ptr++, *ptr++); break;
        case SC_quadricCurveTo2:   ret = sprintf(bptr, "  T %f %f\n", *ptr++, *ptr++); break;
        case SC_quadricCurveRel2:  ret = sprintf(bptr, "  t %f %f\n", *ptr++, *ptr++); break;

        case SC_cubicCurveTo:      ret = sprintf(bptr, "  C %f %f %f %f %f %f\n", *ptr++, *ptr++, *ptr++, *ptr++, *ptr++, *ptr++); break;
        case SC_cubicCurveRel:     ret = sprintf(bptr, "  c %f %f %f %f %f %f\n", *ptr++, *ptr++, *ptr++, *ptr++, *ptr++, *ptr++); break;
        case SC_cubicCurveTo4:     ret = sprintf(bptr, "  S %f %f %f %f\n", *ptr++, *ptr++, *ptr++, *ptr++); break;
        case SC_cubicCurveRel4:    ret = sprintf(bptr, "  s %f %f %f %f\n", *ptr++, *ptr++, *ptr++, *ptr++); break;

        case SC_polygon:		   pts = (int)*ptr++; ret = sprintf(bptr, "  P %d",pts); for(i=0; i<pts; i++) { y = *ptr++; x = *ptr++; ret += sprintf(bptr, " %f %f", x, y); } break;
        case SC_polyline:          pts = (int)*ptr++; ret = sprintf(bptr, "  p %d",pts); for(i=0; i<pts; i++) { y = *ptr++; x = *ptr++; ret += sprintf(bptr, " %f %f", x, y); } break;

        case SC_closePolygon:      ret = sprintf(bptr, "  cP\n"); break;
        case SC_drawPath_Fill:     ret = sprintf(bptr, "  Df\n"); break;
        case SC_drawPath_Stroke:   ret = sprintf(bptr, "  Ds\n"); break;
        case SC_drawPath:          ret = sprintf(bptr, "  D\n"); break;
        case SC_drawPath_FWLC:     ret = sprintf(bptr, "  Dl\n"); break;
        case SC_drawPathNT_Fill:   ret = sprintf(bptr, "  df\n"); break;
        case SC_drawPathNT_Stroke: ret = sprintf(bptr, "  ds\n"); break;
        case SC_drawPathNT:        ret = sprintf(bptr, "  d\n"); break;
        case SC_drawPathNT_FWLC:   ret = sprintf(bptr, "  dl\n"); break;

        case SC_addEllipseCW:      ret = sprintf(bptr, "  Ellipse %f %f %f %f\n", *ptr++, *ptr++, *ptr++, *ptr++); break;
        case SC_addEllipseCCW:     ret = sprintf(bptr, "  ellipse %f %f %f %f\n", *ptr++, *ptr++, *ptr++, *ptr++); break;

        case SC_line:              ret = sprintf(bptr, "  line %f %f %f %f\n", *ptr++, *ptr++, *ptr++, *ptr++); break;
        case SC_triangle:          ret = sprintf(bptr, "  triangle %f %f %f %f %f %f\n", *ptr++, *ptr++, *ptr++, *ptr++, *ptr++, *ptr++); break;
        case SC_rectangle:         ret = sprintf(bptr, "  rect %f %f %f %f\n", *ptr++, *ptr++, *ptr++, *ptr++); break;

        case SC_roundedRect:       ret = sprintf(bptr, "  rRect %f %f %f %f %f\n", *ptr++, *ptr++, *ptr++, *ptr++, *ptr++); break;
        case SC_roundedRect2:      ret = sprintf(bptr, "  rRect2 %f %f %f %f %f %f\n", *ptr++, *ptr++, *ptr++, *ptr++, *ptr++, *ptr++); break;
        case SC_roundedRect4:      ret = sprintf(bptr, "  rRect4 %f %f %f %f %f %f %f %f\n", *ptr++, *ptr++, *ptr++, *ptr++, *ptr++, *ptr++, *ptr++, *ptr++); break;

/* TODO:
        case SC_ellipse:           4 break;
        case SC_arc:               6 break;
        case SC_star:              6 break;
        case SC_curve:             6 break;
        case SC_curve4:            8 break;
*/
        }
        bptr += ret; at += ret;
	}
	if (goon)
		return 0; // buffer full, all of symbol not written to buffer

	return begin+at; // return positive new location to continue to fill buffer
}


