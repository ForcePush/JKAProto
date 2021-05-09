#pragma once
#include <string_view>
#include <cinttypes>
#include "JKAStructs.h"

namespace JKA
{
    inline void VectorCopy(const vec3_t & from, vec3_t & to)
    {
        to[0] = from[0];
        to[1] = from[1];
        to[2] = from[2];
    }

    inline void VectorAdd(const vec3_t & from, vec3_t to)
    {
        to[0] += from[0];
        to[1] += from[1];
        to[2] += from[2];
    }

    int32_t Com_HashKey(std::string_view string, size_t maxLen);

    void BG_PlayerStateToEntityState(playerState_t &ps, entityState_t &s);

    inline int32_t ANGLE2SHORT(float angle)
    {
        return ((int32_t)((angle) * 65536 / 360) & 65535);
    }

    inline float SHORT2ANGLE(int32_t x)
    {
        return ((x) * (360.0f / 65536));
    }
}
