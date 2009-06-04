#include <Fog/Build/Build.h>

#if defined(FOG_OS_WINDOWS)
#include <windows.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
#else
#include <cairo/cairo.h>
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
  _sprite[0].readFile(StubAscii8("babelfish.pcx"));
  _sprite[0].premultiply();
  _sprite[1].readFile(StubAscii8("blockdevice.pcx"));
  _sprite[1].premultiply();
  _sprite[2].readFile(StubAscii8("drop.pcx"));
  _sprite[2].premultiply();
  _sprite[3].readFile(StubAscii8("kweather.pcx"));
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
  virtual void saveResult() = 0;
  virtual const char* name() = 0;

  FOG_INLINE uint32_t randColor() const
  { return (rand() & 0xFFFF) | (rand() << 16); }

  FOG_INLINE Rect randRect(int rw, int rh) const
  {
    return Rect(rand() % (w - rw), rand() % (h - rh), rw, rh);
  }

  FOG_INLINE Point randPoint() const
  {
    return Point(rand() % (w), rand() % (h));
  }

  FOG_INLINE PointF randPointF() const
  {
    return PointF(rand() % (w), rand() % (h));
  }

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
  mod.saveResult();

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

  virtual void saveResult()
  {
    String32 fileName;
    fileName.set(StubAscii8("bench "));
    fileName.append(StubAscii8(name()));
    fileName.append(StubAscii8(".bmp"));
    im.writeFile(fileName);
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
// [BenchmarkModule_Fog_FillRect]
// ============================================================================

struct BenchmarkModule_Fog_FillRect : public BenchmarkModule_Fog
{
  BenchmarkModule_Fog_FillRect(int w, int h) :
    BenchmarkModule_Fog(w, h)
  {
  }

  virtual void doBenchmark(int quantity)
  {
    p.save();
    for (int a = 0; a < quantity; a++)
    {
      Rect r = randRect(128, 128);
      p.setSource(Rgba(randColor()));
      p.fillRect(r);
    }
    p.restore();
    p.flush();
  }

  virtual const char* name() { return mt ? "Fog - FillRect (mt)" : "Fog - FillRect"; }
};

// ============================================================================
// [BenchmarkModule_Fog_FillPath]
// ============================================================================

struct BenchmarkModule_Fog_FillPath : public BenchmarkModule_Fog
{
  BenchmarkModule_Fog_FillPath(int w, int h) :
    BenchmarkModule_Fog(w, h)
  {
  }

  virtual void doBenchmark(int quantity)
  {
    p.save();
    for (int a = 0; a < quantity; a++)
    {
      Path path;
      path.moveTo(randPointF());
      for (int i = 0; i < 3; i++)
      {
        PointF c0 = randPointF();
        PointF c1 = randPointF();
        path.curve3To(c0, c1);
      }

      p.setSource(Rgba(randColor()));
      p.fillPath(path);
    }
    p.restore();
    p.flush();
  }

  virtual const char* name() { return mt ? "Fog - FillPath (mt)" : "Fog - FillPath"; }
};

// ============================================================================
// [BenchmarkModule_Fog_FillPattern]
// ============================================================================

struct BenchmarkModule_Fog_FillPattern : public BenchmarkModule_Fog
{
  BenchmarkModule_Fog_FillPattern(int w, int h) :
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
      Rect r = randRect(128, 128);
      p.fillRect(r);
    }
    p.restore();
    p.flush();
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

    sprintf(buf, "Fog - FillPattern - %s%s", p, mt ? " (mt)" : "");
    return buf;
  }

  Pattern pattern;
  char buf[1024];
};

// ============================================================================
// [BenchmarkModule_Fog_BlitImage]
// ============================================================================

struct BenchmarkModule_Fog_BlitImage : public BenchmarkModule_Fog
{
  BenchmarkModule_Fog_BlitImage(int w, int h) :
    BenchmarkModule_Fog(w, h)
  {
  }

  virtual void doBenchmark(int quantity)
  {
    p.save();
    for (int a = 0; a < quantity; a++)
    {
      Rect r = randRect(128, 128);
      p.drawImage(Point(r.x(), r.y()), sprite[rand() % 4]);
    }
    p.restore();
    p.flush();
  }

  virtual const char* name() { return mt ? "Fog - BlitImage (mt)" : "Fog - BlitImage"; }
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

