// [Fog/Graphics Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_GRAPHICS_FONT_H
#define _FOG_GRAPHICS_FONT_H

// [Dependencies]
#include <Fog/Core/Hash.h>
#include <Fog/Core/Lock.h>
#include <Fog/Core/Misc.h>
#include <Fog/Core/RefData.h>
#include <Fog/Core/Static.h>
#include <Fog/Core/String.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/Graphics/Image.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// forward declarations
struct FontEngine;

// [Fog::TextWidth]

/*! @brief Text width retrieved by @c Fog::Font::getTextWidth() like functions. */
struct TextWidth
{
  int beginWidth;
  int advance;
  int endWidth;
};

// [Fog::Glyph]

struct FOG_API Glyph
{
  // [Data]

  struct FOG_API Data : public Fog::RefDataSimple<Data>
  {
    Data();
    ~Data();

    /*! @brief Glyph image, supported formats are only A8 and XRGB32 for now. */
    Image image;
    /*! @brief X offset for rendering glyph image. */
    int offsetX;
    /*! @brief Y offset for rendering glyph image. */
    int offsetY;
    /*! @brief Begin width. */
    int beginWidth;
    /*! @brief End width. */
    int endWidth;
    /*! @brief Glyph advance. */
    int advance;

    FOG_INLINE Data* ref() { return REF_ALWAYS(); }
    FOG_INLINE void deref() { DEREF_INLINE(); }
    FOG_INLINE void free() { delete this; }

  private:
    FOG_DISABLE_COPY(Data)
  };

  // [Members]

  FOG_DECLARE_D(Data)

  static Fog::Static<Data> sharedNull;

  // [Construction / Destruction]

  Glyph();
  Glyph(const Glyph& other);
  Glyph(Data* d);
  ~Glyph();

  // [Implicit Sharing]

  FOG_INLINE sysuint_t refCount() const { return _d->refCount.get(); }
  FOG_INLINE bool isNull() const { return _d == sharedNull.instancep(); }

  // [Getters]

  FOG_INLINE const Fog::Image& image() const { return _d->image; }
  FOG_INLINE int offsetX() const { return _d->offsetX; }
  FOG_INLINE int offsetY() const { return _d->offsetY; }
  FOG_INLINE int beginWidth() const { return _d->beginWidth; }
  FOG_INLINE int endWidth() const { return _d->endWidth; }
  FOG_INLINE int advance() const { return _d->advance; }

  // [Operator Overload]

  Glyph& operator=(const Glyph& other);
};

// Inlined for maximum performance
FOG_INLINE Glyph::Glyph() : _d(sharedNull.instancep()->ref()) 
{}

FOG_INLINE Glyph::Glyph(const Glyph& other) : _d(other._d->ref()) 
{}

FOG_INLINE Glyph::Glyph(Data* d) : _d(d) 
{}

FOG_INLINE Glyph::~Glyph()
{ _d->deref(); }

FOG_INLINE Glyph& Glyph::operator=(const Glyph& other)
{ Fog::AtomicBase::ptr_setXchg(&_d, other._d->ref())->deref(); return *this; }

// [Fog::GlyphCache]

/*!
  @brief Glyph cache.

  Glyphs are stored in sparse Nx256 array. First pointer points to second
  array where are stored real glyphs. This cache is per face per attribute
  and access is very very fast.

  Address for glyph 'ch' is row[ch >> 8][ch & 255]
*/
struct FOG_API GlyphCache
{
  typedef Glyph::Data* Entity;

  GlyphCache();
  ~GlyphCache();

  bool set(uint32_t uc, Entity data);

  FOG_INLINE Entity get(uint32_t uc) const
  {
    Entity* row;
    uint32_t ucRow = uc >> 8;

    return ucRow < _count && (row = _rows[ucRow]) 
      ? row[uc & 0xFF]
      : 0;
  }

  void free();

private:
  // Lock. Glyph cache is always locked when you are working with it. This
  // allows very fast access when retrieving multiple glyphs from it. Typical
  // font routine should lock cache twice - first to get cached glyphs and 
  // second lock to store newly created glyphs.
  // Fog::Lock lock;
  // 
  // NOTE: Cache moved to FontFace, because it has no sense to duplicate it
  // here, because it's really needed in FontFace itself. This also means
  // that lock is locked only once when accessing multiple glyphs (This is
  // really optimized!)

