#pragma once

#include "parse_utils.hpp"
#include "style.hpp"
#include "function.hpp"

#include <variant>


std::variant<style, error> parse_style(parsed& pr, functions& funcs);


