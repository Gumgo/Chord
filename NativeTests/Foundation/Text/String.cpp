module Chord.Tests;

import std;

import Chord.Foundation;
import :Test;

namespace Chord
{
  template<fixed_char TChar, usz Length>
  struct StringLiteralWrapper
  {
    TChar m_data[Length];
    constexpr const TChar* Pointer() const
      { return m_data; }
  };

  template<fixed_char TChar>
  class SharedStringTests
  {
  public:
    using OtherChar = std::conditional_t<std::same_as<TChar, char>, char32_t, char>;
    using TypedString = String<TChar>;
    using OtherTypedString = String<OtherChar>;

    template<usz Length>
    static consteval auto Str(const char (&str)[Length])
    {
      StringLiteralWrapper<TChar, Length> data;
      for (usz i = 0; i < Length; i++)
        { data.m_data[i] = TChar(str[i]); }
      return data;
    }

    template<usz Length>
    static consteval auto StrOther(const char(&str)[Length])
    {
      StringLiteralWrapper<OtherChar, Length> data;
      for (usz i = 0; i < Length; i++)
        { data.m_data[i] = OtherChar(str[i]); }
      return data;
    }

    static constexpr void DefaultConstruct()
    {
      TypedString str;
      EXPECT(str.IsEmpty());
    }

    static constexpr void CopyConstruct()
    {
      TypedString str(Str("test").Pointer());
      TypedString str2(str);
      EXPECT(str == "test");
      EXPECT(str2 == "test");
    }

    static constexpr void MoveConstruct()
    {
      TypedString str(Str("test").Pointer());
      TypedString str2(std::move(str));
      EXPECT(str.IsEmpty());
      EXPECT(str2 == "test");
    }

    static constexpr void ConstructFromNullTerminatedString()
    {
      TypedString strA(Str("test").Pointer());
      TypedString strB(StrOther("test").Pointer());
      EXPECT(strA == "test");
      EXPECT(strB == "test");
    }

    static constexpr void ConstructFromPointerAndLength()
    {
      TypedString strA(Span(Str("test").Pointer(), 4));
      TypedString strB(Span(StrOther("test").Pointer(), 4));
      EXPECT(strA == "test");
      EXPECT(strB == "test");
    }

    static constexpr void ConstructUnmanaged()
    {
      TypedString str(Unmanaged, Span(Str("test").Pointer(), 4));
      EXPECT(str == "test");
    }

    static constexpr void CopyAssign()
    {
      TypedString str(Str("test").Pointer());
      TypedString str2;
      str2 = str;
      EXPECT(str == "test");
      EXPECT(str2 == "test");
    }

    static constexpr void MoveAssign()
    {
      TypedString str(Str("test").Pointer());
      TypedString str2;
      str2 = std::move(str);
      EXPECT(str.IsEmpty());
      EXPECT(str2 == "test");
    }

    static constexpr void CreateForWrite()
    {
      auto [str, buffer] = TypedString::CreateForWrite(4);
      EXPECT(buffer.Count() == 4);
      buffer[0] = TChar('t');
      buffer[1] = TChar('e');
      buffer[2] = TChar('s');
      buffer[3] = TChar('t');
      EXPECT(str == "test");
    }

    static constexpr void AsSpan()
    {
      TypedString str(Str("test").Pointer());
      auto span = str.AsSpan();
      EXPECT(span.Count() == 4);
      EXPECT(span[0] == 't');
      EXPECT(span[1] == 'e');
      EXPECT(span[2] == 's');
      EXPECT(span[3] == 't');
    }

    static constexpr void CharPtr()
    {
      TypedString strA;
      TypedString strB(Str("test").Pointer());
      TypedString strC(Unmanaged, Span(Str("test").Pointer(), 4));
      EXPECT(strA.CharPtr() == nullptr);
      EXPECT(strB.CharPtr() != nullptr);
      EXPECT(strB.CharPtr()[0] == 't');
      EXPECT(strB.CharPtr()[1] == 'e');
      EXPECT(strB.CharPtr()[2] == 's');
      EXPECT(strB.CharPtr()[3] == 't');
      EXPECT(strC.CharPtr() != nullptr);
      EXPECT(strC.CharPtr()[0] == 't');
      EXPECT(strC.CharPtr()[1] == 'e');
      EXPECT(strC.CharPtr()[2] == 's');
      EXPECT(strC.CharPtr()[3] == 't');
    }