  // Pointers to glyphs
  Entity** _rows;
  // Count of first rows pointer. Initial value is 0
  sysuint_t _count;

private:
  FOG_DISABLE_COPY(GlyphCache)
};

// [Fog::FontMetrics]

struct FontMetrics
{
  uint32_t size;
  uint32_t ascent;
  uint32_t descent;
  uint32_t averageWidth;
  uint32_t maximumWidth;
  uint32_t height;
};

// [Fog::FontAttributes]

struct FontAttributes
{
  uint8_t bold;
  uint8_t italic;
  uint8_t strike;
  uint8_t underline;
};

// [Fog::FontFace]

/*! @brief Font face. */
struct FOG_API FontFace : public Fog::RefDataSimple<FontFace>
{
  Fog::String32 family;
  FontMetrics metrics;
  FontAttributes attributes;

  FontFace();
  virtual ~FontFace();

  virtual FontFace* ref();
  virtual void deref() = 0;
  FOG_INLINE void free() { delete this; }

  virtual void getTextWidth(
    // in
    const Fog::Char32* str, sysuint_t length,
    // out
    TextWidth* textWidth) = 0;

  virtual void getGlyphs(
    // in
    const Fog::Char32* str, sysuint_t length,
    // out
    Glyph* target,
    TextWidth* textWidth) = 0;

private:
  FOG_DISABLE_COPY(FontFace)
};

// [Fog::FontFaceCache]

struct FOG_API FontFaceCache
{
  struct Entry
  {
    /*! @brief Font family name. */
    Fog::String32 family;
    /*! @brief Font size. */
    uint32_t size;
    /*! @brief Font attributes. */
    FontAttributes attributes;
  };

  Fog::Lock lock;
};

// [Fog::Font]

struct FOG_API Font
{
  // [Data]

  struct FOG_API Data : public Fog::RefDataSimple<Data>
  {
    FontFace* face;

    Data();
    ~Data();

    static Data* copy(Data* d);

    FOG_INLINE Data* ref() { return REF_ALWAYS(); }
    FOG_INLINE void deref() { DEREF_INLINE(); }
    FOG_INLINE void free() { delete this; }

  private:
    FOG_DISABLE_COPY(Data)
  };

  static Data* sharedNull;

  // [Members]

  FOG_DECLARE_D(Data)

  // [Construction and destruction]

  Font();
  Font(Data* d);
  Font(const Font& other);
  ~Font();

  // [Implicit sharing and basic flags]

  /*! @copydoc Doxygen::Implicit::refCount(). */
  FOG_INLINE ulong refCount() const { return _d->refCount.get(); }
  /*! @copydoc Doxygen::Implicit::isDetached(). */
  FOG_INLINE bool isDetached() const { return refCount() == 1; }
  /*! @copydoc Doxygen::Implicit::detach(). */
  FOG_INLINE void detach() { if (!isDetached()) _detach(); }
  /*! @copydoc Doxygen::Implicit::_detach(). */
  void _detach();
  /*! @copydoc Doxygen::Implicit::free(). */
  void free();

  // [Font family and metrics]

  FOG_INLINE const Fog::String32& family() const { return _d->face->family; }
  FOG_INLINE const FontMetrics& metrics() const { return _d->face->metrics; }
  FOG_INLINE uint32_t size() const { return metrics().size; }
  FOG_INLINE uint32_t ascent() const { return metrics().ascent; }
  FOG_INLINE uint32_t descent() const { return metrics().descent; }
  FOG_INLINE uint32_t averageWidth() const { return metrics().averageWidth; }
  FOG_INLINE uint32_t maximumWidth() const { return metrics().maximumWidth; }
  FOG_INLINE uint32_t height() const { return metrics().height; }

  FOG_INLINE const FontAttributes& attributes() const { return _d->face->attributes; }
  FOG_INLINE bool isBold() const { return attributes().bold != 0; }
  FOG_INLINE bool isItalic() const { return attributes().italic != 0; }
  FOG_INLINE bool isStrike() const { return attributes().strike != 0; }
  FOG_INLINE bool isUnderline() const { return attributes().underline != 0; }

