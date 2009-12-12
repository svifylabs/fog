#include <Fog/Core.h>
#include <Fog/Graphics.h>
#include <Fog/UI.h>
#include <Fog/Svg.h>
#include <Fog/Xml.h>

using namespace Fog;

// This is for MY testing:)

struct MyWindow : public Window
{
  // [Fog Object System]
  FOG_DECLARE_OBJECT(MyWindow, Window)

  // [Construction / Destruction]
  MyWindow(uint32_t createFlags = 0);
  virtual ~MyWindow();

  // [Event Handlers]
  virtual void onKeyPress(KeyEvent* e);
  virtual void onPaint(PaintEvent* e);

  void paintImage(Painter* painter, const Point& pos, const Image& im, const String& name);

  Image i[2];
  double _subx;
  double _suby;
  double _rotate;
  double _shearX;
  double _shearY;
  double _scale;
  int _spread;
};

FOG_IMPLEMENT_OBJECT(MyWindow)

MyWindow::MyWindow(uint32_t createFlags) :
  Window(createFlags)
{
  setWindowTitle(Ascii8("Testing..."));

  i[0].readFile(Ascii8("babelfish.png"));
  i[1].readFile(Ascii8("kweather.png"));

  // i[0].readFile(Ascii8("texture0.bmp"));
  //i[0].readFile(Ascii8("/my/upload/bmpsuite/g04.bmp"));
  //i[0].readFile(Ascii8("/my/upload/bmpsuite/icons_fullset.png"));

  i[0].convert(PIXEL_FORMAT_PRGB32);
  i[1].convert(PIXEL_FORMAT_PRGB32);
  //i[0] = i[0].scale(Size(32, 32), IMAGE_SCALE_SMOOTH);

  _subx = 0.0;
  _suby = 0.0;
  _rotate = 0.0;
  _shearX = 0.0;
  _shearY = 0.0;
  _scale = 1.0;
  _spread = SPREAD_REPEAT;
}

MyWindow::~MyWindow()
{
}

void MyWindow::onKeyPress(KeyEvent* e)
{
  switch (e->getKey())
  {
    case KEY_LEFT:
      _subx -= 0.05;
      break;
    case KEY_RIGHT:
      _subx += 0.05;
      break;
    case KEY_UP:
      _suby -= 0.05;
      break;
    case KEY_DOWN:
      _suby += 0.05;
      break;
    case KEY_Q:
      _rotate -= 0.01;
      break;
    case KEY_W:
      _rotate += 0.01;
      break;
    case KEY_E:
      _shearX -= 0.01;
      break;
    case KEY_R:
      _shearX += 0.01;
      break;
    case KEY_T:
      _shearY -= 0.01;
      break;
    case KEY_Y:
      _shearY += 0.01;
      break;
    case KEY_A:
      _scale -= 0.01;
      break;
    case KEY_S:
      _scale += 0.01;
      break;
    case KEY_SPACE:
      _spread = (_spread == SPREAD_REPEAT) ? SPREAD_REFLECT : SPREAD_REPEAT;
      break;
  }

  repaint(RepaintWidget);
  base::onKeyPress(e);
}

