#pragma once
#include "jka/JKAStructs.h"
#include "SharedDefs.h"

namespace JKA {
    struct Snapshot {
        clSnapshot_t snap{};
        TimePoint arriveTime{};
    };
}
