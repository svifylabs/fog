/******************************************************************************

								class SymbolCanvas

*******************************************************************************

ABSTRACT:

	A SymbolCanvas is a base to produce symbols. It could not be drawn to the screen
	but it could be transformed into a Symbol that is drawn on the screen.
	It works with dynamic lists to be easily added to.

******************************************************************************/

SymbolCanvas::SymbolCanvas() {
    hasminmax = false;
    hascenter = false;
    name = strdup("<UNDEF>");
}

SymbolCanvas::~SymbolCanvas() {
    clear();
}

Symbol& SymbolCanvas::finalize() {
    Symbol& ret = *new Symbol();

    ctrl.push_back(SC_endOfSymbol);

    ret.cmd_size = ctrl.size();
    ret.cmd = (SymbolCommand*)malloc(sizeof(SymbolCommand)*ret.cmd_size);
    std::list<SymbolCommand>::iterator ci=ctrl.begin();
    for (int i = 0; i<ret.cmd_size; i++, ci++) {
        ret.cmd[i] = *ci;
    }

    ret.data_size = data.size();
    ret.data = (double*)malloc(sizeof(double)*ret.data_size);
    std::list<double>::iterator di=data.begin();
    for (int i = 0; i<ret.data_size; i++, di++) {
        ret.data[i] = *di;
    }

    if (hasminmax) {
        ret.x = smm[0];
        ret.y = smm[1];
        ret.w = smm[2];
        ret.h = smm[3];
    } else {
        ret.internal_minmax();
    }

    if (hascenter) {
        ret.cx = smm[4];
        ret.cy = smm[5];
    } else {
        ret.cx = ret.x + ret.w/2;
        ret.cy = ret.y + ret.h/2;
    }
    ret.strname(name);
    logger.trace("Created symbol \"%s\" from SymbolCanvas: \"%s\"", ret.strname(), name);
    return ret;
}

void SymbolCanvas::clear() {
    ctrl.clear();
    data.clear();
    if (name!=0)
        free(name);
    name = 0;
    hasminmax = false;
    hascenter = false;
}

void SymbolCanvas::debug() {
	logger.debug("DUMP SymbolCanvas @%08x",(void*)this);
	list<double>::iterator di = data.begin();
	for (int i = 0; di!=data.end(); di++) {
		logger.debug("    Data [%2i]: %.0f",i,*di);
	}
}

// observe reverse order of arguments for c calling convention to be able to draw fast

void SymbolCanvas::minmax(double xmin, double ymin, double xmax, double ymax) {
    hasminmax = true;
    smm[0] = xmin;
    smm[1] = ymin;
    smm[2] = xmax;
    smm[3] = ymax;
}

void SymbolCanvas::center(double cx, double cy) {
    hascenter = true;
    smm[4] = cx;
    smm[5] = cy;
}

void SymbolCanvas::fillColor(Graphics::Color c) {
    double evilcast = *(double*)(&c);
    ctrl.push_back(SC_fillColor);
    data.push_back(evilcast);
}
void SymbolCanvas::fillColor(unsigned r, unsigned g, unsigned b, unsigned a) {
    Graphics::Color c(r,g,b,a);
    double evilcast = *(double*)(&c);
    ctrl.push_back(SC_fillColor);
    data.push_back(evilcast);
}
void SymbolCanvas::noFill() { ctrl.push_back(SC_noFill); }
void SymbolCanvas::fillColorIndex(unsigned i) {
    double evilcast = *(double*)(&i);
    ctrl.push_back(SC_fillColorIndex);
    data.push_back(evilcast);
}


void SymbolCanvas::lineColor(Graphics::Color c){
    double evilcast = *(double*)(&c);
    ctrl.push_back(SC_lineColor);
    data.push_back(evilcast);
}
void SymbolCanvas::lineColor(unsigned r, unsigned g, unsigned b, unsigned a) {
    Graphics::Color c(r,g,b,a);
    double evilcast = *(double*)(&c);
    ctrl.push_back(SC_lineColor);
    data.push_back(evilcast);
}
void SymbolCanvas::noLine() { ctrl.push_back(SC_noFill); }
void SymbolCanvas::lineColorIndex(unsigned i) {
    double evilcast = *(double*)(&i);
    ctrl.push_back(SC_lineColorIndex);
    data.push_back(evilcast);
}


