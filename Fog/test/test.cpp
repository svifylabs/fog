#include <Fog/Core.h>
#include <Fog/Graphics.h>
#include <Fog/UI.h>
#include <Fog/Xml.h>

using namespace Fog;

// ============================================================================
// [MyWindow - Declaration]
// ============================================================================

struct MyWindow : public Window
{
  // [Construction / Destruction]

  MyWindow(uint32_t createFlags = 0);
  virtual ~MyWindow();

  // [Event Handlers]

  virtual void onMouse(MouseEvent* e);
  virtual void onKey(KeyEvent* e);
  virtual void onPaint(PaintEvent* e);
};

// ============================================================================
// [MyWindow - Construction / Destruction]
// ============================================================================

MyWindow::MyWindow(uint32_t createFlags) :
  Window(createFlags)
{
  setWindowTitle(StubAscii8("Fog Application"));
}

MyWindow::~MyWindow()
{
}

// ============================================================================
// [MyWindow - Event Handlers]
// ============================================================================

void MyWindow::onMouse(MouseEvent* e)
{
  base::onMouse(e);
}

void MyWindow::onKey(KeyEvent* e)
{
  base::onKey(e);
}

void MyWindow::onPaint(PaintEvent* e)
{
  Painter* p = e->painter();
  double w = width(), h = height();

  p->setSource(0xFFFFFFFF);
  p->clear();
/*
  Path path;
  path.addRound(RectF(10.5, 10.5, 300, 300), PointF(50, 50));

  Pattern pattern;
  pattern.setType(Pattern::LinearGradient);
  pattern.setPoints(PointF(10.5, 10.5), PointF(10.5 + 300, 10.5 + 300));

  pattern.addGradientStop(GradientStop(0.0, Rgba(0xFFFFFF00)));
  pattern.addGradientStop(GradientStop(1.0, Rgba(0xFFFF0000)));
  p->setSource(pattern);
  p->fillPath(path);

  p->setSource(Rgba(0xFF000000));
  p->drawPath(path);
*/

  Font font;
  font.setSize(94);
  FontMetrics metrics = font.metrics();
  int x1= 100, x2 = 500, y;

  p->setSource(0xFFFF0000);
  y = 50;
  p->drawLine(Point(x1, y), Point(x2, y));

  p->setSource(0xFF0000FF);
  y = 50 + metrics.ascent;
  p->drawLine(Point(x1, y), Point(x2, y));

  p->setSource(0xFF0000FF);
  y = 50 + metrics.descent;
  p->drawLine(Point(x1, y), Point(x2, y));

  p->setSource(0xFFFF0000);
  y = 50 + metrics.height;
  p->drawLine(Point(x1, y), Point(x2, y));

  p->setSource(0xFF000000);
  p->drawText(Point(100, 50), StubAscii8("Abcdefgh"), font);
}

// ============================================================================
// [MAIN]
// ============================================================================

#if defined(FOG_OS_WINDOWS)
int FOG_STDCALL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
FOG_UI_MAIN()
#endif
{
  Application app(StubAscii8("UI"));

  MyWindow window;
  window.setSize(Size(640, 480));
  window.show();
  window.addListener(EvClose, &app, &Application::quit);

  return app.run();
}
