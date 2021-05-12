#pragma once
#include <cstdint>
#include <optional>
#include <string>

#include "../SharedDefs.h"
#include "../utility/Span.h"

namespace JKA::Protocol {
    class FragmentBuffer {
    public:
        using FragmentType = std::basic_string<ByteType>;

        FragmentBuffer() noexcept = default;
        FragmentBuffer(const FragmentBuffer &) noexcept = default;
        FragmentBuffer(FragmentBuffer &&) noexcept = default;
        FragmentBuffer & operator=(const FragmentBuffer &) noexcept = default;
        FragmentBuffer & operator=(FragmentBuffer &&) noexcept = default;

        template<typename FragmentA, typename FragmentB>
        friend
            std::enable_if_t<
                std::is_same_v<std::remove_reference_t<FragmentA>, FragmentBuffer> &&
                std::is_same_v<std::remove_reference_t<FragmentB>, FragmentBuffer>
            >
        swap(FragmentA && a, FragmentB && b)
        {
            using std::swap;
            swap(a.fragmentBuffer, b.fragmentBuffer);
            swap(a.fragmentSequence, b.fragmentSequence);
        }

        std::optional<FragmentType>
        processFragment(Utility::Span<const ByteType> fragment,
                        int32_t thisFragmentStart,
                        int32_t thisFragmentSequence)
        {
            if (thisFragmentSequence != fragmentSequence) {
                clearFragmentBuffer();
                fragmentSequence = thisFragmentSequence;
            }

            if (thisFragmentStart != fragmentBuffer.size()) {
                return {};
            }

            appendFragment(fragment);
            
            if (fragment.size() != JKA::FRAGMENT_SIZE) {
                // This fragment was the last one
                auto ret = std::move(fragmentBuffer);
                clearFragmentBuffer();
                return std::optional<FragmentType>(std::move(ret));
            }

            return {};
        }

        void reset() noexcept
        {
            clearFragmentBuffer();
            fragmentSequence = 0;
        }

    private:
        void clearFragmentBuffer()
        {
            fragmentBuffer.clear();
        }

        void appendFragment(Utility::Span<const ByteType> fragment)
        {
            fragmentBuffer.append(fragment.begin(), fragment.end());
        }

        FragmentType fragmentBuffer{};
        int32_t fragmentSequence{};
    };
}
