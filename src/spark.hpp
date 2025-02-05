#pragma once

#include "grammar/parsing.hpp"
#include "grammar/grammar.hpp"
#include "grammar/evaluate.hpp"
#include "utils.hpp"
#include "unicode.hpp"
#include "builtin.hpp"

// cpp
#include <iostream>
#include <optional>
#include <string>
#include <utility>      // move


void output_style(const style& stl);


template<typename Prefix, typename Err>
void output_error(const Prefix& prefix, const Err& err)
{
    std::cerr << prefix << err << "\n";
}
