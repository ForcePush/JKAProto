#pragma once
#include "CompressedMessage.h"
#include "PacketBase.h"
#include "RawPacket.h"
#include "State.h"
#include "../Huffman.h"
#include "../SharedDefs.h"
#include "../utility/Span.h"

namespace JKA::Protocol {
    // Does not own the packet's data.
    class DecryptedClientPacket {
    public:
        constexpr DecryptedClientPacket(CompressedMessage msg,
                                        int32_t seq,
                                        int32_t relAck) noexcept :
            message(std::move(msg)),
            sequence(seq),
            reliableAcknowledge(relAck)
        {}

        CompressedMessage message;
        int32_t sequence{};
        int32_t reliableAcknowledge{};
    };

    class EncryptedClientPacket : public PacketBase {
    public:
        explicit EncryptedClientPacket(RawPacket & packet) :
            PacketBase(packet.getWriteableViewAfterSequence(),
                       packet.getSequence())
        {
        }

        // This function will MODIFY the original packet.
        DecryptedClientPacket decrypt(Q3Huffman & huffman, const State & protocolState) noexcept
        {
            auto msg = CompressedMessage(huffman, data());
            auto reliableAcknowledge = msg.peekLong();

            auto span = data().subspan(CL_DECODE_START);
            auto keyString = Utility::Span(protocolState.reliableCommand(reliableAcknowledge))
                .to_span<const unsigned char>();
            size_t keyIndex = CL_DECODE_KEY_INDEX_START;

            unsigned char key = static_cast<unsigned char>(protocolState.getChallenge() ^ static_cast<uint32_t>(sequence()));
            for (size_t i = 0; i < span.size(); i++) {
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

                span[i] ^= key;
            }

            return DecryptedClientPacket(std::move(msg), sequence(), reliableAcknowledge);
        }
    };
}
