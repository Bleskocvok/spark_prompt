#pragma once

#include "renderer.hpp"

// cpp
#include <ostream>      // ostream

// linux
#include <sys/ioctl.h>  // ioctl

class bash_renderer : public renderer
{

public:
    void render(const style& stl, std::ostream& out) const override;
};
