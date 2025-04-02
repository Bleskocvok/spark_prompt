#include "spark.hpp"

#include "bash_renderer.hpp"
#include "grammar/parsing.hpp"
#include "eval/builtin.hpp"
#include "style/color.hpp"
#include "utils.hpp"
#include "grammar/parsing.hpp"
#include "grammar/grammar.hpp"
#include "eval/evaluate.hpp"
#include "style/style.hpp"
#include "utils.hpp"

// cpp
#include <iostream>
#include <string>
#include <vector>
#include <utility>

void output_style(const style& stl)
{
    auto out = std::ostringstream{};

    bash_renderer renderer;

    renderer.render(stl, out);

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

void functions_help()
{
    auto eval = evaluator{};

    add_builtin(eval, 0);

    auto& out = std::cout;

    out << "FUNCTIONS\n";

    for (const auto& f : eval.funcs)
    {
        auto desc = f.second->arguments_description();
        out << "    (" << f.second->name() << (desc.empty() ? "" : " ")
            << desc
            << ")\n";
    }
}

void show_examples()
{
    auto examples = std::vector<std::string>
    {
R"END(
[ { #ffffff (if (exit) #4F7D27 #750404) '' } (if (exit) ' ✓ ' ' × ') >> ]
[ { #ffffff #005BBB '' } (fmt ' ' (user) ' ') > ]
[ { #000000 #FFD500 '' } (fmt ' ' (host) ' ') >> ]
[ { #ffffff #083B6D '' } (fmt ' ' (pwd_limited 35) ' ') >> ]
[ { #ffffff #005BBB '' } '' > ]
[ { #000000 #FFD500 '' } '' > ]
)END",

R"END(
[ { #ffffff (if (exit) #005BBB #750404) '' } (if (exit) ' ✓ ' ' × ') >> ]
[ { #000000 #FFD500 '' } (fmt ' ' (host) ' ') >> ]
[ { #ffffff #083B6D '' } (fmt ' ' (pwd_limited 35) ' ') >> ]
[ { #000000 #FFD500 '' } '' > ]
)END",

R"END(
[ { #bfbfbf (if (exit) #565656 #750404) '' } (if (exit) ' ✓ ' ' × ') >> ]
[ { #bfbfbf #2b2b2b '' } (fmt ' ' (host) ' ') >> ]
[ { #dfdfdf #1e1e1e '' } (fmt ' ' (pwd_limited 35) ' ') > ]
)END",

R"END(
[ { #bfbfbf (if (exit) #565656 #330202) '' } (if (exit) ' ✓ ' ' × ') | ]
[ { #bfbfbf (if (exit) #2b2b2b #490303) '' } (fmt ' ' (host) ' ') | ]
[ { #dfdfdf (if (exit) #1e1e1e #330202) '' } (fmt ' ' (pwd_limited 35) ' ') || ]
)END",

R"END(
[ { #bfbfbf (if (exit) #2d5e01 #330202) '' } (if (exit) ' ✓ ' ' × ') | ]
[ { #dfdfdf (if (exit) #4d990a #490303) '' } (fmt ' ' (host) ' ') | ]
[ { #dfdfdf (if (exit) #2d5e01 #330202) '' } (fmt ' ' (pwd_limited 35) ' ') || ]
)END",

R"END(
[ { #ffffff (if (exit) #4F7D27 #750404) '' } (if (exit) ' ✓ ' ' × ') | ]
[ { #000000 #FFD500 '' } (fmt ' ' (host) ' ') | ]
[ { #ffffff #083B6D '' } (fmt ' ' (pwd_limited 35) ' ') | ]
[ { #000000 #FFD500 '' } ' # ' > ]
)END",

R"END(
[ { #ffffff (if (exit) #4F7D27 #750404) '' } (if (exit) ' ✓ ' ' × ') | ]
[ { #000000 #aaaaaa '' } (fmt ' ' (host) ' ') > ]
[ { #aaaaaa #000000 '' } '' ~ ]
[ { #000000 #aaaaaa '' } (fmt ' ' (pwd_limited 35) ' ') < ]
[ { #000000 #083B6D '' } ' # ' / ]
)END",

R"END(
[ { #ffffff (if (exit) #4F7D27 #750404) '' } (if (exit) ' ✓ ' ' × ') | ]
[ { #000000 #aaaaaa '' } (fmt ' ' (host) ' ') > ]
[ { #aaaaaa #000000 '' } '' ~ ]
[ { #000000 #aaaaaa '' } (fmt ' ' (pwd_limited 35) ' ') < ]
[ { #000000 #083B6D '' } ' # ' \n ]
[ { #000000 #083B6D '' } ' > ' > ]
)END",

// R"END(
// [ { #ffffff (if (exit) #4F7D27 #750404) '' } (if (exit) ' ✓ ' ' × ') | ]
// [ { #000000 #aaaaaa '' } (fmt ' ' (host) ' ') > ]
// [ { #aaaaaa #000000 '' } '' ~ ]
// [ { #000000 #aaaaaa '' } (fmt ' ' (pwd_limited 35) ' ') < ]
// [ { #000000 #083B6D '' } ' # ' \n ]
// [ { #000000 #083B6D '' } (optional 'ahoj' true) > ]
// [ { #000000 #083B6D '' } (optional 'ahoj') > ]
// )END",
    };

    auto showcase = [](int exit_code, const auto& code)
    {
        parse_evaluate_style(exit_code, code)
            .visit([](const style& st)
        {
            output_style(st);
            return 0;
        },
        [](const fail& f)
        {
            output_error("ERROR: ", f);
            return 1;
        });
    };

    for (const auto& code : examples)
    {
        // std::cout << "\"" << code << "\"\n";
        std::cout << "" << code;

        showcase(0, code);
        showcase(1, code);
    }
}
