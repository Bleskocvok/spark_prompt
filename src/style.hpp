
#pragma once

#include "parse.hpp"
#include "color.hpp"
#include "segment.hpp"
#include "function.hpp"

#include <vector>
#include <variant>      // std::get_if


using error = std::string;

struct parse_error
{
    std::string msg;
    size_t idx;
};


struct style;


std::variant<style, error> parse_style(parsed& pr, functions& funcs);


struct style
{
    std::vector<segment> segments;

    void render() const;
};


