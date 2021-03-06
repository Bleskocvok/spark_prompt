
#include "style.hpp"

#include <iostream>     // cout
#include <utility>      // pair, move


using namespace std::literals;
using namespace std::string_literals;


void style::render(std::ostream& out) const
{
    static const auto arrow = "\uE0B0"s;
    static const auto pseudo = "▶"s;

    for (size_t i = 0; i < segments.size(); i++)
    {
        out << bg_color_str(segments[i].bg)
                  << fg_color_str(segments[i].fg);

        if (segments[i].sp_before)
            out << " ";

        out << segments[i].str;

        if (segments[i].sp_after)
            out << " ";

        color next = i != segments.size() - 1 ? segments[i + 1].bg
                                              : bit3::reset;

        switch (segments[i].end)
        {
            case sep::empty:
                out << fg_color_str(bit3::reset);
                break;

            case sep::space:
                out << fg_color_str(bit3::reset)
                    << " ";
                break;

            case sep::powerline:
                out << bg_color_str(next)
                    << fg_color_str(segments[i].bg)
                    << arrow;
                break;

            case sep::powerline_space:
            {
                // TODO: add option to change color of “thick”
                color thick = rgb{ 0, 0, 0 };
                out << bg_color_str(thick)
                    << fg_color_str(segments[i].bg)
                    << arrow
                    << fg_color_str(thick)
                    << bg_color_str(next)
                    << arrow;
                break;
            }

            case sep::powerline_pseudo:
                out << bg_color_str(next)
                    << fg_color_str(segments[i].bg)
                    << pseudo;
                break;

            case sep::newline:
                out << bg_color_str(bit3::reset)
                    << "\n";
                break;

            case sep::horizontal_space:
                // TODO
                break;
        }
    }
}


static std::variant<color, error> parse_color(parsed& pr)
{
    using namespace std::literals;
    using namespace std::string_literals;

    static const auto rgb_delimiter = ',';
    static const auto alpha = "abcdefghijklmnopqrstuvwxyz"sv;
    static const auto num = "0123456789"sv;

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

    return error{ "invalid color" };
}


static std::variant<std::pair<color, color>, error> parse_colors(parsed& pr)
{
    pr.whitespace();

    auto col = parse_color(pr);
    if (const error* err = std::get_if<error>(&col))
        return *err;

    color fg = std::get<color>(col);

    pr.whitespace();

    if (!pr.symbol(';'))
        return error{ "expected ;" };

    pr.whitespace();

    col = parse_color(pr);

    if (const error* err = std::get_if<error>(&col))
        return *err;

    color bg = std::get<color>(col);

    return std::pair{ fg, bg };
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
        else if (pr.symbol('{'))
        {
            auto res = parse_colors(pr);

            if (const error* err = std::get_if<error>(&res))
                return *err;

            pr.whitespace();

            if (!pr.symbol('}'))
                return error{ "expected closing }" };

            auto [fg, bg] = std::get<std::pair<color, color>>(res);

            result.fg = fg;
            result.bg = bg;
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
