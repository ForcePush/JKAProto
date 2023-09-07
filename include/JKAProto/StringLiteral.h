#pragma once
#include <algorithm>
#include <string_view>

namespace JKA
{
    template <size_t N>
    struct StringLiteral
    {
        constexpr StringLiteral(const char(&str)[N])
        {
            std::copy_n(str, N, value_array);
        }

        static constexpr size_t size()
        {
            return N;
        }

        constexpr std::string_view value() const
        {
            return std::string_view(value_array, size());
        }

        constexpr operator std::string_view() const
        {
            return value();
        }

        constexpr auto operator<=>(const StringLiteral&) const = default;
        constexpr bool operator==(const StringLiteral&) const = default;

        char value_array[N];
    };
}
