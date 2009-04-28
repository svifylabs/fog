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

//! @addtogroup Fog_Core
//! @{

namespace Fog {

struct __G_STRINGFILTER;

// ============================================================================
// [Fog::String]
// ============================================================================

struct FOG_API __G_STRING
{
  // --------------------------------------------------------------------------
  // [String Data]
  // --------------------------------------------------------------------------

  //! @brief String data
  struct FOG_API Data
  {
    // [Flags]

    //! @brief String data flags.
    enum
    {
      //! @brief Null 'd' object. 
      //!
      //! This is very likely object that's shared between all null objects. So
      //! normally only one data instance can has this flag set on.
      IsNull = (1U << 0),

      //! @brief String data are created on the heap. 
      //!
      //! Object is created by function like @c Fog::Memory::alloc() or by
      //! @c new operator. It this flag is not set, you can't delete object from
      //! the heap and object is probabbly only temporary (short life object).
      IsDynamic = (1U << 1),

      //! @brief String data are shareable.
      //!
      //! Object can be directly referenced by internal method @c ref(). 
      //! Sharable data are usually created on the heap and together 
      //! with this flag is set also @c IsDynamic, but it isn't prerequisite.
      IsSharable = (1U << 2),

      //! @brief String data are strong to weak assignments.
      //!
      //! This flag means:
      //!   "Don't assign other data to me, instead, copy them to me!".
      IsStrong = (1U << 3)
    };

    // [Ref]

    Data* ref() const;
    void deref();

    FOG_INLINE Data* refAlways() const
    {
      refCount.inc();
      return (Data*)this;
    }

    FOG_INLINE Data* refInline() const
    {
      if (FOG_LIKELY((flags & IsSharable) != 0))
        return refAlways();
      else
        return copy(this, AllocCantFail);
    }

    FOG_INLINE void derefInline()
    {
      if (refCount.deref() && (flags & IsDynamic) != 0) free(this);
    }

    // [Statics]

    static Data* adopt(void* address, sysuint_t capacity);
    static Data* adopt(void* address, sysuint_t capacity, int allocPolicy, const Char8* str, sysuint_t length);
#if __G_SIZE > 1
    static Data* adopt(void* address, sysuint_t capacity, int allocPolicy, const __G_CHAR* str, sysuint_t length);
#endif // __G_SIZE > 1

    static Data* alloc(sysuint_t capacity, int allocPolicy);
    static Data* alloc(sysuint_t capacity, int allocPolicy, const Char8* str, sysuint_t length);
#if __G_SIZE > 1
    static Data* alloc(sysuint_t capacity, int allocPolicy, const __G_CHAR* str, sysuint_t length);
#endif // __G_SIZE > 1

    static Data* realloc(Data* d, sysuint_t capacity, int allocPolicy);
    static Data* copy(const Data* d, int allocPolicy);
    static void free(Data* d);

    // [Size]

    static FOG_INLINE sysuint_t sizeFor(sysuint_t capacity)
    { return sizeof(Data) + sizeof(__G_CHAR) * capacity; }

    // [Members]

    mutable Atomic<sysuint_t> refCount;

    uint32_t flags;
    uint32_t hashCode;

    sysuint_t capacity;
    sysuint_t length;

    __G_CHAR data[4 / __G_SIZE];
  };

  static Static<Data> sharedNull;

  // --------------------------------------------------------------------------
  // [Typedefs]
  // --------------------------------------------------------------------------

  typedef bool (*EqFn)(const __G_STRING* a, const __G_STRING* b);
  typedef int (*CompareFn)(const __G_STRING* a, const __G_STRING* b);

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  __G_STRING();

  __G_STRING(__G_CHAR ch, sysuint_t length);
  __G_STRING(const __G_STRING& other);
  __G_STRING(const __G_STRING& other1, const __G_STRING& other2);

#if __G_SIZE == 1
  __G_STRING(const Stub8& str);
#else
  __G_STRING(const StubAscii8& str);
  __G_STRING(const StubLocal8& str);
  __G_STRING(const StubUtf8& str);
  __G_STRING(const StubUtf16& str);
  __G_STRING(const StubUtf32& str);
#endif // __G_SIZE 

  __G_STRING(Data* d);

  ~__G_STRING();

  // --------------------------------------------------------------------------
  // [Implicit Sharing]
  // --------------------------------------------------------------------------

  //! @copydoc Doxygen::Implicit::refCount().
  FOG_INLINE sysuint_t refCount() const { return _d->refCount.get(); }
  //! @copydoc Doxygen::Implicit::isDetached().
  FOG_INLINE bool isDetached() const { return refCount() == 1; }
  //! @copydoc Doxygen::Implicit::detach().
  FOG_INLINE void detach() { if (!isDetached()) _detach(); }
  //! @copydoc Doxygen::Implicit::tryDetach().
  FOG_INLINE bool tryDetach() { return _tryDetach(); }

  //! @copydoc Doxygen::Implicit::_detach().
  void _detach();
  //! @copydoc Doxygen::Implicit::_tryDetach().
  bool _tryDetach();

  // --------------------------------------------------------------------------
  // [Flags]
  // --------------------------------------------------------------------------

  //! @copydoc Doxygen::Implicit::flags().
  FOG_INLINE uint32_t flags() const { return _d->flags; }
  //! @copydoc Doxygen::Implicit::isNull().
  FOG_INLINE bool isNull() const { return (_d->flags & Data::IsNull) != 0; }
  //! @copydoc Doxygen::Implicit::isDynamic().
  FOG_INLINE bool isDynamic() const { return (_d->flags & Data::IsDynamic) != 0; }
  //! @copydoc Doxygen::Implicit::isSharable().
  FOG_INLINE bool isSharable() const { return (_d->flags & Data::IsSharable) != 0; }
  //! @copydoc Doxygen::Implicit::isStrong().
  FOG_INLINE bool isStrong() const { return (_d->flags & Data::IsStrong) != 0; }

  //! @copydoc Doxygen::Implicit::setIsSharable().
  void setIsSharable(bool val);
  //! @copydoc Doxygen::Implicit::setIsStrong().
  void setIsStrong(bool val);

  // --------------------------------------------------------------------------
  // [Data]
  // --------------------------------------------------------------------------

  //! @brief Returns count of allocated characters (capacity).
  FOG_INLINE sysuint_t capacity() const { return _d->capacity; }
  //! @brief Returns count of used characters (length).
  FOG_INLINE sysuint_t length() const { return _d->length; }
  //! @brief Returns @c true if string is empty (length == 0).
  FOG_INLINE bool isEmpty() const { return _d->length == 0; }

  err_t reserve(sysuint_t to);
  err_t resize(sysuint_t to);
  err_t grow(sysuint_t by);

  void squeeze();
  void clear();
  void free();

  //! @brief Returns const pointer to string data.
  FOG_INLINE const __G_CHAR* cData() const
  { return _d->data; }