    static constexpr void Length()
    {
      TypedString strA;
      TypedString strB(Str("test").Pointer());
      TypedString strC(Str("longer test").Pointer());
      TypedString strD(Unmanaged, Span(Str("test").Pointer(), 4));
      TypedString strE(Unmanaged, Span(Str("longer test").Pointer(), 11));
      EXPECT(strA.Length() == 0);
      EXPECT(strA.IsEmpty());
      EXPECT(strB.Length() == 4);
      EXPECT(!strB.IsEmpty());
      EXPECT(strC.Length() == 11);
      EXPECT(!strC.IsEmpty());
      EXPECT(strD.Length() == 4);
      EXPECT(!strD.IsEmpty());
      EXPECT(strE.Length() == 11);
      EXPECT(!strE.IsEmpty());
    }

    static constexpr void CharAt()
    {
      TypedString strA(Str("test").Pointer());
      TypedString strB(Unmanaged, Span(Str("test").Pointer(), 4));

      EXPECT(strA.CharPtr()[0] == 't');
      EXPECT(strA.CharPtr()[1] == 'e');
      EXPECT(strA.CharPtr()[2] == 's');
      EXPECT(strA.CharPtr()[3] == 't');
      EXPECT(strB.CharPtr()[0] == 't');
      EXPECT(strB.CharPtr()[1] == 'e');
      EXPECT(strB.CharPtr()[2] == 's');
      EXPECT(strB.CharPtr()[3] == 't');

      EXPECT(strA[0] == 't');
      EXPECT(strA[1] == 'e');
      EXPECT(strA[2] == 's');
      EXPECT(strA[3] == 't');
      EXPECT(strB[0] == 't');
      EXPECT(strB[1] == 'e');
      EXPECT(strB[2] == 's');
      EXPECT(strB[3] == 't');
    }

    static constexpr void Clear()
    {
      TypedString str(Str("test").Pointer());
      TypedString str2(Str("test").Pointer());
      TypedString strCopy = str;
      str.Clear();
      str2.Clear();
      EXPECT(str.IsEmpty());
      EXPECT(str2.IsEmpty());
      EXPECT(!strCopy.IsEmpty());
    }

    static constexpr void Set()
    {
      TypedString strA;
      strA.Set(Str("test").Pointer());
      EXPECT(strA == "test");

      TypedString strB;
      strB.Set(StrOther("test").Pointer());
      EXPECT(strB == "test");

      TypedString strC;
      strC.Set(Span(Str("test").Pointer(), 4));
      EXPECT(strC == "test");
      strC.Set(Span<TChar>());
      EXPECT(strC.IsEmpty());

      TypedString strD;
      strD.Set(Span(StrOther("test").Pointer(), 4));
      EXPECT(strD == "test");
      strD.Set(Span<const OtherChar>());
      EXPECT(strD.IsEmpty());

      TypedString strE;
      strE.Set(Str("longest test string").Pointer());
      EXPECT(strE == "longest test string");
      auto charPointerA = strE.CharPtr();

      TypedString strF = strE;
      strE.Set(Str("long test").Pointer());
      EXPECT(strE == "long test");
      EXPECT(strF == "longest test string");
      EXPECT(strF.CharPtr() == charPointerA); // strF should point to the originally-set string data
      auto charPointerB = strE.CharPtr();
      EXPECT(charPointerA != charPointerB); // strE should have been reallocated because it is shared with strF

      strE.Set("test2");
      EXPECT(strE == "test2");
      EXPECT(strF == "longest test string");
      EXPECT(strE.CharPtr() == charPointerB); // Going from "longer test" to "test2" reduces length so the memory should be reused
    }

    static constexpr void Equals()
    {
      TypedString str(Str("abc").Pointer());
      TypedString strA(Str("abc").Pointer());
      OtherTypedString strB(Str("abc").Pointer());
      TypedString strC(Str("123").Pointer());
      OtherTypedString strD(Str("123").Pointer());
      EXPECT(str == Str("abc").Pointer());
      EXPECT(str == StrOther("abc").Pointer());
      EXPECT(str != Str("123").Pointer());
      EXPECT(str != StrOther("123").Pointer());
      EXPECT(strA == strA);
      EXPECT(strA == strB);
      EXPECT(strA != strC);
      EXPECT(strA != strD);
    }

