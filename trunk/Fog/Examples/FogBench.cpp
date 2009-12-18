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

#define NUM_SPRITES 4

// ============================================================================
// [Sprites]
// ============================================================================

static Image _sprite[NUM_SPRITES];

static void loadSprites()
{
  bool spritesNotFound = false;
  const char* spriteNames[NUM_SPRITES] = {
    "babelfish.pcx",
    "blockdevice.pcx",
    "drop.pcx",
    "kweather.pcx"
  };

  for (int i = 0; i < NUM_SPRITES; i++)
  {
    if (_sprite[i].readFile(Ascii8(spriteNames[i])) != ERR_OK)
    {
      fog_debug("Warning: Can't load sprite './%s'", spriteNames[i]);
      spritesNotFound = true;
    }
    else
    {
      _sprite[i].premultiply();
    }
  }

  if (spritesNotFound)
  {
    fog_debug("\nDownload sprites from these locations and place them to 'bench' working directory:");
    for (int i = 0; i < NUM_SPRITES; i++)
    {
      fog_debug("  http://kobalicek.com/data/fog/sprites/%s", spriteNames[i]);
    }
    fog_debug("\nBlitImage test will be incorrect!\n");
  }
}

// ============================================================================
// [Tools]
// ============================================================================

static FOG_INLINE uint32_t randColor()
{
  return (rand() & 0xFFFF) | (rand() << 16);
}

static FOG_INLINE Rect randRect(int w, int h, int rw, int rh)
{
  return Rect(rand() % (w - rw), rand() % (h - rh), rw, rh);
}

static FOG_INLINE Point randPoint(int w, int h)
{
  return Point(rand() % w, rand() % h);
}

static FOG_INLINE PointD randPointD(int w, int h)
{
  return PointD(rand() % w, rand() % h);
}

// ============================================================================
// [BenchmarkModule]
// ============================================================================

struct BenchmarkModule
{
  BenchmarkModule(int w, int h) : w(w), h(h) {}
  ~BenchmarkModule() {}

  virtual void prepareBenchmark(int sw, int sh, int quantity) {}
  virtual void finishBenchmark() {}
  virtual void doBenchmark(int quantity) = 0;

  virtual void saveResult(int sw, int sh) = 0;

  virtual ByteArray getEngine() = 0;
  virtual ByteArray getType() = 0;
  
  virtual ByteArray getInfo()
  {
    ByteArray i = getEngine();
    i.append(" - ");
    i.append(getType());
    return i;
  }

  int w, h;
};

// ============================================================================
// [BenchmarkRandomizer_Rect]
// ============================================================================

struct BenchmarkRandomizer_Rect
{
  BenchmarkRandomizer_Rect()
  {
    rectData = NULL;
    rgbaData = NULL;
  }

  void prepare(int w, int h, int sw, int sh, int quantity)
  {
    int a;

    rectData = (Rect*)Memory::alloc(sizeof(Rect) * quantity);
    rgbaData = (Argb*)Memory::alloc(sizeof(Argb) * quantity);

    for (a = 0; a < quantity; a++)
    {
      rectData[a] = randRect(w, h, sw, sh);
      rgbaData[a] = randColor();
    }
  }

  void finish()
  {
    Memory::free(rectData);
    Memory::free(rgbaData);
  }

  Rect* rectData;
  Argb* rgbaData;
};

// ============================================================================
// [BenchmarkRandomizer_Polygon]
// ============================================================================

struct BenchmarkRandomizer_Polygon
{
  BenchmarkRandomizer_Polygon()
  {
    polyData = NULL;
    rgbaData = NULL;
  }

  void prepare(int w, int h, int sw, int sh, int quantity)
  {
    double x = 0.0;
    double y = 0.0;
    int a;

    polyData = (PointD*)Memory::alloc(sizeof(PointD) * quantity * 10);
    rgbaData = (Argb  *)Memory::alloc(sizeof(Argb) * quantity);

    for (a = 0; a < quantity * 10; a++)
    {
      if ((a % 10) == 0)
      {
        x = (rand() % (w - sw));
        y = (rand() % (h - sh));
      }
      polyData[a] = randPointD(sw, sh).translated(x, y);
    }

    for (a = 0; a < quantity; a++)
    {
      rgbaData[a] = randColor();
    }
  }

  void finish()
  {
    Memory::free(polyData);
    Memory::free(rgbaData);
  }

  PointD* polyData;
  Argb* rgbaData;
};

// ============================================================================
// [BenchmarkModule_Fog]
// ============================================================================

struct BenchmarkModule_Fog : public BenchmarkModule
{
  FOG_NO_INLINE BenchmarkModule_Fog(int w, int h) :
    BenchmarkModule(w, h)
  {
    im.create(w, h, PIXEL_FORMAT_PRGB32);
    im.clear(0x00000000);
    p.begin(im);
    setEngine(PAINTER_ENGINE_RASTER_ST);

    for (int a = 0; a < NUM_SPRITES; a++)
    {
      sprite[a] = _sprite[a];
    }
  }

