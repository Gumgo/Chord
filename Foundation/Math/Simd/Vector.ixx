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
      using Result = decltype(
        EmulatedSimdOperationImplementation<TElement, ElementCount, Operation>::Run(SimdUnderlyingTypeToEmulated(std::forward<TArgs>(args))...));

      if constexpr (std::same_as<Result, void>)
        { EmulatedSimdOperationImplementation<TElement, ElementCount, Operation>::Run(SimdUnderlyingTypeToEmulated(std::forward<TArgs>(args))...); }
      else
      {
        return SimdUnderlyingTypeFromEmulated(
          EmulatedSimdOperationImplementation<TElement, ElementCount, Operation>::Run(
            SimdUnderlyingTypeToEmulated(std::forward<TArgs>(args))...));
      }
    }
    else
      { return SimdOperationImplementation<TElement, ElementCount, Operation>::Run(std::forward<TArgs>(args)...); }
  }

  export
  {
    template<basic_numeric TElement, usz ElementCountParam>
      requires (IsSimdTypeSupported<TElement, ElementCountParam>)
    class Vector
    {
    public:
      using Element = TElement;
      using FloatVector = Vector<SimdRelatedFloatElement<TElement>, ElementCountParam>;
      using SignedVector = Vector<SimdRelatedSignedElement<TElement>, ElementCountParam>;
      using UnsignedVector = Vector<SimdRelatedUnsignedElement<TElement>, ElementCountParam>;

      static constexpr usz ElementCount = ElementCountParam;

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

      constexpr Vector(const std::same_as<Vector<TElement, ElementCount / 2>>auto& lower, const std::same_as<Vector<TElement, ElementCount / 2>> auto& upper)
        requires Vector::template IsSupported<SimdOperation::Combine>
        : m_data(Run<SimdOperation::Combine>(lower.m_data, upper.m_data))
        { }

      explicit constexpr Vector(const SimdUnderlyingType<TElement, ElementCount>& data)
        : m_data(data)
        { }

      static constexpr Vector NarrowAndCombine(
        const std::same_as<Vector<Widen<TElement>, ElementCount / 2>> auto& lower,
        const std::same_as<Vector<Widen<TElement>, ElementCount / 2>> auto& upper)
        requires Vector::template IsSupported<SimdOperation::NarrowAndCombine>
        { return Vector(Run<SimdOperation::NarrowAndCombine>(lower.m_data, upper.m_data)); }

      static constexpr Vector LoadAligned(const TElement* source)
      {
        // In consteval contexts, we can't convert between int and pointer, so skip this check
        if !consteval
          { ASSERT(IsAlignedPointer(source, SimdUnderlyingTypeAlignment<TElement, ElementCount>)); }
        return Vector(Run<SimdOperation::LoadAligned>(source));
      }

      static constexpr Vector LoadAligned(Span<const TElement> source)
        { return LoadAligned(source.GetBuffer(0, ElementCount)); }

      static constexpr Vector LoadAligned(Span<const TElement> source, basic_integral auto offset)
        { return LoadAligned(source.GetBuffer(offset, ElementCount)); }

      static constexpr Vector LoadUnaligned(const TElement* source)
        { return Vector(Run<SimdOperation::LoadUnaligned>(source)); }

      static constexpr Vector LoadUnaligned(Span<const TElement> source)
        { return LoadUnaligned(source.GetBuffer(0, ElementCount)); }

      static constexpr Vector LoadUnaligned(Span<const TElement> source, basic_integral auto offset)
        { return LoadUnaligned(source.GetBuffer(offset, ElementCount)); }

      constexpr void StoreAligned(TElement* destination) const
      {
        // In consteval contexts, we can't convert between int and pointer, so skip this check
        if !consteval
          { ASSERT(IsAlignedPointer(destination, SimdUnderlyingTypeAlignment<TElement, ElementCount>)); }
        Run<SimdOperation::StoreAligned>(destination, m_data);
      }

      constexpr void StoreAligned(Span<TElement> destination) const
        { StoreAligned(destination.GetBuffer(0, ElementCount)); }

      constexpr void StoreAligned(Span<TElement> destination, basic_integral auto offset) const
        { StoreAligned(destination.GetBuffer(offset, ElementCount)); }

      constexpr void StoreUnaligned(TElement* destination) const
        { Run<SimdOperation::StoreUnaligned>(destination, m_data); }

      constexpr void StoreUnaligned(Span<TElement> destination) const
        { return StoreUnaligned(destination.GetBuffer(0, ElementCount)); }

      constexpr void StoreUnaligned(Span<TElement> destination, basic_integral auto offset) const
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

      template<typename TTo>
      constexpr operator TTo() const
      {
        if constexpr (IsVectorOfType<TTo, s32, ElementCount>())
        {
          static_assert(IsSupported<SimdOperation::ConvertS32>);
          return Vector<s32, ElementCount>(Run<SimdOperation::ConvertS32>(m_data));
        }
        else if constexpr (IsVectorOfType<TTo, s64, ElementCount>())
        {
          static_assert(IsSupported<SimdOperation::ConvertS64>);
          return Vector<s64, ElementCount>(Run<SimdOperation::ConvertS64>(m_data));
        }
        else if constexpr (IsVectorOfType<TTo, u32, ElementCount>())
        {
          static_assert(IsSupported<SimdOperation::ConvertU32>);
          return Vector<u32, ElementCount>(Run<SimdOperation::ConvertU32>(m_data));
        }
        else if constexpr (IsVectorOfType<TTo, u64, ElementCount>())
        {
          static_assert(IsSupported<SimdOperation::ConvertU64>);
          return Vector<u64, ElementCount>(Run<SimdOperation::ConvertU64>(m_data));
        }
        else if constexpr (IsVectorOfType<TTo, f32, ElementCount>())
        {
          static_assert(IsSupported<SimdOperation::ConvertF32>);
          return Vector<f32, ElementCount>(Run<SimdOperation::ConvertF32>(m_data));
        }
        else if constexpr (IsVectorOfType<TTo, f64, ElementCount>())
        {
          static_assert(IsSupported<SimdOperation::ConvertF64>);
          return Vector<f64, ElementCount>(Run<SimdOperation::ConvertF64>(m_data));
        }
        else
          { static_assert(AlwaysFalse<TTo>, "Unsupported conversion"); }
      }

      constexpr auto LowerHalf() const requires Vector::template IsSupported<SimdOperation::LowerHalf>
        { return Vector<TElement, ElementCount / 2>(Run<SimdOperation::LowerHalf>(m_data)); }

      constexpr auto UpperHalf() const requires Vector::template IsSupported<SimdOperation::UpperHalf>
        { return Vector<TElement, ElementCount / 2>(Run<SimdOperation::UpperHalf>(m_data)); }

      constexpr auto WidenAndSplit() const requires Vector::template IsSupported<SimdOperation::WidenAndSplit>
      {
        auto result = Run<SimdOperation::WidenAndSplit>(m_data);
        return std::make_tuple(Vector<Widen<TElement>, ElementCount / 2>(std::get<0>(result)), Vector<Widen<TElement>, ElementCount / 2>(std::get<1>(result)));
      }

      template<s32 A, s32 B>
      constexpr auto Shuffle() const requires Vector::template IsSupported<SimdOperation::Shuffle2>
      {
        static constexpr u32 PackedIndices = PackIndices<ElementCount, A, B>();
        return Vector<TElement, 2>(Run<SimdOperation::Shuffle2>(m_data, std::integral_constant<u32, PackedIndices>()));
      }

      template<s32 A, s32 B, s32 C, s32 D>
      constexpr auto Shuffle() const requires Vector::template IsSupported<SimdOperation::Shuffle4>
      {
        static constexpr u32 PackedIndices = PackIndices<ElementCount, A, B, C, D>();
        return Vector<TElement, 4>(Run<SimdOperation::Shuffle4>(m_data, std::integral_constant<u32, PackedIndices>()));
      }

      template<s32 A, s32 B, s32 C, s32 D, s32 E, s32 F, s32 G, s32 H>
      constexpr auto Shuffle() const requires Vector::template IsSupported<SimdOperation::Shuffle8>
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

      // These operators are defined identically to ~, &, and |. Note that they do not short-circuit. This is done so that the Vector class is compatible with
      // scalar syntax in a templated context.
      constexpr Vector operator!() const requires Vector::template IsSupported<SimdOperation::BitwiseNot>
        { return Vector(Run<SimdOperation::BitwiseNot>(m_data)); }

      constexpr Vector operator&&(const Vector& v) const requires Vector::template IsSupported<SimdOperation::BitwiseAnd>
        { return Vector(Run<SimdOperation::BitwiseAnd>(m_data, v.m_data)); }

      constexpr Vector operator||(const Vector& v) const requires Vector::template IsSupported<SimdOperation::BitwiseOr>
        { return Vector(Run<SimdOperation::BitwiseOr>(m_data, v.m_data)); }

      SimdUnderlyingType<TElement, ElementCount> m_data;

    private:
      template<SimdOperation Operation>
      static constexpr bool IsSupported = IsSimdOperationSupported<TElement, ElementCount, Operation>;

      template<SimdOperation Operation, typename... TArgs>
      static constexpr auto Run(TArgs&&... args)
        { return RunSimdOperation<TElement, ElementCount, Operation>(std::forward<TArgs>(args)...); }

      template<typename T, basic_numeric TElement, usz ElementCount>
      static constexpr bool IsVectorOfType()
      {
        if constexpr (!IsSimdTypeSupported<TElement, ElementCount>)
          { return false; }
        else
          { return std::same_as<T, Vector<TElement, ElementCount>>; }
      }
    };

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::Abs>)
    constexpr Vector<TElement, ElementCount> Abs(const Vector<TElement, ElementCount>& v)
      { return Vector<TElement, ElementCount>(RunSimdOperation<TElement, ElementCount, SimdOperation::Abs>(v)); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::Floor>)
    constexpr Vector<TElement, ElementCount> Floor(const Vector<TElement, ElementCount>& v)
      { return Vector<TElement, ElementCount>(RunSimdOperation<TElement, ElementCount, SimdOperation::Floor>(v)); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::Ceil>)
    constexpr Vector<TElement, ElementCount> Ceil(const Vector<TElement, ElementCount>& v)
      { return Vector<TElement, ElementCount>(RunSimdOperation<TElement, ElementCount, SimdOperation::Ceil>(v)); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::Round>)
    constexpr Vector<TElement, ElementCount> Round(const Vector<TElement, ElementCount>& v)
      { return Vector<TElement, ElementCount>(RunSimdOperation<TElement, ElementCount, SimdOperation::Round>(v)); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::Trunc>)
    constexpr Vector<TElement, ElementCount> Trunc(const Vector<TElement, ElementCount>& v)
      { return Vector<TElement, ElementCount>(RunSimdOperation<TElement, ElementCount, SimdOperation::Trunc>(v)); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::Min>)
    constexpr Vector<TElement, ElementCount> Min(const Vector<TElement, ElementCount>& a, const Vector<TElement, ElementCount>& b)
      { return Vector<TElement, ElementCount>(RunSimdOperation<TElement, ElementCount, SimdOperation::Min>(a, b)); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::Max>)
    constexpr Vector<TElement, ElementCount> Max(const Vector<TElement, ElementCount>& a, const Vector<TElement, ElementCount>& b)
      { return Vector<TElement, ElementCount>(RunSimdOperation<TElement, ElementCount, SimdOperation::Max>(a, b)); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::Reciprocal>)
    constexpr Vector<TElement, ElementCount> Reciprocal(const Vector<TElement, ElementCount>& v)
      { return Vector<TElement, ElementCount>(RunSimdOperation<TElement, ElementCount, SimdOperation::Reciprocal>(v)); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::ReciprocalSqrt>)
    constexpr Vector<TElement, ElementCount> ReciprocalSqrt(const Vector<TElement, ElementCount>& v)
      { return Vector<TElement, ElementCount>(RunSimdOperation<TElement, ElementCount, SimdOperation::ReciprocalSqrt>(v)); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::Sqrt>)
    constexpr Vector<TElement, ElementCount> Sqrt(const Vector<TElement, ElementCount>& v)
      { return Vector<TElement, ElementCount>(RunSimdOperation<TElement, ElementCount, SimdOperation::Sqrt>(v)); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::AndNot>)
    constexpr Vector<TElement, ElementCount> AndNot(const Vector<TElement, ElementCount>& a, const Vector<TElement, ElementCount>& b)
      { return Vector<TElement, ElementCount>(RunSimdOperation<TElement, ElementCount, SimdOperation::AndNot>(a, b)); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::CountLeadingZeros>)
    constexpr typename Vector<TElement, ElementCount>::SignedVector CountLeadingZeros(const Vector<TElement, ElementCount>& v)
      { return typename Vector<TElement, ElementCount>::SignedVector(RunSimdOperation<TElement, ElementCount, SimdOperation::CountLeadingZeros>(v)); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::SumElements>)
    constexpr Vector<TElement, ElementCount> SumElements(const Vector<TElement, ElementCount>& v)
      { return Vector<TElement, ElementCount>(RunSimdOperation<TElement, ElementCount, SimdOperation::SumElements>(v)); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::FMAdd>)
    constexpr Vector<TElement, ElementCount> FMAdd(
      const Vector<TElement, ElementCount>& a,
      const Vector<TElement, ElementCount>& b,
      const Vector<TElement, ElementCount>& c)
      { return Vector<TElement, ElementCount>(RunSimdOperation<TElement, ElementCount, SimdOperation::FMAdd>(a, b, c)); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::FMSub>)
    constexpr Vector<TElement, ElementCount> FMSub(
      const Vector<TElement, ElementCount>& a,
      const Vector<TElement, ElementCount>& b,
      const Vector<TElement, ElementCount>& c)
      { return Vector<TElement, ElementCount>(RunSimdOperation<TElement, ElementCount, SimdOperation::FMSub>(a, b, c)); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::FMAddSub>)
    constexpr Vector<TElement, ElementCount> FMAddSub(
      const Vector<TElement, ElementCount>& a,
      const Vector<TElement, ElementCount>& b,
      const Vector<TElement, ElementCount>& c)
      { return Vector<TElement, ElementCount>(RunSimdOperation<TElement, ElementCount, SimdOperation::FMAddSub>(a, b, c)); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::FMSubAdd>)
    constexpr Vector<TElement, ElementCount> FMSubAdd(
      const Vector<TElement, ElementCount>& a,
      const Vector<TElement, ElementCount>& b,
      const Vector<TElement, ElementCount>& c)
      { return Vector<TElement, ElementCount>(RunSimdOperation<TElement, ElementCount, SimdOperation::FMSubAdd>(a, b, c)); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::FNMAdd>)
    constexpr Vector<TElement, ElementCount> FNMAdd(
      const Vector<TElement, ElementCount>& a,
      const Vector<TElement, ElementCount>& b,
      const Vector<TElement, ElementCount>& c)
      { return Vector<TElement, ElementCount>(RunSimdOperation<TElement, ElementCount, SimdOperation::FNMAdd>(a, b, c)); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::FNMSub>)
    constexpr Vector<TElement, ElementCount> FNMSub(
      const Vector<TElement, ElementCount>& a,
      const Vector<TElement, ElementCount>& b,
      const Vector<TElement, ElementCount>& c)
      { return Vector<TElement, ElementCount>(RunSimdOperation<TElement, ElementCount, SimdOperation::FNMSub>(a, b, c)); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::FNMAddSub>)
    constexpr Vector<TElement, ElementCount> FNMAddSub(
      const Vector<TElement, ElementCount>& a,
      const Vector<TElement, ElementCount>& b,
      const Vector<TElement, ElementCount>& c)
      { return Vector<TElement, ElementCount>(RunSimdOperation<TElement, ElementCount, SimdOperation::FNMAddSub>(a, b, c)); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::FNMSubAdd>)
    constexpr Vector<TElement, ElementCount> FNMSubAdd(
      const Vector<TElement, ElementCount>& a,
      const Vector<TElement, ElementCount>& b,
      const Vector<TElement, ElementCount>& c)
      { return Vector<TElement, ElementCount>(RunSimdOperation<TElement, ElementCount, SimdOperation::FNMSubAdd>(a, b, c)); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::AddSub>)
    constexpr Vector<TElement, ElementCount> AddSub(
      const Vector<TElement, ElementCount>& a,
      const Vector<TElement, ElementCount>& b)
      { return Vector<TElement, ElementCount>(RunSimdOperation<TElement, ElementCount, SimdOperation::AddSub>(a, b)); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::SubAdd>)
    constexpr Vector<TElement, ElementCount> SubAdd(
      const Vector<TElement, ElementCount>& a,
      const Vector<TElement, ElementCount>& b)
      { return Vector<TElement, ElementCount>(RunSimdOperation<TElement, ElementCount, SimdOperation::SubAdd>(a, b)); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::Select>)
    constexpr Vector<TElement, ElementCount> Select(
      const typename Vector<TElement, ElementCount>::SignedVector& condition,
      const Vector<TElement, ElementCount>& trueValue,
      const Vector<TElement, ElementCount>& falseValue)
      { return Vector<TElement, ElementCount>(RunSimdOperation<TElement, ElementCount, SimdOperation::Select>(condition, trueValue, falseValue)); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::GetMask>)
    constexpr s32 GetMask(const Vector<TElement, ElementCount>& v)
      { return RunSimdOperation<TElement, ElementCount, SimdOperation::GetMask>(v); }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::GetMask>)
    constexpr bool TestMaskAllZeros(const Vector<TElement, ElementCount>& v)
      { return GetMask(v) == 0; }

    template<basic_numeric TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount> && IsSimdOperationSupported<TElement, ElementCount, SimdOperation::GetMask>)
    constexpr bool TestMaskAllOnes(const Vector<TElement, ElementCount>& v)
      { return GetMask(v) == ((1 << ElementCount) - 1); }

    // Additional functions which don't rely on IsSimdOperationSupported directly:

    template<std::floating_point TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount>)
    constexpr auto IsInf(const Vector<TElement, ElementCount>& v) -> typename Vector<TElement, ElementCount>::SignedVector
    {
      using UnsignedVector = typename Vector<TElement, ElementCount>::UnsignedVector;
      static constexpr UnsignedVector SignBitMask = UnsignedVector(~FloatTraits<TElement>::SignBitMask);
      static constexpr UnsignedVector ExponentMask = UnsignedVector(FloatTraits<TElement>::ExponentMask);
      return (std::bit_cast<UnsignedVector>(v) & SignBitMask) == ExponentMask;
    }

    template<std::floating_point TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount>)
    constexpr auto IsNaN(const Vector<TElement, ElementCount>& v) -> typename Vector<TElement, ElementCount>::SignedVector
      { return v != v; }

    template<std::floating_point TElement, usz ElementCount>
      requires (IsSimdTypeSupported<TElement, ElementCount>)
    constexpr Vector<TElement, ElementCount> CopySign(const Vector<TElement, ElementCount>& v, const Vector<TElement, ElementCount>& sign)
    {
      using fBBxC = Vector<TElement, ElementCount>;
      using uBBxC = typename Vector<TElement, ElementCount>::UnsignedVector;
      return std::bit_cast<fBBxC>(
        (std::bit_cast<uBBxC>(v) & ~FloatTraits<TElement>::SignBitMask) | (std::bit_cast<uBBxC>(sign) & FloatTraits<TElement>::SignBitMask));
      return v != v;
    }
  }
}