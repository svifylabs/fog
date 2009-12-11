#include <Fog/Core.h>
#include <Fog/Graphics.h>
#include <Fog/UI.h>

using namespace Fog;

struct MyWindow : public Window
{
  // [Fog Object System]
  FOG_DECLARE_OBJECT(MyWindow, Window)

  // [Construction / Destruction]
  MyWindow(uint32_t createFlags = 0);
  virtual ~MyWindow();

  // [Event Handlers]
  virtual void onKeyPress(KeyEvent* e);
  virtual void onPaint(PaintEvent* e);

  void paintImage(Painter* painter, const Point& pos, const Image& im, const String& name);

  Image i[2];
  int _opacity;
};

FOG_IMPLEMENT_OBJECT(MyWindow)

MyWindow::MyWindow(uint32_t createFlags) :
  Window(createFlags)
{
  setWindowTitle(Ascii8("Fog - Composite"));

  i[0].readFile(Ascii8("babelfish.png"));
  i[1].readFile(Ascii8("kweather.png"));

  i[0].premultiply();
  i[1].premultiply();

  _opacity = 255;
}

MyWindow::~MyWindow()
{
}

void MyWindow::onKeyPress(KeyEvent* e)
{
  switch (e->getKey())
  {
    case KEY_Q:
      _opacity = Math::max(_opacity - 1, 0);
      break;
    case KEY_W:
      _opacity = Math::min(_opacity + 1, 255);
      break;
  }

  repaint(RepaintWidget);
  base::onKeyPress(e);
}

void MyWindow::onPaint(PaintEvent* e)
{
  Painter* p = e->getPainter();

  p->setOperator(COMPOSITE_SRC);
  p->setSource(0xFF000000);
  p->clear();

  p->setOperator(COMPOSITE_SRC_OVER);
  p->setSource(0xFFFFFFFF);

  const char* opname[COMPOSITE_COUNT];

  opname[COMPOSITE_SRC       ] = "src";
  opname[COMPOSITE_DST       ] = "dst";
  opname[COMPOSITE_SRC_OVER  ] = "src-over";
  opname[COMPOSITE_DST_OVER  ] = "dst-over";
  opname[COMPOSITE_SRC_IN    ] = "src-in";
  opname[COMPOSITE_DST_IN    ] = "dst-in";
  opname[COMPOSITE_SRC_OUT   ] = "src-out";
  opname[COMPOSITE_DST_OUT   ] = "dst-out";
  opname[COMPOSITE_SRC_ATOP  ] = "src-atop";
  opname[COMPOSITE_DST_ATOP  ] = "dst-atop";
  opname[COMPOSITE_XOR       ] = "xor";
  opname[COMPOSITE_CLEAR     ] = "clear";
  opname[COMPOSITE_ADD       ] = "add";
  opname[COMPOSITE_SUBTRACT  ] = "subtract";
  opname[COMPOSITE_SCREEN    ] = "screen";
  opname[COMPOSITE_DARKEN    ] = "darken";
  opname[COMPOSITE_LIGHTEN   ] = "lighten";
  opname[COMPOSITE_DIFFERENCE] = "difference";
  opname[COMPOSITE_EXCLUSION ] = "exclusion";
  opname[COMPOSITE_INVERT    ] = "invert";
  opname[COMPOSITE_INVERT_RGB] = "invert-rgb";

  int x = 0, y = 0;
  for (int a = 0; a < COMPOSITE_COUNT; a++)
  {
    Image im(i[1]);
    im.drawImage(Point(0, 0), i[0], a, _opacity);
    paintImage(p, Point(x, y), im, Ascii8(opname[a]));

    if (++x == 6) { x = 0; y++; }
  }
}

void MyWindow::paintImage(Painter* p, const Point& pos, const Image& im, const String& name)
{
  int x = 10 + pos.x * 152;
  int y = 10 + pos.y * 152;

  p->drawText(Rect(x, y, 130, 20), name, getFont(), TEXT_ALIGN_CENTER);
  p->drawRect(Rect(x, y + 20, 130, 130));
  p->drawImage(Point(x + 1, y + 21), im);
}

// ============================================================================
// [MAIN]
// ============================================================================

FOG_UI_MAIN()
{
  Application app(Ascii8("UI"));

  MyWindow window;
  window.setSize(Size(20 + 6 * 152 - 22, 20 + 4 * 152));
  window.show();
  window.addListener(EV_CLOSE, &app, &Application::quit);

  return app.run();
}
