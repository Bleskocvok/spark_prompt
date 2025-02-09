#pragma once

#include "style/style.hpp"

#include <iostream>

class renderer
{

public:
    virtual ~renderer() = default;

    virtual void render(const style& stl, std::ostream& out) const = 0;
};
