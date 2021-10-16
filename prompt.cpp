
// standard C includes
#include <cstdlib>      // getenv
#include <cstdio>
#include <cstdint>      // uint8_t

// standard C++ includes
#include <string>       // to_string
#include <string_view>
#include <iostream>
#include <vector>
#include <variant>

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

    return "🞮";
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
        result.erase(0, home.size());
        result.insert(0, "~");
    }

    // cool arrow, but kinda wide
    // result.clear();
    // for (char ch : std::string_view(buffer))
    // {
    //     result += ch == '/' ? " ❯ "s : std::string(1, ch);
    // }

    return result;
}


int main(int argc, char** argv)
{
    int ret = 0;

    if (argc >= 2)
    {
        ret = std::atoi(argv[1]);
    }

    // auto segments = std::vector<segment>
    // {
    //     { .str = exit_symbol(ret), .fg = exit_color(ret) },
    //     { .str = " " },
    //     { .str = username(), .fg = bit3::green },
    //     { .str = hostname(), .fg = bit3::white },
    //     { .str = path(), .fg = bit3::blue },
    //     { .str = "$ " },
    // };

    // for (size_t i = 0; i < segments.size(); i++)
    // {
    //     std::cout << fg_color_str(segments[i].fg) << segments[i].str;
    // }


    // magenta: rgb{ 50, 64, 168 }


    // powerline
    auto segments = std::vector<segment>
    {
        { .str = exit_symbol(ret), .fg = bit3::white, .bg = exit_color(ret) },
        { .str = username(), .fg = bit3::white, .bg = rgb{ 5, 82, 158 } },
        { .str = hostname(), .fg = bit3::white, .bg = rgb{ 107, 105, 97 } },
        { .str = path(),     .fg = bit3::white, .bg = rgb{ 5, 82, 158 } },
    };

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

    std::cout << fg_color_str(bit3::reset) << " \n";

    return 0;
}