  //! @brief Returns mutable pointer to string data.
  __G_CHAR* mData();

  FOG_INLINE __G_CHAR* xData()
  {
    FOG_ASSERT_X(isDetached(), __G_STRING_NAME "::xData() - Not detached data.");
    return _d->data;
  }

  FOG_INLINE const __G_FRIEND* cStr() const 
  { return (const __G_FRIEND*)cData(); }

  FOG_INLINE const __G_FRIEND* mStr() 
  { return (const __G_FRIEND*)mData(); }

  FOG_INLINE __G_FRIEND* xStr()
  {
    FOG_ASSERT_X(isDetached(), __G_STRING_NAME "::xStr() - Not detached data.");
    return (__G_FRIEND*)_d->data;
  }

#if __G_SIZE == FOG_SIZEOF_WCHAR_T
  FOG_INLINE const wchar_t* cStrW() const 
  { return (const wchar_t *)cData(); }

  FOG_INLINE wchar_t* mStrW() 
  { return (wchar_t *)mData(); }

  FOG_INLINE wchar_t* xStrW() 
  { return (wchar_t *)xData(); }
#endif // __G_SIZE == FOG_SIZEOF_WCHAR_T

  FOG_INLINE void xFinalize()
  {
    FOG_ASSERT_X(isDetached(),
      __G_STRING_NAME "::xFinalize() - Non detached data.");

    _d->hashCode = 0;

    FOG_ASSERT_X(_d->data[_d->length] == __G_CHAR(0),
      __G_STRING_NAME "::xFinalize() - Null terminator corrupted.");
  }

  FOG_INLINE void xFinalize(__G_CHAR* end)
  {
    FOG_ASSERT_X(isDetached(),
      __G_STRING_NAME "::xFinalize() - Non detached data.");

    _d->hashCode = 0;
    _d->length = (sysuint_t)(end - _d->data);
    *end = 0;

    FOG_ASSERT_X(_d->length <= _d->capacity,
      __G_STRING_NAME "::xFinalize() - Buffer overflow.");
  }

  FOG_INLINE void xFinalize(__G_FRIEND* end)
  { xFinalize((__G_CHAR*)end); }

  FOG_INLINE __G_CHAR at(sysuint_t index) const
  {
    FOG_ASSERT_X(index < length(),
      __G_STRING_NAME "::at() - Index out of range.");

    return _d->data[index];
  }

  // --------------------------------------------------------------------------
  // [Set]
  // --------------------------------------------------------------------------

  err_t set(__G_CHAR ch, sysuint_t length = 1);

#if __G_SIZE == 1
  err_t set(const Stub8& str);
  err_t set(const StubUtf8& str, const TextCodec& tc);
  err_t set(const StubUtf16& str, const TextCodec& tc);
  err_t set(const StubUtf32& str, const TextCodec& tc);

  FOG_INLINE err_t set(const String16& str, const TextCodec& tc)
  { return set(StubUtf16(str), tc); }

  FOG_INLINE err_t set(const String32& str, const TextCodec& tc)
  { return set(StubUtf32(str), tc); }
#else
  err_t set(const void* str, sysuint_t size, const TextCodec& tc);
  err_t set(const StubAscii8& str);
  err_t set(const StubLocal8& str);
  err_t set(const StubUtf8& str);
  err_t set(const StubUtf16& str);
  err_t set(const StubUtf32& str);
#endif // __G_SIZE 

  err_t set(const __G_STRING& other);
#if __G_SIZE != 1
  err_t set(const __G_STRING_UTFX& other);
#endif // __G_SIZE
  err_t setDeep(const __G_STRING& other);

  err_t setBool(bool b);

  err_t setInt(int32_t n, int base = 10);
  err_t setInt(uint32_t n, int base = 10);
  err_t setInt(int64_t n, int base = 10);
  err_t setInt(uint64_t n, int base = 10);

#if __G_SIZE == 1
  err_t setInt(int32_t n, int base, const FormatFlags& ff);
  err_t setInt(uint32_t n, int base, const FormatFlags& ff);
  err_t setInt(int64_t n, int base, const FormatFlags& ff);
  err_t setInt(uint64_t n, int base, const FormatFlags& ff);
#else
  err_t setInt(int32_t n, int base, const FormatFlags& ff, const Locale* locale = NULL);
  err_t setInt(uint32_t n, int base, const FormatFlags& ff, const Locale* locale = NULL);
  err_t setInt(int64_t n, int base, const FormatFlags& ff, const Locale* locale = NULL);
  err_t setInt(uint64_t n, int base, const FormatFlags& ff, const Locale* locale = NULL);
#endif // __G_SIZE

  err_t setDouble(double d, int doubleForm = DF_SignificantDigits);
#if __G_SIZE == 1
  err_t setDouble(double d, int doubleForm, const FormatFlags& ff);
#else
  err_t setDouble(double d, int doubleForm, const FormatFlags& ff, const Locale* locale = NULL);
#endif // __G_SIZE

  err_t format(const char* fmt, ...);
  err_t vformat(const char* fmt, va_list ap);

  err_t wformat(const __G_STRING& fmt, __G_CHAR lex, const Sequence<__G_STRING>& args);
  err_t wformat(const __G_STRING& fmt, __G_CHAR lex, const __G_STRING* args, sysuint_t length);

  // --------------------------------------------------------------------------
  // [Append]
  // --------------------------------------------------------------------------

  err_t append(__G_CHAR ch, sysuint_t length = 1);

#if __G_SIZE == 1
  err_t append(const Stub8& str);
  err_t append(const StubUtf8& str, const TextCodec& tc);
  err_t append(const StubUtf16& str, const TextCodec& tc);
  err_t append(const StubUtf32& str, const TextCodec& tc);

  FOG_INLINE err_t append(const String16& str, const TextCodec& tc)
  { return append(StubUtf16(str), tc); }

  FOG_INLINE err_t append(const String32& str, const TextCodec& tc)
  { return append(StubUtf32(str), tc); }
#else
  err_t append(const void* str, sysuint_t size, const TextCodec& tc);
  err_t append(const StubAscii8& str);
  err_t append(const StubLocal8& str);
  err_t append(const StubUtf8& str);
  err_t append(const StubUtf16& str);
  err_t append(const StubUtf32& str);
#endif // __G_SIZE 

  err_t append(const __G_STRING& other);
#if __G_SIZE != 1
  err_t append(const __G_STRING_UTFX& other);
#endif // __G_SIZE

