#include <Fog/Core.h>
#include <Fog/Graphics.h>
#include <Fog/UI.h>

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
  virtual void onTimer(TimerEvent* e);
  virtual void onPaint(PaintEvent* e);
  virtual void button0_onClick(MouseEvent* e);
  virtual void button1_onClick(MouseEvent* e);

  // [Painting]

  void paintBackground(Painter* p, const Rect& r);
  void paintComposition(Painter* p, const Rect& r);
  void paintLines(Painter* p, const Rect& r, int count);
  void paintCurves(Painter* p, const Rect& r, int count);
  void paintRects(Painter* p, const Rect& r, int count);
  void paintSprites(Painter* p, const Rect& r, int count);

  // [Members]

  Button button[4];

  Image background;
  Pattern pattern[2];
  int activePattern;

  Image sprite[4];
  Timer timer;

  bool _draggingLeft;
  bool _draggingRight;

  double _rotate;
  double _scale;
};

// ============================================================================
// [MyWindow - Construction / Destruction]
// ============================================================================

MyWindow::MyWindow(uint32_t createFlags) :
  Window(createFlags)
{
  setWindowTitle(StubAscii8("Fog Application"));

  button[0].setRect(Rect(10, 10, 100, 20));
  button[0].show();
  button[0].setText(StubAscii8("Test"));
  button[0].addListener(EvClick, this, &MyWindow::button0_onClick);
  //add(&button[0]);

  button[1].setRect(Rect(10, 40, 100, 20));
  button[1].show();
  button[1].setText(StubAscii8("Test"));
  button[1].addListener(EvClick, this, &MyWindow::button1_onClick);
  //add(&button[1]);

  //background.readFile(StubAscii8("/mnt/data1/Storage/Wallpapers/Fantasmal.bmp"));
  //background.readFile(StubAscii8("/mnt/data1/Storage/Wallpapers/Blue ilusion.jpg"));
  background.readFile(StubAscii8("C:/Shared/Wallpapers/flowerpaper.bmp"));
  //background.to8Bit();
  background.convert(Image::FormatRGB32);

  //sprite[0].readFile(StubAscii8("C:/My/CPlusPlus/BlitJitTest/img/babelfish.bmp"));
  //sprite[0].convert(Image::FormatPRGB32);
  //sprite[0].fillQGradient(Rect(0, 0, sprite[0].width(), sprite[0].height()), 0xFFFFFFFF, 0x00000000, 0xFFFF0000, 0xFF0000FF, false);

  //sprite[0].readFile(StubAscii8("/my/upload/img/babelfish.png"));
  sprite[0].readFile(StubAscii8("C:/My/img/babelfish.pcx"));
  sprite[0].premultiply();
  //sprite[1].readFile(StubAscii8("/my/upload/img/blockdevice.png"));
  sprite[1].readFile(StubAscii8("C:/My/img/blockdevice.pcx"));
  sprite[1].premultiply();
  //sprite[2].readFile(StubAscii8("/my/upload/img/drop.png"));
  sprite[2].readFile(StubAscii8("C:/My/img/drop.pcx"));
  sprite[2].premultiply();
  //sprite[3].readFile(StubAscii8("/my/upload/img/kweather.png"));
  sprite[3].readFile(StubAscii8("C:/My/img/kweather.pcx"));
  sprite[3].premultiply();

  pattern[0].setTexture(sprite[0]);
  pattern[0].setSpread(Pattern::ReflectSpread);

  pattern[1].setType(Pattern::RadialGradient);
  pattern[1].setSpread(Pattern::PadSpread);
  pattern[1].setGradientRadius(130.0);
  pattern[1].addGradientStop(GradientStop(0.0, Rgba(0x00FFFFFF)));
  pattern[1].addGradientStop(GradientStop(0.5, Rgba(0x7FFF0000)));
  pattern[1].addGradientStop(GradientStop(0.7, Rgba(0xFFFFFF00)));
  pattern[1].addGradientStop(GradientStop(1.0, Rgba(0x9F0000FF)));
  pattern[1].setStartPoint(PointF(10, 20));
  pattern[1].setEndPoint(PointF(170, 270));

  activePattern = 1;

  timer.setInterval(TimeDelta::fromMilliseconds(50));
  timer.addListener(EvTimer, this, &MyWindow::onTimer);

  //timer.start();

  _draggingLeft = false;
  _draggingRight = false;
  _rotate = 0.0;
  _scale = 1.0;
}

