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

    // Note: there is currently an MSVC bug (https://developercommunity.visualstudio.com/t/Unable-to-use-virtual-function-in-conste/10820093?sort=active) which
    // prevents constexpr from functioning properly but once this is fixed it should work.
    template<typename TReturn, typename... TArgs>
    class Callable<TReturn(TArgs...)>
    {
    public:
      static constexpr usz MaxByteCount = 32;

      constexpr Callable()
      {
        if consteval
          { m_implementation[0] = new UnassignedImplementation(); }
        else
          { new(Implementation()) UnassignedImplementation(); }
      }

      constexpr ~Callable() noexcept
      {
        if consteval
          { delete Implementation(); }
        else
          { Implementation()->~ImplementationBase(); }
      }

      template<callable_as<TReturn(TArgs...)> TFunc>
      constexpr Callable(TFunc&& func)
      {
        // If TFunc is a function reference (not a lambda or function pointer), turn it into a pointer when passing it into FunctionImplementation
        if constexpr (std::is_function_v<TFunc>)
        {
          if consteval
            { m_implementation[0] = new FunctionImplementation(&func); }
          else
            { new(Implementation()) FunctionImplementation(&func); }
        }
        else
        {
          if consteval
            { m_implementation[0] = new FunctionImplementation(std::forward<TFunc>(func)); }
          else
            { new(Implementation()) FunctionImplementation(std::forward<TFunc>(func)); }
        }
      }

      constexpr Callable(const Callable& other)
      {
        if consteval
          { m_implementation[0] = other.Implementation()->CopyConstruct(); }
        else
          { other.Implementation()->CopyConstructAt(Implementation()); }
      }

      // This overload exists to take priority over the TFunc&& overload
      constexpr Callable(Callable& other)
      {
        if consteval
          { m_implementation[0] = other.Implementation()->CopyConstruct(); }
        else
          { other.Implementation()->CopyConstructAt(Implementation()); }
      }

      constexpr Callable(Callable&& other) noexcept
      {
        if consteval
          { m_implementation[0] = other.Implementation()->MoveConstruct(); }
        else
          { other.Implementation()->MoveConstructAt(Implementation()); }
      }

      template<typename T>
      constexpr Callable(T* instance, TReturn (T::*func)(TArgs...))
      {
        auto wrapper = [instance, func](TArgs... argsInner) { return (instance->*func)(std::forward<TArgs>(argsInner)...); };
        if consteval
          { m_implementation[0] = new FunctionImplementation(std::move(wrapper)); }
        else
          { new(Implementation()) FunctionImplementation(std::move(wrapper)); }
      }

      template<typename T>
      constexpr Callable(const T* instance, TReturn (T::*func)(TArgs...) const)
      {
        auto wrapper = [instance, func](TArgs... argsInner) { return (instance->*func)(std::forward<TArgs>(argsInner)...); };
        if consteval
          { m_implementation[0] = FunctionImplementation::Create(std::move(wrapper)); }
        else
          { new(Implementation()) FunctionImplementation(std::move(wrapper)); }
      }

      constexpr Callable& operator=(const Callable& other)
      {
        if (this != &other)
        {
          if consteval
          {
            delete Implementation();
            m_implementation[0] = other.Implementation()->CopyConstruct();
          }
          else
          {
            Implementation()->~ImplementationBase();
            other.Implementation()->CopyConstructAt(Implementation());
          }
        }

        return *this;
      }

      constexpr Callable& operator=(Callable& other)
      {
        if (this != &other)
        {
          if consteval
          {
            delete Implementation();
            m_implementation[0] = other.Implementation()->CopyConstruct();
          }
          else
          {
            Implementation()->~ImplementationBase();
            other.Implementation()->CopyConstructAt(Implementation());
          }
        }

        return *this;
      }

      constexpr Callable& operator=(Callable&& other) noexcept
      {
        if (this != &other)
        {
          if consteval
          {
            delete Implementation();
            m_implementation[0] = other.Implementation()->MoveConstruct();

            // In non-consteval branches, the implementation of 'other' will replace itself with an UnassignedImplementation during the move. At consteval time,
            // we do this manually here.
            delete other.Implementation();
            other.m_implementation = new UnassignedImplementation();
          }
          else
          {
            Implementation()->~ImplementationBase();
            other.Implementation()->MoveConstructAt(Implementation());
          }
        }

        return *this;
      }

      constexpr bool IsValid() const
        { return Implementation()->IsValid(); }

      constexpr TReturn operator()(TArgs... args)
        { return Implementation()->Call(std::forward<TArgs>(args)...); }

      constexpr TReturn operator()(TArgs... args) const
        { return Implementation()->Call(std::forward<TArgs>(args)...); }

    private:
      class ImplementationBase
      {
      public:
        constexpr ImplementationBase() = default;
        ImplementationBase(const ImplementationBase&) = delete;
        ImplementationBase& operator=(const ImplementationBase&) = delete;
        virtual ~ImplementationBase() = default;

        virtual void CopyConstructAt(void* destination) const = 0;
        virtual void MoveConstructAt(void* destination) = 0;
        virtual ImplementationBase* CopyConstruct() const = 0;
        virtual ImplementationBase* MoveConstruct() = 0;
        virtual bool IsValid() const = 0;
        virtual TReturn Call(TArgs... args) = 0;
        virtual TReturn Call(TArgs... args) const = 0;
      };

      class UnassignedImplementation : public ImplementationBase
      {
      public:
        constexpr UnassignedImplementation() = default;
        UnassignedImplementation(const UnassignedImplementation&) = delete;
        UnassignedImplementation& operator=(const UnassignedImplementation&) = delete;

        virtual void CopyConstructAt(void* destination) const override
          { new(destination) UnassignedImplementation(); }

        virtual void MoveConstructAt(void* destination) override
          { new(destination) UnassignedImplementation(); }

        virtual constexpr ImplementationBase* CopyConstruct() const override
          { return new UnassignedImplementation(); }

        virtual constexpr ImplementationBase* MoveConstruct() override
          { return new UnassignedImplementation(); }

        virtual constexpr bool IsValid() const override
          { return false; }

        virtual constexpr TReturn Call([[maybe_unused]] TArgs... args) override
        {
          ASSERT(false, "Cannot invoke unassigned Callable");
          if constexpr (!std::same_as<TReturn, void>)
            { return *static_cast<TReturn*>(nullptr); }
        }

        virtual constexpr TReturn Call([[maybe_unused]] TArgs... args) const override
        {
          ASSERT(false, "Cannot invoke unassigned Callable");
          if constexpr (!std::same_as<TReturn, void>)
            { return *static_cast<TReturn*>(nullptr); }
        }
      };

      template<typename TFunction>
      class FunctionImplementation : public ImplementationBase
      {
      public:
        constexpr FunctionImplementation(TFunction& function) requires (std::is_copy_constructible_v<TFunction> && std::is_function_v<TFunction>)
          : m_function(function)
        {
          static_assert(sizeof(FunctionImplementation) <= MaxByteCount);
          static_assert(alignof(FunctionImplementation) <= alignof(void*));
        }

        constexpr FunctionImplementation(const TFunction& function) requires (std::is_copy_constructible_v<TFunction> && !std::is_function_v<TFunction>)
          : m_function(function)
        {
          static_assert(sizeof(FunctionImplementation) <= MaxByteCount);
          static_assert(alignof(FunctionImplementation) <= alignof(void*));
        }

        constexpr FunctionImplementation(TFunction&& function)
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

        virtual constexpr ImplementationBase* CopyConstruct() const override
        {
          if constexpr (std::is_copy_constructible_v<TFunction>)
            { return new FunctionImplementation(m_function); }
          else
          {
            ASSERT(false, "Callable cannot be copied, it is move-only");
            return nullptr;
          }
        }

        virtual constexpr ImplementationBase* MoveConstruct() override
          { return new FunctionImplementation(std::move(m_function)); }

        virtual constexpr bool IsValid() const override
          { return true; }

        virtual constexpr TReturn Call(TArgs... args) override
          { return m_function(args...); }

        virtual constexpr TReturn Call(TArgs... args) const override
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

      // This is used in non-consteval branches to avoid confusion. We want to treat m_implementation as an array of bytes which contains the contents of an
      // object derived from ImplementationBase, NOT as an array of pointers to ImplementationBase instances. In consteval contexts this can also be called and
      // just returns the first element which is a pointer to the implementation.
      constexpr ImplementationBase* Implementation()
      {
        if consteval
          { return m_implementation[0]; }
        else
          { return reinterpret_cast<ImplementationBase*>(m_implementation.Elements()); }
      }

      constexpr const ImplementationBase* Implementation() const
      {
        if consteval
          { return m_implementation[0]; }
        else
          { return reinterpret_cast<const ImplementationBase*>(m_implementation.Elements()); }
      }

      // This is an array of ImplementationBase pointers because in constexpr mode, we simply use the 0th element directly. At runtime, we treat this as a raw
      // byte array and call placement new to allocate into it.
      static_assert(MaxByteCount % sizeof(ImplementationBase*) == 0);
      FixedArray<ImplementationBase*, MaxByteCount / sizeof(ImplementationBase*)> m_implementation;
    };
  }
}