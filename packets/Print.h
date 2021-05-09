#pragma once

#include <string>
#include "../jka/JKADefs.h"
#include "../jka/JKAConstants.h"
#include "ConnlessPacket.h"

namespace JKA {
    namespace Packets {
        class Print : public ConnlessDataPacket {
        public:
            Print(std::string_view data) :
                ConnlessDataPacket(getStaticType(), data)
            {
            }

            virtual ~Print() = default;

            static constexpr ConnlessType getStaticType() noexcept
            {
                return CLS_PRINT;
            }

            static inline auto parse(std::string_view data)
            {
                return std::make_unique<Print>(data);
            }
        };
    }
}
