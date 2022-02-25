
// custom includes
#include "color.hpp"
#include "parse.hpp"

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


struct segment
{
    std::string str;

    color fg = bit3::white,
          bg = bit3::reset;
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


void print_powerline(const std::vector<segment>& segments)
{
    static const std::string arrow = "\uE0B0"s;

    for (size_t i = 0; i < segments.size(); i++)
    {
        std::cout << bg_color_str(segments[i].bg)
                  << fg_color_str(segments[i].fg)
                  << " "
                  << segments[i].str
                  << " ";

        color next = i != segments.size() - 1 ? segments[i + 1].bg
                                              : bit3::reset;

        std::cout << bg_color_str(next)
                  << fg_color_str(segments[i].bg)
                  << arrow;
    }
}


void print_pseudo_powerline(const std::vector<segment>& segments)
{
    for (size_t i = 0; i < segments.size(); i++)
    {
        std::cout << bg_color_str(segments[i].bg)
                  << fg_color_str(segments[i].fg)
                  << " "
                  << segments[i].str
                  << " ";
    }
}


void print_normal(const std::vector<segment>& segments)
{
    for (size_t i = 0; i < segments.size(); i++)
    {
        std::cout << bg_color_str(segments[i].bg)
                  << fg_color_str(segments[i].fg)
                  << segments[i].str;
    }
}


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


int main(int argc, char** argv)
{
    int ret = 0;

    if (argc >= 2)
    {
        ret = std::atoi(argv[1]);
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
        { .str = exit_symbol(ret), .fg = bit3::white, .bg = exit_color(ret) },
        { .str = username(), .fg = bit3::white, .bg = rgb{ 5, 82, 158 } },
        { .str = hostname(), .fg = theme.fg, .bg = theme.bg },
        { .str = path(),     .fg = bit3::white, .bg = rgb{ 5, 82, 158 } },
    };

    switch (config.style)
    {
        case configuration::powerline:
            print_powerline(segments);
            break;

        case configuration::pseudo_powerline:
            print_pseudo_powerline(segments);
            break;
        
        default:
            print_normal(segments);
            break;
    }

    if (config.double_line)
    {
        std::cout << fg_color_str(bit3::white) << "\n❯ $";
    }

    std::cout << fg_color_str(bit3::reset) << " \n";

    return 0;
}
