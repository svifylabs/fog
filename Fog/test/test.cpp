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
    add(&button);

    background.readFile(StubAscii8("/mnt/data1/Storage/Wallpapers/Fantasmal.bmp"));
    //background.readFile(StubAscii8("/mnt/data1/Storage/Wallpapers/Blue ilusion.jpg"));

    sprite[0].readFile(StubAscii8("/my/upload/img/babelfish.png"));
    sprite[0].premultiply();
    sprite[1].readFile(StubAscii8("/my/upload/img/blockdevice.png"));
    sprite[1].premultiply();
    sprite[2].readFile(StubAscii8("/my/upload/img/drop.png"));
    sprite[2].premultiply();
    sprite[3].readFile(StubAscii8("/my/upload/img/kweather.png"));
    sprite[3].premultiply();

    timer.setInterval(TimeDelta::fromMilliseconds(200));
    timer.addListener(EvTimer, this, &MyWindow::onTimer);

    //timer.start();
  }

  virtual ~MyWindow() {}

  virtual void onPaint(PaintEvent* e)
  {
    //fog_debug("OnPaint()");

    Painter* p = e->painter();

    int w = width();
    int h = height();

    p->setSource(Rgba(0xFF000000));
    p->clear();
    p->drawImage(Point(0, 0), background);

    //p->setSource(Rgba(0x7FFFFFFF));
    //p->fillRect(Rect(100, 30, 200, 200));
    //p->drawImage(Point(rand()%w, rand()%h), sprite[rand()%4]);

    {
      int x = 0, y = 0;
      int i;

      for (i = 0; i < CompositeCount; i++)
      {
        Image a(sprite[1]);
        Painter pa(a);
        pa.setOp(i);
        pa.drawImage(Point(0, 0), sprite[0]);
        pa.end();

        p->drawImage(Point(x * 130, y * 130), a);
        if (++x == 6) { x = 0; y++; }
      }
      /*Image i(sprite[0]);
      Painter pp(i);
      pp.setOp(CompositeOver);
      pp.drawImage(Point(0, 0), sprite[1]);
      pp.end();
      p->setOp(CompositeSrc);
      p->drawImage(Point(100, 100), i);*/
    }
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

  Button button;
  Image background;
  Image sprite[4];
  Timer timer;
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