  err_t appendBool(bool b);
  err_t appendInt(int32_t n, int base = 10);
  err_t appendInt(uint32_t n, int base = 10);
  err_t appendInt(int64_t n, int base = 10);
  err_t appendInt(uint64_t n, int base = 10);

#if __G_SIZE == 1
  err_t appendInt(int32_t n, int base, const FormatFlags& ff);
  err_t appendInt(uint32_t n, int base, const FormatFlags& ff);
  err_t appendInt(int64_t n, int base, const FormatFlags& ff);
  err_t appendInt(uint64_t n, int base, const FormatFlags& ff);
#else
  err_t appendInt(int32_t n, int base, const FormatFlags& ff, const Locale* locale = NULL);
  err_t appendInt(uint32_t n, int base, const FormatFlags& ff, const Locale* locale = NULL);
  err_t appendInt(int64_t n, int base, const FormatFlags& ff, const Locale* locale = NULL);
  err_t appendInt(uint64_t n, int base, const FormatFlags& ff, const Locale* locale = NULL);
#endif

  err_t appendDouble(double d, int doubleForm = DF_SignificantDigits);
#if __G_SIZE == 1
  err_t appendDouble(double d, int doubleForm, const FormatFlags& ff);
#else
  err_t appendDouble(double d, int doubleForm, const FormatFlags& ff, const Locale* locale = NULL);
#endif

  err_t appendFormat(const char* fmt, ...);
  err_t appendVformat(const char* fmt, va_list ap);

  err_t appendWformat(const __G_STRING& fmt, __G_CHAR lex, const Sequence<__G_STRING>& args);
  err_t appendWformat(const __G_STRING& fmt, __G_CHAR lex, const __G_STRING* args, sysuint_t length);

  // --------------------------------------------------------------------------
  // [Prepend]
  // --------------------------------------------------------------------------

  err_t prepend(__G_CHAR ch, sysuint_t length = 1);

#if __G_SIZE == 1
  err_t prepend(const Stub8& str);
  err_t prepend(const StubUtf8& str, const TextCodec& tc);
  err_t prepend(const StubUtf16& str, const TextCodec& tc);
  err_t prepend(const StubUtf32& str, const TextCodec& tc);

  FOG_INLINE err_t prepend(const String16& str, const TextCodec& tc)
  { return prepend(StubUtf16(str), tc); }

  FOG_INLINE err_t prepend(const String32& str, const TextCodec& tc)
  { return prepend(StubUtf32(str), tc); }
#else
  err_t prepend(const void* str, sysuint_t size, const TextCodec& tc);
  err_t prepend(const StubAscii8& str);
  err_t prepend(const StubLocal8& str);
  err_t prepend(const StubUtf8& str);
  err_t prepend(const StubUtf16& str);
  err_t prepend(const StubUtf32& str);
#endif // __G_SIZE 

  err_t prepend(const __G_STRING& other);
#if __G_SIZE != 1
  err_t prepend(const __G_STRING_UTFX& other);
#endif // __G_SIZE

  // --------------------------------------------------------------------------
  // [Insert]
  // --------------------------------------------------------------------------

  err_t insert(sysuint_t index, __G_CHAR ch, sysuint_t length = 1);

#if __G_SIZE == 1
  err_t insert(sysuint_t index, const Stub8& str);
  err_t insert(sysuint_t index, const StubUtf8& str, const TextCodec& tc);
  err_t insert(sysuint_t index, const StubUtf16& str, const TextCodec& tc);
  err_t insert(sysuint_t index, const StubUtf32& str, const TextCodec& tc);

  FOG_INLINE err_t insert(sysuint_t index, const String16& str, const TextCodec& tc)
  { return insert(index, StubUtf16(str), tc); }

  FOG_INLINE err_t insert(sysuint_t index, const String32& str, const TextCodec& tc)
  { return insert(index, StubUtf32(str), tc); }
#else
  err_t insert(sysuint_t index, const void* str, sysuint_t size, const TextCodec& tc);
  err_t insert(sysuint_t index, const StubAscii8& str);
  err_t insert(sysuint_t index, const StubLocal8& str);
  err_t insert(sysuint_t index, const StubUtf8& str);
  err_t insert(sysuint_t index, const StubUtf16& str);
  err_t insert(sysuint_t index, const StubUtf32& str);
#endif // __G_SIZE 

  err_t insert(sysuint_t index, const __G_STRING& other);
#if __G_SIZE != 1
  err_t insert(sysuint_t index, const __G_STRING_UTFX& other);
#endif // __G_SIZE

  // --------------------------------------------------------------------------
  // [Remove]
  // --------------------------------------------------------------------------

  sysuint_t remove(const Range& range);

  sysuint_t remove(__G_CHAR ch,
    uint cs = CaseSensitive, const Range& range = Range());
  sysuint_t remove(const __G_STRING& other,
    uint cs = CaseSensitive, const Range& range = Range());
  sysuint_t remove(const __G_STRINGFILTER& filter,
    uint cs = CaseSensitive, const Range& range = Range());

  // --------------------------------------------------------------------------
  // [Replace]
  // --------------------------------------------------------------------------

  err_t _replaceMatches(
    Range* m, sysuint_t mcount,
    const __G_CHAR* after, sysuint_t alen);

  err_t replace(const Range& range,
    const __G_STRING& replacement);

  err_t replace(__G_CHAR before, __G_CHAR after, 
    uint cs = CaseSensitive, const Range& range = Range());
  err_t replace(const __G_STRING& before, const __G_STRING& after, 
    uint cs = CaseSensitive, const Range& range = Range());
  err_t replace(const __G_STRINGFILTER& before, const __G_STRING& after, 
    uint cs = CaseSensitive, const Range& range = Range());

  // --------------------------------------------------------------------------
  // [Lower / Upper]
  // --------------------------------------------------------------------------

  err_t lower();
  err_t upper();

  __G_STRING lowered() const;
  __G_STRING uppered() const;

  // --------------------------------------------------------------------------
  // [Whitespaces / Justification]
  // --------------------------------------------------------------------------

  err_t trim();
  err_t simplify();
  err_t truncate(sysuint_t n);
  err_t justify(sysuint_t n, __G_CHAR fill, uint32_t flags);

  __G_STRING trimmed() const;
  __G_STRING simplified() const;
  __G_STRING truncated(sysuint_t n) const;
  __G_STRING justified(sysuint_t n, __G_CHAR fill, uint32_t flags) const;

  // --------------------------------------------------------------------------
  // [Split]
  // --------------------------------------------------------------------------

  Vector<__G_STRING> split(__G_CHAR ch, uint splitBehavior = RemoveEmptyParts, uint cs = CaseSensitive) const;
  Vector<__G_STRING> split(const __G_STRING& pattern, uint splitBehavior = RemoveEmptyParts, uint cs = CaseSensitive) const;
  Vector<__G_STRING> split(const __G_STRINGFILTER& filter, uint splitBehavior = RemoveEmptyParts, uint cs = CaseSensitive) const;

  // --------------------------------------------------------------------------
  // [Substring]
  // --------------------------------------------------------------------------

  __G_STRING substring(const Range& range) const;

