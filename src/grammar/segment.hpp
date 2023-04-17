#pragma once

// custom
#include "color.hpp"

// cpp
#include <ostream>      // ostream
#include <string>       // string


enum class sep : unsigned
{
    empty,
    space,
    powerline,
    powerline_space,
    powerline_pseudo,
    newline,

    rpowerline,
    rpowerline_space,
    rpowerline_pseudo,
};


void render_sep(std::ostream& out, sep s, color fg, color bg, color next_bg);

size_t sep_len(sep s);


struct theme
{
    color fg = bit3::white,
          bg = bit3::reset;

    effect ef = effect::none;
};


struct segment
{
    std::string str;

    theme th;

    sep end = sep::powerline;

    bool h_space = false;

    size_t width() const;
};
