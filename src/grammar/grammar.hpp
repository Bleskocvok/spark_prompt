
#pragma once

#include "parsing.hpp"

#include <memory>       // shared_ptr, make_shared
#include <vector>       // vector
#include <variant>      // variant, visit
#include <string>       // string
#include <cstdint>      // uint8_t
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
struct call;

using node = std::variant<literal_string, literal_color, literal_effect,
                          literal_separator, literal_bool, composite_color,
                          call>;
using node_ptr = std::shared_ptr<node>;


// parsers
struct p_literal_string;
struct p_literal_color;
struct p_literal_effect;
struct p_literal_separator;
struct p_literal_bool;
struct p_composite_color;
struct p_call;
using p_node = p_try_seq<node_ptr, p_literal_string>;


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
    sep data;
    literal_separator(sep data) : data(data) {}

    friend std::ostream& operator<<(std::ostream& out,
                                    const literal_separator& a)
    {
        (void) a;
        return out;
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
    out << "{";
    const char* sep = "";
    for (const auto& val : a.args)
    {
        out << sep << val;
        sep = " ";
    }
    return out << "}";
}


inline std::ostream& operator<<(std::ostream& out, const call& a)
{
    out << "(" << a.name;

    if (!a.args.empty())
        out << " ";

    const char* sep = "";
    for (const auto& val : a.args)
    {
        out << sep << val;
        sep = " ";
    }
    return out << ")";
}


inline std::ostream& operator<<(std::ostream& out, node_ptr obj)
{
    std::visit([&](const auto& val) { out << val; }, *obj);
    return out;
}



struct p_literal_string : p_parser<node_ptr>
{
    p_quoted parser;

    p_literal_string() : parser('"', '\\')
    { }

    maybe<node_ptr> operator()(input& in)
    {
        return parser(in)
            .fmap([](auto str)
            {
                return std::make_shared<node>(literal_string(std::move(str)));
            });
    }
};

struct p_literal_color : p_parser<node_ptr>
{

};

struct p_literal_effect : p_parser<node_ptr>
{

};

struct p_literal_separator : p_parser<node_ptr>
{

};

struct p_literal_bool : p_parser<node_ptr>
{

};

struct p_composite_color : p_parser<node_ptr>
{

};

struct p_call : p_parser<node_ptr>
{

};

