#include <Fog/Core.h>
#include <Fog/Graphics.h>
#include <Fog/Graphics/Raster/Raster_C.h>
#include <Fog/UI.h>
#include <Fog/Svg.h>
#include <Fog/Xml.h>

using namespace Fog;

struct MyPoint
{
  double posx;
  double posy;

  double stepx;
  double stepy;

  void setPos(double x, double y) { posx = x; posy = y; }
  void setStep(double x, double y) { stepx = x; stepy = y; }

  void randomize(int w, int h)
  {
    posx = rand() % w;
    posy = rand() % h;

    stepx = (double)(rand() % 300) / 50.0 + 2.0;
    stepy = (double)(rand() % 300) / 50.0 + 2.0;
  }

  void move(const RectD& bounds)
  {
    posx += stepx;
    posy += stepy;

    if (posx >= bounds.getX2() || posx <= bounds.getX1()) { stepx = -stepx; posx = Math::bound(bounds.getX1(), posx, bounds.getX2()); }
    if (posy >= bounds.getY2() || posy <= bounds.getY1()) { stepy = -stepy; posy = Math::bound(bounds.getY1(), posy, bounds.getY2()); }
  }
};

struct MyColor
{
  double c[3];
  double step[3];

  void randomize()
  {
    for (int i = 0; i < 3; i++)
    {
      c[i] = (double)(rand() % 255);
      step[i] = (double)(5.0 - (rand() % 100) / 10.0);
    }
  }

  void move()
  {
    for (int i = 0; i < 3; i++)
    {
      c[i] += step[i];
      if (c[i] > 255.0) { c[i] = 255.0; step[i] = -step[i]; }
      if (c[i] < 0.0)   { c[i] = 0.0; step[i] = -step[i]; }

      if ((rand() % 255) == 0) step[i] = (double)(5.0 - (rand() % 100) / 10.0);
    }
  }
  Rgba toRgba() { return Rgba((int)c[0], (int)c[1], (int)c[2]); }
};

struct MyWindow : public Window
{
  // [Fog Object System]
  FOG_DECLARE_OBJECT(MyWindow, Window)

  // [Construction / Destruction]
  MyWindow(uint32_t createFlags = 0);
  virtual ~MyWindow();

  // [Event Handlers]
  virtual void onPaint(PaintEvent* e);
  virtual void onTimer(TimerEvent* e);

  // [Members]
  Timer timer;

  enum { NumPoints = 6 };
  MyPoint mp[NumPoints];
  MyColor clr;
};

FOG_IMPLEMENT_OBJECT(MyWindow)

MyWindow::MyWindow(uint32_t createFlags) :
  Window(createFlags)
{
  setWindowTitle(Ascii8("Testing..."));

  for (sysuint_t i = 0; i < NumPoints; i++) mp[i].randomize(200, 200);
  clr.randomize();

  timer.setInterval(TimeDelta::fromMilliseconds(20));
  timer.addListener(EvTimer, this, &MyWindow::onTimer);
  timer.start();
}

MyWindow::~MyWindow()
{
}

void MyWindow::onPaint(PaintEvent* e)
{
  TimeTicks ticks = TimeTicks::highResNow();
  Painter* p = e->getPainter();

  double w = getWidth(), h = getHeight();
  //p->setSource(0xFFFFFFFF);
  //p->clear();

  PointD points[NumPoints];
  RectD bounds(0, 0, w, h);
  for (sysuint_t i = 0; i < NumPoints; i++)
  {
    points[i].set(mp[i].posx, mp[i].posy);
    mp[i].move(bounds);
  }
  clr.move();

  p->setSource(clr.toRgba());
  p->setFillMode(FillEvenOdd);
  p->fillPolygon(points, NumPoints);
}

void MyWindow::onTimer(TimerEvent* e)
{
  repaint(RepaintWidget);
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
