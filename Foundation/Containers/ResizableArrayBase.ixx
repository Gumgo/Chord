export module Chord.Foundation:Containers.ResizableArrayBase;

import std;

import :Containers.Span;
import :Core;

namespace Chord
{
  template<typename TElement>
    requires (!std::is_const_v<TElement>)
  class ResizableArrayBase;

  template<typename TContainer>
  concept resizable_array_subclass = std::derived_from<TContainer, ResizableArrayBase<typename TContainer::Element>>
    && requires (TContainer& container, usz count)
    {
      { container.Capacity() } -> std::same_as<usz>;
      { container.EnsureCapacity(count) } -> std::same_as<void>;
    };

  template<typename TContainer>
  concept non_const_self = non_const<TContainer> && resizable_array_subclass<TContainer>;

  export
  {
    // This requires non-const elements because various operations like Append, Insert, etc. require move constructors which take non-const rvalue references
    template<typename TElement>
      requires (!std::is_const_v<TElement>)
    class ResizableArrayBase : public SpanBase<TElement>
    {
      using Super = SpanBase<TElement>;

    public:
      constexpr usz Count(this auto& self)
        { return self.m_count; }
      constexpr bool IsFull(this auto& self)
        { return self.m_count == self.Capacity(); }

      constexpr void Clear(this non_const_self auto& self);

      constexpr void SetCount(this non_const_self auto& self, usz count)
        requires (std::is_default_constructible_v<TElement>);
      constexpr void SetCount(this non_const_self auto& self, usz count, const TElement& fillValue)
        requires (std::copyable<TElement>);

      constexpr TElement& Append(this non_const_self auto& self, const TElement& value)
        requires (std::copyable<TElement>);
      constexpr TElement& Append(this non_const_self auto& self, TElement&& value)
        requires (std::movable<TElement>);

      template<typename... TArgs>
      constexpr TElement& AppendNew(this non_const_self auto& self, TArgs&&... args)
        requires (std::is_constructible_v<TElement, TArgs...>);
      constexpr Span<TElement> AppendFill(this non_const_self auto& self, usz count, const TElement& fillValue)
        requires (std::copyable<TElement>);
      constexpr Span<TElement> AppendMultiple(this non_const_self auto& self, const std::initializer_list<TElement>& list)
        requires (std::copyable<TElement>);
      template<typename TView>
        requires (std::ranges::viewable_range<TView> && std::same_as<std::remove_cv_t<std::ranges::range_value_t<TView>>, std::remove_cv_t<TElement>>)
      constexpr Span<TElement> AppendMultiple(this non_const_self auto& self, TView&& view)
        requires (std::copyable<TElement>);

      constexpr TElement& Insert(this non_const_self auto& self, basic_integral auto index, const TElement& value)
        requires (std::copyable<TElement>);
      constexpr TElement& Insert(this non_const_self auto& self, basic_integral auto index, TElement&& value)
        requires (std::movable<TElement>);
      template<typename... TArgs>
      constexpr TElement& InsertNew(this non_const_self auto& self, basic_integral auto index, TArgs&&... args)
        requires (std::is_constructible_v<TElement, TArgs...>);
      constexpr Span<TElement> InsertFill(this non_const_self auto& self, basic_integral auto index, usz count, const TElement& fillValue)
        requires (std::copyable<TElement>);
      constexpr Span<TElement> InsertMultiple(this non_const_self auto& self, basic_integral auto index, const std::initializer_list<TElement>& list)
        requires (std::copyable<TElement>);
      template<typename TView>
        requires (std::ranges::viewable_range<TView>&& std::same_as<std::remove_cv_t<std::ranges::range_value_t<TView>>, std::remove_cv_t<TElement>>)
      constexpr Span<TElement> InsertMultiple(this non_const_self auto& self, basic_integral auto index, TView&& view)
        requires (std::copyable<TElement> && std::movable<TElement>);

      constexpr void RemoveByIndex(this non_const_self auto& self, basic_integral auto index)
        requires (std::movable<TElement>);
      constexpr void RemoveByIndex(this non_const_self auto& self, basic_integral auto index, subspan_count auto count)
        requires (std::movable<TElement>);
      constexpr void RemoveByIndexUnordered(this non_const_self auto& self, basic_integral auto index)
        requires (std::movable<TElement>);
      constexpr void RemoveByIndexUnordered(this non_const_self auto& self, basic_integral auto index, subspan_count auto count = 1)
        requires (std::movable<TElement>);
      // !!! maybe add remove-by-value?

    protected:
      constexpr void ShiftElementsUp(this non_const_self auto& self, usz index, usz shiftCount)
      {
        usz elementCount = self.m_count - index;
        for (usz i = 0; i < elementCount; i++)
        {
          usz sourceIndex = self.m_count - i - 1;
          usz destinationIndex = sourceIndex + shiftCount;
          std::construct_at(&self.Elements()[destinationIndex], std::move(self.Elements()[sourceIndex]));
          std::destroy_at(&self.Elements()[sourceIndex]);
        }
      }

      usz m_count = 0;
    };

    template<typename TElement> requires (!std::is_const_v<TElement>)
    constexpr void ResizableArrayBase<TElement>::Clear(this non_const_self auto& self)
    {
      for (usz i = 0; i < self.m_count; i++)
        { std::destroy_at(&self.Elements()[self.m_count - i - 1]); }

      self.m_count = 0;
    }

    template<typename TElement> requires (!std::is_const_v<TElement>)
    constexpr void ResizableArrayBase<TElement>::SetCount(this non_const_self auto& self, usz count)
      requires (std::is_default_constructible_v<TElement>)
    {
      self.EnsureCapacity(count);
      if (self.m_count < count)
      {
        for (usz i = self.m_count; i < count; i++)
          { std::construct_at(&self.Elements()[i]); }
      }
      else
      {
        usz removeCount = self.m_count - count;
        for (usz i = 0; i < removeCount; i++)
          { std::destroy_at(&self.Elements()[self.m_count - i - 1]); }
      }

      self.m_count = count;
    }

    template<typename TElement> requires (!std::is_const_v<TElement>)
    constexpr void ResizableArrayBase<TElement>::SetCount(this non_const_self auto& self, usz count, const TElement& fillValue)
      requires (std::copyable<TElement>)
    {
      self.EnsureCapacity(count);
      if (self.m_count < count)
      {
        for (usz i = self.m_count; i < count; i++)
          { std::construct_at(&self.Elements()[i], fillValue); }
      }
      else
      {
        usz removeCount = self.m_count - count;
        for (usz i = 0; i < removeCount; i++)
          { std::destroy_at(&self.Elements()[self.m_count - i - 1]); }
      }

      self.m_count = count;
    }

    template<typename TElement> requires (!std::is_const_v<TElement>)
    constexpr TElement& ResizableArrayBase<TElement>::Append(this non_const_self auto& self, const TElement& value)
      requires (std::copyable<TElement>)
    {
      usz oldCount = self.m_count;
      usz newCount = oldCount + 1;
      self.EnsureCapacity(newCount);
      std::construct_at(&self.Elements()[oldCount], value);
      self.m_count = newCount;
      return self.Elements()[oldCount];
    }

    template<typename TElement> requires (!std::is_const_v<TElement>)
    constexpr TElement& ResizableArrayBase<TElement>::Append(this non_const_self auto& self, TElement&& value)
      requires (std::movable<TElement>)
    {
      usz oldCount = self.m_count;
      usz newCount = oldCount + 1;
      self.EnsureCapacity(newCount);
      std::construct_at(&self.Elements()[oldCount], std::move(value));
      self.m_count = newCount;
      return self.Elements()[oldCount];
    }

    template<typename TElement> requires (!std::is_const_v<TElement>)
    template<typename... TArgs>
    constexpr TElement& ResizableArrayBase<TElement>::AppendNew(this non_const_self auto& self, TArgs&&... args)
      requires (std::is_constructible_v<TElement, TArgs...>)
    {
      usz oldCount = self.m_count;
      usz newCount = oldCount + 1;
      self.EnsureCapacity(newCount);
      std::construct_at(&self.Elements()[oldCount], std::forward<TArgs>(args)...);
      self.m_count = newCount;
      return self.Elements()[oldCount];
    }

    template<typename TElement> requires (!std::is_const_v<TElement>)
    constexpr Span<TElement> ResizableArrayBase<TElement>::AppendFill(this non_const_self auto& self, usz count, const TElement& fillValue)
      requires (std::copyable<TElement>)
    {
      usz oldCount = self.m_count;
      usz newCount = oldCount + count;
      self.EnsureCapacity(newCount);
      for (usz i = oldCount; i < newCount; i++)
        { std::construct_at(&self.Elements()[i], fillValue); }
      self.m_count = newCount;
      return Span<TElement>(&self.Elements()[oldCount], count);
    }

    template<typename TElement> requires (!std::is_const_v<TElement>)
    constexpr Span<TElement> ResizableArrayBase<TElement>::AppendMultiple(this non_const_self auto& self, const std::initializer_list<TElement>& list)
      requires (std::copyable<TElement>)
      { return self.AppendMultiple(Span<TElement>(list)); }

    template<typename TElement> requires (!std::is_const_v<TElement>)
    template<typename TView>
      requires (std::ranges::viewable_range<TView> && std::same_as<std::remove_cv_t<std::ranges::range_value_t<TView>>, std::remove_cv_t<TElement>>)
    constexpr Span<TElement> ResizableArrayBase<TElement>::AppendMultiple(this non_const_self auto& self, TView&& view)
      requires (std::copyable<TElement>)
    {
      if constexpr (requires (TView v) { std::ranges::size(v); } || requires (TView v) { v.Count(); })
      {
        // This view has a pre-defined size
        usz count;
        if constexpr (requires (TView v) { std::ranges::size(v); })
          { count = std::ranges::size(view); }
        else
          { count = view.Count(); }

        self.EnsureCapacity(self.m_count + count);
        usz oldCount = self.m_count;
        usz newCount = oldCount;
        for (auto&& item : view)
        {
          std::construct_at(&self.Elements()[newCount], std::forward<decltype(item)>(item));
          newCount++;
        }

        self.m_count = newCount;
        return Span<TElement>(&self.Elements()[oldCount], count);
      }
      else
      {
        // This view has no pre-defined size, we have to append elements one at a time
        for (auto &&item : view)
          { self.Append(std::forward<decltype(item)>(item)); }
      }
    }

    template<typename TElement> requires (!std::is_const_v<TElement>)
    constexpr TElement& ResizableArrayBase<TElement>::Insert(this non_const_self auto& self, basic_integral auto index, const TElement& value)
      requires (std::copyable<TElement>)
    {
      usz evaluatedIndex = self.EvaluateSubspanStart(index);
      usz oldCount = self.m_count;
      usz newCount = oldCount + 1;
      self.EnsureCapacity(newCount);
      self.ShiftElementsUp(evaluatedIndex, 1);
      std::construct_at(&self.Elements()[evaluatedIndex], value);
      self.m_count = newCount;
      return self.Elements()[evaluatedIndex];
    }

    template<typename TElement> requires (!std::is_const_v<TElement>)
    constexpr TElement& ResizableArrayBase<TElement>::Insert(this non_const_self auto& self, basic_integral auto index, TElement&& value)
      requires (std::movable<TElement>)
    {
      usz evaluatedIndex = self.EvaluateSubspanStart(index);
      usz oldCount = self.m_count;
      usz newCount = oldCount + 1;
      self.EnsureCapacity(newCount);
      self.ShiftElementsUp(evaluatedIndex, 1);
      std::construct_at(&self.Elements()[evaluatedIndex], std::move(value));
      self.m_count = newCount;
      return self.Elements()[evaluatedIndex];
    }

    template<typename TElement> requires (!std::is_const_v<TElement>)
    template<typename... TArgs>
    constexpr TElement& ResizableArrayBase<TElement>::InsertNew(this non_const_self auto& self, basic_integral auto index, TArgs&&... args)
      requires (std::is_constructible_v<TElement, TArgs...>)
    {
      usz evaluatedIndex = self.EvaluateSubspanStart(index);
      usz oldCount = self.m_count;
      usz newCount = oldCount + 1;
      self.EnsureCapacity(newCount);
      self.ShiftElementsUp(evaluatedIndex, 1);
      std::construct_at(&self.Elements()[evaluatedIndex], std::forward<TArgs>(args)...);
      self.m_count = newCount;
      return self.Elements()[evaluatedIndex];
    }

    template<typename TElement> requires (!std::is_const_v<TElement>)
    constexpr Span<TElement> ResizableArrayBase<TElement>::InsertFill(
      this non_const_self auto& self,
      basic_integral auto index,
      usz count,
      const TElement& fillValue)
      requires (std::copyable<TElement>)
    {
      usz evaluatedIndex = self.EvaluateSubspanStart(index);
      usz oldCount = self.m_count;
      usz newCount = oldCount + count;
      self.EnsureCapacity(newCount);
      self.ShiftElementsUp(evaluatedIndex, count);
      usz endIndex = evaluatedIndex + count;
      for (usz i = evaluatedIndex; i < endIndex; i++)
        { std::construct_at(&self.Elements()[i], fillValue); }
      self.m_count = newCount;
      return Span<TElement>(&self.Elements()[evaluatedIndex], count);
    }

    template<typename TElement> requires (!std::is_const_v<TElement>)
    constexpr Span<TElement> ResizableArrayBase<TElement>::InsertMultiple(
      this non_const_self auto& self,
      basic_integral auto index,
      const std::initializer_list<TElement>& list)
      requires (std::copyable<TElement>)
      { return self.InsertMultiple(index, Span<TElement>(list)); }

    template<typename TElement> requires (!std::is_const_v<TElement>)
    template<typename TView>
      requires (std::ranges::viewable_range<TView>&& std::same_as<std::remove_cv_t<std::ranges::range_value_t<TView>>, std::remove_cv_t<TElement>>)
    constexpr Span<TElement> ResizableArrayBase<TElement>::InsertMultiple(this non_const_self auto& self, basic_integral auto index, TView&& view)
      requires (std::copyable<TElement> && std::movable<TElement>)
    {
      usz evaluatedIndex = self.EvaluateSubspanStart(index);
      if constexpr (requires (TView v) { std::ranges::size(v); } || requires (TView v) { v.Count(); })
      {
        // This view has a pre-defined size
        usz count;
        if constexpr (requires (TView v) { std::ranges::size(v); })
          { count = std::ranges::size(view); }
        else
          { count = view.Count(); }

        usz oldCount = self.m_count;
        usz newCount = oldCount + count;
        self.EnsureCapacity(newCount);
        self.ShiftElementsUp(evaluatedIndex, count);

        usz constructIndex = evaluatedIndex;
        for (auto&& item : view)
        {
          std::construct_at(&self.Elements()[constructIndex], std::forward<decltype(item)>(item));
          constructIndex++;
        }

        self.m_count = newCount;
        return Span<TElement>(&self.Elements()[evaluatedIndex], count);
      }
      else
      {
        // This view has no pre-defined size, we have to insert elements one at a time. Calling Insert() would cause this to be O(n^2) so instead, we'll append
        // to the end and then rotate the new elements into place afterwards
        usz oldCount = self.m_count;
        for (auto &&item : view)
          { self.Append(std::forward<decltype(item)>(item)); }

        // Rotate the new elements into place if necessary
        if (oldCount > self.m_count && evaluatedIndex != oldCount)
          { std::ranges::rotate(&self.Elements()[evaluatedIndex], &self.Elements()[oldCount], &self.Elements()[self.m_count]); }
      }
    }

    template<typename TElement> requires (!std::is_const_v<TElement>)
    constexpr void ResizableArrayBase<TElement>::RemoveByIndex(this non_const_self auto& self, basic_integral auto index)
      requires (std::movable<TElement>)
      { self.RemoveByIndex(index, 1); }

    template<typename TElement> requires (!std::is_const_v<TElement>)
    constexpr void ResizableArrayBase<TElement>::RemoveByIndex(this non_const_self auto& self, basic_integral auto index, subspan_count auto count)
      requires (std::movable<TElement>)
    {
      usz evaluatedIndex = self.EvaluateSubspanStart(index);
      usz evaluatedCount = self.EvaluateSubspanCount(evaluatedIndex, count);
      usz endIndex = self.m_count - evaluatedCount;
      for (usz i = evaluatedIndex; i < endIndex; i++)
      {

        std::destroy_at(&self.Elements()[i]);
        std::construct_at(&self.Elements()[i], std::move(self.Elements()[i + evaluatedCount]));
      }

      for (usz i = self.m_count - evaluatedCount; i < self.m_count; i++)
        { std::destroy_at(&self.Elements()[i]); }

      self.m_count -= evaluatedCount;
    }

    template<typename TElement> requires (!std::is_const_v<TElement>)
    constexpr void ResizableArrayBase<TElement>::RemoveByIndexUnordered(this non_const_self auto& self, basic_integral auto index)
      requires (std::movable<TElement>)
      { self.RemoveByIndexUnordered(index, 1); }

    template<typename TElement> requires (!std::is_const_v<TElement>)
    constexpr void ResizableArrayBase<TElement>::RemoveByIndexUnordered(this non_const_self auto& self, basic_integral auto index, subspan_count auto count)
      requires (std::movable<TElement>)
    {
      usz evaluatedIndex = self.EvaluateSubspanStart(index);
      usz evaluatedCount = self.EvaluateSubspanCount(evaluatedIndex, count);
      usz endIndex = evaluatedIndex + evaluatedCount;
      for (usz i = evaluatedIndex; i < endIndex; i++)
      {
        self.m_count--;
        if (i != self.m_count)
        {
          std::destroy_at(&self.Elements()[i]);
          std::construct_at(&self.Elements()[i], std::move(self.Elements()[self.m_count]));
        }

        std::destroy_at(&self.Elements()[self.m_count]);
      }
    }
  }
}