    static constexpr void FirstIndexOf()
    {
      TypedString str(Str("foo bar quack bar florg").Pointer());

      EXPECT(str.FirstIndexOf(TChar('b')) == 4_usz);
      EXPECT(str.FirstIndexOf(OtherChar('b')) == 4_usz);
      EXPECT(str.FirstIndexOf(TypedString(Str("bar").Pointer())) == 4_usz);
      EXPECT(str.FirstIndexOf(OtherTypedString(StrOther("bar").Pointer())) == 4_usz);
      EXPECT(str.FirstIndexOf(Str("bar").Pointer()) == 4_usz);
      EXPECT(str.FirstIndexOf(StrOther("bar").Pointer()) == 4_usz);

      EXPECT(str.FirstIndexOf(TChar('z')) == std::nullopt);
      EXPECT(str.FirstIndexOf(OtherChar('z')) == std::nullopt);
      EXPECT(str.FirstIndexOf(TypedString(Str("zig").Pointer())) == std::nullopt);
      EXPECT(str.FirstIndexOf(OtherTypedString(StrOther("zig").Pointer())) == std::nullopt);
      EXPECT(str.FirstIndexOf(Str("zig").Pointer()) == std::nullopt);
      EXPECT(str.FirstIndexOf(StrOther("zig").Pointer()) == std::nullopt);
    }

    static constexpr void LastIndexOf()
    {
      TypedString str(Str("foo bar quack bar florg").Pointer());

      EXPECT(str.LastIndexOf(TChar('b')) == 14_usz);
      EXPECT(str.LastIndexOf(OtherChar('b')) == 14_usz);
      EXPECT(str.LastIndexOf(TypedString(Str("bar").Pointer())) == 14_usz);
      EXPECT(str.LastIndexOf(OtherTypedString(StrOther("bar").Pointer())) == 14_usz);
      EXPECT(str.LastIndexOf(Str("bar").Pointer()) == 14_usz);
      EXPECT(str.LastIndexOf(StrOther("bar").Pointer()) == 14_usz);

      EXPECT(str.LastIndexOf(TChar('z')) == std::nullopt);
      EXPECT(str.LastIndexOf(OtherChar('z')) == std::nullopt);
      EXPECT(str.LastIndexOf(TypedString(Str("zig").Pointer())) == std::nullopt);
      EXPECT(str.LastIndexOf(OtherTypedString(StrOther("zig").Pointer())) == std::nullopt);
      EXPECT(str.LastIndexOf(Str("zig").Pointer()) == std::nullopt);
      EXPECT(str.LastIndexOf(StrOther("zig").Pointer()) == std::nullopt);
    }

    static constexpr void Contains()
    {
      TypedString str(Str("foo bar quack bar florg").Pointer());

      EXPECT(str.Contains(TChar('b')));
      EXPECT(str.Contains(OtherChar('b')));
      EXPECT(str.Contains(TypedString(Str("bar").Pointer())));
      EXPECT(str.Contains(OtherTypedString(StrOther("bar").Pointer())));
      EXPECT(str.Contains(Str("bar").Pointer()));
      EXPECT(str.Contains(StrOther("bar").Pointer()));

      EXPECT(!str.Contains(TChar('z')));
      EXPECT(!str.Contains(OtherChar('z')));
      EXPECT(!str.Contains(TypedString(Str("zig").Pointer())));
      EXPECT(!str.Contains(OtherTypedString(StrOther("zig").Pointer())));
      EXPECT(!str.Contains(Str("zig").Pointer()));
      EXPECT(!str.Contains(StrOther("zig").Pointer()));
    }

    static constexpr void StartsWith()
    {
      TypedString str(Str("foo bar quack bar florg").Pointer());

      EXPECT(str.StartsWith(TChar('f')));
      EXPECT(str.StartsWith(OtherChar('f')));
      EXPECT(str.StartsWith(TypedString(Str("foo").Pointer())));
      EXPECT(str.StartsWith(OtherTypedString(StrOther("foo").Pointer())));
      EXPECT(str.StartsWith(Str("foo").Pointer()));
      EXPECT(str.StartsWith(StrOther("foo").Pointer()));

      EXPECT(!str.StartsWith(TChar('z')));
      EXPECT(!str.StartsWith(OtherChar('z')));
      EXPECT(!str.StartsWith(TypedString(Str("zig").Pointer())));
      EXPECT(!str.StartsWith(OtherTypedString(StrOther("zig").Pointer())));
      EXPECT(!str.StartsWith(Str("zig").Pointer()));
      EXPECT(!str.StartsWith(StrOther("zig").Pointer()));
    }

