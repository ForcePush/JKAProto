#pragma once

#include <string>
#include <string_view>
#include <map>

class JKAInfo : public std::map<std::string, std::string, std::less<>>
{
public:
    using MapType = std::map<std::string, std::string, std::less<>>;

    JKAInfo() = default;
    JKAInfo(const JKAInfo &) = default;
    JKAInfo(JKAInfo &&) noexcept = default;
    explicit JKAInfo(std::string_view info);
    JKAInfo & operator=(const JKAInfo &) = default;
    JKAInfo & operator=(JKAInfo &&) = default;

    ~JKAInfo() = default;

    static JKAInfo fromInfostring(std::string_view info);
    std::string toInfostring() const;

    std::string_view getField(std::string_view fieldName) const;
    int64_t getIntField(std::string_view fieldName) const;
};