void SymbolCanvas::lineWidth(double w) {
    ctrl.push_back(SC_lineWidth);
    data.push_back(w);
}

void SymbolCanvas::lineCap(Graphics::LineCap cap) {
    switch(cap) {
    case Graphics::CapButt:   ctrl.push_back(SC_lineCap_Butt); break;
    case Graphics::CapSquare: ctrl.push_back(SC_lineCap_Square); break;
    case Graphics::CapRound:  ctrl.push_back(SC_lineCap_Round); break;
    }
}

void SymbolCanvas::lineJoin(Graphics::LineJoin join){
    switch(join) {
    case Graphics::JoinMiter: ctrl.push_back(SC_lineJoin_Miter); break;
    case Graphics::JoinRound: ctrl.push_back(SC_lineJoin_Round); break;
    case Graphics::JoinBevel: ctrl.push_back(SC_lineJoin_Bevel); break;
    }
}

void SymbolCanvas::fillEvenOdd(bool evenOddFlag) {
    if (evenOddFlag)
        ctrl.push_back(SC_fillEvenOdd_true);
    else
        ctrl.push_back(SC_fillEvenOdd_false);
}

void SymbolCanvas::moveTo(double x, double y) {
    ctrl.push_back(SC_moveTo);
    data.push_back(y);
    data.push_back(x);
}

void SymbolCanvas::moveRel(double dx, double dy) {
    ctrl.push_back(SC_moveRel);
    data.push_back(dy);
    data.push_back(dx);
}

void SymbolCanvas::lineTo(double x, double y) {
    ctrl.push_back(SC_lineTo);
    data.push_back(y);
    data.push_back(x);
}

void SymbolCanvas::lineRel(double dx, double dy) {
    ctrl.push_back(SC_lineRel);
    data.push_back(dy);
    data.push_back(dx);
}

void SymbolCanvas::horLineTo(double x) {
    ctrl.push_back(SC_horLineTo);
    data.push_back(x);
}
void SymbolCanvas::horLineRel(double dx){
    ctrl.push_back(SC_horLineRel);
    data.push_back(dx);
}

void SymbolCanvas::verLineTo(double y){
    ctrl.push_back(SC_verLineTo);
    data.push_back(y);
}

void SymbolCanvas::verLineRel(double dy){
    ctrl.push_back(SC_verLineRel);
    data.push_back(dy);
}


void SymbolCanvas::arcTo(double rx, double ry, double angle, bool largeArcFlag, bool sweepFlag, double x, double y) {

    if (largeArcFlag) {
        if (sweepFlag)
            ctrl.push_back(SC_arcTo_AS);
        else
            ctrl.push_back(SC_arcTo_As);
    } else {
        if (sweepFlag)
            ctrl.push_back(SC_arcTo_aS);
        else
            ctrl.push_back(SC_arcTo_as);
    }

    data.push_back(y);
    data.push_back(x);
    data.push_back(angle);
    data.push_back(ry);
    data.push_back(rx);
}

void SymbolCanvas::arcRel(double rx, double ry, double angle, bool largeArcFlag, bool sweepFlag, double dx, double dy) {

    if (largeArcFlag) {
        if (sweepFlag)
            ctrl.push_back(SC_arcRel_AS);
        else
            ctrl.push_back(SC_arcRel_As);
    } else {
        if (sweepFlag)
            ctrl.push_back(SC_arcRel_aS);
        else
            ctrl.push_back(SC_arcRel_as);
    }

    data.push_back(dy);
    data.push_back(dx);
    data.push_back(angle);
    data.push_back(ry);
    data.push_back(rx);
}

void SymbolCanvas::quadricCurveTo(double xCtrl, double yCtrl, double xTo, double yTo) {
    ctrl.push_back(SC_quadricCurveTo);
    data.push_back(yTo);
    data.push_back(xTo);
    data.push_back(yCtrl);
    data.push_back(xCtrl);
}

void SymbolCanvas::quadricCurveRel(double dxCtrl, double dyCtrl, double dxTo, double dyTo) {
    ctrl.push_back(SC_quadricCurveRel);
    data.push_back(dyTo);
    data.push_back(dxTo);
    data.push_back(dyCtrl);
    data.push_back(dxCtrl);
}

