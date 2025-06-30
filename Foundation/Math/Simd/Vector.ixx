export module Chord.Foundation:Math.Simd.Vector;

import :Containers.Span;
import :Core;
import :Math.Simd.SimdImplementations;
import :Math.Simd.SimdOperation;
import :Math.Simd.SimdUnderlyingType;
import :Utilities.Alignment;

namespace Chord
{
  template<basic_numeric TElement, usz ElementCount, SimdOperation Operation, typename... TArgs>
  static constexpr auto RunSimdOperation(TArgs&&... args)
  {
    if consteval
    {
      return SimdUnderlyingTypeFromEmulated(
        EmulatedSimdOperationImplementation<TElement, ElementCount, Operation>::Run(
          SimdUnderlyingTypeToEmulated(std::forward<TArgs>(args))...));
    }
    else
      { return SimdOperationImplementation<TElement, ElementCount, Operation>::Run(std::forward<TArgs>(args)...); }
  }

  export
  {
    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount>)
    class Vector
    {
    public:
      using Element = TElement;
      using FloatVector = typename SimdRelatedTypes<TElement>::FloatElement;
      using SignedVector = typename SimdRelatedTypes<TElement>::SignedElement;
      using UnsignedVector = typename SimdRelatedTypes<TElement>::UnsignedElement;

      constexpr Vector(Uninitialized_t)
        { }

      constexpr Vector(Zero_t)
        : m_data(Run<SimdOperation::SetZero>())
        { }

      constexpr Vector(const Vector& v)
        : m_data(v.m_data)
        { }

      constexpr Vector(TElement v)
        : m_data(Run<SimdOperation::SetSingle>(v))
        { }

      template<typename... TArgs>
        requires ((std::is_same_v<std::remove_cvref_t<TArgs>, TElement> && ...) && sizeof...(TArgs) == ElementCount)
      constexpr Vector(TArgs&&... v)
        : m_data(Run<SimdOperation::Set>(std::forward<TArgs>(v)...))
        { }

      constexpr Vector(const Vector<TElement, ElementCount / 2>& lower, const Vector<TElement, ElementCount / 2>& upper)
        requires Vector::template IsSupported<SimdOperation::Combine>
        : m_data(Run<SimdOperation::Combine>(lower, upper))
        { }

      explicit constexpr Vector(const SimdUnderlyingType<TElement, ElementCount>::Type& data)
        : m_data(data)
        { }

      static constexpr Vector LoadAligned(const TElement* source)
      {
        ASSERT(IsAlignedPointer(source, SimdUnderlyingTypeAlignment<TElement, ElementCount>));
        return Vector(Run<SimdOperation::LoadAligned>(source));
      }

      static constexpr Vector LoadAligned(Span<const TElement> source, basic_integral auto offset = 0)
        { return LoadAligned(source.GetBuffer(offset, ElementCount)); }

      static constexpr Vector LoadUnaligned(const TElement* source)
        { return Vector(Run<SimdOperation::LoadUnaligned>(source)); }

      static constexpr Vector LoadUnaligned(Span<const TElement> source, basic_integral auto offset = 0)
        { return LoadUnaligned(source.GetBuffer(offset, ElementCount)); }

      constexpr void StoreAligned(const TElement* destination)
      {
        ASSERT(IsAlignedPointer(destination, SimdUnderlyingTypeAlignment<TElement, ElementCount>));
        Run<SimdOperation::StoreAligned>(destination, m_data);
      }

      constexpr void StoreAligned(Span<const TElement> destination, basic_integral auto offset = 0) const
        { StoreAligned(destination.GetBuffer(offset, ElementCount)); }

      constexpr void StoreUnaligned(const TElement* destination)
        { Run<SimdOperation::StoreUnaligned>(destination, m_data); }

      constexpr void StoreUnaligned(Span<const TElement> destination, basic_integral auto offset = 0) const
        { return StoreUnaligned(destination.GetBuffer(offset, ElementCount)); }

      template<usz Index>
      constexpr TElement GetElement() const
      {
        ASSERT(IsInRangeArray(Index, ElementCount));
        return Run<SimdOperation::GetElement>(m_data, std::integral_constant<usz, Index>());
      }