  // --------------------------------------------------------------------------
  // [Conversion]
  // --------------------------------------------------------------------------

  err_t atob(bool* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL) const;
  err_t atoi8(int8_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL) const;
  err_t atou8(uint8_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL) const;
  err_t atoi16(int16_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL) const;
  err_t atou16(uint16_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL) const;
  err_t atoi32(int32_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL) const;
  err_t atou32(uint32_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL) const;
  err_t atoi64(int64_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL) const;
  err_t atou64(uint64_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL) const;
  
#if FOG_SIZEOF_LONG == 32
  FOG_INLINE err_t atol(long* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL) const
  { return atoi32((int32_t*)dst, base, end, parserFlags); }
  FOG_INLINE ulong atoul(ulong* dst, uint base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL) const
  { return atou32((uint32_t*)dst, base, end, parserFlags); }
#else
  FOG_INLINE err_t atol(long* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL) const
  { return atoi64((int64_t*)dst, base, end, parserFlags); }
  FOG_INLINE ulong atoul(ulong* dst, uint base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL) const
  { return atou64((uint64_t*)dst, base, end, parserFlags); }
#endif

#if FOG_ARCH_BITS == 32
  FOG_INLINE err_t atosysint(sysint_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL) const
  { return atoi32((int32_t*)dst, base, end, parserFlags); }
  FOG_INLINE err_t atosysuint(sysuint_t* dst, uint base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL) const
  { return atou32((uint32_t*)dst, base, end, parserFlags); }
#else
  FOG_INLINE err_t atosysint(sysint_t* dst, int base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL) const
  { return atoi64((int64_t*)dst, base, end, parserFlags); }
  FOG_INLINE err_t atosysuint(sysuint_t* dst, uint base = 0, sysuint_t* end = NULL, uint32_t* parserFlags = NULL) const
  { return atou64((uint64_t*)dst, base, end, parserFlags); }
#endif

#if __G_SIZE == 1
  err_t atof(float* dst, sysuint_t* end = NULL, uint32_t* parserFlags = NULL) const;
  err_t atod(double* dst, sysuint_t* end = NULL, uint32_t* parserFlags = NULL) const;
#else
  err_t atof(float* dst, const Locale* locale = NULL, sysuint_t* end = NULL, uint32_t* parserFlags = NULL) const;
  err_t atod(double* dst, const Locale* locale = NULL, sysuint_t* end = NULL, uint32_t* parserFlags = NULL) const;
#endif

  // --------------------------------------------------------------------------
  // [Contains]
  // --------------------------------------------------------------------------

  bool contains(__G_CHAR ch,
    uint cs = CaseSensitive, const Range& range = Range()) const;

  bool contains(const __G_STRING& pattern,
    uint cs = CaseSensitive, const Range& range = Range()) const;

  bool contains(const __G_STRINGFILTER& filter, 
    uint cs = CaseSensitive, const Range& range = Range()) const;

  // --------------------------------------------------------------------------
  // [CountOf]
  // --------------------------------------------------------------------------

  sysuint_t countOf(__G_CHAR ch,
    uint cs = CaseSensitive, const Range& range = Range()) const;
  sysuint_t countOf(const __G_STRING& pattern,
    uint cs = CaseSensitive, const Range& range = Range()) const;
  sysuint_t countOf(const __G_STRINGFILTER& filter,
    uint cs = CaseSensitive, const Range& range = Range()) const;

  // --------------------------------------------------------------------------
  // [IndexOf / LastIndexOf]
  // --------------------------------------------------------------------------

  sysuint_t indexOf(__G_CHAR ch,
    uint cs = CaseSensitive, const Range& range = Range()) const;
  sysuint_t indexOf(const __G_STRING& pattern,
    uint cs = CaseSensitive, const Range& range = Range()) const;
  sysuint_t indexOf(const __G_STRINGFILTER& filter,
    uint cs = CaseSensitive, const Range& range = Range()) const;

  sysuint_t lastIndexOf(__G_CHAR ch,
    uint cs = CaseSensitive, const Range& range = Range()) const;
  sysuint_t lastIndexOf(const __G_STRING& pattern,
    uint cs = CaseSensitive, const Range& range = Range()) const;
  sysuint_t lastIndexOf(const __G_STRINGFILTER& filter,
    uint cs = CaseSensitive, const Range& range = Range()) const;

  // --------------------------------------------------------------------------
  // [StartsWith / EndsWith]
  // --------------------------------------------------------------------------

#if __G_SIZE == 1
  bool startsWith(const Stub8& str, uint cs = CaseSensitive) const;
#else
  bool startsWith(const void* data, sysuint_t size, const TextCodec& tc, uint cs = CaseSensitive) const;
  bool startsWith(const StubAscii8& str, uint cs = CaseSensitive) const;
  bool startsWith(const StubLocal8& str, uint cs = CaseSensitive) const;
  bool startsWith(const StubUtf8& str, uint cs = CaseSensitive) const;
  bool startsWith(const StubUtf16& str, uint cs = CaseSensitive) const;
  bool startsWith(const StubUtf32& str, uint cs = CaseSensitive) const;
#endif // __G_SIZE
  bool startsWith(const __G_STRING& str, uint cs = CaseSensitive) const;
  bool startsWith(const __G_STRINGFILTER& filter, uint cs = CaseSensitive) const;

#if __G_SIZE == 1
  bool endsWith(const Stub8& str, uint cs = CaseSensitive) const;
#else
  bool endsWith(const void* data, sysuint_t size, const TextCodec& tc, uint cs = CaseSensitive) const;
  bool endsWith(const StubAscii8& str, uint cs = CaseSensitive) const;
  bool endsWith(const StubLocal8& str, uint cs = CaseSensitive) const;
  bool endsWith(const StubUtf8& str, uint cs = CaseSensitive) const;
  bool endsWith(const StubUtf16& str, uint cs = CaseSensitive) const;
  bool endsWith(const StubUtf32& str, uint cs = CaseSensitive) const;
#endif // __G_SIZE
  bool endsWith(const __G_STRING& str, uint cs = CaseSensitive) const;
  bool endsWith(const __G_STRINGFILTER& filter, uint cs = CaseSensitive) const;

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  FOG_INLINE __G_STRING& operator=(__G_CHAR ch)
  { set(ch); return *this; }
  
#if __G_SIZE == 1
  FOG_INLINE __G_STRING& operator=(const Stub8& str)
  { set(str); return *this; }
#else
  FOG_INLINE __G_STRING& operator=(const StubAscii8& str)
  { set(str); return *this; }

  FOG_INLINE __G_STRING& operator=(const StubLocal8& str)
  { set(str); return *this; }

  FOG_INLINE __G_STRING& operator=(const StubUtf8& str)
  { set(str); return *this; }

  FOG_INLINE __G_STRING& operator=(const StubUtf16& str)
  { set(str); return *this; }

