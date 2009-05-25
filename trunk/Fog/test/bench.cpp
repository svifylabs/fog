#include <Fog/Build/Build.h>

#if defined(FOG_OS_WINDOWS)
#include <windows.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
#endif // FOG_OS_WINDOWS

#include <Fog/Core.h>
#include <Fog/Graphics.h>

using namespace Fog;

// ============================================================================
// [Sprites]
// ============================================================================

static Image _sprite[4];

static void loadSprites()
{
  //_sprite[0].readFile(StubAscii8("/my/upload/img/babelfish.png"));
  _sprite[0].readFile(StubAscii8("C:/My/img/babelfish.pcx"));
  _sprite[0].premultiply();
  //_sprite[1].readFile(StubAscii8("/my/upload/img/blockdevice.png"));
  _sprite[1].readFile(StubAscii8("C:/My/img/blockdevice.pcx"));
  _sprite[1].premultiply();
  //_sprite[2].readFile(StubAscii8("/my/upload/img/drop.png"));
  _sprite[2].readFile(StubAscii8("C:/My/img/drop.pcx"));
  _sprite[2].premultiply();
  //_sprite[3].readFile(StubAscii8("/my/upload/img/kweather.png"));
  _sprite[3].readFile(StubAscii8("C:/My/img/kweather.pcx"));
  _sprite[3].premultiply();
}

// ============================================================================
// [BenchmarkModule]
// ============================================================================

struct BenchmarkModule
{
  BenchmarkModule(int w, int h) : w(w), h(h) {}
  ~BenchmarkModule() {}

  virtual void doBenchmark(int quantity) = 0;
  virtual const char* name() = 0;

  FOG_INLINE uint32_t randColor() const
  { return (rand() & 0xFFFF) | (rand() << 16); }

  int w, h;
};

static TimeDelta bench(BenchmarkModule& mod, int quantity)
{
  // Clear random seed (so all tests will behave identically)
  srand(43);

  TimeTicks ticks = TimeTicks::highResNow();
  mod.doBenchmark(quantity);
  TimeDelta delta =  TimeTicks::highResNow() - ticks;

  fog_debug("%s - %.3f [ms]", mod.name(), delta.inMillisecondsF());

  return delta;
}

// ============================================================================
// [BenchmarkModule_Fog]
// ============================================================================

struct BenchmarkModule_Fog : public BenchmarkModule
{
  BenchmarkModule_Fog(int w, int h) :
    BenchmarkModule(w, h)
  {
    im.create(w, h, Image::FormatPRGB32);
    im.clear(0x00000000);
    p.begin(im);
    setMultithreaded(false);

    for (int a = 0; a < 4; a++)
      sprite[a] = _sprite[a];
  }

  virtual ~BenchmarkModule_Fog()
  {
    p.end();
  }

  void setMultithreaded(bool mt)
  {
    this->mt = mt;
    p.setProperty(StubAscii8("multithreaded"), Value::fromBool(mt));
  }

  Image im;
  Image sprite[4];
  Painter p;
  bool mt;
};

// ============================================================================
// [BenchmarkModule_Fog_Fill]
// ============================================================================

struct BenchmarkModule_Fog_Fill : public BenchmarkModule_Fog
{
  BenchmarkModule_Fog_Fill(int w, int h) :
    BenchmarkModule_Fog(w, h)
  {
  }

  virtual void doBenchmark(int quantity)
  {
    p.save();
    for (int a = 0; a < quantity; a++)
    {
      int rw = 128;
      int rh = 128;
      Rect r(rand() % (w - rw + 1), rand() % (h - rh + 1), rw, rh);

      p.setSource(Rgba(randColor()));
      p.fillRect(r);
    }
    p.restore();
  }

  virtual const char* name() { return mt ? "Fog - Fill (mt)" : "Fog - Fill"; }
};

// ============================================================================
// [BenchmarkModule_Fog_Blit]
// ============================================================================

struct BenchmarkModule_Fog_Blit : public BenchmarkModule_Fog
{
  BenchmarkModule_Fog_Blit(int w, int h) :
    BenchmarkModule_Fog(w, h)
  {
  }

  virtual void doBenchmark(int quantity)
  {
    p.save();
    for (int a = 0; a < quantity; a++)
    {
      int rx = rand() % (w - 128);
      int ry = rand() % (h - 128);

      p.drawImage(Point(rx, ry), sprite[rand() % 4]);
    }
    p.restore();
  }

  virtual const char* name() { return mt ? "Fog - Blit (mt)" : "Fog - Blit"; }
};

// ============================================================================
// [BenchmarkModule_Fog_Blit]
// ============================================================================

