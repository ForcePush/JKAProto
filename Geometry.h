#pragma once
#include <array>
#include <cassert>
#include <cmath>
#include <type_traits>

#include "jka/JKAConstants.h"
#include "jka/JKAStructs.h"

namespace detail {
    template<typename T, size_t Size>
    struct VecBase {
        static_assert(std::is_arithmetic_v<T>, "T must be an arithmetic type");

        template<typename... ArgsRest>
        constexpr VecBase(const T & arg, ArgsRest && ...args) noexcept : coords({ arg, std::forward<ArgsRest>(args)... }) {}

        template<typename... ArgsRest>
        constexpr VecBase(T && arg, ArgsRest && ...args) noexcept : coords({ std::move(arg), std::forward<ArgsRest>(args)... }) {}

        template<typename U>
        constexpr VecBase(const U(&arr)[Size]) noexcept
        {
            for (size_t i = 0; i < Size; i++) {
                this->coords[i] = arr[i];
            }
        }

        constexpr VecBase(const VecBase &) noexcept = default;
        constexpr VecBase(VecBase &&) noexcept = default;

        ~VecBase() = default;

    protected:
        std::array<T, Size> coords;
    };
}

template<typename T, size_t Size>
struct Vec : public detail::VecBase<T, Size> {
    using detail::VecBase<T, Size>::VecBase;

    constexpr static auto ALL_AXES = static_cast<size_t>(-1);

    constexpr T *toPtr() noexcept
    {
        return this->coords.data();
    }

    constexpr const T *toPtr() const noexcept
    {
        return this->coords.data();
    }

    constexpr const T & operator[](size_t idx) const noexcept
    {
        assert(idx < Size);
        return this->coords[idx];
    }

    constexpr T & operator[](size_t idx) noexcept
    {
        assert(idx < Size);
        return this->coords[idx];
    }

    // Geometry
    
    template<typename OtherT, size_t OtherSize>
    constexpr auto dotProduct(const Vec<OtherT, OtherSize> & other, size_t axisCount = ALL_AXES) const noexcept
    {
        using CommonT = decltype(std::declval<T>() * std::declval<OtherT>() +
                                 std::declval<T>() * std::declval<OtherT>());
        CommonT res{};

        axisCount = std::min(axisCount, Size);
        axisCount = std::min(axisCount, OtherSize);
        // TODO: https://en.wikipedia.org/wiki/Kahan_summation_algorithm ?
        for (size_t i = 0; i < axisCount; i++) {
            res += this->coords[i] * other.coords[i];
        }

        return res;
    }

    constexpr auto length(size_t axisCount = ALL_AXES) const noexcept
    {
        return std::sqrt(dotProduct(*this, axisCount));
    }

    // Inplace basic ops

    template<typename U>
    constexpr Vec & operator+=(const Vec<U, Size> & other) noexcept
    {
        for (size_t i = 0; i < Size; i++) {
            this->coords[i] += other.coords[i];
        }
        return *this;
    }

    template<typename U, size_t USize>
    constexpr Vec & operator+=(const U(&arr)[USize]) noexcept
    {
        *this += Vec{ arr };
        return *this;
    }

    template<typename U>
    constexpr Vec & operator-=(const Vec<U, Size> & other) noexcept
    {
        for (size_t i = 0; i < Size; i++) {
            this->coords[i] -= other.coords[i];
        }
        return *this;
    }

    template<typename U>
    constexpr Vec & operator-=(const U(&arr)[Size]) noexcept
    {
        *this -= Vec{ arr };
        return *this;
    }

    template<typename U>
    constexpr Vec & operator*=(const U & scalar) noexcept
    {
        for (size_t i = 0; i < Size; i++) {
            this->coords[i] *= scalar;
        }
        return *this;
    }

    template<typename U>
    constexpr Vec & operator/=(const U & scalar) noexcept
    {
        for (size_t i = 0; i < Size; i++) {
            this->coords[i] /= scalar;
        }
        return *this;
    }

    // Copying basic ops

    template<typename U>
    constexpr Vec operator+(const Vec<U, Size> & other) const noexcept
    {
        Vec newVec{ *this };
        return newVec += other;
    }

    template<typename U>
    constexpr Vec operator+(const U(&arr)[Size]) const noexcept
    {
        return *this + Vec{ arr };
    }

    template<typename U>
    constexpr Vec operator-(const Vec<U, Size> & other) const noexcept
    {
        Vec newVec{ *this };
        return newVec -= other;
    }

    template<typename U>
    constexpr Vec operator-(const U(&arr)[Size]) const noexcept
    {
        return *this - Vec{ arr };
    }

    template<typename U>
    constexpr Vec operator*(const U & scalar) const noexcept
    {
        Vec newVec{ *this };
        return newVec *= scalar;
    }

    template<typename U>
    constexpr Vec operator/(const U & scalar) const noexcept
    {
        Vec newVec{ *this };
        return newVec /= scalar;
    }

    // Coords getters/setters

    // X coord
    template<size_t DummySize = Size>
    constexpr T & x() noexcept
    {
        static_assert(DummySize > 0);

        return this->coords[0];
    }

    template<size_t DummySize = Size>
    constexpr const T & x() const noexcept
    {
        static_assert(DummySize > 0);

        return this->coords[0];
    }