  bool setFamily(const Fog::String32& family);
  bool setFamily(const Fog::String32& family, uint32_t size);
  bool setFamily(const Fog::String32& family, uint32_t size, const FontAttributes& attributes);

  bool setSize(uint32_t size);
  bool setAttributes(const FontAttributes& a);
  bool setBold(bool val);
  bool setItalic(bool val);
  bool setStrike(bool val);
  bool setUnderline(bool val);

  // [Set]

  const Font& set(const Fog::Font& other);

  // [Text functions]

  void getTextWidth(
    const Fog::String32& text, 
    TextWidth* textWidth) const;

  void getTextWidth(
    const Fog::Char32* text, sysuint_t length, 
    TextWidth* textWidth) const;

  // [Glyph functions]

  void getGlyphs(
    const Fog::Char32* str, sysuint_t lrngth,
    Glyph* glyphs, Fog::TextWidth* textWidth) const;

  // [Overloaded Operators]
  FOG_INLINE const Font& operator=(const Font& other)
  { return set(other); }

  // [Font path management functions]

  static bool addFontPath(const Fog::String32& path);
  static void addFontPaths(const Fog::Sequence<Fog::String32>& paths);
  static bool removeFontPath(const Fog::String32& path);
  static bool hasFontPath(const Fog::String32& path);
  static bool findFontFile(const Fog::String32& fileName, Fog::String32& dest);
  static Fog::Vector<Fog::String32> fontPaths();

  // [Font list management]
  static Fog::Vector<Fog::String32> fontList();

  // [Engine]
  static FontEngine* _engine;
};

/*!
  @brief Font engine.
*/
struct FOG_API FontEngine
{
  FontEngine(const Fog::String32& name);
  virtual ~FontEngine();

  virtual Fog::Vector<Fog::String32> getFonts() = 0;

  virtual FontFace* getDefaultFace() = 0;

  virtual FontFace* getFace(
    const Fog::String32& family, uint32_t size, 
    const FontAttributes& attributes) = 0;

  FOG_INLINE const Fog::String32& name() const
  { return _name; }

protected:
  Fog::String32 _name;

private:
  FOG_DISABLE_COPY(FontEngine)
};

// ----------------------------------------------------------------------------
// Tools
// ----------------------------------------------------------------------------

// [Fog::Glyphs]

/*!
  @brief Class used to retrieve glyphs from @c Fog::Font.
*/
template<sysuint_t N>
struct Glyphs
{
  sysuint_t _count;
  int _advance;
  Fog::MemoryBuffer<N * sizeof(Fog::Glyph)> _buffer;

  FOG_INLINE Glyphs() : 
    _count(0),
    _advance(0)
  {
  }

  FOG_INLINE ~Glyphs()
  {
    free();
  }

  FOG_INLINE Glyph* glyphs() const
  {
    return (Glyph*)_buffer.mem();
  }

  FOG_INLINE sysuint_t count() const
  {
    return _count;
  }

  FOG_INLINE int advance() const
  {
    return _advance;
  }

  int advance(sysuint_t index, sysuint_t length)
  {
    int result = 0;
    sysuint_t end = index + length;

    FOG_ASSERT(index <= end);
    FOG_ASSERT(end <= count());

    Glyph* g = glyphs();
    for (sysuint_t i = index; i != end; i++) result += g[i].advance();

    return result;
  }

  bool getGlyphs(const Font& font, const Fog::Char32* str, sysuint_t length)
  {
    FOG_ASSERT(!glyphs());

    if (!_buffer.alloc(sizeof(Glyph) * length)) return false;

    _count = count;
    return font._d->face->getGlyphs(glyphs(), str, length, &_advance);
  }

  bool getGlyphs(const Font& font, const Fog::String32& str)
  {
    return getGlyphs(font, str.cData(), str.length());
  }

  void free()
  {
    Glyph* g = glyphs();
    if (!g) return;

    for (sysuint_t i = 0; i != _count; i++) g[i]._d->deref();

    _count = 0;
    _advance = 0;
    _buffer.free();
  }
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_GRAPHICS_FONT_H