MyWindow::~MyWindow()
{
}

// ============================================================================
// [MyWindow - Event Handlers]
// ============================================================================

void MyWindow::onMouse(MouseEvent* e)
{
  switch (e->code())
  {
    case EvMousePress:
    {
      switch (e->button())
      {
        case ButtonLeft:
          _draggingLeft = true;
          pattern[activePattern].setStartPoint(e->position());
          repaint(RepaintWidget);
          break;
        case ButtonRight:
          _draggingRight = true;
          pattern[activePattern].setEndPoint(e->position());
          repaint(RepaintWidget);
          break;
      }
      break;
    }
    case EvMouseRelease:
    {
      switch (e->button())
      {
        case ButtonLeft:
          _draggingLeft = false;
          break;
        case ButtonRight:
          _draggingRight = false;
          break;
      }
      break;
    }
    case EvMouseMove:
    case EvMouseOutside:
    {
      if (_draggingLeft)
      {
        pattern[activePattern].setStartPoint(e->position());
        repaint(RepaintWidget);
      }
      if (_draggingRight)
      {
        pattern[activePattern].setEndPoint(e->position());
        repaint(RepaintWidget);
      }
      break;
    }
  }

  base::onMouse(e);
}

void MyWindow::onKey(KeyEvent* e)
{
  if (e->code() == EvKeyPress)
  {
    switch (e->key())
    {
      case KeyEnter:
        repaint(RepaintWidget);
        break;
      case KeyQ:
        _rotate -= 0.1;
        repaint(RepaintWidget);
        break;
      case KeyW:
        _rotate += 0.1;
        repaint(RepaintWidget);
        break;
      case KeyE:
        _scale -= 0.1;
        repaint(RepaintWidget);
        break;
      case KeyR:
        _scale += 0.1;
        repaint(RepaintWidget);
        break;
    }
  }

  base::onKey(e);
}

void MyWindow::onTimer(TimerEvent* e)
{
  //repaint(RepaintWidget);
  //fog_debug("onTimer()");
}