  FOG_INLINE __G_STRING& operator=(const StubUtf32& str)
  { set(str); return *this; }
#endif

  FOG_INLINE __G_STRING& operator=(const __G_STRING& other)
  { set(other); return *this; }

  FOG_INLINE __G_STRING& operator+=(__G_CHAR ch)
  { append(ch); return *this; }

#if __G_SIZE == 1
  FOG_INLINE __G_STRING& operator+=(const Stub8& str)
  { append(str); return *this; }
#else
  FOG_INLINE __G_STRING& operator+=(const StubAscii8& str)
  { append(str); return *this; }

  FOG_INLINE __G_STRING& operator+=(const StubLocal8& str)
  { append(str); return *this; }

  FOG_INLINE __G_STRING& operator+=(const StubUtf8& str)
  { append(str); return *this; }

  FOG_INLINE __G_STRING& operator+=(const StubUtf16& str)
  { append(str); return *this; }

  FOG_INLINE __G_STRING& operator+=(const StubUtf32& str)
  { append(str); return *this; }
#endif
  
  FOG_INLINE __G_STRING& operator+=(const __G_STRING& other)
  { append(other); return *this; }

  // --------------------------------------------------------------------------
  // [ByteSwap]
  // --------------------------------------------------------------------------

#if __G_SIZE > 1
  err_t bswap();
#endif // __G_SIZE > 1

  // --------------------------------------------------------------------------
  // [Comparison]
  // --------------------------------------------------------------------------

  static bool eq(const __G_STRING* a, const __G_STRING* b);
  static bool ieq(const __G_STRING* a, const __G_STRING* b);

  static int compare(const __G_STRING* a, const __G_STRING* b);
  static int icompare(const __G_STRING* a, const __G_STRING* b);

#if __G_SIZE == 1
  bool eq(const Stub8& other, uint cs = CaseSensitive) const;
#else
  bool eq(const StubAscii8& other, uint cs = CaseSensitive) const;
  bool eq(const StubLocal8& other, uint cs = CaseSensitive) const;
  bool eq(const StubUtf8& other, uint cs = CaseSensitive) const;
  bool eq(const StubUtf16& other, uint cs = CaseSensitive) const;
  bool eq(const StubUtf32& other, uint cs = CaseSensitive) const;
#endif
  bool eq(const __G_STRING& other, uint cs = CaseSensitive) const;

#if __G_SIZE == 1
  int compare(const Stub8& other, uint cs = CaseSensitive) const;
#else
  int compare(const StubAscii8& other, uint cs = CaseSensitive) const;
  int compare(const StubLocal8& other, uint cs = CaseSensitive) const;
  int compare(const StubUtf8& other, uint cs = CaseSensitive) const;
  int compare(const StubUtf16& other, uint cs = CaseSensitive) const;
  int compare(const StubUtf32& other, uint cs = CaseSensitive) const;
#endif
  int compare(const __G_STRING& other, uint cs = CaseSensitive) const;

  // --------------------------------------------------------------------------
  // [Encoding]
  // --------------------------------------------------------------------------

#if __G_SIZE == 1
  // Utf8
  err_t utf8Validate();
  err_t utf8Characters(sysuint_t* dst);

  // Encode
  err_t encode(String8& dst, const TextCodec& dst_tc, const TextCodec& src_tc) const;
  err_t toAscii8(String8& dst, const TextCodec& src_tc) const;
  err_t toLocal8(String8& dst, const TextCodec& src_tc) const;
  err_t toUtf8(String8& dst, const TextCodec& src_tc) const;
  err_t toUtf16(String16& dst, const TextCodec& src_tc) const;
  err_t toUtf32(String32& dst, const TextCodec& src_tc) const;
#else
#if __G_SIZE == 2
  err_t utf16Validate(sysuint_t* invalid) const;
  err_t utf16Length(sysuint_t* len);
#endif // __G_SIZE == 2

  // Encode
  err_t encode(String8& dst, const TextCodec& tc);
  err_t toAscii8(String8& dst) const;
  err_t toLocal8(String8& dst) const;
  err_t toUtf8(String8& dst) const;
  err_t toUtf16(String16& dst) const;
  err_t toUtf32(String32& dst) const;
  err_t toStringW(StringW& dst) const;
#endif // __G_SIZE

  // --------------------------------------------------------------------------
  // [FileSystem]
  // --------------------------------------------------------------------------

  err_t slashesToPosix();
  err_t slashesToWin();

  // --------------------------------------------------------------------------
  // [Hash]
  // --------------------------------------------------------------------------

  uint32_t toHashCode() const;

  // --------------------------------------------------------------------------
  // [Members]
  // --------------------------------------------------------------------------

  FOG_DECLARE_D(Data)
};

// ============================================================================
// [Fog::__G_TEMPORARYSTRING<N>]
// ============================================================================

template<sysuint_t N>
struct __G_TEMPORARYSTRING : public __G_STRING
{
  // --------------------------------------------------------------------------
  // [Temporary Storage]
  // --------------------------------------------------------------------------

  // Keep 'Storage' name for this struct for Borland compiler
  struct Storage
  {
    Data _d;
    __G_CHAR _str[N];
  } _storage;

  // --------------------------------------------------------------------------
  // [Construction / Destruction]
  // --------------------------------------------------------------------------

  FOG_INLINE __G_TEMPORARYSTRING() :
    __G_STRING(Data::adopt((void*)&_storage, N))
  {
  }

  FOG_INLINE __G_TEMPORARYSTRING(const __G_STRING& other) :
    __G_STRING(Data::adopt((void*)&_storage, N, AllocCanFail, other.cData(), other.length()))
  {
  }

  FOG_INLINE __G_TEMPORARYSTRING(const __G_TEMPORARYSTRING<N>& other) :
    __G_STRING(Data::adopt((void*)&_storage, N, AllocCanFail, other.cData(), other.length()))
  {
  }

#if __G_SIZE == 1
  FOG_INLINE __G_TEMPORARYSTRING(const Stub8& str) :
    __G_STRING(Data::adopt((void*)&_storage, N, AllocCanFail, str.str(), str.length()))
  {
  }
#else
  FOG_INLINE __G_TEMPORARYSTRING(const StubAscii8& str) :
    __G_STRING(Data::adopt((void*)&_storage, N, AllocCanFail, str.str(), str.length()))
  {
  }

#if __G_SIZE == 2
  FOG_INLINE __G_TEMPORARYSTRING(const StubUtf16& str) :
    __G_STRING(Data::adopt((void*)&_storage, N, AllocCanFail, str.str(), str.length()))
  {
  }
#else
  FOG_INLINE __G_TEMPORARYSTRING(const StubUtf32& str) :
    __G_STRING(Data::adopt((void*)&_storage, N, AllocCanFail, str.str(), str.length()))
  {
  }
#endif // __G_SIZE
#endif // __G_SIZE

