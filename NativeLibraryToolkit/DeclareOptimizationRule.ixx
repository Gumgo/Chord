module;

#include "../NativeLibraryApi/ChordNativeLibraryApi.h"

export module Chord.NativeLibraryToolkit:DeclareOptimizationRule;

import std;

import Chord.Foundation;

namespace Chord
{
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
  }

  template<typename T>
  concept constant_value = std::same_as<T, f32>
    || std::same_as<T, f64>
    || std::same_as<T, s32>
    || std::same_as<T, bool>
    || std::convertible_to<T, const char32_t*>;

  enum class ComponentType
  {
    Variable,
    Constant,
    NativeModuleCallReference,
    NativeModuleCall,
    ArrayReference,
    Array,
    Output,
    Return,
    OutputPattern,
  };

  // !!! should these stay out of syntax namespace?
  struct OptimizationRuleComponentBase
    { };

  template<typename T>
  consteval usz CountComponents()
  {
    if constexpr (constant_value<T>)
      { return 1; }
    else
    {
      static_assert(std::derived_from<T, OptimizationRuleComponentBase>, "Unsupported optimization rule component type");
      if constexpr (requires { { T::ComponentCount } -> std::convertible_to<usz>; })
        { return T::ComponentCount; }
      else
        { return 1; }
    }
  }

  template<ComponentType ComponentTypeArg, s32 ReferenceIdArg>
  struct TypedOptimizationRuleComponent : public OptimizationRuleComponentBase
  {
    static constexpr ComponentType ComponentType = ComponentTypeArg;
    static constexpr s32 ReferenceId = ReferenceIdArg;
  };

  template<s32 ReferenceIdArg, typename... TArguments>
  struct NativeModuleCallOptimizationRuleComponent : public TypedOptimizationRuleComponent<ComponentType::NativeModuleCall, ReferenceIdArg>
  {
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

  template<s32 ReferenceIdArg>
  struct NativeModuleCallReferenceOptimizationRuleComponent : public TypedOptimizationRuleComponent<ComponentType::NativeModuleCallReference, ReferenceIdArg>
  {
    template<typename... TArgs>
    auto operator()(const NativeModule* nativeModule, const TArgs&... args) const
    {
      return NativeModuleCallOptimizationRuleComponent<ReferenceIdArg, TArgs...>
      {
        .m_nativeModule = nativeModule,
        .m_upsampleFactor = 1,
        .m_arguments = std::make_tuple(args...),
      };
    }

    template<typename... TArgs>
    auto operator()(const NativeModule* nativeModule, UpsampleFactor upsampleFactor, const TArgs&... args) const
    {
      return NativeModuleCallOptimizationRuleComponent<ReferenceIdArg, TArgs...>
      {
        .m_nativeModule = nativeModule,
        .m_upsampleFactor = upsampleFactor.m_value,
        .m_arguments = std::make_tuple(args...),
      };
    }
  };

  template<s32 ReferenceIdArg, typename... TElements>
  struct ArrayOptimizationRuleComponent : public TypedOptimizationRuleComponent<ComponentType::Array, ReferenceIdArg>
  {
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

  template<s32 ReferenceIdArg>
  struct ArrayReferenceOptimizationRuleComponent : public TypedOptimizationRuleComponent<ComponentType::ArrayReference, ReferenceIdArg>
  {
    template<typename... TElements>
    auto operator()(const TElements&... elements) const
      { return ArrayOptimizationRuleComponent<ReferenceIdArg, TElements...> { .m_elements = std::make_tuple(elements...) }; }
  };

  template<s32 ReferenceIdArg, typename TPattern>
  struct OutputPatternOptimizationRuleComponent : public TypedOptimizationRuleComponent<ComponentType::OutputPattern, ReferenceIdArg>
  {
    TPattern m_pattern;
  };

  template<s32 ReferenceIdArg>
  struct OutputOptimizationRuleComponent : public TypedOptimizationRuleComponent<ComponentType::Output, ReferenceIdArg>
  {
    template<typename TPattern>
    auto operator()(const TPattern& pattern) const
      { return OutputPatternOptimizationRuleComponent<ReferenceIdArg, TPattern> { .m_pattern = pattern }; }
  };

  template<typename T, typename TFunc>
  void IterateComponents(const T& component, TFunc&& func)
  {
    func(component);
    if constexpr (std::derived_from<T, OptimizationRuleComponentBase>)
    {
      if constexpr (T::ComponentType == ComponentType::NativeModuleCall)
      {
        Unroll<0, std::tuple_size_v<decltype(component.m_arguments)>>(
          [&](auto i)
            { IterateComponents(std::get<decltype(i)::value>(component.m_arguments), std::forward<TFunc>(func)); });
      }
      else if constexpr (T::ComponentType == ComponentType::Array)
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
        using Argument = std::tuple_element_t<ArgumentIndex, decltype(Component::m_arguments)>;
        if constexpr (std::derived_from<Argument, OptimizationRuleComponentBase>)
        {
          if constexpr (Argument::ComponentType == ComponentType::Return)
          {
            ASSERT(!index.has_value(), "Multiple return out arguments specified");
            index = ArgumentIndex;
          }
        }
      });

    ASSERT(index.has_value(), "Return out argument not specified");
    return index.value();
  }

  export
  {
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
      inline constexpr TypedOptimizationRuleComponent<ComponentType::Variable, -1> V;
      inline constexpr TypedOptimizationRuleComponent<ComponentType::Variable, 0> V0;
      inline constexpr TypedOptimizationRuleComponent<ComponentType::Variable, 1> V1;
      inline constexpr TypedOptimizationRuleComponent<ComponentType::Variable, 2> V2;
      inline constexpr TypedOptimizationRuleComponent<ComponentType::Variable, 3> V3;
      inline constexpr TypedOptimizationRuleComponent<ComponentType::Variable, 4> V4;
      inline constexpr TypedOptimizationRuleComponent<ComponentType::Variable, 5> V5;
      inline constexpr TypedOptimizationRuleComponent<ComponentType::Variable, 6> V6;
      inline constexpr TypedOptimizationRuleComponent<ComponentType::Variable, 7> V7;
      inline constexpr TypedOptimizationRuleComponent<ComponentType::Variable, 8> V8;
      inline constexpr TypedOptimizationRuleComponent<ComponentType::Variable, 9> V9;

      inline constexpr TypedOptimizationRuleComponent<ComponentType::Constant, -1> C;
      inline constexpr TypedOptimizationRuleComponent<ComponentType::Constant, 0> C0;
      inline constexpr TypedOptimizationRuleComponent<ComponentType::Constant, 1> C1;
      inline constexpr TypedOptimizationRuleComponent<ComponentType::Constant, 2> C2;
      inline constexpr TypedOptimizationRuleComponent<ComponentType::Constant, 3> C3;
      inline constexpr TypedOptimizationRuleComponent<ComponentType::Constant, 4> C4;
      inline constexpr TypedOptimizationRuleComponent<ComponentType::Constant, 5> C5;
      inline constexpr TypedOptimizationRuleComponent<ComponentType::Constant, 6> C6;
      inline constexpr TypedOptimizationRuleComponent<ComponentType::Constant, 7> C7;
      inline constexpr TypedOptimizationRuleComponent<ComponentType::Constant, 8> C8;
      inline constexpr TypedOptimizationRuleComponent<ComponentType::Constant, 9> C9;

      inline constexpr NativeModuleCallReferenceOptimizationRuleComponent<-1> Call;
      inline constexpr NativeModuleCallReferenceOptimizationRuleComponent<0> Call0;
      inline constexpr NativeModuleCallReferenceOptimizationRuleComponent<1> Call1;
      inline constexpr NativeModuleCallReferenceOptimizationRuleComponent<2> Call2;
      inline constexpr NativeModuleCallReferenceOptimizationRuleComponent<3> Call3;
      inline constexpr NativeModuleCallReferenceOptimizationRuleComponent<4> Call4;
      inline constexpr NativeModuleCallReferenceOptimizationRuleComponent<5> Call5;
      inline constexpr NativeModuleCallReferenceOptimizationRuleComponent<6> Call6;
      inline constexpr NativeModuleCallReferenceOptimizationRuleComponent<7> Call7;
      inline constexpr NativeModuleCallReferenceOptimizationRuleComponent<8> Call8;
      inline constexpr NativeModuleCallReferenceOptimizationRuleComponent<9> Call9;

      inline constexpr ArrayReferenceOptimizationRuleComponent<-1> Array;
      inline constexpr ArrayReferenceOptimizationRuleComponent<0> Array0;
      inline constexpr ArrayReferenceOptimizationRuleComponent<1> Array1;
      inline constexpr ArrayReferenceOptimizationRuleComponent<2> Array2;
      inline constexpr ArrayReferenceOptimizationRuleComponent<3> Array3;
      inline constexpr ArrayReferenceOptimizationRuleComponent<4> Array4;
      inline constexpr ArrayReferenceOptimizationRuleComponent<5> Array5;
      inline constexpr ArrayReferenceOptimizationRuleComponent<6> Array6;
      inline constexpr ArrayReferenceOptimizationRuleComponent<7> Array7;
      inline constexpr ArrayReferenceOptimizationRuleComponent<8> Array8;
      inline constexpr ArrayReferenceOptimizationRuleComponent<9> Array9;

      inline constexpr OutputOptimizationRuleComponent<-1> Out;
      inline constexpr OutputOptimizationRuleComponent<0> Out0;
      inline constexpr OutputOptimizationRuleComponent<1> Out1;
      inline constexpr OutputOptimizationRuleComponent<2> Out2;
      inline constexpr OutputOptimizationRuleComponent<3> Out3;
      inline constexpr OutputOptimizationRuleComponent<4> Out4;
      inline constexpr OutputOptimizationRuleComponent<5> Out5;
      inline constexpr OutputOptimizationRuleComponent<6> Out6;
      inline constexpr OutputOptimizationRuleComponent<7> Out7;
      inline constexpr OutputOptimizationRuleComponent<8> Out8;
      inline constexpr OutputOptimizationRuleComponent<9> Out9;

      inline constexpr TypedOptimizationRuleComponent<ComponentType::Return, -1> Ret;
    }

    // This function is used to declare an optimization rule using a convenient shorthand syntax. It returns a BuiltOptimizationRule instance which holds both
    // the optimization rule itself as well as storage for the optimization rule components.
    //
    // Optimization rules are specified using patterns, where the input pattern is replaced with one or more output patterns. Patterns are built using nested
    // function calls intended to appear similar to how the function would appear in a Chord script (though more verbose). These functions are found within the
    // OptimizationRuleSyntax namespace. Here is an example of an input pattern:
    //
    //   Call(&add, V0, Call(&negate, V1, Ret), Ret)
    //
    // This pattern would match against Chord expressions of the form "V0 + -V1". We could associate this input pattern with the following output pattern:
    //
    //   Call(&subtract, V0, V1, Ret)
    //
    // This would cause Chord expressions of the form "V0 + -V1" to be replaced with "V0 - V1".
    //
    // Because native modules support multiple output parameters, an optimization rule must have an output pattern for every non-consumed output within the
    // input pattern. An output is "consumed" when it is passed down to a nested call. For example, in "Foo(Bar())", the return value of Bar() is consumed by
    // Foo(). Internally, native modules don't actually "return" values, but rather one output parameter may optionally be marked as "return". Therefore, when
    // declaring optimization rules, the consumed output parameter must be explicitly specified with Ret. So the previous example would actually appear as
    // "Call(&foo, Call(&bar, Ret), Ret)", which means: the first (output) argument of Bar() is consumed as the first (input) argument to Foo() and the second
    // (output) argument of Foo() is consumed as the pattern's final resulting value.
    //
    // When additional output arguments are present, they are marked with Out<N> in the input pattern, where N is an integer. For each named output argument in
    // the input pattern, an additional named output pattern must be specified using the syntax: Out<N>(<pattern>). The following is an example where three
    // output patterns are required:
    //
    //   DeclareOptimizationRule(
    //     nativeLibraryId,
    //     U"rule",
    //     Call(&foo, Ret, Out0, Out1),
    //     1.0f,
    //     Out0(2.0f),
    //     Out1(3.0f))
    //
    // In this example, foo's first output argument is replaced with 1.0f, its second with 2.0f, and its third with 3.0f.
    //
    // The following pattern syntax is supported:
    //
    //   <constant>
    //     A constant of type f32, f64, s32, bool, or const char32_t* can be specified to match against a constant with this exact value.
    //
    //   V[<N>]
    //     Matches against any input value within an input pattern. References the correspondingly indexed input pattern value within an output pattern.
    //
    //   C[<N>]
    //     Matches against an input value only if it is a compile-time constant within an input pattern. References the correspondingly indexed input pattern
    //     value within an output pattern.
    //
    //   Call[<N>](nativeModule, [upsampleFactor], <arg0>, <arg1>, ...)
    //     Matches against a call of the provided native module with the provided arguments. upsampleFactor is specified by appending _x to an integer, e.g.
    //     2_x, 3_x, etc.
    //
    //   Call<N>
    //     References the correspondingly indexed native module call within the input pattern. Only allowed in output patterns.
    //
    //   Array[<N>](<el0>, <el1>, ...)
    //     Matches against an array with specific elements.
    //
    //   Array<N>
    //     References the correspondingly indexed array within the input pattern. Only allowed in output patterns.
    //
    //   Out[<N>]
    //     Used as a placeholder for an output argument. An index must be provided when used within input patterns and a correspondingly indexed output pattern
    //     must then be provided. This can also be used to represent a "throwaway" output argument within output patterns.
    //
    //   Ret
    //     Used to mark the output argument with a native module call which is to be consumed either by an outer call, an array, or as the pattern's final
    //     resulting value.
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
      BoundedArray<s32, InputPatternComponentCount> inputPatternOutputReferenceIds;

      auto ValidateUniqueInputComponentReferenceId =
        [&](ComponentType componentType, s32 referenceId)
        {
          if (referenceId < 0)
            { return; }

          bool found = false;
          IterateComponents(
            inputPattern,
            [&](const auto& component)
            {
              using Component = decltype(component);
              if constexpr (std::derived_from<Component, OptimizationRuleComponentBase>)
              {
                if (Component::ComponentType == componentType && Component::ReferenceId == referenceId)
                {
                  ASSERT(!found, "Optimization rule uses multiple components with the same reference ID");
                  found = true;
                }
              }
            });
        };

      auto FindInputComponentIndex =
        [&](ComponentType componentType, s32 referenceId) -> s32
        {
          std::optional<usz> result;
          usz index = 0;
          IterateComponents(
            inputPattern,
            [&](const auto& component)
            {
              if (!result.has_value())
              {
                using Component = std::remove_cvref_t<decltype(component)>;
                if constexpr (std::derived_from<Component, OptimizationRuleComponentBase>)
                {
                  if (Component::ComponentType == componentType && Component::ReferenceId == referenceId)
                    { result = index; }
                }
              }

              index++;
            });

          ASSERT(result.has_value(), "Optimization rule input pattern does not contain a component with the referenced type and ID");
          return Coerce<s32>(result.value());
        };

      auto GetInputComponentIndex =
        [&](const auto& component) -> s32
        {
          std::optional<usz> result;
          usz index = 0;
          IterateComponents(
            inputPattern,
            [&](const auto& otherComponent)
            {
              if constexpr (std::same_as<std::remove_cvref_t<decltype(component)>, std::remove_cvref_t<decltype(otherComponent)>>)
              {
                if (&component == &otherComponent)
                  { result = index; }
              }

              index++;
            });

          ASSERT(result.has_value());
          return Coerce<s32>(result.value());
        };

      auto AddInputPatternOutputReferenceId =
        [&](s32 referenceId)
        {
          ASSERT(referenceId >= 0);
          ASSERT(!inputPatternOutputReferenceIds.Contains(referenceId), "Two input pattern outputs share the same reference ID");
          inputPatternOutputReferenceIds.Append(referenceId);
        };

      auto GetAndRemoveInputPatternOutputIndex =
        [&](s32 referenceId) -> usz
        {
          ASSERT(referenceId >= 0);
          auto index = inputPatternOutputReferenceIds.FirstIndexOf(referenceId);
          ASSERT(index.has_value(), "Input pattern does not contain an output with the provided reference ID");
          inputPatternOutputReferenceIds[index.value()] = -1;
          return index.value();
        };

      auto ProcessComponent =
        [&]<bool IsOutputPattern>(const auto& component)
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
          else if constexpr (Component::ComponentType == ComponentType::Variable)
          {
            if constexpr (!IsOutputPattern)
            {
              ValidateUniqueInputComponentReferenceId(Component::ComponentType, Component::ReferenceId);
              outputComponent.m_type = OptimizationRuleComponentTypeInput;
              outputComponent.m_data.m_inputData.m_mustBeConstant = false;
            }
            else
            {
              static_assert(Component::ReferenceId >= 0, "Optimization rule output pattern variable component must reference the input pattern");
              s32 index = FindInputComponentIndex(Component::ComponentType, Component::ReferenceId);
              ASSERT(index >= 0, "Optimization rule input pattern variable component with the given reference ID does not exist");
              outputComponent.m_type = OptimizationRuleComponentTypeInputReference;
              outputComponent.m_data.m_inputReferenceData.m_index = index;
            }
          }
          else if constexpr (Component::ComponentType == ComponentType::Constant)
          {
            if constexpr (!IsOutputPattern)
            {
              // Constants can be reused within an input pattern. The later uses refer to the initial usage.
              if constexpr (Component::ReferenceId < 0)
              {
                // This constant doesn't have a reference ID
                outputComponent.m_type = OptimizationRuleComponentTypeInput;
                outputComponent.m_data.m_inputData.m_mustBeConstant = true;
              }
              else
              {
                if (FindInputComponentIndex(Component::ComponentType, Component::ReferenceId) == GetInputComponentIndex(component))
                {
                  // This constant is the first use with this reference ID
                  outputComponent.m_type = OptimizationRuleComponentTypeInput;
                  outputComponent.m_data.m_inputData.m_mustBeConstant = true;
                }
                else
                {
                  // This constant refers back to the first use of a constant with this ID
                  outputComponent.m_type = OptimizationRuleComponentTypeInputReference;
                  outputComponent.m_data.m_inputReferenceData.m_index = FindInputComponentIndex(Component::ComponentType, Component::ReferenceId);
                }
              }
            }
            else
            {
              static_assert(Component::ReferenceId >= 0, "Optimization rule output pattern constant component must reference the input pattern");
              outputComponent.m_type = OptimizationRuleComponentTypeInputReference;
              outputComponent.m_data.m_inputReferenceData.m_index = FindInputComponentIndex(Component::ComponentType, Component::ReferenceId);
            }
          }
          else if constexpr (Component::ComponentType == ComponentType::NativeModuleCallReference)
          {
            static_assert(IsOutputPattern, "Optimization rule input pattern native module call component cannot be a reference");
            outputComponent.m_type = OptimizationRuleComponentTypeInputReference;
            outputComponent.m_data.m_inputReferenceData.m_index = FindInputComponentIndex(ComponentType::NativeModuleCall, Component::ReferenceId);
          }
          else if constexpr (Component::ComponentType == ComponentType::NativeModuleCall)
          {
            if constexpr (IsOutputPattern)
              { static_assert(Component::ReferenceId < 0, "Optimization rule output pattern native module call component cannot have a reference ID"); }

            static constexpr s32 OutputParameterIndex = Coerce<s32>(FindReturnOutParameterIndex<Component>());
            outputComponent.m_type = OptimizationRuleComponentTypeNativeModuleCall;
            Span(outputComponent.m_data.m_nativeModuleCallData.m_nativeLibraryId).CopyElementsFrom(nativeLibraryId.Bytes());
            Span(outputComponent.m_data.m_nativeModuleCallData.m_nativeModuleId).CopyElementsFrom(Span(component.m_nativeModule->m_id));
            outputComponent.m_data.m_nativeModuleCallData.m_upsampleFactor = component.m_upsampleFactor;
            outputComponent.m_data.m_nativeModuleCallData.m_outputParameterIndex = OutputParameterIndex;
          }
          else if constexpr (Component::ComponentType == ComponentType::ArrayReference)
          {
            static_assert(IsOutputPattern, "Optimization rule input pattern array component cannot be a reference");
            outputComponent.m_type = OptimizationRuleComponentTypeInputReference;
            outputComponent.m_data.m_inputReferenceData.m_index = FindInputComponentIndex(ComponentType::Array, Component::ReferenceId);
          }
          else if constexpr (Component::ComponentType == ComponentType::Array)
          {
            if constexpr (IsOutputPattern)
              { static_assert(Component::ReferenceId < 0, "Optimization rule output pattern array component cannot have a reference ID"); }

            outputComponent.m_type = OptimizationRuleComponentTypeArray;
            outputComponent.m_data.m_arrayData.m_elementCount = Coerce<int32_t>(std::tuple_size_v<decltype(component.m_elements)>);
          }
          else if constexpr (Component::ComponentType == ComponentType::Output)
          {
            if constexpr (!IsOutputPattern)
            {
              static_assert(Component::ReferenceId >= 0, "Optimization rule output components within the input pattern must have reference IDs");
              AddInputPatternOutputReferenceId(Component::ReferenceId);
            }
            else
              { static_assert(Component::ReferenceId < 0, "Optimization rule output components within the output pattern cannot have reference IDs"); }
            outputComponent.m_type = OptimizationRuleComponentTypeOutput;
          }
          else if constexpr (Component::ComponentType == ComponentType::Return)
            { outputComponent.m_type = OptimizationRuleComponentTypeOutput; }
          else
            { static_assert(AlwaysFalse<Component>, "Unsupported optimization rule component type"); }
        };

      auto ProcessComponentInputPattern = [&](const auto& c) { ProcessComponent.operator()<false>(c); };
      auto ProcessComponentOutputPattern = [&](const auto& c) { ProcessComponent.operator()<true>(c); };

      IterateComponents(inputPattern, ProcessComponentInputPattern);
      result.m_components[componentIndex++].m_type = OptimizationRuleComponentTypeEndOfList;

      result.m_outputPatterns[0] = &result.m_components[componentIndex];
      IterateComponents(returnPattern, ProcessComponentOutputPattern);
      result.m_components[componentIndex++].m_type = OptimizationRuleComponentTypeEndOfList;

      auto outputPatternsTuple = std::make_tuple(outputPatterns...);
      Unroll<0, sizeof...(TOutputPatterns)>(
        [&](auto i)
        {
          static constexpr usz OutputPatternIndex = decltype(i)::value;
          const auto& outputPattern = std::get<OutputPatternIndex>(outputPatternsTuple);
          using OutputPattern = std::remove_cvref_t<decltype(outputPattern)>;
          static_assert(std::derived_from<OutputPattern, OptimizationRuleComponentBase>, "Output patterns must be wrapped in Out<i>(...)");
          static_assert(
            OutputPattern::ComponentType == ComponentType::OutputPattern && OutputPattern::ReferenceId >= 0,
            "Output patterns must be wrapped in Out<i>(...)");

          usz outputIndex = GetAndRemoveInputPatternOutputIndex(OutputPattern::ReferenceId);
          result.m_outputPatterns[outputIndex + 1] = &result.m_components[componentIndex]; // Add 1 because the 0th pattern is for OutReturn()
          IterateComponents(outputPattern.m_pattern, ProcessComponentOutputPattern);
          result.m_components[componentIndex++].m_type = OptimizationRuleComponentTypeEndOfList;
        });


      ASSERT(componentIndex == ComponentCount);
      return result;
    }
  }
}