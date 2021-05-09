#include <JKAProto/jka/JKAEvents.h>
#include <sstream>
#include <string>
#include <map>
#include <array>
#include <cassert>
#include <charconv>

namespace JKA {
    namespace detail {
        constexpr static std::string_view UNKNOWN_EV_PREFIX = "EV_UNKNOWN_";

        struct EvInfo {
            entity_event_t value = entity_event_t::EV_NONE;
            std::string name = "";
            std::string_view comment = "";
            bool known = false;
        };

        using EventToNameTable = std::array<EvInfo, ENTITY_EVENTS_COUNT>;
        using NameToEventTable = std::map<std::string, entity_event_t, std::less<>>;

        inline std::string getUnknownEventName(int32_t ev)
        {
            std::ostringstream ss;
            ss << UNKNOWN_EV_PREFIX << ev;
            return ss.str();
        }

        inline std::optional<entity_event_t> tryParseUnknownEventName(std::string_view name)
        {
            if (name.compare(0, UNKNOWN_EV_PREFIX.size(), UNKNOWN_EV_PREFIX) == 0) {
                std::string_view evNumStr = name.substr(UNKNOWN_EV_PREFIX.size());
                int32_t val = 0;
                auto res = std::from_chars(evNumStr.data(), evNumStr.data() + evNumStr.size(), val);
                if (res.ec != std::errc() && val >= 0 && val < ENTITY_EVENTS_COUNT) {
                    return static_cast<entity_event_t>(val);
                }
            }

            return {};
        }

        inline const EventToNameTable & generateEventToNameTable()
        {
            static EventToNameTable table = {
    #define EV_LIST_ENTRY(ev, comment) EvInfo{ entity_event_t::ev, #ev, comment, true },
                    #include <JKAProto/data/JKAEvents.inc>
    #undef EV_LIST_ENTRY
            };

            for (int32_t i = 0; i < ENTITY_EVENTS_COUNT; i++) {
                if (!table[i].known) {
                    table[i].value = static_cast<entity_event_t>(i);
                    table[i].name = getUnknownEventName(i);
                }
            }

            return table;
        }

        inline const EventToNameTable & getEventToNameTable()
        {
            static const EventToNameTable & table = generateEventToNameTable();
            return table;
        }

        inline NameToEventTable generateNameToEventTable()
        {

#define EV_LIST_ENTRY(ev, comment) { #ev, entity_event_t::ev },
            NameToEventTable table = {
                #include <JKAProto/data/JKAEvents.inc>
            };
#undef EV_LIST_ENTRY

            const auto & eventToNameTable = getEventToNameTable();

            for (int32_t i = 0; i < ENTITY_EVENTS_COUNT; i++) {
                if (!eventToNameTable[i].known) {
                    table.try_emplace(getUnknownEventName(i), static_cast<entity_event_t>(i));
                }
            }

            return table;
        }

        inline const NameToEventTable & getNameToEventTable()
        {
            static auto table = generateNameToEventTable();
            return table;
        }
    }

    std::string_view getEntityEventName(int32_t ev)
    {
        assert(ev >= 0 && ev < ENTITY_EVENTS_COUNT);

        if (ev >= 0 && ev < ENTITY_EVENTS_COUNT) {
            return detail::getEventToNameTable()[ev].name;
        } else {
            return "INVALID_EVENT_VALUE";
        }
    }

    std::string_view getEntityEventName(entity_event_t ev)
    {
        return getEntityEventName(static_cast<int32_t>(ev));
    }

    std::optional<entity_event_t> getEntityEventByName(std::string_view evName)
    {
        const auto & table = detail::getNameToEventTable();
        auto it = table.find(evName);
        if (it != table.end()) {
            return it->second;
        } else {
            return detail::tryParseUnknownEventName(evName);
        }
    }
}
