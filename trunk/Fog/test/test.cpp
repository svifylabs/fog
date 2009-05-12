#include <Fog/Core.h>
#include <Fog/Graphics.h>
#include <Fog/UI.h>

using namespace Fog;

struct MyWindow : public Window
{
  MyWindow(uint32_t createFlags = 0) : 
    Window(createFlags) 
  {
    setWindowTitle(StubAscii8("Fog Application"));

    button.setRect(Rect(10, 10, 100, 20));
    button.show();
    button.setText(StubAscii8("Test"));
    button.addListener(EvClick, this, &MyWindow::button_onClick);
    //add(&button);

    //background.readFile(StubAscii8("/mnt/data1/Storage/Wallpapers/Fantasmal.bmp"));
    //background.readFile(StubAscii8("/mnt/data1/Storage/Wallpapers/Blue ilusion.jpg"));
    background.readFile(StubAscii8("C:/Shared/Wallpapers/flowerpaper.bmp"));
    //background.to8Bit();
    background.convert(Image::FormatRGB32);

    sprite[0].readFile(StubAscii8("C:/My/CPlusPlus/BlitJitTest/img/babelfish.bmp"));
    sprite[0].convert(Image::FormatPRGB32);
    //sprite[0].fillQGradient(Rect(0, 0, sprite[0].width(), sprite[0].height()), 0xFFFFFFFF, 0x00000000, 0xFFFF0000, 0xFF0000FF, false);

    pattern[0].setTexture(sprite[0]);
    pattern[0].setSpread(Pattern::ReflectSpread);

    pattern[1].setType(Pattern::LinearGradient);
    pattern[1].addGradientStop(GradientStop(0.0, Rgba(0xFFFFFFFF)));
    pattern[1].addGradientStop(GradientStop(0.5, Rgba(0xFF00FFFF)));
    pattern[1].addGradientStop(GradientStop(1.0, Rgba(0xFF0000FF)));
    pattern[1].setStartPoint(PointF(10, 20));
    pattern[1].setEndPoint(PointF(100, 230));

    activePattern = 1;

/*
    sprite[0].readFile(StubAscii8("/my/upload/img/babelfish.png"));
    sprite[0].premultiply();
    sprite[1].readFile(StubAscii8("/my/upload/img/blockdevice.png"));
    sprite[1].premultiply();
    sprite[2].readFile(StubAscii8("/my/upload/img/drop.png"));
    sprite[2].premultiply();
    sprite[3].readFile(StubAscii8("/my/upload/img/kweather.png"));
    sprite[3].premultiply();
*/
    timer.setInterval(TimeDelta::fromMilliseconds(200));
    timer.addListener(EvTimer, this, &MyWindow::onTimer);

    //timer.start();

    _draggingLeft = false;
    _draggingRight = false;
  }

  virtual ~MyWindow() {}

