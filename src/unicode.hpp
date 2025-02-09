#pragma once

// c
#include <clocale>      // set_locale, mbstate_t
#include <cwchar>       // mbrlen
#include <cstddef>      // size_t

// cpp
#include <string_view>  // string_view


inline bool set_locale(const char* loc = "")
{
    return std::setlocale(LC_ALL, loc);
}


inline std::size_t unicode_size(std::string_view str)
{
    std::mbstate_t mb{};
    std::size_t size;
    for (size = 0; !str.empty(); size++)
    {
        std::size_t ch_size = std::mbrlen(&str.front(), str.size(), &mb);
        str.remove_prefix(ch_size);
    }
    return size;
}


inline std::size_t size_diff(std::string_view str)
{
    return unicode_size(str) - str.length();
}