      constexpr TElement FirstElement() const
        { return GetElement<0>(); }

      constexpr Vector operator+() const requires Vector::template IsSupported<SimdOperation::UnaryPlus>
        { return Vector(Run<SimdOperation::UnaryPlus>(m_data)); }

      constexpr Vector operator-() const requires Vector::template IsSupported<SimdOperation::Negate>
        { return Vector(Run<SimdOperation::Negate>(m_data)); }

      constexpr Vector operator+(const Vector& v) const requires Vector::template IsSupported<SimdOperation::Add>
        { return Vector(Run<SimdOperation::Add>(m_data, v.m_data)); }

      constexpr Vector operator-(const Vector& v) const requires Vector::template IsSupported<SimdOperation::Subtract>
        { return Vector(Run<SimdOperation::Subtract>(m_data, v.m_data)); }

      constexpr Vector operator*(const Vector& v) const requires Vector::template IsSupported<SimdOperation::Multiply>
        { return Vector(Run<SimdOperation::Multiply>(m_data, v.m_data)); }

      constexpr Vector operator/(const Vector& v) const requires Vector::template IsSupported<SimdOperation::Divide>
        { return Vector(Run<SimdOperation::Divide>(m_data, v.m_data)); }

      constexpr Vector operator~() const requires Vector::template IsSupported<SimdOperation::BitwiseNot>
        { return Vector(Run<SimdOperation::BitwiseNot>(m_data)); }

      constexpr Vector operator&(const Vector& v) const requires Vector::template IsSupported<SimdOperation::BitwiseAnd>
        { return Vector(Run<SimdOperation::BitwiseAnd>(m_data, v.m_data)); }

      constexpr Vector operator|(const Vector& v) const requires Vector::template IsSupported<SimdOperation::BitwiseOr>
        { return Vector(Run<SimdOperation::BitwiseOr>(m_data, v.m_data)); }

      constexpr Vector operator^(const Vector& v) const requires Vector::template IsSupported<SimdOperation::BitwiseXor>
        { return Vector(Run<SimdOperation::BitwiseXor>(m_data, v.m_data)); }

      constexpr Vector operator<<(s32 count) const requires Vector::template IsSupported<SimdOperation::ShiftLeftScalar>
        { return Vector(Run<SimdOperation::ShiftLeftScalar>(m_data, count)); }

      constexpr Vector operator<<(const SignedVector& count) const requires Vector::template IsSupported<SimdOperation::ShiftLeftVector>
        { return Vector(Run<SimdOperation::ShiftLeftVector>(m_data, count.m_data)); }

      constexpr Vector operator>>(s32 count) const requires Vector::template IsSupported<SimdOperation::ShiftRightScalar>
        { return Vector(Run<SimdOperation::ShiftRightScalar>(m_data, count)); }

      constexpr Vector operator>>(const SignedVector& count) const requires Vector::template IsSupported<SimdOperation::ShiftRightVector>
        { return Vector(Run<SimdOperation::ShiftRightVector>(m_data, count.m_data)); }

      constexpr SignedVector operator==(const Vector& v) const requires Vector::template IsSupported<SimdOperation::Equal>
        { return SignedVector(Run<SimdOperation::Equal>(m_data, v.m_data)); }

      constexpr SignedVector operator!=(const Vector& v) const requires Vector::template IsSupported<SimdOperation::NotEqual>
        { return SignedVector(Run<SimdOperation::NotEqual>(m_data, v.m_data)); }

      constexpr SignedVector operator>(const Vector& v) const requires Vector::template IsSupported<SimdOperation::Greater>
        { return SignedVector(Run<SimdOperation::Greater>(m_data, v.m_data)); }

      constexpr SignedVector operator<(const Vector& v) const requires Vector::template IsSupported<SimdOperation::Less>
        { return SignedVector(Run<SimdOperation::Less>(m_data, v.m_data)); }

      constexpr SignedVector operator>=(const Vector& v) const requires Vector::template IsSupported<SimdOperation::GreaterEqual>
        { return SignedVector(Run<SimdOperation::GreaterEqual>(m_data, v.m_data)); }