void SymbolCanvas::quadricCurveTo(double xTo, double yTo) {
    ctrl.push_back(SC_quadricCurveTo2);
    data.push_back(yTo);
    data.push_back(xTo);
}

void SymbolCanvas::quadricCurveRel(double dxTo, double dyTo) {
    ctrl.push_back(SC_quadricCurveRel2);
    data.push_back(dyTo);
    data.push_back(dxTo);
}


void SymbolCanvas::cubicCurveTo(double xCtrl1, double yCtrl1, double xCtrl2, double yCtrl2, double xTo, double yTo) {
    ctrl.push_back(SC_cubicCurveTo);
    data.push_back(yTo);
    data.push_back(xTo);
    data.push_back(yCtrl2);
    data.push_back(xCtrl2);
    data.push_back(yCtrl1);
    data.push_back(xCtrl1);
}

void SymbolCanvas::cubicCurveRel(double dxCtrl1, double dyCtrl1, double dxCtrl2, double dyCtrl2, double dxTo, double dyTo) {
    ctrl.push_back(SC_cubicCurveRel);
    data.push_back(dyTo);
    data.push_back(dxTo);
    data.push_back(dyCtrl2);
    data.push_back(dxCtrl2);
    data.push_back(dyCtrl1);
    data.push_back(dxCtrl1);
}

void SymbolCanvas::cubicCurveTo(double xCtrl2, double yCtrl2, double xTo, double yTo) {
    ctrl.push_back(SC_cubicCurveTo4);
    data.push_back(yTo);
    data.push_back(xTo);
    data.push_back(yCtrl2);
    data.push_back(xCtrl2);
}

void SymbolCanvas::cubicCurveRel(double dxCtrl2, double dyCtrl2, double dxTo, double dyTo) {
    ctrl.push_back(SC_cubicCurveRel4);
    data.push_back(dyTo);
    data.push_back(dxTo);
    data.push_back(dyCtrl2);
    data.push_back(dxCtrl2);
}


void SymbolCanvas::addEllipse(double cx, double cy, double rx, double ry, Graphics::Direction dir) {
    if (dir==Graphics::CW)
        ctrl.push_back(SC_addEllipseCW);
    else
        ctrl.push_back(SC_addEllipseCCW);
    data.push_back(ry);
    data.push_back(rx);
    data.push_back(cy);
    data.push_back(cx);
}

void SymbolCanvas::addEllipse(double cx, double cy, double rx, double ry, bool dir) {
    if (dir)
        ctrl.push_back(SC_addEllipseCW);
    else
        ctrl.push_back(SC_addEllipseCCW);

    data.push_back(ry);
    data.push_back(rx);
    data.push_back(cy);
    data.push_back(cx);
}

// do not reverse polygon and polyline

void SymbolCanvas::polygon(double* xy, int numPoints) {
    double *p = xy;
    ctrl.push_back(SC_polygon);
    data.push_back(numPoints);

    for(int i=0; i<numPoints; i++) {
        data.push_back(*p++);
        data.push_back(*p++);
    }
}

void SymbolCanvas::polyline(double* xy, int numPoints) {
    double *p = xy;
    ctrl.push_back(SC_polyline);
    data.push_back(numPoints);

    for(int i=0; i<numPoints; i++) {
        data.push_back(*p++);
        data.push_back(*p++);
    }
}


void SymbolCanvas::closePolygon() { ctrl.push_back(SC_closePolygon); }

void SymbolCanvas::drawPath(Graphics::DrawPathFlag flag) {
    switch(flag) {
    case Graphics::FillOnly:             ctrl.push_back(SC_drawPath_Fill); break;
    case Graphics::StrokeOnly:           ctrl.push_back(SC_drawPath_Stroke); break;
    case Graphics::FillAndStroke:        ctrl.push_back(SC_drawPath); break;
    case Graphics::FillWithLineColor:    ctrl.push_back(SC_drawPath_FWLC); break;
    }
}

