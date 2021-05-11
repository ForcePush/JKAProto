#pragma once
#include <optional>
#include "CompressedMessage.h"
#include "PacketBase.h"
#include "RawPacket.h"
#include "State.h"
#include "../Huffman.h"
#include "../SharedDefs.h"
#include "../utility/Span.h"

namespace JKA::Protocol {
    // Does not own the packet's data.
    class DecryptedServerPacket {
    public:
        constexpr DecryptedServerPacket(CompressedMessage msg,
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

    class EncryptedServerPacket : public PacketBase {
    public:
        EncryptedServerPacket(RawPacket & packet) noexcept :
            PacketBase(packet)
        {
        }

        // This function will MODIFY the original packet.
        // Either decrypts non-fragmented packet or stores a fragment into
        // protocolState's fragmentBuffer.
        // Updates protocolState's incomingSequence.
        std::optional<DecryptedServerPacket> processRawPacket(
            Q3Huffman & huffman,
            State & protocolState)
        {
            if (sequence() <= protocolState.getIncomingSequence()) {
                return {};  // Duplicating packet
            }

            auto msg = getMessage(huffman);
            if (!fragmented()) {
                return processDecrypt(huffman, msg, protocolState);
            }

            constexpr int32_t SHORT_BITS = 16;
            constexpr int32_t SKIP_FRAGMENT = SHORT_BITS * 2;

            int32_t curFragmentStart = msg.readOOB<SHORT_BITS>();
            int32_t curFragmentLength = msg.readOOB<SHORT_BITS>();

            auto processResult = protocolState.processFragment(data().subspan(SKIP_FRAGMENT),
                                                               curFragmentStart,
                                                               sequence());
            if (processResult.has_value()) {
                auto fullPacket = RawPacket(std::move(processResult.value()));
                auto fullEncryptedPacket = EncryptedServerPacket(fullPacket);
                auto fullMessage = fullEncryptedPacket.getMessage(huffman);
                return fullEncryptedPacket.processDecrypt(huffman, fullMessage, protocolState);
            }

            // Mid-fragment
            return {};
        }

    private:
        EncryptedServerPacket(Utility::Span<ByteType> data,
                              int32_t seq,
                              bool fragmented_) noexcept :
            PacketBase(std::move(data), seq, fragmented_)
        {
        }

        CompressedMessage getMessage(Q3Huffman & huff)
        {
            return CompressedMessage(huff, data());
        }

        DecryptedServerPacket processDecrypt(Q3Huffman & huffman, CompressedMessage & msg, State & protocolState) noexcept
        {
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

            protocolState.updateIncomingSequence(sequence());
            return DecryptedServerPacket(std::move(msg), sequence(), reliableAcknowledge);
        }
    };
}
