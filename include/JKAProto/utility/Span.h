#pragma once
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <numeric>
#include <string>
#include <string_view>
#include <type_traits>

#include "Traits.h"

namespace JKA::Utility {
    inline constexpr size_t SPAN_NPOS = std::numeric_limits<size_t>::max();

    template<typename T>
    class Span;

    template<typename U, typename W>
    Span<U>cast_span_const(const Span<W> & the_span) noexcept;

    template<typename U, typename W>
    Span<U> cast_span(const Span<W> & the_span) noexcept;

    // A non-owning view of size() T objects
    template<typename T>
    class Span {
    public:
        using element_type = T;
        using value_type = std::remove_cv_t<T>;
        using size_type = size_t;
        using difference_type = std::ptrdiff_t;
        using pointer = T *;
        using const_pointer = const T *;
        using reference = T &;
        using const_reference = const T &;
        using iterator = T *;

        constexpr Span() noexcept = default;
        constexpr Span(const Span & other) noexcept = default;
        constexpr Span(Span && other) noexcept = default;
        constexpr Span(pointer ptr, size_type ptr_size) noexcept : data_ptr(ptr), data_size(ptr_size) {}

        template<typename U = value_type, std::enable_if_t<
                std::conjunction_v<
                    std::is_const<element_type>,
                    std::is_same<U, value_type>,
                    std::is_same<U, typename std::char_traits<U>::char_type>
                >
            , int> = 0>
        explicit constexpr Span(std::basic_string_view<U> sv) : 
            data_ptr(sv.data()),
            data_size(sv.size())
        {
        }

        template<size_t Size, typename CharT = value_type, std::enable_if_t<
                std::conjunction_v<
                    std::is_const<element_type>,
                    std::is_same<CharT, value_type>
                >, int> = 0>
        explicit constexpr Span(const CharT (&arr)[Size]) :
            data_ptr(arr),
            data_size((Size > 0 && arr[Size - 1] == '\x00') ? (Size - 1) : Size)
        {
        }

        template<typename U = value_type, std::enable_if_t<
                std::conjunction_v<
                    std::is_same<U, value_type>,
                    std::is_same<U, typename std::char_traits<U>::char_type>
                >
            , int> = 0>
        explicit constexpr Span(std::basic_string<U> & str) : 
            data_ptr(str.data()),
            data_size(str.size())
        {
        }

        template<typename U = value_type, std::enable_if_t<
                std::conjunction_v<
                    std::is_const<element_type>,
                    std::is_same<U, value_type>,
                    std::is_same<U, typename std::char_traits<U>::char_type>
                >
            , int> = 0>
        explicit constexpr Span(const std::basic_string<U> & str) : 
            data_ptr(str.data()),
            data_size(str.size())
        {
        }

        template<typename U>
        Span(Span<U> other) noexcept : Span(cast_span<element_type>(other)) {}

        constexpr Span & operator=(Span other) noexcept
        {
            data_ptr = other.data_ptr;
            data_size = other.data_size;
            return *this;
        }

        template<typename U>
        [[nodiscard]] U *cast_data_const() const noexcept
        {
            static_assert(can_alias_v<value_type, U>,
                "Reading type T object through the U pointer violates "
                "the strict aliasing rule. "
                "See https://en.cppreference.com/w/cpp/language/reinterpret_cast"
            );

            return reinterpret_cast<U*>(const_cast<value_type *>(data()));
        }

        template<typename U>
        [[nodiscard]] U *cast_data() const noexcept
        {
            static_assert(!std::is_const_v<element_type> || std::is_const_v<U>,
                          "Could not cast away const, use cast_data_const() "
                          "if you really need it");
            return cast_data_const<U>();
        }


        [[nodiscard]] constexpr Span<const_pointer> to_const() const noexcept
        {
            return Span<const value_type>(const_cast<const_pointer>(data()));
        }

        constexpr operator Span<const_pointer>() const noexcept
        {
            return to_const();
        }

        template<typename U>
        [[nodiscard]] Span<U> to_span() const noexcept
        {
            static_assert(!std::is_const_v<element_type> || std::is_const_v<U>,
                          "Could not cast away const, use to_span_const() "
                          "if you really need it");
            return cast_span<U>(*this);
        }

        template<typename U>
        [[nodiscard]] Span<U> to_span_const() const noexcept
        {
            return cast_span_const<U>(*this);
        }

        [[nodiscard]] constexpr size_type size() const noexcept
        {
            return data_size;
        }

        [[nodiscard]] constexpr size_type size_bytes() const noexcept
        {
            return size() * sizeof(element_type);
        }

        [[nodiscard]] constexpr pointer data() const noexcept
        {
            return data_ptr;
        }

        [[nodiscard]] constexpr reference operator[](size_type idx) const noexcept
        {
            return data()[idx];
        }

        [[nodiscard]] constexpr iterator begin() const noexcept
        {
            return data();
        }

        [[nodiscard]] constexpr iterator end() const noexcept
        {
            return data() + size();
        }

        [[nodiscard]] constexpr Span first(size_type N) const noexcept
        {
            assert(N <= size());
            return Span(data() + N, size() - N);
        }

        [[nodiscard]] constexpr Span last(size_type N) const noexcept
        {
            assert(N <= size());
            return Span(data() + size() - N, size() - N);
        }

        [[nodiscard]] constexpr Span subspan(size_type offset, size_type count = SPAN_NPOS) const noexcept
        {
            return Span(data() + offset, std::min(count, size() - offset));
        }

        template<typename CharT = char>
        [[nodiscard]] std::basic_string_view<CharT> to_sv() const noexcept
        {
            return std::basic_string_view<CharT>{cast_data<const CharT>(), size_bytes()};
        }

    private:
        pointer data_ptr{};
        size_type data_size{};
    };

    template<typename U, typename W>
    Span<U> cast_span_const(const Span<W> & the_span) noexcept
    {
        Utility::CheckAliasing<W, U>();

        static_assert(sizeof(W) % sizeof(U) == 0,
                      "sizeof(W) must be divisable by sizeof(U)"
        );
        constexpr size_t Us_per_W = (sizeof(W) / sizeof(U));
        size_t new_size = the_span.size() * Us_per_W;
        return Span<U>(the_span.template cast_data_const<U>(), new_size);
    }

    template<typename U, typename W>
    Span<U> cast_span(const Span<W> & the_span) noexcept
    {
        static_assert(!std::is_const_v<W> || std::is_const_v<U>,
                      "Could not cast away const, use cast_span_const() "
                      "if you really need it");
        return cast_span_const<U, W>(the_span);
    }

    template<typename CharT, std::enable_if_t<
            std::is_same_v<CharT, typename std::char_traits<CharT>::char_type>
        , int> = 0>
    Span(std::basic_string_view<CharT> sv) -> Span<const CharT>;

    template<typename CharT, std::enable_if_t<
            std::is_same_v<CharT, typename std::char_traits<CharT>::char_type>
        , int> = 0>
    Span(std::basic_string<CharT> & str) -> Span<CharT>;

    template<typename CharT, std::enable_if_t<
            std::is_same_v<CharT, typename std::char_traits<CharT>::char_type>
        , int> = 0>
    Span(const std::basic_string<CharT> & str) -> Span<const CharT>;

    template<size_t Size, typename CharT>
    Span(const CharT (&arr)[Size]) -> Span<const CharT>;
}
