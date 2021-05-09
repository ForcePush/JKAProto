#pragma once

#include <string>
#include <string_view>
#include <memory>

#include "../Huffman.h"
#include "../JKAInfo.h"
#include "../jka/JKADefs.h"
#include "../jka/JKAConstants.h"
#include "ConnlessPacket.h"

namespace JKA {
    namespace Packets {
        class Disconnect : public ConnlessDataPacket {
        public:
            Disconnect(std::string_view rawData) :
                ConnlessDataPacket(getStaticType(), rawData)
            {
            }

            virtual ~Disconnect() = default;

            static constexpr ConnlessType getStaticType() noexcept
            {
                return CLS_DISCONNECT;
            }

            static inline auto parse(std::string_view data)
            {
                return std::make_unique<Disconnect>(data);
            }
        };
    }
}
