#include <Fog/Core.h>
#include <Fog/Graphics.h>
#include <Fog/UI.h>

using namespace Fog;

struct MyWindow : public Window
{
  MyWindow(uint32_t createFlags = 0) : Window(createFlags) {}
  virtual ~MyWindow() {}

  virtual void onPaint(PaintEvent* e)
  {
    Painter* p = e->painter();

    p->setSource(Rgba(0x00000000));
    p->clear();

    p->setSource(Rgba(0xFFFFFFFF));
    p->setLineWidth(5);

    Path path;
    int w = width();
    int h = height();
    for (sysuint_t i = 0; i < 10; i++)
    {
      path.clear();
      path.moveTo(rand() % w, rand() % h);
      for (sysuint_t j = 0; j < 100; j++)
      {
        path.cubicCurveTo(
          rand() % w, rand() % h,
          rand() % w, rand() % h,
          rand() % w, rand() % h);
      }
      path.closePolygon();

      p->setSource(Rgba(rand() + (rand() << 16)));
      p->fillPath(path);
    }
  }
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
