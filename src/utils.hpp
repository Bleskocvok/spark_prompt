#pragma once

#include <algorithm>
#include <cctype>
#include <string_view>
#include <string>


inline void remove_redundant(std::string& str)
{
    const auto redundant = std::string{ "\\]\\[" };
    auto found = decltype(str.find(redundant)){};
    while ((found = str.find(redundant)) != str.npos)
    {
        str.erase(found, redundant.size());
    }
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
            return -1;

        res += ex * i;
        ex *= 16;
    }
    return res;
}