void SymbolCanvas::drawPathNoTransform(Graphics::DrawPathFlag flag) {
    switch(flag) {
    case Graphics::FillOnly:             ctrl.push_back(SC_drawPathNT_Fill); break;
    case Graphics::StrokeOnly:           ctrl.push_back(SC_drawPathNT_Stroke); break;
    case Graphics::FillAndStroke:        ctrl.push_back(SC_drawPathNT); break;
    case Graphics::FillWithLineColor:    ctrl.push_back(SC_drawPathNT_FWLC); break;
    }
}

void SymbolCanvas::line(double x1, double y1, double x2, double y2) {
    ctrl.push_back(SC_line);

    data.push_back(y2);
    data.push_back(x2);
    data.push_back(y1);
    data.push_back(x1);
}

void SymbolCanvas::triangle(double x1, double y1, double x2, double y2, double x3, double y3) {
    ctrl.push_back(SC_triangle);

    data.push_back(y3);
    data.push_back(x3);
    data.push_back(y2);
    data.push_back(x2);
    data.push_back(y1);
    data.push_back(x1);
}

void SymbolCanvas::rectangle(double x1, double y1, double x2, double y2) {
    ctrl.push_back(SC_rectangle);

    data.push_back(y2);
    data.push_back(x2);
    data.push_back(y1);
    data.push_back(x1);
}

void SymbolCanvas::roundedRect(double x1, double y1, double x2, double y2, double r) {
    ctrl.push_back(SC_roundedRect);

    data.push_back(r);
    data.push_back(y2);
    data.push_back(x2);
    data.push_back(y1);
    data.push_back(x1);

}

void SymbolCanvas::roundedRect(double x1, double y1, double x2, double y2, double rx, double ry)  {
    ctrl.push_back(SC_roundedRect2);

    data.push_back(ry);
    data.push_back(rx);
    data.push_back(y2);
    data.push_back(x2);
    data.push_back(y1);
    data.push_back(x1);
}

void SymbolCanvas::roundedRect(double x1, double y1, double x2, double y2,
        double rxBottom, double ryBottom, double rxTop, double ryTop)
{
    ctrl.push_back(SC_roundedRect4);

    data.push_back(ryTop);
    data.push_back(rxTop);
    data.push_back(ryBottom);
    data.push_back(rxBottom);
    data.push_back(y2);
    data.push_back(x2);
    data.push_back(y1);
    data.push_back(x1);
}

void SymbolCanvas::ellipse(double cx, double cy, double rx, double ry) {
    ctrl.push_back(SC_ellipse);

    data.push_back(ry);
    data.push_back(rx);
    data.push_back(cy);
    data.push_back(cx);
}

void SymbolCanvas::arc(double cx, double cy, double rx, double ry, double start, double sweep)  {
    ctrl.push_back(SC_arc);

    data.push_back(sweep);
    data.push_back(start);
    data.push_back(ry);
    data.push_back(rx);
    data.push_back(cy);
    data.push_back(cx);
}

void SymbolCanvas::star(double cx, double cy, double r1, double r2, double startAngle, int numRays) {
    ctrl.push_back(SC_arc);

    data.push_back((double)numRays);
    data.push_back(startAngle);
    data.push_back(r2);
    data.push_back(r1);
    data.push_back(cy);
    data.push_back(cx);
}

void SymbolCanvas::curve(double x1, double y1, double x2, double y2, double x3, double y3) {
    ctrl.push_back(SC_curve);

    data.push_back(y3);
    data.push_back(x3);
    data.push_back(y2);
    data.push_back(x2);
    data.push_back(y1);
    data.push_back(x1);
}

void SymbolCanvas::curve(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4) {
    ctrl.push_back(SC_curve4);

    data.push_back(y4);
    data.push_back(x4);
    data.push_back(y3);
    data.push_back(x3);
    data.push_back(y2);
    data.push_back(x2);
    data.push_back(y1);
    data.push_back(x1);
}


// argF
//
// Internal function to read a number of double from a string. only used in
// SymbolCanvas::load.
//
int argF(
        int num,        // number of double to read from strtok
        double* arg,    // store the read and converted result here
        int& argAt      // first index of converted value
    )
{
    char *str = strtok(0, " \t");
    int at = 0;
    while (str!=0 && at<num) {
        int ret = sscanf(str,"%lf",&arg[argAt]);
        if (ret!=1)
            return -at; // ERROR: failed to parse a double from string
        argAt++;
        at++;
        str = strtok(0, " \t");
    }
    return at; // returning numer of arguments converted
}


