#pragma once
#include <cassert>
#include "jka/JKAStructs.h"
#include "jka/JKAEvents.h"
#include "AnimationState.h"

namespace JKA {
    // The highest two bits of the 10-bits entityState_t::event field
    // is a 'nonce' which is used to distinguish between multiple successive 
    // events of the same entity_event_type type.
    struct PackedJKAEvent {
        int32_t nonce = 0;  // Bits 8-9
        entity_event_t event = entity_event_t::EV_NONE;  // Bits 0-7

        constexpr PackedJKAEvent() noexcept = default;

        explicit constexpr PackedJKAEvent(entity_event_t event_) :
            event(event_)
        {
        }

        explicit constexpr PackedJKAEvent(int32_t engineEventValue) :
            nonce(engineEventValue & EV_EVENT_BITS),
            event(static_cast<entity_event_t>(engineEventValue & ~EV_EVENT_BITS))
        {
            assert((engineEventValue & ~EV_EVENT_BITS) < ENTITY_EVENTS_COUNT);
        }

        constexpr PackedJKAEvent(const PackedJKAEvent &) noexcept = default;

        constexpr int32_t toEngineEventValue() const noexcept
        {
            return (nonce << EV_BITS_IN_EVENT) | static_cast<int32_t>(event);
        }

        // Ignores nonce, as the original JKA does
        constexpr bool isNoneEvent() const noexcept
        {
            return event == entity_event_t::EV_NONE;
        }

        inline void setNoneEvent() noexcept
        {
            *this = PackedJKAEvent();
        }

        constexpr bool equalIgnoreNonce(entity_event_t other) const
        {
            return event == other;
        }

        constexpr bool operator==(const PackedJKAEvent & other) const
        {
            return (nonce == other.nonce) && (event == other.event);
        }

        constexpr bool operator==(int32_t engineEventValue) const
        {
            return *this == PackedJKAEvent(engineEventValue);
        }

        constexpr bool operator!=(const PackedJKAEvent & other) const
        {
            return !(*this == other);
        }

        constexpr bool operator!=(int32_t engineEventValue) const
        {
            return !(*this == engineEventValue);
        }

        constexpr bool operator<(const PackedJKAEvent & other) const
        {
            return toEngineEventValue() < other.toEngineEventValue();
        }

        constexpr bool operator<(int32_t engineEventValue) const
        {
            return toEngineEventValue() < engineEventValue;
        }
    };

    struct CEntity {
        entityState_t state{};
        // If true, then this entity is present in the current snapshot and
        // should be rendered/handled/etc.;
        // If false, then this entity is either not loaded at all or
        // WAS loaded in some previous snapshots and currently is removed
        // (e.g. other player went too far away and was unloaded... or disconnected!)
        bool valid = false;

        AnimationState torsoAnimState{};
        AnimationState legsAnimState{};

        inline bool isEventEntity() const noexcept
        {
            return state.eType > ET_EVENTS;
        }

        // Does not clear all the state
        inline void removeEntity() noexcept
        {
            valid = false;
            clearLastFiredEvent();
        }

        // Called when an entity is added to the current frame
        inline void addEntity(const entityState_t & newState)
        {
            state = newState;
            valid = true;
        }

        // Called when a valid entity is changed
        inline void changeEntity(const entityState_t & newState)
        {
            state = newState;
        }

        // If the current event is marked as fired 
        // or there is no events at all, returns EV_NONE
        inline entity_event_t getNewCurrentEvent() const noexcept
        {
            if (isEventEntity()) {  // An event entity
                // For event-entities the original code does not
                // check nonce (probably since it is easier to just spawn a new 
                // event-entity)
                if (lastFiredEvent.isNoneEvent()) {
                    return eventFromType().event;
                }
            } else {  // A regular entity
                if (lastFiredEvent != state.event) {
                    return eventFromStateEvent().event;
                }
            }

            return entity_event_t::EV_NONE;
        }

        // Get current event and set is as fired
        inline entity_event_t extractNewCurrentEvent() noexcept
        {
            auto ev = getNewCurrentEvent();
            if (ev != entity_event_t::EV_NONE) {
                setCurrentEventFired();
            }
            return ev;
        }

        // Current event is handled, set lastFiredEvent
        inline void setCurrentEventFired() noexcept
        {
            lastFiredEvent = PackedJKAEvent(currentEvent());
        }

        inline void clearLastFiredEvent() noexcept
        {
            lastFiredEvent.setNoneEvent();
        }

    private:
        // rww: a JKA server sends events as temporary entities that
        // lasts for EVENT_VALID_MSEC milliseconds, so we must ensure
        // that a single event would not fire twice

        PackedJKAEvent lastFiredEvent{ entity_event_t::EV_NONE };

        // Works only if this is an event entity
        inline PackedJKAEvent eventFromType() const noexcept
        {
            assert(isEventEntity());
            return PackedJKAEvent(state.eType - ET_EVENTS);
        }

        // Works for non-event entities
        inline PackedJKAEvent eventFromStateEvent() const noexcept
        {
            assert(!isEventEntity());
            return PackedJKAEvent(state.event);
        }

        // Works for any entity
        inline PackedJKAEvent currentEvent() const noexcept
        {
            if (isEventEntity()) {
                return eventFromType();
            } else {
                return eventFromStateEvent();
            }
        }
    };
}
