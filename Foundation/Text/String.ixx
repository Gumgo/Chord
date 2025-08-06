export module Chord.Foundation:Text.String;

import std;

import :Containers.Span;
import :Core;
import :Utilities.Copy;

namespace Chord
{
  // The underlying data for strings is shared and ref-counted (except for unmanaged strings, see the note below). If a string's ref-count is 1, the string can
  // be mutated in-place rather than making a copy. A few things to take note of:
  // - If the ref-count is 1, it means that this is the only string object pointing to the string data, which means that no other threads are holding onto a
  //   reference, so we can safely modify the string data in-place
  // - When sharing a string across multiple threads, a by-value copy of the string MUST be made. Otherwise, it would be possible to mutate a string on one
  //   thread while it is being accessed on another thread.
  // - As long as the above point is adhered to, std::memory_order_relaxed can be used when changing the string data's ref-count. This is because when passing
  //   a String to another thread, the copy is first made on the original thread, meaning access to the string data does not need synchronization. Once the copy
  //   is made, the ref-count is at least 2, so the string's data will never change after being accessed on another thread for the first time.
  // - An empty string NEVER has StringData associated with it. Any operation that produces an empty string must decrement its ref-count and set m_pointer to
  //   null (and m_length to 0).
  template<fixed_char TChar>
  struct StringData
  {
    StringData(TChar* pointer, usz length, usz capacity)
      : m_pointer(pointer)
      , m_length(length)
      , m_capacity(capacity)
      { }

    StringData(const StringData&) = delete;
    StringData& operator=(const StringData&) = delete;

    std::atomic<usz> m_refCount = 1;
    TChar* m_pointer = nullptr;
    usz m_length = 0;
    usz m_capacity = 0;
  };

  enum class ModifyMode
  {
    Copy,
    InPlace,
  };

  template<fixed_char TCharA, fixed_char TCharB>
  using BiggerChar = std::conditional_t<(sizeof(TCharA) > sizeof(TCharB)), TCharA, TCharB>;

  // Technically, char could be unsigned, so make sure that's not the case
  static_assert(std::is_signed_v<char>);

  template<fixed_char TChar>
  constexpr auto CharToInt(TChar c)
  {
    if constexpr (std::same_as<TChar, char>)
      { return s8(c); }
    else if constexpr (std::same_as<TChar, char32_t>)
      { return u32(c); }
    else
      { static_assert(AlwaysFalse<TChar>, "Unsupported fixed_char type"); }
  }

  template<fixed_char TCharA, fixed_char TCharB>
  constexpr bool CharEqual(TCharA a, TCharB b)
    { return std::cmp_equal(CharToInt(a), CharToInt(b)); }

