
#pragma once

#include "color.hpp"


enum class sep
{
    empty,
    space,
    powerline,
    powerline_space,
    powerline_pseudo,
    newline,
    horizontal_space,
};


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

    bool sp_before = true,
         sp_after = true;
};
