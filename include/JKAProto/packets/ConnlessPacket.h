#pragma once
#include <sstream>
#include <string>
#include <string_view>
#include "../jka/JKADefs.h"
#include "../jka/JKAConstants.h"

namespace JKA {
    namespace Packets {
        class ConnlessPacket {
        public:
            virtual ~ConnlessPacket() = default;

            constexpr ConnlessType getType() const noexcept
            {
                return type;
            }

            constexpr static bool isConnless(std::string_view packet) noexcept
            {
                return !packet.empty() && packet.substr(0, CONNLESS_PREFIX_SIZE) == CONNLESS_PREFIX_S;
            }

            constexpr std::string_view getName() const noexcept
            {
                return CONNLESS_PACKETS[getType()].name;
            }

            virtual std::string getRawPacket() const
            {
                std::ostringstream ss;
                ss << CONNLESS_PREFIX_S << getName();
                return ss.str();
            }

        protected:
            constexpr ConnlessPacket(ConnlessType type_) noexcept : type(type_) {}

        private:
            const ConnlessType type;
        };

        class ConnlessDataPacket : public ConnlessPacket {
        public:
            ConnlessDataPacket(ConnlessType type_, std::string_view data_) :
                ConnlessPacket(type_),
                data(data_)
            {
            }

            virtual ~ConnlessDataPacket() = default;

            constexpr std::string_view getSeparator() const noexcept
            {
                return CONNLESS_PACKETS[getType()].separator;
            }

            virtual std::string getData() const
            {
                return data;
            }

            virtual void setData(std::string_view newData)
            {
                data = newData;
            }

            virtual std::string getRawPacket() const override
            {
                auto basePacket = ConnlessPacket::getRawPacket();
                if (data.empty()) {
                    return basePacket;
                } else {
                    std::ostringstream ss;
                    ss << basePacket << getSeparator() << getData();
                    return ss.str();
                }
            }

        protected:
            std::string data = "";
        };
    }
}