// argI
//
// Internal function to read a number of integers from a string. only used in
// SymbolCanvas::load.
//
int argI(
        int num,        // number of integers to read from strtok
        int* arg,       // store the read and converted result here
        int& argAt      // first index of converted value
    )
{
    char *str = strtok(0, " \t");
    int at = 0;
    while (str!=0 && at<num) {
        int ret = sscanf(str,"%i",&arg[argAt]);
        if (ret!=1)
            return -at; // ERROR: failed to parse an integer from string
        argAt++;
        at++;
        str = strtok(0, " \t");
    }
    return at; // returns numer of arguments converted
}


// load
//
// reads a symbol from a buffer, returns begin plus number of bytes loaded
//
// TODO: returncodes is not nice
long SymbolCanvas::load(
		char* buff, 	        // buffer to read a symbol definition from
		long buffsize,		    // how large is the buffer
		long begin			    // position in buffer to begin load from
	)
{
	char *bptr = (char*)&buff[begin], *tok, linebuff[100], *bline;
	long at = 0, len;
	bool goon = true;
	Graphics::Color c;

    //logger.trace("SymbolCanvas::load buffsize = %ld, begin = %ld,\n%s\n",buffsize,begin,&buff[begin]);
	clear();

    int lineno = 0;
    while(at<buffsize && goon) {

		// get one line into linebuff, increment all pointers and counters
		bline = strchr(bptr, '\n');
		len = bline-bptr;
		if (bline == NULL) {
			return 0; // ERROR: no line found, buffer empty
		}
		strncpy(linebuff, bptr, len);
		linebuff[len] = '\0';
		bptr = bline+1;
		at += len+1;

        lineno++;
        logger.trace("%08d: %s", lineno, linebuff);

		// load strtok
		tok = strtok(linebuff, " \t");
		if (tok==0)
			continue; // empty line

        if (tok[0]=='#')
            continue;

		double a[10];
		int argAt = 0, argi[4], argiAt = 0;
		bool flagTo = false;
		Graphics::DrawPathFlag dpf = Graphics::FillAndStroke;

        double *polygonline;
        double *atpl;
		switch(tok[0]) {

					// A|a rx,ry rot af sf x,y -> SC_arcTo_?? | SC_arcRel_??
		case 'A':	flagTo = true;
		case 'a':	argF(3,a,argAt); argI(2,argi,argiAt); argF(2,a,argAt);
					if (flagTo) arcTo(a[0], a[1], a[2], (argi[0]==1), (argi[1]==0), a[3], a[4]);
					else arcTo(a[0], a[1], a[2], (argi[0]==1), (argi[1]==0), a[3], a[4]);
					break;

					// bB x1,y1 x2,y2 -> SC_boundingBox
		case 'b':	argF(4,a,argAt); minmax(a[0], a[1], a[2], a[3]); break;

					// C|c c1x,c1y c2x,c2y x,y -> SC_cubicCurveTo | SC_cubicCurveRel
					// cP -> SC_closePolygon
					// cS x,y -> SC_centerOfSymbol
		case 'C':	flagTo = true;
		case 'c':	if (strncmp(tok,"cP",2)==0) { closePolygon(); }
                    else if (strncmp(tok,"cS",2)==0) { argF(2,a,argAt); center(a[0], a[1]); }
                    else {
                        argF(6,a,argAt);
                        if (flagTo) cubicCurveTo(a[0], a[1], a[2], a[3], a[4], a[5]);
                        else cubicCurveRel(a[0], a[1], a[2], a[3], a[4], a[5]);
                    }
                    break;

					// D|d[fs l] -> SC_drawPath<NT>_[Fill|Stroke| |FWLC]
		case 'D':	flagTo = true;
		case 'd':	if (tok[1]=='f')      dpf = Graphics::FillOnly;
                    else if (tok[1]=='s') dpf = Graphics::StrokeOnly;
                    else if (tok[1]=='l') dpf = Graphics::FillWithLineColor;
                    if (flagTo) drawPath(dpf);
                    else drawPathNoTransform(dpf);
                    break;

                    //E|e*llipse cx,cy rx,ry		-> SC_addEllipseCW|CCW
		case 'E':	flagTo = true;
		case 'e':	argF(4,a,argAt);
                    addEllipse(a[0], a[1], a[2], a[3], flagTo);
                    break;

					// fC c -> SC_fillColor
					// f[f|t] -> SC_fillEvenOdd
					// fI c -> SC_fillColorIndex
		case 'f':   if (tok[1]=='C') {
                        argI(4,argi,argiAt);
                        fillColor(argi[0], argi[1], argi[2], argi[3]);
                    } else {
                    	if (tok[1]=='I') {
                    		argI(1,argi,argiAt);
                    		fillColorIndex(argi[0]);
                    	} else {
							if (tok[1]=='f')
								fillEvenOdd(false);
							else
								fillEvenOdd(true);
                    	}
                    }
                    break;

                    // H|h x -> SC_horLine[To|Rel]
        case 'H':   argF(1,a,argAt); horLineTo(a[0]); break;
        case 'h':   argF(1,a,argAt); horLineRel(a[0]); break;

                    // j[MRB] -> lineJoin_[Mitter|Round|Bevel]
        case 'j':   if (tok[1]=='M') { lineJoin(Graphics::JoinMiter); break; }
                    if (tok[1]=='R') { lineJoin(Graphics::JoinRound); break; }
                    if (tok[1]=='B') { lineJoin(Graphics::JoinBevel); break; }
                    break; // ERROR: ?

                    // L|l x,y -> SC_line[To|Rel]
                    // li*ne  x1,y1 x2,y2 -> SC_line
                    // l[BRS] -> SC_lineCap_[Butt|Round|Square]
                    // lC c -> SC_lineColor
                    // lW w -> SC_lineWidth
                    // lI c -> SC_lineColorIndex
        case 'L':   argF(2,a,argAt); lineTo(a[0], a[1]); break;
        case 'l':   if (tok[1]==0) { argF(2,a,argAt); lineRel(a[0], a[1]); break; }
                    if (tok[1]=='i') { argF(4,a,argAt); line(a[0], a[1], a[2], a[3]); break; }
                    if (tok[1]=='R') { lineCap(Graphics::CapRound); break; }
                    if (tok[1]=='B') { lineCap(Graphics::CapButt); break; }
                    if (tok[1]=='S') { lineCap(Graphics::CapSquare); break; }
                    if (tok[1]=='C') { argI(4,argi,argiAt); lineColor(argi[0], argi[1], argi[2], argi[3]); break; }
                    if (tok[1]=='W') { argF(1,a,argAt); lineWidth(a[0]); break;}
                    if (tok[1]=='I') { argI(1,argi,argiAt); lineColorIndex(argi[0]); break; }
                    logger.error("While reading symbol %s, does not understand command: \"%s\"", name, tok);
                    break; // ERROR: ?

                    // M|m x,y -> SC_move[To|Rel]
        case 'M':   argF(2,a,argAt); lineTo(a[0], a[1]); break;
        case 'm':   argF(2,a,argAt); lineRel(a[0], a[1]); break;
                    logger.error("While reading symbol %s, does not understand command: \"%s\"", name, tok);
                    break; // ERROR: ?

                    // n[F|L] -> SC_noFill | SC_noLine
        case 'n':   if (tok[1]=='F') { noFill(); break; }
                    if (tok[1]=='L') { noLine(); break; }
                    logger.error("While reading symbol %s, does not understand command: \"%s\"", name, tok);
                    break; // ERROR: ?

                    // P|p num (x,y)*num -> SC_polygon | SC_polyline
        case 'P':   flagTo = true;
        case 'p':   argI(1, argi, argiAt);
                    polygonline = (double*)malloc(sizeof(double)*argi[0]*2);
                    atpl = polygonline;
                    for(int i=0; i<argi[0]; i++) {
                        argF(2,a,argAt);
                        *atpl++ = a[0];
                        *atpl++ = a[1];
                    }
                    if (flagTo) { polygon(polygonline,argi[0]); }
                    else        { polyline(polygonline,argi[0]); }
                    free(polygonline);
                    break;

                    // Q|q cx,cy x,y -> SC_quadricCurve[To|Rel]
        case 'Q':   argF(4,a,argAt); quadricCurveTo(argi[0], argi[1], argi[2], argi[3]); break;
        case 'q':   argF(4,a,argAt); quadricCurveRel(argi[0], argi[1], argi[2], argi[3]); break;

                    // rect x1,y1 x2,y2 -> SC_rectangle
                    // rRect x1,y1 x2,y2 r -> SC_roundedRect
                    // rRect2 x1,y1 x2,y2 r1,r2 -> SC_roundedRect2
                    // rRrct4 x1,y1 x2,y2 r1,r2,r3,r4 -> SC_roundedRect4
        case 'r':   if (strncmp(tok,"rect",4)==0) { argF(4,a,argAt); rectangle(a[0], a[1], a[2], a[3]); break; }
                    if (strncmp(tok,"rRect",4)==0) { argF(5,a,argAt); roundedRect(a[0], a[1], a[2], a[3], a[4]); break; }
                    if (strncmp(tok,"rRect2",4)==0) { argF(6,a,argAt); roundedRect(a[0], a[1], a[2], a[3], a[4], a[5]); break; }
                    if (strncmp(tok,"rRect4",4)==0) { argF(8,a,argAt); roundedRect(a[0], a[1], a[2], a[3], a[4], a[5], a[6], a[7]); break; }
                    logger.error("While reading symbol %s, does not understand command: \"%s\"", name, tok);
                    break; // ERROR: ?

                    // S|s cx,cy x,y -> SC_cubicCurve[To|Rel]4
                    // symbol <NAME> { -> name
        case 'S':   argF(4,a,argAt); cubicCurveTo(a[0], a[1], a[2], a[3]); break;
        case 's':   if (tok[1]==0) { argF(4,a,argAt); cubicCurveRel(a[0], a[1], a[2], a[3]); break; }
                    if (strncmp(tok,"symbol",6)==0) {
                        char *from, *to;
                        from = tok+7;
                        to = strchr(from,'{');
                        if (to < from)
                            to = strchr(from,'\n');
                        if (to < from) {
                            logger.error("failed to read \"symbol <name> {\" definition");
                            return 0; // ERROR: failed to read "symbol <name> {" definition
                        }
                        *to = 0; to--;
                        while(*to==' '||*to=='\t') { *to = 0; to--; }
                        while(*from==' '||*from=='\t')
                            from++;
                        name = strdup(from);
                        logger.trace("begin parsing symbol: %s", name);
                        break;
                    }
                    logger.error("While reading symbol %s, does not understand command: \"%s\"", name, tok);
                    break; // ERROR: ?

        case '{':   continue;

        case '}':   goon = false;
                    if (name==0) {
                        logger.error("Symbol read without name.");
                        return 0;
                    } else
                        logger.trace("end   parsing symbol: %s", name);
                    break;

                    // T|t x,y -> SC_quadricCurve[To|Rel]2
        case 'T':   argF(2,a,argAt); quadricCurveTo(a[0], a[1]); break;
        case 't':   if (tok[1]==0) { argF(2,a,argAt); quadricCurveRel(a[0], a[1]); break; }
                    if (tok[1]=='r') { argF(6,a,argAt); triangle(a[0], a[1], a[2], a[3], a[4], a[5]); break; }
                    logger.error("While reading symbol %s, does not understand command: \"%s\"", name, tok);
                    break; // ERROR: ?

                    // V|v x -> SC_verLine[To|Rel]
        case 'V':   argF(1,a,argAt); verLineTo(a[0]); break;
        case 'v':   argF(1,a,argAt); verLineRel(a[0]); break;

                    // Z -> SC_closePolygon
        case 'Z':   closePolygon(); break;

        default:
                    logger.error("While reading symbol %s, does not understand command: \"%s\"", name, tok);
                    break; // ERROR: ?
        }
    }
   	if (goon)
		return 0; // buffer full, all of symbol not written to buffer

	//logger.trace("SymbolCanvas::load buffer advance = %d", at);
	return at; // return positive new location to continue to fill buffer
}
