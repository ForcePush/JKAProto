#pragma once
#include <array>
#include <string>
#include <string_view>
#include <type_traits>

#include "SharedDefs.h"
#include "utility/Span.h"
#include "jka/JKAConstants.h"

namespace JKA {
    class ReliableCommandsStore {
    public:
        ReliableCommandsStore() noexcept = default;
        ReliableCommandsStore(const ReliableCommandsStore & other) = default;
        ReliableCommandsStore(ReliableCommandsStore && other) noexcept : ReliableCommandsStore()
        {
            swap(*this, other);
        }

        ReliableCommandsStore & operator=(ReliableCommandsStore other) noexcept
        {
            swap(*this, other);
            return *this;
        }

        template<typename StateTA, typename StateTB>
        friend
            std::enable_if_t<
                std::is_same_v<std::remove_reference_t<StateTA>, ReliableCommandsStore> &&
                std::is_same_v<std::remove_reference_t<StateTB>, ReliableCommandsStore>
            >
        swap(StateTA && a, StateTB && b) noexcept
        {
            using std::swap;
            swap(a.reliableCommands, b.reliableCommands);
            swap(a.serverCommands, b.serverCommands);
        }

        void reset() noexcept
        {
            swap(*this, ReliableCommandsStore());
        }

        std::string & reliableCommand(size_t sequence) & noexcept
        {
            return reliableCommands[reliableCommandsIdx(sequence)];
        }

        const std::string & reliableCommand(size_t sequence) const & noexcept
        {
            return reliableCommands[reliableCommandsIdx(sequence)];
        }

        std::string & serverCommand(size_t sequence) & noexcept
        {
            return serverCommands[serverCommandsIdx(sequence)];
        }

        const std::string & serverCommand(size_t sequence) const & noexcept
        {
            return serverCommands[serverCommandsIdx(sequence)];
        }

    private:
        size_t reliableCommandsIdx(size_t sequence) const noexcept
        {
            return sequence % reliableCommands.size();
        }

        size_t serverCommandsIdx(size_t sequence) const noexcept
        {
            return sequence % reliableCommands.size();
        }

        std::array<std::string, MAX_RELIABLE_COMMANDS> reliableCommands{};
        std::array<std::string, JKA::MAX_RELIABLE_COMMANDS> serverCommands{};
    };
}