  FOG_NO_INLINE virtual ~BenchmarkModule_Fog()
  {
    p.end();
  }

  FOG_NO_INLINE virtual void saveResult(int sw, int sh)
  {
    String fileName;
    ByteArray info = getInfo();

    fileName.set(Ascii8("Images/Bench - "));
    fileName.append(Ascii8(info.getData(), info.getLength()));
    if (sw && sh) fileName.appendFormat(" [%dx%d]", sw, sh);
    fileName.append(Ascii8(".bmp"));

    Image t(im);
    t.writeFile(fileName);
  }

  FOG_NO_INLINE virtual ByteArray getEngine()
  {
    ByteArray info;
    info.format("Fog (%s)", mt ? "mt" : "st");
    return info;
  }

  FOG_NO_INLINE void setEngine(int engine)
  {
    this->mt = (engine == PAINTER_ENGINE_RASTER_MT);
    p.setEngine(engine);
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

  virtual void prepareBenchmark(int sw, int sh, int quantity) { randomizer.prepare(w, h, sw, sh, quantity); }
  virtual void finishBenchmark() { randomizer.finish(); }

  virtual void doBenchmark(int quantity)
  {
    p.save();
    for (int a = 0; a < quantity; a++)
    {
      p.setSource(randomizer.rgbaData[a]);
      p.fillRect(randomizer.rectData[a]);
    }
    p.restore();
    p.flush();
  }

  virtual ByteArray getType() { return ByteArray("FillRect"); }

  BenchmarkRandomizer_Rect randomizer;
};

// ============================================================================
// [BenchmarkModule_Fog_FillRound]
// ============================================================================

struct BenchmarkModule_Fog_FillRound : public BenchmarkModule_Fog
{
  BenchmarkModule_Fog_FillRound(int w, int h) :
    BenchmarkModule_Fog(w, h)
  {
  }

  virtual void prepareBenchmark(int sw, int sh, int quantity) { randomizer.prepare(w, h, sw, sh, quantity); }
  virtual void finishBenchmark() { randomizer.finish(); }

  virtual void doBenchmark(int quantity)
  {
    p.save();
    for (int a = 0; a < quantity; a++)
    {
      p.setSource(randomizer.rgbaData[a]);
      p.fillRound(randomizer.rectData[a], Point(8, 8));
    }
    p.restore();
    p.flush();
  }

  virtual ByteArray getType() { return ByteArray("FillRound"); }

  BenchmarkRandomizer_Rect randomizer;
};

// ============================================================================
// [BenchmarkModule_Fog_FillPolygon]
// ============================================================================

struct BenchmarkModule_Fog_FillPolygon : public BenchmarkModule_Fog
{
  BenchmarkRandomizer_Polygon randomizer;

  BenchmarkModule_Fog_FillPolygon(int w, int h) :
    BenchmarkModule_Fog(w, h)
  {
  }

  virtual void prepareBenchmark(int sw, int sh, int quantity) { randomizer.prepare(w, h, sw, sh, quantity); }
  virtual void finishBenchmark() { randomizer.finish(); }

  virtual void doBenchmark(int quantity)
  {
    p.save();
    p.setFillMode(FILL_EVEN_ODD);

    for (int a = 0; a < quantity; a++)
    {
      const PointD* polyData = &randomizer.polyData[a*10];

      Path path;
      for (int i = 0; i < 10; i++)
      {
        PointD c0 = polyData[i];
        if (i == 0)
          path.moveTo(c0);
        else
          path.lineTo(c0);
      }

      p.setSource(randomizer.rgbaData[a]);
      p.fillPath(path);
    }

    p.restore();
    p.flush();
  }

  virtual ByteArray getType() { return ByteArray("FillPolygon"); }
};

// ============================================================================
// [BenchmarkModule_Fog_FillPattern]
// ============================================================================

struct BenchmarkModule_Fog_FillPattern : public BenchmarkModule_Fog
{
  BenchmarkModule_Fog_FillPattern(int w, int h) :
    BenchmarkModule_Fog(w, h)
  {
    pattern.setType(PATTERN_LINEAR_GRADIENT);
    pattern.setPoints(PointD(w/2.0, h/2.0), PointD(30.0, 30.0));
    pattern.addStop(ArgbStop(0.0, Argb(0xFFFFFFFF)));
    pattern.addStop(ArgbStop(0.5, Argb(0xFFFFFF00)));
    pattern.addStop(ArgbStop(1.0, Argb(0xFF000000)));
    pattern.setRadius(250.0);
  }

  virtual void prepareBenchmark(int sw, int sh, int quantity) { randomizer.prepare(w, h, sw, sh, quantity); }
  virtual void finishBenchmark() { randomizer.finish(); }

