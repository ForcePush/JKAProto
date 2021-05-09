#pragma once

#include <string>
#include <string_view>
#include <memory>

#include "../jka/JKADefs.h"
#include "../jka/JKAConstants.h"
#include "../JKAInfo.h"
#include "ConnlessPacket.h"

namespace JKA {
    namespace Packets {
        class Getinfo : public ConnlessDataPacket {
        public:
            static constexpr const char *DEFAULT_CHALLENGE = "xxx";

            Getinfo(std::string_view challenge_ = DEFAULT_CHALLENGE) :
                ConnlessDataPacket(getStaticType(), challenge_)
            {
            }

            virtual ~Getinfo() = default;

            static constexpr ConnlessType getStaticType() noexcept
            {
                return CLS_GETINFO;
            }

            const std::string & getChallenge() const noexcept
            {
                return data;
            }

            static inline auto parse(std::string_view data)
            {
                return std::make_unique<Getinfo>(data);
            }
        };

        class GetinfoResponse : public ConnlessDataPacket {
        public:
            GetinfoResponse(std::string_view data) :
                ConnlessDataPacket(getStaticType(), data),
                info(data)
            {
            }

            virtual ~GetinfoResponse() = default;

            static constexpr ConnlessType getStaticType() noexcept
            {
                return CLS_GETINFO_RESPONSE;
            }

            const JKAInfo & getInfo() const noexcept
            {
                return info;
            }

            static inline auto parse(std::string_view data)
            {
                return std::make_unique<GetinfoResponse>(data);
            }

        protected:
            JKAInfo info{};
        };
    }
}
