#include "CommandParser.h"
#include <charconv>
#include <sstream>

namespace CommandParser {
    Argument::Argument() :
        Argument("")
    {
    }

    Argument::Argument(std::string_view string) :
        data(string)
    {
        setTypeFlag(ArgType::Str);  // Always have a string value
        parseAsInt();
        parseAsFloat();
        parseAsBool();
    }

    const std::string & Argument::getStr() const
    {
        return data;
    }

    int64_t Argument::getInt64() const
    {
        return dataInt;
    }

    int32_t Argument::getInt32() const
    {
        return static_cast<int32_t>(dataInt);
    }

    float Argument::getFloat() const
    {
        return dataFloat;
    }

    bool Argument::getBool() const
    {
        return dataBool;
    }

    void Argument::setTypeFlag(ArgType flag)
    {
        type |= static_cast<TypeType>(flag);
    }

    void Argument::clearTypeFlag(ArgType flag)
    {
        type &=~ static_cast<TypeType>(flag);
    }

    void Argument::parseAsInt()
    {
        auto res = std::from_chars(data.data(), data.data() + data.size(), dataInt);
        if (res.ec == std::errc()) {
            setTypeFlag(ArgType::Int);
        }
    }

    void Argument::parseAsFloat()
    {
        // Rww: gcc still does not support float from_chars(), lol
        const char *begin = data.data();
        const char *end = begin + data.size();
        char *endPtr = nullptr;

        dataFloat = std::strtof(begin, &endPtr);
        if (endPtr == end || dataFloat != 0.0f) {
            setTypeFlag(ArgType::Float);
        } else {
            for (const char *it = endPtr; it < end; it++) {
                if (!std::isspace(static_cast<unsigned char>(*it))) {
                    return;
                }
            }
            setTypeFlag(ArgType::Float);
        }
    }

    void Argument::parseAsBool()
    {
        if (isInt()) {
            dataBool = !(getInt64() == 0);
            setTypeFlag(ArgType::Bool);
        }
    }

    bool Argument::isStr() const
    {
        return is<ArgType::Str>();
    }

    bool Argument::isInt() const
    {
        return is<ArgType::Int>();
    }

    bool Argument::isFloat() const
    {
        return is<ArgType::Float>();
    }

    bool Argument::isBool() const
    {
        return is<ArgType::Bool>();
    }

    bool Argument::operator==(const Argument & other) const
    {
        return data == other.data;
    }

    bool Argument::operator==(std::string_view str) const
    {
        return data == str;
    }

    bool Argument::operator<(const Argument & other) const
    {
        return data < other.data;
    }

    void Argument::setInt(int64_t val)
    {
        setTypeFlag(ArgType::Int);
        dataInt = val;
    }

    void Argument::setFloat(float val)
    {
        setTypeFlag(ArgType::Float);
        dataFloat = val;
    }

    void Argument::setBool(bool val)
    {
        setTypeFlag(ArgType::Bool);
        dataBool = val;
    }

    std::ostream & operator<<(std::ostream & stream, const Argument & arg)
    {
        stream << arg.getStr();
        return stream;
    }

    // =========================================================================================
    // ======================================== Parsing ========================================
    // =========================================================================================
    size_t skipChars(std::string_view str, std::string_view chars, size_t startIdx)
    {
        size_t idx = str.find_first_not_of(chars, startIdx);
        return ((idx == str.npos) ? str.size() : idx);
    }

    // Original JKA engine comments,
    // e.g. '/set name Pada/*wa*/n' -> name == 'Pada n'
    size_t skipComments(std::string_view str, size_t startIdx)
    {
        if (str.size() < startIdx + 2) {  // "//" or "/*" - 2 chars required
            return startIdx;
        }

        if (str[startIdx] != '/') {  // Not a comment
            return startIdx;
        }

        if (str[startIdx + 1] == '/') {  // One-line comment, just ignore the rest of the string
            return str.size();
        }

        if (str[startIdx + 1] != '*') {  // Not an inline comment (/* ... */)
            return startIdx;
        }

        size_t commentEndIdx = str.find("*/", startIdx + 2);  // Skip "/*" and find an end of the comment
        return (commentEndIdx == str.npos) ? str.size() : commentEndIdx + 2;
    }

    size_t advanceToNextToken(std::string_view str, std::string_view sepChars, size_t startIdx)
    {
        size_t idx = skipChars(str, sepChars, startIdx);
        idx = skipComments(str, idx);
        idx = skipChars(str, sepChars, idx);  // Skip spaces again, e.g. "first_token/*comment*/    next_token"
        return idx;
    }

    void flushToken(Command & cmd, std::ostringstream & token, bool & nameParsed)
    {
        if (nameParsed) {
            cmd.args.emplace_back(token.str());
        } else {
            cmd.name = token.str();
            nameParsed = true;
        }
        token.clear();
        token.str("");
    }

    // Checks if there is a comment start at startIdx
    bool isComment(std::string_view str, size_t startIdx)
    {
        if (str.size() < startIdx + 2) {  // "//" or "/*" - 2 chars required
            return false;
        }

        return (str[startIdx] == '/' && (str[startIdx + 1] == '/' || str[startIdx + 1] == '*'));
    }

    // Checks if there is a separator character at startIdx
    bool isSeparator(std::string_view str, std::string_view sepChars, size_t startIdx)
    {
        return (startIdx < str.size() && (sepChars.find(str[startIdx]) != sepChars.npos));
    }

    Command parseCommand(std::string_view cmd, std::string_view sepChars)
    {
        bool isInQuote = false;
        std::ostringstream token{};
        Command res{};
        bool nameParsed = false;

        size_t idx = advanceToNextToken(cmd, sepChars, 0);
        while (idx < cmd.size()) {
            char curChar = cmd[idx];

            // Found a quote character, switch isInQuote
            if (curChar == '"') {
                if (isInQuote) {  // End of a quoted string, advance to next token
                    isInQuote = false;
                    flushToken(res, token, nameParsed);
                    idx = advanceToNextToken(cmd, sepChars, idx + 1 /* Skip " */);
                } else {  // Begin of a quoted string, just skip "
                    isInQuote = true;
                    idx++;
                }
                continue;
            }

            // Found a separator or comment start, advance to next token
            // if we are not inside a quoted string
            if (!isInQuote && (isSeparator(cmd, sepChars, idx) || isComment(cmd, idx))) {
                flushToken(res, token, nameParsed);
                idx = advanceToNextToken(cmd, sepChars, idx);
                continue;
            }

            // A normal character (or we are inside a quoted string)
            token << curChar;
            idx++;
        }

        auto rest = token.str();
        if (!rest.empty()) {
            if (nameParsed) {
                res.args.emplace_back(rest);
            } else {
                res.name = rest;
            }
        }

        return res;
    }
}
