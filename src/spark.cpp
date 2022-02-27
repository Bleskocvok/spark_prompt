// custom includes
#include "style.hpp"
#include "color.hpp"
#include "parse.hpp"
#include "function.hpp"
#include "standard.hpp"
#include "style.hpp"

// standard C includes
#include <cstdlib>      // getenv
#include <cstdio>
#include <cstdint>      // uint8_t
#include <cctype>       // isspace

// standard C++ includes
#include <string>       // to_string, stoi
#include <string_view>
#include <iostream>
#include <vector>
#include <variant>
#include <optional>
#include <algorithm>    // find_if


using namespace std::literals;
using namespace std::string_literals;


void print_prompt(const std::vector<segment>& segments)
{
    static const auto arrow = "\uE0B0"s;
    static const auto pseudo = "▶"s;

    for (size_t i = 0; i < segments.size(); i++)
    {
        std::cout << bg_color_str(segments[i].bg)
                  << fg_color_str(segments[i].fg);

        if (segments[i].sp_before)
            std::cout << " ";
        
        std::cout << segments[i].str;

        if (segments[i].sp_after)
            std::cout << " ";

        color next = i != segments.size() - 1 ? segments[i + 1].bg
                                              : bit3::reset;

        switch (segments[i].end)
        {
            case sep::empty:
                std::cout << fg_color_str(bit3::reset);
                break;

            case sep::space:
                std::cout << fg_color_str(bit3::reset)
                          << " ";
                break;

            case sep::powerline:
                std::cout << bg_color_str(next)
                          << fg_color_str(segments[i].bg)
                          << arrow;
                break;

            case sep::powerline_space:
            {
                // TODO: add option to change color of “thick”
                color thick = rgb{ 0, 0, 0 };
                std::cout << bg_color_str(thick)
                          << fg_color_str(segments[i].bg)
                          << arrow
                          << fg_color_str(thick)
                          << bg_color_str(next)
                          << arrow;
                break;
            }

            case sep::powerline_pseudo:
                std::cout << bg_color_str(next)
                          << fg_color_str(segments[i].bg)
                          << pseudo;
                break;
        }
    }
}


int main(int argc, char** argv)
{
    int exit_code = 0;
    if (argc >= 2)
        exit_code = std::atoi(argv[1]);

    if (argc >= 3 && argv[2] == std::string{ "--preview" })
        USE_INVIS = false;

    functions funcs{};
    funcs.add<username_t>("username");
    funcs.add<hostname_t>("hostname");
    funcs.add<pwd_t>("pwd");
    funcs.add<exit_t>("exit");

    funcs.for_each([&](auto& f) { f.exit_code(exit_code); });

    static const auto theme_def = std::string_view
    {
        "[ ' ' \\exit(✓, ×) ] >> [ {white;5,82,158} \\username() ] :> [ {white;4, 56, 107} '@' \\hostname ] >> [ {255,255,255;5,82,158} \\pwd ] :> "
    };

    auto env = getenv("SPARK_THEME");
    auto theme = std::string{ env == nullptr ? "" : env };
    parsed pr{ theme.empty() ? theme_def : theme };

    auto r = parse_segments(pr, funcs);

    if (const auto* err = std::get_if<error>(&r))
        return std::cout << "error: " << *err << " |> " << "\n", 1;

    auto segments = std::get<std::vector<segment>>(r);

    print_prompt(segments);

    std::cout << fg_color_str(bit3::reset) << " \n";

    return 0;
}