void MyWindow::onPaint(PaintEvent* e)
{
  TimeTicks ticks = TimeTicks::highResNow();

  //fog_debug("OnPaint()");

  Painter* p = e->painter();

  p->setProperty(StubAscii8("multithreaded"), Value::fromBool(false));

  int w = width();
  int h = height();
  Rect boundingRect(0, 0, w, h);

  //paintBackground(p, boundingRect);
  p->setSource(0xFF000000);
  p->clear();

  //paintComposition(p, boundingRect);

  //p->setSource(pattern[activePattern]);
  //p->fillRect(Rect(0, 0, w, h));

/*
  {
    int xx = 40, yy = 40;
    int dx = 170, dy = 170;

    Font font;
    font.setSize(16);

    Image i(128, 128, Image::FormatRGB32);

    for (int a = 0; a < 6; a++)
    {
      const wchar_t* text = NULL;

      i.clear(0xFFFFFFFF);

      switch (a)
      {
        case 0:
          text = L"výplò";
          i.fillRect(Rect(30, 30, 50, 50), 0xFF000000);
          i.fillRect(Rect(45, 45, 50, 50), 0x7FFF0000, true);
          break;
        case 1:
          text = L"kreslení úseèek";
          i.drawLine(Point(30, 20), Point(90, 80), 0xFF000000);
          i.drawLine(Point(30, 30), Point(90, 90), 0xFFFF0000);
          i.drawLine(Point(30, 40), Point(90,100), 0xFF00FF00);
          i.drawLine(Point(30, 50), Point(90,110), 0xFF0000FF);
          break;
        case 2:
        {
          text = L"kreslení pixelù";
          for (int b = 0; b < 100; b++)
          {
            i.drawPixel(Point(14 + (rand() % 100), 14 + (rand() % 100)), rand());
          }
          break;
        }
        case 3:
        {
          text = L"pøechod barev";
          i.fillQGradient(Rect(0, 0, 128, 128), 0xFFFF0000, 0xFFFFFF00, 0xFF000000, 0xFFFFFFFF);
          break;
        }
        case 4:
        {
          text = L"vertikální pøechod";
          i.fillVGradient(Rect(0, 0, 128, 128), 0xFFFF0000, 0xFFFFFF00);
          break;
        }
        case 5:
        {
          text = L"horizontální pøechod";
          i.fillHGradient(Rect(0, 0, 128, 128), 0xFFFF0000, 0xFFFFFF00);
          break;
        }
      }

      p->setSource(0xFF000000);
      p->drawText(Rect(xx, yy, 128, 20), StubW(text), font, TextAlignCenter);
      p->drawRect(Rect(xx-1, yy + 23, 130, 130));
      p->drawImage(Point(xx, yy + 24), i);
      p->flush();

      xx += dx;
      if ((a % 3) == 2) { xx -= dx*3; yy += dy; }
    }
  }
*/
/*
  {
    int xx = 40, yy = 40;
    int dx = 170, dy = 170;

    Font font;
    font.setSize(16);

    Image i;

    for (int a = 0; a < 9; a++)
    {
      const wchar_t* text = NULL;

      i.readFile(StubAscii8("C:/My/img/babelfish.pcx"));

      switch (a)
      {
        case 0:
          text = L"bez úprav";
          break;
        case 1:
          text = L"pøehození RGB";
          i.swapRgb();
          break;
        case 2:
          text = L"pøehození RGBA";
          i.swapRgba();
          break;
        case 3:
          text = L"rotace o 90°";
          i.rotate(Image::Rotate90);
          break;
        case 4:
          text = L"rotace o 180°";
          i.rotate(Image::Rotate180);
          break;
        case 5:
          text = L"rotace o 270°";
          i.rotate(Image::Rotate270);
          break;
        case 6:
          text = L"zrcadlo horizontálnì";
          i.mirror(Image::MirrorHorizontal);
          break;
        case 7:
          text = L"zrcadlo vertikálnì";
          i.mirror(Image::MirrorVertical);
          break;
        case 8:
          text = L"zrcadlo v obou smìrech";
          i.mirror(Image::MirrorBoth);
          break;
      }

      p->setSource(0xFF000000);
      p->drawText(Rect(xx, yy, 128, 20), StubW(text), font, TextAlignCenter);
      p->drawRect(Rect(xx-1, yy + 23, 130, 130));
      p->drawImage(Point(xx, yy + 24), i);
      p->flush();

      xx += dx;
      if ((a % 3) == 2) { xx -= dx*3; yy += dy; }
    }
  }
*/
/*
  {
    Font font;
    Path path;

    AffineMatrix affine;
    affine.rotate(_rotate);
    affine.scale(_scale);
    affine.translate(200, 200);
    p->setMatrix(affine);

    font.setSize(50);
    String32 text(StubAscii8("Test"));
    font.getPath(text.cData(), text.length(), path);

    //p->setSource(0xFFFFFFFF);
    //p->fillPath(path);

    p->setSource(pattern[1]);
    p->fillPath(path);
    //p->drawPath(path);
  }
*/
/*
  for(sysuint_t i = 0; i < 1000; i++)
  {
    Font font;
    Path path;

    AffineMatrix affine;
    affine.rotate(((double)rand() / (double)RAND_MAX) * M_PI * 2.0);
    affine.scale(((double)rand() / (double)RAND_MAX) * 2.0 + 0.5);
    affine.translate(rand() % w, rand() % h);
    p->setMatrix(affine);

    font.setSize(20);
    Char32 text('a' + (rand() % 28));
    font.getPath(&text, 1, path);

    p->setSource(pattern[1]);
    p->fillPath(path);

    p->setSource(0xFFFFFFFF);
    p->drawPath(path);
  }
*/
  //paintComposition(p, boundingRect);

  p->setSource(Rgba(0xFFFFFFFF));
  paintLines(p, boundingRect, 1000);

#if 0
  for (int i = 0; i < 1000; i++)
  {/*
    int x1 = rand() % w;
    int y1 = rand() % h;
    int x2 = rand() % w;
    int y2 = rand() % h;

    p->setSource(Rgba(0x7F000000 | rand()));
    p->fillRound(Rect(
      fog_min(x1, x2), fog_min(y1, y2),
      fog_abs(x2-x1), fog_abs(y2-y1)), Point(5, 5));*/
    int x1 = rand() % w;
    int y1 = rand() % h;
    int x2 = rand() % 30;
    int y2 = rand() % 30;

    p->setSource(Rgba(0x7F000000 | rand()));
    p->fillRound(Rect(x1, y1, x2, y2), Point(5, 5));
  }
#endif
  //paintSprites(p, boundingRect, 1000);

  //p->setSource(0xFF000000);
  //p->clear();

  //p->setSource(pattern[activePattern]);
  //p->fillRect(Rect(0, 0, w, h));

  //p->setSource(Rgba(0x7FFFFFFF));
  //p->fillRect(Rect(100, 30, 200, 200));
  //p->drawImage(Point(rand()%w, rand()%h), sprite[rand()%4]);

    //p->setSource(pattern[activePattern]);
    //p->fillRound(RectF(100, 100, 300, 150), PointF(15, 15));

    //Font font;
    //font.setSize(80);

    //p->fillRect(Rect(10, 10, 600, 300));
    //p->drawText(Point(10, 10), StubAscii8("ABCDEFGHIJKLMNOP"), font);
    //p->drawText(Point(10, 70), StubAscii8("ABCDEFGHIJKLMNOP"), font);
    //p->drawText(Point(10, 130), StubAscii8("ABCDEFGHIJKLMNOP"), font);
    //p->drawText(Point(10, 190), StubAscii8("ABCDEFGHIJKLMNOP"), font);
    //p->drawText(Point(10, 250), StubAscii8("ABCDEFGHIJKLMNOP"), font);
    //p->clear();
    //p->setSource(0xFF000000);

    //p->setLineWidth(4.0);
    //double dashes[] = { 20.0, 20.0 };
    //p->setLineDash(dashes, 2);
    //p->drawLine(pattern[activePattern].startPoint(), pattern[activePattern].endPoint());
    //p->fillRect(Rect(100, 100, 500, 500));

/*
    for (int a = 0; a < 100; a++)
    {
      Path path;
      path.moveTo(PointF(rand()%w, rand()%h));
      for (int i = 0; i < 5; i++)
      {
        path.quadraticCurveTo(PointF(rand()%w, rand()%h), PointF(rand()%w, rand()%h));
      }
      path.closePolygon();
      p->setLineWidth(10.0);
      p->setSource(0xFF | rand());
      p->fillPath(path);
    }
*/
/*
    for (int a = 0; a < 10000; a++)
    {
      //p->setSource(0xFF | rand());
      p->drawImage(Point(rand()%w, rand()%h), sprite[0]);
    }
*/
  /*
  {
    p->setSource(0xFF000000);
    Path path;
    path.addRect(RectF(0, 0, width(), height()));
    p->fillPath(path);
  }


  for (int a = 0; a < 140; a++)
  {
    for (int b = 0; b < 300; b++)
    {
      Path path;
      path.addRect(RectF(5*b, 5*a, 5, 5));
      p->setSource(0xFF000000 | rand());
      p->fillPath(path);
    }
  }*/

  p->flush();
  TimeDelta timeDelta = TimeTicks::highResNow() - ticks;
  //fog_debug("Time: %f ms", timeDelta.inMillisecondsF());
}

