#pragma once
#include "../jka/JKAConstants.h"
#include "../ClientConnection.h"
#include "../ReliableCommandsStore.h"
#include "../SharedDefs.h"
#include "../utility/Span.h"
#include "ServerPacket.h"
#include "FragmentBuffer.h"
#include "PacketEncoder.h"

namespace JKA::Protocol {
    template<typename PacketEncoderT>
    class Netchan {
    public:
        using IncomingPacketType = typename PacketEncoderT::PacketType;
        using OutgoingEncoder = OutgoingPacketEncoder<PacketEncoderT>;
        using OutgoingPacketType = typename OutgoingEncoder::PacketType;

        Netchan() noexcept = default;
        Netchan(const Netchan &) noexcept = default;
        Netchan(Netchan &&) noexcept = default;
        Netchan & operator=(Netchan other) noexcept
        {
            swap(*this, other);
            return *this;
        }

        template<typename NetchanA, typename NetchanB>
        friend
            std::enable_if_t<
                std::is_same_v<std::remove_reference_t<NetchanA>, Netchan> &&
                std::is_same_v<std::remove_reference_t<NetchanB>, Netchan>
            >
        swap(NetchanA && a, NetchanB && b) noexcept
        {
            using std::swap;
            swap(a.incomingSequence, b.incomingSequence);
            swap(a.outgoingSequence, b.outgoingSequence);
            swap(a.fragmentBuffer, b.fragmentBuffer);
        }

        // This function will MODIFY the original packet.
        // Either decrypts non-fragmented packet or stores a fragment into
        // fragmentBuffer.
        // Updates incomingSequence.
        std::optional<IncomingPacketType> processIncomingPacket(RawPacket & packet,
                                                                Q3Huffman & huffman,
                                                                ClientConnection & connection,
                                                                const ReliableCommandsStore & store)
        {
            int32_t sequence = packet.getSequence();
            bool fragmented = packet.isFragmented();
            auto data = packet.getWriteableViewAfterSequence();

            if (!fragmented) JKA_LIKELY {
                return processIncomingData(data, sequence, huffman, connection, store);
            }

            // This is a fragment

            int32_t curFragmentStart  = Utility::bit_reinterpret<int16_t>(data);
            data = data.subspan(sizeof(int16_t));

            int32_t curFragmentLength = Utility::bit_reinterpret<int16_t>(data);
            data = data.subspan(sizeof(int16_t));

            auto fragment = data;
            if (fragment.size() != static_cast<size_t>(curFragmentLength)) {
                return {};
            }

            auto processResult = processFragment(fragment, curFragmentStart, sequence);
            if (processResult.has_value()) {
                packet.reset(std::move(processResult.value()));
                return processIncomingData(packet.getWriteableViewAfterSequence(),
                                           sequence, huffman, connection, store);
            }

            // Mid-fragment
            return {};
        }

        // TODO: outgoing fragmentation
        bool processOutgoingPacket(Utility::Span<ByteType> data,
                                   int32_t currentSequence,
                                   Q3Huffman & huff, 
                                   ClientConnection & connection,
                                   const ReliableCommandsStore & store)
        {
            if (currentSequence < outgoingSequence) JKA_UNLIKELY {
                return false;  // Duplicating outgoing packet
            }

            OutgoingEncoder::encode(data, currentSequence, connection.challenge, huff, store);
            outgoingSequence = currentSequence + 1;
            return true;
        }

        constexpr int32_t getIncomingSequence() const noexcept
        {
            return incomingSequence;
        }

        constexpr int32_t getOutgoingSequence() const noexcept
        {
            return outgoingSequence;
        }

        // Called after the initial 'connect' has been sent
        void setInitialConnectedState() noexcept
        {
            outgoingSequence = 1;
        }

        void reset() noexcept
        {
            incomingSequence = 0;
            outgoingSequence = 0;
            fragmentBuffer.reset();
        }

    private:
        std::optional<IncomingPacketType> processIncomingData(Utility::Span<ByteType> data,
                                                              int32_t sequence,
                                                              Q3Huffman & huffman,
                                                              ClientConnection & connection,
                                                              const ReliableCommandsStore & store)
        {
            if (sequence <= getIncomingSequence()) {
                return {};  // Duplicating packet
            }

            incomingSequence = sequence;
            return std::move(PacketEncoderT::encode(data, sequence, connection.challenge, huffman, store));
        }

        std::optional<FragmentBuffer::FragmentType>
        processFragment(Utility::Span<const ByteType> fragment,
                        int32_t thisFragmentStart,
                        int32_t thisFragmentSequence)
        {
            return fragmentBuffer.processFragment(std::move(fragment),
                                                  thisFragmentStart,
                                                  thisFragmentSequence);
        }

        int32_t incomingSequence{};
        int32_t outgoingSequence{};

        FragmentBuffer fragmentBuffer{};
    };
}
