#include <JKAProto/ServerPacketParser.h>
#include <type_traits>

#include <JKAProto/packets/AllConnlessPackets.h>

namespace JKA {
    Huffman ServerPacketParser::huffman{};

    ServerPacketParser::ServerPacketParser(ClientEventsListener & evListener,
                                           ReliableCommandsStore & reliableCommands,
                                           ClientConnection & connection,
                                           ClientGameState & gameState) :
        evListener(evListener),
        reliableCommands(reliableCommands),
        connection(connection),
        gameState(gameState)
    {
        initCommands();
    }

    void ServerPacketParser::handleOobPacketFromServer(const Packets::ConnlessPacket & packet)
    {
        ConnlessType packetType = packet.getType();
        switch (packetType) 	{
        case CLS_GETCHALLENGE_RESPONSE:
        {
            if (connection.state == CA_CONNECTING) {
                const auto & resp = static_cast<const Packets::GetchallengeResponse &>(packet);
                reset(std::stoi(resp.getChallenge()));

                setConnectionState(CA_CHALLENGING);
            }
            break;
        }
        case CLS_CONNECT_RESPONSE:
        {
            if (connection.state == CA_CHALLENGING) {
                setConnectionState(CA_CONNECTED);
            }
            break;
        }
        }
    }

    void ServerPacketParser::handleOobPacketFromClient(const Packets::ConnlessPacket & packet)
    {
        ConnlessType packetType = packet.getType();
        switch (packetType) 	{
        case JKA::CLS_GETCHALLENGE:
            setConnectionState(CA_CONNECTING);
            break;
        case JKA::CLS_CONNECT:
        {
            const auto & connPacket = static_cast<const Packets::Connect &>(packet);
            std::string clientInfoStr = huffman.decompress(connPacket.getData());
            auto clientInfoView = std::string_view(clientInfoStr);
            
            // Remove the first and the last characters (")
            if (clientInfoView.size() <= 2) {
                return;
            }

            clientInfoView = clientInfoView.substr(1, clientInfoView.size() - 2);
            auto clientInfo = JKAInfo::fromInfostring(clientInfoView);
            setClientInfo(std::move(clientInfo));
            break;
        }
        }
    }

    void ServerPacketParser::handleConnfullPacketFromServer(Protocol::ServerPacket & packet,
                                                            TimePoint arriveTime)
    {
        if (connection.state < CA_CONNECTED) {
            // TODO: wait for other packets
            return;
        }

        connection.reliableAcknowledge = packet.reliableAcknowledge;
        if (connection.reliableAcknowledge + MAX_RELIABLE_COMMANDS < connection.reliableSequence) {
            connection.reliableAcknowledge = connection.reliableSequence;
        }

        connection.lastServerPacketTime = arriveTime;
        connection.serverMessageSequence = packet.sequence;

        auto & message = packet.message;

        while (true) {
            if (message.readcount > message.cursize) {
                break;
            }

            uint8_t cmd = message.readByte();
            if (cmd == svc_EOF) {
                break;
            }

            switch (cmd) {
            default:
                // Illegible server message
                // TODO: throw an exception?
                break;
            case svc_nop:
                break;
            case svc_serverCommand:
                parseServerCommand(message);
                break;
            case svc_gamestate:
                parseGamestate(message);
                break;
            case svc_snapshot:
                parseSnapshot(message);
                break;
            case svc_setgame:
                parseSetGame(message);
                break;
            case svc_download:
                parseDownload(message);
                // TODO: downloads
                return;
            case svc_mapchange:
                // TODO: map change
                break;
            }
        }
    }

    void ServerPacketParser::reset(int32_t newChallenge)
    {
        reliableCommands.reset();
        gameState.reset();
        connection.reset(newChallenge);
    }

    void ServerPacketParser::setConnectionState(connstate_t newState)
    {
        evListener.onConnectionStateChanged(connection.state, newState);
        connection.state = newState;
    }

    void ServerPacketParser::setConfigstring(size_t index, std::string_view newValue)
    {
        evListener.onConfigstringChanged(index, gameState.getConfigString(index), newValue);
        gameState.setConfigString(index, newValue);
    }

    void ServerPacketParser::clearConfigstrings()
    {
        gameState.clearConfigstrings();
    }

