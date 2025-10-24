module;

#include "../NativeLibraryApi/ChordNativeLibraryApi.h"

export module Chord.NativeLibraryToolkit:DeclareOptimizationRule;

import std;

import Chord.Foundation;

namespace Chord
{
  template<typename T>
  concept constant_value = std::same_as<T, f32>
    || std::same_as<T, f64>
    || std::same_as<T, s32>
    || std::same_as<T, bool>
    || std::convertible_to<T, const char32_t*>;

  enum class ComponentType
  {
    NativeModuleCall,
    In,
    Out,
    Array,
    InRef,
  };

  template<typename T>
  consteval usz CountComponents()
  {
    if constexpr (constant_value<T>)
      { return 1; }
    else if constexpr (requires { { T::ComponentCount } -> std::convertible_to<usz>; })
      { return T::ComponentCount; }
    else
    {
      static_assert(AlwaysFalse<T>, "Unsupported optimization rule component type");
      return 0;
    }
  }

  struct OptimizationRuleComponentBase
    { };

  template<typename TValue>
  struct NamedOptimizationRuleComponent
  {
    using Value = TValue;
    static constexpr usz ComponentCount = CountComponents<TValue>();
    const char* m_name = nullptr;
    TValue m_value;
  };

  template<typename... TArguments>
  struct NativeModuleCallOptimizationRuleComponent : public OptimizationRuleComponentBase
  {
    static constexpr ComponentType Type = ComponentType::NativeModuleCall;
    static constexpr usz ComponentCount =
      []()
      {
        usz count = 1;
        Unroll<0, sizeof...(TArguments)>(
          [&](auto i)
            { count += CountComponents<std::tuple_element_t<decltype(i)::value, std::tuple<TArguments...>>>(); });
        return count;
      }();

    const NativeModule* m_nativeModule = nullptr;
    s32 m_upsampleFactor = 1;
    std::tuple<TArguments...> m_arguments;
  };

  template<bool MustBeConstantArg>
  struct InOptimizationRuleComponent : public OptimizationRuleComponentBase
  {
    static constexpr ComponentType Type = ComponentType::In;
    static constexpr usz ComponentCount = 1;
    static constexpr bool MustBeConstant = MustBeConstantArg;
  };

  template<bool IsReturnArg>
  struct OutOptimizationRuleComponent : public OptimizationRuleComponentBase
  {
    static constexpr ComponentType Type = ComponentType::Out;
    static constexpr usz ComponentCount = 1;
    static constexpr bool IsReturn = IsReturnArg;
  };

  template<typename... TElements>
  struct ArrayOptimizationRuleComponent : public OptimizationRuleComponentBase
  {
    static constexpr ComponentType Type = ComponentType::Array;
    static constexpr usz ComponentCount =
      []()
      {
        usz count = 1;
        Unroll<0, sizeof...(TElements)>(
          [&](auto i)
            { count += CountComponents<std::tuple_element_t<decltype(i)::value, std::tuple<TElements...>>>(); });
        return count;
      }();

    std::tuple<TElements...> m_elements;
  };

  struct InRefOptimizationRuleComponent : public OptimizationRuleComponentBase
  {
    static constexpr ComponentType Type = ComponentType::InRef;
    static constexpr usz ComponentCount = 1;
  };

  template<typename T>
  struct UnnamedComponentType
  {
    using Type = T;
  };

  template<typename T>
    requires (requires (T c) { { c.m_name }; })
  struct UnnamedComponentType<T>
  {
    using Type = T::Value;
  };

  template<typename T>
  using UnnamedComponent = typename UnnamedComponentType<T>::Type;

  template<typename T, typename TFunc>
  void IterateComponents(const T& component, TFunc&& func)
  {
    if constexpr (requires (T c) { { c.m_name }; })
      { IterateComponentsNamed(component.m_value, component.m_name, std::forward<TFunc>(func)); }
    else
      { IterateComponentsNamed(component, nullptr, std::forward<TFunc>(func)); }
  }

  template<typename T, typename TFunc>
  void IterateComponentsNamed(const T& component, const char* name, TFunc&& func)
  {
    func(component, name);
    if constexpr (std::derived_from<T, OptimizationRuleComponentBase>)
    {
      if constexpr (T::Type == ComponentType::NativeModuleCall)
      {
        Unroll<0, std::tuple_size_v<decltype(component.m_arguments)>>(
          [&](auto i)
            { IterateComponents(std::get<decltype(i)::value>(component.m_arguments), std::forward<TFunc>(func)); });
      }
      else if constexpr (T::Type == ComponentType::Array)
      {
        Unroll<0, std::tuple_size_v<decltype(component.m_elements)>>(
          [&](auto i)
            { IterateComponents(std::get<decltype(i)::value>(component.m_elements), std::forward<TFunc>(func)); });
      }
    }
  }

