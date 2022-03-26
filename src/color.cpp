
#include "color.hpp"


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


std::string effect_string(effect ef)
{
    if (ef == effect::none)
        return "";
    
    return std::to_string(uint8_t(ef)) + ";";
}


std::string fg_color_str(color col, effect ef)
{
    return std::visit(converter{ effect_string(ef), "38;2;", 0 }, col);
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


