#pragma once
#include <string>
#include "../SharedDefs.h"
#include "../jka/JKAConstants.h"
#include "../utility/BitCast.h"
#include "../utility/Span.h"

namespace JKA::Protocol {
    // Base JKA packet, both client -> server and server -> client.
    // Owns data.
    class RawPacket {
    public:
        using data_type = std::basic_string<ByteType>;

        static constexpr size_t SEQUENCE_LEN = sizeof(int32_t);
        static_assert(SEQUENCE_LEN == 4,
                      "The original JKA code relies on sizeof(int32_t) == 4");

        RawPacket() noexcept(std::is_nothrow_default_constructible_v<decltype(data)>)
            = default;
        RawPacket(const RawPacket & other) = default;
        RawPacket(RawPacket && other) noexcept : RawPacket()
        {
            swap(*this, other);
        }

        explicit RawPacket(std::string data_) noexcept : data(std::move(data_)) {}

        RawPacket & operator=(RawPacket other) noexcept
        {
            swap(*this, other);
            return *this;
        }

        template<typename PA, typename PB>
        friend
            std::enable_if_t<
                std::is_same_v<std::remove_reference_t<PA>, RawPacket> &&
                std::is_same_v<std::remove_reference_t<PB>, RawPacket>
            >
        swap(PA && a, PB && b) noexcept
        {
            using std::swap;
            swap(a.data, b.data);
        }

        // Note: FRAGMENT_BIT is always resetted.
        int32_t getSequence() const noexcept
        {
            if (data.size() >= sizeof(int32_t)) {
                int32_t seq = Utility::bit_reinterpret<int32_t>(data);
                return seq & ~FRAGMENT_BIT;
            } else {
                return 0;
            }
        }

        bool isOOB() const noexcept
        {
            return data.size() >= CONNLESS_PREFIX_S.size() &&
                data.compare(0, CONNLESS_PREFIX_S.size(), CONNLESS_PREFIX_S) == 0;
        }

        bool isConnless() const noexcept
        {
            return isOOB();
        }

        bool isConnfull() const noexcept
        {
            return !isConnless();
        }

        bool isFragmented() const noexcept
        {
            auto seq = getSequence();
            return (seq & FRAGMENT_BIT) != 0;
        }

        Utility::Span<const ByteType> getView() const & noexcept
        {
            return Utility::Span<const ByteType>(data.data(), data.size());
        }

        Utility::Span<const ByteType> getViewAfterSequence() const & noexcept
        {
            return getView().subspan(SEQUENCE_LEN);
        }

        Utility::Span<ByteType> getWriteableView() & noexcept
        {
            return Utility::Span<ByteType>(data.data(), data.size());
        }

        Utility::Span<ByteType> getWriteableViewAfterSequence() & noexcept
        {
            return getWriteableView().subspan(SEQUENCE_LEN);
        }

        data_type & getData() & noexcept
        {
            return data;
        }

        const data_type & getData() const & noexcept
        {
            return data;
        }

        data_type getData() &&
        {
            return data;
        }

        void reset(data_type newData) noexcept
        {
            data = std::move(newData);
        }

    private:
        data_type data{};
    };
}
