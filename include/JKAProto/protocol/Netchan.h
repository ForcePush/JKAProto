#pragma once
#include "../SharedDefs.h"
#include "../utility/Span.h"
#include "../jka/JKAConstants.h"
#include "ServerPacket.h"
#include "FragmentBuffer.h"
#include "State.h"
#include "PacketEncoder.h"

namespace JKA::Protocol {
    template<typename PacketEncoderT>
    class Netchan {
    public:
        using IncomingPacketType = typename PacketEncoderT::PacketType;
        using OutgoingEncoder = OutgoingPacketEncoder<PacketEncoderT>;
        using OutgoingPacketType = typename OutgoingEncoder::PacketType;

        Netchan() noexcept = default;
        explicit Netchan(int32_t challenge) noexcept :
            protocolState{ challenge } {}
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
            swap(a.protocolState, b.protocolState);
            swap(a.incomingSequence, b.incomingSequence);
            swap(a.outgoingSequence, b.outgoingSequence);
            swap(a.fragmentBuffer, b.fragmentBuffer);
        }

        // This function will MODIFY the original packet.
        // Either decrypts non-fragmented packet or stores a fragment into
        // fragmentBuffer.
        // Updates incomingSequence.
        std::optional<IncomingPacketType> processIncomingPacket(RawPacket & packet, Q3Huffman & huffman)
        {
            int32_t sequence = packet.getSequence();
            bool fragmented = packet.isFragmented();
            auto data = packet.getWriteableViewAfterSequence();

            if (sequence <= getIncomingSequence()) {
                return {};  // Duplicating packet
            }

            if (!fragmented) JKA_LIKELY {
                incomingSequence = sequence;
                return std::move(PacketEncoderT::encode(data, sequence, huffman, state()));
            }

            // This is a fragment

            int32_t curFragmentStart  = Utility::bit_reinterpret<int16_t>(data);
            data = data.subspan(sizeof(int16_t));

            int32_t curFragmentLength = Utility::bit_reinterpret<int16_t>(data);
            data = data.subspan(sizeof(int16_t));

            auto fragment = data;
            if (fragment.size() != curFragmentLength) {
                return {};  // TODO: check if sizes must actually match
            }

            auto processResult = processFragment(fragment, curFragmentStart, sequence);
            if (processResult.has_value()) {
                packet.reset(std::move(processResult.value()));
                return processIncomingPacket(packet, huffman);
            }

            // Mid-fragment
            return {};
        }

        // TODO: outgoing fragmentation
        bool processOutgoingPacket(Utility::Span<ByteType> data, int32_t sequence, Q3Huffman & huff)
        {
            if (sequence <= outgoingSequence) JKA_UNLIKELY {
                return false;  // Duplicating outgoing packet
            }

            outgoingSequence = sequence;
            OutgoingEncoder::encode(data, sequence, huff, state());
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

        State & state() noexcept
        {
            return protocolState;
        }

        const State & state() const noexcept
        {
            return protocolState;
        }

        constexpr int32_t getChallenge() const noexcept
        {
            return state().getChallenge();
        }

        constexpr std::string & reliableCommand(size_t idx) &
        {
            return state().reliableCommand(idx);
        }

        constexpr const std::string & reliableCommand(size_t idx) const &
        {
            return state().reliableCommand(idx);
        }

        void reset(int32_t newChallenge = 0) noexcept
        {
            state().reset(newChallenge);
            incomingSequence = 0;
            outgoingSequence = 0;
            fragmentBuffer.reset();
        }

    private:
        std::optional<FragmentBuffer::FragmentType>
        processFragment(Utility::Span<const ByteType> fragment,
                        int32_t thisFragmentStart,
                        int32_t thisFragmentSequence)
        {
            return fragmentBuffer.processFragment(std::move(fragment),
                                                  thisFragmentStart,
                                                  thisFragmentSequence);
        }

        State protocolState{};
        int32_t incomingSequence{};
        int32_t outgoingSequence{};

        FragmentBuffer fragmentBuffer{};
    };
}
