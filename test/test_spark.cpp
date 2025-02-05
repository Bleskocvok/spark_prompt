#include "spark.hpp"

#include "grammar/parsing.hpp"

#include <cstdlib>      // exit
#include <iostream>     // cerr, cout


static bool fail = false;
static int skipped = 0;
static int passed = 0;
static bool verbose = true;


template<typename Pred, typename Print>
void check(const std::string& code, int exit_code, Pred pred, Print print,
           const std::string& expected, const std::string& got)
{
    if (fail)
    {
        skipped++;
        return;
    }

    auto r = parse_evaluate_style(exit_code, code);

    if (pred(r))
    {
        passed++;

        if (verbose)
        {
            std::cerr << "OK | " << expected << " ";
            print(std::cerr, r);
            std::cerr << "\n";
        }

        return;
    }

    fail = true;
    std::cerr << "FAIL | expected: " << expected
              << ", got: " << got << "\n"
              << "    \"" << code << "\"\n";
}

static auto printer = [](auto& out, const maybe<style>& res)
{
    res.visit([&](const style&)
    {
        out << "style";
    },
    [&](const auto& f)
    {
        out << f;
    });
    // return out;
};

void check_pass(const std::string& code, int exit_code = 0)
{
    check(code, exit_code, [](const auto& r){ return !r.is_failed(); }, printer, "PASS", "FAIL");
}

void check_fail(const std::string& code, int exit_code = 0)
{
    check(code, exit_code, [](const auto& r){ return r.is_failed(); }, printer, "FAIL", "PASS");
}

int main()
{
    check_pass("");

    check_pass("[ { #eeeeEe #ff11ff \"\" } (user) >> ]");

    check_pass(R"END([ { #eeeeEe #ff11ff '' } (user) >> ] 
          [ { #eeeeEe #bb00bb '' } (host) >> ]
          [ { #eeeeEe #ff11ff '' } (pwd_limited 35)  :> ]
          )END");

    check_pass("[ { #eeeeEe     #ff11ff '' } 'hello' >> ]");
    check_pass("[ { (rgb 0 1 0) #ff11ff '' }  (user) >> ]");
    check_pass("[ { ( rgb 0 1 0 ) #ff11ff '' }  ( user ) >> ]");
    check_pass("[{( rgb 0 1 0 )#ff11ff ''}( user )>>]");
    check_pass("   [{( rgb 0 1 0 )#ff11ff ''}( user )>>]    ");
    check_pass("[ { (rgb 255 255 255) #ff11ff '' }  (user) >> ]");
    check_pass("(if (exit) [ { #fF11Ff #ff11ff '' }  (user) >> ] [ { #fF11Ff #ff11ff '' }  (user) >> ] )");

    check_fail("[ { (rgb 255 -1 255) #ff11ff '' }  (user) >> ]");
    check_fail("[ { (rgb 255 -255 255) #ff11ff '' }  (user) >> ]");
    check_fail("[ { (rgb 255 1024 255) #ff11ff '' }  (user) >> ]");

    std::cerr << "Passed: " << passed << "\n";
    std::cerr << "Failed: " << int(fail) << "\n";
    std::cerr << "Skipped: " << skipped << "\n";

    return 0;
}
