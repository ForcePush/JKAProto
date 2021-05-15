#pragma once
#include <cinttypes>

#include "jka/JKAEnums.h"
#include "jka/JKAStructs.h"
#include "SharedDefs.h"

namespace JKA {
    // A dataclass that represents the client's connection to the server
    struct ClientConnection {
        void reset(int32_t newChallenge = 0)
        {
            *this = ClientConnection();
            challenge = newChallenge;
        }

        // Connection-related data
        int32_t challenge = 0;
        uint16_t qport = 0;  // TODO: must be the same as in gameState.info

        TimePoint lastServerPacketTime{};
        TimePoint lastClientPacketTime{};
        TimePoint connectTime{};

        connstate_t state = CA_DISCONNECTED;
        TimePoint lastStateChangeTime{};

        // Protocol-related data
        int32_t serverId = 0;

        int32_t reliableSequence = 0;
        int32_t reliableAcknowledge = 0;
        int32_t messageAcknowledge = 0;

        int32_t serverMessageSequence = 0;
        int32_t checksumFeed = 0;

        int32_t serverCommandSequence = 0;
        int32_t lastExecutedServerCommand = 0;
    };
}
