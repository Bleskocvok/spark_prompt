#pragma once

// custom
#include "parse_utils.hpp"
#include "color.hpp"
#include "segment.hpp"
#include "function.hpp"

// cpp
#include <vector>
#include <ostream>


struct style
{
    std::vector<segment> segments;

    void render(std::ostream& out) const;
};


