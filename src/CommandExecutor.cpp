#include "../include/CommandExecutor.h"

CommandExecutor::Command CommandExecutor::parseCommandString(std::string_view commandString)
{
    return CommandParser::parseCommand(commandString);
}

// TODO: case-insensitive?
void CommandExecutor::addCommand(std::string_view command, const Callback & callback)
{
    commands[std::string(command)] = callback;
}

bool CommandExecutor::execute(const Command & command)
{
    auto it = commands.find(command.name);
    if (it == commands.end()) {
        return false;
    } else {
        it->second(command);
        return true;
    }
}
