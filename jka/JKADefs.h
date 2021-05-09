#pragma once
#include <array>

#include "JKAConstants.h"
#include "JKAEnums.h"
#include "JKAStructs.h"
#include "JKADefsNet.h"
#include "JKAFunctions.h"

namespace JKA
{
#define CONLESS_PACKETS_LIST_ENTRY(type, cls_name, str, sep) ConnlessPacketDef{ type, #cls_name, str, sep },
    inline constexpr std::array CONNLESS_PACKETS {
        #include "../data/ConnlessPacketsList.inc"
    };
#undef CONLESS_PACKETS_LIST_ENTRY

    // All valid JKA packet separators, excluding the empty one
    inline constexpr std::string_view CONNLESS_SEPARATORS = " \n\\";

    // Err... There are no array designators ( {[index1] = value1, [index2] = value2} ) in C++,
    // so we need to check that our connless packets array is valid
#define CONLESS_PACKETS_LIST_ENTRY(type_, cls_name, str, sep) static_assert(CONNLESS_PACKETS[type_].type == type_);
    #include "../data/ConnlessPacketsList.inc"
#undef CONLESS_PACKETS_LIST_ENTRY

    static_assert(std::size(CONNLESS_PACKETS) == CLS__MAX);
}
