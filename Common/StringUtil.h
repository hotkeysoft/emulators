#pragma once

#include <string_view>
#include <string>
#include <algorithm>
#include <cctype>
#include <locale>

class StringUtil
{
public:
    static std::string& ToUpper(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        return str;
    }
    static std::string ToUpper(std::string str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::toupper);
        return str;
    }

    static std::string& ToLower(std::string& str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    }
    static std::string ToLower(std::string str)
    {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        return str;
    }

    static bool EndsWith(std::string_view str, std::string_view suffix)
    {
        return (str.size() >= suffix.size()) && (str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0);
    }

    static bool StartsWith(std::string_view str, std::string_view prefix)
    {
        return (str.size() >= prefix.size()) && (str.compare(0, prefix.size(), prefix) == 0);
    }

    static inline void TrimLeft(std::string& str)
    {
        str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
            return !std::isspace(ch);
            }));
    }

    static inline void TrimRight(std::string& str)
    {
        str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
                return !std::isspace(ch);
                }).base(), str.end());
    }

    static inline void Trim(std::string& str)
    {
        TrimRight(str);
        TrimLeft(str);
    }

    static inline std::string TrimLeftCopy(std::string str)
    {
        TrimLeft(str);
        return str;
    }

    static inline std::string TrimRightCopy(std::string str)
    {
        TrimRight(str);
        return str;
    }

    static inline std::string TrimCopy(std::string str)
    {
        Trim(str);
        return str;
    }

};
