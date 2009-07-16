// [Fog/Core Library - C++ API]
//
// [Licence]
// MIT, See COPYING file in package

// [Precompiled headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Properties.h>

namespace Fog {

// ============================================================================
// [Fog::PropertiesData]
// ============================================================================

PropertiesData::PropertiesData() :
  _parent(NULL),
  _bucketsData(NULL),
  _bucketsCount(0),
  _offset(-1)
{

}

PropertiesData::~PropertiesData()
{
  if (_bucketsData) destroy();
}

err_t PropertiesData::init(PropertiesData* parent, const Vector<String32>& properties)
{
  FOG_ASSERT_X(!isInitialized(), "Fog::PropertiesData::init() - Already initialized.");

  int propertiesCount = (int)properties.length();
  int bucketsCount = propertiesCount;
  int offset = parent ? parent->_last : 0;

  _parent = parent;
  if (parent) _properties = _parent->_properties;
  _properties.append(properties);
  _properties.squeeze();

  // Align bucketsCount to be power of 2, minimum value is 8. This allows us
  // to use AND instead of MOD operators in calculating bucket index from hash.
  {
    int t = 8;
    while (bucketsCount > t) t <<= 1;
    bucketsCount = t;
  }

  // One allocation for _buckets and all data. Individual buckets are created
  // using placement new() operator.
  sysuint_t bucketsDataSize = bucketsCount * sizeof(Bucket*);
  sysuint_t propertiesDataSize = propertiesCount * sizeof(Bucket);

  Bucket** bucketsData = (Bucket**)Memory::alloc(bucketsDataSize + propertiesDataSize);
  if (!bucketsData) return Error::OutOfMemory;

  Memory::zero(bucketsData, bucketsDataSize);
  uint8_t* p = (uint8_t*)bucketsData + bucketsDataSize;

  for (int i = 0; i < propertiesCount; i++)
  {
    const String32& name = properties.cAt(i);
    uint32_t hash = name.toHashCode();
    uint32_t hidx = hash & (bucketsCount-1);

    Bucket* b = new(p) Bucket(name, offset + i);
    p += sizeof(Bucket);

    if (bucketsData[hidx] == NULL)
    {
      bucketsData[hidx] = b;
    }
    else
    {
      Bucket* t = bucketsData[hidx];
      while (t->next) t = t->next;
      t->next = b;
    }
  }

  _bucketsData = bucketsData;
  _bucketsCount = bucketsCount;
  _offset = offset;
  _last = offset + propertiesCount;

  return Error::Ok;
}

void PropertiesData::destroy()
{
  if (!_bucketsData) return;

  for (int i = 0; i < _bucketsCount; i++)
  {
    Bucket* bucket = _bucketsData[i];

    while (bucket)
    {
      Bucket* next = bucket->next;
      bucket->~Bucket();
      bucket = next;
    }
  }

  Memory::free(_bucketsData);

  _parent = NULL;
  _bucketsData = NULL;
  _bucketsCount = 0;
  _offset = -1;
}

int PropertiesData::find(const String32& name) const
{
  return _findInline(name);
}

// ============================================================================
// [Fog::PropertiesContainer]
// ============================================================================

PropertiesContainer::PropertiesContainer()
{
}

Vector<String32> PropertiesContainer::propertiesList() const
{
  const PropertiesData* pdata = propertiesData();
  return pdata->_properties;
}

int PropertiesContainer::propertyInfo(const String32& name) const
{
  const PropertiesData* pdata = propertiesData();
  int id = pdata->_findInline(name);
  return propertyInfo(id);
}

err_t PropertiesContainer::getProperty(const String32& name, Value& value) const
{
  const PropertiesData* pdata = propertiesData();
  int id = pdata->_findInline(name);
  return getProperty(id, value);
}

err_t PropertiesContainer::setProperty(const String32& name, const Value& value)
{
  const PropertiesData* pdata = propertiesData();
  int id = pdata->_findInline(name);
  return setProperty(id, value);
}

int PropertiesContainer::propertyToId(const String32& name) const
{
  const PropertiesData* pdata = propertiesData();
  return pdata->_findInline(name);
}

const PropertiesData* PropertiesContainer::propertiesData() const
{
  // Default is to have no properties.
  return NULL;
}

int PropertiesContainer::propertyInfo(int id) const
{
  const PropertiesData* pdata = propertiesData();
  return (id >= 0 && id < pdata->_last) ? Exists : NotExists;
}

err_t PropertiesContainer::getProperty(int id, const Value& value) const
{
  return Error::InvalidPropertyName;
}

err_t PropertiesContainer::setProperty(int id, const Value& value)
{
  return Error::InvalidPropertyName;
}

Value PropertiesContainer::property(const String32& name) const
{
  const PropertiesData* pdata = propertiesData();
  int id = pdata->_findInline(name);

  Value result;
  getProperty(id, result);
  return result;
}

Value PropertiesContainer::property(int id) const
{
  Value result;
  getProperty(id, result);
  return result;
}

} // Fog namespace
