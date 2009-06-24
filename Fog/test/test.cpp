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

  Path path;
  //path.addPie(RectF(w/2.0 - 100.0, h/2.0 - 100.0, 200.0, 200.0), M_PI/4, M_PI*1.5);
  path.addRound(RectF(10.5, 10.5, 300, 300), PointF(50, 50));

  Pattern pat;
  pat.setType(Pattern::LinearGradient);
  pat.setPoints(PointF(10.5, 10.5), PointF(10.5 + 300, 10.5 + 300));

  pat.addGradientStop(GradientStop(0.0, Rgba(0xFFFFFF00)));
  pat.addGradientStop(GradientStop(1.0, Rgba(0xFFFF0000)));
  p->setSource(pat);
  p->fillPath(path);

  pat.addGradientStop(GradientStop(0.0, Rgba(0xFF0000FF)));
  pat.addGradientStop(GradientStop(1.0, Rgba(0xFF000000)));
  p->setSource(pat);
  p->drawPath(path);
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
