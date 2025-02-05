#include "utils.hpp"
#include "spark.hpp"

// cpp
#include <iostream>


void output_style(const style& stl)
{
    auto out = std::ostringstream{};
    stl.render(out);

    auto str = out.str();
    remove_redundant(str);

    std::cout << str << fg_color_str(bit3::reset) << " \n";
}