  // safe shareable __G_TEMPORARYSTRING creation
  FOG_INLINE __G_TEMPORARYSTRING(_CreateSharable) :
    __G_STRING(Data::adopt((void*)&_storage, N))
  {
    _d->flags.set(Data::IsSharable);
  }

  // --------------------------------------------------------------------------
  // [Implicit Data]
  // --------------------------------------------------------------------------

  FOG_INLINE void free()
  {
    if ((void*)_d != (void*)&_storage)
    {
      AtomicBase::ptr_setXchg(&_d, Data::adopt((void*)&_storage, N))->deref();
    }
  }

  // --------------------------------------------------------------------------
  // [Operator Overload]
  // --------------------------------------------------------------------------

  // TODO:
  // These overloads are needed to succesfull use this template (or implicit conversion
  // will break template and new string will be allocated)
  FOG_INLINE __G_TEMPORARYSTRING& operator=(const char* str)
  {
    return reinterpret_cast<__G_TEMPORARYSTRING&>(set(str));
  }

  FOG_INLINE __G_TEMPORARYSTRING& operator=(const __G_CHAR* str)
  {
    return reinterpret_cast<__G_TEMPORARYSTRING&>(set(str));
  }

  FOG_INLINE __G_TEMPORARYSTRING& operator=(const __G_STRING& str)
  {
    return reinterpret_cast<__G_TEMPORARYSTRING&>(set(str));
  }
  FOG_INLINE __G_TEMPORARYSTRING& operator=(const __G_TEMPORARYSTRING<N>& str)
  {
    return reinterpret_cast<__G_TEMPORARYSTRING&>(set(str));
  }
};

// ============================================================================
// [Fog::StringFilter]
// ============================================================================

struct FOG_API __G_STRINGFILTER
{
  // [Construction / Destruction]

  __G_STRINGFILTER();
  virtual ~__G_STRINGFILTER();

  // [Public]

  virtual Range indexOf(
    const __G_CHAR* str, sysuint_t length, 
    uint cs = CaseSensitive, const Range& range = Range()) const;

  virtual Range lastIndexOf(
    const __G_CHAR* str, sysuint_t length, 
    uint cs = CaseSensitive, const Range& range = Range()) const;

  // [Filter Implementation]

  virtual sysuint_t length() const;

  virtual Range match(
    const __G_CHAR* str, sysuint_t length,
    uint cs, const Range& range) const = 0;

private:
  FOG_DISABLE_COPY(__G_STRINGFILTER)
};

// ============================================================================
// [Fog::StringMatcher]
// ============================================================================

//! @brief String matcher.
//!
//! String matcher is designed to match pattern in string as quick as possible.
//! It creates internally small skip table and time to match string with this
//! table is constant.
//!
//! Also String uses StringMatcher to match pattern in very large string, but 
//! string matcher aren't only for large strings, it can be used for example 
//! to match one pattern in more strings, it will be often faster than naive 
//! matching.
//!
//! StringMatcher hasn't many methods to do this job, use @c setPattern()
//! or @c = operator to set pattern and @c indexIn() to match pattern in a given
//! string. In @c indexIn() methods are last argument often case sensitivity
//! that defaults to @c Fog::CaseSensitive.
//!
//! See @c Fog::String and @c Fog::ByteArray methods that has also support for
//! @c Fog::StringMatcher and @c Fog::ByteArrayMatcher.
//!
//! Example:
//! @verbatim
//! StringMatcher32 matcher(StubAscii8("test"));
//!
//! String32 s1(StubAscii8("This is test string..."));
//! String32 s2(StubAscii8("This is another TEST string..."));
//!
//! s1.indexOf(matcher, CaseSensitive);        // result == 8
//! s1.indexOf(matcher, CaseInsensitive);      // result == 8
//!
//! s2.indexOf(matcher, CaseSensitive);        // result == InvalidIndex
//! s2.indexOf(matcher, CaseInsensitive);      // result == 16
//! @endverbatim
//!
//! @sa @c Fog::String, @c Fog::ByteArrayMatcher
struct FOG_API __G_STRINGMATCHER : public __G_STRINGFILTER
{
  //! @brief @c Fog::StringMatcher skip table.
  //!
  //! @note The table is generated on the fly by first search call.
  struct SkipTable
  {
    enum Status
    {
      Uninitialized = 0,
      Initialized = 1,
      Initializing = 2
    };

    uint data[256];
    Atomic<uint> status;
  };

  // [Construction / Destruction]

  __G_STRINGMATCHER();
  __G_STRINGMATCHER(const __G_STRING& pattern);
  __G_STRINGMATCHER(const __G_STRINGMATCHER& matcher);
  ~__G_STRINGMATCHER();

  // [Pattern get / set]

  FOG_INLINE const __G_STRING& pattern() const { return _pattern; }

  err_t setPattern(const __G_STRING& pattern);
  err_t setPattern(const __G_STRINGMATCHER& matcher);

  // [Filter Implementation]

  virtual sysuint_t length() const;

  virtual Range match(
    const __G_CHAR* str, sysuint_t length,
    uint cs, const Range& range) const;

  // [Overloaded Operators]

  FOG_INLINE __G_STRINGMATCHER& operator=(const __G_STRING& pattern)
  { setPattern(pattern); return *this; }

  FOG_INLINE __G_STRINGMATCHER& operator=(const __G_STRINGMATCHER& matcher)
  { setPattern(matcher); return *this; }

  // [Members]

private:
  __G_STRING _pattern;
  mutable SkipTable _skipTable[2];
};

} // Fog namespace

// ============================================================================
// [Global Operator Overload]
// ============================================================================

static FOG_INLINE const Fog::__G_STRING operator+(const Fog::__G_STRING& a, const Fog::__G_STRING& b)
{ return Fog::__G_STRING(a, b); }

static FOG_INLINE const Fog::__G_STRING operator+(const Fog::__G_STRING& a, const Fog::__G_CHAR& b)
{ Fog::__G_STRING t(a); t.append(b); return t; }

static FOG_INLINE const Fog::__G_STRING operator+(const Fog::__G_CHAR& a, const Fog::__G_STRING& b)
{ Fog::__G_STRING t(b); t.append(a); return t; }

#if __G_SIZE == 1
static FOG_INLINE const Fog::__G_STRING operator+(const Fog::__G_STRING& a, const Fog::Stub8& b)
{ Fog::__G_STRING t(a); t.append(b); return t; }

static FOG_INLINE const Fog::__G_STRING operator+(const Fog::Stub8& b, const Fog::__G_STRING& a)
{ Fog::__G_STRING t(b); t.append(a); return t; }
#else
static FOG_INLINE const Fog::__G_STRING operator+(const Fog::__G_STRING& a, const Fog::StubAscii8& b)
{ Fog::__G_STRING t(a); t.append(b); return t; }