struct BenchmarkModule_Fog_Pattern : public BenchmarkModule_Fog
{
  BenchmarkModule_Fog_Pattern(int w, int h) :
    BenchmarkModule_Fog(w, h)
  {
    pattern.setType(Pattern::LinearGradient);
    pattern.setPoints(PointF(w/2.0, h/2.0), PointF(30.0, 30.0));
    pattern.addGradientStop(GradientStop(0.0, Rgba(0xFFFFFFFF)));
    pattern.addGradientStop(GradientStop(0.5, Rgba(0xFFFFFF00)));
    pattern.addGradientStop(GradientStop(1.0, Rgba(0xFF000000)));
    pattern.setGradientRadius(250.0);
  }

  virtual void doBenchmark(int quantity)
  {
    p.save();
    p.setSource(pattern);
    for (int a = 0; a < quantity; a++)
    {
      int rx = rand() % (w - 128);
      int ry = rand() % (h - 128);

      p.fillRect(Rect(rx, ry, 128, 128));
    }
    p.restore();
  }

  virtual const char* name()
  {
    const char* p;

    switch (pattern.type())
    {
      case Pattern::Texture: p = "Texture"; break;
      case Pattern::LinearGradient: p = "LinearGradient"; break;
      case Pattern::RadialGradient: p = "RadialGradient"; break;
      case Pattern::ConicalGradient: p = "ConicalGradient"; break;
    }

    sprintf(buf, "Fog - Pattern - %s%s", p, mt ? " (mt)" : "");
    return buf;
  }

  Pattern pattern;
  char buf[1024];
};

#if defined(FOG_OS_WINDOWS)

// ============================================================================
// [BenchmarkModule_GDI]
// ============================================================================

struct BenchmarkModule_GDI : public BenchmarkModule
{
  BenchmarkModule_GDI(int w, int h) :
    BenchmarkModule(w, h)
  {
    im = createDibSection(w, h);
    HDC dc = CreateCompatibleDC(NULL);
    SelectObject(dc, (HGDIOBJ)im);
    RECT r;
    r.top = 0;
    r.left = 0;
    r.bottom = h;
    r.right = w;
    FillRect(dc, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));
    DeleteDC(dc);

    for (int a = 0; a < 4; a++)
    {
      sprite[a] = createDibSection(128, 128);
      DIBSECTION info;
      GetObject(sprite[a], sizeof(DIBSECTION), &info);

      memcpy(info.dsBm.bmBits, _sprite[a].cFirst(),
        _sprite[a].width() * _sprite[a].height() * 4);
    }
  }

  virtual ~BenchmarkModule_GDI()
  {
    DeleteObject(im);

    for (int a = 0; a < 4; a++)
      DeleteObject(sprite[a]);
  }

  HBITMAP im;
  HBITMAP sprite[4];

  static HBITMAP createDibSection(int w, int h)
  {
    // Create bitmap information
    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize        = sizeof(bmi.bmiHeader);
    bmi.bmiHeader.biWidth       = w;
    // Negative means from top to bottom
    bmi.bmiHeader.biHeight      = -h;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    uint8_t* pixels;
    return CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&pixels, NULL, 0);
  }
};

// ============================================================================
// [BenchmarkModule_GDI_Fill]
// ============================================================================

struct BenchmarkModule_GDI_Fill : public BenchmarkModule_GDI
{
  BenchmarkModule_GDI_Fill(int w, int h) :
    BenchmarkModule_GDI(w, h)
  {
  }

  virtual void doBenchmark(int quantity)
  {
    HDC dc = CreateCompatibleDC(NULL);
    SelectObject(dc, im);
    {
      Gdiplus::Graphics gr(dc);

      for (int a = 0; a < quantity; a++)
      {
        Gdiplus::Color c(randColor());
        Gdiplus::SolidBrush br(c);
        gr.FillRectangle((Gdiplus::Brush*)&br, rand() % (w - 128), rand() % (h - 128), 128, 128);
      }
    }
    DeleteDC(dc);
  }

  virtual const char* name() { return "GDI - Fill"; }
};

// ============================================================================
// [BenchmarkModule_GDI_Blit]
// ============================================================================

struct BenchmarkModule_GDI_Blit : public BenchmarkModule_GDI
{
  BenchmarkModule_GDI_Blit(int w, int h) :
    BenchmarkModule_GDI(w, h)
  {
  }

  virtual void doBenchmark(int quantity)
  {
    HDC dc = CreateCompatibleDC(NULL);

    SelectObject(dc, im);
    {
      Gdiplus::Graphics gr(dc);

      for (int a = 0; a < quantity; a++)
      {
        Gdiplus::Bitmap bm(sprite[rand() % 4], (HPALETTE)NULL);
        gr.DrawImage(&bm, rand() % (w - 128), rand() % (h - 128));
      }
    }
    DeleteDC(dc);
  }

