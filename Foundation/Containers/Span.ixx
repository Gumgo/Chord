export module Chord.Foundation:Containers.Span;

import std;

import :Core;
import :Utilities.Bounds;
import :Utilities.Copy;

namespace Chord
{
  export template<typename TElement>
  class Span;

  template<typename TDestination, typename TSource>
  inline constexpr bool IsCompatibleSpanElementType = false;

  template<typename TElement>
  inline constexpr bool IsCompatibleSpanElementType<TElement, TElement> = true;

  template<typename TDestination, typename TSource> requires (!std::same_as<TDestination, TSource>)
  inline constexpr bool IsCompatibleSpanElementType<TDestination*, TSource*> = IsCompatibleSpanElementType<TDestination, TSource>;

  template<typename TDestination, typename TSource> requires (!std::same_as<const TDestination, TSource>)
  inline constexpr bool IsCompatibleSpanElementType<const TDestination, TSource> = IsCompatibleSpanElementType<TDestination, TSource>;

  template<typename TDestination, typename TSource> requires (!std::same_as<TDestination, TSource>)
  inline constexpr bool IsCompatibleSpanElementType<Span<TDestination>, Span<TSource>> = IsCompatibleSpanElementType<TDestination, TSource>;

  template<typename TContainer>
  inline constexpr bool HasWritableElements = !std::is_const_v<std::remove_pointer_t<decltype(std::declval<TContainer>().Elements())>>;

