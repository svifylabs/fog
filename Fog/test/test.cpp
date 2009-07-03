#include <Fog/Core.h>
#include <Fog/Graphics.h>
#include <Fog/UI.h>
#include <Fog/Xml.h>

using namespace Fog;

#if 0

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
  setWindowTitle(Ascii8("Fog Application"));
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
  p->drawText(Point(100, 50), Ascii8("Abcdefgh"), font);
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
  Application app(Ascii8("UI"));

  MyWindow window;
  window.setSize(Size(640, 480));
  window.show();
  window.addListener(EvClose, &app, &Application::quit);

  return app.run();
}

#endif



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

  void bench();

  double r;
};

// ============================================================================
// [MyWindow - Construction / Destruction]
// ============================================================================

MyWindow::MyWindow(uint32_t createFlags) :
  Window(createFlags)
{
  setWindowTitle(Ascii8("Filters"));
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
  Image im(600, 120, Image::FormatRGB32);
  Painter p(im);

  p.setSource(0xFFFFFFFF);
  p.clear();

  Font font;
  font.setFamily(Ascii8("Courier New"));
  font.setSize(80);
  font.setBold(true);

  p.setSource(0xFF000000);
  p.drawRect(Rect(0, 0, p.width(), p.height()));

  p.setSource(0xFF0000FF);
  p.drawText(Rect(2, 2, p.width(), p.height()), Ascii8("Fog Library"), font, TextAlignCenter);

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
      case KeyB: bench(); break;
    }
  }

  base::onKey(e);
}

void MyWindow::onPaint(PaintEvent* e)
{
  Painter* p = e->painter();
  double w = width(), h = height();

  Image im0 = makeImage();
  Image im1;
  Image im2;
  Image im3;

  ImageFx::boxBlur(im1, im0, r, r, ImageFx::EdgeModeAuto, 0xFF000000);
  ImageFx::stackBlur(im2, im0, r, r, ImageFx::EdgeModeAuto, 0xFF000000);
  //ImageFx::gaussianBlur(im3, im0, r, r, ImageFx::EdgeModeAuto, 0xFF000000);

  p->setSource(0xFFFFFFFF);
  p->clear();

  //p->setSource(0xFF000000);
  //p->drawRect(Rect(49, 49, im.width() + 2, im.height() + 2));

  p->drawImage(Point(50, 50 + (im1.height() + 10) * 0), im1);
  p->drawImage(Point(50, 50 + (im1.height() + 10) * 1), im2);
  p->drawImage(Point(50, 50 + (im1.height() + 10) * 2), im3);
}

void MyWindow::bench()
{
  TimeTicks td;

  Image src = makeImage();
  Image dst;
  int i, count = 1;

  fog_debug("Benchmarking, radius: %g", r);

  td = TimeTicks::highResNow();
  for (i = 0; i < count; i++)
  {
    ImageFx::boxBlur(dst, src, r, r, ImageFx::EdgeModeAuto, 0xFF000000);
  }
  fog_debug("Box blur: %f", (TimeTicks::highResNow() - td).inMillisecondsF());

  td = TimeTicks::highResNow();
  for (i = 0; i < count; i++)
  {
    ImageFx::stackBlur(dst, src, r, r, ImageFx::EdgeModeAuto, 0xFF000000);
  }
  fog_debug("Stack blur: %f", (TimeTicks::highResNow() - td).inMillisecondsF());

  td = TimeTicks::highResNow();
  for (i = 0; i < count; i++)
  {
    ImageFx::gaussianBlur(dst, src, r, r, ImageFx::EdgeModeAuto, 0xFF000000);
  }
  fog_debug("Gaussian blur: %f", (TimeTicks::highResNow() - td).inMillisecondsF());
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
  Application app(Ascii8("UI"));

  MyWindow window;
  window.setSize(Size(715, 515));
  window.show();
  window.addListener(EvClose, &app, &Application::quit);

  return app.run();
}
