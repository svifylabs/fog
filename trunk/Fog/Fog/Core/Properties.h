// [Fog/Core Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Guard]
#ifndef _FOG_CORE_PROPERTIES_H
#define _FOG_CORE_PROPERTIES_H

// [Dependencies]
#include <Fog/Core/String.h>
#include <Fog/Core/Value.h>
#include <Fog/Core/Vector.h>

//! @addtogroup Fog_Core
//! @{

namespace Fog {

// ============================================================================
// [Forward Declarations]
// ============================================================================

struct PropertiesData;
struct PropertiesContainer;

// ============================================================================
// [Fog::IProperties]
// ============================================================================

//! @brief Interface that allows to use properties.
//!
//! If you are looking for easy way to add compatible properties to Fog into
//! your class consider to use @c PropertiesContainer class instead. Anyway, this
//! interface can be used to create our own properties platform not based on
//! Fog library (but compatible).
struct FOG_API IProperties
{
  // [Virtual Methods]

  //! @brief Get list of properties.
  virtual Vector<String32> propertiesList() const = 0;
  //! @brief Get info about property @a name, see @c IProperties::InfoFlags.
  virtual int propertyInfo(const String32& name) const = 0;

  //! @brief Get property @a name to @a value.
  virtual err_t getProperty(const String32& name, Value& value) const = 0;
  //! @brief Set property @a name from @a value.
  virtual err_t setProperty(const String32& name, const Value& value) = 0;

  // [Info Flags]

  //! @brief Information flags that can returned by @c propertyInfo() method.
  //!
  //! If class instance supports properties, always is returned non-zero value.
  //! You can use @c NotExists constant to test property presence. All other
  //! values are bit masks and can be combined.
  //!
  //! If property exists, return value always contains @c Exists bit.
  enum InfoFlags
  {
    //! @brief Property name / id is invalid (this just means no flags).
    NotExists = 0,
    //! @brief Property name / id exists.
    Exists = 0x01,
    //! @brief Property is read-only.
    ReadOnly = 0x02,
    //! @brief Property is not used / forbidden in current state.
    Inactive = 0x04
  };
};

// ============================================================================
// [Fog::PropertiesData]
// ============================================================================

struct FOG_API PropertiesData
{
  // [Constuction / Destruction]

  PropertiesData();
  ~PropertiesData();

  // [Embedded Hash Table]

  struct Bucket
  {
    FOG_INLINE Bucket(const String32& name, int id) : _next(NULL), _name(name), _id(id) {}
    FOG_INLINE ~Bucket() {}

    Bucket* _next;
    String32 _name;
    int _id;
  };

  // [Init / Destroy]

  err_t init(PropertiesData* parent, const Vector<String32>& properties);
  void destroy();

  // [Getters]

  FOG_INLINE bool isInitialized() { return _offset >= 0; }

  FOG_INLINE PropertiesData* parent() const { return _parent; }
  FOG_INLINE const Vector<String32>& properties() const { return _properties; }
  FOG_INLINE int offset() const { return _offset; }

  // [Find]

  int find(const String32& name) const;

  FOG_INLINE int _findInline(const String32& name) const
  {
    const PropertiesData* pdata = this;
    Bucket* bucket;
    uint32_t hash = name.toHashCode();

    do {
      bucket = pdata->_bucketsData[hash & (pdata->_bucketsCount - 1)];
      while (bucket)
      {
        if (bucket->_name == name) return bucket->_id;
        bucket = bucket->_next;
      }
    } while (pdata = pdata->_parent);
    return -1;
  }

  // [Members]
private:

  //! @brief Link to parent properties class list.
  PropertiesData* _parent;
  //! @brief Vector that contains properties for this class and all parent
  //! classes.
  Vector<String32> _properties;

  //! @brief Embedded hash table buckets data.
  Bucket** _bucketsData;
  //! @brief Count of buckets, always power of two.
  int _bucketsCount;

  //! @brief Property offset relative to parent.
  //!
  //! Offset is _parent->_last or zero if no parent exists"
  int _offset;
  //! @brief First invalid ID.
  int _last;

  FOG_DISABLE_COPY(PropertiesData)

  friend struct PropertiesContainer;
};

// ============================================================================
// [Fog::PropertiesContainer]
// ============================================================================

// Trick to allow DECLARE_PROPERTIES_CONTAINER() to base on PropertiesContainer.
template <typename T>
struct _PropertiesContainerBase
{
  static FOG_INLINE PropertiesData* propertiesData() { return T::_propertiesData.instancep(); }
};

template <>
struct _PropertiesContainerBase<PropertiesContainer>
{
  static FOG_INLINE PropertiesData* propertiesData() { return NULL; }
};

#define DECLARE_PROPERTIES_CONTAINER() \
public: \
  static Fog::Static<Fog::PropertiesData> _propertiesData; \
  \
  virtual const Fog::PropertiesData* propertiesData() const { return _propertiesData.instancep(); }

#define INIT_PROPERTIES_CONTAINER(__class__, __base__, __properties__) \
  __class__::_propertiesData.init(); \
  __class__::_propertiesData.instancep()->init(_PropertiesContainerBase<__base__>::propertiesData(), __properties__)

#define DESTROY_PROPERTIES_CONTAINER(__class__) \
  __class__::_propertiesData.destroy()

//! @brief Helper class used to work with properties.
//!
//! This class allows you to use property IDs instead of strings.
struct FOG_API PropertiesContainer : public IProperties
{
  // [Construction / Destruction]

  PropertiesContainer();

  // [Virtual Methods]

  virtual Vector<String32> propertiesList() const;
  virtual int propertyInfo(const String32& name) const;

  virtual err_t getProperty(const String32& name, Value& value) const;
  virtual err_t setProperty(const String32& name, const Value& value);

  //! @brief Convert property name to property id for current class.
  //! @return Property id or -1 if failed.
  virtual int propertyToId(const String32& name) const;

  //! @brief Get property data instance (one per class).
  virtual const PropertiesData* propertiesData() const;
  //! @brief Get info about property @a name, see @c IProperties::InfoFlags.
  virtual int propertyInfo(int id) const;

  //! @brief Get property @a id to @a value.
  virtual err_t getProperty(int id, const Value& value) const;
  //! @brief Set property @a id from @a value.
  virtual err_t setProperty(int id, const Value& value);

  // [Member Methods]

  //! @brief Easy way to get property @a name, no error detection.
  Value property(const String32& name) const;
  //! @brief Easy way to get property @a id, no error detection.
  Value property(int id) const;
};

} // Fog namespace

//! @}

// [Guard]
#endif // _FOG_CORE_PROPERTIES_H