  virtual void doBenchmark(int quantity)
  {
    p.save();
    p.setSource(pattern);
    for (int a = 0; a < quantity; a++)
    {
      p.fillRect(randomizer.rectData[a]);
    }
    p.restore();
    p.flush();
  }

  virtual ByteArray getType()
  {
    const char* p = "";

    switch (pattern.getType())
    {
      case PATTERN_TEXTURE: p = "Texture"; break;
      case PATTERN_LINEAR_GRADIENT: p = "LinearGradient"; break;
      case PATTERN_RADIAL_GRADIENT: p = "RadialGradient"; break;
      case PATTERN_CONICAL_GRADIENT: p = "ConicalGradient"; break;
    }

    return ByteArray(p);
  }

  Pattern pattern;
  char buf[1024];

  BenchmarkRandomizer_Rect randomizer;
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

  virtual void prepareBenchmark(int sw, int sh, int quantity)
  {
    randomizer.prepare(w, h, sw, sh, quantity);
    for (int a = 0; a < 4; a++) images[a] = sprite[a].scale(Size(sw, sh));
  }

  virtual void finishBenchmark() { randomizer.finish(); }

  virtual void doBenchmark(int quantity)
  {
    p.save();
    for (int a = 0; a < quantity; a++)
    {
      p.drawImage(randomizer.rectData[a].getPosition(), images[randomizer.rgbaData[a].value % NUM_SPRITES]);
    }
    p.restore();
    p.flush();
  }

  virtual ByteArray getType() { return ByteArray("BlitImage"); }

  BenchmarkRandomizer_Rect randomizer;
  Image images[NUM_SPRITES];
};

// ============================================================================
// [BenchmarkModule_Fog_BlitImageAffine]
// ============================================================================

struct BenchmarkModule_Fog_BlitImageAffine : public BenchmarkModule_Fog_BlitImage
{
  BenchmarkModule_Fog_BlitImageAffine(int w, int h) :
    BenchmarkModule_Fog_BlitImage(w, h)
  {
  }

  virtual void doBenchmark(int quantity)
  {
    p.save();
    double rot = 0.0;
    for (int a = 0; a < quantity; a++, rot += 0.01)
    {
      Matrix m;
      m.rotate(rot);
      p.setMatrix(m);
      p.drawImage(randomizer.rectData[a].getPosition(), images[randomizer.rgbaData[a].value % NUM_SPRITES]);
    }
    p.restore();
    p.flush();
  }

  virtual ByteArray getType() { return ByteArray("BlitImageAffine"); }
};

// ============================================================================
// [BenchmarkModule_Fog_RasterText]
// ============================================================================

struct BenchmarkModule_Fog_RasterText : public BenchmarkModule_Fog
{
  BenchmarkModule_Fog_RasterText(int w, int h) :
    BenchmarkModule_Fog(w, h)
  {
  }

  virtual void prepareBenchmark(int sw, int sh, int quantity) { randomizer.prepare(w, h, sw, sh, quantity); }
  virtual void finishBenchmark() { randomizer.finish(); }

  virtual void doBenchmark(int quantity)
  {
    String text(Ascii8("abcdef"));
    Font font;

    p.save();
    for (int a = 0; a < quantity; a++)
    {
      p.setSource(randomizer.rgbaData[a]);
      p.drawText(randomizer.rectData[a].getPosition(), text, font);
    }
    p.restore();
    p.flush();
  }

  virtual ByteArray getType() { return ByteArray("RasterText"); }

  BenchmarkRandomizer_Rect randomizer;
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

    for (int a = 0; a < NUM_SPRITES; a++)
    {
      sprite[a] = createDibSection(128, 128);
      DIBSECTION info;
      GetObject(sprite[a], sizeof(DIBSECTION), &info);

      memcpy(info.dsBm.bmBits, _sprite[a].getFirst(),
        _sprite[a].getWidth() * _sprite[a].getHeight() * 4);
    }
  }

  virtual ~BenchmarkModule_GDI()
  {
    DeleteObject(im);

    for (int a = 0; a < NUM_SPRITES; a++) DeleteObject(sprite[a]);
  }

  virtual void saveResult(int sw, int sh)
  {
    String fileName;
    ByteArray info = getInfo();

    fileName.set(Ascii8("Images/Bench - "));
    fileName.append(Ascii8(info.getData(), info.getLength()));
    if (sw && sh) fileName.appendFormat(" [%dx%d]", sw, sh);
    fileName.append(Ascii8(".bmp"));

    DIBSECTION dibs;
    GetObject(im, sizeof(DIBSECTION), &dibs);

    ImageBuffer buffer;
    buffer.width = dibs.dsBm.bmWidth;
    buffer.height = dibs.dsBm.bmHeight;
    buffer.format = PIXEL_FORMAT_PRGB32;
    buffer.stride = dibs.dsBm.bmWidthBytes;
    buffer.data = (uint8_t*)dibs.dsBm.bmBits;

    Image fim;
    fim.adopt(buffer);
    fim.writeFile(fileName);
  }

