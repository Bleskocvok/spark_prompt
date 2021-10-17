
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


static auto COLBEGIN = "\\[\033["s;
static auto COLEND = "m\\]"s;


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


struct converter
{
    std::string param_bit3;
    std::string param_rgb;
    uint8_t shift = 0;

    std::string operator()(bit3 col)
    {
        auto num = static_cast<uint8_t>(col);
        return COLBEGIN
                + param_bit3
                + std::to_string(num == 0 ? 0 : num + shift)
                + COLEND;
    }

    std::string operator()(rgb col)
    {
        return COLBEGIN
                + param_rgb
                + std::to_string(col.r)
                + ";"
                + std::to_string(col.g)
                + ";"
                + std::to_string(col.b)
                + COLEND;
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



// enum class con_type
// {
//     exit_code,
//     login,
//     hostname,
//     path,
//     string,
// };


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
    static const auto delimiter = '|';
    static const auto rgb_delimiter = ',';
    static const auto assign = ':';
    static const auto hostname_allowed = "abcdefghijklmnopqrstuvwxyz0123456789-"sv;
    static const auto alpha = "abcdefghijklmnopqrstuvwxyz"sv;
    static const auto num = "0123456789"sv;

    auto until = [&](char end) -> std::string
    {
        auto result = std::string{};

        while (!str.empty())
        {
            char ch = str.front();
            str.remove_prefix(1);

            if (ch == end)
                break;

            result += ch;
        }
        return result;
    };

    auto whitespace = [&]() -> void
    {
        auto isspace = [&](char ch) -> bool
        {
            return std::isspace(static_cast<unsigned char>(ch));
        };

        while (!str.empty() && isspace(str.front()))
        {
            str.remove_prefix(1);
        }
    };

    auto symbol = [&](char ch) -> bool
    {
        if (str.empty())
            return false;

        if (str.front() != ch)
            return false;
        
        str.remove_prefix(1);
        return true;
    };

    auto next_one_of = [&](std::string_view options) -> bool
    {
        if (str.empty())
            return false;

        return options.find(str.front()) != options.npos;
    };

    auto parse = [&](std::string_view allowed) -> std::string
    {
        auto result = std::string{};
        while (!str.empty() && allowed.find(str.front())
                                != allowed.npos)
        {
            result += str.front();
            str.remove_prefix(1);
        }
        return result;
    };

    auto parse_color = [&]() -> color
    {
        if (next_one_of(alpha))
        {
            auto str = parse(alpha);
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
        if (next_one_of(num))
        {
            rgb col;
            col.r = std::stoi(parse(num));
            whitespace();
            if (!symbol(rgb_delimiter))
                return col;
            whitespace();
            col.g = std::stoi(parse(num));
            whitespace();
            if (!symbol(rgb_delimiter))
                return col;
            whitespace();
            col.b = std::stoi(parse(num));
            return col;
        }
        // error
        return bit3::white;
    };

    while (!str.empty())
    {
        symbol(delimiter);

        whitespace();
        auto name = parse(hostname_allowed);
        whitespace();
        if (!symbol(assign))
            return "missing "s + assign;

        whitespace();
        auto fg = parse_color();
        whitespace();
        if (!symbol(assign))
            return "missing "s + assign;
        whitespace();
        auto bg = parse_color();
        whitespace();

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
