// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Precompiled Headers]
#if defined(FOG_PRECOMP)
#include FOG_PRECOMP
#endif // FOG_PRECOMP

// [Dependencies]
#include <Fog/Core/Assert.h>
#include <Fog/Core/Constants.h>
#include <Fog/Core/Error.h>
#include <Fog/Core/Memory.h>
#include <Fog/Core/String.h>
#include <Fog/Core/StringUtil.h>
#include <Fog/Core/Value.h>

#include <errno.h>

#if defined(FOG_OS_WINDOWS)
#include <windows.h>
#endif // FOG_OS_WINDOWS

namespace Fog {

// ============================================================================
// [Fog::ValueData]
// ============================================================================

ValueData::ValueData(uint32_t _type)
{
  refCount.init(1);
  type = _type;
  flags = 0;
  memset(raw, 0, sizeof(raw));
  ptr = NULL;
}

ValueData::~ValueData()
{
}

void* ValueData::allocData()
{
  return Memory::alloc(sizeof(ValueData));
}

void ValueData::freeData(void* data)
{
  free(data);
}

ValueData* ValueData::ref() const
{
  refCount.inc();
  return const_cast<ValueData*>(this);
}

void ValueData::deref()
{
  if (refCount.deref())
  {
    this->~ValueData();
    freeData(this);
  }
}

// ============================================================================
// [Fog::NullValueData]
// ============================================================================

struct FOG_HIDDEN NullValueData : public ValueData
{
  NullValueData();

  virtual err_t clone(void* dst) const;
  virtual err_t getInt32(int32_t* dst) const;
  virtual err_t getInt64(int64_t* dst) const;
  virtual err_t getDouble(double* dst) const;
  virtual err_t getString(String32* dst) const;

  virtual err_t setValue(void* val);
};

NullValueData::NullValueData() : 
  ValueData(Value::TypeNull)
{
}

err_t NullValueData::clone(void* dst) const
{
  new(dst) NullValueData();
  return Error::Ok;
}

err_t NullValueData::getInt32(int32_t* dst) const
{
  *dst = 0;
  return Error::Ok;
}

err_t NullValueData::getInt64(int64_t* dst) const
{
  *dst = 0;
  return Error::Ok;
}

err_t NullValueData::getDouble(double* dst) const
{
  *dst = 0.0;
  return Error::Ok;
}

err_t NullValueData::getString(String32* dst) const
{
  dst->clear();
  return Error::Ok;
}

err_t NullValueData::setValue(void* val)
{
  return Error::InvalidFunction;
}

// ============================================================================
// [Fog::IntegerValueData]
// ============================================================================

struct FOG_HIDDEN IntegerValueData : public ValueData
{
  IntegerValueData(int64_t content);

  virtual err_t clone(void* dst) const;
  virtual err_t getInt32(int32_t* dst) const;
  virtual err_t getInt64(int64_t* dst) const;
  virtual err_t getDouble(double* dst) const;
  virtual err_t getString(String32* dst) const;

  virtual err_t setValue(void* val);
};

IntegerValueData::IntegerValueData(int64_t content) : 
  ValueData(Value::TypeInteger)
{
  i64 = content;
}

err_t IntegerValueData::clone(void* dst) const
{
  new(dst) IntegerValueData(i64);
  return Error::Ok;
}

err_t IntegerValueData::getInt32(int32_t* dst) const
{
  if (i64 < INT32_MIN)
  {
    *dst = INT32_MIN;
    return Error::Overflow;
  }
  else if (i64 > INT32_MAX)
  {
    *dst = INT32_MAX;
    return Error::Overflow;
  }
  else
  {
    *dst = (int32_t)i64;
    return Error::Ok;
  }
}

err_t IntegerValueData::getInt64(int64_t* dst) const
{
  *dst = i64;
  return Error::Ok;
}

err_t IntegerValueData::getDouble(double* dst) const
{
  *dst = (double)i64;
  return Error::Ok;
}

err_t IntegerValueData::getString(String32* dst) const
{
  dst->setInt(i64);
  return Error::Ok;
}

err_t IntegerValueData::setValue(void* val)
{
  i64 = *(int64_t *)val;
  return Error::Ok;
}

// ============================================================================
// [Fog::DoubleValueData]
// ============================================================================

struct FOG_HIDDEN DoubleValueData : public ValueData
{
  DoubleValueData(double content);

