#pragma once

#include <string>
#include <string_view>
#include <memory>

#include "../jka/JKADefs.h"
#include "../jka/JKAConstants.h"
#include "ConnlessPacket.h"

namespace JKA {
    namespace Packets {
        class Getchallenge : public ConnlessPacket {
        public:
            constexpr Getchallenge() noexcept :
                ConnlessPacket(getStaticType())
            {
            }

            virtual ~Getchallenge() = default;

            static constexpr ConnlessType getStaticType() noexcept
            {
                return CLS_GETCHALLENGE;
            }

            static inline auto parse(std::string_view)
            {
                return std::make_unique<Getchallenge>();
            }
        };

        class GetchallengeResponse : public ConnlessDataPacket {
        public:
            GetchallengeResponse(std::string_view data) :
                ConnlessDataPacket(getStaticType(), data)
            {
            }

            virtual ~GetchallengeResponse() = default;

            const std::string & getChallenge() const noexcept
            {
                return data;
            }

            static inline auto parse(std::string_view data)
            {
                return std::make_unique<GetchallengeResponse>(data);
            }

            static constexpr ConnlessType getStaticType() noexcept
            {
                return CLS_GETCHALLENGE_RESPONSE;
            }
        };
    }
}
