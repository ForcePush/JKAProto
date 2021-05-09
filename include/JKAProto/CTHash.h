#pragma once
#include <cstdint>
#include <string_view>

// Compile-time hash function for string literals.
// Can be used in switch statements, e.g.
// switch(ct_hash(runtime_string)) { case ct_hash("compiletime_string_literal"): ...
// Beware of collisions.

namespace JKA {
    // FNV-1a
    constexpr uint64_t ct_hash(std::string_view str) noexcept
    {
        uint64_t val = 14695981039346656037ull;
        for (const auto & c : str) {
            val ^= static_cast<uint64_t>(c);
            val *= 1099511628211ull;
        }
        return val;
    }
}
