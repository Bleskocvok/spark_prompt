
#pragma once

#include <cstdint>      // uint8_t
#include <variant>
#include <string>
#include <optional>
#include <string_view>


using sv = std::string_view;


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


std::string wrap_invis(const std::string& str)
{
    if (!USE_INVIS)
        return str;

    return INVBEG + str + INVEND;
}


using color = std::variant<bit3, rgb>;


struct converter
{
    std::string param_bit3;
    std::string param_rgb;
    uint8_t shift = 0;

    std::string operator()(bit3 col)
    {
        auto num = static_cast<uint8_t>(col);
        return wrap_invis(
                    COLBEGIN
                    + param_bit3
                    + std::to_string(num == 0 ? 0 : num + shift)
                    + COLEND);
    }

    std::string operator()(rgb col)
    {
        return wrap_invis(
                    COLBEGIN
                    + param_rgb
                    + std::to_string(col.r)
                    + ";"
                    + std::to_string(col.g)
                    + ";"
                    + std::to_string(col.b)
                    + COLEND);
    }
};


std::string fg_color_str(color col)
{
    return std::visit(converter{ "1;", "38;2;", 0 }, col);
}


std::string bg_color_str(color col)
{
    return std::visit(converter{ "", "48;2;", 10 }, col);
}


std::string fg_color(bit3 col, const std::string& str)
{
    return fg_color_str(col) + str + fg_color_str(bit3::reset);
}


std::string bg_color(bit3 col, const std::string& str)
{
    return bg_color_str(col) + str + bg_color_str(bit3::reset);
}


std::optional<std::string> parse_color(std::string_view)
{
    return std::nullopt;
}