      constexpr SignedVector operator<=(const Vector& v) const requires Vector::template IsSupported<SimdOperation::LessEqual>
        { return SignedVector(Run<SimdOperation::LessEqual>(m_data, v.m_data)); }

      constexpr operator Vector<s32, ElementCount>() const requires Vector::template IsSupported<SimdOperation::ConvertS32>
        { return Vector<s32, ElementCount>(Run<SimdOperation::ConvertS32>(m_data)); }

      constexpr operator Vector<u32, ElementCount>() const requires Vector::template IsSupported<SimdOperation::ConvertU32>
        { return Vector<u32, ElementCount>(Run<SimdOperation::ConvertU32>(m_data)); }

      constexpr operator Vector<s64, ElementCount>() const requires Vector::template IsSupported<SimdOperation::ConvertS64>
        { return Vector<s64, ElementCount>(Run<SimdOperation::ConvertS64>(m_data)); }

      constexpr operator Vector<u64, ElementCount>() const requires Vector::template IsSupported<SimdOperation::ConvertU64>
        { return Vector<u64, ElementCount>(Run<SimdOperation::ConvertU64>(m_data)); }

      constexpr operator Vector<f32, ElementCount>() const requires Vector::template IsSupported<SimdOperation::ConvertF32>
        { return Vector<f32, ElementCount>(Run<SimdOperation::ConvertF32>(m_data)); }

      constexpr operator Vector<f64, ElementCount>() const requires Vector::template IsSupported<SimdOperation::ConvertF64>
        { return Vector<f64, ElementCount>(Run<SimdOperation::ConvertF64>(m_data)); }

      constexpr Vector LowerHalf() const requires Vector::template IsSupported<SimdOperation::LowerHalf>
        { return Vector<TElement, ElementCount / 2>(Run<SimdOperation::LowerHalf>(m_data)); }

      constexpr Vector UpperHalf() const requires Vector::template IsSupported<SimdOperation::UpperHalf>
        { return Vector<TElement, ElementCount / 2>(Run<SimdOperation::UpperHalf>(m_data)); }

      template<s32 A, s32 B>
      constexpr Vector<TElement, 2> Shuffle() const requires Vector::template IsSupported<SimdOperation::Shuffle2>
      {
        static constexpr u32 PackedIndices = PackIndices<ElementCount, A, B>();
        return Vector<TElement, 2>(Run<SimdOperation::Shuffle2>(m_data, std::integral_constant<u32, PackedIndices>()));
      }

      template<s32 A, s32 B, s32 C, s32 D>
      constexpr Vector<TElement, 4> Shuffle() const requires Vector::template IsSupported<SimdOperation::Shuffle4>
      {
        static constexpr u32 PackedIndices = PackIndices<ElementCount, A, B, C, D>();
        return Vector<TElement, 4>(Run<SimdOperation::Shuffle4>(m_data, std::integral_constant<u32, PackedIndices>()));
      }

      template<s32 A, s32 B, s32 C, s32 D, s32 E, s32 F, s32 G, s32 H>
      constexpr Vector<TElement, 8> Shuffle() const requires Vector::template IsSupported<SimdOperation::Shuffle8>
      {
        static constexpr u32 PackedIndices = PackIndices<ElementCount, A, B, C, D, E, F, G, H>();
        return Vector<TElement, 8>(Run<SimdOperation::Shuffle8>(m_data, std::integral_constant<u32, PackedIndices>()));
      }

      constexpr Vector& operator=(const Vector& v)
      {
        m_data = v.m_data;
        return *this;
      }

      constexpr Vector& operator+=(const Vector& v) const requires Vector::template IsSupported<SimdOperation::Add>
      {
        *this = *this + v;
        return *this;
      }

      constexpr Vector& operator-=(const Vector& v) const requires Vector::template IsSupported<SimdOperation::Subtract>
      {
        *this = *this - v;
        return *this;
      }

      constexpr Vector& operator*=(const Vector& v) const requires Vector::template IsSupported<SimdOperation::Multiply>
      {
        *this = *this * v;
        return *this;
      }

