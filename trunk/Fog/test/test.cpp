#include <Fog/Core.h>
#include <Fog/Graphics.h>
#include <Fog/Graphics/Raster/Raster_C.h>
#include <Fog/UI.h>
#include <Fog/Svg.h>
#include <Fog/Xml.h>

using namespace Fog;

// ============================================================================
// [MyWindow - Declaration]
// ============================================================================

struct MyWindow : public Window
{
  // [Fog Object System]
  FOG_DECLARE_OBJECT(MyWindow, Window)

  // [Construction / Destruction]
  MyWindow(uint32_t createFlags = 0);
  virtual ~MyWindow();

  // [Event Handlers]
  virtual void onMouse(MouseEvent* e);
  virtual void onKey(KeyEvent* e);
  virtual void onPaint(PaintEvent* e);

  virtual void onTimer(TimerEvent* e);

  // [Members]
  Button button1;
  Timer timer;
  double r;
};

// ============================================================================
// [MyWindow - Construction / Destruction]
// ============================================================================

FOG_IMPLEMENT_OBJECT(MyWindow)

MyWindow::MyWindow(uint32_t createFlags) :
  Window(createFlags)
{
  setWindowTitle(Ascii8("Filters"));

  //button1.setRect(Rect(10, 10, 100, 20));
  //button1.setText(Ascii8("Test"));
  //button1.show();
  //add(&button1);

  timer.setInterval(TimeDelta::fromMilliseconds(5));
  timer.addListener(EvTimer, this, &MyWindow::onTimer);
  timer.start();

  r = 0.0;
}

MyWindow::~MyWindow()
{
}

// ============================================================================
// [MyWindow - Event Handlers]
// ============================================================================

static Image makeImage()
{
  Image im(600, 120, Image::FormatARGB32);
  Painter p(im);

  p.setOp(CompositeClear);
  p.clear();

  p.setOp(CompositeSrcOver);
  p.setSource(Rgba(0xFF000000));
  p.drawRect(Rect(0, 0, 600, 120));

  Font font;
  font.setFamily(Ascii8("Courier New"));
  font.setSize(80);
  font.setBold(true);

  p.setOp(CompositeSrcOver);
  p.setSource(0xFF000000);
  p.drawText(Rect(0, 0, p.width(), p.height()), Ascii8("Fog Library"), font, TextAlignCenter);

  p.end();

  return im;
}

void MyWindow::onMouse(MouseEvent* e)
{
  base::onMouse(e);
}

void MyWindow::onKey(KeyEvent* e)
{
  if (e->code() == EvKeyPress)
  {
    switch (e->key())
    {
      case KeyQ: r += 1.0; repaint(RepaintWidget); break;
      case KeyW: r -= 1.0; repaint(RepaintWidget); break;
    }
  }

  base::onKey(e);
}

void MyWindow::onPaint(PaintEvent* e)
{
  TimeTicks ticks = TimeTicks::highResNow();
  Painter* p = e->painter();

  double w = width(), h = height();

  {
    Pattern pattern;
    pattern.setType(Pattern::LinearGradient);
    pattern.setPoints(PointF(0, 0), PointF(w, h));
    pattern.addGradientStop(GradientStop(0.0, Rgba(0xFFFFFF00)));
    pattern.addGradientStop(GradientStop(1.0, Rgba(0xFFFF0000)));
    p->setSource(pattern);
    p->clear();
  }

  {
    SvgDocument svg;
    //svg.readFile(Ascii8("C:/my/svg/altum_angelfish_01.svg"));
    svg.readFile(Ascii8("C:/my/svg/tiger.svg"));
    //svg.readFile(Ascii8("C:/my/svg/clinton.svg"));
    //svg.readFile(Ascii8("C:/my/svg/longhorn.svg"));
    //svg.readFile(Ascii8("C:/my/svg/brown_fish_01.svg"));

    SvgContext ctx(p);
    p->translate(180.0, 145.0);
    p->scale(w/550.0, h/510.0);
    svg.render(&ctx);
  }

/*
  Image im0 = makeImage();
  Image im1;
  Image im2;
  Image im3;

  BlurImageFilter(BlurImageFilter::BlurTypeBox, r, r, ImageFilter::BorderModeColor, 0x00000000).filterImage(im1, im0);
  BlurImageFilter(BlurImageFilter::BlurTypeStack, r, r, ImageFilter::BorderModeColor, 0x00000000).filterImage(im2, im0);
  //BlurImageFilter(BlurImageFilter::BlurTypeGaussian, r, r, ImageFilter::BorderModeColor, 0x00000000).filterImage(im3, im0);

  p->drawImage(Point(50, 50 + (im1.height() + 10) * 0), im1);
  p->drawImage(Point(50, 50 + (im1.height() + 10) * 1), im2);
  p->drawImage(Point(50, 50 + (im1.height() + 10) * 2), im3);
*/

  p->flush();
  double t = (TimeTicks::highResNow() - ticks).inMillisecondsF();

  String32 a;
  a.format("TIME: %g [ms]", t);
  p->setSource(0xFF000000);
  p->drawText(Rect(0, 0, w, 20), a, font(), TextAlignRight);
}

void MyWindow::onTimer(TimerEvent* e)
{
  //repaint(RepaintWidget);
}

// ============================================================================
// [MAIN]
// ============================================================================

FOG_UI_MAIN()
{
  Application app(Ascii8("UI"));
  // fog_redirect_std_to_file("log.txt");

  MyWindow window;
  window.setSize(Size(715, 515));
  window.show();
  window.addListener(EvClose, &app, &Application::quit);

  return app.run();
}
