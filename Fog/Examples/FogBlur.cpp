#include <Fog/Core.h>
#include <Fog/Graphics.h>
#include <Fog/UI.h>
#include <Fog/Svg.h>
#include <Fog/Xml.h>

using namespace Fog;

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
  double _baseRadius;
};

FOG_IMPLEMENT_OBJECT(MyWindow)

MyWindow::MyWindow(uint32_t createFlags) :
  Window(createFlags)
{
  setWindowTitle(Ascii8("Testing..."));

  i[0].readFile(Ascii8("babelfish.png"));
  i[1].readFile(Ascii8("kweather.png"));

  _baseRadius = 1.0;
}

MyWindow::~MyWindow()
{
}

void MyWindow::onKeyPress(KeyEvent* e)
{
  switch (e->getKey())
  {
    case KEY_Q:
      _baseRadius = Math::max<double>(_baseRadius-1, 0.0);
      break;
    case KEY_W:
      _baseRadius = Math::min<double>(_baseRadius+1, 128.0);
      break;
  }

  repaint(RepaintWidget);
  base::onKeyPress(e);
}

void MyWindow::onPaint(PaintEvent* e)
{
  Painter* p = e->getPainter();

  p->setOperator(COMPOSITE_SRC);
  p->setSource(0xFF000000);
  p->clear();

  p->setOperator(COMPOSITE_SRC_OVER);
  p->setSource(0xFFFFFFFF);

  int y = 0;
  int x;
  double radius;
  String s;

  for (x = 0, radius = _baseRadius; x < 3; x++, radius += 5.0)
  {
    Image im(i[0]);
    im.filter(ImageFilter(BlurParams(BLUR_BOX, radius, radius, BORDER_EXTEND_COLOR, 0x00000000)));
    s.format("Box blur %g", radius);
    paintImage(p, Point(x, y), im, s);
  }

  y++;

  for (x = 0, radius = _baseRadius; x < 3; x++, radius += 5.0)
  {
    Image im(i[0]);
    im.filter(ImageFilter(BlurParams(BLUR_LINEAR, radius, radius, BORDER_EXTEND_COLOR, 0x00000000)));
    s.format("Linear blur %g", radius);
    paintImage(p, Point(x, y), im, s);
  }

  y++;

  for (x = 0, radius = _baseRadius; x < 3; x++, radius += 5.0)
  {
    Image im(i[0]);
    im.filter(ImageFilter(BlurParams(BLUR_GAUSSIAN, radius, radius, BORDER_EXTEND_COLOR, 0x00000000)));
    s.format("Gaussian blur %g", radius);
    paintImage(p, Point(x, y), im, s);
  }
}

void MyWindow::paintImage(Painter* p, const Point& pos, const Image& im, const String& name)
{
  int x = 10 + pos.x * 152;
  int y = 10 + pos.y * 152;

  p->drawText(Rect(x, y, 130, 20), name, getFont(), TEXT_ALIGN_CENTER);
  p->drawRect(Rect(x, y + 20, 130, 130));
  p->blitImage(Point(x + 1, y + 21), im);
}

// ============================================================================
// [MAIN]
// ============================================================================

FOG_UI_MAIN()
{
  Application app(Ascii8("UI"));

  MyWindow window;
  window.setSize(Size(20 + 3 * 152 - 22, 20 + 3 * 152));

  window.addListener(EV_CLOSE, &app, &Application::quit);
  window.show();

  return app.run();
}
