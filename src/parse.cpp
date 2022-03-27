#include "parse.hpp"

#include "utils.hpp"

#include <iostream>     // cout
#include <utility>      // pair, move
#include <unordered_map>
#include <cctype>


static std::variant<color, error> parse_color(parsed& pr)
{
    using namespace std::literals;
    using namespace std::string_literals;

    static const auto rgb_delimiter = ',';
    static const auto alpha = "abcdefghijklmnopqrstuvwxyz"sv;
    static const auto num = "0123456789"sv;
    static const char hash = '#';
    static const auto hexnum = "0123456789aAbBcCdDeEfF"sv;


    static const auto colors = std::unordered_map<std::string, color>
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

    if (pr.next_one_of(alpha))
    {
        auto str = pr.parse(alpha);
        if (auto it = colors.find(str); it != colors.end())
            return it->second;
        return error{ "invalid color" };
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

    if (pr.symbol(hash))
    {
        auto num = pr.parse(hexnum);
        if (num.empty())
            return error{ "hex number cannot be empty" };
        int dec = hex_to_dec(num);
        if (dec < 0 || dec > 255 * 255 * 255)
            return error{ "invalid hex color" };
        uint8_t b = dec % 256;
        uint8_t g = (dec /= 256) % 256;
        uint8_t r = (dec /= 256) % 256;
        return rgb{ r, g, b };
    }

    return error{ "invalid color" };
}


static std::variant<theme, error> parse_theme(parsed& pr)
{
    static const char sep = ';';

    pr.whitespace();

    auto col = parse_color(pr);
    if (const error* err = std::get_if<error>(&col))
        return *err;

    color fg = std::get<color>(col);

    pr.whitespace();

    if (!pr.symbol(sep))
        return error{ "expected ;" };

    pr.whitespace();

    col = parse_color(pr);

    if (const error* err = std::get_if<error>(&col))
        return *err;

    color bg = std::get<color>(col);

    return theme{ fg, bg, effect::none };
}


static std::variant<segment, error> parse_def(parsed& pr, functions& funcs)
{
    return error{ "not implt" };
}


static std::variant<std::string, error> parse_func(parsed& pr, functions& funcs)
{
    return error{ "not implt" };
}


static std::variant<segment, error> parse_segment(parsed& pr, functions& funcs)
{
    using namespace std::literals;
    using namespace std::string_literals;

    static const auto alpha = "abcdefghijklmnopqrstuvwxyz"sv;
    static const char seg_open  = '[';
    static const char seg_close = ']';
    static const char fun_open  = '(';
    static const char fun_close = ')';

    segment result{};

    pr.whitespace();

    if (!pr.symbol(seg_open))
        return error{ "expected "s + seg_open };

    pr.whitespace();

    while (!pr.symbol(seg_close))
    {
        pr.whitespace();

        if (pr.empty())
        {
            return error{ "end reached, but expected "s + seg_close };
        }
        else if (pr.symbol('\\'))
        {
            auto func = pr.parse(alpha);
            auto args = std::vector<std::string>{};

            if (pr.symbol(fun_open))
            {
                while (!pr.symbol(fun_close) && !pr.empty())
                {
                    pr.whitespace();
                    auto arg = pr.until_one_of("),");
                    pr.symbol(',');  // consume comma
                    args.push_back(std::move(arg));
                }
            }

            auto ret = funcs.call(func, result, std::move(args));
            if (const auto* err = std::get_if<func::f_err>(&ret))
                return error{ err->msg };

            result.str += std::get<std::string>(ret);
        }
        else if (pr.symbol('\''))
        {
            auto str = pr.until('\'');
            result.str += str;
        }
        else if (pr.symbol('"'))
        {
            auto str = pr.until('"');
            result.str += str;
        }
        else if (pr.symbol('{'))
        {
            auto res = parse_theme(pr);

            if (const error* err = std::get_if<error>(&res))
                return *err;

            pr.whitespace();

            if (!pr.symbol('}'))
                return error{ "expected closing }" };

            result.th = std::get<theme>(res);
        }
        else
        {
            return error{ "invalid token" };
        }

        pr.whitespace();
    }

    return result;
}


static std::variant<std::vector<segment>, error> parse_segments(parsed& pr,
                                                               functions& funcs)
{
    auto result = std::vector<segment>{};

    bool before = false;

    while (!pr.empty())
    {
        auto var = parse_segment(pr, funcs);
        if (const error* err = std::get_if<error>(&var))
            return *err;
        auto seg = std::get<segment>(var);

        bool sp_after  = pr.whitespace();
        auto between   = pr.parse(":>n\\~");

        sep end = sep::empty;
        if (between.empty() && sp_after)
            end = sep::space;
        else if (between == ":>")
            end = sep::powerline;
        else if (between == ">")
            end = sep::powerline_pseudo;
        else if (between == ">>")
            end = sep::powerline_space;
        else if (between == "\\n")
            end = sep::newline;
        else if (between == "~~~")
            end = sep::horizontal_space;
        else
            return error{ "invalid separator" };

        bool sp_before = pr.whitespace();
        bool fin       = pr.next_one_of("[|");

        seg.end       = end;
        seg.sp_before = before;
        seg.sp_after  = sp_after  && end != sep::space;
        before        = sp_before && end != sep::space && fin;

        result.push_back(std::move(seg));

        // end if found
        pr.symbol('|');
    }
    return result;
}


std::variant<style, error> parse_style(parsed& pr, functions& funcs)
{
    auto r = parse_segments(pr, funcs);
    if (const auto* err = std::get_if<error>(&r))
        return *err;

    return style{ std::move(std::get<std::vector<segment>>(r)) };
}

