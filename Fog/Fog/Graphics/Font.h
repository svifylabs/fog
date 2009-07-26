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
#include <Fog/Core/Static.h>
#include <Fog/Core/String.h>
#include <Fog/Graphics/Constants.h>
#include <Fog/Graphics/Geometry.h>
#include <Fog/Graphics/Glyph.h>
#include <Fog/Graphics/GlyphCache.h>
#include <Fog/Graphics/GlyphSet.h>

//! @addtogroup Fog_Graphics
//! @{

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct FontEngine;
struct Path;

// ============================================================================
// [Fog::TextWidth]
// ============================================================================

//! @brief Text width retrieved by @c Fog::Font::getTextWidth() like functions.
struct TextWidth
{
  int beginWidth;
  int advance;
  int endWidth;
};

// ============================================================================
// [Fog::FontMetrics]
// ============================================================================

struct FontMetrics
{
  uint32_t size;
  uint32_t ascent;
  uint32_t descent;
  uint32_t averageWidth;
  uint32_t maximumWidth;
  uint32_t height;
};

// ============================================================================
// [Fog::FontAttributes]
// ============================================================================

union FontAttributes
{
  struct
  {
    uint8_t bold;
    uint8_t italic;
    uint8_t strike;
    uint8_t underline;
  };
  uint32_t value;
};

// ============================================================================
// [Fog::FontFace]
// ============================================================================

//! @brief Font face.
struct FOG_API FontFace
{
  // [Construction / Destruction]

  FontFace();
  virtual ~FontFace();

  // [Ref / Deref]

  virtual FontFace* ref();
  virtual void deref();

  // [Abstract]

  virtual err_t getGlyphs(const Char32* str, sysuint_t length, GlyphSet& glyphSet) = 0;
  virtual err_t getTextWidth(const Char32* str, sysuint_t length, TextWidth* textWidth) = 0;
  virtual err_t getPath(const Char32* str, sysuint_t length, Path& dst) = 0;
  
  // [Members]

  //! @brief Reference count.
  mutable Atomic<sysuint_t> refCount;

  String32 family;
  FontMetrics metrics;
  FontAttributes attributes;

private:
  FOG_DISABLE_COPY(FontFace)
};

// ============================================================================
// [Fog::FontCache]
// ============================================================================

struct FOG_API FontCache
{
  // [Construction / Destruction]

  FontCache();
  ~FontCache();

  // [Entry]

  struct Entry
  {
    // [Construction / Destruction]

    FOG_INLINE Entry()
    {
      size = 0;
      attributes.value = 0;
    }
    
    FOG_INLINE Entry(const Entry& other) :
      family(other.family)
    {
      size = other.size;
      attributes.value = other.attributes.value;
    }
    
    FOG_INLINE Entry(const String32& _family, uint32_t _size, const FontAttributes& _attributes) :
      family(_family)
    {
      size = _size;
      attributes.value = _attributes.value;
    }

    FOG_INLINE ~Entry() {}

    // [Methods]

    FOG_INLINE uint32_t toHashCode() const
    {
      return family.toHashCode() ^ (size * 133) ^ attributes.value;
    }

    // [Operator Overload]

    FOG_INLINE Entry& operator=(const Entry& other)
    {
      family = other.family;
      size = other.size;
      attributes.value = other.attributes.value;
    }

    FOG_INLINE bool operator==(const Entry& other) const
    {
      return family == other.family &&
             size == other.size &&
             attributes.value == other.attributes.value;
    }

    FOG_INLINE bool operator!=(const Entry& other) const
    {
      return family != other.family ||
             size != other.size ||
             attributes.value != other.attributes.value;
    }

    // [Members]

    //! @brief Font family name.
    String32 family;
    //! @brief Font size.
    uint32_t size;
    //! @brief Font attributes.
    FontAttributes attributes;
  };

  // [Methods]

  FontFace* getFace(const String32& family, uint32_t size, const FontAttributes& attrs);
  err_t putFace(FontFace* face);

  void deleteAll();

  // [Members]

  Lock _lock;
  Hash<Entry, FontFace*> _cache;

private:
  FOG_DISABLE_COPY(FontCache)
};