  HBITMAP im;
  HBITMAP sprite[NUM_SPRITES];

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

  virtual ByteArray getEngine() { return ByteArray("GDI+"); }
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

  virtual void prepareBenchmark(int sw, int sh, int quantity) { randomizer.prepare(w, h, sw, sh, quantity); }
  virtual void finishBenchmark() { randomizer.finish(); }

  virtual void doBenchmark(int quantity)
  {
    HDC dc = CreateCompatibleDC(NULL);
    SelectObject(dc, im);
    {
      Gdiplus::Graphics gr(dc);

      for (int a = 0; a < quantity; a++)
      {
        Rect r = randomizer.rectData[a];
        Gdiplus::Color c(randomizer.rgbaData[a]);
        Gdiplus::SolidBrush br(c);
        gr.FillRectangle((Gdiplus::Brush*)&br, r.getX(), r.getY(), r.getWidth(), r.getHeight());
      }
    }
    DeleteDC(dc);
  }

  virtual ByteArray getType() { return ByteArray("FillRect"); }

  BenchmarkRandomizer_Rect randomizer;
};

// ============================================================================
// [BenchmarkModule_GDI_FillRound]
// ============================================================================

struct BenchmarkModule_GDI_FillRound : public BenchmarkModule_GDI
{
  BenchmarkModule_GDI_FillRound(int w, int h) :
    BenchmarkModule_GDI(w, h)
  {
  }

  virtual void prepareBenchmark(int sw, int sh, int quantity) { randomizer.prepare(w, h, sw, sh, quantity); }
  virtual void finishBenchmark() { randomizer.finish(); }

  virtual void doBenchmark(int quantity)
  {
    HDC dc = CreateCompatibleDC(NULL);
    SelectObject(dc, im);

    {
      Gdiplus::Graphics gr(dc);
      gr.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

      for (int a = 0; a < quantity; a++)
      {
        Rect r = randomizer.rectData[a];
        int d = 8;
        Gdiplus::Color c(randomizer.rgbaData[a]);
        Gdiplus::SolidBrush br(c);

        Gdiplus::GraphicsPath path;
		    path.AddArc(r.getX(), r.getY(), d, d, 180, 90);
		    path.AddArc(r.getX() + r.getWidth() - d, r.getY(), d, d, 270, 90);
		    path.AddArc(r.getX() + r.getWidth() - d, r.getY() + r.getHeight() - d, d, d, 0, 90);
		    path.AddArc(r.getX(), r.getY() + r.getHeight() - d, d, d, 90, 90);
        path.AddLine(r.getX(), r.getY() + r.getHeight() - d, r.getX(), r.getY() + d/2);
        gr.FillPath((Gdiplus::Brush*)&br, &path);
      }
    }

    DeleteDC(dc);
  }

  virtual ByteArray getType() { return ByteArray("FillRound"); }

  BenchmarkRandomizer_Rect randomizer;
};

// ============================================================================
// [BenchmarkModule_GDI_FillPolygon]
// ============================================================================

struct BenchmarkModule_GDI_FillPolygon : public BenchmarkModule_GDI
{
  BenchmarkModule_GDI_FillPolygon(int w, int h) :
    BenchmarkModule_GDI(w, h)
  {
  }

  virtual void prepareBenchmark(int sw, int sh, int quantity) { randomizer.prepare(w, h, sw, sh, quantity); }
  virtual void finishBenchmark() { randomizer.finish(); }

