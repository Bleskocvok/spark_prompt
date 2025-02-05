#include "grammar/parsing.hpp"
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

maybe<style> parse_evaluate_style(int exit_code, const std::string& code)
{
    auto in = input(code);

    auto parser = p_sequence{};

    maybe<std::vector<node_ptr>> parsed = parser(in);

    // if (!parsed)
    // {
    //     output_error("PARSE ERROR: ", parsed.get_fail());
    //     return 1;
    // }
    if (!parsed)
        return parsed.get_fail();

    auto node = std::move(parsed.get());
    auto eval = evaluator{};

    add_builtin(eval, exit_code);

    maybe<style> result = eval(std::move(node));
    return result;
}
