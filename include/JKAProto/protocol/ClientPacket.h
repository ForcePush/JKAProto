#pragma once
#include "CompressedMessage.h"
#include "RawPacket.h"
#include "State.h"
#include "../SharedDefs.h"

namespace JKA::Protocol {
    // Does not own the packet's data.
    class ClientPacket {
    public:
        constexpr ClientPacket(Utility::Span<ByteType> rawData_,
                               CompressedMessage msg,
                               int32_t seq,
                               int32_t serverId_,
                               int32_t messageAcknowledge_,
                               int32_t reliableAcknowledge_) noexcept :
            rawData(std::move(rawData_)),
            message(std::move(msg)),
            sequence(seq),
            serverId(serverId_),
            messageAcknowledge(messageAcknowledge_),
            reliableAcknowledge(reliableAcknowledge_)
        {
        }

        Utility::Span<ByteType> rawData;  // Without sequence
        CompressedMessage message;
        int32_t sequence{};
        int32_t serverId{};
        int32_t messageAcknowledge{};
        int32_t reliableAcknowledge{};
    };
}
