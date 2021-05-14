#pragma once
#include "CompressedMessage.h"
#include "RawPacket.h"
#include "../SharedDefs.h"

namespace JKA::Protocol {
    // Does not own the packet's data.
    class ServerPacket {
    public:
        constexpr ServerPacket(Utility::Span<ByteType> rawData_,
                               CompressedMessage msg,
                               int32_t seq,
                               int32_t relAck) noexcept :
            rawData(std::move(rawData_)),
            message(std::move(msg)),
            sequence(seq),
            reliableAcknowledge(relAck)
        {}

        Utility::Span<ByteType> rawData;  // Without sequence
        CompressedMessage message;
        int32_t sequence{};
        int32_t reliableAcknowledge{};
    };
}
