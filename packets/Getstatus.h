#pragma once
#include <cassert>
#include <charconv>
#include <memory>
#include <ostream>
#include <string>
#include <string_view>
#include <vector>

#include "../jka/JKADefs.h"
#include "../jka/JKAConstants.h"
#include "../JKAInfo.h"
#include "ConnlessPacket.h"

namespace JKA {
    struct ServerStatus {
        struct PlayerStatus {
            PlayerStatus() = default;
            PlayerStatus(const PlayerStatus &) = default;
            PlayerStatus(PlayerStatus &&) = default;
            PlayerStatus & operator=(const PlayerStatus &) = default;
            PlayerStatus & operator=(PlayerStatus &&) = default;
            ~PlayerStatus() = default;

            int score = 0;
            int ping = 0;
            std::string name = "";

            std::string scoreStr = "";
            std::string pingStr = "";

            bool valid = false;
        };


        ServerStatus() = default;
        ServerStatus(JKAInfo && info_, std::vector<PlayerStatus> && players_) :
            info(std::move(info_)),
            players(std::move(players_))
        {
        }
        ServerStatus(const ServerStatus &) = default;
        ServerStatus(ServerStatus &&) noexcept = default;
        ServerStatus & operator=(const ServerStatus &) = default;
        ServerStatus & operator=(ServerStatus &&) = default;
        ~ServerStatus() = default;

        JKAInfo info;
        std::vector<PlayerStatus> players;
    };

    namespace Packets {
        class Getstatus : public ConnlessDataPacket {
        public:
            static constexpr const char *DEFAULT_CHALLENGE = "";

            Getstatus(std::string_view challenge_ = DEFAULT_CHALLENGE) :
                ConnlessDataPacket(getStaticType(), challenge_)
            {
            }

            virtual ~Getstatus() = default;

            static constexpr ConnlessType getStaticType() noexcept
            {
                return CLS_GETSTATUS;
            }

            virtual std::string getChallenge() const
            {
                return data;
            }

            static inline auto parse(std::string_view data)
            {
                return std::make_unique<Getstatus>(data);
            }
        };

        class GetstatusResponse : public ConnlessDataPacket {
        public:
            GetstatusResponse(std::string_view data, ServerStatus && status_) :
                ConnlessDataPacket(getStaticType(), data),
                status(std::move(status_))
            {

            }

            virtual ~GetstatusResponse() = default;

            static constexpr ConnlessType getStaticType() noexcept
            {
                return CLS_GETSTATUS_RESPONSE;
            }

            const ServerStatus & getStatus() const noexcept
            {
                return status;
            }

            static inline std::unique_ptr<GetstatusResponse> parse(std::string_view data)
            {
                // Data format is
                // <infostring>\n<ping1 score1 "player_name1">\n<ping2 score2 "player_name2">\n...
                // NB: We're trying to somewhat adequately parse newlines in infostring or player names

                std::vector<ServerStatus::PlayerStatus> players;
                std::ostringstream infoStream{};
                // Treat any malformed line as part of an infostring
                // until valid playerStatus line is found
                bool validPlayerParsed = false;

                constexpr std::string_view separator = CONNLESS_PACKETS[getStaticType()].separator;
                size_t lastBackslashIdx = data.rfind('\\');
                if (lastBackslashIdx >= data.size()) {
                    // No infostring at all: we could not repair this response
                    return nullptr;
                }

                size_t sepIdx = data.find(separator, lastBackslashIdx);
                infoStream << data.substr(0, sepIdx);
                
                while (sepIdx < data.size()) {
                    data.remove_prefix(sepIdx + 1);
                    sepIdx = data.find(separator);  // Find the end of current line;
                                                    // Normally (if current line is well-formed)
                                                    // sepIdx would not be modified during an iteration,
                                                    // e.g. A\nB\nC\n -> sepIdx in {1, 3, 5, npos};
                                                    // sepIdx could be advanced past the end of a player's nickname
                                                    // if the player has \n in their name

                    // N.B.: some players (hackers) may have '\n' (the separator) in their
                    // names, so we should find the real end of a player's name (it would always be surrounded by ")

                    size_t firstQuoteIdx = data.find('"');
                    if ((firstQuoteIdx != data.npos && firstQuoteIdx >= sepIdx)
                        || (firstQuoteIdx == 0)) {
                        // The first quote char is found, but it is either past the newline characted
                        // or in the beginning of the line (missing score and ping),
                        // so current line is malformed
                        if (!validPlayerParsed) {
                            // Treat malformed line as part of infostring
                            infoStream << separator << data.substr(0, sepIdx);
                        }
                        continue;  // Continue to 
                    }

                    if (firstQuoteIdx == data.npos) {
                        // No start of player name in the whole data, parsing is done
                        if (!validPlayerParsed) {
                            // Treat malformed line as part of infostring
                            infoStream << separator << data.substr(0, sepIdx);
                        }
                        break;
                    }

                    size_t secondQuoteIdx = data.find('"', firstQuoteIdx + 1);
                    if (secondQuoteIdx == data.npos) {
                        // No end of player name in the whole data, parsing is done
                        if (!validPlayerParsed) {
                            // Treat malformed line as part of infostring
                            infoStream << data.substr(0, sepIdx);
                        }
                        break;
                    }

                    size_t nextLineIdx = secondQuoteIdx + 1;
                    auto player = parsePlayerString(data.substr(0, nextLineIdx), firstQuoteIdx, secondQuoteIdx);
                    if (player.valid) {
                        players.push_back(std::move(player));
                        validPlayerParsed = true;

                        sepIdx = nextLineIdx;  // Move sepIdx past the end of player's name
                    } else {
                        if (!validPlayerParsed) {
                            // Treat malformed line as part of infostring
                            infoStream << separator << data.substr(0, sepIdx);
                        } else {
                            // Treat malformed line as a partial player info
                            // (e.g. '1 xxx "Name"' -> we could at least display score and name)
                            players.push_back(std::move(player));
                        }
                    }
                }

                return std::make_unique<GetstatusResponse>(data, ServerStatus(JKAInfo(infoStream.str()), std::move(players)));
            }

        protected:
            static ServerStatus::PlayerStatus parsePlayerString(std::string_view playerString, size_t firstQuoteIdx, size_t secondQuoteIdx)
            {
                assert(firstQuoteIdx < secondQuoteIdx);
                assert(secondQuoteIdx < playerString.size());

                ServerStatus::PlayerStatus player{};
                bool valid = true;
                player.name = std::string(playerString.substr(firstQuoteIdx + 1, secondQuoteIdx - firstQuoteIdx - 1));

                size_t spaceIdx = playerString.find(' ');
                if (spaceIdx == playerString.npos) {
                    return player;
                }

                // Parse score
                player.scoreStr = playerString.substr(0, spaceIdx);
                auto res = std::from_chars(player.scoreStr.data(), player.scoreStr.data() + player.scoreStr.size(), player.score);
                if (res.ec != std::errc()) {  // Parsing error
                    valid = false;
                }

                // Remove score
                playerString.remove_prefix(spaceIdx + 1);

                spaceIdx = playerString.find(' ');
                if (spaceIdx == playerString.npos) {
                    return player;
                }

                // Parse ping
                player.pingStr = playerString.substr(0, spaceIdx);
                res = std::from_chars(player.pingStr.data(), player.pingStr.data() + player.pingStr.size(), player.ping);
                if (res.ec != std::errc()) {  // Parsing error
                    valid = false;
                }

                player.valid = valid;
                return player;
            }

            ServerStatus status;
        };
    }
}
