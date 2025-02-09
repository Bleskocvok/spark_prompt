#pragma once

#include "grammar/parsing.hpp"
#include "style/style.hpp"

// cpp
#include <iostream>
#include <string>


void output_style(const style& stl);

maybe<style> parse_evaluate_style(int exit_code, const std::string& code);


template<typename Prefix, typename Err>
void output_error(const Prefix& prefix, const Err& err)
{
    std::cerr << prefix << err << "\n";
}

void show_examples();

void functions_help();
