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

maybe<style> parse_evaluate_style(int exit_code, const std::string& code);


template<typename Prefix, typename Err>
void output_error(const Prefix& prefix, const Err& err)
{
    std::cerr << prefix << err << "\n";
}

inline void show_examples()
{
    auto examples = std::vector<std::string>
    {
        "[ { #ffffff (if (exit) #4F7D27 #750404) '' }"
            "(if (exit) ' ✓ ' ' × ') >> ]"
        "[ { #ffffff #005BBB '' } (fmt ' ' (user) ' ') :> ]"
        "[ { #000000 #FFD500 '' } (fmt ' ' (host) ' ') >> ]"
        "[ { #ffffff #083B6D '' } (fmt ' ' (pwd_limited 35) ' ') >> ]"
        "[ { #ffffff #005BBB '' } '' :> ]"
        "[ { #000000 #FFD500 '' } '' :> ]",

R"END([ { #ffffff (if (exit) #005BBB #750404) '' }
        (if (exit) ' ✓ ' ' × ') >> ]
        [ { #000000 #FFD500 '' } (fmt ' ' (host) ' ') >> ]
        [ { #ffffff #083B6D '' } (fmt ' ' (pwd_limited 35) ' ') >> ]
        [ { #000000 #FFD500 '' } '' :> ]
)END",

R"END(
        [ { #bfbfbf (if (exit) #565656 #750404) '' }
        (if (exit) ' ✓ ' ' × ') >> ]
        [ { #bfbfbf #2b2b2b '' } (fmt ' ' (host) ' ') >> ]
        [ { #dfdfdf #1e1e1e '' } (fmt ' ' (pwd_limited 35) ' ') :> ]
)END",

R"END(
        [ { #bfbfbf (if (exit) #565656 #750404) '' }
            (if (exit) ' ✓ ' ' × ') | ]
        [ { #bfbfbf (if (exit) #2b2b2b #330202)  '' } (fmt ' ' (host) ' ') | ]
        [ { #dfdfdf (if (exit) #1e1e1e #490303)  '' } (fmt ' ' (pwd_limited 35) ' ') | ]
        [ { #000000 (if (exit) #565656 #330202)  '' } ' # ' | ]
)END",

        R"END([ { #ffffff (if (exit) #4F7D27 #750404) '' }
        (if (exit) ' ✓ ' ' × ') | ]
        [ { #000000 #FFD500 '' } (fmt ' ' (host) ' ') | ]
        [ { #ffffff #083B6D '' } (fmt ' ' (pwd_limited 35) ' ') | ]
        [ { #000000 #FFD500 '' } ' # ' | ])END",
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

    bool fst = true;
    for (const auto& code : examples)
    {
        if (!fst)
            std::cout << "\n\n";
        fst = false;

        std::cout << "\"" << code << "\"\n";

        showcase(0, code);
        showcase(1, code);
    }
}