    static constexpr void EndsWith()
    {
      TypedString str(Str("foo bar quack bar florg").Pointer());

      EXPECT(str.EndsWith(TChar('g')));
      EXPECT(str.EndsWith(OtherChar('g')));
      EXPECT(str.EndsWith(TypedString(Str("florg").Pointer())));
      EXPECT(str.EndsWith(OtherTypedString(StrOther("florg").Pointer())));
      EXPECT(str.EndsWith(Str("florg").Pointer()));
      EXPECT(str.EndsWith(StrOther("florg").Pointer()));

      EXPECT(!str.EndsWith(TChar('z')));
      EXPECT(!str.EndsWith(OtherChar('z')));
      EXPECT(!str.EndsWith(TypedString(Str("zig").Pointer())));
      EXPECT(!str.EndsWith(OtherTypedString(StrOther("zig").Pointer())));
      EXPECT(!str.EndsWith(Str("zig").Pointer()));
      EXPECT(!str.EndsWith(StrOther("zig").Pointer()));
    }

    static constexpr void Append()
    {
      {
        TypedString str(Str("abc").Pointer());
        str.Append(TChar('1'));
        EXPECT(str == "abc1");
      }

      {
        TypedString str(Str("abc").Pointer());
        str.Append(OtherChar('1'));
        EXPECT(str == "abc1");
      }

      {
        TypedString str(Str("abc").Pointer());
        str.Append(TypedString(Str("123").Pointer()));
        EXPECT(str == "abc123");
      }

      {
        TypedString str(Str("abc").Pointer());
        str.Append(OtherTypedString(StrOther("123").Pointer()));
        EXPECT(str == "abc123");
      }

      {
        TypedString str(Str("abc").Pointer());
        str.Append(Str("123").Pointer());
        EXPECT(str == "abc123");
      }

      {
        TypedString str(Str("abc").Pointer());
        str.Append(StrOther("123").Pointer());
        EXPECT(str == "abc123");
      }

      {
        TypedString str(Str("abc").Pointer());
        str.Append(Span(Str("123").Pointer(), 3));
        EXPECT(str == "abc123");
      }

      {
        TypedString str(Str("abc").Pointer());
        str.Append(Span(StrOther("123").Pointer(), 3));
        EXPECT(str == "abc123");
      }

      {
        TypedString str(Str("abc").Pointer());
        str.Append(Str("123").Pointer());
        EXPECT(str == "abc123");
        auto charPointerA = str.CharPtr();

        TypedString strA = str;
        str.Append(Str("xyz").Pointer());
        EXPECT(str == "abc123xyz");
        EXPECT(strA == "abc123");
        EXPECT(strA.CharPtr() == charPointerA); // strA should point to the previous string data
        auto charPointerB = str.CharPtr();
        EXPECT(charPointerA != charPointerB); // str should have been reallocated because it is shared with strA

        str.Set("ABC");
        str.Append("123");
        EXPECT(str == "ABC123");
        EXPECT(strA == "abc123");
        EXPECT(str.CharPtr() == charPointerB); // Length was reduced so the memory should be reused
      }
    }

    static constexpr void Insert()
    {
      {
        TypedString str(Str("abc").Pointer());
        str.Insert(2, TChar('1'));
        EXPECT(str == "ab1c");
      }

      {
        TypedString str(Str("abc").Pointer());
        str.Insert(2, OtherChar('1'));
        EXPECT(str == "ab1c");
      }

      {
        TypedString str(Str("abc").Pointer());
        str.Insert(2, TypedString(Str("123").Pointer()));
        EXPECT(str == "ab123c");
      }

      {
        TypedString str(Str("abc").Pointer());
        str.Insert(2, OtherTypedString(StrOther("123").Pointer()));
        EXPECT(str == "ab123c");
      }

      {
        TypedString str(Str("abc").Pointer());
        str.Insert(2, Str("123").Pointer());
        EXPECT(str == "ab123c");
      }

      {
        TypedString str(Str("abc").Pointer());
        str.Insert(2, StrOther("123").Pointer());
        EXPECT(str == "ab123c");
      }

      {
        TypedString str(Str("abc").Pointer());
        str.Insert(2, Span(Str("123").Pointer(), 3));
        EXPECT(str == "ab123c");
      }

      {
        TypedString str(Str("abc").Pointer());
        str.Insert(2, Span(StrOther("123").Pointer(), 3));
        EXPECT(str == "ab123c");
      }

      {
        TypedString str(Str("abc").Pointer());
        str.Insert(2, Str("123").Pointer());
        EXPECT(str == "ab123c");
        auto charPointerA = str.CharPtr();

        TypedString strA = str;
        str.Insert(2, Str("xyz").Pointer());
        EXPECT(str == "abxyz123c");
        EXPECT(strA == "ab123c");
        EXPECT(strA.CharPtr() == charPointerA); // strA should point to the previous string data
        auto charPointerB = str.CharPtr();
        EXPECT(charPointerA != charPointerB); // str should have been reallocated because it is shared with strA

        str.Set("ABC");
        str.Insert(2, "123");
        EXPECT(str == "AB123C");
        EXPECT(strA == "ab123c");
        EXPECT(str.CharPtr() == charPointerB); // Length was reduced so the memory should be reused
      }

      {
        TypedString str(Str("abc").Pointer());
        str.Insert(0, Str("123").Pointer());
        EXPECT(str == "123abc");
      }

      {
        TypedString str(Str("abc").Pointer());
        str.Insert(3, Str("123").Pointer());
        EXPECT(str == "abc123");
      }
    }