static FOG_INLINE const Fog::__G_STRING operator+(const Fog::__G_STRING& a, const Fog::StubLocal8& b)
{ Fog::__G_STRING t(a); t.append(b); return t; }

static FOG_INLINE const Fog::__G_STRING operator+(const Fog::__G_STRING& a, const Fog::StubUtf8& b)
{ Fog::__G_STRING t(a); t.append(b); return t; }

static FOG_INLINE const Fog::__G_STRING operator+(const Fog::__G_STRING& a, const Fog::StubUtf16& b)
{ Fog::__G_STRING t(a); t.append(b); return t; }

static FOG_INLINE const Fog::__G_STRING operator+(const Fog::__G_STRING& a, const Fog::StubUtf32& b)
{ Fog::__G_STRING t(a); t.append(b); return t; }

static FOG_INLINE const Fog::__G_STRING operator+(const Fog::StubAscii8& b, const Fog::__G_STRING& a)
{ Fog::__G_STRING t(b); t.append(a); return t; }

static FOG_INLINE const Fog::__G_STRING operator+(const Fog::StubLocal8& b, const Fog::__G_STRING& a)
{ Fog::__G_STRING t(b); t.append(a); return t; }

static FOG_INLINE const Fog::__G_STRING operator+(const Fog::StubUtf8& b, const Fog::__G_STRING& a)
{ Fog::__G_STRING t(b); t.append(a); return t; }

static FOG_INLINE const Fog::__G_STRING operator+(const Fog::StubUtf16& b, const Fog::__G_STRING& a)
{ Fog::__G_STRING t(b); t.append(a); return t; }

static FOG_INLINE const Fog::__G_STRING operator+(const Fog::StubUtf32& b, const Fog::__G_STRING& a)
{ Fog::__G_STRING t(b); t.append(a); return t; }
#endif // __G_SIZE

static FOG_INLINE bool operator==(const Fog::__G_STRING& a, const Fog::__G_STRING& b) { return  a.eq(b); }
static FOG_INLINE bool operator!=(const Fog::__G_STRING& a, const Fog::__G_STRING& b) { return !a.eq(b); }
static FOG_INLINE bool operator<=(const Fog::__G_STRING& a, const Fog::__G_STRING& b) { return  a.compare(b) <= 0; }
static FOG_INLINE bool operator< (const Fog::__G_STRING& a, const Fog::__G_STRING& b) { return  a.compare(b) <  0; }
static FOG_INLINE bool operator>=(const Fog::__G_STRING& a, const Fog::__G_STRING& b) { return  a.compare(b) >= 0; }
static FOG_INLINE bool operator> (const Fog::__G_STRING& a, const Fog::__G_STRING& b) { return  a.compare(b) >  0; }

#if __G_SIZE == 1
static FOG_INLINE bool operator==(const Fog::__G_STRING& a, const Fog::Stub8& b) { return  a.eq(b); }
static FOG_INLINE bool operator!=(const Fog::__G_STRING& a, const Fog::Stub8& b) { return !a.eq(b); }
static FOG_INLINE bool operator<=(const Fog::__G_STRING& a, const Fog::Stub8& b) { return  a.compare(b) <= 0; }
static FOG_INLINE bool operator< (const Fog::__G_STRING& a, const Fog::Stub8& b) { return  a.compare(b) <  0; }
static FOG_INLINE bool operator>=(const Fog::__G_STRING& a, const Fog::Stub8& b) { return  a.compare(b) >= 0; }
static FOG_INLINE bool operator> (const Fog::__G_STRING& a, const Fog::Stub8& b) { return  a.compare(b) >  0; }

static FOG_INLINE bool operator==(const Fog::Stub8& a, const Fog::__G_STRING& b) { return  b.eq(a); }
static FOG_INLINE bool operator!=(const Fog::Stub8& a, const Fog::__G_STRING& b) { return !b.eq(a); }
static FOG_INLINE bool operator<=(const Fog::Stub8& a, const Fog::__G_STRING& b) { return  b.compare(a) >= 0; }
static FOG_INLINE bool operator< (const Fog::Stub8& a, const Fog::__G_STRING& b) { return  b.compare(a) >  0; }
static FOG_INLINE bool operator>=(const Fog::Stub8& a, const Fog::__G_STRING& b) { return  b.compare(a) <= 0; }
static FOG_INLINE bool operator> (const Fog::Stub8& a, const Fog::__G_STRING& b) { return  b.compare(a) <  0; }
#else
static FOG_INLINE bool operator==(const Fog::__G_STRING& a, const Fog::StubAscii8& b) { return  a.eq(b); }
static FOG_INLINE bool operator!=(const Fog::__G_STRING& a, const Fog::StubAscii8& b) { return !a.eq(b); }
static FOG_INLINE bool operator<=(const Fog::__G_STRING& a, const Fog::StubAscii8& b) { return  a.compare(b) <= 0; }
static FOG_INLINE bool operator< (const Fog::__G_STRING& a, const Fog::StubAscii8& b) { return  a.compare(b) <  0; }
static FOG_INLINE bool operator>=(const Fog::__G_STRING& a, const Fog::StubAscii8& b) { return  a.compare(b) >= 0; }
static FOG_INLINE bool operator> (const Fog::__G_STRING& a, const Fog::StubAscii8& b) { return  a.compare(b) >  0; }

static FOG_INLINE bool operator==(const Fog::StubAscii8& a, const Fog::__G_STRING& b) { return  b.eq(a); }
static FOG_INLINE bool operator!=(const Fog::StubAscii8& a, const Fog::__G_STRING& b) { return !b.eq(a); }
static FOG_INLINE bool operator<=(const Fog::StubAscii8& a, const Fog::__G_STRING& b) { return  b.compare(a) >= 0; }
static FOG_INLINE bool operator< (const Fog::StubAscii8& a, const Fog::__G_STRING& b) { return  b.compare(a) >  0; }
static FOG_INLINE bool operator>=(const Fog::StubAscii8& a, const Fog::__G_STRING& b) { return  b.compare(a) <= 0; }
static FOG_INLINE bool operator> (const Fog::StubAscii8& a, const Fog::__G_STRING& b) { return  b.compare(a) <  0; }

static FOG_INLINE bool operator==(const Fog::__G_STRING& a, const Fog::StubLocal8& b) { return  a.eq(b); }
static FOG_INLINE bool operator!=(const Fog::__G_STRING& a, const Fog::StubLocal8& b) { return !a.eq(b); }
static FOG_INLINE bool operator<=(const Fog::__G_STRING& a, const Fog::StubLocal8& b) { return  a.compare(b) <= 0; }
static FOG_INLINE bool operator< (const Fog::__G_STRING& a, const Fog::StubLocal8& b) { return  a.compare(b) <  0; }
static FOG_INLINE bool operator>=(const Fog::__G_STRING& a, const Fog::StubLocal8& b) { return  a.compare(b) >= 0; }
static FOG_INLINE bool operator> (const Fog::__G_STRING& a, const Fog::StubLocal8& b) { return  a.compare(b) >  0; }

