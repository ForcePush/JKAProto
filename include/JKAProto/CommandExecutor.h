#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <map>
#include <vector>

#include "CommandParser.h"

namespace JKA {
    class CommandExecutor {
    public:
        using Command = CommandParser::Command;
        using Arguments = std::vector<CommandParser::Argument>;
        using Callback = std::function<void(const Command & command)>;

        CommandExecutor() = default;
        CommandExecutor(const CommandExecutor &) = default;
        CommandExecutor(CommandExecutor &&) noexcept = default;
        CommandExecutor & operator=(const CommandExecutor &) = default;
        CommandExecutor & operator=(CommandExecutor &&) noexcept = default;
        ~CommandExecutor() = default;

        Command parseCommandString(std::string_view commandString);
        void addCommand(std::string_view command, const Callback & callback);

        template<typename ThisType>
        void addCommand(std::string_view command, ThisType *thisPtr, void(ThisType::* memFn)(const Command & command))
        {
            addCommand(command, [thisPtr, memFn](const Command & cmd) mutable {
                std::invoke(memFn, thisPtr, cmd);
            });
        }

        bool execute(const Command & command);

    private:
        std::map<std::string, Callback, std::less<>> commands{};
    };
}