    static constexpr void Remove()
    {
      {
        TypedString str(Str("abc123xyz").Pointer());
        str.Remove(3, 2);
        EXPECT(str == "abc3xyz");
        auto charPointerA = str.CharPtr();

        TypedString strA = str;
        str.Remove(4, 2);
        EXPECT(str == "abc3z");
        EXPECT(strA == "abc3xyz");
        EXPECT(strA.CharPtr() == charPointerA); // strA should point to the previous string data
        auto charPointerB = str.CharPtr();
        EXPECT(charPointerA != charPointerB); // str should have been reallocated because it is shared with strA

        str.Remove(0, 2);
        EXPECT(str == "c3z");
        EXPECT(strA == "abc3xyz");
        EXPECT(str.CharPtr() == charPointerB); // Length was reduced so the memory should be reused
      }

      {
        TypedString str(Str("abc123").Pointer());
        str.Remove(0, 2);
        EXPECT(str == "c123");
      }

      {
        TypedString str(Str("abc123").Pointer());
        str.Remove(4, 2);
        EXPECT(str == "abc1");
      }

      {
        TypedString str(Str("abc123").Pointer());
        str.Remove(0, 6);
        EXPECT(str.IsEmpty());
      }

      {
        TypedString str(Str("abc123").Pointer());
        str.Remove(3, ToEnd);
        EXPECT(str == "abc");
      }
    }

    static constexpr void Substring()
    {
      TypedString strA(Str("abc123").Pointer());
      TypedString strA1 = strA.Substring(0, 6);
      TypedString strA2 = strA.Substring(1, 4);
      TypedString strA3 = strA.Substring(3, ToEnd);
      EXPECT(strA1 == "abc123");
      EXPECT(strA1.CharPtr() == strA.CharPtr());
      EXPECT(strA2 == "bc12");
      EXPECT(strA3 == "123");

      TypedString strB(Unmanaged, Span(Str("abc123").Pointer(), 6));
      TypedString strB1 = strB.Substring(1, 4);
      EXPECT(strB1 == "bc12");
      EXPECT(strB1.CharPtr() == strB.CharPtr() + 1);
    }

    static constexpr void OperatorPlus()
    {
      TypedString str(Str("test").Pointer());

      str += TChar('1');
      EXPECT(str == "test1");

      str += OtherChar('2');
      EXPECT(str == "test12");

      str += TypedString(Str("abc").Pointer());
      EXPECT(str == "test12abc");

      str += OtherTypedString(StrOther("xyz").Pointer());
      EXPECT(str == "test12abcxyz");

      str += Str("ABC").Pointer();
      EXPECT(str == "test12abcxyzABC");

      str += StrOther("XYZ").Pointer();
      EXPECT(str == "test12abcxyzABCXYZ");

      TypedString strA(Str("aa").Pointer());
      TypedString strB(Str("bb").Pointer());
      OtherTypedString strC(StrOther("cc").Pointer());
      const TChar* strD = Str("dd").Pointer();
      const OtherChar* strE = StrOther("ee").Pointer();

      EXPECT(strA + strB == "aabb");
      EXPECT(strA + strC == "aacc");
      EXPECT(strA + strD == "aadd");
      EXPECT(strA + strE == "aaee");
      EXPECT(strD + strA == "ddaa");
      EXPECT(strE + strA == "eeaa");
    }
  };

