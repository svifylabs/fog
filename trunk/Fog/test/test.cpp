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
  }

  virtual ~MyWindow() {}

  virtual void onPaint(PaintEvent* e)
  {
    Painter* p = e->painter();

    int w = width();
    int h = height();

    p->setSource(Rgba(0xFF000000));
    p->clear();
/*
    p->setSource(Rgba(0xFFFFFFFF));
    p->setLineWidth(20);

    p->drawLine(PointF(20, 20), PointF(520, 24));
*/
    //p->setSource(0xFFFFFFFF);
    //p->fillRect(Rect(10, 10, 200, 200));

    Font font;
    font.setFamily(StubAscii8("Arial"));

    p->setSource(0xFFFFFFFF);
    //p->drawText(Point(10, 10), StubAscii8("Test"), font);
/*
    for (sysuint_t i = 0; i < 1000; i++)
    {
      p->drawText(Point(rand() % w, rand() % h), StubAscii8("ABCDEFGHIJKLMOPQRSTUVWXYZ"), font);
    }*/

    for (sysuint_t i = 0; i < 40; i++)
    {
      p->drawText(Point(50, i * 14), StubAscii8("abcdefghijklmopqrstuvwxyz"), font);
    }

    p->setSource(Rgba(0xFFFFFFFF));
    p->fillArc(PointF(100, 100), PointF(200, 200), 0.5, 3.14);

/*
    Path path;
    for (sysuint_t i = 0; i < 100; i++)
    {
      path.clear();
      path.moveTo(rand() % w, rand() % h);
      for (sysuint_t j = 0; j < 30; j++)
      {
        path.cubicCurveTo(
          rand() % w, rand() % h,
          rand() % w, rand() % h,
          rand() % w, rand() % h);
      }
      path.closePolygon();

      p->setSource(Rgba(0xFF000000 | (rand() + (rand() << 16))));
      //p->setSource(0xFFFFFFFF);
      p->setLineWidth(4);
      p->setFillMode(FillNonZero);
      p->drawPath(path);
    }
*/
  }
  
  virtual void onKey(KeyEvent* e)
  {
    if (e->code() == EvKeyPress && e->key() == KeyEnter)
    {
      repaint(RepaintWidget);
    }
  }

  Button button;
};

FOG_UI_MAIN()
{
  Application app(StubAscii8("UI"));

  MyWindow window;
  window.setSize(Size(320, 240));
  window.show();
  window.addListener(EvClose, &app, &Application::quit);

  return app.run();
}
