
#pragma once

#include "parsing.hpp"

#include <memory>       // shared_ptr
#include <vector>       // vector
#include <variant>      // variant
#include <string>       // string
#include <cstdint>      // uint8_t
#include <utility>      // move


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


struct literal_string
{
    std::string data;
    literal_string(std::string data) : data(std::move(data)) {}
};

struct literal_color
{
    rgb data;
    literal_color(rgb data) : data(data) {}
};

struct literal_effect
{
    effect data;
    literal_effect(effect data) : data(data) {}
};

struct literal_separator
{
    sep data;
    literal_separator(sep data) : data(data) {}
};

struct literal_bool
{
    bool value;
    literal_bool(bool value) : value(value) {}
};

struct composite_color
{
    std::vector<node_ptr> args;
    composite_color(std::vector<node_ptr> args) : args(std::move(args)) {}
};

struct call
{
    std::string name;
    std::vector<node_ptr> args;
    call(std::string name, std::vector<node_ptr> args)
        : name(std::move(name)), args(std::move(args)) {}
};


class tree
{
    node_ptr root;
public:

};



// struct literal_string;
// struct literal_color;
// struct literal_effect;
// struct literal_separator;
// struct literal_bool;
// struct composite_color;
// struct call;