void MyWindow::button0_onClick(MouseEvent* e)
{
  activePattern = 1 - activePattern;
  repaint(RepaintWidget);
}

void MyWindow::button1_onClick(MouseEvent* e)
{
  repaint(RepaintWidget);
}

// ============================================================================
// [MyWindow - Painting]
// ============================================================================

void MyWindow::paintBackground(Painter* p, const Rect& r)
{
  Pattern pat;
  pat.setTexture(background);
  pat.setStartPoint(PointF(0, 0));
  p->setSource(pat);
  p->clear();
}

void MyWindow::paintComposition(Painter* p, const Rect& r)
{
  int x = 0, y = 0;
  int dx = 130, dy = 130;
  int i;

  for (i = 0; i < CompositeCount; i++)
  {
    Image a(sprite[0]);
    Painter pa(a);
    pa.setOp(i);
    pa.drawImage(Point(0, 0), sprite[1]);
    pa.end();

    p->drawImage(Point(r.x() + x * dx, r.y() + y * dy), a);
    if (++x == 6) { x = 0; y++; }
  }
}

void MyWindow::paintLines(Painter* p, const Rect& r, int count)
{
  int x = r.x();
  int y = r.y();
  int w = r.width();
  int h = r.height();

  for (int i = 0; i < count; i++)
  {
    p->drawLine(
      PointF(x + (rand() % w), y + (rand() % h)),
      PointF(x + (rand() % w), y + (rand() % h)));
  }
}

