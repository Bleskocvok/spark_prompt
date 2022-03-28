// custom includes
#include "style.hpp"
#include "color.hpp"
#include "utils.hpp"
#include "parse_utils.hpp"
#include "function.hpp"
#include "standard.hpp"
#include "parse.hpp"
#include "unicode.hpp"

// standard C includes
#include <cstdlib>      // getenv
#include <cstdio>
#include <cstdint>      // uint8_t
#include <cctype>       // isspace

// standard C++ includes
#include <string>       // to_string, stoi
#include <string_view>
#include <iostream>
#include <vector>
#include <variant>
#include <optional>
#include <algorithm>    // find_if
#include <sstream>      // ostringstream


int main(int argc, char** argv)
{
    using namespace std::literals;

    if (!set_locale())
        return 1;

    bool validate = false;
    int exit_code = 0;
    for (int i = 1; i < argc; i++)
    {
        auto arg = std::string_view{ argv[i] };
        if (arg.find("--") == 0)
        {
            if (arg.substr(2) == "preview")
            {
                USE_INVIS = false;
            }
            else if (arg.substr(2) == "validate")
            {
                validate = true;
                USE_INVIS = false;
            }
        }
        else
        {
            exit_code = std::stoi(std::string{ arg });
        }
    }

    functions funcs{};
    funcs.add<username_t>("username");
    funcs.add<hostname_t>("hostname");
    funcs.add<pwd_t>("pwd");
    funcs.add<exit_t>("exit");

    funcs.for_each([&](auto& f) { f.exit_code(exit_code); });

    static const auto theme_def = std::string_view
    {
        "[~ \\exit(✓, ×)                    ~] >> "
        "[~ {white;#05529e} \\username()    ~] <: "
        "[~ {white;4,56,107} '@' \\hostname ~] << "
        " ~~ "
        "[~ {#ffffff;5,82,158} \\pwd        ~] :> "
    };

    auto env = getenv("SPARK_THEME");
    auto theme = std::string{ env == nullptr ? "" : env };
    theme = theme.empty() ? theme_def : theme;
    parsed pr{ theme };

    auto r = parse_style(pr, funcs);

    if (const auto* err = std::get_if<error>(&r))
    {
        if (!validate)
            return std::cout << "error: " << *err << " |> " << "\n", 1;

        // bool use_color = isatty(STDOUT_FILENO);
        bool use_color = true;

        std::cout << "" << theme << "\n";

        size_t idx = pr.read_bytes();
        idx += size_diff(theme.substr(0, idx));
        for (size_t i = 0; i < idx; i++)
            std::cout << " ";
        std::cout << (use_color ? fg_color(bit3::red, "^~~") : "^~~"s) << "\n";

        std::cout << "error (" << idx << "): "
                  << (use_color ? fg_color(bit3::red, err->message())
                                : err->message())
                  << "\n";
        return 1;
    }

    auto stl = std::get<style>(r);
    auto out = std::ostringstream{};
    stl.render(out);

    auto str = out.str();
    remove_redundant(str);

    std::cout << str << fg_color_str(bit3::reset) << " \n";

    return 0;
}
