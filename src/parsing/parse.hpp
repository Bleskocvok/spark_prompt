#pragma once

// custom
#include "parse_utils.hpp"
#include "style.hpp"
#include "function.hpp"

// cpp
#include <variant>      // variant


std::variant<style, error> parse_style(parsed& pr, functions& funcs);


