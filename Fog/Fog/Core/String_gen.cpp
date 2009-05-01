// [Fog/Core Library - C++ API]
//
// [Licence] 
// MIT, See COPYING file in package

// [Generator]
#if defined(__G_GENERATE)

#if __G_SIZE == 1
# define __G_STRING String8
# define __G_STRING_NAME "Fog::String8"
# define __G_STRING_STUB Stub8
# define __G_STRING_UTFX None
# define __G_STRING_STUBX None
# define __G_STRINGFILTER StringFilter8
# define __G_STRINGMATCHER StringMatcher8
# define __G_TEMPORARYSTRING TemporaryString8
# define __G_CHAR Char8
# define __G_FRIEND char
#elif __G_SIZE == 2
# define __G_STRING String16
# define __G_STRING_NAME "Fog::String16"
# define __G_STRING_STUB StubUtf16
# define __G_STRING_UTFX String32
# define __G_STRING_STUBX StubUtf32
# define __G_STRINGFILTER StringFilter16
# define __G_STRINGMATCHER StringMatcher16
# define __G_TEMPORARYSTRING TemporaryString16
# define __G_CHAR Char16
# define __G_FRIEND uint16_t
#else
# define __G_STRING String32
# define __G_STRING_NAME "Fog::String32"
# define __G_STRING_STUB StubUtf32
# define __G_STRING_UTFX String16
# define __G_STRING_STUBX StubUtf16
# define __G_STRINGFILTER StringFilter32
# define __G_STRINGMATCHER StringMatcher32
# define __G_TEMPORARYSTRING TemporaryString32
# define __G_CHAR Char32
# define __G_FRIEND uint32_t
#endif

