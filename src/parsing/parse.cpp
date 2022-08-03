#include "parse.hpp"

// custom
#include "utils.hpp"
#include "unicode.hpp"

// cpp
#include <utility>      // move
#include <map>          // map
#include <string>       // string, stoi
#include <vector>       // vector


static color parse_color(parsed& pr)
{
    using namespace std::literals;
    using namespace std::string_literals;

    static const auto rgb_delimiter = ',';
    static const auto alpha = "abcdefghijklmnopqrstuvwxyz"sv;
    static const auto num = "0123456789"sv;
    static const char hash = '#';
    static const auto hexnum = "0123456789aAbBcCdDeEfF"sv;
    static const auto colors = std::map<std::string, color>
    {
        { "black",   bit3::black   },
        { "red",     bit3::red     },
        { "green",   bit3::green   },
        { "yellow",  bit3::yellow  },
        { "blue",    bit3::blue    },
        { "magenta", bit3::magenta },
        { "cyan",    bit3::cyan    },
        { "white",   bit3::white   },
        { "reset",   bit3::reset   },
    };

    if (pr.is_next_one_of(alpha))
    {
        auto str = pr.try_string(alpha);
        if (auto it = colors.find(str); it != colors.end())
            return it->second;
        throw parse_exception{ "invalid color" };
    }

    if (pr.is_next_one_of(num))
    {
        rgb col;

        col.r = std::stoi(pr.try_string(num));
        pr.try_whitespace();

        if (!pr.try_symbol(rgb_delimiter))
            return col;
        pr.try_whitespace();

        col.g = std::stoi(pr.try_string(num));
        pr.try_whitespace();
        if (!pr.try_symbol(rgb_delimiter))
            return col;

        pr.try_whitespace();
        col.b = std::stoi(pr.try_string(num));
        return col;
    }

    if (pr.try_symbol(hash))
    {
        auto num = pr.try_string(hexnum);
        if (num.empty())
            throw parse_exception{ "hex number cannot be empty" };

        int dec = hex_to_dec(num);
        if (dec < 0 || dec > 256 * 256 * 256)
            throw parse_exception{ "invalid hex color" };

        uint8_t b = dec % 256;
        uint8_t g = (dec /= 256) % 256;
        uint8_t r = (dec /= 256) % 256;
        return rgb{ r, g, b };
    }

    throw parse_exception{ "invalid color" };
}


static theme parse_theme(parsed& pr)
{
    static const char sep = ';';

    pr.try_whitespace();

    color fg = parse_color(pr);

    pr.try_whitespace();

    if (!pr.try_symbol(sep))
        throw parse_exception{ "expected ;" };

    pr.try_whitespace();

    color bg = parse_color(pr);

    // TODO: parse effect

    return theme{ fg, bg, effect::none };
}


static segment parse_def(parsed& pr, functions& funcs)
{
    throw parse_exception{ "not implt" };
}


static std::string parse_func(parsed& pr, functions& funcs)
{
    throw parse_exception{ "not implt" };
}


static segment parse_segment(parsed& pr, functions& funcs)
{
    using namespace std::literals;
    using namespace std::string_literals;

    static const auto alpha = "abcdefghijklmnopqrstuvwxyz"sv;
    static const char seg_open  = '[';
    static const char seg_close = ']';
    static const char fun_open  = '(';
    static const char fun_close = ')';

    segment result{};

    pr.try_whitespace();

    pr.symbol(seg_open);

    pr.try_whitespace();

    while (!pr.try_symbol(seg_close))
    {
        pr.try_whitespace();

        if (pr.empty())
        {
            throw parse_exception{ "end reached, but expected "s + seg_close };
        }
        else if (pr.try_symbol('\\'))
        {
            auto func = pr.string(alpha);
            auto args = std::vector<std::string>{};

            if (pr.try_symbol(fun_open))
            {
                while (!pr.try_symbol(fun_close) && !pr.empty())
                {
                    pr.try_whitespace();
                    auto arg = pr.try_until("),");
                    pr.try_symbol(',');  // consume comma
                    args.push_back(std::move(arg));
                }
            }

            auto ret = funcs.call(func, result, std::move(args));
            if (const auto* err = std::get_if<func::f_err>(&ret))
                throw parse_exception{ err->msg };

            result.str += std::get<std::string>(ret);
        }
        else if (pr.try_symbol('~'))
        {
            result.str += ' ';
        }
        else if (pr.try_symbol('\''))
        {
            auto str = pr.try_until("'");
            result.str += str;
        }
        else if (pr.try_symbol('"'))
        {
            auto str = pr.try_until("\"");
            result.str += str;
        }
        else if (pr.try_symbol('{'))
        {
            result.th = parse_theme(pr);

            pr.try_whitespace();

            pr.symbol('}');
        }
        else
        {
            throw parse_exception{ "invalid token" };
        }

        // pr.try_whitespace();
    }

    return result;
}


static sep parse_sep(parsed& pr)
{
    static auto map = std::map<std::string, sep>
    {
        { {},    sep::empty },
        { "~",   sep::space },
        { ":>",  sep::powerline },
        { ">",   sep::powerline_pseudo },
        { ">>",  sep::powerline_space },
        { "\\n", sep::newline },
        { "<:",  sep::rpowerline },
        { "<",   sep::rpowerline_pseudo },
        { "<<",  sep::rpowerline_space },
    };

    auto between = pr.try_string(":><n~\\");
    auto found = map.find(between);
    if (found == map.end())
        throw parse_exception{ "invalid separator" };

    return found->second;
}


static std::vector<segment> parse_segments(parsed& pr, functions& funcs)
{
    auto result = std::vector<segment>{};

    while (!pr.empty())
    {
        segment seg = parse_segment(pr, funcs);

        pr.try_whitespace();

        seg.end = parse_sep(pr);

        pr.try_whitespace();

        if (pr.try_string("|") == "||")
            seg.h_space = true;

        result.push_back(std::move(seg));

        // // end if found
        // pr.try_symbol('|');
    }
    return result;
}


std::variant<style, parse_error> parse_style(parsed& pr, functions& funcs)
{
    try
    {
        auto seg = parse_segments(pr, funcs);
        return style{ std::move(seg) };
    }
    catch (parse_exception& ex)
    {
        // TODO: add line, col
        return parse_error{ 0, 0, std::move(ex.msg) };
    }
}

