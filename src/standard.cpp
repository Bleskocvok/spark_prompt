#include "standard.hpp"

// c
#include <cstdlib>      // getenv

// POSIX
#include <unistd.h>     // posix, geteuid, gethostname, getcwd
#include <sys/types.h>  // geteuid, getpwuid
#include <pwd.h>        // getpwuid
// #include <limits.h>     // PATH_MAX

// // only linux
// #include <sys/sysinfo.h>


using err = func::f_err;


std::variant<std::string, err> username_t::perform(segment&)
{
    struct passwd* user_info = getpwuid(geteuid());
    return user_info->pw_name;
}


std::variant<std::string, err> hostname_t::perform(segment&)
{
    char buffer[256] = { 0 };
    gethostname(buffer, sizeof(buffer));
    return std::string{ buffer };
}


std::variant<std::string, err> exit_t::perform(segment& seg)
{
    auto ok = get(0);
    auto nok = get(1);

    if (exit_code() == 0)
    {
        seg.th.bg = rgb{ 79, 125, 39 };
        return ok;
    }
    seg.th.bg = rgb{ 117, 4, 4 };
    return nok;
}


std::variant<std::string, err> pwd_t::perform(segment&)
{
    // using namespace std::string_literals;

    char* buffer = std::getenv("PWD");
    if (buffer == nullptr)
        return "";

    // char buffer[PATH_MAX] = { 0 };
    // if (getcwd(buffer, sizeof(buffer) - 1) == nullptr)
    //     return "";

    std::string result = buffer;

    struct passwd* user_info = getpwuid(geteuid());
    std::string home = user_info->pw_dir;

    if (result.substr(0, home.size()) == home)
    {
        result.erase(1, home.size() - 1);
        result.front() = '~';
    }

    // // cool arrow, but kinda wide
    // result.clear();
    // for (char ch : std::string_view(buffer))
    // {
    //     result += ch == '/' ? " ❯ "s : std::string(1, ch);
    // }

    static const size_t max_size = 35;

    if (result.size() > max_size)
    {
        result.erase(0, result.size() - max_size);
        result.insert(0, "…");
    }
    return result;
}