  virtual void doBenchmark(int quantity)
  {
    HDC dc = CreateCompatibleDC(NULL);
    SelectObject(dc, im);
    {
      Gdiplus::Graphics gr(dc);
      gr.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);

      for (int a = 0; a < quantity; a++)
      {
        const PointD* polyData = &randomizer.polyData[a*10];

        Gdiplus::GraphicsPath path;
        Gdiplus::PointF lines[10];
        for (int i = 0; i < 10; i++)
        {
          lines[i].X = (Gdiplus::REAL)polyData[i].getX();
          lines[i].Y = (Gdiplus::REAL)polyData[i].getY();
        }
        path.AddLines(lines, 10);
        path.CloseFigure();

        Gdiplus::Color c(randomizer.rgbaData[a]);
        Gdiplus::SolidBrush br(c);

        gr.FillPath((Gdiplus::Brush*)&br, &path);
      }
    }
    DeleteDC(dc);
  }

  virtual ByteArray getType() { return ByteArray("FillPolygon"); }

  BenchmarkRandomizer_Polygon randomizer;
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

  virtual void prepareBenchmark(int sw, int sh, int quantity) { randomizer.prepare(w, h, sw, sh, quantity); }
  virtual void finishBenchmark() { randomizer.finish(); }

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
        Rect r = randomizer.rectData[a];
        gr.FillRectangle((Gdiplus::Brush*)&br, r.getX(), r.getY(), r.getWidth(), r.getHeight());
      }
    }
    DeleteDC(dc);
  }

  virtual ByteArray getType() { return ByteArray("LinearGradient"); }

  BenchmarkRandomizer_Rect randomizer;
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

  virtual void prepareBenchmark(int sw, int sh, int quantity)
  {
    randomizer.prepare(w, h, sw, sh, quantity);
  }

  virtual void finishBenchmark() { randomizer.finish(); }

  virtual void doBenchmark(int quantity)
  {
    HDC dc = CreateCompatibleDC(NULL);

    SelectObject(dc, im);
    {
      Gdiplus::Graphics gr(dc);
      Gdiplus::Bitmap* bm[NUM_SPRITES];

      int a;

      for (a = 0; a < NUM_SPRITES; a++)
      {
        bm[a] = new Gdiplus::Bitmap(
          _sprite[a].getWidth(), 
          _sprite[a].getHeight(),
          _sprite[a].getStride(),
          PixelFormat32bppPARGB,
          (BYTE*)_sprite[a].getFirst());
      }

      for (a = 0; a < quantity; a++)
      {
        int x = randomizer.rectData[a].x;
        int y = randomizer.rectData[a].y;
        gr.DrawImage(bm[randomizer.rgbaData[a].value % NUM_SPRITES], x, y);
      }

      for (a = 0; a < NUM_SPRITES; a++)
      {
        delete bm[a];
      }
    }
    DeleteDC(dc);
  }

  virtual ByteArray getType() { return ByteArray("BlitImage"); }

  BenchmarkRandomizer_Rect randomizer;
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
      (unsigned char*)im.getFirst(), CAIRO_FORMAT_ARGB32,
      im.getWidth(), im.getHeight(), im.getStride());
  }

  virtual ~BenchmarkModule_Cairo()
  {
    cairo_surface_destroy(cim);
  }

  virtual ByteArray getEngine() { return ByteArray("Cairo"); }

  cairo_surface_t* cim;
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

  virtual void prepareBenchmark(int sw, int sh, int quantity) { randomizer.prepare(w, h, sw, sh, quantity); }
  virtual void finishBenchmark() { randomizer.finish(); }

  virtual void doBenchmark(int quantity)
  {
    cairo_t* cr = cairo_create(cim);

    for (int a = 0; a < quantity; a++)
    {
      Rect r = randomizer.rectData[a];
      Argb c(randomizer.rgbaData[a]);

      cairo_set_source_rgba(cr,
        (double)c.r / 255.0, (double)c.g / 255.0, (double)c.b / 255.0, (double)c.a / 255.0);
      cairo_rectangle(cr, r.getX(), r.getY(), r.getWidth(), r.getHeight());
      cairo_fill(cr);
    }

    cairo_destroy(cr);
  }

  virtual ByteArray getType() { return ByteArray("FillRect"); }

  BenchmarkRandomizer_Rect randomizer;
};

// ============================================================================
// [BenchmarkModule_Cairo_FillRound]
// ============================================================================

struct BenchmarkModule_Cairo_FillRound : public BenchmarkModule_Cairo
{
  BenchmarkModule_Cairo_FillRound(int w, int h) :
    BenchmarkModule_Cairo(w, h)
  {
  }

  virtual void prepareBenchmark(int sw, int sh, int quantity) { randomizer.prepare(w, h, sw, sh, quantity); }
  virtual void finishBenchmark() { randomizer.finish(); }

  virtual void doBenchmark(int quantity)
  {
    cairo_t* cr = cairo_create(cim);

    for (int a = 0; a < quantity; a++)
    {
      Rect r = randomizer.rectData[a];
      Argb c(randomizer.rgbaData[a]);

      addRound(cr, r, 8);
      cairo_set_source_rgba(cr,
        (double)c.r / 255.0, (double)c.g / 255.0, (double)c.b / 255.0, (double)c.a / 255.0);
      cairo_fill(cr);
    }

    cairo_destroy(cr);
  }