    void ServerPacketParser::onSystemInfoChanged(const JKAInfo & newSysteminfo)
    {
        if (auto it = newSysteminfo.find("sv_serverid"); it != newSysteminfo.end()) {
            connection.serverId = std::stoi(it->second);
        }

        evListener.onSystemInfoChanged(newSysteminfo);
    }

    void ServerPacketParser::setClientInfo(JKAInfo newInfo)
    {
        evListener.onClientInfoChanged(gameState.info, newInfo);
        gameState.info = std::move(newInfo);

        connection.qport = static_cast<uint16_t>(std::stoi(gameState.info["qport"]));
    }

    void ServerPacketParser::parseServerCommand(Protocol::CompressedMessage & message)
    {
        int32_t seq = message.readLong();
        std::string command = message.readString();

        if (connection.serverCommandSequence >= seq) {
            return;
        }

        connection.serverCommandSequence = seq;
        connection.lastExecutedServerCommand = gameState.curSnap.snap.serverTime;
        reliableCommands.serverCommand(seq) = command;
        onServerReliableCommand(command);
    }

    void ServerPacketParser::parseGamestate(Protocol::CompressedMessage & message)
    {
        entityState_t nullstate{};
        entityState_t *entityPtr = nullptr;

        connection.serverCommandSequence = message.readLong();
        clearConfigstrings();

        while (true) {
            uint8_t cmd = message.readByte();

            if (cmd == svc_EOF) {
                break;
            }

            if (cmd == svc_configstring) {
                uint16_t index = message.readUShort();

                if (index >= MAX_CONFIGSTRINGS) {
                    // TODO: throw an exception?
                    return;
                }

                setConfigstring(index, message.readString(false, true));
            } else if (cmd == svc_baseline) {
                int32_t entityNum = message.readBits<GENTITYNUM_BITS>();
                if (entityNum < 0 || entityNum >= MAX_GENTITIES) {
                    // TODO: throw an exception?
                    return;
                }

                std::memset(&nullstate, 0, sizeof(nullstate));
                entityPtr = &gameState.entityBaselines[entityNum];
                message.readDeltaEntity(&nullstate, entityPtr, entityNum);
            } else {
                // Bad command byte
                // TODO: throw an exception?
                return;
            }
        }

        gameState.clientNum = message.readLong();
        connection.checksumFeed = message.readLong();
        parseRMG(message);

        const JKAInfo *newSysteminfo = gameState.getConfigStringInfo(CS_SYSTEMINFO);
        if (newSysteminfo != nullptr) {
            onSystemInfoChanged(*newSysteminfo);
        }
    }