namespace Fog {

// ============================================================================
// [Fog::String - Construction / Destruction]
// ============================================================================

static inline bool fitToRange(
  const __G_STRING& s, sysuint_t* _start, sysuint_t* _len, const Range& range)
{
  sysuint_t start = range.index;
  sysuint_t slen = s.length();

  if (start >= slen) return false;

  sysuint_t r = slen - start;
  if (r > range.length) r = range.length;

  *_start = start;
  *_len = r;
  return true;
}

__G_STRING::__G_STRING()
{
  _d = sharedNull->refAlways();
}

__G_STRING::__G_STRING(__G_CHAR ch, sysuint_t length)
{
  _d = Data::alloc(length, AllocCanFail);
  if (!_d) { _d = sharedNull->refAlways(); return; }
  if (!length) return;

  StringUtil::fill(_d->data, ch, length);
  _d->length = length;
  _d->data[length] = 0;
}

__G_STRING::__G_STRING(const __G_STRING& other)
{
  _d = other._d->ref();
}

__G_STRING::__G_STRING(const __G_STRING& other1, const __G_STRING& other2)
{
  if (other1.length() == 0)
  {
    _d = other2._d->ref();
  }
  else
  {
    _d = other1._d->ref();
    append(other2);
  }
}

#if __G_SIZE == 1
__G_STRING::__G_STRING(const Stub8& str)
{
  const Char8* s = str.str();
  sysuint_t length = (str.length() == DetectLength)
    ? StringUtil::len(s) : str.length();

  _d = Data::alloc(0, AllocCanFail, s, length);
  if (!_d) _d = sharedNull->refAlways();
}
#else
__G_STRING::__G_STRING(const StubAscii8& str)
{
  const Char8* s = str.str();
  sysuint_t length = (str.length() == DetectLength)
    ? StringUtil::len(s) : str.length();

  _d = Data::alloc(0, AllocCanFail, s, length);
  if (!_d) _d = sharedNull->refAlways();
}

__G_STRING::__G_STRING(const StubLocal8& str)
{
  _d = sharedNull->refAlways();
  set(str);
}

__G_STRING::__G_STRING(const StubUtf8& str)
{
  _d = sharedNull->refAlways();
  set(str);
}

__G_STRING::__G_STRING(const StubUtf16& str)
{
  _d = sharedNull->refAlways();
  set(str);
}

__G_STRING::__G_STRING(const StubUtf32& str)
{
  _d = sharedNull->refAlways();
  set(str);
}
#endif // __G_SIZE 

__G_STRING::__G_STRING(Data* d)
{
  _d = d;
}

__G_STRING::~__G_STRING()
{
  _d->derefInline();
}

// ============================================================================
// [Fog::String - Implicit Sharing]
// ============================================================================

void __G_STRING::_detach()
{
  if (isDetached()) return;
  Data* newd = Data::copy(_d, AllocCantFail);
  AtomicBase::ptr_setXchg(&_d, newd)->deref();
}

bool __G_STRING::_tryDetach()
{
  if (isDetached()) return true;
  Data* newd = Data::copy(_d, AllocCantFail);
  if (!newd) return false;
  AtomicBase::ptr_setXchg(&_d, newd)->deref();
  return true;
}

// ============================================================================
// [Fog::String::Flags]
// ============================================================================

void __G_STRING::setIsSharable(bool val)
{
  if (isSharable() == val) return;

  detach();

  if (val)
    _d->flags |= Data::IsSharable;
  else
    _d->flags &= ~Data::IsSharable;
}

void __G_STRING::setIsStrong(bool val)
{
  if (isStrong() == val) return;

  detach();

  if (val)
    _d->flags |= Data::IsStrong;
  else
    _d->flags &= ~Data::IsStrong;
}

// ============================================================================
// [Fog::String::Data]
// ============================================================================

err_t __G_STRING::reserve(sysuint_t to)
{
  if (to < _d->length) to = _d->length;
  if (_d->refCount.get() == 1 && _d->capacity >= to) goto done;

  if (_d->refCount.get() > 1)
  {
    Data* newd = Data::alloc(to, AllocCanFail, _d->data, _d->length);
    if (!newd) return Error::OutOfMemory;

    AtomicBase::ptr_setXchg(&_d, newd)->deref();
  }
  else
  {
    Data* newd = Data::realloc(_d, to, AllocCanFail);
    if (!newd) return Error::OutOfMemory;

    _d = newd;
  }
done:
  return Error::Ok;
}

err_t __G_STRING::resize(sysuint_t to)
{
  if (_d->refCount.get() == 1 && _d->capacity >= to) goto done;

  if (_d->refCount.get() > 1)
  {
    Data* newd = Data::alloc(to, AllocCanFail, 
      _d->data, to < _d->length ? to : _d->length);
    if (!newd) return Error::OutOfMemory;

    AtomicBase::ptr_setXchg(&_d, newd)->deref();
  }
  else
  {
    Data* newd = Data::realloc(_d, to, AllocCanFail);
    if (!newd) return Error::OutOfMemory;

    _d = newd;
  }

done:
  _d->hashCode = 0;
  _d->length = to;
  _d->data[to] = 0;
  return Error::Ok;
}

err_t __G_STRING::grow(sysuint_t by)
{
  sysuint_t lengthBefore = _d->length;
  sysuint_t lengthAfter = lengthBefore + by;

  FOG_ASSERT(lengthBefore <= lengthAfter);

  if (_d->refCount.get() == 1 && _d->capacity >= lengthAfter) goto done;

  if (_d->refCount.get() > 1)
  {
    sysuint_t optimalCapacity = Std::calcOptimalCapacity(
      sizeof(__G_STRING::Data), sizeof(__G_CHAR), lengthBefore, lengthAfter);

    Data* newd = Data::alloc(optimalCapacity, AllocCanFail, _d->data, _d->length);
    if (!newd) return Error::OutOfMemory;

    AtomicBase::ptr_setXchg(&_d, newd)->deref();
  }
  else
  {
    sysuint_t optimalCapacity = Std::calcOptimalCapacity(
      sizeof(__G_STRING::Data), sizeof(__G_CHAR), lengthBefore, lengthAfter);

    Data* newd = Data::realloc(_d, optimalCapacity, AllocCanFail);
    if (!newd) return Error::OutOfMemory;

    _d = newd;
  }

done:
  _d->hashCode = 0;
  _d->length = lengthAfter;
  _d->data[lengthAfter] = 0;
  return Error::Ok;
}

void __G_STRING::squeeze()
{
  sysuint_t i = _d->length;
  sysuint_t c = _d->capacity;

  // Pad to 16 bytes
#if __G_SIZE == 1
  i = (i + 15) & ~15;
#endif

#if __G_SIZE == 2
  i = (i + 7) & ~7;
#endif

#if __G_SIZE == 4
  i = (i + 3) & ~3;
#endif

  if (i < c)
  {
    Data* newd = Data::alloc(0, AllocCanFail, _d->data, _d->length);
    if (!newd) return;
    AtomicBase::ptr_setXchg(&_d, newd)->deref();
  }
}

void __G_STRING::clear()
{
  if (_d->refCount.get() > 1)
  {
    AtomicBase::ptr_setXchg(&_d, sharedNull->refAlways())->deref();
    return;
  }

  _d->hashCode = 0;
  _d->length = 0;
  _d->data[0] = 0;
}

void __G_STRING::free()
{
  AtomicBase::ptr_setXchg(&_d, sharedNull->refAlways())->deref();
}

__G_CHAR* __G_STRING::mData()
{
  detach();
  _d->hashCode = 0;
  return _d->data;
}

static __G_CHAR* _prepareSet(__G_STRING* self, sysuint_t length)
{
  __G_STRING::Data* d = self->_d;
  if (FOG_UNLIKELY(length == 0)) goto skip;

  if (d->refCount.get() > 1)
  {
    d = __G_STRING::Data::alloc(length, AllocCanFail);
    if (!d) return NULL;
    AtomicBase::ptr_setXchg(&self->_d, d)->derefInline();
  }
  else if (d->capacity < length)
  {
    sysuint_t optimalCapacity = Std::calcOptimalCapacity(
      sizeof(__G_STRING::Data), sizeof(__G_CHAR), d->length, length);

    d = __G_STRING::Data::realloc(d, optimalCapacity, AllocCanFail);
    if (!d) return NULL;
    self->_d = d;
  }

  d->hashCode = 0;
  d->length = length;
  d->data[d->length] = 0;
skip:
  return d->data;
}

static __G_CHAR* _prepareAppend(__G_STRING* self, sysuint_t length)
{
  __G_STRING::Data* d = self->_d;

  sysuint_t lengthBefore = d->length;
  sysuint_t lengthAfter = lengthBefore + length;

  if (FOG_UNLIKELY(length == 0)) goto skip;

  if (d->refCount.get() > 1)
  {
    d = __G_STRING::Data::alloc(lengthAfter, AllocCanFail, d->data, d->length);
    if (!d) return NULL;
    AtomicBase::ptr_setXchg(&self->_d, d)->derefInline();
  }
  else if (d->capacity < lengthAfter)
  {
    sysuint_t optimalCapacity = Std::calcOptimalCapacity(
      sizeof(__G_STRING::Data), sizeof(__G_CHAR), lengthBefore, lengthAfter);

    d = __G_STRING::Data::realloc(d, optimalCapacity, AllocCanFail);
    if (!d) return NULL;
    self->_d = d;
  }

  d->hashCode = 0;
  d->length = lengthAfter;
  d->data[lengthAfter] = 0;
skip:
  return d->data + lengthBefore;
}

static __G_CHAR* _prepareInsert(__G_STRING* self, sysuint_t index, sysuint_t length)
{
  __G_STRING::Data* d = self->_d;

  sysuint_t lengthBefore = d->length;
  sysuint_t lengthAfter = lengthBefore + length;

  if (index > lengthBefore) index = lengthBefore;
  if (FOG_UNLIKELY(!length)) goto skip;

  if (d->refCount.get() > 1 || d->capacity < lengthAfter)
  {
    sysuint_t optimalCapacity = Std::calcOptimalCapacity(
      sizeof(__G_STRING::Data), sizeof(__G_CHAR), lengthBefore, lengthAfter);

    d = __G_STRING::Data::alloc(optimalCapacity, AllocCanFail, d->data, index);
    if (!d) return NULL;

    StringUtil::copy(
      d->data + index + length, self->_d->data + index, lengthBefore - index);
    AtomicBase::ptr_setXchg(&self->_d, d)->derefInline();
  }

  d->hashCode = 0;
  d->length = lengthAfter;
  d->data[lengthAfter] = 0;
skip:
  return d->data + index;
}

static __G_CHAR* _prepareReplace(__G_STRING* self, sysuint_t index, sysuint_t range, sysuint_t replacementLength)
{
  __G_STRING::Data* d = self->_d;

  sysuint_t lengthBefore = d->length;

  FOG_ASSERT(index <= lengthBefore);
  if (lengthBefore - index > range) range = lengthBefore - index;

  sysuint_t lengthAfter = lengthBefore - range + replacementLength;

  if (d->refCount.get() > 1 || d->capacity < lengthAfter)
  {
    sysuint_t optimalCapacity = Std::calcOptimalCapacity(
      sizeof(__G_STRING::Data), sizeof(__G_CHAR), lengthBefore, lengthAfter);

    d = __G_STRING::Data::alloc(optimalCapacity, AllocCanFail, d->data, index);
    if (!d) return NULL;

    StringUtil::copy(
      d->data + index + replacementLength, 
      self->_d->data + index + range, lengthBefore - index - range);
    AtomicBase::ptr_setXchg(&self->_d, d)->derefInline();
  }
  else
  {
    StringUtil::move(
      d->data + index + replacementLength, 
      d->data + index + range, lengthBefore - index - range);
  }

  d->hashCode = 0;
  d->length = lengthAfter;
  d->data[lengthAfter] = 0;
  return d->data + index;
}

// ============================================================================
// [Fog::String::Set]
// ============================================================================

err_t __G_STRING::set(__G_CHAR ch, sysuint_t length)
{
  if (length == DetectLength) return Error::InvalidArgument;

  __G_CHAR* p = _prepareSet(this, length);
  if (!p) return Error::OutOfMemory;

  StringUtil::fill(p, ch, length);
  return Error::Ok;
}

#if __G_SIZE == 1
err_t __G_STRING::set(const Stub8& str)
{
  const Char8* s = str.str();
  sysuint_t length = (str.length() == DetectLength)
    ? StringUtil::len(s) : str.length();

  __G_CHAR* p = _prepareSet(this, length);
  if (!p) return Error::OutOfMemory;

  StringUtil::copy(p, s, length);
  return Error::Ok;
}

err_t __G_STRING::set(const StubUtf8& str, const TextCodec& tc)
{
  String32 s32;
  err_t err;

  if ( (err = TextCodec::utf8().appendToUtf32(s32, str)) ) return err;
  return tc.fromUtf32(*this, s32);
}

err_t __G_STRING::set(const StubUtf16& str, const TextCodec& tc)
{
  clear();
  return tc.appendFromUtf16(*this, str);
}

err_t __G_STRING::set(const StubUtf32& str, const TextCodec& tc)
{
  clear();
  return tc.appendFromUtf32(*this, str);
}

#else
err_t __G_STRING::set(const void* str, sysuint_t size, const TextCodec& tc)
{
#if __G_SIZE == 2
  return tc.toUtf16(*this, Stub8((const char*)str, size));
#else
  return tc.toUtf32(*this, Stub8((const char*)str, size));
#endif
}

err_t __G_STRING::set(const StubAscii8& str)
{
  const Char8* s = str.str();
  sysuint_t length = (str.length() == DetectLength)
    ? StringUtil::len(s) : str.length();

  __G_CHAR* p = _prepareSet(this, length);
  if (!p) return Error::OutOfMemory;

  StringUtil::copy(p, s, length);
  return Error::Ok;
}

err_t __G_STRING::set(const StubLocal8& str)
{
  return set(str.str(), str.length(), TextCodec::local8());
}

err_t __G_STRING::set(const StubUtf8& str)
{
  return set(str.str(), str.length(), TextCodec::utf8());
}

err_t __G_STRING::set(const StubUtf16& str)
{
#if __G_SIZE == 2
  const Char16* s = str.str();
  sysuint_t length = (str.length() == DetectLength)
    ? StringUtil::len(s) : str.length();

  __G_CHAR* p = _prepareSet(this, length);
  if (!p) return Error::OutOfMemory;

  StringUtil::copy(p, s, length);
  return Error::Ok;
#else
  return TextCodec::utf16().toUtf32(*this,
    Stub8((const char*)str.str(), str.length() != DetectLength 
      ? str.length() << 1
      : DetectLength));
#endif // __G_SIZE
}

err_t __G_STRING::set(const StubUtf32& str)
{
#if __G_SIZE == 2
  return TextCodec::utf32().toUtf16(
    *this, Stub8((const char*)str.str(), str.length() != DetectLength 
      ? str.length() << 2 
      : DetectLength));
#else
  const Char32* s = str.str();
  sysuint_t length = (str.length() == DetectLength)
    ? StringUtil::len(s) : str.length();

  __G_CHAR* p = _prepareSet(this, length);
  if (!p) return Error::OutOfMemory;

  StringUtil::copy(p, s, length);
  return Error::Ok;
#endif // __G_SIZE
}
#endif // __G_SIZE 

#if __G_SIZE == 2
err_t __G_STRING::set(const String32& other)
{
  return TextCodec::utf32().toUtf16(
    *this, Stub8((const char*)other.cData(), other.length() << 2));
}
#endif // __G_SIZE
#if __G_SIZE == 4
err_t __G_STRING::set(const String16& other)
{
  return TextCodec::utf16().toUtf32(
    *this, Stub8((const char*)other.cData(), other.length() << 1));
}
#endif // __G_SIZE

err_t __G_STRING::set(const __G_STRING& other)
{
  Data* self_d = _d;
  Data* other_d = other._d;
  if (self_d == other_d) return Error::Ok;

  if ((self_d->flags & Data::IsStrong) != 0 || 
      (other_d->flags & Data::IsSharable) == 0)
  {
    return setDeep(other);
  }
  else
  {
    AtomicBase::ptr_setXchg(&_d, other_d->refInline())->derefInline();
    return Error::Ok;
  }
}

err_t __G_STRING::setDeep(const __G_STRING& other)
{
  Data* self_d = _d;
  Data* other_d = other._d;
  if (self_d == other_d) return Error::Ok;

  __G_CHAR* p = _prepareSet(this, other_d->length);
  if (!p) return Error::OutOfMemory;

  StringUtil::copy(p, other.cData(), other.length());
  return Error::Ok;
}

err_t __G_STRING::setBool(bool b)
{
  return set(StubAscii8(b ? "true" : "false"));
}

err_t __G_STRING::setInt(int32_t n, int base)
{
  clear();
  return appendInt((int64_t)n, base, FormatFlags());
}

err_t __G_STRING::setInt(uint32_t n, int base)
{
  clear();
  return appendInt((uint64_t)n, base, FormatFlags());
}

err_t __G_STRING::setInt(int64_t n, int base)
{
  clear();
  return appendInt(n, base, FormatFlags());
}

err_t __G_STRING::setInt(uint64_t n, int base)
{
  clear();
  return appendInt(n, base, FormatFlags());
}

#if __G_SIZE == 1
err_t __G_STRING::setInt(int32_t n, int base, const FormatFlags& ff)
{
  clear();
  return appendInt((int64_t)n, base, ff);
}

err_t __G_STRING::setInt(uint32_t n, int base, const FormatFlags& ff)
{
  clear();
  return appendInt((uint64_t)n, base, ff);
}

err_t __G_STRING::setInt(int64_t n, int base, const FormatFlags& ff)
{
  clear();
  return appendInt(n, base, ff);
}

err_t __G_STRING::setInt(uint64_t n, int base, const FormatFlags& ff)
{
  clear();
  return appendInt(n, base, ff);
}
#else
err_t __G_STRING::setInt(int32_t n, int base, const FormatFlags& ff, const Locale* locale)
{
  clear();
  return appendInt((int64_t)n, base, ff, locale);
}

err_t __G_STRING::setInt(uint32_t n, int base, const FormatFlags& ff, const Locale* locale)
{
  clear();
  return appendInt((uint64_t)n, base, ff, locale);
}

err_t __G_STRING::setInt(int64_t n, int base, const FormatFlags& ff, const Locale* locale)
{
  clear();
  return appendInt(n, base, ff, locale);
}

err_t __G_STRING::setInt(uint64_t n, int base, const FormatFlags& ff, const Locale* locale)
{
  clear();
  return appendInt(n, base, ff, locale);
}
#endif

err_t __G_STRING::setDouble(double d, int doubleForm)
{
  clear();
  return appendDouble(d, doubleForm, FormatFlags());
}

#if __G_SIZE == 1
err_t __G_STRING::setDouble(double d, int doubleForm, const FormatFlags& ff)
{
  clear();
  return appendDouble(d, doubleForm, ff);
}
#else
err_t __G_STRING::setDouble(double d, int doubleForm, const FormatFlags& ff, const Locale* locale)
{
  clear();
  return appendDouble(d, doubleForm, ff, locale);
}
#endif

err_t __G_STRING::format(const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  clear();
  err_t err = appendVformat(fmt, ap);

  va_end(ap);
  return err;
}

err_t __G_STRING::vformat(const char* fmt, va_list ap)
{
  clear();
  return appendVformat(fmt, ap);
}

err_t __G_STRING::wformat(const __G_STRING& fmt, __G_CHAR lex, const Sequence<__G_STRING>& args)
{
  clear();
  return appendWformat(fmt, lex, args.cData(), args.length());
}

err_t __G_STRING::wformat(const __G_STRING& fmt, __G_CHAR lex, const __G_STRING* args, sysuint_t length)
{
  clear();
  return appendWformat(fmt, lex, args, length);
}

// ============================================================================
// [Fog::String::Append]
// ============================================================================

// For 8 bit and 16 bit implementations
static err_t append_ntoa(String32* self, uint64_t n, int base, const FormatFlags& ff, const Locale* locale, StringUtil::NTOAOut* out);

#if __G_SIZE == 1
static err_t append_ntoa(__G_STRING* self, uint64_t n, int base, const FormatFlags& ff, StringUtil::NTOAOut* out)
#else
static err_t append_ntoa(__G_STRING* self, uint64_t n, int base, const FormatFlags& ff, const Locale* locale, StringUtil::NTOAOut* out)
#endif
{
#if __G_SIZE == 1
  TemporaryString32<256> t;
  err_t err = append_ntoa(&t, n, base, ff, NULL, out);
  if (err) return err;
  return self->append(t, TextCodec::ascii8());
#elif __G_SIZE == 2
  TemporaryString32<256> t;
  err_t err = append_ntoa(&t, n, base, ff, locale, out);
  if (err) return err;
  return self->append(t);
#else // __G_SIZE == 4
  const Locale& l = locale ? *locale : Locale::posix();

  __G_CHAR prefixBuffer[4];
  __G_CHAR* prefix = prefixBuffer;

  sysuint_t width = ff.width;
  sysuint_t precision = ff.precision;
  uint32_t fmt = ff.flags;

  if (out->negative)
    *prefix++ = l.minus();
  else if (fmt & FormatFlags::ShowSign)
    *prefix++ = l.plus();
  else if (fmt & FormatFlags::BlankPositive)
    *prefix++ = l.space();

  if (fmt & FormatFlags::Alternate)
  {
    if (base == 8)
    {
      if (n != 0) { out->result--; *(out->result) = '0'; out->length++; }
    }
    else if (base == 16)
    {
      *prefix++ = '0';
      *prefix++ = (fmt & FormatFlags::CapitalizeEOrX) ? 'X' : 'x';
    }
  }

  sysuint_t prefixLength = (sysuint_t)(prefix - prefixBuffer);
  sysuint_t resultLength = out->length;

  if (width == FormatFlags::NoWidth) width = 0;
  if ((fmt & FormatFlags::ZeroPadded) &&
      precision == FormatFlags::NoPrecision &&
      width > prefixLength + resultLength) precision = width - prefixLength;
  if (precision == FormatFlags::NoPrecision) precision = 0;

  sysuint_t fillLength = (resultLength < precision) ? precision - resultLength : 0;
  sysuint_t fullLength = prefixLength + resultLength + fillLength;
  sysuint_t widthLength = (fullLength < width) ? width - fullLength : 0;

  fullLength += widthLength;

  __G_CHAR* p = _prepareAppend(self, fullLength);
  if (!p) return Error::OutOfMemory;

  // Left justification
  if (!(fmt & FormatFlags::LeftAdjusted))
  {
    StringUtil::fill(p, __G_CHAR(' '), widthLength); p += widthLength;
  }

  // Number with prefix and precision
  StringUtil::copy(p, prefixBuffer, prefixLength); p += prefixLength;

  // Body
  if (base == 10 && l.zero() != __G_CHAR('0'))
  {
    StringUtil::fill(p, l.zero(), fillLength); p += fillLength;

    for (sysuint_t i = 0; i != resultLength; i++) 
      p[i] = l.zero() + (uint32_t)((uint8_t)out->result[i] - (uint8_t)'0');
    p += resultLength;
  }
  else
  {
    StringUtil::fill(p, __G_CHAR('0'), fillLength); p += fillLength;
    StringUtil::copy(p, (Char8*)out->result, resultLength); p += resultLength;
  }

  // Right justification
  if (fmt & FormatFlags::LeftAdjusted)
  {
    StringUtil::fill(p, __G_CHAR(' '), widthLength);
  }
#endif
}

static __G_CHAR* append_exponent(__G_CHAR* dest, uint exp, __G_CHAR zero)
{
  uint t;

  if (exp > 99)
  {
    t = exp / 100; *dest++ = zero + __G_CHAR(t); exp -= t * 100;
  }
  t = exp / 10; *dest++ = zero + __G_CHAR(t); exp -= t * 10;
  *dest++ = zero + __G_CHAR(exp);

  return dest;
}

err_t __G_STRING::append(__G_CHAR ch, sysuint_t length)
{
  if (length == DetectLength) return Error::InvalidArgument;

  __G_CHAR* p = _prepareAppend(this, length);
  if (!p) return Error::OutOfMemory;

  StringUtil::fill(p, ch, length);
  return Error::Ok;
}

#if __G_SIZE == 1
err_t __G_STRING::append(const Stub8& str)
{
  const Char8* s = str.str();
  sysuint_t length = (str.length() == DetectLength)
    ? StringUtil::len(s) : str.length();

  __G_CHAR* p = _prepareAppend(this, length);
  if (!p) return Error::OutOfMemory;

  StringUtil::copy(p, s, length);
  return Error::Ok;
}

err_t __G_STRING::append(const StubUtf8& str, const TextCodec& tc)
{
  String32 s32;
  err_t err;

  if ( (err = TextCodec::utf8().appendToUtf32(s32, str)) ) return err;
  return tc.appendFromUtf32(*this, s32);
}

err_t __G_STRING::append(const StubUtf16& str, const TextCodec& tc)
{
  return tc.appendFromUtf16(*this, str);
}

err_t __G_STRING::append(const StubUtf32& str, const TextCodec& tc)
{
  return tc.appendFromUtf32(*this, str);
}
#else
err_t __G_STRING::append(const void* str, sysuint_t size, const TextCodec& tc)
{
#if __G_SIZE == 2
  return tc.appendToUtf16(*this, Stub8((const char*)str, size));
#else
  return tc.appendToUtf32(*this, Stub8((const char*)str, size));
#endif // __G_SIZE
}

err_t __G_STRING::append(const StubAscii8& str)
{
  const Char8* s = str.str();
  sysuint_t length = (str.length() == DetectLength)
    ? StringUtil::len(s) : str.length();

  __G_CHAR* p = _prepareAppend(this, length);
  if (!p) return Error::OutOfMemory;

  StringUtil::copy(p, s, length);
  return Error::Ok;
}

err_t __G_STRING::append(const StubLocal8& str)
{
  return append(str.str(), str.length(), TextCodec::local8());
}

err_t __G_STRING::append(const StubUtf8& str)
{
  return append(str.str(), str.length(), TextCodec::utf8());
}

err_t __G_STRING::append(const StubUtf16& str)
{
#if __G_SIZE == 2
  const Char16* s = str.str();
  sysuint_t length = (str.length() == DetectLength)
    ? StringUtil::len(s) : str.length();

  __G_CHAR* p = _prepareAppend(this, length);
  if (!p) return Error::OutOfMemory;

  StringUtil::copy(p, s, length);
  return Error::Ok;
#else
  return append(str.str(), str.length(), TextCodec::utf32());
#endif
}

err_t __G_STRING::append(const StubUtf32& str)
{
#if __G_SIZE == 2
  return append(str.str(), str.length(), TextCodec::utf16());
#else
  const Char32* s = str.str();
  sysuint_t length = (str.length() == DetectLength)
    ? StringUtil::len(s) : str.length();

  __G_CHAR* p = _prepareAppend(this, length);
  if (!p) return Error::OutOfMemory;

  StringUtil::copy(p, s, length);
  return Error::Ok;
#endif
}
#endif // __G_SIZE 

err_t __G_STRING::append(const __G_STRING& _other)
{
  if (length() == 0) return set(_other);

  __G_STRING other(_other);

  __G_CHAR* p = _prepareAppend(this, other.length());
  if (!p) return Error::OutOfMemory;

  StringUtil::copy(p, other.cData(), other.length());
  return Error::Ok;
}

#if __G_SIZE == 2
err_t __G_STRING::append(const String32& other)
{
  return TextCodec::utf32().appendToUtf16(
    *this, Stub8((const char*)other.cData(), other.length() << 2));
}
#endif // __G_SIZE
#if __G_SIZE == 4
err_t __G_STRING::append(const String16& other)
{
  return TextCodec::utf16().appendToUtf32(
    *this, Stub8((const char*)other.cData(), other.length() << 1));
}
#endif // __G_SIZE

err_t __G_STRING::appendBool(bool b)
{
  return append(StubAscii8(b ? "true" : "false"));
}

err_t __G_STRING::appendInt(int32_t n, int base)
{
  return appendInt((int64_t)n, base, FormatFlags());
}

err_t __G_STRING::appendInt(uint32_t n, int base)
{
  return appendInt((uint64_t)n, base, FormatFlags());
}

err_t __G_STRING::appendInt(int64_t n, int base)
{
  return appendInt(n, base, FormatFlags());
}

err_t __G_STRING::appendInt(uint64_t n, int base)
{
  return appendInt(n, base, FormatFlags());
}

#if __G_SIZE == 1
err_t __G_STRING::appendInt(int32_t n, int base, const FormatFlags& ff)
{
  return appendInt((int64_t)n, base, ff);
}

err_t __G_STRING::appendInt(uint32_t n, int base, const FormatFlags& ff)
{
  return appendInt((uint64_t)n, base, ff);
}

err_t __G_STRING::appendInt(int64_t n, int base, const FormatFlags& ff)
{
  StringUtil::NTOAOut out;
  StringUtil::itoa(n, base, (ff.flags & FormatFlags::Capitalize) != 0, &out);

  return append_ntoa(this, (uint64_t)n, base, ff, &out);
}

err_t __G_STRING::appendInt(uint64_t n, int base, const FormatFlags& ff)
{
  StringUtil::NTOAOut out;
  StringUtil::utoa(n, base, (ff.flags & FormatFlags::Capitalize) != 0, &out);

  return append_ntoa(this, n, base, ff, &out);
}
#else
err_t __G_STRING::appendInt(int32_t n, int base, const FormatFlags& ff, const Locale* locale)
{
  return appendInt((int64_t)n, base, ff, locale);
}

err_t __G_STRING::appendInt(uint32_t n, int base, const FormatFlags& ff, const Locale* locale)
{
  return appendInt((uint64_t)n, base, ff, locale);
}

err_t __G_STRING::appendInt(int64_t n, int base, const FormatFlags& ff, const Locale* locale)
{
  StringUtil::NTOAOut out;
  StringUtil::itoa(n, base, (ff.flags & FormatFlags::Capitalize) != 0, &out);

  return append_ntoa(this, (uint64_t)n, base, ff, locale, &out);
}

err_t __G_STRING::appendInt(uint64_t n, int base, const FormatFlags& ff, const Locale* locale)
{
  StringUtil::NTOAOut out;
  StringUtil::utoa(n, base, (ff.flags & FormatFlags::Capitalize) != 0, &out);

  return append_ntoa(this, n, base, ff, locale, &out);
}
#endif

err_t __G_STRING::appendDouble(double d, int doubleForm)
{
  return appendDouble(d, doubleForm, FormatFlags());
}

// Defined in StringUtil_dtoa.cpp;
namespace StringUtil { FOG_HIDDEN double _mprec_log10(int dig); }

#if __G_SIZE == 1
err_t __G_STRING::appendDouble(double d, int doubleForm, const FormatFlags& ff)
#else
err_t __G_STRING::appendDouble(double d, int doubleForm, const FormatFlags& ff, const Locale* locale)
#endif
{
#if __G_SIZE == 1
  TemporaryString32<256> t;
  err_t err = t.appendDouble(d, doubleForm, ff, NULL);
  if (err) return err;
  return append(t, TextCodec::ascii8());
#elif __G_SIZE == 2
  TemporaryString32<256> t;
  err_t err = t.appendDouble(d, doubleForm, ff, locale);
  if (err) return err;
  return append(t);
#else
  err_t err = Error::Ok;
  const Locale& l = locale ? *locale : Locale::posix();

  StringUtil::NTOAOut out;

  sysuint_t width = ff.width;
  sysuint_t precision = ff.precision;
  uint32_t fmt = ff.flags;

  sysuint_t beginLength = _d->length;
  sysuint_t numberLength;
  sysuint_t i;
  sysuint_t savedPrecision = precision;
  int decpt;

  uint8_t* bufCur;
  uint8_t* bufEnd;

  __G_CHAR* dest;
  __G_CHAR sign = __G_CHAR('\0');
  __G_CHAR zero = l.zero() - __G_CHAR('0');

  if (precision == FormatFlags::NoPrecision) precision = 6;

  if (d < 0.0)
    { sign = l.minus(); d = -d; }
  else if (fmt & FormatFlags::ShowSign)
    sign = l.plus();
  else if (fmt & FormatFlags::BlankPositive)
    sign = l.space();

  if (sign != 0) append(sign);

  // Decimal form
  if (doubleForm == DF_Decimal)
  {
    StringUtil::dtoa(d, 2, precision, &out);

    decpt = out.decpt;
    if (out.decpt == 9999) goto __InfOrNaN;

    bufCur = out.result;
    bufEnd = bufCur + out.length;

    // reserve some space for number
    i = precision + 16;
    if (decpt > 0) i += (sysuint_t)decpt;

    if ( (err = grow(i)) ) return err;
    dest = xData() + length();

    while (bufCur != bufEnd && decpt > 0) { *dest++ = zero + (uint8_t)*bufCur++; decpt--; }
    // Even if not in buffer
    while (decpt > 0) { *dest++ = zero + __G_CHAR('0'); decpt--; }

    if ((fmt & FormatFlags::Alternate) != 0 || bufCur != bufEnd)
    {
      if (bufCur == out.result) *dest++ = zero + __G_CHAR('0');
      *dest++ = l.decimalPoint();
      while (decpt < 0 && precision > 0) { *dest++ = zero + __G_CHAR('0'); decpt++; precision--; }

      // Print rest of stuff
      while (*bufCur && precision > 0) { *dest++ = zero + __G_CHAR(*bufCur++); precision--; }
      // And trailing zeros
      while (precision > 0) { *dest++ = zero + __G_CHAR('0'); precision--; }
    }

    xFinalize(dest);
  }
  // Exponential form
  else if (doubleForm == DF_Exponent)
  {
__exponentialForm:
    StringUtil::dtoa(d, 2, precision + 1, &out);

    decpt = out.decpt;
    if (decpt == 9999) goto __InfOrNaN;

    // reserve some space for number, we need +X.{PRECISION}e+123
    if ( (err = grow(precision + 10)) ) return err;
    dest = xData() + length();

    bufCur = out.result;
    bufEnd = bufCur + out.length;

    *dest++ = zero + (uint8_t)*(bufCur++);
    if ((fmt & FormatFlags::Alternate) != 0 || precision != 0) *dest++ = l.decimalPoint();
    while (bufCur != bufEnd && precision > 0)
    {
      *dest++ = zero + (uint8_t)*(bufCur++);
      precision--;
    }

    // Add trailing zeroes to fill out to ndigits unless this is
    // DF_SignificantDigits
    if (doubleForm == DF_Exponent)
    {
      for (i = precision; i; i--) *dest++ = zero + __G_CHAR('0');
    }

    // Add the exponent.
    *dest++ = l.exponential();
    decpt--;
    if (decpt < 0)
      { *dest++ = l.minus(); decpt = -decpt; }
    else
      *dest++ = l.plus();

    dest = append_exponent(dest, decpt, zero + __G_CHAR('0'));

    xFinalize(dest);
  }
  // Significant digits form
  else /* if (doubleForm == DF_SignificantDigits) */
  {
    __G_CHAR* save;
    if (d <= 0.0001 || d >= StringUtil::_mprec_log10(precision))
    {
      if (precision > 0) precision--;
      goto __exponentialForm;
    }

    if (d < 1.0)
    {
      // what we want is ndigits after the point
      StringUtil::dtoa(d, 3, ++precision, &out);
    }
    else
    {
      StringUtil::dtoa(d, 2, precision, &out);
    }

    decpt = out.decpt;
    if (decpt == 9999) goto __InfOrNaN;

    // reserve some space for number
    i = precision + 16;
    if (decpt > 0) i += (sysuint_t)decpt;

    if ( (err = grow(i)) ) return err;
    dest = save = xData() + length();

    bufCur = out.result;
    bufEnd = bufCur + out.length;

    while (bufCur != bufEnd && decpt > 0) { *dest++ = zero + *bufCur++; decpt--; precision--; }
    // Even if not in buffer
    while (decpt > 0 && precision > 0) { *dest++ = zero + __G_CHAR('0'); decpt--; precision--; }

    if ((fmt & FormatFlags::Alternate) != 0 || bufCur != bufEnd)
    {
      if (dest == save) *dest++ = zero + __G_CHAR('0');
      *dest++ = l.decimalPoint();
      while (decpt < 0 && precision > 0) { *dest++ = zero + __G_CHAR('0'); decpt++; precision--; }

      // Print rest of stuff
      while (bufCur != bufEnd && precision > 0){ *dest++ = zero + *bufCur++; precision--; }
      // And trailing zeros
      while (precision > 0) { *dest++ = zero + __G_CHAR('0'); precision--; }
    }

    xFinalize(dest);
  }
  goto __ret;

__InfOrNaN:
  err |= append(StubAscii8((const char*)out.result, out.length));
__ret:
  // Apply padding
  numberLength = _d->length - beginLength;
  if (width != (sysuint_t)-1 && width > numberLength)
  {
    sysuint_t fill = width - numberLength;

    if ((fmt & FormatFlags::LeftAdjusted) == 0)
    {
      if (savedPrecision == FormatFlags::NoPrecision)
        err |= insert(beginLength + (sign != 0), zero + __G_CHAR('0'), fill);
      else
        err |= insert(beginLength, __G_CHAR(' '), fill);
    }
    else
    {
      err |= append(__G_CHAR(' '), fill);
    }
  }
  return err;
#endif // __G_SIZE
}

err_t __G_STRING::appendFormat(const char* fmt, ...)
{
  FOG_ASSERT(fmt);

  clear();

  va_list ap;
  va_start(ap, fmt);
  err_t err = appendVformat(fmt, ap);
  va_end(ap);

  return err;
}

err_t __G_STRING::appendVformat(const char* fmt, va_list ap)
{
  // TODO
  return Error::NotImplemented;
}

err_t __G_STRING::appendWformat(const __G_STRING& fmt, __G_CHAR lex, const Sequence<__G_STRING>& args)
{
  return appendWformat(fmt, lex, args.cData(), args.length());
}

err_t __G_STRING::appendWformat(const __G_STRING& fmt, __G_CHAR lex, const __G_STRING* args, sysuint_t length)
{
  const __G_CHAR* fmtBeg = fmt.cData();
  const __G_CHAR* fmtEnd = fmtBeg + fmt.length();
  const __G_CHAR* fmtCur;

  err_t err = Error::Ok;

  for (fmtCur = fmtBeg; fmtCur != fmtEnd; )
  {
    if (*fmtCur == lex)
    {
      fmtBeg = fmtCur;
      if ( (err = append(__G_STRING_STUB(fmtBeg, (sysuint_t)(fmtCur - fmtBeg)))) ) goto done;

      if (++fmtCur != fmtEnd)
      {
        __G_CHAR ch = *fmtCur;

        if (ch >= __G_CHAR('0') && ch <= __G_CHAR('9'))
        {
          uint32_t n = ch.ch() - (uint32_t)'0';
          if (n < length)
          {
            if ( (err = append(args[n])) ) goto done;
            fmtBeg = fmtCur + 1;
          }
        }
        else if (ch == lex)
          fmtBeg++;
      }
      else
        break;
    }
    fmtCur++;
  }

  if (fmtCur != fmtBeg)
  {
    err = append(__G_STRING_STUB(fmtBeg, (sysuint_t)(fmtCur - fmtBeg)));
  }

done:
  return err;
}

// ============================================================================
// [Fog::String::Prepend]
// ============================================================================

err_t __G_STRING::prepend(__G_CHAR ch, sysuint_t length)
{
  return insert(0, ch, length);
}

#if __G_SIZE == 1
err_t __G_STRING::prepend(const Stub8& str)
{
  return insert(0, str);
}

err_t __G_STRING::prepend(const StubUtf8& str, const TextCodec& tc)
{
  return insert(0, str, tc);
}

err_t __G_STRING::prepend(const StubUtf16& str, const TextCodec& tc)
{
  return insert(0, str, tc);
}

err_t __G_STRING::prepend(const StubUtf32& str, const TextCodec& tc)
{
  return insert(0, str, tc);
}
#else
err_t __G_STRING::prepend(const void* str, sysuint_t size, const TextCodec& tc)
{
  return insert(0, str, size, tc);
}

err_t __G_STRING::prepend(const StubAscii8& str)
{
  return insert(0, str);
}

err_t __G_STRING::prepend(const StubLocal8& str)
{
  return insert(0, str);
}

err_t __G_STRING::prepend(const StubUtf8& str)
{
  return insert(0, str);
}

err_t __G_STRING::prepend(const StubUtf16& str)
{
  return insert(0, str);
}

err_t __G_STRING::prepend(const StubUtf32& str)
{
  return insert(0, str);
}
#endif // __G_SIZE 

err_t __G_STRING::prepend(const __G_STRING& other)
{
  return insert(0, other);
}

#if __G_SIZE != 1
err_t __G_STRING::prepend(const __G_STRING_UTFX& other)
{
  return prepend(__G_STRING_STUBX(other.cData(), other.length()));
}
#endif // __G_SIZE

// ============================================================================
// [Fog::String::Insert]
// ============================================================================

err_t __G_STRING::insert(sysuint_t index, __G_CHAR ch, sysuint_t length)
{
  if (length == DetectLength) return Error::InvalidArgument;

  __G_CHAR* p = _prepareInsert(this, index, length);
  if (!p) return Error::OutOfMemory;

  StringUtil::fill(p, ch, length);
  return Error::Ok;
}

#if __G_SIZE == 1
err_t __G_STRING::insert(sysuint_t index, const Stub8& str)
{
  const Char8* s = str.str();
  sysuint_t length = (str.length() == DetectLength)
    ? StringUtil::len(s) : str.length();

  __G_CHAR* p = _prepareInsert(this, index, length);
  if (!p) return Error::OutOfMemory;

  StringUtil::copy(p, s, length);
  return Error::Ok;
}

err_t __G_STRING::insert(sysuint_t index, const StubUtf8& str, const TextCodec& tc)
{
  String8 s8;
  String32 s32;
  err_t err;

  if ( (err = TextCodec::utf8().appendToUtf32(s32, str)) ) return err;
  if ( (err = tc.appendFromUtf32(s8, s32)) ) return err;

  return insert(index, s8);
}

err_t __G_STRING::insert(sysuint_t index, const StubUtf16& str, const TextCodec& tc)
{
  TemporaryString8<TemporaryLength> t;
  err_t err = tc.appendFromUtf16(t, str);
  if (err) return err;

  return insert(index, t);
}

err_t __G_STRING::insert(sysuint_t index, const StubUtf32& str, const TextCodec& tc)
{
  TemporaryString8<TemporaryLength> t;
  err_t err = tc.appendFromUtf32(t, str);
  if (err) return err;

  return insert(index, t);
}
#else
err_t __G_STRING::insert(sysuint_t index, const void* str, sysuint_t size, const TextCodec& tc)
{
  __G_STRING t;
#if __G_SIZE == 2
  err_t err = tc.appendToUtf16(t, Stub8((const char*)str, size));
#else
  err_t err = tc.appendToUtf32(t, Stub8((const char*)str, size));
#endif // __G_SIZE
  if (err) return err;
  return insert(index, t);
}

err_t __G_STRING::insert(sysuint_t index, const StubAscii8& str)
{
  const Char8* s = str.str();
  sysuint_t length = (str.length() == DetectLength)
    ? StringUtil::len(s) : str.length();

  __G_CHAR* p = _prepareInsert(this, index, length);
  if (!p) return Error::OutOfMemory;

  StringUtil::copy(p, s, length);
  return Error::Ok;
}

err_t __G_STRING::insert(sysuint_t index, const StubLocal8& str)
{
  return insert(index, str.str(), str.length(), TextCodec::local8());
}

err_t __G_STRING::insert(sysuint_t index, const StubUtf8& str)
{
  return insert(index, str.str(), str.length(), TextCodec::utf8());
}

err_t __G_STRING::insert(sysuint_t index, const StubUtf16& str)
{
#if __G_SIZE == 2
  const Char16* s = str.str();
  sysuint_t length = (str.length() == DetectLength)
    ? StringUtil::len(s) : str.length();

  __G_CHAR* p = _prepareInsert(this, index, length);
  if (!p) return Error::OutOfMemory;

  StringUtil::copy(p, s, length);
  return Error::Ok;
#else
  return insert(index, str.str(), str.length(), TextCodec::utf16());
#endif // __G_SIZE
}

err_t __G_STRING::insert(sysuint_t index, const StubUtf32& str)
{
#if __G_SIZE == 2
  return insert(index, str.str(), str.length(), TextCodec::utf32());
#else
  const Char32* s = str.str();
  sysuint_t length = (str.length() == DetectLength)
    ? StringUtil::len(s) : str.length();

  __G_CHAR* p = _prepareInsert(this, index, length);
  if (!p) return Error::OutOfMemory;

  StringUtil::copy(p, s, length);
  return Error::Ok;
#endif // __G_SIZE
}
#endif // __G_SIZE 

err_t __G_STRING::insert(sysuint_t index, const __G_STRING& _other)
{
  __G_STRING other(_other);

  __G_CHAR* p = _prepareInsert(this, index, other.length());
  if (!p) return Error::OutOfMemory;

  StringUtil::copy(p, other.cData(), other.length());
  return Error::Ok;
}

#if __G_SIZE != 1
err_t __G_STRING::insert(sysuint_t index, const __G_STRING_UTFX& other)
{
  __G_STRING t;
#if __G_SIZE == 2
  err_t err = TextCodec::utf32().appendToUtf16(
    t, Stub8((const char*)other.cData(), other.length() << 2));
#else
  err_t err = TextCodec::utf16().appendToUtf32(
    t, Stub8((const char*)other.cData(), other.length() << 1));
#endif
  if (err) return err;
  return insert(index, t);
}
#endif // __G_SIZE

// ============================================================================
// [Fog::String - Remove]
// ============================================================================

sysuint_t __G_STRING::remove(const Range& range)
{
  sysuint_t rstart, rlen;
  if (!fitToRange(*this, &rstart, &rlen, range)) return 0;

  sysuint_t lenPart1 = rstart;
  sysuint_t lenPart2 = length() - rstart - rlen;
  sysuint_t lenAfter = length() - rlen;

  if (_d->refCount.get() > 1)
  {
    Data* newd = Data::alloc(lenAfter, AllocCanFail);
    if (!newd) return 0;

    StringUtil::copy(newd->data, _d->data, lenPart1);
    StringUtil::copy(newd->data + rstart, _d->data + rstart + rlen, lenPart2);
    newd->length = lenAfter;
    newd->data[lenAfter] = 0;

    AtomicBase::ptr_setXchg(&_d, newd)->deref();
  }
  else
  {
    StringUtil::move(_d->data + rstart, _d->data + rstart + rlen, lenPart2);
    _d->length = lenAfter;
    _d->data[lenAfter] = 0;
    _d->hashCode = 0;
  }

  return rlen;
}

sysuint_t __G_STRING::remove(__G_CHAR ch, uint cs, const Range& range)
{
  sysuint_t rstart;
  sysuint_t rlen;
  if (!fitToRange(*this, &rstart, &rlen, range)) return 0;

  Data* d = _d;
  sysuint_t length = d->length;
  sysuint_t tail;

  __G_CHAR* strBeg = d->data;
  __G_CHAR* strCur = strBeg + rstart;
  __G_CHAR* strEnd = strCur + rlen;
  __G_CHAR* destCur;

  if (cs == CaseSensitive)
  {
caseSensitive:
    while (strCur != strEnd)
    {
      if (*strCur == ch) goto caseSensitiveRemove;
      strCur++;
    }
    return 0;

caseSensitiveRemove:
    if (d->refCount.get() > 1)
    {
      rstart = strCur - strBeg;
      rlen = strEnd - strCur;

      if (!tryDetach()) return 0;
      d = _d;

      strBeg = d->data;
      strCur = strBeg + rstart;
      strEnd = strCur + rlen;
    }
    destCur = strCur;

    while (strCur != strEnd)
    {
      if (*strCur != ch) *destCur++ = *strCur;
      strCur++;
    }

    tail = length - (rstart + rlen);
    StringUtil::copy(destCur, strCur, tail);
  }
  else
  {
    __G_CHAR chLower = ch.toLower();
    __G_CHAR chUpper = ch.toUpper();
    if (chLower == chUpper) goto caseSensitive;

    while (strCur != strEnd)
    {
      if (*strCur == chLower || *strCur == chUpper) goto caseInsensitiveRemove;
      strCur++;
    }
    return 0;

caseInsensitiveRemove:
    if (d->refCount.get() > 1)
    {
      rstart = strCur - strBeg;
      rlen = strEnd - strCur;

      if (!tryDetach()) return 0;
      d = _d;

      strBeg = d->data;
      strCur = strBeg + rstart;
      strEnd = strCur + rlen;
    }
    destCur = strCur;

    while (strCur != strEnd)
    {
      if (*strCur != chLower && *strCur != chUpper) *destCur++ = *strCur;
      strCur++;
    }

    tail = length - (rstart + rlen);
    StringUtil::copy(destCur, strCur, tail);
  }

  d->length = (sysuint_t)(destCur - d->data);
  d->data[d->length] = 0;
  d->hashCode = 0;
  return length - d->length;
}

sysuint_t __G_STRING::remove(const __G_STRING& other, uint cs, const Range& range)
{
  // TODO
  return Error::NotImplemented;
}

sysuint_t __G_STRING::remove(const __G_STRINGFILTER& filter, uint cs, const Range& range)
{
  // TODO
  return Error::NotImplemented;
}

// ============================================================================
// [Fog::String - Replace]
// ============================================================================

err_t __G_STRING::_replaceMatches(
  Range* m, sysuint_t mcount,
  const __G_CHAR* after, sysuint_t alen)
{
  sysuint_t i;
  sysuint_t pos = 0;
  sysuint_t len = length();
  const __G_CHAR* cur = cData();

  // Get total count of characters we remove.
  sysuint_t mTotal = 0;
  for (i = 0; i < mcount; i++) mTotal += m[i].length;

  // Get total count of characters we add.
  sysuint_t aTotal = alen * mcount;

  // Get target length.
  sysuint_t lenAfter = len - mTotal + aTotal;

  Data* newd = Data::alloc(lenAfter, AllocCanFail);
  if (!newd) return Error::OutOfMemory;

  __G_CHAR* p = newd->data;
  sysuint_t remain = lenAfter;
  sysuint_t t;

  // Serialize
  for (i = 0; i < mcount; i++)
  {
    sysuint_t mstart = m[i].index;
    sysuint_t mlen = m[i].length;

    // Begin
    t = mstart - pos;
    if (t > remain) goto overflow;
    StringUtil::copy(p, cur + pos, t);
    p += t; remain -= t;

    // Replacement
    if (alen > remain) goto overflow;
    StringUtil::copy(p, after, alen);
    p += alen; remain -= alen;

    pos += mstart + mlen;
  }

  // Last piece of string
  t = length() - pos;
  if (t > remain) goto overflow;
  StringUtil::copy(p, cur + pos, t);
  p += t;

  // Be sure that calculated length is correct (if this assert fails, the
  // 'm' and 'mcount' parameters are incorrect).
  FOG_ASSERT(p == newd->data + lenAfter);

  newd->length = lenAfter;
  newd->data[lenAfter] = 0;

  AtomicBase::ptr_setXchg(&_d, newd)->deref();
  return Error::Ok;

overflow:
  newd->deref();
  return Error::Overflow;
}

err_t __G_STRING::replace(const Range& range, const __G_STRING& replacement)
{
  sysuint_t rstart;
  sysuint_t rlen;
  if (!fitToRange(*this, &rstart, &rlen, range)) return Error::Ok;


  // TODO
}

err_t __G_STRING::replace(__G_CHAR before, __G_CHAR after, 
  uint cs, const Range& range)
{
  sysuint_t rstart, rlen;
  if (!fitToRange(*this, &rstart, &rlen, range)) return Error::Ok;

  Data* d = _d;
  sysuint_t length = d->length;

  __G_CHAR* strCur = d->data + rstart;
  __G_CHAR* strEnd = strCur + rlen;

  if (cs == CaseSensitive)
  {
caseSensitive:
    while (strCur != strEnd)
    {
      if (*strCur == before) goto caseSensitiveReplace;
      strCur++;
    }
    return Error::Ok;

caseSensitiveReplace:
    if (d->refCount.get() > 1)
    {
      rstart = (sysuint_t)(strCur - d->data);
      rlen = (sysuint_t)(strEnd - strCur);

      if (!tryDetach()) return Error::OutOfMemory;
      d = _d;

      strCur = d->data + rstart;
      strEnd = strCur + rlen;
    }

    while (strCur != strEnd)
    {
      if (*strCur == before) *strCur = after;
      strCur++;
    }
  }
  else
  {
    __G_CHAR beforeLower = before.toLower();
    __G_CHAR beforeUpper = before.toUpper();

    if (beforeLower == beforeUpper) goto caseSensitive;

    while (strCur != strEnd)
    {
      if (*strCur == beforeLower || *strCur == beforeUpper) goto caseInsensitiveReplace;
      strCur++;
    }
    return Error::Ok;

caseInsensitiveReplace:
    if (d->refCount.get() > 1)
    {
      rstart = (sysuint_t)(strCur - d->data);
      rlen = (sysuint_t)(strEnd - strCur);

      if (!tryDetach()) return Error::OutOfMemory;
      d = _d;

      strCur = d->data + rstart;
      strEnd = strCur + rlen;
    }

    while (strCur != strEnd)
    {
      if (*strCur == beforeLower || *strCur == beforeUpper) *strCur = after;
      strCur++;
    }
  }

  return Error::Ok;
}

err_t __G_STRING::replace(const __G_STRING& before, const __G_STRING& after, 
  uint cs, const Range& range)
{
  __G_STRINGMATCHER matcher(before);
  return replace(matcher, after, cs, range);
}

err_t __G_STRING::replace(const __G_STRINGFILTER& before, const __G_STRING& after, 
  uint cs, const Range& range)
{
  Vector<Range> matches;

  // TODO
  return Error::NotImplemented;
}

// ============================================================================
// [Fog::String - Lower / Upper]
// ============================================================================

err_t __G_STRING::lower()
{
  Data* d = _d;

  __G_CHAR* strCur = d->data;
  __G_CHAR* strEnd = strCur + d->length;

  for (; strCur != strEnd; strCur++)
  {
    if (strCur->isUpper()) goto modify;
  }
  return Error::Ok;

modify:
  {
    sysuint_t n = (sysuint_t)(strCur - d->data);
    if (!tryDetach()) return Error::OutOfMemory;
    d = _d;

    strCur = d->data + n;
    strEnd = d->data + d->length;

    for (; strCur != strEnd; strCur++)
    {
      *strCur = strCur->toLower();
    }
  }
  d->hashCode = 0;
  return Error::Ok;
}

err_t __G_STRING::upper()
{
  Data* d = _d;

  __G_CHAR* strCur = d->data;
  __G_CHAR* strEnd = strCur + d->length;

  for (; strCur != strEnd; strCur++)
  {
    if (strCur->isLower()) goto modify;
  }
  return Error::Ok;

modify:
  {
    sysuint_t n = (sysuint_t)(strCur - d->data);
    if (!tryDetach()) return Error::OutOfMemory;
    d = _d;

    strCur = d->data + n;
    strEnd = d->data + d->length;

    for (; strCur != strEnd; strCur++)
    {
      *strCur = strCur->toUpper();
    }
  }
  d->hashCode = 0;
  return Error::Ok;
}

__G_STRING __G_STRING::lowered() const
{
  __G_STRING t(*this);
  t.lower();
  return t;
}

__G_STRING __G_STRING::uppered() const
{
  __G_STRING t(*this);
  t.upper();
  return t;
}

// ============================================================================
// [Fog::String - Whitespaces / Justification]
// ============================================================================

err_t __G_STRING::trim()
{
  Data* d = _d;
  sysuint_t len = d->length;

  if (!len) return Error::Ok;

  const __G_CHAR* strCur = d->data;
  const __G_CHAR* strEnd = strCur + len;

  while (strCur != strEnd   && strCur->isSpace()) strCur++;
  while (strCur != strEnd-- && strEnd->isSpace()) continue;

  if (strCur != d->data || ++strEnd != d->data + len)
  {
    len = (sysuint_t)(strEnd - strCur);
    if (d->refCount.get() > 1)
    {
      Data* newd = Data::alloc(len, AllocCanFail, strCur, len);
      if (!newd) return Error::OutOfMemory;
      AtomicBase::ptr_setXchg(&_d, newd)->deref();
    }
    else
    {
      if (strCur != d->data) StringUtil::move(d->data, strCur, len);
      d->length = len;
      d->data[len] = 0;
      d->hashCode = 0;
    }
  }

  return Error::Ok;
}

err_t __G_STRING::simplify()
{
  Data* d = _d;
  sysuint_t len = d->length;

  if (!len) return Error::Ok;

  const __G_CHAR* strBeg;
  const __G_CHAR* strCur = d->data;
  const __G_CHAR* strEnd = strCur + len;

  __G_CHAR* dest;

  while (strCur != strEnd   && strCur->isSpace()) strCur++;
  while (strCur != strEnd-- && strEnd->isSpace()) continue;

  strBeg = strCur;
  
  // Left and Right trim is complete...
  
  if (strCur != d->data || strEnd + 1 != d->data + len) goto simp;
  
  for (; strCur < strEnd; strCur++)
  {
    if (strCur[0].isSpace() && strCur[1].isSpace()) goto simp;
  }
  return Error::Ok;

simp:
  strCur = strBeg;
  strEnd++;

  // this is a bit messy, but I will describe how it works. We can't
  // change string that's shared between another one, so if reference
  // count is larger than 1 we will alloc a new D. At the end of this
  // function we will dereference the D and in case that string
  // is shared it wont be destroyed. Only one problem is that we need
  // to increase reference count if string is detached.
  if (d->refCount.get() > 1)
  {
    Data* newd = Data::alloc((sysuint_t)(strEnd - strCur), AllocCanFail);
    if (!newd) return Error::OutOfMemory;
    _d = newd;
  }
  else
  {
    d->refCount.inc();
  }

  dest = _d->data;

  do {
    if    (strCur != strEnd &&  strCur->isSpace()) *dest++ = ' ';
    while (strCur != strEnd &&  strCur->isSpace()) strCur++;
    while (strCur != strEnd && !strCur->isSpace()) *dest++ = *strCur++;
  } while (strCur != strEnd);

  _d->length = (dest - _d->data);
  _d->data[_d->length] = 0;
  _d->hashCode = 0;

  d->deref();

  return Error::Ok;
}

err_t __G_STRING::truncate(sysuint_t n)
{
  Data* d = _d;
  if (d->length <= n) return Error::Ok;

  if (d->refCount.get() > 1)
  {
    Data* newd = Data::alloc(n, AllocCanFail, d->data, n);
    if (!newd) return Error::OutOfMemory;
    AtomicBase::ptr_setXchg(&_d, newd)->deref();
  }
  else
  {
    d->length = n;
    d->data[d->length] = 0;
    d->hashCode = 0;
  }
  return Error::Ok;
}

err_t __G_STRING::justify(sysuint_t n, __G_CHAR fill, uint32_t flags)
{
  Data* d = _d;
  sysuint_t length = d->length;

  if (n <= length) return Error::Ok;

  sysuint_t t = n - length;
  sysuint_t left = 0;
  sysuint_t right = 0;

  if ((flags & CenterJustify) == CenterJustify)
  {
    left = t >> 1;
    right = t - left;
  }
  else if ((flags & LeftJustify) == LeftJustify)
  {
    right = t;
  }
  else if ((flags & RightJustify) == RightJustify)
  {
    left = t;
  }

  err_t err;
  if ( (err = reserve(n)) ) return err;
  if ( (err = prepend(fill, left)) ) return err;
  return append(fill, right);
}

__G_STRING __G_STRING::trimmed() const
{
  __G_STRING t(*this);
  t.trim();
  return t;
}

__G_STRING __G_STRING::simplified() const
{
  __G_STRING t(*this);
  t.simplify();
  return t;
}

__G_STRING __G_STRING::truncated(sysuint_t n) const
{
  __G_STRING t(*this);
  t.truncate(n);
  return t;
}

__G_STRING __G_STRING::justified(sysuint_t n, __G_CHAR fill, uint32_t flags) const
{
  __G_STRING t(*this);
  t.justify(n, fill, flags);
  return t;
}

// ============================================================================
// [Fog::String - Split]
// ============================================================================

Vector<__G_STRING> __G_STRING::split(__G_CHAR ch, uint splitBehavior, uint cs) const
{
  Vector<__G_STRING> result;
  Data* d = _d;

  if (d->length == 0) return result;

  const __G_CHAR* strBeg = d->data;
  const __G_CHAR* strCur = strBeg;
  const __G_CHAR* strEnd = strCur + d->length;

  if (cs == CaseSensitive)
  {
__caseSensitive:
    for (;;)
    {
      if (strCur == strEnd || *strCur == ch)
      {
        sysuint_t splitLength = (sysuint_t)(strCur - strBeg);
        if ((splitLength == 0 && splitBehavior == KeepEmptyParts) || splitLength != 0)
        {
          result.append(__G_STRING(__G_STRING_STUB(strBeg, splitLength)));
        }
        if (strCur == strEnd) break;
        strBeg = ++strCur;
      }
      else
        strCur++;
    }
  }
  else
  {
    __G_CHAR cLower = ch.toLower();
    __G_CHAR cUpper = ch.toUpper();
    if (cLower == cUpper) goto __caseSensitive;

    for (;;)
    {
      if (strCur == strEnd || *strCur == cLower || *strCur == cUpper)
      {
        sysuint_t splitLength = (sysuint_t)(strCur - strBeg);
        if ((splitLength == 0 && splitBehavior == KeepEmptyParts) || splitLength != 0)
        {
          result.append(__G_STRING(__G_STRING_STUB(strBeg, splitLength)));
        }
        if (strCur == strEnd) break;
        strBeg = ++strCur;
      }
      else
        strCur++;
    }
  }
  return result;
}

Vector<__G_STRING> __G_STRING::split(const __G_STRING& pattern, uint splitBehavior, uint cs) const
{
  sysuint_t plen = pattern.length();

  if (!plen)
  {
    Vector<__G_STRING> result;
    result.append(*this);
    return result;
  }
  else if (plen == 1)
  {
    return split(pattern.at(0), splitBehavior, cs);
  }
  else
  {
    __G_STRINGMATCHER matcher(pattern);
    return split(matcher, splitBehavior, cs);
  }
}

Vector<__G_STRING> __G_STRING::split(const __G_STRINGFILTER& filter, uint splitBehavior, uint cs) const
{
  Vector<__G_STRING> result;
  Data* d = _d;

  sysuint_t length = d->length;

  const __G_CHAR* strCur = d->data;
  const __G_CHAR* strEnd = strCur + length;

  for (;;)
  {
    sysuint_t remain = (sysuint_t)(strEnd - strCur);
    Range m = filter.match(strCur, remain, cs, Range(0, remain));
    sysuint_t splitLength = (m.index != InvalidIndex) ? m.index : remain;

    if ((splitLength == 0 && splitBehavior == KeepEmptyParts) || splitLength != 0)
      result.append(__G_STRING(__G_STRING_STUB(strCur, splitLength)));

    if (m.index == InvalidIndex) break;

    strCur += m.index;
    strCur += m.length;
  }

  return result;
}

// ============================================================================
// [Fog::String - Substring]
// ============================================================================

__G_STRING __G_STRING::substring(const Range& range) const
{
  __G_STRING ret;
  sysuint_t i, len;

  if (fitToRange(*this, &i, &len, range))
    ret.set(__G_STRING_STUB(cData() + i, len));

  return ret;
}

// ============================================================================
// [Fog::String - Conversion]
// ============================================================================

err_t __G_STRING::atob(bool* dst, int base, sysuint_t* end, uint32_t* parserFlags) const
{
  return StringUtil::atob(cData(), length(), dst, end, parserFlags);
}

err_t __G_STRING::atoi8(int8_t* dst, int base, sysuint_t* end, uint32_t* parserFlags) const
{
  return StringUtil::atoi8(cData(), length(), dst, base, end, parserFlags);
}

err_t __G_STRING::atou8(uint8_t* dst, int base, sysuint_t* end, uint32_t* parserFlags) const
{
  return StringUtil::atou8(cData(), length(), dst, base, end, parserFlags);
}

err_t __G_STRING::atoi16(int16_t* dst, int base, sysuint_t* end, uint32_t* parserFlags) const
{
  return StringUtil::atoi16(cData(), length(), dst, base, end, parserFlags);
}

err_t __G_STRING::atou16(uint16_t* dst, int base, sysuint_t* end, uint32_t* parserFlags) const
{
  return StringUtil::atou16(cData(), length(), dst, base, end, parserFlags);
}

err_t __G_STRING::atoi32(int32_t* dst, int base, sysuint_t* end, uint32_t* parserFlags) const
{
  return StringUtil::atoi32(cData(), length(), dst, base, end, parserFlags);
}

err_t __G_STRING::atou32(uint32_t* dst, int base, sysuint_t* end, uint32_t* parserFlags) const
{
  return StringUtil::atou32(cData(), length(), dst, base, end, parserFlags);
}

err_t __G_STRING::atoi64(int64_t* dst, int base, sysuint_t* end, uint32_t* parserFlags) const
{
  return StringUtil::atoi64(cData(), length(), dst, base, end, parserFlags);
}

err_t __G_STRING::atou64(uint64_t* dst, int base, sysuint_t* end, uint32_t* parserFlags) const
{
  return StringUtil::atou64(cData(), length(), dst, base, end, parserFlags);
}

#if __G_SIZE == 1
err_t __G_STRING::atof(float* dst, sysuint_t* end, uint32_t* parserFlags) const
{
  return StringUtil::atof(cData(), length(), dst, 
    __G_CHAR('.'), end, parserFlags);
}

err_t __G_STRING::atod(double* dst, sysuint_t* end, uint32_t* parserFlags) const
{
  return StringUtil::atod(cData(), length(), dst,
    __G_CHAR('.'), end, parserFlags);
}
#else
err_t __G_STRING::atof(float* dst, const Locale* locale, sysuint_t* end, uint32_t* parserFlags) const
{
  if (locale == NULL) locale = &Locale::posix();

  return StringUtil::atof(cData(), length(), dst, 
    __G_CHAR(locale->decimalPoint().ch()), end, parserFlags);
}

err_t __G_STRING::atod(double* dst, const Locale* locale, sysuint_t* end, uint32_t* parserFlags) const
{
  if (locale == NULL) locale = &Locale::posix();

  return StringUtil::atod(cData(), length(), dst,
    __G_CHAR(locale->decimalPoint().ch()), end, parserFlags);
}
#endif

// ============================================================================
// [Fog::String - Contains]
// ============================================================================

bool __G_STRING::contains(__G_CHAR ch,
  uint cs, const Range& range) const
{
  sysuint_t i, len;

  if (fitToRange(*this, &i, &len, range))
    return StringUtil::indexOf(cData() + i, len, ch, cs) != InvalidIndex;
  else
    return false;
}

bool __G_STRING::contains(const __G_STRING& pattern,
  uint cs, const Range& range) const
{
  return indexOf(pattern, cs, range) != InvalidIndex;
}

bool __G_STRING::contains(const __G_STRINGFILTER& filter, 
  uint cs, const Range& range) const
{
  Range m = filter.indexOf(cData(), length(), cs, range);
  return m.index != InvalidIndex;
}

// ============================================================================
// [Fog::String - CountOf]
// ============================================================================

sysuint_t __G_STRING::countOf(__G_CHAR ch,
  uint cs, const Range& range) const
{
  sysuint_t i, len;

  if (fitToRange(*this, &i, &len, range))
    return StringUtil::countOf(cData() + i, len, ch, cs);
  else
    return 0;
}

sysuint_t __G_STRING::countOf(const __G_STRING& pattern,
  uint cs, const Range& range) const
{
  if (pattern.isEmpty())
  {
    return 0;
  }
  else if (pattern.length() == 1)
  {
    return countOf(pattern.at(0), cs, range);
  }
  else
  {
    // TODO
  } 
}

sysuint_t __G_STRING::countOf(const __G_STRINGFILTER& filter,
  uint cs, const Range& range) const
{
  // TODO
  return 0;
}

// ============================================================================
// [Fog::String - IndexOf / LastIndexOf]
// ============================================================================

sysuint_t __G_STRING::indexOf(__G_CHAR ch,
  uint cs, const Range& range) const
{
  sysuint_t start, len;
  if (!fitToRange(*this, &start, &len, range)) return InvalidIndex;

  sysuint_t i = StringUtil::indexOf(cData() + start, len, ch, cs);
  return i != InvalidIndex ? i + start : i;
}

sysuint_t __G_STRING::indexOf(const __G_STRING& pattern,
  uint cs, const Range& range) const
{
  if (pattern.isEmpty())
  {
    return InvalidIndex;
  }
  else if (pattern.length() == 1)
  {
    return indexOf(pattern.at(0), cs, range);
  }
  else
  {
    // TODO
  }
}

sysuint_t __G_STRING::indexOf(const __G_STRINGFILTER& filter,
  uint cs, const Range& range) const
{
  sysuint_t start, len;
  if (!fitToRange(*this, &start, &len, range)) return InvalidIndex;

  Range m = filter.match(cData(), length(), cs, Range(start, len));
  return m.index;
}

sysuint_t __G_STRING::lastIndexOf(__G_CHAR ch,
  uint cs, const Range& range) const
{
  sysuint_t start, len;
  if (!fitToRange(*this, &start, &len, range)) return InvalidIndex;

  sysuint_t i = StringUtil::lastIndexOf(cData() + start, len, ch, cs);
  return i != InvalidIndex ? i + start : i;
}

sysuint_t __G_STRING::lastIndexOf(const __G_STRING& pattern,
  uint cs, const Range& range) const
{
  // TODO
  return Error::NotImplemented;
}

sysuint_t __G_STRING::lastIndexOf(const __G_STRINGFILTER& filter,
  uint cs, const Range& range) const
{
  // TODO
  return Error::NotImplemented;
}

// ============================================================================
// [Fog::String - StartsWith / EndsWith]
// ============================================================================

#if __G_SIZE == 1
bool __G_STRING::startsWith(const Stub8& str, uint cs) const
{
  const Char8* s = str.str();
  sysuint_t len = str.length();
  if (len == DetectLength) len = StringUtil::len(s);

  return length() >= len && StringUtil::eq(cData(), s, len, cs);
}
#else
bool __G_STRING::startsWith(const void* data, sysuint_t size, const TextCodec& tc, uint cs) const
{
  err_t err;
  __G_TEMPORARYSTRING<TemporaryLength> other;
#if __G_SIZE == 2
  if ( (err = tc.appendToUtf16(other, Stub8((const char*)data, size))) )
    return err;
#else
  if ( (err = tc.appendToUtf32(other, Stub8((const char*)data, size))) )
    return err;
#endif // __G_SIZE
  return startsWith(other, cs);
}

bool __G_STRING::startsWith(const StubAscii8& str, uint cs) const
{
  const Char8* s = str.str();
  sysuint_t len = str.length();
  if (len == DetectLength) len = StringUtil::len(s);

  return length() >= len && StringUtil::eq(cData(), s, len, cs);
}

bool __G_STRING::startsWith(const StubLocal8& str, uint cs) const
{
  const Char8* s = str.str();
  sysuint_t len = str.length();
  if (len == DetectLength) len = StringUtil::len(s);

  return startsWith((const void*)s, len, TextCodec::local8(), cs);
}

bool __G_STRING::startsWith(const StubUtf8& str, uint cs) const
{
  const Char8* s = str.str();
  sysuint_t len = str.length();
  if (len == DetectLength) len = StringUtil::len(s);

  return startsWith((const void*)s, len, TextCodec::utf8(), cs);
}

bool __G_STRING::startsWith(const StubUtf16& str, uint cs) const
{
#if __G_SIZE == 2
  const Char16* s = str.str();
  sysuint_t len = str.length();
  if (len == DetectLength) len = StringUtil::len(s);

  return length() >= len && StringUtil::eq(cData(), s, len, cs);
#else
  const Char16* s = str.str();
  sysuint_t len = str.length();
  if (len == DetectLength) len = StringUtil::len(s);

  return startsWith((const void*)s, len << 1, TextCodec::utf16(), cs);
#endif // __G_SIZE
}

bool __G_STRING::startsWith(const StubUtf32& str, uint cs) const
{
#if __G_SIZE == 2
  const Char32* s = str.str();
  sysuint_t len = str.length();
  if (len == DetectLength) len = StringUtil::len(s);

  return startsWith((const void*)s, len << 2, TextCodec::utf32(), cs);
#else
  const Char32* s = str.str();
  sysuint_t len = str.length();
  if (len == DetectLength) len = StringUtil::len(s);

  return length() >= len && StringUtil::eq(cData(), s, len, cs);
#endif // __G_SIZE
}
#endif // __G_SIZE

bool __G_STRING::startsWith(const __G_STRING& str, uint cs) const
{
  return length() >= str.length() && 
    StringUtil::eq(cData(), str.cData(), str.length(), cs);
}

bool __G_STRING::startsWith(const __G_STRINGFILTER& filter, uint cs) const
{
  sysuint_t flen = filter.length();

  if (flen == InvalidIndex) flen = length();
  return filter.match(cData(), length(), cs, Range(0, flen)).index == 0;
}

#if __G_SIZE == 1
bool __G_STRING::endsWith(const Stub8& str, uint cs) const
{
  const Char8* s = str.str();
  sysuint_t len = str.length();
  if (len == DetectLength) len = StringUtil::len(s);

  return length() >= len && StringUtil::eq(cData() + length() - len, s, len, cs);
}
#else
bool __G_STRING::endsWith(const void* data, sysuint_t size, const TextCodec& tc, uint cs) const
{
  err_t err;
  __G_TEMPORARYSTRING<TemporaryLength> other;
#if __G_SIZE == 2
  if ( (err = tc.appendToUtf16(other, Stub8((const char*)data, size))) )
    return err;
#else
  if ( (err = tc.appendToUtf32(other, Stub8((const char*)data, size))) )
    return err;
#endif // __G_SIZE
  return endsWith(other, cs);
}

bool __G_STRING::endsWith(const StubAscii8& str, uint cs) const
{
  const Char8* s = str.str();
  sysuint_t len = str.length();
  if (len == DetectLength) len = StringUtil::len(s);

  return length() >= len && StringUtil::eq(cData() + length() - len, s, len, cs);
}

bool __G_STRING::endsWith(const StubLocal8& str, uint cs) const
{
  const Char8* s = str.str();
  sysuint_t len = str.length();
  if (len == DetectLength) len = StringUtil::len(s);

  return endsWith((const void*)s, len, TextCodec::local8(), cs);
}

bool __G_STRING::endsWith(const StubUtf8& str, uint cs) const
{
  const Char8* s = str.str();
  sysuint_t len = str.length();
  if (len == DetectLength) len = StringUtil::len(s);

  return endsWith((const void*)s, len, TextCodec::utf8(), cs);
}

bool __G_STRING::endsWith(const StubUtf16& str, uint cs) const
{
#if __G_SIZE == 2
  const Char16* s = str.str();
  sysuint_t len = str.length();
  if (len == DetectLength) len = StringUtil::len(s);

  return length() >= len && 
    StringUtil::eq(cData() + length() - len, s, len, cs);
#else
  const Char16* s = str.str();
  sysuint_t len = str.length();
  if (len == DetectLength) len = StringUtil::len(s);

  return endsWith((const void*)s, len << 1, TextCodec::utf16(), cs);
#endif // __G_SIZE
}

bool __G_STRING::endsWith(const StubUtf32& str, uint cs) const
{
#if __G_SIZE == 2
  const Char32* s = str.str();
  sysuint_t len = str.length();
  if (len == DetectLength) len = StringUtil::len(s);

  return endsWith((const void*)s, len << 2, TextCodec::utf32(), cs);
#else
  const Char32* s = str.str();
  sysuint_t len = str.length();
  if (len == DetectLength) len = StringUtil::len(s);

  return length() >= len && 
    StringUtil::eq(cData() + length() - len, s, len, cs);
#endif // __G_SIZE
}
#endif // __G_SIZE

bool __G_STRING::endsWith(const __G_STRING& str, uint cs) const
{
  return length() >= str.length() && 
    StringUtil::eq(cData() + length() - str.length(), str.cData(), str.length(), cs);
}

bool __G_STRING::endsWith(const __G_STRINGFILTER& filter, uint cs) const
{
  sysuint_t flen = filter.length();

  if (flen == InvalidIndex)
  {
    sysuint_t i = 0;
    sysuint_t len = length();

    for (;;)
    {
      Range r = filter.match(cData(), len, cs, Range(i));
      if (r.index == InvalidIndex) return false;
      if (r.index + r.length == len) return true;

      i = r.index + 1;
    }
  }
  else
  {
    return flen <= length() &&
      filter.match(
        cData() + length() - flen, length(), cs, Range(0, flen)).index == 0;
  }
}

// ============================================================================
// [Fog::String - ByteSwap]
// ============================================================================

#if __G_SIZE > 1
err_t __G_STRING::bswap()
{
  if (length() == 0) return Error::Ok;
  if (!tryDetach()) return Error::OutOfMemory;

  sysuint_t i, len = length();
  __G_CHAR* ch = _d->data;
  for (i = 0; i < len; i++) ch[i].bswap();

  _d->hashCode = 0;
  return Error::Ok;
}
#endif // __G_SIZE > 1

// ============================================================================
// [Fog::String - Comparison]
// ============================================================================

bool __G_STRING::eq(const __G_STRING* a, const __G_STRING* b)
{
  sysuint_t alen = a->length();
  sysuint_t blen = b->length();
  if (alen != blen) return false;

  return StringUtil::eq(a->cData(), b->cData(), alen, CaseSensitive);
}

bool __G_STRING::ieq(const __G_STRING* a, const __G_STRING* b)
{
  sysuint_t alen = a->length();
  sysuint_t blen = b->length();
  if (alen != blen) return false;

  return StringUtil::eq(a->cData(), b->cData(), alen, CaseInsensitive);
}

int __G_STRING::compare(const __G_STRING* a, const __G_STRING* b)
{
  sysuint_t aLen = a->length();
  sysuint_t bLen = b->length();
  const __G_CHAR* aCur = a->cData();
  const __G_CHAR* bCur = b->cData();
  const __G_CHAR* aEnd = aCur + aLen;

  int c;

  if (bLen < aLen) aEnd = aCur + bLen;

  for (; aCur != aEnd; aCur++, bCur++)
    if ((c = (int)aCur->ch() - (int)bCur->ch())) return c;

  return (int)((sysint_t)aLen - (sysint_t)bLen);
}

int __G_STRING::icompare(const __G_STRING* a, const __G_STRING* b)
{
  sysuint_t aLen = a->length();
  sysuint_t bLen = b->length();
  const __G_CHAR* aCur = a->cData();
  const __G_CHAR* bCur = b->cData();
  const __G_CHAR* aEnd = aCur + aLen;

  int c;

  if (bLen < aLen) aEnd = aCur + bLen;

  for (; aCur != aEnd; aCur++, bCur++)
    if ((c = (int)aCur->toLower().ch() - (int)bCur->toLower().ch())) return c;

  return (int)((sysint_t)aLen - (sysint_t)bLen);
}

#if __G_SIZE == 1
bool __G_STRING::eq(const Stub8& other, uint cs) const
{
  sysuint_t len = other.length();
  if (len == DetectLength)
  {
    const Char8* aCur = cData();
    const Char8* bCur = other.str();

    if (cs == CaseSensitive)
    {
      for (sysuint_t i = length(); i; i--, aCur++, bCur++)
      {
        if (FOG_UNLIKELY(bCur->ch() == 0)) return false;
        if (*aCur != *bCur) return false;
      }
    }
    else
    {
      for (sysuint_t i = length(); i; i--, aCur++, bCur++)
      {
        if (FOG_UNLIKELY(bCur->ch() == 0)) return false;
        if (aCur->toLower().ch() != bCur->toLower().ch()) return false;
      }
    }
    return bCur->ch() == 0;
  }
  else
    return length() == len && StringUtil::eq(cData(), other.str(), len, cs);
}
#else
bool __G_STRING::eq(const StubAscii8& other, uint cs) const
{
  sysuint_t len = other.length();
  if (len == DetectLength)
  {
    const __G_CHAR* aCur = cData();
    const Char8* bCur = other.str();

    if (cs == CaseSensitive)
    {
      for (sysuint_t i = length(); i; i--, aCur++, bCur++)
      {
        if (FOG_UNLIKELY(bCur->ch() == 0)) return false;
        if (*aCur != *bCur) return false;
      }
    }
    else
    {
      for (sysuint_t i = length(); i; i--, aCur++, bCur++)
      {
        if (FOG_UNLIKELY(bCur->ch() == 0)) return false;
        if (aCur->toLower().ch() != bCur->toLower().ch()) return false;
      }
    }
    return bCur->ch() == 0;
  }
  else
    return length() == len && StringUtil::eq(cData(), other.str(), len, cs);
}

bool __G_STRING::eq(const StubLocal8& other, uint cs) const
{
  __G_TEMPORARYSTRING<TemporaryLength> t;
  if (t.set(other) != Error::Ok) return false;
  return eq(t, cs);
}

bool __G_STRING::eq(const StubUtf8& other, uint cs) const
{
  __G_TEMPORARYSTRING<TemporaryLength> t;
  if (t.set(other) != Error::Ok) return false;
  return eq(t, cs);
}

bool __G_STRING::eq(const StubUtf16& other, uint cs) const
{
#if __G_SIZE == 2
  sysuint_t len = other.length();
  if (len == DetectLength)
  {
    const Char16* aCur = cData();
    const Char16* bCur = other.str();

    if (cs == CaseSensitive)
    {
      for (sysuint_t i = length(); i; i--, aCur++, bCur++)
      {
        if (FOG_UNLIKELY(bCur->ch() == 0)) return false;
        if (*aCur != *bCur) return false;
      }
    }
    else
    {
      for (sysuint_t i = length(); i; i--, aCur++, bCur++)
      {
        if (FOG_UNLIKELY(bCur->ch() == 0)) return false;
        if (aCur->toLower().ch() != bCur->toLower().ch()) return false;
      }
    }
    return bCur->ch() == 0;
  }
  else
    return length() == len && StringUtil::eq(cData(), other.str(), len, cs);
#else
  __G_TEMPORARYSTRING<TemporaryLength> t;
  if (t.set(other) != Error::Ok) return false;
  return eq(t, cs);
#endif
}

bool __G_STRING::eq(const StubUtf32& other, uint cs) const
{
#if __G_SIZE == 2
  __G_TEMPORARYSTRING<TemporaryLength> t;
  if (t.set(other) != Error::Ok) return false;
  return eq(t, cs);
#else
  sysuint_t len = other.length();
  if (len == DetectLength)
  {
    const Char32* aCur = cData();
    const Char32* bCur = other.str();

    if (cs == CaseSensitive)
    {
      for (sysuint_t i = length(); i; i--, aCur++, bCur++)
      {
        if (FOG_UNLIKELY(*bCur == 0)) return false;
        if (*aCur != *bCur) return false;
      }
    }
    else
    {
      for (sysuint_t i = length(); i; i--, aCur++, bCur++)
      {
        if (FOG_UNLIKELY(*bCur == 0)) return false;
        if (aCur->toLower().ch() != bCur->toLower().ch()) return false;
      }
    }
    return bCur->ch() == 0;
  }
  else
    return length() == len && StringUtil::eq(cData(), other.str(), len, cs);
#endif
}
#endif

bool __G_STRING::eq(const __G_STRING& other, uint cs) const
{
  return length() == other.length() && 
    StringUtil::eq(cData(), other.cData(), length(), cs);
}

#if __G_SIZE == 1
int __G_STRING::compare(const Stub8& other, uint cs) const
{
  sysuint_t aLen = length();
  sysuint_t bLen = other.length();
  const Char8* aCur = cData();
  const Char8* aEnd = aCur + aLen;
  const Char8* bCur = other.str();

  int c;

  if (bLen == DetectLength)
  {
    if (cs == CaseSensitive)
    {
      for (;;)
      {
        if (FOG_UNLIKELY(aCur == aEnd)) return *bCur ? -1 : 0;
        if ((c = (int)aCur->ch() - (int)bCur->ch())) return c;
        aCur++;
        bCur++;
      }
    }
    else
    {
      for (;;)
      {
        if (FOG_UNLIKELY(aCur == aEnd)) return *bCur ? -1 : 0;
        if ((c = (int)aCur->toLower().ch() - (int)bCur->toLower().ch())) return c;
        aCur++;
        bCur++;
      }
    }
  }
  else
  {
    if (bLen < aLen) aEnd = aCur + bLen;

    if (cs == CaseSensitive)
    {
      for (; aCur != aEnd; aCur++, bCur++)
        if ((c = (int)aCur->ch() - (int)bCur->ch())) return c;
    }
    else
    {
      for (; aCur != aEnd; aCur++, bCur++)
        if ((c = (int)aCur->toLower().ch() - (int)bCur->toLower().ch())) return c;
    }

    return (int)((sysint_t)aLen - (sysint_t)bLen);
  }
}
#else
int __G_STRING::compare(const StubAscii8& other, uint cs) const
{
  sysuint_t aLen = length();
  sysuint_t bLen = other.length();
  const __G_CHAR* aCur = cData();
  const __G_CHAR* aEnd = aCur + aLen;
  const Char8* bCur = other.str();

  int c;

  if (bLen == DetectLength)
  {
    if (cs == CaseSensitive)
    {
      for (;;)
      {
        if (FOG_UNLIKELY(aCur == aEnd)) return *bCur ? -1 : 0;
        if ((c = (int)aCur->ch() - (int)bCur->ch())) return c;
        aCur++;
        bCur++;
      }
    }
    else
    {
      for (;;)
      {
        if (FOG_UNLIKELY(aCur == aEnd)) return *bCur ? -1 : 0;
        if ((c = (int)aCur->toLower().ch() - (int)bCur->toLower().ch())) return c;
        aCur++;
        bCur++;
      }
    }
  }
  else
  {
    if (bLen < aLen) aEnd = aCur + bLen;

    if (cs == CaseSensitive)
    {
      for (; aCur != aEnd; aCur++, bCur++)
        if ((c = (int)aCur->ch() - (int)bCur->ch())) return c;
    }
    else
    {
      for (; aCur != aEnd; aCur++, bCur++)
        if ((c = (int)aCur->toLower().ch() - (int)bCur->toLower().ch())) return c;
    }

    return (int)((sysint_t)aLen - (sysint_t)bLen);
  }
}

int __G_STRING::compare(const StubLocal8& other, uint cs) const
{
  __G_TEMPORARYSTRING<TemporaryLength> t;
  t.set(other);
  return compare(t, cs);
}

int __G_STRING::compare(const StubUtf8& other, uint cs) const
{
  __G_TEMPORARYSTRING<TemporaryLength> t;
  t.set(other);
  return compare(t, cs);
}

int __G_STRING::compare(const StubUtf16& other, uint cs) const
{
#if __G_SIZE == 2
  sysuint_t aLen = length();
  sysuint_t bLen = other.length();
  const Char16* aCur = cData();
  const Char16* aEnd = aCur + aLen;
  const Char16* bCur = other.str();

  int c;

  if (bLen == DetectLength)
  {
    if (cs == CaseSensitive)
    {
      for (;;)
      {
        if (FOG_UNLIKELY(aCur == aEnd)) return *bCur ? -1 : 0;
        if ((c = (int)aCur->ch() - (int)bCur->ch())) return c;
        aCur++;
        bCur++;
      }
    }
    else
    {
      for (;;)
      {
        if (FOG_UNLIKELY(aCur == aEnd)) return *bCur ? -1 : 0;
        if ((c = (int)aCur->toLower().ch() - (int)bCur->toLower().ch())) return c;
        aCur++;
        bCur++;
      }
    }
  }
  else
  {
    if (bLen < aLen) aEnd = aCur + bLen;

    if (cs == CaseSensitive)
    {
      for (; aCur != aEnd; aCur++, bCur++)
        if ((c = (int)aCur->ch() - (int)bCur->ch())) return c;
    }
    else
    {
      for (; aCur != aEnd; aCur++, bCur++)
        if ((c = (int)aCur->toLower().ch() - (int)bCur->toLower().ch())) return c;
    }

    return (int)((sysint_t)aLen - (sysint_t)bLen);
  }
#else
  __G_TEMPORARYSTRING<TemporaryLength> t;
  t.set(other);
  return compare(t, cs);
#endif
}

int __G_STRING::compare(const StubUtf32& other, uint cs) const
{
#if __G_SIZE == 2
  __G_TEMPORARYSTRING<TemporaryLength> t;
  t.set(other);
  return compare(t, cs);
#else
  sysuint_t aLen = length();
  sysuint_t bLen = other.length();
  const Char32* aCur = cData();
  const Char32* aEnd = aCur + aLen;
  const Char32* bCur = other.str();

  int c;

  if (bLen == DetectLength)
  {
    if (cs == CaseSensitive)
    {
      for (;;)
      {
        if (FOG_UNLIKELY(aCur == aEnd)) return *bCur ? -1 : 0;
        if ((c = (int)aCur->ch() - (int)bCur->ch())) return c;
        aCur++;
        bCur++;
      }
    }
    else
    {
      for (;;)
      {
        if (FOG_UNLIKELY(aCur == aEnd)) return *bCur ? -1 : 0;
        if ((c = (int)aCur->toLower().ch() - (int)bCur->toLower().ch())) return c;
        aCur++;
        bCur++;
      }
    }
  }
  else
  {
    if (bLen < aLen) aEnd = aCur + bLen;

    if (cs == CaseSensitive)
    {
      for (; aCur != aEnd; aCur++, bCur++)
        if ((c = (int)aCur->ch() - (int)bCur->ch())) return c;
    }
    else
    {
      for (; aCur != aEnd; aCur++, bCur++)
        if ((c = (int)aCur->toLower().ch() - (int)bCur->toLower().ch())) return c;
    }

    return (int)((sysint_t)aLen - (sysint_t)bLen);
  }
#endif
}
#endif

int __G_STRING::compare(const __G_STRING& other, uint cs) const
{
  sysuint_t aLen = length();
  sysuint_t bLen = other.length();
  const __G_CHAR* aCur = cData();
  const __G_CHAR* aEnd = aCur + aLen;
  const __G_CHAR* bCur = other.cData();

  int c;
  if (bLen < aLen) aEnd = aCur + bLen;

  if (cs == CaseSensitive)
  {
    for (; aCur != aEnd; aCur++, bCur++)
      if ((c = (int)aCur->ch() - (int)bCur->ch())) return c;
  }
  else
  {
    for (; aCur != aEnd; aCur++, bCur++)
      if ((c = (int)aCur->toLower().ch() - (int)bCur->toLower().ch())) return c;
  }

  return (int)((sysint_t)aLen - (sysint_t)bLen);
}

// ============================================================================
// [Fog::String::Encoding]
// ============================================================================

#if __G_SIZE == 1
// Utf8
err_t __G_STRING::utf8Validate()
{
  // TODO
  return Error::NotImplemented;
}

err_t __G_STRING::utf8Characters(sysuint_t* dst)
{
  // TODO
  return Error::NotImplemented;
}

// Encode
err_t __G_STRING::encode(String8& dst, const TextCodec& dst_tc, const TextCodec& src_tc) const
{
  err_t err;
  TemporaryString32<TemporaryLength> dst32;

  if ((err = src_tc.appendToUtf32(dst32, Stub8(*this))) ||
      (err = dst_tc.appendFromUtf32(dst, dst32)))
  {
    return err;
  }

  return Error::Ok;
}

err_t __G_STRING::toAscii8(String8& dst, const TextCodec& src_tc) const
{
  return encode(dst, TextCodec::ascii8(), src_tc);
}

err_t __G_STRING::toLocal8(String8& dst, const TextCodec& src_tc) const
{
  return encode(dst, TextCodec::local8(), src_tc);
}

err_t __G_STRING::toUtf8(String8& dst, const TextCodec& src_tc) const
{
  return encode(dst, TextCodec::utf8(), src_tc);
}

err_t __G_STRING::toUtf16(String16& dst, const TextCodec& src_tc) const
{
  return src_tc.toUtf16(dst, *this); 
}

err_t __G_STRING::toUtf32(String32& dst, const TextCodec& src_tc) const
{
  return src_tc.toUtf32(dst, *this); 
}
#else
#if __G_SIZE == 2
// Utf16
err_t __G_STRING::utf16Validate(sysuint_t* invalid) const
{
  // TODO
  return Error::NotImplemented;
}

err_t __G_STRING::utf16Length(sysuint_t* len)
{
  // TODO
  return Error::NotImplemented;
}
#endif // __G_SIZE == 2

// Encode
err_t __G_STRING::encode(String8& dst, const TextCodec& tc)
{
  return dst.set(*this, tc);
}

err_t __G_STRING::toAscii8(String8& dst) const
{
  return dst.set(*this, TextCodec::ascii8());
}

err_t __G_STRING::toLocal8(String8& dst) const
{
  return dst.set(*this, TextCodec::local8());
}

err_t __G_STRING::toUtf8(String8& dst) const
{
  return dst.set(*this, TextCodec::utf8());
}

err_t __G_STRING::toUtf16(String16& dst) const
{
  return dst.set(*this);
}

err_t __G_STRING::toUtf32(String32& dst) const
{
  return dst.set(*this);
}

err_t __G_STRING::toStringW(StringW& dst) const
{
  return dst.set(*this);
}
#endif // __G_SIZE

// ============================================================================
// [Fog::String::FileSystem]
// ============================================================================

err_t __G_STRING::slashesToPosix()
{
  return replace(__G_CHAR('\\'), __G_CHAR('/'));
}

err_t __G_STRING::slashesToWin()
{
  return replace(__G_CHAR('/'), __G_CHAR('\\'));
}

// ============================================================================
// [Fog::String::Hash]
// ============================================================================

uint32_t __G_STRING::toHashCode() const
{
  uint32_t h = _d->hashCode;
  if (h) return h;

  return (_d->hashCode = hashData((const void*)cData(), length() << 2));
}

// ============================================================================
// [Fog::String::Data]
// ============================================================================

__G_STRING::Data* __G_STRING::Data::ref() const
{
  return refInline();
}

void __G_STRING::Data::deref()
{
  derefInline();
}

__G_STRING::Data* __G_STRING::Data::adopt(
  void* address, sysuint_t capacity)
{
  if (capacity == 0) return __G_STRING::sharedNull->refAlways();

  Data* d = (Data*)address;
  d->refCount.init(1);
  d->flags = 0;
  d->hashCode = 0;
  d->length = 0;
  d->capacity = capacity;
  d->data[0] = 0;

  return d;
}

__G_STRING::Data* __G_STRING::Data::adopt(
  void* address, sysuint_t capacity, int allocPolicy,
  const Char8* str, sysuint_t length)
{
  if (length == DetectLength) length = StringUtil::len(str);

  if (length <= capacity)
  {
    Data* d = adopt(address, capacity);
    d->length = length;
    StringUtil::copy(d->data, str, length);
    d->data[length] = 0;
    return d;
  }
  else
  {
    return alloc(0, AllocCantFail, str, length);
  }
}

#if __G_SIZE > 1
__G_STRING::Data* __G_STRING::Data::adopt(
  void* address, sysuint_t capacity, int allocPolicy,
  const __G_CHAR* str, sysuint_t length)
{
  if (length == DetectLength) length = StringUtil::len(str);

  if (length <= capacity)
  {
    Data* d = adopt(address, capacity);
    d->length = length;
    StringUtil::copy(d->data, str, length);
    d->data[length] = __G_CHAR(0);
    return d;
  }
  else
  {
    return alloc(0, AllocCantFail, str, length);
  }
}
#endif // __G_SIZE > 1

__G_STRING::Data* __G_STRING::Data::alloc(
  sysuint_t capacity, int allocPolicy)
{
  if (capacity == 0) return __G_STRING::sharedNull->refAlways();

  // Pad to 16 bytes
#if __G_SIZE == 1
  capacity = (capacity + 15) & ~15;
#endif

#if __G_SIZE == 2
  capacity = (capacity + 7) & ~7;
#endif

#if __G_SIZE == 4
  capacity = (capacity + 3) & ~3;
#endif

  sysuint_t dsize = sizeFor(capacity);
  Data* d = (Data *)Memory::alloc(dsize);

  if (d)
  {
    d->refCount.init(1);
    d->flags = IsDynamic | IsSharable;
    d->hashCode = 0;
    d->capacity = capacity;
    d->length = 0;
  }
  else if (allocPolicy == AllocCantFail)
  {
    fog_out_of_memory_fatal_format(
      __G_STRING_NAME "::Data", "alloc", 
      "Couldn't allocate %lu bytes of memory for string data", (ulong)dsize);
  }
  return d;
}

__G_STRING::Data* __G_STRING::Data::alloc(
  sysuint_t capacity, int allocPolicy,
  const Char8* str, sysuint_t length)
{
  if (length == DetectLength) length = StringUtil::len(str);
  if (length > capacity) capacity = length;

  if (capacity == 0) return __G_STRING::sharedNull->refAlways();

  Data* d = alloc(capacity, allocPolicy);

  if (d)
  {
    d->length = length;
    StringUtil::copy(d->data, str, length);
    d->data[length] = __G_CHAR(0);
  }

  return d;
}

#if __G_SIZE > 1
__G_STRING::Data* __G_STRING::Data::alloc(
  sysuint_t capacity, int allocPolicy,
  const __G_CHAR* str, sysuint_t length)
{
  if (length == DetectLength) length = StringUtil::len(str);
  if (length > capacity) capacity = length;

  if (capacity == 0) return __G_STRING::sharedNull->refAlways();

  Data* d = alloc(capacity, allocPolicy);

  if (d)
  {
    d->length = length;
    StringUtil::copy(d->data, str, length);
    d->data[length] = __G_CHAR(0);
  }

  return d;
}
#endif // __G_SIZE > 1

__G_STRING::Data* __G_STRING::Data::realloc(Data* d, sysuint_t capacity, int allocPolicy)
{
  FOG_ASSERT(capacity >= d->length);

  sysuint_t dsize = __G_STRING::Data::sizeFor(capacity);

  if ((d->flags & __G_STRING::Data::IsDynamic) != 0)
  {
    if ((d = (Data *)Memory::realloc((void*)d, dsize)) != NULL)
    {
      d->capacity = capacity;
    }
  }
  else
  {
    Data* newd = alloc(capacity, allocPolicy, d->data, d->length);
    if (newd)
    {
      d->deref();
      d = newd;
    }
  }

  if (!d && allocPolicy == AllocCantFail)
  {
    fog_out_of_memory_fatal_format(
      __G_STRING_NAME "::Data", "realloc", 
      "Couldn't allocate %lu bytes of memory for string data", (ulong)dsize);
  }

  return d;
}

__G_STRING::Data* __G_STRING::Data::copy(
  const Data* d, int allocPolicy)
{
  return alloc(0, allocPolicy, d->data, d->length);
}

void __G_STRING::Data::free(Data* d)
{
  Memory::free((void*)d);
}

Static<__G_STRING::Data> __G_STRING::sharedNull;

// ============================================================================
// [Fog::StringFilter]
// ============================================================================

__G_STRINGFILTER::__G_STRINGFILTER()
{
}

__G_STRINGFILTER::~__G_STRINGFILTER()
{
}

Range __G_STRINGFILTER::indexOf(
  const __G_CHAR* str, sysuint_t length, 
  uint cs, const Range& range) const
{
  Range m(InvalidIndex, InvalidIndex);
  if (range.index >= length) return m;

  sysuint_t i = range.index;
  sysuint_t e = (length - i < range.length)
    ? length - i
    : range.index + range.length;

  m = match(str, length, cs, Range(i, e - i));
  return m;
}

Range __G_STRINGFILTER::lastIndexOf(
  const __G_CHAR* str, sysuint_t length,
  uint cs, const Range& range) const
{
  Range m(InvalidIndex, InvalidIndex);
  if (range.index >= length) return m;

  sysuint_t i = range.index;
  sysuint_t e = (length - i < range.length)
    ? length - i
    : range.index + range.length;

  for (;;)
  {
    Range t = match(str, length, cs, Range(i, e - i));
    if (t.index == InvalidIndex) break;

    i = t.index + t.length;
    m = t;
  }

  return m;
}

sysuint_t __G_STRINGFILTER::length() const
{
  return InvalidIndex;
}

// ============================================================================
// [Fog::StringMatcher]
// ============================================================================

static void buildTable(
  __G_STRINGMATCHER::SkipTable* skipTable,
  const __G_CHAR* patternStr,
  sysuint_t patternLength,
  uint cs)
{
  FOG_ASSERT(patternLength > 1);
  FOG_ASSERT(patternLength >= UINT_MAX);

  if (skipTable->status.cmpXchg(
    __G_STRINGMATCHER::SkipTable::Uninitialized,
    __G_STRINGMATCHER::SkipTable::Initializing))
  {
    // Init skip table
    sysuint_t a = 32; // 256 / 8
    sysuint_t i = patternLength;
    uint* data = skipTable->data;

    while (a--)
    {
      data[0] = i; data[1] = i; data[2] = i; data[3] = i;
      data[4] = i; data[5] = i; data[6] = i; data[7] = i;
      data += 8;
    }

    data = skipTable->data;

    if (cs == CaseSensitive)
    {
      while (i--)
      {
        data[patternStr->ch() & 0xFF] = (uint)i; patternStr++;
      }
    }
    else
    {
      while (i--)
      {
        data[patternStr->toLower().ch() & 0xFF] = (uint)i; patternStr++;
      }
    }
    skipTable->status.set(__G_STRINGMATCHER::SkipTable::Initialized);
    return;
  }

  // Wait...another thread creating the table...
  while (skipTable->status.get() != __G_STRINGMATCHER::SkipTable::Initialized) 
  {
    Thread::_yield();
  }
}

__G_STRINGMATCHER::__G_STRINGMATCHER()
{
  _skipTable[CaseInsensitive].status.init(SkipTable::Uninitialized);
  _skipTable[CaseSensitive  ].status.init(SkipTable::Uninitialized);
}

__G_STRINGMATCHER::__G_STRINGMATCHER(const __G_STRING& pattern)
{
  setPattern(pattern);
}

__G_STRINGMATCHER::__G_STRINGMATCHER(const __G_STRINGMATCHER& matcher)
{
  setPattern(matcher);
}

__G_STRINGMATCHER::~__G_STRINGMATCHER()
{
}

err_t __G_STRINGMATCHER::setPattern(const __G_STRING& pattern)
{
  err_t err;
  if ( (err = _pattern.set(pattern)) ) return err;

  // Mark tables as uninitialized
  _skipTable[CaseInsensitive].status.set(SkipTable::Uninitialized);
  _skipTable[CaseSensitive  ].status.set(SkipTable::Uninitialized);

  return Error::Ok;
}

err_t __G_STRINGMATCHER::setPattern(const __G_STRINGMATCHER& matcher)
{
  SkipTable* dstSkipTable = _skipTable;
  const SkipTable* srcSkipTable = matcher._skipTable;

  err_t err;
  if ( (err = _pattern.set(matcher._pattern)) ) return err;

  // Copy skip tables if they are initialized
  for (sysuint_t i = 0; i != 2; i++)
  {
    if (srcSkipTable[i].status.get() == SkipTable::Initialized)
    {
      dstSkipTable[i].status.set(SkipTable::Initialized);
      memcpy(
        &dstSkipTable[i].data,
        &srcSkipTable[i].data,
        sizeof(uint) * 256);
    }
    else
      dstSkipTable[i].status.set(SkipTable::Uninitialized);
  }

  return Error::Ok;
}

Range __G_STRINGMATCHER::match(
  const __G_CHAR* str, sysuint_t length,
  uint cs, const Range& range) const
{
  FOG_ASSERT(length != DetectLength);
  FOG_ASSERT(range.index <= length);
  FOG_ASSERT(length - range.index >= range.length);

  sysuint_t patternLength = _pattern.length();

  // simple reject
  if (patternLength == 0 || patternLength > length) 
  {
    return InvalidIndex;
  }

  // we want 0 or 1
  cs = !!cs;

  const __G_CHAR* strCur = str + range.index;
  const __G_CHAR* patternStr = _pattern.cData();

  // Simple 'Char' search.
  if (patternLength == 1) 
  {
    sysuint_t i = StringUtil::indexOf(
      strCur, range.length, patternStr[0], cs);
    if (i != InvalidIndex) i += range.index;
    return i;
  }

  if (_skipTable[cs].status.get() != __G_STRINGMATCHER::SkipTable::Initialized)
  {
    buildTable(const_cast<SkipTable*>(
      &_skipTable[cs]), 
      patternStr, patternLength, 
      cs);
  }

  const uint* skipTable = _skipTable[cs].data;

  sysuint_t skip;
  sysuint_t remain = range.length;

  patternStr += patternLength - 1;
  strCur     += patternLength - 1;
  remain     -= patternLength - 1;

  if (cs == CaseSensitive)
  {
    for (;;)
    {
      // Get count of characters to skip from skip table
      if ((skip = skipTable[strCur->ch() & 0xFF]) == 0)
      {
        // Check if there is possible match
        while (skip < patternLength)
        {
          if (*(strCur - skip) != *(patternStr - skip)) break;
          skip++;
        }

        // Match
        if (skip >= patternLength) 
          return Range((sysuint_t)(strCur - str) - skip + 1, patternLength);

        if (skipTable[(strCur - skip)->ch() & 0xFF] == patternLength)
          skip = patternLength - skip;
        else
          skip = 1;
      }

      if (remain < skip) break;
      strCur += skip;
      remain -= skip;
    }
  }
  else
  {
    for (;;)
    {
      // Get count of characters to skip from skip table
      if ((skip = skipTable[strCur->toLower().ch() & 0xFF]) == 0)
      {
        // Check if there is possible match
        while (skip < patternLength)
        {
          if ((strCur - skip)->toLower() != (patternStr - skip)->toLower() ) break;
          skip++;
        }

        // Match
        if (skip >= patternLength) 
          return Range((sysuint_t)(strCur - str) - skip + 1, patternLength);

        if (skipTable[(strCur - skip)->toLower().ch() & 0xFF] == patternLength)
          skip = patternLength - skip;
        else
          skip = 1;
      }

      if (remain < skip) break;
      strCur += skip;
      remain -= skip;
    }
  }

  return Range(InvalidIndex);
}

sysuint_t __G_STRINGMATCHER::length() const
{
  return _pattern.length();
}

} // Fog namespace

#undef __G_FRIEND
#undef __G_CHAR
#undef __G_TEMPORARYSTRING
#undef __G_STRINGMATCHER
#undef __G_STRINGFILTER
#undef __G_STRING_STUBX
#undef __G_STRING_UTFX
#undef __G_STRING_STUB
#undef __G_STRING_NAME
#undef __G_STRING

// [Generator]
#endif // __G_GENERATE