  FOG_INLINE void addRound(cairo_t* cr, Rect rect, double radius)
  {
    double x0 = rect.getX();
    double y0 = rect.getY();
    double rect_width = rect.getWidth();
    double rect_height = rect.getHeight();

    double x1 = x0 + rect_width;
    double y1 = y0 + rect_height;

    radius *= 2.0;

    if (!rect_width || !rect_height)
      return;

    if (rect_width/2<radius)
    {
      if (rect_height/2<radius)
      {
        cairo_move_to(cr, x0, (y0 + y1)/2);
        cairo_curve_to(cr, x0 ,y0, x0, y0, (x0 + x1)/2, y0);
        cairo_curve_to(cr, x1, y0, x1, y0, x1, (y0 + y1)/2);
        cairo_curve_to(cr, x1, y1, x1, y1, (x1 + x0)/2, y1);
        cairo_curve_to(cr, x0, y1, x0, y1, x0, (y0 + y1)/2);
      }
      else
      {
        cairo_move_to(cr, x0, y0 + radius);
        cairo_curve_to(cr, x0 ,y0, x0, y0, (x0 + x1)/2, y0);
        cairo_curve_to(cr, x1, y0, x1, y0, x1, y0 + radius);
        cairo_line_to(cr, x1 , y1 - radius);
        cairo_curve_to(cr, x1, y1, x1, y1, (x1 + x0)/2, y1);
        cairo_curve_to(cr, x0, y1, x0, y1, x0, y1- radius);
      }
    }
    else
    {
      if (rect_height/2<radius)
      {
        cairo_move_to(cr, x0, (y0 + y1)/2);
        cairo_curve_to(cr, x0 , y0, x0 , y0, x0 + radius, y0);
        cairo_line_to(cr, x1 - radius, y0);
        cairo_curve_to(cr, x1, y0, x1, y0, x1, (y0 + y1)/2);
        cairo_curve_to(cr, x1, y1, x1, y1, x1 - radius, y1);
        cairo_line_to(cr, x0 + radius, y1);
        cairo_curve_to(cr, x0, y1, x0, y1, x0, (y0 + y1)/2);
      }
      else
      {
        cairo_move_to(cr, x0, y0 + radius);
        cairo_curve_to(cr, x0 , y0, x0 , y0, x0 + radius, y0);
        cairo_line_to(cr, x1 - radius, y0);
        cairo_curve_to(cr, x1, y0, x1, y0, x1, y0 + radius);
        cairo_line_to(cr, x1 , y1 - radius);
        cairo_curve_to(cr, x1, y1, x1, y1, x1 - radius, y1);
        cairo_line_to(cr, x0 + radius, y1);
        cairo_curve_to(cr, x0, y1, x0, y1, x0, y1- radius);
      }
    }
    cairo_close_path(cr);
  }

  virtual ByteArray getType() { return ByteArray("FillRound"); }

  BenchmarkRandomizer_Rect randomizer;
};


// ============================================================================
// [BenchmarkModule_Cairo_FillPolygon]
// ============================================================================

struct BenchmarkModule_Cairo_FillPolygon : public BenchmarkModule_Cairo
{
  BenchmarkModule_Cairo_FillPolygon(int w, int h) :
    BenchmarkModule_Cairo(w, h)
  {
  }

  virtual void prepareBenchmark(int sw, int sh, int quantity) { randomizer.prepare(w, h, sw, sh, quantity); }
  virtual void finishBenchmark() { randomizer.finish(); }

  virtual void doBenchmark(int quantity)
  {
    cairo_t* cr = cairo_create(cim);
    cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);

    for (int a = 0; a < quantity; a++)
    {
      const PointD* polyData = &randomizer.polyData[a*10];

      for (int i = 0; i < 10; i++)
      {
        if (i == 0)
          cairo_move_to(cr, polyData[i].getX(), polyData[i].getY());
        else
          cairo_line_to(cr, polyData[i].getX(), polyData[i].getY());
      }
      cairo_close_path(cr);

      Argb c = randomizer.rgbaData[a];
      cairo_set_source_rgba(cr,
        (double)c.r / 255.0, (double)c.g / 255.0, (double)c.b / 255.0, (double)c.a / 255.0);
      cairo_fill(cr);
    }

    cairo_destroy(cr);
  }

  virtual ByteArray getType() { return ByteArray("FillPolygon"); }

  BenchmarkRandomizer_Polygon randomizer;
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

  virtual void prepareBenchmark(int sw, int sh, int quantity) { randomizer.prepare(w, h, sw, sh, quantity); }
  virtual void finishBenchmark() { randomizer.finish(); }

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

    cairo_pattern_add_color_stop_rgba(pat, 0.0, 1, 1, 1, 1);
    cairo_pattern_add_color_stop_rgba(pat, 0.5, 1, 1, 0, 1);
    cairo_pattern_add_color_stop_rgba(pat, 1.0, 0, 0, 0, 1);
    cairo_set_source(cr, pat);

    for (int a = 0; a < quantity; a++)
    {
      Rect r = randomizer.rectData[a];
      cairo_rectangle(cr, r.getX(), r.getY(), r.getWidth(), r.getHeight());
      cairo_fill(cr);
    }

    cairo_destroy(cr);
    cairo_pattern_destroy(pat);
  }

  virtual ByteArray getType()
  {
    switch (type)
    {
      case 0: return ByteArray("LinearGradient");
      case 1: return ByteArray("RadialGradient");
      default: return ByteArray("");
    }
  }

  int type;

  BenchmarkRandomizer_Rect randomizer;
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

  virtual void prepareBenchmark(int sw, int sh, int quantity)
  {
    randomizer.prepare(w, h, sw, sh, quantity);

    for (int a = 0; a < NUM_SPRITES; a++)
    {
      images[a] = sprite[a].scale(Size(sw, sh));
      cimages[a] = cairo_image_surface_create_for_data(
        (unsigned char*)images[a].getFirst(), CAIRO_FORMAT_ARGB32,
        images[a].getWidth(), images[a].getHeight(), images[a].getStride());
    }
  }

  virtual void finishBenchmark()
  {
    randomizer.finish();

    for (int a = 0; a < NUM_SPRITES; a++)
    {
      cairo_surface_destroy(cimages[a]);
    }
  }

  virtual void doBenchmark(int quantity)
  {
    cairo_t* cr = cairo_create(cim);

    for (int a = 0; a < quantity; a++)
    {
      int x = randomizer.rectData[a].x;
      int y = randomizer.rectData[a].y;
      cairo_set_source_surface(cr, cimages[randomizer.rgbaData[a].value % NUM_SPRITES], x, y);
      cairo_paint(cr);
    }

    cairo_destroy(cr);
  }

  virtual ByteArray getType() { return ByteArray("BlitImage"); }

  BenchmarkRandomizer_Rect randomizer;

  Image images[NUM_SPRITES];
  cairo_surface_t* cimages[NUM_SPRITES];
};