  virtual void saveResult()
  {
    DIBSECTION info;
    GetObject(im, sizeof(DIBSECTION), &info);

    Image fim;
    fim.adopt(info.dsBm.bmWidth, info.dsBm.bmHeight, Image::FormatPRGB32, 
      (uint8_t*)info.dsBm.bmBits, info.dsBm.bmWidthBytes);

    String32 fileName;
    fileName.format("bench %s.bmp", name());
    fim.writeFile(fileName);
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
// [BenchmarkModule_GDI_FillRect]
// ============================================================================

struct BenchmarkModule_GDI_FillRect : public BenchmarkModule_GDI
{
  BenchmarkModule_GDI_FillRect(int w, int h) :
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
        Rect r = randRect(128, 128);
        Gdiplus::Color c(randColor());
        Gdiplus::SolidBrush br(c);
        gr.FillRectangle((Gdiplus::Brush*)&br, r.x(), r.y(), r.width(), r.height());
      }
    }
    DeleteDC(dc);
  }

  virtual const char* name() { return "GdiPlus - FillRect"; }
};

// ============================================================================
// [BenchmarkModule_GDI_FillPath]
// ============================================================================

struct BenchmarkModule_GDI_FillPath : public BenchmarkModule_GDI
{
  BenchmarkModule_GDI_FillPath(int w, int h) :
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
        Gdiplus::GraphicsPath path;

        // 1x MoveTo
        // 3x CurveTo => 7
        Gdiplus::PointF curv[7];
        for (int i = 0; i < 7; i++)
        {
          PointF c0 = randPointF();
          curv[i].X = c0.x();
          curv[i].Y = c0.y();
        }
        path.StartFigure();
        path.AddCurve(curv, 7);
        path.CloseFigure();

        Gdiplus::Color c(randColor());
        Gdiplus::SolidBrush br(c);

        gr.FillPath((Gdiplus::Brush*)&br, &path);
      }
    }
    DeleteDC(dc);
  }

  virtual const char* name() { return "GdiPlus - FillPath"; }
};

// ============================================================================
// [BenchmarkModule_GDI_FillPattern]
// ============================================================================

struct BenchmarkModule_GDI_FillPattern : public BenchmarkModule_GDI
{
  BenchmarkModule_GDI_FillPattern(int w, int h) :
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
        Rect r = randRect(128, 128);
        gr.FillRectangle((Gdiplus::Brush*)&br, r.x(), r.y(), r.width(), r.height());
      }
    }
    DeleteDC(dc);
  }

  virtual const char* name() { return "GdiPlus - FillPattern - LinearGradient"; }
};

// ============================================================================
// [BenchmarkModule_GDI_BlitImage]
// ============================================================================

struct BenchmarkModule_GDI_BlitImage : public BenchmarkModule_GDI
{
  BenchmarkModule_GDI_BlitImage(int w, int h) :
    BenchmarkModule_GDI(w, h)
  {
  }

  virtual void doBenchmark(int quantity)
  {
    HDC dc = CreateCompatibleDC(NULL);

    SelectObject(dc, im);
    {
      Gdiplus::Graphics gr(dc);
      Gdiplus::Bitmap* bm[4];
      bm[0] = new Gdiplus::Bitmap(sprite[0], (HPALETTE)NULL);
      bm[1] = new Gdiplus::Bitmap(sprite[1], (HPALETTE)NULL);
      bm[2] = new Gdiplus::Bitmap(sprite[2], (HPALETTE)NULL);
      bm[3] = new Gdiplus::Bitmap(sprite[3], (HPALETTE)NULL);

      for (int a = 0; a < quantity; a++)
      {
        Rect r = randRect(128, 128);
        gr.DrawImage(bm[rand() % 4], r.x(), r.y());
      }

      delete bm[0];
      delete bm[1];
      delete bm[2];
      delete bm[3];
    }
    DeleteDC(dc);
  }

  virtual const char* name() { return "GdiPlus - BlitImage"; }
};

#else

// ============================================================================
// [BenchmarkModule_Cairo]
// ============================================================================

struct BenchmarkModule_Cairo : public BenchmarkModule_Fog
{
  BenchmarkModule_Cairo(int w, int h) :
    BenchmarkModule_Fog(w, h)
  {
    cim = cairo_image_surface_create_for_data(
      (unsigned char*)im.cFirst(), CAIRO_FORMAT_ARGB32,
      im.width(), im.height(), im.stride());

    for (int a = 0; a < 4; a++)
      csprite[a] = cairo_image_surface_create_for_data(
        (unsigned char*)sprite[a].cFirst(), CAIRO_FORMAT_ARGB32,
        sprite[a].width(), sprite[a].height(), sprite[a].stride());
  }

