module;

#include "../NativeLibraryApi/ChordNativeLibraryApi.h"

export module Chord.NativeLibraryToolkit:DeclareNativeModule;

import std;

import Chord.Foundation;
import :NativeModuleTypes;
import :StackAllocator;

namespace Chord
{
  template<bool MemberFunction, typename TReturn, typename... TArgs>
  struct FunctionTraitsData
  {
    static constexpr bool IsFunction = true;
    static constexpr bool IsMemberFunction = MemberFunction;
    static constexpr usz ArgumentCount = sizeof...(TArgs);

    using Return = TReturn;
    using Arguments = std::tuple<TArgs...>;

    template<typename TReturnTest>
    static constexpr bool Returns()
      { return std::same_as<TReturn, TReturnTest>; }

    template<typename TArgumentTest>
    static constexpr usz GetArgumentsOfTypeCount()
    {
      usz count = 0;
      Unroll<0, ArgumentCount>(
        [&](auto i)
        {
          if constexpr (std::same_as<std::tuple_element_t<decltype(i)::value, Arguments>, TArgumentTest>)
            { count++; }
        });

      return count;
    }

    template<typename TArgumentTest>
    static constexpr bool HasZeroOrOneArgumentsOfType()
    {
      usz count = GetArgumentsOfTypeCount<TArgumentTest>();
      return count == 0 || count == 1;
    }

    template<typename TArgumentTest>
    static constexpr bool HasOneArgumentOfType()
      { return GetArgumentsOfTypeCount<TArgumentTest>() == 1; }
  };

  static_assert(FunctionTraitsData<false, bool>::Returns<bool>());

  template<typename T>
  struct FunctionTraits
  {
    static constexpr bool IsFunction = false;
  };

  template<typename TReturn, typename... TArgs>
  struct FunctionTraits<TReturn(TArgs...)> : public FunctionTraitsData<false, TReturn, TArgs...>
    { };

  template<typename TReturn, typename... TArgs>
  struct FunctionTraits<TReturn(*)(TArgs...)> : public FunctionTraitsData<false, TReturn, TArgs...>
    { };

  template<typename T, typename TReturn, typename... TArgs>
  struct FunctionTraits<TReturn(T::*)(TArgs...)> : public FunctionTraitsData<true, TReturn, TArgs...>
    { };

  template<typename TFunc, typename ResolveArgument>
  auto CallWithArgumentResolution(TFunc func, ResolveArgument&& resolveArgument)
  {
    using Traits = FunctionTraits<TFunc>;
    static_assert(!Traits::IsMemberFunction);
    auto Call =
      [&]<usz... Index>(std::index_sequence<Index...>)
        { return func(resolveArgument.template operator()<std::tuple_element_t<Index, typename Traits::Arguments>>()...); };
    return Call(std::make_index_sequence<Traits::ArgumentCount>{});
  }

  template<typename TInstance, typename TFunc, typename ResolveArgument>
  auto CallWithArgumentResolution(TInstance* instance, TFunc func, ResolveArgument&& resolveArgument)
  {
    using Traits = FunctionTraits<TFunc>;
    static_assert(Traits::IsMemberFunction);
    auto Call =
      [&]<usz... Index>(std::index_sequence<Index...>)
        { return (instance->*func)(resolveArgument.template operator()<std::tuple_element_t<Index, typename Traits::Arguments>>()...); };
    return Call(std::make_index_sequence<Traits::ArgumentCount>{});
  }

  template<typename TNativeModule>
  consteval Guid GetNativeModuleId()
  {
    if constexpr (requires { { TNativeModule::Id } -> std::convertible_to<Guid>; })
      { return TNativeModule::Id; }
    else
    {
      static_assert(AlwaysFalse<TNativeModule>, "Native module is missing static 'Id' field");
      return Guid::Empty();
    }
  }

  template<typename TNativeModule>
  consteval const char32_t* GetNativeModuleName()
  {
    if constexpr (requires { { TNativeModule::Name } -> std::convertible_to<const char32_t*>; })
      { return TNativeModule::Name; }
    else
    {
      static_assert(AlwaysFalse<TNativeModule>, "Native module is missing static 'Name' field");
      return nullptr;
    }
  }

