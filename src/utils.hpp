#pragma once

// c
#include <cctype>       // tolower

// cpp
#include <string_view>  // string_view
#include <string>       // string
#include <stdexcept>    // runtime_error


// TODO: improve this awful complexity
inline void remove_redundant(std::string& str)
{
    const auto redundant = std::string{ "\\]\\[" };
    auto found = decltype(str.find(redundant)){};

    while ((found = str.find(redundant)) != str.npos)
        str.erase(found, redundant.size());
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
