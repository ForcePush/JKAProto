#pragma once
#include <chrono>
#include <cstdint>
#include <string>

namespace JKA {
    using ByteType = char;
    static_assert(sizeof(ByteType) == sizeof(char));

    using Clock = std::chrono::system_clock;
    using TimePoint = Clock::time_point;

    using PrecClock = std::chrono::high_resolution_clock;
    using PrecTimePoint = PrecClock::time_point;

    constexpr inline std::int32_t PROTOCOL_VERSION = 26;
    const inline std::string PROTOCOL_VERSION_STRING = std::to_string(PROTOCOL_VERSION);
}

#if __has_cpp_attribute(likely)
#define JKA_LIKELY [[likely]]
#else
#define JKA_LIKELY
#endif

#if __has_cpp_attribute(unlikely)
#define JKA_UNLIKELY [[unlikely]]
#else
#define JKA_UNLIKELY
#endif

