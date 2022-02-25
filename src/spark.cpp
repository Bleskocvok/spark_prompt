
// custom includes
#include "color.hpp"
#include "parse.hpp"
#include "themes.hpp"

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

// POSIX includes
#include <unistd.h>     // posix, geteuid, gethostname, getcwd
#include <sys/types.h>  // geteuid, getpwuid
#include <pwd.h>        // getpwuid
#include <limits.h>     // PATH_MAX

// // only linux
// #include <sys/sysinfo.h>


using namespace std::literals;
using namespace std::string_literals;


enum class sep
{
    empty,
    space,
    powerline,
    powerline_space,
    powerline_pseudo,
};


struct segment
{
    std::string str;

    color fg = bit3::white,
          bg = bit3::reset;

    sep end = sep::powerline;

    bool sp_before = true,
         sp_after = true;
};


std::string exit_symbol(int val)
{
    if (val == 0)
        return "✓";

    return "×";
}


color exit_color(int val)
{
    if (val == 0)
        return rgb{ 79, 125, 39 };

    return rgb{ 117, 4, 4 };
}


std::string username()
{
    struct passwd* user_info = getpwuid(geteuid());
    return user_info->pw_name;
}


std::string hostname()
{
    char buffer[256] = { 0 };
    gethostname(buffer, sizeof(buffer));
    return "@"s + buffer;
}


std::string path()
{
    char* buffer = getenv("PWD");
    if (buffer == nullptr)
        return "";

    std::string result = buffer;

    struct passwd* user_info = getpwuid(geteuid());
    std::string home = user_info->pw_dir;

    if (result.starts_with(home))
    {
        result.erase(1, home.size() - 1);
        result.front() = '~';
    }

    // cool arrow, but kinda wide
    // result.clear();
    // for (char ch : std::string_view(buffer))
    // {
    //     result += ch == '/' ? " ❯ "s : std::string(1, ch);
    // }

    static const size_t max_size = 35;

    if (result.size() > max_size)
    {
        result.erase(0, result.size() - max_size);
        result.insert(0, "…");
    }

    return result;
}


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
                std::cout << pseudo;
                break;
        }
    }
}


int main(int argc, char** argv)
{
    int ret = 0;

    if (argc >= 2)
    {
        ret = std::atoi(argv[1]);
    }

    if (argc >= 3 && argv[2] == std::string{ "--preview" })
    {
        USE_INVIS = false;
    }

    configuration config;
    config.style = configuration::powerline;
    config.double_line = false;

    const char* env = getenv("SPARK_HOST_THEMES");
    auto themes_str = env == nullptr ? ""sv : std::string_view(env);
    auto err = parse_host_themes(themes_str, config.host_themes);

    auto host = hostname();
    auto contains = [&](const auto& theme)
    {
        return host.find(theme.contains) != host.npos;
    };
    auto found = std::find_if(config.host_themes.begin(),
                              config.host_themes.end(),
                              contains);
    auto theme = configuration::host_theme
            { "", bit3::white, rgb{ 107, 105, 97 } };  // default theme
    if (found != config.host_themes.end())
        theme = *found;

    // powerline
    auto segments = std::vector<segment>
    {
        { .str = exit_symbol(ret),
          .fg = bit3::white,
          .bg = exit_color(ret),
          .end = sep::powerline, },

        { .str = username(),
          .fg = bit3::white,
          .bg = rgb{ 5, 82, 158 },
          .end = sep::powerline },

        { .str = hostname(),
          .fg = theme.fg,
          .bg = theme.bg,
          .end = sep::powerline },

        { .str = path(),
          .fg = bit3::white,
          .bg = rgb{ 5, 82, 158 } },
    };

    print_prompt(segments);

    if (config.double_line)
    {
        std::cout << fg_color_str(bit3::white) << "\n❯ $";
    }

    std::cout << fg_color_str(bit3::reset) << " \n";

    return 0;
}
