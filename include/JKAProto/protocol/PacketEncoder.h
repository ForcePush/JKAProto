#pragma once
#include "../SharedDefs.h"
#include "../utility/Span.h"
#include "../Huffman.h"
#include "ClientPacket.h"
#include "CompressedMessage.h"
#include "RawPacket.h"
#include "ServerPacket.h"
#include "State.h"

namespace JKA::Protocol {
    namespace detail {
        struct BasicEncoder {
            constexpr static void encode(Utility::Span<ByteType> data,
                                         Utility::Span<const unsigned char> keyString,
                                         unsigned char key) noexcept
            {
                size_t keyIndex = 0;
                for (size_t i = 0; i < data.size(); i++) {
                    unsigned char keyChar = 0;
                    if (keyIndex >= keyString.size()) {
                        keyIndex = 0;
                    }

                    if (keyIndex < keyString.size()) {
                        // rww: special case for keyString.size() == 0
                        keyChar = keyString[keyIndex];
                    }

                    if (keyChar == '%') {
                        key ^= '.' << (i & 1);
                    } else {
                        key ^= keyChar << (i & 1);
                    }

                    keyIndex++;

                    data[i] ^= key;
                }
            }
        };
    }

    struct ServerPacketEncoder {
        using PacketType = ServerPacket;

        //   sequence       relAck (huff)    XOR-encoded (huff)
        // [xx xx xx xx]    <xx xx xx xx>    <xx xx ...       >
        // ^                ^                ^
        // start of packet  data             data + CL_DECODE_START
        static PacketType encode(Utility::Span<ByteType> data,
                                 int32_t sequence,
                                 Q3Huffman & huff,
                                 const State & state) noexcept
        {
            auto msg = CompressedMessage(huff, data);
            auto span = data.subspan(CL_DECODE_START);

            int32_t relAck = msg.readLong();

            auto keyString = Utility::Span(state.reliableCommand(relAck));
            unsigned char key = static_cast<unsigned char>(state.getChallenge() ^ sequence);
            detail::BasicEncoder::encode(span, keyString, key);

            return PacketType(std::move(data), std::move(msg), sequence, relAck);
        }
    };

    struct ClientPacketEncoder {
        using PacketType = ClientPacket;

        //   sequence        qport      sId (huff)    mAck (huff)  relAck (huff)    XOR-encoded (huff)
        // [xx xx xx xx]    [xx xx]    <xx xx xx xx> <xx xx xx xx> <xx xx xx xx>    <xx xx ...       >
        // ^                           ^                                            ^
        // start of packet             data                                         data + SV_DECODE_START
        static PacketType encode(Utility::Span<ByteType> data,
                                 int32_t sequence,
                                 Q3Huffman & huff,
                                 const State & state) noexcept
        {
            auto msg = CompressedMessage(huff, data);
            auto span = data.subspan(SV_DECODE_START);

            int32_t serverId = msg.readLong();
            int32_t messageAcknowledge = msg.readLong();
            int32_t reliableAcknowledge = msg.readLong();

            auto keyString = Utility::Span(state.serverCommand(reliableAcknowledge));
            // Note: sId and mAck are not unsigned-casted, in accordance with the original
            // JKA code
            auto key = static_cast<unsigned char>(state.getChallenge() ^ serverId ^ messageAcknowledge);
            detail::BasicEncoder::encode(span, keyString, key);

            return PacketType(std::move(data), std::move(msg),
                              sequence, serverId, messageAcknowledge, reliableAcknowledge);
        }
    };

    namespace detail {
        template<typename Encoder>
        struct OutgoingPacketEncoderImpl {
            static_assert(!sizeof(Encoder*), "Invalid packet encoder. "
                          "Must be either ServerPacketEncoder or ClientPacketEncoder.");
        };

        template<>
        struct OutgoingPacketEncoderImpl<ServerPacketEncoder> {
            using type = ClientPacketEncoder;
        };

        template<>
        struct OutgoingPacketEncoderImpl<ClientPacketEncoder> {
            using type = ServerPacketEncoder;
        };
    }

    template<typename Encoder>
    using OutgoingPacketEncoder = typename detail::OutgoingPacketEncoderImpl<Encoder>::type;
}
