
#pragma once

#include "parsing.hpp"

#include <memory>       // shared_ptr, make_shared
#include <vector>       // vector
#include <variant>      // variant, visit
#include <string>       // string
#include <array>        // array
#include <cstdint>      // uint8_t
#include <cassert>      // assert
#include <utility>      // move
#include <ostream>      // ostream

#include <iomanip>      // setw, setfill, quoted
#include <ios>          // hex



namespace std
{
    template<typename T>
    std::ostream& operator<<(std::ostream& out, const std::vector<T>& vec)
    {
        out << "[";

        const char* sep = "";
        for (const auto& val : vec)
        {
            out << sep << val;
            sep = ",";
        }

        return out << "]";
    }
}



struct rgb
{
    std::uint8_t r, g, b;
};

enum class effect : std::uint8_t
{
    bold, blink
};

enum class sep
{
    empty,
    space,
    powerline,
    powerline_space,
    powerline_pseudo,
    newline,
    fill,

    r_powerline,
    r_powerline_space,
    r_powerline_pseudo,
};


struct literal_string;
struct literal_color;
struct literal_effect;
struct literal_separator;
struct literal_bool;
struct composite_color;
struct composite_segment;
struct call;

using node = std::variant<literal_string, literal_color, literal_effect,
                          literal_separator, literal_bool, composite_color,
                          composite_segment, call>;
using node_ptr = std::shared_ptr<node>;


// combined parser
struct p_node;
// other parsers
struct p_literal_string;
struct p_literal_color;
struct p_literal_effect;
struct p_literal_separator;
struct p_literal_bool;
struct p_composite_color;
struct p_composite_segment;
struct p_call;


template<typename T, typename... Args>
node_ptr make_node(Args&&... args)
{
    return std::make_shared<node>(T(std::forward<Args>(args)...));
}


template<typename Beg,
         typename End,
         typename Func,
         typename T, template<typename> typename Vec>
std::ostream& print_separated(std::ostream& out,
                              const Beg& beg,
                              const End& end,
                              const Vec<T>& vec,
                              Func print_elem)
{
    out << beg;
    const char* sep = "";
    for (const auto& elem : vec)
    {
        out << sep;
        sep = " ";
        print_elem(elem);
    }
    return out << end;
}


struct literal_string
{
    std::string data;
    literal_string(std::string data) : data(std::move(data)) {}

    friend std::ostream& operator<<(std::ostream& out, const literal_string& a)
    {
        return out << std::quoted(a.data, '"', '\\');
    }
};

struct literal_color
{
    rgb data;
    literal_color(rgb data) : data(data) {}

    friend std::ostream& operator<<(std::ostream& out, const literal_color& a)
    {
        using std::setfill, std::hex, std::setw;

        return out << "#" << setfill('0') << hex << setw(2) << int(a.data.r)
                          << setfill('0') << hex << setw(2) << int(a.data.g)
                          << setfill('0') << hex << setw(2) << int(a.data.b);
    }
};

struct literal_effect
{
    effect data;
    literal_effect(effect data) : data(data) {}

    friend std::ostream& operator<<(std::ostream& out, const literal_effect& a)
    {
        (void) a;
        return out;
    }
};

struct literal_separator
{
    std::string data;
    literal_separator(std::string data) : data(std::move(data)) {}

    friend std::ostream& operator<<(std::ostream& out,
                                    const literal_separator& a)
    {
        return out << a.data;
    }
};

struct literal_bool
{
    bool value;
    literal_bool(bool value) : value(value) {}

    friend std::ostream& operator<<(std::ostream& out, const literal_bool& a)
    {
        return out << (a.value ? "true" : "false");
    }
};

struct composite_color
{
    std::vector<node_ptr> args;
    composite_color(std::vector<node_ptr> args) : args(std::move(args)) {}

    friend std::ostream& operator<<(std::ostream&, const composite_color&);
};

struct composite_segment
{
    std::vector<node_ptr> args;
    composite_segment(std::vector<node_ptr> args) : args(std::move(args)) {}

    friend std::ostream& operator<<(std::ostream&, const composite_segment&);
};

struct call
{
    std::string name;
    std::vector<node_ptr> args;
    call(std::string name, std::vector<node_ptr> args)
        : name(std::move(name)), args(std::move(args)) {}

    friend std::ostream& operator<<(std::ostream&, const call&);
};



inline std::ostream& operator<<(std::ostream& out,
                                const composite_color& a)
{
    return print_separated(out, "{ ", " }", a.args,
                [&](const auto& ptr)
                {
                    std::visit([&](const auto& val) { out << val; }, *ptr);
                });
}


inline std::ostream& operator<<(std::ostream& out, const call& a)
{
    out << "(" << a.name;

    if (!a.args.empty())
        out << " ";

    return print_separated(out, "", ")", a.args,
                [&](const auto& ptr)
                {
                    std::visit([&](const auto& val) { out << val; }, *ptr);
                });
}


inline std::ostream& operator<<(std::ostream& out, const composite_segment& a)
{
    return print_separated(out, "[ ", " ]", a.args,
                [&](const auto& ptr)
                {
                    std::visit([&](const auto& val) { out << val; }, *ptr);
                });
}


inline std::ostream& operator<<(std::ostream& out, node_ptr obj)
{
    std::visit([&](const auto& val) { out << val; }, *obj);
    return out;
}



