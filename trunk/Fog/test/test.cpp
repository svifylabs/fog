#include <Fog/Core.h>
#include <Fog/Graphics.h>
#include <Fog/Graphics/Raster/Raster_C.h>
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

  virtual void onTimer(TimerEvent* e);

  void bench();

  Button button1;
  Timer timer;
  double r;
};

// ============================================================================
// [MyWindow - Construction / Destruction]
// ============================================================================

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

  BlurImageFilter(BlurImageFilter::BlurTypeBox, r, r, ImageFilter::BorderModeColor, 0x00000000).filterImage(im1, im0);
  BlurImageFilter(BlurImageFilter::BlurTypeStack, r, r, ImageFilter::BorderModeColor, 0x00000000).filterImage(im2, im0);
  //BlurImageFilter(BlurImageFilter::BlurTypeGaussian, r, r, ImageFilter::BorderModeColor, 0x00000000).filterImage(im3, im0);

  //float kernel[] = { -3.0, -1.5, -1.0, -2.0, 1.0, -2.0, -1.0, -1.5, -3.0 };
  //int size = 9;
  //float div = 0.0;
  //for (int i = 0; i < size; i++) div += kernel[i];
  //ImageFx::convolveSymmetricFloat(im3, im0, kernel, size, div, kernel, size, div, ImageFx::EdgeModeAuto, 0);

  {
    Pattern pattern;
    pattern.setType(Pattern::LinearGradient);
    pattern.setPoints(PointF(0, 0), PointF(w, h));
    pattern.addGradientStop(GradientStop(0.0, Rgba(0xFFFFFF00)));
    pattern.addGradientStop(GradientStop(1.0, Rgba(0xFFFF0000)));
    p->setSource(pattern);
    p->clear();
  }

  //im0.convert(Image::FormatARGB32);
  //im1.convert(Image::FormatARGB32);
  //im2.convert(Image::FormatARGB32);
  //im3.convert(Image::FormatARGB32);

  p->drawImage(Point(50, 50 + (im1.height() + 10) * 0), im1);
  p->drawImage(Point(50, 50 + (im1.height() + 10) * 1), im2);
  p->drawImage(Point(50, 50 + (im1.height() + 10) * 2), im3);

/*
  Image src;
  int i, w;

  // [0] Source image.
  src.readFile(Ascii8("C:/My/img/babelfish.pcx"));
  src.convert(Image::FormatPRGB32); // should already be.
  //src.convert(Image::FormatARGB32); // should already be.
  w = src.width() + 5;

  p->setSource(0xFF000000);
  p->clear();

  // [1] ColorMatrix filtering.
  {
    for (i = 0; i < 6; i++)
    {
      ColorMatrix cm;
      cm.rotateHue(Math::deg2rad(double(i) * 60));

      Image tmp(src);
      tmp.filter(cm);

      p->drawImage(Point(50 + i*w, 50), tmp);
    }
  }
*/
  String32 a;
  a.setDouble(r);
  Application::getApplicationCommand(a);
  p->setSource(0xFF000000);
  p->drawText(Rect(0, 0, w, 20), a, font(), TextAlignCenter);
}

void MyWindow::onTimer(TimerEvent* e)
{
  //repaint(RepaintWidget);
}

void MyWindow::bench()
{
  TimeTicks td;

  Image src = makeImage();
  Image dst;
  int i, count = 1000;

  fog_debug("Benchmarking, radius: %g", r);

  td = TimeTicks::highResNow();
  for (i = 0; i < count; i++)
  {
    BlurImageFilter(ImageFilter::BlurTypeBox, r, r, ImageFilter::BorderModeExtend, 0xFF000000).filterImage(dst, src);
  }
  fog_debug("Box blur: %f", (TimeTicks::highResNow() - td).inMillisecondsF());

  td = TimeTicks::highResNow();
  for (i = 0; i < count; i++)
  {
    BlurImageFilter(ImageFilter::BlurTypeStack, r, r, ImageFilter::BorderModeExtend, 0xFF000000).filterImage(dst, src);
  }
  fog_debug("Stack blur: %f", (TimeTicks::highResNow() - td).inMillisecondsF());
/*
  td = TimeTicks::highResNow();
  for (i = 0; i < count; i++)
  {
    ImageFx::gaussianBlur(dst, src, r, r, ImageFilter::BorderModeExtend, 0xFF000000);
  }
  fog_debug("Gaussian blur: %f", (TimeTicks::highResNow() - td).inMillisecondsF());
*/
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

  //fog_redirect_std_to_file("log.txt");

  MyWindow window;
  window.setSize(Size(715, 515));
  window.show();
  window.addListener(EvClose, &app, &Application::quit);

  return app.run();
}
