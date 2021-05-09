#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <map>
#include <vector>

#include "CommandParser.h"

class CommandExecutor {
public:
    using Command = CommandParser::Command;
    using Arguments = std::vector<CommandParser::Argument>;
    using Callback = std::function<void(const Command & command)>;

    CommandExecutor() = default;
    CommandExecutor(const CommandExecutor &) = default;
    CommandExecutor(CommandExecutor &&) = default;
    CommandExecutor & operator=(const CommandExecutor &) = default;
    CommandExecutor & operator=(CommandExecutor &&) = default;
    ~CommandExecutor() = default;

    Command parseCommandString(std::string_view commandString);
    void addCommand(std::string_view command, const Callback & callback);
    bool execute(const Command & command);

private:
    std::map<std::string, Callback, std::less<>> commands{};
};
