
#pragma once

#include "parse.hpp"
#include "color.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <vector>


struct configuration
{
    enum
    {
        powerline,
        pseudo_powerline,
        normal,
    } style = normal;

    bool double_line = false;

    struct host_theme
    {
        std::string contains;
        color fg = bit3::white,
              bg = bit3::reset;
    };

    std::vector<host_theme> host_themes;
};


std::optional<std::string> parse_host_themes(
        std::string_view str,
        std::vector<configuration::host_theme>& themes)
{
    using namespace std::literals;
    using namespace std::string_literals;

    static const auto delimiter = '|';
    static const auto rgb_delimiter = ',';
    static const auto assign = ':';
    static const auto hostname_allowed = "abcdefghijklmnopqrstuvwxyz0123456789-"sv;
    static const auto alpha = "abcdefghijklmnopqrstuvwxyz"sv;
    static const auto num = "0123456789"sv;

    parsed pr{ str };

    auto parse_color = [&]() -> color
    {
        if (pr.next_one_of(alpha))
        {
            auto str = pr.parse(alpha);
            if (str == "black")
                return bit3::black;
            if (str == "red")
                return bit3::red;
            if (str == "green")
                return bit3::green;
            if (str == "yellow")
                return bit3::yellow;
            if (str == "blue")
                return bit3::blue;
            if (str == "magenta")
                return bit3::magenta;
            if (str == "cyan")
                return bit3::cyan;
            if (str == "white")
                return bit3::white;
            if (str == "reset")
                return bit3::reset;
            // error
            return bit3::white;
        }
        if (pr.next_one_of(num))
        {
            rgb col;
            col.r = std::stoi(pr.parse(num));
            pr.whitespace();
            if (!pr.symbol(rgb_delimiter))
                return col;
            pr.whitespace();
            col.g = std::stoi(pr.parse(num));
            pr.whitespace();
            if (!pr.symbol(rgb_delimiter))
                return col;
            pr.whitespace();
            col.b = std::stoi(pr.parse(num));
            return col;
        }
        // error
        return bit3::white;
    };

    while (!str.empty())
    {
        pr.symbol(delimiter);

        pr.whitespace();
        auto name = pr.parse(hostname_allowed);
        pr.whitespace();
        if (!pr.symbol(assign))
            return "missing "s + assign;

        pr.whitespace();
        auto fg = parse_color();
        pr.whitespace();
        if (!pr.symbol(assign))
            return "missing "s + assign;
        pr.whitespace();
        auto bg = parse_color();
        pr.whitespace();

        themes.push_back({ name, fg, bg });
    }

    return std::nullopt;
}