  // $TODO change these to TEST_METHOD_CONSTEXPR when we upgrade to C++26
  TEST_CLASS(String)
  {
    TEST_METHOD(DefaultConstruct)
    {
      SharedStringTests<char>::DefaultConstruct();
      SharedStringTests<char32_t>::DefaultConstruct();
    }

    TEST_METHOD(CopyConstruct)
    {
      SharedStringTests<char>::CopyConstruct();
      SharedStringTests<char32_t>::CopyConstruct();
    }

    TEST_METHOD(MoveConstruct)
    {
      SharedStringTests<char>::MoveConstruct();
      SharedStringTests<char32_t>::MoveConstruct();
    }

    TEST_METHOD(ConstructFromNullTerminatedString)
    {
      SharedStringTests<char>::ConstructFromNullTerminatedString();
      SharedStringTests<char32_t>::ConstructFromNullTerminatedString();
    }

    TEST_METHOD(ConstructFromPointerAndLength)
    {
      SharedStringTests<char>::ConstructFromPointerAndLength();
      SharedStringTests<char32_t>::ConstructFromPointerAndLength();
    }

    TEST_METHOD(ConstructUnmanaged)
    {
      SharedStringTests<char>::ConstructUnmanaged();
      SharedStringTests<char32_t>::ConstructUnmanaged();
    }

    TEST_METHOD(CharacterReplacement)
    {
      AsciiString str(U"test\x03C0");
      EXPECT(str == "test?");
    }

    TEST_METHOD(CopyAssign)
    {
      SharedStringTests<char>::CopyAssign();
      SharedStringTests<char32_t>::CopyAssign();
    }

    TEST_METHOD(MoveAssign)
    {
      SharedStringTests<char>::MoveAssign();
      SharedStringTests<char32_t>::MoveAssign();
    }

    TEST_METHOD(CreateForWrite)
    {
      SharedStringTests<char>::CreateForWrite();
      SharedStringTests<char32_t>::CreateForWrite();
    }

    TEST_METHOD(AsSpan)
    {
      SharedStringTests<char>::AsSpan();
      SharedStringTests<char32_t>::AsSpan();
    }

    TEST_METHOD(CharPtr)
    {
      SharedStringTests<char>::CharPtr();
      SharedStringTests<char32_t>::CharPtr();
    }

    TEST_METHOD(Length)
    {
      SharedStringTests<char>::Length();
      SharedStringTests<char32_t>::Length();
    }

    TEST_METHOD(CharAt)
    {
      SharedStringTests<char>::CharAt();
      SharedStringTests<char32_t>::CharAt();
    }

    TEST_METHOD(Clear)
    {
      SharedStringTests<char>::Clear();
      SharedStringTests<char32_t>::Clear();
    }

    TEST_METHOD(Set)
    {
      SharedStringTests<char>::Set();
      SharedStringTests<char32_t>::Set();
    }

    TEST_METHOD(Equals)
    {
      SharedStringTests<char>::Equals();
      SharedStringTests<char32_t>::Equals();
    }

    TEST_METHOD(FirstIndexOf)
    {
      SharedStringTests<char>::FirstIndexOf();
      SharedStringTests<char32_t>::FirstIndexOf();
    }

    TEST_METHOD(LastIndexOf)
    {
      SharedStringTests<char>::LastIndexOf();
      SharedStringTests<char32_t>::LastIndexOf();
    }

    TEST_METHOD(Contains)
    {
      SharedStringTests<char>::Contains();
      SharedStringTests<char32_t>::Contains();
    }

    TEST_METHOD(StartsWith)
    {
      SharedStringTests<char>::StartsWith();
      SharedStringTests<char32_t>::StartsWith();
    }

    TEST_METHOD(EndsWith)
    {
      SharedStringTests<char>::EndsWith();
      SharedStringTests<char32_t>::EndsWith();
    }

    TEST_METHOD(Append)
    {
      SharedStringTests<char>::Append();
      SharedStringTests<char32_t>::Append();
    }

    TEST_METHOD(Insert)
    {
      SharedStringTests<char>::Insert();
      SharedStringTests<char32_t>::Insert();
    }

    TEST_METHOD(Remove)
    {
      SharedStringTests<char>::Remove();
      SharedStringTests<char32_t>::Remove();
    }

    TEST_METHOD(Substring)
    {
      SharedStringTests<char>::Substring();
      SharedStringTests<char32_t>::Substring();
    }

    TEST_METHOD(OperatorPlus)
    {
      SharedStringTests<char>::OperatorPlus();
      SharedStringTests<char32_t>::OperatorPlus();
    }
  };
}