      constexpr Vector& operator/=(const Vector& v) const requires Vector::template IsSupported<SimdOperation::Divide>
      {
        *this = *this / v;
        return *this;
      }

      constexpr Vector& operator&=(const Vector& v) const requires Vector::template IsSupported<SimdOperation::BitwiseAnd>
      {
        *this = *this & v;
        return *this;
      }

      constexpr Vector& operator|=(const Vector& v) const requires Vector::template IsSupported<SimdOperation::BitwiseOr>
      {
        *this = *this | v;
        return *this;
      }

      constexpr Vector& operator^=(const Vector& v) const requires Vector::template IsSupported<SimdOperation::BitwiseXor>
      {
        *this = *this ^ v;
        return *this;
      }

      constexpr Vector& operator<<=(s32 count) const requires Vector::template IsSupported<SimdOperation::ShiftLeftScalar>
      {
        *this = *this << count;
        return *this;
      }

      constexpr Vector& operator<<=(const SignedVector& count) const requires Vector::template IsSupported<SimdOperation::ShiftLeftVector>
      {
        *this = *this << count;
        return *this;
      }

      constexpr Vector& operator>>=(s32 count) const requires Vector::template IsSupported<SimdOperation::ShiftRightScalar>
      {
        *this = *this >> count;
        return *this;
      }

      constexpr Vector& operator>>=(const SignedVector& count) const requires Vector::template IsSupported<SimdOperation::ShiftRightVector>
      {
        *this = *this >> count;
        return *this;
      }

      SimdUnderlyingType<TElement, ElementCount>::Type m_data;

    private:
      template<SimdOperation Operation>
      static constexpr bool IsSupported = IsSimdOperationSupported<TElement, ElementCount, Operation>;

