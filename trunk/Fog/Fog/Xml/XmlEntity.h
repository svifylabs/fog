// [Fog/Xml Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_XML_XMLENTITY_H
#define _FOG_XML_XMLENTITY_H

// [Dependencies]
#include <Fog/Build/Build.h>
#include <Fog/Core/Char.h>
#include <Fog/Core/String.h>

//! @addtogroup Fog_Xml
//! @{

namespace Fog {

// ============================================================================
// [Fog::XmlEntity]
// ============================================================================

//! @brief Static class that allows to encode and decode built-in xml named
//! entities.
struct FOG_API XmlEntity
{
  //! @brief Xml entity structure, Entity <=> Character
  struct Pair
  {
    //! @brief Ascii entity name.
    char name[10];
    //! @brief Unicode representation (16 bit).
    uint16_t ch;
  };

  //! @brief Reversed pair structure.
  struct PairRev
  {
    //! @brief Character value.
    uint16_t ch;
    //! @brief Link to real entity position in @c Pair array.
    uint16_t link;
  };

  //! @brief Get all xml entity pairs defined by Fog library.
  static FOG_INLINE const Pair* pairs() { return _pairs; }

  //! @brief Get count of entity pairs returned by @c pairs() method.
  static FOG_INLINE sysuint_t pairsCount() { return _pairsCount; }

  //! @brief Decode named entity into unicode character.
  static uint16_t decode(const Char8* entityName, sysuint_t entityLength = DetectLength);
  //! @overload.
  static FOG_INLINE uint16_t decode(const String8& entityName) { return decode(entityName.cData(), entityName.length()); }

  //! @brief Decode named entity into unicode character.
  static uint16_t decode(const Char16* entityName, sysuint_t entityLength = DetectLength);
  //! @overload.
  static FOG_INLINE uint16_t decode(const String16& entityName) { return decode(entityName.cData(), entityName.length()); }

  //! @brief Decode named entity into unicode character.
  static uint16_t decode(const Char32* entityName, sysuint_t entityLength = DetectLength);
  //! @overload.
  static FOG_INLINE uint16_t decode(const String32& entityName) { return decode(entityName.cData(), entityName.length()); }

  //! @brief Encode unicode character into named entity.
  //! @note Length of @a dest must be at least 16 characters.
  static sysuint_t encode(char* dst, Char32 ch);

  static const Pair* _pairs;
  static sysuint_t _pairsCount;
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_XML_XMLENTITY_H
