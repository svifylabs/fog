#include <Fog/Core.h>
#include <Fog/Graphics.h>
#include <Fog/UI.h>

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

    stepx = (double)(rand() % 300) / 150.0 + 1.5;
    stepy = (double)(rand() % 300) / 150.0 + 1.5;
  }

  void move(const RectD& bounds)
  {
    posx += stepx;
    posy += stepy;

    if (posx >= bounds.getX2() || posx <= bounds.getX1()) { stepx = -stepx; posx = Math::bound(posx, bounds.getX1(), bounds.getX2()); }
    if (posy >= bounds.getY2() || posy <= bounds.getY1()) { stepy = -stepy; posy = Math::bound(posy, bounds.getY1(), bounds.getY2()); }
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
      step[i] = (double)(5.0 - (rand() % 100) / 20.0);
    }
  }

  void move()
  {
    for (int i = 0; i < 3; i++)
    {
      c[i] += step[i];
      if (c[i] > 255.0) { c[i] = 255.0; step[i] = -step[i]; }
      if (c[i] < 0.0)   { c[i] = 0.0; step[i] = -step[i]; }

      if ((rand() % 255) == 0) step[i] = (double)(5.0 - (rand() % 100) / 20.0);
    }
  }
  Argb toArgb() { return Argb((int)c[0], (int)c[1], (int)c[2]); }
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

  enum { NumPoints = 8 };
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
  timer.addListener(EV_TIMER, this, &MyWindow::onTimer);
  timer.start();
}

MyWindow::~MyWindow()
{
}

void MyWindow::onPaint(PaintEvent* e)
{
  Painter* p = e->getPainter();

  Path path;
  for (sysuint_t i = 0; i < NumPoints; i += 2)
  {
    if (i == 0)
      path.moveTo(mp[i].posx, mp[i].posy);
    else
      path.curveTo(mp[i].posx, mp[i].posy, mp[i+1].posx, mp[i+1].posy);
  }

  path.curveTo(mp[0].posx, mp[0].posy);
  path.closePolygon();

  p->setSource((clr.toArgb() & 0x00FFFFFF) | 0x0F000000);
  p->setOperator(COMPOSITE_SCREEN);
  p->setFillMode(FILL_EVEN_ODD);
  p->fillPath(path);

  p->setOperator(COMPOSITE_SUBTRACT);
  p->setSource(0x0280FF8F);
  p->fillRect(Rect(0, 0, getWidth(), getHeight()));
}

void MyWindow::onTimer(TimerEvent* e)
{
  double w = getWidth(), h = getHeight();
  RectD bounds(0, 0, w, h);

  for (sysuint_t i = 0; i < NumPoints; i++) mp[i].move(bounds);
  clr.move();

  repaint(RepaintWidget);
}

// ============================================================================
// [MAIN]
// ============================================================================

FOG_UI_MAIN()
{
  Application app(Ascii8("UI"));

  MyWindow window;
  window.setSize(Size(515, 455));
  window.show();
  window.addListener(EV_CLOSE, &app, &Application::quit);

  return app.run();
}