// ============================================================================
// [BenchmarkModule_Cairo_BlitImageAffine]
// ============================================================================

struct BenchmarkModule_Cairo_BlitImageAffine : public BenchmarkModule_Cairo_BlitImage
{
  BenchmarkModule_Cairo_BlitImageAffine(int w, int h) :
    BenchmarkModule_Cairo_BlitImage(w, h)
  {
  }

  virtual void doBenchmark(int quantity)
  {
    cairo_t* cr = cairo_create(cim);

    double rot = 0.0;
    for (int a = 0; a < quantity; a++, rot += 0.01)
    {
      int x = randomizer.rectData[a].x;
      int y = randomizer.rectData[a].y;
      cairo_rotate(cr, rot);
      cairo_set_source_surface(cr, cimages[randomizer.rgbaData[a].value % NUM_SPRITES], x, y);
      cairo_paint(cr);
      cairo_identity_matrix(cr);
    }

    cairo_destroy(cr);
  }

  virtual ByteArray getType() { return ByteArray("BlitImageAffine"); }
};

#endif 

// ============================================================================
// [benchAll]
// ============================================================================

static TimeDelta bench(BenchmarkModule& mod, int sw, int sh, int quantity)
{
  // Clear random seed (so all tests will behave identically)
  srand(43);

  mod.prepareBenchmark(sw, sh, quantity);
  TimeTicks ticks = TimeTicks::highResNow();
  mod.doBenchmark(quantity);
  TimeDelta delta =  TimeTicks::highResNow() - ticks;
  mod.finishBenchmark();

  ByteArray s;
  ByteArray info = mod.getInfo();

  if (sw == 0 || sh == 0)
    s.format("%-25s -> %.3f [ms]", info.getData(), delta.inMillisecondsF());
  else
    s.format("%-25s [%dx%d] -> %.3f [ms]", info.getData(), sw, sh, delta.inMillisecondsF());

  fog_debug(s.getData());
  mod.saveResult(sw, sh);

  return delta;
}

