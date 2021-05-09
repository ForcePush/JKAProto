#pragma once
#include <cinttypes>
#include <optional>
#include <string_view>

namespace JKA {
    enum {
        HI_NONE,

        HI_SEEKER,
        HI_SHIELD,
        HI_MEDPAC,
        HI_MEDPAC_BIG,
        HI_BINOCULARS,
        HI_SENTRY_GUN,
        HI_JETPACK,

        HI_HEALTHDISP,
        HI_AMMODISP,
        HI_EWEB,
        HI_CLOAK,

        HI_NUM_HOLDABLE
    };
    typedef int holdable_t;


    typedef enum {
        CTFMESSAGE_FRAGGED_FLAG_CARRIER,
        CTFMESSAGE_FLAG_RETURNED,
        CTFMESSAGE_PLAYER_RETURNED_FLAG,
        CTFMESSAGE_PLAYER_CAPTURED_FLAG,
        CTFMESSAGE_PLAYER_GOT_FLAG
    } ctfMsg_t;

    // reward sounds (stored in ps->persistant[PERS_PLAYEREVENTS])
    constexpr int32_t PLAYEREVENT_DENIEDREWARD = 0x0001;
    constexpr int32_t PLAYEREVENT_GAUNTLETREWARD = 0x0002;

    // entityState_t->event values
    // entity events are for effects that take place reletive
    // to an existing entities origin.  Very network efficient.

    // two bits at the top of the entityState->event field
    // will be incremented with each change in the event so
    // that an identical event started twice in a row can
    // be distinguished.  And off the value with ~EV_EVENT_BITS
    // to retrieve the actual event number
    constexpr size_t EV_EVENT_BIT1 = 0x00000100;
    constexpr size_t EV_EVENT_BIT2 = 0x00000200;
    constexpr size_t EV_EVENT_BITS = (EV_EVENT_BIT1 | EV_EVENT_BIT2);

    // rww: not an actual JKA defines

    constexpr size_t EV_BITS_IN_EVENT = 8;
    constexpr size_t EV_BITS_IN_NONCE = 2;

    constexpr int32_t EVENT_VALID_MSEC = 300;

    typedef enum {
        PDSOUND_NONE,
        PDSOUND_PROTECTHIT,
        PDSOUND_PROTECT,
        PDSOUND_ABSORBHIT,
        PDSOUND_ABSORB,
        PDSOUND_FORCEJUMP,
        PDSOUND_FORCEGRIP
    } pdSounds_t;

    enum class entity_event_t : int32_t {
#define EV_LIST_ENTRY(ev, comment) ev,
#include "../data/JKAEvents.inc"
#undef EV_LIST_ENTRY

        MAX_ENTITY_EVENT = 255
    };  // There is a maximum of 256 events (8 bits transmission, 2 high bits for uniqueness)

    constexpr int32_t ENTITY_EVENTS_COUNT = 256;

    std::string_view getEntityEventName(int32_t ev);
    std::string_view getEntityEventName(entity_event_t ev);
    std::optional<entity_event_t> getEntityEventByName(std::string_view evName);
}