  template<typename TNativeModule>
  consteval bool GetNativeModuleHasSideEffects()
  {
    if constexpr (requires { TNativeModule::HasSideEffects; })
    {
      if constexpr (requires { { TNativeModule::HasSideEffects } -> std::convertible_to<bool>; })
        { return TNativeModule::HasSideEffects; }
      else
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'HasSideEffects' field is not a bool");
        return false;
      }
    }
    else
      { return false; }
  }

  template<typename TNativeModule>
  consteval bool GetNativeModuleAlwaysRuntime()
  {
    if constexpr (requires { TNativeModule::AlwaysRuntime; })
    {
      if constexpr (requires { { TNativeModule::AlwaysRuntime } -> std::convertible_to<bool>; })
        { return TNativeModule::AlwaysRuntime; }
      else
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'AlwaysRuntime' field is not a bool");
        return false;
      }
    }
    else
      { return false; }
  }

  template<typename TFunc>
  consteval usz CountNativeModuleParameters()
  {
    using Traits = FunctionTraits<TFunc>;
    usz parameterCount = 0;
    Unroll<0, Traits::ArgumentCount>(
      [&](auto i)
      {
        using Argument = std::tuple_element_t<decltype(i)::value, typename Traits::Arguments>;
        if constexpr (std::derived_from<Argument, NativeModuleArgumentBase>)
          { parameterCount++; }
      });

    return parameterCount;
  }

  template<typename TNativeModuleArgument>
  consteval std::tuple<NativeModuleParameter, bool> BuildNativeModuleParameter()
  {
    NativeModuleParameter parameter =
    {
      .m_direction = TNativeModuleArgument::Direction,
      .m_name = TNativeModuleArgument::Name,
      .m_dataType =
      {
        .m_runtimeMutability = TNativeModuleArgument::RuntimeMutability,
        .m_primitiveType = TNativeModuleArgument::PrimitiveType,
        .m_upsampleFactor = TNativeModuleArgument::UpsampleFactor,
        .m_isArray = TNativeModuleArgument::IsArray,
      },
      .m_disallowBufferSharing = TNativeModuleArgument::DisallowBufferSharing,
    };

    return std::make_tuple(parameter, TNativeModuleArgument::IsReturn);
  }

  template<typename TFunc>
  consteval auto BuildNativeModuleParameters()
  {
    static constexpr usz ParameterCount = CountNativeModuleParameters<TFunc>();
    FixedArray<NativeModuleParameter, ParameterCount> parameters;

    using Traits = FunctionTraits<TFunc>;
    usz parameterIndex = 0;
    s32 returnParameterIndex = -1;
    Unroll<0, Traits::ArgumentCount>(
      [&](auto i)
      {
        using Argument = std::tuple_element_t<decltype(i)::value, typename FunctionTraits<TFunc>::Arguments>;
        if constexpr (std::derived_from<Argument, NativeModuleArgumentBase>)
        {
          auto [parameter, isReturn] = BuildNativeModuleParameter<Argument>();
          parameters[parameterIndex] = parameter;
          if (isReturn)
          {
            ASSERT(returnParameterIndex < 0, "Multiple return parameters specified");
            returnParameterIndex = Coerce<s32>(parameterIndex);
          }

          parameterIndex++;
        }
      });

    return std::make_tuple(parameters, returnParameterIndex);
  }

  template<typename TNativeModule>
  consteval auto GetNativeModuleParameters()
  {
    if constexpr (requires { &TNativeModule::Invoke; })
    {
      using Traits = FunctionTraits<decltype(&TNativeModule::Invoke)>;
      if constexpr (Traits::IsFunction)
        { return BuildNativeModuleParameters<decltype(&TNativeModule::Invoke)>(); }
      else
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'Invoke' is neither a static function nor a member function");
        return std::make_tuple(FixedArray<NativeModuleParameter, 0>(), -1);
      }
    }
    else if constexpr (requires { &TNativeModule::InvokeCompileTime; })
    {
      using Traits = FunctionTraits<decltype(&TNativeModule::InvokeCompileTime)>;
      if constexpr (Traits::IsFunction)
        { return BuildNativeModuleParameters<decltype(&TNativeModule::InvokeCompileTime)>(); }
      else
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'InvokeCompileTime' is neither a static function nor a member function");
        return std::make_tuple(FixedArray<NativeModuleParameter, 0>(), -1);
      }
    }
    else
    {
      static_assert(AlwaysFalse<TNativeModule>, "Native module has neither 'Invoke' nor 'InvokeCompileTime'");
      return std::make_tuple(FixedArray<NativeModuleParameter, 0>(), -1);
    }
  }

  template<typename TNativeModule>
  consteval NativeModuleSignature BuildNativeModuleSignature()
  {
    static constexpr auto ParametersAndReturnParameterIndex = GetNativeModuleParameters<TNativeModule>();
    return
    {
      .m_name = GetNativeModuleName<TNativeModule>(),
      .m_parameters = std::get<0>(ParametersAndReturnParameterIndex).Elements(),
      .m_parameterCount = std::get<0>(ParametersAndReturnParameterIndex).Count(),
      .m_returnParameterIndex = std::get<1>(ParametersAndReturnParameterIndex),
    };
  }

  template<typename TNativeModule>
  consteval usz GetNativeModuleOutParameterCount()
  {
    static constexpr NativeModuleSignature Signature = BuildNativeModuleSignature<TNativeModule>();
    usz count = 0;
    for (usz i = 0; i < Signature.m_parameterCount; i++)
    {
      if (Signature.m_parameters[i].m_direction == ModuleParameterDirectionOut)
        { count++; }
    }

    return count;
  }

  template<typename TNativeModule>
  consteval bool ShouldInstantiateNativeModuleClass()
  {
    // If any native module functions are non-static (except for prepare, which must always be static), we need to instantiate the class
    if constexpr (requires { &TNativeModule::InitializeVoice; })
    {
      using Traits = FunctionTraits<decltype(&TNativeModule::InitializeVoice)>;
      if constexpr (Traits::IsFunction && Traits::IsMemberFunction)
        { return true; }
    }

    if constexpr (requires { &TNativeModule::DeinitializeVoice; })
    {
      using Traits = FunctionTraits<decltype(&TNativeModule::DeinitializeVoice)>;
      if constexpr (Traits::IsFunction && Traits::IsMemberFunction)
        { return true; }
    }

    if constexpr (requires { &TNativeModule::SetVoiceActive; })
    {
      using Traits = FunctionTraits<decltype(&TNativeModule::SetVoiceActive)>;
      if constexpr (Traits::IsFunction && Traits::IsMemberFunction)
        { return true; }
    }

    if constexpr (requires { &TNativeModule::Invoke; })
    {
      using Traits = FunctionTraits<decltype(&TNativeModule::Invoke)>;
      if constexpr (Traits::IsFunction && Traits::IsMemberFunction)
        { return true; }
    }

    if constexpr (requires { &TNativeModule::InvokeCompileTime; })
    {
      using Traits = FunctionTraits<decltype(&TNativeModule::InvokeCompileTime)>;
      if constexpr (Traits::IsFunction && Traits::IsMemberFunction)
        { return true; }
    }

    return false;
  }

  template<typename TNativeModule, typename TArgument>
  consteval usz ResolveNativeModuleArgumentParameterIndex()
  {
    static constexpr NativeModuleSignature Signature = BuildNativeModuleSignature<TNativeModule>();

    // First, find the parameter with the matching name
    usz parameterIndex;
    for (parameterIndex = 0; parameterIndex < Signature.m_parameterCount; parameterIndex++)
    {
      if (NullTerminatedStringsEqual(Signature.m_parameters[parameterIndex].m_name, TArgument::Name))
        { break; }
    }

    ASSERT(parameterIndex < Signature.m_parameterCount, "Native module argument name does not match any parameter");

    // Now, make sure the type matches
    const NativeModuleParameter& parameter = Signature.m_parameters[parameterIndex];
    ASSERT(parameter.m_direction == TArgument::Direction, "Native module parameter/argument direction mismatch");
    ASSERT(parameter.m_dataType.m_runtimeMutability == TArgument::RuntimeMutability, "Native module parameter/argument runtime mutability mismatch");
    ASSERT(parameter.m_dataType.m_primitiveType == TArgument::PrimitiveType, "Native module parameter/argument primitive type mismatch");
    ASSERT(parameter.m_dataType.m_upsampleFactor == TArgument::UpsampleFactor, "Native module parameter/argument upsample factor mismatch");
    ASSERT(parameter.m_dataType.m_isArray == TArgument::IsArray, "Native module parameter/argument is array mismatch");

    return parameterIndex;
  }

  template<typename TNativeModule, typename TArgument>
  auto ResolveNativeModuleArgument(const NativeModuleArguments* arguments)
  {
    static constexpr usz ParameterIndex = ResolveNativeModuleArgumentParameterIndex<TNativeModule, TArgument>();
    return TArgument(arguments->m_arguments[ParameterIndex]);
  }

  template<typename TNativeModule>
  NativeModulePrepare BuildNativeModulePrepare()
  {
    if constexpr (requires { &TNativeModule::Prepare; })
    {
      using Traits = FunctionTraits<decltype(&TNativeModule::Prepare)>;
      if constexpr (!Traits::IsFunction)
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'Prepare' is not a function");
        return nullptr;
      }
      else if constexpr (Traits::IsMemberFunction)
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'Prepare' is a member function");
        return nullptr;
      }
      else if constexpr (!Traits::template Returns<bool>())
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'Prepare' must return a bool");
        return nullptr;
      }
      else if constexpr (!Traits::template HasZeroOrOneArgumentsOfType<NativeModuleCallContext>())
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'Prepare' should have at most one 'context' argument");
        return nullptr;
      }
      else if constexpr (!Traits::template HasZeroOrOneArgumentsOfType<Span<s32>>())
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'Prepare' should have at most one 'outArgumentsLatencyOut' argument");
        return nullptr;
      }
      else
      {
        auto PrepareWrapper =
          [](const NativeModuleContext* context, const NativeModuleArguments* arguments, int32_t* outArgumentLatenciesOut) -> bool
          {
            auto ResolveArgument =
              [&]<typename TArgument>()
              {
                if constexpr (std::same_as<TArgument, NativeModuleCallContext>)
                  { return NativeModuleCallContext(context); }
                else if constexpr (std::derived_from<TArgument, NativeModuleArgumentBase>)
                  { return ResolveNativeModuleArgument<TNativeModule, TArgument>(arguments); }
                else if constexpr (std::same_as<TArgument, Span<s32>>)
                  { return Span<s32>(outArgumentLatenciesOut, GetNativeModuleOutParameterCount<TNativeModule>()); }
                else
                  { static_assert(AlwaysFalse<TNativeModule>, "Unsupported argument type"); }
              };

            return CallWithArgumentResolution(&TNativeModule::Prepare, ResolveArgument);
          };

        return PrepareWrapper;
      }
    }
    else
      { return nullptr; }
  }

  template<typename TNativeModule>
  NativeModuleInitializeVoice BuildNativeModuleInitializeVoice()
  {
    static constexpr bool ShouldInstantiateClass = ShouldInstantiateNativeModuleClass<TNativeModule>();

    if constexpr (requires { &TNativeModule::InitializeVoice; })
    {
      using Traits = FunctionTraits<decltype(&TNativeModule::InitializeVoice)>;
      if constexpr (!Traits::IsFunction)
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'InitializeVoice' is not a function");
        return nullptr;
      }
      else if constexpr (!ShouldInstantiateClass && !Traits::template Returns<void*>())
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'InitializeVoice' must return a void pointer");
        return nullptr;
      }
      else if constexpr (ShouldInstantiateClass && !Traits::template Returns<void>())
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'InitializeVoice' must return void");
        return nullptr;
      }
      else if constexpr (!Traits::template HasZeroOrOneArgumentsOfType<NativeModuleCallContext>())
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'InitializeVoice' should have at most one 'context' argument");
        return nullptr;
      }
      else if constexpr (!Traits::template HasZeroOrOneArgumentsOfType<StackAllocatorCalculator&>())
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'InitializeVoice' should have at most one 'scratchMemoryAllocatorCalculator' argument");
        return nullptr;
      }
      else
      {
        auto InitializeVoiceWrapper =
          [](const NativeModuleContext* context, const NativeModuleArguments* arguments, MemoryRequirement* scratchMemoryRequirementOut) -> void*
          {
            StackAllocatorCalculator scratchMemoryAllocatorCalculator;

            auto ResolveArgument =
              [&]<typename TArgument>() -> decltype(auto)
              {
                if constexpr (std::same_as<TArgument, NativeModuleCallContext>)
                  { return NativeModuleCallContext(context); }
                else if constexpr (std::derived_from<TArgument, NativeModuleArgumentBase>)
                  { return ResolveNativeModuleArgument<TNativeModule, TArgument>(arguments); }
                else if constexpr (std::same_as<TArgument, StackAllocatorCalculator&>)
                  { return scratchMemoryAllocatorCalculator; }
                else
                  { static_assert(AlwaysFalse<TNativeModule>, "Unsupported argument type"); }
              };

            // Make sure this is returned by reference
            static_assert(std::same_as<decltype(ResolveArgument.operator()<StackAllocatorCalculator&>()), StackAllocatorCalculator&>);

            void* result;
            if constexpr (ShouldInstantiateClass)
            {
              // If ShouldInstantiateClass is true, the voice context is the class itself
              TNativeModule* voiceContext = new TNativeModule();
              if constexpr (Traits::IsMemberFunction)
                { CallWithArgumentResolution(voiceContext, &TNativeModule::InitializeVoice, ResolveArgument); }
              else
                { CallWithArgumentResolution(&TNativeModule::InitializeVoice, ResolveArgument); }

              result = voiceContext;
            }
            else
              { result = CallWithArgumentResolution(&TNativeModule::InitializeVoice, ResolveArgument); }

            *scratchMemoryRequirementOut = scratchMemoryAllocatorCalculator.GetMemoryRequirement();
            return result;
          };

        return InitializeVoiceWrapper;
      }
    }
    else if constexpr (ShouldInstantiateClass)
    {
      auto InitializeVoiceWrapper =
        []([[maybe_unused]] const NativeModuleContext* context,
          [[maybe_unused]] const NativeModuleArguments* arguments,
          MemoryRequirement* scratchMemoryRequirementOut) -> void*
        {
          scratchMemoryRequirementOut->m_size = 0;
          scratchMemoryRequirementOut->m_alignment = 0;
          return new TNativeModule();
        };

      return InitializeVoiceWrapper;
    }
    else
      { return nullptr; }
  }

  template<typename TNativeModule>
  NativeModuleDeinitializeVoice BuildNativeModuleDeinitializeVoice()
  {
    static constexpr bool ShouldInstantiateClass = ShouldInstantiateNativeModuleClass<TNativeModule>();

    if constexpr (requires { &TNativeModule::DeinitializeVoice; })
    {
      using Traits = FunctionTraits<decltype(&TNativeModule::DeinitializeVoice)>;
      if constexpr (!Traits::IsFunction)
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'DeinitializeVoice' is not a function");
        return nullptr;
      }
      else if constexpr (!Traits::template Returns<void>())
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'DeinitializeVoice' must return void");
        return nullptr;
      }
      else if constexpr (!Traits::template HasZeroOrOneArgumentsOfType<NativeModuleCallContext>())
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'DeinitializeVoice' should have at most one 'context' argument");
        return nullptr;
      }
      else
      {
        auto DeinitializeVoiceWrapper =
          [](const NativeModuleContext* context) -> void
          {
            auto ResolveArgument =
              [&]<typename TArgument>()
              {
                if constexpr (std::same_as<TArgument, NativeModuleCallContext>)
                  { return NativeModuleCallContext(context); }
                else
                  { static_assert(AlwaysFalse<TNativeModule>, "Unsupported argument type"); }
              };

            if constexpr (Traits::IsMemberFunction)
              { CallWithArgumentResolution(static_cast<TNativeModule*>(context->m_voiceContext), &TNativeModule::DeinitializeVoice, ResolveArgument); }
            else
              { return CallWithArgumentResolution(&TNativeModule::DeinitializeVoice, ResolveArgument); }

            if constexpr (ShouldInstantiateClass)
              { delete static_cast<TNativeModule*>(context->m_voiceContext); }
          };

        return DeinitializeVoiceWrapper;
      }
    }
    else if constexpr (ShouldInstantiateClass)
    {
      auto DeinitializeVoiceWrapper =
        [](const NativeModuleContext* context) -> void
          { delete static_cast<TNativeModule*>(context->m_voiceContext); };

      return DeinitializeVoiceWrapper;
    }
    else
      { return nullptr; }
  }

  template<typename TNativeModule>
  NativeModuleSetVoiceActive BuildNativeModuleSetVoiceActive()
  {
    if constexpr (requires { &TNativeModule::SetVoiceActive; })
    {
      using Traits = FunctionTraits<decltype(&TNativeModule::SetVoiceActive)>;
      if constexpr (!Traits::IsFunction)
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'SetVoiceActive' is not a function");
        return nullptr;
      }
      else if constexpr (!Traits::template Returns<void>())
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'SetVoiceActive' must return void");
        return nullptr;
      }
      else if constexpr (!Traits::template HasZeroOrOneArgumentsOfType<NativeModuleCallContext>())
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'SetVoiceActive' should have at most one 'context' argument");
        return nullptr;
      }
      else if constexpr (!Traits::template HasOneArgumentOfType<bool>())
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'SetVoiceActive' should have a 'voiceActive' argument");
        return nullptr;
      }
      else
      {
        auto SetVoiceActiveWrapper =
          [](const NativeModuleContext* context, bool voiceActive) -> void
          {
            auto ResolveArgument =
              [&]<typename TArgument>()
              {
                if constexpr (std::same_as<TArgument, NativeModuleCallContext>)
                  { return NativeModuleCallContext(context); }
                else if constexpr (std::same_as<TArgument, bool>)
                  { return voiceActive; }
                else
                  { static_assert(AlwaysFalse<TNativeModule>, "Unsupported argument type"); }
              };

            if constexpr (Traits::IsMemberFunction)
              { CallWithArgumentResolution(static_cast<TNativeModule*>(context->m_voiceContext), &TNativeModule::SetVoiceActive, ResolveArgument); }
            else
              { return CallWithArgumentResolution(&TNativeModule::SetVoiceActive, ResolveArgument); }
          };

        return SetVoiceActiveWrapper;
      }
    }
    else
      { return nullptr; }
  }

  template<typename TNativeModule>
  NativeModuleInvoke BuildNativeModuleInvoke()
  {
    if constexpr (requires { &TNativeModule::Invoke; })
    {
      using Traits = FunctionTraits<decltype(&TNativeModule::Invoke)>;
      if constexpr (!Traits::IsFunction)
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'Invoke' is not a function");
        return nullptr;
      }
      else if constexpr (!Traits::template Returns<void>())
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'Invoke' must return void");
        return nullptr;
      }
      else if constexpr (!Traits::template HasZeroOrOneArgumentsOfType<NativeModuleCallContext>())
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'Invoke' should have at most one 'context' argument");
        return nullptr;
      }
      else if constexpr (!Traits::template HasZeroOrOneArgumentsOfType<StackAllocator&>())
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'Invoke' should have at most one 'scratchMemoryAllocator' argument");
        return nullptr;
      }
      else
      {
        auto InvokeWrapper =
          [](const NativeModuleContext* context, const NativeModuleArguments* arguments, void* scratchMemory, size_t scratchMemorySize) -> void
          {
            StackAllocator scratchMemoryAllocator(Span(static_cast<u8*>(scratchMemory), scratchMemorySize));

            auto ResolveArgument =
              [&]<typename TArgument>() -> decltype(auto)
              {
                if constexpr (std::same_as<TArgument, NativeModuleCallContext>)
                  { return NativeModuleCallContext(context); }
                else if constexpr (std::derived_from<TArgument, NativeModuleArgumentBase>)
                  { return ResolveNativeModuleArgument<TNativeModule, TArgument>(arguments); }
                else if constexpr (std::same_as<TArgument, StackAllocator&>)
                  { return scratchMemoryAllocator; }
                else
                  { static_assert(AlwaysFalse<TNativeModule>, "Unsupported argument type"); }
              };

            // Make sure this is returned by reference
            static_assert(std::same_as<decltype(ResolveArgument.operator()<StackAllocator&>()), StackAllocator&>);

            if constexpr (Traits::IsMemberFunction)
              { CallWithArgumentResolution(static_cast<TNativeModule*>(context->m_voiceContext), &TNativeModule::Invoke, ResolveArgument); }
            else
              { CallWithArgumentResolution(&TNativeModule::Invoke, ResolveArgument); }
          };

        return InvokeWrapper;
      }
    }
    else
      { return nullptr; }
  }

  template<typename TNativeModule>
  NativeModuleInvokeCompileTime BuildNativeModuleInvokeCompileTime()
  {
    if constexpr (requires { &TNativeModule::InvokeCompileTime; })
    {
      using Traits = FunctionTraits<decltype(&TNativeModule::InvokeCompileTime)>;
      if constexpr (!Traits::IsFunction)
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'InvokeCompileTime' is not a function");
        return nullptr;
      }
      else if constexpr (!Traits::template Returns<void>())
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'InvokeCompileTime' must return void");
        return nullptr;
      }
      else if constexpr (!Traits::template HasZeroOrOneArgumentsOfType<NativeModuleCallContext>())
      {
        static_assert(AlwaysFalse<TNativeModule>, "Native module 'InvokeCompileTime' should have at most one 'context' argument");
        return nullptr;
      }
      else
      {
        auto InvokeCompileTimeWrapper =
          [](const NativeModuleContext* context, const NativeModuleArguments* arguments) -> void
          {
            auto ResolveArgument =
              [&]<typename TArgument>()
              {
                if constexpr (std::same_as<TArgument, NativeModuleCallContext>)
                  { return NativeModuleCallContext(context); }
                else if constexpr (std::derived_from<TArgument, NativeModuleArgumentBase>)
                  { return ResolveNativeModuleArgument<TNativeModule, TArgument>(arguments); }
                else
                  { static_assert(AlwaysFalse<TNativeModule>, "Unsupported argument type"); }
              };

            if constexpr (Traits::IsMemberFunction)
              { CallWithArgumentResolution(static_cast<TNativeModule*>(context->m_voiceContext), &TNativeModule::InvokeCompileTime, ResolveArgument); }
            else
              { CallWithArgumentResolution(&TNativeModule::InvokeCompileTime, ResolveArgument); }
          };

        return InvokeCompileTimeWrapper;
      }
    }
    else
      { return nullptr; }
  }

  export
  {
    // !!! write doc
    template<typename TNativeModule>
    NativeModule DeclareNativeModule()
    {
      NativeModule nativeModule = {};

      Guid id = GetNativeModuleId<TNativeModule>();
      Span(nativeModule.m_id).CopyElementsFrom(id.Bytes());

      nativeModule.m_signature = BuildNativeModuleSignature<TNativeModule>();
      nativeModule.m_hasSideEffects = GetNativeModuleHasSideEffects<TNativeModule>();
      nativeModule.m_alwaysRuntime = GetNativeModuleAlwaysRuntime<TNativeModule>();

      nativeModule.m_prepare = BuildNativeModulePrepare<TNativeModule>();
      nativeModule.m_initializeVoice = BuildNativeModuleInitializeVoice<TNativeModule>();
      nativeModule.m_deinitializeVoice = BuildNativeModuleDeinitializeVoice<TNativeModule>();
      nativeModule.m_setVoiceActive = BuildNativeModuleSetVoiceActive<TNativeModule>();
      nativeModule.m_invokeCompileTime = BuildNativeModuleInvokeCompileTime<TNativeModule>();
      nativeModule.m_invoke = BuildNativeModuleInvoke<TNativeModule>();

      return nativeModule;
    }
  }
}