#pragma once
#include <cstdint>
#include "RawPacket.h"
#include "../SharedDefs.h"
#include "../utility/Span.h"

namespace JKA::Protocol {
    class PacketBase {
    protected:
        constexpr PacketBase() noexcept = default;
        constexpr PacketBase(Utility::Span<ByteType> dataView_,
                             int32_t seq) noexcept :
            dataView(std::move(dataView_)),
            packetSequence(seq) {}

        PacketBase(RawPacket & packet) noexcept :
            PacketBase(packet.getWriteableViewAfterSequence(), packet.getSequence()) {}

    public:
        constexpr int32_t sequence() const noexcept
        {
            return packetSequence;
        }

        constexpr Utility::Span<ByteType> data() noexcept
        {
            return dataView;
        }

        Utility::Span<const ByteType> data() const noexcept
        {
            return dataView;
        }

        constexpr size_t size() const noexcept
        {
            return dataView.size();
        }

    private:
        Utility::Span<ByteType> dataView{};
        int32_t packetSequence{};
    };
}