    void ServerPacketParser::parseSnapshot(Protocol::CompressedMessage & message)
    {
        clSnapshot_t *old = nullptr;
        Snapshot newSnap = {};
        int32_t oldMessageNum = 0;

        newSnap.arriveTime = connection.lastServerPacketTime;
        newSnap.snap.serverCommandNum = connection.serverCommandSequence;
        newSnap.snap.serverTime = message.readLong();
        newSnap.snap.messageNum = connection.serverMessageSequence;

        uint8_t deltaNum = message.readByte();
        if (!deltaNum) {
            newSnap.snap.deltaNum = -1;
        } else {
            newSnap.snap.deltaNum = newSnap.snap.messageNum - deltaNum;
        }
        newSnap.snap.snapFlags = message.readByte();

        // If the frame is delta compressed from data that we
        // no longer have available, we must suck up the rest of
        // the frame, but not use it, then ask for a non-compressed
        // message 
        if (newSnap.snap.deltaNum <= 0) {
            newSnap.snap.valid = true; // uncompressed frame
            old = nullptr;
        } else {
            old = &gameState.snapshots[newSnap.snap.deltaNum & PACKET_MASK].snap;
            if (!old->valid) {
                // Delta from invalid frame
                // should never happen
            } else if (old->messageNum != newSnap.snap.deltaNum) {
                // The frame that the server did the delta from
                // is too old, so we can't reconstruct it properly.
            } else if (gameState.parseEntitiesNum - old->parseEntitiesNum > MAX_PARSE_ENTITIES - 128) {
                // Delta parseEntitiesNum too old
            } else {
                newSnap.snap.valid = true;  // valid delta parse
            }
        }

        // read areamask
        size_t len = message.readByte();
        if (len >= MAX_MAP_AREA_BYTES) {
            len = MAX_MAP_AREA_BYTES - 1;
        }
        message.readData(Utility::Span(newSnap.snap.areamask, len));

        // read playerinfo
        if (old) {
            message.readDeltaPlayerstate(&old->ps, &newSnap.snap.ps);
            if (newSnap.snap.ps.m_iVehicleNum) {  // this means we must have written our vehicle's ps too
                message.readDeltaPlayerstate(&old->vps, &newSnap.snap.vps, true);
            }
        } else {
            message.readDeltaPlayerstate(nullptr, &newSnap.snap.ps);
            if (newSnap.snap.ps.m_iVehicleNum) { //this means we must have written our vehicle's ps too
                message.readDeltaPlayerstate(nullptr, &newSnap.snap.vps, true);
            }
        }

        // read packet entities
        parsePacketEntities(message, old, &newSnap.snap);

        // Current player's entityState is sent over playerState only
        BG_PlayerStateToEntityState(newSnap.snap.ps, getEntity(newSnap.snap.ps.clientNum).state);
        getEntity(newSnap.snap.ps.clientNum).valid = true;

        // if not valid, dump the entire thing now that it has
        // been properly read
        if (!newSnap.snap.valid) {
            return;
        }

        // clear the valid flags of any snapshots between the last
        // received and this one, so if there was a dropped packet
        // it won't look like something valid to delta from next
        // time we wrap around in the buffer
        oldMessageNum = gameState.curSnap.snap.messageNum + 1;

        if (newSnap.snap.messageNum - oldMessageNum >= PACKET_BACKUP) {
            oldMessageNum = newSnap.snap.messageNum - (PACKET_BACKUP - 1);
        }
        for (; oldMessageNum < newSnap.snap.messageNum; oldMessageNum++) {
            gameState.snapshots[oldMessageNum & PACKET_MASK].snap.valid = false;
        }

        // copy to the current good spot
        gameState.curSnap = newSnap;
        gameState.curSnap.snap.ping = 999;
        // calculate ping time
        // TODO: no ping for now

        // save the frame off in the backup array for later delta comparisons
        gameState.snapshots[gameState.curSnap.snap.messageNum & PACKET_MASK] = gameState.curSnap;
        gameState.serverTime = gameState.curSnap.snap.serverTime;
    }

    void ServerPacketParser::parseSetGame(Protocol::CompressedMessage & message)
    {
        char newGameDir[MAX_QPATH + 1];
        size_t i = 0;

        while (i < MAX_QPATH) {
            uint8_t next = message.readByte();

            if (next) {
                newGameDir[i] = static_cast<char>(next);
            } else {
                break;
            }
            i++;
        }
        newGameDir[i] = 0;

        evListener.onNewGameDir(newGameDir);
    }

    void ServerPacketParser::parseDownload(Protocol::CompressedMessage & message)
    {
        static_cast<void>(message);
        // TODO: downloads 
    }

    // TODO: parse if needed, discarding it for now
    void ServerPacketParser::parseRMG(Protocol::CompressedMessage & message)
    {
        uint16_t rmgHeightMapSize = message.readUShort();
        if (rmgHeightMapSize == 0) {
            return;
        }

        if (rmgHeightMapSize >= static_cast<uint16_t>(gameState.compressedHeightmap.size())) {
            rmgHeightMapSize = static_cast<uint16_t>(gameState.compressedHeightmap.size() - 1);
        }

        static_cast<void>(message.readBit());  // compression flag
        message.readData(Utility::Span(gameState.compressedHeightmap.data(), rmgHeightMapSize));

        uint16_t size = message.readUShort();
        if (size >= gameState.compressedHeightmap.size()) {
            size = static_cast<uint16_t>(gameState.compressedHeightmap.size() - 1);
        }

        static_cast<void>(message.readBit());  // compression flag
        message.readData(Utility::Span(gameState.compressedHeightmap.data(), rmgHeightMapSize));

        // Read the seed		
        static_cast<void>(message.readLong());

        parseAutomapSymbols(message);
    }