  virtual void onPaint(PaintEvent* e)
  {
    //fog_debug("OnPaint()");

    Painter* p = e->painter();

    p->setProperty(StubAscii8("multithreaded"), Value::fromBool(true));

    int w = width();
    int h = height();

    //p->setSource(0xFF000000);
    //p->clear();
/*
    {
      Pattern pat;
      pat.setTexture(background);
      pat.setStartPoint(PointF(0, 0));
      p->setSource(pat);
    }
    p->clear();
*/


    //p->drawImage(Point(0, 0), background);

    //p->setSource(Rgba(0x7FFFFFFF));
    //p->fillRect(Rect(100, 30, 200, 200));
    //p->drawImage(Point(rand()%w, rand()%h), sprite[rand()%4]);
/*
    //{
      int x = 0, y = 0;
      int i;

      for (i = 0; i < CompositeCount; i++)
      {
        Image a(sprite[1]);
        Painter pa(a);
        pa.setOp(i);
        pa.drawImage(Point(0, 0), sprite[2]);
        pa.end();

        p->drawImage(Point(x * 130, y * 130), a);
        if (++x == 6) { x = 0; y++; }
      }
*/
/*
      p->setSource(pattern[activePattern]);

      Font font;
      font.setSize(50);

      //p->fillRect(Rect(10, 10, 600, 300));
      p->drawText(Point(10, 10), StubAscii8("ABCDEFGHIJKLMNOP"), font);
      p->drawText(Point(10, 60), StubAscii8("ABCDEFGHIJKLMNOP"), font);
      p->drawText(Point(10, 110), StubAscii8("ABCDEFGHIJKLMNOP"), font);
      p->drawText(Point(10, 160), StubAscii8("ABCDEFGHIJKLMNOP"), font);
      p->drawText(Point(10, 210), StubAscii8("ABCDEFGHIJKLMNOP"), font);
      p->clear();
      p->setSource(0xFF000000);

      p->setLineWidth(4.0);
      double dashes[] = { 20.0, 20.0 };
      p->setLineDash(dashes, 2);
      p->drawLine(pattern[activePattern].startPoint(), pattern[activePattern].endPoint());
      //p->fillRect(Rect(100, 100, 500, 500));
*/

      Path path;
      path.moveTo(PointF(rand()%w, rand()%h));
      for (int i = 0; i < 5; i++)
      {
        path.quadraticCurveTo(PointF(rand()%w, rand()%h), PointF(rand()%w, rand()%h));
      }
      path.closePolygon();
      p->setLineWidth(10.0);
      p->setSource(0xFFFFFFFF);
      p->fillPath(path);

      //p->fillRect(Rect(100, 100, 400, 500));
      //p->fillRect(Rect(0, 0, w, h));

      //p->setSource(pat);
      //p->clear();
      //p->fillRect(Rect(100, 100, 400, 400));
  
      //p->setSource(Rgba(255, 255, 255, 128));
      //p->setLineWidth(20.0);
      //p->drawLine(Point(0, 0), Point(w, h));
/*
      Image i(sprite[0]);
      Painter pp(i);
      pp.setOp(CompositeOver);
      pp.drawImage(Point(0, 0), sprite[1]);
      pp.end();
      p->setOp(CompositeSrc);
      p->drawImage(Point(100, 100), i);
*/
    //}
/*
    {
      Path path;
      path.lineTo(PointF(rand()%w, rand()%h));
      for (int i = 0; i < 5; i++)
      {
        path.quadraticCurveTo(PointF(rand()%w, rand()%h), PointF(rand()%w, rand()%h));
      }
      path.closePolygon();
      p->fillPath(path);
    }
*/

    //Image i(600, 400, Image::FormatPRGB32);
    //i.clear(0xFF0000FF);
    
    /*
    for (sysuint_t z = 0; z < 100000; z++)
    {
      i.drawLine(Point(rand()%i.width(), rand()%i.height()), Point(rand()%i.width(), rand()%i.height()), 0xFF000000 | rand());
    }
    */

    //Image i(width(), height(), Image::FormatPRGB32);
    //i.fillQGradient(Rect(0, 0, i.width(), i.height()), 0xFFFFFFFF, 0xFF000000, 0xFFFF0000, 0xFFFFFF00);

    //p->setLineWidth(1.0);
    //p->setSource(Rgba(0xFF0000FF));
    //for (sysuint_t ii = 0; ii < 1000; ii++)
    //  p->drawLine(Point(rand()%width(), rand()%height()), Point(rand()%width(), rand()%height()));
  }

  virtual void onMouse(MouseEvent* e)
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

  virtual void onKey(KeyEvent* e)
  {
    if (e->code() == EvKeyPress && e->key() == KeyEnter)
    {
      repaint(RepaintWidget);
    }
  }

  virtual void onTimer(TimerEvent* e)
  {
    //fog_debug("onTimer()");
  }

  virtual void button_onClick(MouseEvent* e)
  {
    activePattern = 1 - activePattern;
    repaint(RepaintWidget);
  }

  Button button;

  Image background;
  Pattern pattern[2];
  int activePattern;

  Image sprite[4];
  Timer timer;

  bool _draggingLeft;
  bool _draggingRight;
};

FOG_UI_MAIN()
{
  Application app(StubAscii8("UI"));

  MyWindow window;
  window.setSize(Size(640, 480));
  window.show();
  window.addListener(EvClose, &app, &Application::quit);

  return app.run();
}
