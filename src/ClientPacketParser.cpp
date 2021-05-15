#include <JKAProto/ClientPacketParser.h>

namespace JKA {
    JKA::ClientPacketParser::ClientPacketParser(ClientEventsListener & evListener,
                                                ReliableCommandsStore & reliableCommands,
                                                ClientConnection & connection,
                                                ClientGameState & gameState) :
        evListener(evListener),
        reliableCommands(reliableCommands),
        connection(connection),
        gameState(gameState)
    {
    }

    void ClientPacketParser::handleOobPacketFromServer(const Packets::ConnlessPacket & packet)
    {
        static_cast<void>(packet);
    }

    void ClientPacketParser::handleOobPacketFromClient(const Packets::ConnlessPacket & packet)
    {
        static_cast<void>(packet);
    }

    void ClientPacketParser::handleConnfullPacketFromClient(Protocol::ClientPacket & packet,
                                                            TimePoint arriveTime)
    {
        if (connection.state < CA_CONNECTED) {
            // TODO: wait for other packets
            return;
        }

        connection.lastClientPacketTime = arriveTime;
        connection.messageAcknowledge = packet.messageAcknowledge;

        int32_t c = 0;
        auto & msg = packet.message;

        while (true) {
            c = msg.readByte();

            if (c == clc_EOF) {
                break;
            }

            if (c != clc_clientCommand) {
                break;
            }

            parseClientCommand(msg);
        }

        if (c == clc_move) {
            parseUserMove(msg, true);
        } else if (c == clc_moveNoDelta) {
            parseUserMove(msg, false);
        } else if (c != clc_EOF) {
            // TODO: invalid command byte, warning?
        }
    }

    void ClientPacketParser::parseClientCommand(Protocol::CompressedMessage & message)
    {
        int32_t seq = message.readLong();
        std::string s = message.readString();

        if (seq > connection.reliableSequence) {
            connection.reliableSequence = seq;
            onClientReliableCommand(seq, std::move(s));
        }
    }

    void ClientPacketParser::parseUserMove(Protocol::CompressedMessage & message, bool delta)
    {
        int32_t cmdCount = message.readByte();

        if (cmdCount < 1 || cmdCount > MAX_PACKET_USERCMDS) {
            return;
        }

        usercmd_t nullcmd{};
        std::array<usercmd_t, MAX_PACKET_USERCMDS> cmds{};
        usercmd_t *cmd{}, *oldcmd{};

        int32_t key = connection.checksumFeed;
        key ^= connection.messageAcknowledge;
        key ^= Com_HashKey(reliableCommands.reliableCommand(connection.reliableAcknowledge), 32);

        oldcmd = &nullcmd;
        for (int32_t i = 0; i < cmdCount; i++) {
            cmd = &cmds[i];
            message.readDeltaUsercmdKey(key, oldcmd, cmd);
            executeUserCmd(*cmd);
            oldcmd = cmd;
        }
    }

    void ClientPacketParser::executeUserCmd(usercmd_t & cmd)
    {
        if (cmd.serverTime < gameState.lastUsercmd.serverTime) {
            return;
        }
        cmd.arriveTime = connection.lastClientPacketTime;
        evListener.onNewUsercmd(cmd);
        gameState.lastUsercmd = cmd;
    }

    void ClientPacketParser::onClientReliableCommand(int32_t sequence, std::string && command)
    {
        evListener.onClientReliableCommand(sequence, CommandParser::parseCommand(command));
        reliableCommands.reliableCommand(sequence) = std::move(command);
    }
}
