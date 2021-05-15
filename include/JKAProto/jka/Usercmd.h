#pragma once

#include "JKAFunctions.h"
#include "JKAStructs.h"
#include "../SharedDefs.h"

namespace JKA {
    typedef struct usercmd_s {
        int32_t serverTime{};
        int32_t angles[3]{};
        int32_t buttons{};
        uint8_t weapon{};           // weapon 
        uint8_t forcesel{};
        uint8_t invensel{};
        uint8_t generic_cmd{};
        int8_t  forwardmove{}, rightmove{}, upmove{};

        TimePoint arriveTime{};

        inline void updateAngles(const vec3_t & viewangles, const decltype(angles) deltaAngles) noexcept
        {
            static_assert(std::size(decltype(viewangles){}) == std::size(decltype(angles){}));

            for (size_t i = 0; i < std::size(angles); i++) {
                angles[i] = ANGLE2SHORT(viewangles[i]) - deltaAngles[i];
            }
        }

        inline void writeAngles(vec3_t & viewangles) const noexcept
        {
            static_assert(std::size(std::remove_reference_t<decltype(viewangles)>{}) == std::size(decltype(angles){}));

            for (size_t i = 0; i < std::size(viewangles); i++) {
                viewangles[i] = SHORT2ANGLE(angles[i]);
            }
        }

        inline void clear()
        {
            *this = {};
        }

    } usercmd_t;
}