// ============================================================================
// [Fog::Font]
// ============================================================================

struct FOG_API Font
{
  // [Data]

  struct FOG_API Data
  {
    // [Construction / Destruction]

    Data();
    ~Data();

    // [Ref / Deref]

    Data* ref() const;
    void deref();

    static Data* copy(Data* d);

    // [Members]

    mutable Atomic<sysuint_t> refCount;
    FontFace* face;

  private:
    FOG_DISABLE_COPY(Data)
  };

  static Data* sharedNull;

  // [Construction and destruction]

  Font();
  Font(const Font& other);
  explicit Font(Data* d);
  ~Font();

  // [Implicit sharing and basic flags]

  //! @copydoc Doxygen::Implicit::refCount().
  FOG_INLINE sysuint_t refCount() const { return _d->refCount.get(); }
  //! @copydoc Doxygen::Implicit::isDetached().
  FOG_INLINE bool isDetached() const { return refCount() == 1; }
  //! @copydoc Doxygen::Implicit::detach().
  FOG_INLINE err_t detach() { return !isDetached() ? _detach() : (err_t)Error::Ok; }
  //! @copydoc Doxygen::Implicit::_detach().
  err_t _detach();
  //! @copydoc Doxygen::Implicit::free().
  void free();

  // [Font family and metrics]

  FOG_INLINE const String32& family() const { return _d->face->family; }
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

  err_t setFamily(const String32& family);
  err_t setFamily(const String32& family, uint32_t size);
  err_t setFamily(const String32& family, uint32_t size, const FontAttributes& attributes);

  err_t setSize(uint32_t size);
  err_t setAttributes(const FontAttributes& a);
  err_t setBold(bool val);
  err_t setItalic(bool val);
  err_t setStrike(bool val);
  err_t setUnderline(bool val);

  // [Set]

  err_t set(const Font& other);

  // [Face Methods]

  err_t getTextWidth(const String32& str, TextWidth* textWidth) const;
  err_t getTextWidth(const Char32* str, sysuint_t length, TextWidth* textWidth) const;
  err_t getGlyphs(const Char32* str, sysuint_t length, GlyphSet& glyphSet) const;
  err_t getPath(const Char32* str, sysuint_t length, Path& dst) const;

  // [Overloaded Operators]

  FOG_INLINE const Font& operator=(const Font& other) { set(other); return *this; }

  // [Font path management functions]

  static bool addFontPath(const String32& path);
  static void addFontPaths(const Sequence<String32>& paths);
  static bool removeFontPath(const String32& path);
  static bool hasFontPath(const String32& path);
  static bool findFontFile(const String32& fileName, String32& dest);
  static Vector<String32> fontPaths();

  // [Font list management]
  static Vector<String32> fontList();

  // [Cache]
  static FontCache* _cache;

  // [Engine]
  static FontEngine* _engine;

  // [Members]

  FOG_DECLARE_D(Data)
};

//! @brief Font engine.
struct FOG_API FontEngine
{
  // [Construction / Destruction]

  FontEngine(const String32& name);
  virtual ~FontEngine();

  // [Abstract]

  virtual Vector<String32> getFonts() = 0;

  virtual FontFace* getDefaultFace() = 0;

  virtual FontFace* cachedFace(
    const String32& family, uint32_t size, 
    const FontAttributes& attributes);

  virtual FontFace* createFace(
    const String32& family, uint32_t size, 
    const FontAttributes& attributes) = 0;

  FOG_INLINE const String32& name() const { return _name; }

  // [Members]

protected:
  String32 _name;

private:
  FOG_DISABLE_COPY(FontEngine)
};

} // Fog namespace

//! @}

// ============================================================================
// [Fog::TypeInfo<>]
// ============================================================================

FOG_DECLARE_TYPEINFO(Fog::Font, Fog::MoveableType)
FOG_DECLARE_TYPEINFO(Fog::FontAttributes, Fog::PrimitiveType)
FOG_DECLARE_TYPEINFO(Fog::FontMetrics, Fog::PrimitiveType)
FOG_DECLARE_TYPEINFO(Fog::TextWidth, Fog::PrimitiveType)

// [Guard]
#endif // _FOG_GRAPHICS_FONT_H
