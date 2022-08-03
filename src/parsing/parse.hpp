#pragma once

// custom
#include "parsed.hpp"
#include "style.hpp"
#include "function.hpp"

// cpp
#include <variant>      // variant


struct parse_error
{
    size_t line;
    size_t col;

    std::string msg;
};


std::variant<style, parse_error> parse_style(parsed& pr, functions& funcs);


