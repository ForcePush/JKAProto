#pragma once

#include <string>
#include <vector>
#include <boost/asio.hpp>

#include "../jka/JKADefs.h"
#include "../jka/JKAConstants.h"
#include "ConnlessPacket.h"

namespace JKA {
    namespace Packets {
        class Getservers : public ConnlessDataPacket {
        public:
            static constexpr char DEFAULT_PROTOCOL[] = "26";

            Getservers(std::string_view protocol = DEFAULT_PROTOCOL) :
                ConnlessDataPacket(getStaticType(), protocol)
            {
            }

            virtual ~Getservers() = default;

            static constexpr ConnlessType getStaticType() noexcept
            {
                return CLS_GETSERVERS;
            }

            const std::string & getProtocol() const noexcept
            {
                return data;
            }

            static inline auto parse(std::string_view data)
            {
                return std::make_unique<Getservers>(data);
            }
        };

        class GetserversResponse : public ConnlessDataPacket {
        public:
            using ServersVector = std::vector<boost::asio::ip::udp::endpoint>;

            GetserversResponse(std::string_view data, ServersVector && servers_) :
                ConnlessDataPacket(getStaticType(), data),
                servers(servers_)
            {
            }

            virtual ~GetserversResponse() = default;

            static constexpr ConnlessType getStaticType() noexcept
            {
                return CLS_GETSERVERS_RESPONSE;
            }

            const ServersVector & getServers() const noexcept
            {
                return servers;
            }

            static inline std::unique_ptr<GetserversResponse> parse(std::string_view data)
            {
                const uint8_t *response = reinterpret_cast<const uint8_t *>(data.data());
                const uint8_t *responseEnd = response + data.size();

                ServersVector servers;

                // Original JKA's implementation
                while (response + 1 < responseEnd) {
                    boost::asio::ip::address_v4::bytes_type ip = {};
                    uint16_t port = 0;

                    // advance to initial token
                    do {
                        if (*response++ == '\\') {
                            break;
                        }
                    } while (response < responseEnd);

                    if (response >= responseEnd - (ip.size() + sizeof(port))) {
                        break;
                    }

                    ip[0] = *response++;
                    ip[1] = *response++;
                    ip[2] = *response++;
                    ip[3] = *response++;

                    // parse out port
                    port = static_cast<uint16_t>(*response++) << 8;
                    port |= static_cast<uint16_t>(*response++);

                    // syntax check
                    if (*response != '\\') {
                        break;
                    }

                    servers.emplace_back(boost::asio::ip::address_v4(ip), port);

                    // parse out EOT
                    if ((responseEnd - response) >= 3 && response[1] == 'E' && response[2] == 'O' && response[3] == 'T') {
                        break;
                    }
                }

                return std::make_unique<GetserversResponse>(data, std::move(servers));
            }

        private:
            ServersVector servers;
        };
    }
}