void MyWindow::onPaint(PaintEvent* e)
{
  TimeTicks ticks = TimeTicks::highResNow();
  Painter* p = e->getPainter();

  p->setOperator(COMPOSITE_SRC);

  p->setSource(0xFF3F3FFF);
  p->clear();
/*
  Region reg;
  reg.unite(Rect(0, 0, 200, 25));
  reg.unite(Rect(0, 50, 200, 25));
  reg.unite(Rect(0, 100, 200, 25));
  reg.unite(Rect(0, 150, 200, 25));
  reg.unite(Rect(0, 200, 200, 25));
  reg.unite(Rect(0, 250, 200, 25));
  reg.unite(Rect(0, 300, 200, 25));
  reg.unite(Rect(220, 0, 200, 25));
  reg.unite(Rect(220, 50, 200, 25));
  reg.unite(Rect(220, 100, 200, 25));
  reg.unite(Rect(220, 150, 200, 25));
  reg.unite(Rect(220, 200, 200, 25));
  reg.unite(Rect(220, 250, 200, 25));
  reg.unite(Rect(220, 300, 200, 25));
  p->setUserRegion(reg);

  Pattern pattern;
  pattern.setTexture(i[0]);
  pattern.setSpread(_spread);
  pattern.setMatrix(Matrix::fromTranslation(_subx, _suby));
  pattern.translate(300, 300);
  pattern.rotate(_rotate);
  pattern.skew(_shearX, _shearY);
  pattern.scale(_scale);
  pattern.translate(-300, -300);

  p->setSource(pattern);
  p->clear();

  {
    p->save();
    p->resetMatrix();

    ColorMatrix cm;
    cm.rotateHue(1.0);
    p->setSource(ColorFilter(cm));
    //p->setSource(0xFFFFFFFF);
    p->setFillMode(FILL_NON_ZERO);
    p->fillRound(Rect(50, 50, 300, 300), Point(100, 100));
    p->restore();
  }
*/
/*
  p->setSource(0xFF000000);
  Path a;
  a.moveTo(100, 100);
  a.lineTo(200, 200);
  a.lineTo(100, 200);
  a.closePolygon();
  p->drawPath(a);
  //p->drawRound(Rect(100, 100, 200, 200), Point(50, 50));
*/
#if 1
  // Clear everything to white.
  p->setSource(Argb(0xFFFFFFFF));
  p->clear();

  // These coordinates describe boundary of object we want to paint.
  double x = 40.5;
  double y = 40.5;
  double w = 300.0;
  double h = 300.0;

  // Create path that will contain rounded rectangle.
  Path path;
  path.addRound(RectD(x, y, w, h), PointD(50.0, 50.0));
/*
  path.moveTo(100, 100);
  path.cubicTo(150, 120, 180, 100, 200, 200);
  path.lineTo(50, 230);
  path.lineTo(140, 280);
*/

  // Create linear gradient pattern.
  Pattern pattern;
  pattern.setType(PATTERN_LINEAR_GRADIENT);
  pattern.setPoints(PointD(x, y), PointD(x + w, y + h));
  pattern.addStop(ArgbStop(0.0, Argb(0xFFFFFF00)));
  pattern.addStop(ArgbStop(1.0, Argb(0xFFFF0000)));

  // Fill path with linear gradient we created.
  p->setSource(pattern);
  p->fillPath(path);

  // Stroke path using solid black color.
  p->setSource(Argb(0xFF000000));
  p->setLineWidth(2);
  p->drawPath(path);
#endif

/*
  p->setOperator(COMPOSITE_SRC_OVER);
  p->setSource(0xFFFFFFFF);

  PointD points[2];
  points[0].set(0, 0);
  points[1].set(getWidth(), getHeight());

  Pattern pattern;
  pattern.setType(PATTERN_LINEAR_GRADIENT);
  pattern.setSpread(SPREAD_REFLECT);
  pattern.setPoints(PointD(points[0]), PointD(points[1].y, points[0].x));

  pattern.addStop(ArgbStop(0.0, 0xFFFFFFFF));
  pattern.addStop(ArgbStop(1.0, 0xFFFF0000));

  p->rotate(_rotate);
  p->translate(300, 300);
  p->setSource(pattern);
  p->clear();

  //pattern.resetStops();
  //pattern.addStop(ArgbStop(0.0, 0xFF000000));
  //pattern.addStop(ArgbStop(1.0, 0xFFFF0000));
  //p->setSource(pattern);
  p->scale(0.1, 0.1);

  p->fillRect(RectD(
    points[0].x,
    points[0].y,
    points[1].x - points[0].x,
    points[1].y - points[0].y));
*/
/*
  int y = 0;
  int x;
  double radius;
  String s;

  y++;

  for (x = 0, radius = _baseRadius; x < 6; x++, radius += 1.0)
  {
    Image im(i[0]);
    ColorMatrix cm;
    cm.rotateHue(radius * 0.08);
    im.filter(cm);
    s.format("ColorMatrix");
    paintImage(p, Point(x, y), im, s);
  }
*/
/*
  y++;

  for (x = 0, radius = _baseRadius; x < 6; x++, radius += 1.0)
  {
    Image im(i[0]);

    s.format("ColorLut");
    paintImage(p, Point(x, y), im, s);
  }
*/
  //p->drawText(Point(mp[0].posx, mp[0].posy), Ascii8("TEST"), Font());
/*
  p->setOperator(COMPOSITE_SRC);
  p->setSource(0xFF000000);
  p->clear();

  p->setOperator(COMPOSITE_SRC_OVER);
  p->setSource(0xFFFFFFFF);

  Font font;
  font.setSize(14);
  p->drawText(Point(100, 10), Ascii8("Test test"), font);

  Path path;
  font.getOutline(Ascii8("Test test"), path);
  //path.scale(0.05, 0.05);
  path.translate(100, 40.8);
  //p->rotate(0.09);
  for (int i = 0; i < 10; i++)
  {
    p->fillPath(path);
    path.translate(0, 20);
    path.scale(1.17, 1.17, true);
  }
*/
/*
  Image ii;
  ImageIO::DecoderDevice* decoder = ImageIO::createDecoderForFile(Ascii8("/my/upload/bmpsuite/ICO/camera.ico"));
  if (decoder)
  {
    int x = 10, y = 10;
    while (decoder->readImage(ii) == ERR_OK)
    {
      ii.convert(PIXEL_FORMAT_PRGB32);
      p->drawImage(Point(x, y), ii);
      y += ii.getHeight();
    }
    delete decoder;
  }
*/

/*
  for (int i = 0; i < 1; i++)
  {
    Path path;
    Font font;
    font.setSize(10);
    p->setSource(0xFF000000 | (rand()*65535+rand()));
    p->drawText(Point(rand() % getWidth(), rand() % getHeight()), Ascii8("Test test"), font);
    //font.getOutline(Ascii8("Test test"), path);
    //path.translate(rand() % getWidth(), rand() % getHeight());
    //p->fillPath(path);
  }
*/

/*
  p->setOperator(COMPOSITE_SRC_OVER);
  p->translate(mp[0].posx, mp[0].posy);

  SvgDocument svg;
  svg.readFile(Ascii8("/my/upload/svg/tiger.svg"));
  SvgContext context(p);
  svg.onRender(&context);
*/

  p->flush();

  TimeDelta delta = TimeTicks::highResNow() - ticks;

  p->setOperator(COMPOSITE_SRC_OVER);
  p->setSource(0xFFFFFFFF);
  p->fillRect(Rect(0, 0, 2000, getFont().getHeight()));
  p->setSource(0xFF000000);

  String s;
  s.format("Size: %d %d, time %g, [PARAMS: %g %g]", getWidth(), getHeight(), delta.inMillisecondsF(), _subx, _suby);
  p->drawText(Point(0, 0), s, getFont());
}

void MyWindow::paintImage(Painter* p, const Point& pos, const Image& im, const String& name)
{
  int x = 10 + pos.x * 152;
  int y = 10 + pos.y * 152;

  p->drawText(Rect(x, y, 130, 20), name, getFont(), TEXT_ALIGN_CENTER);
  p->drawRect(Rect(x, y + 20, 130, 130));
  p->drawImage(Point(x + 1, y + 21), im);
}

// ============================================================================
// [MAIN]
// ============================================================================

FOG_UI_MAIN()
{
  Application app(Ascii8("UI"));

  MyWindow window;
  window.setSize(Size(500, 400));

  window.addListener(EV_CLOSE, &app, &Application::quit);
  window.show();

  return app.run();
}
