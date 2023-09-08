#include "AdvancedCommandExecutor.h"

namespace JKA::executor
{
    AdvancedCommandExecutor::Command AdvancedCommandExecutor::parseCommandString(std::string_view commandString)
    {
        return CommandParser::parseCommand(commandString);
    }

    CommandExecutionResult AdvancedCommandExecutor::execute(const Command& command)
    {
        auto it = overloads.find(command.name);
        if (it == overloads.end()) {
            return CommandExecutionResult::fail_unknown_command();
        }
        return it->second.invoke(command);
    }
}
