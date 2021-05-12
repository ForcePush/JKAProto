#pragma once
#include <cassert>
#include <cstddef>
#include <cstring>
#include <string>
#include <string_view>
#include <type_traits>

#include "Span.h"

namespace JKA::Utility {
    template<typename T, typename CharT>
    std::enable_if_t<
        std::conjunction_v<  // ALL
            std::disjunction<  // EITHER
                std::is_same<std::remove_cv_t<CharT>, char>,
                std::is_same<std::remove_cv_t<CharT>, signed char>,
                std::is_same<std::remove_cv_t<CharT>, unsigned char>,
                std::is_same<std::remove_cv_t<CharT>, std::byte>
            >,
            std::is_trivially_copyable<T>
        >,
        T
    > bit_reinterpret(const CharT *buf) noexcept(std::is_nothrow_default_constructible_v<T>)
    {
        assert(buf != nullptr);

        T tmp;
        std::memcpy(&tmp, buf, sizeof(T));
        return tmp;
    }

    template<typename T, typename CharT>
    T bit_reinterpret(const Utility::Span<CharT> & span) noexcept(std::is_nothrow_default_constructible_v<T>)
    {
        assert(span.size() >= sizeof(T));
        return bit_reinterpret<T, CharT>(span.data());
    }

    template<typename T, typename CharT>
    T bit_reinterpret(std::basic_string_view<CharT> sv) noexcept(std::is_nothrow_default_constructible_v<T>)
    {
        assert(sv.size() >= sizeof(T));
        return bit_reinterpret<T>(sv.data());
    }

    template<typename T, typename CharT>
    T bit_reinterpret(const std::basic_string<CharT> & str) noexcept(std::is_nothrow_default_constructible_v<T>)
    {
        assert(str.size() >= sizeof(T));
        return bit_reinterpret<T, CharT>(str.data());
    }

    template<typename T, typename CharT>
    std::enable_if_t<
        std::conjunction_v<  // ALL
            std::disjunction<  // EITHER
                std::is_same<std::remove_cv_t<CharT>, char>,
                std::is_same<std::remove_cv_t<CharT>, signed char>,
                std::is_same<std::remove_cv_t<CharT>, unsigned char>,
                std::is_same<std::remove_cv_t<CharT>, std::byte>
            >,
            std::is_trivially_copyable<T>
        >
    > bit_write(const T & from, CharT *to) noexcept
    {
        assert(to != nullptr);
        std::memcpy(to, std::addressof(from), sizeof(T));
    }

    template<typename T, typename CharT>
    void bit_write(const T & from, Utility::Span<CharT> to) noexcept
    {
        assert(to.size() >= sizeof(T));
        bit_write<T, CharT>(from, to.data());
    }

    template<typename T, typename CharT>
    void bit_write(const T & from, std::basic_string<CharT> & to) noexcept
    {
        assert(to.size() >= sizeof(T));
        bit_write<T, CharT>(from, to.data());
    }

    template <class To, class From>
    typename std::enable_if_t<
        sizeof(To) == sizeof(From) &&
        std::is_trivially_copyable_v<From> &&
        std::is_trivially_copyable_v<To>,
        To>
    bit_cast(const From & src) noexcept
    {
        To dst;
        std::memcpy(&dst, &src, sizeof(To));
        return dst;
    }
}