  export
  {
    enum class Unmanaged_t { Unmanaged };
    using enum Unmanaged_t;

    template<fixed_char TChar> constexpr TChar DefaultReplacementCharacter = 0;
    template<> constexpr char DefaultReplacementCharacter<char> = '?';
    template<> constexpr char32_t DefaultReplacementCharacter<char32_t> = U'\ufffd';

    // Strings can treat data in two possible ways, "managed" and "unmanaged". Managed strings internally hold a pointer to StringData which is allocated and
    // ref-counted. Unmanaged strings are provided with a pointer and character count on initialization and assume that the string data won't be deallocated
    // until the string object goes out of scope.
    template<fixed_char TChar>
    class String
    {
    public:
      constexpr String() = default;

      constexpr String(const String& other);
      constexpr String(String&& other) noexcept;
      template<fixed_char TOtherChar> requires (!std::is_same_v<TChar, TOtherChar>)
      constexpr String(const String<TOtherChar>& other, TChar replacementCharacter = DefaultReplacementCharacter<TChar>);
      constexpr String(const TChar* pointer);
      template<fixed_char TOtherChar> requires (!std::is_same_v<TChar, TOtherChar>)
      constexpr String(const TOtherChar* pointer, TChar replacementCharacter = DefaultReplacementCharacter<TChar>);
      constexpr String(Span<const TChar> span);
      template<fixed_char TOtherChar> requires (!std::is_same_v<TChar, TOtherChar>)
      constexpr String(Span<const TOtherChar> span, TChar replacementCharacter = DefaultReplacementCharacter<TChar>);
      constexpr String(Unmanaged_t, Span<const TChar> span);

      constexpr ~String() noexcept;

      constexpr String& operator=(const String& other);
      constexpr String& operator=(String&& other) noexcept;

      constexpr Span<const TChar> AsSpan() const;
      constexpr Span<const TChar> AsSpan(basic_integral auto start, subspan_count auto count = ToEnd) const;

      constexpr const TChar* CharPtr() const;
      constexpr operator const TChar* () const;

      constexpr usz Length() const;
      constexpr bool IsEmpty() const;

      constexpr TChar CharAt(basic_integral auto index) const;
      constexpr TChar operator[](basic_integral auto index) const;

      constexpr void Clear();
      constexpr void Set(const TChar* pointer);
      template<fixed_char TOtherChar> requires (!std::is_same_v<TChar, TOtherChar>)
      constexpr void Set(const TOtherChar* pointer, TChar replacementCharacter = DefaultReplacementCharacter<TChar>);
      constexpr void Set(Span<const TChar> span);
      template<fixed_char TOtherChar> requires (!std::is_same_v<TChar, TOtherChar>)
      constexpr void Set(Span<const TOtherChar> span, TChar replacementCharacter = DefaultReplacementCharacter<TChar>);

      template<fixed_char TOtherChar> constexpr bool operator==(const String<TOtherChar>& other) const;
      template<fixed_char TOtherChar> constexpr bool operator==(const TOtherChar* other) const;

      template<fixed_char TOtherChar> constexpr std::optional<usz> FirstIndexOf(TOtherChar c) const;
      template<fixed_char TOtherChar> constexpr std::optional<usz> FirstIndexOf(const String<TOtherChar>& str) const;
      template<fixed_char TOtherChar> constexpr std::optional<usz> FirstIndexOf(const TOtherChar* str) const;

      template<fixed_char TOtherChar> constexpr std::optional<usz> LastIndexOf(TOtherChar c) const;
      template<fixed_char TOtherChar> constexpr std::optional<usz> LastIndexOf(const String<TOtherChar>& str) const;
      template<fixed_char TOtherChar> constexpr std::optional<usz> LastIndexOf(const TOtherChar* str) const;

      template<fixed_char TOtherChar> constexpr bool Contains(TOtherChar c) const;
      template<fixed_char TOtherChar> constexpr bool Contains(const String<TOtherChar>& str) const;
      template<fixed_char TOtherChar> constexpr bool Contains(const TOtherChar* str) const;

      template<fixed_char TOtherChar> constexpr bool StartsWith(TOtherChar c) const;
      template<fixed_char TOtherChar> constexpr bool StartsWith(const String<TOtherChar>& str) const;
      template<fixed_char TOtherChar> constexpr bool StartsWith(const TOtherChar* str) const;

      template<fixed_char TOtherChar> constexpr bool EndsWith(TOtherChar c) const;
      template<fixed_char TOtherChar> constexpr bool EndsWith(const String<TOtherChar>& str) const;
      template<fixed_char TOtherChar> constexpr bool EndsWith(const TOtherChar* str) const;

      constexpr void Append(TChar c);
      template<fixed_char TOtherChar> requires (!std::is_same_v<TChar, TOtherChar>)
      constexpr void Append(TOtherChar c, TChar replacementCharacter = DefaultReplacementCharacter<TChar>);
      constexpr void Append(const String<TChar>& str);
      template<fixed_char TOtherChar> requires (!std::is_same_v<TChar, TOtherChar>)
      constexpr void Append(const String<TOtherChar>& str, TChar replacementCharacter = DefaultReplacementCharacter<TChar>);
      constexpr void Append(const TChar* str);
      template<fixed_char TOtherChar> requires (!std::is_same_v<TChar, TOtherChar>)
      constexpr void Append(const TOtherChar* str, TChar replacementCharacter = DefaultReplacementCharacter<TChar>);

      constexpr void Insert(basic_integral auto index, TChar c);
      template<fixed_char TOtherChar> requires (!std::is_same_v<TChar, TOtherChar>)
      constexpr void Insert(basic_integral auto index, TOtherChar c, TChar replacementCharacter = DefaultReplacementCharacter<TChar>);
      constexpr void Insert(basic_integral auto index, const String<TChar>& str);
      template<fixed_char TOtherChar> requires (!std::is_same_v<TChar, TOtherChar>)
      constexpr void Insert(basic_integral auto index, const String<TOtherChar>& str, TChar replacementCharacter = DefaultReplacementCharacter<TChar>);
      constexpr void Insert(basic_integral auto index, const TChar* str);
      template<fixed_char TOtherChar> requires (!std::is_same_v<TChar, TOtherChar>)
      constexpr void Insert(basic_integral auto index, const TOtherChar* str, TChar replacementCharacter = DefaultReplacementCharacter<TChar>);

      constexpr void Remove(basic_integral auto start, subspan_count auto count);

      constexpr String<TChar> Substring(basic_integral auto start, subspan_count auto count) const;

      template<fixed_char TOtherChar> constexpr String<TChar>& operator+=(TOtherChar c);
      template<fixed_char TOtherChar> constexpr String<TChar>& operator+=(const String<TOtherChar>& str);
      template<fixed_char TOtherChar> constexpr String<TChar>& operator+=(const TOtherChar* str);

    private:
      constexpr StringData<TChar>* TryGetStringData() const;
      constexpr void SetStringData(StringData<TChar>* stringData);
      constexpr void TryIncrementRefCount();
      constexpr void TryDecrementRefCount();

      constexpr std::tuple<ModifyMode, StringData<TChar>*> PrepareForModification(usz newLength);

      template<bool Reverse, fixed_char TOtherChar> static constexpr std::optional<usz> IndexOfInternal(
        Span<const TChar> thisSpan,
        Span<const TOtherChar> otherSpan);

      template<fixed_char TOtherChar> static TChar ConvertCharacter(TOtherChar otherChar, TChar replacementCharacter);
      template<fixed_char TOtherChar> constexpr void InsertInternal(basic_integral auto index, Span<const TOtherChar> otherSpan, TChar replacementCharacter);

      // These internal fields can represent three possible cases:
      // - m_pointer == nullptr:
      //     This is an empty string
      // - m_pointer != nullptr && m_length > 0:
      //     This is an unmanaged string; m_pointer points to the actual string data and m_length is the actual character count
      // - m_pointer != nullptr && m_length == 0:
      //     This is a managed string; m_pointer points to a StringData instance
      void* m_pointer = nullptr;
      usz m_length = 0;
    };

    template<fixed_char TCharA, fixed_char TCharB>
    constexpr String<BiggerChar<TCharA, TCharB>> operator+(const String<TCharA>& strA, const String<TCharB>& strB);

    template<fixed_char TCharA, fixed_char TCharB>
    constexpr String<BiggerChar<TCharA, TCharB>> operator+(const TCharA* strA, const String<TCharB>& strB);

    template<fixed_char TCharA, fixed_char TCharB>
    constexpr String<BiggerChar<TCharA, TCharB>> operator+(const String<TCharA>& strA, const TCharB* strB);

    template<fixed_char TChar>
    constexpr String<TChar>::String(const String& other)
      : m_pointer(other.m_pointer)
      , m_length(other.m_length)
      { TryIncrementRefCount(); }

    template<fixed_char TChar>
    constexpr String<TChar>::String(String&& other) noexcept
      : m_pointer(std::exchange(other.m_pointer, nullptr))
      , m_length(std::exchange(other.m_length, 0_usz))
      { }

    template<fixed_char TChar>
    template<fixed_char TOtherChar> requires (!std::is_same_v<TChar, TOtherChar>)
    constexpr String<TChar>::String(const String<TOtherChar> &other, TChar replacementCharacter)
      { Set(Span(other.CharPtr(), other.Length()), replacementCharacter); }

    template<fixed_char TChar>
    constexpr String<TChar>::String(const TChar* pointer)
      { Set(pointer); }

    template<fixed_char TChar>
    template<fixed_char TOtherChar> requires (!std::is_same_v<TChar, TOtherChar>)
    constexpr String<TChar>::String(const TOtherChar* pointer, TChar replacementCharacter)
      { Set(pointer, replacementCharacter); }

    template<fixed_char TChar>
    constexpr String<TChar>::String(Span<const TChar> span)
      { Set(span); }

    template<fixed_char TChar>
    template<fixed_char TOtherChar> requires (!std::is_same_v<TChar, TOtherChar>)
    constexpr String<TChar>::String(Span<const TOtherChar> span, TChar replacementCharacter)
      { Set(span, replacementCharacter); }


    template<fixed_char TChar>
    constexpr String<TChar>::String(Unmanaged_t, Span<const TChar> span)
      : m_pointer(span.IsEmpty() ? nullptr : const_cast<TChar*>(span.Elements()))
      , m_length(span.Count())
      { }

    template<fixed_char TChar>
    constexpr String<TChar>::~String() noexcept
      { Clear(); }

    template<fixed_char TChar>
    constexpr String<TChar>& String<TChar>::operator=(const String& other)
    {
      if (this != &other)
      {
        Clear();
        m_pointer = other.m_pointer;
        m_length = other.m_length;
        TryIncrementRefCount();
      }

      return *this;
    }

    template<fixed_char TChar>
    constexpr String<TChar>& String<TChar>::operator=(String&& other) noexcept
    {
      if (this != &other)
      {
        Clear();
        m_pointer = std::exchange(other.m_pointer, nullptr);
        m_length = std::exchange(other.m_length, 0_usz);
      }

      return *this;
    }

    template<fixed_char TChar>
    constexpr Span<const TChar> String<TChar>::AsSpan() const
      { return AsSpan(0, ToEnd); }

    template<fixed_char TChar>
    constexpr Span<const TChar> String<TChar>::AsSpan(basic_integral auto start, subspan_count auto count) const
    {
      StringData<TChar>* stringData = TryGetStringData();
      auto [pointer, length] = (stringData == nullptr)
        ? std::make_tuple(reinterpret_cast<const TChar*>(m_pointer), m_length)
        : std::make_tuple(stringData->m_pointer, stringData->m_length);
      auto span = Span(pointer, length);
      return Span(span, start, count);
    }

    template<fixed_char TChar>
    constexpr const TChar* String<TChar>::CharPtr() const
    {
      StringData<TChar>* stringData = TryGetStringData();
      return stringData != nullptr ? stringData->m_pointer : reinterpret_cast<const TChar*>(m_pointer);
    }

    template<fixed_char TChar>
    constexpr String<TChar>::operator const TChar* () const
      { return CharPtr(); }

    template<fixed_char TChar>
    constexpr usz String<TChar>::Length() const
    {
      StringData<TChar>* stringData = TryGetStringData();
      return stringData != nullptr ? stringData->m_length : m_length;
    }

    template<fixed_char TChar>
    constexpr bool String<TChar>::IsEmpty() const
      { return m_pointer == nullptr; } // Note: this relies on the fact that we will never have valid but length-0 string data

    template<fixed_char TChar>
    constexpr TChar String<TChar>::CharAt(basic_integral auto index) const
      { return AsSpan()[index]; }

    template<fixed_char TChar>
    constexpr TChar String<TChar>::operator[](basic_integral auto index) const
      { return CharAt(index); }

    template<fixed_char TChar>
    constexpr void String<TChar>::Clear()
    {
      TryDecrementRefCount();
      m_pointer = nullptr;
      m_length = 0;
    }

    template<fixed_char TChar>
    constexpr void String<TChar>::Set(const TChar* pointer)
    {
      ASSERT(pointer != nullptr);
      Set(Span(pointer, NullTerminatedStringLength(pointer)));
    }

    template<fixed_char TChar>
    template<fixed_char TOtherChar> requires (!std::is_same_v<TChar, TOtherChar>)
    constexpr void String<TChar>::Set(const TOtherChar* pointer, TChar replacementCharacter)
    {
      ASSERT(pointer != nullptr);
      Set(Span(pointer, NullTerminatedStringLength(pointer)), replacementCharacter);
    }

    template<fixed_char TChar>
    constexpr void String<TChar>::Set(Span<const TChar> span)
    {
      if (span.IsEmpty())
      {
        Clear();
        return;
      }

      auto [modifyMode, stringData] = PrepareForModification(span.Count());

      Copy(stringData->m_pointer, span.Elements(), span.Count());
      if (modifyMode == ModifyMode::Copy)
        { SetStringData(stringData); }
      else
      {
        ASSERT(modifyMode == ModifyMode::InPlace);
        stringData->m_length = span.Count();
      }
    }

    template<fixed_char TChar>
    template<fixed_char TOtherChar> requires (!std::is_same_v<TChar, TOtherChar>)
    constexpr void String<TChar>::Set(Span<const TOtherChar> span, TChar replacementCharacter)
    {
      if (span.IsEmpty())
      {
        Clear();
        return;
      }

      auto [modifyMode, stringData] = PrepareForModification(span.Count());

      for (usz i = 0; i < span.Count(); i++)
        { stringData->m_pointer[i] = ConvertCharacter(span[i], replacementCharacter); }

      if (modifyMode == ModifyMode::Copy)
        { SetStringData(stringData); }
      else
      {
        ASSERT(modifyMode == ModifyMode::InPlace);
        stringData->m_length = span.Count();
      }
    }

    template<fixed_char TChar>
    template<fixed_char TOtherChar> constexpr bool String<TChar>::operator==(const String<TOtherChar>& other) const
    {
      auto thisSpan = AsSpan();
      auto otherSpan = other.AsSpan();
      if (thisSpan.Count() != otherSpan.Count())
        { return false; }
      for (auto [a, b] : std::views::zip(thisSpan, otherSpan))
      {
        if (!CharEqual(a, b))
          { return false; }
      }

      return true;
    }

    template<fixed_char TChar>
    template<fixed_char TOtherChar> constexpr bool String<TChar>::operator==(const TOtherChar* other) const
    {
      ASSERT(other != nullptr);
      auto thisSpan = AsSpan();
      const TOtherChar* otherChar = other;
      for (TChar a : thisSpan)
      {
        TOtherChar b = *otherChar;
        otherChar++;
        if (b == 0 || !CharEqual(a, b))
          { return false; }
      }

      return *otherChar == 0;
    }

    template<fixed_char TChar>
    template<fixed_char TOtherChar> constexpr std::optional<usz> String<TChar>::FirstIndexOf(TOtherChar c) const
    {
      auto span = AsSpan();
      for (usz i = 0; i < span.Count(); i++)
      {
        if (CharEqual(c, span[i]))
          { return i; }
      }

      return std::nullopt;
    }

    template<fixed_char TChar>
    template<fixed_char TOtherChar> constexpr std::optional<usz> String<TChar>::FirstIndexOf(const String<TOtherChar>& str) const
      { return IndexOfInternal<false>(AsSpan(), str.AsSpan()); }

    template<fixed_char TChar>
    template<fixed_char TOtherChar> constexpr std::optional<usz> String<TChar>::FirstIndexOf(const TOtherChar* str) const
      { return IndexOfInternal<false>(AsSpan(), Span(str, NullTerminatedStringLength(str))); }

    template<fixed_char TChar>
    template<fixed_char TOtherChar> constexpr std::optional<usz> String<TChar>::LastIndexOf(TOtherChar c) const
    {
      auto thisSpan = AsSpan();
      for (usz i = 0; i < thisSpan.Count(); i++)
      {
        usz j = thisSpan.Count() - i - 1;
        if (CharEqual(thisSpan[j], c))
          { return j; }
      }

      return std::nullopt;
    }

    template<fixed_char TChar>
    template<fixed_char TOtherChar> constexpr std::optional<usz> String<TChar>::LastIndexOf(const String<TOtherChar>& str) const
      { return IndexOfInternal<true>(AsSpan(), str.AsSpan()); }


    template<fixed_char TChar>
    template<fixed_char TOtherChar> constexpr std::optional<usz> String<TChar>::LastIndexOf(const TOtherChar* str) const
      { return IndexOfInternal<true>(AsSpan(), Span(str, NullTerminatedStringLength(str))); }

    template<fixed_char TChar>
    template<fixed_char TOtherChar> constexpr bool String<TChar>::Contains(TOtherChar c) const
      { return FirstIndexOf(c).has_value(); }

    template<fixed_char TChar>
    template<fixed_char TOtherChar> constexpr bool String<TChar>::Contains(const String<TOtherChar>& str) const
      { return FirstIndexOf(str).has_value(); }


    template<fixed_char TChar>
    template<fixed_char TOtherChar> constexpr bool String<TChar>::Contains(const TOtherChar* str) const
      { return FirstIndexOf(str).has_value(); }

    template<fixed_char TChar>
    template<fixed_char TOtherChar> constexpr bool String<TChar>::StartsWith(TOtherChar c) const
    {
      auto thisSpan = AsSpan();
      return thisSpan.Count() >= 1 && CharEqual(thisSpan[0], c);
    }

    template<fixed_char TChar>
    template<fixed_char TOtherChar> constexpr bool String<TChar>::StartsWith(const String<TOtherChar>& str) const
    {
      auto thisSpan = AsSpan();
      auto otherSpan = str.AsSpan();
      if (thisSpan.Count() < otherSpan.Count())
        { return false; }
      return IndexOfInternal<false>(Span(thisSpan, 0, otherSpan.Count()), otherSpan).has_value();
    }

    template<fixed_char TChar>
    template<fixed_char TOtherChar> constexpr bool String<TChar>::StartsWith(const TOtherChar* str) const
    {
      auto thisSpan = AsSpan();
      auto otherSpan = Span(str, NullTerminatedStringLength(str));
      if (thisSpan.Count() < otherSpan.Count())
        { return false; }
      return IndexOfInternal<false>(Span(thisSpan, 0, otherSpan.Count()), otherSpan).has_value();
    }

    template<fixed_char TChar>
    template<fixed_char TOtherChar> constexpr bool String<TChar>::EndsWith(TOtherChar c) const
    {
      auto thisSpan = AsSpan();
      return thisSpan.Count() >= 1 && CharEqual(thisSpan[thisSpan.Count() - 1], c);
    }

    template<fixed_char TChar>
    template<fixed_char TOtherChar> constexpr bool String<TChar>::EndsWith(const String<TOtherChar>& str) const
    {
      auto thisSpan = AsSpan();
      auto otherSpan = str.AsSpan();
      if (thisSpan.Count() < otherSpan.Count())
        { return false; }
      return IndexOfInternal<false>(Span(thisSpan, thisSpan.Count() - otherSpan.Count(), otherSpan.Count()), otherSpan).has_value();
    }

    template<fixed_char TChar>
    template<fixed_char TOtherChar> constexpr bool String<TChar>::EndsWith(const TOtherChar* str) const
    {
      auto thisSpan = AsSpan();
      auto otherSpan = Span(str, NullTerminatedStringLength(str));
      if (thisSpan.Count() < otherSpan.Count())
        { return false; }
      return IndexOfInternal<false>(Span(thisSpan, thisSpan.Count() - otherSpan.Count(), otherSpan.Count()), otherSpan).has_value();
    }

    template<fixed_char TChar>
    constexpr StringData<TChar>* String<TChar>::TryGetStringData() const
      { return m_pointer != nullptr && m_length == 0 ? reinterpret_cast<StringData<TChar>*>(m_pointer) : nullptr; }

    template<fixed_char TChar>
    constexpr void String<TChar>::SetStringData(StringData<TChar>* stringData)
    {
      Clear();
      m_pointer = stringData;
    }

    template<fixed_char TChar>
    constexpr void String<TChar>::TryIncrementRefCount()
    {
      StringData<TChar>* stringData = TryGetStringData();
      if (stringData != nullptr)
        { stringData->m_refCount.fetch_add(1, std::memory_order_relaxed); }
    }

    template<fixed_char TChar>
    constexpr void String<TChar>::TryDecrementRefCount()
    {
      StringData<TChar>* stringData = TryGetStringData();
      if (stringData != nullptr)
      {
        usz oldRefCount = stringData->m_refCount.fetch_sub(1, std::memory_order_relaxed);
        if (oldRefCount == 0)
        {
          std::allocator<TChar>().deallocate(stringData->m_pointer, stringData->m_capacity);
          delete stringData;
        }
      }
    }

    template<fixed_char TChar>
    constexpr std::tuple<ModifyMode, StringData<TChar>*> String<TChar>::PrepareForModification(usz newLength)
    {
      // If the new length would be 0, we should just clear the string
      ASSERT(newLength > 0);

      StringData<TChar>* stringData = TryGetStringData();

      // If we already have string data, we hold the only reference to it, and it has capacity to spare, we can do an in-place modification
      if (stringData != nullptr && stringData->m_refCount.load(std::memory_order_relaxed) == 1 && stringData->m_capacity >= newLength)
        { return std::make_tuple(ModifyMode::InPlace, stringData); }

      return std::make_tuple(
        ModifyMode::Copy,
        new StringData<TChar>(std::allocator<TChar>().allocate(newLength), newLength, newLength));
    }

    template<fixed_char TChar>
    template<bool Reverse, fixed_char TOtherChar> constexpr std::optional<usz> String<TChar>::IndexOfInternal(
      Span<const TChar> thisSpan,
      Span<const TOtherChar> otherSpan)
    {
      if (thisSpan.Count() < otherSpan.Count())
        { return std::nullopt; }

      usz endIndex = thisSpan.Count() - otherSpan.Count() + 1;
      for (usz i = 0; i < endIndex; i++)
      {
        usz offset = Reverse ? (endIndex - i - 1) : i;
        bool found = true;
        for (usz j = 0; j < otherSpan.Count(); j++)
        {
          if (!CharEqual(thisSpan[offset + j], otherSpan[j]))
          {
            found = false;
            break;
          }
        }

        if (found)
          { return offset; }
      }

      return std::nullopt;
    }

    template<fixed_char TChar>
    template<fixed_char TOtherChar> TChar String<TChar>::ConvertCharacter(TOtherChar otherChar, TChar replacementCharacter)
      { return CanCoerce<TChar>(otherChar) ? TChar(otherChar) : replacementCharacter; }

    template<fixed_char TChar>
    template<fixed_char TOtherChar>
    constexpr void String<TChar>::InsertInternal(
      basic_integral auto index,
      Span<const TOtherChar> otherSpan,
      TChar replacementCharacter)
    {
      auto thisSpan = AsSpan();
      usz evaluatedIndex = thisSpan.EvaluateSubspanStart(index);

      if (otherSpan.IsEmpty())
        { return; }

      usz newLength = thisSpan.Count() + otherSpan.Count();
      usz afterInsertCount = thisSpan.Count() - evaluatedIndex;
      auto [modifyMode, stringData] = PrepareForModification(newLength);

      if (modifyMode == ModifyMode::Copy)
      {
        Copy(stringData->m_pointer, thisSpan.GetBuffer(0, evaluatedIndex), evaluatedIndex);
        for (usz i = 0; i < otherSpan.Count(); i++)
          { stringData->m_pointer[evaluatedIndex + i] = ConvertCharacter(otherSpan[i], replacementCharacter); }
        Copy(&stringData->m_pointer[evaluatedIndex + otherSpan.Count()], thisSpan.GetBuffer(evaluatedIndex, afterInsertCount), afterInsertCount);
        SetStringData(stringData);
      }
      else
      {
        ASSERT(modifyMode == ModifyMode::InPlace);

        // Make a space for insertion
        CopyOverlapping(&stringData->m_pointer[evaluatedIndex + otherSpan.Count()], &stringData->m_pointer[evaluatedIndex], afterInsertCount);
        for (usz i = 0; i < otherSpan.Count(); i++)
          { stringData->m_pointer[evaluatedIndex + i] = ConvertCharacter(otherSpan[i], replacementCharacter); }
        stringData->m_length = newLength;
      }
    }

    template<fixed_char TChar>
    constexpr void String<TChar>::Append(TChar c)
      { InsertInternal(Length(), Span<const TChar>(&c, 1), DefaultReplacementCharacter<TChar>); }

    template<fixed_char TChar>
    template<fixed_char TOtherChar> requires (!std::is_same_v<TChar, TOtherChar>)
    constexpr void String<TChar>::Append(TOtherChar c, TChar replacementCharacter)
      { InsertInternal(Length(), Span<const TOtherChar>(&c, 1), replacementCharacter); }

    template<fixed_char TChar>
    constexpr void String<TChar>::Append(const String<TChar>& str)
      { InsertInternal(Length(), str.AsSpan(), DefaultReplacementCharacter<TChar>); }

    template<fixed_char TChar>
    template<fixed_char TOtherChar> requires (!std::is_same_v<TChar, TOtherChar>)
    constexpr void String<TChar>::Append(const String<TOtherChar>& str, TChar replacementCharacter)
      { InsertInternal(Length(), str.AsSpan(), replacementCharacter); }

    template<fixed_char TChar>
    constexpr void String<TChar>::Append(const TChar* str)
      { InsertInternal(Length(), Span(str, NullTerminatedStringLength(str)), DefaultReplacementCharacter<TChar>); }

    template<fixed_char TChar>
    template<fixed_char TOtherChar> requires (!std::is_same_v<TChar, TOtherChar>)
    constexpr void String<TChar>::Append(const TOtherChar* str, TChar replacementCharacter)
      { InsertInternal(Length(), Span(str, NullTerminatedStringLength(str)), replacementCharacter); }

    template<fixed_char TChar>
    constexpr void String<TChar>::Insert(basic_integral auto index, TChar c)
      { InsertInternal(index, Span<const TChar>(&c, 1), DefaultReplacementCharacter<TChar>); }

    template<fixed_char TChar>
    template<fixed_char TOtherChar> requires (!std::is_same_v<TChar, TOtherChar>)
    constexpr void String<TChar>::Insert(basic_integral auto index, TOtherChar c, TChar replacementCharacter)
      { InsertInternal(index, Span<const TOtherChar>(&c, 1), replacementCharacter); }

    template<fixed_char TChar>
    constexpr void String<TChar>::Insert(basic_integral auto index, const String<TChar>& str)
      { InsertInternal(index, str.AsSpan(), DefaultReplacementCharacter<TChar>); }

    template<fixed_char TChar>
    template<fixed_char TOtherChar> requires (!std::is_same_v<TChar, TOtherChar>)
    constexpr void String<TChar>::Insert(basic_integral auto index, const String<TOtherChar>& str, TChar replacementCharacter)
      { InsertInternal(index, str.AsSpan(), replacementCharacter); }

    template<fixed_char TChar>
    constexpr void String<TChar>::Insert(basic_integral auto index, const TChar* str)
      { InsertInternal(index, Span(str, NullTerminatedStringLength(str)), DefaultReplacementCharacter<TChar>); }

    template<fixed_char TChar>
    template<fixed_char TOtherChar> requires (!std::is_same_v<TChar, TOtherChar>)
    constexpr void String<TChar>::Insert(basic_integral auto index, const TOtherChar* str, TChar replacementCharacter)
      { InsertInternal(index, Span(str, NullTerminatedStringLength(str)), replacementCharacter); }

    template<fixed_char TChar>
    constexpr void String<TChar>::Remove(basic_integral auto start, subspan_count auto count)
    {
      auto thisSpan = AsSpan();
      usz evaluatedStart = thisSpan.EvaluateSubspanStart(start);
      usz evaluatedCount = thisSpan.EvaluateSubspanCount(evaluatedStart, count);

      usz newLength = thisSpan.Count() - evaluatedCount;
      if (newLength == 0)
      {
        Clear();
        return;
      }

      usz afterRemoveCount = newLength - evaluatedStart;
      auto [modifyMode, stringData] = PrepareForModification(newLength);

      if (modifyMode == ModifyMode::Copy)
      {
        Copy(stringData->m_pointer, thisSpan.GetBuffer(0, evaluatedStart), evaluatedStart);
        Copy(&stringData->m_pointer[evaluatedStart], thisSpan.GetBuffer(evaluatedStart + evaluatedCount, afterRemoveCount), afterRemoveCount);
        SetStringData(stringData);
      }
      else
      {
        ASSERT(modifyMode == ModifyMode::InPlace);
        CopyOverlapping(&stringData->m_pointer[evaluatedStart], &stringData->m_pointer[evaluatedStart + evaluatedCount], afterRemoveCount);
        stringData->m_length = newLength;
      }
    }

    template<fixed_char TChar>
    constexpr String<TChar> String<TChar>::Substring(basic_integral auto start, subspan_count auto count) const
    {
      auto thisSpan = AsSpan();
      usz evaluatedStart = thisSpan.EvaluateSubspanStart(start);
      usz evaluatedCount = thisSpan.EvaluateSubspanCount(evaluatedStart, count);

      // Simply return a copy if the substring covers the whole string
      if (evaluatedStart == 0 && evaluatedCount == thisSpan.Count())
        { return *this; }

      // If this is an unmanaged string, we can directly return a substring
      return (m_pointer == nullptr || m_length > 0)
        ? String(Unmanaged, Span(thisSpan, start, count))
        : String(Span(thisSpan, start, count));
    }

    template<fixed_char TChar>
    template<fixed_char TOtherChar>
    constexpr String<TChar>& String<TChar>::operator+=(TOtherChar c)
    {
      Append(c);
      return *this;
    }

    template<fixed_char TChar>
    template<fixed_char TOtherChar>
    constexpr String<TChar>& String<TChar>::operator+=(const String<TOtherChar>& str)
    {
      Append(str);
      return *this;
    }

    template<fixed_char TChar>
    template<fixed_char TOtherChar>
    constexpr String<TChar>& String<TChar>::operator+=(const TOtherChar* str)
    {
      Append(str);
      return *this;
    }

    template<fixed_char TCharA, fixed_char TCharB>
    constexpr String<BiggerChar<TCharA, TCharB>> operator+(const String<TCharA>& strA, const String<TCharB>& strB)
    {
      String<BiggerChar<TCharA, TCharB>> result(strA);
      result += strB;
      return result;
    }

    template<fixed_char TCharA, fixed_char TCharB>
    constexpr String<BiggerChar<TCharA, TCharB>> operator+(const TCharA* strA, const String<TCharB>& strB)
    {
      String<BiggerChar<TCharA, TCharB>> result(strA);
      result += strB;
      return result;
    }

    template<fixed_char TCharA, fixed_char TCharB>
    constexpr String<BiggerChar<TCharA, TCharB>> operator+(const String<TCharA>& strA, const TCharB* strB)
    {
      String<BiggerChar<TCharA, TCharB>> result(strA);
      result += strB;
      return result;
    }

    using AsciiString = String<char>;
    using UnicodeString = String<char32_t>;
  }
}