  virtual err_t clone(void* dst) const;
  virtual err_t getInt32(int32_t* dst) const;
  virtual err_t getInt64(int64_t* dst) const;
  virtual err_t getDouble(double* dst) const;
  virtual err_t getString(String32* dst) const;

  virtual err_t setValue(void* val);
};

DoubleValueData::DoubleValueData(double content) : 
  ValueData(Value::TypeDouble)
{
  d = content;
}

err_t DoubleValueData::clone(void* dst) const
{
  new(dst) DoubleValueData(d);
  return Error::Ok;
}

err_t DoubleValueData::getInt32(int32_t* dst) const
{
  if (d < INT32_MIN)
  {
    *dst = INT32_MIN;
    return Error::Overflow;
  }
  else if (d > INT32_MAX)
  {
    *dst = INT32_MAX;
    return Error::Overflow;
  }
  else
  {
    *dst = (int32_t)d;
    return Error::Ok;
  }
}

err_t DoubleValueData::getInt64(int64_t* dst) const
{
  if (d < INT64_MIN)
  {
    *dst = INT64_MIN;
    return Error::Overflow;
  }
  else if (d > INT64_MAX)
  {
    *dst = INT64_MAX;
    return Error::Overflow;
  }
  else
  {
    *dst = (int64_t)d;
    return Error::Ok;
  }
}

err_t DoubleValueData::getDouble(double* dst) const
{
  *dst = d;
  return Error::Ok;
}

err_t DoubleValueData::getString(String32* dst) const
{
  dst->setDouble(d);
  return Error::Ok;
}

err_t DoubleValueData::setValue(void* val)
{
  d = *(double *)val;
  return Error::Ok;
}

// ============================================================================
// [Fog::StringValueData]
// ============================================================================

struct FOG_HIDDEN StringValueData : public ValueData
{
  StringValueData(const String32& content);
  ~StringValueData();

  virtual err_t clone(void* dst) const;
  virtual err_t getInt32(int32_t* dst) const;
  virtual err_t getInt64(int64_t* dst) const;
  virtual err_t getDouble(double* dst) const;
  virtual err_t getString(String32* dst) const;

  virtual err_t setValue(void* val);

