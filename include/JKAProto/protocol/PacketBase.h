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
                             int32_t seq,
                             bool isFragmented_) noexcept :
            dataView(std::move(dataView_)),
            packetSequence(seq),
            isFragmented(isFragmented_)
        {}

        PacketBase(RawPacket & packet) noexcept :
            PacketBase(packet.getWriteableViewAfterSequence(),
                       packet.getSequence(),
                       packet.isFragmented()) {}

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

        constexpr bool fragmented() const noexcept
        {
            return isFragmented;
        }

    private:
        Utility::Span<ByteType> dataView{};
        int32_t packetSequence{};
        bool isFragmented{};
    };
}
