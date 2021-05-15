#pragma once
#include "CompressedMessage.h"
#include "RawPacket.h"
#include "../SharedDefs.h"

namespace JKA::Protocol {
    // Does not own the packet's data.
    class ClientPacket {
    public:
        constexpr ClientPacket(Utility::Span<ByteType> rawData,
                               CompressedMessage msg,
                               int32_t seq,
                               uint16_t qport,
                               int32_t serverId,
                               int32_t messageAcknowledge,
                               int32_t reliableAcknowledge) noexcept :
            rawData(std::move(rawData)),
            message(std::move(msg)),
            sequence(seq),
            qport(qport),
            serverId(serverId),
            messageAcknowledge(messageAcknowledge),
            reliableAcknowledge(reliableAcknowledge)
        {
        }

        Utility::Span<ByteType> rawData;  // Without sequence
        CompressedMessage message;
        int32_t sequence{};
        uint16_t qport{};
        int32_t serverId{};
        int32_t messageAcknowledge{};
        int32_t reliableAcknowledge{};
    };
}