    // TODO: parse if needed, discarding it for now
    void ServerPacketParser::parseAutomapSymbols(Protocol::CompressedMessage & message)
    {
        uint16_t rmgAutomapSymbolCount = message.readUShort();
        if (rmgAutomapSymbolCount >= MAX_AUTOMAP_SYMBOLS) {
            rmgAutomapSymbolCount = MAX_AUTOMAP_SYMBOLS - 1;
        }

        for (size_t i = 0; i < rmgAutomapSymbolCount; i++) {
            // Discarding
            static_cast<void>(message.readByte());
            static_cast<void>(message.readByte());
            static_cast<void>(message.readLong());
            static_cast<void>(message.readLong());
        }
    }

    void ServerPacketParser::parsePacketEntities(Protocol::CompressedMessage & message, clSnapshot_t *oldframe, clSnapshot_t *newframe)
    {
        constexpr int32_t INVALID_NUM = 99999;

        int32_t newnum = 0;
        entityState_t *oldstate = nullptr;
        int32_t oldindex = 0, oldnum = 0;

        newframe->parseEntitiesNum = gameState.parseEntitiesNum;
        newframe->numEntities = 0;

        // delta from the entities present in oldframe
        oldindex = 0;
        oldstate = nullptr;
        if (!oldframe) {
            oldnum = INVALID_NUM;
        } else {
            if (oldindex >= oldframe->numEntities) {
                oldnum = INVALID_NUM;
            } else {
                oldstate = &parsedEntity(oldframe->parseEntitiesNum, oldindex);
                oldnum = oldstate->number;
            }
        }

        while (1) {
            // read the entity index number
            newnum = message.readBits<GENTITYNUM_BITS>();

            if (newnum >= (MAX_GENTITIES - 1)) {
                break;
            }

            if (message.readcount >= message.cursize) {
                return;
            }

            while (oldnum < newnum) {
                // one or more entities from the old packet are unchanged

                parseDeltaEntity(message, newframe, oldnum, oldstate, true);

                oldindex++;

                if (oldindex >= oldframe->numEntities) {
                    oldnum = INVALID_NUM;
                } else {
                    oldstate = &parsedEntity(oldframe->parseEntitiesNum, oldindex);
                    oldnum = oldstate->number;
                }
            }
            if (oldnum == newnum) {
                // delta from previous state
                parseDeltaEntity(message, newframe, newnum, oldstate, false);

                oldindex++;

                if (oldindex >= oldframe->numEntities) {
                    oldnum = INVALID_NUM;
                } else {
                    oldstate = &parsedEntity(oldframe->parseEntitiesNum, oldindex);
                    oldnum = oldstate->number;
                }
                continue;
            }

            if (oldnum > newnum) {
                // delta from baseline
                parseDeltaEntity(message, newframe, newnum, &gameState.entityBaselines[newnum], false);
                continue;
            }

        }

        // any remaining entities in the old frame are copied over
        while (oldnum != INVALID_NUM) {
            // one or more entities from the old packet are unchanged
            parseDeltaEntity(message, newframe, oldnum, oldstate, true);

            oldindex++;

            if (oldindex >= oldframe->numEntities) {
                oldnum = INVALID_NUM;
            } else {
                oldstate = &parsedEntity(oldframe->parseEntitiesNum, oldindex);
                oldnum = oldstate->number;
            }
        }
    }

    void ServerPacketParser::parseDeltaEntity(Protocol::CompressedMessage & message, clSnapshot_t* frame, int32_t newnum, entityState_t* old, bool unchanged)
    {
        // save the parsed entity state into the big circular buffer so
        // it can be used as the source for a later delta
        entityState_t *state = &parsedEntity(gameState.parseEntitiesNum);

        if (unchanged) {
            *state = *old;
        } else {
            message.readDeltaEntity(old, state, newnum);
        }

        if (state->number >= (MAX_GENTITIES - 1) || state->number < 0) {
            auto & curEnt = gameState.currentEntities[old->number];
            if (curEnt.valid) {  // Don't fire spurious remove-events
                onEntityRemoved(curEnt);
            }
            return;  // entity was delta removed
        }

        gameState.parseEntitiesNum++;
        frame->numEntities++;

        auto & curEnt = gameState.currentEntities[state->number];

        // Copy only if current centity is not valid OR the entity is changed
        if (!unchanged) {
            if (!curEnt.valid) {
                onEntityAdded(curEnt, *state);
            } else {
                onEntityChanged(curEnt, *state);
            }
        }
    }

