#pragma once

// c
#include <cctype>       // tolower

// cpp
#include <string_view>  // string_view
#include <string>       // string
#include <stdexcept>    // runtime_error
#include <algorithm>    // remove_if


inline void remove_redundant(std::string& str)
{
    for (unsigned i = 0; i + 3 < str.size(); ++i)
    {
        if (str[i]     == '\\'
         && str[i + 1] == ']'
         && str[i + 2] == '\\'
         && str[i + 3] == '[')
         {
            str[i]     = '\0';
            str[i + 1] = '\0';
            str[i + 2] = '\0';
            str[i + 3] = '\0';
         }
    }

    str.erase(std::remove_if(str.begin(), str.end(),
                             [](char c){ return c == '\0'; }),
              str.end());
}


inline int hex_to_dec(std::string_view str)
{
    using namespace std::literals;

    auto low = [](unsigned char c){ return std::tolower(c); };
    static const auto hexval = "0123456789abcdef"sv;

    int res = 0;
    int ex = 1;
    while (!str.empty())
    {
        char c = str.back();
        str.remove_suffix(1);

        auto i = hexval.find(low(c));
        if (i == hexval.npos)
            throw std::runtime_error("invalid hex character '"s + c + "'");

        res += ex * i;
        ex *= 16;
    }
    return res;
}
