#include <iostream>

#include "grammar/parsing.hpp"
#include "unicode.hpp"
#include "spark.hpp"

// cpp
#include <optional>
#include <string>


struct params_t
{
    bool preview = false;
    bool validate = false;
    bool examples = false;
    bool help = false;
    int exit_code = 0;
    std::optional<std::string> theme;
};


inline params_t parse_params(int argc, const char* const* argv)
{
    params_t params;

    for (int i = 1; i < argc; i++)
    {
        auto arg = std::string_view{ argv[i] };

        if (arg == "--preview" || arg == "-p")
        {
            params.preview = true;
        }
        else if (arg == "--help" || arg == "-h")
        {
            params.help = true;
        }
        else if (arg == "--validate" || arg == "-v")
        {
            params.validate = true;
        }
        else if (arg == "--examples" || arg == "-e")
        {
            params.examples = true;
        }
        else if (arg == "--theme" || arg == "-t")
        {
            if (i == argc)
                throw std::runtime_error("--theme THEME\nmissing operand");
            params.theme = argv[i + 1];
            ++i;
        }
        else
        {
            params.exit_code = std::stoi(std::string{ arg });
        }
    }

    return params;
}


int main(int argc, char** argv)
{
    using namespace std::literals;

    if (!set_locale())
        return 1;

    params_t params;
    try
    {
        params = parse_params(argc, argv);
    }
    catch (std::exception& ex)
    {
        output_error("ERROR: ", ex.what());
        return 1;
    }

    // TODO: solve edge cases
    USE_INVIS = !params.preview && !params.validate;

    // TODO: examples
    // add to examples
    // static const auto default_code =
    //     "[ { #eeeeEe #ff11ff '' } (user) >> ]"
    //     "[ { #eeeeEe #bb00bb '' } (host) >> ]"
    //     "[ { #eeeeEe #ff11ff '' } (pwd_limited 35)  :> ]"s;

    static const char default_code[] =
        "[ { #ffffff (if (exit) #4F7D27 #750404) '' }"
            "(if (exit) ' ✓ ' ' × ') >> ]"
        "[ { #ffffff #005BBB '' } (fmt ' ' (user) ' ') > ]"
        "[ { #000000 #FFD500 '' } (fmt ' ' (host) ' ') >> ]"
        "[ { #ffffff #083B6D '' } (fmt ' ' (pwd_limited 35) ' ') >> ]"
        "[ { #ffffff #005BBB '' } '' > ]"
        "[ { #000000 #FFD500 '' } '' > ]"

        // "[ { #ffffff #005BBB '' } '' :> ]"
        // "[ { #000000 #FFD500 '' } '' :> ]"
        // "[ { #ffffff #005BBB '' } '' :> ]"
        // "[ { #000000 #FFD500 '' } '' :> ]"
        // "[ { #ffffff #005BBB '' } '' :> ]"
        // "[ { #000000 #FFD500 '' } '' :> ]"
        // "[ { #ffffff #005BBB '' } '' :> ]"
        // "[ { #000000 #FFD500 '' } '' :> ]"
        // "[ { #ffffff #005BBB '' } '' :> ]"
        // "[ { #000000 #FFD500 '' } '' :> ]"
        // "[ { #ffffff #005BBB '' } '' :> ]"
        // "[ { #000000 #FFD500 '' } '' :> ]"
        // "[ { #ffffff #005BBB '' } '' :> ]"
        // "[ { #000000 #FFD500 '' } '' :> ]"
        ;


    const char* env_value = std::getenv("SPARK_THEME");
    auto code = env_value == nullptr ? default_code
                                     : std::string{ env_value };

    if (params.help)
    {
        functions_help();
        return 0;
    }

    if (params.examples)
    {
        USE_INVIS = false;
        show_examples();
        return 0;
    }

    if (params.theme)
        code = *params.theme;

    maybe<style> result = parse_evaluate_style(params.exit_code, code);

    return result.visit([](const style& st)
    {
        output_style(st);
        return 0;
    },
    [](const fail& f)
    {
        output_error("EXECUTION ERROR: ", f);
        return 1;
    });
}
