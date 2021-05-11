#pragma once
#include <type_traits>

namespace JKA::Utility {
    template<typename From, typename To>
    struct can_alias : public std::integral_constant<bool,
        std::disjunction_v<  // EITHER
            // Can implicitly convert From* to To*, e.g. To is a public base of From
            std::is_convertible<std::remove_cv_t<From>*, std::remove_cv_t<To>*>,
            // Any type can be 'examined' through the byte array
            std::disjunction<  // EITHER
                std::is_same<std::remove_cv_t<To>, char>,
                std::is_same<std::remove_cv_t<To>, signed char>,
                std::is_same<std::remove_cv_t<To>, unsigned char>,
                std::is_same<std::remove_cv_t<To>, std::byte>
            >,
            // signed From -> unsigned From
            std::conjunction<  // BOTH
                std::is_signed<std::remove_cv_t<From>>,
                std::is_same<std::remove_cv_t<To>, std::make_unsigned_t<std::remove_cv_t<From>>>
            >,
            // unsigned From -> signed From
            std::conjunction<  // BOTH
                std::is_unsigned<std::remove_cv_t<From>>,
                std::is_same<std::remove_cv_t<To>, std::make_signed_t<std::remove_cv_t<From>>>
            >
        >
    > {};

    template<typename From, typename To>
    inline constexpr bool can_alias_v = can_alias<From, To>::value;

    template<typename From, typename To>
    inline constexpr void CheckAliasing() noexcept
    {
        static_assert(can_alias_v<From, To>,
                      "Reading type T object through the U pointer violates "
                      "the strict aliasing rule. "
                      "See https://en.cppreference.com/w/cpp/language/reinterpret_cast" );
    }
}