static void benchAll()
{
  int w = 640, h = 480;
  int quantity = 100000;

  int engine;
  int s;
  Size sizes[] = {
    Size(10, 10),
    Size(16, 16),
    Size(32, 32),
    Size(64, 64),
    Size(128, 128)
  };

  TimeDelta totalFog[3];
  TimeDelta totalCairo;
  TimeDelta totalGdiPlus;

  // --------------------------------------------------------------------------
  // Header
  // --------------------------------------------------------------------------

  static const char* yesno[2] = { "no", "yes" };

  fog_debug("Surface=%dx%d, Quantity=%d", w, h, quantity);
  fog_debug("%s (MMX=%s, SSE=%s, SSE2=%s, SSE3=%s, cores=%u)",
    cpuInfo->brand,
    yesno[cpuInfo->hasFeature(CpuInfo::FEATURE_MMX)],
    yesno[cpuInfo->hasFeature(CpuInfo::FEATURE_SSE)],
    yesno[cpuInfo->hasFeature(CpuInfo::FEATURE_SSE2)],
    yesno[cpuInfo->hasFeature(CpuInfo::FEATURE_SSE3)],
    cpuInfo->numberOfProcessors);
  fog_debug("");

  // --------------------------------------------------------------------------
  // Fog
  // --------------------------------------------------------------------------

  for (engine = 1; engine < 3; engine++)
  {
    for (s = 0; s < FOG_ARRAY_SIZE(sizes); s++)
    {
      // Fog - FillRect
      {
        BenchmarkModule_Fog_FillRect mod(w, h);
        mod.setEngine(engine);
        totalFog[engine] += bench(mod, sizes[s].w, sizes[s].h, quantity);
      }

      // Fog - FillRound
      {
        BenchmarkModule_Fog_FillRound mod(w, h);
        mod.setEngine(engine);
        totalFog[engine] += bench(mod, sizes[s].w, sizes[s].h, quantity);
      }

      // Fog - FillPolygon
      {
        BenchmarkModule_Fog_FillPolygon mod(w, h);
        mod.setEngine(engine);
        totalFog[engine] += bench(mod, sizes[s].w, sizes[s].h, quantity);
      }

      // Fog - FillPattern
      {
        BenchmarkModule_Fog_FillPattern mod(w, h);
        mod.setEngine(engine);

        mod.pattern.setType(PATTERN_LINEAR_GRADIENT);
        totalFog[engine] += bench(mod, sizes[s].w, sizes[s].h, quantity);

        mod.pattern.setType(PATTERN_RADIAL_GRADIENT);
        totalFog[engine] += bench(mod, sizes[s].w, sizes[s].h, quantity);
      }

      // Fog - BlitImage
      {
        BenchmarkModule_Fog_BlitImage mod(w, h);
        mod.setEngine(engine);
        totalFog[engine] += bench(mod, sizes[s].w, sizes[s].h, quantity);
      }

      // Fog - BlitImageAffine
      {
        BenchmarkModule_Fog_BlitImageAffine mod(w, h);
        mod.setEngine(engine);
        totalFog[engine] += bench(mod, sizes[s].w, sizes[s].h, quantity);
      }
    }

    // Fog - RasterText
    {
      BenchmarkModule_Fog_RasterText mod(w, h);
      mod.setEngine(engine);
      totalFog[engine] += bench(mod, 0, 0, quantity);
    }

    fog_debug("");
  }

#if defined(FOG_OS_WINDOWS)

  // --------------------------------------------------------------------------
  // GdiPlus
  // --------------------------------------------------------------------------

  for (s = 0; s < FOG_ARRAY_SIZE(sizes); s++)
  {
    // GdiPlus - FillRect
    {
      BenchmarkModule_GDI_FillRect mod(w, h);
      totalGdiPlus += bench(mod, sizes[s].w, sizes[s].h, quantity);
    }

    // GdiPlus - FillRound
    {
      BenchmarkModule_GDI_FillRound mod(w, h);
      totalGdiPlus += bench(mod, sizes[s].w, sizes[s].h, quantity);
    }

    // GdiPlus - FillPolygon
    {
      BenchmarkModule_GDI_FillPolygon mod(w, h);
      totalGdiPlus += bench(mod, sizes[s].w, sizes[s].h, quantity);
    }

    // GdiPlus - FillPattern
    {
      BenchmarkModule_GDI_FillPattern mod(w, h);
      totalGdiPlus += bench(mod, sizes[s].w, sizes[s].h, quantity);
    }

    // GdiPlus - BlitImage
    {
      BenchmarkModule_GDI_BlitImage mod(w, h);
      totalGdiPlus += bench(mod, sizes[s].w, sizes[s].h, quantity);
    }
  }

  fog_debug("");

#else
  // --------------------------------------------------------------------------
  // Cairo
  // --------------------------------------------------------------------------

  for (s = 0; s < FOG_ARRAY_SIZE(sizes); s++)
  {
    // Cairo - FillRect
    {
      BenchmarkModule_Cairo_FillRect mod(w, h);
      totalCairo += bench(mod, sizes[s].w, sizes[s].h, quantity);
    }

    // Cairo - FillRound
    {
      BenchmarkModule_Cairo_FillRound mod(w, h);
      totalCairo += bench(mod, sizes[s].w, sizes[s].h, quantity);
    }

    // Cairo - FillPolygon
    {
      BenchmarkModule_Cairo_FillPolygon mod(w, h);
      totalCairo += bench(mod, sizes[s].w, sizes[s].h, quantity);
    }

    // Cairo - FillPattern
    {
      BenchmarkModule_Cairo_FillPattern mod(w, h);
      totalCairo += bench(mod, sizes[s].w, sizes[s].h, quantity);

      mod.type = 1;
      totalCairo += bench(mod, sizes[s].w, sizes[s].h, quantity);
    }

    // Cairo - BlitImage
    {
      BenchmarkModule_Cairo_BlitImage mod(w, h);
      totalCairo += bench(mod, sizes[s].w, sizes[s].h, quantity);
    }

    // Cairo - BlitImageAffine
    {
      BenchmarkModule_Cairo_BlitImageAffine mod(w, h);
      totalCairo += bench(mod, sizes[s].w, sizes[s].h, quantity);
    }
  }

  fog_debug("");

#endif // FOG_OS_WINDOWS

  fog_debug("Summary:");
  fog_debug("Fog (st)   - %.3f [ms]", totalFog[1].inMillisecondsF());
  fog_debug("Fog (mt)   - %.3f [ms]", totalFog[2].inMillisecondsF());
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
int main(int argc, char* argv[])
{
  fog_debug("Fog benchmark tool v0.1\n");

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