  template<typename T>
  consteval usz FindReturnOutParameterIndex()
  {
    using Component = std::remove_cvref_t<T>;
    std::optional<usz> index;
    Unroll<0, std::tuple_size_v<decltype(Component::m_arguments)>>(
      [&](auto i)
      {
        static constexpr usz ArgumentIndex = decltype(i)::value;
        using Argument = UnnamedComponent<std::tuple_element_t<ArgumentIndex, decltype(Component::m_arguments)>>;
        if constexpr (std::derived_from<Argument, OptimizationRuleComponentBase>)
        {
          if constexpr (Argument::Type == ComponentType::Out)
          {
            if constexpr (Argument::IsReturn)
            {
              ASSERT(!index.has_value(), "Multiple return out arguments specified");
              index = ArgumentIndex;
            }
          }
        }
      });

    ASSERT(index.has_value(), "Return out argument not specified");
    return index.value();
  }


  export
  {
    struct UpsampleFactor
    {
      s32 m_value = 0;
    };

    constexpr UpsampleFactor operator""_x(unsigned long long value)
    {
      ASSERT(value >= 1, "Upsample factor must be at least 1");
      return UpsampleFactor(Coerce<s32>(value));
    }

    template<usz ComponentCount, usz OutputPatternCount>
    class BuiltOptimizationRule
    {
    public:
      BuiltOptimizationRule()
      {
        m_outputPatterns[OutputPatternCount] = nullptr;
        m_optimizationRule.m_inputPattern = m_components.Elements();
        m_optimizationRule.m_outputPatterns = m_outputPatterns.Elements();
      }

      BuiltOptimizationRule(const BuiltOptimizationRule& other)
        { *this = other; }

      BuiltOptimizationRule& operator=(const BuiltOptimizationRule& other)
      {
        if (this != &other)
        {
          m_components = other.m_components;
          m_outputPatterns = other.m_outputPatterns;

          m_optimizationRule.m_name = other.m_optimizationRule.m_name;
          m_optimizationRule.m_inputPattern = m_components.Elements();
          m_optimizationRule.m_outputPatterns = m_outputPatterns.Elements();

          for (usz i = 0; i < OutputPatternCount; i++)
          {
            auto componentIndex = other.m_outputPatterns[i] - other.m_components.Elements();
            m_outputPatterns[i] = &m_components[componentIndex];
          }
        }

        return *this;
      }

      const OptimizationRule* GetOptimizationRule() const
        { return &m_optimizationRule; }

    private:
      template<typename TInputPattern, typename TReturnPattern, typename... TOutputPatterns>
      friend auto DeclareOptimizationRule(
        const Guid& nativeLibraryId,
        const char32_t* name,
        const TInputPattern& inputPattern,
        const TReturnPattern& returnPattern,
        const TOutputPatterns&... outputPatterns);

      FixedArray<OptimizationRuleComponent, ComponentCount> m_components;
      FixedArray<OptimizationRuleComponent*, OutputPatternCount + 1> m_outputPatterns;
      OptimizationRule m_optimizationRule;
    };

    namespace OptimizationRuleSyntax
    {
      template<typename TValue>
      auto Named(const char* name, const TValue& value)
        { return NamedOptimizationRuleComponent { .m_name = name, .m_value = value }; }

      template<typename... TArgs>
      auto Call(const NativeModule* nativeModule, const TArgs&... args)
      {
        return NativeModuleCallOptimizationRuleComponent
        {
          .m_nativeModule = nativeModule,
          .m_upsampleFactor = 1,
          .m_arguments = std::make_tuple(args...),
        };
      }

      template<typename... TArgs>
      auto Call(const char* name, const NativeModule* nativeModule, const TArgs&... args)
      {
        return NamedOptimizationRuleComponent
        {
          .m_name = name,
          .m_upsampleFactor = 1,
          .m_value = NativeModuleCallOptimizationRuleComponent { .m_nativeModule = nativeModule, .m_arguments = std::make_tuple(args...) },
        };
      }

      template<typename... TArgs>
      auto Call(const NativeModule* nativeModule, UpsampleFactor upsampleFactor, const TArgs&... args)
      {
        return NativeModuleCallOptimizationRuleComponent
        {
          .m_nativeModule = nativeModule,
          .m_upsampleFactor = upsampleFactor.m_value,
          .m_arguments = std::make_tuple(args...),
        };
      }

      template<typename... TArgs>
      auto Call(const char* name, const NativeModule* nativeModule, UpsampleFactor upsampleFactor, const TArgs&... args)
      {
        return NamedOptimizationRuleComponent
        {
          .m_name = name,
          .m_upsampleFactor = upsampleFactor.m_value,
          .m_value = NativeModuleCallOptimizationRuleComponent {.m_nativeModule = nativeModule, .m_arguments = std::make_tuple(args...) },
        };
      }

      auto In()
        { return InOptimizationRuleComponent<false>{}; }

      auto In(const char* name)
        { return NamedOptimizationRuleComponent { .m_name = name, .m_value = InOptimizationRuleComponent<false>{} }; }

      auto Constant()
        { return InOptimizationRuleComponent<true>{}; }

      auto Constant(const char* name)
        { return NamedOptimizationRuleComponent { .m_name = name, .m_value = InOptimizationRuleComponent<true>{} }; }

      auto Out()
        { return OutOptimizationRuleComponent<false>{}; }

      auto Out(const char* name)
        { return NamedOptimizationRuleComponent { .m_name = name, .m_value = OutOptimizationRuleComponent<false>{} }; }

      auto OutReturn()
        { return OutOptimizationRuleComponent<true>{}; }

      template<typename... TElements>
      auto Array(const TElements&... elements)
        { return ArrayOptimizationRuleComponent { .m_elements = std::make_tuple(elements...) }; }

      template<typename... TElements>
      auto Array(const char* name, const TElements&... elements)
        { return NamedOptimizationRuleComponent { .m_name = name, .m_value = ArrayOptimizationRuleComponent { .m_elements = std::make_tuple(elements...) } }; }

      auto InRef(const char* name)
        { return NamedOptimizationRuleComponent { .m_name = name, .m_value = InRefOptimizationRuleComponent{} }; }
    }

    // This function is used to declare an optimization rule using a convenient shorthand syntax. It returns a BuiltOptimizationRule instance which holds both
    // the optimization rule itself as well as storage for the optimization rule components.
    //
    // Optimization rules are specified using patterns, where the input pattern is replaced with one or more output patterns. Patterns are built using nested
    // function calls intended to appear similar to how the function would appear in a Chord script (though more verbose). These functions are found within the
    // OptimizationRuleSyntax namespace. Here is an example of an input pattern:
    //
    //   Call(&add, In("a"), Call(&negate, In("b"), OutReturn()), OutReturn())
    //
    // This pattern would match against Chord expressions of the form "a + -b". We could associate this input pattern with the following output pattern:
    //
    //   Call(&subtract, InRef("a"), InRef("b"), OutReturn())
    //
    // This would cause Chord expressions of the form "a + -b" to be replaced with "a - b".
    //
    // Because native modules support multiple output parameters, an optimization rule must have an output pattern for every non-consumed output within the
    // input pattern. An output is "consumed" when it is passed down to a nested call. For example, in "Foo(Bar())", the return value of Bar() is consumed by
    // Foo(). Internally, native modules don't actually "return" values, but rather one output parameter may optionally be marked as "return". Therefore, when
    // declaring optimization rules, the consumed output parameter must be explicitly specified with OutReturn(). So the previous example would actually appear
    // as "Call(&foo, Call(&bar, OutReturn()), OutReturn())", which means: the first (output) argument of Bar() is consumed as the first (input) argument to
    // Foo() and the second (output) argument of Foo() is consumed as the pattern's final resulting value.
    //
    // When additional output arguments are present, they are marked with Out("name") in the input pattern. For each named output argument in the input pattern,
    // an additional named output pattern must be specified using the syntax: Named("name", <pattern>). The following is an example where three output patterns
    // are required:
    //
    //   DeclareOptimizationRule(
    //     nativeLibraryId,
    //     U"rule",
    //     Call(&foo, OutReturn(), Out("outA"), Out("outB")),
    //     1.0f,
    //     Named("outA", 2.0f),
    //     Named("outB", 3.0f))
    //
    // In this example, foo's first output argument is replaced with 1.0f, its second with 2.0f, and its third with 3.0f.
    //
    // The following pattern syntax is supported:
    //
    //   Named(name, <value>)
    //     Wraps a value (such as a constant) in a name so that it can be referenced in an output pattern. Also used to declare named output patterns.
    //
    //   Call([name], nativeModule, [upsampleFactor], <arg0>, <arg1>, ...)
    //     Matches against a call of the provided native module with the provided arguments. upsampleFactor is specified by appending _x to an integer, e.g.
    //     2_x, 3_x, etc.
    //
    //   <constant>
    //     A constant of type f32, f64, s32, bool, or const char32_t* can be specified to match against a constant with this exact value.
    //
    //   In([name])
    //     Matches against any input value.
    //
    //   Constant([name])
    //     Matches against an input value only if it is a compile-time constant.
    //
    //   Out([name])
    //     Used as a placeholder for an output argument. A name must be provided when used within input patterns and a correspondingly named output pattern must
    //     then be provided. This can also be used to represent a "throwaway" output argument within output patterns.
    //
    //   OutReturn()
    //     Used to mark the output argument with a native module call which is to be consumed either by an outer call, an array, or as the pattern's final
    //     resulting value.
    //
    //   Array(<el0>, <el1>, ...)
    //     Matches against an array with specific elements.
    template<typename TInputPattern, typename TReturnPattern, typename... TOutputPatterns>
    auto DeclareOptimizationRule(
      const Guid& nativeLibraryId,
      const char32_t* name,
      const TInputPattern& inputPattern,
      const TReturnPattern& returnPattern,
      const TOutputPatterns&... outputPatterns)
    {
      // When counting components, add an extra one for each EndOfList element
      static constexpr usz InputPatternComponentCount = CountComponents<TInputPattern>() + 1;
      static constexpr usz ReturnPatternComponentCount = CountComponents<TReturnPattern>() + 1;
      static constexpr usz OutputPatternsComponentCount =
        []()
        {
          usz result = 0;
          Unroll<0, sizeof...(TOutputPatterns)>(
            [&](auto i)
              { result += (CountComponents<std::tuple_element_t<decltype(i)::value, std::tuple<TOutputPatterns...>>>() + 1); });
          return result;
        }();

      static constexpr usz ComponentCount = InputPatternComponentCount + ReturnPatternComponentCount + OutputPatternsComponentCount;

      BuiltOptimizationRule<ComponentCount, 1 + sizeof...(TOutputPatterns)> result;
      result.m_optimizationRule.m_name = name;
      usz componentIndex = 0;
      BoundedArray<const char*, InputPatternComponentCount> inputPatternOutputNames;
      bool inputPatternBuilt = false;

      auto FindInputComponentIndex =
        [&](const char* name) -> s32
        {
          ASSERT(name != nullptr, "Input name not provided");
          ASSERT(inputPatternBuilt, "Input references can only be used in output patterns");

          std::optional<usz> result;
          usz index = 0;
          IterateComponents(
            inputPattern,
            [&]([[maybe_unused]] const auto& component, const char* componentName)
            {
              if (componentName != nullptr && NullTerminatedStringsEqual(name, componentName))
              {
                ASSERT(!result.has_value(), "Multiple input components use the same name");
                result = index;
              }

              index++;
            });

          ASSERT(result.has_value(), "Input component with the provided name not found");
          return Coerce<s32>(result.value());
        };

      auto AddInputPatternOutputName =
        [&](const char* name)
        {
          for (const char* existingName : inputPatternOutputNames)
            { ASSERT(!NullTerminatedStringsEqual(name, existingName), "Two input pattern outputs share the same name"); }
          inputPatternOutputNames.Append(name);
        };

      auto GetAndRemoveInputPatternOutputIndex =
        [&](const char* name) -> usz
        {
          for (usz i = 0; i < inputPatternOutputNames.Count(); i++)
          {
            if (inputPatternOutputNames[i] != nullptr && NullTerminatedStringsEqual(name, inputPatternOutputNames[i]))
            {
              inputPatternOutputNames[i] = nullptr;
              return i;
            }
          }

          ASSERT(false, "Input pattern does not contain an output with the provided name");
          return 0;
        };

      auto ProcessComponent =
        [&](const auto& component, const char* name)
        {
          using Component = std::remove_cvref_t<decltype(component)>;
          OptimizationRuleComponent& outputComponent = result.m_components[componentIndex++];

          if constexpr (constant_value<Component>)
          {
            outputComponent.m_type = OptimizationRuleComponentTypeConstant;
            if constexpr (std::same_as<Component, f32>)
            {
              outputComponent.m_data.m_constantData.m_primitiveType = PrimitiveTypeFloat;
              outputComponent.m_data.m_constantData.m_value.m_floatValue = component;
            }
            else if constexpr (std::same_as<Component, f64>)
            {
              outputComponent.m_data.m_constantData.m_primitiveType = PrimitiveTypeDouble;
              outputComponent.m_data.m_constantData.m_value.m_doubleValue = component;
            }
            else if constexpr (std::same_as<Component, s32>)
            {
              outputComponent.m_data.m_constantData.m_primitiveType = PrimitiveTypeInt;
              outputComponent.m_data.m_constantData.m_value.m_intValue = component;
            }
            else if constexpr (std::same_as<Component, bool>)
            {
              outputComponent.m_data.m_constantData.m_primitiveType = PrimitiveTypeBool;
              outputComponent.m_data.m_constantData.m_value.m_boolValue = component;
            }
            else if constexpr (std::convertible_to<Component, const char32_t*>)
            {
              outputComponent.m_data.m_constantData.m_primitiveType = PrimitiveTypeString;
              outputComponent.m_data.m_constantData.m_value.m_stringValue = component; }
            else
              { static_assert(AlwaysFalse<Component>, "Unsupported optimization rule constant type"); }
          }
          else if constexpr (Component::Type == ComponentType::NativeModuleCall)
          {
            static constexpr s32 OutputParameterIndex = Coerce<s32>(FindReturnOutParameterIndex<Component>());
            outputComponent.m_type = OptimizationRuleComponentTypeNativeModuleCall;
            Span(outputComponent.m_data.m_nativeModuleCallData.m_nativeLibraryId).CopyElementsFrom(nativeLibraryId.Bytes());
            Span(outputComponent.m_data.m_nativeModuleCallData.m_nativeModuleId).CopyElementsFrom(Span(component.m_nativeModule->m_id));
            outputComponent.m_data.m_nativeModuleCallData.m_upsampleFactor = component.m_upsampleFactor;
            outputComponent.m_data.m_nativeModuleCallData.m_outputParameterIndex = OutputParameterIndex;
          }
          else if constexpr (Component::Type == ComponentType::In)
          {
            ASSERT(!inputPatternBuilt, "Wildcard inputs can only be used in input patterns");
            outputComponent.m_type = OptimizationRuleComponentTypeInput;
            outputComponent.m_data.m_inputData.m_mustBeConstant = Component::MustBeConstant;
          }
          else if constexpr (Component::Type == ComponentType::Out)
          {
            outputComponent.m_type = OptimizationRuleComponentTypeOutput;

            if (!inputPatternBuilt)
            {
              // Build up the named outputs
              if constexpr (!Component::IsReturn)
              {
                ASSERT(name != nullptr, "Input pattern non-return outputs must be named");
                AddInputPatternOutputName(name);
              }
            }
            else
              { ASSERT(name == nullptr, "Output pattern outputs should not be named when used in output patterns"); }
          }
          else if constexpr (Component::Type == ComponentType::Array)
          {
            outputComponent.m_type = OptimizationRuleComponentTypeArray;
            outputComponent.m_data.m_arrayData.m_elementCount = Coerce<int32_t>(std::tuple_size_v<decltype(component.m_elements)>);
          }
          else if constexpr (Component::Type == ComponentType::InRef)
          {
            ASSERT(inputPatternBuilt, "Input references can only be used in output patterns");
            outputComponent.m_type = OptimizationRuleComponentTypeInputReference;
            outputComponent.m_data.m_inputReferenceData.m_index = FindInputComponentIndex(name);
          }
          else
            { static_assert(AlwaysFalse<Component>, "Unsupported optimization rule component type"); }
        };

      IterateComponents(inputPattern, ProcessComponent);
      result.m_components[componentIndex++].m_type = OptimizationRuleComponentTypeEndOfList;
      inputPatternBuilt = true;

      result.m_outputPatterns[0] = &result.m_components[componentIndex];
      IterateComponents(returnPattern, ProcessComponent);
      result.m_components[componentIndex++].m_type = OptimizationRuleComponentTypeEndOfList;

      auto outputPatternsTuple = std::make_tuple(outputPatterns...);
      Unroll<0, sizeof...(TOutputPatterns)>(
        [&](auto i)
        {
          static constexpr usz OutputPatternIndex = decltype(i)::value;
          const auto& outputPattern = std::get<OutputPatternIndex>(outputPatternsTuple);
          using OutputPattern = decltype(outputPattern);
          if constexpr (!requires (OutputPattern p) { { p.m_name }; })
            { static_assert(AlwaysFalse<OutputPattern>, "Output patterns must be named"); }
          else
          {
            usz outputIndex = GetAndRemoveInputPatternOutputIndex(outputPattern.m_name);
            result.m_outputPatterns[outputIndex + 1] = &result.m_components[componentIndex]; // Add 1 because the 0th pattern is for OutReturn()
            IterateComponents(outputPattern.m_value, ProcessComponent);
            result.m_components[componentIndex++].m_type = OptimizationRuleComponentTypeEndOfList;
          }
        });


      ASSERT(componentIndex == ComponentCount);
      return result;
    }
  }
}