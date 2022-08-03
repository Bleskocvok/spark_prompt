#pragma once

// custom
#include "segment.hpp"
#include "function.hpp"

// cpp
#include <vector>       // vector
#include <ostream>      // ostream


struct style
{
    std::vector<segment> segments;

    void render(std::ostream& out) const;
};


