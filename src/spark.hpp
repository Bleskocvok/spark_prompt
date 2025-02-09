#pragma once

#include "grammar/parsing.hpp"
#include "grammar/grammar.hpp"
#include "eval/evaluate.hpp"
#include "style/style.hpp"
#include "utils.hpp"
#include "unicode.hpp"
#include "builtin.hpp"

// cpp
#include <iostream>
#include <optional>
#include <string>
#include <utility>      // move


void output_style(const style& stl);

maybe<style> parse_evaluate_style(int exit_code, const std::string& code);


template<typename Prefix, typename Err>
void output_error(const Prefix& prefix, const Err& err)
{
    std::cerr << prefix << err << "\n";
}

void show_examples();

void functions_help();
