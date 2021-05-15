#pragma once
#include <array>
#include <cinttypes>
#include <map>
#include <string>
#include <string_view>

#include "CEntity.h"
#include "CommandExecutor.h"
#include "jka/JKADefs.h"
#include "JKAInfo.h"
#include "Snapshot.h"
#include "SharedDefs.h"
#include "jka/JKAEnums.h"
#include "jka/JKAStructs.h"
#include "jka/Usercmd.h"

namespace JKA {
    // A dataclass that represents the client's view on a server's world
    struct ClientGameState {
        ClientGameState() = default;
        ClientGameState(const ClientGameState &) = delete;  // Too heavy
        ClientGameState(ClientGameState &&) = default;
        ClientGameState & operator=(const ClientGameState &) = delete;  // Too heavy
        ClientGameState & operator=(ClientGameState &&) = default;

        // Warning: very heavy function
        void reset()
        {
            // Gamestate-related data
            info.clear();

            configStrings.clear();
            configStringsInfo.clear();

            // Entities
            entityBaselines.fill({});
            parseEntities.fill({});
            currentEntities.fill({});

            // Snapshots
            clientNum = 0;
            curSnap = {};
            snapshots.fill({});
            parseEntitiesNum = 0;
            serverTime = 0;

            // RMG 
            compressedHeightmap.fill({});
        }

        // Gamestate-related data
        JKAInfo info{};

        std::map<size_t, std::string, std::less<>> configStrings{};
        std::map<size_t, JKAInfo, std::less<>> configStringsInfo{};

        std::string_view getConfigString(size_t index) const &
        {
            auto it = configStrings.find(index);
            if (it != configStrings.end()) {
                return it->second;
            } else {
                return "";
            }
        }
        
        void setConfigString(size_t index, std::string_view newValue)
        {
            configStrings[index] = newValue;
            configStringsInfo[index] = JKAInfo::fromInfostring(newValue);
        }
        
        void clearConfigstrings()
        {
            configStrings.clear();
            configStringsInfo.clear();
        }

        JKAInfo *getConfigStringInfo(size_t index) &
        {
            auto it = configStringsInfo.find(index);
            if (it != configStringsInfo.end()) {
                return std::addressof(it->second);
            } else {
                return nullptr;
            }
        }

        // Entities
        std::array<entityState_t, MAX_GENTITIES> entityBaselines{};
        std::array<entityState_t, MAX_PARSE_ENTITIES> parseEntities{};
        std::array<CEntity, MAX_GENTITIES> currentEntities{};

        // Snapshots
        int32_t clientNum = 0;
        Snapshot curSnap{};
        std::array<Snapshot, PACKET_BACKUP> snapshots{};
        int32_t parseEntitiesNum = 0;  // In the current snapshot
        int32_t serverTime = 0;

        // Usercmds
        usercmd_t lastUsercmd{};

        // RMG (currently unused)
        std::array<ByteType, MAX_HEIGHTMAP_SIZE> compressedHeightmap{};
    };
}