  virtual ~BenchmarkModule_Cairo()
  {
    cairo_surface_destroy(cim);

    for (int a = 0; a < 4; a++)
      cairo_surface_destroy(csprite[a]);
  }

  cairo_surface_t* cim;
  cairo_surface_t* csprite[4];
};

// ============================================================================
// [BenchmarkModule_Cairo_FillRect]
// ============================================================================

struct BenchmarkModule_Cairo_FillRect : public BenchmarkModule_Cairo
{
  BenchmarkModule_Cairo_FillRect(int w, int h) :
    BenchmarkModule_Cairo(w, h)
  {
  }

  virtual void doBenchmark(int quantity)
  {
    cairo_t* cr = cairo_create(cim);

    for (int a = 0; a < quantity; a++)
    {
      Rect r = randRect(128, 128);
      Rgba c(randColor());

      cairo_set_source_rgba(cr,
        (double)c.r / 255.0, (double)c.g / 255.0, (double)c.b / 255.0, (double)c.a / 255.0);
      cairo_rectangle(cr, r.x(), r.y(), r.width(), r.height());
      cairo_fill(cr);
    }

    cairo_destroy(cr);
  }

  virtual const char* name() { return "Cairo - FillRect"; }
};

// ============================================================================
// [BenchmarkModule_Cairo_FillPath]
// ============================================================================

struct BenchmarkModule_Cairo_FillPath : public BenchmarkModule_Cairo
{
  BenchmarkModule_Cairo_FillPath(int w, int h) :
    BenchmarkModule_Cairo(w, h)
  {
  }

  virtual void doBenchmark(int quantity)
  {
    cairo_t* cr = cairo_create(cim);

    for (int a = 0; a < quantity; a++)
    {
      PointF c0 = randPointF();
      cairo_move_to(cr, c0.x(), c0.y());

      for (int i = 0; i < 3; i++)
      {
        PointF c1 = randPointF();
        PointF c2 = randPointF();

        cairo_curve_to(cr, 
          c1.x(),
          c1.y(),
          c2.x(),
          c2.y(),
          c2.x(),
          c2.y());
        c0 = c2;
      }

      Rgba c(randColor());
      cairo_set_source_rgba(cr,
        (double)c.r / 255.0, (double)c.g / 255.0, (double)c.b / 255.0, (double)c.a / 255.0);
      cairo_fill(cr);
    }

    cairo_destroy(cr);
  }

  virtual const char* name() { return "Cairo - FillPath"; }
};

// ============================================================================
// [BenchmarkModule_FillPattern]
// ============================================================================

struct BenchmarkModule_Cairo_FillPattern : public BenchmarkModule_Cairo
{
  BenchmarkModule_Cairo_FillPattern(int w, int h) :
    BenchmarkModule_Cairo(w, h),
    type(0)
  {
  }

  virtual void doBenchmark(int quantity)
  {
    cairo_t* cr = cairo_create(cim);


    cairo_pattern_t *pat;

    if (type == 0)
    {
      pat = cairo_pattern_create_linear(w/2, h/2, 30.0, 30.0);
    }
    else if (type == 1)
    {
      pat = cairo_pattern_create_radial(w/2, h/2, 250.0, 30.0, 30.0, 1.0);
    }

    cairo_pattern_add_color_stop_rgba(pat, 0.0, 0, 0, 0, 1);
    cairo_pattern_add_color_stop_rgba(pat, 0.5, 1, 1, 0, 1);
    cairo_pattern_add_color_stop_rgba(pat, 1.0, 1, 1, 1, 1);
    cairo_set_source(cr, pat);

    for (int a = 0; a < quantity; a++)
    {
      Rect r = randRect(128, 128);
      cairo_rectangle(cr, r.x(), r.y(), r.width(), r.height());
      cairo_fill(cr);
    }

    cairo_destroy(cr);
    cairo_pattern_destroy(pat);
  }

  virtual const char* name()
  {
    switch (type)
    {
      case 0: return "Cairo - FillPattern - LinearGradient";
      case 1: return "Cairo - FillPattern - RadialGradient";
    }
    return NULL;
  }

  int type;
};

// ============================================================================
// [BenchmarkModule_Cairo_BlitImage]
// ============================================================================

struct BenchmarkModule_Cairo_BlitImage : public BenchmarkModule_Cairo
{
  BenchmarkModule_Cairo_BlitImage(int w, int h) :
    BenchmarkModule_Cairo(w, h)
  {
  }

