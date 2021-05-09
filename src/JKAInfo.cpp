#include "../include/JKAInfo.h"

#include <charconv>
#include <string>
#include <sstream>

JKAInfo::JKAInfo(std::string_view info) :
    JKAInfo(JKAInfo::fromInfostring(info))
{
}

JKAInfo JKAInfo::fromInfostring(std::string_view infostring)
{
    auto jkaInfo = JKAInfo();

    if (infostring.size() < 2) {
        return jkaInfo;
    }

    std::string_view key, value;
    bool isKey = true;
    size_t idx = (infostring[0] == '\\');

    // idx is the index of the first non-backslash character in current token
    while (idx < infostring.size()) {
        size_t nextBackslash = infostring.find('\\', idx);
        if (isKey) {
            key = infostring.substr(idx, nextBackslash - idx);
        } else {
            value = infostring.substr(idx, nextBackslash - idx);
            jkaInfo[std::string(key)] = value;
        }

        isKey = !isKey;

        if (nextBackslash == infostring.npos) {
            break;
        } else {
            idx = nextBackslash + 1;
        }
    }

    return jkaInfo;
}

std::string JKAInfo::toInfostring() const
{
    std::ostringstream ss;
    for (auto && kvPair : *this) {
        ss << "\\" << kvPair.first << "\\" << kvPair.second;
    }
    return ss.str();
}

std::string_view JKAInfo::getField(std::string_view fieldName) const
{
    auto it = find(fieldName);
    if (it == end()) {
        return "";
    } else {
        // It is safe to return string_view from iterator's value since
        // std::map's iterator are invalidated on erasure only
        return it->second;
    }
}

int64_t JKAInfo::getIntField(std::string_view fieldName) const
{
    auto str = getField(fieldName);
    int64_t val = 0;
    std::from_chars(str.data(), str.data() + str.size(), val);
    return val;
}