  FOG_INLINE String32* str() const { return (String32*)raw; }
};

StringValueData::StringValueData(const String32& content) : 
  ValueData(Value::TypeString)
{
  new (str()) String32(content);
}

StringValueData::~StringValueData()
{
  str()->~String32();  
}

err_t StringValueData::clone(void* dst) const
{
  new(dst) StringValueData(*str());
  return Error::Ok;
}

err_t StringValueData::getInt32(int32_t* dst) const
{
  return str()->atoi32(dst);
}

err_t StringValueData::getInt64(int64_t* dst) const
{
  return str()->atoi64(dst);
}

err_t StringValueData::getDouble(double* dst) const
{
  return str()->atod(dst);
}

err_t StringValueData::getString(String32* dst) const
{
  dst->set(*str());
  return Error::Ok;
}

err_t StringValueData::setValue(void* val)
{
  str()->set(*(String32*)val);
  return Error::Ok;
}

// ============================================================================
// [Fog::Value]
// ============================================================================

static Static<NullValueData> sharedNullData;
ValueData* Value::sharedNull;

Value::Value()
{
  _d = sharedNull;
  _d->refCount.inc();
}

Value::Value(const Value& other)
{
  _d = other._d->ref();
}

Value::~Value()
{
  _d->deref();
}

Value& Value::operator=(const Value& other)
{
  AtomicBase::ptr_setXchg(&_d, other._d->ref())->deref();
  return *this;
}

Value Value::null()
{
  return Value();
}

Value Value::fromBool(bool val)
{
  return Value(
    new(ValueData::allocData()) 
      IntegerValueData(static_cast<int64_t>(val)));
}

Value Value::fromInt32(int32_t val)
{
  return Value(
    new(ValueData::allocData()) 
      IntegerValueData(static_cast<int64_t>(val)));
}

Value Value::fromInt64(int64_t val)
{
  return Value(
    new(ValueData::allocData()) 
      IntegerValueData(val));
}

Value Value::fromDouble(double val)
{
  return Value(
    new(ValueData::allocData()) 
      DoubleValueData(val));
}

Value Value::fromString(const String32& val)
{
  return Value(
    new(ValueData::allocData()) 
      StringValueData(val));
}

Value Value::fromErrno()
{
  return Value(
    new(ValueData::allocData()) 
      IntegerValueData(errno));
}

#if defined(FOG_OS_WINDOWS)
Value Value::fromWinLastError()
{
  return Value(
    new(ValueData::allocData()) 
      IntegerValueData(GetLastError()));
}
#endif // FOG_OS_WINDOWS

err_t Value::detach()
{
  if (isDetached()) return Error::Ok;

  ValueData* d = (ValueData*)ValueData::allocData();
  if (!d) return Error::OutOfMemory;

  _d->clone(d);
  AtomicBase::ptr_setXchg(&_d, d)->deref();
  return Error::Ok;
}

err_t Value::setNull()
{
  sharedNull->refCount.inc();
  AtomicBase::ptr_setXchg(&_d, sharedNull)->deref();
  return Error::Ok;
}

err_t Value::setInt32(int32_t val)
{
  return setInt64(static_cast<int64_t>(val));
}

err_t Value::setInt64(int64_t val)
{
  if (isDetached())
  {
    if (!isInteger())
    {
      _d->~ValueData();
      new(_d) IntegerValueData(val);
    }
    else
    {
      _d->i64 = val;
    }
  }
  else
  {
    void* p = ValueData::allocData();
    if (!p) return Error::OutOfMemory;

    AtomicBase::ptr_setXchg(&_d, 
      reinterpret_cast<ValueData*>(new(p) IntegerValueData(val)))->deref();
  }
  return Error::Ok;
}

err_t Value::setDouble(double val)
{
  if (isDetached())
  {
    if (!isDouble())
    {
      _d->~ValueData();
      new(_d) DoubleValueData(val);
    }
    else
    {
      _d->d = val;
    }
  }
  else
  {
    void* p = ValueData::allocData();
    if (!p) return Error::OutOfMemory;

    AtomicBase::ptr_setXchg(&_d,
      reinterpret_cast<ValueData*>(new(p) DoubleValueData(val)))->deref();
  }
  return Error::Ok;
}

err_t Value::setString(const String32& val)
{
  if (isDetached())
  {
    if (!isString())
    {
      _d->~ValueData();
      new(_d) StringValueData(val);
    }
    else
    {
      reinterpret_cast<StringValueData*>(_d)->str()->set(val);
    }
  }
  else
  {
    void* p = ValueData::allocData();
    if (!p) return Error::OutOfMemory;

    AtomicBase::ptr_setXchg(&_d,
      reinterpret_cast<ValueData*>(new(p) StringValueData(val)))->deref();
  }
  return Error::Ok;
}

} // Fog namespace

// ============================================================================
// [Library Initializers]
// ============================================================================

FOG_INIT_DECLARE err_t fog_value_init(void)
{
  using namespace Fog;

  sharedNullData.init();
  Value::sharedNull = sharedNullData.instancep();

  return Error::Ok;
}

FOG_INIT_DECLARE void fog_value_shutdown(void)
{
  using namespace Fog;

  sharedNullData.destroy();
}