    template<typename TFrom, size_t DummySize = Size>
    constexpr void x(TFrom && newValue)
        noexcept(noexcept(std::declval<T&>() = std::forward<TFrom>(newValue)))
    {
        static_assert(DummySize > 0);

        x() = std::forward<TFrom>(newValue);
    }

    // Y coord
    template<size_t DummySize = Size>
    constexpr T & y() noexcept
    {
        static_assert(DummySize > 1);

        return this->coords[1];
    }

    template<size_t DummySize = Size>
    constexpr const T & y() const noexcept
    {
        static_assert(DummySize > 1);

        return this->coords[1];
    }

    template<typename TFrom, size_t DummySize = Size>
    constexpr void y(TFrom && newValue)
        noexcept(noexcept(std::declval<T&>() = std::forward<TFrom>(newValue)))
    {
        static_assert(DummySize > 1);

        y() = std::forward<TFrom>(newValue);
    }

    // Z coord
    template<size_t DummySize = Size>
    constexpr T & z() noexcept
    {
        static_assert(DummySize > 2);

        return this->coords[2];
    }

    template<size_t DummySize = Size>
    constexpr const T & z() const noexcept
    {
        static_assert(DummySize > 2);

        return this->coords[2];
    }

    template<typename TFrom, size_t DummySize = Size>
    constexpr void z(TFrom && newValue)
        noexcept(noexcept(std::declval<T&>() = std::forward<TFrom>(newValue)))
    {
        static_assert(DummySize > 2);

        z() = std::forward<TFrom>(newValue);
    }

    // W (fourth) coord
    template<size_t DummySize = Size>
    constexpr T & w() noexcept
    {
        static_assert(DummySize > 3);

        return this->coords[3];
    }

    template<size_t DummySize = Size>
    constexpr const T & w() const noexcept
    {
        static_assert(DummySize > 3);

        return this->coords[3];
    }

    template<typename TFrom, size_t DummySize = Size>
    constexpr void w(TFrom && newValue)
        noexcept(noexcept(std::declval<T&>() = std::forward<TFrom>(newValue)))
    {
        static_assert(DummySize > 3);

        w() = std::forward<TFrom>(newValue);
    }
};

// Array template deduction guide: Vec({1.0f, 2.0f, 3.0f}) -> Vec<float, 3>
template<typename T, size_t Size>
Vec(const T(&arr)[Size]) -> Vec<T, Size>;

// Initializer-list template deduction guide: Vec(1.0f, 2.0f, 3.0f) -> Vec<float, 3>
template<typename Arg, typename... ArgsRest>
Vec(Arg &&, ArgsRest &&...) -> Vec<
    std::enable_if_t<
        !std::is_array_v<std::remove_reference_t<Arg>>
        && (sizeof...(ArgsRest) > 0),
        Arg
    >,
    sizeof...(ArgsRest) + 1  // + 1 for Arg
>;

template<size_t Size>
struct JKAVec : public Vec<float, Size> {
    using Vec<float, Size>::Vec;

    template<size_t DummySize = Size>
    JKAVec(float value) : Vec<float, Size>(value)
    {
        static_assert(DummySize == 1);
    }
};

// Array template deduction guide: JKAVec({1.0f, 2.0f, 3.0f}) -> JKAVec<3>
template<typename T, size_t Size>
JKAVec(const T(&arr)[Size]) -> JKAVec<Size>;

// Initializer-list template deduction guide: JKAVec(1.0f, 2.0f, 3.0f) -> JKAVec<3>.
template<typename Arg, typename... ArgsRest, std::enable_if_t<
    !std::is_array_v<std::remove_reference_t<Arg>>
    && (sizeof...(ArgsRest) > 0),
    int> = 0>
JKAVec(Arg &&, ArgsRest &&...) -> JKAVec<sizeof...(ArgsRest) + 1>;

// One float argument template deduction guide: JKAVec(1.0f) -> JKAVec<1>
template<typename T, std::enable_if_t<std::is_same_v<std::remove_reference_t<T>, float>, int> = 0>
JKAVec(T && value) -> JKAVec<1>;

// JKA-like aliases
using Vec1 = JKAVec<1>;
using Vec2 = JKAVec<2>;
using Vec3 = JKAVec<3>;
using Vec4 = JKAVec<4>;

// e.g. viewangles
struct AnglesIngame : public Vec<float, 3> {
    using Vec<float, 3>::Vec;

    // Pitch
    float & pitch() noexcept
    {
        return this->coords[JKA::PITCH];
    }

    const float & pitch() const noexcept
    {
        return this->coords[JKA::PITCH];
    }

    template<typename TFrom>
    void pitch(TFrom && newValue) noexcept
    {
        pitch() = std::forward<TFrom>(newValue);
    }

    // Yaw
    float & yaw() noexcept
    {
        return this->coords[JKA::YAW];
    }

    const float & yaw() const noexcept
    {
        return this->coords[JKA::YAW];
    }

    template<typename TFrom>
    void yaw(TFrom && newValue) noexcept
    {
        yaw() = std::forward<TFrom>(newValue);
    }

    // Roll
    float & roll() noexcept
    {
        return this->coords[JKA::ROLL];
    }

    const float & roll() const noexcept
    {
        return this->coords[JKA::ROLL];
    }

    template<typename TFrom>
    void roll(TFrom && newValue) noexcept
    {
        roll() = std::forward<TFrom>(newValue);
    }
};
