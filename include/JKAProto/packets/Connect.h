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
        class Connect : public ConnlessDataPacket {
        public:
            Connect(std::string_view rawData) :
                ConnlessDataPacket(getStaticType(), rawData)
            {
            }

            Connect(const JKAInfo & info, Q3Huffman & huff) :
                ConnlessDataPacket(getStaticType(), huff.compress("\"" + info.toInfostring() + "\""))
            {
            }

            Connect(std::string_view infostring, Q3Huffman& huff) :
                ConnlessDataPacket(getStaticType(), huff.compress("\"" + std::string(infostring) + "\""))
            {
            }

            virtual ~Connect() = default;

            static constexpr ConnlessType getStaticType() noexcept
            {
                return CLS_CONNECT;
            }

            static inline auto parse(std::string_view data)
            {
                return std::make_unique<Connect>(data);
            }
        };

        class ConnectResponse : public ConnlessPacket {
        public:
            ConnectResponse() :
                ConnlessPacket(getStaticType())
            {
            }

            virtual ~ConnectResponse() = default;

            static constexpr ConnlessType getStaticType() noexcept
            {
                return CLS_CONNECT_RESPONSE;
            }

            static inline auto parse(std::string_view)
            {
                return std::make_unique<ConnectResponse>();
            }
        };
    }
}
