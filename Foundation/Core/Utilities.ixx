export module Chord.Foundation:Core.Utilities;

import :Core.Types;

namespace Chord
{
  export
  {
    template<typename T>
    constexpr usz BitSize = sizeof(T) * std::numeric_limits<u8>::digits;

    enum class ToEnd_t { ToEnd };
    using enum ToEnd_t;

    enum class Zero_t { Zero };
    using enum Zero_t;
  }

  template<typename... TTypes>
  struct CommonTypeImplementation;

  template<typename TType>
  struct CommonTypeImplementation<TType>
    { using Type = TType; };

  template<typename TTypeA, typename TTypeB>
    requires requires (bool c, TTypeA a, TTypeB b) { c ? a : b; }
  struct CommonTypeImplementation<TTypeA, TTypeB>
  {
    using Type = std::remove_cvref_t<
      decltype(
        std::declval<bool>()
          ? std::declval<TTypeA>()
          : std::declval<TTypeB>())>;
  };

  template<typename TTypeA, typename... TTypeBs>
    requires requires (bool c, TTypeA a) { c ? a : std::declval<typename CommonTypeImplementation<TTypeBs...>::Type>(); }
  struct CommonTypeImplementation<TTypeA, TTypeBs...>
  {
    using Type = std::remove_cvref_t<
      decltype(
        std::declval<bool>()
          ? std::declval<TTypeA>()
          : std::declval<typename CommonTypeImplementation<TTypeBs...>::Type>())>;
  };

  export
  {
    template <typename... TTypes>
      requires requires { typename CommonTypeImplementation<TTypes...>::Type; }
    using CommonType = typename CommonTypeImplementation<TTypes...>::Type;

    template<std::integral... TTypes> constexpr bool AnyUnsigned = (std::is_unsigned_v<TTypes> || ...);
    template<std::integral... TTypes> constexpr bool AnySigned = (std::is_signed_v<TTypes> || ...);

    template<std::integral... TTypes>
    using CommonIntPreferSigned = std::conditional_t<AnySigned<TTypes...>, std::make_signed_t<CommonType<TTypes...>>, CommonType<TTypes...>>;

    template<std::integral... TTypes>
    using CommonIntPreferUnsigned = std::conditional_t<AnyUnsigned<TTypes...>, std::make_unsigned_t<CommonType<TTypes...>>, CommonType<TTypes...>>;
  }
}