static FOG_INLINE bool operator==(const Fog::StubLocal8& a, const Fog::__G_STRING& b) { return  b.eq(a); }
static FOG_INLINE bool operator!=(const Fog::StubLocal8& a, const Fog::__G_STRING& b) { return !b.eq(a); }
static FOG_INLINE bool operator<=(const Fog::StubLocal8& a, const Fog::__G_STRING& b) { return  b.compare(a) >= 0; }
static FOG_INLINE bool operator< (const Fog::StubLocal8& a, const Fog::__G_STRING& b) { return  b.compare(a) >  0; }
static FOG_INLINE bool operator>=(const Fog::StubLocal8& a, const Fog::__G_STRING& b) { return  b.compare(a) <= 0; }
static FOG_INLINE bool operator> (const Fog::StubLocal8& a, const Fog::__G_STRING& b) { return  b.compare(a) <  0; }

static FOG_INLINE bool operator==(const Fog::__G_STRING& a, const Fog::StubUtf8& b) { return  a.eq(b); }
static FOG_INLINE bool operator!=(const Fog::__G_STRING& a, const Fog::StubUtf8& b) { return !a.eq(b); }
static FOG_INLINE bool operator<=(const Fog::__G_STRING& a, const Fog::StubUtf8& b) { return  a.compare(b) <= 0; }
static FOG_INLINE bool operator< (const Fog::__G_STRING& a, const Fog::StubUtf8& b) { return  a.compare(b) <  0; }
static FOG_INLINE bool operator>=(const Fog::__G_STRING& a, const Fog::StubUtf8& b) { return  a.compare(b) >= 0; }
static FOG_INLINE bool operator> (const Fog::__G_STRING& a, const Fog::StubUtf8& b) { return  a.compare(b) >  0; }

static FOG_INLINE bool operator==(const Fog::StubUtf8& a, const Fog::__G_STRING& b) { return  b.eq(a); }
static FOG_INLINE bool operator!=(const Fog::StubUtf8& a, const Fog::__G_STRING& b) { return !b.eq(a); }
static FOG_INLINE bool operator<=(const Fog::StubUtf8& a, const Fog::__G_STRING& b) { return  b.compare(a) >= 0; }
static FOG_INLINE bool operator< (const Fog::StubUtf8& a, const Fog::__G_STRING& b) { return  b.compare(a) >  0; }
static FOG_INLINE bool operator>=(const Fog::StubUtf8& a, const Fog::__G_STRING& b) { return  b.compare(a) <= 0; }
static FOG_INLINE bool operator> (const Fog::StubUtf8& a, const Fog::__G_STRING& b) { return  b.compare(a) <  0; }

static FOG_INLINE bool operator==(const Fog::__G_STRING& a, const Fog::StubUtf16& b) { return  a.eq(b); }
static FOG_INLINE bool operator!=(const Fog::__G_STRING& a, const Fog::StubUtf16& b) { return !a.eq(b); }
static FOG_INLINE bool operator<=(const Fog::__G_STRING& a, const Fog::StubUtf16& b) { return  a.compare(b) <= 0; }
static FOG_INLINE bool operator< (const Fog::__G_STRING& a, const Fog::StubUtf16& b) { return  a.compare(b) <  0; }
static FOG_INLINE bool operator>=(const Fog::__G_STRING& a, const Fog::StubUtf16& b) { return  a.compare(b) >= 0; }
static FOG_INLINE bool operator> (const Fog::__G_STRING& a, const Fog::StubUtf16& b) { return  a.compare(b) >  0; }

static FOG_INLINE bool operator==(const Fog::StubUtf16& a, const Fog::__G_STRING& b) { return  b.eq(a); }
static FOG_INLINE bool operator!=(const Fog::StubUtf16& a, const Fog::__G_STRING& b) { return !b.eq(a); }
static FOG_INLINE bool operator<=(const Fog::StubUtf16& a, const Fog::__G_STRING& b) { return  b.compare(a) >= 0; }
static FOG_INLINE bool operator< (const Fog::StubUtf16& a, const Fog::__G_STRING& b) { return  b.compare(a) >  0; }
static FOG_INLINE bool operator>=(const Fog::StubUtf16& a, const Fog::__G_STRING& b) { return  b.compare(a) <= 0; }
static FOG_INLINE bool operator> (const Fog::StubUtf16& a, const Fog::__G_STRING& b) { return  b.compare(a) <  0; }

static FOG_INLINE bool operator==(const Fog::__G_STRING& a, const Fog::StubUtf32& b) { return  a.eq(b); }
static FOG_INLINE bool operator!=(const Fog::__G_STRING& a, const Fog::StubUtf32& b) { return !a.eq(b); }
static FOG_INLINE bool operator<=(const Fog::__G_STRING& a, const Fog::StubUtf32& b) { return  a.compare(b) <= 0; }
static FOG_INLINE bool operator< (const Fog::__G_STRING& a, const Fog::StubUtf32& b) { return  a.compare(b) <  0; }
static FOG_INLINE bool operator>=(const Fog::__G_STRING& a, const Fog::StubUtf32& b) { return  a.compare(b) >= 0; }
static FOG_INLINE bool operator> (const Fog::__G_STRING& a, const Fog::StubUtf32& b) { return  a.compare(b) >  0; }

static FOG_INLINE bool operator==(const Fog::StubUtf32& a, const Fog::__G_STRING& b) { return  b.eq(a); }
static FOG_INLINE bool operator!=(const Fog::StubUtf32& a, const Fog::__G_STRING& b) { return !b.eq(a); }
static FOG_INLINE bool operator<=(const Fog::StubUtf32& a, const Fog::__G_STRING& b) { return  b.compare(a) >= 0; }
static FOG_INLINE bool operator< (const Fog::StubUtf32& a, const Fog::__G_STRING& b) { return  b.compare(a) >  0; }
static FOG_INLINE bool operator>=(const Fog::StubUtf32& a, const Fog::__G_STRING& b) { return  b.compare(a) <= 0; }
static FOG_INLINE bool operator> (const Fog::StubUtf32& a, const Fog::__G_STRING& b) { return  b.compare(a) <  0; }
#endif // __G_SIZE

//! @}

// ============================================================================
// [Fog::TypeInfo<>]
// ============================================================================

FOG_DECLARE_TYPEINFO(Fog::__G_STRING,
  Fog::MoveableType |
  Fog::TypeInfoHasCompare |
  Fog::TypeInfoHasEq)

FOG_DECLARE_TYPEINFO(Fog::__G_STRINGMATCHER, Fog::MoveableType)

// [Generator]
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

#endif // __G_GENERATE