  export
  {
    template<typename T> concept subspan_count = basic_integral<T> || std::same_as<std::remove_cvref_t<T>, ToEnd_t>;

    template<typename TSource, typename TDestination>
    concept compatible_span_element_to = IsCompatibleSpanElementType<TDestination, TSource>;

    template<typename TContainer>
    concept spannable_container = !std::same_as<Span<typename std::remove_reference_t<TContainer>::Element>, std::remove_cvref_t<TContainer>>
      && requires (std::remove_reference_t<TContainer>& mutableType, const std::remove_reference_t<TContainer>& constType)
      {
        { mutableType.Elements() } -> std::same_as<typename std::remove_reference_t<TContainer>::Element*>;
        { constType.Elements() } -> std::same_as<const typename std::remove_reference_t<TContainer>::Element*>;
        { mutableType.Count() } -> std::same_as<usz>;
        { constType.Count() } -> std::same_as<usz>;
      };

    template<typename TContainer, typename TElement>
    concept typed_spannable_container = spannable_container<TContainer>
      && requires (std::remove_reference_t<TContainer>& mutableType, const std::remove_reference_t<TContainer>& constType)
      {
        { mutableType.Elements() } -> std::same_as<TElement*>;
        { constType.Elements() } -> std::same_as<const TElement*>;
      };

    template<typename TContainer, typename TElement>
    concept compatible_spannable_container_to = spannable_container<TContainer>
      && compatible_span_element_to<typename std::remove_reference_t<TContainer>::Element, TElement>;

    template<typename TContainer, typename TElement>
    concept const_compatible_spannable_container_to = spannable_container<TContainer>
      && compatible_span_element_to<const typename std::remove_reference_t<TContainer>::Element, TElement>;

    template<typename TContainer>
    concept span_or_spannable_container =
      spannable_container<TContainer> || std::same_as<std::remove_cvref_t<TContainer>, Span<typename std::remove_cvref_t<TContainer>::Element>>;

    template<typename TContainer>
    concept has_writable_elements = HasWritableElements<TContainer>;

    // This class just defines span functionality. Derived classes should implement Elements() and Count().
    template<typename TElement>
    class SpanBase : public std::ranges::view_base
    {
    public:
      using Element = TElement;

      constexpr bool IsEmpty(this const auto& self)
        { return self.Count() == 0; }

      constexpr auto* GetBuffer(this auto&& self, basic_integral auto start, subspan_count auto count)
      {
        usz evaluatedStart = self.EvaluateSubspanStart(start);
        std::ignore = self.EvaluateSubspanCount(evaluatedStart, count);
        return self.Elements() + evaluatedStart;
      }

      constexpr bool IndexInRange(this const auto& self, basic_integral auto index)
      {
        auto count = self.Count();
        if constexpr (std::is_signed_v<decltype(index)>)
          { return index >= 0 && std::cmp_less(index, count); }
        else
          { return std::cmp_less(index, count); }
      }

      constexpr usz EvaluateIndex(this const auto& self, basic_integral auto index)
      {
        if constexpr (std::is_signed_v<decltype(index)>)
          { ASSERT(index >= 0); }
        ASSERT(Coerce<usz>(index) < self.Count());
        return Coerce<usz>(index);
      }

      constexpr usz EvaluateSubspanStart(this auto const& self, basic_integral auto index)
      {
        if constexpr (std::is_signed_v<decltype(index)>)
          { ASSERT(index >= 0); }
        ASSERT(Coerce<usz>(index) <= self.Count());
        return Coerce<usz>(index);
      }

      constexpr usz EvaluateSubspanCount(this auto const& self, usz start, subspan_count auto count)
      {
        if constexpr (std::same_as<decltype(count), ToEnd_t>)
          { return self.Count() - start; }
        else
        {
          usz result = Coerce<usz>(count);
          ASSERT(start + result <= self.Count());
          return result;
        }
      }

      constexpr auto* begin(this auto&& self)
        { return self.Elements(); }

      constexpr auto* end(this auto&& self)
        { return self.Elements() + self.Count(); }

      constexpr auto& operator[](this auto&& self, basic_integral auto index)
        { return self.Elements()[self.EvaluateIndex(index)]; }

      constexpr void CopyElementsFrom(this has_writable_elements auto&& self, Span<const TElement> other)
        requires (std::is_copy_assignable_v<TElement>);

      constexpr void MoveElementsFrom(this has_writable_elements auto&& self, Span<TElement> other)
        requires (std::is_move_assignable_v<TElement>);

      constexpr void CopyOverlappingElementsFrom(this has_writable_elements auto&& self, Span<const TElement> other)
        requires (std::is_copy_assignable_v<TElement>);

      constexpr void MoveOverlappingElementsFrom(this has_writable_elements auto&& self, Span<TElement> other)
        requires (std::is_move_assignable_v<TElement>);

      void ZeroElements(this has_writable_elements auto&& self)
        requires (std::is_trivially_copyable_v<std::remove_const_t<TElement>>
          && std::is_trivially_destructible_v<std::remove_const_t<TElement>>
          && !std::is_polymorphic_v<std::remove_const_t<TElement>>)
        { std::memset(self.Elements(), 0, self.Count() * sizeof(TElement)); }

      void Fill(this has_writable_elements auto&& self, const TElement& v)
        requires (std::is_copy_assignable_v<TElement>)
        { std::ranges::fill(self, v); }

      // !!! Add utilities: Contains, ElementsMatch, etc.
    };

    template<typename TElement>
    class Span : public SpanBase<TElement>
    {
    public:
      constexpr Span() = default;
      constexpr Span(const Span&) = default;
      constexpr Span& operator=(const Span&) = default;

      constexpr Span(TElement* elements, usz count)
        : m_elements(elements)
        , m_count(count)
        { }

      constexpr explicit Span(const std::initializer_list<std::remove_const_t<TElement>>& elements)
        requires (std::is_const_v<TElement>)
        : m_elements(elements.begin())
        , m_count(elements.size())
        { }

      template<usz Count>
      constexpr Span(TElement (&ary)[Count])
        : m_elements(ary)
        , m_count(Count)
        { }

      template<compatible_span_element_to<TElement> OtherElement>
      constexpr Span(Span<OtherElement> other)
        : m_elements((TElement*)other.Elements())
        , m_count(other.Count())
        { }

      constexpr Span(Span other, basic_integral auto start, subspan_count auto count)
        : m_elements(other.GetBuffer(start, count))
        , m_count(other.EvaluateSubspanCount(other.EvaluateSubspanStart(start), count))
        { }

      constexpr Span(const const_compatible_spannable_container_to<TElement> auto& other)
        : Span(other.Elements(), other.Count())
        { }

      constexpr Span(compatible_spannable_container_to<TElement> auto& other)
        : Span(other.Elements(), other.Count())
        { }

      constexpr Span(const const_compatible_spannable_container_to<TElement> auto& other, basic_integral auto start, subspan_count auto count)
        : Span(Span<TElement>{other}, start, count)
        { }

      constexpr Span(compatible_spannable_container_to<TElement> auto& other, basic_integral auto start, subspan_count auto count)
        : Span(Span<TElement>{other}, start, count)
        { }

      template<usz Count>
      constexpr Span& operator=(TElement (&ary)[Count])
      {
        m_elements = ary;
        m_count = Count;
        return *this;
      }

      template<compatible_span_element_to<TElement> OtherElement>
      constexpr Span& operator=(Span<OtherElement> other)
      {
        m_elements = (TElement*)other.Elements();
        m_count = other.Count();
        return *this;
      }

      constexpr TElement* Elements() const
        { return m_elements; }

      constexpr usz Count() const
        { return m_count; }

    protected:
      TElement* m_elements = nullptr;
      usz m_count = 0;
    };

    template<typename TElement>
    constexpr void SpanBase<TElement>::CopyElementsFrom(this has_writable_elements auto&& self, Span<const TElement> other)
      requires (std::is_copy_assignable_v<TElement>)
    {
      ASSERT(self.Count() == other.Count());
      Copy(self.Elements(), other.Elements(), other.Count());
    }

    template<typename TElement>
    constexpr void SpanBase<TElement>::MoveElementsFrom(this has_writable_elements auto&& self, Span<TElement> other)
      requires (std::is_move_assignable_v<TElement>)
    {
      ASSERT(self.Count() == other.Count());
      Move(self.Elements(), other.Elements(), other.Count());
    }

    template<typename TElement>
    constexpr void SpanBase<TElement>::CopyOverlappingElementsFrom(this has_writable_elements auto&& self, Span<const TElement> other)
      requires (std::is_copy_assignable_v<TElement>)
    {
      ASSERT(self.Count() == other.Count());
      CopyOverlapping(self.Elements(), other.Elements(), other.Count());
    }

    template<typename TElement>
    constexpr void SpanBase<TElement>::MoveOverlappingElementsFrom(this has_writable_elements auto&& self, Span<TElement> other)
      requires (std::is_move_assignable_v<TElement>)
    {
      ASSERT(self.Count() == other.Count());
      MoveOverlapping(self.Elements(), other.Elements(), other.Count());
    }
  }
}