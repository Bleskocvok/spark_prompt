
#pragma once

#include <cstdint>      // uint8_t
#include <variant>
#include <string>
#include <optional>
#include <string_view>


inline auto INVBEG = std::string{ "\\[" };
inline auto INVEND = std::string{ "\\]" };

inline auto COLBEGIN = std::string{ "\033[" };
inline auto COLEND   = std::string{ "m" };

inline bool USE_INVIS = true;


enum class bit3 : uint8_t
{
    black   = 30,
    red     = 31,
    green   = 32,
    yellow  = 33,
    blue    = 34,
    magenta = 35,
    cyan    = 36,
    white   = 37,
    reset   = 0,
};


struct rgb
{
    uint8_t r = 255,
            g = 255,
            b = 255;
};


using color = std::variant<bit3, rgb>;


inline std::string wrap_invis(const std::string& str)
{
    if (!USE_INVIS)
        return str;

    return INVBEG + str + INVEND;
}

std::string fg_color_str(color col);

std::string bg_color_str(color col);

std::string fg_color(bit3 col, const std::string& str);

std::string bg_color(bit3 col, const std::string& str);

std::optional<std::string> parse_color(std::string_view);
