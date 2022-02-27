
#pragma once

#include "color.hpp"


enum class sep
{
    empty,
    space,
    powerline,
    powerline_space,
    powerline_pseudo,
};


struct segment
{
    std::string str;

    color fg = bit3::white,
          bg = bit3::reset;

    sep end = sep::powerline;

    bool sp_before = true,
         sp_after = true;
};
