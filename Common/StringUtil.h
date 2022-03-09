#pragma once

#include <string_view>

class StringUtil
{
public:
    static bool EndsWith(std::string_view str, std::string_view suffix)
    {
        return (str.size() >= suffix.size()) && (str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0);
    }

    static bool StartsWith(std::string_view str, std::string_view prefix)
    {
        return (str.size() >= prefix.size()) && (str.compare(0, prefix.size(), prefix) == 0);
    }
};
