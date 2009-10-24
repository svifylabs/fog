#include <Fog/Core.h>
#include <Fog/Graphics.h>
#include <Fog/Graphics/Raster/Raster_C.h>
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
  virtual void onPaint(PaintEvent* e);
};

FOG_IMPLEMENT_OBJECT(MyWindow)

MyWindow::MyWindow(uint32_t createFlags) :
  Window(createFlags)
{
  setWindowTitle(Ascii8("Filters"));
}

MyWindow::~MyWindow()
{
}

void MyWindow::onPaint(PaintEvent* e)
{
  TimeTicks ticks = TimeTicks::highResNow();
  Painter* p = e->getPainter();

  double w = getWidth(), h = getHeight();
  p->setSource(0xFFFFFFFF);
  p->clear();

  PointD points[4];
  points[0].set(10, 10);
  points[1].set(110, 50);
  points[2].set(50, 100);
  points[3].set(30, 30);

  p->setSource(0xFF000000);

  p->fillPolygon(points, 4);
}

// ============================================================================
// [MAIN]
// ============================================================================

FOG_UI_MAIN()
{
  Application app(Ascii8("UI"));
  // fog_redirect_std_to_file("log.txt");

  MyWindow window;
  window.setSize(Size(715, 555));
  window.show();
  window.addListener(EvClose, &app, &Application::quit);

  return app.run();
}
