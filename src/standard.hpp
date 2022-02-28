
#pragma once

#include "function.hpp"
#include "style.hpp"

// POSIX includes
#include <unistd.h>     // posix, geteuid, gethostname, getcwd
#include <sys/types.h>  // geteuid, getpwuid
#include <pwd.h>        // getpwuid
#include <limits.h>     // PATH_MAX

// // only linux
// #include <sys/sysinfo.h>


class username_t : public func
{
    std::variant<std::string, f_err> perform(segment&) override
    {
        struct passwd* user_info = getpwuid(geteuid());
        return user_info->pw_name;
    }
};


class hostname_t : public func
{
    std::variant<std::string, f_err> perform(segment&) override
    {
        char buffer[256] = { 0 };
        gethostname(buffer, sizeof(buffer));
        return std::string{ buffer };
    }
};


class exit_t : public func
{
    std::variant<std::string, f_err> perform(segment& seg) override
    {
        auto ok = get(0);
        auto nok = get(1);

        if (exit_code() == 0)
        {
            seg.bg = rgb{ 79, 125, 39 };
            return ok;
        }
        seg.bg = rgb{ 117, 4, 4 };
        return nok;
    }
};


class pwd_t : public func
{
    std::variant<std::string, f_err> perform(segment&) override
    {
        using namespace std::string_literals;

        char* buffer = getenv("PWD");
        if (buffer == nullptr)
            return "";

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
};

