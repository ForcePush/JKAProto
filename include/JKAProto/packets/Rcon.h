#pragma once

#include <string>
#include "../jka/JKADefs.h"
#include "../jka/JKAConstants.h"
#include "ConnlessPacket.h"

namespace JKA {
    namespace Packets {
        class Rcon : public ConnlessDataPacket {
        public:
            Rcon(std::string_view data) :
                ConnlessDataPacket(getStaticType(), data)
            {
            }

            virtual ~Rcon() = default;

            static constexpr ConnlessType getStaticType() noexcept
            {
                return CLS_RCON;
            }

            static inline auto parse(std::string_view data)
            {
                return std::make_unique<Rcon>(data);
            }
        };
    }
}