    CEntity & ServerPacketParser::getEntity(size_t index) &
    {
        return gameState.currentEntities[index];
    }

    const CEntity & ServerPacketParser::getEntity(size_t index) const &
    {
        return gameState.currentEntities[index];
    }

    entityState_t & ServerPacketParser::parsedEntity(size_t parseNum, size_t index) &
    {
        return gameState.parseEntities[(parseNum + index) % MAX_PARSE_ENTITIES];
    }

    const JKA::entityState_t & ServerPacketParser::parsedEntity(size_t parseNum, size_t index) const &
    {
        return gameState.parseEntities[(parseNum + index) % MAX_PARSE_ENTITIES];
    }

    void ServerPacketParser::onEntityRemoved(CEntity & curEnt)
    {
        curEnt.removeEntity();
        evListener.onEntityRemoved(curEnt);
    }

    void ServerPacketParser::onEntityAdded(CEntity & curEnt, entityState_t & newState)
    {
        curEnt.addEntity(newState);
        evListener.onEntityAdded(curEnt, newState);
    }

    void ServerPacketParser::onEntityChanged(CEntity & curEnt, entityState_t & newState)
    {
        evListener.onEntityChanged(curEnt, newState);
        curEnt.changeEntity(newState);
    }

    void ServerPacketParser::initCommands()
    {
        executor.addCommand("disconnect", this, &ServerPacketParser::cmd_disconnect);
        executor.addCommand("cs", this, &ServerPacketParser::cmd_cs);
        executor.addCommand("bcs0", this, &ServerPacketParser::cmd_bcs);  // Bind all three bcs commands to cmd_bcs()
        executor.addCommand("bcs1", this, &ServerPacketParser::cmd_bcs);
        executor.addCommand("bcs2", this, &ServerPacketParser::cmd_bcs);
    }

    // ====================================== Server commands ======================================

    void ServerPacketParser::onServerReliableCommand(std::string_view command)
    {
        auto commandParsed = executor.parseCommandString(command);
        executor.execute(commandParsed);
        evListener.onServerReliableCommand(commandParsed);
    }

    void ServerPacketParser::startBigInfoString(int64_t csNum, std::string_view str)
    {
        bigInfoStringBuffer << "cs " << csNum << " \"" << str;
    }

    void ServerPacketParser::midBigInfoString(std::string_view str)
    {
        bigInfoStringBuffer << str;
    }

    void ServerPacketParser::endBigInfoString(std::string_view str)
    {
        bigInfoStringBuffer << str << "\"";
    }

    std::string ServerPacketParser::flushBigInfoString()
    {
        std::string str = bigInfoStringBuffer.str();
        bigInfoStringBuffer.clear();
        bigInfoStringBuffer.str("");
        return str;
    }

    void ServerPacketParser::cmd_disconnect(const Command & cmd)
    {
        reset();
    }

    void ServerPacketParser::cmd_cs(const Command & cmd)
    {
        if (cmd.args.size() < 2) {
            return;
        }

        auto index = cmd.args[0].getInt64();
        if (index < 0 || index >= MAX_CONFIGSTRINGS) {
            return;
        }

        setConfigstring(index, cmd.concat(1));
    }

    void ServerPacketParser::cmd_bcs(const Command & cmd)
    {
        // bcs[0-2]

        constexpr size_t CMD_LENGTH = 4;
        constexpr size_t CMD_NUM_IDX = 3;
        constexpr size_t MAX_CMD_NUM = 2;

        if (cmd.name.size() != CMD_LENGTH) {
            return;
        }

        size_t cmdNum = static_cast<size_t>(cmd.name[CMD_NUM_IDX] - '0');
        if (cmdNum > MAX_CMD_NUM) {
            return;
        }

        if (cmd.args.size() < 2) {
            return;
        }

        switch (cmdNum) {
        case 0: startBigInfoString(cmd.args[0].getInt64(), cmd.args[1].getStr()); break;
        case 1: midBigInfoString(cmd.args[1].getStr()); break; // Note: we ignore args[0] as original JKA do
        case 2:
        {
            endBigInfoString(cmd.args[1].getStr());
            onServerReliableCommand(flushBigInfoString());  // Execute recursively
            break;
        }
        }
    }
}
