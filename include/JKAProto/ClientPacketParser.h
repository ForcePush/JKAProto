#pragma once
#include "ClientConnection.h"
#include "ClientGameState.h"
#include "ClientEventsListener.h"
#include "CommandParser.h"
#include "packets/ConnlessPacket.h"
#include "protocol/ClientPacket.h"
#include "protocol/Netchan.h"
#include "protocol/PacketEncoder.h"
#include "ReliableCommandsStore.h"

namespace JKA {
    class ClientPacketParser {
    public:
        ClientPacketParser(ClientEventsListener & evListener,
                           ReliableCommandsStore & reliableCommands,
                           ClientConnection & connection,
                           ClientGameState & gameState);

        // Nothing, for consistency only
        void handleOobPacketFromServer(const Packets::ConnlessPacket & packet);
        // Nothing, for consistency only
        void handleOobPacketFromClient(const Packets::ConnlessPacket & packet);

        void handleConnfullPacketFromClient(Protocol::ClientPacket & packet,
                                            TimePoint arriveTime);

    private:
        // Client binary commands
        void parseClientCommand(Protocol::CompressedMessage & message);
        void parseUserMove(Protocol::CompressedMessage & message, bool delta);

        void executeUserCmd(usercmd_t & cmd);

        // Client reliable commands
        void onClientReliableCommand(int32_t sequence, std::string && command);

        ClientEventsListener & evListener;
        ReliableCommandsStore & reliableCommands;
        ClientConnection & connection;
        ClientGameState & gameState;
    };
}
