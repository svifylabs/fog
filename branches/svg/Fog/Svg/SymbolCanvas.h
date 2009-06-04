#ifndef SYMBOLCANVAS_H_INCLUDED
#define SYMBOLCANVAS_H_INCLUDED

class SymbolCanvas {
	std::list<SymbolCommand> ctrl;
	std::list<double> data;
	bool hasminmax, hascenter;
	double smm[6];
	char* name;

public:
	SymbolCanvas();
	virtual ~SymbolCanvas();

	Symbol& finalize();
	void clear();
	long load(char* buff, long buffsize, long begin);
	void debug();

	// general attributes
	void minmax(double xmin, double ymin, double xmax, double ymax);
	void center(double cx, double cy);

    void fillColor(Graphics::Color c);
    void fillColor(unsigned r, unsigned g, unsigned b, unsigned a = 255);
    void noFill();
    void fillColorIndex(unsigned i);

    void lineColor(Graphics::Color c);
    void lineColor(unsigned r, unsigned g, unsigned b, unsigned a = 255);
    void noLine();
    void lineColorIndex(unsigned i);

    void lineWidth(double w);
    void lineCap(Graphics::LineCap cap);
    void lineJoin(Graphics::LineJoin join);
    void fillEvenOdd(bool evenOddFlag);

	// open entities
    void moveTo(double x, double y);
    void moveRel(double dx, double dy);
    void lineTo(double x, double y);
    void lineRel(double dx, double dy);
    void horLineTo(double x);
    void horLineRel(double dx);
    void verLineTo(double y);
    void verLineRel(double dy);

    void arcTo(double rx, double ry, double angle, bool largeArcFlag, bool sweepFlag, double x, double y);
    void arcRel(double rx, double ry, double angle, bool largeArcFlag, bool sweepFlag, double dx, double dy);
    void quadricCurveTo(double xCtrl, double yCtrl, double xTo, double yTo);
    void quadricCurveRel(double dxCtrl, double dyCtrl, double dxTo, double dyTo);
    void quadricCurveTo(double xTo, double yTo);
    void quadricCurveRel(double dxTo, double dyTo);
    void cubicCurveTo(double xCtrl1, double yCtrl1, double xCtrl2, double yCtrl2, double xTo, double yTo);
    void cubicCurveRel(double dxCtrl1, double dyCtrl1, double dxCtrl2, double dyCtrl2, double dxTo, double dyTo);
    void cubicCurveTo(double xCtrl2, double yCtrl2, double xTo, double yTo);
    void cubicCurveRel(double dxCtrl2, double dyCtrl2, double dxTo, double dyTo);

    void addEllipse(double cx, double cy, double rx, double ry, Graphics::Direction dir);
    void addEllipse(double cx, double cy, double rx, double ry, bool dir = true);

    void polygon(double* xy, int numPoints);
    void polyline(double* xy, int numPoints);

    void closePolygon();

    void drawPath(Graphics::DrawPathFlag flag = Graphics::FillAndStroke);
    void drawPathNoTransform(Graphics::DrawPathFlag flag = Graphics::FillAndStroke);

    void line(double x1, double y1, double x2, double y2);
    void triangle(double x1, double y1, double x2, double y2, double x3, double y3);
    void rectangle(double x1, double y1, double x2, double y2);
    void roundedRect(double x1, double y1, double x2, double y2, double r);
    void roundedRect(double x1, double y1, double x2, double y2, double rx, double ry);
    void roundedRect(double x1, double y1, double x2, double y2,
                     double rxBottom, double ryBottom,
                     double rxTop,    double ryTop);
    void ellipse(double cx, double cy, double rx, double ry);
    void arc(double cx, double cy, double rx, double ry, double start, double sweep);
    void star(double cx, double cy, double r1, double r2, double startAngle, int numRays);
    void curve(double x1, double y1, double x2, double y2, double x3, double y3);
    void curve(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4);

};


#endif // SYMBOLCANVAS_H_INCLUDED
