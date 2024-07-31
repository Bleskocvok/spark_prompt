// // custom includes
// #include "style.hpp"
// #include "color.hpp"
// #include "utils.hpp"
// #include "unicode.hpp"

// // standard C includes
// #include <cstdlib>      // getenv
// #include <cstdint>      // uint8_t
// #include <cctype>       // isspace

// // standard C++ includes
// #include <string>       // to_string, stoi, ""s
// #include <string_view>  // string_view
// #include <iostream>     // cout
// #include <vector>       // vector
// #include <variant>      // variant, get, get_if
// #include <optional>     // optional
// #include <algorithm>    // find_if
// #include <sstream>      // ostringstream


// int main(int argc, char** argv)
// {
//     using namespace std::literals;

//     if (!set_locale())
//         return 1;

//     bool validate = false;
//     int exit_code = 0;
//     for (int i = 1; i < argc; i++)
//     {
//         auto arg = std::string_view{ argv[i] };
//         if (arg.find("--") == 0)
//         {
//             if (arg.substr(2) == "preview")
//             {
//                 USE_INVIS = false;
//             }
//             else if (arg.substr(2) == "validate")
//             {
//                 validate = true;
//                 USE_INVIS = false;
//             }
//         }
//         else
//         {
//             exit_code = std::stoi(std::string{ arg });
//         }
//     }

//     functions funcs{};
//     funcs.add<username_t>("username");
//     funcs.add<hostname_t>("hostname");
//     funcs.add<pwd_t>("pwd");
//     funcs.add<exit_t>("exit");

//     funcs.for_each([&](auto& f) { f.exit_code(exit_code); });

//     static const auto theme_def = std::string_view
//     {
//         "[~ \\exit(✓, ×)                    ~] >> "
//         "[~ {white;#05529e} \\username()    ~] :> "
//         "[~ {white;4,56,107} '@' \\hostname ~] >> "
//         // " ~~ "
//         "[~ {#ffffff;5,82,158} \\pwd        ~] :> "
//     };

//     auto env = std::getenv("SPARK_THEME");
//     auto theme = std::string{ env == nullptr ? "" : env };
//     theme = theme.empty() ? theme_def : theme;
//     parsed pr{ theme };

//     auto r = parse_style(pr, funcs);

//     if (const auto* err = std::get_if<parse_error>(&r))
//     {
//         if (!validate)
//             return std::cout << "error: " << err->msg << " |> " << "\n", 1;

//         // bool use_color = isatty(STDOUT_FILENO);
//         bool use_color = true;

//         std::cout << "" << theme << "\n";

//         size_t idx = pr.read_bytes();
//         idx += size_diff(theme.substr(0, idx));
//         for (size_t i = 0; i < idx; i++)
//             std::cout << " ";
//         std::cout << (use_color ? fg_color(bit3::red, "^~~") : "^~~"s) << "\n";

//         std::cout << "error (" << idx << "): "
//                   << (use_color ? fg_color(bit3::red, err->msg) : err->msg)
//                   << "\n";
//         return 1;
//     }

//     auto stl = std::get<style>(r);
//     auto out = std::ostringstream{};
//     stl.render(out);

//     auto str = out.str();
//     remove_redundant(str);

//     std::cout << str << fg_color_str(bit3::reset) << " \n";

//     return 0;
// }




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


struct params_t
{
    bool preview = false;
    bool validate = false;
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
        else if (arg == "--validate" || arg == "-v")
        {
            params.validate = true;
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


inline void output_style(const style& stl)
{
    auto out = std::ostringstream{};
    stl.render(out);

    auto str = out.str();
    remove_redundant(str);

    std::cout << str << fg_color_str(bit3::reset) << " \n";
}


template<typename Prefix, typename Err>
void output_error(const Prefix& prefix, const Err& err)
{
    std::cerr << prefix << err << "\n";
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

    static const auto default_code =
        "[ { #ffffff (if (exit) #4F7D27 #750404) '' }"
            "(if (exit) ' ✓ ' ' × ') >> ]"
        "[ { #ffffff #005BBB '' } (fmt ' ' (user) ' ') :> ]"
        "[ { #000000 #FFD500 '' } (fmt ' ' (host) ' ') >> ]"
        "[ { #ffffff #083B6D '' } (fmt ' ' (pwd_limited 35) ' ') >> ]"
        "[ { #ffffff #005BBB '' } '' :> ]"
        "[ { #000000 #FFD500 '' } '' :> ]"

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

    if (params.theme)
        code = *params.theme;

    auto in = input(code);

    auto parser = p_sequence{};

    maybe<std::vector<node_ptr>> parsed = parser(in);

    if (!parsed)
    {
        output_error("PARSE ERROR: ", parsed.get_fail());
        return 1;
    }

    auto node = std::move(parsed.get());
    auto eval = evaluator{};

    add_builtin(eval, params.exit_code);

    maybe<style> result = eval(std::move(node));

    int r = result.visit([](const style& st)
    {
        output_style(st);
        return 0;
    },
    [](const fail& f)
    {
        output_error("EXECUTION ERROR: ", f);
        return 1;
    });

    return r;
}
