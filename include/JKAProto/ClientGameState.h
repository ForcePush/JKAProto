#pragma once
#include <array>
#include <cinttypes>
#include <map>
#include <string>

#include "CEntity.h"
#include "CommandExecutor.h"
#include "jka/JKADefs.h"
#include "JKAInfo.h"
#include "SharedDefs.h"
#include "jka/JKAEnums.h"
#include "jka/JKAStructs.h"

namespace JKA {
    // A dataclass that represents the client's view on a server's world
    struct ClientGameState {
        ClientGameState() = default;
        ClientGameState(const ClientGameState &) = delete;  // Too heavy
        ClientGameState(ClientGameState &&) noexcept = default;
        ClientGameState & operator=(const ClientGameState &) = delete;  // Too heavy
        ClientGameState & operator=(ClientGameState &&) noexcept = default;

        // Gamestate-related data
        JKAInfo info{};

        std::map<size_t, std::string, std::less<>> configStrings{};
        std::map<size_t, JKAInfo, std::less<>> configStringsInfo{};
        std::ostringstream bigInfoStringBuffer{};  // For bcs0/bcs1/bcs2 server commands

        // Entities
        std::array<entityState_t, MAX_GENTITIES> entityBaselines{};
        std::array<entityState_t, MAX_PARSE_ENTITIES> parseEntities{};
        std::array<CEntity, MAX_GENTITIES> currentEntities{};

        // Snapshots
        int32_t clientNum = 0;
        clSnapshot_t snap{};
        std::array<clSnapshot_t, PACKET_BACKUP> snapshots{};
        int32_t parseEntitiesNum = 0;  // In the current snapshot
        int32_t serverTime = 0;

        // RMG (currently unused)
        std::array<ByteType, MAX_HEIGHTMAP_SIZE> compressedHeightmap{};
    };
}