// -----------------------------------------------------------------------------
// PARSERS
// -----------------------------------------------------------------------------



struct p_literal_string : p_parser<node_ptr>
{
    p_try_seq<std::string, p_quoted<'"',  '\\'>,
                           p_quoted<'\'', '\\'>> parser;

    maybe<node_ptr> operator()(input& in)
    {
        return parser(in)
            .fmap([](auto str)
            {
                return make_node<literal_string>(std::move(str));
            });
    }
};

struct p_literal_color : p_parser<node_ptr>
{
    p_prefixed<p_char<'#'>,
               p_many<p_one_pred<is_hex>, 6, 6>> parser;

    maybe<node_ptr> operator()(input& in)
    {
        constexpr auto hex_to_dec = []() -> std::array<int, 256>
        {
            auto t = std::array<int, 256>{ 0 };
            t['0'] =  0; t['1'] =  1; t['2'] =  2; t['3'] =  3; t['4'] =  4;
            t['5'] =  5; t['6'] =  6; t['7'] =  7; t['8'] =  8; t['9'] =  9;
            t['a'] = 10; t['b'] = 11; t['c'] = 12; t['d'] = 13; t['e'] = 14;
            t['f'] = 15;
            t['A'] = 10; t['B'] = 11; t['C'] = 12; t['D'] = 13; t['E'] = 14;
            t['F'] = 15;
            return t;
        }();

        auto get_hex = [&](char a, char b)
        {
            return hex_to_dec[a] * 16 + hex_to_dec[b];
        };

        return parser(in)
            .fmap([&](auto vec)
            {
                assert(vec.size() == 6);
                rgb color;
                color.r = get_hex(vec[0], vec[1]);
                color.g = get_hex(vec[2], vec[3]);
                color.b = get_hex(vec[4], vec[5]);
                return make_node<literal_color>(color);
            });
    }
};

struct p_literal_effect : p_parser<node_ptr>
{
    p_try_seq<std::string,
              p_string<'b', 'o', 'l', 'd'>
              > parser;

    maybe<node_ptr> operator()(input& in)
    {
        return parser(in)
            .fmap([&](auto& str)
            {
                return make_node<literal_separator>(std::move(str));
            });
    }
};

struct p_literal_separator : p_parser<node_ptr>
{
    p_try_seq<std::string,
              p_string<':', '>'>,   // powerline
              p_string<'<', ':'>,
              p_string<'>', '>'>,   // thick powerline
              p_string<'/', '/'>,   // slope
              p_string<'\\', '\\'>,
              p_string<'<', '<'>,
              p_string<'~'>,        // space
              p_string<'-', '-'>,   // fill
              p_string<'V'>         // newline
              > parser;

    maybe<node_ptr> operator()(input& in)
    {
        return parser(in)
            .fmap([&](auto& str)
            {
                return make_node<literal_separator>(std::move(str));
            });
    }
};

struct p_literal_bool : p_parser<node_ptr>
{
    p_try_seq<std::string,
              p_string<'t', 'r', 'u', 'e'>,
              p_string<'f', 'a', 'l', 's', 'e'>
              > parser;

    maybe<node_ptr> operator()(input& in)
    {
        return parser(in)
            .fmap([&](const std::string& str)
            {
                return make_node<literal_bool>(str == "true");
            });
    }
};

struct p_composite_color : p_parser<node_ptr>
{
    maybe<node_ptr> operator()(input& in);
};

struct p_composite_segment : p_parser<node_ptr>
{
    maybe<node_ptr> operator()(input& in);
};

struct p_call : p_parser<node_ptr>
{
    maybe<node_ptr> operator()(input& in);
};

struct p_node : p_parser<node_ptr>
{
    p_node() = default;

    maybe<node_ptr> operator()(input& in)
    {
        p_spaces_before<p_try_seq<node_ptr,
                                  p_spaces_after<p_literal_string>,
                                  p_spaces_after<p_literal_color>,
                                  p_spaces_after<p_literal_bool>,
                                  p_spaces_after<p_call>,
                                  p_spaces_after<p_composite_color>,
                                  p_spaces_after<p_composite_segment>
                                 >
                        > parser;

        return parser(in);
    }
};


maybe<node_ptr> p_call::operator()(input& in)
{
    auto parser = p_between<p_suffixed<p_char<'('>, p_space>,
                            p_after<p_suffixed<p_alpha_str<1>, p_space>,
                                    p_many<p_node>>,
                            p_char<')'>>{};

    return parser(in)
        .fmap([&](auto pair)
        {
            return make_node<call>(pair.first, std::move(pair.second));
        });
}


maybe<node_ptr> p_composite_color::operator()(input& in)
{
    auto parser = p_between<p_char<'{'>,
                            p_many<p_node>,
                            p_char<'}'>>{};

    return parser(in)
        .fmap([&](auto nodes)
        {
            return make_node<composite_color>(std::move(nodes));
        });
}

maybe<node_ptr> p_composite_segment::operator()(input& in)
{
    auto parser = p_between<p_char<'['>,
                            p_many<p_node>,
                            p_char<']'>>{};

    return parser(in)
        .fmap([&](auto nodes)
        {
            return make_node<composite_segment>(std::move(nodes));
        });
}