  virtual void doBenchmark(int quantity)
  {
    cairo_t* cr = cairo_create(cim);

    for (int a = 0; a < quantity; a++)
    {
      Rect r = randRect(128, 128);
      cairo_set_source_surface(cr, csprite[rand()%4], r.x(), r.y());
      cairo_paint(cr);
    }

    cairo_destroy(cr);
  }

  virtual const char* name() { return "Cairo - BlitImage"; }
};

#endif 

// ============================================================================
// [benchAll]
// ============================================================================

static void benchAll()
{
  int w = 640, h = 480;
  int quantity = 100000;

  TimeDelta totalFog;
  TimeDelta totalFogMT;
  TimeDelta totalCairo;
  TimeDelta totalGdiPlus;

  // Fog - FillRect
  {
    BenchmarkModule_Fog_FillRect mod(w, h);
    mod.setMultithreaded(false);
    totalFog += bench(mod, quantity);
    mod.setMultithreaded(true);
    totalFogMT += bench(mod, quantity);
  }

  // Fog - FillPath
  {
    BenchmarkModule_Fog_FillPath mod(w, h);
    mod.setMultithreaded(false);
    totalFog += bench(mod, quantity);
    mod.setMultithreaded(true);
    totalFogMT += bench(mod, quantity);
  }

  // Fog - FillPattern
  {
    BenchmarkModule_Fog_FillPattern mod(w, h);

    mod.setMultithreaded(false);

    mod.pattern.setType(Pattern::LinearGradient);
    totalFog += bench(mod, quantity);

    mod.pattern.setType(Pattern::RadialGradient);
    totalFog += bench(mod, quantity);

    //mod.pattern.setType(Pattern::ConicalGradient);
    //totalFog += bench(mod, quantity);

    mod.setMultithreaded(true);

    mod.pattern.setType(Pattern::LinearGradient);
    totalFogMT += bench(mod, quantity);

    mod.pattern.setType(Pattern::RadialGradient);
    totalFogMT += bench(mod, quantity);

    //mod.pattern.setType(Pattern::ConicalGradient);
    //totalFogMT += bench(mod, quantity);
  }

  // Fog - BlitImage
  {
    BenchmarkModule_Fog_BlitImage mod(w, h);
    mod.setMultithreaded(false);
    totalFog += bench(mod, quantity);
    mod.setMultithreaded(true);
    totalFogMT += bench(mod, quantity);
  }

#if defined(FOG_OS_WINDOWS)
  // GdiPlus - FillRect
  {
    BenchmarkModule_GDI_FillRect mod(w, h);
    totalGdiPlus += bench(mod, quantity);
  }

  // GdiPlus - FillPath
  {
    BenchmarkModule_GDI_FillPath mod(w, h);
    totalGdiPlus += bench(mod, quantity);
  }

  // GdiPlus - FillPattern
  {
    BenchmarkModule_GDI_FillPattern mod(w, h);
    totalGdiPlus += bench(mod, quantity);
  }

  // GdiPlus - BlitImage
  {
    BenchmarkModule_GDI_BlitImage mod(w, h);
    totalGdiPlus += bench(mod, quantity);
  }
#else
  // Cairo - FillRect
  {
    BenchmarkModule_Cairo_FillRect mod(w, h);
    totalCairo += bench(mod, quantity);
  }

  // Cairo - FillPath
  {
    BenchmarkModule_Cairo_FillPath mod(w, h);
    totalCairo += bench(mod, quantity);
  }

  // Cairo - FillPattern
  {
    BenchmarkModule_Cairo_FillPattern mod(w, h);
    totalCairo += bench(mod, quantity);

    mod.type = 1;
    totalCairo += bench(mod, quantity);
  }

  // Cairo - BlitImage
  {
    BenchmarkModule_Cairo_BlitImage mod(w, h);
    totalCairo += bench(mod, quantity);
  }
#endif // FOG_OS_WINDOWS

  fog_debug("");
  fog_debug("Summary:");
  fog_debug("Fog        - %.3f [ms]", totalFog.inMillisecondsF());
  fog_debug("Fog (mt)   - %.3f [ms]", totalFogMT.inMillisecondsF());
#if defined(FOG_OS_WINDOWS)
  fog_debug("GDI+       - %.3f [ms]", totalGdiPlus.inMillisecondsF());
#else
  fog_debug("Cairo      - %.3f [ms]", totalCairo.inMillisecondsF());
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

  system("pause");
#endif // FOG_OS_WINDOWS

  return 0;
}
