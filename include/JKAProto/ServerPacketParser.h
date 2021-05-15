#pragma once
#include <sstream>

#include "ClientGameState.h"
#include "ClientConnection.h"
#include "CommandExecutor.h"
#include "ClientEventsListener.h"
#include "ReliableCommandsStore.h"
#include "packets/ConnlessPacket.h"
#include "protocol/ServerPacket.h"
#include "protocol/Netchan.h"
#include "protocol/PacketEncoder.h"
#include "SharedDefs.h"

namespace JKA {
    struct ServerPacketParser {
        using Command = CommandParser::Command;

        ServerPacketParser(ClientEventsListener & evListener,
                           ReliableCommandsStore & reliableCommands,
                           ClientConnection & connection,
                           ClientGameState & gameState);

        // 'challengeResponse', 'connectResponse'
        void handleOobPacketFromServer(const Packets::ConnlessPacket & packet);
        // 'getChallenge', 'connect'; trusted
        void handleOobPacketFromClient(const Packets::ConnlessPacket & packet);

        void handleConnfullPacketFromServer(Protocol::ServerPacket & packet,
                                            TimePoint arriveTime);

        void connectSent(JKAInfo info);

    private:
        // TODO: thread_local?
        static Huffman huffman;

        // Connected to a new server
        void reset(int32_t newChallenge = 0);

        void setConnectionState(connstate_t newState);

        void setConfigstring(size_t index, std::string_view newValue);
        void clearConfigstrings();
        void onSystemInfoChanged(const JKAInfo & newSysteminfo);

        void setClientInfo(JKAInfo newInfo);

        // Server binary commands
        void parseServerCommand(Protocol::CompressedMessage & message);
        void parseGamestate(Protocol::CompressedMessage & message);
        void parseSnapshot(Protocol::CompressedMessage & message);
        void parseSetGame(Protocol::CompressedMessage & message);
        void parseDownload(Protocol::CompressedMessage & message);

        // Entities
        void parseRMG(Protocol::CompressedMessage & message);
        void parseAutomapSymbols(Protocol::CompressedMessage & message);
        void parsePacketEntities(Protocol::CompressedMessage & message,
                                 clSnapshot_t *oldframe, clSnapshot_t *newframe);
        void parseDeltaEntity(Protocol::CompressedMessage & message,
                              clSnapshot_t *frame, int32_t newnum,
                              entityState_t *old, bool unchanged);

        CEntity & getEntity(size_t index) &;
        const CEntity & getEntity(size_t index) const &;

        entityState_t & parsedEntity(size_t parseNum, size_t index = 0) &;
        const JKA::entityState_t & parsedEntity(size_t parseNum, size_t index = 0) const &;

        void onEntityRemoved(CEntity & curEnt);
        void onEntityAdded(CEntity & curEnt, JKA::entityState_t & newState);
        void onEntityChanged(CEntity & curEnt, JKA::entityState_t & newState);

        // Server reliable commands
        void initCommands();

        void onServerReliableCommand(std::string_view command);

        void startBigInfoString(int64_t csNum, std::string_view str);  // bcs0
        void midBigInfoString(std::string_view str);                   // bcs1
        void endBigInfoString(std::string_view str);                   // bcs2
        std::string flushBigInfoString();

        void cmd_disconnect(const Command & cmd);
        void cmd_cs(const Command & cmd);
        void cmd_bcs(const Command & cmd);

        ClientEventsListener & evListener;
        ReliableCommandsStore & reliableCommands;
        ClientConnection & connection;
        ClientGameState & gameState;

        CommandExecutor executor{};

        std::ostringstream bigInfoStringBuffer{};  // For bcs0/bcs1/bcs2 server commands
    };
}