      template<SimdOperation Operation, typename... TArgs>
      static constexpr auto Run(TArgs&&... args)
        { RunSimdOperation<TElement, ElementCount, Operation>(std::forward<TArgs>(args)...); }
    };

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::Abs>)
    Vector<TElement, ElementCount> Abs(const Vector<TElement, ElementCount>& v)
      { return RunSimdOperation<TElement, ElementCount, SimdOperation::Abs>(v); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::Floor>)
    Vector<TElement, ElementCount> Floor(const Vector<TElement, ElementCount>& v)
      { return RunSimdOperation<TElement, ElementCount, SimdOperation::Floor>(v); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::Ceil>)
    Vector<TElement, ElementCount> Ceil(const Vector<TElement, ElementCount>& v)
      { return RunSimdOperation<TElement, ElementCount, SimdOperation::Ceil>(v); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::Round>)
    Vector<TElement, ElementCount> Round(const Vector<TElement, ElementCount>& v)
      { return RunSimdOperation<TElement, ElementCount, SimdOperation::Round>(v); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::Min>)
    Vector<TElement, ElementCount> Min(const Vector<TElement, ElementCount>& a, const Vector<TElement, ElementCount>& b)
      { return RunSimdOperation<TElement, ElementCount, SimdOperation::Min>(a, b); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::Max>)
    Vector<TElement, ElementCount> Max(const Vector<TElement, ElementCount>& a, const Vector<TElement, ElementCount>& b)
      { return RunSimdOperation<TElement, ElementCount, SimdOperation::Max>(a, b); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::Reciprocal>)
    Vector<TElement, ElementCount> Reciprocal(const Vector<TElement, ElementCount>& v)
      { return RunSimdOperation<TElement, ElementCount, SimdOperation::Reciprocal>(v); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::ReciprocalSqrt>)
    Vector<TElement, ElementCount> ReciprocalSqrt(const Vector<TElement, ElementCount>& v)
      { return RunSimdOperation<TElement, ElementCount, SimdOperation::ReciprocalSqrt>(v); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::Sqrt>)
    Vector<TElement, ElementCount> Sqrt(const Vector<TElement, ElementCount>& v)
      { return RunSimdOperation<TElement, ElementCount, SimdOperation::Sqrt>(v); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::AndNot>)
    Vector<TElement, ElementCount> AndNot(const Vector<TElement, ElementCount>& a, const Vector<TElement, ElementCount>& b)
      { return RunSimdOperation<TElement, ElementCount, SimdOperation::AndNot>(a, b); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::SumElements>)
    Vector<TElement, ElementCount> SumElements(const Vector<TElement, ElementCount>& v)
      { return RunSimdOperation<TElement, ElementCount, SimdOperation::SumElements>(v); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::FMAdd>)
    Vector<TElement, ElementCount> FMAdd(
      const Vector<TElement, ElementCount>& a,
      const Vector<TElement, ElementCount>& b,
      const Vector<TElement, ElementCount>& c)
      { return RunSimdOperation<TElement, ElementCount, SimdOperation::FMAdd>(a, b, c); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::FMSub>)
    Vector<TElement, ElementCount> FMSub(
      const Vector<TElement, ElementCount>& a,
      const Vector<TElement, ElementCount>& b,
      const Vector<TElement, ElementCount>& c)
      { return RunSimdOperation<TElement, ElementCount, SimdOperation::FMSub>(a, b, c); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::FMAddSub>)
    Vector<TElement, ElementCount> FMAddSub(
      const Vector<TElement, ElementCount>& a,
      const Vector<TElement, ElementCount>& b,
      const Vector<TElement, ElementCount>& c)
      { return RunSimdOperation<TElement, ElementCount, SimdOperation::FMAddSub>(a, b, c); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::FMSubAdd>)
    Vector<TElement, ElementCount> FMSubAdd(
      const Vector<TElement, ElementCount>& a,
      const Vector<TElement, ElementCount>& b,
      const Vector<TElement, ElementCount>& c)
      { return RunSimdOperation<TElement, ElementCount, SimdOperation::FMSubAdd>(a, b, c); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::FNMAdd>)
    Vector<TElement, ElementCount> FNMAdd(
      const Vector<TElement, ElementCount>& a,
      const Vector<TElement, ElementCount>& b,
      const Vector<TElement, ElementCount>& c)
      { return RunSimdOperation<TElement, ElementCount, SimdOperation::FNMAdd>(a, b, c); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::FNMSub>)
    Vector<TElement, ElementCount> FNMSub(
      const Vector<TElement, ElementCount>& a,
      const Vector<TElement, ElementCount>& b,
      const Vector<TElement, ElementCount>& c)
      { return RunSimdOperation<TElement, ElementCount, SimdOperation::FNMSub>(a, b, c); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::FNMAddSub>)
    Vector<TElement, ElementCount> FNMAddSub(
      const Vector<TElement, ElementCount>& a,
      const Vector<TElement, ElementCount>& b,
      const Vector<TElement, ElementCount>& c)
      { return RunSimdOperation<TElement, ElementCount, SimdOperation::FNMAddSub>(a, b, c); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::FNMSubAdd>)
    Vector<TElement, ElementCount> FNMSubAdd(
      const Vector<TElement, ElementCount>& a,
      const Vector<TElement, ElementCount>& b,
      const Vector<TElement, ElementCount>& c)
      { return RunSimdOperation<TElement, ElementCount, SimdOperation::FNMSubAdd>(a, b, c); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::AddSub>)
    Vector<TElement, ElementCount> AddSub(
      const Vector<TElement, ElementCount>& a,
      const Vector<TElement, ElementCount>& b)
      { return RunSimdOperation<TElement, ElementCount, SimdOperation::AddSub>(a, b); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::SubAdd>)
    Vector<TElement, ElementCount> SubAdd(
      const Vector<TElement, ElementCount>& a,
      const Vector<TElement, ElementCount>& b)
      { return RunSimdOperation<TElement, ElementCount, SimdOperation::SubAdd>(a, b); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::Select>)
    Vector<TElement, ElementCount> Select(
      const typename Vector<TElement, ElementCount>::SignedVector& condition,
      const Vector<TElement, ElementCount>& trueValue,
      const Vector<TElement, ElementCount>& falseValue)
      { return RunSimdOperation<TElement, ElementCount, SimdOperation::Select>(condition, trueValue, falseValue); }
  }
}