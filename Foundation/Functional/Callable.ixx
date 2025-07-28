export module Chord.Foundation:Functional.Callable;

import std;

import :Containers;
import :Core;

namespace Chord
{
  export
  {
    template<typename TFunc>
    class Callable;

    template<typename TReturn, typename... TArgs>
    class Callable<TReturn(TArgs...)>
    {
    public:
      static constexpr usz MaxByteCount = 32;

      Callable()
        { new(Implementation()) UnassignedImplementation(); }

      ~Callable() noexcept
        { Implementation()->~ImplementationBase(); }

      template<callable_as<TReturn(TArgs...)> TFunc>
        requires (!std::is_function_v<TFunc>)
      Callable(TFunc&& func)
        { new(Implementation()) FunctionImplementation(std::forward<TFunc>(func)); }

      template<callable_as<TReturn(TArgs...)> TFunc>
        requires (std::is_function_v<TFunc>)
      Callable(TFunc&& func)
        { new(Implementation()) FunctionImplementation(&func); }

      Callable(const Callable& other)
        { other.Implementation()->CopyConstructAt(Implementation()); }

      // This overload exists to take priority over the TFunc&& overload
      Callable(Callable& other)
        { other.Implementation()->CopyConstructAt(Implementation()); }

      Callable(Callable&& other) noexcept
        { other.Implementation()->MoveConstructAt(Implementation()); }

      template<typename T>
      Callable(T* instance, TReturn (T::*func)(TArgs...))
      {
        auto wrapper = [instance, func](TArgs... argsInner) { return (instance->*func)(std::forward<TArgs>(argsInner)...); };
        new(Implementation()) FunctionImplementation(std::move(wrapper));
      }

      template<typename T>
      Callable(const T* instance, TReturn (T::*func)(TArgs...) const)
      {
        auto wrapper = [instance, func](TArgs... argsInner) { return (instance->*func)(std::forward<TArgs>(argsInner)...); };
        new(Implementation()) FunctionImplementation(std::move(wrapper));
      }

      Callable& operator=(const Callable& other)
      {
        if (this != &other)
        {
          Implementation()->~ImplementationBase();
          other.Implementation()->CopyConstructAt(Implementation());
        }

        return *this;
      }

      Callable& operator=(Callable& other)
      {
        if (this != &other)
        {
          Implementation()->~ImplementationBase();
          other.Implementation()->CopyConstructAt(Implementation());
        }

        return *this;
      }

      Callable& operator=(Callable&& other) noexcept
      {
        if (this != &other)
        {
          Implementation()->~ImplementationBase();
          other.Implementation()->MoveConstructAt(Implementation());
        }

        return *this;
      }

      bool IsValid() const
        { return Implementation()->IsValid(); }

      TReturn operator()(TArgs... args)
        { return Implementation()->Call(std::forward<TArgs>(args)...); }

      TReturn operator()(TArgs... args) const
        { return Implementation()->Call(std::forward<TArgs>(args)...); }

    private:
      class ImplementationBase
      {
      public:
        ImplementationBase() = default;
        ImplementationBase(const ImplementationBase&) = delete;
        ImplementationBase& operator=(const ImplementationBase&) = delete;
        virtual ~ImplementationBase() = default;

        virtual void CopyConstructAt(void* destination) const = 0;
        virtual void MoveConstructAt(void* destination) = 0;
        virtual bool IsValid() const = 0;
        virtual TReturn Call(TArgs... args) = 0;
        virtual TReturn Call(TArgs... args) const = 0;
      };

      class UnassignedImplementation : public ImplementationBase
      {
      public:
        UnassignedImplementation() = default;
        UnassignedImplementation(const UnassignedImplementation&) = delete;
        UnassignedImplementation& operator=(const UnassignedImplementation&) = delete;

        virtual void CopyConstructAt(void* destination) const override
          { new(destination) UnassignedImplementation(); }

        virtual void MoveConstructAt(void* destination) override
          { new(destination) UnassignedImplementation(); }

        virtual bool IsValid() const override
          { return false; }

        virtual TReturn Call(TArgs... args) override
        {
          ASSERT(false, "Cannot invoke unassigned Callable");
          if constexpr (std::same_as<TReturn, void>)
            { return *static_cast<TReturn*>(nullptr); }
        }

        virtual TReturn Call(TArgs... args) const override
        {
          ASSERT(false, "Cannot invoke unassigned Callable");
          if constexpr (std::same_as<TReturn, void>)
            { return *static_cast<TReturn*>(nullptr); }
        }
      };

      template<typename TFunction>
      class FunctionImplementation : public ImplementationBase
      {
      public:
        FunctionImplementation(TFunction& function) requires (std::is_copy_constructible_v<TFunction> && std::is_function_v<TFunction>)
          : m_function(function)
        {
          static_assert(sizeof(FunctionImplementation) <= MaxByteCount);
          static_assert(alignof(FunctionImplementation) <= alignof(void*));
        }

        FunctionImplementation(const TFunction& function) requires (std::is_copy_constructible_v<TFunction> && !std::is_function_v<TFunction>)
          : m_function(function)
        {
          static_assert(sizeof(FunctionImplementation) <= MaxByteCount);
          static_assert(alignof(FunctionImplementation) <= alignof(void*));
        }

        FunctionImplementation(TFunction&& function)
          : m_function(std::move(function))
        {
          static_assert(sizeof(FunctionImplementation) <= MaxByteCount);
          static_assert(alignof(FunctionImplementation) <= alignof(void*));
        }

        FunctionImplementation(const FunctionImplementation&) = delete;
        FunctionImplementation& operator=(const FunctionImplementation&) = delete;

        virtual void CopyConstructAt(void* destination) const override
        {
          if constexpr (std::is_copy_constructible_v<TFunction>)
            { new(destination) FunctionImplementation(m_function); }
          else
            { ASSERT(false, "Callable cannot be copied, it is move-only"); }
        }

        virtual void MoveConstructAt(void* destination) override
        {
          new(destination) FunctionImplementation(std::move(m_function));
          this->~FunctionImplementation();
          new(this) UnassignedImplementation();
        }

        virtual bool IsValid() const override
          { return true; }

        virtual TReturn Call(TArgs... args) override
          { return m_function(args...); }

        virtual TReturn Call(TArgs... args) const override
        {
          if constexpr (callable_as<const TFunction, TReturn(TArgs...)>)
            { return m_function(args...); }
          else
          {
            ((std::ignore = args), ...);
            ASSERT(false, "Callable cannot be invoked in a const context");
            return *static_cast<TReturn*>(nullptr);
          }
        }

      private:
        TFunction m_function;
      };

      ImplementationBase* Implementation()
        { return reinterpret_cast<ImplementationBase*>(m_implementation.Elements()); }

      const ImplementationBase* Implementation() const
        { return reinterpret_cast<const ImplementationBase*>(m_implementation.Elements()); }

      alignas(void*) FixedArray<u8, MaxByteCount> m_implementation;
    };
  }
}