void MyWindow::paintCurves(Painter* p, const Rect& r, int count)
{
  int x = r.x();
  int y = r.y();
  int w = r.width();
  int h = r.height();

  for (int i = 0; i < count; i++)
  {
    Path path;
    path.lineTo(PointF(x + (rand() % w), y + (rand() % h)));
    for (int i = 0; i < 5; i++)
    {
      path.quadraticCurveTo(
        PointF(x + (rand() % w), y + (rand() % h)),
        PointF(x + (rand() % w), y + (rand() % h)));
    }
    path.closePolygon();
    p->drawPath(path);
  }
}

void MyWindow::paintRects(Painter* p, const Rect& r, int count)
{
  int x = r.x();
  int y = r.y();
  int w = r.width();
  int h = r.height();

  for (int i = 0; i < count; i++)
  {
    int x1 = rand() % w;
    int y1 = rand() % h;
    int x2 = rand() % w;
    int y2 = rand() % h;

    p->fillRect(Rect(
      fog_min(x1, x2), fog_min(y1, y2),
      fog_abs(x2-x1), fog_abs(y2-y1)));
  }
}

void MyWindow::paintSprites(Painter* p, const Rect& r, int count)
{
  int w = r.width();
  int h = r.height();

  for (int i = 0; i < count; i++)
  {
    int x = rand() % w;
    int y = rand() % h;

    p->drawImage(Point(x, y), sprite[rand()%4]);
  }
}

// ============================================================================
// [MAIN]
// ============================================================================

FOG_UI_MAIN()
{
  Application app(StubAscii8("UI"));

  MyWindow window;
  window.setSize(Size(640, 480));
  window.show();
  window.addListener(EvClose, &app, &Application::quit);

  return app.run();
}
/*
  using namespace Fog;

  // Vytvoøení cílového obrázku o rozmìrech 320x200
  Image dst(320, 200, Image::FormatPRGB32);

  // Vytvoøení zdrojového obrázku
  Image src(320, 200, Image::FormatPRGB32);

  // Vytvoøení grafického kontextu.
  Painter p(dst);

  // Vykreslení zdrojového obrázku
  p.drawImage(Point(0, 0), src);

  // Modifikace zdrojového obrázku zapøíèiní vytvoøení kopie v pøípadì,
  // že je obrázek používán ještì alespoò jedním vláknem (pravdìpodobné).
  src.fillRect(Rect(0, 0, 100, 100));

  // Pokud se chceme problému vyhnout, je nutné pøed src.fillRect() zavolat
  // p.flush().
*/
