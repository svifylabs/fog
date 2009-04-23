#include <Fog/Core.h>
#include <Fog/Graphics.h>
#include <Fog/UI.h>

using namespace Fog;

struct MyWindow : public Window
{
  MyWindow(uint32_t createFlags = 0) : Fog::Window(createFlags) {}
  virtual ~MyWindow() {}

  virtual void onPaint(PaintEvent* e)
  {
    //Painter* p = e->painter();
    //p->fillRect(Fog::Rect(0, 0, clientWidth(), clientHeight()), Fog::Rgba(0xFF0000FF));
    //p->fillHGradient(Fog::Rect(0, 0, 100, 100), Fog::Rgba(0xFFFFFFFF), Fog::Rgba(0xFF000000));

    //for (sysuint_t i = 0; i < 100; i++)
      //p->fillRect(Fog::Rect(0, 0, clientWidth(), clientHeight()), Fog::Rgba(0xFF000000));

    //p->fillRect(Fog::Rect(40, 40, 40, 40), Fog::Rgba(0x800000FF));
    //p->fillRect(Fog::Rect(40, clientHeight() - 80, 40, 40), Fog::Rgba(0x8000FF00));
    //p->fillRect(Fog::Rect(clientWidth() - 80, 40, 40, 40), Fog::Rgba(0x80FF0000));
    //p->fillRect(Fog::Rect(clientWidth() - 80, clientHeight() - 80, 40, 40), Fog::Rgba(0x80FFFFFF));
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