  virtual const char* name() { return "GDI - Blit"; }
};

// ============================================================================
// [BenchmarkModule_GDI_Pattern]
// ============================================================================

struct BenchmarkModule_GDI_Pattern : public BenchmarkModule_GDI
{
  BenchmarkModule_GDI_Pattern(int w, int h) :
    BenchmarkModule_GDI(w, h)
  {
  }

  virtual void doBenchmark(int quantity)
  {
    HDC dc = CreateCompatibleDC(NULL);

    SelectObject(dc, im);
    {
      Gdiplus::Graphics gr(dc);
      Gdiplus::LinearGradientBrush br(
        Gdiplus::PointF(w/2.0, h/2.0), Gdiplus::PointF(30.0, 30.0), 
        Gdiplus::Color(0xFFFFFFFF), Gdiplus::Color(0xFF000000));
      Gdiplus::Color clr[3];
      clr[0].SetValue(0xFFFFFFFF);
      clr[1].SetValue(0xFFFFFF00);
      clr[2].SetValue(0xFF000000);
      Gdiplus::REAL stops[3];
      stops[0] = 0.0;
      stops[1] = 0.5;
      stops[2] = 1.0;
      br.SetInterpolationColors(clr, stops, 3);

      for (int a = 0; a < quantity; a++)
      {
        gr.FillRectangle((Gdiplus::Brush*)&br, rand() % (w - 128), rand() % (h - 128), 128, 128);
      }
    }
    DeleteDC(dc);

    /*{
      Image save;
      DIBSECTION info;
      GetObject(im, sizeof(DIBSECTION), &info);
      err_t err = save.adopt(info.dsBm.bmWidth, info.dsBm.bmHeight, Image::FormatRGB32, 
        (uint8_t*)info.dsBm.bmBits, info.dsBm.bmWidthBytes);
      save.writeFile(StubAscii8("BenchGdiPlus.bmp"));
    }*/
  }

  virtual const char* name() { return "GDI - Pattern - LinearGradient"; }
};

#endif // FOG_OS_WINDOWS

// ============================================================================
// [benchAll]
// ============================================================================

static void benchAll()
{
  int w = 640, h = 480;
  int quantity = 100;

  // FOG - FILL
  {
    BenchmarkModule_Fog_Fill mod(w, h);
    mod.setMultithreaded(false);
    bench(mod, quantity);
    mod.setMultithreaded(true);
    bench(mod, quantity);
  }

  // FOG - BLIT
  {
    BenchmarkModule_Fog_Blit mod(w, h);
    mod.setMultithreaded(false);
    bench(mod, quantity);
    mod.setMultithreaded(true);
    bench(mod, quantity);
  }

  // FOG - PATTERN
  {
    BenchmarkModule_Fog_Pattern mod(w, h);

    mod.setMultithreaded(false);

    mod.pattern.setType(Pattern::LinearGradient);
    bench(mod, quantity);

    mod.pattern.setType(Pattern::RadialGradient);
    bench(mod, quantity);

    mod.pattern.setType(Pattern::ConicalGradient);
    bench(mod, quantity);

    mod.setMultithreaded(true);

    mod.pattern.setType(Pattern::LinearGradient);
    bench(mod, quantity);

    mod.pattern.setType(Pattern::RadialGradient);
    bench(mod, quantity);

    mod.pattern.setType(Pattern::ConicalGradient);
    bench(mod, quantity);
  }

#if defined(FOG_OS_WINDOWS)
  // GDIPLUS - FILL
  {
    BenchmarkModule_GDI_Fill mod(w, h);
    bench(mod, quantity);
  }

  // GDIPLUS - BLIT
  {
    BenchmarkModule_GDI_Blit mod(w, h);
    bench(mod, quantity);
  }

  // GDIPLUS - PATTERN
  {
    BenchmarkModule_GDI_Pattern mod(w, h);
    bench(mod, quantity);
  }
#endif // FOG_OS_WINDOWS
}

// ============================================================================
// [MAIN]
// ============================================================================
#undef main
FOG_UI_MAIN()
{
#if defined(FOG_OS_WINDOWS)
	// Initialize GDI+
  ULONG_PTR gdiplusToken;
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
#endif // FOG_OS_WINDOWS

  loadSprites();
  benchAll();

#if defined(FOG_OS_WINDOWS)
	// Shutdown GDI+
  Gdiplus::GdiplusShutdown(gdiplusToken);
#endif // FOG_OS_WINDOWS

  system("pause");
  return 0;
}
