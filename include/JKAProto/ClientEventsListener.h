#pragma once
#include "jka/JKAEnums.h"
#include "CEntity.h"
#include "CommandParser.h"

namespace JKA {
    struct ClientEventsListener {
        ClientEventsListener() = default;
        ClientEventsListener(const ClientEventsListener &) = delete;
        ClientEventsListener(ClientEventsListener &&) = delete;
        virtual ~ClientEventsListener() = default;

        // Connection
        virtual void onConnectionStateChanged([[maybe_unused]] connstate_t from,
                                              [[maybe_unused]] connstate_t to) {}

        // Game state
        virtual void onEntityRemoved([[maybe_unused]] CEntity & curEnt) {}
        virtual void onEntityAdded([[maybe_unused]] CEntity & curEnt,
                                   [[maybe_unused]] JKA::entityState_t & newState) {}
        virtual void onEntityChanged([[maybe_unused]] CEntity & curEnt,
                                     [[maybe_unused]] JKA::entityState_t & newState) {}

        virtual void onConfigstringChanged([[maybe_unused]] size_t index,
                                           [[maybe_unused]] std::string_view oldValue,
                                           [[maybe_unused]] std::string_view newValue) {}
        virtual void onSystemInfoChanged([[maybe_unused]] const JKAInfo & newSysteminfo) {}
        virtual void onClientInfoChanged([[maybe_unused]] const JKAInfo & oldInfo,
                                         [[maybe_unused]] const JKAInfo & newInfo) {}

        virtual void onNewGameDir([[maybe_unused]] std::string_view newGameDir) {}

        virtual void onNewUsercmd([[maybe_unused]] const usercmd_t & cmd) {}

        // Reliable commands
        virtual void onServerReliableCommand([[maybe_unused]] const CommandParser::Command & cmd) {}
        virtual void onClientReliableCommand([[maybe_unused]] int32_t sequence,
                                             [[maybe_unused]] const CommandParser::Command & cmd) {}
    };
}
